# Snubber, MOV & Thermal Mathematical Modeling
## Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker (SSCB)

---

## 1. Transient Interruption Dynamics & Energy Storage

### 1.1 The Inductive Commutation Problem
When an AC line is interrupted during a short-circuit fault, the current cannot fall to zero instantaneously due to the line loop inductance ($L_{\text{grid}} + L_{\text{cable}} + L_{\text{PCB}}$). 

According to Faraday's Law, as the SiC switch turns off in $t_{\text{off}} < 1\,\mu\text{s}$, the rate of current decay ($di/dt$) induces an inductive flyback overvoltage:
$$V_{\text{DS,peak}} = V_{\text{AC}}(t) + L_{\text{loop}} \left|\frac{di}{dt}\right|$$

#### Numerical Example (Unclamped):
* Grid peak voltage: $V_{\text{AC}} = 325\,\text{V}$
* Line loop inductance: $L_{\text{loop}} = 15\,\mu\text{H}$ (typical for a $15\text{--}20\,\text{m}$ branch circuit)
* Short-circuit interruption current: $I_{\text{fault}} = 60\,\text{A}$
* SiC turn-off time: $t_{\text{off}} = 500\,\text{ns} \implies \left|\frac{di}{dt}\right| = \frac{60\,\text{A}}{500 \times 10^{-9}\,\text{s}} = 120 \times 10^6\,\text{A/s}$
$$V_{\text{DS,peak}} = 325\,\text{V} + (15 \times 10^{-6}\,\text{H}) \times (120 \times 10^6\,\text{A/s}) = 325\,\text{V} + 1800\,\text{V} = 2125\,\text{V}$$

**Hazard:** Since the Wolfspeed C3M0075120K SiC MOSFET is rated for $V_{\text{DS,max}} = 1200\,\text{V}$, an unclamped breaker will suffer catastrophic avalanche breakdown and physical destruction on its first trip.

---

## 2. MOV Clamping & Energy Dissipation Derivations

### 2.1 MOV Selection Criteria
To protect the $1200\,\text{V}$ SiC switches while avoiding leakage dissipation during normal $230\,\text{V}_{\text{RMS}}$ grid operation:
1. Continuous Maximum Working Voltage: $V_{\text{M(AC)}} > 230\,\text{V} \times 1.15 = 265\,\text{V}_{\text{RMS}}$
2. Clamping Voltage ($V_{\text{clamp}}$) at peak fault current ($100\,\text{A}$): $V_{\text{clamp}} < 0.80 \times V_{\text{DS,max}} = 960\,\text{V}$

#### Selected Component: Littelfuse UltraMOV **V275LA40AP**
* $V_{\text{RMS}}$ rating: $275\,\text{V}$
* $V_{\text{DC}}$ rating: $369\,\text{V}$
* Nominal Varistor Voltage $V_N$ @ 1mA: $430\,\text{V} \pm 10\%$
* Maximum Clamping Voltage $V_C$ @ 100A (8/20 $\mu\text{s}$ pulse): **$710\,\text{V}$**
* Maximum Peak Surge Current (8/20 $\mu\text{s}$): **$6500\,\text{A}$**
* Rated Energy Capacity ($10/1000\,\mu\text{s}$): **$150\,\text{Joules}$**

### 2.2 Energy Absorption Derivation
During fault commutation, the MOV clamps the voltage across the opening switch to $V_{\text{clamp}} = 710\,\text{V}$. The stored magnetic energy in the loop inductance is:
$$E_L = \frac{1}{2} L_{\text{loop}} I_{\text{fault}}^2$$

The total energy absorbed by the MOV ($E_{\text{MOV}}$) includes both the stored inductive energy and the energy delivered by the AC grid source during the current ramp-down:
$$E_{\text{MOV}} = E_L \times \left( \frac{V_{\text{clamp}}}{V_{\text{clamp}} - V_{\text{grid,peak}}} \right)$$

#### Quantitative Verification:
For $L_{\text{loop}} = 20\,\mu\text{H}$, $I_{\text{fault}} = 60\,\text{A}$, $V_{\text{grid,peak}} = 325\,\text{V}$, and $V_{\text{clamp}} = 710\,\text{V}$:
$$E_L = \frac{1}{2} \times (20 \times 10^{-6}\,\text{H}) \times (60\,\text{A})^2 = 0.036\,\text{J} = 36.0\,\text{mJ}$$
$$\text{Source Energy Multiplier} = \frac{710\,\text{V}}{710\,\text{V} - 325\,\text{V}} = \frac{710}{385} \approx 1.844$$
$$E_{\text{MOV}} = 36.0\,\text{mJ} \times 1.844 = 66.38\,\text{mJ} = 0.0664\,\text{J}$$

