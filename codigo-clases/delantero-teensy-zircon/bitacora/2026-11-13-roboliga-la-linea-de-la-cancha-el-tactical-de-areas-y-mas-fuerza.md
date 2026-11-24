# 2026-11-13 — En la Roboliga: la línea de la cancha, el Tactical de áreas y más fuerza

**Quiénes:** el equipo del delantero, en la competencia + Claude (código y revisión)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa en el robot la última vez que se miró:** el **partido con saque** atacando el arco
**AMARILLO**, en una versión **anterior** a los cambios de la tarde (sin el +20 % ni la patada 225).
🔴 **Para jugar con todo lo de hoy: `PARTIDO-CON-SAQUE.bat`** con el arco que toque, y en el
cartel tiene que decir `patada: 225 x 500 ms`.

> ### ⚠️ CORRECCIÓN, agregada el 24/11: así quedó de verdad
>
> Esta bitácora se escribió **antes** de los últimos cambios del día. Después, en la misma
> competencia:
>
> - **"Chocaba la pelota, no frenaba a tiempo"**: las velocidades del partido con saque
>   **volvieron a las del principal** (avance 95, al llegar 70, girar buscando 35, pulsitos 78,
>   órbita 67). El **+20 % de abajo se deshizo** en el partido con saque. El Tactical de áreas
>   sigue con su +20 %.
> - **Empieza a aflojar antes:** `XP_FRENAR` 70 → **90** cm.
> - **Patada normal:** 225 → 240 → 210 → **190** (× 500 ms).
> - **Saque:** `GRADOS_SAQUE` 60 → **45**, `VEL_SAQUE` 255 → **230** (× 900 ms).
> - **Lo último cargado:** partido con saque, arco **AMARILLO**, confirmado por el cartel del robot
>   (`patada 190 x 500`, saque a 45° con fuerza 230).
> - **Cámara:** `copias-de-seguridad-camara/1109conhorizonte.py` quedó con el amarillo
>   `(20, 45, -10, 5, 20, 50)`, cambiado por el equipo ese día a las 08:58.
> - **Resultado: TERCER PUESTO**, con 3 puntos: 4-5, 3-7 y 10-0 (dato del commit del 17/11).
> - ❓ **No quedó anotado con qué programa jugó cada partido.** El respaldo del 17/11
>   (`respaldos/delantero-2026-11-11-ROBOLIGA-2026-tercer-puesto.ino`) es el **principal**, con
>   umbrales 300/300/440. Lo último que se vio cargado acá fue el **partido con saque**.
>
> Lo de abajo (patada 225, +20 %) es lo que había **a media tarde**, no lo que jugó al final.

---

## 📋 RESUMEN

| | |
|---|---|
| 🟩 **Línea** | `grabar-linea` en la cancha de la Roboliga → umbrales **437 / 443 / 447** en todos los programas |
| ⚽ **Patada** (partido con saque) | 190 → 210 → **225** |
| 🏎️ **Velocidades** | **+20 %** en el partido con saque (otra vez) y en el Tactical de áreas |
| 🏁 **Tactical nuevo** | **entrar al área amarilla y después al área azul**: programa nuevo `TACTICAL-AREAS.bat` |
| ❓ **Sin anotar** | cómo salieron los partidos y el Tactical de áreas |

---

## 1. 🟩 La línea de la cancha (con `grabar-linea`)

Se cargó `grabar-linea` y se hizo el recorrido sin cable: verde en varios puntos, blanco con
cada sensor y negro. Salieron **14 mesetas**:

| Sensor | Verde (lo más alto) | Blanco (lo más bajo) | **Umbral nuevo** | Antes |
|---|---|---|---|---|
| 1 DERECHO | 123 | 751 | **437** | 300 |
| 2 IZQUIERDO | 128 | 758 | **443** | 300 |
| 3 DELANTERO | 143 | 752 | **447** | 440 |

- Más de **300 de margen** para cada lado: no se confunde.
- La medición completa quedó en `mediciones/medicion-2026-11-13_09-27-13.txt`.
- Se pusieron en **los cuatro programas**: principal, partido con saque, Tactical del 08/11 y la
  prueba del saque. El Tactical de áreas también los tiene, porque es copia del Tactical.
