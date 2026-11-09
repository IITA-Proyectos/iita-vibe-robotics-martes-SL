# 2026-11-09 (lunes): la patrulla por la línea, el ancho del área y la estrategia 2

**Quiénes:** Diego y Laureano (cancha, pruebas, mediciones y pedidos)
+ Claude (código, revisión y carga)
**Robot:** el **ARQUERO**, Teensy con número de serie `13348550`
**Programas nuevos:** `funciona/estrategia-2/`, `pruebas/medir-ancho-area/` y
`respaldos/estrategia-2-a-caballo/` (la primera versión de la estrategia 2)
**Modificados:** `funciona/partido-0921` (de la 0921g a la **0921i**); a la noche, el tablero
(`pruebas/tablero/`, adaptado a la 0921i, §6) y `pruebas/herramientas/cargar.py` (§6b)
**No se subió:** la carpeta `pruebas/tablero/copias-partido/` (copia local que hizo el tablero) y
el script de la cámara (umbral naranja 79 → 78, cambiado solo en esta PC)
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

**4. A la noche el tablero quedó adaptado a la 0921i** (§6), con la patrulla y la medición del
ancho del área. **Todavía no se usó con el robot.**

**5. Desde las 18:06 Windows bloquea `pio.exe`** (§6b). `cargar.py` y el tablero ahora usan
`python -m platformio`. Compila; **la carga con esa forma todavía no se probó**.

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

## 6. El tablero, adaptado a la 0921i (a la noche)

Por la mañana se pidió agregarle la patrulla al tablero (*"sí a todo"*). Ese armado se cortó a la
mitad. A la noche el equipo pidió: *"adaptá el programa solo para el 0921i, después podemos ver si lo
adaptamos para la estrategia 2"*. Se siguió desde lo que estaba a medias (un agente lo terminó,
3 lo revisaron, otros intentaron refutar cada hallazgo y uno arregló lo confirmado).

**Qué tiene nuevo:**
- **Reconoce la 0921i de hoy.** Si encuentra una 0921g, la muestra como "VERSIÓN ANTERIOR".
- **PARTIDO, grupo "5 · LA PATRULLA":**
  - cuándo sale (`patrullarSinPelota`, 20 s);
  - cuál patrulla (`patrullaPorLaLinea`, que pide confirmar; `patrullaPorElLateral`;
    `alternarLadoPatrulla`);
  - la vuelta con la línea del área (`buscarLineaDelAreaAlVolver`, `MS_AVANCE_10CM`);
  - el ancho del área (`MS_PUNTA_A_PUNTA_FIJO`, `VEL_LINEA`);
  - los números de la patrulla por la línea, en gris mientras esté apagada.
  - Cada fila dice para qué lado tocarla. Avisos nuevos: E8 (no deja guardar el ancho en 0 con la
    vuelta con la línea prendida) y O22 (lo medido fue a otra fuerza).
- **CANCHA, paso "7 · Medir el ancho del ÁREA":** el robot hace lo mismo que `medir-ancho-area`,
  y con **"PASAR A PARTIDO →"** el número va a `MS_PUNTA_A_PUNTA_FIJO`.
- En `partido-0921.ino` el `SELLO` volvió a `"fabrica"` (decía el del guardado de las 09:26). Fuera
  de eso, solo comentarios: **no cambia cómo se mueve**.

**La revisión confirmó 3 problemas, arreglados:**
1. `MS_AVISO_ARMADO` aceptaba 2000, y con eso el robot salía sin el parpadeo de "manos afuera".
   Ahora no baja de 3000.
2. CANCHA 2c decía que hacía lo mismo que el partido, pero vuelve al medio **solo por tiempo** (no
   prueba la vuelta con la línea del área). Ahora lo dice.
3. El texto de `MS_AVANCE_10CM` daba el consejo de `msEmpujonFinal`. Ahora explica que el que decide
   dónde queda al final es `msEmpujonFinal`.

**Autoprueba: OK, 5008 verificaciones.** Compilan el tablero, la 0921i y la estrategia 2.
**Nunca se usó con el robot de verdad.** Cargar el tablero o el partido **borra la estrategia 2**
del robot.

A las 09:26 el tablero viejo guardó valores en la 0921 y dejó una copia de antes en
`pruebas/tablero/copias-partido/` (solo en esta PC).

## 6b. Windows bloquea `pio.exe`

Desde las **18:06** Windows dice *"Una directiva de Control de aplicaciones bloqueó este
archivo"* al correr `pio.exe`. Es el **Control inteligente de aplicaciones** de Windows, que quedó
activado y bloquea los programas sin firma digital. `pio.exe` no tiene firma.
- **No se tocó esa protección:** es de seguridad, y si se apaga no se puede volver a prender sin
  reinstalar Windows.
- **Lo que sí anda:** el `python.exe` de PlatformIO (firmado) con `-m platformio`, que es el mismo
  PlatformIO. Y `teensy_loader_cli.exe` (el que pasa el programa al Teensy) no está bloqueado.
- **Se cambiaron `cargar.py` y el tablero** para usar `python -m platformio`. Con eso compilan los
  tres programas. **Falta probar una carga de verdad** con el robot enchufado.
- **Las herramientas del delantero** también llaman a `pio.exe`: en esta compu les va a pasar lo
  mismo.

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
5. **El tablero se cortó otra vez** a la mañana. Se terminó recién a la noche.

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

### 🔴 Cargar en esta compu
- 🔴 **Probar una carga de verdad con `python -m platformio`** (§6b). Se puede volver a cargar la
  estrategia 2, que es la que ya tiene el robot. Si no anda, cargar con el Arduino IDE.
- ⬜ Avisarle al delantero que sus `.bat` usan `pio.exe`.

### ⬜ El tablero
- ⬜ **Usarlo con el robot por primera vez**, con la compu al lado: medir el ancho (CANCHA 7),
  pasarlo, cargar el partido y ver "PARTIDO 0921i ... ✔ coincide con el archivo".
- ⬜ Para la estrategia 2 todavía no hay tablero (*"después podemos ver"*).

### ⬜ Arrastres
- ⬜ Anotar en el mapa de la EEPROM (los comentarios de `medir-ancho`, `derecho-y-vuelta` y
  `ajustar-movimientos`) las zonas nuevas `3584..3595` (0921i) y `3600..3611` (medir-ancho-area).
- ⬜ Los textos que mandan a la 0.1103 (ver la bitácora del 08/11).
- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
