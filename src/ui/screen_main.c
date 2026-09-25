/**
 * screen_main — main riding screen.
 *
 * PHASE 1 PLACEHOLDER: proves layout, fonts and data flow at 1920x720.
 * The real visual style comes from the 2-3 style concepts (next step).
 *
 * Visible window (placeholder stencil): x 60..1860, y 40..680.
 */
#include "screen_main.h"

#include <stdio.h>

#include "tilt_bar.h"
#include "ui.h"

LV_FONT_DECLARE(font_digits_200);
LV_FONT_DECLARE(font_digits_96);

/* ---------- Palette ---------- */

#define C_BG      lv_color_hex(0x07090C)
#define C_TEXT    lv_color_hex(0xE8EEF5)
#define C_DIM     lv_color_hex(0x5A6573)
#define C_OFF     lv_color_hex(0x2A313A)
#define C_ACCENT  lv_color_hex(0x2F80ED)
#define C_RED     lv_color_hex(0xFF4D4F)
#define C_AMBER   lv_color_hex(0xFFB020)
#define C_GREEN   lv_color_hex(0x27C281)

/* ---------- Layout ---------- */

#define RPM_MAX        8000
#define RPM_REDLINE    7000
#define FUEL_LOW_PCT   15
#define KMH_PER_KNOT   1.852f

#define PANEL_W        500
#define PANEL_L_X      150
#define PANEL_R_X      (UI_HOR_RES - PANEL_L_X - PANEL_W)

/* ---------- Widgets ---------- */

typedef struct {
    uint32_t flag;
    const char * text;
    lv_color_t on_color;
    lv_obj_t * label;
} warn_item_t;

static warn_item_t s_warn[] = {
    { DASH_WARN_CHECK_ENGINE, LV_SYMBOL_WARNING " CHECK ENGINE", {0}, NULL },
    { DASH_WARN_OIL_PRESSURE, "OIL",  {0}, NULL },
    { DASH_WARN_OVERHEAT,     "TEMP", {0}, NULL },
    { DASH_WARN_LOW_FUEL,     "FUEL", {0}, NULL },
    { DASH_WARN_LOW_BATTERY,  LV_SYMBOL_BATTERY_1 " BATT", {0}, NULL },
};
#define WARN_COUNT (sizeof(s_warn) / sizeof(s_warn[0]))

static lv_obj_t * s_speed;
static lv_obj_t * s_speed_unit;
static lv_obj_t * s_speed_alt;
static lv_obj_t * s_rpm;
static lv_obj_t * s_rpm_bar;
static lv_obj_t * s_fuel;
static lv_obj_t * s_fuel_bar;
static lv_obj_t * s_clock;
static lv_obj_t * s_mode;
static lv_obj_t * s_dess;
static lv_obj_t * s_source;
static lv_obj_t * s_temp;
static lv_obj_t * s_batt;
static lv_obj_t * s_trip;
static lv_obj_t * s_hours;
static lv_obj_t * s_ibr[3];      /* R N F */
static lv_obj_t * s_brake;
static lv_obj_t * s_code;

/* Last shown values: redraw only on change */
static struct {
    int speed, speed_alt, rpm, fuel, clock, mode, dess, source;
    int temp, batt, trip, hours, ibr, code;
    uint32_t warnings;
} s_shown;

/* ---------- Helpers ---------- */

static lv_obj_t * make_label(lv_obj_t * parent, const lv_font_t * font, lv_color_t color,
                             const char * text)
{
    lv_obj_t * l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    lv_label_set_text(l, text);
    return l;
}

