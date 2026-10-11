/**
 * frame_line — glowing full-width line that dips ("pinches") in the middle
 * to cradle the top / bottom bar. Colour follows the mode accent.
 */
#ifndef FRAME_LINE_H
#define FRAME_LINE_H

#include "lvgl.h"

/* y_out: height at the screen sides, y_in: height inside the notch
 * (x1..x2), curve: width of each S-bend. */
lv_obj_t * frame_line_create(lv_obj_t * parent, int32_t y_out, int32_t y_in,
                             int32_t x1, int32_t x2, int32_t curve);

/* Key-on trace: show the middle `permille` of the line (1000 = all). */
void frame_line_set_reveal(lv_obj_t * obj, int32_t permille);

#endif /* FRAME_LINE_H */
