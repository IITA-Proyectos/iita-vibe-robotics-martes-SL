# Correcciones propuestas

Un parche por cada problema de [`bugs-conocidos.md`](bugs-conocidos.md). Todos pasaron por un
revisor independiente cuyo trabajo era **refutarlos**. Ninguno cayó, pero **tres volvieron
corregidos** — están marcados con 🔧.

> 🚨 **PROPUESTO ≠ ARREGLADO.** Nada de esto se probó en un robot. Que compile no prueba nada.
> Cada parche trae su prueba de banco con criterio de aceptación: **hasta que esa prueba no la
> haga una persona con el robot en la mano, el problema sigue abierto.** El que valida es el
> equipo, no la IA.

## Dónde se aplican los parches

**No en `robots-2025/`.** Esa carpeta es la foto congelada de lo que ganó el Nacional y no se
edita. El flujo es:

1. Copiá `arquero/arquero.ino` (o `delantero/delantero.ino`) a tu carpeta `alumnos/<tunombre>/`.
2. Aplicá el parche **ahí**.
3. Probalo. Anotá el resultado en [`bitacora/`](bitacora/).
4. Si el equipo decide adoptarlo, se sube en un commit aparte y se marca acá abajo.

**Excepción — la librería.** El parche 0 hay que aplicarlo sobre la copia de `zirconLib` que
está **instalada** en `Documentos/Arduino/libraries/`, no sobre la del repo. `#include <zirconLib.h>`
con los signos `< >` significa exactamente eso: *"buscalo en las librerías instaladas"*, no *"al
lado del programa"*. Si editás la copia del repo y compilás, no cambia nada y vas a pensar que el
parche no sirve.

## Orden en que conviene aplicarlos

Este orden no es capricho: hay parches que dependen de otros.

| Paso | Qué | Por qué en ese lugar |
|---|---|---|
| **0** | Parche 0 (la librería) | Sin esto no compila nada. No hay forma de probar ningún otro parche. |
| **1** | **Medir**, no parchear | `getZirconVersion()`, `initialYaw` cinco veces, y el ritmo real de la cámara. **Tres números.** Sin ellos, los parches que siguen son a ciegas. |
| **2** | A5 / DEL-03 (`initialYaw`) | Contamina a todos los demás: si la referencia de rumbo es basura, cualquier tiempo o umbral que midan sale contaminado. |
| **3** | A4 + A3 **juntos** | Tocan el mismo estado y se afectan entre sí. Aplicar uno solo confunde el diagnóstico. |
| **4** | A2 | Último de los del arquero, y con el timeout **medido** en el paso 1, no con el 1500 de fábrica. |
| **5** | DEL-02 | El de mejor relación beneficio/esfuerzo del delantero. |
| **6** | DEL-04, DEL-05, DEL-06 | De a uno, con banco entre medio. |
| **7** | A6 | Solo cuando compilen `ROBOT2`, y después de cerrar A5. Hoy es código muerto. |

---

# PARCHE 0 — Hacer que compile 🔴 P0

**Archivo:** `zirconLib.cpp` **de la copia instalada** en `Documentos/Arduino/libraries/zirconLib/`.

### Antes de tocar nada

En Arduino IDE: `Archivo → Preferencias → Mostrar salida detallada durante: compilación`.
Compilá el programa tal cual está y **anotá la ruta completa del `zirconLib.cpp` que aparece en
la consola**. Esa es la copia que manda. Si es distinta de la del repo, **guardá una copia antes
de pisarla**: puede ser la que ganó el Nacional.

### Parche 0.A — la llave de más

```cpp
// zirconLib.cpp — BORRAR la línea 355 (la llave "}" sola).
// El archivo tiene que TERMINAR así:

bool isCompassCalibrated() {
  return compassCalibrated;
}
```

### Parche 0.B — la variable duplicada

```cpp
// zirconLib.cpp línea 4
// ANTES:
//   Adafruit_BNO055 bno; // Define the Adafruit_BNO055 object
// DESPUÉS:
static Adafruit_BNO055 bno; // static = privada de este archivo, no choca con la del .ino
```

**Qué hace `static`, sin jerga.** Un programa de Arduino se arma en dos etapas: primero el
compilador traduce cada archivo por separado, después el *enlazador* junta todos los pedazos. En
esa segunda etapa, dos variables globales con el mismo nombre en archivos distintos son un
choque. `static` le dice al compilador *"esta es privada de este archivo, no la anuncies afuera"*.
Deja de haber dos con el mismo nombre visible y el choque desaparece.

