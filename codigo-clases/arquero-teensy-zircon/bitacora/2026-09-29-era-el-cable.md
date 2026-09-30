# 2026-09-29 (martes) — Era el cable

**Quiénes:** el equipo (pruebas en cancha y en mesa, y **el hallazgo**) + Claude (código,
carga y lectura por USB)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas nuevos:** `pruebas/derecho-y-vuelta`, `pruebas/giroscopo-dfrobot`,
`pruebas/cuando-se-cae` (con `PROBAR.bat`)
**Modificados:** `pruebas/medir-ancho`, `pruebas/monitor-robot`,
`pruebas/cuadrado-giroscopo`, `funciona/seguir-y-despejar`

---

## 🎯 EL TITULAR

**Después de cuatro clases, el equipo encontró la causa: el cable del giroscopio está roto o
hace falso contacto.** Lo encontraron meneándolo con la mano — al moverlo, el sensor aparecía
y contestaba por consola.

No era el código. No era la batería. No era la librería. No era el bus. **Era un cable.**

Y la clase entera fue el camino hasta ahí, con el paso decisivo a mitad de tarde:

> **QUIETO ANDA. MOVIÉNDOSE SE CAE.**

---

| | |
|---|---|
| §1 | La batería estaba apagada, y eso anula la medición del 22/09 |
| §2 | Cómo usa el giroscopio el delantero (lectura completa) |
| §3 | `derecho-y-vuelta`: el truco del delantero, medido en nuestro robot |
| §4 | 🚨 El bug que casi nos hizo creer que el robot iba perfecto |
| §5 | Los programas de DFRobot |
| §6 | 🎯 La medición que partió el problema al medio |
| §7 | 🎯 **EL HALLAZGO: el cable** |
| §8 | Tres regalos de la última corrida |
| §9 | `cuando-se-cae` y `PROBAR.bat` |
| §10 | Volver a medir el ancho: dos cosas que salieron mal |
| §11 | El arranque del giroscopio, rehecho |
| §12 | Errores del día (incluidos los míos) |
| §13 | Números de hoy |
| §14 | Qué queda pendiente |
| §15 | Lo que se aprendió del método |

---

## 1. La batería estaba apagada, y eso anula la medición del 22/09

La primera cosa que se aclaró hoy es que **la batería estaba apagada**, con el robot
enchufado por USB.

Eso importa mucho, porque el titular de la bitácora del 22/09 fue
**"0 lecturas buenas de 5453"** y quedó escrito como la medición principal del día. Con la
batería apagada **el giroscopio no tiene corriente**: se alimenta de la batería, no del USB.
Un sensor sin alimentación no contesta, y eso es obvio, no es un hallazgo.

⚠️ **Esa medición no probaba nada sobre el conector.** Queda anulada.

Y hay otra trampa encadenada, que viene del 21/09: **con el USB ya puesto, prender la batería
NO reinicia el Teensy.** El programa ya arrancó — sin giroscopio — y se queda con ese
resultado. Prender la batería después no lo cambia. El orden que funciona es:

    prender la bateria  ->  enchufar el USB  ->  y si hace falta, recargar para reiniciar

---

## 2. Cómo usa el giroscopio el delantero (lectura completa)

Pedido del equipo. La otra mesa está en **NDOF (con brújula)** igual que nosotros —
`bno.begin()` sin argumento usa ese modo por defecto— pero **nunca usa el rumbo absoluto**.
Tiene tres usos, todos relativos.

### 2.1 Heading-hold durante la patada

El más interesante, y el que nos sirve directo. Al empezar la patada guarda el rumbo de ese
instante, y durante el golpe corrige contra ése:

```c
if (nuevo == PATEA_ADEL && giroscopoSano()) rumboAlPatear = rumboActual();
...
void avanzarDerecho(int vel, float rumboObjetivo) {
  float err = diferencia(rumboObjetivo, rumboActual());
  int resta = (int)(fabs(err) * KP_PATADA);        // KP = 4.0
  if (resta > RESTA_MAX) resta = RESTA_MAX;        // tope 120
  if (err > 0) vd -= resta; else vi -= resta;      // SOLO frena, nunca acelera
  ...
}
```

**Lo midieron ellos:**

| | Cuánto se torcía |
|---|---|
| Sin corrección | **10,1°** (pico 11,3) |
| Con heading-hold, KP 4.0 | **4,2°** (pico 5,4) |

