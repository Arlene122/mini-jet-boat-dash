/**
 * panel_engine — left zone: RPM, fuel, engine temp, battery.
 */
#ifndef PANEL_ENGINE_H
#define PANEL_ENGINE_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

void panel_engine_create(lv_obj_t * parent);
void panel_engine_update(const dash_data_t * d);

#endif /* PANEL_ENGINE_H */
