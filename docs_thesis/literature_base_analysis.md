# Comprehensive Literature Base & Benchmark Analysis
## Foundations for the Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker

This document provides an exhaustive, comparative analysis of the four primary peer-reviewed base papers informing the topology, protection physics, snubber mathematics, and gate-drive design of this project.

---

## 1. Paper Overview & Structural Mapping

```
+---------------------------------------------------------------------------------------------------+
|                                  THEORETICAL & LITERATURE BASE                                    |
+---------------------------------------------------------------------------------------------------+
                                                  |
       +--------------------+---------------------+---------------------+--------------------+
       |                    |                     |                     |                    |
       v                    v                     v                     v                    v
  [ Wang et al. 2016 ]  [ Li et al. 2019 ]   [ Qin et al. 2019 ]   [ Hyon et al. 2024 ]  [ Our SSCB ]
  IEEE TPEL             Micromachines (MDPI) Electronics (MDPI)    IEEE Access           Double-Pole AC
  --------------------  -------------------  -------------------   -------------------   --------------
  SiC SCWT limits &     400V prototype,      Kelvin source L_AM    Wolfspeed C3M         230V AC RMS,
  electro-thermal       720ns clearing,      damping, DESAT gate   bidirectional pair,   sub-microsecond
  leakage breakdown     dual MOV math        clamp & soft-off      parallel thyristor    AWD + IoT MQTT
```

---

## 2. In-Depth Analysis of Base Papers

### 2.1 Paper 1: Li et al. (Micromachines 2019)
* **Title:** *Design of 400 V Miniature DC Solid State Circuit Breaker with SiC MOSFET*
* **Key Findings:**
  * Demonstrated a physical 400 V DC miniature breaker utilizing a 1200 V / 40 A ROHM SiC MOSFET (SCT2080KE).
  * Validated that SiC achieves a total short-circuit clearing time of **720 ns** at an 80 A peak fault current.
  * Compared with a Silicon IGBT breaker (TX-DA962D6 driver), which took $2.8\,\mu\text{s}$ and reached $300\,\text{A}$ ($4\times$ higher current and $4\times$ longer duration).
* **Mathematical Derivations Adopted in Our System:**
  * Derivation of energy transfer from line inductance $L$ into the Metal Oxide Varistor ($W_{\text{MOV}}$):
    $$i_L(t) = I_{\text{max}} - \left( \frac{V_{\text{MOV}} - V_{\text{DC}}}{L} \right) t$$
    $$t_s = \frac{I_{\text{max}} L}{V_{\text{MOV}} - V_{\text{DC}}}$$
    $$W_{\text{MOV}} = \int_0^{t_s} V_{\text{MOV}} \cdot i_L(t) dt = \frac{1}{2} \left( \frac{V_{\text{DC}}}{V_{\text{MOV}} - V_{\text{DC}}} \right) L I_{\text{max}}^2$$
  * Established the dual MOV coordination principle: internal fast snubber MOV (`05D681K`, breakdown $680\,\text{V}$) absorbs the initial inductive spike, and external energy MOV (`14D511K`, breakdown $510\,\text{V}$) absorbs total trapped line energy.

---

### 2.2 Paper 2: Qin et al. (Electronics 2019)
* **Title:** *A Digital-Controlled SiC-Based Solid State Circuit Breaker with Soft Switch-Off Method for DC Power System*
* **Key Findings:**
  * Investigated the internal electric field distribution and gate oxide breakdown under Hard-Switching Fault (HSF) conditions.
  * Disclosed that the breakdown electric field of $\text{SiO}_2$ is $10\,\text{MV/cm}$, while the SiC semiconductor field reaches $4\,\text{MV/cm}$. Because $\frac{\epsilon_{\text{semi}}}{\epsilon_{\text{oxide}}} \approx 2.5$, the gate oxide experiences extreme electrical stress during faults.
  * Identified the parasitic inductance $L_{AM}$ between the auxiliary Kelvin source and the main power source as a primary generator of gate-source overvoltage ($V_{GS} > 20\,\text{V}$).
  * Proposed an active soft switch-off circuit and gate clamping diode/Zener to hold $V_{GS}$ below $20\,\text{V}$ and reduce drain-source overshoot from $358\,\text{V}$ to $310\,\text{V}$.
