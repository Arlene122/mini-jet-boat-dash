/**
 * warn_banner — shows the highest-priority unacknowledged warning.
 * A warning that clears is forgotten, so it alerts again if it returns.
 */
#include "warn_banner.h"

#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"

/* ---------- Text (order = DASH_WARN_* bit order) ---------- */

typedef struct { const char * title; const char * msg; bool red; } warn_text_t;
static const warn_text_t TEXT[DASH_WARN_COUNT] = {
    { "OIL PRESSURE",  "Stop the engine now.\nCheck oil level.", true },
    { "OVERHEATING",   "Slow down and idle.\nCheck the jet intake for weed.", true },
    { "CHECK ENGINE",  "Engine fault logged.\nRide gently and get it checked.", true },
    { "LOW FUEL",      "Head back to shore.", false },
    { "LOW BATTERY",   "Charging problem.\nAvoid extra loads.", false },
};

/* ---------- State ---------- */

static lv_obj_t * s_box, * s_icon, * s_title, * s_msg, * s_code;
static uint32_t s_acked;
static int s_showing = -1;

/* ---------- Helpers ---------- */

static void show(int bit, const dash_data_t * d)
{
    s_showing = bit;
    lv_color_t c = TEXT[bit].red ? C_RED : C_AMBER;
    lv_obj_set_style_border_color(s_box, c, 0);
    lv_obj_set_style_shadow_color(s_box, c, 0);
    lv_obj_set_style_text_color(s_icon, c, 0);
    lv_obj_set_style_text_color(s_title, c, 0);
    lv_label_set_text(s_title, TEXT[bit].title);
    lv_label_set_text(s_msg, TEXT[bit].msg);
    if((1u << bit) == DASH_WARN_CHECK_ENGINE && d->error_code)
        lv_label_set_text_fmt(s_code, "CODE P%04X", d->error_code);
    else lv_label_set_text(s_code, "");
    lv_obj_set_hidden(s_box, false);
    lv_obj_fade_in(s_box, 150, 0);
}

/* ---------- API ---------- */

void warn_banner_create(lv_obj_t * parent)
{
    s_box = ui_box(parent, PAGE_X, SIDE_Y1, PAGE_W, SIDE_H);
    lv_obj_set_style_bg_color(s_box, lv_color_hex(0x140A0B), 0);
    lv_obj_set_style_bg_opa(s_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(s_box, 18, 0);
    lv_obj_set_style_border_width(s_box, 3, 0);
    lv_obj_set_style_shadow_width(s_box, 40, 0);
    lv_obj_set_style_shadow_opa(s_box, LV_OPA_50, 0);
    lv_obj_set_style_pad_all(s_box, 30, 0);

    s_icon = ui_label(s_box, &lv_font_montserrat_48, C_RED, LV_SYMBOL_WARNING);
    lv_obj_align(s_icon, LV_ALIGN_TOP_LEFT, 0, 0);
    s_title = ui_label(s_box, &lv_font_montserrat_40, C_RED, "");
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 70, 2);
    s_msg = ui_label(s_box, &lv_font_montserrat_28, C_TEXT, "");
    lv_obj_set_width(s_msg, PAGE_W - 60);
    lv_obj_align(s_msg, LV_ALIGN_TOP_LEFT, 0, 90);
    s_code = ui_label(s_box, &lv_font_montserrat_28, C_RED, "");
    lv_obj_align(s_code, LV_ALIGN_TOP_LEFT, 0, 200);
    lv_obj_t * hint = ui_caption(s_box, "PRESS KNOB TO ACKNOWLEDGE");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_hidden(s_box, true);
}

void warn_banner_update(const dash_data_t * d)
{
    s_acked &= d->warnings;              /* cleared faults can alert again */
    uint32_t pending = d->warnings & ~s_acked;
    if(s_showing >= 0 && (pending & (1u << s_showing))) return;   /* keep current */
    for(int i = 0; i < DASH_WARN_COUNT; i++) {
        if(pending & (1u << i)) { show(i, d); return; }
    }
    if(s_showing >= 0) {
        s_showing = -1;
        lv_obj_set_hidden(s_box, true);
    }
}

bool warn_banner_visible(void)
{
    return s_showing >= 0;
}

void warn_banner_ack(void)
{
    if(s_showing < 0) return;
    s_acked |= 1u << s_showing;
    s_showing = -1;
    lv_obj_set_hidden(s_box, true);
}
