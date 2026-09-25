/**
 * tilt_bar — trapezoid bar with a slight 3D tilt (top / bottom status bars).
 */
#ifndef TILT_BAR_H
#define TILT_BAR_H

#include "lvgl.h"

typedef enum {
    TILT_BAR_TOP,     /* wide at top edge, narrower at bottom */
    TILT_BAR_BOTTOM,  /* narrow at top edge, wider at bottom */
} tilt_bar_dir_t;

/* Create a tilted bar. `inset` = how many px each side slants in. */
lv_obj_t * tilt_bar_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           int32_t inset, tilt_bar_dir_t dir);

#endif /* TILT_BAR_H */
