/**
 * tilt_bar — trapezoid bar with a slight 3D tilt that follows the bezel's
 * slanted edges (top / bottom status bars).
 */
#ifndef TILT_BAR_H
#define TILT_BAR_H

#include "lvgl.h"

/* wide_top: true = wide edge on top (bottom bar), false = wide at bottom
 * (top bar). inset_l / inset_r = how far the narrow edge slants in. */
lv_obj_t * tilt_bar_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           int32_t inset_l, int32_t inset_r, bool wide_top);

#endif /* TILT_BAR_H */
