---
name: firmware-teensy-maquina-de-estados
description: Cómo se escribe el firmware de competencia de los robots de fútbol IITA 2025 (Teensy 4.1 + placa Zircon Rev v15, C++ estilo Arduino, archivo .ino, Arduino IDE). Usar SIEMPRE que se vaya a escribir, leer, modificar o pedirle a una IA código de `arquero.ino` o `delantero.ino`, o cuando aparezcan "setup", "loop", "delay", "millis", "millis_inicio_estado", "millis_pelota", "máquina de estados", "FSM", "enum", "switch", "case", "break", "estado", "timeout", "watchdog", "ROBOT1", "ROBOT2", "#define", "Zircon", "Teensy", "código muerto", "no compila", "multiple definition of bno", "Serial.print", "monitor serie", "monitor serial", "19200", "cuántas veces por segundo corre el loop", "el robot se queda trabado", "no sale de ese estado", "anda la primera vez y después no", "el robot no enciende", "se va de la cancha", "toqué un parámetro y no cambió nada", "por qué no puedo usar wait", "esto en Pybricks lo hacía con wait()". Es la skill que evita que una IA (o un chico que viene de Python) escriba código BLOQUEANTE o estilo Pybricks en un robot que no lo tolera. NO usar para calibrar la cámara OpenMV ni el protocolo de 9 bytes (eso va en la skill `openmv-h7-vision-y-enlace-serie`), NO para el giroscopio BNO055 y el rumbo (eso va en `bno055-rumbo-teensy` y en `skills/guia-giroscopio.md`), NO para el reparto de potencia entre los 3 motores omni (eso va en `omni-3-ruedas-teensy`), NO para armar la prueba de banco en sí (eso va en `pruebas-en-banco-robot-real`).
---

# Firmware de competencia en Arduino/Teensy: máquinas de estado sin `delay()`

Esta skill es sobre **cómo se escribe** el programa del robot. No sobre qué tiene que hacer el
robot — sobre la forma del código. Si te salteás esto, la IA te va a escribir algo que parece
razonable, compila, y **hace que el robot se quede duro en la cancha**.

Todo lo que dice acá está verificado contra los archivos reales:
`arquero/arquero.ino` (1207 líneas), `delantero/delantero.ino` (1214 líneas),
`libreria-zircon/zirconLib.cpp` (355 líneas),
`vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py` (156 líneas).
Cada número lleva `archivo:línea`. Si algo no tiene cita, es porque **no está verificado** y
está marcado como tal.

> 🔴 **Antes de nada: la copia del repo hoy NO COMPILA.** Hay una llave `}` de más al final de
> `zirconLib.cpp:355` y la variable `bno` está definida dos veces (`zirconLib.cpp:4` y
> `arquero.ino:68` / `delantero.ino:76`). Los dos están verificados en el código. Si abrís el
> Arduino IDE y te tira `expected declaration before '}' token` o `multiple definition of 'bno'`,
> no rompiste nada: ya estaba. El parche está en `correcciones-propuestas.md`, "PARCHE 0".

---

## 1. El salto mental: venís de Pybricks y eso te va a jugar en contra

En Pybricks escribís así:

```python
robot.straight(300)      # avanza 300 mm — la función NO vuelve hasta terminar
wait(700)                # el programa se queda acá 700 ms
robot.turn(90)
```

Esas tres líneas son **bloqueantes**: cada una toma el control y no lo devuelve hasta que
terminó. En Pybricks eso está bien porque el hub tiene un sistema abajo que sigue atendiendo
otras cosas mientras vos esperás, y porque un robot de línea puede darse el lujo de frenar y
pensar.

En este robot **no hay nada abajo**. No hay sistema operativo, no hay tareas, no hay RTOS.
Lo único que corre es tu `loop()`, una vuelta atrás de otra, para siempre. Si tu código se
queda esperando, **el robot entero se queda esperando**: no lee la cámara, no lee la línea, no
lee el giroscopio, no apaga los motores. Los motores siguen girando con el último valor que les
pusiste.

> **La regla de una línea:** en Pybricks las funciones *hacen y esperan*. Acá las funciones
> *prenden y vuelven enseguida*, y **quién decide cuándo apagar es el ciclo siguiente**.

Mirá `parar()` en `arquero.ino:146-150`: escribe 0 en los tres PWM y vuelve. No espera nada.
Mirá `avanzar()` en `arquero.ino:151-155`: prende motores y vuelve. **No avanza una distancia:
deja los motores prendidos.** Quien los apaga es otro estado, en otra vuelta del loop.

Y esto no es teoría: **buscando la palabra `delay` en los dos `.ino` (2421 líneas) no aparece ni
una vez.** En la librería aparece una sola vez, en `zirconLib.cpp:80`, y está adentro de un
bloque comentado (`/** ... **/`, líneas 62-92) que el compilador ni mira. El equipo 2025 ya
había aprendido esto.

---

## 2. `setup()` y `loop()`: las dos únicas funciones que el Teensy llama solo

Un programa de Arduino tiene exactamente esta forma:

```cpp
void setup() {
  // corre UNA SOLA VEZ, al encender o al apretar reset
}

void loop() {
  // corre para siempre, una vuelta atrás de la otra, sin pausa
}
```

En el arquero, `setup()` está en `arquero.ino:236-256` y hace esto:

| Línea | Qué hace |
|---|---|
| `arquero.ino:237` | `InitializeZircon()` — configura todos los pines (`zirconLib.cpp:40-50`) |
| `arquero.ino:238-239` | abre el monitor USB y el `Serial1` de la cámara, los dos a `BAUD_RATE` = 19200 (`arquero.ino:77`) |
| `arquero.ino:244` | `pinMode(LED_BUILTIN, OUTPUT)` |
| `arquero.ino:246-250` | arranca el BNO055 por I2C (con el `while (1);` de la 248 adentro) |
| `arquero.ino:252-255` | lee `initialYaw`, el rumbo de referencia, y arranca el cronómetro |

`loop()` empieza en `arquero.ino:259` y termina en `arquero.ino:1207`. **Todo el robot vive
adentro de esa función.** Su estructura, en orden:

1. `arquero.ino:263-329` — leer la cámara por `Serial1` (si hay 9 bytes esperando)
2. `arquero.ino:334-340` — leer el giroscopio y calcular `error`
3. `arquero.ino:344-346` — leer los tres sensores de línea (`readLine(1/2/3)`)
4. `arquero.ino:356-1206` — **el `switch(estado)`**: según en qué estado esté, hacer una cosa

Ese orden importa: **primero se lee todo, después se decide.** Todos los `case` del switch
usan las mismas variables (`haypelota`, `Xp`, `Yp`, `error`, `s1`, `s2`, `s3`) leídas arriba,
en esa misma vuelta. Si metés una lectura de sensor adentro de un `case`, rompés esa garantía:
distintos estados van a estar decidiendo con datos de momentos distintos.

Un detalle que se pasa por alto: `arquero.ino:263` es un `if`, no un `while`. O sea, **el loop
consume como máximo UNA trama de cámara por vuelta**, aunque en el buffer haya cinco esperando.

### ¿Cuántas veces por segundo corre?

Honestamente: **no lo sabemos, y hay que medirlo.** Nadie midió el período del loop de este
robot. Lo que sí sabemos leyendo el código es que cada vuelta hace `bno.getEvent(&event)`
(`arquero.ino:335`), que es una lectura por I2C — o sea, un intercambio con un chip externo que
**tarda** y durante el cual el programa espera. Ese es el sospechoso número uno de lo que fija
el ritmo del loop, pero es una **hipótesis, no un dato**.

Medilo así (esto es un agregado de diagnóstico, se saca después). **Propuesto, no compilado ni
probado:**

