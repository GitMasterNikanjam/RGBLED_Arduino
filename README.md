# RGBLED — Arduino RGB LED Library

A lightweight Arduino library to drive **common-cathode** or **common-anode** RGB LEDs.

- Simple color helpers: `red()`, `green()`, `blue()`, `white()`, …
- Deterministic **blocking** and **non-blocking** blinking with exact total duration (remainder distribution)
- Optional **PWM** path with 8-bit color (`setRGB`) and global **brightness**
- **Glitch-free init**: preloads OFF before switching pins to OUTPUT (prevents power-up flash)
- Small, deterministic **error codes** (no dynamic `String`)

---

## Table of Contents

- [Supported Hardware](#supported-hardware)
- [Installation](#installation)
- [Wiring](#wiring)
- [Quick Start](#quick-start)
- [API Overview](#api-overview)
- [Blinking Modes](#blinking-modes)
- [PWM Mode & Brightness](#pwm-mode--brightness)
- [Error Handling](#error-handling)
- [Examples](#examples)
- [Doxygen Docs](#doxygen-docs)
- [Migration Notes](#migration-notes)
- [Changelog](#changelog)
- [License](#license)

---

## Supported Hardware

- Any Arduino-compatible board with 3 GPIOs driving an RGB LED (direct, or via transistors/drivers).
- **Common-Cathode (CC)** and **Common-Anode (CA)** are both supported via `parameters.COMMON_STATE`. The library maps logic levels internally so your user code is the same for both modes. :contentReference[oaicite:0]{index=0}

> **PWM note:** When using PWM color/brightness, choose pins that support your board’s PWM backend. :contentReference[oaicite:1]{index=1}

---

## Installation

### Arduino IDE

1. Create `Documents/Arduino/libraries/RGBLED/`
2. Put `RGBLED.h` and `RGBLED.cpp` in that folder
3. Restart the IDE

### Arduino CLI (Linux-first)

```bash
arduino-cli lib install .
arduino-cli compile --fqbn arduino:avr:uno examples/CompleteDemo
arduino-cli upload  --fqbn arduino:avr:uno -p /dev/ttyACM0 examples/CompleteDemo
````

---

## Wiring

* **Common Cathode (CC):** common pin → **GND**. Writing **HIGH** lights a color.
* **Common Anode (CA):** common pin → **VCC**. Writing **LOW** lights a color.
* Configure once:

```cpp
led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE; // or RGBLED_COMMON_ANODE
```

The library **preloads OFF** to output latches *before* `pinMode(..., OUTPUT)` to avoid a boot flash. 

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
    Serial.println(RGBLED::errorText(led.lastError));
    while (true) {}
  }

  // Infinite blue blink: 1000 ms ON, 1000 ms OFF (non-blocking)
  led.blue();               // cache desired color
  led.blink(1000, 0, false); // number == 0 -> infinite; duration = half-period
}

void loop() {
  led.blinkUpdate();        // required for non-blocking mode
}
```

---

## API Overview

### Configure & Init

```cpp
RGBLED led;
led.parameters.RED_PIN   = <pin>;
led.parameters.GREEN_PIN = <pin>;
led.parameters.BLUE_PIN  = <pin>;
led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE; // or RGBLED_COMMON_ANODE
bool ok = led.init(); // glitch-free, sets outputs OFF, guards subsequent calls
```

Glitch-free init and guard behavior are in `init()`. 

### Basic Control (boolean colors)

```cpp
led.set(true,false,false); // custom (R,G,B)
led.red(); led.green(); led.blue(); led.yellow(); led.cyan(); led.purple(); led.white();
led.off(); led.on(); led.toggle(); led.inverse();
bool on = led.isOn();
bool r,g,b; led.getColor(r,g,b);
```

Implemented via `_applyOutputs()`; digital mode uses HIGH/LOW, PWM mode uses 8-bit duty. 

---

## Blinking Modes

### 1) Finite Blink

```cpp
// Blink for a total 'duration' with a given number of on/off cycles.
led.blink(/*duration_ms=*/1200, /*number=*/4, /*blocking=*/false);
```

* Total **edges** = `2 * number` (ON + OFF)
* Base half-period = `duration / edges`
* **Remainder** (`duration % edges`) is distributed by adding +1 ms to the first remainder edges, ensuring the total elapsed time equals `duration` (applies to blocking and non-blocking). 

In **non-blocking** mode, call `blinkUpdate()` regularly (e.g., in `loop()`).

### 2) **Infinite Blink (new)**

```cpp
// number == 0 -> infinite; duration == half-period.
led.blink(/*halfPeriod_ms=*/1000, /*number=*/0, /*blocking=*/false);
```

* `number == 0` enables **infinite** non-blocking blinking
* `duration` is interpreted as a **half-period** (time spent ON or OFF)
* Use `blinkUpdate()` in your `loop()` to progress the state machine
* Call `stopBlink(true)` to end infinite blinking and turn the LED OFF

> فارسی: در حالت بی‌نهایت، پارامتر `duration` یعنی **نیم‌تناوب**. برای توقف از `stopBlink()` استفاده کنید. این حالت همیشه **نان‌بلوکینگ** است.

---

## PWM Mode & Brightness

Enable PWM if you need 8-bit color or global brightness:

```cpp
led.enablePWM(true);        // enable PWM path
led.setBrightness(200);     // 0..255 global scale
led.setRGB(255, 128, 0);    // 8-bit per channel; digital ON/OFF when PWM disabled
```

* In PWM mode, CA wiring inverts duty internally; CC writes the duty as-is. 
* Default backend is `analogWrite`; override by defining `RGBLED_ANALOG_WRITE(pin, val)` **before** including `RGBLED.h` (e.g., map to ESP32 LEDC). 

---

## Error Handling

* `lastError` uses `RGBLED_Error` (`RGBLED_OK`, `RGBLED_ERR_PARAMS`)
* `RGBLED::errorText(err)` returns a PROGMEM message (`F("...")`) safe for AVR printing 

Example:

```cpp
if (!led.init()) {
  Serial.println(RGBLED::errorText(led.lastError));
}
```

---

## Examples

### 1) Infinite Blue Blink — 1 s ON / 1 s OFF

```cpp
led.blue();
led.blink(1000, 0, false); // infinite; duration = half-period
```

### 2) Stop Infinite Blink and Leave LED OFF

```cpp
if (/*some condition*/) {
  led.stopBlink(true);      // stop + force OFF
}
```

### 3) Finite, Exact-Duration Non-Blocking Blink (1200 ms total, 4 cycles)

```cpp
led.purple();
led.blink(1200, 4, false);
while (led.isBlinking()) {
  led.blinkUpdate();
}
```

---

## Doxygen Docs

This library is fully annotated. To generate HTML:

1. `sudo apt-get install doxygen graphviz`
2. `doxygen -g` then set:

   * `PROJECT_NAME = "RGBLED"`
   * `EXTRACT_ALL = YES`
   * `RECURSIVE = YES`
3. Run `doxygen`

Most details are documented at the source with `@file`, `@class`, `@enum`, and method comments. 

---

## Migration Notes

From v1.0 → **v1.1**:

* **New:** `blink(duration, 0, false)` = **infinite** non-blocking blink, where `duration` is **half-period**.
* **Unchanged:** Finite blink semantics and exact total duration via remainder distribution.
* **Unchanged:** PWM path (`enablePWM`, `setRGB`, `setBrightness`), error codes, and glitch-free init.

If you prefer the old behavior (no infinite mode), just avoid passing `number == 0`.

---

## Changelog

* **v1.1** — Infinite blink support (`number == 0`, `duration = half-period`); docs clarified; examples updated.
* **v1.0** — Deterministic blink with remainder distribution; PWM path; glitch-free init; error codes. 

---

## License

MIT (recommended). Add license text here.

---

### Acknowledgements

* Designed for clarity and determinism on small MCUs (no dynamic allocation in the core path).
* The **glitch-free init** trick prevents power-up flashes. 

```
