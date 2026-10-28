# 2026-10-27 (martes) — El panel de control, el despeje en diagonal y mirar antes de salir

**Quiénes:** Diego y Laureano (cancha, pruebas, ideas y pedidos)
+ Claude (código, revisión y carga)
**Robot:** el **ARQUERO** — Teensy con número de serie `13348550`
**Modificados:** `funciona/seguir-y-despejar` (de la 0.1020 a la **0.1027d**),
`pruebas/ajustar-movimientos` (movimiento 9, **no se cargó**), y el mapa de la EEPROM en
`medir-ancho` y `derecho-y-vuelta`
**En construcción, no subido:** `pruebas/tablero` (la ventana con botones, copiada del delantero)
**PC:** la notebook que viaja (usuario `sarap`). Plataforma `teensy` 6.0.0, Teensyduino 1.62.

---

## 🎯 EL RESUMEN DEL DÍA

**1. Todo lo que se ajusta está ahora arriba de todo, en el PANEL DE CONTROL** (§1). Era un pedido
del profe del 13/10. Se movieron 86 números sin cambiarles el valor.

**2. El despeje sale en DIAGONAL, mirando siempre al frente** (§2). Idea del equipo: usar la
predicción solo para despejar. El giroscopio no gira al robot: le impide girar mientras va en
diagonal.

**3. Una revisión encontró dos errores en la diagonal y un consejo al revés** (§3). Se arreglaron
antes de que el equipo ajustara números en cancha.

**4. En cancha: la diagonal no pasaba de ~10° y a las pelotas rápidas no llegaba** (§4). Con
`umbralDesvio` de 7 a 20, salía un poco más abierto, pero a las rápidas seguía sin llegar.

**5. Idea del equipo: que el robot se quede quieto un momento para medir la pelota** (§5). La
primera versión (c) esperaba demasiado y despejaba menos. La d espera menos. **En el robot quedó
la `0.1027d`, sin probar en cancha.**

**6. El tablero del arquero se está armando** (§7), copiado del que hizo hoy el delantero y con
una pestaña CANCHA para preparar una cancha nueva sin internet.

---

## 1. El PANEL DE CONTROL

Pedido del profe (13/10) y del equipo: *"poner todo lo ajusteable arriba del programa de
partido"*. El delantero ya lo había hecho el 20/10, y se copió la forma.

- **Diez secciones (más la 4b y la 4c)**, de lo que más se toca a lo que menos: 1 interruptores, 2 lo que se mide en
  cada cancha, 3 seguir la pelota, 4 el despeje, 4b la diagonal, 4c mirar antes de salir, 5 ir
  derecho, 6 enderezarse, 7 patrulla, ir al medio y límite lateral, 8 arranque y giroscopio, 9 para
  qué lado, 10 avanzado.
- Cada número tiene al lado **qué hace, si está medido o es de fe, y para qué lado tocarlo**.
- Las reglas de arriba: los decimales van con **punto** (`4.5`, no `4,5`), y los números que tienen
  una copia en las pruebas hay que cambiarlos en las dos partes.
- **Cómo se comprobó que no cambió nada:** un script copió las 86 declaraciones y comparó las 166
  del archivo antes y después. Compilado con el mismo cartel, **mide exactamente lo mismo que la
  0.1020** (código 26448, datos 7352).
- `INTENTOS_GIRO` estaba adentro de `setup()`; ahora está en el panel. `msAdelanteChico` y
  `VEL_ROBOT_CM_S` no se usan en ningún lado: quedaron donde estaban, con un aviso.

---

## 2. El despeje en diagonal

**La idea del equipo:** *"como la predicción no funcionaba tan bien, solo usémosla para los
despejes diagonales [...] cuando termine de despejar, que vuelva desde el mismo ángulo y se centre
de vuelta"*.

El equipo planteó dos opciones: (1) girar con el giroscopio hacia la pelota y despejar derecho, o
(2) ir en diagonal con los motores sin dejar de mirar al frente. **Eligieron la 2, "pero con el
giroscopio para mantenerse siempre al frente".**

