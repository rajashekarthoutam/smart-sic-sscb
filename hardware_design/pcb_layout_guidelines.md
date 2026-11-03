# PCB Layout & Hardware Engineering Guidelines
## Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker (SSCB)

---

## 1. Executive Summary & Design Constraints
The Double-Pole Solid-State Circuit Breaker utilizes four **Wolfspeed C3M0075120K** ($1200\,\text{V}, 75\,\text{m}\Omega$) SiC MOSFETs in a back-to-back common-source topology. Switching transitions during short-circuit clearing exhibit:
* $di/dt > 500\,\text{A}/\mu\text{s}$
* $dv/dt > 100\,\text{V}/\text{ns}$
* Peak interrupted fault currents up to $50\text{--}80\,\text{A}$ within $<1\,\mu\text{s}$

Under these extreme slew rates, parasitic loop inductances and capacitive coupling across isolation barriers can induce destructive gate-source overvoltage, spurious dv/dt re-triggering, or logic ground bounce. This document outlines mandatory PCB layout constraints conforming to **IEC 60664-1** and **IPC-2221B**.

---

## 2. Kelvin Source Separation Architecture

### 2.1 The Physics of Gate-Source Degeneration
In standard 3-pin TO-247 packages, the internal bond-wire and lead inductance ($L_S \approx 5\text{--}10\,\text{nH}$) is shared between the high-current drain-source power path and the low-voltage gate-source drive loop:
$$V_{GS,\text{internal}} = V_{GS,\text{driver}} - L_S \frac{di_D}{dt}$$

During an ultra-fast turn-off event ($di_D/dt = -500\,\text{A}/\mu\text{s}$), a parasitic inductance of only $5\,\text{nH}$ produces:
$$V_{LS} = - (5 \times 10^{-9}\,\text{H}) \times (-500 \times 10^6\,\text{A/s}) = +2.5\,\text{V}$$
This induced positive voltage opposes the driver turn-off signal, dangerously delaying channel extinction and keeping the SiC MOSFET in active linear conduction during the peak of an inductive short circuit.

### 2.2 Wolfspeed C3M0075120K (TO-247-4) Pinout
```
  +---------------------------------------+
  |              C3M0075120K              |
  |                TO-247-4               |
  +---------------------------------------+
      |           |           |        |
    Pin 1       Pin 2       Pin 3    Pin 4
   (Drain)     (Power      (Kelvin   (Gate)
               Source)     Source)
```

### 2.3 Routing Rules for Kelvin Source (Pin 3)
1. **Zero Load Current Rule:** The Kelvin Source (Pin 3) must connect exclusively to the isolated gate driver reference pin (Si8274 `GNDB` / `GNDA`) and the local decoupling capacitor ground terminal. Under no circumstances may load current return through this trace.
2. **Kelvin-Gate Differential Pair:** The Gate trace (Pin 4) and Kelvin Source trace (Pin 3) must be routed as a tightly coupled differential pair:
   * Trace width: $0.35\,\text{mm}$ (14 mil).
   * Spacing: $0.20\,\text{mm}$ (8 mil).
   * Gate loop area must be strictly minimized: $A_{\text{loop}} < 15\,\text{mm}^2$.
3. **Power Source Connection (Pin 2):** Pin 2 must tie directly to the common-source mid-plane connecting $Q_1$ and $Q_2$ using heavy top-layer copper ($70\,\mu\text{m} / 2\,\text{oz}$) with thermal relief vias to inner copper planes.

```
       [ Si8274 Gate Driver ]
         VO_A            GNDA
          |               |
       (Rg_on/off)        |  <-- Kelvin Return Trace (<15 mm2 loop)
          |               |
          v               v
        Pin 4           Pin 3
        (Gate)      (Kelvin Source)
      +---------------------------------+
      |      Wolfspeed C3M0075120K      |
      +---------------------------------+
         Pin 1              Pin 2
        (Drain)         (Power Source)
          |                   |
    [ AC Load Out ]    [ Common Source Bus ]
```

---

## 3. High-Voltage Creepage & Clearance (IEC 60664-1)