**Por qué no rompe el giroscopio** (verificado, no supuesto): la única función de la librería que
usa esa variable es `readCompass()`, y **ninguno de los dos programas la llama nunca** — cero
apariciones en las 1207 y 1214 líneas. Además `readCompass()` está guardada por un `if` que
**nunca es verdadero**, porque la función que lo activaba está enteramente comentada. O sea: esa
variable de la librería hoy no se usa. `static` no le cambia el comportamiento a nadie.

### Orden en que van a ver los errores (para que no se frustren)

1. Compilan → error de la llave en `zirconLib.cpp:355`.
2. Borran la 355, compilan de nuevo → **ahora** aparece `multiple definition of 'bno'`. No es
   que rompieron algo: el segundo error siempre estuvo ahí, tapado por el primero.
3. Aplican el `static`. Compila.

Se verificó si había un **tercer** choque esperando: se buscaron en los `.ino` los 15 nombres de
variables globales que define la librería. `bno` es la única colisión.

### Prueba de banco

Con `#define ROBOT1` activo, apretar **Verificar** (el tilde).
**Criterio de aceptación:** la consola dice *"Compilación completada"* con 0 errores.
**Antes** del parche tiene que fallar mencionando `zirconLib.cpp:355`. Si falla distinto,
**parar y avisar**: hay una tercera copia de la librería dando vueltas.

---

# PASO 1 — Medir. Tres números antes de seguir

No es un parche. Es lo que evita que los siguientes sean a ciegas.

### Número 1 — qué versión de placa detecta

Agregar al final del `setup()`:

```cpp
Serial.print("Placa detectada: "); Serial.println(getZirconVersion());
```

**Esperado:** `Mark1`. Si dice `Naveen1`, **parar y avisar** — los pines de línea y de PWM
cambian, y la librería terminaría configurando el pin RX de la cámara como salida.

### Número 2 — si `initialYaw` sirve

Agregar justo después de donde se lee `initialYaw` (arquero L254 / delantero L278):

```cpp
Serial.print("initialYaw = "); Serial.println(initialYaw);
```

Prender el robot **5 veces**, cada vez apuntándolo a un lado distinto de la mesa (norte, este,
sur, oeste, y otra vez norte). Anotar los 5 valores.

- Si los 5 dan **0.00** o casi, aunque el robot mire para lados distintos → **el bug A5/DEL-03
  está confirmado** y el parche hace falta.
- Si los valores cambian y coinciden con la orientación real (comparar con la brújula del
  celular) → `initialYaw` estaba bien, y el parche queda como red de seguridad.

### Número 3 — a qué ritmo llega la cámara

Agregar dentro del bloque que lee la cámara, apenas valida las tres marcas:

```cpp
Serial.println(millis() - millis_pelota);
```

Robot quieto, pelota fija enfrente, monitor serie a **19200 baudios** (`BAUD_RATE`, línea 77),
mirar 30 segundos.

- Todos los números **menores a 200** → bien, el watchdog de A4 puede usar 500 ms.
- Valores cerca de **500** → subir el umbral del watchdog a (peor valor visto × 3).
- Valores **mayores a 500** → **no aplicar A4 todavía y avisar**: hay un problema de
  comunicación más de fondo.

---

# ARQUERO

## A5 — Arranque del giroscopio 🟡 P1

**Archivo:** `arquero.ino`, reemplazar las líneas **245-256**.

```cpp
//----------GIROSCOPO-------
  bool hayBNO = bno.begin();
  if (!hayBNO) {
    Serial.println("¡No se pudo encontrar el BNO055! Sigo SIN giroscopo.");
  } else {
    bno.setExtCrystalUse(true);
  }

  sensors_event_t event;
  initialYaw = 0;
  if (hayBNO) {
    // El BNO055 necesita tiempo despues de encenderse para que su mezcla de
    // sensores entregue un angulo valido: mientras tanto devuelve 0.00.
    // Leemos durante 1 segundo y nos quedamos con la ULTIMA lectura.
    unsigned long t0 = millis();
    while (millis() - t0 < 1000) {
      bno.getEvent(&event);
      initialYaw = event.orientation.x; // 0..360
      delay(50);
    }
  }
  Serial.print("initialYaw = "); Serial.println(initialYaw);
  millis_inicio_estado = millis();
}
```

