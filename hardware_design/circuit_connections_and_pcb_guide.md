# Complete Circuit Interconnections and PCB Routing Guide
## Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker (SSCB)
### Operating Ratings: 230 V~RMS~ / 16 A~RMS~ (Peak Fault Interruption: 50–80 A, <650 ns)

---

## 1. Ground Domain & Isolation Partitioning

To guarantee stability under $>100\,\text{V/ns}$ slew rates and maintain $>4\,\text{kV}$ isolation, the PCB layout is strictly partitioned into **five galvanically isolated ground planes**:

```
+-----------------------------------------------------------------------------------------------+
|                                      PCB GROUND ARCHITECTURE                                  |
+-----------------------------------------------------------------------------------------------+
| 1. AC_NEUTRAL_IN   : High-current AC return path (mains reference before breaker)             |
| 2. KS_PHASE_GND    : Floating Kelvin Source plane for Phase SiC Pair (Q1/Q2 Pin 3)            |
| 3. KS_NEUTRAL_GND  : Floating Kelvin Source plane for Neutral SiC Pair (Q3/Q4 Pin 3)          |
| 4. MCU_GND (0V)    : Low-voltage control plane (STM32, AMC1301 secondary, ACS758 secondary)  |
| 5. ESP_GND (0V)    : Isolated IoT plane (ESP32, Wi-Fi antenna area, separated by optocouplers)|
+-----------------------------------------------------------------------------------------------+
```

---

## 2. Complete Pin-to-Pin Interconnection Schedule

### 2.1 Power Stage (Phase & Neutral Paths)

#### **Phase Conductor Path (230 V AC, 16 A continuous)**
1. **TB1 Pin 1 (`LINE_IN`)** $\to$ Connects to **ACS758 CS1 Pin 1 & Pin 2** (Current Sensor Primary Input).
2. **ACS758 CS1 Pin 3** $\to$ Connects to **Q1 Pin 1 (`DRAIN`)**.
3. **Q1 Pin 2 (`SOURCE_PWR`)** $\to$ Connects to **Q2 Pin 2 (`SOURCE_PWR`)** via a massive $70\,\mu\text{m}$ (2 oz) top-layer copper polygon plane (`PHASE_COMMON_SOURCE`).
4. **Q2 Pin 1 (`DRAIN`)** $\to$ Connects to:
   * **TB2 Pin 1 (`LINE_OUT`)** (Load Phase terminal).
   * **MOV1 Terminal 1** (Littelfuse UltraMOV Phase leg).
   * **Snubber C_snub1 Terminal 1** (through R_snub1 $47\,\Omega / 5\,\text{W}$).
   * **TRIAC TR1 Terminal MT2** (Dynamic PFC Phase terminal).
   * **HV Sensing Resistor R_div1 Terminal 1** (High-voltage AC divider input).

#### **Neutral Conductor Path (AC Return, 16 A continuous)**
1. **TB1 Pin 2 (`NEUTRAL_IN`)** $\to$ Connects to **ACS758 CS2 Pin 1 & Pin 2** (Current Sensor Primary Input).
2. **ACS758 CS2 Pin 3** $\to$ Connects to **Q3 Pin 1 (`DRAIN`)**.
3. **Q3 Pin 2 (`SOURCE_PWR`)** $\to$ Connects to **Q4 Pin 2 (`SOURCE_PWR`)** via a heavy copper polygon (`NEUTRAL_COMMON_SOURCE`).
4. **Q4 Pin 1 (`DRAIN`)** $\to$ Connects to:
   * **TB2 Pin 2 (`NEUTRAL_OUT`)** (Load Neutral terminal).
   * **MOV1 Terminal 2** (Littelfuse UltraMOV Neutral leg).
   * **Snubber C_snub2 Terminal 1** (through R_snub2 $47\,\Omega / 5\,\text{W}$).
   * **PFC Film Capacitor C_pfc Terminal 2** (Return for dynamic PFC).
   * **HV Sensing Divider Low-Side Reference**.

---

### 2.2 Gate Drive Subsystem (Silicon Labs Si8274 & Mornsun B0515S)

