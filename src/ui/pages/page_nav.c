/**
 * page_nav — two layouts, knob push switches:
 *   MAP    — map fills the panel (offline tiles on SD later; placeholder
 *            grid + boat marker for now), heading / speed strip below.
 *   MARINE — compass, water depth, water temp, tide chart with next
 *            low/high times, GPS position.
 */
#include "pages.h"

#include <math.h>

#include "../tide_chart.h"
#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

/* ---------- Config ---------- */

#define MAP_H      300
#define COMPASS_D  140

/* ---------- Widgets ---------- */

static lv_obj_t * s_map_view, * s_marine_view, * s_tab_map, * s_tab_marine;
static lv_obj_t * s_map_head, * s_map_pos;
static lv_obj_t * s_needle, * s_heading, * s_dir, * s_depth, * s_water, * s_tide_v, * s_tide, * s_pos;
static bool s_show_map = true;

/* ---------- Helpers ---------- */

static const char * compass(float deg)
{
    static const char * n[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
    return n[(int)((fmodf(deg, 360.0f) + 22.5f) / 45.0f) % 8];
}

static void show_view(void)
{
    lv_obj_set_hidden(s_map_view, !s_show_map);
    lv_obj_set_hidden(s_marine_view, s_show_map);
    ui_set_text_color(s_tab_map, s_show_map ? C_TEXT : C_OFF);
    ui_set_text_color(s_tab_marine, s_show_map ? C_OFF : C_TEXT);
}

static void create_map(lv_obj_t * v)
{
    lv_obj_t * map = ui_box(v, 0, 0, PAGE_CONTENT_W, MAP_H);
    lv_obj_set_style_radius(map, 18, 0);
    lv_obj_set_style_clip_corner(map, true, 0);
    lv_obj_set_style_bg_color(map, C_PANEL, 0);
    lv_obj_set_style_bg_opa(map, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(map, C_LINE, 0);
    lv_obj_set_style_border_width(map, 1, 0);
    for(int i = 1; i < 7; i++) ui_fade_line(map, 0, i * MAP_H / 7, PAGE_CONTENT_W, false, LV_OPA_20);
    for(int i = 1; i < 8; i++) ui_fade_line(map, i * PAGE_CONTENT_W / 8, 0, MAP_H, true, LV_OPA_20);

    /* Boat marker (heading-up map: always points up) */
    static const lv_point_precise_t BOAT[] = { { 14, 0 }, { 28, 34 }, { 14, 26 }, { 0, 34 }, { 14, 0 } };
    lv_obj_t * boat = lv_line_create(map);
    lv_line_set_points(boat, BOAT, 5);
    lv_obj_set_style_line_width(boat, 3, 0);
    lv_obj_set_style_line_rounded(boat, true, 0);
    lv_obj_set_style_line_color(boat, ui_theme_accent(), 0);
    lv_obj_add_style(boat, ui_style_accent_text(), 0);
    lv_obj_align(boat, LV_ALIGN_CENTER, 0, 30);
    lv_obj_t * n = ui_label(map, &lv_font_montserrat_16, C_DIM, "N " LV_SYMBOL_UP);
    lv_obj_align(n, LV_ALIGN_TOP_RIGHT, -14, 12);
    lv_obj_t * note = ui_caption(map, "OFFLINE MAPS - COMING LATER");
    lv_obj_set_style_text_font(note, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_letter_space(note, 1, 0);
    lv_obj_align(note, LV_ALIGN_BOTTOM_MID, 0, -12);

    s_map_head = ui_label(v, &lv_font_montserrat_24, C_TEXT, "");
    lv_obj_set_pos(s_map_head, 0, MAP_H + 14);
    s_map_pos = ui_label(v, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_align(s_map_pos, LV_ALIGN_TOP_RIGHT, 0, MAP_H + 20);
}

static void create_marine(lv_obj_t * v)
{
    /* Compass */
    lv_obj_t * c = ui_box(v, 0, 0, COMPASS_D, COMPASS_D);
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
    s_heading = ui_label(c, &lv_font_montserrat_28, C_TEXT, "--");
    lv_obj_align(s_heading, LV_ALIGN_CENTER, 0, 2);
    s_dir = ui_caption(c, "");
    lv_obj_set_style_text_font(s_dir, &lv_font_montserrat_16, 0);
    lv_obj_add_style(s_dir, ui_style_accent_text(), 0);
    lv_obj_align(s_dir, LV_ALIGN_CENTER, 0, 30);

    /* Depth (hero) + water temp */
    s_depth = page_stat(v, 170, 4, "DEPTH", &lv_font_montserrat_40);
    s_water = page_stat(v, 170, 80, "WATER", &lv_font_montserrat_24);

    /* Tide */
    ui_fade_line(v, 0, 156, PAGE_CONTENT_W, false, LV_OPA_30);
    lv_obj_t * tc = ui_caption(v, "TIDE");
    lv_obj_set_style_text_font(tc, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(tc, 0, 170);
    s_tide_v = ui_label(v, &lv_font_montserrat_20, C_TEXT, "");
    lv_obj_align(s_tide_v, LV_ALIGN_TOP_RIGHT, 0, 166);
    s_tide = tide_chart_create(v, 0, 194, PAGE_CONTENT_W, 118);

    /* Position */
    ui_fade_line(v, 0, 322, PAGE_CONTENT_W, false, LV_OPA_30);
    lv_obj_t * pin = ui_label(v, &lv_font_montserrat_16, C_DIM, LV_SYMBOL_GPS);
    lv_obj_add_style(pin, ui_style_accent_text(), 0);
    lv_obj_set_pos(pin, 0, 336);
    s_pos = ui_label(v, &lv_font_montserrat_20, C_TEXT, "");
    lv_obj_set_pos(s_pos, 28, 333);
}

/* ---------- Page ---------- */

static void create(lv_obj_t * p)
{
    s_map_view = ui_box(p, 0, 0, PAGE_CONTENT_W, PAGE_CONTENT_H - 26);
    s_marine_view = ui_box(p, 0, 0, PAGE_CONTENT_W, PAGE_CONTENT_H - 26);
    create_map(s_map_view);
    create_marine(s_marine_view);

    /* View switch hint: MAP | MARINE  (push to switch) */
    s_tab_map = ui_caption(p, "MAP");
    lv_obj_set_style_text_font(s_tab_map, &lv_font_montserrat_16, 0);
    lv_obj_align(s_tab_map, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    s_tab_marine = ui_caption(p, "MARINE");
    lv_obj_set_style_text_font(s_tab_marine, &lv_font_montserrat_16, 0);
    lv_obj_align(s_tab_marine, LV_ALIGN_BOTTOM_LEFT, 64, 0);
    lv_obj_t * hint = ui_label(p, &lv_font_montserrat_16, C_OFF, "PUSH TO SWITCH");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    show_view();
}

static void update(const dash_data_t * d)
{
    const dash_marine_t * m = &d->marine;
    char pos[48] = "No GPS fix";
    if(m->gps_fix) {
        lv_snprintf(pos, sizeof(pos), "%.4f°%c  %.4f°%c", (double)fabsf(m->lat), m->lat < 0 ? 'S' : 'N',
                    (double)fabsf(m->lon), m->lon < 0 ? 'W' : 'E');
    }

    if(s_show_map) {
        if(m->gps_fix) ui_label_printf(s_map_head, "%03d° %s", (int)m->heading_deg, compass(m->heading_deg));
        else ui_label_printf(s_map_head, "--");
        ui_label_printf(s_map_pos, "%s", pos);
        return;
    }

    if(m->gps_fix) {
        int h = (int)m->heading_deg;
        ui_label_printf(s_heading, "%03d°", h);
        ui_label_printf(s_dir, "%s", compass(m->heading_deg));
        int32_t a = (h + 270) % 360;
        lv_arc_set_bg_angles(s_needle, (a + 348) % 360, (a + 12) % 360);
        lv_obj_set_hidden(s_needle, false);
    }
    else {
        ui_label_printf(s_heading, "--");
        ui_label_printf(s_dir, "NO GPS");
        lv_obj_set_hidden(s_needle, true);
    }
    ui_label_printf(s_pos, "%s", pos);

    if(m->depth_ok) ui_label_printf(s_depth, "%.1f m", (double)m->depth_m);
    else ui_label_printf(s_depth, "--");
    if(m->water_temp_ok) ui_label_printf(s_water, "%d%s", settings_temp(m->water_temp_c), settings_temp_unit());
    else ui_label_printf(s_water, "--");

    if(m->tide_ok) {
        bool rising = m->tide_phase < 0.5f;
        ui_label_printf(s_tide_v, "%.2f m  %s", (double)m->tide_m, rising ? LV_SYMBOL_UP : LV_SYMBOL_DOWN);
        int32_t now = d->clock_h < 0 ? -1 : d->clock_h * 60 + d->clock_m;
        tide_chart_set(s_tide, m->tide_phase, now, m->tide_period_min, settings_get()->clock_12h);
        lv_obj_set_hidden(s_tide, false);
    }
    else {
        ui_label_printf(s_tide_v, "needs phone hotspot");
        lv_obj_set_hidden(s_tide, true);
    }
}

static bool input(ui_input_t in)
{
    if(in != UI_IN_SELECT) return false;
    s_show_map = !s_show_map;
    show_view();
    return true;
}

const ui_page_t page_nav = { "NAV", create, update, input };
