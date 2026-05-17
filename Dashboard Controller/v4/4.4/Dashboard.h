#ifndef Dashboard_h
#define Dashboard_h

#include "Arduino.h"

class Dashboard {
  public:
    Dashboard(int speedPin, int fuelPin1, int fuelPin2, int fuelPin3, int fuelPin4, int fuelPin5);

    int getSpeed();
    void setSpeed(int value);
    float getFuel();
    void setFuel(float value);

    void loop();
  private:
    int _speedPin;
    int _fuelPin1;
    int _fuelPin2;
    int _fuelPin3;
    int _fuelPin4;
    int _fuelPin5;

    float _speedRate;
    int _speed;
    float _fuel;

    int _speedPinValue;
    unsigned long _speedTime;

    void updateFuelLevel();
};

#endif
