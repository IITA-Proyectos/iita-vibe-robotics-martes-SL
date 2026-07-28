---
name: openmv-h7-vision-y-enlace-serie
description: Cámara OpenMV H7 del robot de fútbol IITA 2025 (Roboliga 2026) y su enlace serie al Teensy 4.1. Usar SIEMPRE que aparezca "la cámara no ve la pelota", "no ve el arco", "ve fantasmas", "detecta cosas que no son", "hay que recalibrar los colores", "los umbrales no sirven con esta luz", "el robot no reacciona aunque la cámara la ve", "el robot persigue una pelota que no está", "queda pegado / congelado en el último dato", "Xp", "Yp", "Xam", "Yam", "Xaz", "Yaz", "haypelota", "hayarco_amarillo", "hayarco_azul", "codedYp", "los 9 bytes", "la trama", "el paquete de la cámara", "201 202 203", "el +100", "header1", "Serial1", "pin 0", "pin 1", "19200", "UART(3)", "OpenMV", "OpenMV IDE", "MicroPython de la cámara", "enviar_coordenadas", "find_blobs", "blob", "threshold", "umbral LAB", "naranja_threshold", "amarillo_threshold", "azul_threshold", "pixels_threshold", "QVGA", "homografía", "la matriz H", "transformarcoordenadas", "los LEDs de la cámara", "se desincroniza", "se pierde un byte", "trama corrida", "checksum", "balance de blancos", "set_auto_whitebal", "set_auto_gain", "exposición", "Threshold Editor", "get_statistics", "main.py", "no arranca sin la notebook", "watchdog de cámara", "A4", "DEL-06", "dato congelado", "procesar_blob", "cuántos FPS da la cámara", "clock.fps", "merge=True", "el robot ve fantasmas azules". Cubre la división de trabajo cámara/Teensy, umbrales LAB y cómo recalibrarlos bajo luz nueva, find_blobs y el blob más grande, la homografía atada al montaje, el contrato de 9 bytes verificado en los dos lados, las dos fragilidades conocidas del enlace, los LEDs como diagnóstico gratis, y seis pruebas de banco con criterio de aceptación. NO usar para lo que el robot HACE con esos números (máquina de estados, patada, orbitar, retroceder), ni para el giroscopio BNO055, los sensores de línea o los motores omni — eso va en las otras skills del firmware del Teensy.
---

# La cámara OpenMV H7: ver la pelota y los arcos, y contárselo al Teensy

## 1. La idea que hay que tener en la cabeza: son dos robots

El robot que ustedes conocen tiene un cerebro, el Teensy 4.1. Pero arriba lleva **una segunda
computadora completamente independiente**: la OpenMV H7. Tiene su propio procesador, su propia
memoria, su propio programa (en MicroPython, no en C++) y se programa por su propio cable USB,
con el OpenMV IDE.

El reparto de tareas es este, y es total:

| | Cámara OpenMV H7 | Teensy 4.1 |
|---|---|---|
| Lenguaje | MicroPython | C++ estilo Arduino |
| Archivo | `enviar_coordenadas_2_arcos_y_pelota.py` | `arquero.ino` / `delantero.ino` |
| Ve | 320×240 píxeles, a la velocidad que le dé (**nadie midió cuántos cuadros por segundo**; ver §10, Prueba 2) | **nada** |
| Decide | dónde están la pelota y los dos arcos | qué hacer con eso |
| Manda | 9 bytes por el cable serie | — |

**El Teensy nunca ve una imagen.** Ni un píxel. Lo único que le llega son **nueve números** por
un cable. Todo lo que el robot "sabe" del mundo visual cabe en esos nueve bytes.

### ¿Por qué está partido así?

Porque el Teensy no tiene con qué procesar video, y no es una cuestión de "ser más rápido":

- Un cuadro QVGA en RGB565 (`enviar_coordenadas_2_arcos_y_pelota.py:27-28`) son
  320 × 240 × 2 bytes = **153.600 bytes**. El Teensy 4.1 tiene 1 MB de RAM (dato de fábrica, no
  medido acá): le entrarían unos pocos cuadros y nada más, y todavía no procesó nada.
- No tiene por dónde entrar la imagen. Los pines que la cámara usa para hablar con su propio
  sensor no existen como entrada de video en la Zircon: el único cable que las une es el
  **UART** (dos pines, uno de ida y uno de vuelta).
- Y aunque entrara: recorrer 76.800 píxeles y agruparlos por color, tres veces por cuadro, es
  trabajo de sobra para el ciclo de control del robot, que tiene que estar atendiendo motores,
  giroscopio y sensores de línea.

La solución es la que usan todos los equipos serios: **la cámara es un co-procesador de visión**.
Procesa la imagen en su casa y manda el resumen. El resumen acá son coordenadas.

Consecuencia práctica que hay que interiorizar: **cuando la cámara "no anda", hay tres lugares
distintos donde puede estar el problema**, y son independientes:

1. **La óptica y el color** — la cámara no reconoce el objeto (umbrales, luz, foco).
2. **La geometría** — lo reconoce, pero convierte mal el píxel en distancia (homografía).
3. **El enlace** — lo reconoce y lo convierte bien, pero el número no llega o llega corrido
   (los 9 bytes, la sincronización).

Todo el resto de esta skill es aprender a distinguir en cuál de los tres estás. Diagnosticar los
tres a la vez es la forma más rápida de perder una tarde.

---

## 2. El hardware que hay (y el que NO hay)

Verificado en `mapa-pines-teensy.md:26-27` y en el código:

| Qué | Valor | Dónde está escrito |
|---|---|---|
| Cámara | OpenMV **H7 / H7+** | `mapa-pines-teensy.md:26` |
| Puerto en el Teensy | **Serial1** | `arquero.ino:239`, `delantero.ino:263` |
| Pin 0 | **RX** (entra el dato de la cámara) | `mapa-pines-teensy.md:26` |
| Pin 1 | **TX** (el Teensy podría contestar; hoy no hay ni un `Serial1.write()` en los dos `.ino`, ni un `uart.read()` en el `.py`: el enlace es de una sola mano) | `mapa-pines-teensy.md:27` |
| Velocidad | **19200 baudios** | `arquero.ino:77` (`BAUD_RATE`), `.py:6` (`UART(3, 19200)`) |
| Puerto en la cámara | **UART 3** | `.py:6` |
| Resolución | **QVGA** (320×240) | `.py:28` |
| Formato | **RGB565** | `.py:27` |

**Ojo con el pin 0.** Es el RX de la cámara, y hay un camino en la librería que lo podría tocar.
Verificado leyendo `zirconLib.cpp`:

- `setZirconVersion()` (`zirconLib.cpp:52-60`) lee el **pin 32** con pull-down interno: LOW →
  `"Mark1"`, HIGH → `"Naveen1"`. Como el pull-down tira el pin a masa, **lo esperable es que dé
  Mark1**, que es el que coincide con los pines de `ROBOT1` (`arquero.ino:37-59`).
- Pero en la rama `"Naveen1"` de `initializePins()`, las tres asignaciones de pin de PWM están
  **comentadas** (`zirconLib.cpp:266`, `269`, `272`). Como `motor1pwm` es una variable global sin
  inicializar (`zirconLib.cpp:13, 16, 19`), en C++ arranca en **0**. Y más abajo se ejecuta
  `pinMode(motor1pwm, OUTPUT)` (`zirconLib.cpp:322`) — o sea, `pinMode(0, OUTPUT)` **sobre el pin
  RX de la cámara**.

