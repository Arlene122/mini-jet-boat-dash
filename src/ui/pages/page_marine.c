/**
 * page_marine — water temp, tide, GPS position, heading.
 */
#include "pages.h"

#include <math.h>

#include "../ui_theme.h"
#include "../ui_util.h"

static lv_obj_t * s_water, * s_heading, * s_pos, * s_tide, * s_tide_bar, * s_tide_rng;

static const char * compass(float deg)
{
    static const char * n[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
    int i = (int)((fmodf(deg, 360.0f) + 22.5f) / 45.0f) % 8;
    return n[i];
}

static void create(lv_obj_t * p)
{
    s_water = page_row(p, 0, 0, "WATER", &lv_font_montserrat_40);
    s_heading = page_row(p, 230, 0, "HEADING", &lv_font_montserrat_40);
    s_pos = page_row(p, 0, 100, "POSITION", &lv_font_montserrat_28);
    s_tide = page_row(p, 0, 180, "TIDE", &lv_font_montserrat_40);
    s_tide_bar = ui_slim_bar(p, 420, 6, 100);
    lv_obj_set_pos(s_tide_bar, 0, 256);
    s_tide_rng = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_width(s_tide_rng, 420);
    lv_obj_set_pos(s_tide_rng, 0, 268);
}

static void update(const dash_data_t * d)
{
    const dash_marine_t * m = &d->marine;
    if(m->water_temp_ok) ui_label_printf(s_water, "%.1f°C", (double)m->water_temp_c);
    else ui_label_printf(s_water, "--");

    if(m->gps_fix) {
        ui_label_printf(s_heading, "%03d° %s", (int)m->heading_deg, compass(m->heading_deg));
        ui_label_printf(s_pos, "%.4f°%c  %.4f°%c", (double)fabsf(m->lat), m->lat < 0 ? 'S' : 'N',
                        (double)fabsf(m->lon), m->lon < 0 ? 'W' : 'E');
    }
    else {
        ui_label_printf(s_heading, "--");
        ui_label_printf(s_pos, "No GPS fix");
    }

    if(m->tide_ok && m->tide_max_m > m->tide_min_m) {
        ui_label_printf(s_tide, "%.2f m", (double)m->tide_m);
        int pct = (int)((m->tide_m - m->tide_min_m) / (m->tide_max_m - m->tide_min_m) * 100.0f);
        if(lv_bar_get_value(s_tide_bar) != pct) lv_bar_set_value(s_tide_bar, pct, LV_ANIM_OFF);
        ui_label_printf(s_tide_rng, "low %.1f m                                         high %.1f m",
                        (double)m->tide_min_m, (double)m->tide_max_m);
    }
    else {
        ui_label_printf(s_tide, "No data");
        ui_label_printf(s_tide_rng, "needs phone hotspot");
    }
}

const ui_page_t page_marine = { "MARINE", create, update, NULL };
