/**
 * gauge_speed — layered lv_arc rings (track, soft glow, core, bright inner
 * highlight) around a dark disc that picks up the accent at the bottom.
 * Ring always runs in km/h internally; ticks re-space for the shown unit.
 */
#include "gauge_speed.h"

#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"
#include "../settings/settings.h"

LV_FONT_DECLARE(font_digits_160);

/* ---------- Geometry ---------- */

#define ROT       135          /* start lower-left, 270° clockwise */
#define SWEEP     270
#define R_HAIR    GAUGE_R      /* thin outer circle */
#define R_GLOW    (GAUGE_R - 6)
#define R_CORE    (GAUGE_R - 16)
#define R_HI      (GAUGE_R - 21)
#define R_DISC    (GAUGE_R - 34)
#define R_INNER   (GAUGE_R - 70)
#define TICK_STEP 5            /* one tick per 5 units, major every 10 */

/* ---------- Widgets ---------- */

static lv_obj_t * s_glow, * s_core, * s_hi, * s_scale;
static lv_obj_t * s_speed, * s_unit, * s_alt;
static lv_obj_t * s_pill, * s_slot[3], * s_brake;
static int s_shown_speed = -1;   /* number hysteresis: no flicker between two values */

/* ---------- Helpers ---------- */

static lv_obj_t * ring(lv_obj_t * parent, int32_t r, int32_t w, int32_t a0, int32_t a1)
{
    lv_obj_t * a = lv_arc_create(parent);
    lv_obj_remove_style_all(a);
    lv_obj_set_size(a, r * 2, r * 2);
    lv_obj_align(a, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_clickable(a, false);
    lv_arc_set_rotation(a, ROT);
    lv_arc_set_bg_angles(a, a0, a1);
    lv_arc_set_range(a, 0, GAUGE_SPEED_MAX * 10);   /* 0.1 km/h steps: smooth sweep */
    lv_arc_set_value(a, 0);
    lv_obj_set_style_arc_width(a, w, LV_PART_MAIN);
    lv_obj_set_style_arc_width(a, w, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(a, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
    return a;
}

static lv_obj_t * track(lv_obj_t * parent, int32_t r, int32_t w, int32_t a0, int32_t a1, lv_color_t c)
{
    lv_obj_t * a = ring(parent, r, w, a0, a1);
    lv_obj_set_style_arc_opa(a, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_arc_color(a, c, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
    return a;
}

static void pill_slot_style(lv_obj_t * slot, bool active)
{
    lv_obj_set_style_bg_opa(slot, active ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    ui_set_text_color(lv_obj_get_child(slot, 0), active ? C_BG : C_DIM);
}

/* ---------- API ---------- */

void gauge_speed_create(lv_obj_t * parent)
{
    lv_obj_t * g = ui_box(parent, GAUGE_CX - GAUGE_R - 8, GAUGE_CY - GAUGE_R - 8,
                          GAUGE_R * 2 + 16, GAUGE_R * 2 + 16);

    /* Disc */
    lv_obj_t * disc = ui_box(g, 0, 0, R_DISC * 2, R_DISC * 2);
    lv_obj_align(disc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(disc, ui_style_disc(), 0);
    lv_obj_set_style_border_color(disc, C_LINE, 0);
    lv_obj_set_style_border_width(disc, 1, 0);
    track(g, R_INNER, 1, 0, 360, C_LINE);

    /* Soft accent glow pooling at the bottom of the disc */
    static const struct { int32_t w; lv_opa_t opa; int32_t a0, a1; } POOL[] = {
        { 30, (lv_opa_t)14, 50, 130 }, { 14, (lv_opa_t)22, 60, 120 },
    };
    for(unsigned i = 0; i < sizeof(POOL) / sizeof(POOL[0]); i++) {
        lv_obj_t * pool = ring(g, R_DISC - 1, POOL[i].w, 0, 0);
        lv_arc_set_rotation(pool, 0);
        lv_arc_set_bg_angles(pool, POOL[i].a0, POOL[i].a1);
        lv_obj_set_style_arc_opa(pool, POOL[i].opa, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(pool, true, LV_PART_MAIN);
        lv_obj_add_style(pool, ui_style_accent_arc(), LV_PART_MAIN);
    }

    /* Fine ticks inside the ring */
    s_scale = lv_scale_create(g);
    lv_obj_remove_style_all(s_scale);
    lv_obj_set_size(s_scale, (R_DISC - 6) * 2, (R_DISC - 6) * 2);
    lv_obj_align(s_scale, LV_ALIGN_CENTER, 0, 0);
    lv_scale_set_mode(s_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(s_scale, ROT);
    lv_scale_set_major_tick_every(s_scale, 2);
    lv_scale_set_label_show(s_scale, false);
    lv_obj_set_style_arc_width(s_scale, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(s_scale, C_OFF, LV_PART_ITEMS);
    lv_obj_set_style_line_width(s_scale, 2, LV_PART_ITEMS);
    lv_obj_set_style_length(s_scale, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_color(s_scale, C_DIM, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(s_scale, 2, LV_PART_INDICATOR);
    lv_obj_set_style_length(s_scale, 13, LV_PART_INDICATOR);

    /* Rings: hairline, track, glow, core, highlight */
    lv_obj_t * hair = track(g, R_HAIR, 2, 0, 360, C_LINE);
    lv_obj_add_style(hair, ui_style_accent_dim(), LV_PART_MAIN);
    track(g, R_CORE, 8, 0, SWEEP, C_LINE);
    s_glow = ring(g, R_GLOW, 26, 0, SWEEP);
    lv_obj_add_style(s_glow, ui_style_accent_glow(), LV_PART_INDICATOR);
    s_core = ring(g, R_CORE, 8, 0, SWEEP);
    lv_obj_add_style(s_core, ui_style_accent_arc(), LV_PART_INDICATOR);
    s_hi = ring(g, R_HI, 2, 0, SWEEP);
    lv_obj_add_style(s_hi, ui_style_accent_hi(), LV_PART_INDICATOR);

    /* Numbers */
    s_speed = ui_label(g, &font_digits_160, C_TEXT, "0");
    lv_obj_align(s_speed, LV_ALIGN_CENTER, 0, -24);
    s_unit = ui_caption(g, "");
    lv_obj_add_style(s_unit, ui_style_accent_text(), 0);
    lv_obj_align(s_unit, LV_ALIGN_CENTER, 0, 62);
    s_alt = ui_label(g, &lv_font_montserrat_20, C_DIM, "");
    lv_obj_align(s_alt, LV_ALIGN_CENTER, 0, 94);

    /* iBR pill: R N F, active slot filled with accent */
    s_pill = ui_box(g, 0, 0, 174, 46);
    lv_obj_align(s_pill, LV_ALIGN_CENTER, 0, 158);
    lv_obj_set_style_radius(s_pill, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_pill, C_PANEL, 0);
    lv_obj_set_style_bg_opa(s_pill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_pill, C_LINE, 0);
    lv_obj_set_style_border_width(s_pill, 1, 0);
    static const char * letters[3] = { "R", "N", "F" };
    for(int i = 0; i < 3; i++) {
        lv_obj_t * s = ui_box(s_pill, 5 + i * 56, 5, 52, 36);
        lv_obj_set_style_radius(s, LV_RADIUS_CIRCLE, 0);
        lv_obj_add_style(s, ui_style_accent_bg(), 0);
        lv_obj_t * l = ui_label(s, &lv_font_montserrat_24, C_DIM, letters[i]);
        lv_obj_center(l);
        s_slot[i] = s;
        pill_slot_style(s, false);
    }
    s_brake = ui_label(s_pill, &lv_font_montserrat_20, C_RED, "BRAKE");
    lv_obj_set_style_text_letter_space(s_brake, 4, 0);
    lv_obj_center(s_brake);
    lv_obj_set_hidden(s_brake, true);

    gauge_speed_apply_settings();
}

void gauge_speed_apply_settings(void)
{
    /* Ticks every 5 units; the scale covers only whole ticks, so shrink
     * its angle to match the part of the km/h ring it represents. */
    float unit_max = settings_speed((float)GAUGE_SPEED_MAX);
    int32_t ticks = (int32_t)(unit_max / TICK_STEP) + 1;
    int32_t covered = (ticks - 1) * TICK_STEP;
    lv_scale_set_total_tick_count(s_scale, ticks);
    lv_scale_set_range(s_scale, 0, covered);
    lv_scale_set_angle_range(s_scale, (int32_t)(SWEEP * covered / unit_max));
    lv_label_set_text(s_unit, settings_speed_unit());
}

void gauge_speed_set_arc(int32_t speed_dkmh)
{
    speed_dkmh = LV_CLAMP(0, speed_dkmh, GAUGE_SPEED_MAX * 10);
    if(lv_arc_get_value(s_core) == speed_dkmh) return;
    lv_arc_set_value(s_glow, speed_dkmh);
    lv_arc_set_value(s_core, speed_dkmh);
    lv_arc_set_value(s_hi, speed_dkmh);
}

void gauge_speed_update(const dash_data_t * d, bool arc)
{
    /* Only change the number once the speed is clearly past the next value */
    float sp = settings_speed(d->speed_kmh);
    if(s_shown_speed < 0 || sp > s_shown_speed + 0.8f || sp < s_shown_speed - 0.8f) s_shown_speed = (int)(sp + 0.5f);
    ui_label_printf(s_speed, "%d", s_shown_speed);
    float alt = settings_speed_alt(d->speed_kmh);
    ui_label_printf(s_alt, "%d.%d %s", (int)(alt * 10 + 0.5f) / 10, (int)(alt * 10 + 0.5f) % 10,
                    settings_speed_alt_unit());
    if(arc) gauge_speed_set_arc((int32_t)(d->speed_kmh * 10.0f + 0.5f));

    bool brake = d->ibr == DASH_IBR_BRAKE;
    int active = d->ibr == DASH_IBR_REVERSE ? 0 : d->ibr == DASH_IBR_FORWARD ? 2 : 1;
    for(int i = 0; i < 3; i++) {
        lv_obj_set_hidden(s_slot[i], brake);
        pill_slot_style(s_slot[i], i == active);
    }
    lv_obj_set_hidden(s_brake, !brake);
    lv_obj_set_style_border_color(s_pill, brake ? C_RED : C_LINE, 0);
}
