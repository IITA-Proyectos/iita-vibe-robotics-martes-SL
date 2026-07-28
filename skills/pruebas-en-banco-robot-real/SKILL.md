---
name: pruebas-en-banco-robot-real
description: Cómo se prueba un cambio en el robot de fútbol de verdad (Teensy 4.1 + Zircon + OpenMV), con hipótesis previa, criterio de aceptación numérico, control de no-regresión y bitácora. Usar SIEMPRE que aparezca "probar", "probamos", "banco", "prueba de banco", "lo pruebo en el robot", "¿cómo sé si anda?", "anda / no anda", "funciona / no funciona", "compila pero...", "la IA dijo que estaba bien", "ChatGPT / Claude me dijo", "criterio de aceptación", "no-regresión", "regresión", "un cambio por vez", "reproducir el bug", "hacer que falle a propósito", "medir", "cronometrar", "cuánto tarda", "cuánto PWM", "Serial.print", "monitor serie", "imprimir por serial", "debuggear el robot", "ruedas al aire", "batería baja", "el robot arranca solo", "checklist", "bitácora", "qué anoto", "¿esto ya está arreglado?", "¿quién dice que anda?", "lo damos por cerrado". También cuando alguien vaya a aplicar un parche de `futbol-roboliga2026/correcciones-propuestas.md`, o quiera cerrar un tema de `bugs-conocidos.md`. NO usar para robots LEGO Spike Prime / Pybricks (eso va en las skills `pybricks-*`), ni para el diagnóstico de un bug puntual del código 2025 (eso ya está escrito en `futbol-roboliga2026/bugs-conocidos.md` y `correcciones-propuestas.md`; esta skill es CÓMO se prueba, no QUÉ está roto).
---

# Probar en el robot de verdad

Esta es la skill aburrida. También es la que decide si el equipo avanza o gira en falso cuatro
meses. Sin método de prueba, cuatro personas con una IA producen mucho código y cero
conocimiento: cada clase se cambia algo, el robot se comporta raro, se cambia otra cosa, y a las
seis semanas nadie sabe qué versión andaba mejor.

Todo lo que sigue está anclado al robot que tienen: **Teensy 4.1 sobre Zircon Rev v15, 3 motores
por `INA`/`INB`/`PWM`, 3 sensores de línea analógicos, BNO055 por I²C, una OpenMV H7 en `Serial1`
a 19200**. Cada número concreto viene con su `archivo:línea`.

---

## 1. Las tres cosas que NO prueban nada

### "Compila"

Compilar prueba que la gramática de C++ está bien. Nada más. El compilador no sabe qué es un
arco, no sabe que el motor 3 está atrás, y no tiene idea de si tu robot se va de la cancha.

En este repo hay una demostración perfecta, en las dos direcciones:

- La copia de `zirconLib.cpp` que está en el repo **no se puede construir**, por dos motivos
  distintos:
  - Una llave `}` suelta al final (`libreria-zircon/zirconLib.cpp:355`, después de que la línea
    351 ya cerró la última función). Eso lo caza el **compilador**: es un error de gramática.
  - La variable `bno` está definida dos veces: `zirconLib.cpp:4` y `arquero.ino:68`. Y
    `zirconLib.h` **no** la declara como `extern` (miralo: el `.h` solo declara funciones). Eso
    no lo caza el compilador, porque compila cada archivo por separado y en cada uno la
    definición es legal. Lo caza el **enlazador** (el "linker": el programa que pega los
    archivos compilados en un solo binario) cuando encuentra dos `bno` con el mismo nombre.

  Y sin embargo, **este robot ganó el Nacional 2025**. O sea: una copia de código que ni siquiera
  se construye puede convivir con un robot que ganó, porque el binario que está adentro del
  Teensy salió de otra copia que nadie guardó.
- Al revés también: podés hacer que compile perfecto un firmware que mande el robot de cabeza
  contra la pared.

**Compilar es un requisito para poder probar. No es evidencia de nada sobre el robot.**

### "La IA dijo que estaba bien"

Peor todavía, porque suena a autoridad. Una IA no vio tu robot, no midió tu batería y no sabe qué
copia de la librería tenés instalada. Escribe lo más probable, no lo verdadero.

Tres casos reales de este mismo material:

1. En `correcciones-propuestas.md` hay un parche (DEL-04) cuya **primera versión estaba mal**: la
   IA propuso arrancar la rampa de patada en 0 para "restaurar el arranque suave". Un revisor lo
   tumbó con una cuenta de diez segundos: la rampa sube 5 de PWM (`pasoPateo`,
   `delantero.ino:71`) cada 20 ms (`intervaloPateo`, `delantero.ino:73`), y la patada corta dura
   200 ms (`delantero.ino:859`). 200 ÷ 20 = 10 subidas × 5 = **PWM 50 de 255**, cuando el valor
   final buscado es 240 (`velocidadFinalPateo`, `delantero.ino:70`). El parche dejaba al robot
   sin patada.
2. `mapa-pines-teensy.md:28-29` dice que los botones tienen "pull-up interno". `zirconLib.cpp:339-340`
   los configura como `INPUT` **pelado**, sin pull-up. Dos documentos del mismo repo, uno miente.
   **Manda el código.**
3. El `arquero.ino` y el `delantero.ino` son casi el mismo archivo, pero el bloque `ROBOT1` de
   uno **no dice lo mismo** que el bloque `ROBOT1` del otro: umbrales de blanco 500/650/600
   (`arquero.ino:50-52`) contra 600/600/600 (`delantero.ino:50-52`); `patadM2` 150
   (`arquero.ino:54`) contra 170 (`delantero.ino:54`). Y afuera del bloque `#define`,
   `tolerancia_cercania` vale 140.0 en `arquero.ino:110` y 50.0 en `delantero.ino:119`. Si le
   preguntás a una IA "¿cuál es el umbral de blanco del arquero?" y no le decís qué archivo, te
   va a contestar con seguridad total un número que puede ser el equivocado.

Regla: **la IA propone, el banco dispone.** Una respuesta de IA es una hipótesis con buena
redacción.

### "Ya lo probamos una vez y anduvo"

Una pasada no es un resultado. Un robot que hace lo correcto 1 de 3 veces no "anda a veces": está
roto, y todavía no sabés por qué. En banco, el número mínimo decente es **5 repeticiones**, y el
resultado se escribe como fracción: *"5 de 5"*, *"3 de 5"*.

---

## 2. Regla dura: UN CAMBIO POR VEZ

Si tocás tres cosas y el robot empeora, **no sabés cuál fue**. Y no es que "no sabés bien": no
tenés forma de averiguarlo sin deshacer los tres y empezar de nuevo. Perdiste la clase.

