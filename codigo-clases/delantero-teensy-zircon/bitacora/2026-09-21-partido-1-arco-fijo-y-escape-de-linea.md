# 2026-09-21 — Primer partido (5-0), un programa por arco, y por qué se sale en las esquinas

**Quiénes:** la mesa del delantero *(completar nombres)* + Claude (código, carga al robot y análisis)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**Programa cargado al cerrar:** `funciona/delantero/` compilado con **`-e azul`** (ARCO AZUL fijo),
cargado con `CARGAR-ROBOT.bat` y confirmado por el banner.

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🏆 **Tres partidos: 5-0, 3-1 y 3-2** | el 1 esta contado aca; el 2 y el 3 se agregaron el 22/09, en la seccion 7 |
| 🏆 **Primer partido: ganamos 5 a 0** | jugando junto al arquero de la otra mesa, contra un robot con ultrasonido, ToF, varias cámaras y pantallita |
| ✅ **Patada de vuelta a 215** | se terminó el modo prueba lenta del 15/09 (estaba en 110) |
| ⏱️ **Arranque 1 s más rápido** | la cuenta regresiva final pasó de 3 s a 2 s |
| 🔴 **Se salió de la cancha ~3 veces** | un minuto de penalización cada una. Se levanta al ver la línea, y en las esquinas cruza la otra |
| 🔧 **Escape de línea retocado** | retroceso 250 → 210 ms, velocidad 200 → 170, ventana sin mirar la línea 1000 → 300 ms |
| 🎯 **Un programa por arco** | los rivales tapaban el arco al arrancar. El juez permite dos programas: `CARGAR-ROBOT.bat` |
| 💾 **Respaldo del partido 1** | `respaldos/partido1-2026-09-21/`, se carga tal cual |
| 🔍 **Análisis de la órbita y del escape** | dos hallazgos grandes, con ideas rankeadas. **Nada de eso está implementado todavía** |

---

## 1. Cómo arrancó la clase

- **Se clonó el repo en esta PC:** `Documents\Martes SL football\iita-vibe-robotics-martes-SL`.
- **El robot estaba corriendo el firmware del 15/09.** El banner coincidía con el de MODO-PRUEBA, y la patada seguía en modo prueba (110).
- **El cable USB no llega a la cancha:** todo lo que se cargó hoy se cargó con el robot **apoyado en la mesa**.
  - Los números de los sensores de línea que se leyeron hoy son **de la mesa**. No dicen nada del verde de la cancha.
  - Sobre la mesa leen como verde (entre 80 y 370 según el lugar), salvo en las zonas claras.

## 2. Cambios de hoy, en orden

Todos se cargaron y se **confirmaron leyendo el banner**. El `SUCCESS` del cargador no alcanza.

| # | Qué | Antes → después | Por qué |
|---|---|---|---|
| 1 | `MS_ESPERA_ARRANQUE` (nueva) | 3000 → **2000** ms | Empezar más rápido. Es la última de las esperas del arranque: antes hay ~3 s de espera del USB, ~1 s del giroscopio y 2 s mirando los arcos. No toca la detección del arco, porque viene después. |
| 2 | `VEL_PATADA` | 110 → **215** | "Quedó muy lento" de la clase pasada. Fin del modo prueba. Ahora el banner imprime `patada: 215 x 420 ms`: antes no había forma de saber qué patada estaba cargada. |
| 3 | `MS_RETROCESO_LINEA` | 300 → 200 → **250** | Pedidos del equipo. **El partido se jugó con 250** *(a confirmar)*. |
| — | **PARTIDO 1** | | ver sección 3 |
| 4 | `MS_RETROCESO_LINEA` | 250 → **210** | Pedido del equipo después del partido. |
| 5 | `VEL_ESCAPE_FUERTE` | 200 → **170** | "El robot se levanta" al ver la línea. |
| 6 | `MS_CIEGO_LINEA` | 1000 → 500 → **300** | Mientras no mira la línea, en las esquinas cruzaba la otra. **No bajarla de `MS_RETROCESO_LINEA`**: si la ventana termina antes que el retroceso, vuelve el pataleo del 25/08. |
| 7 | Arco fijo por programa | elige mirando → **fijo al cargar** | ver sección 4 |