**⚠️ Este parche cambia una decisión de diseño, y la decisión es del equipo.** Hoy la regla es
*"sin giroscopio no juego"* (el `while(1)`). Con el parche pasa a ser *"sin giroscopio juego
peor"*: si el BNO falla, `error` queda en 0, las funciones de reparto caen siempre en la rama
"derecho", y el arquero barre de costado sin corrección, derivando. Para un arquero eso es
jugable. **Pero si prefieren que no juegue antes que juegue mal, dejen el `while(1)` y apliquen
solo la parte del `initialYaw`.** Las dos posturas son defendibles — elijan y anótenlo.

**Prueba de banco:** repetir el *Número 2* del paso 1.
**Criterio de aceptación:** los 5 valores tienen que ser distintos entre sí, coherentes con la
orientación, y las dos veces que apunta al norte tienen que dar parecido (diferencia < 10°).
**Prueba extra:** desenchufar el BNO055 (I²C, pines 18/19) y prender. Con el parche el robot
tiene que arrancar igual y barrer; antes del parche no movía un motor.

## A4 + A3 — Watchdog de cámara y hueco de `Yp` 🟡 P1

> Van **juntos**. Tocan el mismo estado y se afectan entre sí: aplicar uno solo hace imposible
> atribuir el cambio de comportamiento.

### A4 — Watchdog de cámara

**Archivo:** `arquero.ino`, agregar después de la línea **331**.

```cpp
  // WATCHDOG DE CAMARA: haypelota solo vale si el ULTIMO dato con pelota llego
  // hace menos de 500 ms. Si la camara se cuelga o se desconecta, Xp/Yp quedan
  // viejos y no sirven para decidir nada.
  // 500 ms es el mismo numero que ya usa el delantero.
  if (millis() - millis_pelota >= 500)
  {
    haypelota = false;
  }
```

**⚠️ Efecto que hay que entender:** si la cámara muere, el arquero pasa a barrer de izquierda a
derecha **a ciegas**, sin patear. Eso es mejor que quedarse quieto, pero **no es gratis**: un
arquero barriendo a ciegas puede empujar la pelota para adentro. Es una elección consciente:
preferimos un robot que se mueve mal a un robot que no se mueve.

**No copiar este parche al delantero.** Ahí los estados de centrado usan ventanas de 4000 y
3000 ms, y un apagado duro a los 500 ms puede interactuar de formas que no se analizaron.

**Detalle inofensivo:** en el primer ciclo del loop la resta ya da más de 500 ms y el watchdog
pone `haypelota = false`. No se asusten: `haypelota` ya nace en false.

**Prueba de banco:** con el robot andando y la pelota enfrente, **desenchufar el cable de datos
de la OpenMV** (el que va al pin 0 / RX).
**Criterio de aceptación:** dentro de 1 segundo el robot vuelve a moverse de costado. Antes del
parche se queda quieto indefinidamente — **confirmen ese "antes" primero**. Al volver a
enchufar, tiene que retomar sin reiniciar.

### A3 — El hueco de `Yp = ±4`

**Archivo:** `arquero.ino`, líneas **1038** y **1086**.

```cpp
// ANTES (en las dos líneas):
//   if( (Xp <= tolerancia_cercania) && (abs(Yp) <= 3) )
// DESPUÉS:
     if( (Xp <= tolerancia_cercania) && (abs(Yp) < 5) )
```

**Nada más.** Las líneas 1045 y 1093 (`abs(Yp) >= 5`) quedan igual. Con el cambio los dos
umbrales quedan pegados: `|Yp| < 5` → patear (si está cerca), `|Yp| >= 5` → moverse. No queda
ningún valor de `Yp` sin cubrir, y no se superponen.

Se eligió **agrandar la ventana de patada** en vez de achicar la de movimiento, porque así se
conserva intacto el comportamiento correcto de *"quedarse firme cuando la pelota viene centrada
de lejos"*.

**Riesgo:** el arquero va a patear también con la pelota a `Yp = ±4` en vez de solo `±3`. Un
grado y monedas más de desalineación. Para un despeje no importa: el objetivo es sacarla del
área, no meter gol.

**Prueba de banco:**
1. Descomentar las líneas 292-293 para ver `Xp` e `Yp` por el monitor (agregarles un
   `Serial.println();` al final).