```cpp
// --- MEDIR EL PERIODO DEL LOOP ---
// Pegar como PRIMERAS lineas de loop(), antes de leer la camara.
static unsigned long vueltas = 0;
static unsigned long t_ultimo_reporte = 0;
vueltas++;
if (millis() - t_ultimo_reporte >= 1000) {
  Serial.print("vueltas por segundo = "); Serial.println(vueltas);
  vueltas = 0;
  t_ultimo_reporte = millis();
}
```

Anotá el número **con** la lectura del BNO y **comentando** la línea `arquero.ino:335`.
La diferencia entre los dos números es lo que cuesta el giroscopio. Ese es el método:
**no adivines cuánto cuesta algo, medí con y sin.**

Por qué importa el número: la cámara manda una trama por vuelta de su propio bucle
(`enviar_coordenadas_2_arcos_y_pelota.py:155`). Si el Teensy da menos vueltas por segundo que
la cámara tramas por segundo, el buffer de recepción de `Serial1` se llena de tramas viejas y el
robot decide con la pelota de hace varios cuadros. Eso es exactamente el bug **DEL-06** de
`bugs-conocidos.md`. (Ese doc estima el buffer en 64 bytes = 7 tramas; **ese tamaño no lo
verificamos en el código del core de Teensy** — tomalo como orden de magnitud, no como dato.)

---

## 3. Por qué `delay()` está prohibido adentro del lazo

`delay(700)` en un Arduino hace literalmente esto: se queda contando hasta que pasaron 700 ms
y recién entonces devuelve el control. Durante esos 700 ms:

- no se leen los 9 bytes de la cámara → si llegan varias tramas, el buffer se llena
- no se leen los sensores de línea → **el robot puede cruzar la línea entera y salir de la
  cancha, y el código ni se entera**
- no se lee el giroscopio → `error` queda viejo
- no se toca ningún motor → **siguen girando con lo último que les pusiste**

Ese último punto es el que mata. Mirá `PATEANDO_atras_arquero` (`arquero.ino:1184-1195`): pone
PWM 150 en dos motores hacia atrás y sale del `case`. Si en vez de eso alguien hubiera escrito
`delay(1500)` para "retroceder un segundo y medio", durante ese segundo y medio el robot
retrocede **a ciegas**, sin poder frenar ante nada.

¿Cuánta cancha recorre el robot en 700 ms de ceguera? **No lo sabemos: la velocidad real de
estos robots no está medida en el repo.** Medila con cinta métrica y cronómetro (marcá una
distancia, cronometrá, dividí) — es un ejercicio de 10 minutos y te da el número que convierte
"700 ms" en centímetros.

### La traducción Pybricks → acá

| Lo que hacías en Pybricks | Acá se escribe |
|---|---|
| `wait(700)` | guardar `millis()` al entrar al estado y comparar cada vuelta |
| `robot.straight(300)` | prender motores en un estado + salir por sensor o por tiempo |
| `while sensor.reflection() > 50: wait(10)` | un estado que chequea el sensor una vez por vuelta |
| `robot.turn(90)` | prender giro + salir cuando el giroscopio dice que llegaste |

**Ningún `while` que espera algo.** Un `while (1);` vacío es la versión extrema del problema, y
está en el código real: `arquero.ino:248` (y `delantero.ino:272`). Si el BNO055 no contesta al
encender, el robot entra ahí y **no sale nunca**: `setup()` no termina, `loop()` no arranca, los
motores nunca se prenden. En la cancha se ve como "el robot no enciende". Es el bug **A5** de
`bugs-conocidos.md`.

> Excepción única: esperar en `setup()`, antes de que empiece el partido, es aceptable —
> ahí nadie depende de vos todavía. El parche propuesto para A5
> (`correcciones-propuestas.md:163-188`) usa un `while` **acotado a 1000 ms** con `delay(50)`
> adentro, y está en `setup()` justamente por eso. **Adentro de `loop()`, nunca.**

---

## 4. El patrón `millis()`: cómo se espera sin esperar

`millis()` devuelve cuántos milisegundos pasaron desde que se encendió el Teensy. Es un número
que sube solo. **No espera nada**: preguntarle la hora es instantáneo.

La idea: en vez de *"esperá 700 ms"*, escribís *"¿ya pasaron 700 ms desde que entré acá?"*.
Y esa pregunta se hace **una vez por vuelta del loop**, sin bloquear a nadie.

Las tres piezas:

```cpp
unsigned long millis_inicio_estado = millis();   // arquero.ino:133 — la marca
```

```cpp
// AL ENTRAR a un estado: guardar la marca de tiempo
millis_inicio_estado = millis();                 // arquero.ino:1156
estado = PATEANDO_adelante_arquero;              // arquero.ino:1157
```

```cpp
// ADENTRO del estado: comparar contra la marca
if (millis() - millis_inicio_estado >= 450)      // arquero.ino:1165
{
  parar();
  millis_inicio_estado = millis();
  estado = PATEANDO_pausa_arquero;
}
```

El estado `PATEANDO_adelante_arquero` completo (`arquero.ino:1162-1172`) es el ejemplo canónico
y son 10 líneas:

```cpp
    case PATEANDO_adelante_arquero:

      avanzar_patear();                                    // prende motores y vuelve
      if (millis() - millis_inicio_estado >= 450)          // ¿ya pasaron 450 ms?
      {
        parar();
        millis_inicio_estado = millis();                   // reiniciar el cronometro
        estado = PATEANDO_pausa_arquero;                   // pasar al siguiente
      }

    break;
```

Leelo así: *"mientras esté en este estado, prendo los motores de patada. Cada vuelta pregunto
si ya pasaron 450 ms. Cuando pasan, paro, pongo el reloj en cero y me voy al siguiente estado."*
Entre vuelta y vuelta, el robot sigue leyendo cámara, línea y giroscopio. **Eso es lo que
`delay(450)` te robaría.**

### Por qué siempre se escribe `millis() - marca >= X` y nunca `millis() >= marca + X`

`millis()` devuelve un `unsigned long`, que después de unos 49 días vuelve a cero. Con la resta,
la cuenta sigue dando bien igual (es aritmética que "da la vuelta"). Con la suma, no. Un partido
no dura 49 días, así que en la práctica da igual — pero **escribilo siempre con la resta** para
que el hábito quede. Todos los timeouts del código real están escritos con la resta.

### Los tres relojes que existen en este código

| Variable | Dónde se declara | Qué mide |
|---|---|---|
| `millis_inicio_estado` | `arquero.ino:133` / `delantero.ino:140` | hace cuánto entré al estado actual |
| `millis_inicio_centrando` | `arquero.ino:134` / `delantero.ino:141` | hace cuánto arrancó la maniobra de centrado. En `arquero.ino` aparece una sola vez más (`:549`) y **está en código muerto**; el que la usa de verdad es el delantero (`delantero.ino:569, 623, 628, 630, 650, 694, 699, 701, 721, 792`) |
| `millis_pelota` | `arquero.ino:135` / `delantero.ino:142` | **cuándo fue la última vez que la cámara reportó pelota** — se pisa en `arquero.ino:305` |

Ese tercero es un watchdog (sección 9). Guardá el nombre.

---

## 5. Qué es una máquina de estados y por qué este robot es una

Una **máquina de estados** es una forma de organizar un programa donde:

1. En cada momento el robot está en **exactamente un** estado (una situación, un modo).
2. Cada estado define **qué hacer** mientras estás ahí.
3. Cada estado define **bajo qué condiciones te vas** a otro estado.

Eso es todo. No es un concepto avanzado: es la forma natural de describir un robot que hace
cosas distintas según la situación. "Estoy barriendo a la derecha", "estoy pateando", "estoy
volviendo al arco" son estados.

En C++ se escribe con dos herramientas: un `enum` que lista los estados y un `switch` que elige
qué hacer.

