# 2026-11-08 al 10 — Tactical aprobado, el partido con saque y el saque en el TABLERO

**Cubre tres días** (la última bitácora era la del 03/11):
- **08/11** (sesión extra, en la sede del centro) y **09/11**: vino Máximo solo.
- **10/11** (la última clase antes de la Roboliga): Máximo y Juanse.

**Quiénes:** Máximo y Juanse (mesa del delantero) + Claude (código y revisión)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa en el robot al cerrar el 10/11:** el **TABLERO** (`pruebas/tablero`, saluda
`#HOLA tablero-delantero 2026-11-10`). 🔴 **Para jugar hay que cargar el partido:**
cerrar el TABLERO y `PARTIDO-CON-SAQUE.bat` (o `CARGAR-ROBOT.bat`).

---

## 📋 RESUMEN

| | |
|---|---|
| 🏆 **Tactical Challenge** | **aprobado por el profe** el 08/11: amarillo → azul → tocar la pelota |
| ⚽ **Programa nuevo** | **`PARTIDO-CON-SAQUE.bat`**: el partido con un saque al azar a ciegas (20 % centro, 40 % izquierda, 40 % derecha) |
| 🛡️ **Saca o defiende** | al arrancar: si **ve** la pelota defiende (patea derecho), si **no** la ve saca |
| 🖥️ **TABLERO** | botón nuevo **8 · SAQUE**: mide cuántos grados gira de verdad antes de patear |
| 🛑 **Freno antes de patear** | "cuando patea se desvía": ahora frena 200 ms antes del golpe |
| 📷 **Cámara** | sin el parpadeo del LED, con **horizonte** (no mira arriba: sillas rojas), una versión por sede |
| ⏱️ **Arranque** | de casi 7 s a unos 2–3 s (estimado) |
| ⚠️ **Sin probar en la cancha** | casi todo lo del 09 y 10/11: ver *Qué queda por VER* |

---

## 1. 🏆 Tactical Challenge (08/11) — APROBADO

El desafío del profe: desde un punto cualquiera, ir al **arco amarillo**, después al **arco
azul** y después **tocar la pelota**, siempre en ese orden.

- Programa aparte: `tactical-challenge/` + `TACTICAL-CHALLENGE.bat`. El principal no se tocó.
- Al pedirlo se escribieron **dos órdenes distintos**; se preguntó y quedó **amarillo → azul →
  pelota**.
- **Cómo llega a cada arco**, en tres pasos:
  1. Frenar a 40 cm del arco.
  2. Frenar al ver la línea con el arco a menos de 80 cm. En el amarillo **se quedaba trabado**.
  3. **Cualquier línea**, pero solo si ya vio ESE arco en ese paso. Si no, contaría la línea del
     amarillo como la del azul. Frena, espera 1 s y se despega.
- La pelota la toca despacio (75 × 500 ms) y queda quieto. Anda sin la compu.

## 2. 📷 Otra sede y la cámara

- **08/11, en la sede del centro (con menos luz): "detecta mucho mejor".** El amarillo pasó por
  `(2, 75, -13, 5, 0, 51)` y `(14, 80, -15, 5, 10, 51)`.
- **Sin el parpadeo del LED** (09/11): eran 1,2 s parado al arrancar. Ahora el LED verde queda
  prendido mientras la cámara se prepara.
- **Horizonte** (09/11): las **sillas rojas** del fondo salían como pelota. En el monitor se vio
  `[201, 200, 32, ...]`, una "pelota" a 200 cm entre todas las de 31 cm. Ahora `find_blobs` busca
  solo debajo de la fila `HORIZONTE = 46`, y en la vista previa lo de arriba sale en negro con una
  línea amarilla.
- **10/11: una versión por sede**, en `copias-de-seguridad-camara/`. Las dos tienen horizonte 46 y
  no tienen parpadeo:

| Archivo | Amarillo | Dónde |
|---|---|---|
| `1011centro.py` | `(70, 99, -10, 0, -3, 64)` | sede del centro |
| `1011sanlorenzo.py` | `(50, 97, -25, -3, 50, 111)` (la "opción 5" del 03/11) | San Lorenzo |

