/**
 * page_music — track info from the audio board + prev / play / next.
 * UP/DOWN moves focus between the three buttons, SELECT presses it.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../dash_data/dash_cmd.h"

static lv_obj_t * s_phone, * s_title, * s_artist, * s_bar, * s_time;
static lv_obj_t * s_btn[3];
static int s_focus = 1;
static bool s_connected = true;

static const dash_music_cmd_t BTN_CMD[3] = { DASH_MUSIC_PREV, DASH_MUSIC_PLAY_PAUSE, DASH_MUSIC_NEXT };

static void show_focus(void)
{
    for(int i = 0; i < 3; i++) {
        lv_obj_set_style_border_opa(s_btn[i], i == s_focus ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_opa(s_btn[i], i == s_focus ? LV_OPA_40 : LV_OPA_TRANSP, 0);
    }
}

static void create(lv_obj_t * p)
{
    s_phone = ui_caption(p, "");
    lv_obj_set_pos(s_phone, 0, 0);
    s_title = ui_label(p, &lv_font_montserrat_32, C_TEXT, "");
    lv_label_set_long_mode(s_title, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_width(s_title, 420);
    lv_obj_set_pos(s_title, 0, 36);
    s_artist = ui_label(p, &lv_font_montserrat_24, C_DIM, "");
    lv_obj_set_pos(s_artist, 0, 80);
    s_bar = ui_slim_bar(p, 420, 6, 1000);
    lv_obj_set_pos(s_bar, 0, 130);
    s_time = ui_label(p, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_pos(s_time, 0, 142);

    static const char * sym[3] = { LV_SYMBOL_PREV, LV_SYMBOL_PLAY, LV_SYMBOL_NEXT };
    for(int i = 0; i < 3; i++) {
        lv_obj_t * b = ui_box(p, 40 + i * 130, 190, 96, 96);
        lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(b, C_LINE, 0);
        lv_obj_set_style_bg_opa(b, LV_OPA_60, 0);
        lv_obj_add_style(b, ui_style_accent_border(), 0);
        lv_obj_t * l = ui_label(b, &lv_font_montserrat_40, C_TEXT, sym[i]);
        lv_obj_center(l);
        s_btn[i] = b;
    }
    show_focus();
}

static void update(const dash_data_t * d)
{
    const dash_music_t * m = &d->music;
    if(!m->connected) {
        ui_label_printf(s_phone, "NOT CONNECTED");
        ui_label_printf(s_title, "No phone");
        ui_label_printf(s_artist, "Pair your phone with the dash");
        ui_label_printf(s_time, "");
        if(lv_bar_get_value(s_bar) != 0) lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
    }
    else {
        ui_label_printf(s_phone, "%s", m->phone);
        ui_label_printf(s_title, "%s", m->title);
        ui_label_printf(s_artist, "%s", m->artist);
        ui_label_printf(s_time, "%d:%02d / %d:%02d", m->pos_s / 60, m->pos_s % 60, m->len_s / 60, m->len_s % 60);
        int v = m->len_s ? m->pos_s * 1000 / m->len_s : 0;
        if(lv_bar_get_value(s_bar) != v) lv_bar_set_value(s_bar, v, LV_ANIM_OFF);
    }
    ui_label_printf(lv_obj_get_child(s_btn[1], 0), "%s", m->playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    if(m->connected != s_connected) {
        s_connected = m->connected;
        for(int i = 0; i < 3; i++) lv_obj_set_style_opa(s_btn[i], m->connected ? LV_OPA_COVER : LV_OPA_40, 0);
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
