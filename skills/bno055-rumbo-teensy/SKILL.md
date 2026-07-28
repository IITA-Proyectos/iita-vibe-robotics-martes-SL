---
name: bno055-rumbo-teensy
description: Giroscopio BNO055 en el Teensy 4.1 de los robots de fútbol 2025 de IITA (placa Zircon, arquero.ino / delantero.ino) — rumbo, cero de referencia (initialYaw), normalización del error a ±180 y todo lo que falla con eso. Usar SIEMPRE que aparezca "giroscopio", "giroscopo", "gyro", "BNO", "BNO055", "IMU", "brújula", "rumbo", "heading", "yaw", "currentYaw", "initialYaw", "error", "orientation.x", "getEvent", "sensors_event_t", "setExtCrystalUse", "getSystemStatus", "sysStat", "getCalibration", "sys gyro accel mag", "OPERATION_MODE", "NDOF", "IMUPLUS", "kp", "correccion", "I2C", "0x28", "pines 18 y 19", "while(1)", "no sé para dónde mira el robot", "el robot arranca torcido", "hoy anda y mañana no", "solo anda si lo prendo apuntando al norte", "el robot no hace nada al encender", "se quedó colgado en el setup", "el barrido del arquero sale torcido", "no patea nunca", "patea para cualquier lado", "calibrar el giroscopio", "corrige para el lado largo", "se pasa de 360 y se vuelve loco". NO usar para tunear el reparto de PWM entre las 3 ruedas ni la geometría omni (eso va en la skill de motores/omni); NO usar para la cámara OpenMV ni los sensores de línea (esas son otras skills); NO usar para el robot 2026 de Incheon (otra electrónica, otros números, otro repo).
---

# El rumbo: cómo el robot sabe para dónde está mirando

Este documento es sobre **un solo número**: el ángulo al que apunta el robot. En el código se
llama `currentYaw`, y de él sale `error`, que es lo que decide si el arquero barre derecho o
torcido y si el delantero patea o sigue orbitando.

Todo lo que sigue está verificado contra el código real del repo. Cada número lleva su
`archivo:línea`. Si algo no lo pudimos verificar, lo dice explícitamente.

> ⚠️ **Nada de esto está probado en el robot.** Es lectura de código y propuestas. Que compile no
> prueba nada. El veredicto lo da el que tiene el robot en la mano.

---

## 0. Antes de todo: hoy los programas NO compilan

No arranquen ninguna de las mediciones de §7 y §8 hasta arreglar esto, porque **no van a poder
subir el programa**. Está documentado en `bugs-conocidos.md` (sección "Lo primero") y el parche
está en `correcciones-propuestas.md` (PARCHE 0):

1. **Una llave `}` de más** al final de `zirconLib.cpp` (línea 355). La función anterior ya cerró
   en la 351. El compilador corta ahí.
2. **La variable `bno` está definida dos veces:** en la librería (`zirconLib.cpp:4`) y en cada
   programa (`arquero.ino:68`, `delantero.ino:76`). Las dos son globales públicas → el enlazador
   dice `multiple definition of 'bno'`. **Este error queda tapado por el primero**, así que
   aparece recién cuando borran la llave. No es que rompieron algo.

Ojo con dónde parchean: `#include <zirconLib.h>` con `< >` significa *"buscalo en las librerías
instaladas"*, no *"al lado del programa"*. El archivo que manda es el de
`Documentos/Arduino/libraries/`, no el del repo. Si la copia instalada es distinta de la del repo
(muy probable: **con la del repo no se podía compilar, y sin embargo ganaron el Nacional**),
guárdenla antes de pisarla.

Todo lo que sigue asume que ya compilan.

---

## 1. Qué mide realmente un BNO055 (y por qué "rumbo" ≠ "giroscopio")

El chip que está en la Zircon **no es un giroscopio**. Es un módulo que adentro tiene **tres**
sensores y un microcontrolador propio que los mezcla:

| Sensor interno | Qué mide | Cómo se porta |
|---|---|---|
| **Giroscopio** | velocidad de giro (grados por segundo) | reacciona rapidísimo, pero para saber el ÁNGULO hay que sumar esa velocidad en el tiempo → el error se **acumula** (se llama *drift*, deriva) |
| **Acelerómetro** | hacia dónde tira la gravedad | dice qué es "abajo" → corrige inclinación, no sirve para el rumbo horizontal |
| **Magnetómetro** | el campo magnético del lugar | es una **brújula**: dice dónde está el norte. No deriva, pero se ensucia con cualquier metal o corriente cerca |

El microcontrolador interno del BNO corre un programa de fusión que junta los tres y te entrega
un ángulo listo. Vos leés un solo número y parece magia.

**La consecuencia práctica, que es toda la razón de ser de esta skill:**

> Si la fusión usa el magnetómetro, el ángulo que leés es **absoluto**: está medido contra el
> norte magnético del lugar donde estás parado. Y a 5 cm del chip hay **tres motores de corriente
> continua** que, cuando arrancan, generan su propio campo magnético. Más el metal de la
> estructura, más la batería, más los cables de motor con varios amperes.

Traducción: **la brújula miente al lado de los motores.** Ese es el mecanismo detrás de la mitad
de las cosas raras que hace el robot.

El giroscopio solo (sin brújula) no tiene ese problema, pero deriva: si lo dejás quieto media
hora, el ángulo se va corriendo solito. Es un trade-off, no hay opción gratis.

### En qué modo está hoy nuestro robot: NDOF (usa la brújula)

El código llama `bno.begin()` **sin argumentos** (`arquero.ino:246`, `delantero.ino:270`). El modo
lo elige el valor por defecto del parámetro, y ese valor es **NDOF**:

```cpp
bool begin(adafruit_bno055_opmode_t mode = OPERATION_MODE_NDOF);
```

