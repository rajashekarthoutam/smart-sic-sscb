/**
 * @file sscb_protection.h
 * @brief Ultra-fast sub-microsecond protection engine, Analog Watchdog (AWD)
 *        driver, hardware break integration, and fault state management.
 * @target STM32G431KB
 */

#ifndef SSCB_PROTECTION_H
#define SSCB_PROTECTION_H

#include "sscb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             TYPE DEFINITIONS                               */
/* ========================================================================== */

/**
 * @brief Breaker operational states.
 */
typedef enum {
    SSCB_STATE_INITIALIZING   = 0x00,
    SSCB_STATE_STANDBY_OPEN   = 0x01,
    SSCB_STATE_CLOSED_ACTIVE  = 0x02,
    SSCB_STATE_TRIPPED_FAULT  = 0x03,
    SSCB_STATE_LOCKED_OUT     = 0x04
} sscb_state_t;

/**
 * @brief Specific fault triggers identified by the hardware & software engine.
 */
typedef enum {
    FAULT_NONE                        = 0x00,
    FAULT_PHASE_INSTANTANEOUS_OC      = 0x01, /* AWD1: Phase fast peak overcurrent */
    FAULT_NEUTRAL_INSTANTANEOUS_OC    = 0x02, /* AWD2: Neutral fast peak overcurrent*/
    FAULT_EARTH_LEAKAGE               = 0x03, /* Residual current |Ip - In| > 30mA  */
    FAULT_RMS_OVERCURRENT             = 0x04, /* I2t thermal curve exceeded         */
    FAULT_OVERVOLTAGE                 = 0x05, /* Vrms > 265V                       */
    FAULT_UNDERVOLTAGE                = 0x06, /* Vrms < 180V                       */
    FAULT_MANUAL_TRIP                 = 0x07, /* Local pushbutton / remote command  */
    FAULT_HARDWARE_BREAK_INPUT        = 0x08  /* External TIM1_BKIN tripped        */
} sscb_fault_t;

/**
 * @brief Breaker diagnostic telemetry and protection snapshot.
 */
typedef struct {
    sscb_state_t current_state;
    sscb_fault_t last_fault;
    uint32_t     trip_count;
    uint32_t     trip_timestamp_us;
    float        trip_current_peak_a;
    float        v_rms;
    float        i_phase_rms;
    float        i_neutral_rms;
    float        i_residual_ma;
    float        active_power_w;
    float        apparent_power_va;
    float        power_factor;
    bool         pfc_active;
} sscb_telemetry_t;

/* ========================================================================== */
/*                            FUNCTION PROTOTYPES                             */
/* ========================================================================== */

/**
 * @brief Initialize low-level registers: ADC Analog Watchdog 1 & 2, TIM1 Break
 *        circuitry, and GPIO gate drive outputs.
 */
void SSCB_Protection_Init(void);

/**
 * @brief Closes the solid-state circuit breaker (energizes Phase & Neutral gates).
 * @return true if successfully closed, false if system is in fault/lockout.
 */
bool SSCB_CloseBreaker(void);

/**
 * @brief Immediately trips the breaker into safe disconnected state.
 * @param fault The fault code triggering the trip.
 */
void SSCB_TripBreaker(sscb_fault_t fault);

/**
 * @brief Clears latched faults and returns breaker to STANDBY_OPEN state.
 * @return true if fault successfully cleared.
 */
bool SSCB_ResetFault(void);

/**
 * @brief Periodic thermal I2t, voltage window, and residual current supervisor.
 *        Called every AC mains cycle (20 ms).
 * @param v_rms True-RMS grid voltage.
 * @param i_phase_rms Current RMS on phase line.
 * @param i_neutral_rms Current RMS on neutral line.
 * @param dt_seconds Time interval (e.g. 0.02s).
 */
void SSCB_Protection_ProcessCycle(float v_rms, float i_phase_rms, float i_neutral_rms, float dt_seconds);

/**
 * @brief Updates system microsecond timestamp for trip logging.
 * @param tick_us Current microsecond timestamp (e.g. from DWT->CYCCNT or TIM5).
 */
void SSCB_UpdateTick(uint32_t tick_us);

/**
 * @brief Analog Watchdog hardware ISR callback executed upon sub-microsecond
 *        overcurrent event.
 */
void SSCB_AWD_IRQHandler(void);

/**
 * @brief Retrieve current telemetry snapshot.
 */
const sscb_telemetry_t* SSCB_GetTelemetry(void);

#ifdef __cplusplus
}
#endif

#endif /* SSCB_PROTECTION_H */
