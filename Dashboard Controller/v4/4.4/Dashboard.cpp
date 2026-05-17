#include "Arduino.h"
#include "Dashboard.h"

Dashboard::Dashboard(int speedPin, int fuelPin1, int fuelPin2, int fuelPin3, int fuelPin4, int fuelPin5) {
  // Defaults
  _speedRate = 0.754;
  _speed = 0;
  _fuel = 0.0;

  // Init
  pinMode(speedPin, OUTPUT);
  pinMode(fuelPin1, OUTPUT);
  pinMode(fuelPin2, OUTPUT);
  pinMode(fuelPin3, OUTPUT);
  pinMode(fuelPin4, OUTPUT);
  pinMode(fuelPin5, OUTPUT);
  digitalWrite(speedPin, LOW);
  digitalWrite(fuelPin1, LOW);
  digitalWrite(fuelPin2, LOW);
  digitalWrite(fuelPin3, LOW);
  digitalWrite(fuelPin4, LOW);
  digitalWrite(fuelPin5, LOW);
  _speedPin = speedPin;
  _fuelPin1 = fuelPin1;
  _fuelPin2 = fuelPin2;
  _fuelPin3 = fuelPin3;
  _fuelPin4 = fuelPin4;
  _fuelPin5 = fuelPin5;

  _speedPinValue = 0;
  _speedTime = micros();
}

int Dashboard::getSpeed() {
  return _speed;
}

void Dashboard::setSpeed(int value) {
  if (value < 0) {
    value = 0;
  } else if (value > 300) {
    value = 300;
  }
  if (_speed != value) {
    _speed = value;
  }
}

float Dashboard::getFuel() {
  return _fuel;
}

void Dashboard::setFuel(float value) {
  if (value < 0) {
    value = 0;
  } else if (value > 1) {
    value = 1;
  }
  if (_fuel != value) {
    _fuel = value;
    updateFuelLevel();
  }
}

void Dashboard::updateFuelLevel() {
  float level = round(_fuel * 5);
  if (level > 0) digitalWrite(_fuelPin1, HIGH);
  else digitalWrite(_fuelPin1, LOW);
  if (level > 1) digitalWrite(_fuelPin2, HIGH);
  else digitalWrite(_fuelPin2, LOW);
  if (level > 2) digitalWrite(_fuelPin3, HIGH);
  else digitalWrite(_fuelPin3, LOW);
  if (level > 3) digitalWrite(_fuelPin4, HIGH);
  else digitalWrite(_fuelPin4, LOW);
  if (level > 4) digitalWrite(_fuelPin5, HIGH);
  else digitalWrite(_fuelPin5, LOW);
}

void Dashboard::loop() {
  unsigned long time = micros();
  // Dashboard out
  if (_speed > 0) {
    // Calc period
    unsigned long period = 1000000 / (_speed * _speedRate) / 2;
    // Check timestemp
    if (_speedTime + period < time) {
      _speedTime = time;
      _speedPinValue = 1 - _speedPinValue;
      digitalWrite(_speedPin, _speedPinValue);
    }
  }
}
