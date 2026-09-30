# 2026-09-29 — El giro continuo le gana al pulsado, y las X/Y eran centímetros todo este tiempo

> Robot: **delantero**, Teensy 4.1 sobre placa Zircon, serie `15708680`, compilado con
> `#define ROBOT2`. Cancha de tela.

---

## 📋 RESUMEN DE LA CLASE

Dos cosas grandes y una vergonzosa.

1. **La búsqueda de la pelota cambió de forma.** En vez de girar a pulsos (girar / frenar /
   girar), ahora gira **continuo a 50 con una rampa de arranque**. Medido: llega a la pelota
   en **3,5 s** contra **5,7 s** del pulsado. Al equipo y al profe les gustó cómo queda.

2. **Las `X`/`Y` que manda la cámara están en CENTÍMETROS.** El programa tenía un comentario
   que decía que eran "unidades desconocidas", y por creer eso llevamos semanas ajustando
   distancias a tanteo. Eran centímetros desde el principio. Esto destapó un error que
   bloqueaba el centrado de lejos.

3. **Lo vergonzoso:** el destrabe que el equipo pidió tuvo **dos relojes mal puestos por mí**,
   uno detrás del otro. El primero medía el tiempo equivocado y saltaba de casualidad; cuando
   arreglé eso, dejó de saltar nunca. Está contado abajo sin maquillar.

---

## 1. ✅ Aflojar al acercarse: funciona

Confirmado en cancha. `VEL_AVANCE = 95` de lejos, y desde `XP_FRENAR = 70` cm afloja de a poco
hasta `VEL_AVANCE_CERCA = 70`. Ya no desacomoda la pelota al llegar, que era el problema del
22/09. Queda como está.

---

## 2. 🎯 Frenar la pausa es lo que hace que la cámara vea la pelota

Esto salió de una prueba pensada para otra cosa y es el hallazgo más útil del día.

Durante la búsqueda a pulsos, en la pausa entre pulso y pulso se puede hacer dos cosas:

| | qué hacen las patas | qué hace el robot |
|---|---|---|
| `parar()` | las dos direcciones en 0 | **suelta**: sigue de largo por inercia |
| `frenar()` | las dos direcciones en 0 **+ PWM 255** | **frena**: cortocircuita el motor |

*Mismo estado de las patas, efecto opuesto.* Y la diferencia medida es enorme:

| pausa | cuadros en los que vio la pelota |
|---|---|
| soltando (`parar`) | **45 %** |
| frenando (`frenar`) | **98 %** |

O sea: **la cámara no veía mal la pelota, el robot se movía mientras miraba.** Frenando la
pausa, el robot está quieto durante los 200 ms de pausa y la cámara tiene tiempo de verla.

### Y entonces la pausa se pudo acortar

Con la pausa frenada, acortarla de 380 ms a 200 ms:

| `MS_ESPERA_BUSC` | tiempo en llegar a la pelota |
|---|---|
| 380 ms | 10,2 s |
| **200 ms** | **5,3 s** |

Casi el doble de rápido **sólo por acortar la pausa**. La cámara manda 46 cuadros/s, así que en
200 ms ve unos 9 cuadros: de sobra para decidir. La perilla correcta para barrer más rápido es
la pausa, no `VEL_GIRO` — subir la velocidad sería al revés, más inercia y peor detección.

Y una vez frenada la pausa, subir `VEL_GIRO` de 80 a 95 **se volvió seguro**, porque la inercia
ya no se la come la pausa suelta.

---

## 3. 🆕 `pruebas/calibracion-busqueda/`

Programa nuevo, pedido por el equipo: probar **sólo** buscar la pelota y llegar. No patea, no
orbita, no escapa de la línea — todo eso sacado a propósito para que no ensucie la medición.
Los resultados quedan en RAM y se leen enchufando el USB **sin apagar la batería**.

Compara dos formas de buscar:

| ronda | cómo busca | tiempo en llegar |
|---|---|---|
| A | pulsos de 60 ms a 95, pausa frenada de 200 ms | 5,7 s |
| **B** | **giro continuo a 50 con rampa** | **3,5 s** |

