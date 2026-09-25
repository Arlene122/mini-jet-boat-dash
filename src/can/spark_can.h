/**
 * spark_can — Sea-Doo Spark ECU messages <-> dash_data.
 *
 * ==== PLACEHOLDER PROTOCOL — NOT THE REAL BRP MESSAGES ====
 * The IDs, byte positions and scaling below are INVENTED so the whole
 * pipeline (frame -> decoder -> dash_data -> UI) runs in the simulator now.
 * Everything here is UNVERIFIED. In Phase 4/5 replace this table with
 * messages logged from the real ECU + stock gauge, one at a time.
 */
#ifndef SPARK_CAN_H
#define SPARK_CAN_H

#include "can_frame.h"
#include "../dash_data/dash_data.h"

/* UNVERIFIED placeholder IDs */
enum {
    SPARK_ID_ENGINE = 0x100,   /* rpm u16 LE | temp+40 u8 | battery*10 u8 */
    SPARK_ID_STATUS = 0x101,   /* ibr | mode | fault bits | dess | code u16 LE */
    SPARK_ID_FUEL   = 0x102,   /* level*2 u8 | rate*10 u16 LE (L/h) */
    SPARK_ID_HOURS  = 0x103,   /* engine hours*100 u32 LE */
};

/* ECU-reported fault bits (dash_data warnings subset) */
#define SPARK_FAULT_MASK (DASH_WARN_OIL_PRESSURE | DASH_WARN_OVERHEAT | DASH_WARN_CHECK_ENGINE)

/* Decode one frame into d. Returns true if it was a known engine frame. */
bool spark_can_decode(const can_frame_t * f, dash_data_t * d);

/* Encoders — used by the simulator's fake ECU */
void spark_can_encode_engine(can_frame_t * f, uint16_t rpm, float temp_c, float batt_v);
void spark_can_encode_status(can_frame_t * f, dash_ibr_t ibr, dash_mode_t mode, uint32_t faults,
                             bool dess, uint16_t code);
void spark_can_encode_fuel(can_frame_t * f, float level_pct, float rate_lph);
void spark_can_encode_hours(can_frame_t * f, float hours);

#endif /* SPARK_CAN_H */
