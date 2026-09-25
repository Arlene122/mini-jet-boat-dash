/**
 * panel_engine — big numbers with slim bars, right-aligned toward the gauge.
 */
#include "panel_engine.h"

#include "gauge_speed.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"

LV_FONT_DECLARE(font_digits_72);

/* ---------- Config ---------- */

#define FUEL_LOW_PCT   15
#define TEMP_HOT_C     95
#define BATT_LOW_V10   118

/* ---------- Widgets ---------- */

static lv_obj_t * s_rpm, * s_rpm_bar;
static lv_obj_t * s_fuel, * s_fuel_bar;
static lv_obj_t * s_temp, * s_batt;
static bool s_fuel_low;

/* ---------- Helpers ---------- */

static lv_obj_t * row(lv_obj_t * p, int32_t y, const char * cap, const lv_font_t * font, lv_obj_t ** value)
{
    lv_obj_t * c = ui_caption(p, cap);
    lv_obj_align(c, LV_ALIGN_TOP_RIGHT, 0, y);
    *value = ui_label(p, font, C_TEXT, "");
    lv_obj_align(*value, LV_ALIGN_TOP_RIGHT, 0, y + 22);
    return c;
}

/* ---------- API ---------- */

void panel_engine_create(lv_obj_t * parent)
{
    lv_obj_t * p = ui_box(parent, ENGINE_X, SIDE_Y1, ENGINE_W, SIDE_H);

    row(p, 0, "RPM", &font_digits_72, &s_rpm);
    s_rpm_bar = ui_slim_bar(p, 300, 6, GAUGE_RPM_MAX);
    lv_obj_align(s_rpm_bar, LV_ALIGN_TOP_RIGHT, 0, 112);

    row(p, 140, "FUEL", &font_digits_72, &s_fuel);
    s_fuel_bar = ui_slim_bar(p, 300, 6, 100);
    lv_obj_align(s_fuel_bar, LV_ALIGN_TOP_RIGHT, 0, 252);
    lv_obj_t * e = ui_label(p, &lv_font_montserrat_16, C_DIM, "E");
    lv_obj_align_to(e, s_fuel_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_t * f = ui_label(p, &lv_font_montserrat_16, C_DIM, "F");
    lv_obj_align_to(f, s_fuel_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 4);

    lv_obj_t * c = ui_caption(p, "ENGINE");
    lv_obj_align(c, LV_ALIGN_TOP_RIGHT, -170, 312);
    s_temp = ui_label(p, &lv_font_montserrat_48, C_TEXT, "");
    lv_obj_align(s_temp, LV_ALIGN_TOP_RIGHT, -170, 336);
    c = ui_caption(p, "BATTERY");
    lv_obj_align(c, LV_ALIGN_TOP_RIGHT, 0, 312);
    s_batt = ui_label(p, &lv_font_montserrat_48, C_TEXT, "");
    lv_obj_align(s_batt, LV_ALIGN_TOP_RIGHT, 0, 336);
}

void panel_engine_update(const dash_data_t * d)
{
    int rpm = (d->rpm + 25) / 50 * 50;
    ui_label_printf(s_rpm, "%d", rpm);
    if(lv_bar_get_value(s_rpm_bar) != rpm) lv_bar_set_value(s_rpm_bar, rpm, LV_ANIM_OFF);
    ui_set_text_color(s_rpm, rpm >= GAUGE_RPM_RED ? C_RED : C_TEXT);

    int fuel = (int)(d->fuel_pct + 0.5f);
    ui_label_printf(s_fuel, "%d%%", fuel);
    if(lv_bar_get_value(s_fuel_bar) != fuel) lv_bar_set_value(s_fuel_bar, fuel, LV_ANIM_OFF);
    bool low = fuel <= FUEL_LOW_PCT;
    if(low != s_fuel_low) {
        s_fuel_low = low;
        if(low) lv_obj_set_style_bg_color(s_fuel_bar, C_AMBER, LV_PART_INDICATOR);
        else lv_obj_remove_local_style_prop(s_fuel_bar, LV_STYLE_BG_COLOR, LV_PART_INDICATOR);
        ui_set_text_color(s_fuel, low ? C_AMBER : C_TEXT);
    }

    int temp = (int)(d->engine_temp_c + 0.5f);
    ui_label_printf(s_temp, "%d°", temp);
    ui_set_text_color(s_temp, temp >= TEMP_HOT_C ? C_RED : C_TEXT);

    int b10 = (int)(d->battery_v * 10.0f + 0.5f);
    ui_label_printf(s_batt, "%d.%dV", b10 / 10, b10 % 10);
    ui_set_text_color(s_batt, b10 <= BATT_LOW_V10 ? C_AMBER : C_TEXT);
}
