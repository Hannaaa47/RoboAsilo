#ifndef SENSOR_ULTRASONICO_H
#define SENSOR_ULTRASONICO_H

#include <Arduino.h>

class SensorUltrasonico {
  private:
    int trigPin;
    int echoPin;

  public:
    SensorUltrasonico(int trig, int echo);
    void begin();
    float medirDistancia();
};

#endif