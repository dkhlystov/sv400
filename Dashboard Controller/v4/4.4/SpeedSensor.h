#ifndef SpeedSensor_h
#define SpeedSensor_h

#include "Arduino.h"

class SpeedSensor {
  public:
    SpeedSensor(int pin);

    int getMagnetCount();
    void setMagnetCount(int value);
    int getTireWidth();
    void setTireWidth(int value);
    int getTireAspectRatio();
    void setTireAspectRatio(int value);
    int getTireRimSize();
    void setTireRimSize(int value);

    int getValue();

    void loop();
  private:
    int _pin;
    int _magnetCount;
    int _tireWidth;
    int _tireAspectRatio;
    int _tireRimSize;
    int _value;

    float _rate;
    int _pinValue;
    unsigned long _time;

    void calcRate();
};

#endif
