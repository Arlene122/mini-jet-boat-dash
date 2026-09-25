/**
 * sim_platform — simulator-only CAN loopback controls.
 */
#ifndef SIM_PLATFORM_H
#define SIM_PLATFORM_H

#include <stdbool.h>

#include "../src/can/can_frame.h"

/* Fake ECU puts frames on the (simulated) bus */
void sim_can_inject(const can_frame_t * f);

/* Simulate unplugging the ECU (E key) */
void sim_can_set_connected(bool on);
bool sim_can_connected(void);

#endif /* SIM_PLATFORM_H */
