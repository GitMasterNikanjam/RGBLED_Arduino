/**
 * @file CompleteDemo.ino
 * @brief Demonstration of the RGBLED library: init, presets, PWM color, and blinking modes.
 *
 * Wiring:
 *   - RED   -> pin 9
 *   - GREEN -> pin 10
 *   - BLUE  -> pin 11
 *
 * For common-cathode modules use RGBLED_ACTIVE_HIGH.
 * For common-anode  modules use RGBLED_ACTIVE_LOW.
 *
 * Notes:
 *   - Finite/Blocking blink:    blink(total_duration_ms, number_of_cycles, true)
 *   - Finite/Non-blocking blink: blink(total_duration_ms, number_of_cycles, false) + call blinkUpdate() in loop()
 *   - Infinite blink:           blink(half_period_ms, 0, false) + call blinkUpdate() in loop(), stop with stopBlink()
 */

// -----------------------------------------------------------------------------
// Optional (ESP32):
// Map analog writes if needed. Uncomment and adjust as appropriate.
/*
#define RGBLED_ANALOG_WRITE(pin, val) ledcWrite(/* channel for pin * / 0, (val))
#include "RGBLED.h"
*/
// For AVR/Arduino cores where analogWrite exists, just include normally:
#include "RGBLED.h"

// Choose pins (make sure they're PWM-capable if you plan to use PWM):
constexpr int PIN_R = 9;
constexpr int PIN_G = 10;
constexpr int PIN_B = 11;

// Change to RGBLED_ACTIVE_LOW if your module is common-anode:
constexpr uint8_t ACTIVE_MODE = RGBLED_ACTIVE_HIGH;

RGBLED led;

// For demo flow control:
enum DemoPhase : uint8_t {
  PHASE_PRESETS = 0,
  PHASE_PWM_COLOR,
  PHASE_BLINK_BLOCKING,
  PHASE_BLINK_NONBLOCKING,
  PHASE_BLINK_INFINITE,
  PHASE_DONE
};
DemoPhase phase = PHASE_PRESETS;

unsigned long t0 = 0;            // general-purpose timer
unsigned long infiniteStart = 0;  // when we start infinite blink

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait if needed */ }

  Serial.println(F("\n[RGBLED Demo] Starting..."));

  // Configure parameters
  led.parameters.RED_PIN     = PIN_R;
  led.parameters.GREEN_PIN   = PIN_G;
  led.parameters.BLUE_PIN    = PIN_B;
  led.parameters.ACTIVE_MODE = ACTIVE_MODE;

  // Initialize and validate
  if (!led.init()) {
    Serial.print(F("Init failed: "));
    Serial.println(RGBLED::errorText(led.lastError));
    // Halt demo; library guards further calls if not inited.
    while (true) { delay(1000); }
  }

  Serial.println(F("Init OK."));
  t0 = millis();
}

void loop() {
  // Always pump non-blocking blinking state machine (safe to call even if idle)
  led.blinkUpdate();

  switch (phase) {
    case PHASE_PRESETS:
      demoPresets();              // cycle through basic preset colors
      break;

    case PHASE_PWM_COLOR:
      demoPwmColor();             // show 8-bit color + brightness fade
      break;

    case PHASE_BLINK_BLOCKING:
      demoBlinkBlocking();        // 3 cycles in 1200 ms (blocking)
      break;

    case PHASE_BLINK_NONBLOCKING:
      demoBlinkNonBlocking();     // 5 cycles in 2000 ms (non-blocking)
      break;

    case PHASE_BLINK_INFINITE:
      demoBlinkInfinite();        // 500 ms half-period (toggle every 500 ms), stop after ~6 s
      break;

    case PHASE_DONE:
    default:
      // Idle
      break;
  }
}

// -----------------------------------------------------------------------------
// Demo steps
// -----------------------------------------------------------------------------

void demoPresets() {
  static uint8_t step = 0;
  if (millis() - t0 < 600) return;  // hold each color ~0.6s

  switch (step) {
    case 0: Serial.println(F("[PRESETS] red()"));    led.red();    break;
    case 1: Serial.println(F("[PRESETS] green()"));  led.green();  break;
    case 2: Serial.println(F("[PRESETS] blue()"));   led.blue();   break;
    case 3: Serial.println(F("[PRESETS] yellow()")); led.yellow(); break;
    case 4: Serial.println(F("[PRESETS] purple()")); led.purple(); break;
    case 5: Serial.println(F("[PRESETS] cyan()"));   led.cyan();   break;
    case 6: Serial.println(F("[PRESETS] white()"));  led.white();  break;
    case 7:
      Serial.println(F("[PRESETS] off()"));
      led.off();
      step = 0;
      phase = PHASE_PWM_COLOR;
      t0 = millis();
      return;
  }
  step++;
  t0 = millis();
}

void demoPwmColor() {
  Serial.println(F("[PWM] enablePWM(true), setRGB(255,0,128), brightness sweep"));
  led.enablePWM(true);
  led.setRGB(255, 0, 128);  // magenta-ish

  // Fade brightness from 0 to 255 and back once
  for (int b = 0; b <= 255; ++b) {
    led.setBrightness((uint8_t)b);
    delay(4);
  }
  for (int b = 255; b >= 0; --b) {
    led.setBrightness((uint8_t)b);
    delay(4);
  }

  // Leave medium brightness
  led.setBrightness(160);
  phase = PHASE_BLINK_BLOCKING;
  t0 = millis();
}

void demoBlinkBlocking() {
  Serial.println(F("[BLINK Blocking] 3 cycles in 1200 ms total (each on/off ~200 ms)"));
  // total_duration=1200ms, number=3 cycles, blocking=true
  led.blink(1200, 3, true);

  // After blocking returns, ensure LED stays OFF for a moment
  delay(400);
  phase = PHASE_BLINK_NONBLOCKING;
  t0 = millis();
}

void demoBlinkNonBlocking() {
  Serial.println(F("[BLINK Non-Blocking] 5 cycles in 2000 ms total"));
  // total_duration=2000ms, number=5, blocking=false (progressed by blinkUpdate in loop)
  led.blink(2000, 5, false);

  // Wait until it finishes (non-blocking): we just poll isBlinking()
  while (led.isBlinking()) {
    led.blinkUpdate();
    delay(5); // avoid tight loop
  }

  // Small pause before next phase
  delay(300);
  phase = PHASE_BLINK_INFINITE;
  t0 = millis();
}

void demoBlinkInfinite() {
  Serial.println(F("[BLINK Infinite] half-period=500 ms (toggle every 500 ms); auto-stop after ~6 s"));
  // Infinite mode: number==0, duration interpreted as half-period
  led.blink(500, 0, false);   // always non-blocking in infinite mode
  infiniteStart = millis();

  // Run for ~6 seconds then stop
  while (millis() - infiniteStart < 6000UL) {
    led.blinkUpdate();
    delay(5);
  }

  led.stopBlink(true); // turn OFF on stop
  Serial.println(F("[BLINK Infinite] stopped. Demo done."));
  phase = PHASE_DONE;
}
