/**
 * @file sscb_config.h
 * @brief System configuration, pin mapping, thresholds, and calibration
 *        constants for the Smart Bidirectional Double-Pole SiC SSCB.
 * @target STM32G431KB (Arm Cortex-M4F @ 170MHz)
 */

#ifndef SSCB_CONFIG_H
#define SSCB_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             AC MAINS RATINGS                               */
/* ========================================================================== */
#define AC_NOMINAL_VOLTAGE_RMS      (230.0f)   /* Volts RMS (50Hz standard)    */
#define AC_NOMINAL_FREQ_HZ          (50.0f)    /* Grid frequency in Hz         */
#define AC_NOMINAL_CURRENT_RMS      (16.0f)    /* Rated continuous current (A) */
#define AC_MAX_CONTINUOUS_CURRENT   (20.0f)    /* Upper boundary for thermal   */

/* ========================================================================== */
/*                        PROTECTION THRESHOLDS                               */
/* ========================================================================== */
/* Sub-microsecond Instantaneous Overcurrent (AWD Hardware Thresholds) */
#define TRIP_INSTANTANEOUS_PEAK_A   (40.0f)    /* Hard trip limit in Amperes   */
#define TRIP_NEUTRAL_PEAK_A         (40.0f)    /* Neutral pole overcurrent (A) */

/* Earth Leakage / Ground Fault Residual Current: |I_phase - I_neutral| */
#define TRIP_EARTH_LEAKAGE_MA       (30.0f)    /* 30mA residential RCD standard*/

/* AC Line Voltage Protection Thresholds */
#define TRIP_OVERVOLTAGE_RMS        (265.0f)   /* +15% over nominal 230V       */
#define TRIP_UNDERVOLTAGE_RMS       (180.0f)   /* Brownout cutoff threshold    */

/* Thermal I2t Curve Limit (A^2 * s) */
#define I2T_TRIP_LIMIT              (200.0f)   /* Inverse-time energy threshold*/

/* ========================================================================== */
/*                   ADC & SENSOR CALIBRATION PARAMETERS                      */
/* ========================================================================== */
#define ADC_VREF_MV                 (3300.0f)  /* STM32 ADC reference in mV    */
#define ADC_RESOLUTION              (4096.0f)  /* 12-bit ADC full scale        */
#define ADC_COUNTS_PER_MV           (ADC_RESOLUTION / ADC_VREF_MV)

/* ACS758LCB-050B Bidirectional Current Sensor Configuration */
/* Vout = Vcc/2 + (Ip * Sensitivity). With 3.3V bias divider: 1.65V zero-pt  */
#define ACS758_ZERO_CURRENT_MV      (1650.0f)  /* Quiescent point (0 Amp)      */
#define ACS758_SENSITIVITY_MV_PER_A (40.0f)    /* 40 mV per Ampere             */

/* AMC1301 Isolated Differential Voltage Sensor Configuration */
/* Fixed Gain = 8.2, Linear Differential Input Range = +/- 250 mV              */
/* Resistor divider: 1.2 MegOhm (High side) / 750 Ohm (Low side) = 1601:1     */
#define AMC1301_DIVIDER_RATIO       (1601.0f)
#define AMC1301_DIFFERENTIAL_GAIN   (8.2f)
#define AMC1301_ZERO_VOLTAGE_MV     (1440.0f)  /* Common mode output voltage   */
#define VOLTS_PER_ADC_COUNT         ((ADC_VREF_MV / ADC_RESOLUTION) * (AMC1301_DIVIDER_RATIO / AMC1301_DIFFERENTIAL_GAIN) / 1000.0f)

/* Raw ADC Watchdog Threshold Computation */
/* AWD trip count = ((Vzero +/- (I_trip * Sensitivity)) / Vref) * 4096        */
#define AWD_IPHASE_HI_THRESHOLD     ((uint16_t)(((ACS758_ZERO_CURRENT_MV + (TRIP_INSTANTANEOUS_PEAK_A * ACS758_SENSITIVITY_MV_PER_A)) / ADC_VREF_MV) * 4095.0f))
#define AWD_IPHASE_LO_THRESHOLD     ((uint16_t)(((ACS758_ZERO_CURRENT_MV - (TRIP_INSTANTANEOUS_PEAK_A * ACS758_SENSITIVITY_MV_PER_A)) / ADC_VREF_MV) * 4095.0f))

#define AWD_INEUT_HI_THRESHOLD      ((uint16_t)(((ACS758_ZERO_CURRENT_MV + (TRIP_NEUTRAL_PEAK_A * ACS758_SENSITIVITY_MV_PER_A)) / ADC_VREF_MV) * 4095.0f))
#define AWD_INEUT_LO_THRESHOLD      ((uint16_t)(((ACS758_ZERO_CURRENT_MV - (TRIP_NEUTRAL_PEAK_A * ACS758_SENSITIVITY_MV_PER_A)) / ADC_VREF_MV) * 4095.0f))