El diagnóstico que hicieron es de manual: `avanzar()` manda **el mismo PWM** a las dos ruedas
de adelante, pero el mismo PWM no es la misma velocidad — son dos motores distintos. Y la
rueda trasera queda **suelta**, así que no hay nada que se oponga al giro. Cualquier
desbalance se convierte en curva y nadie la corrige.

> *"No era el contacto con la pelota — es la trayectoria."*

🎯 **El truco que hay que robarles: la corrección SOLO RESTA.** Lo intuitivo sería "si me voy
para la izquierda, acelero la izquierda". Pero a 240 sobre un máximo de 255 no queda lugar
para subir: el lazo pediría acelerar, saturaría y no pasaría nada. Frenando la del lado
contrario siempre se puede, incluso a fondo.

**Nuestro despeje tiene el mismo problema:** sale a 200, derecho, con la trasera suelta.

### 2.2 `rumboCero` — hacia dónde miraba al encenderse

Se fija una vez al arrancar y significa "hacia el arco rival". Dos usos:

- **Si da la vuelta entera y no encuentra el arco**, en vez de rendirse gira hasta mirar al
  `rumboCero` y patea ahí (`PATEAR_AL_RUMBO0`).
- **Para elegir hacia qué lado orbitar**, por el camino más corto:
  `haciaElPositivo = (diferencia(rumboCero, rumboActual()) > 0)`.

### 2.3 Saber si el sensor está vivo

```c
bool giroscopoSano() { return hayGiroscopo && !giroCaido; }
```

y `giroCaido` sale de **preguntarle al chip** cada 500 ms si su fusión sigue corriendo
(`SYS_STATUS == 5`), no de contar ceros.

### 2.4 Cómo lo inicializan

```c
bool arrancarGiroscopo() {
  if (!bno.begin()) return false;        // UN solo intento, NDOF
  bno.setExtCrystalUse(true);
  delay(700);                            // AHORA despues: el cristal reinicia la fusion
  bno.getSystemStatus(&sys, &autotest, &err);
  if (sys != 5) return false;            // 5 = la fusion esta corriendo
  ...
}
```

**Verifiqué el comentario del `delay` en la librería y es cierto.** `setExtCrystalUse()` hace
`setMode(CONFIG)` → escribe → `setMode(el modo de antes)`. Pasar por CONFIG **reinicia la
fusión**, así que cualquier espera anterior se tira a la basura. Ellos lo descubrieron y
movieron el `delay` después.

### 2.5 Comparado con el nuestro

| | Delantero | Arquero |
|---|---|---|
| Reintentos de `begin()` | 1 | 10 ✅ |
| Orden `setExtCrystalUse` / espera | crystal → delay ✅ | delay → crystal ⚠️ |
| Cómo confirma que está vivo | `SYS_STATUS == 5` ✅ | espera un dato ≠ 0 |
| Espera la calibración del giroscopo | ❌ | ❌ |
| Si falla | avisa por consola | **LED rápido** ✅ |

### 2.6 Dos cosas del enfoque que valen más que el código

**Todo uso del giroscopio tiene un camino sin él.** La perilla `USAR_GIROSCOPO` arrancó
apagada, con este comentario:

> *"⚠️ EL FIRMWARE NUNCA LO USÓ... en ESTA placa no está comprobado que el sensor conteste.
> Por eso arranca apagado y por eso, si falla, el robot sigue andando como hasta ayer."*

Con el arco a la vista orbita hacia él; sin verlo y con giroscopio, hacia el rumbo de
arranque; y sin giroscopio, al lado de siempre. **Nunca se queda sin plan.**

**Y no aprovechan nada de la brújula.** `rumboCero` se captura al encender, igual que nuestro
`rumboBase`. Si pasaran a IMUPLUS el programa funcionaría idéntico, y dejarían de estar
expuestos al salto de golpe. Igual que nosotros, tampoco re-anclan el cero en ningún lado.

---

## 3. `derecho-y-vuelta`: el truco del delantero, medido en nuestro robot

Pedido del equipo: un programa simple de avance y retroceso usando su lógica.

Se hizo midiendo **las dos cosas en la misma corrida**, para que la comparación sea justa
—mismos motores, misma batería, mismo piso—:

```
ADELANTE sin corregir  ->  mide cuanto se torcio
ATRAS    sin corregir  ->  mide
ADELANTE corrigiendo   ->  mide
ATRAS    corrigiendo   ->  mide
```

Por eso hace el adelante-atrás **dos veces**: el primer par es el control.

