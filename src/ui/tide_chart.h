/**
 * tide_chart — one tide cycle as a smooth curve: past dimmed, future in
 * accent, a dot at "now", low/high times labelled at the extremes.
 */
#ifndef TIDE_CHART_H
#define TIDE_CHART_H

#include "lvgl.h"

lv_obj_t * tide_chart_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h);

/* phase: 0 = low, 0.5 = high. now_min: minutes since midnight (-1 = unknown).
 * period_min: low-to-low. Redraws only when the shown minute changes. */
void tide_chart_set(lv_obj_t * obj, float phase, int32_t now_min, int32_t period_min, bool clock_12h);

#endif /* TIDE_CHART_H */
