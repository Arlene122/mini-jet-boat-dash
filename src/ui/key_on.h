/**
 * key_on — cluster start-up animation (~2 s) played when the screen first
 * comes on: frame lines trace out from the centre, the speed ring scales
 * up, bars unfold, side gauges and panels glide in. The arc sweep (ui.c)
 * runs on top. The warning banner is never part of it, so warnings always
 * show. Not the splash screen.
 */
#ifndef KEY_ON_H
#define KEY_ON_H

#include "lvgl.h"

typedef enum {
    KO_BG,         /* Glass backdrop: fades in */
    KO_FRAME,      /* frame lines: light trace from the centre out */
    KO_BAR,        /* top / bottom bars: unfold from the centre */
    KO_GAUGE,      /* speed ring (+ Glass disc): scale up */
    KO_BRACKETS,   /* RPM / FUEL gauges: slide out from behind the ring */
    KO_SIDE,       /* side panels (+ Glass plates): glide in from the edges */
    KO_COUNT
} key_on_group_t;

/* Called while the screen is built. key_on_reset() first, then each
 * widget (or every child of scr created since child index `from`). */
void key_on_reset(void);
void key_on_add(key_on_group_t g, lv_obj_t * obj);
void key_on_add_from(key_on_group_t g, lv_obj_t * scr, uint32_t from);

/* Start the animation (call right after the screen is loaded). */
void key_on_play(void);

/* When the arc sweep should start, relative to key_on_play(). */
#define KEY_ON_SWEEP_DELAY_MS  650

#endif /* KEY_ON_H */
