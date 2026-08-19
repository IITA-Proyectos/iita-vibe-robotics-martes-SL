# 2026-08-18 — Sensores mapeados, giroscopio congelado, y una vuelta atrás

**Quiénes:** Gustavo Viollaz (cancha) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa cargado al cerrar:** el del 11/08 + los umbrales de línea medidos hoy

---

## 📋 RESUMEN DE LA CLASE

**Fue una clase difícil, y buena parte fue por mi culpa.** Metí cuatro cambios seguidos sobre un
robot que no podíamos probar bien, el robot empeoró en el piso, y Gustavo decidió volver atrás.
Fue la decisión correcta.

Lo que quedó al final no es código: son **mediciones**. Y son buenas.

| | |
|---|---|
| ✅ **Los tres sensores de línea, mapeados por medición** | cuál es cuál, en qué pin, en qué lado, hacia dónde escapa |
| ✅ **Umbrales de línea medidos en ESTE robot** | y el escape activado por primera vez |
| ✅ **El sensor 1 no está roto: está saturado** | se arregla con un tornillo, no con código |
| 🔴 **El giroscopio contesta pero NO mide** | rumbo congelado en 360 |
| 🔴 **Desde la órbita el robot no puede patear** | y ahora se sabe por qué |
| ↩️ **Vuelta atrás al firmware del 11/08** | y por qué mis cambios lo empeoraron |

---

## 1. Los sensores de línea: mapeados por medición

Método: dos superficies conocidas y el robot quieto, moviendo un sensor por vez al blanco. La
pregunta es *cuál cambió*, que no se presta a confusión. Es el mismo método con el que se
mapearon las ruedas el 28/07.

```
sensor 1 sobre blanco:   765 /  68 /  89
sensor 2 sobre blanco:    94 / 762 /  93
sensor 3 sobre blanco:   102 /  74 / 764
```

| Sensor | Pin | Está entre | Escapa hacia |
|---|---|---|---|
| **1** | A11 (físico 25) | trasera y **derecha** | rueda **izquierda** |
| **2** | A13 (físico 27) | trasera e **izquierda** | rueda **derecha** |
| **3** | A12 (físico 26) | derecha e izquierda → **ADELANTE** | rueda **trasera** = para atrás |

**El firmware ya hacía exactamente esto.** El mapeo de `escaparDeLinea()`, que salía del dibujo
de Gustavo y de los `retroceder1/2/3` del campeón 2025, resultó correcto en los tres. **Cero
líneas cambiadas.**

### 🔧 El sensor 1 no está roto: está SATURADO

Toda la tarde lo di por muerto porque leía **762 sobre verde y 765 sobre blanco** — tres puntos
de diferencia. **Estaba mirando media escala.** Sobre negro lee **94**, igual que los otros dos.

Mide perfecto, pero **llega al tope antes de llegar al verde**. Está más cerca del piso que los
otros, o tiene más ganancia. **Se arregla subiéndolo unos milímetros, no cambiándolo.**

### Los números de las tres superficies

| | Negro | Verde | Blanco |
|---|---|---|---|
| Sensor 1 | 94-102 | **762** | 765 |
| Sensor 2 | 68-74 | 588 | 762 |
| Sensor 3 | 89-93 | 638 | 764 |

**El verde está mucho más cerca del blanco que del negro.** El sensor 2 recorre de 68 a 762 y el
verde cae en 588 — al 75 % del camino hacia el blanco. Por eso los márgenes son finos.

### Los umbrales, ahora de este robot

```cpp
int UMBRAL_LINEA[3] = { 1024, 675, 700 };
```

Los dos últimos son el punto medio entre verde y blanco de cada sensor. El primero va en **1024**
—que el conversor no puede alcanzar, llega a 1023— así que el sensor 1 nunca dispara, y de paso
la comprobación de arranque deja de contarlo sin tocar una línea más.

**Los 620 anteriores venían de la mesa del arquero** (su verde da 350-468). No servían acá: otro
robot, otra altura de sensores. **Es la segunda vez en el día que un dato de la otra mesa no
transfiere.** Conviene medir siempre en el robot propio.

Con esto, el escape quedó **ACTIVADO por primera vez**:

```
Linea: sensores leen 763 / 402 / 661   umbrales 1024 / 675 / 700
Linea: OK, escape ACTIVADO (anula todo lo demas).
```

⚠️ **Nunca se lo vio ejecutarse.** El mapa está medido; la maniobra no.

---

## 2. El giroscopio: contesta pero no mide

Dos errores míos encadenados, y los dos los destrabó Gustavo insistiendo en que el hardware
estaba bien.

**Error 1 — la verificación.** `arrancarGiroscopo()` tomaba sus 20 lecturas de prueba
**inmediatamente después** de `setExtCrystalUse()`, que reinicia la fusión del BNO055 y hace que
devuelva 0.000 **legítimos** mientras converge. Le tomaba el pulso mientras despertaba.