Es peor de lo que parece, porque los cambios acá **interactúan de verdad**. La estructura de este
firmware lo garantiza: en casi todos los estados los chequeos de línea y de timeout están al final
del `case`, **sin `else`**, así que se evalúan siempre y **la última asignación de estado del
ciclo gana**. Mirá `moverce_derecha`: el bloque de la pelota está en `arquero.ino:1036-1066` y el
chequeo de línea blanca en `arquero.ino:1070-1075`, **después**. Si en el mismo ciclo la pelota
está en posición de patada **y** un sensor lee blanco, **gana el blanco** y no patea.

Consecuencia: un cambio en el umbral de línea puede hacer que "deje de andar" la patada. Si esa
clase también tocaste la tolerancia de cercanía, vas a culpar a la tolerancia.

**La excepción, que hay que declarar antes:** a veces dos cambios *deben* ir juntos porque tocan
el mismo estado y por separado son inatribuibles. En `correcciones-propuestas.md` pasa con A4 y A3
(`correcciones-propuestas.md:203`, y está declarado como excepción en la fila 3 de la tabla de
orden, `:37`). A4 es un *watchdog* de cámara — "perro guardián": un reloj que, si hace mucho que
no llega un dato nuevo de la cámara, da la pelota por no vista en vez de seguir decidiendo con
información vieja. A3 es el hueco de `Yp = ±4`. Aplicar uno solo hace imposible saber a qué se
debe el cambio de comportamiento. Cuando eso pase, **se dice en la bitácora antes de probar**:
"esto va como un solo cambio, por esta razón". Lo que no vale es descubrirlo después.

---

## 3. Anatomía de un test de banco

Cinco partes. Si falta una, no es un test, es tocar el robot a ver qué pasa.

### 3.1 Hipótesis — se escribe ANTES de encender

Una frase, en la bitácora, antes de conectar la batería:

> *"Espero que, con la pelota a `Yp = 4` y `Xp < 140`, el robot se quede quieto y no patee."*

Por qué esto no es burocracia: escribir la predicción **antes** es lo único que te protege de la
trampa mental más común del taller, que es mirar el robot hacer cualquier cosa y decir "sí, era lo
que esperaba". Si lo escribiste antes, no podés reescribir la expectativa después.

Y si tu hipótesis falla, **ganaste**: acabás de descubrir que tu modelo del robot estaba mal, que
es exactamente lo que viniste a averiguar.

### 3.2 Montaje

Qué robot, qué archivo, qué `#define`, dónde está apoyado, con qué batería, con qué luz.

En este robot el montaje incluye dos cosas que no son obvias.

**(a) Qué copia de `zirconLib` va a usar el compilador.** `#include <zirconLib.h>` con `< >`
significa "buscala en las librerías instaladas" (`Documentos/Arduino/libraries/`), **no** "la que
está al lado del programa" (`entorno.md:46-51`). Si editás la del repo y compilás, no cambia nada.
Para saber cuál usa: Arduino IDE → `Archivo → Preferencias → Mostrar salida detallada durante:
compilación`, y leer la ruta en la consola.

**(b) Qué "versión de Zircon" cree la librería que es la placa.** Esto es sutil y puede arruinar
una tarde entera. En el arranque, `setZirconVersion()` lee el **pin 32** con resistencia interna
a masa: si lee LOW la llama `"Mark1"`, si no `"Naveen1"` (`zirconLib.cpp:52-60`). Y de esa
decisión dependen los pines analógicos que usa `readLine()`: `A11 / A13 / A12` en Mark1
(`zirconLib.cpp:259-261`) contra `A8 / A9 / A12` en Naveen1 (`zirconLib.cpp:286-288`).

Traducción: si la placa se identifica distinto de lo que ustedes suponen, `s1` y `s2` están
leyendo **otros pines** y los valores no significan nada. Chequealo una vez, al principio, con
una línea en el `setup()`:

```cpp
Serial.print("Zircon = "); Serial.println(getZirconVersion());
```

(`getZirconVersion()` está declarada en `zirconLib.h:27`, así que se puede llamar desde el `.ino`.)
Según `mapa-pines-teensy.md:40-42` los sensores de línea van en A11/A13/A12, o sea que **lo
esperado es `Mark1`**. Si imprime otra cosa, pará y avisá antes de seguir midiendo nada.

### 3.3 Criterio de aceptación — observable y numérico

"Anda mejor" no es un criterio. Un criterio de aceptación tiene que poder ser contestado con
**sí/no por alguien que no participó del cambio**, mirando un número o un conteo.

| ❌ No sirve | ✅ Sirve |
|---|---|
| "El arquero reacciona más rápido" | "Entre que la pelota entra al campo de visión y las ruedas cambian de sentido pasan **menos de 300 ms**, medidos filmando a 60 fps y contando cuadros. 5 de 5 intentos." |
| "Ya no se va de la cancha" | "Con los tres sensores tapados (verificado por el monitor: `s1`, `s2`, `s3` por debajo de su umbral), las ruedas dejan de girar en reversa a los **1500 ± 200 ms**. 5 de 5." |
| "La patada quedó igual" | "La pelota recorre **entre 95 y 115 cm** medidos con cinta métrica desde el punto de contacto. Antes del cambio recorría 105, 100, 110, 100, 105." |
| "El giroscopio ahora sí arranca bien" | "Agregando un `Serial.println(initialYaw);` al final del `setup()` (`initialYaw` se toma en `arquero.ino:254`) y prendiendo el robot 5 veces apuntando a 5 direcciones distintas, imprime **5 valores distintos** coherentes con la dirección, y las dos veces que apunta al mismo lado difieren **menos de 10°**." |

Tres cosas que hacen bueno a un criterio: **un número**, **una tolerancia**, y **cuántas veces de
cuántas**.

### 3.4 Control de no-regresión

Es la pregunta *"¿qué otra cosa pude haber roto sin querer?"*. Se elige **antes** de probar, no
después de que algo salga raro.

"No-regresión" quiere decir: **las cosas que ya daban bien tienen que seguir dando igual**. Para
este robot, el set mínimo del arquero es cuatro:

1. **Barrido lateral:** sigue yendo de un lado al otro y frena al ver blanco
   (`arquero.ino:1070` y `arquero.ino:1117`). Ojo: esos dos chequeos miran **solo `s1` y `s2`**,
   no `s3`.
2. **Rebote en el borde:** al ver blanco entra al impulso de **350 ms** hacia el otro lado
   (`arquero.ino:1130` y `arquero.ino:1142`) y no se queda oscilando trabado en el borde — que es
   exactamente para lo que existen esos dos estados, según el comentario del propio código
   (`arquero.ino:1127` y `arquero.ino:1139`).
3. **Secuencia de patada completa:** pausa 200 ms (`arquero.ino:1154`) → golpe 450 ms
   (`arquero.ino:1165`) → pausa 1000 ms (`arquero.ino:1177`) → retroceso hasta ver blanco
   (`arquero.ino:1190`) → avance 1000 ms (`arquero.ino:1200`) → vuelve a barrer.
