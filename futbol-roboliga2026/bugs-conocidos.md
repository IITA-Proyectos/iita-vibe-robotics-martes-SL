# Problemas conocidos de los programas 2025

Auditoría del 2026-07-28 sobre los dos programas, leídos línea por línea. Cada problema fue
**verificado en el código** y después **puesto a prueba por un revisor independiente cuyo trabajo
era refutarlo**. Los 8 sobrevivieron. Dos bajaron de severidad en esa revisión, y están marcados.

Los parches están en [`correcciones-propuestas.md`](correcciones-propuestas.md).

> ⚠️ **Nada de esto está probado en el robot.** Es lectura de código. Lo que dice "confirmado"
> significa "confirmado que el código dice esto", no "confirmado que pasa en la cancha". Lo
> segundo lo cierra quien tiene el robot en la mano.

---

## Lo primero: hoy los programas NO COMPILAN

| | |
|---|---|
| **Severidad** | 🔴 P0 — bloquea todo |
| **Afecta a** | Los dos robots |
| **Dónde** | `zirconLib.cpp` líneas 4 y 355 |

Son **dos errores encadenados**, y el segundo está tapado por el primero:

**1. Hay una llave `}` de más** al final de `zirconLib.cpp` (línea 355). La función anterior ya
cerró en la 351, así que esa llave no cierra nada. El compilador corta con
`expected declaration before '}' token`.

**2. La variable `bno` está definida dos veces.** Una en la librería (`zirconLib.cpp:4`) y otra
en cada programa (`arquero.ino:68`, `delantero.ino:76`). Las dos son globales públicas, y el
*enlazador* — el paso final que junta la librería con el programa — no sabe cuál es cuál:
`multiple definition of 'bno'`.

**Este segundo error no lo van a ver hasta arreglar el primero**, porque el compilador ni llega
al enlazador. No es que rompieron algo al arreglar: siempre estuvo ahí.

> 🤔 **Entonces, ¿cómo ganaron el Nacional con esto?** No lo ganaron con esto. La copia de
> `zirconLib` que estaba instalada en la computadora del equipo 2025 era, casi seguro, otra —
> sin la llave suelta. **La copia que quedó en el repo está rota.** Antes de pisar nada, busquen
> en `Documentos/Arduino/libraries/` si hay una versión instalada distinta: esa es la que ganó,
> y conviene guardarla.

---

## Robot ARQUERO

### A2 — Retrocede sin límite de tiempo y se va de la cancha

| | |
|---|---|
| **Severidad** | 🔴 P0 |
| **Dónde** | `arquero.ino` líneas 1184-1195, estado `PATEANDO_atras_arquero` |

Después de patear, el arquero da marcha atrás para volver a su arco y usa la línea blanca como
freno. Ese estado tiene **una sola puerta de salida**: que un sensor vea blanco. No hay ningún
reloj que lo saque de ahí.

Si la línea no aparece — sensor sucio, umbral mal calibrado para la luz del gimnasio, cable
flojo, o el robot arrancó el retroceso desde una posición donde va a chocar la pared antes —
**el estado no termina nunca**. El arquero se va del campo por atrás, o se clava contra la pared
con dos motores forzando a PWM 150 hasta que se corta la batería o se recalienta el driver.

Es el modo de falla más caro del arquero: el arco queda vacío el resto del punto y no se
recupera solo.

### A3 — Se planta cuando la pelota queda a `Yp = ±4`

| | |
|---|---|
| **Severidad** | 🟡 P1 — **bajó de P0**, ver abajo |
| **Dónde** | `arquero.ino` líneas 1038 y 1086 |

Hay un umbral para patear (`|Yp| <= 3`) y otro para moverse (`|Yp| >= 5`). Lo que queda en el
medio cae en un `else { parar(); }`. Resultado: pelota cerca y en el medio → el robot mira la
pelota y no hace nada.

**Por qué bajó de P0 a P1:** el equipo anterior lo describió como un rango peligroso, pero `Yp`
llega como un **byte entero**. Así que "entre 3 y 5" significa exactamente **dos valores: +4 y
-4**, sobre un rango que va de -100 a +155. Con la pelota en movimiento lo cruza y se destraba
solo. Es un agujero real y hay que taparlo, pero llamarlo "gap mortal" infla el riesgo.

**El caso que sí importa:** pelota **cerca** (`Xp <= 140`) y quieta justo en `Yp = ±4`. Ahí no
patea y no se mueve, y deja pasar la pelota. Que es exactamente el momento en que el arquero
tiene que despejar.

### A4 — Si la cámara se cuelga, el arquero se queda mirando un fantasma