> ✅ **Verificado, con una salvedad honesta.** Esa línea está en `Adafruit_BNO055.h:285` de la
> librería **Adafruit BNO055 v1.6.4** (`library.properties`), en la copia que hay en este disco:
> `C:/Users/violl/futbol2026/futbol-roboliga2026-iita-salta/firmware/lib/Adafruit_BNO055/`.
> Es la misma librería de Adafruit que usa este proyecto, **pero es la copia de otro repo, no la
> que tiene instalada el IDE de ustedes.** En `Documentos/Arduino/libraries/` de esta máquina no
> se pudo leer el archivo. Chequeo de 1 minuto que conviene hacer una vez: abrir el
> `Adafruit_BNO055.h` de SU instalación, buscar `bool begin(` y confirmar que dice `= OPERATION_MODE_NDOF`.
> Si su versión difiere, avisen: media sección cambia de tono.

NDOF **sí usa el magnetómetro**. O sea: el `currentYaw` de este robot es un **rumbo magnético
absoluto**, con tres motores DC al lado. Eso explica por qué el equipo 2025 reportó *"solo anda si
se enciende apuntando al norte"* — no como comprobación de campo, sino como hipótesis coherente
con el modo en el que está el chip.

---

## 2. Cómo se inicializa y se lee, en ESTE robot

### El objeto y las variables

```cpp
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);   // arquero.ino:68  ·  delantero.ino:76
float correccion;                                   // arquero.ino:69
float error = 0;                                    // arquero.ino:70
float initialYaw = 0;                               // arquero.ino:71
float currentYaw;                                   // arquero.ino:72
float kp = 0.3;                                     // arquero.ino:73
```

- `55` es un ID de sensor. La librería lo copia tal cual en `event.sensor_id` y nada más. Acá da
  igual qué número sea.
- `0x28` es la **dirección I2C**. El BNO puede vivir en `0x28` o `0x29`; acá es `0x28`.
- El constructor tiene un tercer parámetro con valor por defecto `&Wire` (`Adafruit_BNO055.h:282-283`),
  y el código no lo pasa → usa el bus `Wire`, que en el Teensy 4.1 son los **pines 18 (SDA) y 19
  (SCL)** (`mapa-pines-teensy.md:34-35`). Si esos dos cables están flojos, el chip no contesta.

### El arranque (setup)

```cpp
//----------GIROSCOPO-------            // arquero.ino:245-256
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1);                          // arquero.ino:248  ← acá se cuelga
  }
  bno.setExtCrystalUse(true);           // arquero.ino:250

 sensors_event_t event;                 // arquero.ino:252
  bno.getEvent(&event);
  initialYaw   = event.orientation.x;   // arquero.ino:254   // 0..360
  millis_inicio_estado = millis();
```

El delantero tiene **exactamente lo mismo** en `delantero.ino:269-280`.

Tres cosas a fijar de acá:

1. `bno.begin()` devuelve `false` si el chip no contesta en el I2C → `while(1)` (línea 248).
   Adentro, `begin()` reintenta detectar el chip hasta **850 ms** antes de rendirse
   (`Adafruit_BNO055.cpp`, cuerpo de `begin()`). Pero ojo: que devuelva `true` significa
   *"el chip contesta y quedó en modo NDOF"*, **no** *"la fusión ya está entregando ángulos
   válidos"*. Eso es todo el bug (b) de §5.
2. `setExtCrystalUse(true)` le dice al BNO que use un **cristal de 32.768 kHz externo** en vez de
   su reloj interno. Da mejor estabilidad… **si el cristal existe físicamente en la placa**.
   > ❓ **No verificado.** No tenemos el esquemático de la Zircon en este repo: `Zircon.pdf`
   > quedó en el repo del equipo de Incheon (`mapa-pines-teensy.md:117`). Si el BNO está montado
   > como módulo Adafruit, el cristal viene puesto y esto está bien. Si es un chip pelado sin
   > cristal, pedirle el cristal externo lo deja **sin reloj** — falla silenciosa. Es un chequeo
   > de esquemático de 2 minutos que conviene hacer una vez y anotar.
3. `initialYaw` se lee **inmediatamente** después de `begin()`. Volvemos a esto en §5b: es el bug
   más caro de los tres.

### La lectura, una vez por vuelta del loop

```cpp
  // GIROSCOPO                          // arquero.ino:333-340
  sensors_event_t event;
  bno.getEvent(&event);
  currentYaw   = event.orientation.x;   // arquero.ino:336
  error =  currentYaw - initialYaw;     // arquero.ino:337
  if ( error > 180 ) error = error - 360;   // arquero.ino:338
  if (error < -180) error = error + 360;    // arquero.ino:339
  correccion= error * kp;               // arquero.ino:340
```

Idéntico en `delantero.ino:358-365`.

`getEvent(&event)` sin segundo argumento lee el vector **Euler** y lo divide por 16
(`Adafruit_BNO055.cpp`, `getEvent` → `getVector(VECTOR_EULER)`; el chip entrega 1 grado = 16 LSB).
Por eso `orientation.x` sale directamente en grados 0..360.

**`correccion` se calcula y nunca se usa.** Buscamos `correccion` en todo el archivo: aparece
exactamente dos veces, en la declaración (`arquero.ino:69`) y acá (`arquero.ino:340`). Nunca se
escribe a un motor. **No hay un control proporcional de rumbo cerrado en este robot.** El `kp =
0.3` no hace absolutamente nada hoy. Si alguien toca `kp` esperando ver un cambio, no va a ver
ninguno — y no es que se rompió.

### Dónde SÍ se usa el rumbo

En el **arquero**, `error` entra en `aiproporcional()` y `adproporcional()`
(`arquero.ino:187-233`), que son las funciones de barrido lateral. Ahí no hay un PID: hay **tres
bandas**:

```cpp
  if (error>-1 && error <1){ ... }      // arquero.ino:188  ← "estoy derecho"
  else if (error > 0){ ... }            // arquero.ino:193  ← "me fui para un lado"
  else if (error < 0){ ... }            // arquero.ino:201  ← "me fui para el otro"
```

Cada rama reparte distinto el PWM entre las tres ruedas. Es **bang-bang de 3 escalones**, no
proporcional (el nombre de la función engaña). Se ejecutan al entrar a los estados
`moverce_derecha` (llamada en `arquero.ino:1031`), `moverce_izquierda` (`arquero.ino:1079`),
`impulso_derecha` (`arquero.ino:1128`) e `impulso_izquierda` (`arquero.ino:1140`).

