# 2026-10-12 (lunes) — Motores más adentro, `ir-al-medio` entra al partido, y la patrulla

**Quiénes:** el equipo (cancha, partido, y **dos datos de la cancha que cambiaron el programa**)
+ Claude (código, carga y revisiones)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas nuevos:** `pruebas/probar-ruedas`
**Modificados:** `funciona/seguir-y-despejar`, `pruebas/ir-al-medio`, y cuatro `platformio.ini`
**PC:** otra, la del aula (usuario `Alumnos`). Ver §1.

---

## 🎯 EL RESUMEN DEL DÍA

Segundo día de amistosos. **Cuatro partidos: 1-0, 1-1, 2-0 y 6-1** (§7). Cuatro cosas:

**1. Los tres motores se metieron más adentro** con soportes impresos en 3D, para entrar en el
diámetro del reglamento. El equipo probó y dijo *"anda como la clase pasada"*. No se midió nada
todavía (§2).

**2. Se buscó por qué el robot oscila con la pelota lejos.** La hipótesis, a la que llegaron
cuatro análisis por separado: **persigue su propio giro**. Se apagó la predicción para probarlo.
**Falta que el equipo diga si dejó de oscilar** (§4).

**3. `ir-al-medio` entró al programa de partido**, como arranque: si encuentra la línea del área
avanza un poquito; si no, hace la rutina. Y el equipo aportó el dato que faltaba: **retrocediendo
el robot no sube la rampita del borde**, así que el fondo ahora se mide por tiempo (§5 y §6).

**4. El juez volvió a sacar al robot por quedarse quieto** (ya había pasado el 21/09). Se
escribió la patrulla que estaba pendiente desde ese día (§8).

---

| | |
|---|---|
| §1 | La PC del aula: PlatformIO estaba, pero trabado |
| §2 | Los motores más adentro |
| §3 | La carga que se cortó, y el botón blanco |
| §4 | 🎯 **La oscilación con la pelota lejos: el robot persigue su propio giro** |
| §5 | El arranque nuevo: avanzar un poquito, o ir al medio |
| §6 | 🎯 **La rampita: el fondo se mide por tiempo** |
| §7 | Los partidos: 1-0, 1-1, 2-0 y 6-1 |
| §8 | La patrulla |
| §9 | Errores del día (míos) |
| §10 | Números de hoy, y qué quedó cargado |
| §11 | Qué queda pendiente |
| §12 | Lo que se aprendió del método |

---

## 1. La PC del aula: PlatformIO estaba, pero trabado

Se trabajó en una PC distinta de la de siempre. Lo que hubo que resolver, por si vuelve a pasar:

- **`pio` no está en el PATH.** Se llama con la ruta completa:
  `C:\Users\Alumnos\.platformio\penv\Scripts\pio.exe`. Que el comando `pio` "no exista" no
  quiere decir que falte.
- **Todo comando `pio` moría con `HTTPClientError`.** La causa: el **Avast** de esa PC re-firma
  todo el tráfico HTTPS, Windows confía en ese certificado, y el Python de PlatformIO trae su
  propia lista y no lo conoce. Se arregló instalando `pip-system-certs` **dentro** del entorno
  de PlatformIO, para que use los certificados de Windows. **No se desactivó ninguna
  verificación.** A `git` le pasaba lo mismo: en este clon quedó `http.sslBackend=schannel`.
- **Tres sketches no declaraban las librerías del giroscopio** en su `platformio.ini`
  (`probar-giroscopo`, `ver-arcos`, `funciona/despeje-pelota`): en la máquina del profe compilan
  porque las tiene instaladas aparte. Se les agregó `lib_deps`. Y `giro-imu` tenía un carácter
  invisible (BOM) al principio del `.ini` que PlatformIO 6.2 no acepta.
- **Con eso compilan los 27 proyectos de la carpeta del arquero.**
- Arduino IDE hay dos: el **1.8.19 tiene Teensyduino**; el 2.1.0 no tiene el soporte de Teensy.
  Se instalaron las librerías de Adafruit para esa ruta, pero **no se probó compilando**.