**Una cosa que tuve que agregar y el delantero no necesita:** al retroceder, el lado a frenar
**se da vuelta**. Frenar la rueda izquierda yendo para adelante tuerce el robot para un lado;
yendo para atrás, la misma rueda frenada lo tuerce para el otro. El delantero sólo patea
hacia adelante, así que nunca se lo cruzó.

### A pedido del equipo: casi la velocidad mínima y 2 metros

| | Antes | Ahora |
|---|---|---|
| Potencia | 200 | **80** |
| Duración del tramo | 533 ms | **5000 ms** |
| Distancia | ~50 cm | **~2 m** (estimado) |

Dos ajustes obligados por bajar tanto la velocidad:

**Un empujoncito de arranque.** El piso de arranque de estos motores es **~70 PWM**; a 80 el
robot está apenas 10 arriba, y ahí la diferencia entre los dos motores pesa muchísimo más que
a 200 — es muy posible que uno arranque y el otro no. Los primeros 120 ms va a 140 y después
baja a 80.

**El tope de la corrección, de 120 a 45.** El delantero usa 120 sobre una patada de 200 — el
60%. Con 120 sobre una marcha de 80, la primera corrección deja la rueda en cero y el robot
pivotea en el lugar en vez de corregir.

⚠️ Los 2 metros son **una estimación**, no una medición: salen de una regla de tres sobre la
calibración del 04/08 (1 cm cada 10 ms a potencia 200). Cerca del piso de arranque la
velocidad **no** es proporcional al PWM. Hay que mirar cuánto avanza de verdad.

### El historial en la EEPROM

Sin cable el `Serial.println` se tira, así que cada corrida se guarda en la EEPROM: las
últimas 12 con sus cuatro desvíos, más dos contadores que no se pierden nunca — **cuántas
veces se corrió y en cuántas arrancó el giroscopio**. Ese segundo número es el que se venía
contando a mano.

🎯 **También se anotan las corridas en que NO arranca.** Son la mitad del dato: sin ellas la
cuenta de "cuántas de cada diez" no sale.

### El LED, unificado

A pedido del equipo: **parpadeo rápido = no hay giroscopio**, en los cinco programas. Antes
cada uno hablaba un idioma distinto —triple parpadeo, LED apagado, parpadeo rápido— y en la
cancha, si no te acordás cuál está cargado, no dice nada.

### Las 7 pruebas del equipo

**2 no arrancaron** (parpadeo rápido, y el programa no se movió). **5 arrancaron pero iban un
poco chuecas.** O sea **5 de 7**, el primer número real de la intermitencia.

---

## 4. 🚨 El bug que casi nos hizo creer que el robot iba perfecto

La primera tabla del historial salió así:

```
 n   giro  cal    adel-sin  atras-sin  adel-con  atras-con
  1    SI    3      2.8      0.0      0.0      0.0
  2    SI    3      0.0      5.1      1.3      8.9
```

**Cuatro ceros exactos.** Un desvío de 0,0 después de 2 metros no existe — la resolución del
sensor es 1/16 de grado.

Era un agujero del programa: `rumboActual()` no chequeaba si el giroscopio se había callado.
La librería Adafruit devuelve `0.0` en los tres ángulos cuando no puede leer el chip, así que
si se caía durante un tramo la cuenta daba

    0.0 - 0.0 = 0.0

y se anotaba como **"salió perfecto"**. El programa no podía distinguir *fue derechito* de
*se murió el sensor*.

**Arreglado:** `rumboActual()` devuelve −1 cuando el sensor no contesta y cuenta las lecturas
perdidas; un tramo con **una sola** lectura mala ya no se da por bueno (durante ese rato la
corrección manejó a ciegas); la corrección no corrige cuando no hay rumbo; los tramos que no
valen quedan **afuera del promedio** y en la tabla aparecen como `mudo`. Y si se cayó en todos
los de un lado, el programa dice **"NO SE PUEDE COMPARAR"** en vez de inventar una conclusión.

También se subió el formato de la EEPROM a `DVT2`, así que **el historial viejo se descartó
solo** — y había que descartarlo: esos ceros eran caídas disfrazadas de éxito.

⚠️ **Y por eso no se puede concluir nada de la corrida 2.** Dio `sin corregir 5,1` y
`corrigiendo 8,9`, que visto así parece que la corrección empeora las cosas y que el signo
está invertido. Pero si el giroscopio se estaba cayendo, la corrección estaba comiendo basura,
y eso **también** la empeora. Con esos datos las dos explicaciones son indistinguibles.

---

## 5. Los programas de DFRobot

