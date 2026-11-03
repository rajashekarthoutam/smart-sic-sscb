/**
 * @file main.c
 * @brief Main execution entry point for Smart Bidirectional Double-Pole SiC SSCB.
 * @target STM32G431KB (Cortex-M4F @ 170MHz)
 */

#if defined(__has_include)
  #if __has_include("sscb_config.h")
    #include "sscb_config.h"
    #include "sscb_protection.h"
    #include "sscb_metering.h"
    #include "sscb_pfc.h"
    #include "telemetry_uart.h"
  #elif __has_include("../Inc/sscb_config.h")
    #include "../Inc/sscb_config.h"
    #include "../Inc/sscb_protection.h"
    #include "../Inc/sscb_metering.h"
    #include "../Inc/sscb_pfc.h"
    #include "../Inc/telemetry_uart.h"
  #else
    #include "sscb_config.h"
    #include "sscb_protection.h"
    #include "sscb_metering.h"
    #include "sscb_pfc.h"
    #include "telemetry_uart.h"
  #endif
#else
  #include "sscb_config.h"
  #include "sscb_protection.h"
  #include "sscb_metering.h"
  #include "sscb_pfc.h"
  #include "telemetry_uart.h"
#endif

/* Static sample storage buffer (Ping-Pong DMA simulation / buffer) */
static adc_sample_t g_adc_dma_buffer[SAMPLES_PER_CYCLE];
static volatile bool g_dma_cycle_complete = false;

/* Timing ticks */
static uint32_t g_last_telemetry_tick = 0;
static uint32_t g_last_pfc_tick = 0;

/**
 * @brief System Hardware Peripherals Initialization.
 */
static void System_Peripherals_Init(void)
{
    /* 
     * In full STM32Cube environment:
     * - HAL_Init();
     * - SystemClock_Config(); (170 MHz)
     * - MX_GPIO_Init();
     * - MX_DMA_Init();
     * - MX_ADC1_Init(); (Configured for Injected or Continuous DMA conversions)
     * - MX_TIM1_Init(); (Break and Dead-Time Register enabled for PA8 & PA9)
     * - MX_USART2_UART_Init(); (115200 baud, 8N1)
     */
    SSCB_Protection_Init();
    SSCB_Metering_Init();
    SSCB_PFC_Init();
    Telemetry_Init();
}

/**
 * @brief ADC DMA Conversion Complete Callback (every 20ms grid cycle).
 */
void HAL_ADC_ConvCpltCallback(void)
{
    g_dma_cycle_complete = true;
}

/**
 * @brief Main execution loop.
 */
int main(void)
{
    System_Peripherals_Init();

    /* Automatically transition from INIT to STANDBY */
    SSCB_ResetFault();

    /* Close breaker upon power-up initialization (or wait for manual/remote command) */
    SSCB_CloseBreaker();

    uint32_t simulated_tick = 0;

    while (1) {
        simulated_tick++;
        SSCB_UpdateTick(simulated_tick * 1000); /* Increment 1ms (1000 µs) per system loop */

        /* 1. Check for Completed 20ms AC Cycle Buffer */
        if (g_dma_cycle_complete) {
            g_dma_cycle_complete = false;

            sscb_metering_metrics_t metrics;
            SSCB_Metering_ProcessCycle(g_adc_dma_buffer, SAMPLES_PER_CYCLE, &metrics);

            /* Dynamic PFC Compensation Evaluation */
            SSCB_PFC_Evaluate(&metrics);
        }

        /* 2. Periodic Telemetry Transmission (every 100ms) */
        if ((simulated_tick - g_last_telemetry_tick) >= 10) {
            g_last_telemetry_tick = simulated_tick;

            const sscb_telemetry_t* telem = SSCB_GetTelemetry();
            const sscb_metering_metrics_t* met = SSCB_Metering_GetLatest();
            Telemetry_SendPeriodicUpdate(telem, met);
        }

        /* 3. Physical Pushbutton Debouncing & Evaluation (PB1 Trip, PB6 Reset) */
        static uint8_t trip_btn_history = 0xFF;
        static uint8_t reset_btn_history = 0xFF;

        /* Sample PB1 (Active LOW with internal pull-up) */
        trip_btn_history = (trip_btn_history << 1) | ((BTN_TRIP_PORT->IDR & BTN_TRIP_PIN) ? 1 : 0);
        if (trip_btn_history == 0x80) { /* Falling edge confirmed over 8 samples */
            SSCB_TripBreaker(FAULT_MANUAL_TRIP);
        }

        /* Sample PB6 (Active LOW with internal pull-up) */
        reset_btn_history = (reset_btn_history << 1) | ((BTN_RESET_PORT->IDR & BTN_RESET_PIN) ? 1 : 0);
        if (reset_btn_history == 0x80) { /* Falling edge confirmed over 8 samples */
            if (SSCB_ResetFault()) {
                SSCB_CloseBreaker();
            }
        }

        /* Feed Independent Watchdog (IWDG) */
    }

    return 0;
}