2. **Sin el parche todavía:** mover la pelota muy despacio hasta que el monitor muestre `Yp = 4`
   con `Xp < 140`, y dejarla quieta 3 segundos. **El robot tiene que quedarse quieto.** Si no se
   queda quieto, el bug no es lo que pensamos → **parar y avisar**.
3. Aplicar el parche y repetir. **Criterio de aceptación:** 5 de 5 intentos con `Yp = +4` y 5 de
   5 con `Yp = -4`, el robot arranca la patada dentro del primer segundo.
4. **No-regresión:** pelota lejos (`Xp > 140`) y `Yp = 0` → tiene que seguir quieto.

> 📌 **Dato que explica un síntoma raro:** el chequeo de línea blanca se evalúa **siempre**, sin
> `else`, y está **después** del bloque de la pelota. Si en el mismo ciclo se cumple la condición
> de patada **y** un sensor lee blanco, **gana el blanco**. Eso ya era así antes del parche, pero
> explica por qué a veces "no patea aunque esté alineado": es el borde del área, no este bug.

## A2 — Timeout del retroceso 🔴 P0

**Archivo:** `arquero.ino`, agregar dentro del `case PATEANDO_atras_arquero`, **después** del
`if` de la línea 1190 y **antes** del `break`.

```cpp
      // RED DE SEGURIDAD: si en 1500 ms ningun sensor vio la linea, cortar igual.
      // NO vamos a avanzar_despues_de_patear (ese estado avanza 1 s mas a ciegas)
      // porque si llegamos aca es justamente que NO sabemos donde estamos parados.
      if (millis() - millis_inicio_estado >= 1500)
      {
        parar();
        estado = moverce_derecha;
        millis_inicio_estado = millis();
      }
```

**Por qué el orden importa y por qué está bien:** si en el mismo ciclo se ve la línea **y** ya
pasaron 1500 ms, el primer `if` asigna el estado normal **y reinicia el cronómetro**. Entonces el
segundo `if` calcula ≈ 0, que no es ≥ 1500, y no dispara. **La salida normal por sensor gana**;
la red de seguridad no la pisa.

**⚠️ El 1500 es el único número inventado del parche — hay que medirlo.** Cronometrar cuánto
tarda **realmente** el retroceso normal (llamémoslo T) y poner el timeout en **T + 500 ms como
mínimo**. Si T > 1000 ms, cambiar el 1500. Si lo dejan corto, el timeout corta **siempre** y el
arquero nunca vuelve al arco: queda patrullando adelantado.

**⚠️ Esto es una red de seguridad, no un arreglo del problema de fondo.** Si el timeout empieza a
dispararse seguido en cancha, el mensaje **no** es "el parche anda": es que el sensor de línea
está sucio, flojo o mal calibrado para la luz de ese gimnasio. Los umbrales de blanco (líneas
50-52) son valores fijos que nunca se recalibran. **Revisen el sensor, no solo el número.**

**Prueba de banco:**
- **TEST A (medir T):** cronometrar desde que termina la patada hasta que frena solo.
- **TEST B (probar la red):** tapar los tres sensores de línea con cinta negra y levantar el
  robot con las ruedas al aire (o ponerle un libro grueso de tope atrás). Disparar la patada.
  **Criterio de aceptación:** las ruedas dejan de girar en reversa a los ~1500 ms. **Antes** del
  parche, con la cinta puesta giran indefinidamente — verifiquen ese "antes" primero.

---

# DELANTERO

## DEL-03 — Arranque del giroscopio 🔧 **parche corregido** 🟡 P1

El primer parche proponía un `delay(1000)` a ciegas. El revisor lo cambió por algo verificable:
**preguntarle al chip si ya arrancó**, y además imprimir la calibración — porque esperar más
tiempo **no calibra la brújula**, y sin ese número el equipo no tiene forma de saber si el
arreglo sirvió.

**Archivo:** `delantero.ino`, reemplazar las líneas **269-280**.