4. **Compila con los dos `#define`:** cambiar `ROBOT1` (`arquero.ino:10`) por `ROBOT2`
   (`arquero.ino:11`) y verificar que sigue compilando. Barato, y atrapa cambios que rompieron
   el bloque del otro robot.
   ⚠️ **Dejalo como estaba antes de cargar.** Si subís el `.ino` del arquero con `ROBOT2` activo,
   los pines de motor quedan los del delantero y el robot se mueve en direcciones absurdas
   (`entorno.md:83-85`). Compilar ≠ cargar: para esta verificación alcanza con el botón
   "Verificar", no con "Subir".

### 3.5 Repeticiones y anotación

5 repeticiones mínimo. Los números crudos van a la bitácora, **incluidos los que no te gustan**.

### Plantilla del test (pegar en la bitácora)

```markdown
### Test — <nombre corto>

**Robot / archivo / #define:** arquero / <ruta exacta del .ino que cargaste> / ROBOT1
**Batería:** cargada / a mitad de clase / <voltaje si lo midieron>
**Hipótesis (ANTES de encender):** espero que...
**Montaje:** ruedas al aire / en la cancha / con tope atrás / luz de la sala
**Pasos:** 1... 2... 3...
**Criterio de aceptación:** <número + tolerancia + N de N>
**No-regresión:** <las 3-4 cosas que tienen que seguir dando el mismo resultado>

**Resultado:** N de N. Números crudos: ...
**Qué salió distinto de lo esperado:** ...
```

---

## 4. El paso que SIEMPRE se saltea: reproducir la falla primero

Este es el que separa arreglar de creer que arreglaste.

**Si no lograste que el robot falle a propósito, no vas a poder saber si tu arreglo sirvió.**

El motivo es simple y brutal: el modo de falla que estás persiguiendo puede aparecer 1 vez cada
20. Aplicás el parche, probás 3 veces, no falla, decís "arreglado". Después falla en la final.
Nunca supiste si el parche hizo algo, porque nunca supiste cuál era el ritmo de la falla.

El orden correcto tiene tres pasos y el primero es el que se saltea:

1. **Provocar la falla, con el código SIN tocar.** Si no la podés provocar, **parás**: tu modelo
   del problema está mal y el parche va a ser a ciegas.
2. Aplicar **un** cambio.
3. Repetir exactamente el mismo procedimiento y comparar el mismo número.

### Ejemplo A — el hueco de `Yp = ±4` en el arquero

**El problema, en el código:** el arquero patea si la pelota está cerca y muy centrada
(`Xp <= tolerancia_cercania` y `abs(Yp) <= 3`, `arquero.ino:1038`) y se mueve si está desviada
(`abs(Yp) >= 5`, `arquero.ino:1045`). Lo que cae en el medio va a un `else { parar(); }`
(`arquero.ino:1060-1061`). `tolerancia_cercania` vale **140.0** en el arquero (`arquero.ino:110`).
Como `Yp` llega decodificado de un byte (`Yp = codedYp - 100`, `arquero.ino:280`; del otro lado la
cámara suma 100 antes de mandar, `enviar_coordenadas_2_arcos_y_pelota.py:143`), `Yp` es siempre un
entero, así que "entre 3 y 5" son exactamente **dos valores: +4 y −4**.

**Cómo provocarlo (esto es el test, no el arreglo):**

1. Descomentar las impresiones de `Xp`/`Yp` que ya están escritas en `arquero.ino:292-293` y
   agregarles un `Serial.println();` al final para que no quede todo en un renglón.
2. Robot **con las ruedas al aire**, monitor serie abierto.
3. **Antes de mover la pelota, verificá que ningún sensor de línea esté leyendo blanco.** Si `s1`
   o `s2` superan su umbral, el chequeo de `arquero.ino:1070` te va a sacar del estado y vas a ver
   las ruedas moverse **por otro motivo**, y vas a concluir cualquier cosa. Usá el volcado de
   sensores de la sección 6 para confirmarlo.
4. Mover la pelota **muy despacio** hasta que el monitor muestre `Yp = 4` con `Xp` menor a 140.
   Dejarla quieta 3 segundos.
5. **Criterio del test de reproducción:** las ruedas tienen que quedarse **completamente quietas**.

**Y acá está la parte que importa:** si el robot **no** se queda quieto, tu modelo del bug es
falso. No apliques el parche. Parálo, avisá, y averiguá por qué. Un parche aplicado sobre un
diagnóstico equivocado es la forma más eficiente de romper algo que andaba.

Recién si el robot se planta, aplicás el cambio y repetís los mismos 5 pasos, con `Yp = +4` cinco
veces y `Yp = −4` cinco veces.

### Ejemplo B — el retroceso sin timeout

**El problema, en el código:** después de patear, el arquero pasa al estado
`PATEANDO_atras_arquero`, que escribe PWM 150 a los motores 1 y 2 y 0 al motor 3
(`arquero.ino:1186-1188`). La **única** puerta de salida de ese estado es que `s1`, `s2` **o**
`s3` superen su umbral de blanco (`arquero.ino:1190`). No hay reloj. Si la línea no aparece
—sensor sucio, umbral mal calibrado para la luz de ese gimnasio, cable flojo— el estado no
termina nunca.

**Cómo provocarlo:**

1. **Ruedas al aire** (esto no es opcional: el modo de falla es exactamente "se va de la cancha").
2. Tapar los tres sensores de línea con cinta aisladora negra. **Verificá en el monitor** que
   `s1`, `s2` y `s3` quedaron **por debajo** de sus umbrales (500 / 650 / 600 en el arquero,
   `arquero.ino:50-52`). Esto no es un trámite: **medí antes de confiar en tu propio
   instrumento.** Lo único que sabemos con certeza del sensor es lo que dice el código —
   `readLine()` devuelve un `analogRead()` crudo (`zirconLib.cpp:157-170`) y el firmware trata
   *más alto* como blanco (`>= blanco1`). Cómo responde físicamente a un pedazo de cinta pegado
   encima **hay que verlo en el monitor**, no suponerlo.
3. Disparar la patada mostrándole la pelota cerca y centrada a la cámara.
4. **Criterio del test de reproducción:** las ruedas giran en reversa **indefinidamente** (contar
   10 segundos y cortar por batería).

Solo después de ver eso con tus ojos tiene sentido poner un timeout, y solo después de haber
**cronometrado cuánto tarda el retroceso normal** (llamalo T). El timeout va en T + 500 ms como
mínimo. Si lo ponés corto, corta siempre y el arquero nunca vuelve al arco: queda patrullando
adelantado, y vas a creer que rompiste otra cosa.

---

## 5. Cómo se mide en un robot que no tiene instrumentos

