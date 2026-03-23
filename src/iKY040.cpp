#include "iKY040.h"

namespace {
static constexpr int8_t kTransitionTable[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
};
}

iKY040::iKY040(uint8_t pinCLK, uint8_t pinDT, uint8_t pinSW,
               bool direction)
    : _pinCLK(pinCLK),
      _pinDT(pinDT),
      _pinSW(pinSW),
      _direction(direction),
      _started(false),
      _position(0),
      _delta(0),
      _rawAccumulator(0),
      _lastDirection(None),
      _movementAvailable(false),
      _prevState(0),
      _stepsPerDetent(4),
      _mux(portMUX_INITIALIZER_UNLOCKED),
      _debounceMs(30),
      _buttonStableState(false),
      _lastButtonReading(false),
      _lastDebounceTime(0),
      _buttonPressedEvent(false),
      _buttonReleasedEvent(false),
      _buttonClickedEvent(false) {}

uint8_t iKY040::inputModeForPin(uint8_t pin, bool requestPullup) {
  if (!requestPullup) {
    return INPUT;
  }

#if defined(ESP32)
  if (pin >= 34 && pin <= 39) {
    return INPUT;
  }
#endif

  return INPUT_PULLUP;
}

bool iKY040::begin(bool enableInternalPullups, uint8_t stepsPerDetent) {
  if (stepsPerDetent == 0) {
    stepsPerDetent = 1;
  }

  _stepsPerDetent = stepsPerDetent;

  pinMode(_pinCLK, inputModeForPin(_pinCLK, enableInternalPullups));
  pinMode(_pinDT, inputModeForPin(_pinDT, enableInternalPullups));

  if (_pinSW != NO_PIN) {
    pinMode(_pinSW, inputModeForPin(_pinSW, enableInternalPullups));
    _buttonStableState = (digitalRead(_pinSW) == LOW);
    _lastButtonReading = _buttonStableState;
    _lastDebounceTime = millis();
    _buttonPressedEvent = false;
    _buttonReleasedEvent = false;
    _buttonClickedEvent = false;
  }

  uint8_t clk = static_cast<uint8_t>(digitalRead(_pinCLK));
  uint8_t dt = static_cast<uint8_t>(digitalRead(_pinDT));
  _prevState = static_cast<uint8_t>((clk << 1) | dt);

  attachInterruptArg(digitalPinToInterrupt(_pinCLK), encoderISR, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(_pinDT), encoderISR, this, CHANGE);

  _started = true;
  return true;
}

void iKY040::end() {
  if (!_started) {
    return;
  }

  detachInterrupt(digitalPinToInterrupt(_pinCLK));
  detachInterrupt(digitalPinToInterrupt(_pinDT));
  _started = false;
}

int32_t iKY040::getPosition() {
  portENTER_CRITICAL(&_mux);
  int32_t value = _position;
  portEXIT_CRITICAL(&_mux);
  return value;
}

void iKY040::setPosition(int32_t position) {
  portENTER_CRITICAL(&_mux);
  _position = position;
  _delta = 0;
  _rawAccumulator = 0;
  _movementAvailable = false;
  _lastDirection = None;
  portEXIT_CRITICAL(&_mux);
}

int32_t iKY040::getDelta() {
  portENTER_CRITICAL(&_mux);
  int32_t value = _delta;
  portEXIT_CRITICAL(&_mux);
  return value;
}

int32_t iKY040::getAndResetDelta() {
  portENTER_CRITICAL(&_mux);
  int32_t value = _delta;
  _delta = 0;
  _movementAvailable = false;
  portEXIT_CRITICAL(&_mux);
  return value;
}

bool iKY040::available() {
  portENTER_CRITICAL(&_mux);
  bool value = _movementAvailable;
  portEXIT_CRITICAL(&_mux);
  return value;
}

iKY040::Direction iKY040::getDirection() {
  portENTER_CRITICAL(&_mux);
  Direction value = static_cast<Direction>(_lastDirection);
  portEXIT_CRITICAL(&_mux);
  return value;
}

