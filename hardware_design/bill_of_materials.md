# Complete Bill of Materials (BOM)
## Smart Bidirectional Double-Pole SiC Solid-State Circuit Breaker (SSCB)
### Operating Ratings: 230 V~RMS~ / 16 A~RMS~ Continuous (Peak Interruption: 50–80 A, <650 ns)

---

## 1. Executive Procurement Overview

| Category | Description | Primary Distributors | Estimated Subtotal (USD) |
| :--- | :--- | :--- | :--- |
| **Power Stage & Thermal** | 4× 1200V SiC MOSFETs, Heatsink, Sil-Pads, Fan | Mouser / DigiKey | ~$85.00 |
| **Gate Drive & Floating Rails** | Si8274 Isolated Drivers, Mornsun B0515S-1WR3 DC-DC | Mouser / DigiKey / LCSC | ~$28.00 |
| **Sensing & Instrumentation** | ACS758 Current Sensors, AMC1301 Isolated Amp, Op-Amps | Mouser / DigiKey | ~$32.00 |
| **Transient Snubber & MOV** | Littelfuse UltraMOV, High-Voltage Snubber RC | Mouser / DigiKey | ~$12.00 |
| **PFC & Zero-Crossing** | BTA16/24 Triac, MOC3041, Motor-Run PFC Film Cap | Mouser / DigiKey / LCSC | ~$15.00 |
| **Controllers & Cloud IoT** | STM32G431KB Core, ESP32 Co-Processor, Optocouplers | Mouser / DigiKey / LCSC | ~$24.00 |
| **Auxiliary Power & Terminal** | Mean Well IRM-05-5, 30A Phoenix Terminals, LDOs, Passives | Mouser / DigiKey / LCSC | ~$22.00 |
| **Estimated Total BOM Cost** | Single Prototype Unit (Excluding Raw PCB Fabrication) | | **~$218.00** |

---

## 2. Comprehensive Component Bill of Materials

