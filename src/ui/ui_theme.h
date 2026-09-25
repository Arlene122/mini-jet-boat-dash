/**
 * ui_theme — palette + accent colour that tints with the riding mode.
 * Widgets add the shared accent styles; changing mode re-colours them all.
 */
#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

/* ---------- Palette ---------- */
#define C_BG      lv_color_hex(0x05070A)
#define C_PANEL   lv_color_hex(0x0C1117)
#define C_LINE    lv_color_hex(0x1C2632)
#define C_TEXT    lv_color_hex(0xEAF2F8)
#define C_DIM     lv_color_hex(0x6B7886)
#define C_OFF     lv_color_hex(0x263241)
#define C_RED     lv_color_hex(0xFF4D4F)
#define C_AMBER   lv_color_hex(0xFFB020)

void ui_theme_init(void);
void ui_theme_set_mode(dash_mode_t mode);
lv_color_t ui_theme_accent(void);

/* Tones of the current accent (all follow the riding mode):
 * DEEP = saturated/dark (glows, depth), MAIN = accent, LIGHT = highlights,
 * SOFT = muted tint for captions and hairlines */
typedef enum { TONE_DEEP = 0, TONE_MAIN, TONE_LIGHT, TONE_SOFT, TONE_COUNT } ui_tone_t;
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

#endif /* UI_THEME_H */
