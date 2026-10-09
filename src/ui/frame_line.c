/**
 * frame_line — path = flat, S-curve (smoothstep), flat notch, S-curve, flat.
 * Drawn as 3 passes (wide faint, medium, thin bright) for a soft glow.
 * Glow colours are pre-mixed with the background and drawn opaque, so
 * overlapping segment caps don't stack into visible beads.
 * Static: only redrawn when the accent colour changes (or during the
 * key-on trace, which reveals it from the centre out with a spark at each tip).
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
    int32_t reveal;              /* 0..1000 of the half-width shown */
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

/* Keep the part of the path between xl and xr (the path runs left to right). */
static int clip_path(const lv_point_precise_t * p, int n, int32_t xl, int32_t xr, lv_point_precise_t * out)
{
    int m = 0;
    for(int i = 0; i < n - 1; i++) {
        lv_point_precise_t a = p[i], b = p[i + 1];
        if(b.x < xl || a.x > xr || b.x == a.x) continue;
        if(a.x < xl) { a.y += (b.y - a.y) * (xl - a.x) / (b.x - a.x); a.x = xl; }
        if(b.x > xr) { b.y = a.y + (b.y - a.y) * (xr - a.x) / (b.x - a.x); b.x = xr; }
        if(m == 0) out[m++] = a;
        out[m++] = b;
    }
    return m;
}

static void spark(lv_layer_t * layer, lv_point_precise_t p, int32_t r, lv_color_t c, lv_opa_t opa)
{
    lv_draw_rect_dsc_t rd;
    lv_draw_rect_dsc_init(&rd);
    rd.radius = LV_RADIUS_CIRCLE;
    rd.bg_color = c;
    rd.bg_opa = opa;
    lv_area_t a = { (int32_t)p.x - r, (int32_t)p.y - r, (int32_t)p.x + r, (int32_t)p.y + r };
    lv_draw_rect(layer, &rd, &a);
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
    if(c->reveal <= 0) return;
    bool tracing = c->reveal < 1000;
    if(tracing) {
        static lv_point_precise_t q[FRAME_PTS_MAX];
        int32_t cx = a.x1 + UI_HOR_RES / 2, hw = (UI_HOR_RES / 2 + 8) * c->reveal / 1000;
        n = clip_path(p, n, cx - hw, cx + hw, q);
        lv_memcpy(p, q, sizeof(q[0]) * (size_t)n);
        if(n < 2) return;
    }
    lv_color_t acc = ui_theme_accent();
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_color_t deep = ui_theme_tone(TONE_DEEP);
    draw_pass(layer, p, n, lv_color_mix(deep, C_BG, (lv_opa_t)ui_glow(LV_OPA_20)), ui_glow(8), LV_OPA_COVER);
    draw_pass(layer, p, n, lv_color_mix(deep, C_BG, (lv_opa_t)ui_glow(LV_OPA_60)), LV_MAX(2, ui_glow(4)), LV_OPA_COVER);
    draw_pass(layer, p, n, lv_color_mix(acc, ui_theme_tone(TONE_MID), LV_OPA_60), 2, LV_OPA_COVER);
    if(tracing) {
        lv_color_t lt = ui_theme_tone(TONE_LIGHT);
        for(int k = 0; k < 2; k++) {
            lv_point_precise_t tip = p[k ? n - 1 : 0];
            spark(layer, tip, 9, acc, LV_OPA_20);
            spark(layer, tip, 5, lt, LV_OPA_50);
            spark(layer, tip, 2, lv_color_white(), LV_OPA_COVER);
        }
    }
}

void frame_line_set_reveal(lv_obj_t * obj, int32_t permille)
{
    frame_cfg_t * c = lv_obj_get_user_data(obj);
    if(!c || c->reveal == permille) return;
    c->reveal = permille;
    lv_obj_invalidate(obj);
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
    *c = (frame_cfg_t){ y_out, y_in, x1, x2, curve, 1000 };
    lv_obj_set_user_data(obj, c);
    lv_obj_add_event_cb(obj, frame_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, frame_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}
