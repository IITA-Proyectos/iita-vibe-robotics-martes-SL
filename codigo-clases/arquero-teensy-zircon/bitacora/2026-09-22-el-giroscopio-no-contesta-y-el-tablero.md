# 2026-09-22 (martes) — El giroscopio no contesta, y ahora tenemos un tablero para verlo

**Quiénes:** el equipo + Claude (código, carga y lectura por USB)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas nuevos:** `pruebas/monitor-robot`, `pruebas/giro-imu`,
`funciona/seguir-y-despejar-imu`

**Resumen del día:** clase de diagnóstico, no de cancha. Se construyó un **tablero** que
muestra todo el robot en una pantalla que se refresca en su lugar, y con él se midió algo
que hasta hoy era una sospecha: **el giroscopio no contesta en absoluto** —
*0 lecturas buenas de 5453*. No es que se caiga: nunca arrancó. También se leyeron los
programas de 2025 y el del delantero, y se dejó escrita una versión del arquero en
**modo IMU** (sin brújula) lista para probar.

| | |
|---|---|
| §1 | Los amistosos del lunes (lo que falta anotar) |
| §2 | `cuadrado-giroscopo`: por qué parecía andar mejor |
| §3 | 🎯 **El tablero: `monitor-robot`** |
| §4 | 🔴 **La medición del día: el giroscopio está AUSENTE** |
| §5 | Cómo usaban el giroscopio en 2025 (y el delantero hoy) |
| §6 | La versión en modo IMU |
| §7 | Trampas y errores del día |

---

## 1. Los amistosos del lunes

Se jugaron **tres** contra la sede de Salta: **ganamos 5-0, perdimos 3-1, ganamos 3-2**.
Dos de tres.

⚠️ **En las bitácoras del 21/09 sólo figura el primero.** Los otros dos se jugaron después
de escribirlas, así que **no están anotados en ningún lado**: ni los resultados, ni qué se
vio, ni si el giroscopio aguantó. Lo único que quedó es el recuerdo del equipo de que
**en el último partido el giroscopio se rompió o no respondía por momentos**.

⬜ **Pendiente:** que el equipo cuente los partidos 2 y 3 y se agreguen a la bitácora del 21.

---

## 2. `cuadrado-giroscopo`: por qué parecía andar mejor

El equipo se acordaba de que con ese programa el giroscopio andaba mejor. Se cargó **tal
cual, sin tocarle nada**, para probarlo. Y leyéndolo apareció la razón — pero no la que
se esperaba.

**Lo que tiene y el programa de juego no:**

1. **Se da cuenta de que el sensor se calló.** La librería Adafruit devuelve `0.0` en los
   tres ángulos cuando no puede leer el chip. El sketch cuenta esas lecturas: **10
   seguidas en cero exacto = se cayó**.
2. **Lo revive en pleno movimiento**, y el orden importa:

   ```
   parar()  ← PRIMERO corta los motores
   Wire.end() → Wire.begin() → bno.begin() → 700 ms → setExtCrystalUse(true)
   20 lecturas: si menos de 10 sirven, NO lo da por recuperado
   ```

   Corta los motores primero **porque si la causa es el tirón de corriente de las tres
   ruedas, mientras sigan andando el chip no arranca nunca**. Y pide 20 lecturas porque el
   síntoma de este sensor es contestar "sí, existo" y devolver ceros: con una sola no se
   distingue de uno sano.
3. **Sin giroscopio no se mueve.** Prefiere quedarse quieto antes que manejar a ciegas.

**Lo que NO tiene:** al *arrancar* es **peor** que el programa de juego — llama a
`bno.begin()` una sola vez, sin reintentos, mientras que `seguir-y-despejar` insiste 10
veces. O sea que lo que lo hacía parecer confiable no era el arranque: **era que se
recuperaba a mitad de camino**. Que es exactamente el síntoma del lunes.

---

## 3. 🎯 El tablero: `pruebas/monitor-robot`

Pedido del equipo: ver los datos **cómodos**, no dato tras dato bajando por la pantalla.

Es un tablero de 25 líneas que **se queda quieto y sólo cambian los números**, como el
tablero de un auto:

```
=== MONITOR DEL ROBOT ARQUERO =============================     550 s ===

GIROSCOPIO                    [ AUSENTE: no contesta ]
  yaw     ->        --
  pitch   ->        --
  roll    ->        --
  calibracion  sistema 0   giro 0   acel 0   brujula 0   (con 0 el rumbo no vale)
  modo ?    estado: sin dato
  0 C   bus 400 kHz en 0x28   lecturas 0 ok / 5453 malas
  CAIDAS 0

SENSORES DE LUZ                         es BLANCO si pasa de 425
  frente    ->   505   BLANCO
  atras izq ->   651   BLANCO
  atras der ->   601   BLANCO

CAMARA                                   25.0 paquetes por segundo
  pelota    ->   38.0 cm de frente,  -8.7 cm al costado
  arcos     -> azul no     amarillo no

OPCIONES  1 reiniciar giro   2 reiniciar Teensy   3 yaw a cero
          4 NDOF   5 IMUPLUS   6 cristal   7 bus   8 direccion
          9 escanear I2C   h historial   m sacudon   0 pausar   ? ayuda
```

**Cómo hace para no irse hacia abajo.** Manda códigos que la consola entiende como órdenes
en vez de texto: `ESC[H` = "volvé arriba con el cursor", `ESC[K` = "borrá lo que quede a la
derecha". Cada vuelta vuelve arriba y reescribe las 25 líneas encima de las viejas.

⚠️ **Windows no los interpreta hasta que el programa de la PC se lo pide.** Hubo que
parchear `mirar.py` para que lo pida al abrirse (`ENABLE_VIRTUAL_TERMINAL_PROCESSING`).

Se abre con **`pruebas/monitor-robot/MONITOR.bat`**, que además agranda la ventana a 90×32
— el tablero necesita 26 renglones y si la ventana es más chica se ve temblando.

### Lo que distingue, y que antes se veía todo igual

| | Qué significa |
|---|---|
| **AUSENTE** | el chip no contesta nada en el bus → es eléctrico: VIN, GND, SDA, SCL |
| **MUDO** | contesta que existe, pero devuelve ceros |
| **SANO** | está dando datos |

🎯 Y cuando está MUDO, muestra **el modo del chip leído en crudo**. Si dice `CONFIG`, el
BNO055 **se reinició solo** — la sospecha que quedó anotada el 21/09 y nunca se pudo
probar. Si sigue en `NDOF` y aun así da ceros, es otra cosa.

---

## 4. 🔴 La medición del día: el giroscopio está AUSENTE

Con el tablero corriendo, esto es lo que mide:

| Qué | Valor |
|---|---|
| Lecturas buenas del giroscopio | **0** |
| Lecturas malas | **5453** |
| Tiempo encendido | 550 s |
| Estado | **AUSENTE — no contesta en el bus** |

**No es que se caiga a mitad de partido: nunca contestó ni una vez.** Se le mandaron las
teclas `4` (NDOF) y `5` (IMUPLUS) y las dos fallaron en el primer paso — el chip no aparece
en el bus, así que no hay modo que cambiar.

**Y el resto del robot anda perfecto**, lo cual acota el problema:

| Qué | Cómo anda |
|---|---|
| Cámara | ✅ 25 paquetes por segundo, viendo la pelota a 38 cm |
| Sensores de luz | ✅ los tres leyendo (505 / 651 / 601 — todos "BLANCO" porque está sobre la mesa, no sobre el verde) |
| Tablero | ✅ 5 refrescos por segundo |
| Giroscopio | ❌ **AUSENTE** |

⬜ **Lo primero a descartar es que la batería estuviera apagada**: el giroscopio come de la
batería, no del USB. No se llegó a confirmar en la clase.

---

## 5. Cómo usaban el giroscopio en 2025 (y el delantero hoy)

### Los programas de 2025: como brújula, y casi sin usarlo

```c
if (!bno.begin()) {               // sin argumento = NDOF, con brujula
  Serial.println("¡No se pudo encontrar el BNO055!");
  while (1);                      // se cuelga para siempre
}
bno.setExtCrystalUse(true);
bno.getEvent(&event);
initialYaw = event.orientation.x; // el "frente", leido al instante
```

**Respuesta a la pregunta: brújula (NDOF).** `begin()` sin argumento usa el modo por
defecto, que mezcla acelerómetro + giroscopio + **magnetómetro**.

Pero lo grande es otra cosa: **no lo usaban**. Está muerto en tres capas.

1. **El de la `zirconLib` no existe.** `CalibrateCompass()` está entera comentada con
   `/** ... **/`, y la llamada dentro de `InitializeZircon()` también. Como
   `compassCalibrated` sólo se ponía en `true` ahí adentro, queda en `false` para siempre y
   `readCompass()` devuelve `0`. Ninguno de los dos robots la llama, igual.
2. **La corrección se calcula y se tira.** `correccion = error * kp` (kp = 0.3) aparece
   **dos veces en todo el archivo**: donde se declara y donde se calcula. Nunca se usa.
3. **`CENTRANDO_giroscopo`** está en la lista de estados del arquero y no tiene `case`.

