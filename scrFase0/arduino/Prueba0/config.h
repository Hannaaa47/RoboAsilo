//en este archivo voy a definir todos los PINes

#ifndef _CONFIG_H_
#define _CONFIG_H_

//motor driver BTS7960 izquierdo
#define PIN_MOT_IZQ_RPWM 5
#define PIN_MOT_IZQ_LPWM 6
#define PIN_MOT_IZQ_R 7
#define PIN_MOT_IZQ_L 8

//motor driver BTS7960 derecho
#define PIN_MOT_DER_RPWM 9
#define PIN_MOT_DER_LPWM 10
#define PIN_MOT_DER_R 11
#define PIN_MOT_DER_L 12

//encoder izquierdo
#define PIN_ENC_IZQ_A 2
#define PIN_ENC_IZQ_B 3

//encoder derecho
#define PIN_ENC_DER_A 18
#define PIN_ENC_DER_B 19

//huskylens                                
#define PIN_HUSKY_SDA 20
#define PIN_HUSKY_SCL 21

//IMU MPU6050
#define PIN_IMU_SDA 20
#define PIN_IMU_SCL 21

//modulo bluetooth HC-05 Bluetooth                                
#define PIN_BLE_TX 17
#define PIN_BLE_RX 16

//ultrasonico frente
#define PIN_UTS_FRE_TRIG 22
#define PIN_UTS_FRE_ECHO 23

//ultrasonico izquierdo
#define PIN_UTS_IZQ_TRIG 24
#define PIN_UTS_IZQ_ECHO 25

//ultrasonico derecho
#define PIN_UTS_DER_TRIG 26
#define PIN_UTS_DER_ECHO 27

//seguidor de linea QTR-8A
#define PIN_QTR_1 A0
#define PIN_QTR_2 A1
#define PIN_QTR_3 A2
#define PIN_QTR_4 A3
#define PIN_QTR_5 A4
#define PIN_QTR_6 A5
#define PIN_QTR_7 A6
#define PIN_QTR_8 A7

#endif