/**
 * dash_task — the dash's data loop: drain CAN frames, decode, watch for
 * stale ECU data, derive dash-side warnings and trip stats, commit.
 * Simulator: called from an LVGL timer. P4: its own FreeRTOS task.
 */
#ifndef DASH_TASK_H
#define DASH_TASK_H

#include <stdint.h>

void dash_task_init(void);
void dash_task_step(uint32_t now_ms);

#endif /* DASH_TASK_H */
