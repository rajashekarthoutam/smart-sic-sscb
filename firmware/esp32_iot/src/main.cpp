/**
 * @file main.cpp
 * @brief ESP32 FreeRTOS IoT Co-Processor Application for SiC SSCB.
 *        Bridges isolated optocoupled UART telemetry to Wi-Fi/MQTT.
 */

#if defined(__has_include)
  #if __has_include("esp_sscb_config.h")
    #include "esp_sscb_config.h"
  #elif __has_include("../include/esp_sscb_config.h")
    #include "../include/esp_sscb_config.h"
  #else
    #include "esp_sscb_config.h"
  #endif
#else
  #include "esp_sscb_config.h"
#endif

#if defined(__has_include) && __has_include(<Arduino.h>)
  #include <Arduino.h>
  #include <WiFi.h>
  #include <PubSubClient.h>
  #include <ArduinoJson.h>
#else
  #include <cstdint>
  #include <cstdio>
  #include <cstring>
  #include <string>

  typedef uint8_t byte;
  #define SERIAL_8N1 0x06
  #define WL_CONNECTED 3
  #define pdMS_TO_TICKS(ms) (ms)

  class HardwareSerial {
  public:
      void begin(unsigned long, uint32_t = 0, int = -1, int = -1) {}
      int available() { return 0; }
      int read() { return -1; }
      void print(const char*) {}
      void println(const char* = "") {}
      size_t write(const uint8_t*, size_t s) { return s; }
  };
  extern HardwareSerial Serial;
  extern HardwareSerial Serial2;

  class WiFiClient {};
  class WiFiClass {
  public:
      void begin(const char*, const char*) {}
      int status() { return WL_CONNECTED; }
  };
  extern WiFiClass WiFi;

  class PubSubClient {
  public:
      PubSubClient() {}
      PubSubClient(WiFiClient&) {}
      void setServer(const char*, uint16_t) {}
      void setCallback(void (*)(char*, byte*, unsigned int)) {}
      bool connect(const char*) { return true; }
      bool connected() { return true; }
      void loop() {}
      bool publish(const char*, const char*, bool = false) { return true; }
      bool subscribe(const char*) { return true; }
  };

  class DeserializationError {
  public:
      operator bool() const { return false; }
  };

  template<size_t N>
  class StaticJsonDocument {
  public:
      class Element {
      public:
          template<typename T>
          void operator=(T) {}
          operator const char*() const { return ""; }
      };
      Element operator[](const char*) { return Element(); }
  };

  template<typename T, typename U>
  inline DeserializationError deserializeJson(T&, U&) { return DeserializationError(); }
  template<typename T, typename U>
  inline size_t serializeJson(const T&, U&) { return 0; }

  inline unsigned long millis() { return 0; }
  inline void delay(unsigned long) {}
  inline void vTaskDelay(uint32_t) {}
  inline void xTaskCreatePinnedToCore(void (*)(void*), const char*, uint32_t, void*, int, void*, int) {}
#endif

/* Protocol constants */
#define TELEMETRY_HEADER1   0xAA
#define TELEMETRY_HEADER2   0x55
#define TELEMETRY_FOOTER1   0x0D
#define TELEMETRY_FOOTER2   0x0A

#define PKT_PERIODIC        0x01
#define PKT_ALARM           0x02
#define PKT_COMMAND         0x03

#define CMD_TRIP            0x10
#define CMD_CLOSE           0x20
#define CMD_RESET           0x30

/* Telemetry data cache */
struct SSCBData {
    uint8_t state;
    uint8_t last_fault;
    uint16_t trip_count;
    float v_rms;
    float i_phase_rms;
    float i_neutral_rms;
    float active_power_w;
    float power_factor;
    bool pfc_active;
} g_live_data;

struct AlarmData {
    uint8_t fault_code;
    float trip_current_peak;
    uint32_t timestamp_us;
} g_last_alarm;

static volatile bool g_new_telemetry_available = false;
static volatile bool g_new_alarm_available = false;

