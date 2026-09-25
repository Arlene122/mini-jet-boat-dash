/**
 * bracket_gauge — track + fill drawn as opaque, pre-mixed passes (glow,
 * core, highlight). The fill runs deep tone -> main -> light toward the
 * head, so it reads with depth. Ticks at 25 / 50 / 75 % on the inner side.
 * Colour family: accent tones, amber (warn) or red (critical).
 */
#include "bracket_gauge.h"

#include <math.h>

#include "ui_theme.h"

/* ---------- Config ---------- */

#define PAD      18
#define W_TRACK  12
#define W_GLOW   24
#define W_CORE   9
#define W_HI     3
#define SUBSTEPS 28          /* colour steps along the fill */

typedef struct {
    lv_point_precise_t p[4];   /* relative to object */
    int32_t permille;
    bracket_state_t st;
} bracket_t;

/* ---------- Geometry helpers ---------- */

typedef struct { lv_point_precise_t q[4]; float seg[3]; float total; } path_t;

static void path_init(path_t * pa, const bracket_t * b, const lv_area_t * a)
{
    pa->total = 0;
    for(int i = 0; i < 4; i++) {           /* bottom -> top */
        pa->q[i].x = a->x1 + b->p[3 - i].x;
        pa->q[i].y = a->y1 + b->p[3 - i].y;
    }
    for(int i = 0; i < 3; i++) {
        float dx = pa->q[i + 1].x - pa->q[i].x, dy = pa->q[i + 1].y - pa->q[i].y;
        pa->seg[i] = sqrtf(dx * dx + dy * dy);
        pa->total += pa->seg[i];
    }
}

/* Point at distance d along the path (and unit direction there) */
static lv_point_precise_t path_at(const path_t * pa, float d, float * ux, float * uy)
{
    for(int i = 0; i < 3; i++) {
        if(d <= pa->seg[i] || i == 2) {
            float t = pa->seg[i] > 0 ? LV_MIN(d, pa->seg[i]) / pa->seg[i] : 0;
            float dx = pa->q[i + 1].x - pa->q[i].x, dy = pa->q[i + 1].y - pa->q[i].y;
            if(ux) { *ux = dx / pa->seg[i]; *uy = dy / pa->seg[i]; }
            lv_point_precise_t r = { pa->q[i].x + dx * t, pa->q[i].y + dy * t };
            return r;
        }
        d -= pa->seg[i];
    }
    return pa->q[3];
}

static void seg(lv_layer_t * layer, lv_point_precise_t a, lv_point_precise_t b, lv_color_t c, int32_t w)
{
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = c;
    ld.width = w;
    ld.round_start = 1;
    ld.round_end = 1;
    ld.p1 = a;
    ld.p2 = b;
    lv_draw_line(layer, &ld);
}

/* Draw from 0 to len along the path, colour from c0 to c1 */
static void stroke(lv_layer_t * layer, const path_t * pa, float len, lv_color_t c0, lv_color_t c1, int32_t w)
{
    float step = len / SUBSTEPS;
    for(int i = 0; i < SUBSTEPS && step > 0.1f; i++) {
        lv_point_precise_t a = path_at(pa, step * i, NULL, NULL);
        lv_point_precise_t b = path_at(pa, step * (i + 1), NULL, NULL);
        seg(layer, a, b, lv_color_mix(c1, c0, (lv_opa_t)(255 * (i + 1) / SUBSTEPS)), w);
    }
}

/* ---------- Drawing ---------- */

static void bracket_draw_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    const bracket_t * b = lv_obj_get_user_data(obj);

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    path_t pa;
    path_init(&pa, b, &a);

    /* Track */
    for(int i = 0; i < 3; i++) seg(layer, pa.q[i], pa.q[i + 1], C_LINE, W_TRACK);

    /* Ticks at 25/50/75 %, on the side facing the ring */
    float cx = (a.x1 + a.x2) / 2.0f, mx = (pa.q[1].x + pa.q[2].x) / 2.0f;
    float inward = cx > mx ? 1.0f : -1.0f;
    for(int k = 1; k <= 3; k++) {
        float ux, uy;
        lv_point_precise_t p = path_at(&pa, pa.total * k / 4.0f, &ux, &uy);
        float nx = -uy, ny = ux;                     /* normal */
        if(nx * inward < 0) { nx = -nx; ny = -ny; }
        float l0 = W_TRACK / 2.0f + 4, l1 = l0 + (k == 2 ? 12 : 7);
        lv_point_precise_t t0 = { p.x + nx * l0, p.y + ny * l0 };
        lv_point_precise_t t1 = { p.x + nx * l1, p.y + ny * l1 };
        seg(layer, t0, t1, k == 2 ? C_DIM : C_OFF, 2);
    }

    if(b->permille <= 0) return;

    lv_color_t deep, main, light;
    if(b->st == BRACKET_CRIT) { main = C_RED; deep = lv_color_hex(0x8A1C1E); light = lv_color_hex(0xFFC2C3); }
    else if(b->st == BRACKET_WARN) { main = C_AMBER; deep = lv_color_hex(0x8A5A06); light = lv_color_hex(0xFFE3A8); }
    else { main = ui_theme_tone(TONE_MAIN); deep = ui_theme_tone(TONE_DEEP); light = ui_theme_tone(TONE_LIGHT); }

    float len = pa.total * b->permille / 1000.0f;
    stroke(layer, &pa, len, lv_color_mix(deep, C_BG, LV_OPA_30), lv_color_mix(main, C_BG, LV_OPA_30), W_GLOW);
    stroke(layer, &pa, len, deep, main, W_CORE);
    stroke(layer, &pa, len, main, light, W_HI);

    /* Bright head */
    lv_point_precise_t head = path_at(&pa, len, NULL, NULL);
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
