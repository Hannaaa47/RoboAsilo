//estoy probando tener el codigo mas estructurado
//la primera prueba sera configurar tres sensores ultrasonicos

#include "SensorUltrasonico.h"

// Crear instancias
SensorUltrasonico sensor1(2, 3);
SensorUltrasonico sensor2(4, 5);
SensorUltrasonico sensor3(6, 7);

void setup() {
  Serial.begin(9600);

  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
}

void loop() {
  Serial.print("Sensor 1: ");
  Serial.println(sensor1.medirDistancia());

  Serial.print("Sensor 2: ");
  Serial.println(sensor2.medirDistancia());

  Serial.print("Sensor 3: ");
  Serial.println(sensor3.medirDistancia());

  Serial.println("------------------");
  delay(500);
}