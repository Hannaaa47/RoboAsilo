//primera prueba 
// ═══════════════════════════════════════════════════════════
//  robot_asilo.ino
//  Archivo principal - Modo de prueba por componente
//
//  Comandos Serial (9600 baud):
//    '1' → TEST_MOTORES
//    '2' → TEST_ULTRASONICOS
//    '3' → TEST_IMU
//    '4' → TEST_ENCODERS
//    '0' → IDLE (detener todo)
//
//  Subcomandos en TEST_MOTORES:
//    'w' → adelante
//    's' → atrás
//    'a' → girar izquierda
//    'd' → girar derecha
//    'f' → freno
//    '+' → aumentar velocidad
//    '-' → disminuir velocidad
// ═══════════════════════════════════════════════════════════

#include "config.h"
#include "motorDriver.h"
#include "ultrasonico.h"
//#include "imu.h"
#include "Bluetooth.h"

// ── Objetos globales ─────────────────────────────────────
MotorDriver motorIzq(PIN_MOT_IZQ_RPWM, PIN_MOT_IZQ_LPWM,
                     PIN_MOT_IZQ_R,  PIN_MOT_IZQ_L,
                     PIN_ENC_IZQ_A,    PIN_ENC_IZQ_B);

MotorDriver motorDer(PIN_MOT_DER_RPWM, PIN_MOT_DER_LPWM,
                     PIN_MOT_DER_R,  PIN_MOT_DER_L,
                     PIN_ENC_DER_A,    PIN_ENC_DER_B);

Ultrasonico utsFre(PIN_UTS_FRE_TRIG, PIN_UTS_FRE_ECHO);
Ultrasonico utsIzq(PIN_UTS_IZQ_TRIG, PIN_UTS_IZQ_ECHO);
Ultrasonico utsDer(PIN_UTS_DER_TRIG, PIN_UTS_DER_ECHO);

Bluetooth bt(Serial1);

//IMU_Sensor imu;

// ── ISRs de encoders (deben ser funciones globales) ───────
void ISR_EncIzq() { motorIzq.contarEncoder(); }
void ISR_EncDer() { motorDer.contarEncoder(); }

// ── Máquina de estados ────────────────────────────────────
enum EstadoPrueba {
  IDLE,
  TEST_MOTORES,
  TEST_ULTRASONICOS,
  TEST_IMU,
  TEST_ENCODERS
};

EstadoPrueba estadoActual = IDLE;

// ── Variables de control ──────────────────────────────────
int    velocidadActual  = 150;      // PWM inicial (0-255)
const int PASO_VEL      = 20;       // Incremento por '+'/'-'

// Timers no bloqueantes
unsigned long tUltrasonico = 0;
unsigned long tIMU         = 0;
unsigned long tEncoder     = 0;

const unsigned long INTERVALO_PRINT_MS = 300;

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  while (!Serial);  // Esperar conexión serial (útil en Mega)

  // bluetooth
  bt.begin(9600);
  bt.enviarLineaFlash(F("Hola bb"));
  
  // ── Inicializar motores ──────────────────────────────────
  motorIzq.begin(ISR_EncIzq);
  motorDer.begin(ISR_EncDer);

  // Asegurarse que los motores inicien deshabilitados
  motorIzq.habilitar(false);
  motorDer.habilitar(false);

  // ── Inicializar ultrasónicos ─────────────────────────────
  utsFre.begin();
  utsIzq.begin();
  utsDer.begin();

/*
  // ── Inicializar IMU ──────────────────────────────────────
  Serial.println(F("Inicializando IMU... mantener robot quieto y plano"));
  if (!imu.begin()) {
    Serial.println(F("ERROR: MPU-6050 no detectado. Revisar conexion I2C."));
  } else {
    Serial.println(F("IMU OK"));
  }
*/

  imprimirMenu();
}

// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  unsigned long ahora = millis();

  // ── Actualizar sensores siempre ──────────────────────────
  utsFre.update();
  utsIzq.update();
  utsDer.update();
  //imu.update();
// ── Leer comando serial ──────────────────────────────────
/*
  if (Serial.available()) {
    char cmd = Serial.read();
    procesarComando(cmd);
  }
*/
  if (bt.disponible()) {
    char cmd = bt.leer();
    procesarComando(cmd);
  }

  // ── Máquina de estados ───────────────────────────────────
  switch (estadoActual) {

    // ── IDLE ──────────────────────────────────────────────
    case IDLE:
      // No hace nada, espera comando
      break;

    // ── TEST MOTORES ──────────────────────────────────────
    case TEST_MOTORES:
      // El control es por comandos w/a/s/d en procesarComando()
      // Aquí solo se imprime velocidad actual cada cierto tiempo
      if (ahora - tEncoder >= INTERVALO_PRINT_MS) {
        tEncoder = ahora;
        Serial.print(F("  RPM izq: ")); Serial.print(motorIzq.getRPM(), 1);
        Serial.print(F("  RPM der: ")); Serial.print(motorDer.getRPM(), 1);
        Serial.print(F("  PWM: "));    Serial.println(velocidadActual);
      }
      break;

    // ── TEST ULTRASONICOS ─────────────────────────────────
    case TEST_ULTRASONICOS:
      if (ahora - tUltrasonico >= INTERVALO_PRINT_MS) {
        tUltrasonico = ahora;
        Serial.print(F("  Frente: ")); Serial.print(utsFre.getDistancia(), 1); Serial.print(F(" cm"));
        Serial.print(F("  Izq: "));    Serial.print(utsIzq.getDistancia(), 1); Serial.print(F(" cm"));
        Serial.print(F("  Der: "));    Serial.print(utsDer.getDistancia(), 1); Serial.println(F(" cm"));
      }
      break;

    // ── TEST IMU ──────────────────────────────────────────
    case TEST_IMU:
    /*
      if (ahora - tIMU >= INTERVALO_PRINT_MS) {
        tIMU = ahora;
        Serial.print(F("  Pitch: ")); Serial.print(imu.getPitch(), 2);
        Serial.print(F("  Roll: "));  Serial.print(imu.getRoll(),  2);
        Serial.print(F("  Yaw: "));   Serial.print(imu.getYaw(),   2);
        Serial.print(F("  Estable: "));
        Serial.println(imu.estaEstable() ? F("SI") : F("NO"));
      }
    */
      break;

    // ── TEST ENCODERS ─────────────────────────────────────
    case TEST_ENCODERS:
      if (ahora - tEncoder >= INTERVALO_PRINT_MS) {
        tEncoder = ahora;
        Serial.print(F("  Enc izq: ")); Serial.print(motorIzq.getEncoderCount());
        Serial.print(F("  Enc der: ")); Serial.print(motorDer.getEncoderCount());
        Serial.print(F("  RPM izq: ")); Serial.print(motorIzq.getRPM(), 1);
        Serial.print(F("  RPM der: ")); Serial.println(motorDer.getRPM(), 1);
      }
      break;
  }
}

