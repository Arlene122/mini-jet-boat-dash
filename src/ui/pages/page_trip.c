/**
 * page_trip — distance as the hero, three small stats in a row with
 * hairline dividers, fuel used underneath.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

LV_FONT_DECLARE(font_digits_72);

static lv_obj_t * s_dist, * s_unit, * s_top, * s_top_u, * s_time, * s_rpm, * s_fuel;

static lv_obj_t * mini(lv_obj_t * p, int32_t x, const char * cap)
{
    lv_obj_t * c = ui_caption(p, cap);
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_letter_space(c, 1, 0);
    lv_obj_set_pos(c, x, 150);
    lv_obj_t * v = ui_label(p, &lv_font_montserrat_28, C_TEXT, "--");
    lv_obj_set_pos(v, x, 172);
    return v;
}

static void create(lv_obj_t * p)
{
    lv_obj_t * c = ui_caption(p, "DISTANCE");
    lv_obj_set_style_text_font(c, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(c, 0, 0);
    s_dist = ui_label(p, &font_digits_72, C_TEXT, "0.0");
    lv_obj_set_pos(s_dist, 0, 20);
    s_unit = ui_caption(p, "");
    lv_obj_add_style(s_unit, ui_style_accent_text(), 0);

    ui_fade_line(p, 0, 128, PAGE_CONTENT_W, false, LV_OPA_30);
    s_top = mini(p, 0, "TOP SPEED");
    s_top_u = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    s_time = mini(p, 128, "RIDE TIME");
    s_rpm = mini(p, 245, "MAX RPM");
    ui_fade_line(p, 114, 150, 60, true, LV_OPA_40);
    ui_fade_line(p, 231, 150, 60, true, LV_OPA_40);
    ui_fade_line(p, 0, 236, PAGE_CONTENT_W, false, LV_OPA_30);

    s_fuel = page_stat(p, 0, 258, "FUEL USED", &lv_font_montserrat_28);
}

static void update(const dash_data_t * d)
{
    const dash_trip_t * t = &d->trip;
    ui_label_printf(s_dist, "%.1f", (double)settings_dist(d->trip_km));
    ui_label_printf(s_unit, "%s", settings_dist_unit());
    lv_obj_align_to(s_unit, s_dist, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, -14);
    ui_label_printf(s_top, "%d", (int)(settings_speed(t->top_speed_kmh) + 0.5f));
    ui_label_printf(s_top_u, "%s", settings_speed_unit());
    lv_obj_align_to(s_top_u, s_top, LV_ALIGN_OUT_RIGHT_BOTTOM, 6, -4);
    ui_label_printf(s_time, "%d:%02d", (int)(t->ride_time_s / 3600), (int)(t->ride_time_s / 60 % 60));
    ui_label_printf(s_rpm, "%d", t->max_rpm);
    ui_label_printf(s_fuel, "%.1f L", (double)t->fuel_used_l);
}

const ui_page_t page_trip = { "TRIP", create, update, NULL };