### 3.1 Voltage Breakdown Ratings
* **Mains Nominal:** $230\,\text{V}_{\text{RMS}} \pm 10\%$ ($325\,\text{V}_{\text{peak}}$ nominal, $358\,\text{V}_{\text{peak}}$ maximum).
* **SiC Breakdown Limit:** $1200\,\text{V}_{\text{DC}}$.
* **Transient Overvoltage Category:** Category III ($4000\,\text{V}$ impulse withstand).
* **Pollution Degree:** 2 (non-conductive pollution with occasional condensation).
* **Material Group:** IIIa ($175 \le \text{CTI} < 400$, standard FR-4).

### 3.2 Spacing Specifications
| Boundary Description | Working Peak Voltage | Required Clearance (Air) | Required Creepage (FR-4 Surface) | Implemented PCB Geometry |
| :--- | :--- | :--- | :--- | :--- |
| **Phase to Neutral (AC Mains)** | $400\,\text{V}_{\text{peak}}$ | $3.0\,\text{mm}$ | $5.0\,\text{mm}$ | **$6.5\,\text{mm}$** |
| **Across Open SSCB Contacts (Drain-Drain)** | $1200\,\text{V}_{\text{peak}}$ | $5.5\,\text{mm}$ | $10.0\,\text{mm}$ | **$10.5\,\text{mm}$ + Milled Slot** |
| **High Voltage AC to STM32 Logic GND** | $4000\,\text{V}_{\text{impulse}}$ | $6.0\,\text{mm}$ | $8.0\,\text{mm}$ | **$8.5\,\text{mm}$ Reinforced Barrier** |
| **Gate Driver Input to Output (Si8274)** | $2500\,\text{V}_{\text{RMS}}$ | $4.5\,\text{mm}$ | $6.5\,\text{mm}$ | **$7.0\,\text{mm}$ + Isolation Slot** |

### 3.3 Isolation Milling Slots
Where creepage distances cannot be met due to compact IC packages (e.g., SOIC-8 wide body or SOIC-16), physical air slots must be routed through the PCB:
* **Slot Width:** $\ge 1.5\,\text{mm}$ (60 mil).
* **Plated Finish:** Non-plated edge slot.
* **Locations:**
  * Beneath the isolation barrier of **Si8274** gate drivers.
  * Beneath the isolation barrier of **AMC1301** differential voltage amplifiers.
  * Beneath the isolation barrier of **PC817** optocouplers.
  * Beneath the isolation gap of the **B0515S-1W** floating DC-DC modules.

---

## 4. Common-Mode Transient Immunity (CMTI) & Ground Plane Separation

### 4.1 Displacement Current Physics
The Si8274 features a CMTI rating of $>200\,\text{V/ns}$. However, parasitic capacitance $C_{\text{barrier}}$ across the isolation barrier or floating power supplies allows high-frequency common-mode current to inject directly into the control ground:
$$i_{\text{CM}} = C_{\text{barrier}} \frac{dv}{dt}$$
At $dv/dt = 100\,\text{V/ns}$:
* If $C_{\text{barrier}} = 10\,\text{pF} \implies i_{\text{CM}} = (10 \times 10^{-12}\,\text{F}) \times (100 \times 10^{9}\,\text{V/s}) = 1.0\,\text{A}$ peak!

This $1\,\text{A}$ transient current flowing across the logic ground plane will corrupt the sensitive ADC reference ($V_{\text{REF}} = 3.3\,\text{V}$), leading to false Analog Watchdog trips or microcontroller resets.

### 4.2 Multi-Domain Ground Isolation Architecture
The PCB contains **four completely galvanically isolated ground planes**:
1. **`GND_LOGIC` (Primary Ground):** Low-noise reference plane for STM32G431KB, $+3.3\,\text{V}$ LDO, and input side of Si8274 / AMC1301 / PC817.
2. **`GND_ESP32` (IoT Telemetry Ground):** Dedicated ground plane for ESP32 and Wi-Fi antenna return, isolated from STM32 by PC817 optocouplers to prevent 2.4GHz RF hash from entering the precision ADC.
3. **`GND_GATE_PHASE` ($S_{12}$):** Floating high-voltage reference tied exclusively to Kelvin Source of $Q_1 / Q_2$.
4. **`GND_GATE_NEUTRAL` ($S_{34}$):** Floating high-voltage reference tied exclusively to Kelvin Source of $Q_3 / Q_4$.

