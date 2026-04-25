# **RoboAsilo**

# Descripción
Robot vigilante para estar de guardia en la robobatalla.

Es un robot para la materia de Mecatronica Aplica, el robot va a trabajar en un asilo detectando personas caídas o si tienen alguna emergencia que puedan hacer alguna señal al robot para enviar la alerta a una aplicación mientras el robot patrulla dentro de un área,  va a seguir una línea el robot que es lo que a nosotros se nos hacía más fácil,  tiene que trabajar en automático.

# Componentes:
- Arduino mega
- Cámara huskylens
- Seguidor de línea qtr-8a
- 3 sensores ultrasónicos
- Motores con encoder integrado bts7960
- Sensor imu mpu-6050

# Funcionalidades
- Aplicación por bluetooth
- Lecturas del huskylens
    - Funcion detectar emergencia
- Funciones basicas motores
- Lecturas encoder
- Lecturas IMU
- Lecturas seguidor de linea 
    - Funcion seguir linea
- Lecturas ultrasonicos
    - Funcion evadir obstáculos

# Arquitectura
Maquina de estados
Ademas de procurar tener una buena estructura modular, separando los archivos de cada componente para que sea facil de detectar fallas.

# Fase 1 
Probar que funcionen los compenentes por separado, para entender como funcionan y por si hay alguna falla en el hardware haya tiempo para arreglarlo. 

## Lecturas del huskylens
https://youtu.be/E140gPLPz4A?si=2EMIuysWRlCKQAyY&t=1490

## Lecturas ultrasonicos
https://www.youtube.com/watch?v=n-gJ00GTsNg

## Funciones puente H bts7960
https://www.youtube.com/watch?v=QNr5GDi4eHI

## Lecturas encoder
https://www.youtube.com/watch?v=ADPpHxj5Nbg

## Lecturas IMU mpu-6050
https://youtu.be/XCyRXMvVSCw?si=MpEgV4WczVPrpx8V

## Lecturas seguidor de linea qtr-8a
https://youtu.be/80O2q3_rfFw?si=a85RyeK9PMbEcW0w

## Aplicación por bluetooth
Que funcione solo como salida de alertas

Opciones: 
- Usar android studio 
https://www.youtube.com/watch?v=aE8EbDmrUfQ

- Usa app inventor
https://www.youtube.com/watch?v=aQcJ4uHdQEA

# Fase 2
Empezar a implementar las funcionalidades del robot y unir los modulos en la maquina de estados

## Funcion seguir linea
Opcion primitiva
https://www.youtube.com/watch?v=fHo2RYgrmJw

Segunda opcion 
https://www.youtube.com/watch?v=bZJYAhzlYa0&t=5s

## Funcion evadir obstáculo
https://www.youtube.com/watch?v=dO3eBJTHg68

No se que tan util sea
La logica que sigue es que si el frente esta bloqueado entonces gira a la direccion en la que hay mas espacio 

## Funcion detectar emergencia

# Fase 3
Ajustar detalles y arreglar errores