static lv_obj_t * make_bar(lv_obj_t * parent, int32_t x, int32_t y, int32_t max, lv_color_t color)
{
    lv_obj_t * b = lv_bar_create(parent);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, PANEL_W, 22);
    lv_bar_set_range(b, 0, max);
    lv_obj_set_style_bg_color(b, C_OFF, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(b, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(b, 4, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(b, color, LV_PART_INDICATOR);
    lv_obj_set_style_anim_duration(b, 0, LV_PART_MAIN);
    return b;
}

static int round_i(float v)
{
    return (int)(v < 0 ? v - 0.5f : v + 0.5f);
}

/* ---------- Build ---------- */

static void build_top_bar(lv_obj_t * scr)
{
    lv_obj_t * bar = tilt_bar_create(scr, 60, 40, 1800, 90, 60, TILT_BAR_TOP);

    s_clock = make_label(bar, &lv_font_montserrat_48, C_TEXT, "--:--");
    lv_obj_align(s_clock, LV_ALIGN_LEFT_MID, 130, 0);

    s_mode = make_label(bar, &lv_font_montserrat_32, C_ACCENT, "");
    lv_obj_align(s_mode, LV_ALIGN_CENTER, 0, 0);

    s_dess = make_label(bar, &lv_font_montserrat_28, C_GREEN, "");
    lv_obj_align(s_dess, LV_ALIGN_RIGHT_MID, -300, 0);

    s_source = make_label(bar, &lv_font_montserrat_24, C_DIM, "");
    lv_obj_align(s_source, LV_ALIGN_RIGHT_MID, -130, 0);
}

static void build_bottom_bar(lv_obj_t * scr)
{
    lv_obj_t * bar = tilt_bar_create(scr, 60, 590, 1800, 90, 60, TILT_BAR_BOTTOM);

    s_temp = make_label(bar, &lv_font_montserrat_28, C_TEXT, "");
    lv_obj_align(s_temp, LV_ALIGN_LEFT_MID, 130, 0);
    s_batt = make_label(bar, &lv_font_montserrat_28, C_TEXT, "");
    lv_obj_align(s_batt, LV_ALIGN_LEFT_MID, 300, 0);

    /* Warning row: always visible, dim when inactive */
    lv_obj_t * row = lv_obj_create(bar);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 30, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, -60, 0);

    s_warn[0].on_color = C_RED;
    s_warn[1].on_color = C_RED;
    s_warn[2].on_color = C_RED;
    s_warn[3].on_color = C_AMBER;
    s_warn[4].on_color = C_AMBER;
    for(size_t i = 0; i < WARN_COUNT; i++) {
        s_warn[i].label = make_label(row, &lv_font_montserrat_28, C_OFF, s_warn[i].text);
    }

    s_trip = make_label(bar, &lv_font_montserrat_28, C_TEXT, "");
    lv_obj_align(s_trip, LV_ALIGN_RIGHT_MID, -330, 0);
    s_hours = make_label(bar, &lv_font_montserrat_28, C_TEXT, "");
    lv_obj_align(s_hours, LV_ALIGN_RIGHT_MID, -130, 0);
}

static void build_centre(lv_obj_t * scr)
{
    s_speed = make_label(scr, &font_digits_200, C_TEXT, "0");
    lv_obj_align(s_speed, LV_ALIGN_TOP_MID, 0, 140);

    s_speed_unit = make_label(scr, &lv_font_montserrat_32, C_DIM, "km/h");
    lv_obj_align(s_speed_unit, LV_ALIGN_TOP_MID, 0, 370);

    s_speed_alt = make_label(scr, &lv_font_montserrat_28, C_DIM, "");
    lv_obj_align(s_speed_alt, LV_ALIGN_TOP_MID, 0, 415);

    /* iBR: R N F + BRAKE */
    static const char * letters[3] = { "R", "N", "F" };
    for(int i = 0; i < 3; i++) {
        s_ibr[i] = make_label(scr, &lv_font_montserrat_48, C_OFF, letters[i]);
        lv_obj_align(s_ibr[i], LV_ALIGN_TOP_MID, (i - 1) * 90, 470);
    }
    s_brake = make_label(scr, &lv_font_montserrat_28, C_RED, "BRAKE");
    lv_obj_align(s_brake, LV_ALIGN_TOP_MID, 250, 482);
    lv_obj_set_hidden(s_brake, true);

    s_code = make_label(scr, &lv_font_montserrat_28, C_RED, "");
    lv_obj_align(s_code, LV_ALIGN_TOP_MID, -300, 482);
}

static void build_side_panels(lv_obj_t * scr)
{
    lv_obj_t * l = make_label(scr, &lv_font_montserrat_28, C_DIM, "RPM");
    lv_obj_set_pos(l, PANEL_L_X, 190);
    s_rpm = make_label(scr, &font_digits_96, C_TEXT, "0");
    lv_obj_set_pos(s_rpm, PANEL_L_X, 225);
    s_rpm_bar = make_bar(scr, PANEL_L_X, 360, RPM_MAX, C_ACCENT);

    l = make_label(scr, &lv_font_montserrat_28, C_DIM, "FUEL");
    lv_obj_align(l, LV_ALIGN_TOP_RIGHT, -PANEL_L_X, 190);
    s_fuel = make_label(scr, &font_digits_96, C_TEXT, "0%");
    lv_obj_align(s_fuel, LV_ALIGN_TOP_RIGHT, -PANEL_L_X, 225);
    s_fuel_bar = make_bar(scr, PANEL_R_X, 360, 100, C_GREEN);
}

lv_obj_t * screen_main_create(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_scrollable(scr, false);
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    build_top_bar(scr);
    build_side_panels(scr);
    build_centre(scr);
    build_bottom_bar(scr);

    /* Force the first update to draw everything */
    lv_memset(&s_shown, 0xFF, sizeof(s_shown));
    return scr;
}

/* ---------- Update (only changed widgets) ---------- */

static const char * mode_name(dash_mode_t m)
{
    switch(m) {
        case DASH_MODE_SPORT: return "SPORT";
        case DASH_MODE_ECO:   return "ECO";
        case DASH_MODE_SLOW:  return "SLOW SPEED";
        default:              return "TOURING";
    }
}

static const char * source_name(dash_source_t s)
{
    switch(s) {
        case DASH_SRC_REPLAY: return "REPLAY";
        case DASH_SRC_CAN:    return "CAN";
        default:              return "SIM";
    }
}

static void update_speed(const dash_data_t * d)
{
    int v = round_i(d->speed_kmh);
    if(v != s_shown.speed) {
        s_shown.speed = v;
        lv_label_set_text_fmt(s_speed, "%d", v);
    }
    int kn10 = round_i(d->speed_kmh / KMH_PER_KNOT * 10.0f);
    if(kn10 != s_shown.speed_alt) {
        s_shown.speed_alt = kn10;
        lv_label_set_text_fmt(s_speed_alt, "%d.%d kn", kn10 / 10, kn10 % 10);
    }
}

static void update_engine(const dash_data_t * d)
{
    int rpm = (d->rpm + 25) / 50 * 50;   /* steady digits: 50 rpm steps */
    if(rpm != s_shown.rpm) {
        s_shown.rpm = rpm;
        lv_label_set_text_fmt(s_rpm, "%d", rpm);
        lv_bar_set_value(s_rpm_bar, rpm, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(s_rpm_bar, rpm >= RPM_REDLINE ? C_RED : C_ACCENT,
                                  LV_PART_INDICATOR);
    }

    int fuel = round_i(d->fuel_pct);
    if(fuel != s_shown.fuel) {
        s_shown.fuel = fuel;
        lv_label_set_text_fmt(s_fuel, "%d%%", fuel);
        lv_bar_set_value(s_fuel_bar, fuel, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(s_fuel_bar, fuel <= FUEL_LOW_PCT ? C_AMBER : C_GREEN,
                                  LV_PART_INDICATOR);
    }

    int temp = round_i(d->engine_temp_c);
    if(temp != s_shown.temp) {
        s_shown.temp = temp;
        lv_label_set_text_fmt(s_temp, "%d°C", temp);
    }

    int batt = round_i(d->battery_v * 10.0f);
    if(batt != s_shown.batt) {
        s_shown.batt = batt;
        lv_label_set_text_fmt(s_batt, "%d.%d V", batt / 10, batt % 10);
    }

    int hours = (int)(d->engine_hours * 10.0f);
    if(hours != s_shown.hours) {
        s_shown.hours = hours;
        lv_label_set_text_fmt(s_hours, "%d.%d h", hours / 10, hours % 10);
    }

    int trip = (int)(d->trip_km * 10.0f);
    if(trip != s_shown.trip) {
        s_shown.trip = trip;
        lv_label_set_text_fmt(s_trip, "TRIP %d.%d km", trip / 10, trip % 10);
    }
}

static void update_status(const dash_data_t * d)
{
    int clock = d->clock_h < 0 ? -1 : d->clock_h * 60 + d->clock_m;
    if(clock != s_shown.clock) {
        s_shown.clock = clock;
        if(clock < 0) lv_label_set_text(s_clock, "--:--");
        else lv_label_set_text_fmt(s_clock, "%02d:%02d", d->clock_h, d->clock_m);
    }

    if((int)d->mode != s_shown.mode) {
        s_shown.mode = d->mode;
        lv_label_set_text(s_mode, mode_name(d->mode));
    }

    if((int)d->dess_ok != s_shown.dess) {
        s_shown.dess = d->dess_ok;
        lv_label_set_text(s_dess, d->dess_ok ? LV_SYMBOL_OK " DESS" : LV_SYMBOL_CLOSE " NO KEY");
        lv_obj_set_style_text_color(s_dess, d->dess_ok ? C_GREEN : C_RED, 0);
    }

    if((int)d->source != s_shown.source) {
        s_shown.source = d->source;
        lv_label_set_text(s_source, source_name(d->source));
    }

    if((int)d->ibr != s_shown.ibr) {
        s_shown.ibr = d->ibr;
        int active = d->ibr == DASH_IBR_REVERSE ? 0 : d->ibr == DASH_IBR_FORWARD ? 2 : 1;
        for(int i = 0; i < 3; i++) {
            lv_obj_set_style_text_color(s_ibr[i], i == active ? C_TEXT : C_OFF, 0);
        }
        lv_obj_set_hidden(s_brake, d->ibr != DASH_IBR_BRAKE);
    }

    if(d->warnings != s_shown.warnings) {
        s_shown.warnings = d->warnings;
        for(size_t i = 0; i < WARN_COUNT; i++) {
            bool on = (d->warnings & s_warn[i].flag) != 0;
            lv_obj_set_style_text_color(s_warn[i].label, on ? s_warn[i].on_color : C_OFF, 0);
        }
    }

    if((int)d->error_code != s_shown.code) {
        s_shown.code = d->error_code;
        if(d->error_code) lv_label_set_text_fmt(s_code, "CODE P%04X", d->error_code);
        else lv_label_set_text(s_code, "");
    }
}

void screen_main_update(const dash_data_t * d)
{
    update_speed(d);
    update_engine(d);
    update_status(d);
}