Se guarda en la cámara **como `main.py`**: con otro nombre no arranca sola.

**Ojo, al final del 10/11** el equipo guardó `1109conhorizonte.py` con el amarillo
`(60, 99, -25, -3, 50, 85)` y **`HORIZONTE = 0`**, o sea **sin horizonte** (busca en toda la
imagen). No quedó anotado por qué: ¿el horizonte cortaba los arcos de lejos? Las dos de `1011`
siguen con 46.

## 3. 🤖 El principal (`CARGAR-ROBOT.bat`)

- **Patada:** 08/11 se probaron 170 y 190, y **con 190 se volcaba un poco** → quedó en 180. El
  09/11 se pidió **190**. Hoy está en **190 × 450**.
- **Umbrales de línea** (09/11): **300 / 300 / 440** (derecho / izquierdo / delantero; antes
  424 / 434 / 440). Igual en el Tactical y en el partido con saque.

## 4. ⚽ El partido con saque (`PARTIDO-CON-SAQUE.bat`)

Es una **copia del principal** con un saque al arrancar. Pregunta 1 azul / 2 amarillo, igual que
`CARGAR-ROBOT.bat`, y confirma el arco en el cartel. Lo que se cambie en uno **no pasa al otro**.

**El saque**, que se fue armando en estos días:
1. **El rumbo del giroscopio.** Si lo apuntaban a mano a la pared, el "rumbo 0" (el arco rival)
   quedaba mal todo el partido. Por eso **se pone mirando al arco** y el saque se tuerce solo.
2. **Al azar.** Cada vez que se prende sortea: **20 % centro, 40 % izquierda, 40 % derecha**.
3. **A ciegas** (09/11). No busca la pelota con la cámara: **rodea la pelota** con la órbita y el
   giroscopio le dice cuándo llegó. La pelota tiene que estar **a la distancia de la órbita**.
4. **60°** en vez de 45°, por la cuenta del rebote desde el centro: pared a ~91 cm, arco a
   ~110 cm → ~59°.
5. **Números propios**, probados en el TABLERO: `VEL_SAQUE 255`, `MS_SAQUE 900`, trim 10,
   KP 2.5, KD 0, rampa 15, y atrás 155 × 300.
6. **Freno antes de patear** (10/11): `MS_FRENO_SAQUE 200`.
7. **Saca o defiende** (10/11): mira 300 ms con la cámara. Si **ve** la pelota (lejos, saca el
   rival) → **defiende**: patea derecho al frente. Si **no** la ve (pegada) → **saca** con el
   sorteo. Interruptor: `DECIDIR_SACA_O_DEFIENDE`.

**El resto del partido con saque:**

| | Valor | Cuándo |
|---|---|---|
| Patada normal | **190 × 500**, trim 6, KP 5.0, KD 0.3, atrás 100 × 400 | 10/11 (antes 200, 220, 235 × 525) |
| Freno antes de la patada normal | `MS_FRENO_PATADA 200` ms | 10/11 |
| Escape de la línea | `VEL_ESCAPE_FUERTE 155` | 09/11 |
| Velocidades | **+20 %**: avance 114, llegada 84, giro 42, pulsitos 94, órbita 80 | 09/11 |
| Puntería | **12°** con el arco lejos, **10°** cerca (antes 8°) | 09/11 (se probó 15) |
| Lado de la órbita | siempre hacia el lado más corto al **rumbo 0**; si va al revés, se da vuelta solo y se acuerda | 09/11 |
| Arranque | espera lo justo al giroscopio: **fusión en 287 ms** medidos (antes 900 fijos) | 09/11 |

**La órbita fluida** (que rodee sin frenar y se hamaque viendo el arco) se hizo el 09/11, se
cargó y **se deshizo** a pedido del equipo. Quedó la órbita de siempre. No quedó anotado qué se
vio con ella.