### El `enum`: la lista de estados posibles

El `enum` del arquero ocupa `arquero.ino:114-130` (transcripto acá con saltos de línea distintos
para que se lea; en el archivo hay nombres que comparten renglón):

```cpp
enum Estado {
  // --- ARQUERO ---
  impulso_inicial,
  moverce_izquierda, moverce_derecha,
  impulso_izquierda, impulso_derecha,
  PATEANDO_pausa_inicial_arquero, PATEANDO_adelante_arquero, PATEANDO_atras_arquero,
  PATEANDO_pausa_arquero, avanzar_despues_de_patear,

  // --- DELANTERO ---
  AVANCE_INICIO, PRIMER_IMPULSO_INICIAL_GIRANDO,
  IMPULSO_INICIAL_GIRANDO, GIRANDO,
  APUNTAR_PELOTA, APUNTAR_PELOTA_antihorario, AVANZANDO,
  CENTRANDO_horario, IMPULSO_CENTRANDO_antihorario, CENTRANDO_antihorario,
  IMPULSO_CENTRANDO_horario, CENTRANDO_giroscopo,
  PATEANDO_corto_pausa_inicial, PATEANDO_corto_adelante, PATEANDO_corto_pausa,
  PATEANDO_corto_atras,
  PATEANDO_pausa_inicial, PATEANDO_adelante, PATEANDO_pausa, PATEANDO_atras,
  AVANZANDO_POR_TIEMPO,
  DETECTA_LINEA_1, DETECTA_LINEA_2, DETECTA_LINEA_3
};                                               // arquero.ino:130
Estado estado = impulso_inicial;                 // arquero.ino:131 — el estado INICIAL
```

Un `enum` es una lista de nombres. El compilador les da números por atrás (0, 1, 2...), pero
vos nunca los usás: escribís el nombre. La ventaja sobre usar `int estado = 3;` es que el
compilador te avisa si escribís mal un nombre, y que el código se lee.

Ese `enum` tiene **34 nombres**, y en el `switch` hay **32 `case`**. Los dos que no tienen `case`
en `arquero.ino` son `PRIMER_IMPULSO_INICIAL_GIRANDO` y `CENTRANDO_giroscopo`. Ojo con eso:
si el robot alguna vez llegara a un estado sin `case`, el `switch` **no haría absolutamente
nada** esa vuelta — los motores se quedarían con el último valor y no habría forma de salir.
(Hoy ninguno de los dos se asigna nunca, así que no puede pasar; pero es el modo de falla que
te espera si agregás un estado al `enum` y te olvidás del `case`.)

**Fijate en `arquero.ino:131`: `Estado estado = impulso_inicial;`.** Esa línea decide con qué
estado arranca el robot. En el delantero la misma línea es `delantero.ino:138` y dice
`Estado estado = AVANCE_INICIO;`. Cambiar esa sola línea cambia por dónde entra el robot a la
máquina — y, como vas a ver en la sección 6, cambia **qué mitad del programa existe**.

### El `switch`: elegir qué hacer

```cpp
switch (estado)                 // arquero.ino:356
{
  case moverce_derecha:
    adproporcional();           // lo que hago mientras estoy en este estado
    // ... condiciones de salida ...
  break;                        // el break es OBLIGATORIO

  case moverce_izquierda:
    aiproporcional();
    // ...
  break;
}
```

**El `break;` no es decorativo.** Si te lo olvidás, C++ sigue ejecutando el `case` siguiente
(se llama *fall-through*). El robot haría dos estados a la vez. Es un error silencioso: compila
perfecto. **Cada `case` termina en `break;` — chequealo cada vez que agregues uno.**

### Cómo se lee el grafo de estados de un archivo (a mano, sin herramientas)

Un `.ino` de 1200 líneas no te muestra el grafo. Lo armás vos, y lleva 20 minutos:

1. Buscá todas las líneas que asignan un estado, con la forma `estado = ALGO;`. En
   `arquero.ino` hay **69** (sin contar la declaración de la 131). Cada una es una **flecha**
   del grafo. En `delantero.ino` hay 64.
   ⚠️ Si buscás el texto `estado = ` a lo bruto vas a contar 196 líneas: la mayoría son
   `millis_inicio_estado = millis()`, que **no** cambian de estado. Buscá bien.
2. Para cada una, anotá: *desde qué `case` está* → *a qué estado va*.
3. Empezá por el estado inicial (`arquero.ino:131`, `impulso_inicial`) y seguí las flechas.
   Todo lo que alcanzás es **código vivo**. Lo que no alcanzás es **código muerto**.

Lo hice para el bloque del arquero (`case impulso_inicial` empieza en `arquero.ino:1016`, el
switch cierra en `arquero.ino:1206`). Hay **16 asignaciones de estado** ahí adentro:
líneas 1024, 1041, 1051, 1056, 1073, 1089, 1099, 1104, 1120, 1132, 1144, 1157, 1169, 1179,
1192 y 1202. Sus destinos son sólo estos diez:

```
impulso_inicial ──(40 ms, L1022)──> moverce_derecha
moverce_derecha ──(pelota cerca y centrada, L1041)──> PATEANDO_pausa_inicial_arquero
moverce_derecha ──(|Yp|>=5, L1051/1056)──> moverce_derecha / moverce_izquierda
moverce_derecha ──(ve blanco, L1073)──> impulso_izquierda
moverce_izquierda ──(idem, L1089/1099/1104/1120)──> ...
impulso_derecha ──(350 ms, L1130)──> moverce_derecha
impulso_izquierda ──(350 ms, L1142)──> moverce_izquierda
PATEANDO_pausa_inicial_arquero ──(200 ms, L1154)──> PATEANDO_adelante_arquero
PATEANDO_adelante_arquero ──(450 ms, L1165)──> PATEANDO_pausa_arquero
PATEANDO_pausa_arquero ──(1000 ms, L1177)──> PATEANDO_atras_arquero
PATEANDO_atras_arquero ──(ve blanco, L1190)──> avanzar_despues_de_patear
avanzar_despues_de_patear ──(1000 ms, L1200)──> moverce_derecha
```

**Es un conjunto cerrado de 10 estados.** Ninguna flecha sale hacia `GIRANDO`, `APUNTAR_PELOTA`,
`CENTRANDO_horario` ni ninguno de los otros 24 nombres del `enum`. Con `#define ROBOT1`
(`arquero.ino:10`), **el bloque del delantero — líneas 358 a 1012, o sea 655 líneas — es código
muerto**: está compilado, ocupa memoria, pero nunca se ejecuta.

> ⚠️ **Inconsistencia que hay que registrar:** `bugs-conocidos.md` dice "13 asignaciones de
> estado del bloque del arquero". Contándolas una por una en el archivo me dan **16**. La
> conclusión (conjunto cerrado, el bloque del delantero es inalcanzable) es la misma, pero el
> número del doc está mal. **El código manda.** Si contás vos y te da otra cosa, decilo.

Cómo se **confirma** que algo es código muerto (no alcanza con el razonamiento — el
razonamiento puede tener un agujero): metés un `Serial.println("ENTRE A X");` como primera
línea del `case` sospechoso, jugás tres minutos completos y mirás el monitor. Si el mensaje
nunca aparece, está muerto. Si aparece una sola vez, tu análisis estaba mal. Ese test está
escrito con criterio de aceptación en `correcciones-propuestas.md`, sección A6.

**Por qué te importa:** si le pedís a una IA "arreglá el bug de la línea 606" y esa línea está
en código muerto, vas a "arreglar" algo y el robot no va a cambiar en nada. Vas a pensar que el
parche no sirve. Ese es exactamente el caso del bug **A6**.

---

## 6. Los dos `.ino` son parientes, no gemelos