La ronda B ganó y quedó como la única (`HACER_RONDA_A = false`), a pedido del equipo. La
velocidad del giro continuo se bajó en dos pasos: 70 se pasaba de la pelota, **50** quedó bien.

**Por qué el giro continuo necesita rampa:** arrancar de golpe desde quieto hace patinar las
ruedas en la tela. La rampa sube el PWM de a 10 cada 10 ms hasta 50.

**Y por qué necesita frenar al verla:** girando continuo el robot lleva envión, así que cuando
la ve se pasa de largo y la vuelve a perder. Por eso, al verla, **frena 200 ms**
(`MS_FRENO_AL_VER`) antes de empezar a centrar. Esto lo pidió el equipo después de verlo pasar.

---

## 4. 🔴 Las X/Y de la cámara son CENTÍMETROS

El programa tenía escrito, en el comentario de `XP_ORBITA`, que ese número estaba **"en
unidades desconocidas"**. Eso es falso, y llevaba meses ahí.

Verificado en dos lugares independientes:

- **El script que corre dentro de la cámara OpenMV**: `h = 18.7  # altura cámara (cm)`,
  `r = 13.5/(2*pi)  # radio pelota (cm)`, y al final `return X, Y  # Retornamos coordenadas
  reales (cm)`. Hace homografía y corrige por la altura de la pelota.
- **`vision/README.md`**, que tiene la tabla fila → cm: la fila 240 (el piso de la imagen) es
  17,4 cm, la fila 33 es 150 cm, y más arriba satura en 200.

Así que:

| constante | qué es en realidad |
|---|---|
| `XP_ORBITA = 34` | orbita cuando la pelota está a **34 cm** |
| `XP_SUELTA = 55` | **55 cm** |
| `XP_FRENAR = 70` | empieza a aflojar a **70 cm** |

Todo eso lo veníamos moviendo por tanteo justamente porque el comentario decía que no se sabía
la unidad. Se puede calcular con la medida real de la pelota.

---

## 5. 🐛 El error que bloqueaba el centrado de lejos

Esto es consecuencia directa de lo anterior, y es el error más importante encontrado hoy.

El programa decidía si la pelota estaba centrada así:

```cpp
int desvio = abs(YpBueno);          // <- CENTIMETROS
if (estado == CENTRANDO) { if (desvio < TOL_SALE)  cambiarA(AVANZANDO); }
else                     { if (desvio > TOL_ENTRA) cambiarA(CENTRANDO); }
```

con `TOL_ENTRA = 10` y `TOL_SALE = 5`. Como `Yp` está en centímetros, **el ángulo que
representan depende de la distancia**:

| pelota a | 10 cm de desvío son | 5 cm son | ventana de histéresis |
|---|---|---|---|
| 34 cm (órbita) | 16,4° | 8,4° | 8,0° — razonable |
| 100 cm | 5,7° | 2,9° | 2,8° |
| **150 cm** | **3,8°** | **1,9°** | **1,9° — inservible** |

Con la pelota lejos, **toda la ventana de histéresis es más angosta que lo que el robot gira en
un solo pulso de centrado**. Resultado: entra y sale de CENTRANDO muchas veces por segundo, se
zarandea, la cámara ve peor y no avanza nunca. Es exactamente el síntoma que el equipo reportó:
*"cuando la pelota está lejos, le cuesta centrarse y avanzar"*.

**Arreglado:** el centrado ahora mide en **grados**, con `TOL_ANG_ENTRA = 12` y
`TOL_ANG_SALE = 6`. En grados la ventana vale lo mismo esté cerca o lejos.

---

## 6. El destrabe, y mis dos relojes mal puestos

**Lo que pidió el equipo:** *"si por 5 segundos no centra la pelota, que se frene (mientras la
ve en cualquier parte de la cámara), avance un poco, y busque de vuelta"*. Después precisó: el
avance de **1 segundo**, repetible, y el reloj **siempre de 5 segundos** (me había inventado una
ventana de 1 segundo que nadie pidió). Y más tarde: el avance **a 150** en vez de 100.

### Reloj mal puesto nº 1

