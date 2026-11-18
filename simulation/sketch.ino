#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// EES-001 SMART PLANT MONITOR
// Simulation 08 - OLED Integration
// ==========================================

// Pin configuration
const int INPUT_PIN = 34;
const int OLED_SDA = 21;
const int OLED_SCL = 22;

// OLED configuration
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Moisture classification thresholds
const int WET_THRESH = 1500;
const int MOIST_THRESH = 3001;

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

void updateDisplay(int adc) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (wateringState == SENSOR_ERROR) {
    // Warning triangle
    display.drawTriangle(
      64, 8,
      32, 54,
      96, 54,
      SSD1306_WHITE
    );

    display.setTextSize(3);
    display.setCursor(56, 20);
    display.print("!");

  } else if (wateringState == WATER_NEEDED) {
    // Water droplet outline
    display.drawLine(64, 6, 42, 35, SSD1306_WHITE);
    display.drawLine(42, 35, 38, 45, SSD1306_WHITE);
    display.drawLine(38, 45, 40, 54, SSD1306_WHITE);
    display.drawLine(40, 54, 48, 60, SSD1306_WHITE);
    display.drawLine(48, 60, 64, 63, SSD1306_WHITE);

    display.drawLine(64, 6, 86, 35, SSD1306_WHITE);
    display.drawLine(86, 35, 90, 45, SSD1306_WHITE);
    display.drawLine(90, 45, 88, 54, SSD1306_WHITE);
    display.drawLine(88, 54, 80, 60, SSD1306_WHITE);
    display.drawLine(80, 60, 64, 63, SSD1306_WHITE);

  } else if (wateringState == NO_WATER_NEEDED) {
    // Sun center
    display.drawCircle(64, 32, 16, SSD1306_WHITE);

    // Sun rays
    display.drawLine(64, 3, 64, 11, SSD1306_WHITE);
    display.drawLine(64, 53, 64, 61, SSD1306_WHITE);

    display.drawLine(35, 32, 43, 32, SSD1306_WHITE);
    display.drawLine(85, 32, 93, 32, SSD1306_WHITE);

    display.drawLine(43, 11, 49, 17, SSD1306_WHITE);
    display.drawLine(79, 47, 85, 53, SSD1306_WHITE);

    display.drawLine(85, 11, 79, 17, SSD1306_WHITE);
    display.drawLine(49, 47, 43, 53, SSD1306_WHITE);

  } else {
    // UNKNOWN: Initial sensor status
    display.setTextSize(2);
    display.setCursor(58, 24);
    display.print("?");
  }

  display.display();
}


// ==========================================
// SETUP
// ==========================================

void setup() {
  Serial.begin(115200);

  pinMode(INPUT_PIN, INPUT);

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

void loop() {

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

    updateDisplay(analog_reading);

    delay(1000);
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

    updateDisplay(analog_reading);

    delay(1000);
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


  // ----------------------------------------
  // 8. UPDATE OLED
  // ----------------------------------------

  updateDisplay(analog_reading);

  delay(1000);
}