```
+-------------------------------------------------------------------------+
|                              PCB SUBSTRATE                              |
|                                                                         |
|  [ GND_LOGIC ]             [ GND_GATE_PHASE ]     [ GND_GATE_NEUT ]     |
|  STM32G431KB Micro         Floating Q1/Q2 Ref      Floating Q3/Q4 Ref   |
|  3.3V Low-Noise            (Source S12)           (Source S34)          |
|        |                          |                      |              |
|        +---[ Milled Slot ]--------+----[ Milled Slot ]---+              |
|        |                          |                      |              |
|  [ GND_ESP32 ]                    |                      |              |
|  ESP32 IoT Node                   |                      |              |
|  Wi-Fi / MQTT                     +----------------------+              |
+-------------------------------------------------------------------------+
```

### 4.3 Floating DC-DC Layout (B0515S-1W)
* Remove all inner copper planes directly beneath the B0515S-1W converters to eliminate capacitive plane-to-plane coupling.
* Keep total isolation capacitance across each supply below $2.5\,\text{pF}$.
* Place a $10\,\mu\text{F}$ X7R ceramic capacitor directly across the $15\,\text{V}$ output pins, paralleled with a $100\,\text{nF}$ high-frequency decoupling capacitor located within $3\,\text{mm}$ of the Si8274 $V_{\text{DDB}}$ pin.

---

## 5. Gate Drive Loop Optimization

### 5.1 Asymmetric Gate Resistors
SiC MOSFETs exhibit extremely low input capacitance ($C_{\text{iss}} \approx 650\,\text{pF}$ for C3M0075120K) and low threshold voltage ($V_{\text{th}} \approx 2.1\,\text{V}$ at $25^\circ\text{C}$, dropping to $1.5\,\text{V}$ at $150^\circ\text{C}$). To balance turn-on $dv/dt$ EMI against ultra-fast turn-off clearing:
* **Turn-On Resistor ($R_{G,\text{on}}$):** $10.0\,\Omega$ (Limits turn-on $di/dt$ and mitigates reverse-recovery stress).
* **Turn-Off Resistor ($R_{G,\text{off}}$):** $3.3\,\Omega$ in series with a low-capacitance Schottky diode (**BAT54** or **BAS40**) in parallel with $R_{G,\text{on}}$, ensuring gate impedance is held below $4\,\Omega$ during turn-off to clamp Miller capacitance current $C_{\text{gd}} \frac{dv}{dt}$.
* **Ferrite Bead:** Place a high-frequency ferrite bead (Murata **BLM18KG601SN1D**, $600\,\Omega$ @ 100MHz) directly adjacent to the Gate pin to damp VHF LC oscillations.

```
                  +---[ 10.0 Ohm Rg_on ]---+
                  |                        |
  Si8274 Out -----+                        +---> [ Ferrite Bead ] ---> Pin 4 (Gate)
                  |                        |
                  +---|<|---[ 3.3 Ohm ]----+
                    (BAT54)   Rg_off
```

---

## 6. Snubber & MOV Placement Rules
1. **Zero Lead Inductance:** Metal Oxide Varistors (MOVs) protecting the SiC switches must be placed directly across the Drain of $Q_1$ and Drain of $Q_2$ on the Phase pole, and $Q_3$ to $Q_4$ on the Neutral pole.
2. **Copper Pours:** Do not route MOV connections with thin traces. Use polygon copper pours with a minimum width of $5.0\,\text{mm}$ ($200\,\text{mil}$) to handle transient peak fault currents of up to $500\,\text{A}$ without significant $L \frac{di}{dt}$ voltage drop.
3. **Snubber Capacitors:** High-voltage polypropylene film snubber capacitors (e.g., KEMET PHE450, $10\,\text{nF}, 1600\,\text{V}_{\text{DC}}$) must be mounted in parallel with the MOVs within $10\,\text{mm}$ of the SiC drain terminals.
