/**
 * @file sscb_metering.c
 * @brief High-precision AC electrical metering engine implementation.
 * @target STM32G431KB
 */

#if defined(__has_include)
  #if __has_include("sscb_metering.h")
    #include "sscb_metering.h"
    #include "sscb_protection.h"
  #elif __has_include("../Inc/sscb_metering.h")
    #include "../Inc/sscb_metering.h"
    #include "../Inc/sscb_protection.h"
  #else
    #include "sscb_metering.h"
    #include "sscb_protection.h"
  #endif
#else
  #include "sscb_metering.h"
  #include "sscb_protection.h"
#endif
#include <math.h>
#include <string.h>

/* ========================================================================== */
/*                             STATIC VARIABLES                               */
/* ========================================================================== */

static sscb_metering_metrics_t g_latest_metrics = {0};

/* Measured DC zero-current / zero-voltage offsets (ADC counts) */
static uint16_t g_offset_v     = 2048; /* Midscale 1.65V (12-bit) */
static uint16_t g_offset_i_ph  = 2048; /* ACS758 zero-current ref  */
static uint16_t g_offset_i_nt  = 2048; /* ACS758 zero-current ref  */

/* Conversion scaling constants */
static float g_v_scale  = 0.0f;
static float g_i_scale  = 0.0f;

/* ========================================================================== */
/*                        METERING INITIALIZATION                             */
/* ========================================================================== */

void SSCB_Metering_Init(void)
{
    /* 
     * Compute scale factors based on ADC reference and sensor gains:
     * Voltage:
     * ADC_mV = ADC_count * (3300.0 / 4096.0)
     * V_diff_sensor_input = (ADC_mV) / AMC1301_DIFFERENTIAL_GAIN
     * V_grid_actual = V_diff_sensor_input * AMC1301_DIVIDER_RATIO / 1000.0
     */
    g_v_scale = (ADC_VREF_MV / ADC_RESOLUTION) * (AMC1301_DIVIDER_RATIO / AMC1301_DIFFERENTIAL_GAIN) / 1000.0f;

    /*
     * Current:
     * ACS758: 40 mV / Ampere
     * I_actual = (ADC_count * (3300.0 / 4096.0)) / 40.0
     */
    g_i_scale = (ADC_VREF_MV / ADC_RESOLUTION) / ACS758_SENSITIVITY_MV_PER_A;

    memset(&g_latest_metrics, 0, sizeof(g_latest_metrics));
    g_latest_metrics.power_factor = 1.0f;
    g_latest_metrics.grid_frequency_hz = AC_NOMINAL_FREQ_HZ;
}

/* ========================================================================== */
/*                         CYCLE PROCESSING ROUTINE                           */
/* ========================================================================== */

void SSCB_Metering_ProcessCycle(const adc_sample_t* samples, uint16_t count,
                               sscb_metering_metrics_t* out_metrics)
{
    if (samples == NULL || count == 0 || out_metrics == NULL) {
        return;
    }

    double sum_v_sq   = 0.0;
    double sum_ip_sq  = 0.0;
    double sum_in_sq  = 0.0;
    double sum_power  = 0.0;

    int16_t v_prev = 0;
    int16_t i_prev = 0;
    int16_t v_zero_cross_idx = -1;
    int16_t i_zero_cross_idx = -1;

    for (uint16_t k = 0; k < count; ++k) {
        /* Remove sensor quiescent DC bias */
        int32_t raw_v_diff  = (int32_t)samples[k].raw_v_grid - (int32_t)g_offset_v;
        int32_t raw_ip_diff = (int32_t)samples[k].raw_i_phase - (int32_t)g_offset_i_ph;
        int32_t raw_in_diff = (int32_t)samples[k].raw_i_neut - (int32_t)g_offset_i_nt;

        /* Convert to SI physical units (Volts, Amperes) */
        float v_inst  = (float)raw_v_diff * g_v_scale;
        float ip_inst = (float)raw_ip_diff * g_i_scale;
        float in_inst = (float)raw_in_diff * g_i_scale;

        /* Integrate squares for True-RMS */
        sum_v_sq  += (double)(v_inst * v_inst);
        sum_ip_sq += (double)(ip_inst * ip_inst);
        sum_in_sq += (double)(in_inst * in_inst);

        /* Integrate instantaneous power p(t) = v(t) * i(t) */
        sum_power += (double)(v_inst * ip_inst);

        /* Detect rising zero-crossings for phase angle & frequency verification */
        if (v_prev < 0 && raw_v_diff >= 0 && v_zero_cross_idx < 0) {
            v_zero_cross_idx = (int16_t)k;
        }
        if (i_prev < 0 && raw_ip_diff >= 0 && i_zero_cross_idx < 0) {
            i_zero_cross_idx = (int16_t)k;
        }
        v_prev = (int16_t)raw_v_diff;
        i_prev = (int16_t)raw_ip_diff;
    }

    /* Compute True RMS values over one full grid period */
    float v_rms  = (float)sqrt(sum_v_sq / (double)count);
    float ip_rms = (float)sqrt(sum_ip_sq / (double)count);
    float in_rms = (float)sqrt(sum_in_sq / (double)count);

    /* Real Active Power P (W) */
    float active_p = (float)(sum_power / (double)count);

    /* Apparent Power S (VA) = V_RMS * I_RMS */
    float apparent_s = v_rms * ip_rms;

    /* Displacement Power Factor cos(phi) = |P| / S */
    float pf = 1.0f;
    float reactive_q = 0.0f;

    if (apparent_s > 0.5f) {
        pf = fabsf(active_p) / apparent_s;
        if (pf > 1.0f) {
            pf = 1.0f;
        }

        /* Reactive Power Q = sqrt(S^2 - P^2) */
        float s2_minus_p2 = (apparent_s * apparent_s) - (active_p * active_p);
        if (s2_minus_p2 > 0.0f) {
            reactive_q = sqrtf(s2_minus_p2);
        }
    }

    /* Determine lagging (inductive) vs leading (capacitive) current */
    bool is_lagging = true;
    if (v_zero_cross_idx >= 0 && i_zero_cross_idx >= 0) {
        if (i_zero_cross_idx > v_zero_cross_idx) {
            is_lagging = true;  /* Current zero-cross occurs after voltage: Lagging */
        } else {
            is_lagging = false; /* Current leads voltage: Capacitive */
        }
    }

    /* Update metrics structure */
    out_metrics->v_rms             = v_rms;
    out_metrics->i_phase_rms       = ip_rms;
    out_metrics->i_neutral_rms     = in_rms;
    out_metrics->active_power_w    = active_p;
    out_metrics->apparent_power_va = apparent_s;
    out_metrics->reactive_power_var= reactive_q;
    out_metrics->power_factor      = pf;
    out_metrics->grid_frequency_hz = AC_NOMINAL_FREQ_HZ;
    out_metrics->current_lagging   = is_lagging;

    /* Cache to static variable */
    g_latest_metrics = *out_metrics;

    /* Feed RMS figures into the thermal, voltage window & residual protection supervisor */
    SSCB_Protection_ProcessCycle(v_rms, ip_rms, in_rms, (float)count / (float)SAMPLING_FREQ_HZ);
}

const sscb_metering_metrics_t* SSCB_Metering_GetLatest(void)
{
    return &g_latest_metrics;
}
