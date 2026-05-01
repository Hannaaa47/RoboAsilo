#include "Bluetooth.h"

Bluetooth::Bluetooth(HardwareSerial& puerto) {
  _serial = &puerto;
}

void Bluetooth::begin(long baud) {
  _serial->begin(baud);
}

bool Bluetooth::disponible() {
  return _serial->available();
}

char Bluetooth::leer() {
  return _serial->read();
}

void Bluetooth::enviar(const char* msg) {
  _serial->print(msg);
}

void Bluetooth::enviarLinea(const char* msg) {
  _serial->println(msg);
}

void Bluetooth::enviarFlash(const __FlashStringHelper* msg) {
  _serial->print(msg);
}

void Bluetooth::enviarLineaFlash(const __FlashStringHelper* msg) {
  _serial->println(msg);
}