// ════════════════════════════════════════════════════════════
//  PROCESAR COMANDO
// ════════════════════════════════════════════════════════════
void procesarComando(char cmd) {
  switch (cmd) {

    // ── Cambios de estado ──────────────────────────────────
    case '0':
      detenerTodo();
      estadoActual = IDLE;
      Serial.println(F("\n[IDLE] Todo detenido."));
      imprimirMenu();
      break;

    case '1':
      detenerTodo();
      motorIzq.habilitar(true);
      motorDer.habilitar(true);
      estadoActual = TEST_MOTORES;
      Serial.println(F("\n[TEST MOTORES] w=adelante s=atras a=izq d=der f=freno +/- velocidad"));
      break;

    case '2':
      detenerTodo();
      estadoActual = TEST_ULTRASONICOS;
      Serial.println(F("\n[TEST ULTRASONICOS] Leyendo distancias..."));
      break;

    case '3':
    /*
      detenerTodo();
      estadoActual = TEST_IMU;
      imu.resetYaw();
      Serial.println(F("\n[TEST IMU] Mostrando pitch/roll/yaw..."));
      */
      break;

    case '4':
      detenerTodo();
      motorIzq.habilitar(true);
      motorDer.habilitar(true);
      motorIzq.resetEncoder();
      motorDer.resetEncoder();
      estadoActual = TEST_ENCODERS;
      Serial.println(F("\n[TEST ENCODERS] Gira ruedas manualmente o usa w/s. Conteo en tiempo real."));
      break;

    // ── Subcomandos de motores ─────────────────────────────
    case 'w':
      if (estadoActual == TEST_MOTORES || estadoActual == TEST_ENCODERS) {
        motorIzq.setVelocidad(velocidadActual);
        motorDer.setVelocidad(velocidadActual);
        Serial.println(F("  >> Adelante"));
      }
      break;

    case 's':
      if (estadoActual == TEST_MOTORES || estadoActual == TEST_ENCODERS) {
        motorIzq.setVelocidad(-velocidadActual);
        motorDer.setVelocidad(-velocidadActual);
        Serial.println(F("  >> Atras"));
      }
      break;

    case 'a':
      if (estadoActual == TEST_MOTORES) {
        motorIzq.setVelocidad(-velocidadActual);
        motorDer.setVelocidad(velocidadActual);
        Serial.println(F("  >> Girando izquierda"));
      }
      break;

    case 'd':
      if (estadoActual == TEST_MOTORES) {
        motorIzq.setVelocidad(velocidadActual);
        motorDer.setVelocidad(-velocidadActual);
        Serial.println(F("  >> Girando derecha"));
      }
      break;

    case 'f':
      if (estadoActual == TEST_MOTORES || estadoActual == TEST_ENCODERS) {
        motorIzq.frenar();
        motorDer.frenar();
        Serial.println(F("  >> Freno activo"));
      }
      break;

    case '+':
      if (estadoActual == TEST_MOTORES || estadoActual == TEST_ENCODERS) {
        velocidadActual = constrain(velocidadActual + PASO_VEL, 0, 255);
        Serial.print(F("  >> Velocidad: ")); Serial.println(velocidadActual);
      }
      break;

    case '-':
      if (estadoActual == TEST_MOTORES || estadoActual == TEST_ENCODERS) {
        velocidadActual = constrain(velocidadActual - PASO_VEL, 0, 255);
        Serial.print(F("  >> Velocidad: ")); Serial.println(velocidadActual);
      }
      break;

    case 'r':
      // Reset encoders desde cualquier estado
      motorIzq.resetEncoder();
      motorDer.resetEncoder();
      Serial.println(F("  >> Encoders reseteados"));
      break;
  }
}

// ════════════════════════════════════════════════════════════
//  DETENER TODO
// ════════════════════════════════════════════════════════════
void detenerTodo() {
  motorIzq.detener();
  motorDer.detener();
  motorIzq.habilitar(false);
  motorDer.habilitar(false);
}

// ════════════════════════════════════════════════════════════
//  MENU
// ════════════════════════════════════════════════════════════
void imprimirMenu() {
  Serial.println(F(""));
  Serial.println(F("════════════════════════════════"));
  Serial.println(F("   ROBOT ASILO - MODO PRUEBA    "));
  Serial.println(F("════════════════════════════════"));
  Serial.println(F("  1 → Test Motores"));
  Serial.println(F("  2 → Test Ultrasonicos"));
  Serial.println(F("  3 → Test IMU"));
  Serial.println(F("  4 → Test Encoders"));
  Serial.println(F("  0 → IDLE / Detener todo"));
  Serial.println(F("════════════════════════════════"));
}

