# 2026-10-27 — El tablero, el programa corto, y la patada a 180

**Quiénes:** Máximo y Juanse (mesa del delantero) + Claude (código, revisión y carga)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**PC:** la computadora móvil del equipo (`AGUSTINAAG`)
**Programa cargado al cerrar:** ❓ **no lo sé.** Lo último que vi en el robot fue el **tablero**
(`pruebas/tablero`), con los números de la patada de antes. Los números nuevos están en el
principal pero **no vi que se cargaran**. 🔴 **Antes de jugar: `CARGAR-ROBOT.bat`, y en el cartel
tiene que decir `patada: 180 x 450 ms`.**

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🆕 **TABLERO del delantero** | una ventana con botones: pelota y arcos vistos desde arriba, sensores, giroscopio, y los movimientos con nota y gráfico |
| ✂️ **El programa principal, corto** | de 2920 a **1239 líneas**. Arriba solo lo esencial (24 números). Juega **igual, byte por byte** |
| 🎯 **La patada, ajustada con el tablero** | `VEL_PATADA` 150 → **180** · `MS_PATADA` 500 → **450** · `KP_PATADA` 4.0 → **6.0** |
| 🔄 **El giro de búsqueda** | `VEL_GIRO_CONTINUO` 40 → **35** |
| ❓ **Qué nota dio el tablero** | **no quedó anotado**: se preguntó dos veces |
| 🔴 **Nada de esto jugó un partido** | ni la patada nueva ni el giro a 35 |

---

## 1. 🆕 El TABLERO del delantero

**El pedido:** el equipo leyó la herramienta del arquero (`ajustar-movimientos`, 20/10) y pidió
*"adaptala, pero más interactiva: una ventana con interfaz visual y botones. Simple"*. Y además
*"una ventana para ver los valores de los sensores y una interfaz para ver la posición de la
pelota"*, como el dibujo con símbolos del arquero, **sin la predicción**.

**Lo que quedó:**

| archivo | qué es |
|---|---|
| `TABLERO.bat` | muestra qué robot hay; **carga sólo si es el delantero**; carga el programa del tablero y abre la ventana |
| `TABLERO-DEMO.bat` | la misma ventana con un **robot de mentira** (datos inventados), para conocerla sin robot |
| `pruebas/tablero/tablero.ino` | el programa del robot: no juega, espera órdenes de la ventana |
| `pruebas/tablero/tablero.py` | la ventana (Python con tkinter: anda sin internet y sin instalar nada) |

**Las pestañas:**

- **PELOTA:** el robot visto desde arriba, con la pelota y los arcos donde los ve la cámara. Las
  rayas de X = 32 cm (ahí empieza a rodearla) y X = 55 cm (ahí la suelta).
- **SENSORES:** los 3 sensores de línea con su umbral, el giroscopio como brújula y la cámara.
- **MOVIMIENTOS:** 1 patada (golpe + retroceso), 2 avance, 3-5 escape de línea por sensor,
  6 órbita y 7 giro de búsqueda. Cada uno da **nota** (PROLIJO / CASI / ZIGZAGUEA / SE TUERCE,
  o GIRA_BIEN / SE_PLANTA / NO_ARRANCA), un **gráfico** del desvío, un **consejo** de qué número
  tocar, `−` y `+` para los números, un botón rojo **PARAR** (o Esc), y **Copiar al panel**.
- **CONSOLA:** todo lo que dice el robot.

**Las reglas que trae:**

- **Nada pasa solo al principal.** Lo que se cambia en el tablero vive en el robot hasta que se
  apaga. Para pasarlo: **Copiar al panel**, y cambiar sólo el número de cada línea en el panel
  del principal (no pegar todo junto: queda repetido y no compila).
- **No mira la línea blanca** mientras prueba: hay que ponerlo con lugar.
- `LAZO`, `TRASERA_TRABADA`, `SIN_ESPERA` y los `MS_PRUEBA_...` son **sólo de la prueba**.

**Cómo se armó y qué se comprobó:**

- Lo armaron dos ayudantes en paralelo (el programa del robot y la ventana) con un mismo
  "contrato" de qué se mandan. Después lo revisaron otros cuatro: si copia bien al principal, si
  los dos lados se entienden, si es seguro y si la ventana se entiende. **Encontraron 23 cosas, y
  se arreglaron.** Las más importantes: el `.bat` no carga si no ve al delantero, y la ventana no
  le manda nada a un programa que no sea el tablero.
- El programa del robot **compila sin ninguna advertencia**. La ventana pasa **3451
  verificaciones automáticas** sin abrirse, y la demo **se abrió en esta PC sin fallas**. Al
  `TABLERO.bat` se le probaron 17 casos en seco: delantero, otro robot, ninguno y respuestas raras.
- **El equipo lo usó con el robot**: de ahí salieron los números de la patada (punto 3).

**Lo que queda flojo:**