En el **delantero**, `error` **no termina moviendo ruedas**, y esto merece la explicación completa
porque a primera vista parece que sí:

- `delantero.ino` **también** tiene `aiproporcional()`/`adproporcional()` usando `error`
  (`delantero.ino:211-257`) — los dos archivos salieron del mismo programa, aunque después
  divergieron. Con una diferencia: ahí la banda del medio
  es `error>-2 && error<2` (`delantero.ino:212` y `:236`), no `±1` como en el arquero.
- Pero esas funciones se llaman **solamente** desde `moverce_derecha`/`moverce_izquierda`/
  `impulso_*` (`delantero.ino:1047, 1095, 1145, 1157`), que son **estados del arquero**.
- Y el delantero arranca en `Estado estado = AVANCE_INICIO;` (`delantero.ino:138`). Ningún estado
  del delantero asigna un estado del arquero: las únicas asignaciones a `moverce_*` /
  `impulso_derecha` / `impulso_izquierda` están en las líneas 1040, 1067, 1072, 1089, 1115, 1120,
  1137, 1149, 1161 y 1207 — **todas dentro del bloque del arquero** (que empieza en
  `delantero.ino:1030`).

**Conclusión: en el delantero ese bloque es código muerto.** Lo que `error` sí decide ahí es
**cuándo patear**:
`abs(error) <= 1` dispara la patada como salida alternativa a la de cámara (`delantero.ino:628` y
`:699`); `abs(error) <= 50` habilita avanzar tras 9 s girando (`delantero.ino:448`);
`abs(error) <= 80` decide, al pisar línea blanca, si patea o sigue orbitando
(`delantero.ino:659` y `:730`).

> Resumen: en el arquero un rumbo malo se ve como **barrido torcido**; en el delantero se ve como
> **no patea nunca** o **patea para cualquier lado**. Mismo bug, síntomas totalmente distintos.

### Lo que hay en la librería y NO se usa

`zirconLib.cpp:4` declara **otro** objeto `Adafruit_BNO055 bno;` (esto es la mitad del problema de
enlazado de §0). Tiene una función `readCompass()` (`zirconLib.cpp:94-104`) que **ningún programa
llama nunca** (grep en los dos `.ino`: cero resultados; solo está declarada en `zirconLib.h:11`).
Y aunque la llamaran, devolvería `0`: `compassCalibrated` arranca en `false` (`zirconLib.cpp:7`) y
la única función que lo pondría en `true` es `CalibrateCompass()`, que está **comentada entera**
(`zirconLib.cpp:62-92`) y cuya llamada también está comentada (`zirconLib.cpp:46`).

Vale la pena mirar por qué la comentaron: adentro tiene

```cpp
    while (mag < 3) {                   // zirconLib.cpp:74
      bno.getCalibration(&system, &gyro, &accel, &mag);
      ...
```

Un `while` que espera a que el **magnetómetro** llegue a calibración 3. Al lado de tres motores,
eso puede no pasar nunca → arranque colgado para siempre. Alguien se comió ese problema y lo
resolvió comentando la función. Es la solución equivocada al problema correcto (§6).

---

## 3. El concepto central: rumbo ABSOLUTO vs rumbo RELATIVO al arranque

Esto es lo más importante de todo el documento.

`currentYaw` viene del chip en **0 a 360** (así lo dice el propio comentario del código,
`arquero.ino:254`, y lo confirma la conversión Euler de la librería). Ese número está medido
contra **algo del mundo** — el norte magnético, porque el modo es NDOF (§1). Al robot **no le
importa** el norte. Al robot le importa: *"¿estoy derecho respecto de cómo me pusieron en la
cancha?"*.

Por eso el código hace la resta:

```cpp
error = currentYaw - initialYaw;   // arquero.ino:337
```

`initialYaw` es **la foto del rumbo en el instante del arranque**. Es el cero de referencia. Y
`error` significa, literalmente: **"cuántos grados me desvié de como me apoyaron cuando me
prendieron"**.

```
        currentYaw = 217°   (absoluto, contra el norte)
        initialYaw = 200°   (absoluto, foto del arranque)
        ------------------
        error      =  17°   (relativo: me fui 17° desde que arranqué)
```

Toda la máquina de estados razona con `error`, no con `currentYaw`. **Eso está bien pensado.**
El robot no necesita saber dónde está el norte; necesita saber cuánto giró.

**Y acá está el punto de falla:** ese diseño entero descansa sobre `initialYaw`. Si `initialYaw`
está mal, `error` deja de significar "cuánto me desvié" y pasa a significar "para dónde está el
norte". No hay ningún otro lugar del programa donde eso se pueda detectar o corregir. Un solo
número, leído una sola vez, en un solo instante, del que depende todo el partido.

---

## 4. El wrap-around: por qué la resta sola no alcanza

Los ángulos son un círculo, no una recta. Después de 359° viene 0°, no 360°. La resta pelada no
sabe eso.

Ejemplo concreto. Prendieron el robot apuntando a `initialYaw = 350°`. El robot gira **10 grados**
y ahora `currentYaw = 0°`:

```
error = 0 - 350 = -350
```

−350 grados. El robot se movió 10 grados y la cuenta dice que se movió 350 en el otro sentido.
Si eso alimentara una corrección, el robot intentaría **corregir dando casi una vuelta entera
para el lado largo**, en vez de 10 grados para el lado corto. En el delantero, `abs(error) <= 1`
(`delantero.ino:628`) nunca se cumpliría: nunca patearía por esa vía.

**El código lo arregla bien.** Estas dos líneas son correctas y no hay que tocarlas:

```cpp
  if ( error > 180 ) error = error - 360;   // arquero.ino:338
  if (error < -180) error = error + 360;    // arquero.ino:339
```

Qué hacen, en castellano: *"si el desvío calculado da más de media vuelta, es porque estás
midiendo por el lado largo; medí por el otro lado."* Con el ejemplo: −350 < −180 → −350 + 360 =
**+10**. Diez grados para el lado correcto. Perfecto.

