#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// ── Filtro complementario ────────────────────────────
// 0.96 = 96% giroscopio, 4% acelerómetro
#define IMU_ALPHA           0.96

// Intervalo mínimo entre lecturas en ms
#define IMU_INTERVALO_MS    10

// Umbral para detectar inclinación peligrosa (grados)
#define IMU_UMBRAL_CAIDA    45.0

class IMU_Sensor {
  private:
    Adafruit_MPU6050 _mpu;

    // Ángulos filtrados finales
    float _pitch;
    float _roll;
    float _yaw;

    // Offsets del giroscopio (calibración)
    float _gyroOffsetX;
    float _gyroOffsetY;
    float _gyroOffsetZ;

    // Control de tiempo
    unsigned long _lastUpdate;
    bool _inicializado;

    void _calcularOffsets(int muestras = 200);

  public:
    IMU_Sensor();

    // Regresa false si no se detecta el sensor
    bool begin();

    // Llamar en cada loop()
    void update();

    // Ángulos en grados
    float getPitch();
    float getRoll();
    float getYaw();

    // Para la FSM
    bool estaInclinado(float umbral = IMU_UMBRAL_CAIDA);
    bool estaEstable();

    void resetYaw();
    bool estaInicializado();
};

#endif