---
title: "Mapa de pines Teensy 4.1 - Ambos robots"
date: 2026-03-20
author: "Claude (Anthropic - Claude Sonnet 4.6)"
requested-by: "Maria Virginia Viollaz (@mariaviollaz)"
ai-assisted: true
ai-tool: "Claude (Anthropic - Claude Sonnet 4.6)"
status: final
tags: [electronica, teensy, pines, arquero, delantero, zircon]
robot: ambos
area: electronica
tipo: referencia
---

# Mapa de pines Teensy 4.1 - Ambos robots

Referencia completa de la asignacion de pines del Teensy 4.1 en la placa Zircon Rev v15.
Basado en el esquematico `Zircon.pdf` (2024-08-05) y el codigo de ambos robots.

---

## Pines compartidos (identicos en arquero y delantero)

| Pin Teensy | Funcion          | Componente          | Notas                        |
|-----------|------------------|---------------------|------------------------------|
| 0         | RX (Serial1)     | OpenMV H7/H7+       | UART a 19200 baud            |
| 1         | TX (Serial1)     | OpenMV H7/H7+       | UART a 19200 baud            |
| 9         | PushButton       | Boton 1             | Pull-up interno              |
| 10        | PushButton1      | Boton 2             | Pull-up interno              |
| 14        | smoothedBall1    | Sensor IR pelota 1  | TSSP58038, activo bajo       |
| 15        | smoothedBall2    | Sensor IR pelota 2  | TSSP58038, activo bajo       |
| 16        | smoothedBall3    | Sensor IR pelota 3  | TSSP58038, activo bajo       |
| 17        | smoothedBall4    | Sensor IR pelota 4  | TSSP58038, activo bajo       |
| 18        | SDA (I2C)        | BNO055 IMU          | Wire library, addr 0x28      |
| 19        | SCL (I2C)        | BNO055 IMU          | Wire library, addr 0x28      |
| 20        | smoothedBall5    | Sensor IR pelota 5  | TSSP58038, activo bajo       |
| 21        | smoothedBall6    | Sensor IR pelota 6  | TSSP58038, activo bajo       |
| 22        | smoothedBall7    | Sensor IR pelota 7  | TSSP58038, activo bajo       |
| 23        | smoothedBall8    | Sensor IR pelota 8  | TSSP58038, activo bajo       |
| A11 (25)  | Line1            | Sensor de linea 1   | Analogico                    |
| A12 (26)  | Line3            | Sensor de linea 3   | Analogico                    |
| A13 (27)  | Line2            | Sensor de linea 2   | Analogico                    |

---

## Pines de motores - Robot Arquero (ROBOT1)

| Pin Teensy | Funcion | Motor | Driver Zircon |
|-----------|---------|-------|----------------|
| 2         | INA1    | Motor 1 | U5           |
| 5         | INB1    | Motor 1 | U5           |
| 3         | PWM1    | Motor 1 | U5           |
| 8         | INA2    | Motor 2 | U17          |
| 7         | INB2    | Motor 2 | U17          |
| 6         | PWM2    | Motor 2 | U17          |
| 11        | INA3    | Motor 3 | U7           |
| 12        | INB3    | Motor 3 | U7           |
| 4         | PWM3    | Motor 3 | U7           |

---

## Pines de motores - Robot Delantero (ROBOT2)

| Pin Teensy | Funcion | Motor | Driver Zircon |
|-----------|---------|-------|----------------|
| 8         | INA1    | Motor 1 | U17          |
| 7         | INB1    | Motor 1 | U17          |
| 6         | PWM1    | Motor 1 | U17          |
| 11        | INA2    | Motor 2 | U7           |
| 12        | INB2    | Motor 2 | U7           |
| 4         | PWM2    | Motor 2 | U7           |
| 2         | INA3    | Motor 3 | U5           |
| 5         | INB3    | Motor 3 | U5           |
| 3         | PWM3    | Motor 3 | U5           |

---

## Equivalencia de motores entre robots

Los robots comparten el mismo hardware (Zircon + Teensy 4.1) pero los motores
estan fisicamente cableados de forma diferente. La equivalencia es:

| Motor Arquero | Motor Delantero | Pines (INA/INB/PWM) | Driver |
|--------------|----------------|---------------------|--------|
| Motor 1      | Motor 3        | 2 / 5 / 3           | U5     |
| Motor 2      | Motor 1        | 8 / 7 / 6           | U17    |
| Motor 3      | Motor 2        | 11 / 12 / 4         | U7     |

La seleccion del mapeo correcto se hace en tiempo de compilacion con:
- `#define ROBOT1` para compilar el arquero
- `#define ROBOT2` para compilar el delantero

---

## Pines NO utilizados en el esquematico base

Los siguientes pines del Teensy 4.1 no estan conectados en la Zircon Rev v15:
13 (LED onboard), 24, 28-41 (disponibles para expansion).

---

## Archivos relacionados

> **Rutas corregidas el 2026-07-28.** Este documento viene del repo del equipo de Incheon y
> apuntaba a carpetas que en este repo no existen. Los equivalentes acá son:

- Codigo arquero: [`arquero/arquero.ino`](arquero/arquero.ino)
- Codigo delantero: [`delantero/delantero.ino`](delantero/delantero.ino)
- Libreria: [`libreria-zircon/zirconLib.cpp`](libreria-zircon/zirconLib.cpp)
- Como funciona cada robot: [`arquero/COMO-FUNCIONA.md`](arquero/COMO-FUNCIONA.md) ·
  [`delantero/COMO-FUNCIONA.md`](delantero/COMO-FUNCIONA.md)

Quedan en el repo del equipo de Incheon
([open-soccer-robocup-team2026](https://github.com/IITA-Proyectos/open-soccer-robocup-team2026)),
no se copiaron acá:

- Esquematico de la placa: `hardware/electronics/Zircon.pdf`
- Journal donde se documento la diferencia de pines entre robots:
  `journal/2026-03-20-diferencias-pines-motores-arquero-delantero.md`
