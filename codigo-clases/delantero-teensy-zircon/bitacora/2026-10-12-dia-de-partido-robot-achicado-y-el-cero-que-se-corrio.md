# 2026-10-12 — Día de partido con el robot achicado: 1-1, 2-0 y 6-1, y el cero que se corrió

**Quiénes:** la mesa del delantero *(completar nombres)* + Claude (código, análisis y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**Dónde:** otra cancha y **otra PC** (`DESKTOP-F068CC9`, usuario `Alumnos`), no la de siempre
**Programa cargado al cerrar:** `funciona/delantero/`, cargado por el equipo con
`CARGAR-ROBOT-SIN-POWERSHELL.bat`. **No sé con qué versión exacta ni con qué arco** (ver punto 1).

---

## 📋 RESUMEN DEL DÍA

| | |
|---|---|
| 🏆 **Tres partidos: 1-1, 2-0 y 6-1** | contados por el equipo |
| 🔩 **Robot achicado** | motores sobre soportes, ruedas más adentro, base un poco más alta. `L` nuevo **sin medir** |
| ✅ **Línea remedida en esta cancha** | `390/427/413` → **`335/401/403`**, más de 400 cuentas de separación en los tres |
| 🎛️ **Tres potencias bajadas por miedo al vuelco** | patada 215 → **150**, escape 170 → **135**, giro de búsqueda 50 → **40** |
| 🆕 **Regla de la vuelta** | a **1 vuelta o 10 s** sin patear, patea al rumbo cero **sin dejar de orbitar** |
| 🔴 **El cero se corrió en el partido 1** | pateó casi hacia nuestro arco. Causa **sin confirmar**. Dos protecciones escritas después |
| 📷 **La cámara no ve los arcos con esta luz** | quedó para después. Apagar el LED rojo también |
| 🛠️ **PowerShell roto en esta PC** | los `.bat` de siempre no abren. Se cargó todo el día con los de repuesto |
| ⚠️ **No sabemos qué versión jugó cada partido** | se cambiaron números entre carga y carga sin anotar |
| 🔴 **Nada está subido a GitHub** | todos los cambios de hoy viven sólo en esta PC |

---

## 0. Lo que cambió antes de empezar

Tres cosas nuevas a la vez, y las tres el mismo día del partido:

**El robot.** Para entrar en la medida que pide la competencia, los motores ahora van sobre unos
soportes que los meten más adentro: el diámetro del robot se achicó. De paso, toda la placa y la
base quedaron *"un poquito más arriba"*.

**La cancha.** Otro lugar, otra luz, otra superficie.

**La PC.** No es la de siempre. El repo se clonó de cero en
`Documents\martes1830delantero\iita-vibe-robotics-martes-SL`.

> 🔴 **`L` (del centro del robot al centro de una rueda) ya no es 8,75 cm y NO se midió.** Se pidió
> dos veces y no hubo tiempo. Todo lo que el código dice de `R = 2·L = 17,5 cm` quedó viejo hasta
> que alguien apoye una regla.

---

## 1. 🏆 Los partidos

| | resultado |
|---|---|
| Partido 1 | **empate 1 a 1** |
| Partido 2 | **ganamos 2 a 0** |
| Partido 3 | **ganamos 6 a 1** |

Lo que contó el equipo, con sus palabras:

- *"La pelota sí llega al arco"* (con la patada bajada).
- *"En un momento el 0 grados se descalibró y pateaba casi hacia nuestro arco, pero no llegaba
  por suerte."* (partido 1 — ver punto 5)
- *"La camarita, cuando está lejos o más o menos cerca, NO VE LOS ARCOS. Para mí es la
  iluminación que tiene la cancha, porque con más luz sí los ve."* (ver punto 6)

### ❓ Lo que NO sabemos, y conviene no inventar

- **Con qué valores se jugó cada partido.** El equipo cargó siempre con el `.bat`, que compila el
  `.ino` como esté en ese momento, y el `.ino` cambió ocho veces en la mañana. Yo no vi ninguna
  carga de partido: la última vez que tuve el robot enchufado tenía `grabar-linea`.
- **Lo único seguro:** el partido 1 se jugó **sin** las dos protecciones del punto 5, porque se
  escribieron después (el `.ino` quedó guardado a las 12:05 y el 1-1 se contó a las 12:00).
- **Si los partidos 2 y 3 se jugaron con esas protecciones.** Depende de si se volvió a cargar
  después de las 12:05.
- **Con qué arco** se jugó cada uno.
- **Si se levantó o volcó**, que era el miedo del día. Se preguntó dos veces y no quedó dicho.
- **Si se salió de la cancha**, y cuántas veces.
- **Si el giro de búsqueda a 40 arrancó siempre** o alguna vez se quedó zumbando.

Es el mismo agujero que quedó anotado el 21/09, y volvió a pasar.

---

## 2. ✅ La línea, medida en esta cancha

`pruebas/grabar-linea/`, 75,8 s de grabación, 10 mesetas, clasificadas solas. La cargó y la leyó
el equipo. Guardada en `mediciones/medicion-2026-10-12_11-00-07.txt`.

| sensor | verde hasta | blanco desde | separación | umbral nuevo | era |
|---|---|---|---|---|---|
| 1 derecho | 132 | 538 | 406 | **335** | 390 |
| 2 izquierdo | 123 | 680 | 557 | **401** | 427 |
| 3 delantero | 163 | 643 | 480 | **403** | 413 |

```cpp
int UMBRAL_LINEA[3] = { 335, 401, 403 };   // cancha del 12/10
```

- El verde de esta cancha es **oscuro**, parecido al de la tela (allá llegaba a 132/99/140).
- Los umbrales viejos también caían dentro del hueco; los nuevos quedan centrados. El que más
  ganó es el sensor 1, cuyo blanco flojo dio 538 (meseta 5; probablemente la línea no le quedó entera debajo).
- Se actualizó también `UMBRAL_ACTUAL` en `grabar-linea.ino`, para que su aviso *"CRUZA EL
  UMBRAL DE HOY"* no mienta.

> **Un susto que no era:** antes de medir, con el robot enchufado, los tres sensores leían blanco
> (`arranque 750/654/744`) y el programa escapaba de la línea sin parar. **Era la mesa.** Sobre el
> verde de la cancha leen 60 a 160.

---

## 3. 🎛️ Los números que se cambiaron, y por dónde pasó cada uno

Todos decididos por el equipo, **sin probar entre uno y otro**.

| constante | venía | recorrido de hoy | quedó |
|---|---|---|---|
| `VEL_PATADA` | 215 | 152 → 145 → 150 | **150** |
| `VEL_ESCAPE_FUERTE` | 170 | 150 → 140 → 135 | **135** |
| `VEL_GIRO_CONTINUO` | 50 | 45 → 40 | **40** |
| `UMBRAL_LINEA` | 390/427/413 | medido | **335/401/403** |
| `VUELTAS_PARA_RUMBO0` | — | 2 → 1 | **1** |
| `MS_ORBITA_MAX` | 20 s | → 10 s | **10 s** |

Lo que esos números le hacen a las inversiones bruscas (la cuenta es el salto de PWM de las dos
ruedas de adelante, de un instante al otro; **calculado del código, no medido**):

| cambio de estado | antes | ahora |
|---|---|---|
| patada → escape de línea (sensor delantero) | +215 → −170 = **385** | +150 → −135 = **285** |
| patada → marcha atrás (`PATEA_ATRAS`) | +215 → −110 = **325** | +150 → −110 = **260** |
| destrabe → escape de línea | +150 → −170 = **320** | +150 → −135 = **285** |
| avance → escape de línea | +95 → −170 = **265** | +95 → −135 = **230** |

> ⚠️ **Después de la patada no hay freno.** El programa pasa de empujar para adelante a empujar
> para atrás en la misma vuelta de `loop()`, sin pausa ni rampa. Es el lugar donde meter un freno
> eléctrico corto si el robot llega a levantarse. Hoy no se tocó.

> ⚠️ **El giro a 40 está cerca del piso de PWM**, que en esta superficie nunca se midió. Si alguna
> vez se queda zumbando sin girar al buscar, volver a 45 o 50.

---

## 4. 🆕 La regla de la vuelta: patear al rumbo cero SIN dejar de orbitar

**Lo que pidió el equipo:** *"en la órbita, si no ve el arco, que si da dos vueltas a la pelota y
no patea, que patee a cero grados"*. Después lo bajó a **una** vuelta, porque la cámara casi no
veía los arcos, y el reloj de respaldo de 20 a **10 segundos**.

**Cómo quedó:**

1. El giroscopio cuenta cuánto giró el robot alrededor de la pelota (una vuelta a la pelota son
   360° de rumbo). Se lee cada 20 ms, no en cada vuelta del `loop()`: es una consulta I2C.
2. A **1 vuelta** sin patear, o a los **10 s** (lo que pase primero), deja de esperar al arco.
3. **Sigue orbitando**, y patea apenas queda mirando al rumbo cero (±12°) con la pelota a menos de
   12° del frente.
4. Si en 15 s más no llega a quedar mirando al cero, vuelve a buscar.
5. Si antes ve el arco alineado, patea al arco como siempre: esa condición va primero.

**Por qué no gira en el lugar.** El plan B que ya existía (`APUNTA_RUMBO0`, del 11/08) giraba sobre
el eje hasta mirar al cero. Girando sobre el eje la pelota se le va del cuadro, y ese estado suelta
la jugada apenas deja de verla. Orbitando, la pelota queda siempre adelante: cuando el rumbo pasa
por el cero, el robot **ya está detrás de la pelota** y empujarla la manda para allá.

Perillas: `RUMBO0_ORBITANDO` (en `false` vuelve al giro en el lugar), `VUELTAS_PARA_RUMBO0`,
`MS_ORBITA_MAX`, `MS_RUMBO0_MAX`. El diagnóstico de la órbita ahora imprime `girado=` y
`BUSCA EL CERO`, y el banner dice `orbita: a las 1 vueltas (o 10 s) sin patear...`.

> 🚨 **El cero es hacia donde mira el robot AL PRENDER LA BATERÍA.** Con esta regla el cero se usa
> muchísimo más que antes, así que el ritual de arranque pesa mucho más: apoyarlo bien derecho
> mirando al arco rival, y recién ahí prenderlo.

---

## 5. 🔴 El cero se corrió en el partido 1

**Lo que se vio:** *"en un momento el 0 grados se descalibró y pateaba casi hacia nuestro arco"*.
La pelota no llegó. Es lo más grave del día: un cero equivocado convierte la regla del punto 4 en
una máquina de goles en contra.

**No está medido por qué pasó.** Hay dos causas posibles y no se pudo separar cuál fue:

- **A. La brújula.** El programa arrancaba el BNO055 con `bno.begin()` a secas, que es el modo
  `NDOF`: usa el magnetómetro para el rumbo. Al lado de tres motores la brújula se equivoca, y
  además ese modo corrige el rumbo **de golpe** cuando la brújula termina de calibrarse, cosa que
  pasa justo después de girar un rato (es un comportamiento conocido de ese modo del sensor; en
  este robot no lo medimos).
- **B. Un reinicio.** El cero se toma cada vez que el programa arranca. Si el robot se reinició en
  la cancha (una caída de tensión, o apagarlo y prenderlo) mirando para otro lado, quedó con otro
  cero. Se preguntó si lo vieron quedarse quieto unos segundos y volver a arrancar, y **no quedó
  contestado**.

**Lo que se escribió después del partido 1** (cada una con su interruptor, en el panel):

| | qué hace | para volver atrás |
|---|---|---|
| `GIRO_SIN_BRUJULA = true` | arranca el BNO055 en modo IMU (`OPERATION_MODE_IMUPLUS`): rumbo sólo con giroscopio y acelerómetro, relativo a cómo estaba al encenderse | `false` |
| `VETO_ARCO_PROPIO = true` | si la cámara vio **nuestro** arco en los últimos 300 ms, no patea al rumbo cero: sigue orbitando | `false` |

Las dos compilan. **Ninguna se probó en banco**, y no sé si llegaron a jugar (punto 1).

> ⚠️ **Cambiar el modo del giroscopio toca todo lo que usa el rumbo:** la patada derecha, el
> sentido de la órbita y el cero. Antes de confiarle un partido hay que hacer la prueba de un
> minuto: cargarlo, girar el robot a mano y ver que `rumbo=` acompañe y vuelva al mismo número.
> `pruebas/rumbo-vivo/` es para eso, pero ojo: esa prueba arranca el sensor por su cuenta con
> `bno.begin()` a secas, o sea en el modo viejo. Lo mismo hacen **todas** las pruebas que usan el
> giroscopio, `patada-derecha` incluida.

> ⚠️ **El veto sólo protege donde la cámara ve nuestro arco.** Con la luz de hoy lo ve de cerca,
> que es justo donde importa, pero de lejos no.

---

## 6. 📷 La cámara no ve los arcos con esta luz

**Lo que observó el equipo:** de lejos, y también de más o menos cerca, no los ve. Con más luz sí.

**Lo que dice el script de la cámara** (`enviar_coordenadas_2_arcos_y_pelota.py`), leído hoy:

- Pide un arco **grande**: `pixels_threshold=300` para el azul y `600` para el amarillo. De lejos
  el arco es chico; si encima sólo una parte pasa el filtro de color, no llega.
- Los umbrales de color (`azul_threshold`, `amarillo_threshold`) son los de la luz del laboratorio
  de 2025. Con menos luz los colores se apagan y dejan de entrar en el rango.
- Ganancia y balance de blancos siguen en automático (pendiente desde el 04/08).

**Tres caminos, anotados y sin hacer** (*"después del partido vemos mejor lo de la cámara"*):

1. En el robot: depender menos de ver el arco. **Es lo que se hizo** (punto 4).
2. En la cámara: bajar el tamaño mínimo del arco (azul 300 → 150, amarillo 600 → 300). A la mitad
   lo ve ~40 % más lejos. Contra: puede tomar por arco algo azul o amarillo de afuera.
3. En la cámara: medir los colores con la luz de la cancha en el OpenMV IDE y ensanchar los rangos.

### El LED rojo

Pedido del equipo: **apagar el rojo** del LED de la cámara (se prende cuando ve la pelota), porque
los robots rivales se pueden confundir y un juez puede no dejar jugar así. Decisión: *"sólo apagá
el rojo"*; el azul y el verde quedan. **No se hizo:** la cámara nunca se enchufó a la PC. Es una
línea del `main.py` que vive en la cámara (aparece como un pendrive al enchufarla por su USB).
Antes de tocarlo, guardar una copia del original.

---

## 7. 🛠️ Esta PC: PowerShell roto, y los lanzadores de repuesto

**El síntoma:** cualquier cosa que use PowerShell termina en *"Se han producido uno o varios
errores."* `CARGAR-ROBOT.bat`, `MEDIR-TUTI-ROBOT.bat` y `CALIBRAR-ROBOT.bat` son los tres de
PowerShell, así que en esta PC no abre ninguno.

**La causa, medida:** se arrancó el motor de PowerShell directo, desde un programita de prueba, y
dijo qué le falta:

```
FileNotFoundException: No se puede cargar el archivo o ensamblado
'Microsoft.Management.Infrastructure, Version=1.0.0.0, ... PublicKeyToken=31bf3856ad364e35'
```

- La carpeta de ese componente **no está** donde .NET lo busca (`C:\Windows\Microsoft.NET\assembly\GAC_MSIL\`).
- El archivo **sí está** guardado en el almacén de Windows (`C:\Windows\WinSxS\msil_microsoft.management.infrastructure_...`).
- Y no es uno solo. Con una copia de prueba de ese archivo al lado del programita, el motor
  arrancó (versión 5.1.19041.6456), pero `Get-CimInstance` siguió fallando: también falta
  `Microsoft.Management.Infrastructure.CimCmdlets`, y los scripts lo usan para buscar el robot.
  **Verificados faltan esos dos.** Una búsqueda más amplia mostró más nombres, pero muchos son de
  funciones opcionales de Windows que no están instaladas, así que no sé cuántos faltan de verdad.

**O sea que no es un programa que se baje.** Los archivos están en el disco; Windows los perdió de
su lugar. Se repara con `sfc /scannow` desde una cuenta administradora. `Alumnos` no lo es (los
administradores de esta PC son `Admin` y `Administrador`), así que **no se reparó**.

Lo demás de la PC está bien: PlatformIO 6.2.0, la plataforma Teensy y el Teensy Loader estaban
instalados, y **los 20 programas del delantero compilaron acá sin errores** (firmware en sus tres
variantes, respaldo del partido 1 y las 16 pruebas).

### Los de repuesto

| archivo | qué es |
|---|---|
| `robot-sin-powershell.py` | hace lo mismo que `cargar-robot.ps1` y `medir-robot.ps1`, con el Python que trae PlatformIO |
| `CARGAR-ROBOT-SIN-POWERSHELL.bat` | el de partido: 1 azul, 2 amarillo, mismos controles, confirma el arco por el banner |
| `MEDIR-ROBOT-SIN-POWERSHELL.bat` | menú de pruebas, `L` para leer el robot, `V` para ver qué robot hay enchufado |

El equipo pidió *"no armes otras cosas"* y arreglar los `.bat` de siempre. No se pudo por lo de
arriba, y **todo lo de hoy se cargó con los de repuesto**: `grabar-linea`, la lectura de la
medición y los programas de partido. Los `.bat` y `.ps1` originales no se tocaron.

### Dos cosas que fallaron y hay que saber

- **Desde la sesión de Claude la carga no anduvo:** `Unable find Teensy Loader. Is the Teensy
  Loader application running?` Con doble clic del equipo sí. No sé por qué.
- **El robot se cayó del USB dos veces:** Windows lo mostró como *"Dispositivo USB desconocido
  (Error de solicitud de descriptor de dispositivo)"*, en dos puertos distintos. Cambiando de
  puerto volvió. Es cable, puerto o contacto; no se llegó a averiguar cuál.

---

## 8. El vuelco: lo que se calculó y lo que NO se midió

**Calculado, suponiendo el peso centrado:** el robot se apoya en el triángulo de las tres ruedas.
Hacia adelante el borde es la línea entre las dos delanteras, a `L/2` del centro (4,4 cm con el
`L` viejo); hacia la rueda trasera hay el doble. Por eso lo que lo voltea es invertir cuando viene
yendo para adelante. Ruedas más adentro achican ese margen en la misma proporción que `L`, y la
base más alta lo empeora.

**No medido:** `L` nuevo, la altura del peso, y sobre todo **si el robot se levanta o no** con los
números de hoy. La recomendación fue mirar 3 o 4 patadas antes de bajar nada; el equipo bajó tres
potencias de entrada y jugó. Con tres resultados buenos **no sabemos cuál de los tres cambios hizo
falta**, ni si alguno sobró.

---

## 9. Números de hoy, todos juntos

```
PARTIDOS             1-1 · 2-0 · 6-1

LÍNEA (esta cancha)  verde hasta 132/123/163 · blanco desde 538/680/643
                     umbrales 335 / 401 / 403 · confirma 5 ms
                     al verla: retrocede 210 ms a 135, ciego hasta 300 ms

PATADA               150 x 500 ms · rampa +15 cada 5 ms · trim 5 · KP 4.0 · KD 0.6
                     después: marcha atrás 110 x 700 ms (sin freno en el medio)

BUSCAR               giro continuo a 40, rampa +10 cada 10 ms, freno 200 ms al verla
ÓRBITA               Xp<32 · impulso 120 x 300 ms -> crucero 67
RUMBO CERO           a 1 vuelta o 10 s sin patear · patea a ±12° del cero con la pelota a ±12°
                     tope 15 s más · giroscopio SIN brújula · veto si ve nuestro arco

LO QUE TRAÍA EL ROBOT (10:48, antes de tocar nada)
                     arco AMARILLO · patada 215 · escape 170 · umbrales 390/427/413
                     cámara 45 paq/s, 0 bytes tirados · loop 365.152 /s

PC                   20 de 20 programas compilan · PowerShell roto · Alumnos no es administrador
```

---

## ⚠️ Qué queda por VER

1. 🔴 **Qué versión y qué arco jugó cada partido.** Que lo anote quien cargó, mientras se acuerda.
2. 🔴 **Si el robot se levanta o vuelca** con la base nueva. Filmarlo de costado a ras del piso.
3. 🔴 **Por qué se corrió el cero** (brújula o reinicio), y si el modo sin brújula lo arregla.
4. **Si las dos protecciones del punto 5 llegaron a jugar.**
5. **Si el giro a 40 arranca siempre** en esta superficie.
6. **La patada medida:** el robot guarda el desvío de las últimas 5, pero hoy nunca se lo enchufó
   con la batería prendida después de un partido, así que no se leyó.

## 🔧 Qué queda por HACER — en orden

1. 🔴 **Subir todo a GitHub.** Hoy vive sólo en esta PC: el `.ino`, `grabar-linea.ino`, los tres
   archivos de repuesto, las mediciones y esta bitácora.
2. 🔴 **Medir `L` nuevo** con regla, y corregir los `17,5 cm` del código.
3. 🔴 **Probar en banco el giroscopio sin brújula** antes del próximo partido (un minuto).
4. **La cámara:** apagar el LED rojo (decidido), y ver los arcos con poca luz (caminos 2 y 3).
5. **Reparar PowerShell en esta PC** (`sfc /scannow`, cuenta administradora), o dejar anotado que
   acá se usan los `...-SIN-POWERSHELL.bat`.
6. **Revisar el cable y el conector USB** del robot.
7. 🐛 **Una trampa en los lanzadores, los de repuesto y los de siempre:** después de cargar
   `grabar-linea` preguntan *"¿Querés leer el robot ahora?"*, y decir que sí le manda la tecla que
   **corta la grabación** antes de ir a la cancha. Hoy no mordió. Hay que sacar esa pregunta para
   las pruebas que graban solas.
8. **Los comentarios `SIN PROBAR`** que quedaron en el `.ino` al lado de cada número de hoy:
   cambiarlos por lo que se vio, cuando se sepa qué jugó.
9. Siguen de antes: `pruebas/tabla-camara/`, el piso de PWM, `calibracion-quieto` y
   `calibracion-moviendo` (nunca corridos), y `MEJORAS-PENDIENTES.md` vencido.

## 📋 Para la próxima competencia

- **Probar la PC el día antes:** doble clic en `CARGAR-ROBOT.bat` y que abra el menú. Hoy se
  descubrió que no abría con el partido encima.
- **Medir la línea apenas se llega.** Con `grabar-linea` fueron menos de 5 minutos, de punta a punta.
- **El ritual del cero:** robot derecho mirando al arco rival, y recién ahí la batería. Cada vez
  que se lo prende, incluido en el entretiempo.
- **Antes de cada partido, tres cosas en un papel:** qué versión está cargada, qué arco, y después
  qué se vio. Segunda vez que falta.

---

## Errores míos de hoy, anotados

1. **Dije varias veces "lo cargo yo desde acá" y no pude.** La carga desde mi sesión falló por el
   Teensy Loader. Todas las cargas las hizo el equipo.
2. **Me pisé con el equipo en el puerto serie.** Un lector mío quedó colgado con el puerto tomado
   mientras el equipo usaba el menú. Con el robot enchufado, **una sola mano por vez**.
3. **Leí el robot 30 segundos después de que el equipo ya lo había leído** y recibí cero bytes:
   `grabar-linea` vuelca una sola vez. La medición estaba a salvo porque el lector guarda siempre
   en `mediciones/` antes de mostrar. Esa regla del 08/09 hoy salvó la medición.
4. **Insistí seis veces con "¿azul o amarillo?"** cuando el flujo de partido ya lo resuelve: el
   arco se elige en el menú del `.bat`, después del sorteo. La pregunta sobraba.
5. **Dije "los 21 programas compilan" y son 20:** conté `grabar-linea` dos veces.
6. **Dije que la PC podía dar de baja sólo un archivo y después "varios".** Verificados son dos
   (punto 7). No sé cuántos más.

---

## Nota de método

**Lo que salió bien fue lo que ya estaba armado para apurarse.** `grabar-linea` clasificó sola y
entregó los tres umbrales en un minuto y medio de cancha. El panel de control dejó hacer ocho
tandas de cambios sin buscar nada. El lanzador de partido confirmó el arco por el banner. Ninguna de
esas tres cosas se hizo hoy: se hicieron en clases tranquilas, pensando en un día como éste.

**Lo que salió mal es lo de siempre: se cambió más rápido de lo que se anotó.** Tres potencias, la
regla de la vuelta, el reloj y dos protecciones, todo en una mañana y sin una sola prueba entre
medio. Con 1-1, 2-0 y 6-1 el conjunto anduvo. Pero no sabemos qué parte del conjunto hizo el
trabajo, y eso es justo lo que hay que saber para el próximo partido.

**Y una regla nueva que deja el día:** un plan B que se usa poco puede tener un defecto escondido
durante meses. El rumbo cero existe desde el 11/08 y nunca había pateado al revés, porque casi
nunca se disparaba. Hoy, con la cámara sin ver los arcos, pasó a dispararse en cada jugada, y el
defecto apareció en el primer partido. **Cuando un respaldo pasa a ser el camino principal, hay
que probarlo como camino principal.**
