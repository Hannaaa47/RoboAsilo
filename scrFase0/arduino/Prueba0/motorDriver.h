#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

// ── Límites de velocidad ────────────────────────────
#define MOT_PWM_MAX     255
#define MOT_PWM_MIN     0
#define MOT_PWM_MINIMO  40    // Por debajo de esto el motor no gira

class MotorDriver {
  private:
    // Pines BTS7960
    uint8_t _rpwm;   // PWM hacia adelante
    uint8_t _lpwm;   // PWM hacia atrás
    uint8_t _ren;    // Enable derecha
    uint8_t _len;    // Enable izquierda

    // Estado interno
    int16_t _velocidad;       // -255 a 255 (negativo = reversa)
    bool    _habilitado;

    // Pines de encoder
    uint8_t _encA;
    uint8_t _encB;

    // Conteo de encoder (volatile porque se modifica en ISR)
    volatile long _encoderCount;

    // PID velocidad
    float _kp, _ki, _kd;
    float _errorPrev;
    float _integral;
    float _targetRPM;
    float _currentRPM;
    unsigned long _lastPIDTime;
    long  _lastEncoderCount;

    // Constantes encoder
    float _pulsosRevolucion;  // Pulsos por revolucion del encoder

    void _aplicarPWM(int16_t velocidad);

  public:
    // Constructor: pines del driver + pines encoder
    MotorDriver(uint8_t rpwm, uint8_t lpwm,
                uint8_t ren,  uint8_t len,
                uint8_t encA, uint8_t encB,
                float pulsosRev = 360.0);

    void begin(void(*isrCallback)() = nullptr);

    // Control directo (-255 a 255)
    void  setVelocidad(int16_t velocidad);

    // Control por RPM con PID
    void  setTargetRPM(float rpm);
    void  setPID(float kp, float ki, float kd);
    void  updatePID();          // Llamar en cada loop()

    // Encoder
    void  contarEncoder();      // Llamar desde ISR externa
    long  getEncoderCount();
    void  resetEncoder();
    float getRPM();

    // Utilidades
    void  frenar();             // Freno activo (ambos PWM en HIGH)
    void  detener();            // Apagar PWM (deja rodar libre)
    void  habilitar(bool estado);
    bool  estaHabilitado();
    int16_t getVelocidad();
};

#endif