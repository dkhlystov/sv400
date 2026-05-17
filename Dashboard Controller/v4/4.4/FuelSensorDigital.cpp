#include "Arduino.h"
#include "FuelSensorDigital.h"

FuelSensorDigital::FuelSensorDigital(int pin) {
  // Defaults
  _rawData = 0;
  _emptyValue = 10;
  _fullValue = 20;
  _value = 0;

  // Init
  pinMode(pin, INPUT_PULLUP);
  _pin = pin;
  _pinValue = digitalRead(_pin);
  _time = micros();
}

int FuelSensorDigital::getRawData() {
  return _rawData;
}

int FuelSensorDigital::getEmptyValue() {
  return _emptyValue;
}

void FuelSensorDigital::setEmptyValue(int value) {
  if (value < 0) {
    value = 0;
  }
  if (_emptyValue != value) {
    _emptyValue = value;
  }
}

int FuelSensorDigital::getFullValue() {
  return _fullValue;
}

void FuelSensorDigital::setFullValue(int value) {
  if (value < 0) {
    value = 0;
  }
  if (_fullValue != value) {
    _fullValue = value;
  }
}

float FuelSensorDigital::getValue() {
  return _value;
}

void FuelSensorDigital::loop() {
  unsigned long time = micros();
  int value = digitalRead(_pin);

  // Calc freq
  int freq = _rawData;
  if (value != _pinValue) {
    _pinValue = value;
    if (value) {
      // Period
      unsigned long period = (time - _time);
      // Freq
      freq = round(1000000 / period);
      _time = time;
    }
  } else if ((freq > 0) && (_time + 1000000 < time)) {
    freq = 0;
  }

  // Calc value
  if (_rawData != freq) {
    _rawData = freq;
    if (_fullValue == _emptyValue) {
      _value = 0;
    } else {
      _value = (_rawData - _emptyValue) / (_fullValue - _emptyValue);
    }
    if (_value < 0) {
      _value = 0;
    } else if (_value > 1) {
      _value = 1;
    }
  }
}