El equipo encontró los ejemplos de DFRobot para el BNO055 y propuso probarlos. **La idea era
buena y sirvió exactamente para lo que tenía que servir.**

El motivo: DFRobot es un driver **completamente independiente** del de Adafruit — otro código,
otra gente, escrito de cero para el mismo chip. Si el sensor le contesta a uno y no al otro,
el problema es la librería.

Se instaló (no estaba) y se armó `pruebas/giroscopo-dfrobot` con los dos ejemplos combinados.
Cinco cambios al original, todos con motivo: 19200 baudios en vez de 115200 (es a lo que abre
`mirar.bat`), motores apagados, **no se cuelga** (el original hace
`while(bno.begin() != eStatusOK)` y no sale nunca), parpadeo rápido del LED, y teclas `n`/`u`
para cambiar de modo.

### El driver de DFRobot es mejor en dos cosas

```c
// DFRobot: lee CHIP_ID PRIMERO y se va si no coincide
if ((lastOperateStatus == eStatusOK) && (temp == CHIP_ID_DEFAULT)) {
  reset();
  do { temp = getReg(SYS_STATUS); delay(10); timeOut++; }
  while ((temp != 0) && (timeOut < 100));        // ← CON TOPE (1 segundo)
  if (timeOut == 100) lastOperateStatus = eStatusErrDeviceReadyTimeOut;
} else
  lastOperateStatus = eStatusErrDeviceNotDetect;
```

**1. Ningún lazo sin salida.** El de Adafruit es
`while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) { delay(10); }` — **sin tope**, justo después
de resetear el chip. Si no vuelve, el programa muere ahí. (Ese mismo agujero se arregló hoy en
`monitor-robot`, donde colgaba la herramienta justo cuando más se la necesita.)

**2. Dice POR QUÉ falló**, no sólo que falló: `device not detected`,
`device ready time out`, `device internal status error`. Tres causas que apuntan a lugares
distintos y que Adafruit devuelve todas como `false`.

⚠️ **Y una diferencia que no se ve:** las dos librerías configuran los ejes distinto —
Adafruit usa `REMAP_CONFIG_P2`, DFRobot usa `eMapConfig_P1`. El mismo giro físico puede hacer
subir el rumbo con una y bajarlo con la otra. **Si algún día cambiamos de librería en el
programa de juego, todos los signos hay que volver a probarlos.**

### El resultado

**~50 intentos, todos `NO SE DETECTA EL CHIP`** (30 en un minuto, 22 en otros 45 segundos).
Mismo veredicto que Adafruit.

Eso **cerró la puerta del software**. Ya no era "Adafruit no lo encuentra": el chip no
levantaba el teléfono en el bus.

---

## 6. 🎯 La medición que partió el problema al medio

Dos cosas, casi seguidas.

### `buscar-i2c`

```
placa: pin32=BAJO  -> ZIRCON "Mark1"  (el giroscopio deberia ir en Wire, 18/19)
cables:  SDA(18)=ALTO   SCL(19)=ALTO
Wire  (18/19): vacio     Wire1 (16/17): vacio     Wire2 (24/25): vacio
--- vuelta 7: aparecio 0 veces, falto 7 veces ---
```

Tres hipótesis cerradas: **la placa es Mark1** (el bus correcto era el que usábamos), **los
tres buses vacíos** (no estaba en otro lado), y **ninguna línea de datos pegada a masa** (no
hay cortocircuito ni cable pelado tocando GND).

⚠️ **Pero cuidado con leer de más ese ALTO.** El comentario del programa dice que prueba que
la placa del sensor tiene corriente, y eso sólo vale si las únicas resistencias de pull-up
están en la plaquita del sensor. **Si la Zircon tiene las suyas, los cables darían ALTO igual
con el sensor sin alimentar.** No sabemos cuál de las dos es, así que lo honesto es: *las
líneas de datos están sanas*, y nada más.

### Y después, el dato que lo definió

Con el bug del §4 arreglado, la siguiente corrida de `derecho-y-vuelta` dio:

```
 n   giro  cal    adel-sin  atras-sin  adel-con  atras-con
  1    SI    3      mudo      mudo      mudo      mudo
```

| | |
|---|---|
| Arrancó | **SÍ** |
| Calibración | **3/3** — o sea que estuvo sano varios segundos quieto |
| Los cuatro tramos, con los motores andando | **se cayó en todos** |

> ## QUIETO ANDA. MOVIÉNDOSE SE CAE.

**Y explica todo lo del día de una sola vez:**

