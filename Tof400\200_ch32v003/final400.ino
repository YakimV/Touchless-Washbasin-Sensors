#include <Wire.h>
#include <VL53L1X.h>

#define OUT_PIN PC4

// Distance thresholds (mm)
const uint16_t DISTANCE_ON = 200;    // Trigger ON threshold
const uint16_t DISTANCE_OFF = 205;   // Trigger OFF threshold (hysteresis)

// Debounce cycles before turning off
const uint8_t TURN_OFF_COUNT = 2;

// Region of Interest (ROI / FOV) dimensions (valid range: 4 to 16; smaller narrows the optical beam)
const uint8_t ROI_WIDTH = 4;
const uint8_t ROI_HEIGHT = 4;

// Sensor ranging configuration
const VL53L1X::DistanceMode DISTANCE_MODE = VL53L1X::Short; // Short (up to 1.3 m), Medium (up to 3 m), Long (up to 4 m)
const uint32_t TIMING_BUDGET = 20000;                       // Timing budget in microseconds (min 20000 µs)
const uint32_t UPDATE_PERIOD = 20;                          // Inter-measurement period in milliseconds

VL53L1X sensor;

bool is_on = false;
uint8_t away_count = 0;

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  digitalWrite(OUT_PIN, LOW);

  Wire.begin();
  Wire.setClock(400000); // 400 kHz fast I2C

  sensor.setTimeout(500);

  if (!sensor.init()) {
    while (1);
  }

  sensor.setROISize(ROI_WIDTH, ROI_HEIGHT);
  sensor.setDistanceMode(DISTANCE_MODE);
  sensor.setMeasurementTimingBudget(TIMING_BUDGET);
  sensor.startContinuous(UPDATE_PERIOD);
}

void loop() {
  uint16_t distance = sensor.read();
  bool is_data_valid = (sensor.ranging_data.range_status == 0); // Status 0 indicates valid ranging

  if (!sensor.timeoutOccurred()) {
    bool object_in_zone = false;

    if (is_data_valid && distance > 0) {
      if (!is_on && distance <= DISTANCE_ON) {
        object_in_zone = true;
      } else if (is_on && distance <= DISTANCE_OFF) {
        object_in_zone = true;
      }
    }

    // Turn ON logic
    if (object_in_zone) {
      away_count = 0;
      if (!is_on) {
        is_on = true;
        digitalWrite(OUT_PIN, HIGH);
      }
    } 
    // Turn OFF logic with debounce
    else {
      away_count++;

      if (away_count >= TURN_OFF_COUNT) {
        if (is_on) {
          is_on = false;
          digitalWrite(OUT_PIN, LOW);
        }
        away_count = TURN_OFF_COUNT;
      }
    }
  }
}
