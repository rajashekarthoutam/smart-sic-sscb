# Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker (SSCB)
### Autonomous Sub-Microsecond Analog Watchdog Protection, Dynamic PFC, and Cloud Telemetry

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: STM32G4](https://img.shields.io/badge/Control%20Core-STM32G431KB%20(170MHz)-brightgreen.svg)](firmware/stm32g431kb/)
[![Co-Processor: ESP32](https://img.shields.io/badge/IoT%20Co--Processor-ESP32%20FreeRTOS-red.svg)](firmware/esp32_iot/)
[![Power Switch: Wolfspeed SiC](https://img.shields.io/badge/Power%20Device-Wolfspeed%201200V%20SiC%20(C3M)-orange.svg)](https://www.wolfspeed.com)
[![Interruption Speed](https://img.shields.io/badge/Trip%20Latency-%3C650%20ns%20(Sub--microsecond)-success.svg)](#sub-microsecond-hardware-trip-path)

---

## Academic Project Details
* **Project Title:** Smart Bi-Directional Double-Pole Solid State Circuit Breaker (SSCB) with SiC Power MOSFETs, STM32 and ESP32
* **Institution:** Department of Electronics and Communication Engineering, Joginpally B.R. Engineering College (UGC Autonomous, Affiliated to JNTUH, Accredited by NBA & NAAC A+ Grade)
* **Academic Year:** 2026–2027
* **Project Batch:**
  * **Mankala Shailusha** (23J21A0424)
  * **Thoutam Rajashekar** (24J25A0424)
  * **G Krithin** (24J25A0407)
* **Internal Guide:** Mrs. Sagupha Parween, B.Tech., M.Tech., Assistant Professor
* **Project Coordinator:** Dr. A. Rajaiah, B.E., M.Tech., Ph.D., Associate Professor
* **Head of the Department:** Dr. B. Abdul Raheem, Professor & HOD, Dept. of ECE

---

## 1. System Overview

Traditional electromechanical circuit breakers (MCBs) rely on spring-loaded mechanical contacts and thermal-magnetic trips that suffer from **10 to 30 ms mechanical inertia**, molten contact erosion, and high-temperature plasma arcs.

This repository presents the complete engineering implementation of a **Smart Bidirectional Double-Pole AC Solid-State Circuit Breaker (SSCB)**:
* **Topology:** Double-Pole (Phase & Neutral isolation), Bidirectional AC Solid-State Circuit Breaker.
* **Power Stage:** 4× **Wolfspeed C3M0075120K** (1200 V, 75 mΩ) SiC MOSFETs in common-source back-to-back pairs ($Q_1/Q_2$ for Phase, $Q_3/Q_4$ for Neutral).
* **Hard-Real-Time Trip Engine:** **STM32G431KB** (Arm Cortex-M4F @ 170 MHz) executing zero-CPU autonomous gate shutdown in **<515–650 ns** using internal **Analog Watchdogs (AWD1)** routed directly to Advanced Control Timer **`TIM1_BKIN` (Break Input)**.
* **Transient Energy Absorption:** Coordinated Littelfuse UltraMOV **V275LA40AP** (150 J rating) and high-frequency RC snubber network (47 Ω, 10 nF).
* **Power Quality Enhancement:** Active zero-crossing switched parallel PFC TRIAC/capacitor bank for dynamic displacement power factor ($\cos\phi$) correction.
* **Isolated Cloud Telemetry:** Opto-isolated UART link (2× PC817) to an **ESP32** co-processor publishing True-RMS $V_{\text{RMS}}$, $I_{\text{RMS}}$, active/apparent power, and alarms to Wi-Fi/MQTT (HiveMQ).

---

## 2. System Architecture

```text
========================================================================================
                                 AC MAINS (230V RMS, 50Hz)
========================================================================================
        |                                                              |
    [Phase L_in]                                                 [Neutral N_in]
        |                                                              |
   +----+----+                                                    +----+----+
   | ACS758  | (Hall Current)                                     | ACS758  | (Hall Current)
   +----+----+                                                    +----+----+
        |                                                              |
        +---------------+                                              +
        |               |                                              |
     [AMC1301]          |                                              |
   (Diff Voltage)       |                                              |
        |               |                                              |
        |      +--------+--------+                            +--------+--------+
        |      | Phase SiC Pair  |                            | Neutral SiC Pair|
        |      | Q1 (S) <-> (S) Q2|                            | Q3 (S) <-> (S) Q4|
        |      +--------+--------+                            +--------+--------+
        |               |                                              |
        |               +--------------+              +----------------+
        |                              |              |
        |                       +------+--------------+------+
        |                       | Active PFC Switched Triac  |
        |                       |       Capacitor Bank       |
        |                       +------+--------------+------+
        |                              |              |
        |                              v              v
        |                        [Phase L_out]  [Neutral N_out]
        |                              |              |
        |                        ==============================
        |                                   AC LOAD
        |                        ==============================
        |
        v (Differential analog)
+--------------------------------------------------------------------------------------+
|                     HARD-REAL-TIME CONTROL CORE (STM32G431KB)                        |
|                                                                                      |
|   +-------------------+    Hardware TRGO   +------------------------------------+    |
|   | Dual Analog       |------------------->| Advanced Timer TIM1 Break Circuit  |    |
|   | Watchdog (AWD1/2) |                    | (Forces Gate PWM LOW in <30ns)     |    |
|   +-------------------+                    +------------------+-----------------+    |
|             |                                                 |                      |
|             | Interrupt                                       | Gate Control         |
|             v                                                 v                      |
|   +-------------------+                    +------------------------------------+    |
|   | Protection & DSP  |                    | Si8274 High-CMTI Isolated Drivers  |    |
|   | Metering Engine   |                    | (Powered by 4x B0515S-1W +15V)     |    |
|   +-------------------+                    +------------------------------------+    |
+-------------+------------------------------------------------------------------------+
              | Optocoupled Isolated UART (2x PC817)
              v
+--------------------------------------------------------------------------------------+
|                      IOT TELEMETRY & CLOUD ENGINE (ESP32)                            |
|                                                                                      |
|   - Binary Packet Framing & CRC-16 Checksum Validation                               |
|   - FreeRTOS Dual-Task Architecture (UART Reader + Network Engine)                   |
|   - MQTT Client: Publishes "sscb/telemetry" & "sscb/alarms"; Subscribes "commands"   |
+--------------------------------------------------------------------------------------+
```

---

## 3. Sub-Microsecond Hardware Trip Path

Under short circuits, commercial SiC MOSFETs exhibit a Short-Circuit Withstand Time (SCWT) of only **5.0–6.6 μs** before destructive thermal runaway occurs (*Wang et al., IEEE TPEL 2016*). Standard software interrupt handlers cannot guarantee safety.

The proposed architecture bypasses the CPU core entirely:
```text
Fault Inception
      |
      v
ACS758 Sensor Delay (150 ns)
      |
      v
ADC Conversion Complete @ 4 MSPS (250 ns)
      |
      v
AWD Digital Comparator Out-of-Window (20 ns)
      |
      v  [Direct Silicon-Level Hardware TRGO Interconnect]
TIM1 Break MOE Cleared (<30 ns)
      |
      v
Si8274 Isolated Driver Cutoff (30 ns)
      |
      v
Wolfspeed SiC Channel Pinched Off (35 ns)
      |
      +------------------------------------> TOTAL TRIP TIME: ~515 ns (<1 μs)
```

---

## 4. Literature Foundations & Academic Benchmarks

This system is built upon landmark peer-reviewed research papers:

| Citation | Core Focus | Key Value to This Project |
| :--- | :--- | :--- |
| **H. Li et al.** (*Micromachines*, 2019) | 400V SiC SSCB with 720ns clearing | Derivation of MOV energy absorption: $W_{\text{MOV}} = \frac{1}{2}\left(\frac{V_{\text{DC}}}{V_{\text{MOV}}-V_{\text{DC}}}\right) L I_{\text{max}}^2$ |
| **H. Qin et al.** (*Electronics*, 2019) | Soft switch-off & Kelvin source $L_{AM}$ | Demonstrates Kelvin-to-power source inductive decoupling and gate clamping |
| **B. J. Hyon et al.** (*IEEE Access*, May 2024) | Bidirectional Wolfspeed C3M SSCB | Validates back-to-back common-source topology and parallel thyristor commutation |
| **Z. Wang et al.** (*IEEE TPEL*, 2016) | SiC short-circuit withstand physics | Establishes the $5\,\mu\text{s}$ SCWT limit and the necessity of $<1\,\mu\text{s}$ autonomous hardware clearing |

*See [`docs_thesis/literature_base_analysis.md`](docs_thesis/literature_base_analysis.md) for full mathematical comparisons.*

---

## 5. Repository Structure

```text
smart-sic-sscb/
├── docs_thesis/
│   ├── FIRST 5 PAGES MAJOR PROJECT.docx                           # Official Preliminary Pages (Title, Certificate, Abstract, Index)
│   ├── major project documentation 1st sem (Updated with Chapter 4).docx # Official 55-Page Major Project-I Thesis (Chapters 1–7 + IEEE References)
│   ├── SSCB_Final_Presentation.pptx                              # Major Project Defense Presentation Slide Deck
│   ├── IEEE_Base_Paper_SSCB.md                                    # Base IEEE Conference & Transactions Paper Analysis
│   ├── SSCB_Technical_Thesis.md                                   # Comprehensive Academic Thesis Markdown Draft
│   └── literature_base_analysis.md                                # Mathematical Synthesis of Foundation Papers
├── firmware/
│   ├── stm32g431kb/                                               # STM32G431KB Embedded C Real-Time Firmware
│   │   └── Core/
│   │       ├── Inc/
│   │       │   ├── sscb_config.h                                  # System ratings, ADC AWD thresholds, pinout mappings
│   │       │   ├── sscb_protection.h                              # AWD registers, trip states, fault code enums
│   │       │   ├── sscb_metering.h                                # True-RMS, Active/Reactive Power, cos(phi) DSP
│   │       │   ├── sscb_pfc.h                                     # Dynamic zero-crossing TRIAC capacitor controller
│   │       │   └── telemetry_uart.h                               # Opto-isolated packet protocol & CRC-16 engine
│   │       └── Src/
│   │           ├── sscb_protection.c                              # Sub-microsecond AWD handler & I2t thermal loop
│   │           ├── sscb_metering.c                                # 128-sample AC cycle DSP engine
│   │           ├── sscb_pfc.c                                     # Hysteresis zero-voltage crossing PFC control
│   │           ├── telemetry_uart.c                               # Isolated packet serializer/parser
│   │           └── main.c                                         # Deterministic 170 MHz control loop
│   └── esp32_iot/                                                 # ESP32 FreeRTOS IoT Telemetry Application
│       ├── include/
│       │   └── esp_sscb_config.h                                  # Wi-Fi credentials, MQTT TLS topics, UART pins
│       └── src/
│           └── main.cpp                                           # Dual-task FreeRTOS ring-buffer & MQTT TLS client
├── hardware_design/
│   ├── bill_of_materials.md                                       # Complete procurement schedule with exact MPNs
│   ├── circuit_connections_and_pcb_guide.md                      # Pin-to-pin wiring netlist and isolation plane guide
│   ├── pcb_layout_guidelines.md                                   # Kelvin source routing, IEC 60664, 200 kV/μs CMTI rules
│   └── snubber_and_thermal_calculations.md                        # Inductive loop math, MOV joule ratings & heatsink sizing
├── datasheets/                                                    # Official Manufacturer Datasheets (11 Component PDFs)
│   ├── acs758lcb_050b.pdf                                         # Allegro 50A Galvanically Isolated Current Sensor
│   ├── amc1301.pdf                                                # TI 5kVrms Precision Reinforced Isolated Amplifier
│   ├── bta16.pdf                                                  # STMicroelectronics 16A Snubberless TRIAC
│   ├── esp32_wroom_32e.pdf                                        # Espressif Dual-Core Wi-Fi & BLE Module
│   ├── meanwell_irm05.pdf                                         # Mean Well 5W 5V Miniature AC-DC Converter
│   ├── moc3041m.pdf                                               # onsemi 400V Zero-Crossing Phototriac Driver
│   ├── mornsun_b0515s.pdf                                         # Mornsun 1W 5V-to-15V Floating Gate DC-DC
│   ├── si8274ab.pdf                                               # Skyworks High-Speed Isolated Dual Gate Driver
│   ├── stm32g431kb.pdf                                            # STMicroelectronics Arm Cortex-M4F Mixed-Signal MCU
│   ├── ultramov_v275la40ap.pdf                                    # Littelfuse 150J Metal Oxide Varistor (MOV)
│   └── wolfspeed_c3m0075120k.pdf                                  # Wolfspeed 1200V 75mΩ SiC Power MOSFET
├── figures/                                                       # Architecture Schematics, Pinouts & Hardware Photographs
├── LICENSE                                                        # MIT Open-Source License
└── README.md                                                      # Project Overview & System Documentation
```

---

## 6. Sourcing & PCB Design

All physical CAD assets, footprints, and symbols have been specified to allow designers to select their preferred manufacturing packages (e.g., standard Through-Hole THT vs. High-Density SMD), create custom libraries, and import vendor models directly from distributors (SnapEDA, Ultra Librarian, SamacSys, Mouser, DigiKey).

Refer to:
* [`hardware_design/bill_of_materials.md`](hardware_design/bill_of_materials.md) for the complete BOM with exact MPNs and vendor links.
* [`hardware_design/circuit_connections_and_pcb_guide.md`](hardware_design/circuit_connections_and_pcb_guide.md) for pin-to-pin interconnection and netlist reference.
* [`hardware_design/pcb_layout_guidelines.md`](hardware_design/pcb_layout_guidelines.md) for creepage, clearance, and Kelvin source routing rules.

---

## 7. License

This project is licensed under the [MIT License](LICENSE).