> **Matiz honesto:** en `arquero.ino`, `InitializeZircon()` corre en la línea **237** y
> `Serial1.begin(BAUD_RATE)` en la **239**, o sea *después*. Si `Serial1.begin()` vuelve a tomar
> el pin 0 para el UART, el `pinMode` anterior quedaría anulado y no pasaría nada. **No lo demos
> ni por confirmado ni por descartado: no lo probamos.** El test de 5 minutos es imprimir
> `getZirconVersion()` (declarada en `zirconLib.h:27`) en el `setup()` y leer qué dice.

### Lo que este robot NO tiene — no lo busques y no lo pidas

- **No hay OpenMV N6.** Hay otro robot del IITA que usa una OpenMV N6 (otro modelo de cámara,
  con acelerador de redes neuronales) y su propio protocolo. **Ese código no corre acá**: es otro
  chip, otro sensor de imagen, otro módulo de MicroPython y otro contrato de datos. Sus umbrales
  LAB tampoco sirven acá (sensor distinto → responde distinto al mismo color). Si en algún
  documento aparece "N6", "NPU", "YOLO", "FOMO" o "CRC8": es del otro robot, no de este.
- **No hay segunda cámara.** Una sola, mirando para adelante. Lo que queda fuera del campo
  visual, sencillamente no existe para el robot.
- **No hay ToF, ni ultrasonido, ni encoders, ni odometría.** La cámara es la única fuente de
  "dónde está la pelota lejos". Los 8 sensores IR TSSP58038 son la fuente de "dónde está la
  pelota cerca", y son otro sistema, con otra skill.
- **No hay aprendizaje automático en la cámara.** Todo es detección por color. Y está bien:
  para una pelota naranja y dos arcos de colores planos, la detección por color es lo más
  rápido y lo más fácil de calibrar que existe.

---

## 3. Umbrales LAB: cómo se le explica un color a la cámara

### Qué es LAB y por qué no RGB

Una cámara ve en RGB (rojo, verde, azul). El problema de RGB es que **cuando cambia la luz,
cambian los tres números a la vez**. Una pelota naranja bajo el sol es (255, 140, 40); la misma
pelota a la sombra es (120, 65, 18). Son números completamente distintos para el mismo objeto.
Si escribís un rango en RGB, te queda atado a una iluminación.

LAB separa el problema en dos partes:

- **L = Luminosidad.** Cuánta luz llega. 0 = negro, 100 = blanco. **Esto es lo que cambia
  cuando cambia la iluminación.**
- **A = eje verde ↔ rojo.** Negativo = verde, positivo = rojo/magenta.
- **B = eje azul ↔ amarillo.** Negativo = azul, positivo = amarillo.

A y B son la **cromaticidad**: "de qué color es", casi independientemente de cuánta luz haya.
La pelota naranja al sol y a la sombra tiene A y B parecidos y L muy distinto.

Por eso se usa LAB: **te deja decir "es naranja" sin tener que decir "y está iluminado así"**.
No es magia — si la luz cambia mucho, A y B también se corren un poco, sobre todo si el balance
de blancos de la cámara se movió. Pero se corren mucho menos que en RGB.

> En OpenMV, A y B van aproximadamente de -128 a +127, y L de 0 a 100. Los números concretos que
> ves en los umbrales de abajo son consistentes con ese rango.

### Cómo se lee un threshold de 6 números

Un umbral es una tupla `(L_min, L_max, A_min, A_max, B_min, B_max)`. Es una **caja** en el
espacio de color: un píxel "pertenece" si sus tres coordenadas caen dentro de los tres rangos
a la vez.

### Los tres umbrales reales de este robot

`enviar_coordenadas_2_arcos_y_pelota.py:58-60`:

```python
naranja_threshold  = (21, 67, 18, 79, -32, 127)   # Pelota naranja
amarillo_threshold = (17, 70, -27, 14, 38, 111)   # Arco amarillo
azul_threshold     = (4, 36, -13, 57, -64, -4)    # Arco azul
```

Leídos en castellano, un rango por vez:

| | L (brillo) | A (verde↔rojo) | B (azul↔amarillo) | Qué está diciendo |
|---|---|---|---|---|
| Pelota naranja | 21 a 67 | **+18 a +79** | -32 a **+127** | "medio oscuro a medio claro, bastante rojo, y el eje azul-amarillo casi no me importa" |
| Arco amarillo | 17 a 70 | -27 a +14 (neutro) | **+38 a +111** | "ni rojo ni verde, y muy del lado amarillo" |
| Arco azul | **4 a 36** (oscuro) | -13 a +57 (ancho) | **-64 a -4** | "oscuro y del lado azul; el eje rojo-verde casi no me importa" |

**Tres observaciones que salen de mirar los números, no de suponer:**

1. **El naranja está definido casi solo por A.** Su rango de B va de -32 a 127, que es
   prácticamente "cualquier cosa". O sea: la caja acepta cualquier cosa rojiza de brillo medio.
   Un buzo rojo, una zapatilla bordó o una parte roja del robot rival caen adentro.
2. **El azul exige que sea OSCURO** (L de 4 a 36) y no filtra por A. Una sombra profunda, una
   goma negra o un robot negro-azulado pueden entrar.
3. Estos números **son de la luz del laboratorio del IITA en 2025**. No son una propiedad de la
   pelota: son una propiedad de *la pelota bajo esa luz, con ese balance de blancos*. En el
   gimnasio de la Roboliga la luz va a ser otra. **Asuman desde ahora que hay que recalibrar.**

---

## 4. `find_blobs` y quedarse con el más grande

### Qué hace `find_blobs`

`find_blobs` recorre la imagen, marca cada píxel que cae dentro de la caja LAB, y después
**agrupa los píxeles marcados que están pegados entre sí**. Cada grupo conectado es un *blob*.
De cada blob te da su rectángulo, su centro (`cx()`, `cy()`) y su cantidad de píxeles
(`pixels()`).

Filtros que se le pasan (`.py:120-122`):

- `pixels_threshold=N` — descarta los blobs con menos de N píxeles marcados. **Es el filtro
  anti-ruido.**
- `area_threshold=N` — lo mismo pero sobre el área del rectángulo que lo envuelve.
- `merge=True` — si un objeto queda partido en dos o tres pedacitos (por un brillo, una sombra
  o una línea que lo cruza), los junta en uno solo. Sin esto, la pelota con un reflejo encima
  se detecta como dos blobs chicos y ninguno pasa el filtro de tamaño.

### El código real

```python
naranja_blobs  = img.find_blobs([naranja_threshold],  pixels_threshold=7,   area_threshold=7,   merge=True)
azul_blobs     = img.find_blobs([azul_threshold],     pixels_threshold=300, area_threshold=300, merge=True)
amarillo_blobs = img.find_blobs([amarillo_threshold], pixels_threshold=600, area_threshold=600, merge=True)
```
`enviar_coordenadas_2_arcos_y_pelota.py:120-122`

### Quedarse con el más grande

```python
largest_blob = max(blobs, key=lambda b: b.pixels())
```
`enviar_coordenadas_2_arcos_y_pelota.py:84`