| | |
|---|---|
| **Severidad** | 🟡 P1 |
| **Dónde** | `arquero.ino` líneas 263-332 |
| **Estado** | ⭐ **Nuevo — no estaba en la lista del equipo anterior** |

Todo lo que el arquero sabe de la pelota (`haypelota`, `Xp`, `Yp`) se actualiza **solo** cuando
llegan 9 bytes por `Serial1`. Si la cámara deja de mandar — cable flojo, la OpenMV se colgó, se
reinició — esa condición nunca se cumple y las variables quedan **congeladas en el último valor**.
Nada las envejece.

Lo peor es que **el equipo anterior ya resolvió este problema… pero solo para el delantero.** En
las líneas 511, 562, 627 y 733 hay chequeos de "¿hace cuánto que no veo la pelota?" que sacan al
robot del estado. Los cuatro están en estados del delantero, que en este build no se ejecutan
nunca. En los estados del arquero, esa variable no aparece ni una vez.

Se combina de la peor manera con A3: cualquier freno momentáneo se vuelve **permanente**.

### A5 — El "cero" del giroscopio se lee antes de que el sensor esté listo

| | |
|---|---|
| **Severidad** | 🟡 P1 |
| **Dónde** | `arquero.ino` líneas 246-256 (y `delantero.ino` 269-280, ver DEL-03) |

Dos cosas en el mismo bloque de 10 líneas:

**1. `while(1)` deja el robot muerto y mudo.** Si el BNO055 no contesta al arrancar, el programa
entra en un bucle vacío y se queda ahí para siempre. `setup()` nunca termina, los motores nunca
se prenden. El único aviso sale por USB, que en cancha nadie está mirando. Robot que "no
enciende" → primer sospechoso el giroscopio, no la batería.

**2. `initialYaw` se lee 0 ms después de encender el sensor.** `initialYaw` es la brújula-cero
del robot: el rumbo al que apuntaba cuando lo prendieron, y **todo lo demás se mide contra eso**.
Pero se lee inmediatamente después de `bno.begin()`, y el BNO055 necesita un rato para que su
mezcla de sensores entregue un ángulo válido — hasta entonces devuelve `0.00`.

Si `initialYaw` queda en 0, entonces `error` deja de significar "cuánto me desvié de como
arranqué" y pasa a significar "para dónde está el norte magnético". Y en el arquero ese `error`
**sí mueve las ruedas** (elige entre tres repartos de potencia). El barrido lateral sale torcido,
de forma distinta según para dónde lo apoyaron al prenderlo. Es el síntoma clásico de
**"hoy anda y mañana no"**.

> 📌 Esto es lo que el equipo anterior llamó "BUG B", pero por el camino equivocado. La cuenta de
> las líneas 337-339 **está bien hecha**. El problema no es la fórmula, es que `initialYaw`
> probablemente valga 0. **Falta confirmarlo en banco** — el test está en el doc de correcciones.

### A6 — `currentYaw` crudo: real, pero en código muerto

| | |
|---|---|
| **Severidad** | 🔵 P2 — **no tocar ahora** |
| **Dónde** | `arquero.ino` líneas 606, 642, 656, 671, 712, 748, 762, 776 y 422 |

Hay comparaciones tipo `if (currentYaw <= 10 or currentYaw >= 350)`, que preguntan *"¿estoy
apuntando al norte?"* en vez de *"¿estoy apuntando a donde arranqué?"*. El bug es real.

**Pero todas viven en estados del DELANTERO**, y con `ROBOT1` compilado son inalcanzables. Se
verificó siguiendo a mano las 13 asignaciones de estado del bloque del arquero: forman un
conjunto cerrado que nunca sale hacia esos estados.

Y hay un detalle revelador: **en el archivo del delantero esas mismas condiciones ya están
reescritas con `abs(error)`**. O sea, el equipo 2025 encontró el problema y lo arregló de un solo
lado. Eso, por sí solo, prueba que estaban trabajando activamente sobre el giroscopio en 2025.

> ⚠️ El síntoma que reportaron ("el arquero solo anda si se enciende apuntando al norte")
> **no puede venir de estas líneas**, porque el arquero no las ejecuta. Si ese síntoma se vio de
> verdad, la causa más probable es **A5**. Verifiquen A5 primero.

---

## Robot DELANTERO

### DEL-02 — El arco rival está fijo en AMARILLO

| | |
|---|---|
| **Severidad** | 🟡 P1 — **bajó de P0** |
| **Dónde** | `delantero.ino` líneas 354-356, usadas en 621 y 692 |

