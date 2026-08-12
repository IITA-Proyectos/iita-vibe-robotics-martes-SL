# ⚽ Fútbol — Roboliga 2026

> 📦 **SE MUDÓ (2026-08-11).** El trabajo de cada robot ahora vive junto, en `codigo-clases/`,
> una carpeta por robot:
>
> | Robot | Dónde está ahora |
> |---|---|
> | **DELANTERO** | [`../codigo-clases/delantero-teensy-zircon/`](../codigo-clases/delantero-teensy-zircon/) |
> | **ARQUERO** | [`../codigo-clases/arquero-teensy-zircon/`](../codigo-clases/arquero-teensy-zircon/) |
>
> Ahí están el firmware vivo, las pruebas, las bitácoras y lo de la cámara.
>
> **Esta carpeta se queda con el material 2025**, que es compartido por los dos robots y no se
> toca sin avisar a la otra mesa: [`robots-2025/`](robots-2025/) (el código campeón del Nacional,
> el script de la cámara y el mapa de pines), [`bugs-conocidos.md`](bugs-conocidos.md),
> [`correcciones-propuestas.md`](correcciones-propuestas.md) y [`entorno.md`](entorno.md).

Equipo del taller de los martes. Retomamos los **dos robots de fútbol con los que IITA Salta
salió campeón del Nacional 2025** y los llevamos a la Roboliga Argentina 2026.

> **Los robots están INTACTOS.** Mismo Teensy 4.1, misma placa Zircon Rev v15, mismas cámaras
> OpenMV H7, mismos puertos, mismo cableado. No hay que construir nada: hay que **entender lo
> que ya anda** y mejorarlo.

## ✅ Identificación de los robots (medido en banco, 2026-07-28)

| Robot | Se compila con | Rueda IZQUIERDA | Rueda DERECHA | Rueda ATRÁS |
|---|---|---|---|---|
| **DELANTERO** — el que el equipo llama *"robot 2"* | `#define ROBOT2` | pines **8/7/6** (U17) | **11/12/4** (U7) | **2/5/3** (U5) |
| **ARQUERO** — el otro | `#define ROBOT1` | *sin medir todavía* | | |

**Cómo se supo:** la rueda **trasera** es el dato sin ambigüedad, y está en los pines 2/5/3 — que
son el `M3` del `ROBOT2`. En el arquero el `M3` son los 11/12/4. Método y evidencia en
[`bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md`](../codigo-clases/delantero-teensy-zircon/bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md).

> 🚨 **No confundas la etiqueta física con el `#define`.** El equipo llama *"robot 2"* a esta
> unidad y en el código `ROBOT2` significa DELANTERO — acá **coinciden por casualidad**, pero no
> confíes en eso para el otro robot. Lo mejor: dejar de numerarlos y etiquetarlos por puesto.

> ⚠️ **Los rótulos izquierda/derecha del código 2025 están ESPEJADOS.** Los comentarios dicen
> `M1 = motor derecho` y `M2 = motor izquierdo`, pero medido en el robot es al revés: están
> escritos mirando al robot **de frente**, no desde el robot. El código funciona igual — lo que
> engaña es el comentario. Detalle y evidencia en
> [`bitacora/2026-07-28-identificacion-robot-arquero.md`](../codigo-clases/arquero-teensy-zircon/bitacora/2026-07-28-identificacion-arquero.md).

---

## ⚠️ Esto NO es LEGO