De todos los blobs que pasaron el filtro, se queda con el que tiene **más píxeles**. La lógica
es "el más grande es el de verdad, los otros son ruido". **El criterio se sostiene** mientras el
objeto real sea el más grande de su color en la imagen, y **se cae** cuando hay algo de ese color
más grande que el objeto: ahí el robot apunta con toda confianza al lugar equivocado. No hay
ninguna otra verificación: ni forma, ni redondez, ni proporción.

### ⚠️ El azul es la mitad de exigente que el amarillo — y eso es un riesgo real

Los dos arcos son **objetos del mismo tamaño físico**. Pero:

- amarillo: `pixels_threshold=600`
- azul: `pixels_threshold=**300**`

El detector de azul acepta manchas de **la mitad de tamaño** que el de amarillo. Combinado con
que su umbral LAB pide "oscuro y azulado, cualquier A" (§3), el arco azul es claramente **el más
propenso a falso positivo** de los tres: una sombra grande, un pantalón, una parte oscura de la
estructura de la cancha o un robot negro pueden juntar 300 píxeles sin problema.

Y el falso positivo del azul no es inocuo: `hayarco_azul` sale de ahí (`arquero.ino:313-316`),
y `Xaz`/`Yaz` viajan igual en la trama. Cualquier lógica futura que use el arco azul va a estar
apuntando a un fantasma.

**Propuesta (falta validar en banco):** igualar `pixels_threshold` de azul y amarillo en 600, o
al menos medir cuántos píxeles ocupa cada arco a 50 / 100 / 150 cm y elegir el número con datos.
No lo cambien "porque sí": si el arco azul de la cancha real ocupa menos píxeles (está más lejos,
o es más chico de lo que creen), subirlo a 600 lo apaga por completo. **Primero medir, después
tocar.**

---

## 5. La homografía: por qué un píxel no es una distancia

### El problema

`find_blobs` te dice "la pelota está en el píxel (180, 210)". Eso no le sirve al robot. El robot
necesita "la pelota está a 60 cm adelante y 12 cm a la derecha".

Convertir uno en otro no es una regla de tres. La cámara mira el piso **en diagonal**, así que
el piso aparece deformado por perspectiva: un centímetro cerca del robot ocupa muchos píxeles,
y un centímetro lejos ocupa poquísimos. Las líneas paralelas del campo convergen.

### La solución: una matriz de 3×3

La transformación que endereza una perspectiva plana se llama **homografía**. Es una matriz de
3×3 que, aplicada a un punto de la imagen, devuelve el punto correspondiente sobre el plano del
piso. Los nueve números salen de una calibración: se ponen marcas en posiciones conocidas del
piso, se anota en qué píxel aparece cada una, y se resuelve el sistema.

```python
def transformarcoordenadas(u, v):
    H = [[ 4.49341044e-02, -9.48228474e-01,  7.78932109e+02],
         [-2.39913185e+00, -5.65934886e-02,  3.91128921e+02],
         [-1.81344856e-03,  1.15408531e-01,  1.00000000e+00]]

    denominator = H[2][0]*u + H[2][1]*v + H[2][2]
    x = (H[0][0]*u + H[0][1]*v + H[0][2]) / denominator
    y = (H[1][0]*u + H[1][1]*v + H[1][2]) / denominator

    return x, y
```
`enviar_coordenadas_2_arcos_y_pelota.py:65-76`

La división por `denominator` **es** la perspectiva: es lo que hace que la escala cambie según
dónde estés en la imagen.

### La corrección por la altura de la pelota

Después de la homografía hay un segundo paso:

```python
h = 18.7              # altura cámara (cm)          -> .py:48
r = 13.5/(2*math.pi)  # radio pelota (cm) ≈ 2,15 cm -> .py:49
...
X = x * (h - r) / h
Y = y * (h - r) / h
```
`enviar_coordenadas_2_arcos_y_pelota.py:48-49, 96-97`

El mecanismo: la homografía asume que lo que ves está **apoyado en el piso**. Pero el centro del
blob de la pelota no está en el piso, está a `r` centímetros de altura (el radio; sale de una
circunferencia medida de 13,5 cm). El rayo que va del lente al centro de la pelota, si lo
prolongás hasta el piso, cae **más lejos** de lo que la pelota realmente está. El factor
`(h - r)/h` = (18,7 - 2,15)/18,7 ≈ **0,885** es lo que el código usa para achicar esa
exageración. (Que el número esté bien elegido para este montaje **no está medido**: se comprueba
con la Prueba 5 del §10.)

> 🔎 **Detalle a mirar, no un bug declarado:** ese factor se aplica **también a los arcos**,
> porque `procesar_blob` es una sola función que se usa para los tres colores
> (`.py:79-107`, llamada en `.py:138-140`). El `r` es el radio de la pelota, no la altura del
> centro del arco. Falta chequear con la cancha real si eso mete un error apreciable en `Xam` y
> `Xaz`. Es una observación de lectura de código, no algo medido.

### ⚠️ La regla dura: NO TOQUEN ESOS NUEVE NÚMEROS

La matriz `H` está atada a **cosas físicas del robot**:

- la **altura** a la que está montada la cámara (los 18,7 cm de `.py:48`),
- el **ángulo** con el que mira,
- el **lente** que tiene puesto,
- la **resolución** QVGA (`.py:28`) — la matriz mapea *píxeles*, y en otra resolución los
  píxeles son otros,
- el `hmirror` + `vflip` (`.py:42-43`), que dan vuelta la imagen antes de todo esto.

Si alguna de esas cosas cambia, **la matriz deja de servir y hay que recalibrarla entera**. En
particular:

- Aflojaste y volviste a apretar el soporte de la cámara → **recalibrar**.
- Cambiaste `QVGA` por `QQVGA` para ganar FPS → **recalibrar** (y revisar los
  `pixels_threshold`, porque el área en píxeles se hace cuatro veces más chica).
- Le cambiaste el lente → **recalibrar**.
- Cambiaste la luz del gimnasio → **NO hace falta tocar la matriz.** La geometría no depende de
  la luz.

**Esa última distinción es la más útil de todo el capítulo:**

> **El COLOR (los umbrales LAB) depende de la LUZ → se recalibra en cada cancha nueva.**
> **La GEOMETRÍA (la matriz H) depende del MONTAJE → NO se toca mientras el robot no se desarme.**

Muchos equipos pierden horas moviendo la homografía cuando el problema era la luz. No hagan eso.
Si nadie desarmó el robot, **esos nueve números son sagrados**.

---

## 6. El contrato de datos: 9 bytes, verificado en los dos lados

Este es el punto de contacto entre las dos computadoras. Los dos programas tienen que estar de
acuerdo **byte por byte**, y ninguno de los dos verifica que el otro cumpla.

### El formato

| Byte | Contenido | Rango |
|---|---|---|
| 0 | **201** — marca "acá empieza, y viene la pelota" | fijo |
| 1 | `Xp` — distancia de la pelota | 0..200 |
| 2 | `codedYp` = `Yp + 100` — lateral de la pelota | 0..200 |
| 3 | **202** — marca "ahora el arco amarillo" | fijo |
| 4 | `Xam` | 0..200 |
| 5 | `codedYam` = `Yam + 100` | 0..200 |
| 6 | **203** — marca "ahora el arco azul" | fijo |
| 7 | `Xaz` | 0..200 |
| 8 | `codedYaz` = `Yaz + 100` | 0..200 |

