# La librería de la placa Zircon

Es la capa que traduce "prendé el motor 2 a 150" a "poné estos tres pines en estos estados".
Los dos programas del robot la usan.

- [`zirconLib.h`](zirconLib.h) — la lista de funciones que ofrece (15 líneas)
- [`zirconLib.cpp`](zirconLib.cpp) — cómo están hechas (356 líneas)

---

## 🚨 Antes que nada: NO COMPILA tal cual

Son **DOS errores encadenados**, y el segundo está tapado por el primero.

### Error 1 — una llave `}` de más (línea 355)

Está suelta, después de que `isCompassCalibrated()` ya cerró en la línea 351.

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

Verificado de dos formas independientes: leyendo el archivo, y contando llaves con un script que
ignora comentarios (desbalance de exactamente 1). El error dirá algo tipo
`expected declaration before '}' token`.

### Error 2 — la variable `bno` está definida dos veces

`zirconLib.cpp:4` declara `Adafruit_BNO055 bno;` y **cada programa declara otra con el mismo
nombre** (`arquero.ino:68`, `delantero.ino:76`). Las dos son globales públicas y el *enlazador*
—el paso final que junta la librería con el programa— no puede decidir cuál es cuál:
`multiple definition of 'bno'`.

**No vas a ver este error hasta arreglar el primero**, porque el compilador ni llega al
enlazador. No es que rompiste algo: siempre estuvo ahí.

El arreglo es agregarle `static` (privada de este archivo):

```cpp
static Adafruit_BNO055 bno;
```

No rompe el giroscopio: **ningún programa llama nunca a `readCompass()`** (cero apariciones en
las 1207 y 1214 líneas), y aunque lo llamara, esa función está guardada por un `if` que nunca es
verdadero. Los programas hablan con el BNO055 con su **propio** objeto.

### Dónde se aplican estos parches

⚠️ **No acá.** Esta carpeta es una copia congelada. Y sobre todo: el archivo que el Arduino IDE
realmente compila **no es este** — es el que está instalado en `Documentos/Arduino/libraries/`.
El `#include <zirconLib.h>` con los signos `< >` significa *"buscalo en las librerías
instaladas"*. Si editás la copia del repo y compilás, no cambia nada y vas a pensar que el parche
no sirve.

Paso a paso, con la prueba de banco: [`../../correcciones-propuestas.md`](../../correcciones-propuestas.md).

> 🤔 **¿Y cómo ganaron el Nacional con esto?** No lo ganaron con esto. La copia instalada en la
> computadora del equipo 2025 era, casi seguro, otra. **La que quedó en el repo está rota.**
> Antes de pisar nada, fijate si hay una versión instalada distinta: esa es la que ganó.

**Atajo si esto te frena:** `../delantero/sin-zirconlib/sin-zirconlib.ino` no usa la librería.
Te sirve para tener el robot moviéndose hoy mismo — pero ojo, **es otro programa, no el que ganó
el nacional**.

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