- **los ceros de la tabla anterior** eran esto mismo, disfrazado de "salió perfecto";
- **el "arrancó pero iba un poco chueco" de las 5 corridas**: arrancaba porque quieto el
  sensor está bien, y se caía en el primer metro. Así que **los tramos "corrigiendo" nunca
  corrigieron nada** — iban tan a ciegas como el control. Por eso no se notaba diferencia;
- **el 0 de 30 con DFRobot**: fue justo después de siete corridas manejando;
- **y lo que quedó sin explicar el 21/09** — *"el LED que empieza a temblar después de
  moverse"*, que en esa bitácora figura como "ninguna de las dos hipótesis lo explica".

---

## 7. 🎯 EL HALLAZGO: el cable

**El equipo lo encontró meneando el cable con la mano: al moverlo, el sensor hacía contacto y
contestaba por consola.**

Todo encaja:

| Lo que vimos | Por qué |
|---|---|
| Quieto anda, moviéndose se cae | la vibración abre el contacto |
| Los 4 tramos `mudo` con calibración 3/3 | sano al arrancar, se corta al andar |
| 5 de 7 arranques | según cómo quedara apoyado el cable |
| 0 de 30 con DFRobot, y después vuelve solo | se abrió del todo y se volvió a acomodar |
| SDA y SCL en ALTO pero el chip sin contestar | las líneas sanas hasta el corte |
| El LED que temblaba después de moverse (21/09) | lo mismo, sin saberlo |
| Los 80° del 01/09 | lo mismo |

**Lo que NO era, descartado con datos a lo largo de cuatro clases:** la batería (8,23 V
medidos el 15/09), el calentamiento, el bus I2C equivocado, el código de inicialización
(idéntico al de 2025), y la librería (DFRobot, ~50 intentos).

### Cómo arreglarlo

1. **Cambiar el cable, no parcharlo.** Cuatro hilos: VIN, GND, SDA (18), SCL (19). Lo típico
   es un hilo cortado **dentro del aislante, cerca del conector**, donde el cable flexiona —
   que es justo lo que la vibración abre y cierra. Por fuera se ve perfecto, y por eso aguantó
   cuatro clases escondido.
2. **Sujetarlo** para que no flexione con la vibración. Si no, el cable nuevo termina igual.
3. **Volver a menearlo después de cambiarlo.** Si moviéndolo todavía aparece y desaparece, no
   quedó arreglado.

### Lo que desbloquea

Cinco cosas que estaban frenadas por el giroscopio: el **despeje en diagonal** (escrito el
08/09, nunca probado), la regla **"si está quieto y torcido, se endereza"**, el **centrado con
el arco** y la elección azul/amarillo, la **versión en modo IMU**, y el **heading-hold en el
despeje**.

---

## 8. Tres regalos de la última corrida

Se volvió a cargar el programa de DFRobot "por curiosidad" después del hallazgo. **Anduvo** —
y salieron tres cosas que no esperábamos.

```
[5] reset + begin: todo bien
    >> ARRANCO, en modo NDOF (con brujula)
modo NDOF   head 359.9  roll 1.3  pitch -0.8   giro dps x 0.0 y -0.1 z 0.1   calib sis 0 giro 3 acel 0 brujula 0
```

### 8.1 `head 359.9` — justo el borde

Acordarse del comentario del delantero:

> *"en ESTE robot el rumbo de reposo cae justo en el borde 359,9/0,0, y el método viejo lo
> daba por muerto en pleno juego"*

**Al arquero le pasa lo mismo.** Está parado en 359,9. Un paso menos y lee 0.0. Y en una de
las líneas capturadas salió `head 359.9  roll 0.0  pitch 0.0` — **dos de tres ángulos en
cero**. Si el head hubiera caído del otro lado del borde, los tres daban 0.0 y el programa de
juego habría declarado el sensor muerto estando perfecto. **No era un riesgo teórico: casi
pasó, en esa captura.**

### 8.2 La prueba de "tres ceros = está mudo" falló, y la vimos fallar

```
MUDO (puros ceros)   giro dps x -0.1 y 0.0 z 0.1   calib sis 0 giro 0
```

Dice MUDO... **pero los datos crudos de velocidad de giro estaban llegando** (−0,1 / 0,0 /
0,1). Si el chip no contestara, esos también serían cero. **El chip estaba vivo**: lo que
pasaba es que la fusión todavía no había arrancado y por eso los ángulos daban 0,0,0.

