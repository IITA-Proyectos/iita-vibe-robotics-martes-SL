# La librería de la placa Zircon

Es la capa que traduce "prendé el motor 2 a 150" a "poné estos tres pines en estos estados".
Los dos programas del robot la usan.

- [`zirconLib.h`](zirconLib.h) — la lista de funciones que ofrece (15 líneas)
- [`zirconLib.cpp`](zirconLib.cpp) — cómo están hechas (356 líneas)

---

## 🚨 Antes que nada: NO COMPILA tal cual

`zirconLib.cpp` tiene **una llave de cierre `}` de más en la línea 355**. Está suelta, después de
que la función `isCompassCalibrated()` ya cerró en la línea 351.

```cpp
349  bool isCompassCalibrated() {
350    return compassCalibrated;
351  }
352
353
354
355  }        // ← ESTA sobra. Borrala.
356
```

**El arreglo es borrar la línea 355.** Nada más. Verificado leyendo el archivo el 28-jul-2026.

Si intentás compilar sin arreglarlo, el error va a decir algo tipo
`expected declaration before '}' token`. No es tu culpa ni un problema de tu instalación:
viene así desde 2025.

> Como el archivo de esta carpeta es una copia congelada del original, **no lo arreglamos acá**.
> Copiá la librería a tu carpeta de trabajo y arreglala ahí. Si el equipo decide adoptar el
> arreglo para todos, se hace en un commit aparte y se anota en la bitácora.

**Atajo si esto te frena:** `../delantero/variantes/delantero-sin-zirconlib.ino` no usa la
librería. Te sirve para tener el robot moviéndose hoy mismo.

---

## Qué ofrece

```cpp
void   InitializeZircon();              // arranca todo: pines, BNO055, versión de placa
void   setZirconVersion();              // detecta si la placa es Mark1 o Naveen1
String getZirconVersion();

double readCompass();                   // rumbo del giroscopio BNO055
int    readBall(int ballSensorNumber);  // sensor IR de pelota, 1 a 8
int    readLine(int lineNumber);        // sensor de línea, 1 a 3 (analógico)
int    readButton(int buttonNumber);    // botón 1 o 2

void   motor1(int power, bool direction);
void   motor2(int power, bool direction);
void   motor3(int power, bool direction);

void   initializePins();
```

---

## Dos cosas que te van a morder

**1. Hay dos versiones de placa con pines distintos.** `setZirconVersion()` elige entre `Mark1`,
`Naveen1` y un caso por defecto (`zirconLib.cpp:263`, `:290`). **Los tres asignan pines
diferentes.** Si el robot mueve el motor equivocado, empezá por acá: fijate qué versión está
detectando y compará con [`../mapa-pines-teensy.md`](../mapa-pines-teensy.md).

**2. `isCompassCalibrated()` existe en el `.cpp` pero no está declarada en el `.h`.** O sea:
está escrita pero no la podés llamar desde el programa del robot sin agregarla al `.h` primero.

---

## El giroscopio vive acá

`readCompass()` es la puerta al **BNO055**, el giroscopio que dice hacia dónde mira el robot
(I²C, dirección `0x28`, pines 18 y 19 del Teensy).

**Sí se usaba en 2025**, en contra de lo que dice alguna documentación vieja. Cómo entra
exactamente al control de cada robot está detallado, con números de línea, en los
`COMO-FUNCIONA.md` de arquero y delantero.

Un detalle que conviene saber de entrada: **si el BNO055 no arranca, el programa se queda
colgado** en un bucle infinito y el robot no hace absolutamente nada. Si algún día el robot
"no enciende", ese es el primer sospechoso — no la batería.
