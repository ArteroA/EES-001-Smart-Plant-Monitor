# EES-001 — Smart Plant Monitor

**Status:** Simulation prototype — physical build pending  
**Platform:** ESP32-WROVER / Arduino C++  
**Simulation:** [Open Wokwi project](https://wokwi.com/projects/480783464526701569)

## Purpose
Develop a practical, minimalist soil-moisture indicator for an indoor Monstera deliciosa. The device will use a capacitive soil-moisture sensor and a 128×64 SSD1306 OLED. The display shows only an actionable status; numerical readings and diagnostics are available through Serial.

## User interface
- **Sun:** No watering needed
- **Droplet:** Watering needed
- **Warning triangle:** Sensor error
- **Question mark:** Watering status not yet established (including a brief moment following sensor recovery)

## Architecture
`Soil moisture → capacitive sensor → ESP32 ADC → filtering/state logic → OLED`

In the Wokwi simulation, a **potentiometer** substitutes for the physical soil-moisture sensor.

| Connection | ESP32 pin |
| --- | --- |
| Analog sensor / simulated potentiometer | GPIO 34 (ADC) |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| OLED power | 3V3 |
| OLED ground | GND |

## Simulated behavior
- **ADC increases as the simulated soil gets drier** (simulation convention; verify actual sensor direction).
- Water-needed transition: ADC **> 3000**.
- No-water-needed transition: ADC **< 2700**.
- Between 2700 and 3000, retain the previous watering state (**hysteresis**).
- Sensor-invalid range: ADC **< 400** or **> 3600**, with error after three consecutive invalid readings.
- Recovery: three consecutive valid readings; currently a brief **?** appears before classification resumes.
- Serial diagnostics include soil classification, sensor state, and experimental watering/stuck-sensor detection.

**Important:** All numerical thresholds are simulation assumptions, not calibrated physical values.

## Validation status
See [engineering-log.md](engineering-log.md) for observed tests, limitations, and next steps. The display states and sensor error/recovery were tested in Wokwi; the physical circuit and long-duration detection features have **not** been fully validated.

## Project files
- [Engineering log](engineering-log.md): dated decisions, learning, tests, outstanding work
- [Simulation notes](simulation/README.md): Wokwi link and export instructions
- [Source notes](src/README.md): code publication status
- [Build images](images/README.md): planned documentation photographs

## Next milestones
1. Export the exact current Wokwi sketch and circuit diagram into this repository.
2. Receive and inspect the capacitive sensor and OLED.
3. Measure real ADC values in dry, appropriately moist, and saturated/drained soil.
4. Calibrate thresholds, verify sensor polarity, and test hysteresis.
5. Assemble and validate the physical system; photograph and document results.

## Design philosophy
Simulation first, incremental validation, honest distinction between prototype and completed build, and a simple OLED that communicates only what the plant owner needs.