Todo el resto de este repo es **LEGO Spike Prime + Pybricks + Python**, programado desde
[code.pybricks.com](https://code.pybricks.com) por Bluetooth.

**Estos robots no tienen nada de eso.** Son:

| | Taller (LEGO) | Fútbol 2025 |
|---|---|---|
| Cerebro | Hub Spike Prime | **Teensy 4.1** sobre placa **Zircon Rev v15** |
| Lenguaje | Python (Pybricks) | **C++ (Arduino)** |
| Cómo se carga | Bluetooth, code.pybricks.com | **Cable USB**, Arduino IDE / PlatformIO |
| Motores | `Motor(Port.E)` | pines `INA` / `INB` / `PWM` a mano |
| Visión | — | **cámara OpenMV H7** en MicroPython, por cable serie |

👉 **Antes de pedirle código a una IA para estos robots**, pegale la directiva
[`directivas-ia/system-prompts/soccer-teensy-zircon.md`](../directivas-ia/system-prompts/soccer-teensy-zircon.md).
Si no, te va a escribir código Pybricks para un robot que no lo entiende, y va a *parecer*
correcto. Es el error más caro y más fácil de cometer en este track.

---

## 📁 Qué hay acá

| Carpeta / archivo | Qué es |
|---|---|
| ⭐ [`firmware/`](firmware/) | **El firmware vivo de cada robot** — lo que se lleva a la cancha |
| [`pruebas/`](../codigo-clases/delantero-teensy-zircon/pruebas/) | Programas cortos de diagnóstico. Descartables |
| [`robots-2025/`](robots-2025/) | Los programas y la documentación de los robots tal cual están hoy |
| [`robots-2025/mapa-pines-teensy.md`](robots-2025/mapa-pines-teensy.md) | ⭐ **Empezá por acá.** Qué pin del Teensy va a qué cosa, en los dos robots |
| [`robots-2025/arquero/`](robots-2025/arquero/) | Programa del arquero (1207 líneas) + cómo funciona, línea por línea |
| [`robots-2025/delantero/`](robots-2025/delantero/) | Programa del delantero (1214 líneas) + cómo funciona, línea por línea |
| [`robots-2025/vision-openmv/`](robots-2025/vision-openmv/) | El programa de la cámara y el protocolo con el que le habla al Teensy |
| [`robots-2025/libreria-zircon/`](robots-2025/libreria-zircon/) | La librería de la placa (⚠️ tiene un problema, leé su README) |
| `bugs-conocidos.md` | Los problemas heredados de 2025, con síntoma y cómo se ven en cancha |
| `correcciones-propuestas.md` | Los parches propuestos para esos problemas — **propuestos, sin validar en banco** |
| [`entorno.md`](entorno.md) | Cómo instalar todo y cómo cargar un programa al robot |
| [`bitacora/`](../codigo-clases/delantero-teensy-zircon/bitacora/) | Qué hicimos cada día. Una entrada por clase |

Las reglas de la competencia van en
[`competition-packs/rcj-soccer/`](../competition-packs/rcj-soccer/).
Tu código personal y tus pruebas siguen yendo a tu carpeta `alumnos/<tunombre>/`, como siempre.

---

## 👥 Dos sesiones en paralelo, una por robot

Desde el 2026-08-04 se trabaja con **una rama y un worktree por robot**, para que las dos
sesiones no se pisen:

| Robot | Rama | Carpeta en disco |
|---|---|---|
| **Delantero** | `robot/delantero` | `C:\Users\violl\iita-martes-delantero` |
| **Arquero** | `robot/arquero` | `C:\Users\violl\iita-martes-arquero` |
| (integración) | `main` | `C:\Users\violl\iita-viberobotev3` |

Cada sesión toca **sólo** la carpeta de su robot dentro de [`firmware/`](firmware/). Lo compartido
—`robots-2025/`, `pruebas/`, `bugs-conocidos.md`— se toca avisando a la otra.

> 🚨 **Un solo robot conectado por vez al cargar.** El cargador del Teensy no elige a qué placa le
> manda el programa. Con los dos enchufados, una sesión puede flashear el robot de la otra.
> Verificar con `pio device list` antes de subir. Detalle en [`firmware/README.md`](../codigo-clases/delantero-teensy-zircon/README.md).

## 🚦 Por dónde arrancar

1. Leé [`entorno.md`](entorno.md) e instalá las herramientas. **No toques el robot todavía.**
2. Leé [`robots-2025/mapa-pines-teensy.md`](robots-2025/mapa-pines-teensy.md). Es corto y es el
   mapa de todo.
3. Elegí un robot y leé su `COMO-FUNCIONA.md`. Está escrito línea por línea contra el código real:
   cada tiempo, cada PWM, cada umbral, con el número de línea donde vive.
4. **Primera tarea de verdad:** cargar el programa tal cual está y confirmar que el robot se
   comporta como dice el `COMO-FUNCIONA.md`. Ver más abajo por qué esto no es un trámite.
5. Recién ahí, mirá `bugs-conocidos.md` y `correcciones-propuestas.md`.

---

## 🔑 Lo primero que hay que entender: son DOS COPIAS que se separaron

`arquero.ino` y `delantero.ino` **salieron del mismo programa**, y los dos contienen la máquina
de estados del arquero *y* la del delantero en el mismo `switch`. Pero **no son el mismo archivo**:
en algún momento el equipo 2025 los copió y los siguió editando **por separado**. Verificado:

| | arquero.ino | delantero.ino |
|---|---|---|
| `avanzar_despues_de_patear` (un estado) | existe | **no existe** |
| `APUNTAR_PELOTA_horario` (otro estado) | **no existe** | existe |
| `tolerancia_cercania` | `140.0` (L110) | `50.0` (L119) |
| umbrales de blanco del bloque ROBOT1 | 500 / 650 / 600 | 600 / 600 / 600 |

> 🚨 **Consecuencia práctica: dar vuelta el `#define` de `delantero.ino` NO te da el firmware del
> arquero.** Te da una versión **vieja** del arquero, con otros umbrales y sin algunos estados.
> Cada robot tiene SU archivo. No los mezclen.

Dentro de cada archivo, lo que cambia entre un robot y otro es:

| | Arquero | Delantero |
|---|---|---|
| `#define` activo | `ROBOT1` (línea 10) | `ROBOT2` (línea 11) |
| Estado inicial | `impulso_inicial` (línea 131) | `AVANCE_INICIO` (línea 138) |
| Qué bloque corre | el del arquero | el del delantero |
| Qué bloque queda muerto | el del delantero | el del arquero |

**Consecuencia práctica:** si encontrás un problema en el código, lo primero que tenés que
preguntarte es *"¿este pedazo se ejecuta en ESTE robot, o está en el bloque muerto del otro?"*.
Muchas líneas que parecen bugs nunca se ejecutan.

**Y una regla estructural del código que hay que tener en la cabeza:** en casi todos los estados,
los chequeos de línea blanca y de timeout están **al final del `case`, sin `else`**. Se evalúan
siempre, y **la última asignación de estado del ciclo gana**. Eso hace que a veces un "bug"
evidente no se manifieste nunca… y que a veces un cambio inocente rompa algo lejos.

---

## ⚠️ Por qué la primera tarea es cargar el programa sin tocar nada

Los robots están intactos, pero **nadie sabe qué binario tiene adentro el Teensy hoy**, y no se
puede leer de vuelta para verificarlo. Lo único que sabemos es qué hay en estos archivos.

Además hay una duda honesta que arrastramos del equipo anterior y que **todavía no está resuelta**:
el archivo del arquero que tenemos acá se subió a GitHub el **6-feb-2026**, o sea *después* del
Nacional (diciembre 2025). El del delantero se subió el 31-oct-2025. Entre esas dos fechas no hay
ni un commit. Así que no hay forma de probar, mirando el historial, cuál fue exactamente el
programa que corrió en la cancha.

**No importa tanto como parece**, y por eso no frena nada: lo que importa es que **el código que
tenemos coincida con el robot que tenemos enfrente**. Eso se verifica en banco, en la primera
clase, comparando el comportamiento real contra el `COMO-FUNCIONA.md`. Si coincide, listo:
esta es la base. Si no coincide, lo anotamos en la bitácora y lo investigamos.

En el repo del equipo anterior existe una versión **anterior** del arquero (`ARQUERO/MARTES28_10`,
del 31-oct-2025), por si hiciera falta comparar:
[repo 2025](https://github.com/IITA-Proyectos/RoboCupJunior-Soccer-Open-League-2025).

---

## 📌 Reglas de este track

1. **Los archivos de `robots-2025/` no se editan.** Son la foto de lo que ganó el Nacional.
   Si querés probar un cambio, copiá el programa a tu carpeta `alumnos/<tunombre>/` y trabajá ahí.
2. **Nada se da por arreglado hasta que anda en el robot.** Que compile no prueba nada.
   Que la IA diga que está bien, tampoco. Lo cierra quien tiene la placa en la mano.
3. **Una entrada de bitácora por clase**, en `bitacora/`. Qué probamos, qué pasó, qué quedó.
   Sirve para no repetir el mismo experimento tres veces.
4. **Un cambio por vez.** Si tocás tres cosas y el robot empeora, no sabés cuál fue.
5. **Antes de pedirle código a la IA**, pasale la directiva y el archivo del robot.

---

## 🧾 De dónde salió este material

Todo viene de dos repos del IITA, copiado el **28-jul-2026**:

- [`RoboCupJunior-Soccer-Open-League-2025`](https://github.com/IITA-Proyectos/RoboCupJunior-Soccer-Open-League-2025)
  — el repo original del equipo 2025.
- [`open-soccer-robocup-team2026`](https://github.com/IITA-Proyectos/open-soccer-robocup-team2026)
  — el repo del equipo que fue a Incheon, que ya había curado y analizado el código 2025.

Los **programas** se copiaron **byte a byte** (verificado por hash de git, ver
[`robots-2025/README.md`](robots-2025/README.md)). A los **documentos** se les corrigieron las
rutas y algunos errores heredados; están marcados uno por uno.

> **Equipo hermano:** hay otro equipo IITA yendo a la misma Roboliga 2026 con los robots de
> Incheon, en el repo `futbol-roboliga2026-iita-salta`. Robots parecidos, no iguales — el de
> ellos tiene 3 placas, 2 cámaras, IMU, ToF y ultrasonido. El nuestro es el de 2025: una placa,
> una cámara. **No mezclar el código de los dos.**
