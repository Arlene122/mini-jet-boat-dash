/**
 * ui_theme — palette + accent colour that tints with the riding mode.
 * Widgets add the shared accent styles; changing mode re-colours them all.
 */
#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

/* ---------- Palette ---------- */
/* Luxury: warm white, graphite greys, calm alert colours */
#define C_BG      lv_color_hex(0x060708)
#define C_PANEL   lv_color_hex(0x0E1013)
#define C_LINE    lv_color_hex(0x1F2328)
#define C_TEXT    lv_color_hex(0xF1EEE9)
#define C_DIM     lv_color_hex(0x80838A)
#define C_OFF     lv_color_hex(0x2D3238)
#define C_RED     lv_color_hex(0xF2555A)
#define C_AMBER   lv_color_hex(0xF0B04A)

void ui_theme_init(void);
void ui_theme_set_mode(dash_mode_t mode);
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