- `pruebas/herramientas/robot.ps1` **no sirve en esa PC**: tiene rutas de otra máquina.

**Con qué se compiló hoy** (el `entorno.md` pide anotarlo): PlatformIO Core 6.2.0, plataforma
`teensy` 5.0.0, `framework-arduinoteensy` 1.159.0, Adafruit BNO055 1.6.4, Unified Sensor 1.1.15,
BusIO 1.17.4.

---

## 2. Los motores más adentro

**Qué cambió:** los tres motores se corrieron hacia el centro del robot con soportes impresos en
3D. La placa base, los sensores de línea, la cámara y el giroscopio son los mismos y están
donde estaban. El equipo cree que el robot **quedó un poco más alto**.

**Qué se sabe:** el equipo lo probó con el programa de partido y dijo *"anda como la clase
pasada"*.

**Qué NO se sabe, porque no se midió nada:**

- cuánto se metieron las ruedas (ni la distancia al centro de antes ni la de ahora);
- si las tres se metieron lo mismo (de eso depende que la mezcla 50/50/89 siga yendo derecho);
- la altura de la cámara (era ~8 cm) y lo que leen los sensores de línea sobre el verde y el
  blanco (eran 56–95 y ~755, con el umbral en 425);
- el piso de arranque de los motores (era ~70).

Lo único seguro por geometría: **con las ruedas más cerca del centro, la misma corrección de
rumbo hace girar más al robot.** Nadie tocó un número y el lazo de rumbo quedó más fuerte.

Se cargó `monitor-robot` y se abrió el tablero para mirar los tres números, pero **no quedó
anotada ninguna lectura**.

**`pruebas/probar-ruedas` (nuevo).** No había ningún sketch que moviera una rueda por vez. Este
no arranca solo: teclas `1` `2` `3` mueven cada rueda, `a` `d` `g` avanzan, van de costado y
giran, y `+` `-` cambian la potencia para buscar el piso. **Compila; no se probó en el robot.**

---

## 3. La carga que se cortó, y el botón blanco

Una carga falló a mitad de camino (`error writing to Teensy`) y **el Teensy desapareció del
USB por completo**: Windows no lo veía ni como dispositivo desconocido.

Lo que pasaba: con el programa a medio escribir, **el Teensy no se presenta solo a la PC**. Hay
que apretar una vez **el botón blanco del Teensy** (no los de la Zircon): se prende un LED rojo
y la PC hace el sonido. Mientras tanto se dejó el cargador corriendo en espera, y escribió el
programa en el momento en que apareció.

Dos cosas más que sirven:

- Ese botón **frena el programa** y deja al Teensy esperando una carga. No tocarlo "por las
  dudas" con el robot listo para jugar.
- Si el cargador quedó esperando y el robot se enchufó después, se lo puede mandar al modo carga
  desde la PC, sin botón: `tool-teensy\teensy_reboot.exe -s`.

**No se supo por qué se cortó la primera vez.** Se sospecha del cable o del hub USB.

---

## 4. 🎯 La oscilación con la pelota lejos: el robot persigue su propio giro

**Lo que contó el equipo:**

> *"cuando la pelota está lejos el robot empieza a oscilar, es como que le cuesta quedarse
> quieto, se corrige y se pasa para un lado de largo, nota que se pasó y corrige hacia el otro
> lado... después cuando despeja, también oscila demasiado pero aun así mantiene el frente
> bien. Solo que sale muy improlijo y le erra a la pelota."*

Es lo que había quedado abierto el 06/10 (§11 de esa bitácora), ahora con una pista nueva:
**pasa cuando la pelota está lejos.**

### La hipótesis

Cuatro análisis independientes llegaron a lo mismo por caminos distintos:

1. **La cámara va montada en el robot.** El programa saca la "velocidad de la pelota" restando
   dos fotos. No puede saber si se movió la pelota o si el que se movió o giró fue el robot.
2. Desde el 06/10 esa velocidad se multiplica por **0,5 s** para adelantarse (`msAnticipacionSeguir`).
3. **Girar el robot es como mover una linterna:** 1 grado corre la pelota **0,5 cm** en la imagen
   si está a 30 cm, pero **1,75 cm** si está a 1 m. Más que la zona muerta de 1,4 cm.