Esto no es obvio y confunde a todo el mundo la primera vez.

`arquero.ino` y `delantero.ino` **contienen los dos las dos máquinas de estados**: la del
arquero y la del delantero, en el mismo `switch` (los dos archivos tienen 32 `case`). Cuál de
las dos corre lo deciden dos líneas:

| | arquero.ino | delantero.ino |
|---|---|---|
| El `#define` | `#define ROBOT1` (L10), `//#define ROBOT2` (L11) | `//#define ROBOT1` (L10), `#define ROBOT2` (L11) |
| El estado inicial | `Estado estado = impulso_inicial;` (L131) | `Estado estado = AVANCE_INICIO;` (L138) |

El `#define` elige el bloque de `#if defined(...)` de arriba, que asigna **pines de motor
distintos**: en el arquero el Motor 1 está en los pines 2/5/3 (`arquero.ino:38-40`), en el
delantero el Motor 1 está en 8/7/6 (`delantero.ino:14-16`). Los robots tienen los motores
cableados distinto — está documentado en `mapa-pines-teensy.md`.

El estado inicial elige **por qué mitad del switch entra el robot**, y como cada mitad es un
conjunto cerrado, la otra queda inalcanzable.

### Pero NO son el mismo archivo con dos líneas cambiadas

Esto es importante y es fácil de creer mal. Los dos programas **divergieron**: son dos
descendientes de un mismo firmware, editados por separado. Verificado:

| Qué | `arquero.ino` | `delantero.ino` |
|---|---|---|
| Nombres del `enum` | tiene `avanzar_despues_de_patear` (L119) | **no lo tiene**; tiene `APUNTAR_PELOTA_horario` (L132), que el arquero **no tiene** |
| `case` exclusivos | `case avanzar_despues_de_patear` (L1197) | `case APUNTAR_PELOTA_horario` (L800) |
| `tolerancia_cercania` | `140.0` (L110) | `50.0` (L119) — **no está bajo `#define`**, es una constante suelta |
| Bloque `ROBOT1` (el del arquero) | `blanco` 500/650/600 (L50-52), `patadM2 150` (L54) | `blanco` 600/600/600 (L50-52), `patadM2 170` (L54) |
| Bloque `ROBOT2` (el del delantero) | `patadM2 200` (L30) | `patadM2 170` (L30) |
| Banda muerta del giroscopio en `ai/adproporcional` | `error > -1 && error < 1` (L188, L212) | `error > -2 && error < 2` (L212, L236) |
| Rampa de patada | no tiene: `avanzar_patear()` es fijo (L174-178) | rampa por `millis()` (L181-201) + variables (L69-73) |
| `Ycontrincante` | no existe | `delantero.ino:66` |

**Consecuencia práctica número 1:** dar vuelta el `#define` de `delantero.ino` **no te da el
firmware del arquero**. Te da el bloque del arquero con los umbrales de blanco equivocados
(600/600/600 en vez de 500/650/600) y otra `tolerancia_cercania`. Si querés el arquero, subí
`arquero.ino`.

**Consecuencia práctica número 2:** cuando le pidas algo a una IA, decile explícitamente cuál de
los dos archivos y cuál `#define` está activo. Si no, te va a "mejorar" código que tu robot no
ejecuta, o te va a citar una línea del archivo equivocado.

---

## 7. LA REGLA ESTRUCTURAL: sin `else`, y la última asignación gana

Esta es la parte más difícil del código y la que más los va a morder. Leela dos veces.

En casi todos los `case` de este programa, las condiciones de salida están escritas como `if`
**sueltos, uno atrás del otro, sin `else`**. Eso significa que **todos se evalúan siempre**, en
la misma vuelta, aunque uno anterior ya haya cambiado de estado.

Y como `estado` es una variable normal, si tres `if` distintos le asignan valores distintos en
la misma vuelta, **el que queda es el último que escribió**. Los anteriores se pisan y no dejan
rastro.

### El ejemplo caminado: `moverce_derecha`

`arquero.ino:1030-1076`. Lo escribo tal cual está, sacando comentarios:

```cpp
    case moverce_derecha:
      adproporcional();                                    // L1031: prende motores

      if (haypelota)                                       // L1036
      {
        if( (Xp <= tolerancia_cercania) && (abs(Yp) <= 3) ) // L1038
        {
          parar();
          estado = PATEANDO_pausa_inicial_arquero;         // L1041  ← asignación A
          millis_inicio_estado = millis();
        }

        if (abs(Yp) >= 5)                                  // L1045  ← if SUELTO, no else if
        {
          pd = 1.5;
          if( Yp < 0 ) { estado = moverce_derecha;   ... }  // L1051  ← asignación B
          else         { estado = moverce_izquierda; ... }  // L1056  ← asignación B
        }
        else { parar(); }                                  // L1060-1061
      }
      else { pd = 1; }                                     // L1063-1066

      if (s1 >= blanco1 or s2 >= blanco2)                  // L1070  ← if SUELTO otra vez
      {
        parar();
        estado = impulso_izquierda;                        // L1073  ← asignación C
        millis_inicio_estado = millis();
      }
    break;
```

Ahora caminá una vuelta con estos datos: **la cámara ve la pelota cerca y centrada
(`Xp` = 120, `Yp` = 2), y al mismo tiempo el sensor `s1` está sobre la línea blanca.**

1. Línea 1031 → prende los motores del barrido a la derecha.
2. Línea 1036 → `haypelota` es true, entra.
3. Línea 1038 → `Xp` (120) `<= tolerancia_cercania` (140, `arquero.ino:110`) ✔ y `abs(Yp)` (2)
   `<= 3` ✔ → **`estado = PATEANDO_pausa_inicial_arquero`**. Va a patear.
4. Línea 1045 → `abs(Yp)` (2) `>= 5` ✘ → cae en el `else` de la 1060 → `parar()`. **Ojo: apaga
   los motores que la 1031 acababa de prender.** No cambia `estado`.
5. Línea 1070 → `s1 >= blanco1` ✔ → **`estado = impulso_izquierda`**. Pisa lo de la 1041.
6. `break`. Sale del switch.

**Resultado: `estado` vale `impulso_izquierda`. La patada nunca ocurrió.** El paso 3 se
ejecutó, corrió, y su efecto fue borrado 30 líneas después por el paso 5.

En la cancha esto se ve como *"a veces no patea aunque esté perfectamente alineado"*. Y no es un
bug: es la prioridad implícita del código. **Lo que está más abajo en el `case` gana.** El
equipo 2025 puso la línea blanca abajo de todo, o sea que en la práctica dijo *"salir de la
línea manda sobre cualquier otra cosa"*. Es una decisión defendible. Lo que está mal es que
**no está escrita en ningún lado**: hay que descubrirla leyendo el orden.

### Las tres consecuencias que tenés que tener en la cabeza

**(a) Un bug evidente puede no manifestarse nunca.** El bug A3 (`bugs-conocidos.md`, líneas
`arquero.ino:1038` y `:1086`) dice que con `Yp = ±4` el arquero se planta: no patea (necesita
`<=3`) y no se mueve (necesita `>=5`), cae en el `else { parar(); }` de la 1060. Es real. Pero
si en esa misma vuelta un sensor lee blanco, la línea 1073 lo saca igual y el síntoma no
aparece. **Que no lo veas no significa que no esté.**

**(b) Un cambio inocente puede romper algo lejos.** Si agregás un `if` nuevo al final de un
`case` — abajo de todo, "porque es donde hay lugar" — le acabás de dar **máxima prioridad**
sobre todas las salidas que ya estaban. Sin querer.

**(c) El orden de los `if` es la especificación.** No hay otra. Si movés un bloque de lugar
"para que quede más ordenado", **cambiaste el comportamiento del robot**. Reordenar no es
cosmético en este código.