```cpp
//----------GIROSCOPO-------
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

  // El BNO055 tiene adentro un programa que mezcla giroscopo + acelerometro +
  // brujula para darte el angulo. Ese programa TARDA en arrancar. Si le
  // preguntamos enseguida contesta 0.00, y ese 0.00 queda como "para donde
  // miraba el robot al encender" durante TODO el partido.
  // En vez de esperar un tiempo inventado, le preguntamos al chip cuando esta
  // listo: getSystemStatus devuelve 5 = "fusion corriendo".
  // El tope de 3 segundos es para que el robot nunca quede colgado en cancha.
  uint8_t sys_stat = 0, self_test = 0, sys_err = 0;
  unsigned long t_espera = millis();
  do {
    bno.getSystemStatus(&sys_stat, &self_test, &sys_err);
    delay(20);
  } while ((sys_stat != 5) && (millis() - t_espera < 3000));

  // Descartamos las primeras lecturas y nos quedamos con la ultima.
  sensors_event_t event;
  for (int k = 0; k < 10; k++) {
    bno.getEvent(&event);
    delay(20);
  }
  initialYaw = event.orientation.x; // 0..360

  // DIAGNOSTICO PARA EL BANCO (no cambia el comportamiento, solo imprime).
  // Si sysStat no llega a 5, o si calMag es 0, la referencia NO es confiable:
  // el angulo puede pegar un salto a mitad de partido y romper "error".
  uint8_t calSys = 0, calGyro = 0, calAcc = 0, calMag = 0;
  bno.getCalibration(&calSys, &calGyro, &calAcc, &calMag);
  Serial.print("sysStat="); Serial.print(sys_stat);
  Serial.print(" calSys="); Serial.print(calSys);
  Serial.print(" calGyro="); Serial.print(calGyro);
  Serial.print(" calMag="); Serial.print(calMag);
  Serial.print(" initialYaw="); Serial.println(initialYaw);

  millis_inicio_estado = millis();
}
```

**No se cambió ninguna condición de la máquina de estados.** Ni la 448, ni la 628, ni la 659, ni
la 699, ni la 730. El código ganó un nacional; no se reescribe.

**⚠️ Este parche CAMBIA EL SIGNIFICADO de `error`.** Si hoy `initialYaw` vale 0 y después del
parche vale, por ejemplo, 137, entonces todas las comparaciones (`<=1`, `<=50`, `<=80`) empiezan
a medir otra cosa y el robot se va a comportar distinto, **quizá peor al principio**.
**NO subir este parche el día del torneo.** Aplicarlo, correr el test, y después volver a probar
los cinco umbrales en la cancha.

**Prueba de banco:** prender el robot 5 veces apuntándolo a 5 direcciones bien distintas y anotar
el `initialYaw=` impreso.
**Criterio de aceptación:** 5 números distintos que sigan la dirección real del robot. Si da
`0.00` siempre, el arreglo no alcanzó. Si `calMag` sale 0, la referencia no es confiable aunque
`initialYaw` se vea bien — el ángulo puede saltar a mitad de partido.

## DEL-02 — Elegir a qué arco atacar 🟡 P1

**Archivo:** `delantero.ino`.

```cpp
// ---- PARCHE A: reemplazar las líneas 64-66 ----
// ARCO CONTRINCANTE
// true  = hay que hacer gol en el arco AMARILLO
// false = hay que hacer gol en el arco AZUL
// >>> CAMBIAR ESTA LINEA Y VOLVER A SUBIR EL PROGRAMA ANTES DE CADA PARTIDO,
// >>> SEGUN DE QUE LADO NOS TOCO ATACAR <<<
const bool ATACAR_ARCO_AMARILLO = true;

bool ARCO_CONTRINCANTE = false;
int Ycontrincante = 0;

// ---- PARCHE B: reemplazar las líneas 354-356 ----
// --- COLOCAR CUAL ES EL ARCO AL QUE AHI QUE HACER GOL ---
  if (ATACAR_ARCO_AMARILLO)
  {
    ARCO_CONTRINCANTE = hayarco_amarillo;
    Ycontrincante = Yam;
  }
  else
  {
    ARCO_CONTRINCANTE = hayarco_azul;
    Ycontrincante = Yaz;
  }
```

Con `ATACAR_ARCO_AMARILLO = true` el comportamiento es **idéntico** al de 2025: riesgo de
regresión cero mientras no se toque la constante.

### Cuatro cosas que hay que saber antes de aplicarlo

**(a) Esto NO arregla la puntería en general.** Solo arregla una de las tres formas de patear.
Las otras dos siguen sin mirar ningún arco: la de "4 segundos orbitando + orientación casi igual
a la inicial", y la patada corta al pisar línea blanca. Si el robot arranca encendido mirando su
propio arco, la salida de 4 s dispara igual hacia el lado equivocado. **Eso es otro tema.**

