#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// ==========================================
// EES-001 SMART PLANT MONITOR
// Simulation 10 - OLED Dashboard (provisional calibration)
// ==========================================
// Pin configuration
const int INPUT_PIN = 34;
const int OLED_SDA = 21;
const int OLED_SCL = 22;
const int OUTPIN_LED_YELLOW = 25;
const int OUTPIN_LED_BLUE = 26;
const int OUTPIN_LED_RED = 27;
// OLED configuration
Adafruit_SSD1306 display(128, 64, &Wire, -1);
// Moisture classification thresholds
const int WET_THRESH = 1500;
const int MOIST_THRESH = 3001;
// Provisional relative-wetness calibration (NOT volumetric water content)
const int ADC_WET_CAL = 1080;
const int ADC_DRY_CAL = 3300;
// Watering hysteresis thresholds
const int WATER = 3000;
const int NO_WATER = 2700;
// Sensor validation
// Temporary values until physical calibration
const int SENSOR_MIN = 400;
const int SENSOR_MAX = 3600;
const int MAX_INVALID = 3;
const int MAX_VALID = 3;
// Stuck-sensor detection
const int STUCK_TOLERANCE = 5;
const unsigned long STUCK_TIMEOUT =
  12UL * 60UL * 60UL * 1000UL;
// Automatic watering detection
const int WATERING_DROP = 300;
const unsigned long WATERING_WINDOW = 120000UL;
const unsigned long WATERING_COOLDOWN = 300000UL;
// Historical readings
const int HISTORY_SIZE = 121;
int adcHistory[HISTORY_SIZE];
unsigned long timeHistory[HISTORY_SIZE];
int historyIndex = 0;
int historyCount = 0;
// System states
enum WaterState {
  UNKNOWN,
  NO_WATER_NEEDED,
  WATER_NEEDED,
  SENSOR_ERROR
};
WaterState wateringState = UNKNOWN;
// Sensor validation counters
int validCount = 0;
int invalidCount = 0;
// Stuck-sensor variables
int referenceReading = 0;
unsigned long stableStartTime = 0;
bool monitoringStarted = false;
bool stuckWarning = false;
// Watering detection variables
unsigned long lastWateringTime = 0;
bool hasWateringOccurred = false;
const unsigned long SENSOR_INTERVAL = 1000;
unsigned long previousSensorTime = 0;
const unsigned long BLINK_INTERVAL = 500;
unsigned long previousBlinkTime = 0;
bool redLedOn = false;
// ==========================================
// HISTORY MANAGEMENT
// ==========================================
void clearHistory() {
  historyIndex = 0;
  historyCount = 0;
}
void storeReading(int adc) {
  adcHistory[historyIndex] = adc;
  timeHistory[historyIndex] = millis();
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  if (historyCount < HISTORY_SIZE) {
    historyCount++;
  }
}
// ==========================================
// AUTOMATIC WATERING DETECTION
// ==========================================
bool detectWatering(int currentADC) {
  unsigned long now = millis();
  for (int i = 0; i < historyCount; i++) {
    // Ignore readings older than two minutes
    if (now - timeHistory[i] > WATERING_WINDOW) {
      continue;
    }
    // Significant ADC decrease indicates
    // a possible watering event
    if (adcHistory[i] - currentADC >= WATERING_DROP) {
      return true;
    }
  }
  return false;
}
// ==========================================
// OLED DISPLAY
// ==========================================
// Relative wetness, based on provisional dry/wet reference points.
// Recalibrate for the actual sensor and Monstera potting mix.
int relativeMoisturePercent(int adc) {
  long percent = map(adc, ADC_DRY_CAL, ADC_WET_CAL, 0, 100);
  return constrain(percent, 0L, 100L);
}

void updateDisplay(int adc) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Header (always visible, including sensor errors)
  display.setCursor(0, 0);
  display.print("MONSTERA MONITOR");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // Raw measurement and provisional percentage
  display.setCursor(0, 13);
  display.print("ADC:");
  display.print(adc);
  display.setCursor(76, 13);
  display.print("~");
  display.print(relativeMoisturePercent(adc));
  display.print("%");

  display.setCursor(0, 25);
  display.print("SOIL: ");
  if (adc < WET_THRESH) display.print("WET");
  else if (adc < MOIST_THRESH) display.print("MOIST");
  else display.print("DRY");
  // Classifications are untrusted when sensor readings are invalid.
  if (adc < SENSOR_MIN || adc > SENSOR_MAX) display.print(" (INVALID)");

  display.setCursor(0, 37);
  display.print("STATUS: ");
  switch (wateringState) {
    case SENSOR_ERROR: display.print("SENSOR ERROR"); break;
    case UNKNOWN: display.print("UNKNOWN"); break;
    case WATER_NEEDED: display.print("WATER NEEDED"); break;
    case NO_WATER_NEEDED: display.print("OK"); break;
  }

  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);
  display.setCursor(0, 53);
  display.print("LAST: ");
  if (!hasWateringOccurred && lastWateringTime == 0) {
    display.print("NOT RECORDED");
  } else {
    // Elapsed time since a *probable* detected watering event.
    unsigned long elapsedMinutes = (millis() - lastWateringTime) / 60000UL;
    if (elapsedMinutes < 60) {
      display.print(elapsedMinutes);
      display.print("m AGO");
    } else if (elapsedMinutes < 1440) {
      display.print(elapsedMinutes / 60);
      display.print("h AGO");
    } else {
      display.print(elapsedMinutes / 1440);
      display.print("d AGO");
    }
  }
  display.display();
}