Después de estas dos líneas, `error` está garantizado en el rango **−180 a +180**. Es la única
propiedad que la máquina de estados puede asumir sobre él, y es de la que dependen todos los
`abs(error) <= N`.

### Cómo escribirlo como función (para no repetirlo)

```cpp
// Devuelve la diferencia angular más corta entre dos rumbos, en el rango -180..+180.
// a y b vienen en 0..360.
float diferenciaAngular(float a, float b) {
  float d = a - b;
  while (d >  180.0) d -= 360.0;
  while (d < -180.0) d += 360.0;
  return d;
}
// Uso:  error = diferenciaAngular(currentYaw, initialYaw);
```

El `while` en vez del `if` es más robusto: aguanta entradas fuera de 0..360 (por ejemplo si
alguien suma un offset). Con entradas 0..360 el `if` alcanza, por eso el código actual está bien.

> ⚠️ Si algún día alguien "simplifica" quitando esas dos líneas, o suma un ángulo a `error`
> **después** de normalizar sin volver a normalizar, el bug del lado largo vuelve. Y es un bug
> que **casi nunca aparece en la mesa** (solo cuando la referencia queda cerca de 0/360), así que
> pasa el test y falla en el torneo.

---

## 5. Los tres modos de falla que ya están documentados

Estos tres están en `bugs-conocidos.md`, verificados contra el código. Acá van explicados de
**síntoma hacia causa**, que es como los van a encontrar en la mesa.

### (a) "El robot no hace NADA al encender"

**Síntoma:** enchufan la batería y no pasa nada. Ni un motor. Ni un ruido. El LED de la placa
puede estar prendido. Parece batería descargada.

**Causa:**

```cpp
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1);                          // arquero.ino:248
  }
```

`while(1);` es un bucle vacío infinito. El programa entra ahí y **no sale nunca**. `setup()`
nunca termina, `loop()` nunca empieza, los motores nunca se inicializan. El único aviso sale por
el cable USB — que en cancha nadie tiene enchufado.

**Cuándo se dispara:** cuando `bno.begin()` devuelve `false`, o sea cuando el chip no contesta en
el I2C ni siquiera después de los 850 ms de reintentos que hace la librería adentro. Cable de SDA
(pin 18) o SCL (pin 19) flojo, conector mal puesto, alimentación del módulo, soldadura fría. Es un
fallo **físico e intermitente**, del tipo que aparece justo después de que el robot recibió un
golpe.

**Cómo lo reconocen sin USB:** no lo reconocen. Ese es el problema. Un robot que no arranca en
cancha les va a hacer sospechar de la batería, del fusible, del cargador — y el culpable número
uno es el giroscopio.

**Qué se puede hacer (propuesto, falta validar en banco):** hay dos posturas defendibles y la
decisión es del equipo, no de la IA:

- **"Sin giroscopio no juego"** — dejar el `while(1)`, pero hacerlo **visible**: parpadear el LED
  de la placa en un patrón reconocible.
- **"Sin giroscopio juego peor"** — seguir sin giroscopio: `error` queda en 0, el arquero cae
  siempre en la rama del medio de `ai/adproporcional` (`error>-1 && error<1` es verdadero con
  `error` en 0) y barre sin corrección. Para un arquero eso es jugable. Para el delantero es peor:
  con `error` clavado en 0, `abs(error)<=1` es **siempre** verdadero, así que la salida de
  `delantero.ino:628` / `:699` dispara la patada apenas se cumplan los 4 s orbitando, **sin
  importar para dónde esté mirando**. Ojo con eso.

Versión "visible" del while(1), que no cambia ninguna decisión de diseño:

```cpp
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1) {                       // sigue sin jugar, pero AVISA
      digitalWrite(LED_BUILTIN, HIGH); delay(120);
      digitalWrite(LED_BUILTIN, LOW);  delay(120);
    }
  }
```

No hace falta agregar `pinMode(LED_BUILTIN, OUTPUT)`: ya está en `arquero.ino:244`, dos líneas
antes del bloque del giroscopio. `LED_BUILTIN` en el Teensy 4.1 es el **pin 13**, el LED que está
soldado en la propia placa Teensy — `mapa-pines-teensy.md:98` aclara que el pin 13 no está
conectado a nada en la Zircon, así que lo que van a ver es la lucecita del Teensy, no un LED
grande. Es visible pero hay que saber dónde mirar. El mismo LED se usa en `arquero.ino:261` para
mostrar `haypelota`, pero no hay conflicto: si entran a este `while`, nunca llegan al `loop()`.

Parpadeo rápido = "no encuentro el giroscopio". Anótenlo en el equipo o no sirve de nada.

### (b) "Hoy anda y mañana no" — el cero leído demasiado pronto

**Síntoma:** el robot se comporta distinto en cada encendido. Hoy el arquero barre derecho,
mañana barre torcido. Cambia según **para dónde lo apoyaron** cuando lo prendieron. Nadie tocó el
código. En el delantero: a veces patea, a veces orbita 25 segundos sin patear nunca.

**Causa:**

```cpp
 sensors_event_t event;
  bno.getEvent(&event);
  initialYaw   = event.orientation.x;   // arquero.ino:254
```

`initialYaw` se lee **0 ms después** de que `bno.begin()` devolvió `true`. Pero el BNO055 tiene un
microcontrolador adentro que tiene que arrancar, correr sus self-tests y poner en marcha el
programa de fusión. Hasta que eso pasa, **devuelve 0.00**. `begin()` devolviendo `true` solo
prueba que el chip contesta por I2C y que quedó configurado en NDOF, no que la fusión ya esté
corriendo.

Si `initialYaw` queda en **0**, entonces:

```
error = currentYaw - 0 = currentYaw
```

Y ahí `error` dejó de significar "cuánto me desvié desde que arranqué" y pasó a significar
**"para dónde está el norte magnético"**. El arquero elige su reparto de PWM según hacia dónde
está el norte de la cancha. Cambian el robot de sala, cambia el comportamiento. Es la definición
del síntoma *"hoy anda y mañana no"*.