**El número lo gritaba y no lo leí:** daba **siempre** 9 de 20 útiles. Con lecturas cada 50 ms
son 11 ceros al principio y 9 buenas al final. Lo vi cuatro veces.

**Error 2 — mi conclusión.** Corregí eso, apareció `rumbo=360` en el monitor, y canté victoria.
Pero que entregue un número no prueba que mida. **Gustavo giró el robot a mano 20 segundos:**

```
20:10:05   rumbo=360
20:10:11   rumbo=360
20:10:19   rumbo=360
20:10:25   rumbo=360
```

**Congelado.** El sensor contesta por I2C pero la fusión no corre.

**Sospechoso concreto, sin probar:** `bno.setExtCrystalUse(true)` le dice al BNO que use un
cristal externo de 32 kHz. Si esta placa no lo tiene, el reloj queda mal y la fusión se detiene —
exactamente este síntoma. El arquero usa la misma línea y a él le anda, así que puede ser una
diferencia entre las dos placas. **Queda para probar.**

---

## 3. La vuelta atrás, y por qué

Después de cuatro cambios míos el robot en el piso *funcionaba malísimo, encendía y no buscaba la
pelota, hacía cosas raras*. Gustavo pidió volver al estado del 11/08. Se hizo.

**El sospechoso principal de las cosas raras es mi propio arreglo del giroscopio.** Antes, el
robot lo daba por muerto y **no lo usaba**. Con el arreglo pasa a **creer que funciona** — pero
el rumbo está congelado. Entonces al terminar la órbita entra en `APUNTA_RUMBO0` y trata de
apuntar hacia un rumbo que nunca cambia: **gira en el lugar 6 segundos** hasta rendirse. Eso no
pasaba el 11/08.

**No se reescribió la historia**: el repo es compartido con la mesa del arquero y un rebase les
rompería la copia. Los commits del día quedan en el log y hay un commit que los deshace.

---

## 4. Números medidos hoy

| Medición | Valor |
|---|---|
| Cámara | **46 paquetes/s** |
| Bytes descartados resincronizando | **0/s** — el enlace con la cámara está sano |
| `loop()` | **611.000 vueltas/s** (17.500 cuando lee los sensores de línea) |
| Placa (pin 32) | **Mark1** → sensores en A11/A13/A12 |
| Giroscopio | contesta, **congelado en 360** |

---

## 5. 🛠️ El build se cuelga — el rodeo, por si vuelve a pasar

`scons` (el organizador del build de PlatformIO) **se cuelga de forma reproducible en el paso
`Linking`**. Compila los 200 archivos en 12 segundos y después se queda clavado sin llegar a
invocar el enlazador. Se descartaron por prueba: la caché, los permisos, el Teensy Loader. **El
enlazador está sano** — corriendo su comando a mano termina al instante.

El rodeo que funcionó, cuatro pasos:

1. `pio run -v` → se cuelga, pero **imprime el comando de enlazado** antes.
2. Correr ese comando a mano con el toolchain en el `PATH` → produce `firmware.elf`.
3. `arm-none-eabi-objcopy -O ihex -R .eeprom firmware.elf firmware.hex`.
4. `teensy_post_compile -file=firmware -path=... -board=TEENSY41 -reboot`.

**Y una lección operativa:** `teensy_restart.exe` **no reinicia** la placa. Para releer el banner
hay que **desenchufar y enchufar el USB**. El botón de la placa NO sirve: es el de programación,
y mantenerlo apretado borra el programa.

---

## Qué queda pendiente

1. **Ver el escape de línea andar** — que se aleje, no que se meta más adentro. Encender el robot
   **sobre el verde**.
2. **Medir verde y blanco en varios puntos de la cancha.** Los umbrales de hoy salen de una sola
   lectura en un punto.
3. **Subir el sensor 1** unos milímetros y volver a medir las tres superficies.
4. **Probar el giroscopio sin `setExtCrystalUse(true)`.**
5. **Desde la órbita la pelota queda a 22-34° y la tolerancia es 8: la patada no puede
   dispararse.** Sospecha: `XP_ORBITA = 22` está en unidades que no son centímetros — la otra
   mesa midió que en el arquero la cámara exagera 4 veces. Se cierra con la cinta métrica:
   pelota a 30, 60 y 100 cm y leer `Xp`.
6. **El arco amarillo nunca junta muestras** — la cámara le pide 600 px contra 300 del azul.

## Nota de método

Tres veces hoy culpé al hardware y las tres veces el problema era mío: el techo del arco, la
protección que apagaba el escape, y la verificación del giroscopio. **La próxima conviene
arrancar mirando el código y no el robot** — sobre todo cuando el equipo dice que el hardware
anda, porque hasta ahora tuvieron razón siempre.
