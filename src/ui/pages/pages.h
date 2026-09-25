/**
 * pages — the extra (non-engine) screens shown in the page zone.
 * Swipe order: NAV, MUSIC, LIGHTS, RIDE. SETTINGS opens in the same zone.
 * Each page builds itself once, then updates only changed values.
 * Style: one hero value per page, hairlines instead of boxes.
 */
#ifndef PAGES_H
#define PAGES_H

#include <stdbool.h>

#include "lvgl.h"
#include "../ui_input.h"
#include "../../dash_data/dash_data.h"

typedef struct {
    const char * title;
    void (*create)(lv_obj_t * parent);        /* parent = page content area */
    void (*update)(const dash_data_t * d);    /* only while visible */
    bool (*input)(ui_input_t in);             /* UP / DOWN / SELECT; NULL = none */
} ui_page_t;

/* Swipe pages (knob turn) */
extern const ui_page_t page_nav;
extern const ui_page_t page_music;
extern const ui_page_t page_lights;
extern const ui_page_t page_ride;

/* Not in the swipe order: opened by knob hold / Settings button */
extern const ui_page_t page_settings;

#define PAGE_PAD        30    /* gap from the zone's hairline */
#define PAGE_CONTENT_W  345
#define PAGE_CONTENT_H  390
#define LIST_ROW_H      42

/* Small caption above a value; returns the value label */
lv_obj_t * page_stat(lv_obj_t * parent, int32_t x, int32_t y, const char * caption,
                     const lv_font_t * font);

/* List row: caption left, value right, fading hairline under it.
 * Returns the value label; *row (optional) gets the row object. */
lv_obj_t * page_list_row(lv_obj_t * parent, int32_t y, const char * caption, lv_obj_t ** row);

#endif /* PAGES_H */
