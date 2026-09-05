#include <Wire.h>
#include <VL53L0X.h>

#define OUT_PIN PC4

// Distance thresholds (in mm)
const uint16_t DISTANCE_ON   = 135;   // Target distance to trigger output ON
const uint16_t DISTANCE_OFF  = 140;   // Target distance to turn output OFF (hysteresis)
const uint16_t VOID_DISTANCE = 1200;  // Maximum limit; values beyond this force an immediate shutoff

// Minimum signal rate limit in MCPS to reject optical noise and stray reflections
const float SIGNAL_RATE_LIMIT = 0.8;

// Debounce cycles (~33 ms per cycle)
const uint8_t TURN_ON_COUNT  = 3;
const uint8_t TURN_OFF_COUNT = 3;

VL53L0X sensor;

bool is_on = false;
uint8_t away_count = 0;
uint8_t detect_count = 0;

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  digitalWrite(OUT_PIN, HIGH); // Idle state (active-low)

  Wire.begin();
  Wire.setClock(400000); // 400 kHz Fast-mode I2C

  sensor.setTimeout(150); // Prevent bus lockup
  
  if (!sensor.init()) {
    while (1); 
  }

  sensor.setSignalRateLimit(SIGNAL_RATE_LIMIT);
  sensor.setMeasurementTimingBudget(33000); // 33 ms per measurement
  sensor.startContinuous();
}

void loop() {
  uint16_t distance = sensor.readRangeContinuousMillimeters();
  bool timeout = sensor.timeoutOccurred();

  // Detect timeouts, sensor zero-reads, out-of-range values, or error codes (>= 8000)
  bool in_void = (timeout || distance == 0 || distance > VOID_DISTANCE || distance >= 8000);
  bool object_in_zone = false;

  if (!in_void) {
    if (!is_on && distance <= DISTANCE_ON) {
      object_in_zone = true;
    } else if (is_on && distance <= DISTANCE_OFF) {
      object_in_zone = true;
    }
  }

  if (object_in_zone) {
    away_count = 0;

    if (!is_on) {
      detect_count++;
      if (detect_count >= TURN_ON_COUNT) {
        is_on = true;
        digitalWrite(OUT_PIN, LOW); // Active-low: pull LOW on trigger
      }
    }
  } else {
    detect_count = 0;

    if (is_on) {
      if (in_void) {
        away_count = TURN_OFF_COUNT; // Instant shutoff on abrupt loss of signal
      } else {
        away_count++; // Debounced shutoff if target gradually drifted past DISTANCE_OFF
      }

      if (away_count >= TURN_OFF_COUNT) {
        is_on = false;
        digitalWrite(OUT_PIN, HIGH); // Restore idle state (HIGH)
      }
    }
  }
}