#### **Phase Gate Driver (U1: SI8274AB1-IS1)**
* **Primary Side (Referenced to `MCU_GND`):**
  * Pin 1 (`VDDI`): Connect to `+3.3V_MCU` (with 100 nF ceramic decoupling cap to `MCU_GND`).
  * Pin 2 (`INA`): Connect to STM32 `TIM1_CH1` (PA8) through $100\,\Omega$ damping resistor.
  * Pin 3 (`INB`): Connect to STM32 `TIM1_CH1` (PA8) — *both channels switch synchronously*.
  * Pin 4 (`EN`): Connect to STM32 `GATE_EN` (PB0) — active-high enable.
  * Pin 8 (`GNDI`): Connect to `MCU_GND`.
* **Secondary Side Channel A (Driving Q1):**
  * Pin 16 (`VDDA`): Connect to Isolated Supply PS1 Pin 4 (`+15V_ISO1`).
  * Pin 15 (`VOA`): Connect to gate network:
    * In series with **Rg1_on ($10\,\Omega$)** to **Q1 Pin 4 (`GATE`)**.
    * In parallel with **Rg1_off ($4.7\,\Omega$)** in series with **BAT54 Diode D1** (Cathode facing VOA) to Q1 Pin 4.
    * 18V Zener **ZD1** connected between Q1 Pin 4 (`GATE`) and Q1 Pin 3 (`KELVIN_SOURCE`).
  * Pin 14 (`GNDA`): Connect directly to **Q1 Pin 3 (`KELVIN_SOURCE`)** and PS1 Pin 3 (`0V_ISO1`).
* **Secondary Side Channel B (Driving Q2):**
  * Pin 11 (`VDDB`): Connect to Isolated Supply PS2 Pin 4 (`+15V_ISO2`).
  * Pin 10 (`VOB`): Connect to gate network:
    * In series with **Rg2_on ($10\,\Omega$)** to **Q2 Pin 4 (`GATE`)**.
    * In parallel with **Rg2_off ($4.7\,\Omega$)** + **BAT54 Diode D2** to Q2 Pin 4.
    * 18V Zener **ZD2** between Q2 Pin 4 and Q2 Pin 3.
  * Pin 9 (`GNDB`): Connect directly to **Q2 Pin 3 (`KELVIN_SOURCE`)** and PS2 Pin 3 (`0V_ISO2`).

#### **Neutral Gate Driver (U2: SI8274AB1-IS1)**
* **Primary Side (Referenced to `MCU_GND`):**
  * Pin 1 (`VDDI`): Connect to `+3.3V_MCU`.
  * Pin 2 (`INA`), Pin 3 (`INB`): Connect to STM32 `TIM1_CH2` (PA9) through $100\,\Omega$.
  * Pin 4 (`EN`): Connect to STM32 `GATE_EN` (PB0).
  * Pin 8 (`GNDI`): Connect to `MCU_GND`.
* **Secondary Side Channels A & B (Driving Q3 & Q4):**
  * Channels A and B replicate the exact isolated gate drive structure above, connecting to Isolated Supplies PS3 and PS4, and driving **Q3 Pin 4/Pin 3** and **Q4 Pin 4/Pin 3**.

---

### 2.3 Sensing Subsystem (ACS758 Current & AMC1301 Voltage)

#### **1. Phase Hall Current Sensor (CS1: Allegro ACS758LCB-050B)**
* Pin 1 & Pin 2: AC Mains Phase In (`LINE_IN` busbar pad).
* Pin 3: AC Mains Phase to SiC Drain (`LINE_SIC` busbar pad).
* Pin 4 (`VCC`): Connect to `+5V_ANALOG` (filtered with 100 nF + 10 µF).
* Pin 5 (`GND`): Connect to `MCU_GND`.
* Pin 6 (`VIOUT`): Output ($40\,\text{mV/A}$, $2.5\,\text{V}$ quiescent).
  * Route through RC antialiasing filter ($100\,\Omega$ + $1\,\text{nF}$ COG ceramic) to **STM32 Pin PA0 (`ADC1_IN1`)**.

