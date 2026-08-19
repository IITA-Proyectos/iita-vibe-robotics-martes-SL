# 2026-08-18 — Los sensores de línea, mapeados y medidos en el robot

**Quiénes:** Gustavo Viollaz (cancha) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`

## El resultado

Los tres sensores de línea quedaron **medidos**, no deducidos, moviendo el robot y leyendo qué
canal cambiaba. Método: dos superficies conocidas y el robot quieto — la pregunta es
*"¿cuál cambió?"*, que no se presta a confusión. Es el mismo método con el que se mapearon las
ruedas el 28/07 y los sensores del arquero el 11/08.

| Sensor | Pin | Lado que vigila | Escapa hacia |
|---|---|---|---|
| **1** | A11 (físico 25) | trasera ↔ **derecha** | rueda **izquierda** |
| **2** | A13 (físico 27) | trasera ↔ **izquierda** | rueda **derecha** |
| **3** | A12 (físico 26) | **adelante** (izq ↔ der) | rueda **trasera** = para atrás |

**El firmware ya hacía exactamente esto.** El mapeo de `escaparDeLinea()`, que venía del dibujo
de Gustavo y de los `retroceder1/2/3` del campeón 2025, resultó correcto en los tres. No hubo
que cambiar nada.

### Las tres lecturas que lo probaron

```
sensor 1 sobre blanco:   765 /  68 /  89
sensor 2 sobre blanco:    94 / 762 /  93
sensor 3 sobre blanco:   102 /  74 / 764
```

## Los números de cada superficie

| | Negro | Verde | Blanco |
|---|---|---|---|
| Sensor 1 | 94-102 | **762** | 765 |
| Sensor 2 | 68-74 | 588 | 762 |
| Sensor 3 | 89-93 | 638 | 764 |

**El verde está mucho más cerca del blanco que del negro.** El sensor 2 recorre de 68 a 762, y
el verde cae en 588 — al 75 % del camino hacia el blanco. Por eso los márgenes son finos y por
eso el umbral es tan sensible.

### 🔧 El sensor 1 no está roto: está SATURADO

Durante toda la tarde di por muerto al sensor 1, porque leía 762 sobre verde y 765 sobre blanco
— tres puntos de diferencia. **Estaba mirando media escala.** Sobre negro lee **94**, igual que
los otros dos.

O sea: el sensor mide perfecto, pero **llega al tope antes de llegar al verde**. Está más cerca
del piso que los otros dos, o tiene más ganancia.

**No se arregla con software: hay que subirlo unos milímetros.** Mientras tanto queda
neutralizado con umbral 1024 (inalcanzable para un conversor que llega a 1023), que es lo
correcto: mejor que no dispare nunca a que dispare siempre.

## Los umbrales, ahora sí de este robot

```cpp
int UMBRAL_LINEA[3] = { 1024, 675, 700 };
```

Los dos últimos son el punto medio entre el verde y el blanco de cada sensor:
`(588+762)/2 = 675` y `(638+764)/2 = 701`.

**Los 620 anteriores venían de la mesa del arquero** (su verde da 350-468). No servían acá: otro
robot, otra altura de sensores. Es la segunda vez en el día que un dato de la otra mesa no
transfiere — **conviene medir siempre en el robot propio.**

Con estos umbrales, el escape de línea quedó **ACTIVADO por primera vez**:

```
Linea: sensores leen 763 / 402 / 661   umbrales 1024 / 675 / 700
Linea: OK, escape ACTIVADO (anula todo lo demas).
```

## Lo que quedó pendiente

- ⬜ **Ver el escape andar.** Que se aleje de la línea y no se meta más adentro. El mapeo está
  medido, pero la maniobra nunca se ejecutó.
- ⬜ **Subir el sensor 1** unos milímetros y volver a medir las tres superficies.
- ⬜ **El giroscopio contesta pero no mide**: rumbo congelado en 360 durante 20 segundos
  girándolo a mano. Medido, sin resolver.
- ⬜ **Durante la órbita la pelota queda a 22-34°** con tolerancia de 8: la patada no puede
  dispararse desde la órbita. Sospecha: `XP_ORBITA = 22` está en unidades que no son cm.
- ⬜ **El arco amarillo nunca junta muestras** — la cámara le pide 600 px contra 300 del azul.

## Nota de método

Hoy se revirtió el firmware al estado del 11/08 después de que cuatro cambios seguidos lo
dejaran peor en el piso. La decisión la tomó Gustavo y fue la correcta. Lo que sobrevivió al
revert no fue código: fueron **las mediciones**. Siguen valiendo todas.
