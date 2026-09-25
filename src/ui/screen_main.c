/**
 * screen_main — composes the zones from ui_layout.h and updates the
 * top / bottom bars.
 */
#include "screen_main.h"

#include "frame_line.h"
#include "gauge_speed.h"
#include "page_host.h"
#include "panel_engine.h"
#include "tilt_bar.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"
#include "warn_banner.h"

/* ---------- Widgets ---------- */

typedef struct { uint32_t flag; const char * text; bool red; lv_obj_t * label; } warn_item_t;
static warn_item_t s_warn[] = {
    { DASH_WARN_CHECK_ENGINE, LV_SYMBOL_WARNING " ENGINE", true,  NULL },
    { DASH_WARN_OIL_PRESSURE, "OIL",                        true,  NULL },
    { DASH_WARN_OVERHEAT,     "TEMP",                       true,  NULL },
    { DASH_WARN_LOW_FUEL,     "FUEL",                       false, NULL },
    { DASH_WARN_LOW_BATTERY,  LV_SYMBOL_BATTERY_1 " BATT",  false, NULL },
};
#define WARN_ITEMS (sizeof(s_warn) / sizeof(s_warn[0]))

static lv_obj_t * s_trip, * s_clock, * s_hours;
static lv_obj_t * s_mode, * s_dess, * s_source;
static bool s_sweep;

#define C_WARN_OFF lv_color_hex(0x46525F)   /* readable on the bar gradient */

/* ---------- Build ---------- */

static void build_top_bar(lv_obj_t * scr)
{
    frame_line_create(scr, FRAME_TOP_OUT, FRAME_TOP_IN, NOTCH_X1, NOTCH_X2, NOTCH_CURVE);
    lv_obj_t * bar = tilt_bar_create(scr, BAR_X, TOP_BAR_Y, BAR_W, BAR_H, BAR_TILT, true);
    s_trip = ui_label(bar, &lv_font_montserrat_24, C_TEXT, "");
    lv_obj_align(s_trip, LV_ALIGN_LEFT_MID, 70, 0);
    s_clock = ui_label(bar, &lv_font_montserrat_40, C_TEXT, "--:--");
    lv_obj_add_style(s_clock, ui_style_accent_text(), 0);
    lv_obj_align(s_clock, LV_ALIGN_CENTER, 0, 0);
    s_hours = ui_label(bar, &lv_font_montserrat_24, C_TEXT, "");
    lv_obj_align(s_hours, LV_ALIGN_RIGHT_MID, -70, 0);
}

static void build_bottom_bar(lv_obj_t * scr)
{
    frame_line_create(scr, FRAME_BOT_OUT, FRAME_BOT_IN, NOTCH_X1, NOTCH_X2, NOTCH_CURVE);
    lv_obj_t * bar = tilt_bar_create(scr, BAR_X, BOT_BAR_Y, BAR_W, BAR_H, BAR_TILT, false);
    s_mode = ui_label(bar, &lv_font_montserrat_24, C_TEXT, "");
    lv_obj_add_style(s_mode, ui_style_accent_text(), 0);
    lv_obj_set_style_text_letter_space(s_mode, 3, 0);
    lv_obj_align(s_mode, LV_ALIGN_LEFT_MID, 50, 0);

    lv_obj_t * row = lv_obj_create(bar);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 24, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, -10, 0);
    for(size_t i = 0; i < WARN_ITEMS; i++) {
        s_warn[i].label = ui_label(row, &lv_font_montserrat_20, C_WARN_OFF, s_warn[i].text);
    }

    s_source = ui_label(bar, &lv_font_montserrat_20, C_DIM, "");
    lv_obj_align(s_source, LV_ALIGN_RIGHT_MID, -45, 0);
    s_dess = ui_label(bar, &lv_font_montserrat_20, C_TEXT, "");
    lv_obj_align(s_dess, LV_ALIGN_RIGHT_MID, -100, 0);
}

lv_obj_t * screen_main_create(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_scrollable(scr, false);
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    build_top_bar(scr);
    build_bottom_bar(scr);
    panel_engine_create(scr);
    gauge_speed_create(scr);
    page_host_create(scr);
    warn_banner_create(scr);
    return scr;
}

/* ---------- Update ---------- */

static void update_bars(const dash_data_t * d)
{
    ui_label_printf(s_trip, "TRIP  %.1f km", (double)d->trip_km);
    if(d->clock_h < 0) ui_label_printf(s_clock, "--:--");
    else ui_label_printf(s_clock, "%02d:%02d", d->clock_h, d->clock_m);
    ui_label_printf(s_hours, "%.1f h", (double)d->engine_hours);

    static const char * modes[DASH_MODE_COUNT] = { "TOURING", "SPORT", "ECO" };
    ui_label_printf(s_mode, "%s", modes[d->mode]);

    ui_label_printf(s_dess, "%s", d->dess_ok ? LV_SYMBOL_OK " DESS" : LV_SYMBOL_CLOSE " NO KEY");
    ui_set_text_color(s_dess, d->dess_ok ? C_DIM : C_RED);
    static const char * src[] = { "SIM", "REPLAY", "CAN" };
    ui_label_printf(s_source, "%s", src[d->source]);

    for(size_t i = 0; i < WARN_ITEMS; i++) {
        bool on = (d->warnings & s_warn[i].flag) != 0;
        ui_set_text_color(s_warn[i].label, on ? (s_warn[i].red ? C_RED : C_AMBER) : C_WARN_OFF);
    }
}

void screen_main_update(const dash_data_t * d)
{
    ui_theme_set_mode(d->mode);
    update_bars(d);
    panel_engine_update(d);
    gauge_speed_update(d, !s_sweep);
    page_host_update(d);
    warn_banner_update(d);
}

void screen_main_set_sweep(bool on)
{
    s_sweep = on;
}