Está documentado como **A5** (arquero) y **DEL-03** (delantero) en `bugs-conocidos.md`.

**Antes de parchear: midan.** Agreguen esta línea justo después de `arquero.ino:254` (o
`delantero.ino:278`):

```cpp
  Serial.print("initialYaw = "); Serial.println(initialYaw);
```

Prendan el robot **5 veces**, cada vez apuntando a un lado distinto de la mesa, y anoten los 5
números. **Si los 5 dan 0.00 aunque el robot mire para lados distintos, el bug está confirmado.**
Si dan valores distintos y coherentes con la orientación real, `initialYaw` estaba bien y el
parche queda de red de seguridad. **Sin este dato, cualquier parche es a ciegas.**

**El arreglo propuesto** (falta validar en banco) no es "poner un `delay(1000)` y rezar": es
**preguntarle al chip si ya arrancó**. El BNO tiene un registro de estado del sistema, y el valor
**5 significa "la fusión está corriendo"** (los 7 valores posibles están comentados en
`Adafruit_BNO055.cpp`, dentro de `getSystemStatus`).

> 🛑 **Esto REEMPLAZA las líneas 245-256 de `arquero.ino` (o 269-280 de `delantero.ino`), no se
> agrega arriba.** Si lo pegan encima sin borrar el bloque viejo, quedan dos
> `sensors_event_t event;` en el mismo `setup()` y **no compila**. Este es el mismo parche que
> está en `correcciones-propuestas.md` (DEL-03).

```cpp
//----------GIROSCOPO-------
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

  // El programa de fusión del BNO TARDA en arrancar. Si le preguntamos enseguida
  // contesta 0.00, y ese 0.00 queda como "para donde miraba el robot al encender"
  // durante TODO el partido. En vez de esperar un tiempo inventado, le preguntamos
  // al chip: getSystemStatus devuelve 5 = "fusion corriendo".
  // El tope de 3 s evita quedar colgado en cancha.
  uint8_t sys_stat = 0, self_test = 0, sys_err = 0;
  unsigned long t_espera = millis();
  do {
    bno.getSystemStatus(&sys_stat, &self_test, &sys_err);
  } while ((sys_stat != 5) && (millis() - t_espera < 3000));

  // Descartamos las primeras lecturas y nos quedamos con la última.
  sensors_event_t event;
  for (int k = 0; k < 10; k++) { bno.getEvent(&event); delay(20); }
  initialYaw = event.orientation.x;   // 0..360

  Serial.print("sysStat="); Serial.print(sys_stat);
  Serial.print(" initialYaw="); Serial.println(initialYaw);

  millis_inicio_estado = millis();
```

> 📌 **Por qué el `do/while` no lleva `delay` adentro:** `getSystemStatus()` termina con un
> `delay(200)` **propio**, dentro de la librería (`Adafruit_BNO055.cpp`, última línea de la
> función). O sea que cada vuelta ya cuesta ~200 ms sola, y en 3000 ms entran unas 14 consultas.
> Agregarle un `delay(20)` encima no aporta nada. **Corolario importante: nunca llamen a
> `getSystemStatus()` dentro del `loop()`** — les mete 200 ms de freno en cada vuelta y el robot
> reacciona tardísimo. `getCalibration()`, en cambio, es una sola lectura I2C sin ningún `delay`:
> esa sí se puede llamar seguido.

> ⚠️ **Este parche CAMBIA EL SIGNIFICADO de `error`.** Si hoy `initialYaw` vale 0 y después del
> parche vale 137, entonces `abs(error)<=1`, `<=50` y `<=80` empiezan a medir otra cosa y el
> robot **se va a comportar distinto, quizá peor al principio**. No es una regresión: es que
> recién ahora esos umbrales miden lo que decían medir. **Nunca subir este parche el día del
> torneo.** Aplicarlo, medir, y después re-probar los cinco umbrales en cancha.

### (c) Comparar el rumbo CRUDO contra valores absolutos

**Síntoma reportado por el equipo 2025:** *"el robot solo anda si se enciende apuntando al
norte"*.

**Causa:** hay comparaciones que usan `currentYaw` (crudo, 0..360, contra el norte) en vez de
`error` (normalizado, contra el arranque):

```cpp
if ( (millis()- millis_inicio_estado >= 5000) && ((currentYaw <= 10) or (currentYaw >= 350)))
                                                        // arquero.ino:606 y 712
if ((currentYaw <= 90) or (currentYaw >= 270))          // arquero.ino:642, 656, 671, 748, 762, 776
```

`currentYaw <= 10 or currentYaw >= 350` pregunta literalmente **"¿estoy apuntando al norte?"**,
cuando lo que se quería preguntar era **"¿estoy apuntando a donde arranqué?"**. Con la referencia
correcta eso se escribe `abs(error) <= 10`.

Hay un tercer caso, más sutil, en `arquero.ino:422`:

```cpp
if ((millis() - millis_inicio_estado >= 8000) && ((error <= 0) or (error >= 350)))
```

Usa `error` (bien) pero lo compara contra umbrales de la escala 0..360 (mal). Como `error` está
normalizado a ±180 (§4), **`error >= 350` no puede ser verdadero nunca**, y `error <= 0` es
verdadero la mitad del tiempo por puro azar. Es una comparación que quedó a medio migrar.

**La prueba de que el equipo 2025 encontró esto:** en el archivo del delantero, esa misma línea
ya está reescrita:

```cpp
if ((millis() - millis_inicio_estado >= 9000) && (abs(error) <= 50))   // delantero.ino:448
```

y las otras también (`abs(error)<=1` en `delantero.ino:628` y `:699`, `abs(error)<=80` en
`:659` y `:730`). **Lo arreglaron de un solo lado.** El archivo del arquero quedó con la versión
vieja.

