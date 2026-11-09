# 2026-11-09 (lunes): la patrulla por la línea, el ancho del área y la estrategia 2

**Quiénes:** Diego y Laureano (cancha, pruebas, mediciones y pedidos)
+ Claude (código, revisión y carga)
**Robot:** el **ARQUERO**, Teensy con número de serie `13348550`
**Programas nuevos:** `funciona/estrategia-2/`, `pruebas/medir-ancho-area/` y
`respaldos/estrategia-2-a-caballo/` (la primera versión de la estrategia 2)
**Modificados:** `funciona/partido-0921` (de la 0921g a la **0921i**)
**No se subió:** el tablero (`pruebas/tablero/`, quedó a medio adaptar, §6), la carpeta
`pruebas/tablero/copias-partido/` (copia local que hizo el tablero) y el script de la cámara
(umbral naranja 79 → 78, cambiado solo en esta PC)
**PC:** la notebook que viaja. Plataforma `teensy` 6.0.0, Teensyduino 1.62.

---

## 🎯 EL RESUMEN DEL DÍA

**1. Hay un programa nuevo, la estrategia 2** (§4). El arquero se mueve **solo de costado, con los
sensores de atrás arriba de la línea del área**, siguiendo la pelota. Así no se aleja del arco.
**La versión B se probó y anda.** Antes de un partido se le agregó el despeje, **sin probar**, y
eso es **lo que quedó en el robot**.

**2. El programa de juego pasó a la 0921i** (§2):
- arranca a los 3 s de prenderlo;
- la patrulla va a los dos laterales, uno cada vez, a los 20 s sin pelota;
- volviendo al medio, si encuentra la línea del área avanza 15 cm y va al medio con el ancho medido.

**3. Se midió el ancho del área:** 2548 ms de punta a punta a 90 (§3).

**4. El tablero NO está adaptado a la 0921i** (§6). **No usar `PARTIDO.bat` ni `TABLERO.bat`.**

---

## 1. La patrulla por la línea (a la mañana)

**El pedido:** *"en el partido que tuvimos la clase pasada el robot se iba del arco y regalaba el
gol"*. Querían que, después de un rato sin pelota, el robot encontrara la línea del área y la
recorriera con los sensores de atrás hasta que un sensor no la viera más, y de ahí volviera al
medio.

**Cómo se llegó:**
- **Primera versión:** se corregía adelante/atrás todo el tiempo para quedar en la línea. *"Anda
  muy improlijo y oscila"*: en una prueba de 3 encontró la punta, y en esa se metió en el área.
- **Segunda, idea del equipo:** retrocede hasta la línea, **espera 3 s quieto** (para que vean
  dónde quedó), y va de costado **solo con el giroscopio**, sin corregir con los sensores. La punta
  es cuando el sensor de atrás del lado hacia el que va ve verde un rato seguido. *"Estuvo bien. A
  la izquierda perfecto"*. A la derecha, después de la punta, hacía un movimiento de más.
