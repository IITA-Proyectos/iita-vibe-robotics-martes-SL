# Cómo instalar todo y cargar un programa al robot

Acá no hay Bluetooth ni code.pybricks.com. Son **dos programas separados, en dos chips
separados, con dos herramientas distintas**, y los dos se cargan **por cable USB**.

| Qué | Chip | Herramienta | Lenguaje |
|---|---|---|---|
| El cerebro del robot | Teensy 4.1 (sobre la placa Zircon) | **Arduino IDE + Teensyduino**, o PlatformIO | C++ |
| La visión | cámara OpenMV H7 | **OpenMV IDE** | MicroPython |

Son independientes: podés cargar uno sin tocar el otro. De hecho conviene, para no cambiar
dos cosas a la vez.

---

## 1. El Teensy (el cerebro)

### Instalar

1. **Arduino IDE** — [arduino.cc/en/software](https://www.arduino.cc/en/software).
2. **Teensyduino**, el complemento de PJRC que le enseña al Arduino IDE qué es un Teensy —
   [pjrc.com/teensy/td_download.html](https://www.pjrc.com/teensy/td_download.html).
   Sin esto, la placa Teensy 4.1 **no aparece** en la lista.
3. En el Arduino IDE, `Herramientas → Placa → Teensyduino → Teensy 4.1`.

> Quien prefiera **PlatformIO** dentro de VSCode: la plataforma es `teensy`, la placa `teensy41`
> y el framework `arduino`. Es más cómodo para trabajar con git, pero para arrancar el Arduino
> IDE alcanza y sobra. Que cada uno use lo que le resulte — **pero anoten en la bitácora con qué
> lo compilaron**, porque si a uno le anda y a otro no, ahí está la diferencia.

### Librerías que hacen falta

Los programas empiezan con esto (`arquero.ino:1-5`):

```cpp
#include <Arduino.h>
#include <zirconLib.h>        // ← la de la placa, está en robots-2025/libreria-zircon/
#include <Wire.h>             // ← viene con Arduino
#include <Adafruit_Sensor.h>  // ← instalar desde el Gestor de Librerías
#include <Adafruit_BNO055.h>  // ← instalar desde el Gestor de Librerías
```

En `Herramientas → Gestionar librerías`, buscá e instalá **"Adafruit BNO055"**; el IDE te va a
ofrecer instalar también **"Adafruit Unified Sensor"** — decile que sí, hace falta.

`zirconLib` **no está en el gestor**: es del IITA. Copiá `zirconLib.h` y `zirconLib.cpp` a
`Documentos/Arduino/libraries/zirconLib/`.

> ⚠️ **Tiene que ir sí o sí en la carpeta de librerías, NO al lado del programa.** El
> `#include <zirconLib.h>` usa los signos `< >`, que significan *"buscalo en las librerías
> instaladas"*. Si la dejás junto al `.ino`, no la va a encontrar.

> 🚨 **`zirconLib.cpp` NO COMPILA tal cual — son dos errores, no uno.** Una llave de más y una
> variable duplicada. Leé
> [`robots-2025/libreria-zircon/README.md`](robots-2025/libreria-zircon/README.md) antes de
> pelearte con el compilador, y los parches en
> [`correcciones-propuestas.md`](correcciones-propuestas.md).

### Dependencias, para que a todos les compile igual

| Qué | Dónde se consigue |
|---|---|
| Arduino IDE | arduino.cc |
| Teensyduino | pjrc.com |
| Adafruit BNO055 | Gestor de Librerías |
| Adafruit Unified Sensor | Gestor de Librerías (lo pide la anterior) |
| zirconLib | este repo → `Documentos/Arduino/libraries/zirconLib/` |

**Anoten en la bitácora las versiones que instalaron.** Si a uno le compila y a otro no, la
diferencia casi siempre está en esta tabla.

### Elegir qué robot estás programando

Esto es **lo más importante y lo más fácil de olvidarse.** Arriba de todo del programa
(líneas 10-11):

```cpp
// ELEGI 1
#define ROBOT1        // ← ARQUERO
//#define ROBOT2      // ← DELANTERO
```

**Un solo `#define` sin comentar.** Si elegís el equivocado, el programa compila igual y se carga
igual, pero **los motores están cableados distinto en cada robot**: te vas a mover en direcciones
absurdas y vas a creer que rompiste algo. No rompiste nada, elegiste mal.

Regla práctica: **antes de cargar, mirá la línea 10 y 11 y decí en voz alta qué robot es.**

### Cargar

1. Conectá el Teensy por USB.
2. `Herramientas → Puerto` → elegí el puerto del Teensy.
3. Botón **Subir**. Se abre solo el "Teensy Loader" y parpadea.
4. Si no arranca la carga: apretá el **botón físico del Teensy** (el blanco, sobre la placa).

---

## 2. La cámara OpenMV (la visión)

1. Instalá el **OpenMV IDE** — [openmv.io/pages/download](https://openmv.io/pages/download).
2. Conectá la cámara por su propio cable USB (es un USB distinto al del Teensy).
3. Abrí
   [`robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py`](robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py).
4. Botón **play** (el ▶ verde abajo a la izquierda) → corre, y **ves lo que ve la cámara**.
   Esto es oro para calibrar: los rectángulos de colores son los blobs que detecta.
5. Cuando funcione y lo quieras dejar fijo en la cámara:
   `Herramientas → Save open script to OpenMV Cam (as main.py)`. Así arranca sola al enchufarla,
   sin computadora.

> Mientras esté conectada al IDE y corriendo con ▶, **también está mandando datos al Teensy**.
> Podés tener el robot funcionando y ver la imagen al mismo tiempo. Usalo.

---

## 3. Ver qué está pensando el robot

El Teensy puede imprimir texto a la computadora por el mismo cable USB: es la forma más rápida
de entender por qué hace lo que hace.

En el Arduino IDE: `Herramientas → Monitor Serie`.

⚠️ **Cuidado con confundir dos cosas distintas:**

- El **Monitor Serie** (`Serial`) va por el **USB a tu compu**. Sirve para mirar.
- La **cámara** habla por **`Serial1`** (pines 0 y 1 del Teensy), a **19200 baudios**.
  Ese es otro canal, no lo ves en el monitor.

Son independientes. Podés imprimir al monitor lo que llega de la cámara — de hecho, es lo primero
que conviene hacer para verificar que el enlace anda.

---

## 4. Encendido: el orden importa

> 🚨 **NO HAY BOTÓN DE ARRANQUE.** El programa empieza a mover motores **apenas hay energía**.
> Los botones existen en la placa (pines 9 y 10) pero el programa nunca los usa. Apoyá el robot
> donde querés que arranque **antes** de conectar la batería, y no lo tengas en la mano.

1. Poné el robot **quieto sobre la mesa**.
2. Encendelo y **esperá unos segundos sin moverlo**. El giroscopio necesita ese rato para
   estabilizarse; si lo movés mientras arranca, el rumbo le va a quedar torcido.
3. Fijate en los **LEDs de la cámara** (rojo = ve la pelota, verde = arco amarillo, azul = arco
   azul). Si están todos apagados apuntando a la cancha, el problema es de visión, no del robot.

Si el robot **no hace absolutamente nada** al encender: el primer sospechoso es el giroscopio,
no la batería. Si el BNO055 no inicializa, el programa se queda colgado en un bucle infinito a
propósito.

---

## Checklist antes de cada prueba

- [ ] ¿El `#define` es el del robot que tengo en la mano?
- [ ] ¿Batería cargada? (un robot con poca batería se comporta distinto y te hace perder horas)
- [ ] ¿Cambié **una sola cosa** desde la prueba anterior?
- [ ] ¿Sé qué espero que pase, **antes** de apretar el botón?
- [ ] ¿Voy a anotar el resultado en la bitácora?

Ese último punto no es burocracia: sin él, en la clase que viene nadie se acuerda de qué valor
probaron y repiten el mismo experimento.