> 🔵 **Pero OJO — hoy esto es código muerto en el arquero.** Todas las líneas con `currentYaw`
> (606, 642, 656, 671, 712, 748, 762, 776) y la 422 viven en estados del **DELANTERO**. El arquero
> arranca en `Estado estado = impulso_inicial;` (`arquero.ino:131`) y su bloque de estados
> (`arquero.ino:1014-1205`) forma un conjunto cerrado que nunca sale hacia esos estados. O sea:
> **el síntoma "solo anda apuntando al norte" NO puede venir de estas líneas en el arquero.** Si
> ese síntoma se vio de verdad, la causa más probable es **(b)**. Verifiquen (b) primero.
>
> El fix de (c) solo importa el día que compilen `ROBOT2` en esa base, o cuando unifiquen los dos
> archivos. El parche está en `correcciones-propuestas.md` (A6), marcado como "no aplicar ahora".

---

## 6. Calibración: los cuatro números

El BNO055 **no guarda su calibración al apagarse**. Cada vez que lo prenden arranca
**des-calibrado**. No hay forma de que "se acuerde" de la calibración de ayer, salvo que el Teensy
se la guarde y se la devuelva. Eso hoy no existe en este código y es trabajo para más adelante.

El chip reporta cuatro números, cada uno de **0 a 3**, donde 3 es calibración completa:

| Número | Qué calibra | Cómo se calibra | Cuánto tarda |
|---|---|---|---|
| **GYRO** | el cero del giroscopio | robot **quieto** unos segundos, en cualquier posición | segundos, es trivial |
| **ACCEL** | el acelerómetro | apoyarlo en 6 posiciones estables distintas (las 6 caras de un cubo) | es el más tedioso |
| **MAG** | la brújula | mover el robot en el aire, rotándolo en varias direcciones | **puede no llegar nunca al lado de los motores** |
| **SYS** | resumen de la fusión | sale de los otros tres | — |

Se leen así:

```cpp
uint8_t calSys = 0, calGyro = 0, calAcc = 0, calMag = 0;
bno.getCalibration(&calSys, &calGyro, &calAcc, &calMag);
Serial.print("sys="); Serial.print(calSys);
Serial.print(" gyro="); Serial.print(calGyro);
Serial.print(" accel="); Serial.print(calAcc);
Serial.print(" mag="); Serial.println(calMag);
```

**El código actual NO los mira nunca.** Verificado: `getCalibration` aparece exactamente una vez
en todo el repo, en `zirconLib.cpp:75`, **dentro del bloque comentado**. Los dos `.ino` no la
llaman. Es decir: **hoy el robot juega sin tener idea de si su rumbo es confiable.**

### Por qué esperar más tiempo NO calibra la brújula

Este es el malentendido más común y hay que matarlo:

- **GYRO** se calibra **con el tiempo quieto**. Esperar sirve.
- **ACCEL** se calibra **cambiando de pose**. Esperar no sirve, hay que moverlo.
- **MAG** se calibra **rotando el sensor por distintas orientaciones del campo magnético**.
  Esperar **no sirve para nada**. Y si el campo magnético del entorno está contaminado por los
  motores, la estructura y los cables de potencia, puede **no llegar nunca a 3** por más que
  esperen una hora.

Por eso el `while (mag < 3)` de `zirconLib.cpp:74` es una bomba: es un bucle que espera algo que
puede no ocurrir jamás. Comentar toda la función (como hicieron) evita el cuelgue pero tira
también la información. Lo correcto es **medir y mostrar**, nunca **bloquear**.

**Regla operativa:** nunca poner un `while` esperando que un número de calibración llegue a 3.
Medilo, imprimilo, y seguí.

### Qué mirar según el modo

- En NDOF, que es el modo en el que está hoy (§1): `calMag = 0` significa que el rumbo absoluto
  **no es confiable** y puede pegar un salto a mitad de partido, aunque `initialYaw` se vea
  razonable al arrancar.
- Si el chip estuviera en un modo **sin brújula**: `calMag` se queda en 0 **para siempre y está
  bien** — no es un sensor fallado, es que ese modo no usa el magnetómetro. Ahí solo importa
  `calGyro = 3`.

Confundir estos dos casos hace que la gente persiga un fantasma durante horas.

### Idea a evaluar (propuesta, NO aplicar sin medir antes)

Si el magnetómetro nunca calibra (test 4 de §8), existe la opción de pedirle al chip un modo
**sin magnetómetro**: `IMUPLUS`, que fusiona giroscopio + acelerómetro. Ventaja: el rumbo pasa a
ser **relativo al encendido por construcción** — que es exactamente lo que el código quiere — y
deja de importarle el norte y los motores. Desventaja: **deriva**, hay que medir cuánta en banco
(robot quieto 5 minutos, anotar cuánto se corrió el ángulo).

```cpp
// PROPUESTO, falta validar en banco.
if (!bno.begin(OPERATION_MODE_IMUPLUS)) { ... }
```

> 📌 **Va SIN `Adafruit_BNO055::` adelante.** La constante pertenece al `enum
> adafruit_bno055_opmode_t`, que está declarado **fuera** de la clase
> (`Adafruit_BNO055.h:60-75`; la clase recién empieza en la línea 81). Escribir
> `Adafruit_BNO055::OPERATION_MODE_IMUPLUS` **no compila**. Verificado sobre la copia v1.6.4 que
> hay en este disco; si su versión instalada es otra, confírmenlo antes.

**Esto cambia el comportamiento del robot entero.** No es un ajuste: es cambiar de qué está
hecho el número. Se prueba en banco, con tiempo, nunca cerca de una fecha de competencia.

---

## 7. Árbol de diagnóstico para la mesa

Empezar arriba y bajar. Cada paso dice **qué medir** y **qué significa**.
Requisito previo: que el programa compile y suba (§0).

**Paso 0 — ¿el robot arranca?**
- No se mueve NADA, ni un motor → sospechar el `while(1)` de `arquero.ino:248`. Enchufar USB,
  abrir el monitor serie a **19200 baudios** (`BAUD_RATE`, `arquero.ino:77`) y prender. Si sale
  `"¡No se pudo encontrar el BNO055!"` → **es el I2C**: revisar pines 18/19 y la alimentación del
  módulo. Si sale `"Decodificador iniciado"` (`arquero.ino:241`) y después nada, también.