## 3. El partido

- **Ganamos 5 a 0.** El delantero "funcionó bastante decentemente".
- **Hubo que sacarlo unas 3 veces**, con un minuto de penalización cada una, porque se salía de la cancha. Lo que vio el equipo, con sus palabras:

  > "dependiendo cómo detecta la línea, el robot se levanta, y si está en una esquina, llega a leer la otra línea mientras está retrocediendo y ya se sale de la cancha"

- **Al arrancar, los robots rivales tapaban el arco.** En los 2 s que el robot mira al encender no lo llegaba a ver, y se quedaba con el arco por defecto (AZUL). En la mesa también se vio: en dos arranques vio el arco azul **2 veces** y **0 veces**, y hacen falta 3.

⚠️ **Con qué versión se jugó:** con la última cargada antes de que nos contaran del partido: patada 215, retroceso **250 ms a 200**, **1000 ms** sin mirar la línea. Falta que el equipo lo confirme.

## 4. 🎯 Un programa por arco: `CARGAR-ROBOT.bat`

**El juez permite tener dos programas.** Se resolvió así:
- **Es el mismo código compilado dos veces.** `funciona/delantero/platformio.ini` tiene `[env:azul]` y `[env:amarillo]`, que definen `ATACAR_AZUL` o `ATACAR_AMARILLO`. Un arreglo se hace una sola vez y vale para los dos.
- **Con el arco fijo, el robot no mira al encender.** Los rivales ya no le tapan nada, y arranca 2 s antes.
- **Sin ninguna de las dos opciones** (Arduino IDE, o `pio run -e teensy41`), queda como antes: elige mirando.
- `default_envs = teensy41`, para que un `pio run -t upload` sin `-e` no cargue los tres programas uno atrás del otro.

**El script** (`CARGAR-ROBOT.bat` + `cargar-robot.ps1`, con un acceso directo `CARGAR DELANTERO` en el Escritorio de esta PC). Muestra un menú (1 = azul, 2 = amarillo) y:
1. Revisa que haya **un solo robot enchufado** y que sea **el delantero** (serie 15708680). Si es el arquero, no carga nada.
2. Abre el Teensy Loader, compila y carga.
3. Vuelve a comprobar que el que regresó por USB es el delantero.
4. **Lee el banner** y exige la línea `ARCO: <el elegido>  (fijo por programa)`. Si no la encuentra, termina en rojo.
5. Cierra el Teensy Loader.

**Probado en el robot:** `amarillo` pasándole el arco directamente, `azul` desde el menú, y otra vez `azul` después de la revisión. **La carga completa tarda 11 s** (la primera compilación de cada arco, ~25-45 s).

**Revisión adversarial del script** (6 agentes): encontró 5 problemas reales, ya corregidos.
- **En modo carga (PID 0478)** la Teensy informa el serie en hexadecimal y dividido por 10. Ahora el script lo convierte y **rechaza al arquero sin preguntar**. Verificado con el registro de esta PC: `0017F834` → **15708680** (delantero) y `00196E9D` → **16667170** (el arquero, que también se enchufó alguna vez en esta notebook).
- **El Teensy Loader quedaba abierto en modo Auto** con el programa del delantero. Si después se ponía el arquero en modo carga en esta PC, le podía grabar ese programa. Ahora se cierra al terminar.
- **Abrir el puerto serie** ahora tiene reintentos. Antes, un fallo momentáneo se reportaba como "hay un monitor serie abierto".
- **La pantalla del menú avisa: batería apagada.** Si no, el robot sale andando apenas termina de cargar.
- **El programa sin arco fijo** ya no imprime "ARCO: AZUL" antes de elegir el arco.

