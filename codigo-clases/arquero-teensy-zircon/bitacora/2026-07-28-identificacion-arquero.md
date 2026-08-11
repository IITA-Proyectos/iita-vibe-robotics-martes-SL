# 2026-07-28 — Identificado NUESTRO robot: es el ARQUERO

> 📌 **Nota agregada el 2026-08-04 — leer antes de creer que esta bitácora está mal.**
>
> Este archivo estuvo borrado del repo por un tiempo. Lo borró la sesión de la **otra mesa**
> ([`futbol-roboliga2026/bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md`](../../../futbol-roboliga2026/bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md)),
> que lo tomó por un error propio y lo dio de baja.
>
> **No era un error: son DOS ROBOTS DISTINTOS.** El equipo trabaja en dos mesas a la vez —
> una con el delantero y la otra con el arquero. Las dos midieron, las dos midieron bien, y
> les dio distinto porque **los robots están cableados distinto**. Que es, justamente, la
> razón por la que existe el `#define ROBOT1` / `ROBOT2`.
>
> | Robot | Trasera medida en | `M3` del... | Conclusión |
> |---|---|---|---|
> | El de esta mesa | **11/12/4** (U7) | `ROBOT1` | **ARQUERO** ✅ |
> | El de la otra mesa | **2/5/3** (U5) | `ROBOT2` | **DELANTERO** ✅ |
>
> Las dos conclusiones son correctas. Contradicen sólo si uno supone que hablan del mismo robot.
>
> 🚨 **Para que no vuelva a pasar:** las dos unidades se llaman "robot 2" en distintos momentos,
> y las dos mesas escriben en el mismo repo. **Antes de dar de baja la bitácora de otro, fijate
> si está hablando de tu robot.** Y lo que ya estaba propuesto en las dos bitácoras: etiquetar
> físicamente cada unidad con indeleble, **ARQUERO (ROBOT1)** y **DELANTERO (ROBOT2)**, y dejar
> de numerarlas.

**Quiénes:** Gustavo Viollaz (banco) + Claude (código y análisis)
**Robot:** el que Gustavo llama **"robot 2"** (etiqueta física del equipo)
**Programa cargado:** `pruebas/identificar-robot/identificar-robot.ino`, `MODO_UNA_RUEDA`, `VEL = 100`

---

## 🚨 Cuidado con los dos "2"

Hay dos numeraciones distintas y **no coinciden**. Confundirlas hace que se cargue el firmware
del robot equivocado.

| | |
|---|---|
| **"robot 2"** | la etiqueta **física** que le puso el equipo a esta unidad |
| **`ROBOT2`** | el `#define` del código, que significa **DELANTERO** |

**Este robot — el "robot 2" del equipo — es el ARQUERO, y se programa con `#define ROBOT1`.**

Sugerencia para que esto no vuelva a pasar: dejar de numerar los robots y llamarlos por su
puesto. Etiqueta física con indeleble: **ARQUERO (ROBOT1)** y **DELANTERO (ROBOT2)**.

---

## Qué queríamos probar

Los dos robots son idénticos por fuera y tienen las ruedas enchufadas en drivers distintos.
Queríamos saber cuál era cuál sin depender de abrir el chasis ni de seguir cables.

## Qué hicimos

Se cargó el sketch compilado como `ROBOT2`, modo "una rueda por vez", con el robot levantado y
las tres ruedas al aire. El sketch energiza un driver por vez, 2 s cada uno.

## Qué pasó de verdad

Orden observado, **en primera persona** (parado sobre el robot, mirando hacia adelante):

| Paso | Pines energizados | Driver | Rueda que giró |
|---|---|---|---|
| 1º (`M1`) | 8 / 7 / 6 | **U17** | derecha |
| 2º (`M2`) | 11 / 12 / 4 | **U7** | **atrás** |
| 3º (`M3`) | 2 / 5 / 3 | **U5** | izquierda |

Esto es **medición directa**, no interpretación: qué rueda física cuelga de qué driver.

---

## Conclusión: es el ARQUERO

La prueba está en la **rueda trasera**, que es la única etiqueta sin ambigüedad — "atrás" no se
puede confundir, izquierda/derecha sí.

La trasera cuelga de **U7 = pines 11/12/4**. Según los propios `.ino`:

- **Arquero (`ROBOT1`)** → `M3` = pines **11/12/4** ✅ **coincide**
- **Delantero (`ROBOT2`)** → `M3` = pines 2/5/3 ❌ (ahí medimos la rueda izquierda)

Y `M3` es *"motor atras"* en los comentarios de los dos archivos
(`arquero.ino:198`, `delantero.ino:222`).

**Corroboración física independiente:** `avanzar()` (`arquero.ino:151-155`) deja `M3` en **PWM 0**.
En un omni de tres ruedas, la que no aporta nada al avance recto es justamente la trasera. Los
dos caminos dan lo mismo.

---

## Hallazgo secundario: los rótulos izquierda/derecha del código 2025 están ESPEJADOS

Con el mapeo del arquero (`ROBOT1`), los comentarios del código dicen:

| Motor | Pines | Comentario del código 2025 | **Medido en banco** |
|---|---|---|---|
| `M1` | 2/5/3 (U5) | *"motor derecho"* | **izquierda** |
| `M2` | 8/7/6 (U17) | *"motor izquierdo"* | **derecha** |
| `M3` | 11/12/4 (U7) | *"motor atras"* | atrás ✅ |

**Los dos laterales están cambiados, limpio** — no es ruido de medición. Es el error clásico de
escribir izquierda/derecha **mirando al robot de frente** en vez de desde el robot. Gustavo midió
en primera persona, así que su marco es el correcto.

**Por qué importa:** las funciones de patrulla del arquero se llaman `adproporcional()` (avance
derecho) y `aiproporcional()` (avance izquierdo), y toda la lógica de "a qué lado voy" del arquero
cuelga de esos nombres. Si alguien asume que los comentarios dicen la verdad, va a razonar al
revés sobre hacia dónde se mueve el robot.

**Lo que NO cambia:** el robot 2025 funcionaba bien con estos nombres. El código no está roto —
lo que está mal es la **etiqueta**, no el comportamiento. No hay que "arreglar" nada en el
firmware por esto; hay que **no dejarse engañar por los comentarios** al leerlo.

---

## ✅ Confirmación: HECHA, y dio bien

Se recompiló el mismo sketch como **`ROBOT1`** y se cargó (verificado por serie: imprime
`compilado como ROBOT1 (ARQUERO)`). El criterio era:

> Con el `#define` correcto, la **tercera** rueda en girar (`M3`) tiene que ser la **de atrás**.

**Segunda corrida — predicho vs. observado:**

| Paso | Pines | Driver | Predicho desde la 1ª medición | **Observado** |
|---|---|---|---|---|
| 1º `M1` | 2/5/3 | U5 | izquierda | **izquierda** ✅ |
| 2º `M2` | 8/7/6 | U17 | derecha | **derecha** ✅ |
| 3º `M3` | 11/12/4 | U7 | **atrás** | **atrás** ✅ |

**Tres de tres.** La tercera rueda es la trasera → el `#define` compilado es el correcto.

**Cerrado: este robot es el ARQUERO y se compila con `#define ROBOT1`.**

Dos corridas independientes, con dos `#define` distintos, dan el mismo mapeo físico. La
identificación no descansa en ninguna suposición: U5 = izquierda, U17 = derecha, U7 = atrás,
medido dos veces.

De paso, la segunda corrida **vuelve a confirmar el espejado**: con `ROBOT1`, `M1` (comentado
*"motor derecho"*) es físicamente la rueda **izquierda**, y `M2` (*"motor izquierdo"*) es la
**derecha**. Mismo resultado que la primera corrida, por otro camino.

---

## Qué queda pendiente

- ⬜ Etiqueta física permanente en este robot: **ARQUERO (ROBOT1)**.
- ⬜ Correr la misma prueba en el otro robot. Compilado como `ROBOT2`, la tercera rueda tiene que
  ser la trasera. Si da eso, es el delantero y quedan los dos identificados.

## Nota sobre el firmware que tenía adentro

Gustavo autorizó explícitamente sobrescribirlo, sabiendo que la memoria del Teensy **no se puede
leer de vuelta**. O sea: **se perdió la posibilidad de saber qué binario corrió en el Nacional
2025** en este robot. Queda la misma duda abierta para el otro robot: si se quiere conservar esa
evidencia, hay que observar su comportamiento **antes** de flashearlo (el arquero barre de lado a
lado al encender; el delantero avanza y después gira sobre su eje).