**X = qué tan lejos está** (hacia adelante). **Y = para qué lado está** (negativo a un lado,
positivo al otro). Se ve en cómo lo usa el arquero: `Xp <= tolerancia_cercania` significa "la
pelota está cerca" y `abs(Yp) <= 3` significa "está centrada" (`arquero.ino:1038`, dentro del
`case moverce_derecha` que arranca en la 1030).

> **Los umbrales no son iguales en los dos robots**, así que no los mezclen:
> `tolerancia_cercania` vale **140.0** en el arquero (`arquero.ino:110`) y **50.0** en el
> delantero (`delantero.ino:119`); y el "está centrada" es `abs(Yp) <= 3` en el arquero
> (`arquero.ino:1038`) contra `abs(Yp) <= 5` en el delantero (`delantero.ino:1054`). Qué unidad
> real es ese "140" **no está medido** — la Prueba 5 del §10 es la que lo traduce a centímetros.

### Lado que MANDA — la cámara

```python
    # Codificar Y
    codedYp  = Yp  + 100
    codedYam = Yam + 100
    codedYaz = Yaz + 100

    packet = [
        201, int(Xp),  int(codedYp),
        202, int(Xam), int(codedYam),
        203, int(Xaz), int(codedYaz)
    ]

    uart.write(bytearray(packet))
    print("Enviando:", packet)
```
`enviar_coordenadas_2_arcos_y_pelota.py:142-156`

### Lado que RECIBE — el Teensy

```cpp
  if (Serial1.available() >= 9)
  {
    header1 = Serial1.read();
    if (header1 == 201)
    {
      codedXp  = Serial1.read();
      codedYp  = Serial1.read();
      header2  = Serial1.read();
      codedXam = Serial1.read();
      codedYam = Serial1.read();
      header3  = Serial1.read();
      codedXaz = Serial1.read();
      codedYaz = Serial1.read();

      if (header1 == 201 && header2 == 202 && header3 == 203)
      {
        Xp  = codedXp;
        Yp  = codedYp  - 100;
        Xam = codedXam;
        Yam = codedYam - 100;
        Xaz = codedXaz;
        Yaz = codedYaz - 100;
        ...
        if ( Xp == 0 ) { haypelota = false; }
        else { haypelota = true; millis_pelota = millis(); }
        ...
      }
    }
    else
    {
      hayarco_azul = false;
      hayarco_amarillo = false;
      haypelota = false;
    }
  }
```
`arquero.ino:263-329` (idéntico en `delantero.ino:287-353`)

### Por qué la Y va corrida +100

**Un byte guarda 0 a 255. No existen los negativos.** Pero la pelota puede estar a la izquierda,
y eso es un `Y` negativo.

La solución es la más vieja del oficio: **sumar un desplazamiento antes de mandar y restarlo al
recibir**. La cámara manda `Y + 100`; el Teensy hace `- 100`. Así un `Y = -30` viaja como el byte
70, que es perfectamente legal.

Por eso la cámara recorta `Y` a ±100 antes de codificar (`.py:102-105`): así `codedY` queda entre
0 y 200 y nunca se sale del byte.

### El recorte a 0..200 no es un detalle: es lo que hace que las marcas sirvan

Juntando los tres recortes de `procesar_blob` (`.py:100-105`):

```python
    if X>200:
        X = 200
    if Y+100>200:
        Y = 100
    if Y+100<0:
        Y = -100
```

**los seis bytes de datos quedan siempre entre 0 y 200.** Y las tres marcas son 201, 202 y 203,
que están **fuera** de ese rango. O sea: las marcas de inicio **no pueden confundirse con un
dato**. Eso no es casualidad, es la parte bien hecha del protocolo, y es de lo que depende que el
lector del Teensy pueda volver a engancharse cuando se desacomoda (§7).

> **Ojo con la asimetría, que es el agujero real:** la X **no** lleva desplazamiento, viaja cruda,
> y el recorte es **incompleto**: `.py:100-101` solo recorta por arriba, no por abajo. Si la
> homografía llegara a devolver una X negativa, `int(X)` es negativo y `bytearray()` recibe un
> valor fuera de 0..255. En Python eso levanta un `ValueError`; **falta confirmar el
> comportamiento exacto del MicroPython de esta cámara**, pero el efecto esperable es que el
> script se corte y la cámara **deje de mandar del todo**. Y en el arquero, que la cámara deje de
> mandar es el bug A4 (§7): el robot se queda congelado en el último dato. Es una revisión de 2
> minutos que vale la pena hacer.

### El 0 significa "no lo veo" — NO "está pegado"

Este es el punto que más confunde y hay que tenerlo clarísimo:

```python
def procesar_blob(blobs, dibujar_color):
    if not blobs:
        return 0, 0  # Si no hay detección, se manda 0
```
`enviar_coordenadas_2_arcos_y_pelota.py:79-81`

Cuando la cámara **no encuentra** el objeto, manda `X = 0`. Y del otro lado:

```cpp
if ( Xp == 0 ) { haypelota = false; }
```
`arquero.ino:300-301`

O sea: **`X = 0` es el código de "no lo veo"**, no "lo tengo a distancia cero". Las tres
banderas (`haypelota`, `hayarco_amarillo`, `hayarco_azul`) salen exactamente de esa comparación
(`arquero.ino:300-316`).

Dos consecuencias reales:

1. **Es ambiguo.** Si algún día la homografía devolviera de verdad `X = 0` para un objeto
   pegadísimo al robot, el Teensy lo leería como "desapareció". En la práctica es improbable
   (la pelota pegada probablemente ni entre en el campo visual), pero es una ambigüedad del
   contrato, no una casualidad afortunada. Un protocolo mejor usaría un valor **imposible** como
   centinela — por ejemplo 255, que las coordenadas reales nunca alcanzan porque están
   recortadas a 200.
2. **La Y no tiene ese problema**, porque va corrida: cuando no hay detección se manda
   `Y = 0` → `codedY = 100` → el Teensy decodifica `Yp = 0`. Es un valor válido y no dice nada.
   **La única bandera de "no lo veo" es la X.**

### Un dato que sirve para razonar los rangos

El Teensy lee un byte crudo, que puede valer de 0 a 255, así que `Yp = codedYp - 100` podría en
teoría llegar a **+155**. Que en la práctica no pase de +100 **depende enteramente del recorte de
la cámara** (`.py:102-105`). Si algún día ven un `Yp` mayor a 100 en el monitor serie, no es
que la pelota se fue de la cancha: es que perdieron el sincronismo y están leyendo un byte que
no era la Y. Es un **síntoma de diagnóstico gratis**.

---

## 7. Las dos fragilidades del enlace (están en `bugs-conocidos.md`, DEL-06)

El enlace tiene una parte bien resuelta (las marcas fuera del rango de datos, §6) y una parte sin
defensa. Hay que saber cuál es cuál, porque si no se arregla lo que no está roto.

### ⚠️ Corrección a `bugs-conocidos.md` — leé esto antes de creerle al documento

`bugs-conocidos.md` (DEL-06, punto 1) afirma: *"`Xp`, `Yp`, `Xam`… viajan como bytes de 0 a 255,
así que el valor 201 puede aparecer como dato"*.

