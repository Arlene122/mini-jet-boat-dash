/**
 * screen_main — main riding screen: bars, engine panel, gauge, page card.
 */
#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

lv_obj_t * screen_main_create(void);

/* Push new values; only widgets whose shown value changed are redrawn. */
void screen_main_update(const dash_data_t * d);

/* Key-on sweep: arcs follow the sweep instead of live data while true. */
void screen_main_set_sweep(bool on);

#endif /* SCREEN_MAIN_H */
