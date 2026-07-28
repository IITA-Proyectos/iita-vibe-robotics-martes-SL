# Robot: Fútbol IITA 2025 — Teensy 4.1 + Zircon + OpenMV

Definición de los **dos robots de fútbol** del Nacional 2025. Pasale este archivo a la IA
cuando le pidas código para ellos, junto con la directiva
[`../directivas-ia/system-prompts/soccer-teensy-zircon.md`](../directivas-ia/system-prompts/soccer-teensy-zircon.md).

> ⚠️ Este robot **no es LEGO**. El resto de `robots/` son definiciones de Spike Prime en Python
> para Pybricks. Este es C++ sobre Teensy y se carga por cable USB.

---

## Ficha

| | |
|---|---|
| Cerebro | **Teensy 4.1** (ARM Cortex-M7 @ 600 MHz) |
| Placa | **Zircon Rev v15** (PCB comercial de Robomov) |
| Lenguaje | C++ estilo Arduino (`.ino`), `setup()` / `loop()` |
| Cómo se carga | **Cable USB**, Arduino IDE + Teensyduino (o PlatformIO `teensy41`) |
| Tracción | **3 motores omni** — se mueve en cualquier dirección sin girar |
| Visión | **OpenMV H7** independiente, le manda coordenadas por serie |
| Rumbo | **BNO055** por I²C (`0x28`) |
| Línea | 3 sensores analógicos |
| Pelota | 8 sensores IR TSSP58038 (activos en bajo) |
| Kicker | **No hay solenoide.** La patada es un avance fuerte de las ruedas |
| Lo que NO tiene | encoders, odometría, ToF, ultrasonido, segunda cámara |

---

## Elegir el robot: el `#define`

Los dos robots corren **el mismo programa**. Se elige con un `#define` arriba de todo
(líneas 10-11), y **los motores están cableados distinto en cada uno**:

```cpp
// ELEGI 1
#define ROBOT1        // ARQUERO   — estado inicial: impulso_inicial
//#define ROBOT2      // DELANTERO — estado inicial: AVANCE_INICIO
```

## Pines

### Motores

| Robot | Motor 1 | Motor 2 | Motor 3 |
|---|---|---|---|
| **ROBOT1 — Arquero** | INA 2 / INB 5 / PWM 3 | INA 8 / INB 7 / PWM 6 | INA 11 / INB 12 / PWM 4 |
| **ROBOT2 — Delantero** | INA 8 / INB 7 / PWM 6 | INA 11 / INB 12 / PWM 4 | INA 2 / INB 5 / PWM 3 |

Es el **mismo hardware con los motores conectados en otro orden**. Equivalencia: el Motor 1 del
arquero es el Motor 3 del delantero, el 2 es el 1, y el 3 es el 2.

### Todo lo demás (igual en los dos)

| Pin | Qué es |
|---|---|
| 0 / 1 | `Serial1` RX/TX → **cámara OpenMV, 19200 baudios** |
| 9 / 10 | botones 1 y 2 (pull-up interno) |
| 14-17, 20-23 | 8 sensores IR de pelota |
| 18 / 19 | I²C SDA/SCL → **BNO055** (`0x28`) |
| A11 (25) | sensor de línea 1 |
| A13 (27) | sensor de línea 2 |
| A12 (26) | sensor de línea 3 |

Mapa completo con drivers y notas:
[`../futbol-roboliga2026/robots-2025/mapa-pines-teensy.md`](../futbol-roboliga2026/robots-2025/mapa-pines-teensy.md)

---

## Qué recibe de la cámara

9 bytes de corrido, sin parar, a 19200 baudios:

```
[201][Xp][Yp+100]  [202][Xam][Yam+100]  [203][Xaz][Yaz+100]
 pelota             arco amarillo          arco azul
```

Coordenadas en cm. **A la Y hay que restarle 100** (viaja corrida porque un byte no lleva
negativos). Si no ve algo, manda 0 — que significa "no lo veo", no "está pegado".

---

## Librería de la placa

```cpp
#include <zirconLib.h>

InitializeZircon();          // pines + BNO055 + versión de placa
readCompass();               // rumbo del giroscopio
readBall(1..8);              // sensor IR de pelota
readLine(1..3);              // sensor de línea (analógico)
readButton(1..2);
motor1(power, direction);    // ídem motor2 y motor3
```

⚠️ `zirconLib.cpp` **no compila** hasta borrar la llave de más de la línea 355.

---

## Cosas que hay que saber antes de tocar nada

1. **Un solo `#define` sin comentar.** Cargar el programa equivocado no rompe el robot, pero se
   mueve para cualquier lado y te hace perder la tarde.
2. **Si el BNO055 no inicializa, el programa se cuelga a propósito** en un bucle infinito.
   Robot que "no enciende" → sospechar del giroscopio antes que de la batería.
3. **Chequeos de línea y timeouts van al final del `case`, sin `else`.** Se evalúan siempre y la
   última asignación de estado gana. Un cambio inocente puede romper algo lejos.
4. **Sin encoders**: no hay forma de saber cuánto giró una rueda. Todo se mide por **tiempo**
   (`millis()`) y por **rumbo del giroscopio**. Nada de "avanzá 30 cm".
5. **Los PWM tienen piso**: por debajo de cierto valor el motor zumba y no arranca.
6. **Encender quieto** y esperar unos segundos: el giroscopio necesita estabilizarse.

Programas y documentación línea por línea:
[`../futbol-roboliga2026/robots-2025/`](../futbol-roboliga2026/robots-2025/)
