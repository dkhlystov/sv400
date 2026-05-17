#ifndef FuelSensorAnalog_h
#define FuelSensorAnalog_h

#include "Arduino.h"

class FuelSensorAnalog {
  public:
    FuelSensorAnalog(int pinOut, int pinIn);

    boolean isOn();
    int getRawData();
    int getThresholdLow();
    void setThresholdLow(int value);
    int getThresholdHigh();
    void setThresholdHigh(int value);
    float getTarget();
    void setTarget(float value);

    float getValue();

    void loop();
  private:
    int _pinOut;
    int _pinIn;

    unsigned long _duration;

    boolean _on;
    int _rawData;
    int _thresholdLow;
    int _thresholdHigh;
    float _target;

    unsigned long _time;
};

#endif
