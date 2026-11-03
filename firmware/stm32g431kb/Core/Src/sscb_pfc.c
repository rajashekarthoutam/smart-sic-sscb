/**
 * @file sscb_pfc.c
 * @brief Dynamic Power Factor Correction TRIAC stage implementation.
 * @target STM32G431KB
 */

#if defined(__has_include)
  #if __has_include("sscb_pfc.h")
    #include "sscb_pfc.h"
  #elif __has_include("../Inc/sscb_pfc.h")
    #include "../Inc/sscb_pfc.h"
  #else
    #include "sscb_pfc.h"
  #endif
#else
  #include "sscb_pfc.h"
#endif

static bool g_pfc_target_state = false;
static bool g_pfc_hardware_active = false;
static uint16_t g_triac_gate_pulse_ticks = 0;

void SSCB_PFC_Init(void)
{
    g_pfc_target_state = false;
    g_pfc_hardware_active = false;
    g_triac_gate_pulse_ticks = 0;

    /* Configure PB4 as Output push-pull; ensure gate starts LOW */
    GPIOB->BSRR = ((uint32_t)PFC_TRIAC_PIN) << 16U;
}

void SSCB_PFC_Evaluate(const sscb_metering_metrics_t* metrics)
{
    if (metrics == NULL) {
        return;
    }

    /* Light load interlock: avoid overcorrecting or resonance */
    if (metrics->i_phase_rms < PFC_MIN_LOAD_CURRENT_A) {
        SSCB_PFC_ForceDisable();
        return;
    }

    /* Hysteresis control loop */
    if (metrics->current_lagging && (metrics->power_factor < PFC_PF_ENGAGE_THRESHOLD)) {
        /* Inductive load with low power factor: request capacitor engagement */
        g_pfc_target_state = true;
    } else if ((metrics->power_factor > PFC_PF_DISENGAGE_THRESHOLD) || (!metrics->current_lagging)) {
        /* Unity/high PF reached, or load turned capacitive: request disengagement */
        g_pfc_target_state = false;
    }
}

void SSCB_PFC_ZeroCrossHandler(void)
{
    /* 
     * Synchronous switching:
     * To prevent massive inrush currents into the PFC capacitor, the TRIAC is fired
     * precisely at AC voltage zero-crossing (or at capacitor pre-charged matching voltage).
     */
    if (g_pfc_target_state) {
        /* Fire TRIAC optocoupler pulse (PB4 High) */
        g_pfc_hardware_active = true;
        g_triac_gate_pulse_ticks = 20; /* 20 timer ticks (~200 microseconds pulse) */
        GPIOB->BSRR = (uint32_t)PFC_TRIAC_PIN;
    } else {
        /* Stop firing gate pulses; TRIAC naturally commutates OFF at current zero-crossing */
        g_pfc_hardware_active = false;
        GPIOB->BSRR = ((uint32_t)PFC_TRIAC_PIN) << 16U;
    }
}

bool SSCB_PFC_IsActive(void)
{
    return g_pfc_hardware_active;
}

void SSCB_PFC_ForceDisable(void)
{
    g_pfc_target_state = false;
    g_pfc_hardware_active = false;
    g_triac_gate_pulse_ticks = 0;
    /* Force PB4 LOW */
    GPIOB->BSRR = ((uint32_t)PFC_TRIAC_PIN) << 16U;
}
