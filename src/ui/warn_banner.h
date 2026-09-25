/**
 * warn_banner — full-card alert for a NEW warning: icon, plain words and
 * what to do. Knob push acknowledges; the bottom-bar icon stays lit
 * until the fault clears (warnings always visible).
 */
#ifndef WARN_BANNER_H
#define WARN_BANNER_H

#include <stdbool.h>

#include "lvgl.h"
#include "../dash_data/dash_data.h"

void warn_banner_create(lv_obj_t * parent);  /* covers the page card zone */
void warn_banner_update(const dash_data_t * d);
bool warn_banner_visible(void);
void warn_banner_ack(void);

#endif /* WARN_BANNER_H */
