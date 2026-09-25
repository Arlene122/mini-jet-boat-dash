/**
 * page_lights — top view of the boat with 5 lights.
 * UP/DOWN selects a light, SELECT toggles it (via dash_cmd -> relays).
 * Light names are placeholders until the owner defines them.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../dash_data/dash_cmd.h"

/* ---------- Boat outline (content-area coords, bow up) ---------- */

#define HULL_X  150
static const lv_point_precise_t HULL[] = {
    { 80, 0 }, { 120, 30 }, { 150, 110 }, { 160, 200 }, { 155, 300 },
    { 5, 300 }, { 0, 200 }, { 10, 110 }, { 40, 30 }, { 80, 0 },
};

typedef struct { int32_t x, y, w, h; const char * name; } light_pos_t;
static const light_pos_t LIGHTS[DASH_LIGHT_COUNT] = {
    { 62, 40, 36, 36, "LIGHT 1  (bow)" },
    { 24, 140, 18, 60, "LIGHT 2  (port)" },
    { 118, 140, 18, 60, "LIGHT 3  (starboard)" },
    { 62, 210, 36, 36, "LIGHT 4  (deck)" },
    { 40, 270, 80, 16, "LIGHT 5  (stern)" },
};

static lv_obj_t * s_light[DASH_LIGHT_COUNT];
static lv_obj_t * s_name, * s_state;
static int s_focus;
static uint8_t s_shown = 0xFF;

static void show(uint8_t lights)
{
    for(int i = 0; i < DASH_LIGHT_COUNT; i++) {
        bool on = lights & (1u << i);
        lv_obj_t * o = s_light[i];
        lv_obj_set_style_bg_opa(o, on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_opa(o, on ? LV_OPA_60 : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(o, i == s_focus ? C_TEXT : C_DIM, 0);
        lv_obj_set_style_border_width(o, i == s_focus ? 3 : 2, 0);
    }
    ui_label_printf(s_name, "%s", LIGHTS[s_focus].name);
    bool on = lights & (1u << s_focus);
    ui_label_printf(s_state, "%s", on ? "ON" : "OFF");
    if(on) lv_obj_remove_local_style_prop(s_state, LV_STYLE_TEXT_COLOR, 0);
    else lv_obj_set_style_text_color(s_state, C_DIM, 0);
}

static void create(lv_obj_t * p)
{
    lv_obj_t * hull = lv_line_create(p);
    lv_line_set_points(hull, HULL, sizeof(HULL) / sizeof(HULL[0]));
    lv_obj_set_style_line_color(hull, C_DIM, 0);
    lv_obj_set_style_line_width(hull, 2, 0);
    lv_obj_set_style_line_rounded(hull, true, 0);
    lv_obj_set_pos(hull, HULL_X, 0);

    for(int i = 0; i < DASH_LIGHT_COUNT; i++) {
        const light_pos_t * l = &LIGHTS[i];
        lv_obj_t * o = ui_box(p, HULL_X + l->x, l->y, l->w, l->h);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_add_style(o, ui_style_accent_bg(), 0);
        lv_obj_set_style_shadow_width(o, 24, 0);
        s_light[i] = o;
    }
    s_name = ui_caption(p, "");
    lv_obj_align(s_name, LV_ALIGN_BOTTOM_LEFT, 0, -8);
    s_state = ui_label(p, &lv_font_montserrat_28, C_DIM, "");
    lv_obj_add_style(s_state, ui_style_accent_text(), 0);
    lv_obj_align(s_state, LV_ALIGN_BOTTOM_RIGHT, 0, -4);
}

static void update(const dash_data_t * d)
{
    if(d->lights == s_shown) return;
    s_shown = d->lights;
    show(d->lights);
}

static bool input(ui_input_t in)
{
    switch(in) {
        case UI_IN_UP:     s_focus = (s_focus + DASH_LIGHT_COUNT - 1) % DASH_LIGHT_COUNT; break;
        case UI_IN_DOWN:   s_focus = (s_focus + 1) % DASH_LIGHT_COUNT; break;
        case UI_IN_SELECT: dash_cmd_light_toggle(s_focus); return true;
        default: return false;
    }
    show(s_shown);
    return true;
}

const ui_page_t page_lights = { "LIGHTS", create, update, input };