#### **2. Neutral Hall Current Sensor (CS2: Allegro ACS758LCB-050B)**
* Pin 1 & Pin 2: AC Mains Neutral In (`NEUTRAL_IN`).
* Pin 3: AC Mains Neutral to SiC Drain (`NEUTRAL_SIC`).
* Pin 4 (`VCC`), Pin 5 (`GND`): `+5V_ANALOG`, `MCU_GND`.
* Pin 6 (`VIOUT`): Connect through RC filter ($100\,\Omega$ + $1\,\text{nF}$) to **STM32 Pin PA1 (`ADC2_IN1`)**.

#### **3. Isolated Voltage Sensor (U3: Texas Instruments AMC1301DWVR)**
* **High-Voltage Side (Referenced to Mains):**
  * Mains Phase Voltage passes through three series $400\,\text{k}\Omega / 2512$ resistors (R_div1, R_div2, R_div3 = $1.2\,\text{M}\Omega$ total) into low-side resistor R_div_lo ($750\,\Omega$ to Neutral).
  * Pin 1 (`VDD1`): Powered from floating 5V supply (or derived rail).
  * Pin 2 (`VINP`): Top of $750\,\Omega$ divider resistor ($\pm 250\,\text{mV}$ peak at $325\,\text{V}_{\text{peak}}$ mains).
  * Pin 3 (`VINN`): Bottom of $750\,\Omega$ divider (Mains Neutral reference).
  * Pin 4 (`GND1`): Mains Neutral reference.
* **Isolated Low-Voltage Side (Referenced to `MCU_GND`):**
  * Pin 5 (`GND2`): Connect to `MCU_GND`.
  * Pin 6 (`VOUTN`): Inverted differential output to OPA2340 buffer Pin 6.
  * Pin 7 (`VOUTP`): Non-inverted differential output to OPA2340 buffer Pin 5.
  * Pin 8 (`VDD2`): Connect to `+5V_ANALOG`.
* **Buffer Amplifier (U4: OPA2340 Dual Op-Amp):**
  * Configured as a differential-to-single-ended converter with $1.65\,\text{V}$ common-mode offset.
  * Output connects to **STM32 Pin PA2 (`ADC1_IN2`)**.

---

### 2.4 Dynamic PFC & Zero-Crossing TRIAC Subsystem

1. **Control Signal:** STM32 Pin **PB4 (`PFC_CTRL`)** connects through a $330\,\Omega$ resistor to **MOC3041 Optotriac Pin 1 (Anode)**.
2. **MOC3041 Pin 2 (Cathode):** Connects to `MCU_GND`.
3. **MOC3041 Pin 4:** Connects through $330\,\Omega / 1\,\text{W}$ resistor (R_triac_gate) to **BTA16 TRIAC Pin 1 (MT1)**.
4. **MOC3041 Pin 6:** Connects to **BTA16 TRIAC Pin 3 (GATE)**.
5. **BTA16 TRIAC Pin 2 (MT2):** Connects to **Phase Output (`LINE_OUT`)**.
6. **BTA16 TRIAC Pin 1 (MT1):** Connects to **PFC Capacitor C_pfc Terminal 1** ($15\,\mu\text{F} / 450\,\text{VAC}$).
7. **PFC Capacitor C_pfc Terminal 2:** Connects to **Neutral Output (`NEUTRAL_OUT`)**.

---

### 2.5 Opto-Isolated Telemetry Subsystem (STM32 $\to$ ESP32)

* **Telemetry TX (STM32 to ESP32):**
  * STM32 Pin **PA3 (`USART2_TX`)** $\to$ $330\,\Omega$ resistor $\to$ **OC1 (PC817) Pin 1 (Anode)**.
  * OC1 Pin 2 (Cathode) $\to$ Connect to `MCU_GND`.
  * OC1 Pin 3 (Emitter) $\to$ Connect to `ESP_GND`.
  * OC1 Pin 4 (Collector) $\to$ Connect to **ESP32 GPIO 16 (`RXD2`)** pulled up to `+3.3V_ESP` with $4.7\,\text{k}\Omega$.