/* ========================================================================== */
/*                             METERING BUFFER                                */
/* ========================================================================== */
#define SAMPLES_PER_CYCLE           (128U)     /* 128 samples per 20ms AC wave */
#define SAMPLING_FREQ_HZ            (SAMPLES_PER_CYCLE * 50U) /* 6400 Hz       */
#define DMA_BUFFER_SIZE             (SAMPLES_PER_CYCLE * 2U)  /* Ping-Pong buf */

/* ========================================================================== */
/*                          GPIO PIN DEFINITIONS                              */
/* Portable GPIO pin masks and register definitions for host simulation/tests */
#ifndef STM32G431xx
#define GPIO_PIN_0                  (1U << 0)
#define GPIO_PIN_1                  (1U << 1)
#define GPIO_PIN_2                  (1U << 2)
#define GPIO_PIN_3                  (1U << 3)
#define GPIO_PIN_4                  (1U << 4)
#define GPIO_PIN_5                  (1U << 5)
#define GPIO_PIN_6                  (1U << 6)
#define GPIO_PIN_8                  (1U << 8)
#define GPIO_PIN_9                  (1U << 9)

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t CCR5;
    volatile uint32_t CCR6;
    volatile uint32_t CCMR3;
    volatile uint32_t DTR2;
    volatile uint32_t ECR;
    volatile uint32_t TISEL;
    volatile uint32_t AF1;
    volatile uint32_t AF2;
    volatile uint32_t OR;
} TIM_TypeDef;

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
    volatile uint32_t PRESC;
} USART_TypeDef;

extern GPIO_TypeDef g_cfg_gpioa;
extern GPIO_TypeDef g_cfg_gpiob;
extern TIM_TypeDef  g_mock_tim1;
extern USART_TypeDef g_mock_usart2;

#define GPIOA                       (&g_cfg_gpioa)
#define GPIOB                       (&g_cfg_gpiob)
#define TIM1                        (&g_mock_tim1)
#define USART2                      (&g_mock_usart2)

#define TIM_BDTR_MOE                (1U << 15)
#define TIM_SR_BIF                  (1U << 7)
#endif

/* Si8274 Gate Driver PWM / Enable Controls (TIM1 Hardware Break) */
#define GATE_PHASE_PIN              (GPIO_PIN_8)   /* PA8: TIM1_CH1 (Phase Gate) */
#define GATE_PHASE_PORT             (GPIOA)
#define GATE_NEUTRAL_PIN            (GPIO_PIN_9)   /* PA9: TIM1_CH2 (Neutral Gate)*/
#define GATE_NEUTRAL_PORT           (GPIOA)

/* Hardware Break 1 Input Pin */
#define TIM1_BKIN_PIN               (GPIO_PIN_6)   /* PA6: TIM1_BKIN             */
#define TIM1_BKIN_PORT              (GPIOA)

/* Active PFC TRIAC Switch Trigger Pin */
#define PFC_TRIAC_PIN               (GPIO_PIN_4)   /* PB4: Optical TRIAC gate    */
#define PFC_TRIAC_PORT              (GPIOB)

/* Analog Input Pin Assignments */
#define SENSE_CURR_PHASE_PIN        (GPIO_PIN_0)   /* PA0: ADC1_IN1 / COMP1_INP (CS1) */
#define SENSE_CURR_PHASE_PORT       (GPIOA)
#define SENSE_CURR_NEUT_PIN         (GPIO_PIN_1)   /* PA1: ADC2_IN1 / COMP2_INP (CS2) */
#define SENSE_CURR_NEUT_PORT        (GPIOA)
#define SENSE_VOLT_LINE_PIN         (GPIO_PIN_2)   /* PA2: ADC1_IN2 (AMC1301 Output)  */
#define SENSE_VOLT_LINE_PORT        (GPIOA)

/* Gate Driver Enable Output */
#define GATE_EN_PIN                 (GPIO_PIN_0)   /* PB0: Si8274 Enable (Active High)*/
#define GATE_EN_PORT                (GPIOB)

/* Visual Status Indicators */
#define LED_STATUS_OK_PIN           (GPIO_PIN_3)   /* PB3: Green LED (Normal Run)*/
#define LED_STATUS_OK_PORT          (GPIOB)
#define LED_FAULT_PIN               (GPIO_PIN_5)   /* PB5: Red LED (Tripped)     */
#define LED_FAULT_PORT              (GPIOB)

/* Manual Physical Pushbutton / Relay Override (Relocated to prevent ADC contention) */
#define BTN_TRIP_PIN                (GPIO_PIN_1)   /* PB1: Manual Trip Button    */
#define BTN_TRIP_PORT               (GPIOB)
#define BTN_RESET_PIN               (GPIO_PIN_6)   /* PB6: Manual Reset Button   */
#define BTN_RESET_PORT              (GPIOB)

/* Isolated Telemetry UART (to ESP32 via PC817 Optocouplers) */
#define TELEMETRY_UART              (USART2)
#define TELEMETRY_BAUDRATE          (115200U)

#ifdef __cplusplus
}
#endif

#endif /* SSCB_CONFIG_H */
