/**
 * page_trip — this ride's stats (units follow the speed setting).
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

static lv_obj_t * s_dist, * s_top, * s_rpm, * s_time, * s_fuel;

static void create(lv_obj_t * p)
{
    const lv_font_t * f = &lv_font_montserrat_32;
    s_dist = page_row(p, 0, 0, "DISTANCE", f);
    s_top = page_row(p, PAGE_COL2, 0, "TOP SPEED", f);
    s_time = page_row(p, 0, 96, "RIDE TIME", f);
    s_rpm = page_row(p, PAGE_COL2, 96, "MAX RPM", f);
    s_fuel = page_row(p, 0, 192, "FUEL USED", f);
}

static void update(const dash_data_t * d)
{
    const dash_trip_t * t = &d->trip;
    ui_label_printf(s_dist, "%.1f %s", (double)settings_dist(d->trip_km), settings_dist_unit());
    ui_label_printf(s_top, "%d", (int)(settings_speed(t->top_speed_kmh) + 0.5f));
    ui_label_printf(s_time, "%d:%02d", (int)(t->ride_time_s / 3600), (int)(t->ride_time_s / 60 % 60));
    ui_label_printf(s_rpm, "%d", t->max_rpm);
    ui_label_printf(s_fuel, "%.1f L", (double)t->fuel_used_l);
}

const ui_page_t page_trip = { "TRIP", create, update, NULL };
