/**
 * can_frame / can_bus — classic CAN frames and the receive interface.
 *
 * SAFETY: there is deliberately NO transmit function. Nothing may be sent
 * on the real bus until the message is decoded and verified in the
 * simulator / replay (Claude_Rules.md).
 */
#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
} can_frame_t;

/* Non-blocking: true + frame if one is waiting.
 * Implemented by sim/sim_can_bus.c (PC) and the P4 TWAI driver later. */
bool can_bus_receive(can_frame_t * f);

#endif /* CAN_FRAME_H */
