#include "RGBLED.h"

RGBLED led;

void setup() {
  // Set your pins and wiring
  led.parameters.RED_PIN = 9;
  led.parameters.GREEN_PIN = 10;
  led.parameters.BLUE_PIN = 11;
  led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE; // or RGBLED_COMMON_ANODE

  led.init();

  // Cache the desired color = blue
  led.blue();

  // Infinite blink: half-period = 1000 ms (i.e., 1 s ON, 1 s OFF forever)
  led.blink(1000, 0, false);
}

void loop() {
  // Non-blocking update must be called periodically
  led.blinkUpdate();

  // ... your other logic runs freely here ...
}