Este robot **no tiene encoders, ni odometría, ni ToF, ni ultrasonido**. No sabe cuánto se movió.
Casi todos sus desplazamientos son "tanto tiempo a tanto PWM": 40 ms de impulso inicial
(`arquero.ino:1022`), 350 ms de rebote (`arquero.ino:1130`), 450 ms de patada
(`arquero.ino:1165`), 1000 ms de reposicionamiento (`arquero.ino:1200`). Los pocos que no van por
reloj terminan por un **sensor** (ver blanco: `arquero.ino:1070`, `:1117`, `:1190`). Ninguno
termina por distancia recorrida, porque no hay con qué medirla.

Eso no significa que no se pueda medir. Significa que los instrumentos son otros. Son seis, y
todos usan cosas que el equipo ya tiene.

### Instrumento 1 — el cronómetro del celular

Para tiempos de **más de ~1 segundo**. Tu reacción al apretar mete un error de 200-300 ms, así
que no sirve para medir los 350 ms del rebote, pero sí para medir cuánto tarda la secuencia
completa de despeje o cuánto tarda el retroceso hasta encontrar la línea (el T del ejemplo B).

Truco para bajar el error: cronometrá **10 repeticiones seguidas y dividí por 10**. El error de
tus dedos se reparte.

### Instrumento 2 — la cinta métrica

Para todo lo que sea distancia. Es el único instrumento que mide el efecto **físico** real, y es
el que hay que usar cuando el criterio es "la patada quedó igual".

Procedimiento: marcá con cinta de papel el punto de partida de la pelota, pateá, marcá dónde
quedó, medí. Cinco veces. Anotá los cinco números, no el promedio: la **dispersión** es el dato
(si da 100, 102, 40, 98, 101, tenés un problema intermitente que el promedio te esconde).

### Instrumento 3 — filmar con el celular y contar cuadros

El mejor instrumento que tienen para tiempos cortos, y el que menos se usa.

**El mecanismo:** el celular graba a una cantidad fija de cuadros por segundo. A 30 fps, cada
cuadro es un tic de **33 ms**. A 60 fps, de **16,7 ms**. A 120/240 fps (cámara lenta), de 8,3 /
4,2 ms. Si contás cuántos cuadros pasan entre dos eventos, multiplicás por el tic y tenés el
tiempo con una precisión que el cronómetro no te va a dar nunca.

**Confirmá a qué fps graba TU celular** (está en la configuración de la cámara) — no lo asumas.

Para qué sirve acá:
- Medir los 350 ms del impulso de rebote (`arquero.ino:1130`): a 60 fps son ~21 cuadros.
- Verificar que el robot queda **realmente quieto** durante una pausa, y no "casi quieto".
- Ver en qué momento exacto cambia de sentido una rueda respecto de cuándo aparece la pelota.

Cómo se cuenta sin volverse loco: que en el cuadro haya **algo que cambie de forma visible** para
marcar el inicio y el final — sirven los LEDs de la OpenMV (instrumento 4), una marca de cinta de
color en una rueda, o tu propia mano entrando a cuadro. Después pasá el video cuadro por cuadro en
la app de fotos (en la mayoría se hace arrastrando despacio la barra de tiempo).

### Instrumento 4 — los tres LEDs de la OpenMV

El programa de la cámara ya prende un LED por cada cosa que detecta, **sin computadora**:

| LED | Qué significa | Dónde está en el código |
|---|---|---|
| rojo | ve la pelota naranja | `enviar_coordenadas_2_arcos_y_pelota.py:11` y `:125-127` |
| verde | ve el arco amarillo | `:12` y `:133-135` |
| azul | ve el arco azul | `:13` y `:129-131` |

Es el instrumento ideal para calibrar visión en cancha: apuntás la cámara y **mirás la cámara**,
no la pantalla. Y como está del lado de la OpenMV, es **independiente** del Teensy: si el LED rojo
prende pero el robot no reacciona, el problema está del cable para acá.

### Instrumento 5 — el LED del Teensy (a confirmar)

`arquero.ino:261` hace `digitalWrite(LED_BUILTIN, haypelota);` en cada vuelta del loop. O sea: si
ese LED se ve, **el robot te está diciendo si el Teensy cree que ve la pelota** — que no es lo
mismo que lo que ve la cámara, y por eso los dos LEDs juntos parten el problema en dos.

⚠️ **No damos por hecho que se vea.** `mapa-pines-teensy.md:98` dice que el pin 13 (el LED
onboard) **no está conectado en la Zircon Rev v15**. En un Teensy 4.1 el LED está soldado a la
propia placa del Teensy, pero puede quedar tapado por la Zircon montada encima. **Tarea de banco
de 2 minutos: prender el robot, mostrarle y sacarle la pelota, y mirar si algún LED del Teensy
acompaña.** Si no se ve, no sirve como instrumento; se puede cablear un LED a un pin libre (24,
28-41, `mapa-pines-teensy.md:98`), pero eso ya es un cambio de hardware y va con su propia
entrada de bitácora.

### Instrumento 6 — imprimir por Serial

El más potente y el más traicionero. Va con su propia sección, abajo.

---

## 6. El monitor serie: cómo usarlo sin arruinar la medición

Hay **dos canales serie distintos** y confundirlos es un clásico (`entorno.md:122-126`):

- `Serial` = el cable USB a la computadora. Es el que ves en el Monitor Serie del Arduino IDE.
  Se abre en `arquero.ino:238` con `BAUD_RATE = 19200` (`arquero.ino:77`).
- `Serial1` = los pines 0 y 1, que van a la OpenMV. Se abre en `arquero.ino:239`, también a 19200.
  **Ese canal no lo ves en el monitor.**

Poné el monitor en **19200**, que es el número que declara el código. (En el Teensy el `Serial`
por USB no siempre depende del baud elegido, pero no hay ninguna razón para probar suerte: si ves
caracteres raros, el baud es el primer sospechoso.)

Del lado de la cámara, la OpenMV ya imprime cada paquete que manda
(`enviar_coordenadas_2_arcos_y_pelota.py:156`): la consola del OpenMV IDE es un **segundo
instrumento independiente**. Si el Teensy no ve la pelota pero la OpenMV está imprimiendo
coordenadas distintas de cero, el problema está en el cable o en el lector, no en la visión. Ese
tipo de "partir el problema en dos" es la mitad del diagnóstico.

### La advertencia que no se puede saltear

**Imprimir cuesta tiempo, y este loop no tiene ningún `delay()`** (verificado: no hay una sola
llamada a `delay()` en los dos `.ino`). El loop corre tan rápido como lo dejen la lectura I²C del
BNO055 y las impresiones. Si metés 8 `Serial.print` por vuelta, el loop se frena, y las
transiciones que dependen de `millis()` empiezan a llegar tarde.

Eso significa que **medir puede cambiar lo que estás midiendo**. No es paranoia: es la causa más
común de "lo arreglé, y cuando saqué los prints volvió a fallar".

Las tres reglas:

1. **Nunca imprimas en cada vuelta del loop.** Imprimí cada 200 ms, o solo cuando algo cambia.
2. **Medí el ritmo del loop antes y después** de agregar prints (código abajo). Si baja mucho,
   tus mediciones de tiempo están contaminadas.
