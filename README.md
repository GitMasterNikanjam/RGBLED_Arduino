# RGBLED — Arduino RGB LED Library

A lightweight Arduino library to drive **common-cathode** or **common-anode** RGB LEDs with:

* Simple color helpers (`red()`, `green()`, `blue()`, `white()`, …) 
* Deterministic **blocking** and **non-blocking** blinking with exact total duration (remainder distribution) 
* Optional **PWM** path with 8-bit color (`setRGB`) and global **brightness** control 
* **Glitch-free init** (preloads OFF level before switching pins to OUTPUT) 
* Deterministic, RAM-safe **error codes** (no `String` allocations) 

---

## Table of Contents

* [Supported Hardware](#supported-hardware)
* [Installation](#installation)
* [Wiring](#wiring)
* [Quick Start](#quick-start)
* [API Overview](#api-overview)
* [Blinking: Blocking vs Non-Blocking](#blinking-blocking-vs-non-blocking)
* [PWM Mode & Brightness](#pwm-mode--brightness)
* [Error Handling](#error-handling)
* [Examples](#examples)
* [Doxygen Docs](#doxygen-docs)
* [Migration Notes](#migration-notes)
* [License](#license)

---

## Supported Hardware

* Any Arduino-compatible board with 3 GPIOs driving an RGB LED (direct or via transistor/driver).
* **Common-Cathode** (CC) or **Common-Anode** (CA) wiring is supported via `parameters.COMMON_STATE`. The library maps logic levels internally using `_onLevel` so your code stays the same for both modes. 

> **Note:** For PWM color/brightness, use PWM-capable pins on your board.

---

## Installation

### Arduino IDE

1. Copy `RGBLED.h` and `RGBLED.cpp` into a folder named `RGBLED` inside your `Documents/Arduino/libraries/` directory.
2. Restart the IDE.

### Arduino CLI (Linux-first)

```bash
arduino-cli lib install .
arduino-cli compile --fqbn arduino:avr:uno examples/CompleteDemo
arduino-cli upload  --fqbn arduino:avr:uno -p /dev/ttyACM0 examples/CompleteDemo
```

---

## Wiring

* **Common Cathode (CC):** connect LED common pin to **GND**. Writing **HIGH** lights a color.
* **Common Anode (CA):** connect LED common pin to **VCC**. Writing **LOW** lights a color.
  Set:

```cpp
led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE; // or RGBLED_COMMON_ANODE
```

The library computes `_onLevel` and handles inversion internally. 

The library also **preloads OFF** to the output latches *before* `pinMode(…, OUTPUT)` to avoid a power-up flash. 

---

## Quick Start

```cpp
#include <Arduino.h>
#include "RGBLED.h"

RGBLED led;

void setup() {
  led.parameters.RED_PIN   = 9;
  led.parameters.GREEN_PIN = 10;
  led.parameters.BLUE_PIN  = 11;
  led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE;

  if (!led.init()) {
    Serial.begin(115200);
    Serial.print(F("Init failed: "));
    Serial.println(RGBLED::errorText(led.lastError)); // no String allocation
    while (true) {}
  }

  led.red();
  delay(500);
  led.cyan();
  delay(500);

  // Non-blocking blink: total 1200 ms, 4 cycles; progress via loop()
  led.purple();
  led.blink(1200, 4, false);
}

void loop() {
  led.blinkUpdate(); // progresses non-blocking blink
}
```

---

## API Overview

### Configuration & Init

```cpp
RGBLED led;
led.parameters.RED_PIN   = <pin>;
led.parameters.GREEN_PIN = <pin>;
led.parameters.BLUE_PIN  = <pin>;
led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE; // or RGBLED_COMMON_ANODE
bool ok = led.init(); // glitch-free, sets outputs OFF and guards subsequent calls
```

Glitch-free init and `_initFlag` guard are implemented in `init()`. 

### Basic Control (boolean colors)

```cpp
led.set(true,false,false); // custom (R,G,B)
led.red(); led.green(); led.blue(); led.yellow(); led.cyan(); led.purple(); led.white();
led.off(); led.on(); led.toggle(); led.inverse();
bool on = led.isOn();
bool r,g,b; led.getColor(r,g,b);
```

APIs and behavior are defined in the header. 

### Blinking

```cpp
led.blink(total_ms, cycles, /*blocking=*/true_or_false);
while (led.isBlinking()) led.blinkUpdate(); // only needed for non-blocking
led.stopBlink(/*turnOff=*/true);
```

Non-blocking engine uses `_blinkDelayMs`, `_blinkRemainder`, `_currentDelay`, and `millis()` unsigned arithmetic.  

### PWM & 8-bit Color (optional)

```cpp
led.enablePWM(true);        // turn on PWM path
led.setBrightness(200);     // global 0..255
led.setRGB(255, 128, 0);    // 8-bit color (maps to digital ON/OFF when PWM disabled)
```

PWM functions and macros are defined in the header and implemented in `_applyOutputs()`.  

---

## Blinking: Blocking vs Non-Blocking

**Total duration** is the exact time for the entire on↔off sequence:

* Number of edges = `2 * cycles`.
* Base half-period = `duration / edges`.
* **Remainder** (`duration % edges`) is distributed by adding +1 ms to the **first N edges**, ensuring total time equals `duration`. This applies to both blocking and non-blocking modes. 

Non-blocking mode toggles in `blinkUpdate()` using `millis()`; it’s safe across overflow due to unsigned subtraction. 

---

## PWM Mode & Brightness

* Enable with `enablePWM(true)`.
* Set 8-bit per-channel color with `setRGB(r,g,b)` and global brightness with `setBrightness(b)`.
* If PWM is **disabled**, any non-zero 8-bit value maps to ON for that channel. Behavior is unified via `_applyOutputs()`. 

### Selecting the PWM backend

By default, the library uses `analogWrite`. You can override via a macro:

```cpp
#define RGBLED_ANALOG_WRITE(pin, val) analogWrite((pin), (val)) // default
// Example (ESP32 + LEDC):
// #define RGBLED_ANALOG_WRITE(pin, val) ledcWrite(myChannelFor(pin), (val))
#include "RGBLED.h"
```

The macro and usage notes are documented in the header. 

---

## Error Handling

* `lastError` holds the last error code; values are from `RGBLED_Error` (`RGBLED_OK`, `RGBLED_ERR_PARAMS`).
* `errorText(err)` returns a PROGMEM string suitable for printing (Flash-safe on AVR). 

Example:

```cpp
if (!led.init()) {
  Serial.println(RGBLED::errorText(led.lastError));
}
```

---

## Examples

* **CompleteDemo.ino** – exercises digital colors, blocking/non-blocking blink, PWM 8-bit color, and brightness sweep. (See the example provided with this library or paste it into `examples/CompleteDemo/CompleteDemo.ino`.)
  The demo calls `blink(1200, 4, false)` and validates elapsed time to ~1200 ms using the remainder distribution logic. 

---

## Doxygen Docs

This library is fully annotated for Doxygen. To generate HTML docs:

1. Install Doxygen (`sudo apt-get install doxygen graphviz`).
2. Create a `Doxyfile` or run `doxygen -g` and set:

   * `PROJECT_NAME = "RGBLED"`
   * `EXTRACT_ALL = YES`
   * `RECURSIVE = YES`
3. Run `doxygen` in the project root.

The header contains detailed `@file`, `@class`, `@enum`, `@struct`, and method docs. 

---

## Migration Notes

From earlier versions using `String errorMessage`:

* **Now:** use `lastError` (`RGBLED_Error`) and `errorText()` for human messages (no heap/fragmentation on AVR). 
* `blink()` semantics are the same, but total duration is **exact** via remainder distribution (both blocking and non-blocking). 
* Added PWM path: `enablePWM()`, `setRGB()`, `setBrightness()` (optional; digital mode still supported). 

---

## License

MIT (recommended). Add your preferred license text here.

---

### Acknowledgements

* Designed for clarity, determinism, and embedded friendliness (no dynamic allocation in the core path).
* Thanks to the glitch-free init trick (preloading OFF), your LED won’t flash at boot. 

