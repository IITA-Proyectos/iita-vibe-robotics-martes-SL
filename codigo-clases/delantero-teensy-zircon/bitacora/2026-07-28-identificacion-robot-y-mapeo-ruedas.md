# 2026-07-28 — Mapeo de ruedas medido, y el robot es el DELANTERO

**Quiénes:** Gustavo Viollaz (banco) + Claude (código y análisis)
**Robot:** el que el equipo llama **"robot 2"**
**Resultado:** es el **DELANTERO** → se compila con **`#define ROBOT2`**

---

## 🚨 Los dos "2" no son lo mismo

| | |
|---|---|
| **"robot 2"** | la etiqueta **física** que le puso el equipo |
| **`ROBOT2`** | el `#define` del código, que significa **DELANTERO** |

Acá **coinciden por casualidad**, pero no confíen en eso para el otro robot.
Lo mejor: dejar de numerarlos y etiquetarlos por puesto con indeleble.

---

## El resultado que importa

Medido, no deducido:

| Pines | Driver | Rueda |
|---|---|---|
| **8 / 7 / 6** | U17 | **IZQUIERDA** |
| **11 / 12 / 4** | U7 | **DERECHA** |
| **2 / 5 / 3** | U5 | **TRASERA** |

Con el `#define ROBOT2` del código 2025, eso significa:

| Motor del código | Pines | Rueda real | Lo que dice el comentario |
|---|---|---|---|
| `M1` | 8/7/6 | **izquierda** | *"motor derecho"* ❌ |
| `M2` | 11/12/4 | **derecha** | *"motor izquierdo"* ❌ |
| `M3` | 2/5/3 | **trasera** | *"motor atras"* ✅ |

### Por qué decimos que es el DELANTERO

La rueda **trasera** es el dato sin ambigüedad — "atrás" no se confunde con nada,
izquierda/derecha sí. Está en los pines **2/5/3**, y ésos son el `M3` del `ROBOT2`.
En el `ROBOT1` (arquero) el `M3` son los pines 11/12/4.

### Hallazgo: los rótulos izquierda/derecha del código 2025 están ESPEJADOS

`M1` está comentado como *"motor derecho"* y es físicamente el **izquierdo**; `M2` como
*"izquierdo"* y es el **derecho**. Los dos cambiados, limpio. Es el error clásico de escribir
izquierda/derecha **mirando al robot de frente** en vez de desde el robot.

**El firmware no está roto** — lo que engaña es la etiqueta. No hay que "arreglar" nada por esto;
hay que **no dejarse engañar por los comentarios** al leer el código. Importa porque
`adproporcional()` / `aiproporcional()` (avance derecho / izquierdo) y toda la lógica de patrulla
del arquero cuelgan de esos nombres.

---

## Cómo se llegó, incluidos los errores

Esta parte sirve más que el resultado.

### Lo que NO funcionó: preguntar "¿cuál rueda se mueve?"

Las primeras pruebas movían **una rueda por vez** en una secuencia cronometrada y pedían anotar
el orden. Dieron **resultados contradictorios entre corridas**, y sobre esos datos malos se
construyeron tres conclusiones que después hubo que dar de baja:

- ❌ *"el robot es el arquero"* — es el delantero
- ❌ *"la rueda derecha anda en un solo sentido"* — no era la derecha, y no era problema de sentido
- ❌ *"PWM = 0 no apaga el motor"* — se dedujo de una rueda mal identificada

**Por qué falla:** hay que seguir un orden con el ojo mientras pasan cosas, acordarse de qué paso
va y decidir rápido. Es fácil confundir "derecha" con "trasera" en un robot omni, donde las tres
ruedas están en diagonal y ninguna está claramente "al costado".

### Lo que SÍ funcionó: preguntar "¿cuál rueda quedó QUIETA?"

Mover **dos** ruedas y apagar **una**. Una rueda parada entre dos que giran **no se puede
confundir**: no hay que acordarse de nada ni seguir ningún orden.

Dos pruebas de ese tipo, apagando ruedas distintas, dieron resultados **consistentes entre sí**:

| Prueba | Se apagó | Quedó quieta | Se deduce |
|---|---|---|---|
| 1 | 11/12/4 | la derecha | 11/12/4 = derecha |
| 2 | 8/7/6 | la izquierda | 8/7/6 = izquierda |
| — | — | — | 2/5/3 = trasera, por descarte |

**Lección para el equipo:** al diseñar un test de banco, elegí siempre la pregunta más fácil de
contestar. **"¿Cuál falta?" es más confiable que "¿cuál es?"**.

---

## Estado del robot

✅ **Anda.** Con [`pruebas/adelante-atras/`](../pruebas/adelante-atras/adelante-atras.ino):
izquierda y derecha giran, la trasera queda quieta, alternando adelante y atrás.
Validado por Gustavo en banco.

Detalles operativos:

- **Potencia:** 130 resultó demasiado. Quedó en **70** y anda bien. El código 2025 usa 100 en
  `avanzar()`. Falta medir el piso real de arranque de cada rueda —
  [`pruebas/piso-de-pwm/`](../pruebas/piso-de-pwm/piso-de-pwm.ino) está escrito para eso.
- **Para apagar un motor** se ponen las dos patas de dirección en 0, no solo el PWM en 0. Queda
  como práctica del equipo por precaución. Si en esta placa `PWM = 0` alcanza o no,
  **NO está confirmado**: la observación que lo sugería vino de una rueda mal identificada.
  Hay que volver a medirlo limpio.

---

## Pendiente

- ⬜ Etiqueta física permanente: **DELANTERO (ROBOT2)** en este robot.
- ⬜ Correr [`quien-es-quien`](../pruebas/quien-es-quien/quien-es-quien.ino) en el **otro robot**.
  Si la trasera le da en los pines 11/12/4, es el arquero y quedan los dos identificados.
- ⬜ Medir el piso de PWM de las 6 combinaciones (3 ruedas × 2 sentidos).
- ⬜ Re-medir limpio si `PWM = 0` apaga el motor o no.
- ⬜ Verificar si `M1`=izquierda / `M2`=derecha vale también para el arquero. Se supone que sí
  (para eso está el `#define`), pero **no está medido**.

## Nota sobre el firmware que tenía adentro

Gustavo autorizó sobrescribirlo sabiendo que la memoria del Teensy **no se puede leer de vuelta**.
Se perdió la posibilidad de saber qué binario corrió en el Nacional 2025 en este robot. Para el
otro robot, si se quiere conservar esa evidencia: **observar su comportamiento antes de flashearlo**.

## Nota sobre el USB

A mitad de la sesión el Teensy dejó de enumerar y Windows reportaba *"Error de restablecimiento
de puerto"*. Se resolvió **cambiando de puerto USB**. Si vuelve a pasar en clase: primero cambiar
de puerto, después el cable.