- Cargado en el robot (partido con saque, arco azul): el cartel y la telemetría confirmaron
  `umbrales 437/443/447`. **En el verde de esta cancha lee 36 / 37 / 71.**

## 2. ⚽ El partido con saque: más fuerza

| | Antes | Ahora |
|---|---|---|
| Patada normal | 190 × 500 | **225 × 500** (pasó por 210) |
| Avance (lejos) | 114 | **137** |
| Al llegar | 84 | **101** |
| Girar buscando | 42 | **50** |
| Pulsitos para enderezarse | 94 | **113** |
| Órbita (y la vuelta del saque) | 80 | **96** |

- El saque sigue igual (255 × 900, ya estaba al máximo).
- **Solo en el partido con saque.** El principal sigue con su patada en 190 y sus velocidades.
- **Riesgos que quedaron dichos:**
  - con más patada, que levante la trompa;
  - con los pulsitos a 113, que se pase al enderezarse;
  - con la órbita a 96, que pierda la pelota al rodearla, o que el saque a ciegas se pase de los 60°.

## 3. 🏁 Tactical de áreas (el desafío de hoy)

**El desafío:** desde un punto cualquiera, **entrar al área amarilla** y después **al área azul**,
guiándose con los arcos.

**Lo que se definió con el equipo:**
- el área tiene **línea blanca**;
- "entrar" es **cruzar la línea y frenar adentro**;
- al final **se queda quieto**.

**El programa:** `tactical-areas/` + **`TACTICAL-AREAS.bat`**. Es una copia del Tactical aprobado
el 08/11, que **no se tocó**.
1. **Área amarilla:**
   - Va hacia el arco amarillo con la cámara.
   - Cuando ve la línea, después de haber visto ese arco, sigue derecho `MS_ENTRAR` (400 ms) a
     `VEL_ENTRAR` **sin mirar la línea**, y frena adentro.
   - Espera 1 s.
2. **Sale para atrás** `MS_ENTRAR + MS_SALIR_EXTRA` (700 ms), sin mirar la línea. Si no saliera,
   yendo al azul confundiría la línea del área amarilla con la del área azul.
3. **Área azul:** lo mismo, y queda quieto. Al final muestra el tiempo de cada área.

**Números:** `VEL_ENTRAR` 80 → **96** (con el +20 %), `MS_ENTRAR` 400, `MS_SALIR_EXTRA` 300.
Velocidades +20 %: avance 114, llegada 84, giro 42, pulsitos 94.

**Riesgo:** si el punto al azar queda **adentro del área azul**, yendo al amarillo toma la línea del
área azul como si fuera la amarilla.

**Estado:** compila. **No quedó anotado si se cargó ni cómo salió.**

---

## ⚠️ Qué queda por VER

1. 🔴 **Cómo anduvo el robot en los partidos y en el Tactical de áreas.** No quedó escrito en el chat.
2. **Con la patada 225:** ¿se vuelca? ¿Sale derecha (con el freno de 200 ms antes)?
3. **Con las velocidades +20 %:** ¿se pasa al enderezarse o pierde la pelota al rodearla?
4. **Tactical de áreas:** ¿queda adentro del área con 400 ms a 96, o llega al fondo?

## 🔧 Qué queda por HACER

1. 🔴 **Cargar la última versión** del partido con saque, con el arco que toque, y leer el cartel:
   `patada: 225 x 500 ms`.
2. **Subir a GitHub** lo de hoy: los umbrales, el Tactical de áreas, la patada y las velocidades.
3. **Completar esta bitácora** con los resultados de la Roboliga.

---

## Nota de método

**Lo que salió bien:** se midió la línea **en la cancha de la competencia** antes de jugar, y los
números salieron clarísimos. Cada programa nuevo (el Tactical de áreas) se hizo **aparte**, sin
tocar el que ya estaba aprobado.

**Lo que falta:** con tantos cambios en un día de competencia, anotar después de cada partido qué
programa estaba cargado y qué hizo el robot. Dos renglones alcanzan.
