#include <pebble.h>
#include "analog-watchface.h"

#define app__log(a) app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, a);

static Window *s_main_window;
static Layer *s_ticks_layer;
static Layer *s_wall_time_layer;

static TextLayer *s_batt_text_layer;
static TextLayer *s_date_text_layer;

static GFont s_font;

static int minute_when_last_updated;

#define TICK_RADIUS       68
#define SECOND_RADIUS     64
#define MINUTE_RADIUS     56
#define HOUR_RADIUS       36

#define OPTION_SHOW_DATE       0
#define OPTION_SHOW_BATTERY    1
#define OPTION_USE_BOLD_FONT   2
#define OPTION_USE_LARGER_FONT 3

static bool show_date;
static bool show_battery;
static bool use_bold_font;
static bool use_larger_font;

static GRect bounds;
static GPoint center;

GPoint tick_angle_point(GPoint center, int radius, int angle) {
    int x = center.x + (int)(radius * 1.0 * sin_lookup(angle) / TRIG_MAX_RATIO + 0.5);
    int y = center.y - (int)(radius * 1.0 * cos_lookup(angle) / TRIG_MAX_RATIO + 0.5);
    return GPoint(x, y);
}

GPoint tick_point(GPoint center, int radius, int degrees) {
    int angle = (int)(TRIG_MAX_ANGLE * degrees / 360.0 + 0.5);
    return tick_angle_point(center, radius, angle);
}

void draw_ticks(GContext *ctx, GPoint center, int radius, int num_ticks, int ticks_modulo, int thick) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
    for (int i = 0; i < num_ticks; i += 1) {
        GPoint p = tick_point(center, radius, i * 360 / num_ticks);
        if (i % ticks_modulo == 0) {
            if (thick) {
                graphics_fill_rect(ctx, GRect(p.x - 1, p.y - 1, 3, 3), 0, GCornerNone);
            } else {
                GPoint p1 = tick_point(center, radius + 1, i * 360 / num_ticks);
                GPoint p2 = tick_point(center, radius - 1, i * 360 / num_ticks);
                graphics_draw_line(ctx, p1, p2);
            }
        } else {
            graphics_draw_pixel(ctx, p);
        }
    }
}

static void ticks_update_proc(Layer *layer, GContext *ctx) {
    draw_ticks(ctx, center, TICK_RADIUS, 60, 5, 1);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    int second_angle = (int)(TRIG_MAX_ANGLE / 60      *                                            t->tm_sec  + 0.5);
    int minute_angle = (int)(TRIG_MAX_ANGLE / 3600.0  * (                         t->tm_min * 60 + t->tm_sec) + 0.5);
    int hour_angle   = (int)(TRIG_MAX_ANGLE / 43200.0 * (t->tm_hour % 12 * 3600 + t->tm_min * 60 + t->tm_sec) + 0.5);

    GPoint second = tick_angle_point(center, SECOND_RADIUS, second_angle);
    GPoint minute = tick_angle_point(center, MINUTE_RADIUS, minute_angle);
    GPoint hour   = tick_angle_point(center, HOUR_RADIUS,   hour_angle);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
  
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, center, second);

    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, center, minute);
    graphics_draw_line(ctx, center, hour);
}

static void update_date(struct tm *tick_time) {
    static char date_buffer[] = "WED 12/31";
    if (minute_when_last_updated != tick_time->tm_min) {
        strftime(date_buffer, sizeof(date_buffer), "%a %m/%d", tick_time);
        text_layer_set_text(s_date_text_layer, date_buffer);
    }
    minute_when_last_updated = tick_time->tm_min;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_wall_time_layer);
    if (show_date) {
        update_date(tick_time);
    }
}

static void on_battery_state_change(BatteryChargeState charge) {
    static char buffer[] = "100%C";
    int l;
  
    snprintf(buffer, sizeof(buffer), "%d%%", charge.charge_percent);
    if (charge.is_charging) {
        l = strlen(buffer);
        strncpy(buffer + l, "C", sizeof(buffer) - l);
    }
    text_layer_set_text(s_batt_text_layer, buffer);
}

