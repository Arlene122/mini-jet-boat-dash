/**
 * page_host — the right-hand card that shows one page at a time,
 * with title + position dots. Knob turns change page.
 */
#ifndef PAGE_HOST_H
#define PAGE_HOST_H

#include "lvgl.h"
#include "ui_input.h"
#include "../dash_data/dash_data.h"

void page_host_create(lv_obj_t * parent);
void page_host_update(const dash_data_t * d);
void page_host_step(int dir);       /* +1 next, -1 previous */
void page_host_back(void);           /* close settings, else first page */
void page_host_toggle_settings(void); /* knob hold / Settings button */
bool page_host_input(ui_input_t in);

#endif /* PAGE_HOST_H */