#### MOV Stress & Safety Margin:
$$\text{Safety Margin} = \frac{E_{\text{rated}}}{E_{\text{MOV}}} = \frac{150\,\text{J}}{0.0664\,\text{J}} \approx 2259\times$$
The actual energy absorbed per trip is **less than $0.05\%$** of the MOV's rated single-pulse capacity. This allows the breaker to endure thousands of consecutive fault interruptions without degradation.

---

## 3. High-Frequency RC Snubber Network Design

To suppress high-frequency parasitic LC ringing between $L_{\text{loop}}$ and the SiC MOSFET output capacitance ($C_{\text{oss}} \approx 65\,\text{pF}$ at $400\,\text{V}$), an RC snubber is placed in parallel with each bidirectional switch.

### 3.1 Parameter Calculation
1. **Snubber Capacitance ($C_s$):** Choose $C_s \ge 3 \times C_{\text{oss}}$ to dominate device capacitance:
   $$C_s = 10\,\text{nF} \quad (\text{Polypropylene Film, } 1600\,\text{V}_{\text{DC}})$$
2. **Characteristic Impedance ($Z_0$):**
   $$Z_0 = \sqrt{\frac{L_{\text{loop}}}{C_s}} = \sqrt{\frac{20 \times 10^{-6}}{10 \times 10^{-9}}} = \sqrt{2000} \approx 44.7\,\Omega$$
3. **Damping Resistance ($R_s$):** For critical damping ($\zeta = 1.0$):
   $$R_s \approx Z_0 = 47.0\,\Omega \quad (5\,\text{W Non-inductive wirewound or thick-film})$$
4. **Snubber Resistor Continuous Power Dissipation:**
   $$P_{\text{snub}} = C_s \times V_{\text{AC,RMS}}^2 \times 2\pi f = (10 \times 10^{-9}\,\text{F}) \times (230\,\text{V})^2 \times (2\pi \times 50\,\text{Hz}) = 0.166\,\text{W}$$
   A $5\,\text{W}$ rated resistor operates at $3.3\%$ capacity, ensuring low operating temperature.

---

## 4. Steady-State Thermal Analysis & Heatsink Sizing

### 4.1 Conduction Loss Formulation
In the double-pole topology, current flows through 4 SiC MOSFETs in series:
* Phase Pole: $Q_1$ and $Q_2$ in series.
* Neutral Pole: $Q_3$ and $Q_4$ in series.

Since the MOSFET channels are turned ON bidirectionally with $V_{GS} = +15\,\text{V}$, both forward and reverse current flow through the SiC channel rather than the body diode, ensuring minimum $R_{\text{DS(on)}}$ conduction.

#### Temperature-Dependent On-Resistance:
Wolfspeed C3M0075120K on-resistance follows:
$$R_{\text{DS(on)}}(T_j) = R_{\text{DS(on)}}(25^\circ\text{C}) \times \left[ 1 + \alpha (T_j - 25^\circ\text{C}) \right]$$
Where $R_{\text{DS(on)}}(25^\circ\text{C}) = 75\,\text{m}\Omega$ and $\alpha \approx 0.0045/^\circ\text{C}$.
At expected junction temperature $T_j = 100^\circ\text{C}$:
$$R_{\text{DS(on)}}(100^\circ\text{C}) = 75 \times [1 + 0.0045 \times 75] = 75 \times 1.3375 = 100.3\,\text{m}\Omega$$

#### Continuous Power Dissipation:
At rated nominal current $I_{\text{RMS}} = 16.0\,\text{A}$:
$$P_{\text{FET}} = I_{\text{RMS}}^2 \times R_{\text{DS(on)}}(100^\circ\text{C}) = (16.0\,\text{A})^2 \times 0.1003\,\Omega = 25.68\,\text{W per MOSFET}$$
$$\text{Total System Conduction Loss } P_{\text{total}} = 4 \times 25.68\,\text{W} = \mathbf{102.72\,\text{W}}$$

### 4.2 Thermal Resistance Network
```
  [ Junction Tj ]
        |
     R_theta_jc  (0.65 °C/W per TO-247-4)
        |
  [ Case Tc ]
        |
     R_theta_cs  (0.20 °C/W Sil-Pad / Mica)
        |
  [ Heatsink Ts ]
        |
     R_theta_sa  (Heatsink to Ambient)
        |
  [ Ambient Ta = 40°C ]
```

