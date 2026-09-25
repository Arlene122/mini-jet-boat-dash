/**
 * page_host — the page zone: no box, content floats in its own zone,
 * set off by a fading vertical hairline (mirrors the engine side) and a
 * fading header rule. All pages are built once and hidden; only the
 * visible one is updated. A short fade on change.
 */
#include "page_host.h"

#include "pages/pages.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "ui_util.h"

/* ---------- Config ---------- */

static const ui_page_t * const PAGES[] = {
    &page_marine, &page_music, &page_lights, &page_trip, &page_system, &page_settings,
};
#define PAGE_COUNT ((int)(sizeof(PAGES) / sizeof(PAGES[0])))
#define PAD_L      PAGE_PAD
#define HEAD_H     58

/* ---------- State ---------- */

static lv_obj_t * s_zone, * s_title;
static lv_obj_t * s_content[PAGE_COUNT];
static lv_obj_t * s_dot[PAGE_COUNT];
static int s_cur;
static const dash_data_t * s_last;

/* ---------- Helpers ---------- */

static void show_page(int idx)
{
    lv_obj_set_hidden(s_content[s_cur], true);
    lv_obj_set_style_bg_color(s_dot[s_cur], C_OFF, 0);
    lv_obj_set_width(s_dot[s_cur], 6);
    s_cur = idx;
    lv_obj_set_hidden(s_content[s_cur], false);
    lv_obj_remove_local_style_prop(s_dot[s_cur], LV_STYLE_BG_COLOR, 0);
    lv_obj_set_width(s_dot[s_cur], 20);
    lv_label_set_text(s_title, PAGES[s_cur]->title);
    lv_obj_fade_in(s_content[s_cur], 180, 0);
    if(s_last) PAGES[s_cur]->update(s_last);
}

/* ---------- API ---------- */

void page_host_create(lv_obj_t * parent)
{
    s_zone = ui_box(parent, PAGE_X, SIDE_Y1, PAGE_W, SIDE_H);
    ui_fade_line(s_zone, 0, 0, SIDE_H, true, LV_OPA_60);

    s_title = ui_label(s_zone, &lv_font_montserrat_20, C_TEXT, "");
    lv_obj_add_style(s_title, ui_style_accent_text(), 0);
    lv_obj_set_style_text_letter_space(s_title, 5, 0);
    lv_obj_set_pos(s_title, PAD_L, 14);

    lv_obj_t * dots = ui_box(s_zone, 0, 0, LV_SIZE_CONTENT, 6);
    lv_obj_set_flex_flow(dots, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(dots, 6, 0);
    lv_obj_align(dots, LV_ALIGN_TOP_RIGHT, 0, 22);

    ui_fade_line(s_zone, PAD_L, HEAD_H - 14, PAGE_CONTENT_W, false, LV_OPA_50);

    for(int i = 0; i < PAGE_COUNT; i++) {
        s_dot[i] = ui_box(dots, 0, 0, 6, 6);
        lv_obj_set_style_radius(s_dot[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_add_style(s_dot[i], ui_style_accent_bg(), 0);
        lv_obj_set_style_bg_color(s_dot[i], C_OFF, 0);

        s_content[i] = ui_box(s_zone, PAD_L, HEAD_H + 8, PAGE_CONTENT_W, SIDE_H - HEAD_H - 8);
        PAGES[i]->create(s_content[i]);
        lv_obj_set_hidden(s_content[i], true);
    }
    s_cur = 0;
    show_page(0);
}

void page_host_update(const dash_data_t * d)
{
    s_last = d;
    PAGES[s_cur]->update(d);
}

void page_host_step(int dir)
{
    show_page((s_cur + dir + PAGE_COUNT) % PAGE_COUNT);
}

void page_host_home(void)
{
    if(s_cur != 0) show_page(0);
}

bool page_host_input(ui_input_t in)
{
    return PAGES[s_cur]->input ? PAGES[s_cur]->input(in) : false;
}
