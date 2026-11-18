# EES-001 — Smart Plant Monitor

**Status:** Simulation prototype — physical build pending  
**Platform:** ESP32-WROVER / Arduino C++  
**Simulation:** [Open Wokwi project](https://wokwi.com/projects/480783464526701569)

## Purpose
Develop a practical, minimalist soil-moisture indicator for an indoor Monstera deliciosa. The device will use a capacitive soil-moisture sensor and a 128×64 SSD1306 OLED. The current simulated OLED dashboard shows raw ADC, provisional relative-moisture percentage, soil classification, watering status, and time since a probable watering event. Readings remain visible during sensor errors; Serial provides additional diagnostics.

## User interface (Simulation 10)
- OLED: raw ADC (0–4095), estimated relative moisture %, soil classification, status, and probable last-watering elapsed time.
- Yellow LED: no watering needed; blue LED: watering needed; red LED: solid during UNKNOWN, blinking during SENSOR_ERROR.
- The OLED retains the raw ADC and provisional estimate during UNKNOWN and SENSOR_ERROR for troubleshooting; estimates are unreliable during sensor faults.

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
See [engineering-log.md](engineering-log.md) for observed tests, limitations, and next steps. Simulation 10 was reported complete by the project owner, with screenshot evidence of the OLED dashboard, blue LED, and sensor recovery. Long-duration detection features and physical operation have **not** been fully validated.

## Project files
- [Engineering log](engineering-log.md): dated decisions, learning, tests, outstanding work
- [Simulation notes](simulation/README.md): Wokwi link and export instructions
- [Source notes](src/README.md): code publication status
- [Build images](images/README.md): planned documentation photographs

## Next milestones
1. Review the archived [Simulation 10 sketch](simulation/sim10/sketch.ino) and [diagram](simulation/sim10/diagram.json).
2. Receive and inspect the capacitive sensor and OLED.
3. Measure real ADC values in dry, appropriately moist, and saturated/drained soil.
4. Calibrate thresholds, verify sensor polarity, and test hysteresis.
5. Assemble and validate the physical system; photograph and document results.

## Design philosophy
Simulation first, incremental validation, honest distinction between prototype and completed build, and an OLED dashboard that exposes useful measurements and diagnostics.

## Simulation 10 archive (2026-11-18)
- [Sketch](simulation/sim10/sketch.ino) and [Wokwi circuit](simulation/sim10/diagram.json) include three LEDs and the OLED dashboard.
- Temporary mapping: ADC 1080 = 100% relative wetness; ADC 3300 = 0%, clamped outside endpoints. This is not volumetric water content.
- Potentiometer movement can cause false probable-watering events; the displayed time is elapsed since boot, not a persistent timestamp.
- Wokwi completion is user-reported; physical calibration and testing are pending.
