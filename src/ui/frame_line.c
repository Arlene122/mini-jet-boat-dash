/**
 * frame_line — path = flat, S-curve (smoothstep), flat notch, S-curve, flat.
 * Drawn as 3 passes (wide faint, medium, thin bright) for a soft glow.
 * Glow colours are pre-mixed with the background and drawn opaque, so
 * overlapping segment caps don't stack into visible beads.
 * Static: only redrawn when the accent colour changes.
 */
#include "frame_line.h"

#include "ui.h"
#include "ui_theme.h"

/* ---------- Config ---------- */

#define CURVE_STEPS  18
#define GLOW_PAD     10
#define FRAME_PTS_MAX     (4 + 2 * CURVE_STEPS)

typedef struct {
    int32_t y_out, y_in, x1, x2, curve;
} frame_cfg_t;

/* ---------- Path ---------- */

static int build_path(const frame_cfg_t * c, int32_t ox, int32_t oy, lv_point_precise_t * p)
{
    int n = 0;
    p[n].x = ox; p[n++].y = oy + c->y_out;
    for(int side = 0; side < 2; side++) {
        int32_t xa = side == 0 ? c->x1 - c->curve : c->x2;
        for(int i = 0; i <= CURVE_STEPS; i++) {
            float t = (float)i / CURVE_STEPS;
            float s = t * t * (3.0f - 2.0f * t);
            if(side == 1) s = 1.0f - s;
            p[n].x = ox + xa + (lv_value_precise_t)(t * c->curve);
            p[n++].y = oy + c->y_out + (lv_value_precise_t)(s * (c->y_in - c->y_out));
        }
    }
    p[n].x = ox + UI_HOR_RES; p[n++].y = oy + c->y_out;
    return n;
}

static void draw_pass(lv_layer_t * layer, const lv_point_precise_t * p, int n,
                      lv_color_t color, int32_t width, lv_opa_t opa)
{
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = color;
    ld.width = width;
    ld.opa = opa;
    ld.round_start = 1;
    ld.round_end = 1;
    for(int i = 0; i < n - 1; i++) {
        ld.p1 = p[i];
        ld.p2 = p[i + 1];
        lv_draw_line(layer, &ld);
    }
}

static void frame_draw_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    const frame_cfg_t * c = lv_obj_get_user_data(obj);
    lv_area_t a;
    lv_obj_get_coords(obj, &a);

    lv_point_precise_t p[FRAME_PTS_MAX];
    int n = build_path(c, a.x1, a.y1 - lv_obj_get_y(obj), p);
    lv_color_t acc = ui_theme_accent();
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_color_t deep = ui_theme_tone(TONE_DEEP);
    draw_pass(layer, p, n, lv_color_mix(deep, C_BG, LV_OPA_20), 8, LV_OPA_COVER);
    draw_pass(layer, p, n, lv_color_mix(deep, C_BG, LV_OPA_60), 4, LV_OPA_COVER);
    draw_pass(layer, p, n, lv_color_mix(acc, ui_theme_tone(TONE_MID), LV_OPA_60), 2, LV_OPA_COVER);
}

static void frame_delete_cb(lv_event_t * e)
{
    lv_free(lv_obj_get_user_data(lv_event_get_target_obj(e)));
}

/* ---------- API ---------- */

lv_obj_t * frame_line_create(lv_obj_t * parent, int32_t y_out, int32_t y_in,
                             int32_t x1, int32_t x2, int32_t curve)
{
    int32_t y_min = LV_MIN(y_out, y_in) - GLOW_PAD;
    int32_t y_max = LV_MAX(y_out, y_in) + GLOW_PAD;

    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, 0, y_min);
    lv_obj_set_size(obj, UI_HOR_RES, y_max - y_min);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    lv_obj_add_style(obj, ui_style_accent_text(), 0);   /* redraw on mode change */

    frame_cfg_t * c = lv_malloc(sizeof(*c));
    *c = (frame_cfg_t){ y_out, y_in, x1, x2, curve };
    lv_obj_set_user_data(obj, c);
    lv_obj_add_event_cb(obj, frame_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, frame_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}
