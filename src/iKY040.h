#ifndef IKY040_H
#define IKY040_H

#include <Arduino.h>

/**
 * @file iKY040.h
 * @brief Interrupt-driven KY-040 rotary encoder library for ESP32 boards.
 */

/**
 * @brief Driver for KY-040 rotary encoders with optional push button support.
 *
 * This class uses interrupt-based quadrature decoding for CLK/DT and
 * software debouncing for the SW button (if connected).
 */
class iKY040 {
 public:
  /**
   * @brief Rotation direction reported by the decoder.
   */
  enum Direction : int8_t {
    /** @brief No movement detected since last reset/clear. */
    None = 0,
    /** @brief Positive rotation direction. */
    Clockwise = 1,
    /** @brief Negative rotation direction. */
    CounterClockwise = -1
  };

  /** @brief Default CLK pin for M5Stack Core2/ESP32 wiring. */
  static constexpr uint8_t DEFAULT_PIN_CLK = 13;
  /** @brief Default DT pin for M5Stack Core2/ESP32 wiring. */
  static constexpr uint8_t DEFAULT_PIN_DT = 14;
  /** @brief Default SW (button) pin for M5Stack Core2/ESP32 wiring. */
  static constexpr uint8_t DEFAULT_PIN_SW = 26;
  /** @brief Sentinel value used to indicate that SW is not connected. */
  static constexpr uint8_t NO_PIN = 255;

  /**
   * @brief Builds a KY-040 driver instance.
   * @param pinCLK GPIO connected to encoder CLK.
   * @param pinDT GPIO connected to encoder DT.
   * @param pinSW GPIO connected to encoder SW button, or NO_PIN to disable.
   * @param direction Set true to invert the reported rotation direction.
   */
  explicit iKY040(uint8_t pinCLK = DEFAULT_PIN_CLK,
                  uint8_t pinDT = DEFAULT_PIN_DT,
                  uint8_t pinSW = DEFAULT_PIN_SW,
                  bool direction = true);

  /**
   * @brief Initializes GPIO modes and attaches encoder interrupts.
   * @param enableInternalPullups Enables INPUT_PULLUP where supported.
   * @param stepsPerDetent Number of quadrature transitions per detent.
   * @return true when initialization completes.
   */
  bool begin(bool enableInternalPullups = true, uint8_t stepsPerDetent = 4);
  /**
   * @brief Stops the encoder and detaches interrupts.
   */
  void end();

  /**
   * @brief Returns the accumulated encoder position.
   * @return Signed position in detent units.
   */
  int32_t getPosition();
  /**
   * @brief Sets the logical position and clears movement tracking state.
   * @param position New logical position.
   */
  void setPosition(int32_t position);

  /**
   * @brief Returns the pending movement delta.
   * @return Signed delta since the last reset/clear.
   */
  int32_t getDelta();
  /**
   * @brief Returns delta and clears it atomically.
   * @return Signed delta since the previous reset.
   */
  int32_t getAndResetDelta();
  /**
   * @brief Indicates whether movement is available.
   * @return true when at least one detent was detected.
   */
  bool available();

  /**
   * @brief Returns the last detected movement direction.
   * @return Last direction, or None when no movement is pending.
   */
  Direction getDirection();
  /**
   * @brief Returns and clears the last detected direction.
   * @return Last direction before clearing.
   */
  Direction getAndClearDirection();
  /**
   * @brief Clears movement delta, availability flag, and last direction.
   */
  void clearMovement();

  /**
   * @brief Updates debounced button state and button events.
   *
   * Call this once per loop() when SW is connected.
   */
  void update();
  /**
   * @brief Returns the current raw button state.
   * @return true when button is currently pressed.
   */
  bool isPressed();
  /**
   * @brief Returns and clears the "pressed" edge event.
   * @return true if a press event was detected.
   */
  bool wasPressed();
  /**
   * @brief Returns and clears the "released" edge event.
   * @return true if a release event was detected.
   */
  bool wasReleased();
  /**
   * @brief Returns and clears the click event.
   * @return true when a complete press-release click was detected.
   */
  bool wasClicked();
  /**
   * @brief Sets software debounce time for the SW button.
   * @param debounceMs Debounce time in milliseconds.
   */
  void setButtonDebounce(uint16_t debounceMs);

  /**
   * @brief Returns the configured transitions per detent.
   * @return Number of transitions required to increment/decrement position.
   */
  uint8_t getStepsPerDetent() const;
  /**
   * @brief Updates transitions per detent.
   * @param steps Number of transitions per detent (minimum 1).
   */
  void setStepsPerDetent(uint8_t steps);

  /**
   * @brief Returns GPIO used for CLK.
   * @return CLK GPIO number.
   */
  uint8_t getPinCLK() const;
  /**
   * @brief Returns GPIO used for DT.
   * @return DT GPIO number.
   */
  uint8_t getPinDT() const;
  /**
   * @brief Returns GPIO used for SW.
   * @return SW GPIO number, or NO_PIN when button input is disabled.
   */
  uint8_t getPinSW() const;

 private:
  static void IRAM_ATTR encoderISR(void* arg);
  void IRAM_ATTR handleEncoderISR();
  static uint8_t inputModeForPin(uint8_t pin, bool requestPullup);

  uint8_t _pinCLK;
  uint8_t _pinDT;
  uint8_t _pinSW;
  bool _direction;
  bool _started;

  volatile int32_t _position;
  volatile int32_t _delta;
  volatile int8_t _rawAccumulator;
  volatile int8_t _lastDirection;
  volatile bool _movementAvailable;
  volatile uint8_t _prevState;

  uint8_t _stepsPerDetent;
  portMUX_TYPE _mux;

  uint16_t _debounceMs;
  bool _buttonStableState;
  bool _lastButtonReading;
  uint32_t _lastDebounceTime;
  bool _buttonPressedEvent;
  bool _buttonReleasedEvent;
  bool _buttonClickedEvent;
};


#endif
