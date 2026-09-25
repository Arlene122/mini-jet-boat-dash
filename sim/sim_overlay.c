/**
 * sim_overlay — stencil image on the top layer + help panel.
 *   S: stencil opacity 100% -> 50% -> off
 *   H: show / hide key help
 */
#include "sim_overlay.h"

#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH

LV_IMAGE_DECLARE(stencil_img);   /* generated from assets/ by tools/stencil_to_c.py */

/* ---------- State ---------- */

static lv_obj_t * s_stencil;
static lv_obj_t * s_help;
static int s_stencil_step;       /* 0 = 100%, 1 = 50%, 2 = off */

static const char * HELP_KEYS =
    "Up / Down\nSpace\nLeft / Right\nF N R B\nM\nK\n1 2 3 4 5\n0\nA\nS\nH";
static const char * HELP_TEXT =
    "throttle +/-\n"
    "throttle off\n"
    "fuel -/+ 5%\n"
    "iBR fwd / neutral / rev / brake\n"
    "riding mode\n"
    "DESS key on/off\n"
    "check eng / oil / overheat / low fuel / low batt\n"
    "clear warnings\n"
    "auto demo\n"
    "stencil 100% / 50% / off\n"
    "hide this help";

/* ---------- Helpers ---------- */

static void apply_stencil(void)
{
    static const lv_opa_t opa[] = { LV_OPA_COVER, LV_OPA_50, LV_OPA_TRANSP };
    lv_obj_set_hidden(s_stencil, s_stencil_step == 2);
    lv_obj_set_style_image_opa(s_stencil, opa[s_stencil_step], 0);
}

/* ---------- API ---------- */

void sim_overlay_init(void)
{
    lv_obj_t * top = lv_layer_top();

    s_stencil = lv_image_create(top);
    lv_image_set_src(s_stencil, &stencil_img);
    lv_obj_set_pos(s_stencil, 0, 0);
    apply_stencil();

    /* Help panel: title + two columns (keys | action) */
    s_help = lv_obj_create(top);
    lv_obj_remove_style_all(s_help);
    lv_obj_set_size(s_help, 900, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(s_help, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_help, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(s_help, 24, 0);
    lv_obj_set_style_pad_column(s_help, 32, 0);
    lv_obj_set_style_radius(s_help, 12, 0);
    lv_obj_set_style_text_font(s_help, &lv_font_montserrat_24, 0);
    lv_obj_set_flex_flow(s_help, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * title = lv_label_create(s_help);
    lv_label_set_text(title, "FAKE SPARK ECU - KEYS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x2F80ED), 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t * keys = lv_label_create(s_help);
    lv_label_set_text(keys, HELP_KEYS);
    lv_obj_set_style_text_color(keys, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t * text = lv_label_create(s_help);
    lv_label_set_text(text, HELP_TEXT);
    lv_obj_set_style_text_color(text, lv_color_hex(0xB8C2CE), 0);
    lv_obj_center(s_help);
}

bool sim_overlay_key(int key)
{
    switch(key) {
        case SDLK_s:
            s_stencil_step = (s_stencil_step + 1) % 3;
            apply_stencil();
            return true;
        case SDLK_h:
            lv_obj_set_hidden(s_help, !lv_obj_is_hidden(s_help));
            return true;
        default:
            return false;
    }
}
