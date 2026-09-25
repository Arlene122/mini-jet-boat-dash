/**
 * gauge_speed — built from lv_arc + lv_scale. Arcs sweep 240 deg starting
 * lower-left (rotation 150), leaving the bottom open for the iBR letters.
 * Each arc has a wide low-opacity twin behind it for a soft glow.
 */
#include "gauge_speed.h"

#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"

LV_FONT_DECLARE(font_digits_160);

/* ---------- Geometry ---------- */

#define ARC_ROT        150
#define ARC_SWEEP      240
#define R_SPEED        GAUGE_R
#define R_SCALE        (GAUGE_R - 12)
#define R_RPM          162
#define W_LINE         5
#define W_GLOW         18

/* ---------- Widgets ---------- */

static lv_obj_t * s_speed_arc, * s_speed_glow;
static lv_obj_t * s_rpm_arc, * s_rpm_glow;
static lv_obj_t * s_speed, * s_knots;
static lv_obj_t * s_ibr[3];      /* R N F */
static lv_obj_t * s_brake;
static bool s_redline;

/* ---------- Helpers ---------- */

/* Plain arc centred on the gauge. value_arc=false -> static track */
static lv_obj_t * make_arc(lv_obj_t * parent, int32_t r, int32_t w, int32_t max)
{
    lv_obj_t * a = lv_arc_create(parent);
    lv_obj_remove_style_all(a);
    lv_obj_set_size(a, r * 2, r * 2);
    lv_obj_align(a, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_clickable(a, false);
    lv_arc_set_rotation(a, ARC_ROT);
    lv_arc_set_bg_angles(a, 0, ARC_SWEEP);
    lv_arc_set_range(a, 0, max);
    lv_arc_set_value(a, 0);
    lv_obj_set_style_arc_width(a, w, LV_PART_MAIN);
    lv_obj_set_style_arc_width(a, w, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(a, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
    return a;
}

static lv_obj_t * make_track(lv_obj_t * parent, int32_t r, int32_t w, int32_t a0, int32_t a1,
                             lv_color_t c, lv_opa_t opa)
{
    lv_obj_t * a = make_arc(parent, r, w, 1);
    lv_arc_set_bg_angles(a, a0, a1);
    lv_obj_set_style_arc_color(a, c, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(a, opa, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
    return a;
}

static void make_scale(lv_obj_t * parent)
{
    lv_obj_t * sc = lv_scale_create(parent);
    lv_obj_remove_style_all(sc);
    lv_obj_set_size(sc, R_SCALE * 2, R_SCALE * 2);
    lv_obj_align(sc, LV_ALIGN_CENTER, 0, 0);
    lv_scale_set_mode(sc, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(sc, 0, GAUGE_SPEED_MAX);
    lv_scale_set_total_tick_count(sc, GAUGE_SPEED_MAX / 5 + 1);   /* tick every 5 km/h */
    lv_scale_set_major_tick_every(sc, 2);                         /* label every 10 */
    lv_scale_set_angle_range(sc, ARC_SWEEP);
    lv_scale_set_rotation(sc, ARC_ROT);
    lv_scale_set_label_show(sc, true);
    lv_obj_set_style_arc_width(sc, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(sc, C_OFF, LV_PART_ITEMS);
    lv_obj_set_style_line_width(sc, 2, LV_PART_ITEMS);
    lv_obj_set_style_length(sc, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_color(sc, C_DIM, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(sc, 3, LV_PART_INDICATOR);
    lv_obj_set_style_length(sc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_text_font(sc, &lv_font_montserrat_20, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(sc, C_DIM, LV_PART_INDICATOR);
    lv_obj_set_style_pad_radial(sc, 10, LV_PART_INDICATOR);
}

/* ---------- API ---------- */

void gauge_speed_create(lv_obj_t * parent)
{
    lv_obj_t * g = ui_box(parent, GAUGE_CX - GAUGE_R - 10, GAUGE_CY - GAUGE_R - 10,
                          GAUGE_R * 2 + 20, GAUGE_R * 2 + 20);

    /* Speed arc: glow, track, value */
    s_speed_glow = make_arc(g, R_SPEED + 6, W_GLOW, GAUGE_SPEED_MAX);
    lv_obj_add_style(s_speed_glow, ui_style_accent_glow(), LV_PART_INDICATOR);
    make_track(g, R_SPEED, W_LINE, 0, ARC_SWEEP, C_LINE, LV_OPA_COVER);
    s_speed_arc = make_arc(g, R_SPEED, W_LINE, GAUGE_SPEED_MAX);
    lv_obj_add_style(s_speed_arc, ui_style_accent_arc(), LV_PART_INDICATOR);
    make_scale(g);

    /* RPM arc: glow, track, redline zone, value */
    int32_t red_a = ARC_SWEEP * GAUGE_RPM_RED / GAUGE_RPM_MAX;
    s_rpm_glow = make_arc(g, R_RPM + 6, W_GLOW - 4, GAUGE_RPM_MAX);
    lv_obj_add_style(s_rpm_glow, ui_style_accent_glow(), LV_PART_INDICATOR);
    make_track(g, R_RPM, 3, 0, red_a, C_LINE, LV_OPA_COVER);
    make_track(g, R_RPM, 3, red_a, ARC_SWEEP, C_RED, LV_OPA_60);
    s_rpm_arc = make_arc(g, R_RPM, 3, GAUGE_RPM_MAX);
    lv_obj_add_style(s_rpm_arc, ui_style_accent_arc(), LV_PART_INDICATOR);

    /* Numbers */
    s_speed = ui_label(g, &font_digits_160, C_TEXT, "0");
    lv_obj_align(s_speed, LV_ALIGN_CENTER, 0, -26);
    lv_obj_t * unit = ui_caption(g, "KM/H");
    lv_obj_add_style(unit, ui_style_accent_text(), 0);
    lv_obj_align(unit, LV_ALIGN_CENTER, 0, 56);
    s_knots = ui_label(g, &lv_font_montserrat_24, C_DIM, "");
    lv_obj_align(s_knots, LV_ALIGN_CENTER, 0, 88);

    /* iBR in the bottom gap */
    static const char * letters[3] = { "R", "N", "F" };
    for(int i = 0; i < 3; i++) {
        s_ibr[i] = ui_label(g, &lv_font_montserrat_48, C_OFF, letters[i]);
        lv_obj_align(s_ibr[i], LV_ALIGN_CENTER, (i - 1) * 72, 184);
    }
    s_brake = ui_label(g, &lv_font_montserrat_32, C_RED, "BRAKE");
    lv_obj_set_style_text_letter_space(s_brake, 4, 0);
    lv_obj_align(s_brake, LV_ALIGN_CENTER, 0, 184);
    lv_obj_set_hidden(s_brake, true);
}

void gauge_speed_set_arcs(int32_t speed_kmh, int32_t rpm)
{
    if(speed_kmh > GAUGE_SPEED_MAX) speed_kmh = GAUGE_SPEED_MAX;
    if(lv_arc_get_value(s_speed_arc) != speed_kmh) {
        lv_arc_set_value(s_speed_arc, speed_kmh);
        lv_arc_set_value(s_speed_glow, speed_kmh);
    }
    int32_t r = rpm / 25 * 25;  /* ignore tiny jitter */
    if(lv_arc_get_value(s_rpm_arc) != r) {
        lv_arc_set_value(s_rpm_arc, r);
        lv_arc_set_value(s_rpm_glow, r);
    }
    bool red = rpm >= GAUGE_RPM_RED;
    if(red != s_redline) {
        s_redline = red;
        if(red) {
            lv_obj_set_style_arc_color(s_rpm_arc, C_RED, LV_PART_INDICATOR);
            lv_obj_set_style_arc_color(s_rpm_glow, C_RED, LV_PART_INDICATOR);
        }
        else {
            lv_obj_remove_local_style_prop(s_rpm_arc, LV_STYLE_ARC_COLOR, LV_PART_INDICATOR);
            lv_obj_remove_local_style_prop(s_rpm_glow, LV_STYLE_ARC_COLOR, LV_PART_INDICATOR);
        }
    }
}

void gauge_speed_update(const dash_data_t * d, bool arcs)
{
    int v = (int)(d->speed_kmh + 0.5f);
    ui_label_printf(s_speed, "%d", v);
    int kn10 = (int)(d->speed_kmh / 1.852f * 10.0f + 0.5f);
    ui_label_printf(s_knots, "%d.%d kn", kn10 / 10, kn10 % 10);
    if(arcs) gauge_speed_set_arcs(v, d->rpm);

    bool brake = d->ibr == DASH_IBR_BRAKE;
    int active = d->ibr == DASH_IBR_REVERSE ? 0 : d->ibr == DASH_IBR_FORWARD ? 2 : 1;
    for(int i = 0; i < 3; i++) {
        lv_obj_set_hidden(s_ibr[i], brake);
        ui_set_text_color(s_ibr[i], i == active ? C_TEXT : C_OFF);
    }
    lv_obj_set_hidden(s_brake, !brake);
}