O sea que el test de los tres ceros **miente en las dos direcciones**: da "muerto" con el chip
sano (esto) y da "salió perfecto" con el chip muerto (§4). Por eso hay que pasarlo a
`SYS_STATUS == 5`, como el delantero. Ya no es una mejora opcional.

### 8.3 La deriva del modo IMU, medida

```
giro dps x ±0.1  y ±0.1  z ±0.1     calib giro 3
```

Con el robot quieto y el giroscopio calibrado 3/3, la velocidad de giro oscila **±0,1 grado
por segundo alrededor de cero** — ruido, no un sesgo. Eso es lo que no podíamos medir y es
**buena noticia para el modo IMU**: la deriva va a ser chica.

**Y un detalle que refuerza lo mismo:** `calib sis 0 ... brujula 0`. En NDOF el sistema no se
calibra hasta que la **brújula** se calibra, y hasta entonces el rumbo no es confiable y puede
saltar — el candidato para los 80° del 01/09. Ahí está, a la vista, corriendo en NDOF con la
brújula en 0. El giroscopo (lo único que el arquero necesita) ya está en 3.

---

## 9. `cuando-se-cae` y `PROBAR.bat`

Se escribió para separar las dos causas que quedaban antes del hallazgo:

- **A) eléctrica** — el tirón de corriente hunde la tensión y el chip se reinicia;
- **B) mecánica** — la vibración abre un contacto flojo.

Las dos dicen "se cae cuando se mueve", así que la idea fue **medir por rampa**: subir la
potencia de a poco y anotar **en qué PWM exacto** se cae.

    umbral repetible entre corridas  ->  CORRIENTE (hay un consumo que no alcanza)
    umbral desparramado              ->  VIBRACION (no hay umbral)

Y con dos cargas: **2 ruedas** (como el despeje) y **3 ruedas girando en el lugar** (más
corriente, y no se traslada, así que es la más segura para la mesa). Si con 3 se cae antes que
con 2, otra vez apunta a la corriente.

También: **`PROBAR.bat`** (con `probar.py`), que larga varias vueltas seguidas sin recargar
nada, junta los umbrales y saca el veredicto solo. El criterio: si los umbrales caen dentro de
una ventana de **40 PWM** hay umbral; la rampa va de 60 a 220, y 40 es un cuarto del
recorrido — más que eso ya no se puede llamar "el mismo punto".

**Al final del día se pasó a modo VIGILANCIA** (`MOVER_LOS_MOTORES = false`): los motores no
se mueven, el programa sólo mira el giroscopio y cuenta caídas y vueltas. Con la causa ya
encontrada, mover los motores no prueba nada más — y esto sí sirve: **se menea el cable y se
mira el número moverse.** Con el cable arreglado, se pone en `true` y vuelve la rampa.

---

## 10. Volver a medir el ancho: dos cosas que salieron mal

Pedido del equipo, y con un motivo bueno: **los 4618/4396 ms del 21/09 se midieron con el
cable roto.** La medición usa el giroscopio para no torcerse, así que si se caía en el medio,
esos tiempos no son los de ir derecho.

### Lo que pasó al probar

**1. "Se queda quieto y no responde" no era un cuelgue ni el giroscopio.** El robot lo dijo
él mismo:

```
ahora: ERROR, no se guardo nada: arranco con un sensor de atras sobre una linea.
```

Si al prenderlo un sensor de atrás ya ve blanco, "llegar al costado" no significa nada, y el
programa prefiere avisar antes que medir mal. **Les ahorró guardar un número falso.**

⚠️ **Sobre la mesa esto salta siempre**: los sensores en la mesa dan 505 / 651 / 601 contra un
umbral de 425. Para el robot, la mesa entera es blanca. En la cancha significa que arrancó con
la cola sobre la línea del área — hay que apoyarlo un poco más adelante.

**2. El tambaleo yendo a la derecha.** Fui a buscar un bug que sospechaba —que después de una
caída el término acumulado del control diera un salto— y **no estaba**: el tope de `dt` ya
existe (`if (dt > 0.5) dt = 0.5`).

Queda la explicación simple: **la corrección prendiéndose y apagándose.** Cuando el
giroscopio se cae, `correccionDeRumbo()` devuelve 0 y el robot se va torciendo; cuando vuelve,
encuentra un error grande y corrige fuerte. Prende-apaga-prende = tambaleo. Y la asimetría
izquierda/derecha no dice nada con el cable roto: deslizarse para un lado lo flexiona distinto
que para el otro.

**Conclusión: no se puede medir el ancho hasta que el cable esté cambiado.** Medir ahora da
números que hay que tirar, exactamente como los del 21/09.

