# Simulation 09 — Functional Art Indicator

**Status:** New simulation design, **not yet tested in Wokwi**.

## Files
- `sketch.ino`: ESP32 Arduino sketch
- `diagram.json`: Wokwi circuit
- `libraries.txt`: display libraries

## New behavior
- Yellow LED on GPIO25: sun / moisture sufficient.
- Blue LED on GPIO26: droplet / watering needed.
- Both off during UNKNOWN or SENSOR_ERROR.
- OLED on I²C GPIO21/22: raw ADC, **simulated-only** moisture percentage, status.
- Potentiometer on GPIO34 models moisture; ADC increases as soil gets drier.
- Hysteresis: >3000 dry, <2700 sufficient; intermediate values hold state.
- Three consecutive invalid readings (<400 or >3600) cause sensor error; three consecutive valid readings confirm recovery.
- On confirmed recovery, classify immediately if outside hysteresis band; otherwise UNKNOWN until enough evidence exists.

## Exclusions and caveats
- No actual neon/LED strip power circuit is represented: ordinary indicator LEDs stand in for the artwork. Actual LED neon requires an appropriate external supply and switching driver.
- Percent is a linear *illustration*, not calibrated volumetric water content.
- No last-watering date, real-time clock, or watering prediction. Those require a time source, event logging, and eventually measured drying trends.
- No long-term sensor drift, watering-event detection, or stuck-sensor feature in this new baseline. Those can be evaluated independently.
- Wokwi ESP32 DevKit C V4 is a simulation substitute for the intended physical ESP32-WROVER.

## Wokwi verification checklist
- [ ] Project compiles and OLED initializes.
- [ ] ADC <2700: yellow LED on, blue off, OLED says NO WATER NEEDED.
- [ ] ADC >3000: blue LED on, yellow off, OLED says WATER NEEDED.
- [ ] ADC 2700–3000: retain previous decision.
- [ ] Three invalid readings: both LEDs off, OLED SENSOR ERROR.
- [ ] Three valid readings: recovery and correct classification.
- [ ] Check resistor wiring and LED orientation in the Wokwi diagram.

To try: create a new Wokwi ESP32 project and replace its `sketch.ino` and `diagram.json` with these files. Add the two Adafruit libraries if Wokwi prompts.