**El flujo del día de partido:**
1. Sorteo de lado.
2. En la mesa: batería apagada, USB enchufado, `CARGAR DELANTERO`.
3. Esperar el cartel verde: `LISTO … (confirmado por el robot)`.
4. Desenchufar el USB.
5. En la cancha: apoyar el robot **mirando al arco rival** y recién ahí prender la batería.

El arco **queda grabado en el programa**: no se borra al apagarlo, y solo cambia si se vuelve a cargar. El rumbo cero sí se toma cada vez que se prende; por eso el paso 5.

## 5. 💾 Respaldo del partido 1

`respaldos/partido1-2026-09-21/` tiene su propio `platformio.ini`, así que se carga tal cual desde su carpeta. Es el programa del partido (patada 215, retroceso 250 ms a 200, 1000 ms sin mirar la línea) más un encabezado y una línea del banner (`*** RESPALDO PARTIDO 1 ***`). Se comparó línea por línea con la copia de trabajo.

⚠️ **El respaldo no tiene arco fijo:** elige mirando, y ahí vuelve el problema de los rivales tapando el arco.

## 6. 🔍 La órbita: por qué pierde la pelota (análisis, 9 agentes)

**Es un compás.** Con las dos ruedas de adelante quietas, el robot gira alrededor de un punto que está **17,5 cm adelante de su centro** (R = 2L). Se verificó con la geometría de las ruedas. Si la pelota no está justo en esa "punta seca", al girar se corre de costado en la imagen. A los 90° de giro queda así:

| distancia real de la pelota al centro | 10 cm | 14 cm | **17,5 cm** | 22 cm | 26 cm |
|---|---|---|---|---|---|
| ángulo en la imagen | 23° | 11° | **0°** | 14° | 26° |

- La patada exige menos de **8°**, y la cámara ve hasta **~34°** a cada lado.
- Coincide con lo medido el **25/08**: arco a 0,3–0,8° (casi perfecto) y pelota a 21–29° del frente, sin patear.
- **Nadie sabe dónde queda la pelota al empezar a orbitar.** `XP_ORBITA = 34` está en unidades de la cámara.

⚠️ **Corrección importante:** el "cuando la cámara dice 48, la pelota está a 12 cm" que usábamos **se midió en la cámara del ARQUERO**, y el arquero mismo lo corrigió el 25/08 (factor 2,87). **La escala de la cámara del delantero es desconocida**: `pruebas/tabla-camara/` nunca se corrió.

**Otras causas encontradas (verificadas en el código):**
- **Mientras orbita no corrige nada:** nunca usa `Yp`.
- **Pelota pegada y pelota perdida son el mismo dato** (`Xp = 0`): a los 300 ms se va a BUSCANDO. Hipótesis: pasa si la punta seca cae en la zona ciega.
- **Un solo cuadro con `Xp > 55`** (por ejemplo, otra mancha naranja) lo saca a AVANZANDO.
- **Entra a orbitar aunque la pelota esté de costado:** solo mira la distancia.
- **Las ruedas de adelante a PWM 30 están sueltas el 88 % del tiempo.** Si la trasera las arrastra un 10 %, la punta seca se corre a **13** o a **24 cm**.
- **`sentidoParaOrbitar()`:** según la cuenta, con el arco a la vista orbita por el camino **largo**. Pero eso **contradice lo que se vio el 01/09**. Hay que probarlo, no cambiarlo a ciegas.
- **Al perderla, BUSCANDO gira siempre a la izquierda y despacio**, aunque la pelota se haya ido para el otro lado.

**Ideas, en el orden en que conviene probarlas** (de a una, cada una detrás de un interruptor):

