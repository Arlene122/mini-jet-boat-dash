/**
 * ui_theme — mode accents, each a family of muted "luxury" tones:
 * Touring steel-blue / ice, Sport copper, Eco sage.
 */
#include "ui_theme.h"

/* ---------- State ---------- */

static lv_style_t s_text, s_arc, s_glow, s_bg, s_border, s_hi, s_dim, s_disc, s_card, s_soft, s_track;
static lv_color_t s_tone[TONE_COUNT];
static lv_color_t s_accent;
static int s_mode = -1;

/* deep, main, light per mode */
static const uint32_t MODE_TONES[DASH_MODE_COUNT][3] = {
    [DASH_MODE_TOURING] = { 0x255B86, 0x84CFE4, 0xDDF1F6 },
    [DASH_MODE_SPORT]   = { 0x8E4526, 0xE39A67, 0xF4DCC8 },
    [DASH_MODE_ECO]     = { 0x2E6547, 0x86C7A0, 0xDAEEE1 },
};

/* ---------- API ---------- */

void ui_theme_init(void)
{
    lv_style_init(&s_text);
    lv_style_init(&s_arc);
    lv_style_init(&s_glow);
    lv_style_init(&s_bg);
    lv_style_set_bg_opa(&s_bg, LV_OPA_COVER);
    lv_style_init(&s_border);
    lv_style_set_border_width(&s_border, 2);
    lv_style_set_shadow_width(&s_border, 18);
    lv_style_set_shadow_opa(&s_border, LV_OPA_40);
    lv_style_init(&s_soft);
    lv_style_init(&s_track);
    lv_style_init(&s_hi);
    lv_style_init(&s_dim);
    lv_style_init(&s_disc);
    lv_style_set_bg_opa(&s_disc, LV_OPA_COVER);
    lv_style_init(&s_card);
    lv_style_set_bg_opa(&s_card, LV_OPA_COVER);
    ui_theme_set_mode(DASH_MODE_TOURING);
}

void ui_theme_set_mode(dash_mode_t mode)
{
    if((int)mode == s_mode) return;
    s_mode = mode;
    s_tone[TONE_DEEP] = lv_color_hex(MODE_TONES[mode][0]);
    s_tone[TONE_MAIN] = lv_color_hex(MODE_TONES[mode][1]);
    s_tone[TONE_LIGHT] = lv_color_hex(MODE_TONES[mode][2]);
    s_tone[TONE_MID] = lv_color_mix(s_tone[TONE_MAIN], s_tone[TONE_DEEP], LV_OPA_50);
    s_tone[TONE_SOFT] = lv_color_mix(s_tone[TONE_MAIN], C_DIM, LV_OPA_40);
    s_tone[TONE_TRACK] = lv_color_mix(s_tone[TONE_DEEP], C_LINE, LV_OPA_20);
    s_accent = s_tone[TONE_MAIN];
    lv_style_set_text_color(&s_soft, s_tone[TONE_SOFT]);
    lv_style_set_arc_color(&s_track, s_tone[TONE_TRACK]);
    lv_style_set_text_color(&s_text, s_accent);
    lv_style_set_arc_color(&s_arc, s_accent);
    lv_style_set_arc_color(&s_glow, s_tone[TONE_DEEP]);
    lv_style_set_arc_opa(&s_glow, LV_OPA_30);
    lv_style_set_bg_color(&s_bg, s_accent);
    lv_style_set_shadow_color(&s_bg, s_accent);
    lv_style_set_border_color(&s_border, s_accent);
    lv_style_set_shadow_color(&s_border, s_accent);
    lv_style_set_arc_color(&s_hi, s_tone[TONE_LIGHT]);
    lv_style_set_arc_color(&s_dim, lv_color_mix(s_tone[TONE_DEEP], C_BG, LV_OPA_60));
    /* Solid fills: dark low-contrast gradients band visibly */
    lv_style_set_bg_color(&s_disc, lv_color_mix(s_tone[TONE_DEEP], C_BG, (lv_opa_t)26));
    lv_style_set_bg_color(&s_card, lv_color_mix(s_accent, C_PANEL, (lv_opa_t)14));
    lv_obj_report_style_change(NULL);
}

lv_color_t ui_theme_accent(void) { return s_accent; }
lv_color_t ui_theme_tone(ui_tone_t t) { return s_tone[t]; }
lv_style_t * ui_style_soft_text(void) { return &s_soft; }
lv_style_t * ui_style_track_arc(void) { return &s_track; }
lv_color_t ui_theme_accent_mix(lv_opa_t amount) { return lv_color_mix(s_accent, C_BG, amount); }
lv_style_t * ui_style_accent_hi(void) { return &s_hi; }
lv_style_t * ui_style_accent_dim(void) { return &s_dim; }
lv_style_t * ui_style_disc(void) { return &s_disc; }
lv_style_t * ui_style_card(void) { return &s_card; }
lv_style_t * ui_style_accent_text(void) { return &s_text; }
lv_style_t * ui_style_accent_arc(void) { return &s_arc; }
lv_style_t * ui_style_accent_glow(void) { return &s_glow; }
lv_style_t * ui_style_accent_bg(void) { return &s_bg; }
lv_style_t * ui_style_accent_border(void) { return &s_border; }
