# 2026-08-11 — Sensores de línea, despeje que vuelve al área, y el arquero siguiendo la pelota

**Quiénes:** Gustavo Viollaz + los chicos (banco y cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas nuevos:** `pruebas/ver-linea`, `pruebas/buscar-sensores`, `pruebas/calibrar-linea`,
`funciona/seguir-y-despejar`. Modificado: `funciona/despeje-pelota`.

**Consigna del profe para hoy**, en dos partes:

1. **Despeje**, con un cambio: en vez de retroceder una distancia fija, **retroceder hasta
   detectar la línea blanca del área**, y después un movimiento chico hacia adelante.
2. **Movimiento en paralelo al arco** para protegerlo, compensando con el giroscopio el hecho de
   que la rueda trasera deja al robot chueco al final del movimiento.

Se hicieron las dos.

---

## 0. Antes de empezar: el lío de las dos bitácoras, resuelto

La otra mesa había **borrado nuestra bitácora de identificación** creyendo que se contradecían.
No se contradicen: **son dos robots distintos, cableados distinto**, y cada mesa midió bien el
suyo.

| Robot | Trasera medida en | `M3` del... | Es |
|---|---|---|---|
| El de esta mesa | 11/12/4 (U7) | `ROBOT1` | **ARQUERO** |
| El de la otra mesa | 2/5/3 (U5) | `ROBOT2` | **DELANTERO** |

Se restauró nuestra bitácora con una nota explicándolo. **El equipo trabaja en dos mesas a la
vez y las dos escriben en el mismo repo: antes de dar de baja la medición de otro, hay que
fijarse si habla de tu robot.**

---

## 1. Los sensores de línea: dos pruebas que mintieron y una que funcionó

El robot tiene tres sensores de línea en los pines A11, A13 y A12. El código los llama 1, 2 y 3,
pero **nadie había anotado cuál es cuál en el robot**. Mismo problema que con las ruedas.

### Lo que NO funcionó (dos veces)

**Tapar un sensor con algo blanco y mirar cuál número salta.** Se hizo dos veces, y las dos veces
saltó el mismo canal (A13). Conclusión aparente: *"dos de los tres sensores están rotos"*.

**Era falso.** Con la tapa apoyada encima, solo uno quedaba realmente cubierto. El robot estaba
sano; la prueba estaba mal diseñada.

### El descarte que aclaró todo

Antes de dar por rotos dos sensores se corrió `pruebas/buscar-sensores`, que lee **las 18
entradas analógicas del Teensy a la vez** y las ordena por cuánto se movieron. La idea: si los
sensores estuvieran en otros pines (la librería tiene tres versiones de placa distintas),
aparecerían solos.

Pasando una hoja por debajo de toda la panza:

| Pin | Se movió |
|---|---|
| A12 | **684** |
| A11 | **681** |
| A13 | **661** |
| A0-A3, A6-A9 | ~620 cada uno (son los infrarrojos de la pelota, reaccionan igual) |
| A4, A5 | 3 y 5 (son SDA/SCL del giroscopio, clavados en 1023 — correcto) |

**Los tres sensores de línea andan, y están donde decía la librería.**

### Lo que SÍ funcionó: dos superficies conocidas y el robot quieto

El método bueno apareció solo: apoyar el robot **a caballo entre la mesa blanca y un pad negro**.
Nada que sostener con la mano, nada que pase rápido, y se lee tranquilo.

| Posición | A13 | A11 | A12 |
|---|---|---|---|
| Los dos de atrás en el pad, el de adelante en la mesa | 150 | 114 | **761** |
| Girado 180° (al revés) | **765** | **765** | 124 |
| Solo el de atrás-izquierda en el pad | **153** | 765 | 761 |

Los números **se dieron vuelta exactamente** al girarlo. Eso es lo que convierte una observación
en una medición.

### 🎯 EL MAPA DE SENSORES

| Pin | Posición |
|---|---|
| **A12** | **adelante** |
| **A13** | **atrás IZQUIERDA** |
| **A11** | **atrás DERECHA** |

**Más claro = número más alto.** Blanco ~765, oscuro ~120.

> **Lección, la misma que ya había aprendido la otra mesa con las ruedas:** diseñá el test
> alrededor de la pregunta más fácil de contestar. Una superficie fija bajo un robot quieto
> gana por lejos a una mano moviendo un papel.

---

## 2. Calibración en la cancha: verde, blanco y el umbral

Con `pruebas/calibrar-linea`, que mide solo y sin computadora (avisa con el LED qué medir, se
acuerda, y después se le pregunta con la tecla `m`).

### El verde, impecable

| Sensor | Verde |
|---|---|
| atrás-IZQ (A13) | **356** [355-359] |
| atrás-DER (A11) | **465** [463-468] |
| adelante (A12) | **350** [349-354] |

**4 o 5 puntos de variación en 79.000 muestras.** Número sólido.

### El blanco, sucio pero utilizable

| Sensor | Blanco |
|---|---|
| atrás-IZQ | promedio 555, rango **[295-768]** |
| atrás-DER | promedio 490, rango **[288-761]** |

El sketch avisó **"SE SUPERPONEN, no hay umbral que sirva"**. Era un artefacto de la muestra: en
los 4 segundos del blanco el robot no estuvo todo el tiempo sobre la línea. Los 290 son verde
colado; **los ~760 son la línea de verdad**, y coinciden con la mesa blanca del banco.

**Arreglo aplicado al sketch:** para el blanco ahora se usa el **máximo**, no el mínimo. El
blanco es lo más claro que hay en la cancha, así que el máximo de esa fase sí es la línea. El
promedio y el mínimo se siguen mostrando para poder darse cuenta de que la muestra salió sucia.

### 🎯 UMBRAL: 620

```
verde más claro (A11):  468
blanco de verdad:      ~760
umbral = punto medio =  614  ->  se puso 620
```

> **Los dos sensores de atrás NO son iguales.** Sobre el mismo verde, uno marca 356 y el otro
> 465 — **110 puntos de diferencia**. Distinta altura o suciedad. Con 620 los dos quedan
> holgados, pero si algún día uno empieza a fallar, ese es el primer sospechoso.

---

## 3. Despeje: volver por la línea en vez de por tiempo

**Antes:** avanzar 30 cm, retroceder un tiempo fijo.
**Ahora:** avanzar, **retroceder hasta que un sensor de atrás pisa blanco**, enderezarse, avanzar 10 cm.

**Por qué es mejor, y no es solo prolijidad:** el tiempo obliga a calibrar en centímetros y se
desajusta con la batería, con el piso, y con si la ida chocó la pelota o no. Ya nos había pasado:
con el mismo tiempo, la vuelta daba 53 cm donde la ida daba 30. **La línea del área es una marca
física: siempre está en el mismo lugar.**

### Decisiones tomadas y por qué

- **Frenan solo los dos sensores de ATRÁS.** Son los que cruzan la línea primero yendo marcha
  atrás. El de adelante se lee nada más que para mirarlo.
- **El retroceso va más lento que la ida.** El robot no tiene freno: al cortar los motores sigue
  de largo. Yendo rápido se pasaba varios centímetros de la línea.
  Probado en cancha: 140 resultó **"muy rápido"** → bajado a **110**.
- **El empujoncito de 10 cm va DESPUÉS de enderezarse**, no antes. Si fuera al revés, esos 10 cm
  saldrían torcidos y arruinarían el enderezado.
- **Freno de emergencia a los 1200 ms** si no encuentra la línea. El profe dejó pendiente qué
  hacer en ese caso; esto es solo para que el robot no se escape. El código 2025 tiene ese bug
  exacto: `PATEANDO_atras_arquero` sale **solo** por línea, sin timeout.

### Resultado en cancha

**"Despeja bien"**, y con el retroceso a 110, **"quedó decente"**.

⚠️ **"No se enderezó bien"** — ver pendientes.

---

## 4. El arquero completo: seguir la pelota de costado

`funciona/seguir-y-despejar` — el programa nuevo, que junta todo.

**Sin pelota:** quieto. **Ve la pelota lejos:** se corre de costado para ponerse enfrente, sin
dejar de mirar al frente. **La pelota se acerca:** despeja, vuelve por la línea, se endereza.

### Cómo se mezcla moverse con no girar

El código 2025 resolvía esto con **tablas de valores fijos**: tres ramas de `if` con números
escritos a mano (50/50/89, 65/40/100...) según cuánto estaba torcido.

Acá se hace **sumando dos cosas independientes**:

```
lo que cada rueda tiene que hacer para IR DE COSTADO
+ lo que cada rueda tiene que hacer para NO GIRAR
= lo que se le manda a esa rueda
```

Ir de costado son las tres ruedas en proporción **50/50/89** (sale de la geometría del robot, es
la del 2025). No girar son las tres parejas. Como son independientes, se calculan por separado y
se suman — y se puede tocar una sin romper la otra.

Para poder sumarlos hizo falta manejar los motores **con signo** (positivo = una pata alta) en
vez de "magnitud + dirección". No se puede sumar "50 para allá" con "20 para el otro lado" si no
hay signos.

La corrección de rumbo es la misma que en las rectas del cuadrado: proporcional + acumulada, con
tope anti-windup. Con solo proporcional quedaba un error fijo de 7°; con el acumulado bajó a
menos de 2°.

### 🎯 LOS DOS SIGNOS, DESCUBIERTOS

Ninguno se puede deducir leyendo el código. Si están al revés, el robot **se aleja de la pelota
en vez de seguirla**.

| Qué | Cómo se midió | Resultado |
|---|---|---|
| Sentido lateral | tecla `v`: movimiento corto de prueba | **correcto**, `lateralInvertido = false` |
| Signo de la cámara | pelota puesta a la DERECHA → la cámara mandó **Yp = −24** | **invertido**, `camaraYInvertida = true` |

O sea: **en la cámara, el negativo es la derecha.** Los dos quedaron escritos en el sketch, no en
teclas, porque las teclas se pierden al cortar la batería.

### Otros detalles del diseño

- **Zona muerta de 4 cm**: si la pelota está casi enfrente, no se mueve. Sin esto el robot
  tiembla persiguiendo el ruido de la cámara.
- **Cada sensor vigila su lado**: yendo a la izquierda mira el de atrás-izquierda, yendo a la
  derecha el de atrás-derecha. Así el sensor del lado opuesto no lo frena por nada.
- **Necesita ver la pelota 3 cuadros seguidos** para disparar el despeje.

### Resultado en cancha

**"Está bastante bien."** Sigue la pelota de costado y despeja.

### La distancia de disparo: 30 → 20 → 48

Se probaron tres valores en el día. Quedó en **48 cm**.

> 🚨 **REGLA que quedó escrita en el código:** `umbralCm` **nunca** puede ser mayor que el alcance
> de la ida. Si el robot dispara a 48 cm y la ida son 30, **frena 18 cm antes de llegar y no toca
> la pelota nunca**. Al subir el disparo a 48 hubo que subir la ida a **533 ms (~50 cm)**.
>
> Esto se puede cambiar libremente **porque la vuelta ya no depende de la distancia**: vuelve
> buscando la línea. Antes, cambiar la ida obligaba a recalibrar la vuelta.

---

## 5. 🚨 HALLAZGO DEL FINAL: los centímetros de la cámara NO son centímetros

Se descubrió al final de la clase, midiendo con la regla:

> **Cuando la cámara dice 48 cm, la pelota está a 12 cm de verdad.**

Está exagerando por un factor de **cuatro**.

### De dónde viene

La cámara no mide distancia. Ve un punto en la imagen y lo convierte a centímetros con una
**matriz de conversión** (una homografía) que está escrita en su programa
(`enviar_coordenadas_2_arcos_y_pelota.py`). Esa matriz **fue calibrada para una altura de cámara
de 18,7 cm**, y el propio README de la visión lo advierte:

> *"Esa matriz está calibrada para la altura de cámara de 18,7 cm. Si movés la cámara de lugar,
> la matriz deja de servir."*

Si la cámara de este robot está montada más baja, más alta o con otro ángulo que en 2025, la
conversión queda mal. Es lo que estamos viendo.

### Qué rompe esto

**Todo el programa está hablando en una unidad que no es la que dice.** No solo la distancia:

| Parámetro | Dice | En realidad es |
|---|---|---|
| `umbralCm = 48` | "despeja a 48 cm" | despeja a **~12 cm reales** |
| `umbralDesvio = 15` | "desviada 15 cm" | unidades de cámara, no cm |
| `ZONA_MUERTA_PELOTA = 4` | "4 cm" | unidades de cámara, no cm |

**Y deja el empujón de ida sobredimensionado.** Se puso en 533 ms (~50 cm reales) creyendo que la
pelota estaba a 48 cm. Está a 12. El robot sale 50 cm cuando le alcanzaría con 25 o 30, y se aleja
del arco mucho más de lo necesario.

⚠️ **Los centímetros del despeje SÍ son reales** (los 30 cm, los 10 cm, los 533 ms) porque salieron
de medir con regla el movimiento del robot. Los que no son reales son los que vienen de la cámara.
**No mezclar las dos cosas.**

### Qué medir la próxima clase

Con la regla y la pelota, en la cancha. Poner la pelota a **10, 20, 30, 40 y 50 cm reales** y
anotar qué dice la cámara en cada posición. Cinco lecturas, se hace con la tecla `i`.

Eso responde dos preguntas, y la segunda es la importante:

1. **¿Cuánto exagera?**
2. **¿Exagera parejo?** Si a 10 cm exagera por 4 y a 40 exagera por 2, la relación es curva y no
   se arregla con una multiplicación. **Sospecha:** estas matrices suelen fallar peor cerca del
   robot, así que probablemente no sea parejo.

### La decisión que hay que tomar después de medir

**Opción A — que el programa hable en unidades de cámara.** Renombrar todo (`umbralCm` pasa a ser
`umbralCamara`, etc.) para que nadie crea que son centímetros. Rápido y honesto, pero los números
del programa siguen sin significar nada físico.

**Opción B — convertir a centímetros de verdad.** Aplicar la corrección en el Teensy (o mejor,
recalibrar la matriz en la cámara con `calibrar-umbrales.py` y la altura real). Más trabajo, pero
después los números del programa son distancias reales y se pueden razonar.

---

## Números de hoy, para no volver a medirlos

| Qué | Valor |
|---|---|
| Sensor de adelante | **A12** |
| Sensor de atrás IZQUIERDA | **A13** |
| Sensor de atrás DERECHA | **A11** |
| Blanco / oscuro (banco) | ~765 / ~120 (más claro = más alto) |
| Verde en cancha | A13 **356**, A11 **465**, A12 **350** |
| Blanco en cancha | **~760** |
| **Umbral de blanco** | **620** |
| Potencia del retroceso | **110** (140 era muy rápido) |
| Freno de emergencia del retroceso | 1200 ms |
| `lateralInvertido` | **false** |
| `camaraYInvertida` | **true** (Yp negativo = derecha) |
| Distancia de disparo del despeje | **48 de cámara = ~12 cm REALES** |
| Ida del despeje | **533 ms ≈ 50 cm reales** (quedó grande, ver §5) |
| Empujón final | 133 ms ≈ 10 cm |
| Proporción de ruedas para ir de costado | 50 / 50 / 89 |
| Corrección de rumbo | KP 5.0, KI 1.5, tope 30 |

---

## Qué queda pendiente

### 🔴 Lo primero la próxima clase

- ⬜ **Medir la tabla de conversión de la cámara** (ver §5). Pelota a 10, 20, 30, 40 y 50 cm
  reales, anotar qué dice la cámara en cada una. **Va primero porque de esto dependen todos los
  demás números del comportamiento**: mientras no sepamos qué significan, cualquier umbral que
  ajustemos lo estamos ajustando a ciegas.
- ⬜ Con esa tabla, **decidir A o B** (unidades de cámara renombradas, o convertir a cm reales) y
  **achicar el empujón de ida**, que hoy quedó en 50 cm reales para una pelota que está a 12.

- ⬜ **El enderezado del despeje no funcionó bien.** Se reportó *"no se enderezó bien"* y quedó
  sin diagnosticar. **Importa más de lo que parece: es el mismo mecanismo que mantiene al robot
  derecho mientras se mueve de costado**, que es justo lo que pidió el profe.
  Ya se le agregó al sketch que **se acuerde de qué pasó** y lo cuente con la tecla `i`,
  distinguiendo cuatro casos: se enderezó bien / lo intentó y no llegó / **ni lo intentó porque
  el giroscopio estaba mudo** / estaba desactivado. Antes se salteaba en silencio, y desde afuera
  eso se ve igual que "se enderezó mal".
  **Un despeje más con el cable puesto alcanza para saber cuál de los cuatro es.**

- ⬜ **Probar el disparo a 48 cm en cancha.** Se cargó pero no se llegó a probar. A más distancia,
  más chance de que confunda algo naranja del entorno.

- ⬜ **El robot ahora sale ~50 cm del arco a despejar.** Es más de lo que mide el área de fondo.
  Ver si es aceptable para un arquero o si conviene acortar la ida y bajar el disparo.

### Del giroscopio (viene de la clase pasada)

- ⬜ **Probar con la batería cargada.** El giroscopio se queda mudo cuando la batería está baja
  (se alimenta de ahí, no del USB). Es lo que cierra ese caso.
- ⬜ Capacitor de reserva junto al sensor.

### De la cámara

- ⬜ 🚨 **Zona muerta justo adelante** — lo aprendió la otra mesa: cuando la pelota se le mete
  encima, la cámara manda `Xp = 0`, que nuestro código lee como *"no la veo"*.
  **Perder de vista una pelota que tenías pegada no es perderla: es tenerla.** Sin resolver.
- ⬜ Los umbrales de color son de la luz del laboratorio 2025. En cancha hay que recalibrarlos.
  La otra mesa subió `futbol-roboliga2026/vision/calibrar-umbrales.py` para eso.

### Del comportamiento

- ⬜ Qué hacer si el retroceso **no encuentra la línea**. Hoy frena y avisa (freno de emergencia).
  El profe lo dejó pendiente: ¿reintentar? ¿retroceder por tiempo? ¿buscar la línea de costado?
- ⬜ El robot **no vuelve al centro del arco** cuando pierde de vista la pelota. Se queda donde
  quedó. Puede ser un problema si la pelota reaparece del otro lado.

---

## Cómo se trabajó

Otra vez todo **sin abrir el Arduino IDE**, con los scripts de
[`../pruebas/herramientas/`](../pruebas/herramientas/).

**El patrón que se repitió todo el día**, y que conviene tener presente:

> Cada vez que el código tenía un nombre (sensor 1, motor M3, "derecha") y el robot tenía una
> posición física, **nadie había escrito la correspondencia**. Y no se puede deducir leyendo:
> hay que medirla. Pasó con las ruedas la clase pasada, con los tres sensores de línea hoy, y
> con los dos signos del seguimiento lateral.
>
> Cuando aparezca un comportamiento absurdo, **el primer sospechoso es una de esas
> correspondencias**, no la lógica del programa.