### Cómo se convive con esto

No lo reescribas. El código ganó un Nacional y una reescritura grande te deja sin línea base
para comparar. Lo que sí se hace:

1. **Antes de agregar un `if` a un `case`, preguntate dónde va.** Arriba = baja prioridad.
   Abajo = pisa a todos. Elegí a propósito y **escribí por qué en un comentario**.
2. **Cuando un `if` deba ser exclusivo con el anterior, poné `else if` y decilo.** El delantero
   ya lo hace en un lugar: `delantero.ino:628` es un `else if` deliberado respecto del `if` de
   la 621.
3. **Para depurar, imprimí el estado UNA VEZ POR VUELTA, después del switch** (sección 10). Así
   ves al ganador, no a los perdedores.

---

## 8. Los tres errores clásicos de máquina de estados

Los tres están en este robot, con nombre y apellido en `bugs-conocidos.md`. No son ejemplos
inventados.

### (a) Estado sin timeout de escape → el robot se queda ahí para siempre

**El caso real: `PATEANDO_atras_arquero`** — bug A2, severidad P0.

`arquero.ino:1184-1195` completo:

```cpp
    case PATEANDO_atras_arquero:
    // va atras hasta que detecta blanco
      analogWrite(PWM1, 150); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
      analogWrite(PWM2, 150); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
      analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);

      if ((s1 >= blanco1) or (s2 >= blanco2) or (s3 >= blanco3))   // L1190
      {
        estado = avanzar_despues_de_patear;
        millis_inicio_estado = millis();
      }
    break;
```

**Una sola puerta de salida: que un sensor vea blanco.** No hay reloj. Si la línea no aparece
—sensor sucio, umbral mal calibrado para la luz de ese gimnasio, cable flojo, o el robot va a
chocar la pared antes— el estado **no termina nunca** y los dos motores siguen a PWM 150 hacia
atrás hasta que se corta la batería.

Compará con `PATEANDO_adelante_arquero` (`arquero.ino:1162-1172`): sale por tiempo, 450 ms. Ese
no puede colgarse. La diferencia entre los dos estados es el reloj.

**La regla:** *todo estado necesita al menos una salida que dependa sólo del tiempo.* Si tu
única condición de salida depende de un sensor, el modo de falla del sensor **es** el modo de
falla del estado.

El parche propuesto está en `correcciones-propuestas.md` (A2), y trae dos cosas que valen más
que el código: **el timeout hay que medirlo**, no inventarlo (cronometrar cuánto tarda el
retroceso normal y poner ese valor + 500 ms mínimo); y si el timeout empieza a saltar seguido
en cancha, **el mensaje no es "el parche sirvió", es "el sensor de línea está sucio"**.

### (b) Variable acumulada que no se resetea al entrar al estado → sube sola y no baja nunca

**El caso real: `velocidadActualPateo`** — bug DEL-04.

`delantero.ino:69-73` declara las variables de la rampa de patada, y `delantero.ino:181-201` la
implementa:

```cpp
int velocidadActualPateo = 0;      // L69  ← GLOBAL: sobrevive entre estados
int velocidadFinalPateo = 240;     // L70
int pasoPateo = 5;                 // L71
unsigned long tiempoAnteriorPateo = 0;   // L72
int intervaloPateo = 20;           // L73

void avanzar_patear() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnteriorPateo >= intervaloPateo)   // L185
  {
    tiempoAnteriorPateo = tiempoActual;
    if (velocidadActualPateo < velocidadFinalPateo)           // L189
    {
      velocidadActualPateo += pasoPateo;                      // L191  ← SUBE
      if (velocidadActualPateo > velocidadFinalPateo)
        {velocidadActualPateo = velocidadFinalPateo;}
    }
    analogWrite(PWM1, velocidadActualPateo); ...              // L197-199
  }
}
```

La intención se lee: arrancar suave y subir de a 5 cada 20 ms hasta 240, para no pegarle un
tirón de corriente a la batería. **Pero en todo el archivo `velocidadActualPateo` sólo sube.**
Es global (`delantero.ino:69`), o sea que conserva su valor de una patada a la otra, y nadie la
pone en cero al entrar a un estado de patada.

Seguí la cuenta: el delantero arranca en `AVANCE_INICIO` (`delantero.ino:138`), que llama a
`avanzar_patear()` durante 700 ms (`delantero.ino:384-385`). Si el loop diera al menos una
vuelta cada 20 ms, en 700 ms serían 35 incrementos = PWM 175. **Ojo: ese "175" depende del
ritmo del loop, que no está medido** (sección 2) — si el loop fuera más lento que 20 ms,
sube menos. Lo que **no** depende de nada es lo otro: sube y no baja. Según el código, la rampa
suave sólo puede ocurrir la primera vez en la vida del robot; las siguientes patadas arrancan
con la variable ya arriba. **Propuesto como lectura de código — falta confirmarlo en banco**
imprimiendo `velocidadActualPateo` al entrar a cada patada.

El costo escondido: si probás a cambiar `pasoPateo` o `intervaloPateo` para tunear la patada,
**es esperable que no veas ningún efecto**, porque la variable ya está arriba. Vas a pensar que
el robot no responde.

**La regla:** *toda variable que se acumula tiene que resetearse en la ENTRADA al estado, no en
la salida.* La salida puede no ejecutarse nunca (por la regla del punto anterior, o porque otro
`if` te pisó el estado).

El patrón correcto — poner el reset **adentro del `case`, no adentro del `if`** — está en el
parche DEL-04 de `correcciones-propuestas.md`. Va adentro del `case` para que se reescriba en
cada vuelta mientras dura la pausa, sin importar cuántas vueltas dé.

### (c) Un reloj para dos cosas distintas → una espera que no ocurre nunca

**El caso real: `GIRANDO` del delantero** — bug DEL-05.

`delantero.ino:432-452` (recortado: después de la 452 siguen las salidas por línea blanca,
`delantero.ino:455-470`, y recién ahí está el `break;`):

```cpp
    case GIRANDO:
      if (haypelota)
      {
        parar();
        if(millis() - millis_inicio_estado >= 700)  // L436  ← ¿hace cuánto FRENÉ?
        {
          estado = APUNTAR_PELOTA;
          millis_inicio_estado = millis();
        }
      }
      else { girar(); }

      if ((millis() - millis_inicio_estado >= 9000) && (abs(error) <= 50))  // L448 ← ¿hace cuánto GIRO?
      {
        millis_inicio_estado = millis();
        estado = AVANZANDO_POR_TIEMPO;
      }
      // ... siguen las salidas por linea blanca (L455-470) y el break; ...
```

Dos preguntas distintas, **el mismo reloj**. `millis_inicio_estado` se puso en cero cuando el
robot entró a `GIRANDO` (`delantero.ino:401`). La línea 436 quiere decir *"frená 700 ms para que
se vaya la inercia antes de apuntar"*, pero está midiendo *"hace cuánto que estoy girando"* —
y como el robot lleva varios segundos girando buscando la pelota, **la condición ya está
cumplida antes de que aparezca la pelota**.

Resultado: en la misma vuelta en que la cámara reporta pelota, frena y cambia de estado. La
espera dura menos de un milisegundo. El robot entra a apuntar **todavía girando a velocidad**,
se pasa de largo, corrige, se vuelve a pasar.

La variante peor está en `IMPULSO_INICIAL_GIRANDO` (`delantero.ino:393-413`). Ahí la línea 399
saca del estado a los 70 ms y **reinicia `millis_inicio_estado`** (L401), y recién en la 408
pregunta si pasaron 1000 ms. Como el reloj se acaba de poner en cero, la resta da ≈0. Y antes de
los 70 ms, la resta da menos de 70. **Esa condición no puede ser verdadera nunca, en ningún
camino.** (En `arquero.ino` la misma estructura está en `:374` con 50 ms en vez de 70, y
`arquero.ino:383` tiene el mismo defecto — pero ojo, eso está en el bloque muerto del arquero.)

