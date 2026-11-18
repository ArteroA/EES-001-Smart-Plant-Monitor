# EES-001 — Engineering Log

> Working project record. Entries below summarize discussions and user-reported Wokwi observations, not independently certified hardware tests.

## 2026-11 — Requirements and simulation prototype
**Goal:** Build an ESP32-based soil moisture monitor for an indoor Monstera deliciosa, with a simple graphical OLED.

**Design decisions**
- Use an ESP32-WROVER and 0.96-inch 128×64 SSD1306 I²C OLED.
- Simulate soil moisture with a potentiometer on GPIO34 before connecting a physical capacitive sensor.
- Show sun / droplet / warning icons; keep raw ADC and diagnostic messages in Serial.
- Use hysteresis to avoid rapid changes around a single threshold.
- Defer Wi-Fi time synchronization, persistent history, and additional display details to avoid unnecessary complexity.
- Use one sensor in the root zone initially; drainage and bottom-of-pot moisture cannot be proven by a single sensor.

**Simulation observations reported**
- OLED I²C initialization and wiring functioned.
- Moisture state and corresponding graphical icons functioned.
- Three invalid readings triggered sensor error.
- Three subsequent valid readings allowed recovery; a brief unknown `?` state appeared on the third valid reading before normal classification on the next loop.
- Watering-event detection was explored, but the complete cooldown and long-duration stuck-sensor behavior remain unverified.

**Thresholds used in simulation**
- Sensor validity: 400–3600 inclusive.
- Water needed: ADC > 3000.
- No water needed: ADC < 2700.
- Intermediate readings retain the previous state.
- These values are **not physical calibration data**.

**Engineering concepts practiced**
- Analog-to-digital conversion and 12-bit ADC values
- I²C communication (SDA/SCL)
- Hysteresis and state machines
- Consecutive-sample error detection and recovery
- Distinguishing sensor measurements from actionable decisions
- Simulation limitations and empirical calibration

## 2026-11-18 — Physical design decisions
- Chose a natural branch rather than a moss pole for Monstera support, to reduce maintenance.
- Proposed fixing the branch to an independent wooden base, weighted by the plant pot, keeping the branch out of damp soil.
- Support stability, fastening, drainage, and possible insect damage in the branch still need inspection and testing.
- Plan to calibrate soil moisture using physical observations (including the finger test) alongside ADC readings.
- Long-duration excessive moisture warnings are a potential later enhancement, not a validated feature.

## Open tests
- [ ] Export exact Wokwi sketch and diagram
- [ ] Verify actual capacitive sensor output direction and operating voltage
- [ ] Record dry/moist/wet ADC calibration readings
- [ ] Validate hysteresis and error/recovery with physical sensor
- [ ] Evaluate sensor stability and drift
- [ ] Test prolonged-operation behavior and watering-event cooldown
- [ ] Check branch/base support stability and drainage
- [ ] Photograph wiring, assembly, and finished prototype

## Status
**Simulation prototype — physical build pending.** Do not describe this project as a completed or physically validated plant monitor yet.

## 2026-11-18 — Simulation 08 source archived
- Received the full current sketch, Wokwi diagram JSON, and screenshot from the project owner.
- Committed `simulation/sketch.ino`, `simulation/diagram.json`, and `simulation/libraries.txt`.
- Screenshot shows ADC 3535, dry-soil Serial output, and water-droplet OLED icon.
- Wokwi board is ESP32 DevKit C V4; planned physical board is ESP32-WROVER.
- Next: verify clean import of the committed simulation files; add screenshot to repository when binary upload is supported.
