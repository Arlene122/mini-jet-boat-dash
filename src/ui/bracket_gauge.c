/**
 * bracket_gauge — track + fill drawn as opaque, pre-mixed passes (glow,
 * core, highlight) so overlapping joints stay clean. Colour: accent,
 * amber (warn) or red (critical).
 */
#include "bracket_gauge.h"

#include <math.h>

#include "ui_theme.h"

/* ---------- Config ---------- */

#define PAD      16
#define W_TRACK  12
#define W_GLOW   24
#define W_CORE   9
#define W_HI     3

typedef struct {
    lv_point_precise_t p[4];   /* relative to object */
    int32_t permille;
    bracket_state_t st;
} bracket_t;

/* ---------- Drawing ---------- */

static void seg(lv_layer_t * layer, lv_point_precise_t a, lv_point_precise_t b,
                lv_color_t c, int32_t w)
{
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = c;
    ld.width = w;
    ld.opa = LV_OPA_COVER;
    ld.round_start = 1;
    ld.round_end = 1;
    ld.p1 = a;
    ld.p2 = b;
    lv_draw_line(layer, &ld);
}

/* Draw the path from the foot back toward the top, up to `len` px */
static lv_point_precise_t path(lv_layer_t * layer, const lv_point_precise_t * q, float len,
                               lv_color_t c, int32_t w)
{
    lv_point_precise_t head = q[0];
    for(int i = 0; i < 3 && len > 0; i++) {
        float dx = q[i + 1].x - q[i].x, dy = q[i + 1].y - q[i].y;
        float l = sqrtf(dx * dx + dy * dy);
        float t = len >= l ? 1.0f : len / l;
        lv_point_precise_t e = { q[i].x + dx * t, q[i].y + dy * t };
        seg(layer, q[i], e, c, w);
        head = e;
        len -= l;
    }
    return head;
}

static void bracket_draw_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    const bracket_t * b = lv_obj_get_user_data(obj);

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    lv_point_precise_t q[4];   /* foot -> top */
    float total = 0;
    for(int i = 0; i < 4; i++) {
        q[i].x = a.x1 + b->p[3 - i].x;
        q[i].y = a.y1 + b->p[3 - i].y;
        if(i) total += sqrtf((q[i].x - q[i - 1].x) * (q[i].x - q[i - 1].x) +
                             (q[i].y - q[i - 1].y) * (q[i].y - q[i - 1].y));
    }

    path(layer, q, total, C_LINE, W_TRACK);
    if(b->permille <= 0) return;

    lv_color_t c = b->st == BRACKET_CRIT ? C_RED : b->st == BRACKET_WARN ? C_AMBER : ui_theme_accent();
    float len = total * b->permille / 1000.0f;
    path(layer, q, len, lv_color_mix(c, C_BG, LV_OPA_20), W_GLOW);
    path(layer, q, len, c, W_CORE);
    lv_point_precise_t head = path(layer, q, len, lv_color_mix(lv_color_white(), c, LV_OPA_60), W_HI);

    /* Bright head */
    lv_draw_rect_dsc_t rd;
    lv_draw_rect_dsc_init(&rd);
    rd.radius = LV_RADIUS_CIRCLE;
    rd.bg_color = lv_color_white();
    lv_area_t h = { (int32_t)head.x - 5, (int32_t)head.y - 5, (int32_t)head.x + 5, (int32_t)head.y + 5 };
    lv_draw_rect(layer, &rd, &h);
}

static void bracket_delete_cb(lv_event_t * e)
{
    lv_free(lv_obj_get_user_data(lv_event_get_target_obj(e)));
}

/* ---------- API ---------- */

lv_obj_t * bracket_gauge_create(lv_obj_t * parent, const lv_point_t pts[4])
{
    int32_t x1 = pts[0].x, y1 = pts[0].y, x2 = x1, y2 = y1;
    for(int i = 1; i < 4; i++) {
        x1 = LV_MIN(x1, pts[i].x); y1 = LV_MIN(y1, pts[i].y);
        x2 = LV_MAX(x2, pts[i].x); y2 = LV_MAX(y2, pts[i].y);
    }
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x1 - PAD, y1 - PAD);
    lv_obj_set_size(obj, x2 - x1 + 2 * PAD, y2 - y1 + 2 * PAD);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    lv_obj_add_style(obj, ui_style_accent_text(), 0);   /* redraw on mode change */

    bracket_t * b = lv_malloc_zeroed(sizeof(*b));
    for(int i = 0; i < 4; i++) {
        b->p[i].x = pts[i].x - x1 + PAD;
        b->p[i].y = pts[i].y - y1 + PAD;
    }
    lv_obj_set_user_data(obj, b);
    lv_obj_add_event_cb(obj, bracket_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, bracket_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}

void bracket_gauge_set(lv_obj_t * obj, int32_t permille, bracket_state_t st)
{
    bracket_t * b = lv_obj_get_user_data(obj);
    permille = LV_CLAMP(0, permille, 1000);
    if(b->permille == permille && b->st == st) return;
    b->permille = permille;
    b->st = st;
    lv_obj_invalidate(obj);
}
