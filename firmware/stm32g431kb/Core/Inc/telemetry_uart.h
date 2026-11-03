/**
 * @file telemetry_uart.h
 * @brief Opto-isolated UART binary framing protocol and CRC-16 engine
 *        for communication between STM32G431KB and ESP32 co-processor.
 */

#ifndef TELEMETRY_UART_H
#define TELEMETRY_UART_H

#include "sscb_config.h"
#include "sscb_protection.h"
#include "sscb_metering.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Packet framing constants */
#define TELEMETRY_HEADER_BYTE1      (0xAA)
#define TELEMETRY_HEADER_BYTE2      (0x55)
#define TELEMETRY_FOOTER_BYTE1      (0x0D) /* '\r' */
#define TELEMETRY_FOOTER_BYTE2      (0x0A) /* '\n' */

#define MAX_PAYLOAD_LEN             (64U)
#define MAX_PACKET_LEN              (MAX_PAYLOAD_LEN + 8U)

/* Packet Types */
typedef enum {
    PKT_TYPE_PERIODIC_TELEMETRY = 0x01,
    PKT_TYPE_EMERGENCY_ALARM    = 0x02,
    PKT_TYPE_REMOTE_COMMAND     = 0x03,
    PKT_TYPE_COMMAND_ACK        = 0x04
} telemetry_pkt_type_t;

/* Remote Command IDs (ESP32 -> STM32) */
typedef enum {
    CMD_TRIP_BREAKER            = 0x10,
    CMD_CLOSE_BREAKER           = 0x20,
    CMD_RESET_FAULT             = 0x30,
    CMD_PING                    = 0x40
} remote_cmd_id_t;

/* ========================================================================== */
/*                            FUNCTION PROTOTYPES                             */
/* ========================================================================== */

/**
 * @brief Initialize UART hardware and ring buffers.
 */
void Telemetry_Init(void);

/**
 * @brief Transmit periodic telemetry snapshot packet over isolated UART.
 */
void Telemetry_SendPeriodicUpdate(const sscb_telemetry_t* telemetry,
                                  const sscb_metering_metrics_t* metrics);

/**
 * @brief Transmit immediate high-priority emergency trip alarm frame.
 */
void Telemetry_SendEmergencyAlarm(sscb_fault_t fault, float trip_current, uint32_t timestamp_us);

/**
 * @brief Parse incoming byte stream from isolated UART receiver.
 * @param byte Incoming byte from UART RX interrupt or DMA.
 */
void Telemetry_ProcessByte(uint8_t byte);

/**
 * @brief Compute standard CRC-16-CCITT (0x1021, init 0xFFFF).
 */
uint16_t Telemetry_ComputeCRC16(const uint8_t* data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_UART_H */
