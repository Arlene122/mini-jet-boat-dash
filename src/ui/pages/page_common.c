/**
 * page_common — helpers shared by pages.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"

lv_obj_t * page_row(lv_obj_t * parent, int32_t x, int32_t y, const char * caption,
                    const lv_font_t * font)
{
    lv_obj_t * c = ui_caption(parent, caption);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(c, x, y);
    lv_obj_t * v = ui_label(parent, font, C_TEXT, "--");
    lv_obj_set_pos(v, x, y + 22);
    return v;
}