**Eso no es cierto en este robot.** El `.py` recorta los seis datos a 0..200 antes de armar el
paquete (`.py:100-105`, ver §6), así que 201 **nunca** sale como dato. El propio
`bugs-conocidos.md` avisa, en su cierre, que **el `.py` no fue auditado** — y esa afirmación es
justamente la que se les escapó por no leerlo.

Se los marcamos en vez de taparlo, porque la conclusión práctica cambia: **la sincronización de
este protocolo es mejor de lo que dice el documento.** Lo que sigue roto es otra cosa.

### Fragilidad 1 — cuando se pierde un byte, se decide con datos viejos y en silencio

**El mecanismo, paso a paso.** El enlace es un cable con un conector. Si se pierde **un solo
byte** — interferencia, un contacto flojo, la cámara reiniciándose en medio de una escritura —
todos los bytes siguientes quedan **corridos en uno**. Hay dos casos, y dan resultados distintos:

**Caso A — el byte se pierde entre tramas.** El lector agarra como `header1` un byte de datos
(que está entre 0 y 200) → **no es 201** → cae en el `else` de `arquero.ino:323-328`, apaga las
tres banderas y **descarta un solo byte**. Al descartar de a uno, en unas pocas vueltas de
`loop()` tiene que volver a engancharse con el 201 verdadero. **Leyendo el código, la
recuperación está garantizada por el recorte a 0..200** — pero medida en banco, no está: eso es
la Prueba 6 del §10.

**Caso B — el byte se pierde después de que el lector ya leyó el 201.** Este es el feo. El lector
sigue leyendo 8 bytes, pero corridos: donde esperaba el `header2` encuentra un dato cualquiera.
El `if` de la línea **277** falla… **y no hay `else`**. No se actualiza nada, no se avisa nada,
**y `Xp`, `Yp`, `Xam`… quedan con los valores de la trama anterior**. El robot sigue decidiendo
con datos viejos y nadie se entera. Recién se realinea en las vueltas siguientes, por el Caso A.

**En una frase:** el protocolo *se recupera*, pero mientras se recupera **miente en silencio**.
Lo que falta no es la resincronización — es el aviso de que el dato está podrido.

**Cómo se ve en la cancha:** el robot "poseído". Arranca hacia donde la pelota estaba, pega
tirones, o se queda mirando fijo un lugar vacío.

**La defensa de fondo** sería un **byte de verificación** (un checksum: por ejemplo el XOR de los
bytes de datos, mandado como byte extra). Con eso, una trama corrida da un XOR que no coincide y
se descarta con certeza. **Requiere tocar los dos programas a la vez** — el `.py` y el `.ino`, y
además la trama deja de tener 9 bytes — y por eso `correcciones-propuestas.md:614-615` lo deja
explícitamente como tema aparte, no como parche mínimo.

El parche mínimo que sí está propuesto (`correcciones-propuestas.md:591-599`) es agregarle un
`else` al `if` de las tres marcas, para que una trama dudosa apague las banderas en vez de
dejar el dato viejo pegado:

```cpp
      if (header1 == 201 && header2 == 202 && header3 == 203)
      {
        ...
      }
      else
      {
        // Trama rota o desalineada: NO seguimos creyendo los datos viejos.
        hayarco_azul = false;
        hayarco_amarillo = false;
        haypelota = false;
      }
```

> ⚠️ Efecto secundario real, y no menor: apagar `haypelota` ante cualquier trama dudosa produce
> **parpadeo** donde antes había un valor pegado. En un estado que persigue la pelota, eso se
> traduce en frenar/arrancar entrecortado. La alternativa es contar tramas malas y recién apagar
> después de 3 seguidas. **Propuesto, falta validar en banco.**

### Fragilidad 2 — no hay descarte de tramas viejas

**El mecanismo.** El Teensy tiene un buffer de recepción: los bytes que llegan se van guardando
solos aunque el programa esté ocupado. `bugs-conocidos.md` (DEL-06, punto 2) lo estima en **64
bytes, que son 7 tramas** — *ese número viene del documento, no lo verificamos en el código de
la librería de Arduino; conviene confirmarlo midiendo `Serial1.available()`.*

El lector del Teensy lee **exactamente una trama por vuelta de `loop()`**: el `if` de la línea
263 no es un `while`. Si la cámara manda más rápido de lo que el Teensy consume, la cola crece.
Y una cola de datos de posición es veneno: el robot decide con la posición de hace varios
cuadros. Peor: cuando el buffer se llena, los bytes nuevos se pierden **en el medio de una
trama**, y volvés a la fragilidad 1.

**El parche propuesto** (`correcciones-propuestas.md:582-589`) es tirar las tramas viejas de a
9 bytes exactos, para no romper el alineamiento, y quedarse con la última. Va **antes** del
`if (Serial1.available() >= 9)` de la línea 263:

```cpp
  // Descartar tramas viejas: si en el buffer hay mas de una, tiramos las viejas
  // de a 9 bytes (asi no rompemos el alineamiento) y nos quedamos con la ultima.
  while (Serial1.available() >= 18)
  {
    for (int k = 0; k < 9; k++) { Serial1.read(); }
  }
```

> ⚠️ **Medir antes de aplicar.** Si el loop del Teensy corre más rápido que la cámara, ese
> `while` **no se ejecuta nunca** y el parche no aporta nada — pero tampoco molesta. Y si sí se
> ejecuta, el robot pasa a tener datos más frescos y puede volverse **más nervioso**: hoy el
> retraso del buffer le estaba haciendo de filtro. Puede requerir reajustar tolerancias.
> **Propuesto, falta validar en banco.**

### Lo que el enlace SÍ tiene previsto y conviene no romper

Si la cámara se desconecta **del todo**, `Serial1.available()` nunca llega a 9, el bloque entero
no corre, y en el delantero están escritos los chequeos de `millis() - millis_pelota >= 500`
(`delantero.ino:531, 582, 786, 831`) que lo sacan del estado. **En el delantero, la desconexión
total está contemplada en el código** — falta validarla en banco (Prueba 6 del §10).

**En el arquero NO está contemplada.** `millis_pelota` se actualiza en `arquero.ino:305`, pero
las cuatro comparaciones que la usan (`arquero.ino:511, 562, 627, 733`) caen dentro de los `case`
`APUNTAR_PELOTA` (480), `AVANZANDO` (542), `CENTRANDO_horario` (592) y `CENTRANDO_antihorario`
(699), que son **estados del delantero**.

¿Por qué no se ejecutan nunca? Ojo con el mecanismo, porque es fácil decirlo mal: **no es el
`#define ROBOT1`**. Ese `#define` solo elige pines y umbrales de blanco (`arquero.ino:37-59`); el
`switch` con TODOS los estados de los dos robots se compila entero igual. Lo que los deja
inalcanzables son dos cosas verificables:

1. el estado inicial es `impulso_inicial` (`arquero.ino:131`), que es un estado del arquero, y
2. revisando uno por uno los `case` del arquero (`arquero.ino:1016-1207`), **ninguno asigna un
   estado del delantero**: solo saltan entre `moverce_derecha`, `moverce_izquierda`,
   `impulso_derecha`, `impulso_izquierda`, los cuatro `PATEANDO_*_arquero` y
   `avanzar_despues_de_patear`.