3. **Sacá los prints antes de dar un tema por cerrado**, y volvé a correr el test. Si el resultado
   cambia sin los prints, el que mandaba era el print.

### Medidor de ritmo del loop

```cpp
// ---- Variables globales (arriba, junto a las otras) ----
unsigned long loops_contados = 0;
unsigned long t_ultimo_reporte = 0;

// ---- Al FINAL del loop(): en arquero.ino va entre la llave que cierra el
//      switch (línea 1206) y la que cierra el loop (línea 1207) ----
loops_contados++;
if (millis() - t_ultimo_reporte >= 1000) {
  Serial.print("loops/seg = "); Serial.println(loops_contados);
  loops_contados = 0;
  t_ultimo_reporte = millis();
}
```

Corré esto **primero, sin ningún otro print**, y anotá el número en la bitácora. Ese número es el
ritmo base del robot y va a servir para todo lo demás. Si más adelante alguien dice "el robot está
lento", tenés con qué comparar.

### Traza de estados: solo imprime cuando cambia

Es el instrumento más útil para todo lo que sea máquina de estados, y no frena el loop porque
imprime muy de vez en cuando.

```cpp
// ---- Variable global: va JUSTO DESPUES de arquero.ino:131,
//      que es donde se declara `estado`. Si la ponés antes, no compila. ----
Estado estado_anterior = estado;

// ---- Al final del loop() ----
if (estado != estado_anterior) {
  Serial.print(millis());
  Serial.print(" ms  ->  estado ");
  Serial.println((int)estado);   // número del enum
  estado_anterior = estado;
}
```

Con eso tenés, en el monitor, la película exacta del robot con marca de tiempo: podés **restar**
dos líneas y saber cuánto duró un estado, con precisión de milisegundos y sin cronómetro.

Un `enum` de C++ es una lista de nombres a los que el compilador les asigna números **en el orden
en que están escritos**, empezando por 0. Por eso `Serial.println((int)estado)` imprime un número
y no el nombre: hay que traducirlo con esta tabla.

**Los números del enum del arquero** (declarado en `arquero.ino:114-130`; los estados del arquero
están en las líneas `116-119`):

| Nº | Estado |
|---|---|
| 0 | `impulso_inicial` |
| 1 | `moverce_izquierda` |
| 2 | `moverce_derecha` |
| 3 | `impulso_izquierda` |
| 4 | `impulso_derecha` |
| 5 | `PATEANDO_pausa_inicial_arquero` |
| 6 | `PATEANDO_adelante_arquero` |
| 7 | `PATEANDO_atras_arquero` |
| 8 | `PATEANDO_pausa_arquero` |
| 9 | `avanzar_despues_de_patear` |

⚠️ **El orden de la lista no es el orden en que se ejecutan.** La patada corre
5 → 6 → **8** → **7** → 9: pausa inicial, adelante, pausa, atrás, avanzar (mirá las asignaciones
en `arquero.ino:1157`, `:1169`, `:1179`, `:1192`). Que en la traza aparezca un 8 antes de un 7 es
lo normal, no un bug.

⚠️ **Estos números no valen para el otro archivo.** El enum de `delantero.ino:123-137` tiene los
mismos primeros estados pero **no incluye** `avanzar_despues_de_patear`, así que ahí `AVANCE_INICIO`
(`delantero.ino:129`) es el 9, mientras que en `arquero.ino:122` es el 10. De ahí para arriba se
desfasa todo, y encima el delantero agrega estados que el arquero no tiene (`APUNTAR_PELOTA_horario`,
`delantero.ino:132`). Si copiás la tabla de un archivo al otro vas a leer mal la traza y a
diagnosticar cualquier cosa. **Contá el enum del archivo que estás compilando.**

### Volcado de sensores cada 200 ms

`s1`, `s2` y `s3` se leen dentro del `loop()` (`arquero.ino:344-346`), así que este bloque va
**después** de esas líneas.

```cpp
// ---- Variable global ----
unsigned long t_volcado = 0;

// ---- Dentro del loop(), después de leer s1/s2/s3 ----
if (millis() - t_volcado >= 200) {
  t_volcado = millis();
  Serial.print("s1="); Serial.print(s1);
  Serial.print(" s2="); Serial.print(s2);
  Serial.print(" s3="); Serial.print(s3);
  Serial.print(" | pelota="); Serial.print(haypelota);
  Serial.print(" Xp="); Serial.print(Xp);
  Serial.print(" Yp="); Serial.print(Yp);
  Serial.print(" | yaw="); Serial.print(currentYaw);
  Serial.print(" err="); Serial.println(error);
}
```

Con esto podés hacer la calibración de la que **no hay ningún número anotado** (la carpeta
`futbol-roboliga2026/bitacora/` hoy tiene solo el `README.md`): los umbrales de blanco están fijos
en el código (`arquero.ino:50-52`) y no hay en el firmware ninguna rutina que los recalcule. Apoyá
el robot sobre el verde de la cancha y anotá `s1/s2/s3`; apoyalo sobre la línea blanca y anotá
otra vez. Si el valor sobre verde está cerca del umbral, la falla no es del programa: es que el
umbral está mal para la luz de esa sala. **Ese es el número que hay que llevar a la Roboliga.**

### Contador de tramas de la cámara

Una **trama** es un paquete completo de datos: acá son 9 bytes seguidos, y las posiciones 1ª, 4ª y
7ª tienen que valer 201, 202 y 203 — son las **marcas** que dicen "esto es un paquete de verdad y
está alineado". El lector del Teensy solo actualiza `Xp`/`Yp`/`haypelota` cuando esas tres marcas
dan (`arquero.ino:263-277`); del lado de la OpenMV el paquete se arma en
`enviar_coordenadas_2_arcos_y_pelota.py:149-153` y se manda en `:155`. Saber a qué ritmo llegan
esas tramas es prerrequisito de cualquier decisión sobre timeouts.

```cpp
// ---- Variables globales ----
unsigned long tramas_ok = 0;
unsigned long t_reporte_cam = 0;

// ---- Dentro del if de arquero.ino:277 (el de las tres marcas), como primera línea ----
tramas_ok++;

// ---- Al final del loop() ----
if (millis() - t_reporte_cam >= 1000) {
  Serial.print("tramas/seg = "); Serial.print(tramas_ok);
  Serial.print("   bytes en cola = "); Serial.println(Serial1.available());
  tramas_ok = 0;
  t_reporte_cam = millis();
}
```

Cómo se lee: `bytes en cola` creciendo significa que la cámara manda más rápido de lo que el
Teensy consume y estás decidiendo con datos viejos. `tramas/seg` en cero con la pelota enfrente
significa que el problema es el enlace, no la lógica.

---

## 7. Seguridad de banco