- **Tercera, pedido del equipo:** *"sería mejor medir por tiempo"* (opción b, *"pero guardalo en la
  memoria"*). Ahora busca la punta **solo a la izquierda**, y de ahí va al medio **por tiempo**: la
  mitad del ancho medido. El tiempo queda guardado en la EEPROM (zona `3584..3595`, marca `LIN1`).

Esta patrulla **quedó escrita en la 0921i pero apagada** (`patrullaPorLaLinea = false`). El equipo
después eligió la patrulla de antes (§2).

---

## 2. La 0921i, en el orden en que se pidió

| Pedido | Qué cambió |
|---|---|
| *"necesitamos que en 3 segundos arranque a moverse"* | `MS_AVISO_ARMADO` 2000 → **3000**, y **se cuenta desde que se prende**. El LED parpadea rápido solo el último segundo |
| *"apagale la patrulla para el primer partido"* | apagada para ese partido |
| *"el avance después de encontrar la línea del área cambiala a 15"* | `msEmpujonFinal` 320 → **450** ms (~15 cm) |
| *"activa la patrulla"* + *"la de antes, que iba hacia un lateral y luego volvía al medio por tiempo"* + *"que se vaya hacia ambos lados"* + *"que inicie a 20"* | la patrulla vieja, **alternando de lado** (`alternarLadoPatrulla`), a los **20 s** sin pelota (`MS_QUIETO_PARA_PATRULLAR` 10000 → 20000) |
| *"si al volver al medio por tiempo ve la línea blanca, que avance 10 cm"* y después *"ponele a 15"* | `buscarLineaDelAreaAlVolver`: volviendo del lateral, primero tiene que ver verde 150 ms y después blanco 20 ms. Ahí **avanza 15 cm** (`MS_AVANCE_10CM` = 450) y va al medio con **1274 ms** (la mitad del ancho medido, §3) |

**La corrección del equipo:** al principio entendí que, después de ver la línea, iba con el tiempo
del lateral. *"No, la idea es que si toca la línea del área, que vuelva por el tiempo que medimos
para la nueva patrulla de hoy"*. Así quedó.

**Sigue igual:** el límite lateral (`volverAlMedioEnElLateral`), ir al medio si no hay línea, y lo
demás de la 0921g. La línea `#PARTIDO` ahora dice `version=0921i`.

**Lo que dijo el equipo después:** *"la dirección de la patrulla es aleatoria"*. Propusieron una
prueba para distinguir el lateral de la cancha de la línea del área: avanzar 15 cm y hacer un
pulsito para ver si sigue viendo la línea. Les contesté que creo que la regla va al revés, y
les pregunté dos cosas que **no se contestaron**. Se pasó a la estrategia 2.

---

## 3. El ancho del área: `pruebas/medir-ancho-area/`

Pedido: un programa aparte que, **desde adentro del área**, vaya a un costado hasta que el sensor
de adelante vea la línea y después cruce al otro costado, midiendo el tiempo.

- **Primer problema:** al frenar se pasaba un poco de la primera línea, y al volver **la contaba
  como el otro costado**. Arreglado: primero tiene que salir de la primera línea (ver verde) y
  recién después cuenta el blanco.
- **Medido: 2548 ms a 90.** Se puso fijo en la 0921i (`MS_PUNTA_A_PUNTA_FIJO = 2548`).
- Guarda la última medición en la EEPROM (zona `3600..3611`, marca `MAA1`). Por el cable dice
  `MEDIR-ANCHO-AREA: la ultima medicion fue X ms`.

---

## 4. La estrategia 2

**El pedido:** *"otro programa que se desplace lateralmente por la línea [...] que el robot
solamente se desplace por esa línea del área para nunca quedarse lejos y dejar descubierto el
arco"*, con el límite en las puntas, y *"al arrancar que retroceda hasta la línea"*.

### Versión A, "a caballo" (en `respaldos/estrategia-2-a-caballo/`)
La línea queda **entre** el sensor de adelante (en la cancha) y los de atrás (adentro del área), como
lo pidió el equipo. Sigue la pelota de costado con los números de la 0921 (70 a 160, zona muerta
1,4 cm).

**Prueba:** *"funciona el programa, pero los sensores de atrás detectan a cada rato la línea blanca
del área y no avanza"*. El margen es casi cero: la distancia entre sensores apenas supera el ancho
de la línea. Además, cuando un solo sensor de atrás veía blanco, el robot frenaba a chequear si era
la esquina.

### Versión B (`funciona/estrategia-2/`): la que quedó
De 5 ideas, el equipo eligió la 5: **los dos sensores de atrás ARRIBA de la línea**, y el de
adelante en el verde de la cancha. Ver blanco atrás pasa a ser lo normal.
1. A los 3 s de prenderlo retrocede hasta que un sensor de atrás ve la línea, y frena.
2. Sigue la pelota de costado (70 a 160).
3. Se acomoda solo si dura un ratito: si el de adelante ve blanco, va para adelante; si los dos de
   atrás ven verde, va para atrás. En los dos casos sigue hasta volver a pisar la línea.
4. **La punta:** yendo para un lado, si el sensor de ese lado ve verde y el otro sigue en blanco,
   frena y hace un pulsito para atrás. Si vuelve a ver blanco, se había corrido; si no, es la punta
   y no sigue para ese lado.

**Prueba:** *"esta bien, funciona"*. ✅

### El despeje (agregado a las apuradas, **sin probar**)
*"Agregale el despeje. Ahora más que nunca es importante que al volver quede alineado [...] que en
2 minutos hay partido"*.
- Con la pelota a **30 cm o menos y de frente** (3 cuadros seguidos), sale con **200 durante 533
  ms**, corriéndose hacia la pelota. Son los números de la 0921.
- Frena 0,3 s, **se endereza con el giroscopio** (3° de margen), **vuelve derecho para atrás** a 110
  hasta la línea y ahí se acomoda arriba de ella.
- Se apaga con `despejeActivado = false`, y queda igual que la versión B probada.

### La revisión de la versión B
Tres revisores (estados, escenarios en la cancha, convenciones), y cada hallazgo lo revisó otro
agente que intentó refutarlo: **15 agentes, 12 hallazgos, 8 confirmados**. Juntando los repetidos
quedan **3 problemas, sin arreglar todavía**:
1. **Se puede salir por la esquina:** mientras se acomoda adelante/atrás, sigue a la pelota de
   costado y no mira la punta.
2. **Empujado hacia el arco, se mete adentro:** si los tres sensores ven verde, siempre cree que se
   fue para adelante y retrocede. Hay que acordarse de cuál fue el último sensor que vio blanco.
3. **Puntas falsas:** al acomodarse frena apenas toca el blanco y queda en el borde de la línea.
   Hay que empujar un ratito más (~0,04 s).

El despeje casi no entró en la revisión: se agregó mientras corría.

---

## 5. Lo que se cargó hoy, en orden

1. La patrulla por la línea, varias veces (§1).
2. `medir-ancho-area`, dos veces (§3).
3. La 0921i, con cada cambio del §2.
4. La estrategia 2 A, la B, y la B con despeje.

| En el robot | Estado |
|---|---|
| **estrategia 2, versión B con despeje** | 🚨 **LA QUE ESTÁ EN EL ROBOT.** La B sin despeje se probó y anda. **El despeje, el enderezarse y la vuelta a la línea nunca se probaron** |

Para volver al programa de juego hay que cargar `funciona/partido-0921` (0921i).

---

## 6. El tablero

Por la mañana se pidió agregarle la patrulla al tablero (*"sí a todo"*). El armado se cortó a la
mitad. `tablero.py` y `tablero.ino` quedaron modificados en esta PC, **su autoprueba falla**, y no
saben de la 0921i.
- **No se subieron.** En GitHub sigue el tablero del 08/11, que espera `version=0921g`.
- **No usar `PARTIDO.bat` ni `TABLERO.bat`** hasta adaptarlo: el tablero del 08/11 no reconoce la
  0921i.
- A las 09:26 el tablero guardó valores en la 0921 y dejó una copia de antes en
  `pruebas/tablero/copias-partido/` (solo en esta PC).

---

## 7. Errores del día (míos)

1. **La primera patrulla por la línea oscilaba.** Corregía adelante/atrás todo el tiempo; la idea
   del equipo (solo giroscopio + pausas de 3 s) anduvo mejor.
2. **`medir-ancho-area` contaba dos veces la misma línea.** No tuve en cuenta que al frenar se
   pasaba.
3. **La estrategia 2 A tenía un margen casi cero.** El equipo lo había avisado (*"es posible que vea
   la línea blanca"*).
4. **El despeje se cargó sin probar.** Fue a pedido y por el apuro, pero el robot jugó con algo sin
   probar.
5. **El tablero se cortó otra vez** y quedó a medio adaptar.

---

## 8. Qué queda pendiente

### 🔴 La estrategia 2
- 🔴 **Contar cómo anduvo el despeje en el partido**: si se endereza, si vuelve a la línea y si
  queda alineado.
- 🔴 **Arreglar los 3 problemas de la revisión** (§4), decírselo antes al equipo y probarlos.
- ⬜ **Medir los sensores con `pruebas/calibrar-linea`** y ponerle a cada uno su umbral. En la B,
  si un sensor de atrás confunde el verde con blanco, no ve la punta y se pasa.
- ⬜ Patrulla: todavía no tiene.

### ⬜ El programa de juego (0921i)
- ⬜ **La vuelta con la línea del área** (15 cm + 1274 ms) **no tiene resultado de prueba claro.**
- ⬜ **Recordar de qué lado se fue la pelota** y buscarla con pulsitos (*"primero vamos al
  partido y luego vemos"*). Propuesta: hasta 4 pulsitos, solo si el último desvío era de más de
  5 cm, con un interruptor para apagarlo.
- ⬜ `MS_MEDIO_AL_MEDIO` (2228) se midió antes de mover los motores.

### ⬜ El tablero
- ⬜ Adaptarlo a la 0921i: la patrulla, la medición del ancho del área y los valores de fábrica
  nuevos.

### ⬜ Arrastres
- ⬜ Anotar en el mapa de la EEPROM (los comentarios de `medir-ancho`, `derecho-y-vuelta` y
  `ajustar-movimientos`) las zonas nuevas `3584..3595` (0921i) y `3600..3611` (medir-ancho-area).
- ⬜ Los textos que mandan a la 0.1103 (ver la bitácora del 08/11).
- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