O sea: si la cámara del arquero se cuelga, `haypelota`, `Xp` e `Yp` quedan **congelados en el
último valor para siempre**. Eso es el bug A4 de `bugs-conocidos.md`, y el parche propuesto (un
watchdog de cámara) está en `correcciones-propuestas.md:203-235`.

---

## 8. Los LEDs de la cámara: diagnóstico gratis antes de tocar el Teensy

La OpenMV tiene un LED RGB que el programa usa como tablero de instrumentos. **Está ahí, ya
programado, y casi nadie lo mira.**

```python
led_rojo  = pyb.LED(1)    # Naranja / Pelota
led_verde = pyb.LED(2)    # Amarillo / Arco derecho
led_azul  = pyb.LED(3)    # Azul / Arco izquierdo
```
`enviar_coordenadas_2_arcos_y_pelota.py:11-13`

**Al arrancar** parpadea el verde dos veces, con 0,4 s prendido y 0,4 s apagado
(`.py:16-22`) — y eso pasa **antes** de `sensor.reset()`.

**En cada vuelta del bucle** (`.py:124-135`):

```python
    led_rojo.off()
    if naranja_blobs:  led_rojo.on()      # ve la PELOTA

    led_azul.off()
    if azul_blobs:     led_azul.on()      # ve el arco AZUL

    led_verde.off()
    if amarillo_blobs: led_verde.on()     # ve el arco AMARILLO
```

Y del lado del Teensy hay un LED equivalente:

```cpp
 digitalWrite(LED_BUILTIN, haypelota);
```
`arquero.ino:261` — el LED de la placa del Teensy prende cuando `haypelota` es verdadero.

### La tabla de diagnóstico de 10 segundos

Pongan la pelota adelante del robot y miren **los dos LEDs**:

| LED cámara (rojo) | LED Teensy | Qué significa | Dónde buscar |
|---|---|---|---|
| No parpadea nada al encender | — | El script de la cámara **no arrancó** | La cámara no tiene alimentación, o el `.py` no está guardado como `main.py` en la cámara |
| Parpadea al arrancar, después apagado con la pelota adelante | apagado | La cámara vive pero **no reconoce el color** | **Umbrales LAB / luz** (§3 y §9). El Teensy está bien. |
| Prendido | apagado | La cámara la ve, **el número no llega o llega mal** | **El enlace**: cable, pin 0, baudios, sincronismo (§6, §7) |
| Prendido | prendido | Cámara y enlace **sanos** | Si igual el robot no reacciona: es la máquina de estados o la geometría (homografía). Otra skill. |
| Apagado | prendido y **fijo** | La cámara dejó de ver, pero el Teensy quedó **congelado** en el último dato | Exactamente el bug A4 (§7). Falta el watchdog. |

**Esa tabla te ahorra la mitad de las tardes de debugging.** Divide el problema en dos mitades
antes de abrir un solo archivo.

---

## 9. Recalibrar bajo luz nueva — el procedimiento

**Este es EL problema que van a tener en la Roboliga.** Los tres umbrales de `.py:58-60` fueron
elegidos bajo la luz del laboratorio del IITA en 2025. Un gimnasio con tubos fluorescentes, o
con luz natural entrando por una ventana, o con LEDs, da otros números. Y no lo van a poder
hacer la noche anterior desde casa: **hay que hacerlo en la cancha, con la luz de la cancha**.

Vayan preparados: **notebook + cable USB de la cámara + OpenMV IDE ya instalado + la pelota
oficial**. Presupuesten 15-20 minutos.

### Paso 0 — condiciones (no salteen esto)

- El robot **armado y en el piso**, con la cámara en su soporte definitivo. No con la cámara en
  la mano ni apoyada en una mesa: la altura y el ángulo cambian todo.
- La luz **de la cancha**, con las luces que van a estar prendidas durante el partido.
- Si hay gente alrededor que va a estar en el partido, que esté. Los falsos positivos vienen del
  público.

### Paso 1 — conectar y ver

1. Cable USB de la cámara a la notebook. Abrir **OpenMV IDE**.
2. Abrir el archivo `enviar_coordenadas_2_arcos_y_pelota.py`.
3. Botón de conectar (el enchufe abajo a la izquierda) → botón de **Play**.
4. Mirar la ventana **Frame Buffer** arriba a la derecha.

**Si el Frame Buffer se ve a color y se mueve → el sensor está sano.** Ya descartaste la mitad
de las causas posibles. Si se ve negro o congelado, el problema no es de color: es la cámara,
el cable o el firmware.

### Paso 2 — leer los colores reales

Poné la pelota en el campo visual. En el IDE:
**Tools → Machine Vision → Threshold Editor** (el nombre del menú puede variar un poco según la
versión del IDE). Elegí la fuente **Frame Buffer**, y el espacio de color **LAB**.

Vas a ver seis deslizadores: son exactamente los seis números de la tupla. Movelos hasta que
**la pelota quede blanca y todo lo demás negro** en la vista binaria. Ese es el umbral.

**Cómo se mueven bien:**
1. Empezá con los seis rangos bien abiertos y andá **cerrando**.
2. Ajustá **A y B primero** (el color), dejando L bien abierto.
3. Recién al final cerrá **L** lo justo para sacar sombras y brillos.
4. **Movés la pelota por toda la imagen** (cerca, lejos, a los costados, en la sombra del robot)
   y verificás que siga saliendo blanca. Si al moverla desaparece, el rango está demasiado
   apretado.

Método complementario, más objetivo que el ojo: pausar con la pelota adelante y pedir las
estadísticas de un recuadro sobre la pelota.

```python
# Pegar temporalmente dentro del while, y mirar la terminal del IDE.
# El (x, y, ancho, alto) hay que ajustarlo al lugar donde esté la pelota.
print(img.get_statistics(roi=(140, 100, 40, 40)))
```

Eso te da la media y la desviación de L, A y B de esa zona. El umbral razonable es
**media ± 2 desviaciones**, por canal, y después lo afinás con el editor. *(La firma de
`get_statistics` con `roi` es API estándar de OpenMV; si el IDE se queja, revisá la
documentación de la versión que tengan instalada.)*

### Paso 3 — repetir para los tres colores

Mismo procedimiento con el arco amarillo y con el arco azul. **El azul es el que más atención
pide**, por lo de §4: prestá atención a que la caja no se coma sombras.

### Paso 4 — anotar y guardar

Reemplazar las tres líneas en el `.py` **dejando las viejas comentadas con la fecha y el lugar**:

```python
# --- Umbrales LAB ---
# 2025, laboratorio IITA (los que ganaron el Nacional):
# naranja_threshold  = (21, 67, 18, 79, -32, 127)
# amarillo_threshold = (17, 70, -27, 14, 38, 111)
# azul_threshold     = (4, 36, -13, 57, -64, -4)

# 2026-XX-XX, <lugar>, luz <describir: tubos / LED / sol por ventana>:
naranja_threshold  = (..., ..., ..., ..., ..., ...)
amarillo_threshold = (..., ..., ..., ..., ..., ...)
azul_threshold     = (..., ..., ..., ..., ..., ...)
```

Guardar en la cámara (**Tools → Save open script to OpenMV Cam**, para que quede como `main.py`
y arranque solo sin la notebook). **Si no lo guardan en la cámara, el robot desenchufado de la
notebook corre el programa VIEJO.** Es un clásico de torneo.

### Paso 5 — el balance de blancos (el enemigo silencioso)

