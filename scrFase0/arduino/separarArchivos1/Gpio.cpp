#include <Arduino.h> //para trabajar los archivos por separado
#include "Parametros.h"

void gpio_init (){
  pinMode(Pin_LED, OUTPUT);
  digitalWrite(Pin_LED, LOW);
}