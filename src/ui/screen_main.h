/**
 * screen_main — main riding screen (Phase 1 placeholder layout).
 */
#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

lv_obj_t * screen_main_create(void);

/* Push new values; only widgets whose shown value changed are redrawn. */
void screen_main_update(const dash_data_t * d);

#endif /* SCREEN_MAIN_H */
