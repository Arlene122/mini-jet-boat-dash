/**
 * pages — the extra (non-engine) screens shown in the page card.
 * Each page builds itself once, then updates only changed values.
 * Content area is about 333 x 360 px.
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

extern const ui_page_t page_marine;
extern const ui_page_t page_music;
extern const ui_page_t page_lights;
extern const ui_page_t page_trip;
extern const ui_page_t page_system;
extern const ui_page_t page_settings;

#define PAGE_CONTENT_W 333   /* PAGE_W - 2 x padding */
#define PAGE_COL2      176   /* x of the second column */

/* Shared: small caption + value, returns the value label */
lv_obj_t * page_row(lv_obj_t * parent, int32_t x, int32_t y, const char * caption,
                    const lv_font_t * font);

#endif /* PAGES_H */
