#include "imu.h"

// ─────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────
IMU_Sensor::IMU_Sensor() {
  _pitch        = 0.0;
  _roll         = 0.0;
  _yaw          = 0.0;
  _gyroOffsetX  = 0.0;
  _gyroOffsetY  = 0.0;
  _gyroOffsetZ  = 0.0;
  _lastUpdate   = 0;
  _inicializado = false;
}

// ─────────────────────────────────────────────────────
//  begin
// ─────────────────────────────────────────────────────
bool IMU_Sensor::begin() {
  if (!_mpu.begin()) {
    return false;
  }

  // Rango del acelerómetro: ±2g suficiente para movimiento normal de robot
  _mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

  // Rango del giroscopio: ±250°/s suficiente para giros del robot
  _mpu.setGyroRange(MPU6050_RANGE_250_DEG);

  // Filtro de ruido interno del chip: 21Hz filtra vibraciones de motores
  // Esta es la ventaja de Adafruit: un solo método hace lo que antes
  // requería escribir directamente en registros del sensor
  _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Calibrar giroscopio con robot quieto y plano
  _calcularOffsets(200);

  _lastUpdate   = millis();
  _inicializado = true;
  return true;
}

// ─────────────────────────────────────────────────────
//  _calcularOffsets (privado)
//  Promedia N muestras en reposo para eliminar deriva
//  El delay aquí es aceptable: solo corre una vez en begin()
// ─────────────────────────────────────────────────────
void IMU_Sensor::_calcularOffsets(int muestras) {
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < muestras; i++) {
    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);

    // g ya viene en rad/s directamente desde Adafruit
    sumX += g.gyro.x;
    sumY += g.gyro.y;
    sumZ += g.gyro.z;
    delay(5);
  }

  _gyroOffsetX = sumX / muestras;
  _gyroOffsetY = sumY / muestras;
  _gyroOffsetZ = sumZ / muestras;
}

// ─────────────────────────────────────────────────────
//  update — Filtro complementario
//
//  Adafruit entrega datos ya convertidos:
//  - acelerómetro en m/s²
//  - giroscopio  en rad/s
//
//  angulo = ALPHA*(angulo + gyro*dt) + (1-ALPHA)*acelerometro
//           └───────────────────────┘  └────────────────────┘
//               Giroscopio               Acelerómetro
//            preciso, pero deriva      ruidoso, pero absoluto
// ─────────────────────────────────────────────────────
void IMU_Sensor::update() {
  if (!_inicializado) return;

  unsigned long ahora = millis();
  float dt = (ahora - _lastUpdate) / 1000.0;

  if (dt < (IMU_INTERVALO_MS / 1000.0)) return;
  _lastUpdate = ahora;

  // Leer sensor — Adafruit ya convierte a unidades físicas
  sensors_event_t a, g, temp;
  _mpu.getEvent(&a, &g, &temp);

  // Quitar offset del giroscopio (rad/s)
  float gyroX = g.gyro.x - _gyroOffsetX;
  float gyroY = g.gyro.y - _gyroOffsetY;
  float gyroZ = g.gyro.z - _gyroOffsetZ;

  // Ángulos desde acelerómetro (m/s² → grados)
  // atan2 da la referencia absoluta contra la gravedad
  float pitchAcel = atan2(a.acceleration.y,
                    sqrt(a.acceleration.x * a.acceleration.x +
                         a.acceleration.z * a.acceleration.z)) * 180.0 / PI;

  float rollAcel  = atan2(-a.acceleration.x,
                    sqrt(a.acceleration.y * a.acceleration.y +
                         a.acceleration.z * a.acceleration.z)) * 180.0 / PI;

  // Filtro complementario
  // gyro está en rad/s → convertir a grados/s multiplicando por (180/PI)
  _pitch = IMU_ALPHA * (_pitch + gyroX * (180.0/PI) * dt) + (1.0 - IMU_ALPHA) * pitchAcel;
  _roll  = IMU_ALPHA * (_roll  + gyroY * (180.0/PI) * dt) + (1.0 - IMU_ALPHA) * rollAcel;

  // Yaw solo giroscopio — deriva con el tiempo, útil para maniobras cortas
  _yaw += gyroZ * (180.0/PI) * dt;
}

// ─────────────────────────────────────────────────────
//  Getters
// ─────────────────────────────────────────────────────
float IMU_Sensor::getPitch() { return _pitch; }
float IMU_Sensor::getRoll()  { return _roll;  }
float IMU_Sensor::getYaw()   { return _yaw;   }

void IMU_Sensor::resetYaw() { _yaw = 0.0; }

bool IMU_Sensor::estaInicializado() { return _inicializado; }

// ─────────────────────────────────────────────────────
//  Utilidades para la FSM
// ─────────────────────────────────────────────────────
bool IMU_Sensor::estaInclinado(float umbral) {
  return (abs(_pitch) > umbral || abs(_roll) > umbral);
}

bool IMU_Sensor::estaEstable() {
  return (abs(_pitch) < 5.0 && abs(_roll) < 5.0);
}