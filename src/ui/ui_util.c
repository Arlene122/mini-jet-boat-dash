/**
 * ui_util — helpers.
 */
#include "ui_util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ui_theme.h"

lv_obj_t * ui_label(lv_obj_t * parent, const lv_font_t * font, lv_color_t color, const char * text)
{
    lv_obj_t * l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    lv_label_set_text(l, text);
    return l;
}

lv_obj_t * ui_caption(lv_obj_t * parent, const char * text)
{
    lv_obj_t * l = ui_label(parent, &lv_font_montserrat_20, C_DIM, text);
    lv_obj_set_style_text_letter_space(l, 3, 0);
    return l;
}

void ui_label_printf(lv_obj_t * label, const char * fmt, ...)
{
    char buf[96];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if(strcmp(lv_label_get_text(label), buf) != 0) lv_label_set_text(label, buf);
}

void ui_set_text_color(lv_obj_t * obj, lv_color_t c)
{
    if(!lv_color_eq(lv_obj_get_style_text_color(obj, 0), c)) lv_obj_set_style_text_color(obj, c, 0);
}

lv_obj_t * ui_box(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_scrollable(o, false);
    lv_obj_set_clickable(o, false);
    return o;
}

lv_obj_t * ui_slim_bar(lv_obj_t * parent, int32_t w, int32_t h, int32_t max)
{
    lv_obj_t * b = lv_bar_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_size(b, w, h);
    lv_bar_set_range(b, 0, max);
    lv_obj_set_style_bg_color(b, C_OFF, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(b, h / 2, LV_PART_MAIN);
    lv_obj_set_style_radius(b, h / 2, LV_PART_INDICATOR);
    lv_obj_add_style(b, ui_style_accent_bg(), LV_PART_INDICATOR);
    return b;
}