iKY040::Direction iKY040::getAndClearDirection() {
  portENTER_CRITICAL(&_mux);
  Direction value = static_cast<Direction>(_lastDirection);
  _lastDirection = None;
  portEXIT_CRITICAL(&_mux);
  return value;
}

void iKY040::clearMovement() {
  portENTER_CRITICAL(&_mux);
  _delta = 0;
  _movementAvailable = false;
  _lastDirection = None;
  portEXIT_CRITICAL(&_mux);
}

void iKY040::update() {
  if (_pinSW == NO_PIN) {
    return;
  }

  bool reading = (digitalRead(_pinSW) == LOW);
  uint32_t now = millis();

  if (reading != _lastButtonReading) {
    _lastDebounceTime = now;
    _lastButtonReading = reading;
  }

  if ((now - _lastDebounceTime) >= _debounceMs && reading != _buttonStableState) {
    _buttonStableState = reading;

    if (_buttonStableState) {
      _buttonPressedEvent = true;
    } else {
      _buttonReleasedEvent = true;
      _buttonClickedEvent = true;
    }
  }
}

bool iKY040::isPressed() {
  if (_pinSW == NO_PIN) {
    return false;
  }
  return digitalRead(_pinSW) == LOW;
}

bool iKY040::wasPressed() {
  bool value = _buttonPressedEvent;
  _buttonPressedEvent = false;
  return value;
}

bool iKY040::wasReleased() {
  bool value = _buttonReleasedEvent;
  _buttonReleasedEvent = false;
  return value;
}

bool iKY040::wasClicked() {
  bool value = _buttonClickedEvent;
  _buttonClickedEvent = false;
  return value;
}

void iKY040::setButtonDebounce(uint16_t debounceMs) {
  _debounceMs = debounceMs;
}

uint8_t iKY040::getStepsPerDetent() const {
  return _stepsPerDetent;
}

void iKY040::setStepsPerDetent(uint8_t steps) {
  if (steps == 0) {
    steps = 1;
  }
  portENTER_CRITICAL(&_mux);
  _stepsPerDetent = steps;
  _rawAccumulator = 0;
  portEXIT_CRITICAL(&_mux);
}

uint8_t iKY040::getPinCLK() const {
  return _pinCLK;
}

uint8_t iKY040::getPinDT() const {
  return _pinDT;
}

uint8_t iKY040::getPinSW() const {
  return _pinSW;
}

void IRAM_ATTR iKY040::encoderISR(void* arg) {
  if (arg == nullptr) {
    return;
  }
  static_cast<iKY040*>(arg)->handleEncoderISR();
}

void IRAM_ATTR iKY040::handleEncoderISR() {
  uint8_t clk = static_cast<uint8_t>(digitalRead(_pinCLK));
  uint8_t dt = static_cast<uint8_t>(digitalRead(_pinDT));
  uint8_t currentState = static_cast<uint8_t>((clk << 1) | dt);
  uint8_t tableIndex = static_cast<uint8_t>((_prevState << 2) | currentState);
  int8_t transition = kTransitionTable[tableIndex];
  _prevState = currentState;

  if (_direction) {
    transition = -transition;
  }

  if (transition == 0) {
    return;
  }

  portENTER_CRITICAL_ISR(&_mux);
  _rawAccumulator += transition;

  if (_rawAccumulator >= static_cast<int8_t>(_stepsPerDetent)) {
    _rawAccumulator = 0;
    ++_position;
    ++_delta;
    _lastDirection = Clockwise;
    _movementAvailable = true;
  } else if (_rawAccumulator <= -static_cast<int8_t>(_stepsPerDetent)) {
    _rawAccumulator = 0;
    --_position;
    --_delta;
    _lastDirection = CounterClockwise;
    _movementAvailable = true;
  }

  portEXIT_CRITICAL_ISR(&_mux);
}