### 2.1 Power Stage & Thermal Management

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **Q1, Q2, Q3, Q4** | SiC MOSFET 1200V, 21 mΩ (Gen-2 Low-Loss) or 75 mΩ (Gen-1), Kelvin Source | Wolfspeed | **C3M0021120K** (or **C3M0075120K**) | TO-247-4 | 4 | [Mouser](https://www.mouser.com/c/?q=C3M0021120K) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=C3M0021120K) |
| **HS1** | Extruded Aluminum Heatsink ($R_{\theta\text{sa}} \le 0.40^\circ\text{C/W}$) for 4× TO-247 | Wakefield-Vette / Fischer | **483-300AB** or **SK 483** | Extrusion | 1 | [Mouser](https://www.mouser.com/c/?q=Wakefield+TO-247+heatsink) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=Wakefield+heatsink+TO-247) |
| **PAD1-4** | High Thermal Conductivity Insulator Sheet (Sil-Pad / Ceramic, >6 kV isolation) | Bergquist / Henkel | **SP900S-0.009-00-54** | TO-247 | 4 | [Mouser](https://www.mouser.com/c/?q=SP900S-0.009) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=SP900S) |
| **FAN1** | 60mm × 60mm × 25mm 12V/5V High-CFM Brushless Cooling Fan (Optional for 21mΩ) | Delta Electronics | **EFB0612HA** | 60mm | 1 | [Mouser](https://www.mouser.com/c/?q=EFB0612HA) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=EFB0612HA) |

---

### 2.2 Isolated Gate Drive Subsystem

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **U1, U2** | Dual Galvanic Isolated Gate Driver, 2.5 kVrms, >200 V/ns CMTI, 30ns prop delay | Skyworks / Silicon Labs | **SI8274AB1-IS1** | SOIC-16 Narrow | 2 | [Mouser](https://www.mouser.com/c/?q=SI8274AB1-IS1) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=SI8274AB1-IS1) |
| **PS1, PS2, PS3, PS4** | Isolated DC-DC (+15V / -3.5V Asymmetric SiC Drive or +15V Single, 3kV Isolation) | Mornsun / RECOM | **QA01D** or **B0515S-1WR3** | SIP-7 / SIP-4 | 4 | [Mouser](https://www.mouser.com/c/?q=QA01D) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=QA01D) \| [LCSC](https://www.lcsc.com/search?q=QA01D) |
| **Rg1_on - Rg4_on** | 10 Ω 1% 1206 Thick Film Gate Turn-on Resistors | Vishay Dale | **CRCW120610R0FKEA** | 1206 | 4 | [Mouser](https://www.mouser.com/c/?q=CRCW120610R0FKEA) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=CRCW120610R0FKEA) |
| **Rg1_off - Rg4_off** | 4.7 Ω 1% 1206 Thick Film Gate Turn-off Resistors | Vishay Dale | **CRCW12064R70FKEA** | 1206 | 4 | [Mouser](https://www.mouser.com/c/?q=CRCW12064R70FKEA) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=CRCW12064R70FKEA) |
| **D1, D2, D3, D4** | Fast Switching Schottky Diode for turn-off bypass (30V, 200mA) | onsemi / Diodes Inc. | **BAT54-7-F** | SOT-23 | 4 | [Mouser](https://www.mouser.com/c/?q=BAT54-7-F) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=BAT54-7-F) |
| **ZD1, ZD2, ZD3, ZD4** | 18V 500mW Gate-Source Transient Clamping Zener Diodes | onsemi | **BZX84C18LT1G** | SOT-23 | 4 | [Mouser](https://www.mouser.com/c/?q=BZX84C18LT1G) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=BZX84C18LT1G) |

---

### 2.3 Current & Isolated Voltage Sensing Subsystem

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **CS1, CS2** | Galvanically Isolated Hall-Effect Current Sensor IC (±50A, 4.8 kVrms isolation, 120 kHz) | Allegro MicroSystems | **ACS758LCB-050B-PFF-T** (or **ACS770LCB-050B-PFF-T**) | CB-PFF 5-pin | 2 | [Mouser](https://www.mouser.com/c/?q=ACS758LCB-050B-PFF-T) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=ACS758LCB-050B-PFF-T) |
| **U3** | Reinforced Isolated Differential Amplifier ($\pm 250\text{mV}$ input, Gain 8.2, 15 kV/µs CMTI) | Texas Instruments | **AMC1301DWVR** | SOIC-8 Wide | 1 | [Mouser](https://www.mouser.com/c/?q=AMC1301DWVR) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=AMC1301DWVR) |
| **R_div1, R_div2, R_div3** | 400 kΩ 0.1% 2512 High Voltage Resistors (in series for 1.2 MΩ creepage safety) | Vishay Dale | **CRCW2512400KFKEG** | 2512 | 3 | [Mouser](https://www.mouser.com/c/?q=CRCW2512400KFKEG) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=CRCW2512400KFKEG) |
| **R_div_lo** | 750 Ω 0.1% 25 ppm Precision Low-Side Divider Resistor | Susumu / TE | **RG2012P-751-B-T5** | 0805 | 1 | [Mouser](https://www.mouser.com/c/?q=RG2012P-751-B-T5) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=RG2012P-751-B-T5) |
| **U4** | Precision Rail-to-Rail Dual Op-Amp (for differential to single-ended ADC buffer) | Texas Instruments | **OPA2340UA** or **TLV2372IDR** | SOIC-8 | 1 | [Mouser](https://www.mouser.com/c/?q=OPA2340UA) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=OPA2340UA) |

---

### 2.4 Transient Overvoltage Clamping & RC Snubber Network

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **MOV1, MOV2** | UltraMOV Metal Oxide Varistor (275V~RMS~, 710V clamp, 150 Joules, 10 kA peak) | Littelfuse | **V275LA40AP** | Radial 20mm Disc | 2 | [Mouser](https://www.mouser.com/c/?q=V275LA40AP) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=V275LA40AP) |
| **C_snub1, C_snub2** | 10 nF 1600V DC / 650V AC High-Pulse Metallized Polypropylene Film Capacitor | KEMET | **R76UR2100SE30J** | Radial Film (15mm pitch) | 2 | [Mouser](https://www.mouser.com/c/?q=R76UR2100SE30J) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=R76UR2100SE30J) |
| **R_snub1, R_snub2** | 47 Ω 5W Non-Inductive Thick Film / Ceramic Resistor | Ohmite / Vishay | **TCH35P47R0JE** (or **EVE**) | TO-220 or Power Radial | 2 | [Mouser](https://www.mouser.com/c/?q=TCH35P47R0JE) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=TCH35P47R0JE) |

---

### 2.5 Dynamic PFC & Zero-Crossing Switching Subsystem

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **TR1** | Snubberless Isolated Tab TRIAC (16A/25A, 600V/800V, 35mA Igt) | STMicroelectronics | **BTA16-600BW3G** or **BTA24-800BW** | TO-220AB Isolated | 1 | [Mouser](https://www.mouser.com/c/?q=BTA16-600BW3G) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=BTA16-600BW3G) |
| **U5** | Zero-Crossing Phototriac Driver IC (400V Vdrm, 600% isolation) | onsemi / Fairchild | **MOC3041M** | DIP-6 / SMD-6 | 1 | [Mouser](https://www.mouser.com/c/?q=MOC3041M) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=MOC3041M) |
| **C_pfc** | 15 µF 450 VAC Metallized Polypropylene Motor Run / Power Factor Capacitor | Epcos / TDK / KEMET | **B32332I6156J080** | Can / Stud Mount | 1 | [Mouser](https://www.mouser.com/c/?q=B32332I6156J080) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=B32332I6156J080) |
| **R_triac_gate** | 330 Ω 1W Metal Oxide Current Limiting Resistor | Vishay BCcomponents | **PR01000103300JA100** | Axial 1W | 2 | [Mouser](https://www.mouser.com/c/?q=PR01000103300JA100) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=PR01000103300JA100) |

---

### 2.6 Microcontroller & Cloud IoT Processing Units

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **MCU1** | High-Performance Arm Cortex-M4F MCU @ 170MHz, Dual Fast ADCs, AWD Comparators | STMicroelectronics | **STM32G431KBT6** (or **NUCLEO-G431KB**) | LQFP-32 or Nucleo Board | 1 | [Mouser](https://www.mouser.com/c/?q=NUCLEO-G431KB) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=NUCLEO-G431KB) |
| **IOT1** | Wi-Fi & Bluetooth Dual-Core MCU Module, 4MB Flash | Espressif Systems | **ESP32-WROOM-32E** (or **ESP32-DevKitC-32E**) | SMD Module / Dev Board | 1 | [Mouser](https://www.mouser.com/c/?q=ESP32-DevKitC-32E) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=ESP32-DevKitC-32E) |
| **OC1, OC2** | General Purpose 4-Pin Phototransistor Optocoupler (5 kVrms isolation) | Everlight / Sharp | **EL817(S)(TA)** or **PC817** | SOP-4 / DIP-4 | 2 | [Mouser](https://www.mouser.com/c/?q=EL817) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=EL817) \| [LCSC](https://www.lcsc.com/search?q=PC817) |

---

### 2.7 Auxiliary Isolated Power Supply & Passive Components

| RefDes | Description | Mfr | MPN | Package | Qty | Real-Time Buying Links |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| **PS_AUX** | Compact AC-DC Converter Module (85–264 VAC In, 5 VDC Out, 5W, 3 kVrms isolation) | Mean Well | **IRM-05-5** | PCB Mount (Enclosed) | 1 | [Mouser](https://www.mouser.com/c/?q=IRM-05-5) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=IRM-05-5) |
| **REG1, REG2** | 3.3V 1A Low-Dropout Linear Voltage Regulator | Texas Instruments / Diodes | **AZ1117EH-3.3TRG1** or **LM1117-3.3** | SOT-223 | 2 | [Mouser](https://www.mouser.com/c/?q=AZ1117EH-3.3TRG1) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=AZ1117EH-3.3) |
| **FUSE1** | 500 mA 250V Time-Lag Ceramic Cartridge Fuse (Auxiliary supply protection) | Littelfuse | **0215.500MXP** | 5×20 mm Cartridge | 1 | [Mouser](https://www.mouser.com/c/?q=0215.500MXP) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=0215.500MXP) |
| **TB1, TB2** | High-Current 32A 600V Screw Terminal Block (7.62 mm pitch, $L_{\text{in}}, N_{\text{in}}, L_{\text{out}}, N_{\text{out}}$) | Phoenix Contact | **FRONT 4-H-7,62** (or **1703021**) | PCB Screw Clamp | 4 pos | [Mouser](https://www.mouser.com/c/?q=1703021) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=1703021) |
| **C_bulk** | 47 µF 25V Low-ESR Aluminum Electrolytic Capacitors | Panasonic | **EEU-FR1E470** | Radial | 4 | [Mouser](https://www.mouser.com/c/?q=EEU-FR1E470) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=EEU-FR1E470) |
| **C_decouple** | 100 nF 50V X7R Ceramic Decoupling Capacitors | Murata / KEMET | **GRM188R71H104KA93D** | 0603 / 0805 | 20 | [Mouser](https://www.mouser.com/c/?q=GRM188R71H104KA93D) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=GRM188R71H104KA93D) |
| **SW1, SW2** | SPST-NO Sealed 6mm Tactile Pushbuttons (Trip on PB1, Reset on PB6) | C&K / Omron | **B3F-1000** or **PTS645** | 6×6 mm Through-hole | 2 | [Mouser](https://www.mouser.com/c/?q=B3F-1000) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=B3F-1000) |
| **LED1, LED2** | 3mm Standard Through-Hole LEDs (Green OK on PB3, Red Fault on PB5) | Kingbright / Lite-On | **WP710A10GD** / **WP710A10ID** | Radial 3mm | 2 | [Mouser](https://www.mouser.com/c/?q=WP710A10GD) \| [DigiKey](https://www.digikey.com/en/products/result?keywords=WP710A10GD) |

---

## 3. Purchasing & Sourcing Guidelines

1. **SiC MOSFET Authenticity:** Purchase Wolfspeed C3M0075120K exclusively from authorized franchised distributors (Mouser, DigiKey, Farnell/Newark). Avoid unverified marketplace brokers due to the prevalence of counterfeit wide-bandgap semiconductors.
2. **Kelvin Source Routing Verification:** Ensure that the ordered Wolfspeed devices are the **4-lead TO-247-4 (`K` suffix)** version and NOT standard 3-lead TO-247-3 (`D` suffix), as the separate Pin 3 Kelvin connection is essential for the $<650\,\text{ns}$ turn-off without gate ringing.
3. **Mornsun DC-DC Isolation Capacitance:** Verify that gate driver isolated supplies have ultra-low barrier coupling capacitance ($C_{io} < 3\,\text{pF}$) to maintain $>150\,\text{V/ns}$ common-mode transient immunity (CMTI).
4. **Resistor Creepage in High-Voltage Sensing:** Use three 400 kΩ resistors in series (as specified above) rather than a single 1.2 MΩ resistor, ensuring compliance with IEC 60664-1 clearance and preventing surface arc-over across package terminals under mains surge conditions.
