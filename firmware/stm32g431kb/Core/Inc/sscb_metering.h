/**
 * @file sscb_metering.h
 * @brief High-precision AC electrical metering engine for True RMS Voltage,
 *        Current, Active/Reactive/Apparent Power, and Power Factor (cos phi).
 * @target STM32G431KB
 */

#ifndef SSCB_METERING_H
#define SSCB_METERING_H

#include "sscb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             TYPE DEFINITIONS                               */
/* ========================================================================== */

/**
 * @brief Raw ADC channel triple representing an instantaneous sample.
 */
typedef struct {
    uint16_t raw_v_grid;   /* AMC1301 differential voltage output */
    uint16_t raw_i_phase;  /* ACS758 Phase current output         */
    uint16_t raw_i_neut;   /* ACS758 Neutral current output       */
} adc_sample_t;

/**
 * @brief Computed AC grid electrical metrics.
 */
typedef struct {
    float v_rms;            /* True RMS Line-to-Neutral Voltage (V)  */
    float i_phase_rms;      /* True RMS Phase Current (A)           */
    float i_neutral_rms;    /* True RMS Neutral Current (A)         */
    float active_power_w;   /* Real Active Power (Watts)            */
    float apparent_power_va;/* Apparent Power (Volt-Amperes)        */
    float reactive_power_var;/* Reactive Power (VAR)                */
    float power_factor;     /* Displacement Power Factor cos(phi)   */
    float grid_frequency_hz;/* Measured AC Grid Frequency (Hz)      */
    bool  current_lagging;  /* true = inductive, false = capacitive */
} sscb_metering_metrics_t;

/* ========================================================================== */
/*                            FUNCTION PROTOTYPES                             */
/* ========================================================================== */

/**
 * @brief Initialize metering subsystem, calibrate sensor DC offsets, and
 *        start DMA conversion.
 */
void SSCB_Metering_Init(void);

/**
 * @brief Process a completed buffer of AC samples (1 full mains cycle = 128 samples).
 * @param samples Array of 128 adc_sample_t objects.
 * @param count Number of samples (should equal SAMPLES_PER_CYCLE).
 * @param out_metrics Pointer to structure receiving computed electrical metrics.
 */
void SSCB_Metering_ProcessCycle(const adc_sample_t* samples, uint16_t count,
                               sscb_metering_metrics_t* out_metrics);

/**
 * @brief Retrieve the latest computed metering metrics.
 */
const sscb_metering_metrics_t* SSCB_Metering_GetLatest(void);

#ifdef __cplusplus
}
#endif

#endif /* SSCB_METERING_H */