Hoy el script hace esto:

```python
sensor.set_auto_whitebal(True)
sensor.set_auto_gain(True)
#sensor.set_auto_exposure(True)
sensor.skip_frames(time=2000)
```
`enviar_coordenadas_2_arcos_y_pelota.py:31-36`

Es decir: **deja el balance de blancos y la ganancia en automático, para siempre**. Le da 2
segundos para acomodarse al arrancar, y después los deja libres.

El problema del automático: la cámara "aprende" el color promedio de lo que ve. Cuando entra al
cuadro un robot negro, o el arco, o se apaga una luz, **el automático se reajusta y corre los
valores A y B de todo lo demás**. Los umbrales que calibraste hace 5 minutos dejan de encajar,
sin que nadie haya tocado nada. Es la causa clásica de *"recién andaba y ahora no"*.

**Propuesta (falta validar en banco):** dejar que se acomode al arrancar, **leer** los valores a
los que llegó, y **fijarlos**. Se hace agregando esto justo después del `skip_frames`:

```python
# --- Paso A: correr esto UNA VEZ para leer los valores de esta cancha ---
print("gain_db  :", sensor.get_gain_db())
print("rgb_gain :", sensor.get_rgb_gain_db())
print("exposure :", sensor.get_exposure_us())

# --- Paso B: pegar los valores impresos aca y dejar esto en competencia ---
# sensor.set_auto_gain(False, gain_db=<pegar>)
# sensor.set_auto_whitebal(False, rgb_gain_db=(<pegar>, <pegar>, <pegar>))
# sensor.set_auto_exposure(False, exposure_us=<pegar>)
```

> ⚠️ **Nada de esto está probado en este robot.** Las firmas `set_auto_gain(False, gain_db=...)`,
> `set_auto_whitebal(False, rgb_gain_db=(...))` y `set_auto_exposure(False, exposure_us=...)` son
> la API estándar de OpenMV y el script ya tiene una línea comentada con esa forma
> (`.py:39`: `#sensor.set_auto_exposure(False, exposure_us=37000)`), pero **hay que confirmarlas
> contra la documentación de la versión de firmware que tenga esa cámara** antes de dar por hecho
> que corren. Y hay que **recalibrar los umbrales LAB después de fijar los automáticos**, no
> antes: fijarlos corre los colores.
>
> **Riesgo de hacerlo:** si fijás la exposición y después cambia la luz de verdad (nube, alguien
> apaga una fila de luces), la cámara ya no compensa y podés perder la pelota entera. El
> automático es peor para la estabilidad del color pero mejor para sobrevivir a un cambio grande.
> Es un intercambio real, no una mejora gratis. **Decisión de banco, con la cancha real.**

---

## 10. Probar el enlace en banco — con criterio de aceptación

Seis pruebas, de la más barata a la más cara. **Cada una tiene un criterio numérico: si no lo
cumple, no se pasa a la siguiente.**

### Prueba 1 — ¿la cámara ve? (3 minutos, sin Teensy)

Cámara conectada al IDE, robot armado, pelota adelante.
**Criterio:** el Frame Buffer se ve a color; con la pelota adelante el LED rojo queda prendido;
al sacarla se apaga; al moverla el rectángulo la sigue (el rectángulo lo dibuja `.py:87`).
**Sin esto, todo lo demás es al pedo.**

### Prueba 2 — ¿la cámara manda lo que creemos, y a cuántos cuadros? (5 minutos, sin Teensy)

El script ya imprime la trama: `print("Enviando:", packet)` (`.py:156`). Mirar la terminal del
IDE con la pelota adelante.
**Criterio:** cada línea tiene 9 números, el 1°, 4° y 7° son **exactamente 201, 202 y 203**, y
todos los otros están entre **0 y 200**. Con la pelota tapada, el 2° número (`Xp`) tiene que ser
**0**.

**Aprovechen y midan los FPS de la cámara, que nadie los midió nunca.** El script ya crea el
reloj (`clock = time.clock()`, `.py:45`) y lo pisa en cada vuelta (`clock.tick()`, `.py:113`),
pero **nunca imprime el resultado**. Agregando una línea al final del `while`:

```python
    print("fps:", clock.fps())
```

**Ese número es el techo real de todo el sistema de visión.** Anótenlo: si la cámara da 25 cuadros
por segundo, ninguna mejora del lado del Teensy va a conseguir datos más frescos que 40 ms.
(Y ojo: `print()` por USB también cuesta tiempo, así que el número con el IDE conectado es un
piso pesimista del que da desconectada.)

### Prueba 3 — ¿el número llega al Teensy? (10 minutos)

Descomentar los `Serial.print` del bloque de lectura (`arquero.ino:292-297`), compilar, cargar,
abrir el monitor serie. *(En el Teensy el baudio del monitor USB no cambia nada, pero
`Serial.begin(BAUD_RATE)` está en 19200 — `arquero.ino:238` — así que poné 19200 y listo.)*
**Criterio:** los valores que aparecen en el monitor del Teensy coinciden con los que imprime el
IDE de la cámara. Si difieren, o si aparecen `Yp` mayores a 100, hay problema de sincronismo.

### Prueba 4 — ¿a qué ritmo llegan? (5 minutos)

Es la medición #3 de `correcciones-propuestas.md:139`. Agregar una sola línea en
`arquero.ino`, **justo adentro del `if` de la línea 277** (el que valida las tres marcas), o sea
inmediatamente después de la llave de la línea 278:

```cpp
Serial.println(millis() - millis_pelota);
```

**Dónde va importa.** Tiene que estar **antes de la línea 305**, que es donde `millis_pelota` se
actualiza. Si la ponen después, siempre van a leer 0 y no van a medir nada. Y ojo con qué mide:
`millis_pelota` solo se refresca cuando **se ve la pelota** (`arquero.ino:300-306`), así que la
prueba solo tiene sentido con la pelota fija y visible.

Robot quieto, pelota fija adelante, monitor serie a 19200, 30 segundos.
**Criterio:** todos los números **menores a 200 ms** → bien. Cerca de 500 → subir el umbral de
cualquier watchdog a (peor valor visto × 3). **Mayores a 500 → hay un problema de comunicación
de fondo y no se sigue adelante hasta entenderlo.**

Contexto para interpretar el número, con la única cuenta que sí podemos hacer de antemano: a
19200 baudios en formato 8N1 son 10 bits por byte (8 de dato + arranque + parada), o sea 1.920
bytes por segundo. **Los 9 bytes tardan 4,7 ms en el cable.** Si el intervalo medido es de 150 ms,
el cable está usando el 3 % del tiempo: el cuello de botella está en otro lado — el bucle de la
cámara (tres `find_blobs` sobre QVGA) o el bucle del Teensy. **Cuál de los dos es, lo dice
comparar este número con los FPS de la Prueba 2**, no la intuición.

### Prueba 5 — ¿el número significa lo que creemos? (20 minutos)

La geometría. Robot quieto en el piso. Cinta métrica. Pelota a **30, 50, 80, 100 y 150 cm**
exactamente adelante, y después a 50 cm pero corrida 20 cm a cada lado.
Anotar `Xp` e `Yp` en cada posición.
**Criterio:**
- `Xp` **crece siempre** al alejar la pelota (monótono, sin saltos raros).
- La relación entre `Xp` y los centímetros reales es **estable** — no importa si el factor es 1,
  0,9 o 1,3, importa que sea **el mismo en todas las distancias**. Si a 30 cm el factor es uno y
  a 150 cm es otro, la homografía no está describiendo bien este montaje.
