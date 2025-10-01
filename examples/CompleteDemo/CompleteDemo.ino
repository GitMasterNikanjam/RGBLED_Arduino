/**
 * @example CompleteDemo.ino
 * @brief Full functional test for the RGBLED library: digital mode, blocking & non-blocking
 *        blink, PWM 8-bit color, and brightness sweep.
 *
 * Wiring:
 *   - Select pins below for your board. For Arduino UNO/Nano, use PWM-capable pins for PWM demo.
 *   - COMMON_CATHODE: common pin to GND (logic HIGH lights a color).
 *   - COMMON_ANODE:   common pin to VCC (logic LOW lights a color).
 *
 * Notes:
 *   - Non-blocking blink requires calling led.blinkUpdate() frequently in loop().
 *   - PWM path uses analogWrite() by default. You can override with RGBLED_ANALOG_WRITE macro
 *     before including RGBLED.h if you want to map to a different backend (e.g., ledcWrite on ESP32).
 */

#include <Arduino.h>

// ---- OPTIONAL (uncomment for ESP32 custom mapping) ----
// #define RGBLED_ANALOG_WRITE(pin, val) ledcWrite((pin), (val))

#include "RGBLED.h"

// ---------------------- Pin Selection ----------------------

// Fallback (adjust as needed)
static constexpr uint8_t PIN_R = A8;
static constexpr uint8_t PIN_G = A9;
static constexpr uint8_t PIN_B = A10;

// Set your wiring mode here:
static constexpr RGBLED_CommonMode COMMON_MODE = RGBLED_COMMON_CATHODE;
// static constexpr RGBLED_CommonMode COMMON_MODE = RGBLED_COMMON_ANODE;

// ---------------------- Library Instance ----------------------
RGBLED led;

// ---------------------- Demo State Machine ----------------------
enum class Phase : uint8_t {
  Init,
  DigitalColors,
  BlockingBlink,
  NonBlockingBlink_Start,
  NonBlockingBlink_Run,
  PWM_Enable,
  PWM_Colors,
  PWM_BrightnessSweep,
  Done
};

static Phase phase = Phase::Init;
static uint32_t t0 = 0;
static uint8_t stepIdx = 0;

void runDigitalColors()
{
  // Cycle through basic colors in digital (boolean) mode
  switch (stepIdx) {
    case 0: led.red();    break;
    case 1: led.green();  break;
    case 2: led.blue();   break;
    case 3: led.yellow(); break;
    case 4: led.cyan();   break;
    case 5: led.purple(); break;
    case 6: led.white();  break;
    case 7: led.off();    break;
    default: break;
  }
  stepIdx++;
}

void runPWMColors()
{
  // Show a few 8-bit mixes (requires PWM enabled)
  switch (stepIdx) {
    case 0: led.setRGB(255, 0,   0  ); break; // red
    case 1: led.setRGB(0,   255, 0  ); break; // green
    case 2: led.setRGB(0,   0,   255); break; // blue
    case 3: led.setRGB(255, 255, 0  ); break; // yellow
    case 4: led.setRGB(0,   255, 255); break; // cyan
    case 5: led.setRGB(255, 0,   255); break; // magenta
    case 6: led.setRGB(255, 255, 255); break; // white
    case 7: led.setRGB(16,  8,   0  ); break; // warm dim
    default: break;
  }
  stepIdx++;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) { /* wait for USB CDC, if any */ }

  Serial.println(F("\n[RGBLED Demo] Starting..."));

  // Configure parameters
  led.parameters.RED_PIN   = PIN_R;
  led.parameters.GREEN_PIN = PIN_G;
  led.parameters.BLUE_PIN  = PIN_B;
  led.parameters.COMMON_STATE = COMMON_MODE;

  // Initialize
  if (!led.init()) {
    Serial.print(F("Init failed: "));
    Serial.println(RGBLED::errorText(led.lastError));
    // Halt here if init fails
    while (true) { delay(1000); }
  }

  Serial.print(F("Init OK. Mode = "));
  Serial.println((COMMON_MODE == RGBLED_COMMON_CATHODE) ? F("COMMON_CATHODE") : F("COMMON_ANODE"));

  phase = Phase::DigitalColors;
  stepIdx = 0;
  t0 = millis();
}

void loop()
{
  switch (phase) {
    case Phase::DigitalColors:
      // Show each color for 500 ms
      if (millis() - t0 >= 500) {
        t0 = millis();
        if (stepIdx <= 7) {
          runDigitalColors();
        } else {
          // Also exercise inverse/toggle quickly
          led.white();
          delay(200);
          led.inverse(); // should turn all off (invert 1,1,1 -> 0,0,0)
          delay(200);
          led.toggle();  // back ON (cached color after inverse)
          delay(200);
          led.off();

          phase = Phase::BlockingBlink;
          stepIdx = 0;
        }
      }
      break;

    case Phase::BlockingBlink:
      Serial.println(F("[Demo] Blocking blink: yellow, 3 cycles in 1000 ms total"));
      led.yellow();
      led.blink(1000, 3, true); // blocking: should take ~1s total
      phase = Phase::NonBlockingBlink_Start;
      break;

    case Phase::NonBlockingBlink_Start:
      Serial.println(F("[Demo] Non-blocking blink: purple, 4 cycles in 1200 ms total"));
      led.purple();
      led.blink(1200, 4, false); // non-blocking
      t0 = millis();
      phase = Phase::NonBlockingBlink_Run;
      break;

    case Phase::NonBlockingBlink_Run:
      led.blinkUpdate(); // progress non-blocking blink
      if (!led.isBlinking()) {
        uint32_t elapsed = millis() - t0;
        Serial.print(F("Non-blocking blink finished. Elapsed ≈ "));
        Serial.print(elapsed);
        Serial.println(F(" ms"));
        phase = Phase::PWM_Enable;
      }
      break;

    case Phase::PWM_Enable:
      Serial.println(F("[Demo] Enabling PWM path and testing 8-bit colors"));
      led.enablePWM(true);
      led.setBrightness(255);
      stepIdx = 0;
      t0 = millis();
      phase = Phase::PWM_Colors;
      break;

    case Phase::PWM_Colors:
      if (millis() - t0 >= 500) {
        t0 = millis();
        if (stepIdx <= 7) {
          runPWMColors();
        } else {
          phase = Phase::PWM_BrightnessSweep;
          stepIdx = 0;
          led.setRGB(255, 128, 0); // orange as base color
          Serial.println(F("[Demo] Brightness sweep on PWM (0 -> 255)"));
        }
      }
      break;

    case Phase::PWM_BrightnessSweep:
      if (millis() - t0 >= 80) {
        t0 = millis();
        uint8_t b = stepIdx * 16; // 0,16,32,...,255
        led.setBrightness(b);
        stepIdx++;
        if (stepIdx > 16) {
          led.off();
          phase = Phase::Done;
          Serial.println(F("[Demo] Completed."));
        }
      }
      break;

    case Phase::Done:
      // Demo finished. Keep calling blinkUpdate() just in case (no-op here).
      led.blinkUpdate();
      // Idle forever; nothing more to do.
      break;
  }
}