- Se mueve pero raro → seguir.

**Paso 1 — ¿el chip está listo cuando le preguntamos?**
- Medir: imprimir `sys_stat` de `bno.getSystemStatus(...)` **en el setup**, justo antes de leer
  `initialYaw`. (En el `loop()` no: mete 200 ms de freno por vuelta.)
- `5` = fusión corriendo, la lectura vale. `2/3/4` = todavía arrancando, **la lectura no vale**.
  `1` = error del sistema (imprimir también `sys_err`). `0` = idle. `6` = corriendo pero **sin**
  fusión.

**Paso 2 — ¿el cero de referencia sirve?**
- Medir: el test de los 5 encendidos de §5b.
- 5 veces `0.00` → bug (b) confirmado. 5 números distintos y coherentes → el cero está bien.

**Paso 3 — ¿el número sigue al robot?**
- Medir: robot quieto, monitor serie imprimiendo `currentYaw`, girarlo **90° a mano**.
- El número cambia ~90 → el sensor vive. No cambia nada → congelado (I2C intermitente o el chip
  se colgó). Cambia ~16 veces menos de lo esperado (unos 5,6° en vez de 90) → la conversión de
  escala se rompió: el chip entrega Euler a razón de 1 grado = 16 LSB y la librería divide por 16
  (`Adafruit_BNO055.cpp`, `getVector`, caso `VECTOR_EULER`). Si alguien tocó eso, aparece este
  síntoma.
- Ojo con el **sentido**: anoten si girando a la derecha el número sube o baja. Si algún día
  cierran un lazo de control, un signo invertido hace que el robot **corrija al revés** y se vaya
  girando cada vez más rápido.

**Paso 4 — ¿deriva?**
- Medir: robot **quieto** 5 minutos con `currentYaw` imprimiendo. Anotar el valor al inicio y al
  final.
- Se corrió mucho → deriva de giroscopio (esperable, hay que medir cuánto). Pega **saltos**
  bruscos estando quieto → es la **brújula** ensuciándose, no el giroscopio.

**Paso 5 — ¿los motores lo ensucian?**
- Medir: robot **quieto**, `currentYaw` imprimiendo, y hacer girar los motores en el lugar unos
  segundos (con las ruedas al aire para que no se mueva).
- Si el número se va mientras los motores giran y vuelve al parar → **es interferencia magnética
  sobre la brújula**. Esa es la prueba que decide si vale la pena el modo sin magnetómetro (§6).

**Paso 6 — ¿la calibración?**
- Medir: imprimir los 4 números de `getCalibration()` una vez por segundo durante un minuto.
- `gyro` debería llegar a 3 con el robot quieto en segundos. Si no llega, hay vibración o el
  robot no está realmente quieto. Si `mag` se queda en 0 (y el modo es NDOF) → rumbo no confiable.

---

## 8. Cómo se prueba el rumbo en banco

Robot sobre la mesa, ruedas al aire o sobre un trapo, USB conectado, monitor serie a **19200**.

### Sketch de diagnóstico (no toca la lógica del robot, solo imprime)

Agregar al final de `setup()` y dentro de `loop()`. Es temporal: se saca después de medir.

```cpp
// --- en setup(), después de leer initialYaw ---
  uint8_t s = 0, st = 0, se = 0;
  bno.getSystemStatus(&s, &st, &se);      // OJO: esta llamada tarda ~200 ms. Solo en setup().
  Serial.print("sysStat="); Serial.print(s);
  Serial.print(" selfTest=0x"); Serial.print(st, HEX);   // 0x0F = los 4 self-tests pasaron
  Serial.print(" sysErr="); Serial.print(se);
  Serial.print(" initialYaw="); Serial.println(initialYaw);

// --- en loop(), después de calcular error ---
  static unsigned long t_print = 0;
  if (millis() - t_print >= 200) {        // 5 veces por segundo, no satura el puerto
    t_print = millis();
    uint8_t cS=0, cG=0, cA=0, cM=0;
    bno.getCalibration(&cS, &cG, &cA, &cM);   // barata: una sola lectura I2C, sin delay
    Serial.print("yaw="); Serial.print(currentYaw, 1);
    Serial.print(" ini="); Serial.print(initialYaw, 1);
    Serial.print(" err="); Serial.print(error, 1);
    Serial.print(" cal S/G/A/M=");
    Serial.print(cS); Serial.print(cG); Serial.print(cA); Serial.println(cM);
  }
```

> ⚠️ Imprimir a 19200 baudios es **lento**. Si imprimen en cada vuelta del loop, el `Serial.print`
> frena el programa entero y el robot reacciona tarde a todo. Por eso el `if` de 200 ms. **Y por
> eso este código sale del robot antes de jugar.**

### Los cuatro tests, con criterio de aceptación

**Test 1 — el cero de referencia.** Prender 5 veces, apuntando a 5 direcciones bien distintas
(dos de ellas iguales: la primera y la última).
- ✅ **Acepta si:** los 5 valores de `initialYaw` son distintos entre sí, siguen la orientación
  real del robot, y las dos veces que apuntó igual dan una diferencia **menor a 10°**.
- ❌ **Falla si:** dan todos `0.00`, o si las dos iguales difieren mucho.

**Test 2 — sigue el giro.** Marcar en la mesa un ángulo de 90° con una escuadra. Robot quieto,
anotar `yaw`. Girarlo a mano exactamente 90°. Anotar.
- ✅ **Acepta si:** la diferencia (normalizada, §4) está entre **80 y 100 grados**, y el sentido
  es el mismo siempre.
- ❌ **Falla si:** no cambia, cambia ~16 veces menos de lo esperado (ver Paso 3 de §7) o el
  signo cambia entre repeticiones.

**Test 3 — deriva en reposo.** Robot quieto y **con los motores apagados** 5 minutos. Anotar `yaw`
al minuto 0 y al minuto 5.
- ✅ **Acepta si:** la diferencia es menor a **unos pocos grados** en 5 minutos. Como un partido
  dura menos, eso es tolerable.
