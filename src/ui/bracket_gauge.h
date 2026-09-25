/**
 * bracket_gauge — slim angled "bracket" bar gauge that fills from its foot
 * upward along the path, with a glowing fill and a bright head.
 */
#ifndef BRACKET_GAUGE_H
#define BRACKET_GAUGE_H

#include "lvgl.h"

typedef enum { BRACKET_NORMAL = 0, BRACKET_WARN, BRACKET_CRIT } bracket_state_t;

/* pts (screen coords): top-inner, outer corner, bottom-outer, foot-inner.
 * Fill starts at the foot. */
lv_obj_t * bracket_gauge_create(lv_obj_t * parent, const lv_point_t pts[4]);

/* permille 0..1000; redraws only if value or state changed */
void bracket_gauge_set(lv_obj_t * obj, int32_t permille, bracket_state_t st);

#endif /* BRACKET_GAUGE_H */
