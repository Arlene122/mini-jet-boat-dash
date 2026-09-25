/**
 * page_music — large artwork tile, title/artist, hairline progress and
 * minimal controls. UP/DOWN moves focus, SELECT presses.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../dash_data/dash_cmd.h"

#define ART   132

static lv_obj_t * s_phone, * s_title, * s_artist, * s_bar, * s_pos, * s_len;
static lv_obj_t * s_btn[3], * s_play_ring;
static int s_focus = 1;
static bool s_connected = true;

static const dash_music_cmd_t BTN_CMD[3] = { DASH_MUSIC_PREV, DASH_MUSIC_PLAY_PAUSE, DASH_MUSIC_NEXT };
static const int32_t BTN_X[3] = { 70, PAGE_CONTENT_W / 2, PAGE_CONTENT_W - 70 };
#define BTN_Y 322

static void show_focus(void)
{
    for(int i = 0; i < 3; i++) {
        if(i == s_focus) lv_obj_add_style(s_btn[i], ui_style_accent_text(), 0);
        else lv_obj_remove_style(s_btn[i], ui_style_accent_text(), 0);
    }
    lv_obj_set_style_border_opa(s_play_ring, s_focus == 1 ? LV_OPA_COVER : LV_OPA_40, 0);
}

static void create(lv_obj_t * p)
{
    lv_obj_t * art = ui_box(p, 0, 0, ART, ART);
    lv_obj_set_style_radius(art, 20, 0);
    lv_obj_add_style(art, ui_style_disc(), 0);
    lv_obj_set_style_border_color(art, C_LINE, 0);
    lv_obj_set_style_border_width(art, 1, 0);
    lv_obj_t * note = ui_label(art, &lv_font_montserrat_48, C_TEXT, LV_SYMBOL_AUDIO);
    lv_obj_add_style(note, ui_style_accent_text(), 0);
    lv_obj_center(note);

    int32_t tx = ART + 20, tw = PAGE_CONTENT_W - tx;
    s_phone = ui_caption(p, "");
    lv_obj_set_style_text_font(s_phone, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(s_phone, tx, 20);
    s_title = ui_label(p, &lv_font_montserrat_28, C_TEXT, "");
    lv_label_set_long_mode(s_title, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_width(s_title, tw);
    lv_obj_set_pos(s_title, tx, 46);
    s_artist = ui_label(p, &lv_font_montserrat_20, C_DIM, "");
    lv_label_set_long_mode(s_artist, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_width(s_artist, tw);
    lv_obj_align_to(s_artist, s_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    s_bar = ui_slim_bar(p, PAGE_CONTENT_W, 3, 1000);
    lv_obj_set_pos(s_bar, 0, 176);
    s_pos = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_pos(s_pos, 0, 188);
    s_len = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_align(s_len, LV_ALIGN_TOP_RIGHT, 0, 188);

    s_play_ring = ui_box(p, BTN_X[1] - 40, BTN_Y - 40 - 50, 80, 80);
    lv_obj_set_style_radius(s_play_ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(s_play_ring, ui_style_accent_border(), 0);
    lv_obj_set_style_shadow_width(s_play_ring, 0, 0);

    static const char * sym[3] = { LV_SYMBOL_PREV, LV_SYMBOL_PLAY, LV_SYMBOL_NEXT };
    for(int i = 0; i < 3; i++) {
        s_btn[i] = ui_label(p, &lv_font_montserrat_32, C_TEXT, sym[i]);
        lv_obj_align(s_btn[i], LV_ALIGN_CENTER, BTN_X[i] - PAGE_CONTENT_W / 2, BTN_Y - 50 - PAGE_CONTENT_H / 2);
    }
    show_focus();
}

static void update(const dash_data_t * d)
{
    const dash_music_t * m = &d->music;
    if(!m->connected) {
        ui_label_printf(s_phone, "NOT CONNECTED");
        ui_label_printf(s_title, "No phone");
        ui_label_printf(s_artist, "Pair with the dash");
        ui_label_printf(s_pos, "");
        ui_label_printf(s_len, "");
        if(lv_bar_get_value(s_bar) != 0) lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
    }
    else {
        ui_label_printf(s_phone, "%s", m->phone);
        ui_label_printf(s_title, "%s", m->title);
        ui_label_printf(s_artist, "%s", m->artist);
        ui_label_printf(s_pos, "%d:%02d", m->pos_s / 60, m->pos_s % 60);
        ui_label_printf(s_len, "%d:%02d", m->len_s / 60, m->len_s % 60);
        int v = m->len_s ? m->pos_s * 1000 / m->len_s : 0;
        if(lv_bar_get_value(s_bar) != v) lv_bar_set_value(s_bar, v, LV_ANIM_OFF);
    }
    lv_obj_align_to(s_artist, s_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    ui_label_printf(s_btn[1], "%s", m->playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    if(m->connected != s_connected) {
        s_connected = m->connected;
        for(int i = 0; i < 3; i++) lv_obj_set_style_opa(s_btn[i], m->connected ? LV_OPA_COVER : LV_OPA_30, 0);
    }
}

static bool input(ui_input_t in)
{
    switch(in) {
        case UI_IN_UP:     if(s_focus > 0) s_focus--; show_focus(); return true;
        case UI_IN_DOWN:   if(s_focus < 2) s_focus++; show_focus(); return true;
        case UI_IN_SELECT: dash_cmd_music(BTN_CMD[s_focus]); return true;
        default: return false;
    }
}

const ui_page_t page_music = { "MUSIC", create, update, input };
