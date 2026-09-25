/**
 * tilt_bar — trapezoid status bar with a slight 3D tilt toward the screen
 * centre and a horizontal gradient (dark ends, accent-tinted middle).
 */
#ifndef TILT_BAR_H
#define TILT_BAR_H

#include "lvgl.h"

/* narrow_bottom: true for the top bar (inner edge = bottom), false for the
 * bottom bar. inset = how far the inner edge is pulled in at each end. */
lv_obj_t * tilt_bar_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           int32_t inset, bool narrow_bottom);

#endif /* TILT_BAR_H */