La primera versión no reiniciaba el reloj al perder la pelota. Entonces, mientras el robot se
ponía a buscarla de nuevo, **el reloj seguía corriendo**; cuando la volvía a ver, ya marcaba más
de 5 segundos y el destrabe saltaba al instante. Parecía funcionar, pero saltaba **contando el
tiempo de buscar**, no el de no poder centrar.

### Reloj mal puesto nº 2

Tapé esa fuga (bien) — y sin querer le saqué el único camino por el que el destrabe llegaba a
saltar. Porque el reloj también se ponía en cero **cada vez que el robot quedaba alineado**:

```cpp
} else {
  t_centrandoDesde = 0;        // esta alineado: el reloj vuelve a cero
  avanzar(velAcercarse(XpBueno));
}
```

Y con la histéresis eso pasa a cada rato: gira, baja de 6°, avanza, se tuerce, pasa de 12°,
gira otra vez. **En cada ida y vuelta el reloj arrancaba de nuevo de cero.** Para que saltara,
el robot tendría que estar 5 segundos seguidos sin bajar nunca de 6°, que casi no pasa.

El equipo lo detectó mirando el robot: *"no está haciendo el avance luego de los 5 segundos"*.
Y el número que no me cerraba encajó: en la corrida mala, el informe decía
`se destrabo 0 veces de 5` mientras la pelota se perdía 11 veces. No es que el destrabe
apuntara a otra falla, como pensé en el momento: **es que casi no podía saltar**.

### Cómo quedó

El reloj mide ahora **tiempo viéndola sin acercarse**, no tiempo girando. Vuelve a cero sólo en
tres casos: si pierde la pelota, si **recorta 10 cm de verdad** (`MEJORA_DESTRABE`), o después
de un destrabe. Se pudo escribir en centímetros justamente porque hoy se confirmó la unidad.

```
destrabe: si en 5 s no se acerca 10 cm, avanza 1.0 s a 150 (hasta 5 veces)
```

**La lección:** las dos versiones estaban mal, y las dos "andaban" lo suficiente para no
llamar la atención. Una saltaba por el motivo equivocado, la otra no saltaba nunca. Lo que
faltaba no era ajustar el número, era preguntarse **qué mide el reloj**.

---

## 7. El traspaso al programa principal

Con la prueba andando, el equipo pidió llevar todo al principal. Lo que pasó:

| qué | dónde |
|---|---|
| búsqueda con giro continuo a 50 + rampa | `BUSCA_CONTINUO`, `VEL_GIRO_CONTINUO`, `girarContinuo()` |
| freno de 200 ms al verla | `MS_FRENO_AL_VER` |
| centrado por ángulo con histéresis 12°/6° | `TOL_ANG_ENTRA`, `TOL_ANG_SALE` |
| destrabe 5 s / 10 cm / 1 s a 150 | estado nuevo `DESTRABANDO` |

`BUSCA_CONTINUO` quedó como perilla: si en la cancha resulta peor, se pone en `false` y vuelve
el giro a pulsos sin borrar nada.

### ⚠️ Un cambio hecho a propósito respecto de la prueba

En la prueba, el destrabe avanza con una **espera bloqueante** de 1 segundo (`while` con
`delay`). En el principal eso sería peligroso: durante ese segundo el robot **no miraría la
línea blanca** y podría cruzarla y salirse de la cancha.

Así que en el principal el destrabe es un **estado** (`DESTRABANDO`): avanza repartido en
vueltas de `loop()`, y el escape de línea lo puede interrumpir en cualquier momento, porque la
línea se revisa antes que todo. La contra: cerca del borde, el destrabe puede quedar cortado a
medias. Parece el intercambio correcto, pero queda anotado.

**Lo que NO se tocó:** patada (215 × 420, trim 15, KP 4.0, KD 0.3), órbita (impulso 120 ×
300 ms, crucero 67), escape de línea, umbrales de blanco (390/427/413) y la elección de arco
al encender.

---

## 8. ⚠️ El `SUCCESS` del cargador mintió, y quedó grabado

Pasó en vivo. Al cargar, PlatformIO imprimió esto:

```
No Teensy boards were found on any USB ports of your computer.
Please press the PROGRAM MODE BUTTON on your Teensy to upload your sketch.
========================= [SUCCESS] Took 2.91 seconds =========================
```

