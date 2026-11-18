# Wokwi Simulation 08 — OLED Integration

[Open interactive simulation](https://wokwi.com/projects/480783464526701569)

## Archived files
- [sketch.ino](sketch.ino): source code supplied by the project owner
- [diagram.json](diagram.json): Wokwi circuit configuration supplied by the project owner
- [libraries.txt](libraries.txt): Adafruit display dependencies (library list reconstructed from sketch includes; confirm Wokwi library manager if necessary)

Wokwi uses **board-esp32-devkit-c-v4**, a potentiometer (GPIO34), and an SSD1306 OLED (SDA GPIO21, SCL GPIO22). The planned physical board is an ESP32-WROVER, so verify pinout and board selection before construction.

## Recorded evidence
The user-provided screenshot shows the droplet icon with ADC 3535 and Serial messages “Dry Soil” and “Watering Needed.” Other OLED states and recovery were reported as tested in earlier sessions.

**Note:** The screenshot was supplied in chat but has not yet been committed as a binary image. Code was transcribed from chat formatting; test a fresh Wokwi import to confirm reproducibility.

All thresholds are simulation assumptions, not real sensor calibration.

## Simulation 10 — OLED dashboard and three LEDs (accepted 2026-11-18)
- [Final sketch](sim10/sketch.ino)
- [Final Wokwi diagram](sim10/diagram.json)
- User-provided screenshot confirmed ADC 3126, dry soil, WATER NEEDED, blue LED lit, and recovery messages in Serial.
- OLED shows raw ADC, provisional moisture percentage, soil and watering status, and elapsed time since probable watering.
- Yellow LED GPIO25, blue GPIO26, red GPIO27, each with 330-ohm series resistor. Potentiometer GPIO34; OLED SDA21/SCL22.
- This is a simulation milestone, not physical validation. Sensor thresholds, percentage calibration, watering-event detection and long-duration behavior require physical testing.
- Earlier `simulation/sketch.ino` and `simulation/diagram.json` are retained as Simulation 08 snapshots, not the latest circuit.
