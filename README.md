# iKY040

Library for **Arduino + ESP32**, focused on **M5Stack Core2 for AWS**, to read a **KY-040** rotary encoder using interrupt-driven quadrature decoding.

## Features

- Interrupt-based reading of **CLK** and **DT** channels.
- Optional **SW** button support.
- Software debounce for button input.
- Accumulated position tracking.
- Delta tracking since the last read.
- Rotation direction detection.
- Software direction inversion.
- Default pins chosen for convenient ESP32 wiring: **CLK = GPIO13**, **DT = GPIO14**, **SW = GPIO26**.

## Default wiring

> Always use **3.3 V**, not 5 V, to match the ESP32 logic level on Core2.

| KY-040 | Function    | Core2 / ESP32 default |
|--------|-------------|-----------------------|
| CLK    | Channel A   | GPIO13                |
| DT     | Channel B   | GPIO14                |
| SW     | Push button | GPIO26                |
| +      | Power       | 3V3                   |
| GND    | Ground      | GND                   |

## Why these default pins

**GPIO13** and **GPIO14** are used for encoder lines because both are standard ESP32 digital GPIOs: they support interrupts and `INPUT_PULLUP`, which usually provides a simpler and more robust connection than input-only pins such as GPIO36. For the **SW** button, **GPIO26** works well as a regular digital input.

## Installation

1. Download or copy the `iKY040` folder into your Arduino `libraries` folder.
2. Restart the Arduino IDE.
3. Open the `BasicRead` example.

## Basic usage with default pins

```cpp
#include <iKY040.h>

iKY040 encoder;  // CLK=13, DT=14, SW=26

void setup() {
  Serial.begin(115200);
  encoder.begin(true, 4);  // pull-up where supported, 4 transitions per detent
}

void loop() {
  encoder.update();

  int32_t delta = encoder.getAndResetDelta();
  if (delta != 0) {
    Serial.println(encoder.getPosition());
  }

  if (encoder.wasClicked()) {
    Serial.println("Click");
  }
}
```

## Custom pins

```cpp
#include <iKY040.h>

iKY040 encoder(32, 33, 25);
```

## Main API

### Constructor

```cpp
iKY040(uint8_t pinCLK = 13, uint8_t pinDT = 14, uint8_t pinSW = 26, bool direction = true);
```

- `pinCLK`: CLK channel GPIO.
- `pinDT`: DT channel GPIO.
- `pinSW`: button GPIO; use `iKY040::NO_PIN` if not used.
- `direction`: default is `true`; when enabled, reported rotation is inverted.

### Initialization

```cpp
bool begin(bool enableInternalPullups = true, uint8_t stepsPerDetent = 4);
```

- `enableInternalPullups`: enables `INPUT_PULLUP` on supported pins.
- `stepsPerDetent`: usually `4` for a standard KY-040.

### Movement

```cpp
int32_t getPosition();
int32_t getDelta();
int32_t getAndResetDelta();
bool available();
Direction getDirection();
Direction getAndClearDirection();
void setPosition(int32_t position);
void clearMovement();
```

### Button

```cpp
void update();
bool isPressed();
bool wasPressed();
bool wasReleased();
bool wasClicked();
void setButtonDebounce(uint16_t debounceMs);
```

> For button events (`wasPressed`, `wasReleased`, `wasClicked`), call `update()` on every `loop()` iteration.


## Without using the button

```cpp
#include <iKY040.h>

iKY040 encoder(13, 14, iKY040::NO_PIN);
```