**La regla:** *un reloj por cosa que medís.* Si tu estado responde dos preguntas de tiempo
distintas, necesita dos variables. Se ve en el código real: el delantero **ya tiene** un segundo
reloj para el centrado (`millis_inicio_centrando`, `delantero.ino:141`) precisamente porque
`millis_inicio_estado` se le reiniciaba en el medio de la maniobra. La solución ya está aplicada
en un lado y falta en otro.

---

## 9. Watchdog de sensores: un dato viejo es peor que no tener dato

Este es el concepto menos intuitivo de la skill, y el más importante.

Si un sensor no reporta, tu código lo nota (podés preguntar "¿llegó algo?"). Si un sensor
reporta y **después se muere**, tu código **no nota nada**: la variable sigue teniendo el último
valor, y ese valor se ve exactamente igual que un valor bueno. El robot decide con confianza
total sobre algo que ya no existe.

En este robot pasa así. Todo lo que el arquero sabe de la pelota (`haypelota`, `Xp`, `Yp`) se
actualiza **solamente** adentro del `if (Serial1.available() >= 9)` de `arquero.ino:263`. Si la
cámara deja de mandar —cable flojo, la OpenMV se colgó, se reinició— esa condición nunca se
cumple, el bloque no corre, y las variables quedan **congeladas en el último valor**. Nada las
envejece. Es el bug **A4**.

El arquero entonces se queda persiguiendo una pelota fantasma: se mueve hacia donde la pelota
estaba hace 10 segundos, o se queda plantado esperando una condición que nunca va a cambiar.

**Lo que ya existe en el código:** el patrón está escrito. `arquero.ino:305` pisa
`millis_pelota = millis()` cada vez que llega un dato válido **con pelota**. Eso es "la marca de
frescura". Y hay cuatro lugares que la leen, con **tres umbrales distintos**:

| Línea | Umbral | En qué estado |
|---|---|---|
| `arquero.ino:511` | 500 ms | delantero |
| `arquero.ino:562` | 500 ms | delantero |
| `arquero.ino:627` | 4000 ms | delantero |
| `arquero.ino:733` | 3000 ms | delantero |

```cpp
if (millis() - millis_pelota >= 500)   // arquero.ino:511 — si deja de ver la pelota
{
  millis_inicio_estado = millis();
  estado = IMPULSO_INICIAL_GIRANDO;
}
```

**Los cuatro están en estados del delantero.** Con `#define ROBOT1`, ninguno se ejecuta (ver
sección 5). En los diez estados vivos del arquero, `millis_pelota` **no aparece ni una vez** —
verificado buscando la variable en las 1207 líneas: sólo aparece en 135, 305, 511, 562, 627 y
733. El equipo 2025 resolvió el problema y lo resolvió de un solo lado.

### El patrón, generalizado — y su trampa

Antes del código, la trampa, porque es fácil comerse esta y creer que estás cubierto:

`millis_pelota` **no marca "llegó una trama"**: marca **"llegó una trama Y había pelota"**. Se
pisa adentro del `else` de `arquero.ino:302-306`, o sea sólo cuando `Xp != 0`. Entonces un
watchdog que sólo mire `millis_pelota` **no puede distinguir** "la cámara se murió" de "la
cámara está viva y no hay pelota en el campo de visión". Para las dos cosas necesitás **dos
marcas**.

**Propuesto — no compilado ni probado en banco:**

```cpp
// --- WATCHDOG DE FRESCURA ---
// Regla: un dato de sensor tiene FECHA DE VENCIMIENTO. Pasado ese tiempo,
// se trata como "no hay dato", no como "el ultimo dato que vi".

// (1) Dos marcas distintas, junto a las globales (cerca de arquero.ino:135):
unsigned long millis_trama = millis();   // ultima trama VALIDA (haya pelota o no)
// millis_pelota ya existe: arquero.ino:135

// (2) Marcar la trama valida: adentro del if de arquero.ino:277, junto a
//     los Xp = codedXp; etc.  ->  millis_trama = millis();
//     (millis_pelota ya se pisa en arquero.ino:305, no se toca)

// (3) Envejecer, en loop(), DESPUES de leer la camara y ANTES del switch:
const unsigned long VENCE_PELOTA_MS = 500;   // medir antes de fijar este numero
const unsigned long VENCE_CAMARA_MS = 500;   // idem

if (millis() - millis_pelota >= VENCE_PELOTA_MS)
{
  haypelota = false;      // hace rato que no veo la pelota: no hay pelota
}

if (millis() - millis_trama >= VENCE_CAMARA_MS)
{
  // la CAMARA no contesta: es otra falla, y amerita otra reaccion
  haypelota = false;
  hayarco_amarillo = false;
  hayarco_azul = false;
  // aca va lo que el equipo decida hacer sin camara (ver punto 3 de abajo)
}
```

Tres cosas que hacen la diferencia entre un watchdog que sirve y uno decorativo:

1. **Los números hay que medirlos, no inventarlos.** Imprimí `millis() - millis_trama` justo
   después de validar una trama, robot quieto y pelota fija, 30 segundos. Si el peor valor que
   ves es 120 ms, 500 ms es un umbral razonable (4×). Si ves valores cerca de 500, tenés un
   problema de comunicación más de fondo y **el watchdog no es el arreglo**. El procedimiento
   está en `correcciones-propuestas.md`, "PASO 1 — Número 3".
2. **Va una sola vez, arriba, no adentro de cada `case`.** Si lo ponés adentro de un `case`, los
   otros estados siguen usando el dato podrido.
3. **Tenés que decidir qué hace el robot sin el dato, y es una decisión de equipo.** Si el
   arquero se queda sin cámara, ¿barre a ciegas de un lado a otro o se queda quieto? Barrer a
   ciegas puede empujar la pelota **para adentro** de tu propio arco. No hay respuesta obvia:
   elijan, anótenlo, y prueben las dos.

### El otro agujero: trama corrupta ≠ trama ausente

`arquero.ino:277` chequea que las tres marcas sean 201/202/203 y sólo entonces copia los datos.
Bien. Pero **si el chequeo falla, no hace nada**: ya se comió los 8 bytes y las variables quedan
con los valores viejos. Y esto no es hipotético: `Xp`, `Yp`, `Xam` viajan como bytes crudos
dentro del paquete que arma la cámara (`enviar_coordenadas_2_arcos_y_pelota.py:149-155`), así
que **el valor 201 puede aparecer como dato** y enganchar al lector con el byte equivocado. Es
el bug **DEL-06**.

Regla general: **cada vez que escribís un `if` que valida algo, preguntate qué pasa cuando la
validación falla.** Si la respuesta es "nada", ahí tenés un dato viejo haciéndose pasar por
nuevo.

---

## 10. Debug por Serial: imprimir sin frenar el lazo

`Serial.print()` es tu única ventana al robot. También es una forma fácil de arruinar el timing.

### Las reglas

**1. Nunca imprimas todas las vueltas.** No sabemos cuántas vueltas por segundo da este loop
(sección 2), pero si son muchas, imprimir en cada una tira un chorro ilegible y le cuesta tiempo
al lazo. **Imprimí en los CAMBIOS.**

**2. Imprimí siempre con marca de tiempo.** Sin `millis()` adelante, no podés reconstruir la
secuencia ni medir cuánto duró un estado. Con la marca, el monitor se vuelve un cronómetro.

**3. Medí lo que te cuestan tus prints.** No hay ningún número medido en este repo sobre cuánto
cuesta un `Serial.print` acá, y **no lo inventes**. Usá el contador de vueltas por segundo de la
sección 2 con los prints puestos y sacados, y compará los dos números. Ese es el dato.