🎯 **Conclusión: no hay precedente que copiar.** No se puede decir "a ellos NDOF les
andaba", porque **nunca manejaron con el rumbo**. Si les saltaba 80°, no se enteraban.

Los cuatro problemas de esa inicialización son, uno por uno, los que nos vinieron mordiendo:
el `while (1)` mudo (de ahí salió nuestro LED de aviso), el `begin()` sin reintentos (lo
arreglamos el 01/09), leer `initialYaw` al instante sin esperar dato, y el modo con brújula.

### El delantero 2026: también NDOF, pero mejor resuelto

```c
bool arrancarGiroscopo() {
  if (!bno.begin()) return false;        // NDOF tambien
  bno.setExtCrystalUse(true);
  delay(700);
  bno.getSystemStatus(&sys, &autotest, &err);
  if (sys != 5) return false;            // 5 = la fusion esta corriendo
}
```

Hacen **dos cosas que nosotros no**:

**1. Le preguntan al chip en vez de adivinar.** Cada 500 ms leen `SYS_STATUS`; si no vale
5, lo dan por caído. Y el comentario dice por qué cambiaron:

> *"en ESTE robot el rumbo de reposo cae justo en el borde 359,9/0,0, y el método viejo lo
> daba por muerto en pleno juego"*

🎯 **Es exactamente la trampa que hay que evitar en modo IMU** (donde el yaw arranca en 0.0
exacto). Ellos la sufrieron en cancha con NDOF por casualidad de hacia dónde apuntaba el
robot; nosotros llegamos al mismo arreglo razonando. Dos caminos distintos, el mismo pozo.

**2. Arreglaron el orden de `setExtCrystalUse`:**

> *"AHORA después: `setExtCrystalUse` reinicia la fusión, así que esperar antes no servía"*

⬜ Nosotros todavía tenemos `delay(1000); bno.setExtCrystalUse(true);` — **ese segundo se
tira a la basura**. No nos rompe nada porque después esperamos un dato de verdad, pero con
la cuenta de arranque en 2 s, un segundo regalado pesa.

---

## 6. La versión en modo IMU

Se creó **`funciona/seguir-y-despejar-imu/`**, una copia completa. El original en
`funciona/seguir-y-despejar/` **no se tocó ni una línea**.

| | NDOF (el original) | IMUPLUS (la copia) |
|---|---|---|
| El cero es | el norte magnético | donde estaba al prender |
| Se corre solo | no | **sí, despacio** |
| Lo tuerce el metal y los motores | **sí** | no |
| Puede pegar un salto de golpe | **sí** | no |

Cambios respecto del original:

1. `bno.begin(OPERATION_MODE_IMUPLUS)` en vez de `bno.begin()`.
2. **La trampa del cero exacto.** En IMUPLUS el yaw arranca en `0.0`; si el robot está
   plano, los tres ángulos pueden dar cero y el programa lo leería como sensor muerto. Se
   agregó `chipSigueFusionando()`, que lee el registro `0x39` y pide que valga 5 — el mismo
   método del delantero.
3. **Esperar la calibración antes de fijar el cero** (hasta 6 s, quieto). El BNO055 mide su
   propio error de reposo quedándose quieto y recién ahí lo descuenta. Sin eso la deriva no
   es de un grado por **minuto** sino de hasta uno por **segundo**.
4. **Volver a tomar el cero en la línea blanca.** Cuando vuelve del despeje y **los dos**
   sensores de atrás ven blanco en la misma vuelta, el robot está cuadrado con la línea por
   construcción: ahí se vuelve a fijar `rumboBase`.

   ⚠️ Con **uno solo no**: con uno el robot está en diagonal y guardaría un cero torcido
   para el resto del partido. Frenar sigue usando "alguno" (como siempre), pero re-anclar
   exige "los dos".

⬜ **Compila, pero NO se llegó a cargar** — el USB estaba desenchufado al cerrar.

---

## 7. Trampas y errores del día

**El `while` sin salida de Adafruit.** Adentro de `bno.begin()`, línea 40 de
`Adafruit_BNO055.cpp`:

```c
write8(BNO055_SYS_TRIGGER_ADDR, 0x20);     // resetear el chip
delay(30);
while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) { delay(10); }   // ← sin salida
```

Si el chip no vuelve del reset, **el programa se queda ahí para siempre**. En una
herramienta de diagnóstico eso es lo peor posible, porque se usa justo cuando el sensor anda
mal. El monitor ahora hace el reset él mismo con tope de 1 segundo y sólo llama a `begin()`
si el chip volvió.

**Escanear un bus I2C sin abrirlo.** La tecla de escaneo miraba `Wire1` y `Wire2` sin
hacerles `begin()`. Sin eso el Teensy ni les conecta los pines: **contestaba "vacío" con
total seguridad aunque el giroscopio estuviera ahí**.

