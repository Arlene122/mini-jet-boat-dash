/**
 * fake_ecu — pretend 2015 Sea-Doo Spark ECU for the simulator.
 *
 * Engine data goes out as CAN frames (placeholder protocol) through the
 * simulated bus -> dash_task decoder, like on the boat. Also fakes GPS,
 * audio board and light relays.
 */
#ifndef FAKE_ECU_H
#define FAKE_ECU_H

#include <stdbool.h>

void fake_ecu_init(void);

/* Handle one key press (SDL keycode). Returns true if used. */
bool fake_ecu_key(int sdl_key);

#endif /* FAKE_ECU_H */
