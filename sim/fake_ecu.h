/**
 * fake_ecu — pretend 2015 Sea-Doo Spark ECU for the simulator.
 *
 * Phase 1: writes values straight into dash_data (source = SIM).
 * Phase 2: will emit CAN frames that go through the same decoder as the boat.
 */
#ifndef FAKE_ECU_H
#define FAKE_ECU_H

#include <stdbool.h>

void fake_ecu_init(void);

/* Handle one key press (SDL keycode). Returns true if used. */
bool fake_ecu_key(int sdl_key);

#endif /* FAKE_ECU_H */