**Cómo se calcula cada rueda** (la pregunta del equipo: *"¿o siempre es la misma velocidad?"*). No:
cambia con cada pelota. Cada rueda es la suma de tres cosas:

| | rueda izquierda | rueda derecha | rueda trasera |
|---|---|---|---|
| avanzar | + avance | − avance | 0 |
| correrse de costado | + 50 % costado | + 50 % costado | − 89 % costado |
| no girar (giroscopio) | + c | + c | + c |

- **El costado** sale de una regla de tres: si la pelota está a 30 cm y 6 al costado, hay que
  correrse 6 por cada 30 que se avanza. Más la velocidad de la pelota, si se pudo medir.
- **Ninguna rueda puede pasar de 210** (deja 45 para el giroscopio). Si no entra, se achican el
  avance y el costado **juntos**, para que la dirección no cambie.
- **La ida se estira** cuando el avance baja, para llegar igual de lejos.
- **La vuelta** desanda la misma diagonal.

Ejemplo con la pelota a 30 cm y 6 cm a la derecha: avance 186, costado 46 → izquierda 209,
derecha −163, trasera −40 (más la corrección), ida de 411 ms.

**No sale** si tendría que salir a más de 30° (`MAX_ANGULO_DIAGONAL`) o si el avance queda debajo
de 140. **Sin giroscopio sale derecho** (§3).

**La velocidad de la pelota** se usa solo si el robot no se movió de costado en los últimos 400 ms
(`MS_QUIETO_PARA_PREDECIR`), por lo de la cámara arriba del robot (bitácora del 20/10).

---

## 3. La revisión de la diagonal

Nueve agentes de revisión, solo leyendo el código (sin robot). Lo **confirmado y arreglado**, en la
0.1027b:

- 🚨 **Sin giroscopio salía en diagonal igual.** Sin rumbo, el robot gira, y después no se puede
  enderezar (por eso se había apagado el 15/09). Ahora en ese caso sale derecho, como el límite
  lateral, la patrulla e ir al medio.
- **En la ida, el costado arrancaba entero y el avance subía con la rampa**: salía unas 1,3 veces
  más abierto que lo calculado, y los primeros ~70 ms iba casi puro de costado. Ahora los dos suben
  juntos (`avanzarPersiguiendo`).
- 🚨 **El consejo para `rendimientoCostado` estaba al revés**, en el panel y en la herramienta. El
  costado se **divide** por ese número: **más chico = MÁS costado. Si no llega de costado, BAJARLO.**

Lo **descartado** por la verificación: que la vuelta casi no se corría de costado (el mismo
argumento valdría para la ida). Hay que mirarlo en cancha, pero no es un error del código.

Lo que **no se verificó** (quedó en pendientes): cerca del lateral, la diagonal puede cruzar la
línea del costado y la vuelta frenar en esa línea; y con la pelota muy cerca y corrida, el tope de
30° hace que no salga, cuando antes salía derecho.

---

## 4. En cancha: la 0.1027 y la 0.1027b

| Versión | Qué vio el equipo |
|---|---|
| **0.1027** | *"cuando la pelota va lento sí despeja pero cuando va rápido no se anima"*; la diagonal *"como mucho tiene un ángulo de 10 grados izquierda o derecha"* |
| **0.1027b** (arreglos + `umbralDesvio` 7 → 20) | *"salió un poco más, despejaba un poco más en diagonal"*; las rápidas *"no llega a despejarlas, pero sí reacciona"* |

**Por qué ~10°** (lectura de Claude, no medido aparte): con `umbralDesvio` 7, a 30 cm el máximo es
13°. Y como el robot sigue la pelota de costado, cuando sale la tiene casi enfrente (2 a 5 cm): de
4 a 10°.