- **No se vio cómo queda en la pantalla de la notebook** (1366×768). Si algo se corta, avisar.
- **PARAR manda la tecla aunque el robot tenga otro programa** (avisa en rojo que ese no obedece).
- **Si hay OTRO robot enchufado en modo carga** al mismo tiempo que el delantero, `TABLERO.bat` no
  lo ve (en esta PC no está `wmic`). Regla de siempre: **un solo robot enchufado**.

**De paso, en `PROBAR-PATADA` (`pruebas/patada-derecha`):** el momento en que se clava el
giroscopio ahora se sortea entre 200 y 520 ms, como pasa en el partido cuando se viene de la
órbita. Antes se sorteaba entre 0 y 500.

---

## 2. ✂️ El programa principal, corto

**El pedido:** *"es MUY LARGO, tiene demasiados comentarios de cosas que ya sabemos. Sacar la
mayor parte, dejar algunos que expliquen cosas específicas, y SOBRE TODO dejar arriba de todo
sólo las variables esenciales."*

**Cómo quedó** `funciona/delantero/delantero.ino`:

- **2920 → 1239 líneas.** De 1640 líneas de comentario quedaron unas 80, todas cortas.
- **Arriba: LO ESENCIAL**, 24 números: buscar/centrar/avanzar, órbita, patada (con TRIM, KP, KD),
  puntería de lejos, rumbo cero y línea blanca.
- **Después: OTROS AJUSTES (casi nunca se tocan)**, los otros 56.
- **Comentarios que quedaron, a propósito:** `frenar()` contra `parar()`, cómo funciona la órbita,
  para dónde escapa cada sensor, que la línea manda sobre todo, no escribir funciones antes del
  `enum Estado`, y no cambiar la línea `ARCO:` del cartel (`CARGAR-ROBOT.bat` la lee).
- **Se sacaron las 7 variables que no hacían nada.**
- **Verificado:** los tres programas de partido salieron **idénticos byte por byte** a los de
  antes del recorte.
- **La versión con todos los comentarios** quedó en
  `respaldos/delantero-2026-10-27-con-comentarios-largos.ino`.

---

## 3. 🎯 Los números que salieron del tablero

El equipo los pasó con **Copiar al panel** y quedaron en el principal:

| | antes | ahora |
|---|---|---|
| `VEL_PATADA` | 150 | **180** |
| `MS_PATADA` | 500 | **450** |
| `KP_PATADA` | 4.0 | **6.0** |
| `VEL_GIRO_CONTINUO` | 40 | **35** |

Se cambiaron también en el tablero y en `PROBAR-PATADA`, para que "volver a los números del
partido" vuelva a éstos. Todo compila.

### ❓ Lo que NO sabemos

- **Qué nota dio cada uno en el tablero**, con qué pico y si salió derecha. Se preguntó dos veces
  y no quedó dicho.
- **Si el giro a 35 arranca siempre.** Con 40 ya estaba anotado que podía quedarse zumbando.
  En el tablero es el movimiento 7 (`GIRO`): tiene que dar **GIRA_BIEN**, nunca NO_ARRANCA.
- **Si con 180 el robot se levanta** al frenar después de patear. El 12/10 se bajó de 215 a
  150 justamente por eso, con el robot recién achicado.

---

## ⚠️ Qué queda por VER

1. 🔴 **La patada nueva en la cancha:** que vaya derecha, que la pelota llegue, y que **no se
   levante**.
2. 🔴 **El giro a 35:** que siempre arranque. Si zumba, volver a 40.
3. La ventana del tablero en la pantalla de la notebook.

## 🔧 Qué queda por HACER — en orden

1. 🔴 **Antes de jugar: `CARGAR-ROBOT.bat`**, con el arco que toque, y leer el cartel:
   `patada: 180 x 450 ms` y `KP 6.0`.
2. **Anotar las notas del tablero** cada vez que se pase un número al principal.
3. Probar en el tablero lo que todavía no se miró: escape de línea (que no gire), órbita (que no
   se plante después de recentrar) y la tecla `SIN_ESPERA` en la patada.
4. Siguen de antes: medir `L`, revisar el cable USB, la cámara, decidir el nombre con la mesa del
   arquero, y correr `CALIBRAR-ROBOT.bat` entero una vez.

---

## Nota de método

**Dos cosas que pidieron hoy se pudieron comprobar en serio.** El programa corto se comparó byte
por byte, así que no hay que creer que "no cambió nada": se vio. Y el tablero lo revisaron
lectores que no lo habían escrito, y encontraron 23 cosas antes de que el robot se moviera una vez.

**Lo que falló es lo de siempre:** salieron cuatro números nuevos y no quedó escrito qué se vio
con ellos. El tablero da la nota y el gráfico en la pantalla; copiar dos renglones a la bitácora
es lo que falta para que dentro de una semana se sepa por qué la patada está en 180.
