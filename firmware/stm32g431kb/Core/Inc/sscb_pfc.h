/**
 * @file sscb_pfc.h
 * @brief Dynamic Power Factor Correction (PFC) TRIAC/Capacitor stage controller.
 * @target STM32G431KB
 */

#ifndef SSCB_PFC_H
#define SSCB_PFC_H

#include "sscb_config.h"
#include "sscb_metering.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             PFC THRESHOLDS                                 */
/* ========================================================================== */
#define PFC_PF_ENGAGE_THRESHOLD     (0.85f)  /* Engage capacitor if PF < 0.85 (lagging) */
#define PFC_PF_DISENGAGE_THRESHOLD  (0.95f)  /* Disengage capacitor if PF > 0.95        */
#define PFC_MIN_LOAD_CURRENT_A      (1.5f)   /* Do not engage for very light loads      */

/* ========================================================================== */
/*                            FUNCTION PROTOTYPES                             */
/* ========================================================================== */

/**
 * @brief Initialize PFC GPIOs and timer trigger outputs for TRIAC gate control.
 */
void SSCB_PFC_Init(void);

/**
 * @brief Evaluate electrical metrics and determine if PFC capacitor bank should
 *        be engaged or disengaged.
 * @param metrics Pointer to current metering telemetry.
 */
void SSCB_PFC_Evaluate(const sscb_metering_metrics_t* metrics);

/**
 * @brief Zero-crossing interrupt handler to fire TRIAC gate pulse synchronously.
 */
void SSCB_PFC_ZeroCrossHandler(void);

/**
 * @brief Check whether PFC capacitor bank is actively connected.
 * @return true if TRIAC is engaged.
 */
bool SSCB_PFC_IsActive(void);

/**
 * @brief Force disengagement of PFC stage (e.g. during faults or breaker open).
 */
void SSCB_PFC_ForceDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* SSCB_PFC_H */
