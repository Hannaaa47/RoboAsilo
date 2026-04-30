#include "SensorUltrasonico.h"
#include <Arduino.h>
//  “si uso funciones o tipos de Arduino fuera del .ino → necesito Arduino.h”
SensorUltrasonico::SensorUltrasonico(int trig, int echo) {
  trigPin = trig;
  echoPin = echo;
}

void SensorUltrasonico::begin() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float SensorUltrasonico::medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.034 / 2;

  return distancia;
}