| # | Idea | Por qué |
|---|---|---|
| 0 | **Medir primero:** `tabla-camara` con la pelota a 11,5 / 14,5 / **17,5** / 20,5 / 30 cm del **centro**, y el borde de la zona ciega | Si a 17,5 cm da `Xp = 0`, la órbita es ciega por diseño y cambia todo el orden |
| 1 | **Caja negra:** contar por qué termina cada órbita y cuántos cuadros dan `Xp = 0` | No cambia el juego y dice qué causa pesa más |
| 2 | Salir a AVANZANDO recién con **~10 cuadros seguidos** con `Xp > 55` | Cambio chico, no depende de nada sin medir |
| 3 | **Dos pruebas de signo** del sentido de la órbita, sin tocar código | Si se confirma, el arreglo es cambiar un solo signo |
| 4 | **Pelota pegada ≠ perdida:** aguantar ~1,5 s si la última lectura estaba cerca y centrada | El campeón 2025 aguantaba 3 s |
| 5 | Entrar a orbitar solo con la pelota a **menos de 15°** del frente | Si no, primero centrar |
| 6 | **Buscar hacia el lado** por donde se fue la pelota | Es barata y acorta cada pérdida |
| 7 | **"Volante":** golpecitos de las ruedas de adelante mientras orbita, si la pelota se corre más de 6° | Es la única que ataca la causa principal, pero toca el corazón de la órbita que ganó 5-0. Va última |

## 7. 🔍 El escape de línea: por qué se sale en las esquinas (análisis, 7 agentes)

**Solo el sensor de adelante (S3) retrocede derecho para atrás.** Cuando dispara primero un sensor de costado, el escape sale a **60° del frente**, y **la mitad del movimiento es hacia adelante**. Se verificó con la cinemática de las ruedas.

| sensor que dispara primero | dirección del escape |
|---|---|
| S3 delantero | 180° (derecho para atrás) |
| S1 derecho | +60° (adelante a la izquierda) |
| S2 izquierdo | −60° (adelante a la derecha) |

- **En una esquina, ese "adelante" suele ser la otra línea.** Como el robot no mira la línea durante el escape, la cruza de largo sin reaccionar.
- **Cuándo pasa:** cuando toca la línea con un costado (patada torcida, bordeando la línea, orbitando) o justo con una rueda. En ese caso los dos sensores vecinos llegan casi juntos, y "el primero" es tirar una moneda.
- **Bajar la velocidad no cambia la dirección.**

**Otros hallazgos (verificados en el código):**
- **Al terminar la ventana sin mirar la línea, el primer escape puede usar datos viejos.** El cronómetro de cada sensor queda congelado durante esa ventana. Un sensor que ya veía blanco al disparar y lo sigue viendo se confirma en el acto, sin el filtro de 5 ms. Si para entonces el sensor de adelante ya ve verde, ese sensor viejo decide la dirección. Con la ventana en 300 ms, esto es más probable.
- **`frenoFuerte` se calcula y nunca se usa.** Lo mismo pasa con `VEL_FRENO`, `MS_FRENO` y otras constantes. Además hay comentarios que describen código que ya no existe ("primero frenar", "las esquinas salen solas").
- **Si la línea corta la patada, el robot se saltea `PATEA_ATRAS`** (700 ms a 110).

**"Se levanta"** (deducido del código; **falta mirarlo**):
- Cuando dispara S3, las dos ruedas de adelante pasan de **+215 a −170 de un instante al otro** (un salto de 385 cuentas), con la trasera suelta.
- El borde de apoyo de adelante está a **4,4 cm** del centro y la rueda trasera a **8,75 cm**. Lo más probable es que **se levante la cola**.
- ➜ **Pregunta para el equipo:** ¿se levanta la cola o la trompa?

**La lógica propuesta, verificada por un juez contra el código: "sumar la esquina"** (NO implementada):
- **Qué hace:** si el escape lo disparó un sensor **de costado** y, mientras escapa, **otro** sensor que estaba en verde pasa a blanco, esa es la otra línea. Entonces **suma** su dirección a la del escape (`escaparDeLinea()` ya sabe sumar), **una sola vez**, y estira un poco el retroceso.
- **Por qué no vuelve el pataleo:** la dirección nunca se reemplaza y ninguna rueda se invierte al sumar.
- **Por qué no suma cuando dispara S3:** ese escape ya es un retroceso puro. Y en una patada de frente, si el robot se pasa de largo, los costados pisarían la **misma** línea y se confundiría con una esquina.
- **Se prueba en dos cargas:**
  1. Interruptor `SUMAR_ESQUINA = false`: juega igual que hoy, pero avisa `+++ blanco NUEVO en sensor K` para confirmar el diagnóstico.
  2. `SUMAR_ESQUINA = true`: un solo cambio.
  - Se puede probar en la mesa armando una esquina con cinta blanca.