No es una sección de relleno. El firmware maneja los motores **a lazo abierto**: escribe un PWM y
no mira ningún sensor para saber si el motor hizo lo que le pidió (no puede: no hay encoders).
Nadie corrige, nadie limita. Y el `.ino` escribe los PWM con `analogWrite()` directo: la librería
tiene un tope propio (`motorLimit` = 100, `zirconLib.cpp:9`, aplicado en `motor1/2/3()`,
`zirconLib.cpp:174`), pero **ninguno de los dos programas usa esas funciones**, así que ese tope
no está actuando. Por eso la patada puede escribir 250 (`patadM1`, `arquero.ino:55`).

### El robot arranca solo al conectar la batería

**No hay botón de arranque.** Los botones existen físicamente (pines 9 y 10,
`zirconLib.cpp:256-257`) y la librería tiene `readButton()` (`zirconLib.cpp:144`), pero
**ninguno de los dos programas la llama nunca** (verificado: cero apariciones de `readButton` en
los dos `.ino`). El `setup()` termina y el `loop()` entra directo al `switch`, con
`estado = impulso_inicial` (`arquero.ino:131`), que es un impulso lateral con `PWM1 = 1.8*50 = 90`,
`PWM2 = 90` y `PWM3 = 1.8*85 = 153` (`arquero.ino:1018-1020`).

Traducción: **en el instante en que enchufás la batería, el robot se va de la mesa.**
Consecuencia práctica: se conecta la batería con el robot **ya apoyado donde tiene que estar**, o
con las ruedas al aire. Nunca con el robot en la mano.

**La única excepción, y conviene conocerla:** si el BNO055 no responde por I²C (el bus de dos
cables, pines 18/19, por el que habla el giroscopio), el `setup()` se queda colgado a propósito en
un `while (1);` (`arquero.ino:246-249`) y el robot **no mueve un motor**. O sea: "no hace
absolutamente nada al encender" **no** es síntoma de batería, es el primer sospechoso el
giroscopio (`entorno.md:145-147`). Y ojo con la trampa de banco: si el gyro está desconectado, tu
test de movimiento no está midiendo nada, está midiendo un robot colgado.

**Arranque diferido, solo para banco:**

```cpp
// ⚠️ SOLO PARA BANCO — SACAR ANTES DE JUGAR.
// Al final del setup(): frena el arranque hasta que mandes cualquier tecla
// por el monitor serie.
// OJO: si esto queda puesto en cancha y no hay USB conectado,
// el robot NO ARRANCA NUNCA.
  while (!Serial.available()) { parar(); }
  while (Serial.available()) { Serial.read(); }   // vaciar lo que mandaste
```

### Ruedas al aire para probar lógica

Todo lo que sea "¿entra al estado correcto?", "¿ve la pelota?", "¿el umbral está bien?" se prueba
con el robot **sobre un soporte, con las tres ruedas girando en el aire**. Sirve una caja, dos
libros, un rollo de cinta. Ganás dos cosas: no se cae, y podés mirar el sentido de giro de cada
rueda, que en un omni de 3 es la mitad del diagnóstico.

Lo que **no** podés probar así: nada que dependa de fricción, inercia o distancia recorrida. La
patada, el rebote de 350 ms y cualquier ajuste de PWM tienen que ir al piso.

### Tope físico para probar escapes

Cuando el test es "¿qué pasa cuando el robot no encuentra la línea?", el resultado esperado es que
se vaya. Poné un **tope físico** —un libro grueso, una caja pesada— del lado hacia el que va a
escapar, o hacé el test con las ruedas al aire. Y tené la mano en el conector de la batería.

### No agregues un "modo banco" que apague los motores

Es tentador poner un `if (MODO_BANCO) return;` antes de escribir los PWM. **No lo hagas para
probar lógica de movimiento**: estarías probando un robot distinto del que va a jugar. Las ruedas
al aire son la versión honesta de lo mismo — el firmware es idéntico, lo que cambia es la física.

---

## 8. La batería: la causa nº 1 de perder una tarde

**Cargá la batería antes de la clase, y tené la segunda cargando.**

El mecanismo, no la superstición: `analogWrite(PWM, 150)` no manda "150 de fuerza". Manda un
**ciclo de trabajo**: el driver prende y apaga el motor muy rápido, y con 150 de 255 el motor está
conectado el ~59% del tiempo. La tensión promedio que llega al motor es ese porcentaje **de la
tensión de la batería**. Batería más baja → menos tensión promedio → menos velocidad y menos par,
**con el mismo número en el código**.

Y como en este robot **no hay encoders**, nada compensa eso. Los 450 ms de patada
(`arquero.ino:1165`) son 450 ms siempre; lo que cambia es cuánto recorre la pelota. Los 350 ms de
rebote (`arquero.ino:1130`) son 350 ms siempre; lo que cambia es cuánto se corrió el robot.

**El síntoma clásico y carísimo:** ajustás un tiempo durante media hora, lo dejás perfecto, y a la
clase siguiente con la batería llena se pasa de largo. O al revés: probás un parche a las 11 de la
mañana y da 5 de 5; lo probás a las 12:30 con la batería gastada, da 1 de 5, y pasás una hora
buscando un bug de software que no existe.

**Qué hacer con esto:**

- Anotar en la bitácora, en cada test, **en qué estado estaba la batería**. Aunque sea "recién
  cargada" / "una hora de uso" / "última media hora". Si pueden medir el voltaje con un
  multímetro, mucho mejor: es el número más barato de todos.
- Si un resultado cambia y no cambiaste el código, **la batería es el primer sospechoso**, no el
  último.
- Antes de dar por cerrado un ajuste de tiempo o de PWM, **repetilo con la batería llena y con la
  batería usada**, y anotá los dos números. Un ajuste que solo da bien con la batería al 100% es
  un ajuste que va a fallar en la final.

### Sobre los pisos de PWM (concepto, no número)

Un motor con carga **no arranca** por debajo de cierto PWM: el par que genera no alcanza para
vencer su propia fricción. Por debajo de ese valor, escribir 20 o escribir 35 da lo mismo: el
motor no se mueve. A eso se le dice **piso de PWM**, y existe en todos los motores con escobillas.

Importa acá porque el firmware tiene multiplicadores que bajan mucho el PWM. **Pero atención a
qué robot le importa a cada uno** — esto se equivoca fácil:

| Dónde | PWM que sale | En qué robot corre de verdad |
|---|---|---|
| `girar()`: `100 * g` con `g = 0.3` (`delantero.ino:148-150` y `:88`) | **30** | **Solo delantero.** La única llamada a `girar()` está en `delantero.ino:444`, adentro del bloque del delantero. |
| `CENTRANDO_*`: `60 * c` con `c = 0.4` (`delantero.ino:615-617`, `:686-688`, `:32`) | **24** (y 72 el motor 3) | **Solo delantero.** |
| `aiproporcional()` / `adproporcional()`: `pd * 40` con `pd = 1` (`arquero.ino:199`, `:229`, `:87`) | **40** | **Arquero** (es el PWM más bajo que el arquero manda en sus propios estados). |