**(b) NO reemplazar la constante por un botón todavía**, aunque sea tentador. Los botones existen
(pines 9 y 10), pero hay una contradicción sin resolver: el mapa de pines dice "pull-up interno"
y `zirconLib.cpp:339-340` los configura como `INPUT` **pelado**. Si el pull-up no existe en la
placa, el pin queda flotando y el robot **elegiría arco al azar en cada encendido** — peor que la
constante, porque el modo de falla es gol en contra. Primero medir con el multímetro.

**(c) Si usan la rama azul, recalibren la cámara antes.** En el script de la OpenMV el umbral
azul es **la mitad de exigente** que el amarillo (300 contra 600 píxeles), así que es más propenso
a agarrar manchas azules que no son el arco: partes de otros robots, ropa del público.

**(d) Procedimiento de partido.** Que la línea de la constante sea **lo único** que se toca.
Subir, confirmar por el monitor serie, y recién entrar. Un checklist pegado al robot vale más que
el comentario en el código.

**Prueba de banco:** imprimir `ARCO_CONTRINCANTE` y `Ycontrincante`. Con
`ATACAR_ARCO_AMARILLO = true`, tapar el arco azul y mostrar solo el amarillo → tiene que imprimir
`rival=1`. Tapar el amarillo y mostrar el azul → `rival=0`. Después poner la constante en `false`
y repetir: tiene que dar exactamente al revés. **Si en alguna de las 4 combinaciones falla, el
problema está en la cámara, no en este parche.**

## DEL-04 — Arranque de la patada 🔧 **parche corregido** 🟡 P1

> El parche original ponía la velocidad en **0** para restaurar la rampa. **El revisor lo tumbó**
> con un cálculo: la patada corta dura 200 ms y la rampa sube 5 de PWM cada 20 ms, o sea que
> arrancando de 0 llega a **PWM 50** y ahí se corta. Probablemente el motor ni arranque. La
> patada larga llegaría a 125 en vez de 240. **El parche original dejaba al robot sin patada.**

El defecto real no es "falta la rampa": es que **con qué velocidad arranca la patada depende de
lo que el robot venía haciendo antes**. Eso se arregla haciendo el arranque **determinista**, y
el valor por defecto se elige igual al que el robot tenía de hecho en el Nacional (240). Así el
comportamiento queda idéntico al que ganó, el código deja de mentir, y queda **una sola perilla**
para que el equipo pruebe la rampa cuando quiera.

**Archivo:** `delantero.ino`.

```cpp
// ---- PARCHE 0: agregar DESPUÉS de la línea 73 ----
// Con que PWM ARRANCA cada patada. 240 = igual que velocidadFinalPateo, o sea
// la patada arranca a full: EXACTAMENTE lo que hacia el robot en el Nacional
// 2025 (la rampa suave nunca llegaba a usarse porque velocidadActualPateo
// quedaba pegada arriba de una patada a la otra).
// Si quieren probar el arranque suave: bajen esto a 0 Y lean la nota de abajo.
int velocidadInicialPateo = 240;

// ---- PARCHE A: reemplazar las líneas 847-855 (patada corta) ----
    case PATEANDO_corto_pausa_inicial:
      parar();
      velocidadActualPateo = velocidadInicialPateo; // arranque determinista:
      tiempoAnteriorPateo  = millis();              // no heredar la patada anterior
      if (millis() - millis_inicio_estado >= 500)
      {
        estado = PATEANDO_corto_adelante;
        millis_inicio_estado = millis();
      }

    break;

// ---- PARCHE B: reemplazar las líneas 891-897 (patada larga) ----
    case PATEANDO_pausa_inicial:
      parar();
      velocidadActualPateo = velocidadInicialPateo; // arranque determinista
      tiempoAnteriorPateo  = millis();
      if (millis() - millis_inicio_estado >= 1000)
      {
        estado = PATEANDO_adelante;
        millis_inicio_estado = millis();
      }
```

> ⚠️ En el parche B se reemplazan **solo** las líneas 891 a 897. Los chequeos de línea de las
> 899-913 y el `break;` de la 914 quedan tal cual, **no se borran**.

**Por qué va acá:** las 6 puertas de entrada a una patada pasan primero por uno de estos dos
estados de pausa. Y poner la asignación **dentro del `case`** (no dentro del `if`) hace que se
reescriba en cada vuelta mientras dura la pausa, así el valor que se lleva a la patada es siempre
el correcto sin importar cuántas vueltas dé.