**Descartadas, con motivo:**
- **Bajar más la ventana sin mirar la línea, o cortar el escape cuando vuelve a ver verde:** traen de vuelta el riesgo de pataleo.
- **Frenar o hacer una rampa antes de invertir:** no arreglan la esquina y hacen que se pase más de la línea.

## 8. 💡 El LED de la cámara

Sale del script de la cámara (`robots-2025/vision-openmv/…py:124-135`):
- 🔴 **rojo** = pelota · 🟢 **verde = arco AMARILLO** · 🔵 **azul** = arco azul.
- **Al prender**, parpadea 2 veces en verde: eso avisa que el programa de la cámara arrancó.
- **Es un solo LED RGB, así que los colores se mezclan:** amarillo = pelota + arco amarillo, violeta = pelota + arco azul, celeste = los dos arcos, blanco = las tres cosas.
- **El rojo se prende con apenas 7 píxeles naranjas**, así que cualquier cosa naranja al fondo también lo prende. Los arcos piden 300 píxeles (azul) y 600 (amarillo).

## 9. Números de hoy, todos juntos

```
CARGADO AL CERRAR   ARCO AZUL fijo (env azul) · patada 215 x 420 ms, rampa +15/5 ms
                    al ver linea: retrocede 210 ms a 170, 300 ms sin mirar la linea, confirma 5 ms
                    umbrales 390/427/413 · orbita Xp<34, impulso 120 x 300 ms -> 67, max 20 s
                    avance 95 · cuenta de arranque 2 s (sin mirar arcos)
JUGO EL PARTIDO 1   igual, pero retroceso 250 ms a 200, 1000 ms sin mirar la linea,
                    y elegia el arco mirando 2 s al encender            (a confirmar)
CAMARA              42-46 paq/s, 0 bytes tirados · GIROSCOPO SYS_STATUS=5 en todos los arranques
ARCO AL ENCENDER    en la mesa: azul 2 muestras (hacen falta 3) y 0 muestras -> "no vi ningun arco"
CARGA CON SCRIPT    11 s (ya compilado) · primera compilacion de cada arco ~25-45 s
SERIES TEENSY       delantero 15708680 (modo carga 0017F834) · arquero 16667170 (00196E9D)
```

---

## ⚠️ Qué queda por VER

1. 🔴 **Con qué versión se jugó cada partido.** Del 1 suponemos 250 ms a 200 y 1000 ms sin mirar la línea. De los partidos 2 y 3 sabemos que fueron con `CARGAR-ROBOT.bat` y que se salió menos, pero no en qué momento se cargó cada valor.
2. ~~🔴 **Retroceso 210 ms a 170 con 300 ms sin mirar la línea:** no se vio en una jugada completa.~~ ✅ **Se jugaron los partidos 2 y 3 con eso y se salió MENOS de la cancha** (sección 7). Sigue sin anotarse qué hace exactamente en las esquinas ni si se levanta.
3. **¿Se levanta la cola o la trompa?** Filmar a ras del piso.
4. ~~**El script en un partido de verdad:** que el cartel verde salga y que el robot ataque el arco elegido.~~ ✅ **Se usó en los partidos 2 y 3** (sección 7).

## 🔧 Qué queda por HACER

