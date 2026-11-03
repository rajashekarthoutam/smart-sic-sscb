/**
 * @file telemetry_uart.c
 * @brief Opto-isolated UART binary framing protocol implementation with CRC-16.
 * @target STM32G431KB
 */

#if defined(__has_include)
  #if __has_include("telemetry_uart.h")
    #include "telemetry_uart.h"
  #elif __has_include("../Inc/telemetry_uart.h")
    #include "../Inc/telemetry_uart.h"
  #else
    #include "telemetry_uart.h"
  #endif
#else
  #include "telemetry_uart.h"
#endif
#include <string.h>

/* Parser state machine */
typedef enum {
    RX_STATE_WAIT_H1 = 0,
    RX_STATE_WAIT_H2,
    RX_STATE_TYPE,
    RX_STATE_LEN,
    RX_STATE_PAYLOAD,
    RX_STATE_CRC1,
    RX_STATE_CRC2,
    RX_STATE_F1,
    RX_STATE_F2
} rx_parser_state_t;

static rx_parser_state_t g_rx_state = RX_STATE_WAIT_H1;
static uint8_t g_rx_type = 0;
static uint8_t g_rx_len = 0;
static uint8_t g_rx_payload[MAX_PAYLOAD_LEN];
static uint8_t g_rx_payload_idx = 0;
static uint16_t g_rx_crc_received = 0;

/* UART hardware transmit hook */
static void HW_UART_Transmit(const uint8_t* data, uint16_t length)
{
    /* 
     * In STM32G4 hardware:
     * HAL_UART_Transmit_DMA(&huart2, (uint8_t*)data, length);
     * or non-blocking ring buffer.
     */
    (void)data;
    (void)length;
}

/* ========================================================================== */
/*                             CRC-16-CCITT                                   */
/* ========================================================================== */

