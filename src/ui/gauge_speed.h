/**
 * gauge_speed — centre gauge: big digital speed, slim speed arc with ticks,
 * RPM arc inside it, knots, and iBR (R N F / BRAKE) in the bottom gap.
 */
#ifndef GAUGE_SPEED_H
#define GAUGE_SPEED_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

#define GAUGE_SPEED_MAX  105   /* km/h: Spark top speed (~80) + 25 */
#define GAUGE_RPM_MAX    8000
#define GAUGE_RPM_RED    7000

void gauge_speed_create(lv_obj_t * parent);

/* Arcs only (used by the key-on sweep) */
void gauge_speed_set_arcs(int32_t speed_kmh, int32_t rpm);

/* Full update from data (numbers, arcs, iBR) */
void gauge_speed_update(const dash_data_t * d, bool arcs);

#endif /* GAUGE_SPEED_H */
