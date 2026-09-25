/**
 * page_common — helpers shared by pages.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"

lv_obj_t * page_stat(lv_obj_t * parent, int32_t x, int32_t y, const char * caption,
                     const lv_font_t * font)
{
    lv_obj_t * c = ui_caption(parent, caption);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(c, x, y);
    lv_obj_t * v = ui_label(parent, font, C_TEXT, "--");
    lv_obj_set_pos(v, x, y + 20);
    return v;
}

lv_obj_t * page_list_row(lv_obj_t * parent, int32_t y, const char * caption, lv_obj_t ** row)
{
    lv_obj_t * r = ui_box(parent, 0, y, PAGE_CONTENT_W, LIST_ROW_H);
    lv_obj_t * c = ui_caption(r, caption);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_align(c, LV_ALIGN_LEFT_MID, 14, 0);
    lv_obj_t * v = ui_label(r, &lv_font_montserrat_24, C_TEXT, "--");
    lv_obj_align(v, LV_ALIGN_RIGHT_MID, 0, 0);
    ui_fade_line(parent, 0, y + LIST_ROW_H, PAGE_CONTENT_W, false, LV_OPA_30);
    if(row) *row = r;
    return v;
}
