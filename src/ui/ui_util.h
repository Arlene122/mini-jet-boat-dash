/**
 * ui_util — small helpers shared by screens and pages.
 */
#ifndef UI_UTIL_H
#define UI_UTIL_H

#include "lvgl.h"

/* Label with font + colour */
lv_obj_t * ui_label(lv_obj_t * parent, const lv_font_t * font, lv_color_t color, const char * text);

/* Small dim, letter-spaced caption ("FUEL", "RPM") */
lv_obj_t * ui_caption(lv_obj_t * parent, const char * text);

/* printf into a label, but only touch it if the text changed (no redraw) */
void ui_label_printf(lv_obj_t * label, const char * fmt, ...);

/* Change text colour only if different */
void ui_set_text_color(lv_obj_t * obj, lv_color_t c);

/* Plain transparent container without styles */
lv_obj_t * ui_box(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h);

/* Hairline that fades out at both ends (vertical or horizontal) */
lv_obj_t * ui_fade_line(lv_obj_t * parent, int32_t x, int32_t y, int32_t len, bool vertical, lv_opa_t peak);

/* Slim progress bar (accent indicator) */
lv_obj_t * ui_slim_bar(lv_obj_t * parent, int32_t w, int32_t h, int32_t max);

#endif /* UI_UTIL_H */
