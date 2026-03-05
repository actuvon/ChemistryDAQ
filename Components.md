| Comp. #     | Name                           | Desc                                                    | Connections | Diagrams |
| ----------- | ------------------------------ | ------------------------------------------------------- | ----------- | -------- |
| **C1**      | **Motherboard**                | Main proto-board, hosts electronic modules/MCUs         |             |          |
|             |                                |                                                         |             |          |
| **C1-1**    | **Arduino Nano**               | Acts as the ADC                                         |             |          |
| C1-1:0      | USB Port                       |                                                         |             |          |
| C1-1:1      | SCK                            | Pin D13                                                 | C1-7-1:6    |          |
| C1-1:3      | Analog Reference               |                                                         |             |          |
| C1-1:4      | Amp 1 Analog IN                | Pin A0                                                  | C1-2-1      |          |
| C1-1:5      | Amp 2 Analog IN                | Pin A1                                                  | C1-2-2      |          |
| C1-1:6      | Amp 3 Analog IN                | Pin A2                                                  | C1-2-3      |          |
| C1-1:7      | Amp 4 Analog IN                | Pin A3                                                  | C1-2-4      |          |
| C1-1:22     | Data Ready                     | Pin D4                                                  |             |          |
| C1-1:28     | SS                             | Pin D10                                                 | C1-7-1:5    |          |
| C1-1:29     | MOSI                           | Pin D11                                                 | C1-7-1:2    |          |
| C1-1:30     | MISO                           | Pin D12                                                 | C1-7-1:1    |          |
|             |                                |                                                         |             |          |
| **C1-2**    | **Amplifier Subassemblies**    |                                                         |             |          |
| C1-2-1      | Amplifier 1 Subassembly        | All parts related to amplifier 1 / CH1                  |             |          |
| C1-2-1-1    | Amplifier                      | AD620 amplifier module                                  |             |          |
| C1-2-1-1:1  | Vin                            | 5V power in                                             | C1-4-1:3    |          |
| C1-2-1-1:2  | GND                            |                                                         |             |          |
| C1-2-1-1:3  | S-                             | Raw signal in (negative side)                           |             |          |
| C1-2-1-1:4  | S+                             | Raw signal in (positive side)                           |             |          |
| C1-2-1-1:5  | GND                            |                                                         |             |          |
| C1-2-1-1:6  | Vout                           | Amplified signal output (1.5V - 3.5V)                   | C1-4-1:3    |          |
| C1-2-1-1:7  | GND                            |                                                         |             |          |
| C1-2-1-1:8  | V-                             | Negative voltage supply                                 |             |          |
| C1-2-1-1:9  | Rg (a)                         | Gain resistor terminal a (~248Ω)                        |             |          |
| C1-2-1-1:10 | Rg (b)                         | Gain resistor terminal b (~248Ω)                        |             |          |
| C1-2-1-2    | Input Header                   | 1 x 4 female header                                     |             |          |
| C1-2-1-3    | Output Header                  | 1 x 4 female header                                     |             |          |
| C1-2-1-4    | Rg Header                      | 1 x 2 female header                                     |             |          |
| C1-2-1-5    | Rg                             | Gain resistor, sets the gain of the amplifier           |             |          |
| C1-2-1-5-1  | Rg, a                          | 22 Ω                                                    |             |          |
| C1-2-1-5-2  | Rg, b                          | 180 Ω                                                   |             |          |
| C1-2-1-5-3  | Rg, adjust                     | 0 - 100 Ω                                               |             |          |
|             |                                |                                                         |             |          |
| C1-2-2      | Amplifier 2 Subassembly        | All parts related to amplifier 2 / CH2                  |             |          |
| C1-2-2-1    | Amplifier                      | AD620 amplifier module                                  |             |          |
| C1-2-2-1:3  | S-                             | Raw signal in (negative side)                           |             |          |
| C1-2-2-1:4  | S+                             | Raw signal in (positive side)                           |             |          |
| C1-2-2-1:6  | Vout                           | Amplified signal output (1.5V - 3.5V)                   |             |          |
|             |                                |                                                         |             |          |
| C1-2-3      | Amplifier 3 Subassembly        | All parts related to amplifier 3 / CH3                  |             |          |
| C1-2-3-1    | Amplifier                      | AD620 amplifier module                                  |             |          |
| C1-2-3-1:3  | S-                             | Raw signal in (negative side)                           |             |          |
| C1-2-3-1:4  | S+                             | Raw signal in (positive side)                           |             |          |
| C1-2-3-1:6  | Vout                           | Amplified signal output (1.5V - 3.5V)                   |             |          |
|             |                                |                                                         |             |          |
| C1-2-4      | Amplifier 4 Subassembly        | All parts related to amplifier 4 / CH4                  |             |          |
| C1-2-4-1    | Amplifier                      | AD620 amplifier module                                  |             |          |
| C1-2-4-1:3  | S-                             | Raw signal in (negative side)                           |             |          |
| C1-2-4-1:4  | S+                             | Raw signal in (positive side)                           |             |          |
| C1-2-4-1:6  | Vout                           | Amplified signal output (1.5V - 3.5V)                   |             |          |
|             |                                |                                                         |             |          |
| **C1-3**    | **ESP32**                      | Master microcontroller                                  |             |          |
| C1-3:22     | Data Ready                     | GPIO 17                                                 |             |          |
| C1-3:23     | CS                             | GPIO 5                                                  |             |          |
| C1-3:24     | SCK                            | GPIO 18                                                 |             |          |
| C1-3:25     | MISO                           | GPIO 19                                                 |             |          |
| C1-3:26     | SDA                            | GPIO 21                                                 |             |          |
| C1-3-29     | SCL                            | GPIO 22                                                 |             |          |
| C1-3:30     | MOSI                           | GPIO 23                                                 |             |          |
|             |                                |                                                         |             |          |
| **C1-4**    | **5V Regulator Subassembly**   |                                                         |             |          |
| C1-4-1      | 5V Regulator                   | L7805CV                                                 |             |          |
| C1-4-1:1    | Input (12V)                    |                                                         |             |          |
| C1-4-1:2    | GND                            |                                                         |             |          |
| C1-4-1:3    | Output (5V)                    | 5V rail output source                                   |             |          |
| C1-4-2      | Input Capacitor                | 47 µF, 50 V                                             |             |          |
| C1-4-3      | Output Capacitor               | 4.7 µF, 50 V                                            |             |          |
| C1-4-4      | Power LED                      |                                                         |             |          |
| C1-4-5      | LED Resistor                   | 2.2 kΩ                                                  |             |          |
| C1-4-6      | Utility Header                 | Auxiliary 5 V supply header                             |             |          |
|             |                                |                                                         |             |          |
| **C1-5**    | **Voltage Reference Subasm**   | ~8.5 mA capacity, 5.000 VDC                             |             |          |
| C1-5-1      | Adjustable Band Gap Reference  | TL431AC                                                 |             |          |
| C1-5-2      | Power Resistor                 | 820 Ω                                                   |             |          |
| C1-5-3      | Adjustment Potentiometer       | 0 - 1 kΩ                                                |             |          |
| C1-5-4      | R1, a                          | 15 kΩ                                                   |             |          |
| C1-5-5      | R1, b                          | 15 kΩ                                                   |             |          |
| C1-5-6      | R2, a                          | 15 kΩ                                                   |             |          |
| C1-5-7      | R2, b                          | 15 kΩ                                                   |             |          |
| C1-5-8      | Access Point                   | 1 x 1 female header to measure output during adjustment |             |          |
|             |                                |                                                         |             |          |
| **C1-6**    | **Calibration Point Subasm**   |                                                         |             |          |
| C1-6-1      | Adjustment Potentiometer       | 0 - 5 kΩ                                                |             |          |
| C1-6-2      | R1                             | 47 kΩ                                                   |             |          |
| C1-6-3      | R2                             | 100 Ω                                                   |             |          |
| C1-6-4      | Calibration Point Header       | 1 x 3 male header providing 10 mV, 0 V, and 0 V         |             |          |
|             |                                |                                                         |             |          |
| **C1-7**    | **Level Shifters Subassembly** |                                                         |             |          |
| C1-7-1      | Level Shifter 1                |                                                         |             |          |
| C1-7-1:1    | HV4: MISO                      |                                                         | C1-1:30     |          |
| C1-7-1:2    | HV3: MOSI                      |                                                         | C1-1:29     |          |
| C1-7-1:3    | GND: 0V                        |                                                         |             |          |
| C1-7-1:4    | HV:  5V                        |                                                         |             |          |
| C1-7-1:5    | HV2: SS                        |                                                         | C1-1:28     |          |
| C1-7-1:6    | HV1: SCK                       |                                                         | C1-1:1      |          |
| C1-7-1:7    | LV1: SCK                       |                                                         |             |          |
| C1-7-1:8    | LV2: SS                        |                                                         |             |          |
| C1-7-1:9    | LV:  3.3V                      |                                                         |             |          |
| C1-7-1:10   | GND: 0V                        |                                                         |             |          |
| C1-7-1:11   | LV3: MOSI                      |                                                         |             |          |
| C1-7-1:12   | LV4: MISO                      |                                                         |             |          |
| C1-7-2      | Level Shifter 2                |                                                         |             |          |
|             |                                |                                                         |             |          |
| **C1-8**    | **SD Module Subasm**           |                                                         |             |          |
|             |                                |                                                         |             |          |
| **C1-9**    | **RTC Module Subasm**          |                                                         |             |          |
|             |                                |                                                         |             |          |
| **C1-10**   | **OLED Module Subasm**         |                                                         |             |          |
| C1-10:1     | GND                            |                                                         |             |          |
| C1-10:2     | Vcc                            |                                                         |             |          |
| C1-10:3     | SCL                            |                                                         |             |          |
| C1-10:4     | SDA                            |                                                         |             |          |