**Por qué la aclaración:** esas mismas funciones y estados están escritos también dentro de
`arquero.ino` (`girar()` en `arquero.ino:140-144`, `CENTRANDO_horario` en `arquero.ino:592-596`),
pero **nunca se ejecutan cuando compilás como arquero**: la única llamada a `girar()` en ese
archivo está en `arquero.ino:419`, que cae dentro del bloque del delantero, y el bloque del
arquero recién empieza en `arquero.ino:1014`. Es código muerto. Si medís el piso de PWM "porque
el `girar()` del arquero manda 30", estás midiendo algo que en el arquero no pasa nunca.

Si el piso real de estos motores está por encima del PWM que se manda, ese movimiento
simplemente **no ocurre**: el motor zumba y no gira.

**El piso de ESTE robot no lo sabe nadie: hay que medirlo.** Otros robots del IITA tienen pisos
medidos y velocidades tope medidas, pero **son otros motores, otra electrónica, otro peso y otra
batería — esos números no valen acá y no se copian.**

Cómo se mide, con el robot **apoyado en el piso** (con las ruedas al aire da un valor más bajo y
mentiroso, porque no hay carga): un programa aparte, que escriba un PWM fijo a **un solo motor** y
lo suba de a 5 cuando vos se lo pedís. Este es completo y copiable — va en un `.ino` **nuevo**, no
adentro del programa del robot:

```cpp
// MEDIDOR DE PISO DE PWM — sketch aparte, un motor por vez.
// Pines del ARQUERO (#define ROBOT1), motor 1: arquero.ino:38-40.
// Para los otros motores del arquero: motor 2 = 8/7/6, motor 3 = 11/12/4
// (arquero.ino:42-48). Para el DELANTERO los pines son otros: delantero.ino:14-24.
#define INA 2
#define INB 5
#define PWMPIN 3

int pwm = 0;
bool sentido = true;   // probá las DOS direcciones: no tienen por que dar igual

void setup() {
  pinMode(INA, OUTPUT); pinMode(INB, OUTPUT); pinMode(PWMPIN, OUTPUT);
  Serial.begin(19200);
  Serial.println("ENTER = subir 5 | 'r' = invertir sentido | '0' = parar");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'r') { sentido = !sentido; }
    else if (cmd == '0') { pwm = 0; }
    else if (cmd == '\n') { pwm += 5; if (pwm > 255) pwm = 255; }

    digitalWrite(INA, sentido);
    digitalWrite(INB, !sentido);
    analogWrite(PWMPIN, pwm);

    Serial.print("pwm = "); Serial.print(pwm);
    Serial.print("  sentido = "); Serial.println(sentido);
  }
}
```

Dos detalles de uso:

- **A diferencia del programa del robot, este no arranca solo:** los pines quedan en 0 hasta que
  vos mandás algo por el monitor. Igual: robot apoyado y mano en el conector de la batería, porque
  el primer ENTER lo pone en marcha.
- En el Monitor Serie del Arduino IDE, poné el final de línea en **"Nueva línea"**, si no el
  `'\n'` nunca llega y ENTER no hace nada.

No usa `zirconLib` a propósito: cuantas menos cosas haya corriendo, menos cosas pueden explicar el
resultado. La contra es que `readLine()` y compañía no están disponibles en este sketch.

El piso es el PWM en el que la rueda empieza a girar de manera **consistente**: subís, anotás,
ponés `0`, y volvés a subir hasta ese valor **5 veces**. Si arranca las 5, ese es el piso; si
arranca 3 de 5, seguís subiendo. Ese número es el piso **de ese motor, en esa dirección, con esa
batería, con ese peso encima**. Repetí para los tres motores y las dos direcciones: **no tienen
por qué dar igual**, y la diferencia entre ellos explica un montón de "se va torcido".

---

## 9. La bitácora

Ya existe y ya tiene formato: `futbol-roboliga2026/bitacora/`, una entrada por clase, archivo
`YYYY-MM-DD-tema-corto.md`. La plantilla está en `bitacora/README.md` y se usa tal cual:

```markdown
# 2026-MM-DD — <tema>

**Quiénes:**
**Robot:** arquero / delantero
**Programa cargado:** <archivo + qué #define estaba activo>

## Qué queríamos probar
Una frase. Qué esperábamos que pasara ANTES de encender.

## Qué hicimos
Qué cambiamos, con valores concretos. Un cambio por vez.

## Qué pasó de verdad
Lo que vimos, no lo que queríamos ver. Si salió mal, mejor: eso es lo que sirve.

## Números
Valores medidos, tiempos, PWM, umbrales de línea, lo que hayan anotado.

## Qué queda pendiente
Lo que no llegamos a probar, o la duda que quedó abierta.
```

Fijate que **"Qué queríamos probar" es la hipótesis** y se escribe antes. No es un resumen a
posteriori.

### Qué anotar, y por qué duele no hacerlo

- **Los números, aunque parezcan obvios.** El PWM, el umbral, el tiempo del estado, la distancia
  que recorrió la pelota. Dentro de tres semanas nadie se acuerda, y van a repetir el mismo
  experimento por tercera vez. Es el costo más caro y más invisible del taller.
- **Lo que salió mal.** Un experimento fallido bien anotado vale más que uno exitoso sin anotar,
  porque descarta un camino para siempre. Nadie va a pensar que son malos por anotar un fracaso;
  el que queda mal es el que hace perder tres clases repitiendo algo que ya se probó.
- **Con qué versión exacta.** Qué archivo, qué `#define`, qué valores. **"El arquero" no
  identifica nada.** El bloque `ROBOT1` existe en los *dos* `.ino` y no dice lo mismo (sección 1).
  Y ojo con el atajo mental de "pongo `ROBOT1` en `delantero.ino` y listo": eso te da los **pines**
  del arquero, pero el estado inicial de ese archivo sigue siendo `AVANCE_INICIO`
  (`delantero.ino:138`), así que correrías la máquina de estados del **delantero** con los pines
  del arquero. Lo que define qué robot es no es solo el `#define`: es el archivo entero.
- **El estado de la batería.** Ver sección 8.
- **Si probaste un parche de `correcciones-propuestas.md`**, anotá el resultado en la bitácora
  **y** llená su fila en la tabla de "Registro de qué se probó" al final de ese archivo. Un parche
  sin fila ahí **no está validado**, por más que lo hayan probado.

---

## 10. Quién cierra un tema

**Regla del IITA. No es una sugerencia y no se negocia.**

> **Un tema lo cierra una persona con el robot en la mano, que corrió el test y anotó el
> resultado. No lo cierra el compilador, no lo cierra la IA, y no lo cierra el que escribió el
> parche mirando la pantalla.**

Los tres estados posibles de cualquier cambio son:

| Estado | Qué significa | Quién lo puede declarar |
|---|---|---|
| **Propuesto** | Está escrito. Puede incluso compilar. | Cualquiera, incluida la IA. |
| **Probado en banco** | Se corrió el test, con criterio de aceptación, y hay números anotados. | La persona que lo probó. |
| **Adoptado** | El equipo decidió que va al programa que se lleva a competir. | El equipo, con el coach. |

Y el vocabulario que va con eso. Cuando escriban en el repo o le contesten a la IA:

- ✅ "Propuesto, falta validar en banco."
- ✅ "Probado el 12/08: 5 de 5, la pelota recorrió entre 95 y 115 cm."
- ✅ "Falló 2 de 5, no sabemos por qué. Queda abierto."
- ❌ "Anda."
- ❌ "Ya está arreglado." (si no hay una fila en la bitácora con números)
- ❌ "Compila, así que debería andar."

Esto aplica también cuando la IA les diga que algo funciona. **La IA no puede cerrar un tema.**
Si les escribe "listo, ya quedó arreglado", la respuesta correcta es pedirle el test de banco con
criterio de aceptación, no agradecer.

---

## 11. Checklist de pre-prueba (imprimir y pegar al robot)

```
ANTES DE CONECTAR LA BATERÍA
[ ] Batería cargada. La segunda, en el cargador.
[ ] Sé qué archivo cargué y qué #define estaba activo (ROBOT1 / ROBOT2).
[ ] Cargué UN solo cambio respecto de la última prueba anotada.
[ ] Escribí la hipótesis en la bitácora. Antes. Una frase.
[ ] Sé cuál es mi criterio de aceptación, con número y con "N de N".
[ ] Sé qué 3 cosas tienen que seguir dando el mismo resultado (no-regresión).
[ ] El robot está apoyado donde va a arrancar, o con las ruedas al aire.
    (ESTE ROBOT ARRANCA SOLO AL ENCHUFAR LA BATERÍA. No hay botón.)
[ ] Si el test es de escape: hay un tope físico, o ruedas al aire.
[ ] Tengo la mano en el conector de la batería.
[ ] Monitor serie abierto a 19200.
[ ] Cámara: LED rojo de la OpenMV, o su consola en el OpenMV IDE, confirmando que ve la pelota.
[ ] Giroscopio enchufado (si no, el setup() se cuelga y no se mueve NADA).

DESPUÉS
[ ] Anoté los números crudos, los 5, no el promedio.
[ ] Anoté lo que salió distinto de lo que esperaba.
[ ] Anoté el estado de la batería.
[ ] Corrí la no-regresión.
[ ] Saqué los Serial.print de diagnóstico y volví a probar.
[ ] Si probé un parche: llené su fila en correcciones-propuestas.md.
```

---

## 12. Errores típicos de banco

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| "Ayer andaba y hoy no", sin cambios de código | Batería, o luz distinta en la sala (umbrales de blanco fijos, `arquero.ino:50-52`) | Medir `s1/s2/s3` sobre verde y sobre blanco antes de tocar nada |
| "No hace absolutamente nada al encender, ni un motor" | El BNO055 no respondió y el `setup()` quedó colgado en `while (1);` (`arquero.ino:246-249`) | Revisar el conector I²C (pines 18/19). No es la batería |
| "Los valores de línea no tienen sentido" | La librería puede estar leyendo **otros pines**: `readLine()` usa A11/A13/A12 o A8/A9/A12 según lo que dé el pin 32 al arrancar (`zirconLib.cpp:52-60`, `:259-261`, `:286-288`) | Imprimir `getZirconVersion()` en el `setup()` (sección 3.2). Se espera `Mark1` |
| "Lo arreglé, y al sacar los prints volvió a fallar" | Los prints frenaban el loop y cambiaban los tiempos | Medir loops/seg con y sin prints; imprimir cada 200 ms, no cada vuelta |
| "Cambié el valor y no pasó nada" | Editaste una copia que no es la que compila (típico con `zirconLib`) | Activar salida detallada en el IDE y leer la ruta real del archivo |
| "Cambié el valor y no pasó nada" (bis) | Tocaste el bloque `#define` del robot que **no** está compilado | Confirmar qué `#define` está activo (`arquero.ino:10-11`) |
| "El bug no aparece nunca en banco" | No estás reproduciendo la condición, estás esperando que aparezca | Provocarla a propósito (sección 4) |
| "Patea a veces sí y a veces no, alineado igual" | Un chequeo de línea al final del `case` te pisa el estado (`arquero.ino:1070`) | Traza de estados: mirá a qué estado saltó realmente |
| "El robot está poseído / va hacia donde la pelota estaba" | Tramas de cámara viejas o desalineadas | Contador de tramas + `Serial1.available()` |
| "Anduvo, 1 de 1" | No es un resultado | 5 repeticiones, anotar la fracción |
| "El motor no se mueve con PWM bajo" | Piso de PWM (sección 8) | Medir el piso con el robot en el piso, los 3 motores, las 2 direcciones |
| "Probé con las ruedas al aire y en el piso hace otra cosa" | Sin carga no hay fricción ni inercia | Todo lo que dependa de física, al piso |

---

## 13. Lo que este robot NO tiene (para no perder tiempo buscándolo)

Si una IA, un doc viejo o el repo del equipo hermano les habla de esto, **no existe en este
robot**:

- **Encoders y odometría.** Ningún motor mide cuánto giró. Todo desplazamiento es tiempo × PWM.
- **ToF, ultrasonido, OTOS.** No hay sensores de distancia de ningún tipo.
- **Segunda cámara.** Una sola OpenMV, en `Serial1` (`arquero.ino:239`).
- **Solenoide / kicker.** La "patada" es un avance fuerte de las ruedas
  (`avanzar_patear()`, `arquero.ino:174-178`).
- **RTOS, FreeRTOS, micro-ROS.** Un `loop()` pelado, sin `delay()`.
- **Envs de compilación, tests host-native, simulación.** No hay ningún `platformio.ini` en este
  repo, así que **no hay "env" que elegir**: se compila el `.ino` y se carga por USB. (Si alguien
  prefiere PlatformIO en vez del Arduino IDE, `entorno.md:26-29` explica cómo; pero eso es cambiar
  de herramienta, no un entorno de build que ya exista.)
- **Múltiples placas.** Una Zircon Rev v15, un Teensy 4.1.
- **Los 8 sensores IR de pelota.** Existen físicamente (`mapa-pines-teensy.md:30-39`) y la
  librería tiene `readBall()` (`zirconLib.cpp:106`), pero **ninguno de los dos programas la llama
  nunca** (verificado: cero apariciones en los dos `.ino`). Toda la información de pelota viene de
  la cámara. Usarlos sería una mejora, no una reparación — y es un tema aparte.

Si alguien quiere agregar alguna de estas cosas, buenísimo: es un proyecto, no un parche, y
empieza igual que todo lo demás — con una hipótesis, un criterio de aceptación y una fila en la
bitácora.
