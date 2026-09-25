/**
 * gauge_speed — centre speed ring: glowing 270° ring, fine ticks, big
 * digital speed in the chosen unit, secondary unit, iBR pill.
 */
#ifndef GAUGE_SPEED_H
#define GAUGE_SPEED_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

#define GAUGE_SPEED_MAX  105   /* km/h: Spark top speed (~80) + 25 */
#define GAUGE_RPM_MAX    8000
#define GAUGE_RPM_RED    7000

void gauge_speed_create(lv_obj_t * parent);

/* Ring only (key-on sweep), in 0.1 km/h */
void gauge_speed_set_arc(int32_t speed_dkmh);

/* Numbers, ring, iBR */
void gauge_speed_update(const dash_data_t * d, bool arc);

/* Re-label ticks / units after a settings change */
void gauge_speed_apply_settings(void);

#endif /* GAUGE_SPEED_H */
