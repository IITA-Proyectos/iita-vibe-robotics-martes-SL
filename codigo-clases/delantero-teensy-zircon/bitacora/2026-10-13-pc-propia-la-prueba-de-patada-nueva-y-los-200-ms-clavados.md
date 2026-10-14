# 2026-10-13 — PC propia, la prueba de patada nueva, y 200 ms con el programa clavado

**Quiénes:** Máximo y Juanse (mesa del delantero) + Claude (código, lectura del serie y análisis)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**PC:** `AGUSTINAAG`, usuario `agust` — **computadora móvil del equipo**. De ahora en más se
puede probar **en la cancha con el robot enchufado**.
**Programa cargado al cerrar:** 🔴 **la PRUEBA `pruebas/patada-derecha`, no el de partido.** Lo
último que contestó el robot por serie fue la ayuda de la prueba; después lo desenchufaron y no
vi que se volviera a cargar nada. **Antes de jugar hay que cargar con `CARGAR-ROBOT.bat`.**

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 💻 **PC nueva lista** | PlatformIO con todo lo de Teensy instalado. **20 de 20 programas compilan** |
| 📷 **OpenMV IDE no mostraba la imagen** | el Viewer sí. Causa probable leída de la configuración; **sin confirmar** |
| 📷 **Umbral del arco amarillo cambiado** | por la luz. El azul quedó igual. **No sé si quedó grabado en la cámara** |
| 🎯 **La patada vieja, medida hoy** | sin corregir **−35,1°**; con el corrector viejo +2,7° pero **pico −17,6°** |
| 🔴 **El programa se clava 200 ms** | la librería del giroscopio espera 200 ms cada vez que se le pregunta si está vivo. **Leído en el código y visto una vez en el robot** |
| 🆕 **`pruebas/patada-derecha` reescrita** | golpe **+ retroceso**, con gráfico, y los números se cambian con teclas |
| 🆕 **`PROBAR-PATADA.bat`** | carga la prueba y abre la consola |
| ✅ **El principal NO se tocó** | `funciona/delantero/` está igual que ayer. Decisión del equipo |
| ⏳ **La patada NO se llegó a medir** | se acabó el tiempo. Es lo primero de la próxima |

---

## 0. Cómo arrancó la clase

Lo que contó el equipo del 12/10, y que da el tema del día:

> *"Ayer vimos que el robot anduvo bastante bien. Lo que nos perjudicó más fue la patada, que
> empieza, se tuerce, intenta acomodarse pero la pelota ya salió desviada."*

Y el pedido:

> *"Queremos que en el programa de la patada que mide cuántos grados se desvía, que tenga la
> lógica de la patada completa, es decir PATEA Y EL RETROCESO, para ajustar que vaya recto."*

---

## 1. 💻 La PC nueva

El repo se clonó en `C:\soccer-robotito`, **sólo con la carpeta del delantero** (lo demás no se
bajó a disco, a pedido del equipo).

Tenía PlatformIO 6.2.0 pero **vacío**: sin la plataforma Teensy, sin compilador y sin Teensy
Loader. `CARGAR-ROBOT.bat` hubiera cortado con *"No encuentro el Teensy Loader"*. Se instaló
compilando el firmware una vez:

| | versión |
|---|---|
| plataforma Teensy | 6.0.0 |
| compilador (`toolchain-gccarmnoneeabi-teensy`) | 1.150201.0 (GCC 15.2.1) |
| `framework-arduinoteensy` / `tool-teensy` (el Loader) | 1.162.0 |
| Adafruit BNO055 / Unified Sensor / BusIO | 1.6.4 / 1.1.15 / 1.17.4 |

- **Bajó todo sin errores de red** (acá no apareció el problema del antivirus con el SSL).
- **Los 20 programas compilan:** el firmware en sus tres variantes, el respaldo del partido 1 y
  las 16 pruebas. El firmware, sin ninguna advertencia. Cada carpeta ya bajó sus librerías, así
  que deberían compilar sin internet (no lo probé desconectando).
- **PowerShell anda en esta PC.** `CARGAR-ROBOT.bat` se ensayó sin robot: encuentra las
  herramientas y frena donde debe, en *"No hay ningún robot enchufado"*. El lanzador de repuesto
  (`...-SIN-POWERSHELL`) también corre.