**4. Sacalos antes del torneo**, o dejalos detrás de una constante que puedas apagar en un solo
lugar (**propuesto, no probado**):

```cpp
#define DEBUG 1     // poner en 0 antes de competir
#if DEBUG
  #define DBG(x)   Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif
```

Con `DEBUG 0` el preprocesador borra las llamadas antes de compilar: no queda código.

### El print que más sirve: cambios de estado con marca de tiempo

Va **al final de `loop()`, después del `switch`**. Ahí es donde ves al ganador de la vuelta —
que es lo que importa por la regla de la sección 7. **Propuesto, no compilado ni probado:**

```cpp
// --- Pegar junto a las otras globales, despues de arquero.ino:131 ---
Estado estado_anterior = impulso_inicial;

const char* nombreEstado(Estado e) {
  switch (e) {
    case impulso_inicial:                  return "impulso_inicial";
    case moverce_izquierda:                return "moverce_izquierda";
    case moverce_derecha:                  return "moverce_derecha";
    case impulso_izquierda:                return "impulso_izquierda";
    case impulso_derecha:                  return "impulso_derecha";
    case PATEANDO_pausa_inicial_arquero:   return "PAT_pausa_inicial";
    case PATEANDO_adelante_arquero:        return "PAT_adelante";
    case PATEANDO_pausa_arquero:           return "PAT_pausa";
    case PATEANDO_atras_arquero:           return "PAT_atras";
    case avanzar_despues_de_patear:        return "avanzar_despues";
    default:                               return "OTRO(delantero)";
  }
}

// --- Pegar como ULTIMAS lineas de loop(), despues del cierre del switch
//     (o sea despues de arquero.ino:1206, antes del } final de la 1207) ---
if (estado != estado_anterior)
{
  Serial.print(millis());
  Serial.print(" ms  ");
  Serial.print(nombreEstado(estado_anterior));
  Serial.print(" -> ");
  Serial.println(nombreEstado(estado));
  estado_anterior = estado;
}
```

> ⚠️ `nombreEstado()` está escrito para el `enum` de **`arquero.ino`**. En `delantero.ino` el
> `enum` es distinto (sección 6): `avanzar_despues_de_patear` no existe ahí, así que ese `case`
> no compila en el delantero. Si lo llevás al otro archivo, sacá esa línea.

Salida esperable (ejemplo armado a mano con los timeouts del código, **no es una corrida real**):

```
  4120 ms  moverce_derecha -> impulso_izquierda
  4470 ms  impulso_izquierda -> moverce_izquierda
  6890 ms  moverce_izquierda -> PAT_pausa_inicial
  7090 ms  PAT_pausa_inicial -> PAT_adelante
  7540 ms  PAT_adelante -> PAT_pausa
```

De un vistazo verificás los tiempos contra el código: 4470−4120 = 350 ms = el timeout de
`impulso_izquierda` (`arquero.ino:1142`). 7090−6890 = 200 ms = `arquero.ino:1154`.
7540−7090 = 450 ms = `arquero.ino:1165`. **Si un número no coincide, encontraste algo.**

Y el diagnóstico que más vale: si ves un estado que entra y **nunca sale**, es el error (a) de
la sección 8. Si ves dos estados alternando cada pocos milisegundos, tenés un rebote entre dos
condiciones que se contradicen.

### El monitor

Abrilo a **19200** (`arquero.ino:77`: `const long BAUD_RATE = 19200;`, usado en `:238`). Si ves
caracteres raros, casi siempre es que el monitor está en otra velocidad.

Ese mismo 19200 es el de `Serial1` (`arquero.ino:239`) y tiene que coincidir con el de la cámara
(`enviar_coordenadas_2_arcos_y_pelota.py:6`: `uart = UART(3, 19200)`). **No cambies uno sin el
otro** o el robot deja de recibir la pelota.

### Cuidado con el LED

`arquero.ino:261` hace `digitalWrite(LED_BUILTIN, haypelota)` en cada vuelta. Ya tenés un
indicador gratis: **el LED de la placa prendido = el robot cree que ve la pelota.** Úsenlo.
Combinado con el watchdog de la sección 9, ese LED se vuelve el chequeo más rápido de
"¿la cámara está viva?".

---

## 11. Cómo pedirle código a una IA para este robot sin que te escriba Pybricks

Una IA que no sabe en qué robot está va a escribir, en este orden de probabilidad:
`delay()`, `while` que espera, `Servo.h`, `motor.setSpeed()` de alguna librería que no tenés, o
directamente Python. Todo eso compila mal o anda mal. La solución no es pelearse con la IA: es
darle el contexto que no tiene.

### El bloque de contexto (copiá y pegá esto antes de tu pedido)

```
Robot de fútbol RoboCup Junior, temporada 2025, IITA Salta.
HARDWARE (esto es TODO lo que existe, no supongas nada más):
- Teensy 4.1 sobre placa Zircon Rev v15. C++ estilo Arduino, archivo .ino, Arduino IDE,
  se sube por USB.
- 3 motores omni manejados por pines INA/INB (digitalWrite, dirección) + PWM (analogWrite 0-255).
- 3 sensores de línea ANALÓGICOS: readLine(1), readLine(2), readLine(3) -> analogRead.
  Umbrales de blanco por #define (arquero.ino: 500/650/600).
- 8 sensores IR de pelota TSSP58038 (activos en bajo) -> readBall(1..8). Hoy no se llaman nunca.
- 1 giroscopio BNO055 por I2C en 0x28 (pines 18/19), librería Adafruit_BNO055.
- 1 cámara OpenMV H7 en Serial1 (pines 0/1) a 19200 baudios, trama de 9 bytes
  con marcas 201/202/203.
NO HAY: encoders, odometría, ToF, ultrasonido, segunda cámara, solenoide de patada,
RTOS, FreeRTOS, PlatformIO, micro-ROS. La "patada" es un avance fuerte de las ruedas.
REGLAS DE CÓDIGO (no negociables):
- PROHIBIDO delay() dentro de loop(). PROHIBIDO cualquier while que espere algo.
- Toda espera se hace con millis(): guardar la marca al ENTRAR al estado y comparar.
- El programa es una máquina de estados: enum Estado + switch(estado) dentro de loop().
- En este código los chequeos de salida son if SUELTOS sin else al final del case: se
  evalúan SIEMPRE y la ÚLTIMA asignación de estado de la vuelta GANA. No reordenes nada
  sin decirme que lo estás haciendo y por qué.
- Todo estado necesita al menos una salida por tiempo.
- Toda variable acumulada se resetea en la ENTRADA al estado.
- No inventes valores medidos (PWM mínimo, velocidad, ganancias): si hace falta un número,
  decime cómo medirlo.
FORMATO DE RESPUESTA:
- Dame un diff mínimo, no una reescritura.
- Citá archivo:línea de cada cosa que toques o menciones.
- Decime qué NO verificaste.
```

Después de eso, agregá: **cuál de los dos archivos** (`arquero.ino` o `delantero.ino`),
**qué `#define` está activo** (`ROBOT1` o `ROBOT2`), y **pegá el `case` completo** que querés
tocar, no un fragmento.

### Cómo se verifica lo que te devuelve (checklist, 5 minutos)

Antes de subir nada al robot, sobre el código que te dio la IA:

1. **`delay(` — buscá la palabra.** Si aparece adentro de `loop()`, tirá esa respuesta.
2. **`while (` — buscá.** Si hay un `while` que espera una condición de sensor, tirala.
3. **¿Cada variable existe?** Si el código usa `ballDistance`, `robot.drive`, `gyro.angle()`:
   buscá el nombre en el `.ino`. Si no aparece, la IA se lo inventó. Las variables globales
   que **sí** existen están declaradas en `arquero.ino:65-136`.
