/**
 * ui_theme — mode accents, each a family of tones.
 *   Classic: Touring steel-blue / ice, Sport copper, Eco sage.
 *   Glass:   Touring ocean, Sport aqua, Eco teal (palette tokens).
 */
#include "ui_theme.h"

/* Classic fonts */
LV_FONT_DECLARE(font_digits_160);
LV_FONT_DECLARE(font_digits_72);
LV_FONT_DECLARE(font_digits_34);
/* Glass fonts (tools/make_fonts.py) */
LV_FONT_DECLARE(glass_text_16);
LV_FONT_DECLARE(glass_text_20);
LV_FONT_DECLARE(glass_text_24);
LV_FONT_DECLARE(glass_text_28);
LV_FONT_DECLARE(glass_text_32);
LV_FONT_DECLARE(glass_text_40);
LV_FONT_DECLARE(glass_text_48);
LV_FONT_DECLARE(glass_digits_160);
LV_FONT_DECLARE(glass_digits_72);
LV_FONT_DECLARE(glass_digits_34);

/* ---------- State ---------- */

static lv_style_t s_text, s_arc, s_glow, s_bg, s_border, s_hi, s_dim, s_disc, s_card, s_soft, s_track;
static lv_color_t s_tone[TONE_COUNT];
static lv_color_t s_accent;
static int s_mode = -1;
static ui_theme_id_t s_theme = UI_THEME_CLASSIC;

/* deep, main, light per mode, per theme */
static const uint32_t MODE_TONES[UI_THEME_COUNT][DASH_MODE_COUNT][3] = {
    [UI_THEME_CLASSIC] = {
        [DASH_MODE_TOURING] = { 0x255B86, 0x84CFE4, 0xDDF1F6 },
        [DASH_MODE_SPORT]   = { 0x8E4526, 0xE39A67, 0xF4DCC8 },
        [DASH_MODE_ECO]     = { 0x2E6547, 0x86C7A0, 0xDAEEE1 },
    },
    [UI_THEME_GLASS] = {
        [DASH_MODE_TOURING] = { PAL_OCEAN_700, PAL_OCEAN_400, PAL_AQUA_100 },
        [DASH_MODE_SPORT]   = { PAL_TEAL_600,  PAL_AQUA_300,  PAL_FOAM },
        [DASH_MODE_ECO]     = { PAL_TEAL_700,  PAL_TEAL_400,  PAL_AQUA_100 },
    },
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
    lv_style_set_shadow_width(&s_border, ui_glow(18));
    lv_style_set_shadow_opa(&s_border, (lv_opa_t)ui_glow(LV_OPA_40));
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

void ui_theme_select(ui_theme_id_t id)
{
    s_theme = id < UI_THEME_COUNT ? id : UI_THEME_CLASSIC;
    lv_style_set_shadow_width(&s_border, ui_glow(18));
    lv_style_set_shadow_opa(&s_border, (lv_opa_t)ui_glow(LV_OPA_40));
    int m = s_mode < 0 ? DASH_MODE_TOURING : s_mode;
    s_mode = -1;                       /* force re-tint */
    ui_theme_set_mode((dash_mode_t)m);
}

bool ui_theme_glass(void) { return s_theme == UI_THEME_GLASS; }

int32_t ui_glow(int32_t v) { return s_theme == UI_THEME_GLASS ? v / 2 : v; }

const lv_font_t * ui_font(int size)
{
    bool g = s_theme == UI_THEME_GLASS;
    switch(size) {
        case 16: return g ? &glass_text_16 : &lv_font_montserrat_16;
        case 20: return g ? &glass_text_20 : &lv_font_montserrat_20;
        case 24: return g ? &glass_text_24 : &lv_font_montserrat_24;
        case 28: return g ? &glass_text_28 : &lv_font_montserrat_28;
        case 32: return g ? &glass_text_32 : &lv_font_montserrat_32;
        case 40: return g ? &glass_text_40 : &lv_font_montserrat_40;
        default: return g ? &glass_text_48 : &lv_font_montserrat_48;
    }
}

const lv_font_t * ui_digits(int size)
{
    bool g = s_theme == UI_THEME_GLASS;
    switch(size) {
        case 160: return g ? &glass_digits_160 : &font_digits_160;
        case 72:  return g ? &glass_digits_72 : &font_digits_72;
        default:  return g ? &glass_digits_34 : &font_digits_34;
    }
}

void ui_theme_set_mode(dash_mode_t mode)
{
    if((int)mode == s_mode) return;
    s_mode = mode;
    s_tone[TONE_DEEP] = lv_color_hex(MODE_TONES[s_theme][mode][0]);
    s_tone[TONE_MAIN] = lv_color_hex(MODE_TONES[s_theme][mode][1]);
    s_tone[TONE_LIGHT] = lv_color_hex(MODE_TONES[s_theme][mode][2]);
    s_tone[TONE_MID] = lv_color_mix(s_tone[TONE_MAIN], s_tone[TONE_DEEP], LV_OPA_50);
    s_tone[TONE_SOFT] = lv_color_mix(s_tone[TONE_MAIN], C_DIM, LV_OPA_40);
    s_tone[TONE_TRACK] = lv_color_mix(s_tone[TONE_DEEP], C_LINE, LV_OPA_20);
    s_accent = s_tone[TONE_MAIN];
    lv_style_set_text_color(&s_soft, s_tone[TONE_SOFT]);
    lv_style_set_arc_color(&s_track, s_tone[TONE_TRACK]);
    lv_style_set_text_color(&s_text, s_accent);
    lv_style_set_arc_color(&s_arc, s_accent);
    lv_style_set_arc_color(&s_glow, s_tone[TONE_DEEP]);
    lv_style_set_arc_opa(&s_glow, (lv_opa_t)ui_glow(LV_OPA_30));
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