El robot orbita la pelota hasta alinearla con el arco **amarillo**, y ahí patea. Las coordenadas
del arco azul se calculan… y se tiran a la basura. **Si en el sorteo les toca defender el arco
amarillo, el delantero hace goles en contra de manera sistemática.** No a veces: el 50 % de los
partidos, decidido por el sorteo de lado.

**Por qué bajó a P1:** hay otra forma de disparar la patada (4 segundos orbitando + orientación
casi igual a la inicial) que no mira ningún arco. O sea, el robot **patea igual**. Lo que está
roto es la puntería, no la patada. Por eso pudo ganar un nacional con esto adentro.

Es el hallazgo con peor relación daño/esfuerzo de todo el archivo: 15 minutos de trabajo contra
perder partidos enteros.

### DEL-03 — Mismo problema del `initialYaw`, pero acá decide las patadas

| | |
|---|---|
| **Severidad** | 🟡 P1 — **bajó de P0** |
| **Dónde** | `delantero.ino` líneas 269-280 |

Es el mismo defecto que A5, pero en el delantero pega distinto. Acá el giroscopio **no mueve las
ruedas**: decide **cuándo patear**. De `error` cuelgan las cinco decisiones más importantes:

- línea 448 — avanzar después de 9 s girando, si está más o menos derecho
- líneas 628 y 699 — **DISPARAR LA PATADA** cuando `abs(error) <= 1`
- líneas 659 y 730 — al pisar una línea blanca, decidir si patea o si sigue orbitando
  (una salvaguarda **anti-autogol**)

Si `initialYaw` es basura, el robot **patea para cualquier lado** o se queda orbitando la pelota
25 segundos sin patear nunca — y el comportamiento cambia si mueven la cancha de sala.

### DEL-04 — La rampa suave de la patada solo funciona la primera vez

| | |
|---|---|
| **Severidad** | 🟡 P1 — **bajó de P0** |
| **Dónde** | `delantero.ino` línea 69 (la variable) y 189-198 (la rampa) |

`velocidadActualPateo` sube de 5 en 5 hasta 240 y **nunca vuelve a cero**. Como es global,
conserva el valor entre patadas.

Al encender, el estado inicial llama a la función de pateo durante 700 ms → llega a PWM ~175.
De ahí no baja nunca. **La primera patada arranca en 175, y todas las siguientes arrancan
directo en 240.** El arranque suave que los chicos programaron funciona una sola vez en toda la
vida del robot.

**Por qué bajó a P1:** el robot ganó el Nacional **con** este bug activo. O sea, la patada a full
es el comportamiento que vieron y ajustaron en cancha. No es "el robot no anda", es "el robot no
hace lo que el código dice que hace". Y arreglarlo mal deja la patada corta inútil — leer bien
el parche antes de tocar.

Costo escondido: cualquier intento de tunear `pasoPateo` o `intervaloPateo` **hoy no tiene ningún
efecto observable**. Si prueban y "no responde", no es que se rompió: es este bug.

### DEL-05 — La pausa de 700 ms "por la inercia" no ocurre nunca

| | |
|---|---|
| **Severidad** | 🟡 P1 |
| **Dónde** | `delantero.ino` líneas 432-441 |
| **Estado** | ⭐ **Nuevo** |

Cuando el robot ve la pelota mientras gira buscándola, el código dice: frená y esperá 700 ms a
que se vaya la inercia, después apuntá. Pero el cronómetro que mira **se puso en cero al entrar
al estado GIRANDO**, no cuando apareció la pelota. Como el robot lleva varios segundos girando,
la condición **ya está cumplida desde antes de ver la pelota**.

Resultado: en el mismo ciclo en que aparece la pelota, frena y cambia de estado. La espera dura
menos de un milisegundo. El robot entra a apuntar **todavía girando a velocidad**, se pasa de
largo, corrige, se vuelve a pasar. Es una de las causas típicas de *"tarda una eternidad en
decidirse a ir a la pelota"*.

En las líneas 405-413 está el mismo error pero peor: ahí la condición **no puede cumplirse nunca**,
porque el reloj se reinicia 70 ms antes.

### DEL-06 — El lector de la cámara no se defiende de una trama rota

| | |
|---|---|
| **Severidad** | 🟡 P1 |
| **Dónde** | `delantero.ino` líneas 287-353 |

Dos agujeros:

**1. Si la trama llega corrupta, se sigue creyendo la anterior.** El código verifica que las tres
marcas sean 201/202/203, pero si no lo son, **no hace nada**: ya se comió los 8 bytes y las
variables quedan con los valores viejos. Y esto no es teórico: `Xp`, `Yp`, `Xam`… viajan como
bytes de 0 a 255, así que **el valor 201 puede aparecer como dato**. Cuando pasa, el lector se
engancha con el byte equivocado y pierde el sincronismo.

