/**
 * sim_fonts — Outfit / Sora / Inter side by side, same samples, so the
 * owner can pick the Glass font. Regenerate with:
 *   python3 tools/make_fonts.py <outfit|sora|inter>
 */
#include "sim_fonts.h"

#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH

LV_FONT_DECLARE(cmp_outfit_digits_160);
LV_FONT_DECLARE(cmp_outfit_text_28);
LV_FONT_DECLARE(cmp_sora_digits_160);
LV_FONT_DECLARE(cmp_sora_text_28);
LV_FONT_DECLARE(cmp_inter_digits_160);
LV_FONT_DECLARE(cmp_inter_text_28);

static lv_obj_t * s_panel;

static void column(lv_obj_t * p, int i, const char * name, const lv_font_t * dig, const lv_font_t * txt)
{
    lv_obj_t * c = lv_obj_create(p);
    lv_obj_remove_style_all(c);
    lv_obj_set_size(c, 560, 600);
    lv_obj_set_pos(c, 40 + i * 600, 50);

    lv_obj_t * t = lv_label_create(c);
    lv_label_set_text(t, name);
    lv_obj_set_style_text_font(t, txt, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(0x5FE0E0), 0);

    lv_obj_t * d = lv_label_create(c);
    lv_label_set_text(d, "108");
    lv_obj_set_style_text_font(d, dig, 0);
    lv_obj_set_style_text_color(d, lv_color_hex(0xF2FBFC), 0);
    lv_obj_set_pos(d, 0, 50);

    lv_obj_t * s = lv_label_create(c);
    lv_label_set_text(s, "KM/H    6450 RPM\nTOURING    12:45\nTrip 23.4 km   42.0 h\nOcean Drive - Duke Dumont");
    lv_obj_set_style_text_font(s, txt, 0);
    lv_obj_set_style_text_color(s, lv_color_hex(0xC9D4DC), 0);
    lv_obj_set_style_text_line_space(s, 10, 0);
    lv_obj_set_pos(s, 0, 260);
}

bool sim_fonts_key(int key)
{
    if(key != SDLK_g) return false;
    if(s_panel) {
        lv_obj_delete(s_panel);
        s_panel = NULL;
        return true;
    }
    s_panel = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_panel);
    lv_obj_set_size(s_panel, 1920, 720);
    lv_obj_set_style_bg_color(s_panel, lv_color_hex(0x071426), 0);
    lv_obj_set_style_bg_opa(s_panel, LV_OPA_COVER, 0);
    column(s_panel, 0, "OUTFIT", &cmp_outfit_digits_160, &cmp_outfit_text_28);
    column(s_panel, 1, "SORA", &cmp_sora_digits_160, &cmp_sora_text_28);
    column(s_panel, 2, "INTER", &cmp_inter_digits_160, &cmp_inter_text_28);
    return true;
}
