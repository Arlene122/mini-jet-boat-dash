/**
 * ui_theme — palette, theme (Classic / Glass), fonts and the accent tone
 * family that tints with the riding mode. Widgets add the shared styles;
 * changing mode re-colours them all. Changing theme rebuilds the screen.
 */
#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

/* ---------- Palette ---------- */
/* ---------- Glass palette tokens: blue-teal spectrum ---------- */
#define PAL_NAVY_950    0x040A14
#define PAL_NAVY_900    0x071426
#define PAL_NAVY_800    0x0B2140
#define PAL_OCEAN_700   0x123A66
#define PAL_OCEAN_600   0x1B5A8F
#define PAL_OCEAN_500   0x2A7DBA
#define PAL_OCEAN_400   0x4FA3DB
#define PAL_TEAL_700    0x0B5C5E
#define PAL_TEAL_600    0x0F7C80
#define PAL_TEAL_500    0x16A3A3
#define PAL_TEAL_400    0x2CC4BE
#define PAL_AQUA_300    0x5FE0E0
#define PAL_AQUA_200    0x9CEFF0
#define PAL_AQUA_100    0xD6FAFA
#define PAL_FOAM        0xF2FBFC
#define PAL_PERIWINKLE  0x8C9EFF   /* reverse gear: blue edge, not a warning hue */
/* Warnings only: orange / red */
#define PAL_WARN_ORANGE 0xF5A04A
#define PAL_ALERT_RED   0xF2555A

/* iBR state colours (Glass) */
#define IBR_COL_FWD     PAL_AQUA_300
#define IBR_COL_NEUTRAL PAL_FOAM
#define IBR_COL_REV     PAL_PERIWINKLE
#define IBR_COL_BRAKE   PAL_ALERT_RED

/* ---------- Neutrals ---------- */
/* Luxury: warm white, graphite greys, calm alert colours */
#define C_BG      lv_color_hex(0x060708)
#define C_PANEL   lv_color_hex(0x0E1013)
#define C_LINE    lv_color_hex(0x1F2328)
#define C_TEXT    lv_color_hex(0xF1EEE9)
#define C_DIM     lv_color_hex(0x80838A)
#define C_OFF     lv_color_hex(0x2D3238)
#define C_RED     lv_color_hex(0xF2555A)
#define C_AMBER   lv_color_hex(0xF0B04A)

typedef enum { UI_THEME_CLASSIC = 0, UI_THEME_GLASS, UI_THEME_COUNT } ui_theme_id_t;

void ui_theme_init(void);
void ui_theme_set_mode(dash_mode_t mode);

/* Classic = original look. Glass = frosted panels, ripple, new font,
 * blue-teal palette, half glow, slim perspective bars, big gear. */
void ui_theme_select(ui_theme_id_t id);
bool ui_theme_glass(void);

/* Glow sizes / opacities scaled for the theme (Glass = 50 %) */
int32_t ui_glow(int32_t v);

/* Fonts for the current theme: text sizes 16/20/24/28/32/40/48,
 * digits 160/72/34 (tabular in Glass) */
const lv_font_t * ui_font(int size);
const lv_font_t * ui_digits(int size);
#define UI_FONT(sz)   ui_font(sz)
#define UI_DIGITS(sz) ui_digits(sz)
lv_color_t ui_theme_accent(void);

/* Tones of the current accent (all follow the riding mode):
 * (see ui_tone_t) */
typedef enum {
    TONE_DEEP = 0,   /* dark, saturated: glows, depth */
    TONE_MID,        /* between deep and main: fill start, secondary marks */
    TONE_MAIN,       /* the accent */
    TONE_LIGHT,      /* highlights, fill tips */
    TONE_SOFT,       /* muted: captions, units */
    TONE_TRACK,      /* barely tinted: empty tracks */
    TONE_COUNT
} ui_tone_t;
lv_color_t ui_theme_tone(ui_tone_t t);

/* Accent pre-mixed into the background (opaque, no overlap artefacts) */
lv_color_t ui_theme_accent_mix(lv_opa_t amount);

/* Shared accent styles */
lv_style_t * ui_style_accent_text(void);   /* text colour */
lv_style_t * ui_style_accent_arc(void);    /* arc colour (gauge indicator) */
lv_style_t * ui_style_accent_glow(void);   /* arc colour, low opacity */
lv_style_t * ui_style_accent_bg(void);     /* bg + shadow colour (bars, dots, lights) */
lv_style_t * ui_style_accent_border(void); /* border + shadow colour (focus) */
lv_style_t * ui_style_accent_hi(void);     /* arc colour, bright highlight */
lv_style_t * ui_style_accent_dim(void);    /* arc colour, faint hairline */
lv_style_t * ui_style_disc(void);          /* gauge disc: dark, accent-tinted */
lv_style_t * ui_style_card(void);          /* page card background */
lv_style_t * ui_style_soft_text(void);     /* SOFT tone text (captions, units) */
lv_style_t * ui_style_track_arc(void);     /* TRACK tone arc (empty ring track) */

#endif /* UI_THEME_H */
