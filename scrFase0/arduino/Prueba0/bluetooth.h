#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

class Bluetooth {
  public:
    Bluetooth(HardwareSerial& puerto);

    void begin(long baud = 9600);

    bool disponible();
    char leer();

    void enviar(const char* msg);
    void enviarLinea(const char* msg);

    void enviarFlash(const __FlashStringHelper* msg);
    void enviarLineaFlash(const __FlashStringHelper* msg);

  private:
    HardwareSerial* _serial;
};

#endif