## 5. 🖥️ El TABLERO: el 8 · SAQUE (10/11)

- **Botón nuevo, 8 · SAQUE.** Elegís el lado con `LADO_SAQUE` (-1 izquierda / 0 centro /
  1 derecha), y hace el saque como en el partido.
- **Mide:**
  - cuántos grados giró **de verdad** antes de patear, si se pasó y en cuánto llegó;
  - si se dio vuelta;
  - si el golpe salió derecho, con la misma nota que la PATADA.
- **Notas y gráfico:** hay una nota nueva, **NO LLEGA**. En el gráfico se ve la vuelta (verde),
  el freno (gris), el golpe (naranja) y la marcha atrás (azul).
- **"Copiar al panel"** marca los del saque `// en partido-con-saque`: van ahí, **no** en el
  principal.
- La vuelta del saque usa `VEL_ORB_SAQUE`, que en el partido se llama `VEL_ORB_TRASERA` (80).
- Autoprueba de la ventana: **OK, 3481 verificaciones**. Cargado y confirmado en el robot.
  **Todavía no se probó en la cancha.**

## 6. 🧰 Cosas que aprendimos

- **El SUCCESS del cargador no prueba nada.** El 09/11 la carga "salió bien" y el robot **seguía
  con el TABLERO**: la ventana del TABLERO tenía el puerto tomado. **Antes de cargar, cerrar el
  TABLERO**, y leer siempre el cartel.
- **El TABLERO no se abre dos veces:** cada ventana aparecen como **dos programas** en la compu,
  por cómo arranca el Python de PlatformIO.
- **`vision/camara-delantero-2026-11-09.py` quedó con solo un `9` adentro** (se sobrescribió sin
  querer). Se borró. Lo bueno estaba en `copias-de-seguridad-camara/`.

---

## ⚠️ Qué queda por VER (casi nada de esto se probó en la cancha)

1. 🔴 **El saque en el TABLERO:** cuántos grados gira de verdad, si se pasa y si con el freno el
   golpe sale derecho. Con los dos lados.
2. 🔴 **Saca o defiende:** al sacar, la pelota tiene que quedar **pegada** (que no la vea). Al
   defender, si el rival **tapa** la pelota, el robot cree que saca y rodea sin pelota.
3. **Que no se vuelque** con el saque a 255 × 900.
4. **El freno de la patada normal** (200 ms): si sale más derecha, y si el rival le gana la pelota.
   El TABLERO no lo puede medir, porque su PATADA arranca quieta.
5. **El arranque real con batería:** en la telemetría dice `arranco a los XXXX ms`.
6. **La órbita hacia el 0:** si alguna vez dice `(0 al reves)`.

## 🔧 Qué queda por HACER — en la Roboliga (11/11)

1. 🔴 **`grabar-linea`** en la cancha de allá: los umbrales 300 / 300 / 440 son de acá.
2. 🔴 **La cámara:** elegir la versión que más se parezca a esa luz, y si no anda, medir los LAB
   allá. **Horizonte:** decidir si va (46) o no (0). Si va, desde la otra punta los arcos tienen
   que quedar debajo de la línea amarilla.
3. **Elegir el programa** (principal o partido con saque) y cargarlo con el arco del sorteo de
   lado. Si cambian de lado, se vuelve a cargar con el otro arco.
4. **Cerrar el TABLERO antes de cargar**, y leer el cartel.

---

## Nota de método

**Lo que salió bien:** el Tactical se aprobó, y los números del saque salieron de probarlos en
el TABLERO, no de adivinarlos. Para los cambios grandes se dejaron **interruptores**
(`DECIDIR_SACA_O_DEFIENDE`, `SAQUE_AL_AZAR`, `MS_FRENO_* = 0`) para volver atrás en la cancha
sin reprogramar nada.

**Lo que falta:** se cambiaron muchos números en pocos días, y de la mayoría no quedó escrito qué
se vio en la cancha. Mañana, después de cada partido, alcanza con dos renglones: qué cargamos,
qué hizo el robot.
