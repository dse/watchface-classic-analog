#ifndef WATCH_H
#define WATCH_H

static void main_window_create();
static void main_window_destroy();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);

/* Persistent storage key */
#define SETTINGS_KEY 1

typedef struct WatchSettings {
    bool show_date;
    bool show_battery;
    bool use_bold_font;
    bool use_larger_font;
} WatchSettings;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ROUND(a) (int)((a) + 0.5)

#endif