* **Direct Influence on Our Design:**
  * Formulates the rules for our [`pcb_layout_guidelines.md`](file:///c:/Users/rajas/antigravity-work/solid%20state%20breaker/hardware_design/pcb_layout_guidelines.md), strictly enforcing Kelvin source trace separation to decouple $L_{AM}$ from the gate drive loop.

---

### 2.3 Paper 3: Hyon et al. (IEEE Access 2024)
* **Title:** *Design of a Solid State Circuit Breaker Using a SiC MOSFET for LVDC Applications*
* **Key Findings:**
  * Implemented a bidirectional breaker for $380\,\text{V}$ DC microgrids using discrete **Wolfspeed C3M0016120D ($1.2\,\text{kV}, 16\,\text{m}\Omega$)** devices in common-source back-to-back orientation.
  * Introduced an active parallel bypass circuit: an R-C-D snubber path ($R_3\text{-}C_3\text{-}D_3$) assisted by an active Thyristor ($T_1$) branch ($R_1\text{-}C_1\text{-}T_1$).
  * Experimental results showed that firing the parallel thyristor reduced peak $V_{DS}$ spike from $917\,\text{V}$ to $775\,\text{V}$ at $120\,\text{A}$ fault current.
  * Derived the second-order RLC differential equations governing capacitor energy storage:
    $$V_{C3}(t) = V_{\text{dc}} + e^{-\alpha t} (D_1 \cos\omega_d t + D_2 \sin\omega_d t)$$
    $$C_3 \ge L_{\text{lo}} \left( \frac{k I_{\text{flt}}}{V_{\text{spk}} - V_{\text{dc}}} \right)^2$$
* **Direct Influence on Our Design:**
  * Validates the exact device family (Wolfspeed 1200V C3M SiC) and common-source bidirectional pairing.
  * Corroborates our project's parallel TRIAC/capacitor stage, which performs both dynamic power factor correction during normal operation and active energy bypass during faults.

---

### 2.4 Paper 4: Wang et al. (IEEE Transactions on Power Electronics 2016)
* **Title:** *Temperature Dependent Short Circuit Capability of Silicon Carbide (SiC) Power MOSFETs*
* **Key Findings:**
  * Landmark study establishing the thermal and electrical failure boundaries of 1200 V SiC MOSFETs across case temperatures from $25^\circ\text{C}$ to $200^\circ\text{C}$ and DC bus voltages up to $750\,\text{V}$.
  * Proved that while Silicon IGBTs can endure short circuits for $>10\,\mu\text{s}$, commercial SiC MOSFETs endure:
    * At $V_{\text{DC}} = 600\,\text{V}, T_c = 25^\circ\text{C}$: $\text{SCWT} \approx 11\text{--}12\,\mu\text{s}$.
    * At $V_{\text{DC}} = 750\,\text{V}, T_c = 200^\circ\text{C}$: **$\text{SCWT} \le 5.0\text{--}6.6\,\mu\text{s}$**.
  * Formulated the 1D electro-thermal heat diffusion model:
    $$\frac{\partial}{\partial x} \left( k_p \frac{\partial T}{\partial x} \right) + Q = \rho c_p \frac{\partial T}{\partial t}$$
    with temperature-dependent thermal conductivity $k_p(T) = \frac{1}{-0.0003 + 1.05 \times 10^{-5} T}$.
  * Modeled the dominant leakage current: Shockley-Read-Hall (SRH) thermal generation current ($I_{g,\text{th}}$):
    $$I_{g,\text{th}} = \frac{q S n_i}{\tau_g} \sqrt{\frac{2 \epsilon_s}{q} \left( \frac{N_d + N_a}{N_d N_a} \right) V_{\text{DC}}}$$
    with intrinsic carrier density $n_i(T) = 1.7 \times 10^{16} T^{3/2} e^{-\frac{2.08 \times 10^4}{T}}$.
  * **Crucial Takeaway:** Heat flux diffuses less than $200\,\mu\text{m}$ during a short circuit. External heatsinks and thermal packaging cannot assist during the initial $1\text{--}5\,\mu\text{s}$ fault window. Therefore, **autonomous hardware clearing $<1\,\mu\text{s}$ is mandatory to prevent thermal runaway and permanent destruction**.

---

## 3. Comprehensive Feature & Performance Comparison

| Feature / Metric | Qin et al. (2019) | Li et al. (2019) | Hyon et al. (2024) | Wang et al. (2016) | **Proposed System (This Project)** |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Circuit Topology** | Unidirectional DC | Unidirectional DC | Bidirectional DC | Destructive Test Circuit | **Bidirectional Double-Pole AC (Phase & Neutral)** |
| **Operating Voltage** | 270 V DC | 400 V DC | 380 V DC | 400–750 V DC | **230 V~RMS~ (50 Hz AC Grid)** |
| **Rated Current** | 100 A | 10 A | 10 A (120 A fault) | 20–28 A rated | **16 A Continuous (40 A AWD Peak Trip)** |
| **Semiconductor Devices** | 1200V ROHM SCH2080KE | 1200V ROHM SCT2080KE | **1200V Wolfspeed C3M0016120D** | 1200V Cree 1G/2G & ROHM | **4× 1200V Wolfspeed C3M0075120K** |
| **Fault Clearing Latency** | $\approx 2\,\mu\text{s}$ | **720 ns** | $\approx 2\text{--}3\,\mu\text{s}$ | Fails in $\le 5\,\mu\text{s}$ | **$<515\text{--}650\,\text{ns}$ (Autonomous Hardware AWD)** |
| **Control / Trip Engine** | DSP (TMS320F2812) + DESAT | Op-Amp / SR Latch | Desaturation Driver | External Pulse Generator | **STM32G431KB Analog Watchdog + TIM1 Break** |
| **Overvoltage Snubber** | $L_{AM}$ source damping | Dual MOV (14D511K + 05D681K) | RCD + Parallel Thyristor | Clamped inductive | **Littelfuse UltraMOV V275LA40AP + RC Snubber** |
| **Power Quality Correction** | None | None | None | None | **Dynamic Zero-Crossing TRIAC Capacitor PFC Stage** |
| **IoT & Telemetry** | None | None | None | None | **Isolated Opto-UART (2× PC817) + ESP32 MQTT** |
