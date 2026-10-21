# 2026-10-20 — Base nueva, todo lo ajustable arriba, y la línea medida otra vez

**Quiénes:** Máximo y Juanse (mesa del delantero) + Claude (código, revisión y carga)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**PC:** la computadora móvil del equipo (`AGUSTINAAG`)
**Programa cargado al cerrar:** `funciona/delantero/`, entorno **`teensy41`** (el de prueba: elige
el arco mirando al prenderse), con los umbrales nuevos `424 / 434 / 440`. Lo cargó Claude y se
confirmó leyendo el cartel. 🔴 **Para un partido hay que cargar con `CARGAR-ROBOT.bat`**, que fija
el arco.

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🛠️ **Base nueva impresa en 3D** | para que la luz roja de los sensores no se refleje en la cancha, y queda más fachero |
| ✅ **Línea medida con la base puesta** | separa mejor que el 12/10. Umbrales **335/401/403 → 424/434/440** |
| 🎛️ **Panel de control nuevo** | las **80** variables ajustables arriba, en un solo bloque, cada una con su descripción. Pedido del profe |
| ✅ **El robot juega igual** | los tres programas de partido salieron idénticos byte por byte (antes de los umbrales) |
| 🔍 **14 descripciones corregidas** | una revisión independiente miró las 80 contra el código |
| 🧭 **Giroscopio verificado a mano** | con el programa principal: arranca en modo sin brújula y el rumbo acompaña el giro |
| 📛 **Nombre del robot / equipo** | TITÁN no lo aceptaron los jueces. Quedaron opciones, **sin decidir** |
| ⏳ **La patada sigue sin medirse** | segunda clase seguida. Es lo primero de la próxima |

---

## 1. 🛠️ La base nueva

El equipo le puso al robot una base de plástico de impresora 3D: *"para que no se refleje en la
cancha el color rojo del sensor, y así es más fachero"*. Es liviana, así que no cambia el peso ni
cómo vuelca.

Lo que se cuidó al ponerla, por si la vuelven a cambiar:

- **Que no agrande el robot.** El 12/10 se achicó justo para entrar en la medida de la competencia.
- **Que no roce el piso** (el borde de la cancha tiene rampitas), ni tape la cámara ni el frente.
- **Volver a medir la línea**, porque la base cambia la luz que les llega a los sensores (punto 2).

> Pendiente que se podía aprovechar con el robot abierto y no se hizo: **medir `L`** (del centro
> del robot al centro de una rueda) con regla. Sigue sin medirse desde el 12/10.

---

## 2. ✅ La línea, medida con la base puesta

`pruebas/grabar-linea/`, 53,1 s, 7 mesetas, clasificadas solas. Guardada en
`mediciones/medicion-2026-10-20_20-57-16.txt`.

| sensor | verde hasta | blanco desde | separación | umbral nuevo | era |
|---|---|---|---|---|---|
| 1 derecho | 134 | 714 | 580 | **424** | 335 |
| 2 izquierdo | 122 | 747 | 625 | **434** | 401 |
| 3 delantero | 175 | 705 | 530 | **440** | 403 |

- **La base no empeoró nada.** La separación es más grande que la del 12/10 (406/557/480). Los
  tres sensores quedan con más de 250 de margen para cada lado.
- Los umbrales viejos igual caían dentro del hueco: el robot no estaba ciego. Los nuevos quedan
  centrados para esta cancha con la base.
- **Se cambió sólo eso.** Comparando el programa compilado contra el de antes: las instrucciones
  son idénticas y lo único distinto son los tres números (`335/401/403` → `424/434/440`).

> Son números de **esta cancha**. En la de la competencia hay que volver a medir apenas se llega.

**Un susto que no es:** al cargarlo sobre la mesa, el cartel dijo `ya lee blanco al arrancar
(sensores 1 2 3)` y escapó de la línea sin parar. **Era la mesa**, que es blanca (leían
751/763/749). En el verde leen menos de 175. Pasó igual el 12/10.

---

## 3. 🎛️ El panel de control: todo lo ajustable arriba

**El pedido**, del profe Franco (bitácora del arquero del 13/10) y del equipo:

> *"Todo lo ajustable arriba, en un solo bloque, las variables que nosotros cambiamos y que vos
> creas necesario, porque puede haber ocasiones en que no podamos tener Claude ni internet. Y en
> lo posible con una mini descripción al lado de cada una."*

**Lo que quedó** en `funciona/delantero/delantero.ino`, líneas ~75 a ~222:

- **80 variables en 14 secciones:** interruptores, buscar, centrar, destrabe, avanzar, órbita,
  puntería, patada, patada derecha, rumbo cero, línea blanca, arranque, para qué lado gira, y
  avanzado.
- **Cada una en un renglón, con su descripción al lado:**
  ```cpp
  const int TRIM_PATADA = 5;   // compensacion fija. Si se va siempre a la DERECHA: subir. A la IZQUIERDA: bajar
  ```
- **Arriba de todo, una guía:** qué son ms, cm y fuerza (PWM), y las reglas de siempre: un cambio
  por vez, anotar en la bitácora qué se vio, leer el cartel después de cargar.
- **La historia de cada número no se borró.** El panel viejo pasó a ser "NOTAS DEL PANEL", justo
  abajo. Donde antes se declaraba cada variable quedó un comentario que dice que se mudó arriba.
- **7 variables que no hacen nada** (sobras del escape de línea viejo: `VEL_ESCAPE`,
  `MS_ESCAPE_EXTRA`, `MS_ESCAPE_COMPROMISO`, `MS_ESPERA_LINEA`, `MS_ESCAPE_CIEGO`, `VEL_FRENO`,
  `MS_FRENO`) no van al panel y quedaron marcadas `[SIN USO: cambiarla no hace nada]`.

**Cómo se comprobó que no cambió nada del juego:**

1. Los valores los copió un programa, no a mano, y después comparó los 88 uno por uno contra el
   archivo viejo. También comprobó que las 992 líneas de código quedaran iguales y en el mismo orden.
2. Se compilaron `teensy41`, `azul` y `amarillo` y se compararon contra lo compilado antes del
   cambio: **idénticos byte por byte**.

### 🔍 La revisión de las descripciones

Una descripción equivocada en un panel que se va a usar sin ayuda puede costar un partido. Cuatro
revisores independientes leyeron las 80 contra el código, y cada queja la miró otro revisor tratando
de refutarla. **14 quedaron confirmadas y se corrigieron.** Las tres que más importa saber:

1. **La puntería casi nunca usa `TOL_ANG_PELOTA` ni `TOL_ANG_ALINEADO`.** Valen sólo con el arco
   a menos de 50 cm (`ARCO_LEJOS_MIN`). En casi todos los tiros mandan `TOL_ANG_LEJOS_PELOTA` y
   `TOL_ANG_LEJOS_ARCO`. Para ajustar la puntería, son ésas.
2. **`MAX_DESTRABES` no es un tope y no suelta la pelota.** Gastados los 5 destrabes, espera otros
   3 s, vuelve el cupo a cero y sigue con **la misma** pelota. El programa imprime *"suelto esta
   pelota y busco de nuevo"*, y eso **no es cierto**. No se cambió (sería cambiar cómo juega).
3. **La órbita puede quedarse plantada después de recentrar.** *Hipótesis, sin medir.* El golpe
   de arranque (120) es sólo al entrar a orbitar. Después de recentrar, rearranca desde quieto
   con `VEL_ORB_TRASERA` = 67, apenas debajo del piso de ~70.

También quedó escrito que con `RUMBO0_ORBITANDO = false` el robot **no cuenta vueltas** y
`VETO_ARCO_PROPIO` **deja de proteger**.

---

## 4. 🧭 El giroscopio, con el programa principal

Se cargó el principal (entorno `teensy41`) desde esta PC. Esta vez la carga desde la sesión de
Claude sí anduvo (el 12/10, en otra PC, no). El cartel dijo:

```
Linea: sensores leen 751 / 763 / 749   umbrales 424 / 434 / 440   confirma 5 ms
Giroscopo: (fusion corriendo, SYS_STATUS=5) OK. Rumbo cero = 0.0 grados
   giroscopo SIN brujula (modo IMU): el cero no depende de los imanes
```

Después, con la batería apagada, el equipo giró el robot a mano y se leyó el rumbo cada 2 s:

```
0 -> 0 -> 287 -> 290 -> 15 -> 49 -> 355 -> 321 -> 321
```

- ✅ **El rumbo acompaña el giro y vuelve cerca del cero** (15 y 355 son ±5–15°). Nunca dijo
  `CAIDO`.
- ✅ **El modo sin brújula del 12/10 arranca y mide.** Era un pendiente desde ese día.
- ❓ **No se anotó cuánto lo giraron** en cada vuelta, así que no se puede decir si 287 son
  exactamente los grados que se giró. Para eso hay que girarlo contra una marca (90° justos).