### Si algún día quieren la rampa de verdad

La rampa sube 5 de PWM cada 20 ms = 250 de PWM por segundo. Pero:

| Patada | Dura | Arrancando de 0 llega a |
|---|---|---|
| corta | 200 ms | **PWM 50** de 255 |
| larga | 500 ms | **PWM 125** de 255 |

Para que llegue a 240 dentro de la ventana hay que subir `pasoPateo`: la corta necesita ≥ 24, la
larga ≥ 10.

**Y una contra física que conviene decir de una:** cualquier rampa entrega **menos golpe total**
que arrancar a full, porque el motor pasa parte de la patada a media potencia. **La rampa sirve
para no pegarle un tirón de corriente a la batería y a los drivers, no para pegarle más fuerte a
la pelota.** Es un trade-off, no una mejora gratis.

**Prueba de banco:** con `velocidadInicialPateo = 240` **no debería cambiar nada** respecto de
hoy. **Criterio de aceptación:** ejecutar 5 patadas cortas midiendo con cinta métrica cuánto
recorre la pelota, antes y después. Si cambia algo, **el parche está mal aplicado y hay que
volver atrás**.

## DEL-05 — La pausa de 700 ms 🟡 P1

**Archivo:** `delantero.ino`.

```cpp
// ---- PARCHE A: agregar después de la línea 143 ----
int i = 0;
unsigned long millis_freno_pelota = 0;   // cuando empezo a frenar tras ver la pelota

// ---- PARCHE B: reemplazar las líneas 432-445 ----
    case GIRANDO:
      if (haypelota)
      {
        parar();
        // arrancar el cronometro de frenado la PRIMERA vez que vemos la pelota
        if (millis_freno_pelota == 0) { millis_freno_pelota = millis(); }
        if (millis() - millis_freno_pelota >= 700) // esperar 700ms por la inercia
        {
          estado = APUNTAR_PELOTA;
          millis_inicio_estado = millis();
          millis_freno_pelota = 0;
        }
      }
      else
      {
        girar();
        millis_freno_pelota = 0;   // si perdimos la pelota, el cronometro se borra
      }
```

**El mecanismo del arreglo:** el problema era que el estado usaba **un solo reloj para dos cosas**
(cuánto lleva girando, y cuánto lleva frenando). El parche le da al frenado su propio cronómetro.

**⚠️ Agrega 700 ms de demora REAL que hoy no existe.** El robot va a **parecer más lento** en
reaccionar. Es el comportamiento que los chicos habían querido programar, pero **hay que
confirmar que 700 ms es el número correcto y no 300**.

**No tocar las líneas 405-413 en el mismo cambio**, aunque tengan el mismo error. Dejarlas hasta
que este parche esté validado, para poder atribuir cualquier cambio de comportamiento a una sola
cosa.

**Prueba de banco:** imprimir cada cambio de estado con su marca de tiempo. Robot en la cancha
**sin pelota**, dejarlo entrar en `GIRANDO`, contar 3 segundos, y recién ahí meter la pelota en
su campo de visión.
**Criterio de aceptación:** entre que el robot frena en seco y el cambio a `APUNTAR_PELOTA` tienen
que pasar **entre 650 y 800 ms**. Hoy esa diferencia es de menos de 50 ms. Complementario: filmar
con el celular y confirmar que queda **totalmente quieto** antes de empezar a apuntar.

## DEL-06 — Defender el lector de la cámara 🟡 P1

**Archivo:** `delantero.ino`. Son dos cambios sobre el bloque de las líneas 287-353.

```cpp
// ---- CAMBIO 1: agregar ANTES del "if (Serial1.available() >= 9)" ----
  // Descartar tramas viejas: si en el buffer hay mas de una, tiramos las viejas
  // de a 9 bytes (asi no rompemos el alineamiento) y nos quedamos con la ultima.
  // La posicion de la pelota de hace 200 ms no sirve para nada.
  while (Serial1.available() >= 18)
  {
    for (int k = 0; k < 9; k++) { Serial1.read(); }
  }

// ---- CAMBIO 2: al "if (header1 == 201 && header2 == 202 && header3 == 203)"
//      que hoy NO tiene else, agregarle este else ----
      else
      {
        // Trama rota o desalineada: NO seguimos creyendo los datos viejos.
        hayarco_azul = false;
        hayarco_amarillo = false;
        haypelota = false;
      }
```