---

## 11. El arranque del giroscopio, rehecho

En `medir-ancho` se juntó todo lo aprendido entre el 01/09 y hoy:

| | De dónde salió |
|---|---|
| 10 reintentos de `begin()` | nuestro, la carrera de encendido del 01/09 |
| `setExtCrystalUse` **antes** del `delay` | del delantero — el cristal reinicia la fusión |
| Preguntarle `SYS_STATUS == 5` al chip | del delantero, y hoy confirmado dos veces |
| Esperar la calibración quieto (3/3) | hoja de datos de Bosch §3.11.2 |

Lo central: **`rumboActual()` ya no adivina.** Cuando ve tres ceros, le pregunta al chip. Si
está fusionando, los ceros son una postura real (el robot parado en el borde de 0°) y no una
caída.

### Y en el programa de partido: un solo intento

Pedido del equipo al cierre: que `funciona/seguir-y-despejar` inicialice **con un solo
intento**, como el delantero. Quedó como constante con nombre:

```c
const int INTENTOS_GIRO = 1;   // volver a 10 es cambiar este numero
```

⚠️ **La contra, anotada:** los 10 intentos se pusieron el 01/09 para tapar la carrera de
encendido, y ese día se confirmaron 3 corridas buenas de 3. Con uno solo, si el sensor no
despertó, el robot juega el partido entero sin rumbo.

**El argumento a favor, que hoy pesa más:** con el cable roto, los reintentos hacen que "a
veces arranque" en vez de fallar parejo — y "a veces" es justamente lo que costó cuatro clases
de diagnóstico. Con el cable arreglado conviene volver a 10: es gratis cuando el sensor está
sano, porque el segundo intento nunca llega a correr.

---

## 12. Errores del día (incluidos los míos)

**Borré la medición del ancho del 21/09.** El historial que agregué a `derecho-y-vuelta` lo
puse en la dirección 0 de la EEPROM... que es justo donde `medir-ancho` guarda los tiempos de
cruce. La primera corrida los pisó. Ya está movido y el reparto quedó documentado en los dos
programas:

```
    0 ..  255   medir-ancho / medir-ancho-sin-giro
  256 ..  767   derecho-y-vuelta
  768 .. 4095   libre
```

Los números no se perdieron — están en la bitácora del 21/09 — pero **la EEPROM es una sola y
la comparten todos los programas.** Quien agregue otro que guarde algo, que respete el mapa.

**Afirmé que el robot se re-referenciaba en la línea blanca, y era falso.** Al escribir la
versión IMU (22/09) justifiqué el cambio diciendo que después de cada despeje el robot corrige
el rumbo contra la línea. Eso **no existía en el código**: `rumboBase` se asignaba en un solo
lugar y la línea blanca sólo frenaba el retroceso. Lo encontró una auditoría, no una prueba.
Sin el arreglo, la versión IMU habría sido **peor** que la NDOF.

**`PROBAR.bat` mostró la salida de `medir-ancho`** y por un momento pareció un bug. No lo es:
el `.bat` **no elige qué corre el robot** — sólo le manda la tecla `g` y lee lo que contesta.
El robot tenía otro programa cargado. Quedó anotado arriba de todo en el archivo, porque es el
error más fácil de cometer con esa herramienta.

**Un aviso quedó desactualizado.** El encabezado de `probar.py` decía "LAS RUEDAS GIRAN"
después de pasar a modo vigilancia, donde no giran. Corregido. Un aviso de seguridad que
miente es peor que ninguno.

---

## 13. Números de hoy

| Qué | Valor |
|---|---|
| Arranques buenos del giroscopio (7 pruebas del equipo) | **5 de 7** |
| DFRobot: intentos, todos "device not detected" | **~50** |
| `buscar-i2c`: apariciones en 7 escaneos | **0** |
| Estado de SDA(18) / SCL(19) | **ALTO / ALTO** (líneas sanas) |
| Placa | Zircon **Mark1** (pin 32 en bajo) |
| Tramos válidos en la corrida decisiva | **0 de 4** (los cuatro `mudo`) |
| Calibración del giroscopo, quieto | **3/3** |
| Rumbo de reposo del arquero | **359,9°** ⚠️ al borde |
| Velocidad de giro en reposo (deriva) | **±0,1 °/s** |
| Calibración de la brújula en NDOF | **0/3** |
| Sensores de línea sobre la mesa | 505 / 651 / 601 (umbral 425) |
| Velocidad y tramo de `derecho-y-vuelta` | 80 de potencia, 5000 ms, ~2 m |
| Piso de arranque de los motores | ~70 PWM |
| Referencia del delantero (patada) | 10,1° → **4,2°** con heading-hold |