//=========================================
// LED setup
//=========================================
void updateLEDs() {
  // Default: turn all LEDs off
  digitalWrite(OUTPIN_LED_YELLOW, LOW);
  digitalWrite(OUTPIN_LED_BLUE, LOW);
  if (wateringState != SENSOR_ERROR) {
    digitalWrite(OUTPIN_LED_RED, LOW);
  }
  switch (wateringState) {
    case NO_WATER_NEEDED:
      digitalWrite(OUTPIN_LED_YELLOW, HIGH);
      break;
    case WATER_NEEDED:
      digitalWrite(OUTPIN_LED_BLUE, HIGH);
      break;
    case UNKNOWN:
      digitalWrite(OUTPIN_LED_RED, HIGH);
      break;
    case SENSOR_ERROR:
      // Red LED is managed by updateErrorBlink()
      break;
  }
}
void updateErrorBlink() {
  unsigned long now = millis();
  if (wateringState == SENSOR_ERROR) {
    if (now - previousBlinkTime >= BLINK_INTERVAL) {
      previousBlinkTime = now;
      redLedOn = !redLedOn;
      digitalWrite(OUTPIN_LED_RED, redLedOn ? HIGH : LOW);
    }
  } else {
    redLedOn = false;
    previousBlinkTime = now;
  }
}
// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPIN_LED_YELLOW, OUTPUT);
  pinMode(OUTPIN_LED_BLUE, OUTPUT);
  pinMode(OUTPIN_LED_RED, OUTPUT);
  // Initialize I2C
  Wire.begin(OLED_SDA, OLED_SCL);
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (true) {
      delay(1000);
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("EES-001");
  display.println("Initializing...");
  display.display();
  delay(1000);
}
// ==========================================
// MAIN LOOP
// ==========================================
void processSensor();

void loop() {
  unsigned long now = millis();
  if (now - previousSensorTime >= SENSOR_INTERVAL) {
    previousSensorTime = now;
    processSensor();
  }
  updateErrorBlink();
}
void processSensor() {
  int analog_reading = analogRead(INPUT_PIN);
  unsigned long now = millis();
  Serial.print("ADC: ");
  Serial.println(analog_reading);
  // ----------------------------------------
  // 1. SENSOR VALIDATION
  // ----------------------------------------
  if (analog_reading < SENSOR_MIN ||
      analog_reading > SENSOR_MAX) {
    invalidCount++;
    validCount = 0;
    clearHistory();
    monitoringStarted = false;
    stuckWarning = false;
    if (invalidCount >= MAX_INVALID) {
      wateringState = SENSOR_ERROR;
      Serial.println("Sensor Error");
    } else {
      Serial.println("Checking Sensor - Invalid Reading");
    }
    updateLEDs();
    updateDisplay(analog_reading);
     return;
  }
  // Reading is valid
  invalidCount = 0;
  // ----------------------------------------
  // 2. SENSOR RECOVERY
  // ----------------------------------------
  if (wateringState == SENSOR_ERROR) {
    validCount++;
    clearHistory();
    if (validCount >= MAX_VALID) {
      wateringState = UNKNOWN;
      validCount = 0;
      Serial.println("Sensor Recovered - Status Unknown");
    } else {
      Serial.println("Confirming Sensor Recovery");
    }
    updateLEDs();
    updateDisplay(analog_reading);
    return;
  }
  // ----------------------------------------
  // 3. STUCK-SENSOR MONITORING
  // ----------------------------------------
  if (!monitoringStarted) {
    referenceReading = analog_reading;
    stableStartTime = now;
    monitoringStarted = true;
  }
  int difference =
    abs(analog_reading - referenceReading);
  if (difference > STUCK_TOLERANCE) {
    referenceReading = analog_reading;
    stableStartTime = now;
    stuckWarning = false;
  }
  if (now - stableStartTime >= STUCK_TIMEOUT &&
      !stuckWarning) {
    stuckWarning = true;
    Serial.println(
      "Warning: Sensor Reading Unusually Stable"
    );
  }
  // ----------------------------------------
  // 4. AUTOMATIC WATERING DETECTION
  // ----------------------------------------
  bool cooldownFinished =
    !hasWateringOccurred ||
    (now - lastWateringTime >= WATERING_COOLDOWN);
  if (cooldownFinished) {
    // Start a fresh history after cooldown
    if (hasWateringOccurred) {
      clearHistory();
      hasWateringOccurred = false;
    }
    if (detectWatering(analog_reading)) {
      Serial.println("Probable Watering Detected!");
      lastWateringTime = now;
      hasWateringOccurred = true;
      clearHistory();
    } else {
      storeReading(analog_reading);
    }
  }
  // ----------------------------------------
  // 5. SOIL MOISTURE CLASSIFICATION
  // ----------------------------------------
  if (analog_reading < WET_THRESH) {
    Serial.println("Wet Soil");
  } else if (analog_reading < MOIST_THRESH) {
    Serial.println("Moist Soil");
  } else {
    Serial.println("Dry Soil");
  }
  // ----------------------------------------
  // 6. WATERING HYSTERESIS
  // ----------------------------------------
  if (analog_reading > WATER) {
    wateringState = WATER_NEEDED;
  } else if (analog_reading < NO_WATER) {
    wateringState = NO_WATER_NEEDED;
  }
  // ----------------------------------------
  // 7. SERIAL STATUS
  // ----------------------------------------
  if (wateringState == WATER_NEEDED) {
    Serial.println("Watering Needed");
  } else if (wateringState == NO_WATER_NEEDED) {
    Serial.println("No Watering Needed");
  } else if (wateringState == UNKNOWN) {
    Serial.println("Watering Status: Unknown");
  } else {
    Serial.println("Sensor Error");
  }
  updateLEDs();
  // ----------------------------------------
  // 8. UPDATE OLED
  // ----------------------------------------
  updateDisplay(analog_reading);
}