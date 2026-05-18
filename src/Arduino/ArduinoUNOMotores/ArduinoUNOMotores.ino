#include <SoftwareSerial.h>
/*
//PINES DEL ARDUINO MEGA
#define PIN_SIERRA_RPWM 9
#define PIN_SIERRA_LPWM 10
#define PIN_SIERRA_R    26
#define PIN_SIERRA_L    27

#define PIN_MOT_IZQ_RPWM 7
#define PIN_MOT_IZQ_LPWM 8
#define PIN_MOT_IZQ_R    24
#define PIN_MOT_IZQ_L    25

#define PIN_MOT_DER_RPWM 5
#define PIN_MOT_DER_LPWM 6
#define PIN_MOT_DER_R    22
#define PIN_MOT_DER_L    23

#define PIN_BLE_TX 18
#define PIN_BLE_RX 19
*/

#define PIN_SIERRA_RPWM 11   
#define PIN_SIERRA_LPWM 4    
#define PIN_SIERRA_R    A0  
#define PIN_SIERRA_L    A1   

#define PIN_MOT_IZQ_RPWM 9   
#define PIN_MOT_IZQ_LPWM 10  
#define PIN_MOT_IZQ_R    7  
#define PIN_MOT_IZQ_L    8  

#define PIN_MOT_DER_RPWM 5   
#define PIN_MOT_DER_LPWM 6   
#define PIN_MOT_DER_R    12  
#define PIN_MOT_DER_L    13  

#define PIN_BLE_TX 2
#define PIN_BLE_RX 3

SoftwareSerial bluetooth(PIN_BLE_RX, PIN_BLE_TX);

bool sierraEncendida = false;

void motorIzq(int velocidad) {
  if (velocidad > 0) {
    analogWrite(PIN_MOT_IZQ_RPWM, 0);
    analogWrite(PIN_MOT_IZQ_LPWM, velocidad);
  } else if (velocidad < 0) {
    analogWrite(PIN_MOT_IZQ_RPWM, abs(velocidad));
    analogWrite(PIN_MOT_IZQ_LPWM, 0);
  } else {
    analogWrite(PIN_MOT_IZQ_RPWM, 0);
    analogWrite(PIN_MOT_IZQ_LPWM, 0);
  }
}

void motorDer(int velocidad) {
  if (velocidad > 0) {
    analogWrite(PIN_MOT_DER_RPWM, velocidad);
    analogWrite(PIN_MOT_DER_LPWM, 0);
  } else if (velocidad < 0) {
    analogWrite(PIN_MOT_DER_RPWM, 0);
    analogWrite(PIN_MOT_DER_LPWM, abs(velocidad));
  } else {
    analogWrite(PIN_MOT_DER_RPWM, 0);
    analogWrite(PIN_MOT_DER_LPWM, 0);
  }
}

void sierra(bool on) {
  if (on == sierraEncendida) return;

  if (on) {
    for (int v = 0; v <= 255; v += 5) {
      analogWrite(PIN_SIERRA_RPWM, v);
      analogWrite(PIN_SIERRA_LPWM, 0);
      delay(20);
    }
    sierraEncendida = true;

  } else {
    for (int v = 255; v >= 0; v -= 5) {
      analogWrite(PIN_SIERRA_RPWM, v);
      analogWrite(PIN_SIERRA_LPWM, 0);
      delay(20);
    }

    analogWrite(PIN_SIERRA_RPWM, 0);
    analogWrite(PIN_SIERRA_LPWM, 0);

    sierraEncendida = false;
  }
}

void detener() {
  motorIzq(0);
  motorDer(0);
}

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  pinMode(PIN_MOT_IZQ_R, OUTPUT);
  pinMode(PIN_MOT_IZQ_L, OUTPUT);
  pinMode(PIN_MOT_DER_R, OUTPUT);
  pinMode(PIN_MOT_DER_L, OUTPUT);
  pinMode(PIN_SIERRA_R, OUTPUT);
  pinMode(PIN_SIERRA_L, OUTPUT);

  digitalWrite(PIN_MOT_IZQ_R, HIGH);
  digitalWrite(PIN_MOT_IZQ_L, HIGH);
  digitalWrite(PIN_MOT_DER_R, HIGH);
  digitalWrite(PIN_MOT_DER_L, HIGH);
  digitalWrite(PIN_SIERRA_R, HIGH);
  digitalWrite(PIN_SIERRA_L, HIGH);

  detener();
  sierra(false);

  Serial.println("Listo.");
}


void loop() {
  char cmd = 0;

  if (bluetooth.available()) cmd = bluetooth.read();
  else if (Serial.available()) cmd = Serial.read();

  if (cmd == 0) return;

  cmd = toupper(cmd);

  switch (cmd) {
    case 'F': 
      motorIzq(230);
      motorDer(230);
      Serial.println("Adelante");
      break;

    case 'B': 
      motorIzq(-230);
      motorDer(-230);
      Serial.println("Atras");
      break;

    case 'L': 
      motorIzq(-150);
      motorDer(150);
      Serial.println("Girando izquierda");
      break;

    case 'R': 
      motorIzq(150);
      motorDer(-150);
      Serial.println("Girando derecha");
      break;

    case 'X':
      sierra(true);
      Serial.println("Sierra encendida");
      break;

    case 'O':
      sierra(false);
      Serial.println("Sierra apagada");
      break;

    case 'P':
      detener();
      Serial.println("Parking");
      break;
    
    case 'S': // stop
      detener();
      sierra(false);
      Serial.println("Paren todo");
      break;

    default:
      Serial.println("Comando no reconocido");
      break;
  }
}

