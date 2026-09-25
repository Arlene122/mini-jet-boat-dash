/**
 * sim_overlay — simulator-only layers: bezel stencil and key help.
 * Never built for the boat (the real bezel covers the screen edges).
 */
#ifndef SIM_OVERLAY_H
#define SIM_OVERLAY_H

#include <stdbool.h>

void sim_overlay_init(void);

/* Handle one key press (SDL keycode). Returns true if used. */
bool sim_overlay_key(int sdl_key);

#endif /* SIM_OVERLAY_H */