/* Wi-Fi & MQTT Client */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* ========================================================================== */
/*                             CRC-16-CCITT                                   */
/* ========================================================================== */

static uint16_t ComputeCRC16(const uint8_t* data, uint16_t length)
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
/*                       TRANSMIT COMMAND TO STM32                            */
/* ========================================================================== */

static void SendCommandToSTM32(uint8_t cmd_code)
{
    uint8_t pkt[16];
    uint8_t idx = 0;

    pkt[idx++] = TELEMETRY_HEADER1;
    pkt[idx++] = TELEMETRY_HEADER2;
    pkt[idx++] = PKT_COMMAND;
    pkt[idx++] = 1; /* Payload length */
    pkt[idx++] = cmd_code;

    /* Compute CRC over Type + Len + Payload */
    uint16_t crc = ComputeCRC16(&pkt[2], 3);
    pkt[idx++] = (uint8_t)((crc >> 8) & 0xFF);
    pkt[idx++] = (uint8_t)(crc & 0xFF);

    pkt[idx++] = TELEMETRY_FOOTER1;
    pkt[idx++] = TELEMETRY_FOOTER2;

    Serial2.write(pkt, idx);
}

/* ========================================================================== */
/*                           MQTT CALLBACK & CONNECT                          */
/* ========================================================================== */

void OnMqttMessage(char* topic, byte* payload, unsigned int length)
{
    char msg[64];
    if (length >= sizeof(msg)) length = sizeof(msg) - 1;
    memcpy(msg, payload, length);
    msg[length] = '\0';

    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) return;

    const char* action = doc["cmd"];
    if (action) {
        if (strcmp(action, "trip") == 0) {
            SendCommandToSTM32(CMD_TRIP);
        } else if (strcmp(action, "close") == 0) {
            SendCommandToSTM32(CMD_CLOSE);
        } else if (strcmp(action, "reset") == 0) {
            SendCommandToSTM32(CMD_RESET);
        }
    }
}

static bool EnsureMqttConnected(void)
{
    static uint32_t last_reconnect_attempt = 0;
    if (mqttClient.connected()) {
        return true;
    }

    if (millis() - last_reconnect_attempt >= 3000) {
        last_reconnect_attempt = millis();
        if (mqttClient.connect(MQTT_CLIENT_ID)) {
            mqttClient.subscribe(TOPIC_COMMANDS);
            mqttClient.publish(TOPIC_STATUS, "{\"status\":\"online\"}", true);
            return true;
        }
    }
    return false;
}

/* ========================================================================== */
/*                         ISOLATED UART RX TASK                              */
/* ========================================================================== */