4. **¿Cada pin existe?** Comparalo contra `mapa-pines-teensy.md` y contra los `#define` de
   `arquero.ino:37-59`. Un `analogWrite` a un pin que no está cableado no hace nada, y peor:
   `pinMode()` sobre el pin 0 le pisa el RX de la cámara.
5. **¿Cada `case` termina en `break;`?**
6. **¿Cada estado nuevo tiene salida por tiempo?**
7. **¿Cada estado nuevo del `enum` tiene su `case`?** (Ver sección 5: hoy hay 34 nombres y 32
   `case`.)
8. **¿Cada variable acumulada se resetea al entrar?**
9. **¿Citó archivo:línea?** Si te dice "en la función de patada" sin número, no lo leyó: lo
   está adivinando. Pedile la línea. Si te da una línea, **abrila y confirmá que dice lo que
   la IA dice que dice.** Esto pasa seguido y es la verificación que más bugs atrapa.
10. **¿Te dio números "medidos"?** Si te tira un piso de PWM, una velocidad tope o una ganancia
    como si fuera de este robot, desconfiá: no hay ninguno medido en este repo (sección 12).
11. **¿El código que tocó está vivo?** Chequealo con el método de la sección 5. Si está en el
    bloque del otro robot, no vas a ver ningún cambio.

### Y después: compilar no prueba nada

Que el Arduino IDE diga "Compilación completada" significa **exactamente una cosa**: que la
gramática de C++ está bien. No significa que el robot haga lo que querés, ni que no se cuelgue,
ni que los motores giren para el lado correcto.

**El veredicto lo da el robot en la mano, con un criterio de aceptación escrito antes de
probar.** "Anduvo bien" no es un resultado. "5 de 5 intentos con la pelota en `Yp = +4` arrancó
la patada dentro del primer segundo" sí lo es. Ese formato —con prueba de banco y criterio
medible— está en cada parche de `correcciones-propuestas.md`; copiá esa forma. Cómo se arma una
prueba de banco está en la skill `pruebas-en-banco-robot-real`.

Y anotá el resultado en `futbol-roboliga2026/bitacora/`. Un parche que nadie anotó es un parche
que el próximo va a tener que volver a descubrir.

---

## 12. Lo que este robot NO tiene (para que no te lo vendan)

Si una fuente, un tutorial o una IA te habla de estas cosas **en el contexto de este robot**,
está hablando de otro:

| Cosa | Estado real |
|---|---|
| Encoders / odometría | **No existen.** El robot no sabe cuánto se movió. Todo se mide en tiempo. |
| Sensores ToF (VL53L7CX) / ultrasonido | **No existen.** El robot no sabe a qué distancia está de la pared. |
| Solenoide de patada | **No existe.** La patada es un avance fuerte de ruedas (`arquero.ino:174-178`). |
| Dribbler | **No existe** en estos dos programas. |
| Segunda cámara / cámaras traseras | **No existen.** Una sola OpenMV en `Serial1`. |
| RTOS / FreeRTOS / tareas | **No existen.** Un solo `loop()` cooperativo. |
| PlatformIO / envs de compilación | **No se usan.** Es Arduino IDE, `.ino`, subida por USB. |
| micro-ROS, ESP-NOW, comunicación entre robots | **No existen** en este código. |
| Los 8 sensores IR de pelota | **Existen físicamente** (pines 14-17 y 20-23, `zirconLib.cpp:247-254`) y hay función `readBall()` (`zirconLib.cpp:106-142`), pero **ninguno de los dos programas la llama** — verificado buscando `readBall` en los dos `.ino`: cero apariciones. La pelota se detecta sólo por cámara. Es una capacidad disponible sin usar. |
| Botón de arranque | Los botones existen (pines 9 y 10, `zirconLib.cpp:256-257`) y hay `readButton()` (`zirconLib.cpp:144-155`), pero **el arquero no los usa nunca**: el robot arranca a mover motores apenas hay batería. Ojo al apoyarlo en la mesa. |

Y un aviso sobre números, que es la trampa más fácil de comerse:

**En este repo no hay ni un solo valor medido del robot 2025.** Ni piso de PWM, ni velocidad
tope, ni ganancia de control validada. Los números que sí están (`450`, `350`, `150`, `blanco1
500`) son **valores que alguien escribió en 2025**, no mediciones documentadas.

Si leés valores medidos de **otro** robot IITA —por ejemplo el del equipo de Incheon 2026—
**no los copies**. Es otra electrónica, otros motores, otra masa, otra placa. El **concepto**
vale y se explica solo: existe un piso de PWM por debajo del cual el motor no arranca, y se mide
subiendo el PWM de a 5 con la rueda en el aire hasta que empieza a girar, y después con el robot
apoyado (que da un número más alto). **El número de ESTE robot hay que medirlo en ESTE robot**,
y anotarlo en la bitácora con fecha y estado de batería.

---

## 13. Errores típicos, resumidos

| Síntoma en la cancha | Causa probable | Dónde mirar |
|---|---|---|
| No compila, `expected declaration before '}'` o `multiple definition of 'bno'` | la librería del repo está rota | `zirconLib.cpp:355` y `:4`; parche 0 en `correcciones-propuestas.md` |
| El robot no enciende / no mueve un motor | `while (1);` del BNO en `setup()` | `arquero.ino:246-249` |
| Se queda duro en un estado y no sale | estado sin salida por tiempo | sección 8(a), `arquero.ino:1184-1195` |
| Anduvo bien la primera vez y después no | variable acumulada sin resetear | sección 8(b), `delantero.ino:69` |
| Una espera programada "no se nota" | un reloj usado para dos cosas | sección 8(c), `delantero.ino:436` + `:448` |
| A veces no patea aunque esté alineado | otro `if` más abajo pisó el estado | sección 7, `arquero.ino:1070-1075` |
| Persigue una pelota que no está | falta watchdog de frescura | sección 9, `arquero.ino:263-329` |
| Arranca hacia donde la pelota **estaba** | buffer de `Serial1` lleno de tramas viejas | bug DEL-06 |
| Toqué un parámetro y no cambió nada | estás tocando código muerto, o una variable que ya está saturada | secciones 5 y 8(b) |
| Cambié el `#define` y el robot se porta raro | los dos `.ino` divergieron: umbrales y tolerancias distintas | sección 6 |
| "Hoy anda, mañana no", según cómo lo apoyás | `initialYaw` leído antes de que el BNO esté listo | bug A5, `arquero.ino:252-255` |

---

## 14. Archivos de referencia

- `robots-2025/arquero/arquero.ino` — 1207 líneas, `#define ROBOT1`, estado inicial `impulso_inicial`.
- `robots-2025/delantero/delantero.ino` — 1214 líneas, `#define ROBOT2`, estado inicial `AVANCE_INICIO`.
- `robots-2025/libreria-zircon/zirconLib.cpp` / `.h` — 355 líneas; `readLine` (`:157`), `readBall` (`:106`), `readButton` (`:144`), `InitializeZircon` (`:40`), `setZirconVersion` (`:52`).
- `robots-2025/mapa-pines-teensy.md` — qué pin es qué, en los dos robots.
- `robots-2025/arquero/COMO-FUNCIONA.md` y `robots-2025/delantero/COMO-FUNCIONA.md` — transcripción fiel del comportamiento.
- `bugs-conocidos.md` — los problemas verificados en código (A1/DEL-01, A2 a A6, DEL-02 a DEL-06).
- `correcciones-propuestas.md` — un parche por problema, **cada uno con su prueba de banco y criterio de aceptación**. Ninguno probado todavía.
- `robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py` — 156 líneas; el que arma la trama de 9 bytes (`:149-155`).
