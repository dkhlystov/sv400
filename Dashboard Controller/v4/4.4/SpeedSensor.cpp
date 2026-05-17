#include "Arduino.h"
#include "SpeedSensor.h"

SpeedSensor::SpeedSensor(int pin) {
  // Defaults
  _magnetCount = 4;
  _tireWidth = 120;
  _tireAspectRatio = 60;
  _tireRimSize = 17;
  _value = 0;

  // Init
  calcRate();
  pinMode(pin, INPUT_PULLUP);
  _pin = pin;
  _pinValue = digitalRead(_pin);
  _time = micros();
}

int SpeedSensor::getMagnetCount() {
  return _magnetCount;
}

void SpeedSensor::setMagnetCount(int value) {
  if (value < 1) {
    value = 1;
  }
  if (_magnetCount != value) {
    _magnetCount = value;
    calcRate();
  }
}

int SpeedSensor::getTireWidth() {
  return _tireWidth;
}

void SpeedSensor::setTireWidth(int value) {
  if (value < 0) {
    value = 0;
  }
  if (_tireWidth != value) {
    _tireWidth = value;
    calcRate();
  }
}

int SpeedSensor::getTireAspectRatio() {
  return _tireAspectRatio;
}

void SpeedSensor::setTireAspectRatio(int value) {
  if (value < 0) {
    value = 0;
  }
  if (_tireAspectRatio != value) {
    _tireAspectRatio = value;
    calcRate();
  }
}

int SpeedSensor::getTireRimSize() {
  return _tireRimSize;
}

void SpeedSensor::setTireRimSize(int value) {
  if (value < 0) {
    value = 0;
  }
  if (_tireRimSize != value) {
    _tireRimSize = value;
    calcRate();
  }
}

int SpeedSensor::getValue() {
  return _value;
}

void SpeedSensor::calcRate() {
  int wheelLength = round((_tireRimSize * 25.4 + _tireWidth * _tireAspectRatio / 100 * 2) * 3.1416);
  _rate = wheelLength * 3.6 / _magnetCount / 1000;
}

void SpeedSensor::loop() {
  unsigned long time = micros();
  int value = digitalRead(_pin);
  if (value != _pinValue) {
    // Calc period
    unsigned long period = (time - _time) * 2;
    // Calc speed
    int speed = round(1000000 / period * _rate);
    if (speed < 300) {
      _value = speed;
    }
    _time = time;
    _pinValue = value;
  } else if (_value > 0 && _time + 500000 < time) {
    _value = 0;
  }
}
