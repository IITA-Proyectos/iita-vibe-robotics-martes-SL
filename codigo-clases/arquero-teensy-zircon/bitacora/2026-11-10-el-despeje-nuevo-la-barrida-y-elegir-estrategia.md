# 2026-11-10 al 13/11: el despeje nuevo, la barrida, el tablero de la 0921k y ELEGIR-ESTRATEGIA

**Quiénes:** Diego y Laureano (cancha, pruebas y pedidos) + Claude (código, revisión y carga)
**Robot:** el **ARQUERO**, Teensy con número de serie `13348550`
**Programas nuevos:** [`ELEGIR-ESTRATEGIA.bat`](../ELEGIR-ESTRATEGIA.bat) +
`pruebas/herramientas/elegir_estrategia.py` (13/11), `respaldos/partido-0921i/` y
`respaldos/partido-0921j/`
**Modificados:**
- `funciona/estrategia-2`: de la B del 09/11 a la **2026-11-10d**.
- `funciona/partido-0921`: de la 0921i a la 0921j y a la **0921k**.
- El tablero (`pruebas/tablero/`), adaptado a la 0921k el 12-13/11.
- El `README.md`.

**No se subió:**
- `pruebas/tablero/copias-partido/` (copias locales del tablero);
- las carpetas `__pycache__`;
- el script de la cámara (umbral naranja 79 → 78, cambiado solo en esta PC).

**PC:** la notebook que viaja. Las horas son las de la compu.

---

## 🎯 EL RESUMEN

**1. La estrategia 2 se ajustó en la cancha (10/11)** (§2).
- Despeje más lento, con **rampa en cada motor**.
- Espera a quedar plano antes de volver, y se endereza y vuelve más despacio.
- Costado a 130.
- El equipo: *"estaba mejor que antes"*.

**2. El programa de juego tiene el despeje de la estrategia 2 (0921j, §3)** y, en vez de la
patrulla, **la barrida (0921k, §4)**: a los 4 s sin ver la pelota, gira en el lugar buscándola.
**La 0921k es la que está en el robot.** Las tres ruedas y el saltito de 3 cm más **no se
probaron todavía en la cancha.**

**3. El tablero maneja la 0921k (12-13/11)** (§5). **Todavía no se usó con el robot.**

**4. Hay `ELEGIR-ESTRATEGIA.bat` (13/11)** (§6): escribís 1 o 2 y carga esa estrategia. Antes de
cargar mira que el robot enchufado sea el arquero. **Todavía no se hizo una carga de verdad con
él.**

---

## 1. El 10/11 a la tarde: "solo prende y no hace nada"

- El robot tenía **el programa del tablero**, que no juega: espera órdenes por el cable.
  `TABLERO.bat` lo carga si apretás **Enter sin escribir nada**. Se volvió a cargar la 0921i.
- Una carga se cortó a la mitad y el Teensy quedó en **MODO CARGA** (sin programa y sin puerto COM).
  `cargar.py` no lo veía, así que se cargó directo con `python -m platformio`.
- `pio.exe` volvía a andar ese día, y **la carga con `python -m platformio` se probó con el robot:
  anduvo** (era un pendiente del 09/11).
- *"Ya lo probamos y funciona, cargá la estrategia 2"*.

---

## 2. La estrategia 2, en el orden en que se pidió (versión 2026-11-10d)

| Pedido | Qué cambió |
|---|---|
| *"bajale la velocidad general y también se vio que el robot a la hora de despejar se va para la izquierda"* (aclararon: solo el despeje, y es un saltito al arrancar) | La ida 200 → **160**, y 533 → **666 ms** para llegar igual de lejos (~50 cm). Primero se probó que la ida arranque en 70: no lo sacó |
| *"sigue el mismo problema, ¿se puede hacer una rampa de velocidad para cada motor?"* (solo en el despeje; el robot estaba quieto antes de salir) | **Rampa en cada motor** (`aplicarSuave`): cada rueda sube **10 cada 10 ms** con todo lo que le llega (avance, correrse hacia la pelota y el giroscopio). Antes, lo de costado y el giroscopio llegaban de golpe. Bajar es en el acto. Solo en el despeje |
| *"en el despeje lo hace bien pero en el retroceso lo hace chueco"* (zigzag) | **Espera a quedar plano** antes de enderezarse y volver, como la 0921: hasta 1,5° de la inclinación de siempre durante 100 ms, nunca más de 0,8 s. Correrse hacia la pelota en la ida: 80 → **60** |
| *"bajá la velocidad de los laterales porque [...] se desvía al golpear la pelota, además en el despeje cuando se quiere enderezar se pasa y vuelve chueco también porque vuelve muy rápido"* | Costado 160 → **130**. Enderezarse: máximo 120 → **85** (menos de 70 no gira). Vuelta 110 → 90 → **80**, con tope 1500 → **2500 ms** (a 80 tarda más en llegar) |

