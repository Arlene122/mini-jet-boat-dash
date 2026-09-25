/**
 * page_marine — compass ring with heading as the hero, water temp and
 * tide beside it, GPS position underneath.
 */
#include "pages.h"

#include <math.h>

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

#define COMPASS_D  150

static lv_obj_t * s_needle, * s_heading, * s_dir, * s_water, * s_tide, * s_tide_bar, * s_range, * s_pos;

static const char * compass(float deg)
{
    static const char * n[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
    return n[(int)((fmodf(deg, 360.0f) + 22.5f) / 45.0f) % 8];
}

static void create(lv_obj_t * p)
{
    /* Compass: ring + ticks + heading marker arc */
    lv_obj_t * c = ui_box(p, 0, 6, COMPASS_D, COMPASS_D);
    lv_obj_t * sc = lv_scale_create(c);
    lv_obj_remove_style_all(sc);
    lv_obj_set_size(sc, COMPASS_D, COMPASS_D);
    lv_scale_set_mode(sc, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(sc, 0, 360);
    lv_scale_set_angle_range(sc, 360);
    lv_scale_set_rotation(sc, 270);
    lv_scale_set_total_tick_count(sc, 37);
    lv_scale_set_major_tick_every(sc, 9);
    lv_scale_set_label_show(sc, false);
    lv_obj_set_style_arc_color(sc, C_LINE, LV_PART_MAIN);
    lv_obj_set_style_arc_width(sc, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(sc, C_OFF, LV_PART_ITEMS);
    lv_obj_set_style_line_width(sc, 2, LV_PART_ITEMS);
    lv_obj_set_style_length(sc, 5, LV_PART_ITEMS);
    lv_obj_set_style_line_color(sc, C_DIM, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(sc, 2, LV_PART_INDICATOR);
    lv_obj_set_style_length(sc, 10, LV_PART_INDICATOR);

    s_needle = lv_arc_create(c);
    lv_obj_remove_style_all(s_needle);
    lv_obj_set_size(s_needle, COMPASS_D, COMPASS_D);
    lv_obj_set_clickable(s_needle, false);
    lv_obj_set_style_arc_width(s_needle, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_needle, true, LV_PART_MAIN);
    lv_obj_add_style(s_needle, ui_style_accent_arc(), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(s_needle, LV_OPA_TRANSP, LV_PART_INDICATOR);

    lv_obj_t * n = ui_label(c, &lv_font_montserrat_16, C_DIM, "N");
    lv_obj_align(n, LV_ALIGN_TOP_MID, 0, 14);
    s_heading = ui_label(c, &lv_font_montserrat_32, C_TEXT, "--");
    lv_obj_align(s_heading, LV_ALIGN_CENTER, 0, 2);
    s_dir = ui_caption(c, "");
    lv_obj_set_style_text_font(s_dir, &lv_font_montserrat_16, 0);
    lv_obj_add_style(s_dir, ui_style_accent_text(), 0);
    lv_obj_align(s_dir, LV_ALIGN_CENTER, 0, 34);

    /* Right column */
    s_water = page_stat(p, 180, 8, "WATER", &lv_font_montserrat_40);
    s_tide = page_stat(p, 180, 92, "TIDE", &lv_font_montserrat_28);
    s_tide_bar = ui_slim_bar(p, PAGE_CONTENT_W - 180, 3, 100);
    lv_obj_set_pos(s_tide_bar, 180, 150);
    s_range = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_pos(s_range, 180, 158);

    /* Position */
    ui_fade_line(p, 0, 212, PAGE_CONTENT_W, false, LV_OPA_30);
    lv_obj_t * pin = ui_label(p, &lv_font_montserrat_20, C_DIM, LV_SYMBOL_GPS);
    lv_obj_add_style(pin, ui_style_accent_text(), 0);
    lv_obj_set_pos(pin, 0, 236);
    s_pos = ui_label(p, &lv_font_montserrat_20, C_TEXT, "");
    lv_obj_set_pos(s_pos, 32, 236);
}

static void update(const dash_data_t * d)
{
    const dash_marine_t * m = &d->marine;
    if(m->gps_fix) {
        int h = (int)m->heading_deg;
        ui_label_printf(s_heading, "%03d°", h);
        ui_label_printf(s_dir, "%s", compass(m->heading_deg));
        /* marker: 24° wide, centred on heading (0° = up) */
        int32_t a = (h + 270) % 360;
        lv_arc_set_bg_angles(s_needle, (a + 348) % 360, (a + 12) % 360);
        lv_obj_set_hidden(s_needle, false);
        ui_label_printf(s_pos, "%.4f°%c   %.4f°%c", (double)fabsf(m->lat), m->lat < 0 ? 'S' : 'N',
                        (double)fabsf(m->lon), m->lon < 0 ? 'W' : 'E');
    }
    else {
        ui_label_printf(s_heading, "--");
        ui_label_printf(s_dir, "NO GPS");
        lv_obj_set_hidden(s_needle, true);
        ui_label_printf(s_pos, "Waiting for GPS fix");
    }

    if(m->water_temp_ok) ui_label_printf(s_water, "%d°", settings_temp(m->water_temp_c));
    else ui_label_printf(s_water, "--");

    if(m->tide_ok && m->tide_max_m > m->tide_min_m) {
        ui_label_printf(s_tide, "%.2f m", (double)m->tide_m);
        int pct = (int)((m->tide_m - m->tide_min_m) / (m->tide_max_m - m->tide_min_m) * 100.0f);
        if(lv_bar_get_value(s_tide_bar) != pct) lv_bar_set_value(s_tide_bar, pct, LV_ANIM_OFF);
        ui_label_printf(s_range, "%.1f - %.1f m", (double)m->tide_min_m, (double)m->tide_max_m);
    }
    else {
        ui_label_printf(s_tide, "--");
        ui_label_printf(s_range, "needs phone hotspot");
    }
}

const ui_page_t page_marine = { "MARINE", create, update, NULL };
