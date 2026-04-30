#include <Arduino.h>
#include "Parametros.h"
#include "Blink.h"

void Blink_LED(){
  digitalWrite(Pin_LED, HIGH);
  delay(Tiempo);
  digitalWrite(Pin_LED, LOW);
  delay(Tiempo);
}