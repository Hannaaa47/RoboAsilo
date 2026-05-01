// ultrasonico.h
#ifndef _ULTRASONICO_H_
#define _ULTRASONICO_H_

#include <Arduino.h>

#define INTERVALO_US  60000UL  // 60ms entre mediciones
#define TIMEOUT_US    23200UL  // ~4 metros máximo

class Ultrasonico {
  private:
    uint8_t _trigPin, _echoPin; 
    float   _distancia;
    unsigned long _lastTrigger;

    enum Estado {
      UTS_IDLE,
      UTS_TRIG_LOW,
      UTS_TRIG_HIGH,
      UTS_ESPERANDO_ECHO,
      UTS_MIDIENDO
    };
    Estado _estado;

  public:
    Ultrasonico(uint8_t trig, uint8_t echo);
    void  begin();
    void  update();        
    float getDistancia();
    bool  hayObstaculo(float umbralCm = 20.0);
};

#endif