**⚠️ Medir ANTES de aplicar.** Si el loop corre más rápido que la cámara, el `while` de descarte
**no se ejecuta nunca** y esa mitad del parche no aporta nada. Contar tramas y mirar
`Serial1.available()`.

**⚠️ Dos efectos secundarios reales:**
1. Con datos más frescos el robot puede volverse **más nervioso** y necesitar reajuste de la
   tolerancia de apuntado (línea 120, 15 grados). Hoy el retraso del buffer lo estaba "suavizando".
2. El `else` nuevo pone `haypelota = false` ante cualquier trama dudosa → va a haber **parpadeo**
   donde antes había un valor pegado. En `GIRANDO` eso se traduce en un frenar/girar entrecortado.
   Si se ve feo, la alternativa es contar tramas malas y recién apagar `haypelota` después de 3
   seguidas.

**La solución de fondo** — un byte de verificación en la trama — requiere tocar también el
programa MicroPython de la OpenMV. Eso ya no es un parche mínimo: es un tema aparte.

**Prueba de banco:**
- **PASO 1 (medir):** contador de tramas malas + imprimir `Serial1.available()` cada segundo,
  60 segundos con la pelota moviéndose. **Criterio:** tramas malas = 0 y `available()` nunca
  supera 17. Si llega a 40-64, el buffer se está llenando y el descarte hace falta de verdad.
- **PASO 2 (robustez):** desenchufar el cable de la cámara 2 segundos y volver a enchufar.
  **Criterio:** en menos de 1 segundo vuelve a seguir la pelota, y durante la desconexión **nunca**
  persigue una pelota que no está.

---

# A6 — Para cuando compilen ROBOT2 🔵 P2

**No aplicar ahora.** Es código muerto en el build del arquero, y cambiar 8 comparaciones en
estados que **no se pueden probar** es exactamente como se rompe un código que funcionaba.

Además **depende de cerrar A5 primero**: si `initialYaw` es basura, se cambia un umbral roto por
otro umbral roto.

```cpp
// ===== líneas 606 y 712 (idénticas) =====
// ANTES: if ( (millis()- millis_inicio_estado >= 5000) && ((currentYaw <= 10) or (currentYaw >= 350)))
// DESPUÉS:
        if ( (millis()- millis_inicio_estado >= 5000) && (abs(error) <= 10))

// ===== líneas 642, 656, 671, 748, 762, 776 (las seis idénticas) =====
// ANTES: if ((currentYaw <= 90) or (currentYaw >= 270))
// DESPUÉS:
            if (abs(error) <= 90)

// ===== línea 422 =====
// ANTES: if ((millis() - millis_inicio_estado >= 8000) && ((error <= 0) or (error >= 350)))
// DESPUÉS:
      if ((millis() - millis_inicio_estado >= 8000) && (error <= 0))
```

Los tres cambios dicen **exactamente lo mismo que la intención original**, pero medido contra el
rumbo de arranque (`error`) en vez de contra el norte magnético (`currentYaw`). El tercero
además elimina una condición que **nunca podía ser verdadera**: `error` ya está acotado a
±180, así que `error >= 350` es imposible.

### La prueba de que hoy es código muerto (2 minutos, y enseña a leer el grafo de estados)

1. Agregar al principio del `case CENTRANDO_horario`:
   `Serial.println("ENTRE A CENTRANDO_horario");`
2. Jugar 3 minutos completos con el arquero: pelota, patadas, líneas blancas, todo.

**Criterio de aceptación:** ese mensaje **no tiene que aparecer nunca**. Si aparece aunque sea una
vez, el análisis de alcanzabilidad está mal y hay que revisar todo este hallazgo de cero —
**avisen**. Después, borrar el `Serial.println`.

---

## Registro de qué se probó

Llenar a medida que se validen. Un parche sin fila acá **no está validado**.

| Parche | Aplicado por | Fecha | Resultado en banco | Adoptado |
|---|---|---|---|---|
| 0 — librería | | | | |
| Paso 1 — medir | | | | |
| A5 | | | | |
| A4 + A3 | | | | |
| A2 | | | | |
| DEL-03 | | | | |
| DEL-02 | | | | |
| DEL-04 | | | | |
| DEL-05 | | | | |
| DEL-06 | | | | |
| A6 | | | | |