- `Yp` cambia de signo al pasar la pelota de un lado al otro, y vale ≈ 0 con la pelota centrada.
- Anotar la distancia máxima a la que todavía la detecta. **Ese número es el alcance real del
  robot** y hay que conocerlo.

### Prueba 6 — robustez del enlace (5 minutos)

`correcciones-propuestas.md:621-623`: desenchufar el cable de la cámara **2 segundos** y volver
a enchufar.
**Criterio:** en menos de 1 segundo vuelve a seguir la pelota, y durante la desconexión el robot
**nunca** persigue una pelota que no está.

*(Leyendo el código, en el arquero esta prueba **debería fallar**: no hay nada que envejezca
`Xp`, `Yp` ni `haypelota` — es el bug A4 del §7. Háganla igual y **confírmenlo con el robot**
antes de aplicar el watchdog: si no vieron el "antes", después no van a poder decir si el parche
cambió algo. Ver un bug fallar a propósito, una vez, enseña más que leerlo diez veces.)*

> **Ninguna de estas pruebas la puede cerrar una IA.** Ni "compila" ni "el código dice esto"
> prueban nada. El veredicto lo da el que tiene el robot en la mano.

---

## 11. Errores típicos — tabla de síntoma → causa

| Síntoma | Causa más probable | Trampa (lo que parece) | Qué hacer |
|---|---|---|---|
| La cámara no ve la pelota, pero el Frame Buffer se ve a color | Umbrales LAB de otra luz | "se rompió la cámara" | Recalibrar (§9). La óptica está sana. |
| Detecta bien en el IDE, el robot no reacciona | El enlace: cable, pin 0, sincronismo | "la detección no anda" | Tabla de LEDs (§8) → Pruebas 2 y 3 (§10) |
| El robot persigue una pelota que no está / queda clavado mirando un punto | Dato congelado: la cámara dejó de mandar y nada envejece las variables | "la máquina de estados se colgó" | Es A4. Watchdog de cámara. |
| Arranca hacia donde la pelota **estaba**; tirones raros | Tramas viejas encoladas, o sincronismo perdido | "el robot está poseído" | §7, fragilidades 1 y 2. Medir `Serial1.available()` primero. |
| La pelota se detecta partida en dos manchas chicas y ninguna pasa el filtro | Un brillo o una sombra la parten | "la cámara pierde la pelota" | `merge=True` ya está puesto (`.py:120`); revisar `pixels_threshold` y el foco del lente |
| Ve "el arco azul" donde no hay arco | `pixels_threshold=300` (la mitad del amarillo) + umbral azul que solo pide "oscuro y azulado" | "el threshold está mal calibrado" | §4. Medir el tamaño real del arco en píxeles antes de tocar el número. |
| Ve "la pelota" en algo rojo del público o del robot rival | El umbral naranja está definido casi solo por A; su B acepta casi todo | "hay ruido" | Cerrar el rango de B (§3). Verificar que la pelota real siga entrando. |
| Andaba y de golpe dejó de andar, sin que nadie tocara nada | Balance de blancos / ganancia automáticos que se reajustaron | "se descalibró el sensor" | §9 paso 5 |
| Recalibraron los colores y ahora las distancias están mal | No puede ser la luz: la geometría no depende de la luz | "la calibración rompió la homografía" | Buscá si alguien movió la cámara o cambió el `framesize`. Los colores y la geometría son independientes. |
| Cambiaron QVGA por otra resolución y todo se fue al diablo | La matriz H mapea **píxeles**; en otra resolución los píxeles son otros | "el código nuevo rompió algo" | Volver a QVGA (`.py:28`) o recalibrar la homografía entera |
| El monitor serie muestra `Yp` mayor a 100 | Perdiste el sincronismo: estás leyendo un byte que no era la Y | "la pelota se fue de la cancha" | §6, última nota |
| La cámara no arranca sola cuando la desenchufás de la notebook | El script no está guardado **en la cámara** | "la cámara se rompió al desconectarla" | Tools → Save open script to OpenMV Cam |

**Anti-racionalizaciones**, para cortar las discusiones circulares:

- *"La cámara está rota"* → si el Frame Buffer se ve a color y el rectángulo sigue la pelota, la
  óptica está **sana**. El problema es calibración o transporte.
- *"Hay que retocar la homografía porque cambió la luz"* → **no.** La geometría no depende de la
  luz. Si nadie desarmó el robot, esos nueve números no se tocan.
- *"Copiemos los umbrales de la cámara del otro robot"* → no sirven. Otro sensor y otra luz.
- *"El código compila, entonces el enlace anda"* → compilar no prueba absolutamente nada sobre
  el enlace. Prueba 3 del §10, o no se afirma.
- *"El IDE dibuja el rectángulo, entonces el Teensy tiene la coordenada"* → tampoco. Son dos
  computadoras distintas con un cable en el medio.
- *"Perdimos el sincronismo porque un dato valió 201"* → **no puede pasar** mientras el recorte
  del `.py` esté en su lugar (`.py:100-105`): los datos nunca pasan de 200. Si de verdad se
  desincroniza, el byte se perdió en el cable o la cámara se reinició — no es el protocolo.
  (`bugs-conocidos.md` dice lo contrario; está equivocado, ver §7.)
- *"Le pusimos el `else` del parche y ahora parpadea, entonces el parche está mal"* → puede ser
  al revés: el parpadeo es el dato malo **haciéndose visible**. Antes estaba igual de malo, pero
  callado. Decidan con la Prueba 6, no con la sensación.

---

## 12. Fuentes (todo lo de esta skill sale de acá)

- `futbol-roboliga2026/robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py` — el
  programa de la cámara. **Fuente de verdad de los umbrales, la homografía y el formato que sale
  al cable.**
- `futbol-roboliga2026/robots-2025/arquero/arquero.ino:263-329` — el lector del lado Teensy.
  Idéntico en `delantero/delantero.ino:287-353`.
- `futbol-roboliga2026/robots-2025/mapa-pines-teensy.md:26-27` — pines 0/1, Serial1, 19200.
- `futbol-roboliga2026/robots-2025/libreria-zircon/zirconLib.cpp:13-19, 52-60, 234-322` —
  autodetección Mark1/Naveen1 y el `pinMode(0, OUTPUT)` que podría pisar el RX (§2).
- `futbol-roboliga2026/bugs-conocidos.md` — A4 (sin watchdog de cámara) y DEL-06 (las dos
  fragilidades del enlace). **Ojo: su afirmación de que "201 puede aparecer como dato" está
  desmentida por el `.py` — ver el recuadro del §7.** El propio documento avisa que el `.py` no
  fue auditado.
- `futbol-roboliga2026/correcciones-propuestas.md:139-153, 203-235, 577-623` — los parches
  propuestos y sus pruebas de banco.

**Lo que sale de otras skills, no de esta:**
- Qué hace el robot con `Xp`, `Yp`, `haypelota`, `ARCO_CONTRINCANTE`: la máquina de estados.
- El giroscopio BNO055, los sensores de línea, los 8 IR de pelota, los motores omni: cada uno
  con su skill.
