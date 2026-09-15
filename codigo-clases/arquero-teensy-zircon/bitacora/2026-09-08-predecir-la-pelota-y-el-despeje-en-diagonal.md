# 2026-09-08 — El arquero aprendió a anticipar, y el giroscopio desapareció

**Quiénes:** Diego y Laureano (mesa y cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa:** `funciona/seguir-y-despejar`. Herramienta nueva: `pruebas/herramientas/mirar.bat`

**Resumen del día:** se trabajó el pedido del equipo de que el robot **anticipe** en vez de
correr siempre atrás de la pelota. Quedaron tres cosas nuevas andando — predicción, monitor en
vivo y despeje en diagonal — y al final apareció un problema que **no es de código**: el
giroscopio dejó de existir para el robot.

| | |
|---|---|
| §1 | Predecir adónde va a estar la pelota |
| §2 | El monitor en vivo, y por qué hacía falta |
| §3 | El despeje en diagonal — la idea la puso el equipo |
| §4 | Cómo se calcula el ángulo sin calibrar nada |
| §5 | La traba que impedía usar la diagonal |
| §6 | 🚨 El giroscopio no contesta, y no es la batería |

---

## 1. Predecir adónde va a estar la pelota

**El problema, contado por el equipo el 01/09:**

> *"Cuando la pelota viene rápido en diagonal, el robot despeja pero la pelota a veces pasa de
> largo, porque el robot se movió hacia adelante en un eje que **ya quedó antiguo**."*

El diagnóstico era correcto: todo el control apuntaba a **donde la pelota ESTÁ**, y para cuando
el robot terminaba de reaccionar, ya no estaba ahí.

### Cómo se saca la velocidad

Más simple de lo que suena. La cámara manda 26 veces por segundo, así que:

```
velocidad = (dónde está ahora − dónde estaba antes) / tiempo que pasó
```

Si en un cuadro la pelota estaba a 10 cm al costado y en el siguiente a 12, se corrió 2 cm en
38 ms.

⚠️ **Hay que suavizarla.** Restar dos lecturas seguidas es la forma más ruidosa de medir que
existe: la resta se lleva los errores de las dos lecturas juntos, y encima se divide por un
tiempo muy corto, lo que agranda la equivocación. Por eso la velocidad no se usa cruda, se
promedia con las anteriores pesando más lo nuevo.

### Y la anticipación

```
dónde va a estar = dónde está + velocidad × 250 ms
```

Esos **250 ms** son el tiempo que el robot tarda en reaccionar: ~115 ms esperando los 3 cuadros
seguidos que exige antes de creerle a la cámara, más ~135 ms arrancando los motores.

Es lo mismo que hace un arquero de verdad cuando se tira **antes** de que la pelota llegue.

### Un número que salió gratis

Con la pelota **quieta** sobre la mesa, el robot midió una velocidad de hasta **3,6 cm/s**. Eso
es puro ruido de la cámara, y define el piso: cualquier velocidad menor que eso no significa
nada. Quedó como constante (`PISO_VELOCIDAD = 4,0`).

### Una corrección al registro

En la bitácora del 01/09 quedó escrito que esto estaba **bloqueado** hasta medir la conversión
del eje Y de la cámara. **Era un error.** Solo haría falta si quisiéramos que los números
signifiquen centímetros; para el control alcanza con trabajar en unidades consistentes.

---

## 2. El monitor en vivo

El equipo pidió ver los datos de la cámara en la terminal, **sin que el robot se mueva**.

Se agregó la tecla `M`: imprime 5 veces por segundo una línea con todo lo que el robot ve y
decide — distancia, desvío, velocidad, predicción, la fuerza que aplicaría y si dispararía.
**Y apaga el robot al encenderse**, para poder mover la pelota con la mano sin que salga
corriendo.

```
  t(s)    dist   desvio    vel   predic  fuerza seg disp
   12.4   27.2    -9.4    0.0    -9.4    IZQ 108  0  torcida
```

### Y una herramienta que faltaba: `mirar.bat`

Hasta ahora, para hablarle al robot había que pedírselo a Claude. Ahora hay un monitor propio
que se abre con **doble clic**, muestra todo en vivo y **deja escribirle teclas directamente**.

Encuentra el Teensy solo, buscándolo por el identificador del fabricante (`VID 0x16C0`, PJRC),
así que **no importa en qué COM caiga** — que era un dolor de cabeza recurrente.

⚠️ Mientras esa ventana está abierta, **Claude no puede hablarle al robot**: el puerto serie lo
toma uno solo. Ya nos hizo fallar una carga. Se cierra con Ctrl+C.

---

## 3. El despeje en diagonal — la idea la puso el equipo

Se plantearon dos caminos y se discutieron antes de escribir nada:

| | Idea |
|---|---|
| **A** | Predecir: calcular la velocidad y apuntar a donde va a estar |
| **B** | No ir a ciegas: corregir de costado **mientras** carga |

Claude recomendó la **B**. El equipo escuchó, probó, y después propuso **una tercera que es
mejor que las dos**:

> *"Que despeje usando el giroscopio **en diagonal** si es necesario, **prediciendo el ángulo**
> con el que debe salir a despejar."*

### Por qué la idea del equipo es mejor

La **B** necesita ver la pelota durante toda la carga. Pero el propio equipo había señalado el
problema: *"a veces la pierde, o está muy cerca"*. **Cuando la pelota se le mete encima, la
cámara manda `Xp = 0`** — y la B se queda sin información justo en el momento decisivo.

La idea del equipo **calcula el ángulo una sola vez, antes de salir**, cuando todavía la ve
bien. Después no necesita verla más: se compromete con esa diagonal y el giroscopio la sostiene.

> **El giroscopio no dirige: IMPIDE QUE EL ROBOT GIRE.** Eso es justo lo que hace posible la
> diagonal. Sin él, al mezclar avance con costado el robot se iría torciendo y la "diagonal"
> saldría curva.

### Las dos reglas que puso el equipo

1. **Si la cuenta dice que no llega, NO SALE.** Un arquero que sale y no llega queda fuera de
   posición **y** además le hacen el gol: es lo peor de los dos mundos. Se queda tapando.
2. **Vuelve por la misma diagonal** hasta pisar la línea blanca. Si la ida fue torcida y la
   vuelta fuera derecha, el robot terminaría corrido del centro. El costado se invierte y se
   escala en la misma proporción que la potencia, para que el **ángulo** sea el mismo.

---

## 4. Cómo se calcula el ángulo sin calibrar nada

Hay **dos motivos** para irse en diagonal, y se suman.

### Motivo 1 — la pelota está corrida

Es una regla de tres y nada más. Si está a 30 cm adelante y 6 al costado, hay que correrse 6 por
cada 30 que se avanza:

```
costado = (6 / 30) × avance
```

🎯 **Lo lindo:** los dos números **se dividen entre sí**, así que cualquier error de escala de la
cámara **se cancela solo**. Este término no necesita ninguna calibración. Es puntería con una
regla.

### Motivo 2 — la pelota se está corriendo sola

Si cruza a 20 cm/s, el robot tiene que correrse a esos **mismos** 20 cm/s. Si los dos van al
mismo ritmo de costado, **se encuentran sí o sí**, tarden lo que tarden en juntarse. Acá sí se
usa un número medido: a potencia 200 el robot hace 100 cm/s, o sea **2 de PWM por cada cm/s**.

### El reparto del motor

Una rueda no pasa de **255**. La más cargada recibe el avance entero más **la mitad** del
costado. Se le reservan 45 para el giroscopio y quedan **210** para el movimiento.

> 🚨 **Si no entra, se achican el avance y el costado JUNTOS — nunca solo el costado.**
> Recortar solo el costado dejaría al robot yendo en una dirección que **no es la que calculó**:
> ni derecho ni en la diagonal buena. Achicando los dos en la misma proporción, la diagonal
> apunta exactamente igual y solo se recorre más despacio. La ida se estira para compensar.

### Quedó un solo número de fe

`rendimientoCostado = 0,80` — cuánto rinde el costado comparado con el avance, con el mismo PWM.
Sale de la geometría del 50/50/89, no de una medición.

Y tiene una ventaja: **tapa también el otro agujero conocido** (que al eje Y de la cámara se le
aplica el mismo factor 2,87 que al X sin haberlo medido). Si ese estuviera errado, los dos
términos de la cuenta se escalan igual, y esta misma perilla los corrige. **Una perilla, dos
agujeros** — aunque nunca sepamos cuál era.

---

## 5. La traba que impedía usar la diagonal

Un análisis del código encontró que **la diagonal casi nunca se iba a activar**.

El robot solo dispara si la pelota va a estar a **≤ 5,2 cm** de estar enfrente. Ese límite se
había puesto cuando el despeje era **derecho**: no tenía sentido salir si la pelota no estaba
justo adelante, porque yendo derecho no la alcanzaba nunca.

Pero con la diagonal **sí puede alcanzar pelotas corridas**. Con el límite en 5,2 el robot seguía
negándose a salir **exactamente en los casos que la diagonal venía a resolver**.

> Era como enseñarle a correr en diagonal y dejarle puesta la prohibición de moverse del centro.

El equipo lo subió a **7,0 cm**, que ensancha el pasillo de ~10 a ~14 cm (unos ±13 grados desde
30 cm). Se puede ensanchar **porque si no llega, no sale**: sin esa protección, subirlo sería
peligroso.

---

## 6. 🚨 El giroscopio no contesta, y no es la batería

Al cargar el programa de juego y llevarlo a la cancha, **el LED empezó a parpadear rápido** — el
aviso de "sin giroscopio" que el equipo había pedido la clase pasada.

### El equipo refutó la hipótesis principal

Midieron la batería: **8,23 V**, más que la nominal de 7,8. Con eso **la explicación del 01/09
(batería floja) queda descartada** para este caso.

Eso es progreso, y lo hicieron ellos con una medición.

### La segunda hipótesis también cayó

Claude propuso que el sensor tardaba en dar datos después de encender (contesta que existe antes
de tener un rumbo válido). El equipo propuso la prueba correcta —**esperar a que devuelva algún
dato**— y se programó: ahora el robot no se arma hasta tener rumbo, y **mide cuánto tardó**.

Pero al leerlo, el robot contestó otra cosa:

```
!! SIN GIROSCOPIO despues de 10 intentos
   giroscopio: NUNCA APARECIO al encender
```

**`bno.begin()` falla 10 veces seguidas en 3 segundos.** El chip no contesta en el bus. Ni
siquiera llega a saludar, así que no hay ningún dato que esperar.

### Lo que sí sabemos

| Momento | Giroscopio |
|---|---|
| 19:40, en la mesa | `contesto al intento 1` ✅ |
| *el robot va a la cancha y vuelve* | |
| 20:10, en la mesa | `NUNCA APARECIO` ❌ |

Y una pista fuerte: **la cámara anda perfecto** (veía la pelota a 27,2 cm) y los sensores de
línea también. O sea que el robot tiene energía. **Lo único que falta es el giroscopio.**

**Entre las dos lecturas, el robot viajó.** Todo apunta a algo físico, no a código.

---

## Números de hoy

| Qué | Valor |
|---|---|
| Anticipación de la predicción | **250 ms** (115 de los 3 cuadros + 135 de arranque) |
| Ruido de velocidad con la pelota quieta | **3,6 cm/s** — ese es el piso |
| `PISO_VELOCIDAD` | 4,0 cm/s |
| `umbralDesvio` | 5,2 → **7,0 cm reales** |
| `PWM_POR_CM_S` | **2,0** (200 de PWM = 100 cm/s, medido el 04/08) |
| `rendimientoCostado` | **0,80** — el único número de fe |
| Reparto del motor | 210 para moverse, 45 reservados para el giroscopio |
| Avance mínimo del despeje | 140 de PWM (debajo de eso no llega) |
| Batería medida | **8,23 V** (nominal 7,8) |
| Parpadeo de "sin giroscopio" | 10 por segundo |

---

## Qué queda pendiente

### 🔴 Lo primero la próxima clase

- ⬜ **Revisar el conector del giroscopio.** SDA en el pin **18**, SCL en el **19**, más
  alimentación y masa. Que no conteste *nada* suele ser el cable de alimentación o el de masa;
  uno de datos flojo daría lecturas raras, no silencio.
- ⬜ **Si el conector está bien: escáner de I2C.** Un programa corto que pregunta a las 128
  direcciones posibles y lista lo que hay. Si el `0x28` no aparece, es hardware seguro. Si
  aparece, el problema es otro. Quedó propuesto y sin escribir.
- ⬜ **Nada de lo nuevo se pudo probar en cancha**, porque sin giroscopio la diagonal es **peor**
  que ir derecho: la mezcla lateral genera una rotación que nadie corrige. Tecla `D` para
  apagarla mientras tanto.

### Del comportamiento (todo cargado, nada probado)

- ⬜ **Predicción** — probar con la pelota cruzando rápido. Si se adelanta de más, bajar la
  anticipación con `w`.
- ⬜ **Despeje en diagonal** — si le pasa por delante a la pelota, **bajar** `rendimientoCostado`
  con `r` (si rinde menos de lo que creíamos, hay que pedirle más).
- ⬜ **Persecución durante el despeje** (la "opción B") — quedó escrita y **apagada**, tecla `P`.
  Se dejó para no mezclar dos cosas nuevas en la misma corrida.
- ⬜ **Achicar el avance del despeje.** Sigue saliendo ~50 cm para una pelota que está a 30. La
  cuenta para 35 cm: `(35 + 3,3) × 10 = 383 ms`.
- ⬜ **Volver al centro al perder la pelota**, con el arco azul.

### De la cámara

- ⬜ **Medir la conversión del eje Y.** Ya no bloquea nada, pero sigue siendo un número supuesto.
- ⬜ **La zona muerta de cerca.** El robot ya cuenta en cuántos cuadros del avance llegó a ver la
  pelota (tecla `i`). Falta hacer un despeje y leerlo.

### De hardware

- ⬜ **Un LED que se vea.** El del pin 13 está tapado por la batería. Pines **9** y **10**
  libres; el código ya está preparado con `PIN_AVISO`, es cambiar un número.

---

## Lo que se aprendió del método

**1. La mejor idea del día la puso el equipo, y superó a la de Claude.**
Claude recomendó corregir sobre la marcha. El equipo propuso calcular el ángulo una vez y
comprometerse — y tenían razón por un motivo concreto que ellos mismos habían observado: *la
cámara pierde la pelota justo cuando la tiene encima*. Una solución que depende de mirar se
queda ciega en el peor momento.

**2. Una medición del equipo tiró abajo la hipótesis principal.**
8,23 V. Sin ese número hubiéramos seguido persiguiendo la batería toda la clase.

**3. Arreglar algo no sirve si el arreglo nunca se activa.**
La diagonal estaba bien programada y no se iba a usar jamás, porque otro número la vetaba. Vale
la pena preguntarse siempre: *¿en qué casos se va a ejecutar esto que acabo de escribir?*

**4. Cuando algo funcionaba y dejó de funcionar, mirá qué pasó en el medio.**
El giroscopio andaba a las 19:40 y no andaba a las 20:10. Lo que pasó en el medio fue un viaje a
la cancha. Eso vale más que cualquier hipótesis sobre el código.