* **Command RX (ESP32 to STM32):**
  * ESP32 Pin **GPIO 17 (`TXD2`)** $\to$ $330\,\Omega$ resistor $\to$ **OC2 (PC817) Pin 1 (Anode)**.
  * OC2 Pin 2 (Cathode) $\to$ Connect to `ESP_GND`.
  * OC2 Pin 3 (Emitter) $\to$ Connect to `MCU_GND`.
  * OC2 Pin 4 (Collector) $\to$ Connect to **STM32 Pin PA4 (`USART2_RX`)** pulled up to `+3.3V_MCU` with $4.7\,\text{k}\Omega$.

---

### 2.6 Auxiliary Power Supply Architecture

1. **Mains AC Input:** Connected to Mean Well **IRM-05-5** AC-DC converter module through a $500\,\text{mA}$ ceramic fuse (`FUSE1`).
2. **IRM-05-5 5V Output:** Provides primary `+5V_RAW` rail ($3\,\text{kV}$ isolation from mains).
3. **Linear Regulators:**
   * **REG1 (AZ1117-3.3):** Converts `+5V_RAW` to **`+3.3V_MCU`** for STM32, AMC1301 secondary, and Si8274 primary.
   * **REG2 (AZ1117-3.3):** Separately converts `+5V_RAW` across the opto barrier to **`+3.3V_ESP`** for the ESP32 module.
4. **Gate Driver Floating DC-DC Converters:**
   * 4× Mornsun **QA01D** or **B0515S-1WR3** (with Zener split) inputs connect to `+5V_RAW` and `MCU_GND`.
   * Outputs provide four galvanically isolated $+15\,\text{V} / -3.5\,\text{V}$ asymmetric rails (`ISO1` through `ISO4`) referenced to their respective Kelvin Source pins to prevent $dv/dt$ Miller parasitic re-turn-on.

---

### 2.7 User Interface & Physical Pushbuttons (Relocated to Port B)

To guarantee that analog inputs `PA0` and `PA1` are never shunted by digital switch lines, physical controls are dedicated to Port B:
1. **Manual Trip Button (`BTN_TRIP`):** Connected between STM32 Pin **PB1** and `MCU_GND` (Internal Pull-Up enabled). Pressing pulls PB1 LOW to initiate instantaneous hardware trip.
2. **Manual Reset Button (`BTN_RESET`):** Connected between STM32 Pin **PB6** and `MCU_GND` (Internal Pull-Up enabled). Pressing pulls PB6 LOW to clear latched faults.
3. **Status OK LED (Green):** STM32 Pin **PB3** $\to$ $470\,\Omega$ current-limiting resistor $\to$ Green LED Anode $\to$ Cathode to `MCU_GND`.
4. **Fault Alarm LED (Red):** STM32 Pin **PB5** $\to$ $470\,\Omega$ current-limiting resistor $\to$ Red LED Anode $\to$ Cathode to `MCU_GND`.

---

## 3. PCB Layout & Routing Directives (Summary Checklist)

| Parameter | Required Specification | Design Enforcement |
| :--- | :--- | :--- |
| **High-Current Traces** | $16\,\text{A}_{\text{RMS}}$ continuous ($50\text{--}80\,\text{A}$ surge) | Minimum width: $6.5\,\text{mm}$ on $70\,\mu\text{m}$ ($2\,\text{oz}$) outer copper with solid polygon pours. |
| **Kelvin Source Pair** | Pin 3 (`KS`) and Pin 4 (`GATE`) differential pair | Trace width: $0.35\,\text{mm}$, spacing: $0.20\,\text{mm}$, loop area $A_{\text{loop}} < 15\,\text{mm}^2$. |
| **High-Voltage Creepage** | $>5.08\,\text{mm}$ (conforming to IEC 60664-1) | Milled insulation isolation slots ($1.5\,\text{mm}$ wide) between Drain and Source pads. |
| **Isolation Barrier** | $4000\,\text{V}_{\text{impulse}}$ reinforced isolation | $8.0\,\text{mm}$ keepout void under AMC1301, Si8274, and PC817 optocouplers (no copper on any layer). |
| **Decoupling Capacitors** | Low ESR X7R ceramic capacitors | Placed within $3.0\,\text{mm}$ of every IC $V_{\text{DD}}$ pin directly referenced to local ground. |