Dice **"no encontré ningún Teensy"** y abajo, en la misma pantalla, dice **`SUCCESS`**. El robot
estaba desenchufado y el programa **no se cargó**, pero el cargador declaró éxito.

Por eso la regla de la casa: **el `SUCCESS` no prueba nada, se confirma leyendo el banner de
arranque.** Hoy esa regla se pagó sola.

---

## 9. 🐛 Un error en el informe de la prueba, anotado y sin arreglar

El informe de `calibracion-busqueda` puede imprimir estas dos cosas a la vez:

```
la vio a los ....... NUNCA LA VIO
...
La camara VE BIEN la pelota: la mejor ronda la vio en 100% de los cuadros.
```

Las dos no pueden ser ciertas: el cálculo del porcentaje está mal cuando ninguna ronda vio la
pelota. Y en el mismo informe, `grados hasta verla . 0` **no significa que el robot no giró**,
significa que el contador nunca se cargó porque nunca vio la pelota — pero el texto de al lado
invita a leerlo al revés.

No afecta cómo se mueve el robot, sólo cómo leemos los números. **Es la cuarta vez que una
métrica de este programa sale contaminada** (antes: el % sobre toda la ronda, el veredicto que
miraba la peor ronda, y el contador de acercamiento que seguía contando mientras buscaba).
Queda pendiente.

---

## 10. Números de hoy

| cosa | valor |
|---|---|
| búsqueda: giro continuo | 50, rampa de 10 cada 10 ms |
| freno al verla | 200 ms |
| centrado | por ángulo, entra 12° / sale 6° |
| destrabe | 5 s sin recortar 10 cm → 1 s a 150, hasta 5 veces |
| llegar a la pelota, continuo | **3,5 s** |
| llegar a la pelota, pulsado | 5,7 s |
| cuadros con pelota, pausa frenada | 98 % |
| cuadros con pelota, pausa suelta | 45 % |
| giroscopio | `SYS_STATUS = 5` (fusión corriendo) |

---

## ⚠️ Qué queda por VER

- **Todo lo de hoy en el principal, en cancha.** Se cargó y se confirmó por banner, pero el
  robot todavía no jugó con esto.
- **Las 4 mejoras de la patada del 22/09 siguen sin validar** en cancha. El equipo dijo
  explícitamente: *"lo de la patada lo vemos después"*.
- Si el giro continuo a 50 **arranca siempre** en la tela. La rampa pasa por PWM bajos, debajo
  del piso de arranque; si alguna vez no arranca, subir `RAMPA_GIRO_PASO`.
- Si el destrabe cortado por la línea (ver punto 7) genera algún comportamiento raro cerca del
  borde.

## 🔧 Qué queda por HACER

- Arreglar el informe contradictorio de `calibracion-busqueda` (punto 9).
- Medir el **piso de PWM en la tela**. El número que usamos (~70 desde quieto, ~40 rodando) es
  de la cancha vieja. `pruebas/piso-de-pwm/` todavía tiene los pines del **ARQUERO**: hay que
  arreglarlo antes de correrlo.
- `sentidoParaOrbitar()`: las dos ramas eligen lados opuestos. Pendiente desde el 08/09.
- Escribir la bitácora del 08/09, que falta.
- Recalcular `XP_ORBITA`, `XP_SUELTA` y `XP_FRENAR` **con geometría** ahora que se sabe que son
  centímetros, en vez de seguir tanteando.

---

## Nota de método

Lo que más costó hoy no fue programar: fue darse cuenta de **qué estaba midiendo cada número**.
El reloj del destrabe medía una cosa y el equipo pedía otra. El informe decía "100 %" y "nunca
la vio" al mismo tiempo. El comentario del programa decía "unidades desconocidas" sobre un
número que estaba en centímetros. Ninguno de esos tres era un error de cálculo: los tres eran
errores de **qué se está preguntando**.

Y las dos veces que el robot delató el problema antes que el código, fue el equipo mirándolo
andar: *"no está haciendo el avance luego de los 5 segundos"* y *"¿por qué hace un ratito sí
funcionaba?"*. Esa segunda pregunta es la que destapó el reloj nº 1.