void UartRxTask(void* pvParameters)
{
    enum RxState { WAIT_H1, WAIT_H2, TYPE, LEN, PAYLOAD, CRC1, CRC2, F1, F2 };
    RxState state = WAIT_H1;

    uint8_t pkt_type = 0;
    uint8_t pkt_len = 0;
    uint8_t payload[64];
    uint8_t p_idx = 0;
    uint16_t received_crc = 0;

    for (;;) {
        while (Serial2.available()) {
            uint8_t b = Serial2.read();

            switch (state) {
                case WAIT_H1:
                    if (b == TELEMETRY_HEADER1) state = WAIT_H2;
                    break;
                case WAIT_H2:
                    if (b == TELEMETRY_HEADER2) state = TYPE;
                    else state = WAIT_H1;
                    break;
                case TYPE:
                    pkt_type = b;
                    state = LEN;
                    break;
                case LEN:
                    pkt_len = b;
                    p_idx = 0;
                    if (pkt_len > sizeof(payload)) state = WAIT_H1;
                    else if (pkt_len == 0) state = CRC1;
                    else state = PAYLOAD;
                    break;
                case PAYLOAD:
                    payload[p_idx++] = b;
                    if (p_idx >= pkt_len) state = CRC1;
                    break;
                case CRC1:
                    received_crc = ((uint16_t)b << 8);
                    state = CRC2;
                    break;
                case CRC2:
                    received_crc |= b;
                    state = F1;
                    break;
                case F1:
                    if (b == TELEMETRY_FOOTER1) state = F2;
                    else state = WAIT_H1;
                    break;
                case F2:
                    if (b == TELEMETRY_FOOTER2) {
                        /* Check CRC */
                        uint8_t check_buf[66];
                        check_buf[0] = pkt_type;
                        check_buf[1] = pkt_len;
                        memcpy(&check_buf[2], payload, pkt_len);
                        uint16_t calc_crc = ComputeCRC16(check_buf, pkt_len + 2);

                        if (calc_crc == received_crc) {
                            if (pkt_type == PKT_PERIODIC && pkt_len >= 26) {
                                g_live_data.state = payload[0];
                                g_live_data.last_fault = payload[1];
                                g_live_data.trip_count = payload[2] | ((uint16_t)payload[3] << 8);

                                memcpy(&g_live_data.v_rms, &payload[4], 4);
                                memcpy(&g_live_data.i_phase_rms, &payload[8], 4);
                                memcpy(&g_live_data.i_neutral_rms, &payload[12], 4);
                                memcpy(&g_live_data.active_power_w, &payload[16], 4);
                                memcpy(&g_live_data.power_factor, &payload[20], 4);
                                g_live_data.pfc_active = (payload[24] != 0);

                                g_new_telemetry_available = true;
                            } else if (pkt_type == PKT_ALARM && pkt_len >= 10) {
                                g_last_alarm.fault_code = payload[1];
                                memcpy(&g_last_alarm.trip_current_peak, &payload[2], 4);
                                memcpy(&g_last_alarm.timestamp_us, &payload[6], 4);

                                g_new_alarm_available = true;
                            }
                        }
                    }
                    state = WAIT_H1;
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* ========================================================================== */
/*                            MAIN ARDUINO ENTRY                              */
/* ========================================================================== */

void setup()
{
    Serial.begin(115200);

    /* Initialize isolated UART connecting to STM32 PC817 optocouplers */
    Serial2.begin(OPTO_UART_BAUDRATE, SERIAL_8N1, OPTO_UART_RX_PIN, OPTO_UART_TX_PIN);

    /* Connect to Wi-Fi */
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    /* Initialize MQTT */
    mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqttClient.setCallback(OnMqttMessage);

    /* Spawn FreeRTOS background task for isolated UART reception */
    xTaskCreatePinnedToCore(UartRxTask, "UartRxTask", 4096, NULL, 5, NULL, 1);
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED) {
        if (EnsureMqttConnected()) {
            mqttClient.loop();

            /* Handle immediate alarm publication */
            if (g_new_alarm_available) {
                g_new_alarm_available = false;

                StaticJsonDocument<256> doc;
                doc["event"] = "TRIP_ALARM";
                doc["fault_code"] = g_last_alarm.fault_code;
                doc["peak_current_a"] = g_last_alarm.trip_current_peak;
                doc["timestamp_us"] = g_last_alarm.timestamp_us;

                char buf[256];
                serializeJson(doc, buf);
                mqttClient.publish(TOPIC_ALARMS, buf, true);
            }

            /* Handle periodic telemetry publication */
            static uint32_t last_pub = 0;
            if (millis() - last_pub >= 500 && g_new_telemetry_available) {
                last_pub = millis();
                g_new_telemetry_available = false;

                StaticJsonDocument<384> doc;
                doc["state"] = g_live_data.state;
                doc["fault"] = g_live_data.last_fault;
                doc["trips"] = g_live_data.trip_count;
                doc["v_rms"] = g_live_data.v_rms;
                doc["i_phase_rms"] = g_live_data.i_phase_rms;
                doc["i_neutral_rms"] = g_live_data.i_neutral_rms;
                doc["power_w"] = g_live_data.active_power_w;
                doc["power_factor"] = g_live_data.power_factor;
                doc["pfc_active"] = g_live_data.pfc_active;

                char buf[384];
                serializeJson(doc, buf);
                mqttClient.publish(TOPIC_TELEMETRY, buf);
            }
        }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}