**Por qué no llega a las rápidas:** apunta adonde la pelota **está**, no adonde **va**. La ida dura
~0,4 s: una pelota a 30 cm/s se corre 12 cm en ese rato. La predicción casi nunca actuaba, porque
el robot casi siempre se está moviendo de costado cuando la sigue.

**A 45° no puede salir:** tope de 30°, los motores dan como mucho ~38°, y a 45° se correría de
costado lo mismo que avanza (35 cm).

---

## 5. Mirar antes de salir (idea del equipo)

**La idea:** *"que si la pelota entra en rango de despeje, el robot se quede quieto y use ese
pequeño tiempo [100 a 300 ms] para calcular la trayectoria real"*, y *"si es necesario más tiempo o
más distancia, calculalo vos y probamos"*.

**Cómo quedó** (sección 4c del panel, fase `MIRANDO`):

1. Con la pelota a `distanciaParaMirar`, el robot **frena** y la **mira quieto**. Quieto, la
   cámara no confunde el movimiento del robot con el de la pelota.
2. Con todos los cuadros de la mirada traza la **recta** de la pelota (cuadrados mínimos). La
   pendiente es la velocidad, de costado y de acercamiento. Es menos ruidosa que restar dos
   lecturas.
3. Sale recién con la pelota a `umbralCm` (30), hacia donde **va a estar**. Si se viene encima, lo
   tiene en cuenta: se encuentran antes.
4. Por el cable (`MIRAR.bat`) escribe lo que vio: `MIRE 5 cuadros: a 28 cm, costado 6.0 cm, va de
   costado a 25 cm/s, se acerca a 40 cm/s -> SALE, costado 61` (o por qué no sale).

| Versión | Empieza a mirar | Mira como mínimo | Si la pelota no llega a 30 cm | Qué vio el equipo |
|---|---|---|---|---|
| **0.1027c** | 45 cm | 300 ms (100 frenando + 200 midiendo) | esperaba siempre hasta 700 ms | *"espera mucho para poder despejar y eso hace que despeje muchas menos pelotas que antes"* |
| **0.1027d** | 40 cm | 180 ms (60 + 120) y 3 cuadros | espera solo si la pelota **viene** y llega antes de 500 ms; si no, vuelve a seguirla enseguida | sin probar |

En la d, además: **si el robot ya estaba quieto** y la pelota ya está a 30 cm, no se frena a mirar
y sale enseguida, como la b.

Para volver a la b: `medirAntesDeDespejar = false`.

---

## 6. ¿El BNO puede medir la diagonal? (pregunta del equipo)

*"Con el BNO, ¿sería posible predecir algo de ese movimiento diagonal mirando al frente?"*

- **Mantenerlo mirando al frente: sí, y ya lo hace.**
- **Saber cuánto se corrió: no, en la práctica.** El giroscopio mide hacia dónde **mira**, no
  dónde **está**. El acelerómetro, integrado dos veces, junta error muy rápido (cuenta de Claude:
  ~1 cm en 0,4 s, ~5 cm en 1 s, ~45 cm en 3 s), y la vibración y la inclinación de la ida lo
  empeoran.
- Lo que resolvió el problema real (que la cámara se confunde con el robot moviéndose) fue **frenar
  y mirar quieto** (§5), no el BNO.

---

## 7. El tablero del arquero (en construcción)

El delantero subió hoy su **TABLERO**: una ventana con botones que habla con el robot por USB, con
pestañas PELOTA, SENSORES, MOVIMIENTOS y CONSOLA, robot de mentira (`--demo`) y autoprueba. El
equipo pidió *"una igual para nosotros con los movimientos para nuestro robot"*, y después
*"también ponele las mediciones, por ejemplo la del medio que teníamos, y las cosas necesarias por
si cambia la cancha"*.

Se está armando en segundo plano, con revisión al final:

- **MOVIMIENTOS**: los del arquero, copiados del partido 0.1027, con la mirada incluida.
- **CANCHA**: los pasos en orden para una cancha nueva (1ro `umbralBlanco`, 2do
  `MS_MEDIO_AL_MEDIO`...), con botón para que el robot mida, el valor de hoy al lado y "Copiar al
  panel". Una hoja de cancha guarda las mediciones de cada cancha. **Es el `.bat` de preparación de
  cancha sin internet que pidió el profe.**
- `TABLERO.bat` va a cargar solo si ve al **arquero**, y se niega con el delantero (`15708680`).

**No está subido** en este commit: se sube cuando esté revisado y probado con el robot.

---

## 8. Lo que quedó en el robot

| Cartel | Estado |
|---|---|
| `0.1027d EN PRUEBA (diagonal + MIRAR CORTO + umbralDesvio 20)` | 🚨 **LA QUE ESTÁ EN EL ROBOT.** Sin probar en cancha |

Es la ESTABLE 0.1013 con: el freno de 400 ms antes de volver (20/10), el despeje en diagonal, los
arreglos de la revisión, `umbralDesvio` 20 (para probar) y mirar antes de salir (versión corta).

---

## 9. Errores del día (míos)

1. **Escribí al revés el consejo de `rendimientoCostado`**, en el panel y en la herramienta. Lo
   encontró la revisión antes de que lo usaran.
2. **La 0.1027 se cargó antes de que terminara la revisión**, así que la primera prueba en cancha
   se hizo sin la protección del giroscopio y con la diagonal más abierta de lo calculado.
3. **La 0.1027c esperaba demasiado.** Elegí 45 cm, 300 ms y 700 ms pensando en una pelota que
   viene, y no en la que no viene: con esa, el robot se quedaba quieto en vez de seguirla.
4. **Al principio no encontré la herramienta del delantero**: todavía no estaba subida, y no lo
   pregunté hasta tarde.

---

## 10. Qué queda pendiente

### 🔴 Lo primero, en la cancha, con la 0.1027d

- 🔴 **¿Despeja tantas pelotas como la b?** Si no, comparar con `medirAntesDeDespejar = false`.
- 🔴 **Pasar las líneas `MIRE ...`** de MIRAR.bat de unas cuantas pelotas: dicen cuánto miró, qué
  velocidad midió y por qué salió o no.
- 🔴 **¿Llega a las pelotas rápidas?** Si dice `NO SALE (muy abierto)`, la próxima perilla es el
  tope de 30° (y el presupuesto de la rueda).
- 🔴 **`rendimientoCostado`** (0,80, de fe): si no llega de costado, **bajar** de a 0,05.
- 🔴 **Con `umbralDesvio` 20 y sin giroscopio** sale derecho a pelotas que derecho no alcanza: si el
  LED tiembla rápido, volver a 7.

### ⬜ Decisiones del equipo

- ⬜ **Pelota cerca y corrida** (a menos de ~12 cm y 7 al costado): el tope de 30° hace que no
  salga. ¿Salir derecho, o salir a 30°?
- ⬜ **Cerca del lateral**, la diagonal puede cruzar la línea del costado, y la vuelta frenar en esa
  línea creyendo que es la del área.

### ⬜ El tablero

- ⬜ Terminarlo, revisarlo y probarlo con el robot (MOVIMIENTOS y CANCHA).
- ⬜ La herramienta `ajustar-movimientos` (movimiento 9) tiene problemas conocidos de la revisión
  (cancelar borra el informe anterior, el ángulo puede ser de la pasada anterior, termina en el
  primer "no sale"). Si el tablero la reemplaza, no hace falta arreglarlos.

### ⬜ Arrastres del 20/10

- ⬜ La regla de 55/50 cm (dejar de seguirla de lejos y volver al medio).
- ⬜ Que no se aleje del arco: andar con los sensores de atrás sobre la línea del área.
- ⬜ La cámara a cada lado (37 / 50 / 54 cm): cinta a 50 cm y anotar.
- ⬜ Revisar el código del límite lateral. Medir con los motores nuevos.
- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
- ⬜ `cargar.py` no reconoce un Teensy en modo carga.
