/**
 * cluster_brackets — RPM (left) and FUEL (right) bracket gauges with their
 * numbers, flanking the speed ring.
 */
#ifndef CLUSTER_BRACKETS_H
#define CLUSTER_BRACKETS_H

#include "lvgl.h"
#include "../dash_data/dash_data.h"

void cluster_brackets_create(lv_obj_t * parent);
void cluster_brackets_update(const dash_data_t * d, bool bars);
void cluster_brackets_sweep(int32_t permille);   /* key-on sweep */

#endif /* CLUSTER_BRACKETS_H */