### 4.3 Required Heatsink Thermal Resistance ($R_{\theta\text{sa}}$)
Let design target junction temperature be $T_j = 105^\circ\text{C}$ in an ambient environment of $T_a = 40^\circ\text{C}$:
$$\Delta T_{\text{total}} = T_j - T_a = 105^\circ\text{C} - 40^\circ\text{C} = 65.0^\circ\text{C}$$

For 4 identical MOSFETs mounted on a shared common heatsink:
$$\Delta T_{\text{total}} = P_{\text{total}} \times R_{\theta\text{sa}} + P_{\text{FET}} \times (R_{\theta\text{jc}} + R_{\theta\text{cs}})$$
$$65.0^\circ\text{C} = 102.72 \times R_{\theta\text{sa}} + 25.68 \times (0.65 + 0.20)$$
$$65.0 = 102.72 \times R_{\theta\text{sa}} + 25.68 \times 0.85$$
$$65.0 = 102.72 \times R_{\theta\text{sa}} + 21.83$$
$$102.72 \times R_{\theta\text{sa}} = 43.17$$
$$R_{\theta\text{sa}} \le \frac{43.17}{102.72} = \mathbf{0.420^\circ\text{C/W}}$$

### 4.4 Heatsink Selection & Cooling Mode
* **Natural Convection Limit (75 mΩ Gen-1):** An extruded heatsink of dimensions $150\,\text{mm} \times 100\,\text{mm} \times 40\,\text{mm}$ provides $R_{\theta\text{sa}} \approx 1.2\text{--}1.5^\circ\text{C/W}$, which is sufficient for continuous operation up to $8.5\,\text{A}_{\text{RMS}}$ ($28\,\text{W}$ total).
* **Forced-Air Convection (75 mΩ Gen-1 at 16A Continuous):** Adding a compact $60\,\text{mm} \times 60\,\text{mm}$ $12\,\text{V}$ brushless DC fan delivering $2.0\,\text{m/s}$ linear airflow reduces heatsink thermal resistance to **$0.35^\circ\text{C/W}$**, holding $T_j$ comfortably below $98^\circ\text{C}$.

---

### 4.5 Gen-2 Low-Loss Optimization (Wolfspeed C3M0021120K: 21 mΩ)

Upgrading the power switches to **Wolfspeed C3M0021120K** ($1200\,\text{V}, 21\,\text{m}\Omega$) drastically transforms system thermal feasibility:

1. **On-Resistance at $T_j = 100^\circ\text{C}$:**
   $$R_{\text{DS(on)}}(100^\circ\text{C}) = 21\,\text{m}\Omega \times [1 + 0.0045 \times 75] = 21 \times 1.3375 = \mathbf{28.09\,\text{m}\Omega}$$
2. **Continuous Conduction Loss at $16\,\text{A}_{\text{RMS}}$:**
   $$P_{\text{FET,Gen2}} = (16.0\,\text{A})^2 \times 0.02809\,\Omega = 7.19\,\text{W per MOSFET}$$
   $$P_{\text{total,Gen2}} = 4 \times 7.19\,\text{W} = \mathbf{28.76\,\text{W}}$$
3. **Thermal Loss Comparison:**
   $$\text{Loss Reduction} = \frac{102.72\,\text{W} - 28.76\,\text{W}}{102.72\,\text{W}} \times 100\% = \mathbf{72.0\% \text{ reduction}}$$
4. **Natural Convection Passive Sizing:**
   With $P_{\text{total}} = 28.76\,\text{W}$ and $P_{\text{FET}} = 7.19\,\text{W}$:
   $$\Delta T = 65.0^\circ\text{C} = 28.76 \times R_{\theta\text{sa}} + 7.19 \times (0.43 + 0.20) = 28.76 \times R_{\theta\text{sa}} + 4.53$$
   $$R_{\theta\text{sa}} \le \frac{65.0 - 4.53}{28.76} = \frac{60.47}{28.76} = \mathbf{2.10^\circ\text{C/W}}$$
   A standard compact extruded heatsink ($100\,\text{mm} \times 60\,\text{mm} \times 25\,\text{mm}$) achieves $R_{\theta\text{sa}} \approx 1.8\text{--}2.0^\circ\text{C/W}$, allowing **completely silent, fan-less natural convection cooling** within a standard electrical enclosure!