uint16_t Telemetry_ComputeCRC16(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; ++i) {
        crc ^= ((uint16_t)data[i] << 8);
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/* ========================================================================== */
/*                            INIT & TRANSMISSION                             */
/* ========================================================================== */

void Telemetry_Init(void)
{
    g_rx_state = RX_STATE_WAIT_H1;
    g_rx_payload_idx = 0;
}

void Telemetry_SendPeriodicUpdate(const sscb_telemetry_t* telemetry,
                                  const sscb_metering_metrics_t* metrics)
{
    if (telemetry == NULL || metrics == NULL) {
        return;
    }

    uint8_t packet[MAX_PACKET_LEN];
    uint8_t payload[32];
    uint8_t p_idx = 0;

    /* Pack periodic telemetry payload */
    payload[p_idx++] = (uint8_t)telemetry->current_state;
    payload[p_idx++] = (uint8_t)telemetry->last_fault;
    payload[p_idx++] = (uint8_t)(telemetry->trip_count & 0xFF);
    payload[p_idx++] = (uint8_t)((telemetry->trip_count >> 8) & 0xFF);

    memcpy(&payload[p_idx], &metrics->v_rms, sizeof(float)); p_idx += sizeof(float);
    memcpy(&payload[p_idx], &metrics->i_phase_rms, sizeof(float)); p_idx += sizeof(float);
    memcpy(&payload[p_idx], &metrics->i_neutral_rms, sizeof(float)); p_idx += sizeof(float);
    memcpy(&payload[p_idx], &metrics->active_power_w, sizeof(float)); p_idx += sizeof(float);
    memcpy(&payload[p_idx], &metrics->power_factor, sizeof(float)); p_idx += sizeof(float);

    payload[p_idx++] = (uint8_t)(telemetry->pfc_active ? 1 : 0);
    payload[p_idx++] = 0x00; /* Reserved */

    /* Assemble framing */
    uint16_t pkt_len = 0;
    packet[pkt_len++] = TELEMETRY_HEADER_BYTE1;
    packet[pkt_len++] = TELEMETRY_HEADER_BYTE2;
    packet[pkt_len++] = (uint8_t)PKT_TYPE_PERIODIC_TELEMETRY;
    packet[pkt_len++] = p_idx;

    memcpy(&packet[pkt_len], payload, p_idx);
    pkt_len += p_idx;

    /* Compute CRC-16 over Type + Len + Payload */
    uint16_t crc = Telemetry_ComputeCRC16(&packet[2], (uint16_t)(p_idx + 2));
    packet[pkt_len++] = (uint8_t)((crc >> 8) & 0xFF);
    packet[pkt_len++] = (uint8_t)(crc & 0xFF);

    packet[pkt_len++] = TELEMETRY_FOOTER_BYTE1;
    packet[pkt_len++] = TELEMETRY_FOOTER_BYTE2;

    HW_UART_Transmit(packet, pkt_len);
}

void Telemetry_SendEmergencyAlarm(sscb_fault_t fault, float trip_current, uint32_t timestamp_us)
{
    uint8_t packet[MAX_PACKET_LEN];
    uint8_t payload[16];
    uint8_t p_idx = 0;

    payload[p_idx++] = (uint8_t)SSCB_STATE_TRIPPED_FAULT;
    payload[p_idx++] = (uint8_t)fault;
    memcpy(&payload[p_idx], &trip_current, sizeof(float)); p_idx += sizeof(float);
    memcpy(&payload[p_idx], &timestamp_us, sizeof(uint32_t)); p_idx += sizeof(uint32_t);

    uint16_t pkt_len = 0;
    packet[pkt_len++] = TELEMETRY_HEADER_BYTE1;
    packet[pkt_len++] = TELEMETRY_HEADER_BYTE2;
    packet[pkt_len++] = (uint8_t)PKT_TYPE_EMERGENCY_ALARM;
    packet[pkt_len++] = p_idx;

    memcpy(&packet[pkt_len], payload, p_idx);
    pkt_len += p_idx;

    uint16_t crc = Telemetry_ComputeCRC16(&packet[2], (uint16_t)(p_idx + 2));
    packet[pkt_len++] = (uint8_t)((crc >> 8) & 0xFF);
    packet[pkt_len++] = (uint8_t)(crc & 0xFF);

    packet[pkt_len++] = TELEMETRY_FOOTER_BYTE1;
    packet[pkt_len++] = TELEMETRY_FOOTER_BYTE2;

    HW_UART_Transmit(packet, pkt_len);
}

/* ========================================================================== */
/*                             RX PACKET PARSER                               */
/* ========================================================================== */

static void HandleIncomingCommand(uint8_t cmd_id)
{
    switch ((remote_cmd_id_t)cmd_id) {
        case CMD_TRIP_BREAKER:
            SSCB_TripBreaker(FAULT_MANUAL_TRIP);
            break;
        case CMD_CLOSE_BREAKER:
            SSCB_CloseBreaker();
            break;
        case CMD_RESET_FAULT:
            SSCB_ResetFault();
            break;
        case CMD_PING:
        default:
            break;
    }
}

void Telemetry_ProcessByte(uint8_t byte)
{
    switch (g_rx_state) {
        case RX_STATE_WAIT_H1:
            if (byte == TELEMETRY_HEADER_BYTE1) {
                g_rx_state = RX_STATE_WAIT_H2;
            }
            break;

        case RX_STATE_WAIT_H2:
            if (byte == TELEMETRY_HEADER_BYTE2) {
                g_rx_state = RX_STATE_TYPE;
            } else {
                g_rx_state = RX_STATE_WAIT_H1;
            }
            break;

        case RX_STATE_TYPE:
            g_rx_type = byte;
            g_rx_state = RX_STATE_LEN;
            break;

        case RX_STATE_LEN:
            g_rx_len = byte;
            if (g_rx_len > MAX_PAYLOAD_LEN) {
                g_rx_state = RX_STATE_WAIT_H1; /* Overflow abort */
            } else if (g_rx_len == 0) {
                g_rx_state = RX_STATE_CRC1;
            } else {
                g_rx_payload_idx = 0;
                g_rx_state = RX_STATE_PAYLOAD;
            }
            break;

        case RX_STATE_PAYLOAD:
            g_rx_payload[g_rx_payload_idx++] = byte;
            if (g_rx_payload_idx >= g_rx_len) {
                g_rx_state = RX_STATE_CRC1;
            }
            break;

        case RX_STATE_CRC1:
            g_rx_crc_received = ((uint16_t)byte << 8);
            g_rx_state = RX_STATE_CRC2;
            break;

        case RX_STATE_CRC2:
            g_rx_crc_received |= (uint16_t)byte;
            g_rx_state = RX_STATE_F1;
            break;

        case RX_STATE_F1:
            if (byte == TELEMETRY_FOOTER_BYTE1) {
                g_rx_state = RX_STATE_F2;
            } else {
                g_rx_state = RX_STATE_WAIT_H1;
            }
            break;

        case RX_STATE_F2:
        {
            if (byte == TELEMETRY_FOOTER_BYTE2) {
                /* Validate CRC-16 — variables scoped inside compound block (C11 §6.8.4.2) */
                uint8_t  crc_check_buf[MAX_PAYLOAD_LEN + 2U];
                uint16_t computed_crc;
                crc_check_buf[0] = g_rx_type;
                crc_check_buf[1] = g_rx_len;
                if (g_rx_len > 0U) {
                    memcpy(&crc_check_buf[2], g_rx_payload, g_rx_len);
                }
                computed_crc = Telemetry_ComputeCRC16(crc_check_buf, (uint16_t)(g_rx_len + 2U));

                if (computed_crc == g_rx_crc_received) {
                    if (g_rx_type == PKT_TYPE_REMOTE_COMMAND && g_rx_len >= 1U) {
                        HandleIncomingCommand(g_rx_payload[0]);
                    }
                }
            }
            g_rx_state = RX_STATE_WAIT_H1;
            break;
        }
    }
}