**Lo que dijo el equipo:** *"estaba mejor que antes"*. Siguen sin arreglar los 3 problemas de la
revisión del 09/11 (bitácora del 09/11, §4).

---

## 3. La 0921j: el despeje de la estrategia 2 en el programa de juego

**El pedido:** *"poné el despeje del último programa de la estrategia dos a este"*. Eligieron
**todo el despeje** y **reemplazar la 0921i**. Después: *"solo el programa, después el tablero"*.

Se copió el despeje de la 10d, con sus números:
1. **La ida:** 160 durante 666 ms, con rampa en cada motor.
2. **La pausa:** espera a quedar plano, igual que la 0921i.
3. **Se endereza antes de volver** (`enderezarAntesDeVolver`): gira en el lugar hasta quedar a 3°
   del rumbo del arranque, con fuerza de 70 a 85, frena 150 ms y recién ahí vuelve.
4. **La vuelta:** para atrás a **80** (`potenciaVueltaDespeje`), tope **2500 ms**.
   `potenciaRetroceso` (110) y `MS_MAX_RETROCESO` (1500) **no se tocaron**: los usan el arranque,
   la patrulla y la rutina de ir al medio.
5. **Al llegar a la línea:** igual que la 0921i.

La 0921i quedó copiada en `respaldos/partido-0921i/`, y la 0921j en `respaldos/partido-0921j/`.

---

## 4. La 0921k: LA BARRIDA en vez de la patrulla

**La idea del equipo** (en un entretiempo): *"cuando pasen 7 segundos el robot va a mover en su eje
el motor de atrás para su lado derecho y izquierdo constantemente hasta que encuentre la pelota [...]
que se posicione en el ángulo 0 grados y que se vaya hacia el lado que detectó la pelota"*.
Eligieron:
- hacerla sobre la 0921j;
- que **reemplace a la patrulla** de ahora;
- girar solo con la rueda de atrás;
- **±45° de a 15°**.

**Cómo quedó:**
1. **Cuándo empieza:** sin ver la pelota durante **4 s**, empieza a barrer.
2. **El giro:** de a 15°, en este orden: +15, +30, +45, −15, −30, −45 y 0. En cada paso frena y mira
   200 ms (en el 0, medio segundo).
3. **Si la ve** (3 cuadros seguidos):
   - anota de qué lado está **de verdad** (para dónde miraba el robot + dónde estaba en la imagen);
   - vuelve a 0°;
   - va **de costado** a 100 para ese lado hasta verla, y ahí la sigue y la despeja como siempre.
4. **Si en 1,5 s de costado no la ve,** vuelve el mismo tiempo para el otro lado.
5. **Si gira al revés,** se da cuenta y da vuelta el signo solo.
6. **Si lo levantan o lo empuja un rival** mientras barre, corta esa barrida. **Nunca queda apagada
   para todo el partido.**
7. **Mientras barre,** el LED hace dos destellos cortos por segundo. **Sin giroscopio, no barre.**

**Los pedidos de después** (la primera se cargó *"así como está"*, antes de terminar la
revisión, y después revisada):

| Pedido | Qué cambió |
|---|---|
| *"cambiale el tiempo de la patrulla que sea 4 seg"* | `MS_SIN_PELOTA_PARA_BARRER` 7000 → **4000** |
| *"en vez de que gire con la rueda de atrás que gire con las tres en su eje para que el giro no ocupe tanto lugar"* | `barridaConTresRuedas = true`: gira **en el lugar** con las tres ruedas a **80** (`PWM_BARRIDA_TRES`) |
| *"después de despejar retrocede, ve la línea y en ese saltito que hace aumentale 3 cm más"* (solo después del despeje) | `MS_EXTRA_EMPUJON_DESPEJE = 70`: el empujoncito dura 450 + 70 ms. El del arranque no cambió |

La patrulla vieja quedó con `patrullarSinPelota = false`, con su código entero. La línea `#PARTIDO`
dice `version=0921k`.

---

## 5. El tablero, adaptado a la 0921k (12-13/11)

Pedido: *"hacelo"* (adaptar el tablero) *"y explicá la diferencia entre versiones"*.
- **Cómo se hizo:** un agente lo armó, tres lo revisaron (valores, que se entienda, los movimientos),
  otros intentaron refutar cada hallazgo, y uno arregló lo confirmado. Fueron **11 agentes** y
  **5 problemas confirmados, arreglados**.
