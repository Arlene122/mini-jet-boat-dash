/**
 * page_marine — water temp, heading, GPS position, tide.
 */
#include "pages.h"

#include <math.h>

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

static lv_obj_t * s_water, * s_heading, * s_pos, * s_tide, * s_tide_bar, * s_lo, * s_hi;

static const char * compass(float deg)
{
    static const char * n[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
    return n[(int)((fmodf(deg, 360.0f) + 22.5f) / 45.0f) % 8];
}

static void create(lv_obj_t * p)
{
    s_water = page_row(p, 0, 0, "WATER", &lv_font_montserrat_32);
    s_heading = page_row(p, PAGE_COL2, 0, "HEADING", &lv_font_montserrat_32);
    s_pos = page_row(p, 0, 92, "POSITION", &lv_font_montserrat_20);
    s_tide = page_row(p, 0, 170, "TIDE", &lv_font_montserrat_32);
    s_tide_bar = ui_slim_bar(p, PAGE_CONTENT_W, 4, 100);
    lv_obj_set_pos(s_tide_bar, 0, 244);
    s_lo = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_pos(s_lo, 0, 256);
    s_hi = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_align(s_hi, LV_ALIGN_TOP_RIGHT, 0, 256);
}

static void update(const dash_data_t * d)
{
    const dash_marine_t * m = &d->marine;
    if(m->water_temp_ok) ui_label_printf(s_water, "%d%s", settings_temp(m->water_temp_c), settings_temp_unit());
    else ui_label_printf(s_water, "--");

    if(m->gps_fix) {
        ui_label_printf(s_heading, "%03d° %s", (int)m->heading_deg, compass(m->heading_deg));
        ui_label_printf(s_pos, "%.4f°%c   %.4f°%c", (double)fabsf(m->lat), m->lat < 0 ? 'S' : 'N',
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
        ui_label_printf(s_lo, "LOW %.1f m", (double)m->tide_min_m);
        ui_label_printf(s_hi, "HIGH %.1f m", (double)m->tide_max_m);
    }
    else {
        ui_label_printf(s_tide, "No data");
        ui_label_printf(s_lo, "needs phone hotspot");
        ui_label_printf(s_hi, "");
    }
}

const ui_page_t page_marine = { "MARINE", create, update, NULL };
