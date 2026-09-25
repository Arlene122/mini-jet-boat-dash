/**
 * ui_theme — mode accents: Touring ice-cyan, Sport warm orange,
 * Eco green.
 */
#include "ui_theme.h"

/* ---------- State ---------- */

static lv_style_t s_text, s_arc, s_glow, s_bg, s_border;
static lv_color_t s_accent;
static int s_mode = -1;

static const uint32_t MODE_ACCENT[DASH_MODE_COUNT] = {
    [DASH_MODE_TOURING] = 0x6FE3FF,
    [DASH_MODE_SPORT]   = 0xFF8A3D,
    [DASH_MODE_ECO]     = 0x5EE08A,
};

/* ---------- API ---------- */

void ui_theme_init(void)
{
    lv_style_init(&s_text);
    lv_style_init(&s_arc);
    lv_style_init(&s_glow);
    lv_style_set_arc_opa(&s_glow, LV_OPA_20);
    lv_style_init(&s_bg);
    lv_style_set_bg_opa(&s_bg, LV_OPA_COVER);
    lv_style_init(&s_border);
    lv_style_set_border_width(&s_border, 2);
    lv_style_set_shadow_width(&s_border, 18);
    lv_style_set_shadow_opa(&s_border, LV_OPA_40);
    ui_theme_set_mode(DASH_MODE_TOURING);
}

void ui_theme_set_mode(dash_mode_t mode)
{
    if((int)mode == s_mode) return;
    s_mode = mode;
    s_accent = lv_color_hex(MODE_ACCENT[mode]);
    lv_style_set_text_color(&s_text, s_accent);
    lv_style_set_arc_color(&s_arc, s_accent);
    lv_style_set_arc_color(&s_glow, s_accent);
    lv_style_set_bg_color(&s_bg, s_accent);
    lv_style_set_shadow_color(&s_bg, s_accent);
    lv_style_set_border_color(&s_border, s_accent);
    lv_style_set_shadow_color(&s_border, s_accent);
    lv_obj_report_style_change(NULL);
}

lv_color_t ui_theme_accent(void) { return s_accent; }
lv_style_t * ui_style_accent_text(void) { return &s_text; }
lv_style_t * ui_style_accent_arc(void) { return &s_arc; }
lv_style_t * ui_style_accent_glow(void) { return &s_glow; }
lv_style_t * ui_style_accent_bg(void) { return &s_bg; }
lv_style_t * ui_style_accent_border(void) { return &s_border; }