- **`partido-0921.ino` no se tocó:** su huella es la misma antes y después. El 13/11 solo se
  corrigió un comentario que decía que el tablero no la maneja; compila igual, del mismo tamaño.

**Qué tiene nuevo:**
- **Reconoce la 0921k.** La 0921j, la 0921i y la 0921g salen como "VERSIÓN ANTERIOR".
- **PARTIDO: grupo 3 · EL DESPEJE**, en el orden en que despeja: la ida con la rampa en cada motor,
  la pausa, enderezarse, la vuelta y el empujoncito con los 3 cm de más.
- **PARTIDO: grupo 5 · LA BARRIDA** (nuevo). Lo que casi nunca se toca está en AVANZADO.
- **Las filas grises** son las que el robot no usa:
  - la patrulla vieja (pasó al grupo 6), porque está la barrida;
  - `PWM_BARRIDA`, porque gira con las tres ruedas.
- **Reglas nuevas:**
  - **E9:** el mínimo de giro más grande que el máximo.
  - **O23:** los pasos de la barrida no llegan al máximo.
  - **O24:** barrida sin límite lateral.
  - **O26:** la rampa tan suave que la ida pasa más de la mitad subiendo.
  - **O27:** lo mismo para enderezarse y la barrida.
- **La rampa de cada motor** ya no deja valores con los que la ida no sale: `PASO_RAMPA_RUEDA` de 5
  a 30, `MS_RAMPA_RUEDA` de 5 a 20.
- **Para apurar la barrida,** bajar `MS_MIRAR_BARRIDA` o subir `PWM_BARRIDA_TRES`.
  `GRADOS_PASO_BARRIDA` no la apura: son siempre 7 paradas.
- **MOVIMIENTOS 6 y 9 siguen con el despeje VIEJO,** y ahora el botón lo dice. Copiar el de la
  0921k a `tablero.ino` era código nuevo que mueve el robot sin poder probarlo. **El despeje de
  verdad se prueba con el partido cargado y con pelota.**
- **CANCHA 5:** botón **"IDA 666 ms"**, que manda la fuerza del partido (160). Las idas medidas a
  otra fuerza no cuentan para PASAR A PARTIDO.
- **DISENO.md:** secciones 16.12 y 16.12.1. **El contrato del cable no cambió.**

**Autoprueba: OK, 5261 verificaciones.** Compilan el tablero, la 0921k y la estrategia 2.

### Las versiones del programa de juego, para no perderse

| Versión | Qué tiene |
|---|---|
| 0921d (03/11) | La del 21/09 + costado 160/70, sin predicción, margen 20 cm, persigue en la ida, 300 ms antes de volver |
| 0921f (03/11) | La d + límite lateral, patrulla e ir al medio al arrancar, espera a quedar plano |
| 0921g (08/11) | La f + la línea `#PARTIDO` para el tablero (juega igual) |
| 0921i (09/11) | La g + arranca a los 3 s, empujón de 15 cm, patrulla a los dos laterales a los 20 s, vuelta con la línea del área |
| 0921j (10/11) | La i + el despeje de la estrategia 2 |
| **0921k (10/11)** | La j + **la barrida** en vez de la patrulla + 3 cm más después del despeje. **La del robot** |

---

## 6. ELEGIR-ESTRATEGIA.bat (13/11)

**El pedido:** *"¿podés crear a través de la terminal para que podamos poner cuál estrategia elegir?
ya sea la 1 o la 2"*. Eligieron que **cargue uno u otro programa** (no los dos adentro del robot), y
que la 1 sea **la 0921k**.

**Cómo se usa:**
1. Doble clic en `ELEGIR-ESTRATEGIA.bat` (carpeta `arquero-teensy-zircon`). La ventana muestra qué
   versión tiene cada carpeta y si la 1 tiene los valores de fábrica o cambiados con el tablero.
2. Escribir **1** (`funciona/partido-0921`) o **2** (`funciona/estrategia-2`). **Con Enter solo o
   0 no carga nada.**
3. Mira qué robot hay enchufado con `tablero.py --quien` (no abre ningún puerto). Si no es el
   arquero, pide que dejen enchufado solo el arquero.
4. Vuelve a avisar que el robot arranca solo, y **carga solo si escriben S**.
5. Compila **sin tocar el robot**. Si no compila, el robot sigue como estaba.
6. Mira el robot otra vez y recién ahí carga con `cargar.py`.

`ELEGIR-ESTRATEGIA.bat --sin-cargar` prueba la ventana sin compilar ni cargar.