static void message_handler(DictionaryIterator *received, void *context) {
    bool refresh_window = 0;
    Tuple *tuple_show_date         = dict_find(received, OPTION_SHOW_DATE);
    Tuple *tuple_show_battery      = dict_find(received, OPTION_SHOW_BATTERY);
    Tuple *tuple_use_bold_font     = dict_find(received, OPTION_USE_BOLD_FONT);
    Tuple *tuple_use_larger_font   = dict_find(received, OPTION_USE_LARGER_FONT);

    if (tuple_show_date) {
        refresh_window = 1;
        show_date = (bool)tuple_show_date->value->int32;
    }
    if (tuple_show_battery) {
        refresh_window = 1;
        show_battery = (bool)tuple_show_battery->value->int32;
    }
    if (tuple_use_bold_font) {
        refresh_window = 1;
        use_bold_font = (bool)tuple_use_bold_font->value->int32;
    }
    if (tuple_use_larger_font) {
        refresh_window = 1;
        use_larger_font = (bool)tuple_use_larger_font->value->int32;
    }

    persist_write_bool(OPTION_SHOW_DATE,       show_date);
    persist_write_bool(OPTION_SHOW_BATTERY,    show_battery);
    persist_write_bool(OPTION_USE_BOLD_FONT,   use_bold_font);
    persist_write_bool(OPTION_USE_LARGER_FONT, use_larger_font);

    if (refresh_window) {
        main_window_destroy();
        main_window_create();
    }
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    static BatteryChargeState battery_state;

    show_date = 0;
    show_battery = 0;
    use_bold_font = 0;
    use_larger_font = 0;

    if (persist_exists(OPTION_SHOW_DATE)) {
        show_date = persist_read_bool(OPTION_SHOW_DATE);
    }
    if (persist_exists(OPTION_SHOW_BATTERY)) {
        show_battery = persist_read_bool(OPTION_SHOW_BATTERY);
    }
    if (persist_exists(OPTION_USE_BOLD_FONT)) {
        use_bold_font = persist_read_bool(OPTION_USE_BOLD_FONT);
    }
    if (persist_exists(OPTION_USE_LARGER_FONT)) {
        use_larger_font = persist_read_bool(OPTION_USE_LARGER_FONT);
    }

    bounds = layer_get_bounds(window_layer);
    if (show_date || show_battery) {
        if (use_larger_font) {
            bounds.origin.y += 23; /* for font size = 18 */
        } else {
            bounds.origin.y += 21; /* for font size = 14 */
        }
    } else {
        bounds.origin.y += 14;
    }
    bounds.size.h   -= 28;
    center = grect_center_point(&bounds);

    window_set_background_color(window, GColorBlack);

    s_ticks_layer = layer_create(bounds);
    layer_set_update_proc(s_ticks_layer, ticks_update_proc);
    layer_add_child(window_layer, s_ticks_layer);

    s_wall_time_layer = layer_create(bounds);
    layer_set_update_proc(s_wall_time_layer, canvas_update_proc);
    layer_add_child(window_layer, s_wall_time_layer);

    s_date_text_layer = NULL;
    s_batt_text_layer = NULL;

    if (use_bold_font) {
        if (use_larger_font) {
            s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        } else {
            s_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
        }
    } else {
        if (use_larger_font) {
            s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
        } else {
            s_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
        }
    }

    if (show_date) {
        s_date_text_layer = text_layer_create(GRect(0, 0, 90, use_larger_font ? 18 : 14));
        text_layer_set_background_color(s_date_text_layer, GColorBlack);
        text_layer_set_text_color(s_date_text_layer, GColorWhite);
        text_layer_set_font(s_date_text_layer, s_font);
        text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentLeft);
        layer_add_child(window_layer, text_layer_get_layer(s_date_text_layer));
    }

    if (show_battery) {
        s_batt_text_layer = text_layer_create(GRect(90, 0, 54, use_larger_font ? 18 : 14));
        text_layer_set_background_color(s_batt_text_layer, GColorBlack);
        text_layer_set_text_color(s_batt_text_layer, GColorWhite);
        text_layer_set_font(s_batt_text_layer, s_font);
        text_layer_set_text_alignment(s_batt_text_layer, GTextAlignmentRight);
        layer_add_child(window_layer, text_layer_get_layer(s_batt_text_layer));
    }

    bounds = layer_get_bounds(s_ticks_layer);
    center = grect_center_point(&bounds);

    minute_when_last_updated = -1;

    layer_mark_dirty(s_wall_time_layer);
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    if (show_battery) {
        battery_state = battery_state_service_peek();
        on_battery_state_change(battery_state);
        battery_state_service_subscribe(on_battery_state_change);
    }
}

static void main_window_unload(Window *window) {
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();
    minute_when_last_updated = -1;
  
    if (s_date_text_layer) {
        text_layer_destroy(s_date_text_layer);
        s_date_text_layer = NULL;
    }
    if (s_batt_text_layer) {
        text_layer_destroy(s_batt_text_layer);
        s_batt_text_layer = NULL;
    }
    layer_destroy(s_wall_time_layer);
}

static void main_window_create() {
    s_main_window = window_create();
    WindowHandlers wh = {
	.load   = main_window_load,
        .unload = main_window_unload
    };
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, wh);
    window_stack_push(s_main_window, true);
}

static void main_window_destroy() {
    window_stack_pop(true);
    window_destroy(s_main_window);
}

static void init(void) {
    main_window_create();
    app_message_open(app_message_inbox_size_maximum(),
                     app_message_outbox_size_maximum());
    app_message_register_inbox_received(message_handler);
}

static void deinit(void) {
    app_message_deregister_callbacks();
    main_window_destroy();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

