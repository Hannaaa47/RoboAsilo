#include "Parametros.h"
#include "Gpio.h"
#include "Blink.h"

int Tiempo = 0; 

void setup() {
  gpio_init();
  Tiempo = 100 * Tiempo_Blink;

}

void loop() {
  Blink_LED();
  

}
