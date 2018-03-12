#ifndef ANALOG_WATCHFACE_H
#define ANALOG_WATCHFACE_H

static void main_window_create();
static void main_window_destroy();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ROUND(a) (int)((a) + 0.5)

#endif