5. **Esquina:** cargar "sumar la esquina" en dos pasos (primero solo el aviso, después el interruptor prendido).
6. **Órbita:** medir primero con `tabla-camara` desde el **centro** (10 min, sin motores, se puede hacer en la mesa). Después la caja negra y los pasos de la tabla de la sección 6.
7. 🐛 **`pruebas/piso-de-pwm/` tiene los pines del ARQUERO** (`piso-de-pwm.ino:38`). En este robot, "IZQUIERDA" movería la trasera, "DERECHA" la izquierda y "TRASERA" la derecha. Corregirlo antes de correrlo. Sigue sin correrse desde julio.
8. **Medir el escape con cinta métrica** en tres casos (robot quieto sobre la línea, avanzando, pateando): cuánto se pasa de la línea y dónde queda.
9. **Limpieza sin cambiar el comportamiento:** borrar `frenoFuerte`, `VEL_FRENO`, `MS_FRENO` y las otras constantes sin uso, y corregir los comentarios viejos.
10. Sigue pendiente del 15/09: **girar el robot a mano** para verificar el giroscopio, y la **bitácora del 08/09**.

---

## 7. 🏆 Partidos 2 y 3 — anotados el 22/09

Esto se agrego al dia siguiente, contado por el equipo. **Las secciones de arriba se
escribieron durante la clase y solo cubrian el partido 1.**

| | resultado |
|---|---|
| Partido 1 | **ganamos 5 a 0** |
| Partido 2 | **perdimos 3 a 1** |
| Partido 3 | **ganamos 3 a 2** |

### ✅ Lo que quedo confirmado

**El escape retocado funciono: en los partidos 2 y 3 se salio MENOS de la cancha.**
En el partido 1 se habia salido unas 3 veces, con un minuto de penalizacion cada una. Los
tres numeros que cambiaron entre el partido 1 y hoy:

| | partido 1 | partidos 2 y 3 |
|---|---|---|
| `VEL_ESCAPE_FUERTE` | 200 | **170** |
| `MS_RETROCESO_LINEA` | 250 ms | **210 ms** |
| `MS_CIEGO_LINEA` | 1000 ms | **300 ms** |

Eso cierra el pendiente 2 de la lista de arriba: **el retroceso de 210 ms a 170 con 300 ms sin
mirar la linea se vio en partido**, y anduvo mejor que el anterior.

**Y el programa de arco fijo se uso en partidos de verdad.** Del partido 2 en adelante siempre
se cargo con `CARGAR-ROBOT.bat`, eligiendo el arco en la mesa antes de jugar. El equipo lo
resumio asi: *"es una muy buena idea"*. Cierra el pendiente 4.

### ❓ Lo que NO sabemos, y conviene no inventar

- **Por que se perdio el partido 2.** Se salio menos de la cancha, asi que la derrota fue por
  otra cosa: no esta anotado cual.
- **Si entre el partido 2 y el 3 se cambio algo mas** ademas de esas tres constantes, o si el
  3-2 salio con lo mismo y cambio el rival o la suerte.
- **Con que valores exactos se jugo cada partido.** Sabemos como quedo el codigo al final del
  dia, no en que momento se cargo cada version.

Es el mismo agujero que ya senala la nota de metodo de abajo: se cambiaron numeros rapido sin
anotar que se vio con cada uno. **El proximo partido conviene anotar tres cosas antes de
empezar: que version esta cargada, que arco, y despues cuantas veces se salio.**

---

## Nota de método

**Hoy los números del escape se cambiaron rápido y sin anotar qué se vio con cada uno:** retroceso 300 → 200 → 250 → 210, ventana 1000 → 500 → 300. Cada vez que se cambió, el robot se cargó y se confirmó, así que sabemos **qué** estaba cargado en cada momento. Lo que no sabemos es **qué hizo** con cada valor. La próxima vez, antes de pedir el número siguiente: una línea con qué se vio. Si no, dentro de tres semanas no se sabe por qué quedó 210.

**Y la mejor solución del día no fue de código: fue de reglamento.** Pelear con la cámara para que vea un arco tapado por dos robots rivales no tenía salida. Preguntarle al juez si se podían tener dos programas, sí.
