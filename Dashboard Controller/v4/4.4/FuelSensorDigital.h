#ifndef FuelSensorDigital_h
#define FuelSensorDigital_h

#include "Arduino.h"

class FuelSensorDigital {
  public:
    FuelSensorDigital(int pin);

    int getRawData();
    int getEmptyValue();
    void setEmptyValue(int value);
    int getFullValue();
    void setFullValue(int value);

    float getValue();

    void loop();
  private:
    int _pin;
    int _rawData;
    int _emptyValue;
    int _fullValue;

    float _value;

    int _pinValue;
    unsigned long _time;
};

#endif
