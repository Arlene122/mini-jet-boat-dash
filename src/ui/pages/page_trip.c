/**
 * page_trip — this ride's stats.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"

static lv_obj_t * s_dist, * s_top, * s_rpm, * s_time, * s_fuel;

static void create(lv_obj_t * p)
{
    s_dist = page_row(p, 0, 0, "DISTANCE", &lv_font_montserrat_40);
    s_top = page_row(p, 240, 0, "TOP SPEED", &lv_font_montserrat_40);
    s_time = page_row(p, 0, 100, "RIDE TIME", &lv_font_montserrat_40);
    s_rpm = page_row(p, 240, 100, "MAX RPM", &lv_font_montserrat_40);
    s_fuel = page_row(p, 0, 200, "FUEL USED", &lv_font_montserrat_40);
}

static void update(const dash_data_t * d)
{
    const dash_trip_t * t = &d->trip;
    ui_label_printf(s_dist, "%.1f km", (double)d->trip_km);
    ui_label_printf(s_top, "%d km/h", (int)(t->top_speed_kmh + 0.5f));
    ui_label_printf(s_time, "%d:%02d", (int)(t->ride_time_s / 3600), (int)(t->ride_time_s / 60 % 60));
    ui_label_printf(s_rpm, "%d", t->max_rpm);
    ui_label_printf(s_fuel, "%.1f L", (double)t->fuel_used_l);
}

const ui_page_t page_trip = { "TRIP", create, update, NULL };