**La revisión** (4 agentes) confirmó **1 problema serio**, arreglado antes de usarlo:
- **El problema:** si el delantero quedaba en MODO CARGA enchufado a esta PC, `cargar.py` no lo
  veía. El cargador le graba primero a ese, así que la estrategia podía terminar en el delantero y
  la ventana decía "LISTO".
- **El arreglo:** ahora mira con `--quien`, como el tablero.

También se arreglaron cosas menores:
- `cargar.py` corre sin teclado: una S de más ya no contesta nada por ellos.
- La versión se vuelve a leer antes de preguntar.
- Si escriben mal `--sin-cargar` (por ejemplo `--sin-carga`), no hace nada.
- Sin el Python de PlatformIO avisa y no sigue.
- Si se corta a la mitad, dice cómo salir del MODO CARGA (Arduino IDE).
- Avisa que el tablero le carga otro programa al robot.

**Se probó sin robot** con todas las respuestas posibles: hace lo correcto en todas.

---

## 7. Lo que se cargó el 10/11, en orden

1. La 0921i, dos veces: una directo, por el MODO CARGA (§1), y otra *"cargá la 0921i"* a las
   19:51.
2. La estrategia 2, con cada cambio del §2 (la 10d, a las 19:49).
3. La 0921k varias veces: tal como estaba, revisada, con 4 s, y al final con las tres ruedas y el
   saltito (21:28).

| En el robot | Estado |
|---|---|
| **0921k** (tres ruedas + 3 cm) | 🚨 **LA QUE ESTÁ EN EL ROBOT.** La barrida con las tres ruedas y el saltito de 3 cm más no se probaron en la cancha |

Para cambiar de estrategia: `ELEGIR-ESTRATEGIA.bat`.

---

## 8. Errores (míos)

1. **El despeje de la estrategia 2 del 09/11 se había cargado sin probar,** y tenía el tirón a la
   izquierda. Hicieron falta 4 vueltas en la cancha (§2).
2. **Empezar la ida en 70 no sacó el tirón.** Venía de la teoría del 03/11; la rampa en cada motor,
   que pidió el equipo, fue la que sirvió.
3. **La 0921k se cargó antes de terminar la revisión** (a pedido: *"cargalo así como está"*).
   Después se cargó la revisada.
4. **La primera versión de ELEGIR-ESTRATEGIA no miraba si había un Teensy en MODO CARGA.** Lo
   encontró la revisión antes de que se usara.
5. **`TABLERO.bat` carga con Enter solo.** Por eso el robot quedó con el programa del tablero el
   10/11. Sigue así: ELEGIR-ESTRATEGIA hace al revés (Enter = no carga).

---

## 9. Qué queda pendiente

### 🔴 La 0921k (el programa de juego)
- 🔴 **Probar en la cancha la barrida con las tres ruedas:**
  - que gire para el lado correcto (si no, se da vuelta sola);
  - que 80 alcance para girar;
  - que no se mueva de lugar.
- 🔴 **Probar el saltito de 3 cm más** después del despeje.
- ⬜ **Recordar de qué lado se fue la pelota** y buscarla con pulsitos (quedó para después).
- ⬜ La vuelta con la línea del área (15 cm + 1274 ms) sigue sin un resultado de prueba claro.
- ⬜ `MS_MEDIO_AL_MEDIO` (2228) se midió antes de mover los motores.

### 🔴 La estrategia 2
- 🔴 **Arreglar los 3 problemas de la revisión del 09/11** (§4 de esa bitácora), decírselo antes al
  equipo y probarlos.
- ⬜ **Medir los sensores con `pruebas/calibrar-linea`** y ponerle a cada uno su umbral.
- ⬜ No tiene patrulla ni tablero (*"después vemos"*).

### ⬜ Las herramientas
- ⬜ **Usar el tablero con el robot por primera vez**, con la compu al lado: cargar el partido y ver
  "PARTIDO 0921k ... ✔ coincide con el archivo".
- ⬜ **Hacer la primera carga de verdad con ELEGIR-ESTRATEGIA**, con la batería apagada.
- ⬜ La línea `#PARTIDO` no manda `barridaActivada`: eso lo confirma solo el SELLO.
- ⬜ El "≈ cm" de `msAdelante` en el tablero no tiene en cuenta la rampa.
- ⬜ Avisarle al delantero que sus `.bat` usan `pio.exe`, que Windows a veces bloquea en esta compu.

### ⬜ Arrastres
- ⬜ Anotar en el mapa de la EEPROM las zonas `3584..3595` (0921i) y `3600..3611`
  (medir-ancho-area).
- ⬜ Los textos que mandan a la 0.1103 (ver la bitácora del 08/11).
- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
