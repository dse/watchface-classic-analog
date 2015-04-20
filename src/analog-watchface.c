/*
 * main.c
 * Creates a Window, Layer and assigns an `update_proc` to draw 
 * the 'P' in the Pebble logo.
 */

#include <pebble.h>

static Window *s_main_window;
static Layer *s_ticks_layer;
static Layer *s_canvas_layer;

#define TICK_RADIUS       68
#define SECOND_RADIUS     64
#define MINUTE_RADIUS     56
#define HOUR_RADIUS       36
#define MINUTE_HAND_WIDTH 2
#define HOUR_HAND_WIDTH   2

static const GPathInfo MINUTE_HAND_POINTS = {
  4,
  (GPoint []) {
    { MINUTE_HAND_WIDTH, MINUTE_HAND_WIDTH },
    { -MINUTE_HAND_WIDTH, MINUTE_HAND_WIDTH },
    { -MINUTE_HAND_WIDTH, -MINUTE_RADIUS },
    { MINUTE_HAND_WIDTH, -MINUTE_RADIUS }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  4,
  (GPoint []){
    { HOUR_HAND_WIDTH, HOUR_HAND_WIDTH },
    { -HOUR_HAND_WIDTH, HOUR_HAND_WIDTH },
    { -HOUR_HAND_WIDTH, -HOUR_RADIUS },
    { HOUR_HAND_WIDTH, -HOUR_RADIUS }
  }
};

static GPath *s_minute_arrow, *s_hour_arrow;

static GRect bounds;
static GPoint center;
static GPoint center1, center2, center3;
static int radius1;

GPoint tick_point (GPoint center, int radius, int degrees) {
  int angle = (int)(TRIG_MAX_ANGLE * degrees / 360.0 + 0.5);
  int x = center.x + (int)(radius * 1.0 * sin_lookup(angle) / TRIG_MAX_RATIO + 0.5);
  int y = center.y - (int)(radius * 1.0 * cos_lookup(angle) / TRIG_MAX_RATIO + 0.5);
  return GPoint(x, y);
}

void draw_ticks (GContext *ctx, GPoint center, int radius, int num_ticks, int ticks_modulo, int thick) {
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
  draw_ticks(ctx, center1, radius1, 20, 4, 1);
  draw_ticks(ctx, center2, radius1, 60, 5, 0);
  draw_ticks(ctx, center3, radius1, 60, 5, 0);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  int second_angle = (int)(TRIG_MAX_ANGLE * t->tm_sec / 60.0    + 0.5);
  GPoint second = tick_point(center, SECOND_RADIUS, t->tm_sec * 6);
  graphics_draw_line(ctx, center, second);

  int minute_angle = (int)(TRIG_MAX_ANGLE * (                         t->tm_min * 60 + t->tm_sec) / 3600.0  + 0.5);
  int hour_angle   = (int)(TRIG_MAX_ANGLE * (t->tm_hour % 12 * 3600 + t->tm_min * 60 + t->tm_sec) / 43200.0 + 0.5);

  gpath_rotate_to(s_minute_arrow, minute_angle);
  //  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  gpath_rotate_to(s_hour_arrow, hour_angle);
  //  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  bounds = layer_get_bounds(window_layer);
  center = grect_center_point(&bounds);

  center1 = tick_point(center, TICK_RADIUS / 2,   0);
  center2 = tick_point(center, TICK_RADIUS / 2, 120);
  center3 = tick_point(center, TICK_RADIUS / 2, 240);
  radius1 = (int)(TICK_RADIUS / 2.5 + 0.5);

  window_set_background_color(window, GColorBlack);

  s_ticks_layer = layer_create(bounds);
  layer_set_update_proc(s_ticks_layer, ticks_update_proc);
  layer_add_child(window_layer, s_ticks_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
  // Destroy Layer
  layer_destroy(s_canvas_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
	.unload = main_window_unload,
	});
  window_stack_push(s_main_window, true);
  
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

