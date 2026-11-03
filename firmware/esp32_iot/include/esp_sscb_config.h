/**
 * @file esp_sscb_config.h
 * @brief Configuration parameters for ESP32 IoT co-processor.
 */

#ifndef ESP_SSCB_CONFIG_H
#define ESP_SSCB_CONFIG_H

/* Wi-Fi Credentials */
#define WIFI_SSID               "SSCB_LAB_WIFI"
#define WIFI_PASS               "HighVoltageSiC2026"

/* MQTT Broker Settings */
#define MQTT_BROKER_HOST        "broker.hivemq.com"
#define MQTT_BROKER_PORT        1883
#define MQTT_CLIENT_ID          "ESP32_SiC_SSCB_01"

/* MQTT Topics */
#define TOPIC_TELEMETRY         "sscb/telemetry"
#define TOPIC_ALARMS            "sscb/alarms"
#define TOPIC_COMMANDS          "sscb/commands"
#define TOPIC_STATUS            "sscb/status"

/* Isolated UART Pins (connecting to PC817 Optocouplers) */
/* UART_NUM_2 is defined by ESP-IDF; provide a portable fallback for host builds */
#ifndef UART_NUM_2
#define UART_NUM_2              (2)
#endif
#define OPTO_UART_NUM           UART_NUM_2
#define OPTO_UART_RX_PIN        16
#define OPTO_UART_TX_PIN        17
#define OPTO_UART_BAUDRATE      115200

#endif /* ESP_SSCB_CONFIG_H */

