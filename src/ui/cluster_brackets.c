/**
 * cluster_brackets — numbers sit inside each bracket, next to the ring.
 */
#include "cluster_brackets.h"

#include "bracket_gauge.h"
#include "gauge_speed.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"

/* ---------- Config ---------- */

#define FUEL_LOW_PCT  15
#define MIRROR(x)     (UI_HOR_RES - (x))

/* ---------- Widgets ---------- */

static lv_obj_t * s_rpm_brk, * s_fuel_brk;
static lv_obj_t * s_rpm, * s_fuel;

/* ---------- Helpers ---------- */

static lv_obj_t * value_block(lv_obj_t * parent, int32_t x, int32_t y, lv_align_t align,
                              const char * caption)
{
    lv_obj_t * v = ui_label(parent, &lv_font_montserrat_40, C_TEXT, "--");
    lv_obj_align(v, align, x, y);
    lv_obj_t * c = ui_caption(parent, caption);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_align_to(c, v, align == LV_ALIGN_TOP_LEFT ? LV_ALIGN_OUT_BOTTOM_LEFT : LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
    return v;
}

/* ---------- API ---------- */

void cluster_brackets_create(lv_obj_t * parent)
{
    const lv_point_t l[4] = {
        { BRK_IN_X, BRK_TOP_Y }, { BRK_OUT_X, BRK_MID_Y1 },
        { BRK_OUT_X, BRK_MID_Y2 }, { BRK_IN_X, BRK_BOT_Y },
    };
    const lv_point_t r[4] = {
        { MIRROR(BRK_IN_X), BRK_TOP_Y }, { MIRROR(BRK_OUT_X), BRK_MID_Y1 },
        { MIRROR(BRK_OUT_X), BRK_MID_Y2 }, { MIRROR(BRK_IN_X), BRK_BOT_Y },
    };
    s_rpm_brk = bracket_gauge_create(parent, l);
    s_fuel_brk = bracket_gauge_create(parent, r);

    /* Numbers sit inside the long side, just below the 50 % tick */
    s_rpm = value_block(parent, BRK_OUT_X + 14, GAUGE_CY + 40, LV_ALIGN_TOP_LEFT, "RPM");
    s_fuel = value_block(parent, -(BRK_OUT_X + 14), GAUGE_CY + 40, LV_ALIGN_TOP_RIGHT, "FUEL");
}

void cluster_brackets_update(const dash_data_t * d, bool bars)
{
    if(!d->ecu_ok) {
        ui_label_printf(s_rpm, "--");
        ui_label_printf(s_fuel, "--");
        ui_set_text_color(s_rpm, C_DIM);
        ui_set_text_color(s_fuel, C_DIM);
        if(bars) {
            bracket_gauge_set(s_rpm_brk, 0, BRACKET_NORMAL);
            bracket_gauge_set(s_fuel_brk, 0, BRACKET_NORMAL);
        }
        return;
    }
    int rpm = (d->rpm + 25) / 50 * 50;
    bool red = rpm >= GAUGE_RPM_RED;
    ui_label_printf(s_rpm, "%d", rpm);
    ui_set_text_color(s_rpm, red ? C_RED : C_TEXT);

    int fuel = (int)(d->fuel_pct + 0.5f);
    bool low = fuel <= FUEL_LOW_PCT;
    ui_label_printf(s_fuel, "%d%%", fuel);
    ui_set_text_color(s_fuel, low ? C_AMBER : C_TEXT);

    if(bars) {
        bracket_gauge_set(s_rpm_brk, rpm * 1000 / GAUGE_RPM_MAX, red ? BRACKET_CRIT : BRACKET_NORMAL);
        bracket_gauge_set(s_fuel_brk, fuel * 10, low ? BRACKET_WARN : BRACKET_NORMAL);
    }
}

void cluster_brackets_sweep(int32_t permille)
{
    bracket_gauge_set(s_rpm_brk, permille, BRACKET_NORMAL);
    bracket_gauge_set(s_fuel_brk, permille, BRACKET_NORMAL);
}