---

## 14. Qué queda pendiente

### 🔴 Lo primero, y ahora sí es una sola cosa

- ⬜ **CAMBIAR EL CABLE DEL GIROSCOPIO.** Todo lo demás espera esto. Con `PROBAR.bat` en modo
  vigilancia se comprueba en el momento: se menea y los contadores no se tienen que mover.
- ⬜ **Sujetar el cable nuevo** para que no flexione.

### Cuando el cable esté

- ⬜ **Volver a medir el ancho** (`medir-ancho`), apoyándolo despegado de la línea.
- ⬜ **Correr `derecho-y-vuelta`** para saber si el truco del delantero sirve en nuestro robot.
  Hoy no se pudo: los cuatro tramos salieron `mudo`.
- ⬜ **Correr `cuando-se-cae` con `MOVER_LOS_MOTORES = true`**, para descartar que además haya
  un problema de corriente escondido detrás del cable.
- ⬜ **Probar la versión IMU** (`funciona/seguir-y-despejar-imu`, compila desde el 22/09 y
  nunca se cargó).
- ⬜ **Volver `INTENTOS_GIRO` a 10** en el programa de partido.

### Mejoras ya identificadas, sin implementar

- ⬜ **`SYS_STATUS == 5` en vez de contar ceros**, en el programa de partido. Hoy vimos las dos
  fallas del método viejo, y el robot se para en 359,9 — a un paso del borde donde miente.
- ⬜ **Mover `setExtCrystalUse` antes del `delay`** en el programa de partido: un segundo de
  arranque regalado, y la cuenta está en 2 s.
- ⬜ **El heading-hold en el despeje.** Nuestro despeje sale a 200 con la trasera suelta, que
  es exactamente el caso que el delantero midió en 10,1°.
- ⬜ **Que el programa de partido vuelva a buscar el giroscopio si se cae** jugando.
  `cuadrado-giroscopo` ya tiene el mecanismo escrito y probado.
- ⬜ La patrulla sin pelota (21/09), con los tiempos ya medidos — pero hay que re-medirlos.
- ⬜ La regla "si está quieto y torcido, se endereza" (15/09).
- ⬜ Un LED que se vea (pines 9 o 10 libres).
- ⬜ Anotar los partidos 2 y 3 del 21/09, que no quedaron en ninguna bitácora.

---

## 15. Lo que se aprendió del método

**1. El software no era el problema, pero el software encontró el problema.** La distinción
AUSENTE / MUDO / SANO, la detección de mudo, la rampa: eso convirtió "anda a veces" en
**"quieto anda, moviéndose se cae"**, y de ahí al cable fue directo. Cuatro clases de
herramientas de diagnóstico se pagaron enteras hoy.

**2. Un instrumento que no puede dar el resultado contrario no mide nada.** La versión vieja
de `derecho-y-vuelta` anotaba 0,0 grados cuando el sensor se moría — el mejor resultado
posible en el peor caso posible. Dos corridas de datos limpios que en realidad eran basura.
**Antes de creerle a una medición, hay que preguntarse cómo se vería una falla.**

**3. Probar la hipótesis independiente aunque parezca perder tiempo.** Cargar DFRobot no
arregló nada, pero **cerró la puerta del software con autoridad**. Sin eso, siempre iba a
quedar la duda de "¿y si era la librería?". Descartar también es avanzar.

**4. Los números que se descartan hay que anotarlos igual.** Las corridas donde el giroscopio
no arranca son la mitad del dato: sin ellas no existe el "5 de 7".

**5. Leer el código de la otra mesa paga.** Tres cosas salieron de ahí hoy: el truco de que la
corrección sólo frene, el orden de `setExtCrystalUse`, y preguntarle `SYS_STATUS` al chip. Y
lo mejor: **llegaron al mismo pozo por otro camino** — ellos lo sufrieron en cancha con NDOF
por casualidad de hacia dónde apuntaba el robot, nosotros lo predijimos razonando sobre el
modo IMU. Dos caminos distintos, el mismo borde de 359,9/0,0.

**6. Verificar antes de acusar.** Hoy estuve a punto de reportar un bug en el término
acumulado del control por una caída del sensor. Fui a mirar y el tope ya estaba puesto. Un bug
inventado manda a arreglar lo que no está roto.
