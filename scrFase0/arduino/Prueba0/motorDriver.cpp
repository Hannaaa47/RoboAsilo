#include "motorDriver.h"
#include <Arduino.h>
// ─────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────
MotorDriver::MotorDriver(uint8_t rpwm, uint8_t lpwm,
                         uint8_t ren,  uint8_t len,
                         uint8_t encA, uint8_t encB,
                         float pulsosRev) {
  _rpwm = rpwm;
  _lpwm = lpwm;
  _ren  = ren;
  _len  = len;
  _encA = encA;
  _encB = encB;

  _pulsosRevolucion = pulsosRev;

  // Estado inicial
  _velocidad     = 0;
  _habilitado    = false;
  _encoderCount  = 0;
  _lastEncoderCount = 0;

  // PID por defecto (ajustar en calibración)
  _kp = 1.0;
  _ki = 0.0;
  _kd = 0.0;
  _errorPrev   = 0;
  _integral    = 0;
  _targetRPM   = 0;
  _currentRPM  = 0;
  _lastPIDTime = 0;
}

// ─────────────────────────────────────────────────────
//  begin
// ─────────────────────────────────────────────────────
void MotorDriver::begin(void(*isrCallback)()) {
  pinMode(_rpwm, OUTPUT);
  pinMode(_lpwm, OUTPUT);
  pinMode(_ren,  OUTPUT);
  pinMode(_len,  OUTPUT);
  pinMode(_encA, INPUT_PULLUP);
  pinMode(_encB, INPUT_PULLUP);

  analogWrite(_rpwm, 0);
  analogWrite(_lpwm, 0);
  digitalWrite(_ren, LOW);
  digitalWrite(_len, LOW);

  // Attach interrupt si se pasó una ISR
  if (isrCallback != nullptr) {
    attachInterrupt(digitalPinToInterrupt(_encA), isrCallback, CHANGE);
  }

  // Habilitar driver por defecto
  habilitar(true);

  // PID con valores por defecto
  setPID(1.5, 0.1, 0.05);

  _habilitado = true;
}

// ─────────────────────────────────────────────────────
//  habilitar / deshabilitar driver
// ─────────────────────────────────────────────────────
void MotorDriver::habilitar(bool estado) {
  _habilitado = estado;
  digitalWrite(_ren, estado ? HIGH : LOW);
  digitalWrite(_len, estado ? HIGH : LOW);

  if (!estado) {
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, 0);
    _velocidad = 0;
  }
}

bool MotorDriver::estaHabilitado() {
  return _habilitado;
}

// ─────────────────────────────────────────────────────
//  _aplicarPWM  (privado)
//  velocidad: -255 (reversa máx) a 255 (adelante máx)
// ─────────────────────────────────────────────────────
void MotorDriver::_aplicarPWM(int16_t velocidad) {
  // Clamp al rango válido
  velocidad = constrain(velocidad, -MOT_PWM_MAX, MOT_PWM_MAX);

  // Zona muerta: si es muy bajo no aplica para no dañar motores
  if (abs(velocidad) < MOT_PWM_MINIMO && velocidad != 0) {
    velocidad = (velocidad > 0) ? MOT_PWM_MINIMO : -MOT_PWM_MINIMO;
  }

  if (velocidad > 0) {
    // Adelante
    analogWrite(_rpwm, velocidad);
    analogWrite(_lpwm, 0);
  } else if (velocidad < 0) {
    // Reversa
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, abs(velocidad));
  } else {
    // Detener
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, 0);
  }
}

// ─────────────────────────────────────────────────────
//  setVelocidad  - control directo sin PID
// ─────────────────────────────────────────────────────
void MotorDriver::setVelocidad(int16_t velocidad) {
  if (!_habilitado) return;
  _velocidad = constrain(velocidad, -MOT_PWM_MAX, MOT_PWM_MAX);
  _aplicarPWM(_velocidad);
}

int16_t MotorDriver::getVelocidad() {
  return _velocidad;
}

// ─────────────────────────────────────────────────────
//  Freno activo vs detener libre
// ─────────────────────────────────────────────────────
void MotorDriver::frenar() {
  // Freno activo: ambos canales en HIGH bloquean el motor
  analogWrite(_rpwm, MOT_PWM_MAX);
  analogWrite(_lpwm, MOT_PWM_MAX);
  _velocidad = 0;
}

void MotorDriver::detener() {
  // Deja el motor rodar libre
  analogWrite(_rpwm, 0);
  analogWrite(_lpwm, 0);
  _velocidad = 0;
}

// ─────────────────────────────────────────────────────
//  Encoder
// ─────────────────────────────────────────────────────

// Llamar desde ISR externa en el .ino
// Ejemplo:
//   void ISR_EncIzq() { motorIzq.contarEncoder(); }
//   attachInterrupt(digitalPinToInterrupt(PIN_ENC_IZQ_A), ISR_EncIzq, CHANGE);
void MotorDriver::contarEncoder() {
  // Leer canal B para determinar direccion
  bool direccion = digitalRead(_encB);
  if (direccion) {
    _encoderCount++;
  } else {
    _encoderCount--;
  }
}

long MotorDriver::getEncoderCount() {
  return _encoderCount;
}

void MotorDriver::resetEncoder() {
  _encoderCount = 0;
  _lastEncoderCount = 0;
}

float MotorDriver::getRPM() {
  return _currentRPM;
}

// ─────────────────────────────────────────────────────
//  PID de velocidad
// ─────────────────────────────────────────────────────
void MotorDriver::setPID(float kp, float ki, float kd) {
  _kp = kp;
  _ki = ki;
  _kd = kd;
  // Resetear estado del PID al cambiar ganancias
  _errorPrev = 0;
  _integral  = 0;
}

void MotorDriver::setTargetRPM(float rpm) {
  _targetRPM = rpm;
}

// Llamar en cada loop() para control PID
void MotorDriver::updatePID() {
  if (!_habilitado) return;

  unsigned long ahora = millis();
  float dt = (ahora - _lastPIDTime) / 1000.0;  // segundos

  // Evitar dt=0 o muy grande en primer ciclo
  if (dt < 0.01 || dt > 0.5) {
    _lastPIDTime = ahora;
    _lastEncoderCount = _encoderCount;
    return;
  }

  // Calcular RPM actual
  long deltaPulsos = _encoderCount - _lastEncoderCount;
  _currentRPM = (deltaPulsos / _pulsosRevolucion) * (60.0 / dt);
  _lastEncoderCount = _encoderCount;
  _lastPIDTime = ahora;

  // Calcular error
  float error = _targetRPM - _currentRPM;

  // Términos PID
  _integral += error * dt;
  _integral = constrain(_integral, -200, 200);  // Anti-windup

  float derivada = (error - _errorPrev) / dt;
  _errorPrev = error;

  float salida = (_kp * error) + (_ki * _integral) + (_kd * derivada);

  // Convertir salida PID a PWM (-255 a 255)
  int16_t pwm = constrain((int16_t)salida, -MOT_PWM_MAX, MOT_PWM_MAX);
  _aplicarPWM(pwm);
}