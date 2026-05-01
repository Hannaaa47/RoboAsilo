#include "ultrasonico.h"
#include <Arduino.h>

Ultrasonico::Ultrasonico(uint8_t trig, uint8_t echo) {
  _trigPin   = trig;
  _echoPin   = echo;
  _distancia = 0;
  _estado    = UTS_IDLE;
  _lastTrigger = 0;
}

void Ultrasonico::begin() {
  pinMode(_trigPin, OUTPUT);
  pinMode(_echoPin, INPUT);
  digitalWrite(_trigPin, LOW);
}

// Se llama en cada ciclo del loop()
void Ultrasonico::update() {
  unsigned long ahora = micros();

  switch (_estado) {

    case UTS_IDLE:
      // Esperar intervalo entre mediciones
      if (ahora - _lastTrigger >= INTERVALO_US) {
        digitalWrite(_trigPin, LOW);
        _lastTrigger = ahora;
        _estado = UTS_TRIG_LOW;
      }
      break;

    case UTS_TRIG_LOW:
      // Mantener LOW por 2us antes del pulso
      if (ahora - _lastTrigger >= 2) {
        digitalWrite(_trigPin, HIGH);
        _lastTrigger = ahora;
        _estado = UTS_TRIG_HIGH;
      }
      break;

    case UTS_TRIG_HIGH:
      // Pulso HIGH de 10us
      if (ahora - _lastTrigger >= 10) {
        digitalWrite(_trigPin, LOW);
        _lastTrigger = ahora;
        _estado = UTS_ESPERANDO_ECHO;
      }
      break;

    case UTS_ESPERANDO_ECHO:
      // Esperar flanco de subida del eco
      if (digitalRead(_echoPin) == HIGH) {
        _lastTrigger = ahora;
        _estado = UTS_MIDIENDO;
      }
      // Timeout si no llega eco
      if (ahora - _lastTrigger >= TIMEOUT_US) {
        _distancia = -1;  // Sin objeto detectado
        _estado = UTS_IDLE;
      }
      break;

    case UTS_MIDIENDO:
      // Esperar flanco de bajada del eco
      if (digitalRead(_echoPin) == LOW) {
        long duracion = ahora - _lastTrigger;
        _distancia = duracion * 0.034 / 2.0;
        _estado = UTS_IDLE;
        _lastTrigger = ahora;
      }
      // Timeout si el pulso es demasiado largo
      if (ahora - _lastTrigger >= TIMEOUT_US) {
        _distancia = -1;
        _estado = UTS_IDLE;
      }
      break;
  }
}

float Ultrasonico::getDistancia() {
  return _distancia;
}

bool Ultrasonico::hayObstaculo(float umbralCm) {
  return (_distancia > 0 && _distancia <= umbralCm);
}