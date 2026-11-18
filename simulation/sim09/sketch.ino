#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// EES-001 Simulation 09: decorative indicator + diagnostic OLED
// Potentiometer simulates the soil sensor; higher ADC = drier.
// ALL thresholds and moisture percentages are simulation-only.
constexpr int SENSOR_PIN = 34;
constexpr int SUN_LED = 25;
constexpr int DROP_LED = 26;
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;
constexpr int SENSOR_MIN = 400;
constexpr int SENSOR_MAX = 3600;
constexpr int WATER_ON = 3000;
constexpr int WATER_OFF = 2700;
constexpr int CONFIRM_SAMPLES = 3;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
enum State { UNKNOWN, OK, NEEDS_WATER, SENSOR_ERROR };
State state = UNKNOWN;
int invalidSamples = 0;
int recoverySamples = 0;

const char* statusText() {
  switch (state) {
    case OK: return "NO WATER NEEDED";
    case NEEDS_WATER: return "WATER NEEDED";
    case SENSOR_ERROR: return "SENSOR ERROR";
    default: return "CHECKING...";
  }
}

void updateOutputs(int adc) {
  digitalWrite(SUN_LED, state == OK ? HIGH : LOW);
  digitalWrite(DROP_LED, state == NEEDS_WATER ? HIGH : LOW);

  // Illustration only: this is NOT a calibrated soil moisture percentage.
  int simulatedPercent = constrain(map(adc, SENSOR_MIN, SENSOR_MAX, 100, 0), 0, 100);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("EES-001 / SIM 09");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 17);
  display.print("ADC: ");
  display.println(adc);
  display.setCursor(0, 29);
  display.print("SIM MOIST: ");
  if (state == SENSOR_ERROR) {
    display.println("--");
  } else {
    display.print(simulatedPercent);
    display.println("%");
  }
  display.setCursor(0, 42);
  display.println(statusText());
  display.setCursor(0, 55);
  display.print("TIME/PRED: LATER");
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(SUN_LED, OUTPUT);
  pinMode(DROP_LED, OUTPUT);
  digitalWrite(SUN_LED, LOW);
  digitalWrite(DROP_LED, LOW);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    while (true) delay(1000);
  }
  updateOutputs(0);
}

void loop() {
  int adc = analogRead(SENSOR_PIN);
  bool valid = adc >= SENSOR_MIN && adc <= SENSOR_MAX;

  if (!valid) {
    invalidSamples++;
    recoverySamples = 0;
    if (invalidSamples >= CONFIRM_SAMPLES) state = SENSOR_ERROR;
  } else {
    invalidSamples = 0;
    if (state == SENSOR_ERROR) {
      recoverySamples++;
      if (recoverySamples >= CONFIRM_SAMPLES) {
        recoverySamples = 0;
        state = UNKNOWN;
      } else {
        // Keep warning active until recovery is confirmed.
        Serial.printf("ADC=%d | %s | recovery %d/%d\n",
                      adc, statusText(), recoverySamples, CONFIRM_SAMPLES);
        updateOutputs(adc);
        delay(1000);
        return;
      }
    }
    // Hysteresis: retain previous state between 2700 and 3000.
    if (adc > WATER_ON) state = NEEDS_WATER;
    else if (adc < WATER_OFF) state = OK;
  }

  Serial.printf("ADC=%d | %s | valid=%s\n",
                adc, statusText(), valid ? "yes" : "no");
  updateOutputs(adc);
  delay(1000);
}
