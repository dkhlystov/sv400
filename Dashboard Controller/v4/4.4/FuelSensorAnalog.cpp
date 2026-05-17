#include "Arduino.h"
#include "FuelSensorAnalog.h"

FuelSensorAnalog::FuelSensorAnalog(int pinOut, int pinIn) {
  // Defaults
  _on = false;
  _thresholdLow = 500;
  _thresholdHigh = 500;
  _target = 0.2;

  _duration = 500;

  // Init
  pinMode(pinOut, OUTPUT);
  pinMode(pinIn, INPUT);
  _pinOut = pinOut;
  _pinIn = pinIn;
  digitalWrite(_pinOut, LOW);
  _time = millis();
}

boolean FuelSensorAnalog::isOn() {
  return _on;
}

int FuelSensorAnalog::getRawData() {
  return _rawData;
}

int FuelSensorAnalog::getThresholdLow() {
  return _thresholdLow;
}

void FuelSensorAnalog::setThresholdLow(int value) {
  if (value < 0) {
    value = 0;
  } else if (value > 1023) {
    value = 1023;
  }
  if (_thresholdLow != value) {
    _thresholdLow = value;
    if (_thresholdHigh < value) {
      _thresholdHigh = value;
    }
  }
}

int FuelSensorAnalog::getThresholdHigh() {
  return _thresholdHigh;
}

void FuelSensorAnalog::setThresholdHigh(int value) {
  if (value < 0) {
    value = 0;
  } else if (value > 1023) {
    value = 1023;
  }
  if (_thresholdHigh != value) {
    _thresholdHigh = value;
    if (_thresholdLow > value) {
      _thresholdLow = value;
    }
  }
}

float FuelSensorAnalog::getTarget() {
  return _target;
}

void FuelSensorAnalog::setTarget(float value) {
  if (value < 0) {
    value = 0;
  } else if (value > 1) {
    value = 1;
  }
  if (_target != value) {
    _target = value;
  }
}

float FuelSensorAnalog::getValue() {
  return _on ? _target : 1;
}

void FuelSensorAnalog::loop() {
  if (_time + _duration < millis()) {
    _time = millis();

    digitalWrite(_pinOut, HIGH);
    _rawData = analogRead(_pinIn);
    if (_rawData < _thresholdLow) {
      digitalWrite(_pinOut, LOW);
    }

    if (_on) {
      if (_rawData > _thresholdHigh) {
        _on = false;
      }
    } else {
      if (_rawData < _thresholdLow) {
        _on = true;
      }
    }
  }
}