**`setClock` antes de `begin()` no sirve.** Adentro de `begin()` la librería reabre el bus,
y eso lo devuelve a 100 kHz. La tecla que cambia la velocidad no hacía nada. Va después.

**Se tildó la PC.** La revisión adversarial del monitor se lanzó con **48 agentes en
paralelo** y dejó la máquina sin aire. Encontró 43 hallazgos reales (los de arriba salen de
ahí), pero el costo fue perder varios minutos de clase. La segunda revisión se hizo con
**4 agentes** y anduvo igual de bien.

**Un error propio, anotado para no repetirlo.** Al escribir la versión IMU se justificó el
cambio diciendo que *"después de cada despeje el robot vuelve a la línea blanca para
corregir lo que se haya corrido"*. **Eso no existía en el código**: `rumboBase` se asignaba
en un solo lugar de todo el programa y la línea blanca sólo frenaba el retroceso —
corregía posición, nunca rumbo. Lo encontró la auditoría. Sin ese arreglo la versión IMU
habría sido **peor** que la NDOF, porque el programa corrige contra `rumboBase` y entonces
la deriva no se queda en un número: **se convierte en giro de verdad**. Al minuto 6 el
robot, quieto en su arco, empezaría a girar solo.

---

## Números de hoy

| Qué | Valor |
|---|---|
| Lecturas buenas del giroscopio | **0 de 5453** |
| Cámara | 25 paquetes/s, pelota a 38 cm |
| Sensores de luz (sobre la mesa) | 505 / 651 / 601 |
| Refresco del tablero | 5 por segundo, 25 líneas |
| Hallazgos de la revisión del monitor | 43 (48 agentes — tildó la PC) |
| Hallazgos de la auditoría de la versión IMU | 4 agentes, y encontró el grande |

---

## Qué queda pendiente

### 🔴 Lo primero

- ⬜ **¿Estaba prendida la batería?** El giroscopio come de ahí. Es lo primero a descartar
  antes de seguir buscando.
- ⬜ **El conector: VIN y GND.** Si la batería estaba prendida y el chip igual no contesta,
  esto ya no es software. Viene del 15/09 y hoy sumó la evidencia más fuerte: 0 de 5453.
- ⬜ **Correr `giro-imu`**: prender y apagar el robot 10 veces y contar cuántas arranca.
  Cargado y listo; no se alcanzó a hacer.
- ⬜ **Cargar `seguir-y-despejar-imu`** (compila, nunca se cargó) y probarlo en cancha.

### Del día

- ⬜ Anotar los partidos 2 y 3 del lunes en la bitácora del 21/09.
- ⬜ Mover `setExtCrystalUse` antes del `delay`, como el delantero — un segundo de arranque.
- ⬜ **Volver la espera extra del giroscopio a 15 s** (viene del 21/09).
- ⬜ **Que el programa de juego lo vuelva a buscar si se cae jugando** — hoy se confirmó que
  `cuadrado-giroscopo` ya tiene ese mecanismo escrito y probado; es cuestión de portarlo.

### De antes

- ⬜ La patrulla sin pelota (21/09), con los tiempos ya medidos.
- ⬜ La regla general "si está quieto y torcido, se endereza" (15/09).
- ⬜ Un LED que se vea (pines 9 o 10 libres).

---

## Lo que se aprendió del método

**1. Medir convierte una sospecha en un hecho.** Veníamos diciendo "el giroscopio anda a
veces". El tablero lo cambió por **0 de 5453**, que es una frase completamente distinta:
no es intermitente, no contesta nunca. Con eso la lista de sospechosos se achica sola.

**2. Que el diagnóstico distinga, no que resuma.** Lo más útil del tablero no es que muestre
el rumbo: es que separa AUSENTE de MUDO. Desde el programa de juego las dos se veían igual —
"el robot está chueco"— y apuntan a lugares opuestos: una al cable, la otra al chip.

**3. Leer el código viejo sirve, aunque sea para descartarlo.** La pregunta era "¿cómo lo
usaban en 2025?". La respuesta —**no lo usaban**— vale más que cualquier técnica que
hubiéramos copiado, porque evita razonar sobre un precedente que no existe.

**4. Revisar lo propio antes de cargarlo.** El error de la línea blanca lo encontró una
auditoría, no una prueba en cancha. Si se cargaba así, el robot habría empezado a girar solo
al minuto 6 de partido y habríamos culpado al sensor otra vez.

**5. Cuatro agentes alcanzan; cuarenta y ocho tildan la máquina.** Encontraron lo mismo.
