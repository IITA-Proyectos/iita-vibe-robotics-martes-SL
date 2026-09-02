# 2026-09-01 — El robot terminaba apuntando al oeste, y era la batería

**Quiénes:** Diego y Laureano (cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa:** `funciona/seguir-y-despejar`

> Laureano volvió después de faltar al 18/08 y al 25/08. La primera parte de la clase fue
> ponerlo al día con lo de la cámara y la ubicación inicial.

**Resumen del día:** el robot no andaba derecho y terminaba girado. Se encontraron **tres bugs
reales** en el código de enderezado, se arreglaron los tres... y el problema seguía. Al final
apareció que la causa de fondo era otra: **la batería baja deja mudo al giroscopio mientras los
motores siguen andando perfectamente.**

| | |
|---|---|
| §1 | El diagnóstico en vivo: el robot dijo qué le pasaba |
| §2, §3, §4 | Tres bugs encontrados y arreglados |
| §5 | Oscilaba: freno eléctrico |
| §6 | 🎯 La batería — sospecha fuerte, **falta la prueba de control** |
| §7 | El LED no se ve, y el aviso que pidió el equipo |
| §8 | Lo que viene: predecir la trayectoria de la pelota |

---

## 1. El síntoma, y el diagnóstico en vivo

**Reportado en cancha:** *"se desvió hacia los costados y terminó apuntando hacia el oeste"*.

Con el robot conectado por USB y todavía corriendo, la tecla `i` contestó esto:

```
rumbo: 274.6   base 354.7        <- 80 grados torcido
no llego a enderezarse en 3 s    <- lo intentó y NO PUDO
derecho, a 80.1 grados           <- y se declaró "derecho" igual
```

Tres cosas de una sola lectura:

1. **El giroscopio funcionaba** — contestaba 274,6, no ceros.
2. **`rumboBase` estaba bien guardado** en 354,7 y no se había reseteado.
3. **El enderezado corría y fallaba**, y encima después mentía.

Se confirmó con una segunda lectura: el error seguía clavado en 80°.

> **Lección:** el robot sabía perfectamente qué le pasaba. Lo único que faltaba era
> preguntarle. Media clase de hipótesis se resolvió con una tecla.

---

## 2. Bug 1 — el enderezado no tenía fuerza para girar

La cuenta que hacía el robot con 80 grados de error:

```
80 × KP_ACOMODO (1,5) = 120   →   recortado a PWM_MAX_ACOMODO = 60
```

Y acá está el detalle: **un motor parado necesita ~70 de PWM para empezar a moverse** (medido
en banco; ya rodando le alcanza ~40). Con 60 las ruedas **zumban y no giran**.

El mínimo estaba peor: `PWM_MIN_ACOMODO = 35`. Con 35 no se movía nunca, así que **los errores
chicos tampoco se corregían jamás**.

**Los dos valores estaban por debajo del piso de arranque del motor. El enderezado nunca
funcionó, desde que se escribió.**

| | Era | Quedó |
|---|---|---|
| `PWM_MIN_ACOMODO` | 35 | **75** |
| `PWM_MAX_ACOMODO` | 60 | 110 → **85** (ver §5) |

---

## 3. Bug 2 — la rampa que no se reiniciaba

En el seguimiento lateral, cuando la pelota queda casi enfrente, el robot hace `parar()`. Pero
no reiniciaba la rampa de arranque suave.

Es como **apagar el motor del auto sin levantar el pie del acelerador**: la variable
`potenciaRampa` se quedaba en 120, y el siguiente arranque salía **a fondo de golpe** en vez de
subir de a poco.

Y eso es exactamente lo que hace **patinar las ruedas** — el hallazgo del 18/08. Como no
patinan igual las dos, cada patinazo **tuerce un poquito al robot**. Pasaba muchas veces por
minuto.

**Arreglo:** `reiniciarRampaMovimiento()` en ese `parar()`.

También se subió `MAX_CORRECCION_RUMBO` de **70 a 100**: con el despeje a potencia 200, 70 era
poca autoridad para pelearle a la torcedura.

---

## 4. Bug 3 — la carrera del giroscopio al encender

Este apareció por una observación del equipo que vale oro:

> *"la primera vez andaba mal, terminaba en cualquier lado, y lo probamos de nuevo y en esta
> prueba el robot anduvo recto"*

**Mismo programa, dos resultados distintos.** Eso no es un error de lógica: es algo que a veces
sale bien y a veces mal.

La causa estaba en una sola línea del arranque:

```c
hayGiroscopo = bno.begin();     // se preguntaba UNA sola vez
```

Cuando se prende la batería, el Teensy arranca en milisegundos pero el **BNO055 tarda casi un
segundo** en poder contestar por I2C. Si el Teensy pregunta antes, recibe "no estoy" — **y no
vuelve a preguntar nunca más**. `hayGiroscopo = false` para toda la corrida, sin corrección de
rumbo en ningún movimiento.

**Arreglo:** insistir hasta 10 veces, separadas por 300 ms.

**Resultado:** se probó **3 veces seguidas cortando la batería entre una y otra, y las 3
anduvieron bien.** La carrera quedó resuelta.

> Y quedó desmentido el comentario que tenía el código: *"sin giroscopio el robot igual sigue y
> despeja, lo único que pierde es mantenerse derecho"*. **Falso.** Sin giroscopio el robot
> termina en cualquier lado.

---

## 5. Después de arreglar eso: oscilaba

Con el mínimo en 75 el robot **por fin giraba**, pero apareció el problema opuesto:

> *"el giro es demasiado rápido, oscila varias veces antes de quedarse quieto y a veces queda
> mal acomodado"*

### La tensión de fondo

```
el motor NO ARRANCA con menos de ~70 de PWM
pero 75 de corrido cuando faltan 4 grados lo pasan de largo
```

**El escalón más chico que sabe dar el motor es más grande que la precisión que le pedimos.**
Es un auto cuyo acelerador recién funciona de media marcha para arriba: no podés avanzar de a
poquito.

### Dos arreglos, a pedido del equipo

1. **`PWM_MAX_ACOMODO` 110 → 85** — menos velocidad, menos inercia.
2. **`frenar()` en vez de `parar()`** al llegar al punto.

El segundo importa más de lo que parece: `parar()` **suelta** los motores, así que el robot
llegaba al punto y **la inercia lo seguía llevando de largo**. Parte del sobrepaso ni siquiera
venía del control. Es el mismo freno eléctrico que se midió el 18/08 para el regreso del
despeje.

---

## 6. 🎯 LA CAUSA DE FONDO: la batería

Después de los tres arreglos, **el problema volvió a aparecer**: seguía sin andar derecho y no
se acomodaba bien.

El equipo pidió entonces algo que resultó ser la clave:

> *"decidimos que pongas una luz que parpadee rápido, cosa de que marque diferencia y nos demos
> cuenta de que el giroscopio no está funcionando"*

Se agregó (ver §7). **En su primera prueba, el LED parpadeó rápido.** O sea: el giroscopio
estaba caído.

Midieron la batería con el profe Gustavo:

| | |
|---|---|
| Batería medida | **7,4 V** |
| Lo normal | **7,8 V** |

Cambiaron la batería y **anduvo bien**.

### ⚠️ Por qué esto todavía NO está confirmado

Lo marcó el propio equipo: *"obviamente pudo ser casualidad"*. Tienen razón — **es una sola
corrida buena después del cambio.**

**La prueba de control que falta, y es barata:**

1. Poner de nuevo la batería de 7,4 V. **Si vuelve el parpadeo rápido, el vínculo está probado.**
2. Volver a la buena: el parpadeo tiene que irse.
3. Con la buena, la tecla `i` tiene que decir *"contestó siempre, no se cayó nunca"*.

> Es la misma regla que ya nos mordió dos veces: **si la respuesta fuera la contraria, ¿este
> test lo mostraría?** Este sí: la batería mala tiene que hacer volver el parpadeo.

### Por qué es tan difícil de ver

El modo de falla es **asimétrico**: con 7,4 V el giroscopio se cae **pero los motores siguen
andando perfectamente**. El robot se mueve, la cámara anda, los sensores de línea andan. Se ve
exactamente igual que un robot sano — solo que sin brújula.

**Se ve idéntico a un bug de código.** Por eso se perdieron clases buscando en el lugar
equivocado, y por eso este día se encontraron tres bugs reales que igual no eran *el* problema.

Y una consecuencia útil: **el giroscopio es lo primero que cae, antes que los motores.** O sea
que sirve de **aviso temprano de batería baja**.

---

## 7. El LED no se ve, y el aviso nuevo

Otra cosa que levantó el equipo, y que invalidaba medio diseño anterior:

> *"el LED es la única forma de avisarnos ciertos eventos y está muy difícil de ver, está debajo
> de la batería y el chasis lo tapa"*

Y peor todavía, sobre los mensajes por consola:

> *"no podemos ver la terminal ni tampoco usarla si está desconectado del USB"*

Es exacto, y es **peor de lo que parece**: el Teensy **descarta** lo que imprime cuando no hay
nadie escuchando. Los `Serial.println` con el USB desenchufado no quedan esperando — se pierden.
O sea que todos los avisos del firmware, que solo ocurren en la cancha, **no los leía nadie
nunca**.

### Lo que se hizo

**1. Aviso por LED.** Parpadeo de **10 por segundo** — el más rápido de todo el programa (el que
le sigue va a 5/s). **Pisa cualquier otra señal.** Se prende si el giroscopio no apareció al
encender o si dejó de contestar en cualquier momento.

⚠️ El LED del pin 13 es de **un solo color**, no puede ser rojo. El código quedó preparado con
`#define PIN_AVISO`: si sueldan un LED rojo a un pin libre (el **9** o el **10**), es cambiar un
número.

**2. El robot se acuerda.** La tecla `i` ahora contesta una de estas tres:

```
giroscopio: contesto siempre, no se cayo nunca
giroscopio: SE CAYO 3 vez/veces durante la corrida
giroscopio: NUNCA APARECIO al encender
```

La cuenta arranca al armarse, no antes — mientras el sensor se despierta contesta ceros un rato,
y eso no es una caída.

---

## 8. Lo que viene: predecir la trayectoria de la pelota

Planteado por el equipo al final de la clase, para trabajar la próxima:

> *"cuando la pelota naranja vaya más rápido el robot tiene que predecir la trayectoria para que
> sea más eficiente. Notamos que si el robot solo usa un eje para seguir la pelota lateralmente,
> cuando la pelota viene rápido diagonalmente el robot despeja pero la pelota a veces pasa de
> largo, porque el robot se movió hacia adelante en un eje que **ya quedó antiguo**."*

**El diagnóstico del equipo es correcto.** El robot apunta a **donde la pelota ESTÁ**, y para
cuando termina de reaccionar la pelota **ya no está ahí**. Todo el control es proporcional a la
posición actual, sin ninguna noción de **velocidad**.

Dónde se pierde el tiempo:

- el despeje dispara con la pelota a 30 cm y después **avanza ~533 ms a ciegas**, sin volver a
  mirar la cámara;
- el seguimiento lateral también es puro proporcional a la posición.

**Dos caminos, y no compiten:**

| | Idea | Comentario |
|---|---|---|
| **A** | **Predecir** | Calcular la velocidad de la pelota con lecturas seguidas y apuntar a donde va a estar |
| **B** | **No ir a ciegas** | Que durante el avance siga corrigiendo de costado. El robot es un omni: **puede avanzar y correrse al mismo tiempo** |

La **B** puede ser más simple y atacar el mismo problema. Conviene evaluar las dos.

⚠️ **Requisito previo:** para hablar de velocidad en cm/s hace falta la conversión del **eje Y**
de la cámara, que **todavía no se midió**. La de distancia sí (2,87 unidades = 1 cm real).

---

## Números de hoy

| Qué | Valor |
|---|---|
| **Batería con el giroscopio caído** | **7,4 V** (lo normal: 7,8 V) |
| Piso de arranque del motor | ~70 de PWM parado, ~40 ya rodando |
| `PWM_MIN_ACOMODO` | 35 → **75** |
| `PWM_MAX_ACOMODO` | 60 → 110 → **85** |
| `MAX_CORRECCION_RUMBO` | 70 → **100** |
| Reintentos de `bno.begin()` | 1 → **10, cada 300 ms** |
| Parpadeo de aviso "sin giroscopio" | **10 por segundo** (el más rápido del programa) |
| Error de rumbo medido en la falla | **80,1 grados** |
| Corridas buenas seguidas tras arreglar la carrera | **3 de 3** |

---

## Qué queda pendiente

### 🔴 Lo primero la próxima clase

- ⬜ **La prueba de control de la batería** (ver §6). Poner la de 7,4 V y ver si vuelve el
  parpadeo. Es lo que convierte "cambiamos algo y anduvo" en "sabemos por qué anduvo".
- ⬜ **Medir la batería antes de cada sesión.** Y si el robot no anda derecho, **descartar la
  batería ANTES de tocar código**. Nos costó buena parte de esta clase.

### Del comportamiento

- ⬜ **Predecir la trayectoria de la pelota** (§8). Es el pedido nuevo del equipo.
- ⬜ **Probar el enderezado con freno y techo 85.** Se cargó pero no se llegó a evaluar limpio,
  porque la batería tapaba todo.
- ⬜ **Los "empujoncitos"**: si todavía oscila, la idea que salió del análisis es controlar el
  **tiempo** en vez de la fuerza — un empujón de 85 que dura 60 ms arranca seguro y mueve
  poquito. Y leer el giroscopio **solo con el robot quieto y frenado**, porque mirándolo mientras
  gira contesta dónde *estaba*.
- ⬜ **Achicar el avance del despeje.** Sigue saliendo ~50 cm para una pelota que está a 30.
- ⬜ **Volver al centro al perder la pelota**, con el arco azul. Ya está toda la maquinaria.

### De la cámara

- ⬜ **Medir la conversión del eje Y.** Bloquea lo de la predicción.
- ⬜ 🚨 **Zona muerta justo adelante:** `Xp = 0` significa a la vez "no la veo" y "la tengo
  encima".

### De hardware

- ⬜ **Un LED que se vea.** Pin 9 o 10 libres. Hoy el aviso está en un LED tapado por la batería.
- ⬜ Capacitor de reserva junto al giroscopio (viene del 04/08).

---

## Lo que se aprendió del método

**1. El robot sabía. Nadie le preguntaba.**
Una tecla resolvió lo que media clase de hipótesis no había podido. Todo lo que el robot mide y
no cuenta es trabajo perdido.

**2. Arreglar bugs reales no garantiza arreglar EL problema.**
Se encontraron tres bugs de verdad —el enderezado sin fuerza, la rampa, la carrera del
arranque— y los tres había que arreglarlos. Pero ninguno era la causa de fondo. **Un arreglo
que no cambia el síntoma no significa que estaba bien: significa que además hay otra cosa.**

**3. Dos corridas que dan distinto son un dato, no mala suerte.**
*"La primera vez mal, la segunda bien"* fue lo que destapó la carrera del arranque. Mismo
programa + resultados distintos = hay algo que a veces pasa y a veces no. **Vale la pena
contarlo siempre, aunque parezca que no viene al caso.**

**4. Desconfiar de la propia solución.**
*"Obviamente pudo ser casualidad"* es la frase más valiosa del día. Cambiar algo y que ande no
prueba por qué anda. La prueba de control quedó anotada como pendiente y hay que hacerla.