- El Arduino IDE instalado (1.8.57, de la Tienda) **no tiene soporte para Teensy**. No hace
  falta: todo va por PlatformIO.

⚠️ **Es otro compilador que el de las PCs anteriores.** Compila limpio, pero el firmware de
partido compilado en esta PC **todavía no jugó**.

---

## 2. 📷 OpenMV IDE no mostraba la imagen (el Viewer sí)

Instalados: OpenMV IDE 5.1.0 y OpenMV Viewer 5.0.0.

**Lo que se encontró** leyendo la configuración que cada programa guardó en la PC:

| | ajuste | qué hace al conectarse |
|---|---|---|
| Viewer | `ViewerStopScriptConnectDisconnect=false` | deja corriendo el programa de la cámara y muestra lo que ve |
| IDE | `StopScriptConnectDisconnect=true` | **frena** el programa de la cámara. Sin nada corriendo no hay imagen hasta darle Play |

Además el editor del IDE había quedado sin ningún archivo abierto, o sea sin nada para correr.
Segunda causa posible, sin descartar: el IDE 5.1 tiene arriba a la derecha del panel de imagen
un selector de "fuente" que puede estar apagado.

> **Es una explicación leída de la configuración. El equipo no llegó a confirmar si con Play
> aparece la imagen.** Después siguieron trabajando con el script abierto en el IDE.

Dos cosas que conviene no olvidar:

- 🚨 **No actualizar el firmware de la cámara** si el IDE lo ofrece. Quedó tildado "no volver a
  preguntar", pero no sé si esa vez se aceptó. Antes de cualquier actualización, copiar el
  `main.py` de la cámara a la PC.
- **Después de usar el IDE, apagar y prender la cámara.** El IDE también frena el programa al
  desconectarse.

---

## 3. 📷 El umbral del arco amarillo

El equipo ajustó valores en el script de la cámara porque con esta luz no veía los arcos. Lo que
quedó guardado en la PC, comparado línea por línea con el original del repo:

| | original | hoy |
|---|---|---|
| arco amarillo | `(17, 70, -27, 14, 38, 111)` | **`(17, 93, -27, 14, 35, 111)`** |
| arco azul | `(4, 36, -13, 57, -64, -4)` | igual, sin cambios |
| pelota | `(21, 67, 18, 79, -32, 127)` | igual (se coló un `+` en el comentario, no afecta) |

En el amarillo: brillo máximo 70 → 93 y piso del canal amarillo 38 → 35. Acepta amarillos más
claros y un poco más lavados.

Copia guardada en [`../vision/camara-delantero-2026-10-13.py`](../vision/camara-delantero-2026-10-13.py).
El original compartido con el arquero (`futbol-roboliga2026/robots-2025/vision-openmv/`) **no se
tocó**.

### ❓ Lo que NO sabemos

- **Si el cambio quedó grabado en la cámara.** Darle Play en el IDE lo corre sólo mientras está
  conectada; para que quede hay que guardarlo en la cámara como `main.py`. Se preguntó y no quedó
  contestado. La cámara ya estaba desenchufada cuando miré.
- **Si el azul también se tocó.** El equipo dijo que ajustó los dos arcos; en el archivo guardado
  el azul es idéntico al original.
- **Si ahora ve los arcos de lejos**, y si el amarillo más abierto se prende con cosas que no son
  el arco. Mirar el LED verde con el arco lejos, cerca y sin arco a la vista.

⚠️ El ajuste se hizo con **ganancia y balance de blancos en automático** (pendiente desde el
04/08): el umbral puede correrse cuando cambie lo que la cámara tiene enfrente.

---

## 4. 🎯 La patada vieja, medida hoy

Antes de tocar nada el equipo corrió la `patada-derecha` **vieja** (215 × 420 ms, trasera
suelta, corrector viejo que sólo frena). Guardada en
`mediciones/medicion-2026-10-13_20-45-09.txt`:

```
A) SIN correccion:              -35.1 grados   (pico -40.0)
B) CON heading-hold, KP = 4.0:    2.7 grados   (pico -17.6)
```

La misma prueba, a lo largo del tiempo:

| | sin corregir | con corrector | pico con corrector |
|---|---|---|---|
| 22/09 | −28,4° | −4,4° | −12,9° |
| 06/10 | −11,9° | +1,9° | +3,7° |
| **13/10** | **−35,1°** | **+2,7°** | **−17,6°** |

- **Sigue yéndose a la DERECHA**, y más que nunca. Entre el 06/10 y hoy el robot se achicó
  (12/10): es el cambio más grande que hubo en el medio. *Que sea por eso es una suposición.*
- El trim del principal se bajó de 15 a **5** el 06/10, cuando el crudo era −11,9°. Con −35,1°
  ese 5 **probablemente quedó corto**. Falta medirlo con la prueba nueva.
- ⚠️ Estos números son de una patada que **el robot ya no pega** (215 × 420, sin trim, sin
  derivativo, trasera suelta). Sirven para ver la tendencia, no para ajustar.

---

## 5. 🔴 El programa se clava 200 ms cada vez que le pregunta al giroscopio si está vivo

Es el hallazgo del día, y salió de leer la librería, no el programa nuestro.

**El mecanismo.** Desde el 01/09, `rumboActual()` le pregunta al chip cada 500 ms si su fusión
sigue corriendo, con `bno.getSystemStatus()`. Esa función de la librería (Adafruit BNO055 1.6.4,
`Adafruit_BNO055.cpp`, última línea de la función, línea 318) **termina con `delay(200)`**.
Son 200 ms con el programa entero parado: no corrige el rumbo, no mira la línea blanca, no lee
la cámara. Los motores siguen con lo último que se les mandó.

**Dónde pega**, según quién llama a `rumboActual()` en el principal:

| dónde | cada cuánto llama | qué pasa |
|---|---|---|
| **el golpe** (`PATEA_ADEL`) | en cada vuelta | el golpe dura 500 ms y el chequeo es cada 500: **cae adentro de casi todas las patadas** |
| la órbita (desde el 12/10) | cada 20 ms | 200 ms clavado cada ~700 ms |
| la telemetría (desde el 15/09) | cada 2 s | 200 ms clavado cada 2 s **en cualquier estado** |

