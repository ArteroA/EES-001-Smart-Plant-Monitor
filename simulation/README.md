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
