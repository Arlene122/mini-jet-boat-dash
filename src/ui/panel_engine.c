/**
 * panel_engine — left zone, mirror of the page zone: fading hairline on
 * its inner (right) edge, stats right-aligned toward the gauges.
 */
#include "panel_engine.h"

#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"
#include "../settings/settings.h"

/* ---------- Config ---------- */

#define TEMP_HOT_C    95
#define BATT_LOW_V10  118
#define PAD           30
#define ROW_H         138
#define TOP           40

/* ---------- Widgets ---------- */

static lv_obj_t * s_temp, * s_temp_u, * s_batt, * s_rate;

/* ---------- Helpers ---------- */

static lv_obj_t * stat(lv_obj_t * p, int32_t y, const char * cap, const char * unit, lv_obj_t ** unit_lbl)
{
    int32_t right = -PAD;
    lv_obj_t * c = ui_caption(p, cap);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_align(c, LV_ALIGN_TOP_RIGHT, right, y);

    lv_obj_t * row = ui_box(p, 0, 0, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_column(row, 8, 0);
    lv_obj_align(row, LV_ALIGN_TOP_RIGHT, right, y + 22);
    lv_obj_t * v = ui_label(row, &lv_font_montserrat_48, C_TEXT, "--");
    lv_obj_t * u = ui_label(row, &lv_font_montserrat_20, C_DIM, unit);
    lv_obj_set_style_pad_bottom(u, 9, 0);
    if(unit_lbl) *unit_lbl = u;
    return v;
}

/* ---------- API ---------- */

void panel_engine_create(lv_obj_t * parent)
{
    lv_obj_t * p = ui_box(parent, ENGINE_X, SIDE_Y1, ENGINE_W, SIDE_H);
    ui_fade_line(p, ENGINE_W - 1, 0, SIDE_H, true, LV_OPA_60);
    s_temp = stat(p, TOP, "ENGINE TEMP", "", &s_temp_u);
    s_batt = stat(p, TOP + ROW_H, "BATTERY", "V", NULL);
    s_rate = stat(p, TOP + ROW_H * 2, "FUEL USE", "L/h", NULL);
    for(int i = 1; i < 3; i++) {
        ui_fade_line(p, ENGINE_W - PAD - 200, TOP + ROW_H * i - 24, 200, false, LV_OPA_30);
    }
}

void panel_engine_update(const dash_data_t * d)
{
    if(!d->ecu_ok) {
        ui_label_printf(s_temp, "--");
        ui_label_printf(s_batt, "--");
        ui_label_printf(s_rate, "--");
    }
    else {
        ui_label_printf(s_temp, "%d", settings_temp(d->engine_temp_c));
        ui_set_text_color(s_temp, d->engine_temp_c >= TEMP_HOT_C ? C_RED : C_TEXT);
        int b10 = (int)(d->battery_v * 10.0f + 0.5f);
        ui_label_printf(s_batt, "%d.%d", b10 / 10, b10 % 10);
        ui_set_text_color(s_batt, b10 <= BATT_LOW_V10 ? C_AMBER : C_TEXT);
        ui_label_printf(s_rate, "%.1f", (double)d->fuel_rate_lph);
    }
    ui_label_printf(s_temp_u, "%s", settings_temp_unit());
}