**Por qué importa para la patada.** Si el programa se clava en medio del golpe, el corrector no
corrige durante 200 de los 500 ms. Encaja con lo que ve el equipo (*"se tuerce, intenta
acomodarse pero la pelota ya salió"*) y con lo que midió el robot el 06/10: picos de −12 a −47°
con los **signos mezclados**, mientras la prueba vieja (que **no** hace ese chequeo) daba 1,9°.

### Qué está medido y qué no

| | |
|---|---|
| ✅ **Leído en el código** | el `delay(200)` está en la librería que se compila, y `rumboActual()` la llama |
| ✅ **Visto UNA vez en el robot** | en la única patada de hoy con la prueba nueva, la última muestra del golpe es a los **475 ms** y la siguiente recién a los **699 ms**: ~200 ms sin una sola vuelta de programa. El golpe duró ~699 ms en vez de 500 |
| 🟡 **Indicios en mediciones viejas** | `loop` 365.152/s el 12/10 contra 415.000–492.000 el 11/08, cuando no había chequeos; cámara 42–46 paq/s desde el 21/09 contra 46 fijo antes. Son estados distintos: **indicio, no prueba** |
| ❌ **NO medido** | que eso sea lo que tuerce la patada. En esa única patada **el robot no avanzó** (punto 7), así que se vio el programa clavado pero no su efecto |

**Cómo se mide la próxima:** la prueba nueva tiene la tecla `c`, que hace el mismo chequeo
leyendo el registro del chip (`0x39`) directo, sin la espera. Tres o cuatro patadas sin `c`, tres
o cuatro con `c`, y comparar los picos.

> Si se confirma, el arreglo en el principal es uno solo (dentro de `rumboActual()`) y de paso
> saca los 200 ms clavados de la órbita y de la telemetría, o sea también **ratos sin mirar la
> línea blanca**.

---

## 6. 🆕 `pruebas/patada-derecha` reescrita, y `PROBAR-PATADA.bat`

### La decisión del equipo sobre dónde se hace el cambio

Primero lo armé como un modo de prueba **dentro del programa principal** (así usaba las mismas
líneas que juegan), y comprobé que los tres programas de partido quedaban idénticos byte por
byte. El equipo dijo que no:

> *"Cambialo en el programa aparte que hay de la patada, no en el programa principal. Después de
> que lo probemos y funcione ahí, recién lo cambiamos en el principal."*

Se deshizo entero. **`funciona/delantero/` está igual que en GitHub.**

### Qué hace la prueba nueva

No busca la pelota: espera teclas por la consola. Con `p`, a los 1,5 s hace:

```
GOLPE       150 x 500 ms   rampa +15 cada 5 ms · trim 5 · KP 4.0 · KD 0.6 · trasera trabada
RETROCESO   110 x 700 ms   sin freno en el medio
```

Las funciones de la patada son **copia textual del principal de hoy**, marcadas en el archivo con
`COPIA DEL PRINCIPAL`. Incluye el chequeo de 200 ms, para medir lo mismo que pasa en el partido.

Después de cada patada informa: desvío al terminar el golpe, **el pico y en qué milisegundo**,
el pico de los primeros 200 ms (lo que ve la pelota), cómo quedó tras el retroceso, cuánto duró
el golpe de verdad, **si el programa se clavó y desde cuándo**, y un gráfico cada 20 ms.
Negativo = se fue a la derecha.

| tecla | qué hace |
|---|---|
| `p` | patear. Cualquier tecla mientras se mueve lo **frena** |
| `t`/`T` · `k`/`K` · `d`/`D` | trim ∓1 · KP ∓0,5 · KD ∓0,1 |
| `v`/`V` · `m`/`M` | potencia ∓5 · duración ∓20 ms |
| `l` | corrector sí / no (en "no" patea con el trim solo: es para ajustar el trim) |
| `c` | chequeo del giroscopio con / sin la espera de 200 ms |
| `b` | trasera trabada / suelta durante el golpe |
| `g` · `?` | gráfico sí/no · ayuda y valores |

**`PROBAR-PATADA.bat`** muestra qué robot hay enchufado, abre el Teensy Loader, carga la prueba
y abre la consola. ⚠️ A diferencia de `CARGAR-ROBOT.bat`, **no frena solo si el enchufado es el
arquero**: muestra cuál es y confía en el que lo corre.

### Lo que la prueba NO hace, a propósito

No mira la línea blanca ni la cámara. Hay que ponerla con lugar adelante.

### ⚠️ La trampa de siempre, ahora con nombre y apellido

Es una **copia** de la patada. Si el principal cambia y la prueba no, vuelve a medir una patada
que el robot no pega (ya pasó el 22/09 y el 06/10). Y al revés: **lo que se ajuste acá no existe
en el partido hasta que se lleve al principal.**

---

## 7. La primera corrida: el robot casi no se movió

Lo que reportó el equipo: *"cuando aprieto P casi no se mueve el robot"*, y después *"se movió
como 1 cm nomás"*.

Lo que se verificó:

- **Los números eran los del principal.** Se le preguntó al robot por serie y contestó
  `150 x 500 ms  trim 5  KP 4.0  KD 0.6  corrector SI`.
- **La orden a los motores es idéntica** a la del principal, comparada lado a lado.
- **El gráfico:** en todo el golpe y el retroceso el giroscopio no vio más de **0,2°** de giro.

**La causa la encontró el equipo:** *"ya sé por qué no avanzó, fue mi culpa"*. **No quedó dicho
qué fue.** Conviene anotarlo acá la próxima, para que no vuelva a pasar con un partido encima.

De rebote, esa corrida dejó un dato: el giroscopio en **modo sin brújula** (el del 12/10, que
nunca se había probado en banco) **arrancó y midió** — la prueba lo usa, y con el robot quieto
dio ruido de ±0,2°. Falta lo que importa: girar el robot a mano y ver que acompañe.

---

## 8. Números de hoy, todos juntos

```
PC                   AGUSTINAAG · PlatformIO 6.2.0 · Teensy 6.0.0 · GCC 15.2.1
                     20 de 20 programas compilan · PowerShell OK

PATADA VIEJA         215 x 420 ms, trasera suelta (prueba vieja, 20:45)
                     sin corregir -35.1 (pico -40.0) · con corrector viejo +2.7 (pico -17.6)

PATADA DEL PRINCIPAL 150 x 500 ms · rampa +15/5 ms · trim 5 · KP 4.0 · KD 0.6 · trasera trabada
(sin cambios)        despues: marcha atras 110 x 700 ms

ESPERA DEL GIROSCOPIO  delay(200) en getSystemStatus (Adafruit BNO055 1.6.4, linea 318)
                       chequeo cada 500 ms · visto en el robot: sin muestras de 475 a 699 ms

CAMARA               amarillo (17,70,-27,14,38,111) -> (17,93,-27,14,35,111)
                     azul y pelota sin cambios · OpenMV IDE 5.1.0 · Viewer 5.0.0
```

---

## ⚠️ Qué queda por VER

1. 🔴 **La patada con la prueba nueva.** No se midió ninguna de verdad.
2. 🔴 **Si la espera de 200 ms es lo que la tuerce** (teclas sin `c` contra con `c`).
3. **Qué fue lo que hizo que el robot no avanzara** en la primera corrida.
4. **Si el cambio de la cámara quedó grabado**, si el azul se tocó, y si ahora ve los arcos.
5. **Si con Play el OpenMV IDE muestra la imagen.**
6. **El giroscopio sin brújula, girando el robot a mano.**

## 🔧 Qué queda por HACER — en orden

1. 🔴 **Antes de jugar: cargar el de partido** con `CARGAR-ROBOT.bat`. El robot quedó con la
   prueba adentro.
2. 🔴 **Medir la patada**, así:
   - `PROBAR-PATADA.bat`, robot en el piso con lugar adelante y el cable flojo;
   - 3 o 4 patadas sin tocar nada, mirando si aparece `EL PROGRAMA ESTUVO CLAVADO`;
   - tecla `c` y otras 3 o 4; comparar los picos;
   - recién después el trim: `l` (corrector no) y mover `t`/`T` hasta que el golpe crudo salga
     derecho;
   - anotar la parte de arriba del informe de cada patada.
3. **Llevar al principal lo que haya andado, de a un cambio**, y cargar y mirar entre uno y otro.
4. **La cámara:** confirmar el punto 4 de arriba, congelar ganancia y balance de blancos, y
   apagar el LED rojo (decidido el 12/10, sin hacer).
5. Siguen de antes: medir `L` nuevo, `tabla-camara`, el piso de PWM (`piso-de-pwm` tiene los
   pines del arquero), `calibracion-quieto` y `calibracion-moviendo` nunca corridos,
   `MEJORAS-PENDIENTES.md` vencido.

---

## Errores míos de hoy, anotados

1. **Metí el modo de prueba en el programa principal sin que lo pidieran.** El equipo quería el
   cambio en la prueba aparte y el principal intacto hasta verlo andar. Hubo que deshacerlo y
   rehacerlo: tiempo de clase perdido. **Regla que queda: lo nuevo va primero a `pruebas/`.**
2. **Dije "la batería está apagada"** antes de tener el dato. Se había movido 1 cm: estaba
   prendida.
3. **Después sospeché de la rueda trasera trabada**, lo dejé escrito en el código como hipótesis
   y le agregué una tecla. No era eso: fue un descuido del equipo. El comentario ya está
   corregido. Dos explicaciones mías seguidas sin una medición atrás.
4. **Tardé demasiado.** El equipo tuvo que pedir *"apurate por favor"*, y la patada, que era el
   tema del día, no se llegó a medir.

---

## Nota de método

**Lo que salió bien fue hacer que el robot conteste en vez de suponer.** *"¿Tiene los mismos
valores que el principal?"* se cerró en segundos preguntándole al robot por serie. Y el gráfico
de una patada que "no anduvo" igual mostró el hueco de 200 ms.

**Lo que salió mal es conocido:** se explicó antes de medir. El 06/10 quedó escrito *"cuando algo
'no lo hace', no dar otra explicación: hacer que el robot imprima lo que decide"*. Hoy el robot ya
lo estaba imprimiendo, y aun así di dos explicaciones antes de preguntarle al equipo qué había
pasado.

**Y la decisión del equipo de no tocar el principal fue la correcta.** El robot jugó ayer con ese
programa; lo que hoy se probó a las apuradas no llegó a validarse. Si hubiera quedado adentro del
principal, hoy habría un programa de partido con cosas nuevas sin ver andar.