- ⚠️ **A los 17 s se cortó el USB** (*"El dispositivo no reconoce el comando"*). Con el robot en la
  mano puede haber sido el cable. Ya se había caído el USB dos veces el 12/10: **revisar cable y
  conector**.

---

## 5. 📛 El nombre

- Propuesta de Claude, por Boca (el equipo lo pidió): **TITÁN** (por Palermo). **Los jueces
  dijeron que no es correcto.** No quedó anotado por qué.
- Después: la mesa del arquero es de **River**, así que hace falta un nombre para todo el equipo,
  no necesariamente de fútbol. Opciones que quedaron: **SUPERCLÁSICO** (un robot de Boca y uno de
  River en el mismo equipo), **PONCHO**, **CARDÓN**, **OMNI**, **EMPANADA SALTEÑA**.
- **No se decidió.** Hay que hablarlo con la mesa del arquero.

---

## 6. Otras cosas del día

- **La carpeta de la PC ahora tiene el repo completo** (antes sólo estaba la del delantero).
- **Las ideas del profe** (bitácora del arquero del 13/10), y cómo está el delantero:

| pide el profe | en el delantero |
|---|---|
| un `.bat` que guíe por cada parte del robot y dé los valores | ✅ existe `CALIBRAR-ROBOT.bat` (06/10). 🔴 nunca se corrió entero |
| todo lo ajustable arriba | ✅ **hecho hoy** |
| una herramienta para ajustar la corrección de rumbo de a poco | ✅ para la patada: `PROBAR-PATADA.bat` (13/10). 🔴 sin usar |
| que el robot se autodiagnostique con el giroscopio | ✅ para la patada. ❌ no para el avance ni la órbita |

- **Regla de trabajo del profe**, que Claude adopta también con esta mesa: antes de cambiar código,
  decir qué se entendió, mostrar qué hace hoy el programa y preguntar.

---

## 7. Números de hoy, todos juntos

```
LINEA (cancha del aula, con la base)  verde hasta 134/122/175 · blanco desde 714/747/705
                                      umbrales 424 / 434 / 440 (eran 335/401/403)
GIROSCOPIO       SYS_STATUS=5 · modo IMU sin brujula · rumbo cero 0.0
                 a mano: 0 -> 287 -> 15 -> 49 -> 355 -> 321 · nunca CAIDO
PANEL            80 variables arriba · 7 sin uso marcadas · 14 descripciones corregidas
                 teensy41 / azul / amarillo identicos byte por byte (antes de los umbrales)
CARGADO AL CERRAR  principal, entorno teensy41, umbrales 424/434/440
```

---

## ⚠️ Qué queda por VER

1. 🔴 **La patada.** Segunda clase sin medirla.
2. **Si el robot escapa bien de la línea** con los umbrales nuevos, en la cancha (sobre la mesa no
   se puede ver).
3. **El giroscopio contra una marca:** girarlo 90° justos y ver que el rumbo cambie ~90.
4. Si la órbita se planta después de recentrar (punto 3 de la revisión).

## 🔧 Qué queda por HACER — en orden

1. 🔴 **Antes de jugar: `CARGAR-ROBOT.bat`** con el arco que toque. Lo cargado hoy elige mirando.
2. 🔴 **Medir la patada** con `PROBAR-PATADA.bat`: sin tocar nada, después con la tecla `c`, y
   recién después el trim (bitácora del 13/10).
3. **Revisar el cable y el conector USB** del robot.
4. **Decidir el nombre** con la mesa del arquero.
5. **Correr `CALIBRAR-ROBOT.bat` entero** una vez, como ensayo de lo que se hace al llegar a la
   competencia.
6. Siguen de antes: medir `L`, la cámara (si el umbral del amarillo quedó grabado, el LED rojo,
   congelar ganancia), `tabla-camara`, el piso de PWM.

---

## Nota de método

**Hoy rindió no tener que adivinar.** El panel se movió con un programa que copiaba los valores
y se comprobó compilando, en vez de prometer que "no cambió nada". Las descripciones las revisó
gente que no las había escrito, y encontraron 14 problemas: tres de ellos llevaban a hacer lo
contrario de lo que había que hacer, o a tocar la perilla equivocada. Y el giroscopio se dio por bueno girándolo a mano, no porque el cartel dijera OK.

**Lo que falta es lo de siempre: medir la patada.** Ya hay herramienta, ya hay plan, ya está el
robot armado. Es lo primero de la próxima, antes que cualquier otra cosa.