4. El círculo: arranca de costado → se tuerce un par de grados → el giroscopio lo endereza → la
   cámara lee ese enderezarse como *"la pelota cruzó rápido"* → orden fuerte para el otro lado.

**No está corrigiendo brusco a la pelota. Está persiguiendo su propio giro.**

Otro número que salió de las cuentas: fuera de la zona muerta la fuerza mínima es 45, y de
costado eso son **22 en las dos ruedas de adelante** (debajo de su piso) y **40 en la de
atrás**. Con correcciones chicas empuja casi solo la de atrás, y el robot colea en vez de
correrse.

### Lo que se hizo

**Se apagó la predicción** (`predecirTrayectoria = false`). Es una prueba de sí o no: si deja de
oscilar, era eso.

🔴 **FALTA EL RESULTADO.** El robot jugó y ganó con la predicción apagada, pero **nadie anotó si
con la pelota lejos y quieta dejó de ir de lado a lado.** Es lo primero para preguntar.

### El despeje desprolijo

Durante la salida **no hay nada que mueva al robot de costado** (la persecución y la diagonal
están apagadas): solo trabaja el giroscopio. Por eso *"mantiene el frente bien"*. Lo desprolijo
tiene dos sospechosos y el código solo no los separa:

- dispara **en pleno vaivén** y sale resbalando de costado;
- el lazo de rumbo quedó más fuerte con los motores más adentro (§2).

**La prueba que decide: un despeje desde parado.** Robot quieto, tapar la cámara, pelota quieta
y centrada a 20–25 cm, destapar. Si sale derecho, era el vaivén. Si igual sale meneando la
nariz, es el rumbo.

### Lo que quedó sin tocar, para después y de a uno

| Cambio | Para qué | Por qué no hoy |
|---|---|---|
| `msAnticipacionSeguir` 500 → 250 | recuperar algo de adelanto | es el paso intermedio, si se extraña la predicción |
| `ZONA_MUERTA_PELOTA` 1,4 → 3,0 | que un giro chico no lo saque de la zona quieta | número a ojo; solo encima del anterior |
| `KP_RUMBO` más bajo | compensar los motores más adentro | **necesita la regla**: sin las dos medidas no se toca |
| Descontar el giro con el giroscopio | el arreglo de fondo | depende de un signo; hay que comprobarlo en el tablero antes |

---

## 5. El arranque nuevo: avanzar un poquito, o ir al medio

**Pedido del equipo:** *"cuando comience el partido, si el robot detecta la línea blanca del área
apenas avance un poco, pero si no la ve entonces que haga lo de volver al medio."*

Al armarse el robot sigue retrocediendo hasta la línea del área, como antes. Cambia lo de después:

| | Antes | Ahora |
|---|---|---|
| **Encuentra la línea** | se endereza y se queda encima | se endereza y **avanza ~10 cm** |
| **No la encuentra** en 1,2 s | se queda donde frenó | hace **la rutina de `ir-al-medio`** |

La rutina se portó al programa de partido con los mismos pasos y los mismos números de la
prueba. Diferencias a propósito:

- **No usa `delay()`** entre pasos, para no dejar de leer la cámara.
- **Si no cierra, no se queda clavado** avisando con el LED: se endereza y juega desde donde
  quedó.
- **Sin giroscopio no arranca**: de costado y sin rumbo el robot gira solo.

Una decisión mía que conviene que el equipo confirme: *"no la ve"* quedó como **"retrocedió 1,2 s
y no apareció"**, no como "no la ve en el instante de armarse". Así se puede seguir apoyando el
robot un poco adelante de la línea, que es la costumbre.

Dos interruptores, uno por cosa: `avanzarAlUbicarse` e `irAlMedioSiNoHayLinea`.

**Tres revisores lo miraron** (fidelidad con la prueba, máquina de estados, partido). No
encontraron nada que rompa el juego. De lo que señalaron se corrigió:

- el avance del arranque arrastraba el enfriamiento del despeje: **1,3 s sin mirar la pelota en
  cada encendido**. Ahora pasa directo a esperar;
- si al frenar el retroceso termina parado sobre una línea, la da por encontrada;
- la rutina mira que el giroscopio conteste **en ese momento**, no solo al armarse.

Y una advertencia que quedó en pie: **mientras hace la rutina el arco queda vacío unos 15 s.**
Bien apoyado (centrado, 5 a 15 cm adelante de la línea), la rutina no corre nunca.

---

## 6. 🎯 La rampita: el fondo se mide por tiempo

**El dato lo trajo el equipo**, y no estaba en ningún archivo:

> *"en los bordes de la cancha hay rampas que es para que la pelota vuelva a la cancha y el
> robot no la puede subir y por eso no detecta la línea... no puede subir por la rueda trasera
> pero con las otras sí."*

O sea: **de costado sí sube** y ve las líneas laterales; **retrocediendo no**, y los sensores de
atrás nunca llegan a pisar la línea del fondo. El paso 3 de la rutina (*"atrás hasta ver la
línea del fondo"*) se quedaba esperando hasta el tope.

**El arreglo:** el paso 3 termina con **lo que pase primero**, ve la línea o pasan
`MS_MEDIO_FONDO_POR_TIEMPO` = **2000 ms**. Si se cumple el tiempo, se da por hecho que está
apoyado contra la rampa —que es una marca física tan buena como la línea— y la rutina sigue.

⚠️ **Los 2000 ms son a ojo, no una medición.** Alcanzan para unos 80 a 110 cm. Para afinarlo:
cronometrar cuánto tarda en tocar la rampa desde el lugar más lejano donde se lo vaya a apoyar,
y sumarle medio segundo.

El cambio está **en los dos lados**: `funciona/seguir-y-despejar` y `pruebas/ir-al-medio`.

La pregunta que había quedado el 15/09 (*"la rampita de los bordes: no se investigó. ¿Se le
puede subir el robot?"*) quedó contestada: **de costado sí, para atrás no.**

---

## 7. Los partidos: 1-0, 1-1, 2-0 y 6-1

Los resultados, como los dictó el equipo al final del día:

| Partido | Resultado | |
|---|---|---|
| 1 | **1-0** | |
| 2 | **1-1** | el gol que nos hicieron fue **en contra, de nuestro delantero** |
| 3 | **2-0** 👏 | el que se festejó. El juez sacó al arquero por quedarse quieto |
| 4 | **6-1** | |

🎯 Leídos con nuestros goles primero (así se contó el 2-0): **tres ganados y un empate, y en
cuatro partidos al arquero le hicieron un solo gol de un rival** (el del cuarto); el otro fue en
contra.

**Con qué programa se jugó cada uno:** solo está claro el tercero. Se jugó con el arranque nuevo
(rutina prendida), el fondo por tiempo y la predicción apagada, que se había cargado justo
antes. Del primero y el segundo no quedó anotado. El cuarto fue después de cargar la patrulla,
así que lo más probable es que se haya jugado con ella (la versión sin corregir), pero **nadie
lo confirmó**.

Lo que contó el equipo después del tercero:

> *"ahora el robot cuando no ve la pelota hace el movimiento pero se quedaba quieto para siempre
> y el juez pensó que se había roto y lo sacó."*

Es exactamente lo del 21/09. Esa vez quedó pedida la patrulla y no se llegó a cargar.

**Falta anotar:**

- contra quién fue cada partido;
- qué pasó en la cancha con la oscilación (§4) y con la rutina (¿terminó en el medio?);
- en el cuarto: **¿la patrulla salió? ¿el juez lo volvió a sacar?**

---

## 8. La patrulla

**Pedido del equipo:** *"que cuando termine de hacer el movimiento para volver al medio, después
de 15 segundos se vaya hacia un lateral y vuelva al medio por tiempo."*

**Cómo quedó.** Esperando, sin pelota a la vista, **15 s sin moverse**:

    va al lateral izquierdo  ->  vuelve al medio por tiempo (2228 ms)
        ->  retrocede hasta la linea del area  ->  se endereza y avanza el poquito

Son los pasos 5, 6 y 7 de la rutina de ir al medio. Sirve para dos cosas: **el juez ve que está
vivo**, y el robot **vuelve a quedar centrado** (el seguimiento lo va corriendo y nada lo traía).

Se armó mirando los seis problemas que la revisión del 21/09 había dejado anotados (la tabla
describe la versión corregida, la del archivo):

| Problema del 21/09 | Hoy |
|---|---|
| 1. Sin giroscopio gira y se va de la cancha | ✅ sin giroscopio **no patrulla** (pero entonces se queda quieto: ver pendientes) |
| 2. Pisa otra línea antes del lateral y "vuelve al medio" hacia el borde | ⬜ **sin tocar**: depende de la cancha. Es lo primero a mirar en la prueba |
| 3. Si no ve la línea, volvía el tiempo entero | ✅ al cumplirse el tope vuelve **solo el tiempo del medio** |
| 4. Una pelota que aparece de a ratos lo deja quieto igual | ✅ el reloj cuenta desde que **se movió**, no desde que vio la pelota |
| 5. Pelota pegada al frente (la cámara manda 0) y se va en plena disputa | 🟡 a medias: pide 3 s seguidos sin pelota además de los 15 |
| 6. Se va corriendo en profundidad | ✅ **cada** patrulla termina en la línea del área |

Además: **si aparece la pelota 5 cuadros seguidos, corta la patrulla y la sigue.** Y los
laterales, igual que el fondo, ahora terminan **por tiempo** si no ven la línea (6 s), en vez de
abandonar y dejar al robot estacionado en el borde. Eso vale también para la rutina del arranque.

**El costo:** cada patrulla dura 7 a 8 s, y unos 5 el arco queda descubierto. Se repite cada
~22 s. Si el juez aguanta más de 15 s quieto, conviene subir `MS_QUIETO_PARA_PATRULLAR`.

**Plan B, con un interruptor:** `patrullaPorElLateral = false` hace una patrulla corta: retrocede
a la línea del área, se endereza y avanza el poquito. Dura ~1,3 s y **no descubre el arco**; lo
que se pierde es que no lo vuelve a centrar de costado.

**Tres revisores también la miraron.** Nada que cuelgue al robot. Encontraron dos cosas
importantes, ya corregidas en el archivo:

- el reloj seguía corriendo **mientras el robot miraba la pelota quieto**: si estaba 15 s
  alineado y un rival se la tapaba un instante, salía al lateral en el acto;
- un reflejo naranja de **un solo cuadro** reiniciaba los 15 s sin que el robot se moviera.

🚨 **OJO: la versión con esas dos correcciones NO es la que quedó en el robot.** Ver §10.

---

## 9. Errores del día (míos)

1. **Dije que faltaba PlatformIO y estaba instalado.** Solo había mirado el PATH. Lo mismo con
   el Arduino IDE: dije que no había y había dos. **Antes de decir que algo falta hay que
   buscarlo de verdad** (menú Inicio, registro, carpetas del usuario).
2. **Cargué `monitor-robot` sin poder saber qué versión tenía el robot.** El programa de partido
   no imprime nada por USB y su cartel decía "ESTABLE 0.0929" igual antes y después del 06/10.
   Desde hoy el cartel cambia con cada versión (§10).
3. **El primer arranque nuevo que cargué tenía 1,3 s de más sin mirar la pelota.** Reusé la fase
   del empujoncito del despeje sin ver que al terminar pasa por el enfriamiento. Compilaba y
   andaba; lo encontró la revisión, leyendo.
4. **La patrulla se cargó antes de que terminara su revisión**, a pedido y con el apuro del
   partido. La revisión después encontró dos cosas importantes. Es justo lo que la bitácora del
   21/09 advierte: *"un cambio sin probar en un partido es una apuesta"*.
5. **Hoy se cargaron varios cambios juntos** (arranque, fondo por tiempo, predicción apagada), y
   la regla del equipo es uno por vez. Como son cosas que se ven en momentos distintos se pueden
   separar mirando, pero si algo anda raro no va a ser obvio cuál fue.

---

## 10. Números de hoy, y qué quedó cargado

**Ninguno de estos números está medido en cancha. Son cuentas con los números del código.**

| Qué | Valor |
|---|---|
| Lo que corre la pelota en la imagen por cada grado que gira el robot | **0,52 cm** a 30 cm · **1,75 cm** a 1 m |
| Zona muerta del seguimiento | 1,4 cm → a 1 m alcanzan **0,8°** para salir |
| Salto del adelanto al cruzar el piso de velocidad | 4 cm/s × 0,5 s = **2,0 cm** (más que la zona muerta) |
| Fuerza mínima de costado (45) repartida en las ruedas | **22 / 22 / 40** |
| Enfriamiento que se le sacó al arranque | **~1,3 s** |
| Rutina de ir al medio, completa | **~15 s** típico |
| Una patrulla | **7 a 8 s**, ~5 con el arco descubierto, cada ~22 s |
| Contraste del README y las bitácoras del 29/09 y 06/10 contra el código | 27 afirmaciones: **17 confirmadas, 8 a medias, 2 sin poder verificar, 0 falsas** |

### Lo que cambió en `funciona/seguir-y-despejar`

| | Antes de hoy | Ahora |
|---|---|---|
| `predecirTrayectoria` | true | **false** — para probar (§4) |
| `avanzarAlUbicarse` | (no existía: se quedaba en la línea) | **true** |
| `irAlMedioSiNoHayLinea` | (no existía: se quedaba donde frenó) | **true** |
| `MS_MEDIO_FONDO_POR_TIEMPO` | (en la prueba: tope de 4000 ms y FALLO) | **2000** — a ojo |
| Laterales sin línea, a los 6000 ms | (en la prueba: FALLO) | **sigue por tiempo** |
| `patrullarSinPelota` | (no existía) | **true** |
| `MS_QUIETO_PARA_PATRULLAR` | — | **15000** |
| `MS_SIN_PELOTA_PARA_PATRULLAR` | — | **3000** |
| `CUADROS_PARA_CORTAR_PATRULLA` | — | **5** |
| `patrullaPorElLateral` | — | **true** (false = patrulla corta) |

Los números de la rutina (110 / 100 / 400 / 943 / 2228 / 6000) son los de `pruebas/ir-al-medio`,
**medidos el 06/10, antes de mover los motores**.

### 🚨 Lo que quedó cargado en el robot

El cartel que imprime al arrancar dice qué versión es:

| Cartel | Qué tiene | Estado |
|---|---|---|
| `0.1012 EN PRUEBA (ubicacion nueva)` | arranque + rutina + fondo por tiempo + predicción apagada | **la que ganó 2-0**. No quedó guardada aparte |
| `0.1012 EN PRUEBA (ubicacion nueva + patrulla)` | lo anterior + patrulla **sin** las dos correcciones | 🚨 **LA QUE ESTÁ EN EL ROBOT** al escribir esto |
| `0.1012 EN PRUEBA (ubicacion nueva + patrulla corregida)` | lo anterior + las correcciones de la revisión | es la del archivo. **Compila; nunca se cargó** |

🧊 La copia congelada `respaldos/estable-0.0929/` no se tocó. Tampoco se probó con los motores
nuevos.

---

## 11. Qué queda pendiente

### 🔴 Lo primero

- 🔴 **Cargar la patrulla corregida y probarla en cancha.** Sin pelota, dejarlo hacer **tres
  patrullas seguidas**: tiene que frenar en seco al pisar la línea del lateral (no quedarse
  empujando la pared) y terminar las tres veces en el medio. Si alguna "vuelve al medio" y queda
  corrido a la derecha, pisó otra línea antes del lateral (problema 2 del 21/09).
- 🔴 **Decir si dejó de oscilar con la predicción apagada** (§4). Pelota quieta a 1 m, 15–20 cm
  al costado: contar cuántas veces cambia de lado. Repetir a 40 cm.
- ⬜ **El despeje desde parado**, que separa "hereda el vaivén" de "el rumbo quedó fuerte".
- ⬜ **Probar la rutina del arranque** apoyándolo lejos de la línea: ¿termina en el medio?

### Usar el robot con este programa

- ⚠️ **Batería prendida = robot en el piso de la cancha.** Sobre la mesa los sensores leen
  blanco: al armarse avanza 10 cm, y a los 15 s sale de costado hasta 6 s, y lo repite.
- ⚠️ **Si el LED tiembla al prender no hay giroscopio y no va a patrullar.** Apagar, esperar
  10 s y prender de nuevo antes del saque.
- ⚠️ **Si el juez lo saca y lo devuelve**, apagarlo y prenderlo con la llave.

### Medir, con los motores nuevos

- ⬜ **La regla:** del centro del robot al apoyo de cada rueda (las tres), y a los agujeros
  viejos de la placa. Sin esos dos números no se toca `KP_RUMBO`.
- ⬜ **El tablero** (`monitor-robot`): sensores de línea sobre el verde y el blanco, y la pelota a
  30 cm del frente (tiene que marcar ~30). Dice si el robot quedó más alto.
- ⬜ **`probar-ruedas`:** que cada motor esté en su conector, y el piso de arranque nuevo.
- ⬜ **`medir-ancho`:** si los 2228 ms siguen valiendo.
- ⬜ **Cuánto tarda en tocar la rampa del fondo**, para cambiar los 2000 ms a ojo por un número.
- ⬜ **El diámetro del robot y el reglamento de la Roboliga 2026.** No está en el repo: lo único
  que hay es el límite genérico de RoboCupJunior (22 cm, 1 kg).

### Del código

- ⬜ **Sin giroscopio no patrulla**, y entonces el juez lo vuelve a sacar. Falta un movimiento
  corto que no dependa del rumbo.
- ⬜ **Después de un corte queda corrido del medio** hasta la patrulla siguiente. Idea: una
  espera más corta cuando la última patrulla fue cortada.
- ⬜ Los cuatro cambios de la tabla de §4, de a uno.
- ⬜ El encabezado de `seguir-y-despejar.ino` todavía dice que el cable del giroscopio está roto
  y que el 06/10 se tocaron dos números (fueron cinco). Y el `README.md` dice umbral 620 (es 425).

### Del repo

- ⬜ **Todo lo de hoy quedó en la rama `robot/arquero`, no en `main`.** Es código que jugó un solo
  partido y una patrulla sin probar: que el profe lo mire antes de pasarlo a `main`. Para
  seguir trabajando en otra PC: `git switch robot/arquero` después de clonar.

### De antes

Sigue en pie todo lo de la lista del [06/10](2026-10-06-el-delay-escondido-y-la-prediccion.md)
que no se nombra arriba.

---

## 12. Lo que se aprendió del método

**El dato que destrabó la rutina no estaba en el código ni en ninguna bitácora: estaba en la
cancha.** El robot no sube la rampita para atrás. Nadie lo había escrito, y el programa esperaba
una línea a la que no se puede llegar. **Lo que se ve en la cancha hay que anotarlo aunque
parezca obvio**, porque el que escribe el código no lo ve.

**"Por tiempo" es una respuesta válida cuando hay una pared.** Si el robot empuja contra el
borde durante un rato, está en el borde, haya visto la línea o no. El mismo razonamiento del
fondo sirvió después para los laterales.

**El cartel de versión tiene que cambiar cada vez que se carga algo distinto.** Hoy hubo tres
programas diferentes con casi el mismo nombre en la misma tarde, y sin el cartel no hay forma de
saber cuál está en el robot.

**Cuatro miradas separadas valen más que una larga.** La causa de la oscilación la encontraron
cuatro análisis que no se veían entre sí, cada uno mirando otra cosa (el lazo, la geometría, el
despeje, la historia). Que coincidan por caminos distintos es lo que da confianza; y aun así
**sigue siendo una hipótesis hasta que el robot la confirme en la cancha.**

**Y lo de siempre: nada funciona porque compila.** De lo de hoy, lo único probado en la cancha
es el arranque nuevo, y lo probó un partido.