**2. Se lee una sola trama por vuelta de loop.** Si la cámara manda más rápido de lo que el
Teensy consume, el buffer (64 bytes = 7 tramas) se llena de tramas viejas y el robot decide con
datos de hasta 7 cuadros atrás. Cuando desborda, se pierden bytes en el medio de una trama y el
sincronismo se rompe otra vez.

En cancha se ve como *"el robot está poseído"*: arranca hacia donde la pelota **estaba**, o pega
tirones raros como si viera dos pelotas.

> ✅ Algo bueno que el código YA tiene y conviene no romper: si la cámara se desconecta **del
> todo**, el lector deja de correr, los timeouts de 500 ms saltan y el robot vuelve a girar. La
> desconexión total está cubierta. Lo que no está cubierto es la corrupción **parcial**, que es
> peor porque es silenciosa.

---

## Dos cosas más que conviene mirar antes de tocar código

### La placa se autodetecta leyendo un pin, y hay que confirmar qué elige

`setZirconVersion()` (`zirconLib.cpp:52-60`) decide si la placa es "Mark1" o "Naveen1" leyendo el
**pin 32** con pull-down interno: LOW → Mark1, HIGH → Naveen1. Como el pull-down interno tira el
pin a masa, **por defecto va a leer Mark1**, que es el que coincide con los pines de `ROBOT1` y
con el mapa de pines. O sea: probablemente esté bien.

Pero **si alguna vez saliera "Naveen1"**, las consecuencias son feas: los sensores de línea pasan
a otros pines (lecturas basura) y los pines de PWM de motor quedan en 0, con lo cual la librería
haría `pinMode(0, OUTPUT)` **sobre el pin RX de la cámara**.

Es un test de 5 minutos que vale la pena hacer una vez: imprimir `getZirconVersion()` en el
`setup()` y confirmar que dice `Mark1`.

### El robot arranca solo al enchufar la batería

`readButton()` existe en la librería y los botones están en los pines 9 y 10 — pero **el arquero
no los usa nunca**. No hay botón de arranque: el programa empieza a mover motores apenas hay
energía. Téngalo en cuenta al apoyarlo en la mesa.

Ojo si piensan agregarlo: el mapa de pines dice "pull-up interno" pero `zirconLib.cpp:339-340`
configura los botones como `INPUT` pelado, **sin** pull-up. Hay que medir con el multímetro qué
lee el pin antes de confiar en el botón.

---

## Resumen

| # | Robot | Problema | Sev. | ¿Confirmado en código? |
|---|---|---|---|---|
| A1 / DEL-01 | ambos | La librería no compila (llave + `bno` duplicado) | 🔴 P0 | ✅ |
| A2 | arquero | Retroceso sin timeout → se va de la cancha | 🔴 P0 | ✅ |
| A3 | arquero | Se planta con la pelota en `Yp = ±4` | 🟡 P1 | ✅ |
| A4 | arquero | Sin watchdog de cámara → persigue un fantasma | 🟡 P1 | ✅ |
| A5 | arquero | `initialYaw` inválido + `while(1)` mudo | 🟡 P1 | ✅ |
| A6 | arquero | `currentYaw` crudo — **en código muerto** | 🔵 P2 | ❌ inalcanzable |
| DEL-02 | delantero | Arco rival fijo en amarillo → autogoles | 🟡 P1 | ✅ |
| DEL-03 | delantero | `initialYaw` inválido → patea para cualquier lado | 🟡 P1 | ✅ |
| DEL-04 | delantero | La rampa de patada solo funciona una vez | 🟡 P1 | ✅ |
| DEL-05 | delantero | La pausa de 700 ms no ocurre | 🟡 P1 | ✅ |
| DEL-06 | delantero | Lector de cámara sin defensa ante trama rota | 🟡 P1 | ✅ |

## Lo que NO se auditó

Honestidad sobre el alcance, para que nadie asuma que está todo cubierto:

- **La máquina de estados del delantero no se recorrió completa.** Se auditaron los puntos
  señalados y algunos vecinos, pero el delantero tiene 1214 líneas y su camino principal
  (`AVANCE_INICIO` → `GIRANDO` → `APUNTAR_PELOTA` → `AVANZANDO` → `CENTRANDO_*` → patada)
  merece una pasada propia. **Falta la mitad del trabajo.**
- **El programa de la cámara (`.py`) no se auditó**, y es de donde salen `Xp` e `Yp`, o sea la
  entrada de la que dependen A3, A4 y DEL-06.
- **Nada se probó en hardware.** Todo es lectura de código.