- ❌ **Falla si:** se corre decenas de grados, o si pega saltos bruscos estando quieto.
- 📌 **No hay un número "correcto" copiado de otro robot.** El valor de ESTE robot hay que
  medirlo. Anótenlo en el repo con fecha, y a partir de ahí ese es el número del equipo.

**Test 4 — interferencia de los motores.** Robot quieto con las ruedas al aire. Anotar `yaw`.
Hacer girar los tres motores 10 segundos. Anotar durante y después.
- ✅ **Acepta si:** el `yaw` vuelve a estar dentro de unos pocos grados del valor inicial cuando
  los motores paran.
- ❌ **Falla si:** se corre mucho mientras giran y **no vuelve**. Eso es contaminación magnética
  → el rumbo absoluto no sirve en esta plataforma y hay que evaluar el modo sin brújula (§6).

**Cierre:** ninguno de estos tests lo cierra la IA. Los cierra el que tiene el robot en la mano,
con los números anotados. Si no hay números anotados, el test no se hizo.

---

## 9. Tabla rápida: síntoma → causa probable

| Lo que ven en la mesa | Causa más probable | Dónde mirar |
|---|---|---|
| No compila: `expected declaration before '}'` | llave de más al final de la librería | `zirconLib.cpp:355` · §0 |
| No compila: `multiple definition of 'bno'` | `bno` global en la librería y en el `.ino` | `zirconLib.cpp:4` vs `arquero.ino:68` · §0 |
| No se mueve nada al encender | `while(1)` porque el BNO no contestó en el I2C | `arquero.ino:248` · pines 18/19 |
| El barrido del arquero sale torcido, distinto cada día | `initialYaw` leído antes de que la fusión arranque | `arquero.ino:254` · test §5b |
| El delantero no patea nunca | `abs(error)<=1` nunca se cumple porque el cero está mal | `delantero.ino:628`, `:699` |
| El delantero patea para cualquier lado | mismo cero mal, pero cayendo del otro lado | `delantero.ino:278` → `:628`, `:699` |
| "Solo anda apuntando al norte" | rumbo crudo comparado contra absolutos… **pero en el arquero eso es código muerto** → sospechar el cero | `arquero.ino:606,642,...` (muertas) vs `:254` |
| El rumbo pega saltos estando quieto | la brújula ensuciándose (motores/metal) | test §8-4 |
| El rumbo se corre lento y parejo | deriva del giroscopio, esperable | test §8-3 |
| Toqué `kp` y no cambió nada | `correccion` se calcula y nunca se usa | `arquero.ino:340` (única otra aparición: `:69`) |
| El robot se cuelga esperando calibrar | `while` esperando que un número de calib llegue a 3 | patrón de `zirconLib.cpp:74` |
| Anda en la mesa, falla justo cuando el rumbo pasa por 0/360 | falta normalizar a ±180 en algún cálculo nuevo | `arquero.ino:338-339` |
| El loop se volvió lentísimo después de tocar el giroscopio | llamaron `getSystemStatus()` dentro del `loop()` (tiene `delay(200)`) | §5b, nota del `do/while` |

---

## 10. Lo que este robot NO tiene (y no hay que fingir que sí)

Van a encontrar documentos, skills y chats que hablan de cosas que **no existen en esta
plataforma**. Si alguien las nombra, es de otro robot (típicamente el de Incheon 2026, que tiene
otra electrónica):

- **No hay encoders** en las ruedas → no hay forma de saber cuánto giró una rueda. El rumbo del
  BNO es la **única** fuente de orientación que tiene el robot.
- **No hay odometría, ni OTOS, ni sensores ToF, ni ultrasonido, ni segunda cámara.** No se puede
  corregir el rumbo con una referencia externa. Lo que dice el BNO es lo que hay.
- **No hay un segundo BNO** ni un segundo bus I2C en uso. Un solo sensor en `Wire` (pines 18/19),
  sin red. (El `Wire2` que aparece en `zirconLib.cpp:66` está adentro de la función comentada.)
- **No hay PlatformIO ni "envs" de compilación.** Esto es Arduino IDE, un `.ino`, se carga por
  USB. Lo que elige el robot es el `#define ROBOT1` / `#define ROBOT2` (`arquero.ino:10-11`).
- **No hay RTOS, ni tareas, ni watchdog de firmware.** Un solo `loop()` que corre todo, en orden,
  para siempre.
- **No se accede al BNO por registros crudos.** Todo pasa por la librería Adafruit. Si un doc les
  muestra un mapa de registros del BNO055, es información de fondo, no algo que este código haga.
- **La memoria de calibración del BNO en el Teensy no está hecha.** Se puede hacer (guardar los
  offsets del chip y devolvérselos al arrancar), pero hoy no existe una línea de eso en el código.
  Es trabajo para más adelante, no algo que puedan "activar".

Y sobre los números: **cualquier valor medido de otro robot (pisos de PWM, velocidades tope,
ganancias, grados de deriva) no aplica acá.** Otra electrónica, otros motores, otra estructura.
El concepto se copia; el número se mide.

---

## 11. Reglas para no romper esto

1. **Primero que compile.** Sin el PARCHE 0 (§0) no hay medición posible, y todo lo demás es
   teoría.
2. **`error` es el número que manda, no `currentYaw`.** Toda comparación nueva se escribe contra
   `abs(error)`, nunca contra el rumbo crudo.
3. **Todo ángulo que se reste se normaliza a ±180.** Sin excepción.
4. **Nunca un `while` esperando calibración.** Medir e imprimir, seguir de largo.
5. **Nada con `delay` adentro va en el `loop()`** — y `getSystemStatus()` tiene un `delay(200)`
   escondido.
6. **Un cambio por vez, con el número anotado antes y después.** Si aplican el fix del cero Y
   cambian un umbral en la misma subida, no van a saber cuál hizo qué.
7. **Los `Serial.print` de diagnóstico salen del robot antes de jugar.** A 19200 baudios frenan
   el loop.
8. **Que compile no prueba nada.** El veredicto de "anda" lo da el banco, con los cuatro tests de
   §8 y sus números escritos.
