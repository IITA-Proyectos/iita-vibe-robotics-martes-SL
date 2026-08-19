# 2026-08-18 — El robot patinaba al arrancar: freno, arranque suave y rumbo sostenido

**Quiénes:** Gustavo Viollaz + los chicos (banco y cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO**
**Programa:** `funciona/seguir-y-despejar`. Nuevo: `pruebas/probar-freno`.

**Resumen del día:** entramos creyendo que el problema era el giroscopio y resultó ser
**mecánico** — las ruedas patinaban al arrancar. Encontrarlo cambió todo lo que hicimos después.

---

## 1. Se sacó el límite lateral del seguimiento

**Pedido:** que el robot siga la pelota de costado **sin tope**. Lo único que lo detiene ahora es
perder de vista la pelota.

**Motivo del equipo:** el robot volvía chueco de algunos despejes, y estando torcido sus dos
sensores de atrás quedan en diagonal respecto de la línea — uno la pisa **antes** de que el robot
esté realmente en el borde, y el seguimiento se cortaba de más.

**Lo que NO se tocó:** el regreso del despeje sigue usando los dos sensores de atrás para saber
dónde parar. Son dos usos distintos de los mismos sensores y sólo se sacó el primero.

⚠️ **Esto sacó el síntoma, no la causa.** Quedó anotado en el código. Si algún día el enderezado
queda fino, conviene volver a probar con el límite puesto.

⚠️ **Consecuencia:** el robot puede terminar lejos del arco y ahí se queda — todavía no sabe
volver al centro.

---

## 2. El freno: una prueba que no servía y una que sí

### El problema

Volviendo del despeje, el robot detectaba la línea pero **se pasaba**: quedaba más adelante o más
atrás, distinto cada vez. La causa: no frenaba. `parar()` sólo suelta los motores, y el robot
sigue de largo por inercia.

### 🚨 La prueba en la mesa NO era una medición

Se probó con el cable puesto sobre la mesa: retroceso de 250 ms, frenando vs. soltando. Dieron
igual, y la conclusión aparente fue *"esta placa no soporta el freno"*.

**Era falso, y lo detectó Gustavo:** *"el robot casi ni llegó a moverse, si te digo que se movió
3 cm te miento"*.

En 3 cm el robot **nunca agarra velocidad**: está acelerando todo el tiempo y se detiene casi
solo. Un freno sólo se nota si hay inercia que frenar.

> **La lección:** un experimento que no puede distinguir las dos respuestas **no es una
> medición**, por más prolijo que sea el procedimiento. Antes de correr un test, preguntarse:
> *"si la respuesta fuera la otra, ¿este test lo mostraría?"*

### La prueba que sí sirvió

`pruebas/probar-freno`: tres corridas de **1 segundo** (para llegar a velocidad real), en el
**piso**, sin cable, marcando dónde quedaba cada una. Las tres formas de detenerse en un solo
viaje.

| Corrida | Cómo se detiene | Resultado |
|---|---|---|
| 1 | las dos patas de dirección en **ALTO**, PWM al máximo | ✅ **frena** |
| 2 | las dos patas en **BAJO**, PWM al máximo | ✅ **frena** |
| 3 | soltar (lo que había antes) | quedó **más lejos** |

**Las dos variantes del freno eléctrico funcionan.** Se usa la 2.

> ⚠️ **La variante 2 se parece peligrosamente a soltar:** mismo estado de las patas de dirección,
> y la única diferencia es el PWM. Con PWM en **cero** el driver apaga la salida y el motor queda
> suelto; con PWM al **máximo**, cortocircuitado. **Efecto opuesto.** Está avisado en el código.

**Resultado en cancha:** el robot **queda sobre la línea**. Problema resuelto.

---

## 3. 🎯 EL HALLAZGO DEL DÍA: las ruedas patinan al arrancar

Después del freno seguía apareciendo que *"a veces queda chueco"*. La descripción de Gustavo
resolvió el caso, y vale la pena citarla porque es exactamente el tipo de observación que sirve:

> *"Cuando avanza para despejar, arranca muy rápido, **patina** cuando arranca con esa
> potencia/velocidad y se va chueco hacia adelante, y cuando vuelve, vuelve marcha atrás en la
> misma posición o **apuntando donde terminó el avance**. Pasa lo mismo cuando la pelota va muy
> rápido y el robot la quiere seguir."*

### Qué estaba pasando

El robot arrancaba de golpe a potencia 200. Las ruedas patinan, **y no patinan igual las dos**:
una agarra antes que la otra, y ese instante de diferencia lo tuerce.

### Y algo que estaba invisible

**El avance del despeje no usaba el giroscopio para nada.** Se usaba para el movimiento lateral
y para enderezarse al final, pero **el avance salía a ciegas 50 cm**. Si se torcía al arrancar,
nadie lo corregía.

Y el retroceso tampoco corregía → **volvía por la misma diagonal torcida**. De ahí el
*"apuntando donde terminó el avance"*: literalmente estaba deshaciendo el camino chueco.

### Los cuatro arreglos

1. **Arranque suave en el avance** — sube a fondo en ~200 ms en vez de saltar.
2. **Rumbo sostenido durante el avance** — el giroscopio corrige los 50 cm enteros.
3. **Arranque suave en el movimiento lateral** — por lo de la pelota rápida.
4. **Arranque suave y rumbo sostenido en el retroceso.**

Más una quinta que apareció escribiéndolo: **cuando la pelota cruza de lado**, el robot tiene que
invertir la marcha. Sin reiniciar la rampa saldría para el otro lado a fondo de una — un tirón
peor que el del arranque, porque viene con velocidad en contra.

### Cómo se mezclan moverse y no girar

Se calculan **por separado y se suman**:

```
lo que cada rueda hace para MOVERSE (adelante, atrás o de costado)
+ lo que cada rueda hace para NO GIRAR
= lo que se le manda a esa rueda
```

Avanzar son las dos de adelante opuestas entre sí; girar son las tres parejas; ir de costado son
las tres en proporción 50/50/89. Como son movimientos independientes, se suman.

**La fuerza pasa por la rampa; la corrección de rumbo NO** — tiene que actuar enseguida, y además
es chica: no hace patinar nada.

### Resultado en cancha

> *"Funciona bastante bien, cuando arranca parece que va derecho y cuando retrocede se nota la
> corrección del giroscopio."*

---

## 4. El empujoncito final, medido

Se volvió a poner el empujón de 10 cm después de frenar y enderezarse, para que el robot termine
donde arrancó y no pegado a la línea. **Despacio** (potencia 100, no 200), con arranque suave y
rumbo sostenido, y **frenando al terminar** — en un movimiento tan corto, soltar se lleva puesta
buena parte de los 10 cm.

### La medición, y por qué la regla de tres no alcanzaba

| Tiempo | Distancia |
|---|---|
| 400 ms | **13 cm** (dos corridas) |
| **320 ms** | **9,5 cm** ✅ |

Para pasar de 13 a 10 cm, la regla de tres daba 308 ms. **No aplica**, porque los primeros 100 ms
el robot está acelerando por la rampa y ese tramo rinde menos distancia. Al acortar el tiempo
total, la rampa pasa a ser una porción más grande del viaje.

Descontando la rampa (~50 ms de recorrido equivalente) y el retardo mecánico de arranque
(~33 ms, medido el 04/08):

```
velocidad = 13 cm / (400 - 50 - 33) ms = 0,041 cm/ms
para 10 cm  ->  10/0,041 + 50 + 33  =  ~320 ms
```

**Predicho 10 cm, medido 9,5.** El modelo dio bien; la regla de tres hubiera dado ~9,1.

---

## Números de hoy

| Qué | Valor |
|---|---|
| Freno eléctrico | **las dos variantes funcionan**; se usa "las dos patas en bajo + PWM al máximo" |
| Duración del freno | 200 ms y después soltar |
| Rampa de arranque | +10 de PWM cada 10 ms (~200 ms hasta el fondo) |
| Empujón final | potencia 100, **320 ms ≈ 9,5 cm** |
| Velocidad a potencia 100 | 0,041 cm/ms (descontando rampa y arranque) |
| Límite lateral | **sacado** |

---

## Cómo quedó el arquero

| Paso | Cómo |
|---|---|
| Sigue la pelota de costado | sin límite, arranque suave, rumbo sostenido |
| Cambia de lado | sin tirón: la rampa se reinicia |
| Despeja | arranque suave **y sosteniendo el rumbo** los 50 cm |
| Vuelve | igual, hasta pisar la línea |
| Al pisarla | **frena** — queda sobre la línea |
| Se endereza | giroscopio |
| Empujoncito | 10 cm, despacio, derecho, frenando |

---

## Qué queda pendiente

### 🔴 Lo primero la próxima clase

- ⬜ **Medir la tabla de conversión de la cámara.** Viene de la clase pasada y sigue siendo lo más
  importante: **cuando la cámara dice 48 cm, la pelota está a 12**. Todos los umbrales del
  comportamiento están en una unidad que no sabemos qué significa, así que cualquier ajuste que
  hagamos lo estamos haciendo a ciegas.
  Poner la pelota a 10, 20, 30, 40 y 50 cm reales y anotar qué dice la cámara. Ver §5 de la
  [bitácora del 11/08](2026-08-11-sensores-de-linea-y-arquero-completo.md).
- ⬜ Con esa tabla, **achicar el avance del despeje**: hoy son 50 cm reales para una pelota que
  está a 12.

### Del comportamiento

- ⬜ **El robot no vuelve al centro del arco** cuando pierde de vista la pelota. Se queda donde
  quedó. Ahora que no hay límite lateral, esto pesa más que antes.
- ⬜ **Volver a probar el límite lateral** una vez que el enderezado esté fino. Se sacó por un
  síntoma, no porque estuviera mal la idea.
- ⬜ Qué hacer si el retroceso **no encuentra la línea**. Hoy frena a los 1200 ms y avisa.

### De la cámara

- ⬜ 🚨 **Zona muerta justo adelante**: la cámara manda `Xp = 0` tanto cuando no ve la pelota como
  cuando la tiene pegada encima. El programa lee ese cero como *"no la veo"*.
- ⬜ Los umbrales de color son de la luz del laboratorio 2025.

### Del giroscopio

- ⬜ **Probar con la batería cargada.** Viene de dos clases atrás. El giroscopio se queda mudo
  cuando la batería está baja.

---

## Lo que se aprendió del método

**Dos veces en el día una prueba nos mintió, y las dos veces el que la descubrió fue el que
miraba el robot, no el que miraba el código.**

1. *"El robot casi ni se movió"* → la prueba del freno en la mesa no era una medición.
2. *"Patina cuando arranca"* → el problema no era el giroscopio, era mecánico.

Yo tenía la hipótesis equivocada las dos veces y estaba por mandar a diagnosticar el enderezado.

> **Contar lo que se ve, no lo que uno cree que falla.** *"Arranca muy rápido y patina"* vale
> muchísimo más que *"anda mal el giroscopio"* — la primera dice dónde mirar, la segunda manda a
> buscar al lugar equivocado.
