---
name: omni-3-ruedas-teensy
description: Tren motriz omnidireccional de 3 ruedas a 120 grados sobre Teensy 4.1 + placa Zircon, tal como está cableado en los robots de fútbol 2025 del IITA (arquero y delantero). Usar SIEMPRE que haya que entender, tocar o depurar CÓMO SE MUEVE el robot - "el robot se mueve mal", "traslada al revés", "va en diagonal cuando le pido de costado", "da vueltas en vez de ir derecho", "gira solo mientras se mueve al costado", "una rueda no arranca", "el motor zumba y no gira", "le subo el PWM y no cambia nada", "quiero que vaya de costado / strafe", "cómo hago que avance", "cómo prendo un motor", "analogWrite", "digitalWrite", "PWM1 PWM2 PWM3", "INA1 INB1", "motor 1 / motor 2 / motor 3", "rueda trasera", "rueda delantera izquierda/derecha", "omni", "omnidireccional", "ruedas locas", "rodillos", "120 grados", "cinemática", "vx vy omega", "matriz de las ruedas", "descomponer la velocidad", "piso de PWM", "zona muerta", "arranque", "kickstart", "impulso_inicial", "girar()", "avanzar()", "parar()", "retroceder1/2/3", "avanzar_patear()", "aiproporcional", "adproporcional", "pd", "patadM1", "patadM2", "ROBOT1", "ROBOT2", "el define", "cargué el programa equivocado", "el robot está roto". También para pasar de las tablas de PWM hechas a mano a una cinemática de verdad. NO usar para el rumbo/heading en sí (de dónde sale `error`, calibración y arranque del BNO055 - eso va en la skill del giroscopio), NO para los sensores de línea ni la cámara OpenMV, NO para la máquina de estados / estrategia (eso va en la skill de la FSM).
---

# Tren motriz omni de 3 ruedas — Teensy 4.1 + Zircon

Esta skill es sobre **el movimiento**: cómo se mueve un robot de 3 ruedas omni a 120°, y cómo
se programa eso en **este** robot, el que ganó el Nacional 2025.

Regla de la casa, antes de arrancar: **acá no hay nada "probado"**. Todo lo que dice esta skill
sobre el código está leído línea por línea y citado como `archivo:línea`. Todo lo que propone
como mejora está **propuesto, falta validar en banco**. Que compile no prueba absolutamente nada.
El veredicto lo da el que tiene el robot en la mano.

---

## 1. Por qué 3 ruedas omni son otra cosa que 2 ruedas

Ustedes vienen del Spike Prime: dos ruedas motrices y una rueda loca atrás. Ese robot tiene una
limitación que quizás nunca notaron porque es tan obvia que se vuelve invisible:

> **Un robot de 2 ruedas NO puede moverse hacia el costado.** Para llegar a un punto que está a
> su izquierda tiene que primero girar, después avanzar, después volver a girar. Tres maniobras.

Eso se llama ser **no holónomo**: tiene 3 grados de libertad en el piso (x, y, orientación) pero
solo puede comandar 2 (avanzar y girar). La dirección hacia la que apunta y la dirección hacia la
que se mueve están **atadas**.

Una rueda omni rompe ese candado. Es una rueda con **rodillos libres montados alrededor del
neumático**, con sus ejes perpendiculares al eje de la rueda. Consecuencia mecánica:

- **En la dirección de rodado** (la de siempre, hacia adelante-atrás de la rueda) la rueda agarra
  y empuja normal.
- **En la dirección perpendicular** los rodillos giran solos y la rueda **se deja arrastrar sin
  oponerse**. Casi no hay fricción lateral.

Cada rueda omni, entonces, aporta fuerza **en un solo eje** y es transparente en el otro. Poné
tres ruedas así, separadas 120° alrededor del centro del robot, y las tres direcciones de rodado
apuntan a tres lados distintos que cubren todo el plano. Combinando cuánto empuja cada una,
podés armar **cualquier** vector de velocidad y, encima, cualquier velocidad de giro, de forma
independiente.

Eso es un robot **holónomo**: 3 grados de libertad, 3 comandos. El robot puede **trasladarse al
costado sin girar la cabeza**, o girar sin moverse del lugar, o las dos cosas al mismo tiempo.

Para un arquero esto es todo el juego: patrulla la línea de gol de izquierda a derecha
**mirando siempre a la cancha**, sin perder de vista la pelota. Con dos ruedas eso es imposible.

Y ojo con el precio, que es real:
- El agarre lateral es **casi nulo** — un empujón de costado te mueve el robot.
- Los rodillos hacen que la rueda "vibre" al rodar; no es tan suave como una rueda maciza.
- Nunca sabés dónde estás: no hay encoders en este robot (§9), así que todo es a ciegas y por tiempo.

---

## 2. La matemática: repartir vx, vy y ω entre las 3 ruedas

### 2.1 Qué le pedís al robot

Tres números, siempre los mismos tres:

| Símbolo | Qué es | Unidad |
|---|---|---|
| `vx` | velocidad hacia la **derecha** del robot (positivo = a la derecha) | mm/s |
| `vy` | velocidad hacia el **frente** del robot (positivo = adelante) | mm/s |
| `ω` (omega) | velocidad de **giro** sobre su propio eje (positivo = antihorario visto desde arriba) | rad/s |

Esos ejes son **del robot, no de la cancha**. Si el robot está torcido 30°, su "frente" está
torcido 30°. Es una convención; lo importante es **elegir una y no mezclarla nunca**.

### 2.2 La fórmula

A cada rueda `i` le corresponde un **ángulo de posición** `θ_i`: dónde está atornillada esa rueda,
medido desde el eje +X (la derecha del robot) girando en sentido antihorario. Y todas están a la
misma distancia `R` del centro.

```
v_i  =  -vx · sin(θ_i)  +  vy · cos(θ_i)  +  ω · R
```

Tres términos, tres mecanismos distintos:

1. **`-vx·sin(θ_i) + vy·cos(θ_i)`** es una **proyección**. El par `(-sin θ_i, cos θ_i)` es la
   dirección en la que esa rueda empuja. De todo el vector de velocidad que vos pediste, la rueda
   solo "siente" la parte que cae sobre su propia dirección; la parte perpendicular se la comen
   los rodillos. Por eso es un producto punto y no una suma cualquiera.
2. **`ω·R`** es el término de giro, y es **idéntico para las tres**. Si el robot gira sobre su
   centro, las tres ruedas están a la misma distancia `R` del eje de giro, así que las tres tienen
   que rodar a la misma velocidad lineal. `R` es el brazo de palanca.
3. El resultado `v_i` es **con signo**: positivo = la rueda gira para un lado, negativo = para el
   otro. En este robot el signo se traduce a los pines INA/INB (§3), nunca a un "PWM negativo".

### 2.3 Los ángulos de ESTE robot

**El código de este robot no calcula ninguna cinemática.** No hay matriz, no hay senos ni cosenos:
hay **tablas de PWM hechas a mano**, un valor fijo por movimiento (`arquero.ino:140-184`). Así que
los ángulos no están escritos en ningún lado. Pero **se pueden deducir**, y el resultado se puede
verificar contra tres funciones del código, lo cual es una prueba bastante fuerte.

Lo que sí está escrito, en los comentarios del propio código
(`arquero.ino:194-199`, dentro de `aiproporcional()`):

```
   //motor izquierdo      -> INA2/INB2/PWM2   = motor 2
   //motor derecho        -> INA1/INB1/PWM1   = motor 1
   //motor atras          -> INA3/INB3/PWM3   = motor 3
```

O sea: **M1 = delantera derecha, M2 = delantera izquierda, M3 = trasera.** Con el frente en +Y y
la derecha en +X, sus ángulos de posición son:

| Rueda | Posición | θ (desde +X, antihorario) | −sin θ (coef. de vx) | cos θ (coef. de vy) |
|---|---|---|---|---|
| M1 | delantera **derecha** | 30° | **−0,500** | **+0,866** |
| M2 | delantera **izquierda** | 150° | **−0,500** | **−0,866** |
| M3 | **trasera** | 270° | **+1,000** | **0,000** |

Y ahora las tres verificaciones contra el código real:

| Movimiento | Lo que predice la fórmula | Lo que hace el código | ¿Coincide? |
|---|---|---|---|
| Avanzar (vy>0) | `[+0,866 ; −0,866 ; 0]` — las dos delanteras iguales y **al revés una de otra**, la trasera **quieta** | `avanzar()` (`arquero.ino:151-155`): PWM1=100 dir(1,0), PWM2=100 dir(0,1), **PWM3=0** | Sí |
| Girar (ω) | `[+R ; +R ; +R]` — **las tres iguales, mismo sentido** | `girar()` (`arquero.ino:140-144`): las tres a `100*g`=30 (`g=0.3`, `arquero.ino:80`) y las tres con dir(0,1) | Sí |
| Ir de costado (vx) | `[−0,5 ; −0,5 ; +1,0]` — delanteras iguales entre sí, **trasera al doble y al revés** | `aiproporcional()` banda central (`arquero.ino:188-192`): PWM1=50, PWM2=50, **PWM3=89**, la trasera con la dirección opuesta a las delanteras | Sí en el patrón. En el número: 89/50 = **1,78**, no 2,00 |

Esa última diferencia (1,78 en vez de 2,00) es la primera pista de un tema grande: **el PWM no es
la velocidad**. Volvemos en §6.

**Una cuarta verificación, y la contra que trae.** Hay tres funciones más que encajan solas con el
modelo: `retroceder1()`, `retroceder2()` y `retroceder3()` (`arquero.ino:157-171`). Cada una deja
**una rueda en PWM 0** y las otras dos en 100 con direcciones opuestas entre sí. Traducido a la
suma con signo de §2.5, las tres dan **cero**: son traslaciones puras, sin nada de giro. Y "una
rueda en cero" es exactamente lo que sale de la fórmula cuando el robot se mueve **perpendicular
a la dirección de rodado de esa rueda**. O sea: alguien las armó con la geometría en la cabeza,
aunque no la haya escrito.

**Precisión importante:** estas funciones son evidencia del **código fuente**, no de lo que corre en
cada robot. En el build del arquero (`ROBOT1`) `girar()` (llamada en `arquero.ino:419`) y
`retroceder1/2/3` (`980`, `992`, `1004`) viven en estados del **delantero** y **nunca se ejecutan**;
la que sí corre en el arquero es `avanzar()` (`arquero.ino:1199`). En el build del delantero pasa al
revés. Los dos archivos salieron del mismo programa y la numeración de motores es la misma
(motor 1 = delantera derecha en ambos), así que la geometría que se deduce vale igual — pero no
digan "el arquero gira con `girar()`", porque no.

La contra, que hay que decir: metiendo esos PWM en la fórmula con los ángulos de la tabla,
`retroceder1()` y `retroceder2()` salen con una componente **hacia adelante**, no hacia atrás.
O el nombre de la función es descuidado, o el "frente" del robot no está donde lo pusimos. Los
nombres de las funciones **no son evidencia**. La prueba de §5.2 sí.

> Lo honesto: la tabla de ángulos es **deducida** (comentarios del código + geometría de un omni de
> 3 a 120° + las firmas de arriba). **No está escrita en ningún archivo del robot y no se midió con
> transportador.** Tampoco se midió `R`. Antes de basar código nuevo en ella, hagan la prueba rueda
> por rueda de §5.2.

**Detalle de C++ que conviene ver una vez:** `analogWrite(PWM1, 100 * g)` con `g` de tipo `float`
da un número con coma (30,000002) y `analogWrite` lo **trunca**, no lo redondea. Acá sale 30 y
está bien, pero si la cuenta hubiera dado 29,9999 el motor recibiría **29**. Cuando multipliquen
PWM por factores, hagan la cuenta a mano y miren qué entero queda.

### 2.4 Un ejemplo numérico, a mano, sin saltear pasos

Supongamos `R = 90 mm` (medilo con una regla desde el centro del robot al centro de una rueda; en
este robot **no está en el código**) y pedimos: **avanzar a 300 mm/s mientras gira despacio a
1 rad/s** (≈ 57°/s).

Datos: `vx = 0`, `vy = 300`, `ω = 1`. Primero el término de giro, que es común:
`ω·R = 1 × 90 = 90 mm/s`.

**Rueda M1 (θ = 30°):** `sin 30° = 0,5` ; `cos 30° = 0,866`

```
v1 = -0 × 0,5  +  300 × 0,866  +  90
v1 =    0      +     259,8     +  90   =  +349,8 mm/s
```

**Rueda M2 (θ = 150°):** `sin 150° = 0,5` ; `cos 150° = -0,866`

```
v2 = -0 × 0,5  +  300 × (-0,866)  +  90
v2 =    0      +      -259,8      +  90   =  -169,8 mm/s
```

**Rueda M3 (θ = 270°):** `sin 270° = -1` ; `cos 270° = 0`

```
v3 = -0 × (-1)  +  300 × 0  +  90
v3 =     0      +     0     +  90   =   +90,0 mm/s
```

Resultado: `[+349,8 ; −169,8 ; +90,0]`.

Leelo así: las dos delanteras hacen casi todo el avance (una para un lado, la otra para el otro —
sus componentes laterales se cancelan y quedan sumando hacia adelante), la trasera **no aporta
nada al avance** pero igual gira, porque le toca su parte del giro. Y las tres tienen `+90` sumado
encima: eso es el giro repartido.

Si ponés `ω = 0` te queda `[+259,8 ; −259,8 ; 0]`: la trasera **parada**. Exactamente lo que hace
`avanzar()` con `PWM3=0`. La cuenta cierra con el robot real.

### 2.5 El truco al revés: leer las tablas de PWM del código

Si dan vuelta las tres ecuaciones (álgebra de secundaria, sumar y restar) sale algo muy útil:

```
ω · R  =  (v1 + v2 + v3) / 3
```

**La suma con signo de las tres ruedas te dice cuánto gira el robot.** Si suman cero, no gira. Es
el chequeo más rápido que existe para saber si un movimiento "de costado" va a salir derecho o
torcido, y en §7 lo usamos para desarmar lo que hace el código del arquero.

---

## 3. Cómo se maneja un motor en ESTE robot

### 3.1 Los tres pines por motor

Cada motor tiene **tres** pines: dos digitales de dirección (INA, INB) y uno de PWM (potencia).
Cita: los `#define` de `arquero.ino:38-48` (dentro del bloque `#if defined(ROBOT1)`, que abre en la
37) y el mapa `mapa-pines-teensy.md:46-58`.

| | INA | INB | PWM | Driver en la Zircon |
|---|---|---|---|---|
| Motor 1 (del. derecha) | 2 | 5 | 3 | U5 |
| Motor 2 (del. izquierda) | 8 | 7 | 6 | U17 |
| Motor 3 (trasera) | 11 | 12 | 4 | U7 |

(Eso es para el **arquero**, `#define ROBOT1`. En el delantero los pines son otros: §4. Es EL tema
de este robot.)

Y la regla, que se ve en `parar()` (`arquero.ino:146-150`):

| INA | INB | Qué hace el motor |
|---|---|---|
| 1 | 0 | gira en un sentido |
| 0 | 1 | gira en el sentido contrario |
| 0 | 0 | **libre** (coast): no frena, se deja girar |
| 1 | 1 | (no se usa en este código) |

**El signo del movimiento lo dan SIEMPRE los pines de dirección, nunca el PWM.** `analogWrite()`
solo acepta números positivos; un PWM de −50 no existe. Esta es la traducción entre "la fórmula
me dio `v1 = −169,8`" y "qué le escribo al Teensy".

`analogWrite()` en el Teensy 4.1 acepta **0 a 255** con la resolución por defecto de 8 bits.
Verificado: **no hay ninguna llamada a `analogWriteResolution()` ni a `analogWriteFrequency()`** en
ninguno de los dos `.ino`, así que corre todo con los valores por defecto del core de Teensy.

### 3.2 Las funciones reales, tal cual están en el robot

Estas son literales de `arquero.ino:140-155` (las mismas están en `delantero.ino:147-162`):

```cpp
void girar() {
  analogWrite(PWM1, 100 * g); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, 100 * g); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 100 * g); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
}

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void avanzar() {
  analogWrite(PWM1, 100); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, 100); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}
```

Tres cosas para no pasar de largo:

- **`g = 0.3`** (`arquero.ino:80`), así que `girar()` manda PWM = 30, no 100. Igual `a = 0.4`
  (`arquero.ino:81`) para apuntar, y `c = 0.4` / `ic = 0.5` (`arquero.ino:56-57`) para centrar.
  Los factores están arriba de todo del archivo y los valores adentro de las funciones: para saber
  qué PWM sale de verdad **hay que multiplicar los dos**.
- En `avanzar()` la trasera queda con `PWM3 = 0` **pero con dirección (1,0)**. Con PWM 0 no importa,
  pero es una inconsistencia de estilo respecto a `parar()`, que pone (0,0).
- La "patada" no es un solenoide: son **las ruedas a fondo**. En el arquero es `avanzar_patear()`
  (`arquero.ino:174-178`): PWM1 = `patadM1` = 250 y PWM2 = `patadM2` = 150 (`arquero.ino:54-55`),
  con la trasera en cero y en (0,0). Ojo que **las dos delanteras NO van al mismo PWM** (250 contra
  150): mirado con la suma con signo de §2.5, eso no es un avance puro — mientras patea, el robot
  también gira. Puede ser a propósito (compensar algo) o puede ser un ajuste hecho a ojo; **no
  está documentado en ningún lado y no se midió.**
- En el **delantero** la función se llama igual pero **es otra cosa** (`delantero.ino:181-201`):
  no usa `patadM1` ni `patadM2`, usa una rampa — sube `velocidadActualPateo` de a `pasoPateo = 5`
  cada `intervaloPateo = 20 ms` hasta `velocidadFinalPateo = 240` (`delantero.ino:69-73`) y le manda
  **el mismo número a las dos delanteras**. `patadM1`/`patadM2` en el delantero solo sobreviven en
  `retroceder_patear()` (`delantero.ino:204-208`). Y esa rampa tiene un bug conocido (DEL-04 en
  `bugs-conocidos.md`): la variable nunca vuelve a cero, así que la rampa suave ocurre **una sola
  vez en la vida del robot**.

### 3.3 Quién pone los pines en OUTPUT (y por qué importa)

Los `.ino` **no llaman a `pinMode()` para los motores** — el único `pinMode` que tienen es el del
LED (`arquero.ino:244`). Los pines de motor los configura `InitializeZircon()`
(`arquero.ino:237`), que adentro llama a `initializePins()` (`zirconLib.cpp:234-342`).

Y ahí hay una trampa que conviene conocer: la librería **autodetecta el modelo de placa leyendo el
pin 32** (`zirconLib.cpp:52-60`) — LOW da "Mark1", HIGH da "Naveen1". Como usa pull-down interno,
por defecto debería dar Mark1, que es el que coincide con los pines de `ROBOT1`. Pero si alguna vez
diera "Naveen1", la rama de `zirconLib.cpp:263-289` deja las tres variables de PWM **sin asignar
(valen 0)** — las líneas están comentadas — y después `zirconLib.cpp:322/325/328` hace
`pinMode(0, OUTPUT)` tres veces sobre el pin 0, que es **el RX de la cámara**. En la misma rama
`ballpin7` y `ballpin8` pasan a 18 y 19, que son **SDA y SCL del giroscopio**.
Test de 5 minutos que vale la pena hacer una sola vez: imprimir `getZirconVersion()` en el `setup()`
y confirmar que dice `Mark1`.

**Dato importante:** ninguno de los dos `.ino` usa `motor1()`, `motor2()` ni `motor3()` de la
librería (verificado: cero llamadas). Escriben `analogWrite`/`digitalWrite` directo. Por eso el tope
`motorLimit = 100` de `zirconLib.cpp:9` **no aplica**, y por eso la patada puede mandar PWM 250.

### 3.4 Una capa de motor con signo (propuesto, sin validar)

Si en algún momento quieren pasar de las tablas a la cinemática, el primer ladrillo es una función
que acepte **PWM con signo** y se ocupe sola de los pines. Es un cambio chico y de bajo riesgo
porque no altera ningún comportamiento por sí solo:

```cpp
// PWM con signo: >0 un sentido, <0 el otro, 0 = libre (coast, igual que parar()).
// La correspondencia "positivo = tal sentido físico" NO está verificada:
// hay que confirmarla rueda por rueda en banco (ver 5.2) antes de confiar en ella.
void rueda(int pinINA, int pinINB, int pinPWM, int pwm) {
  if (pwm > 0)      { digitalWrite(pinINA, 1); digitalWrite(pinINB, 0); }
  else if (pwm < 0) { digitalWrite(pinINA, 0); digitalWrite(pinINB, 1); }
  else              { digitalWrite(pinINA, 0); digitalWrite(pinINB, 0); }

  int magnitud = abs(pwm);
  if (magnitud > 255) magnitud = 255;     // analogWrite no acepta mas de 255
  analogWrite(pinPWM, magnitud);
}

void rueda1(int pwm) { rueda(INA1, INB1, PWM1, pwm); }   // delantera derecha
void rueda2(int pwm) { rueda(INA2, INB2, PWM2, pwm); }   // delantera izquierda
void rueda3(int pwm) { rueda(INA3, INB3, PWM3, pwm); }   // trasera
```

Con eso, `avanzar()` se reescribe como `rueda1(100); rueda2(-100); rueda3(0);` — mismo efecto, pero
ahora los signos son leíbles y coinciden con la fórmula. **Propuesto, falta validar en banco.**

---

## 4. El tema más importante de ESTE robot: `ROBOT1` vs `ROBOT2`

### 4.1 Qué pasa

Los dos robots comparten la misma placa Zircon y el mismo Teensy, **pero los motores están
soldados a drivers distintos**. Lo que en el arquero es "el motor de adelante a la derecha" en el
delantero cuelga de otro chip driver.

La solución del equipo 2025 fue resolverlo en **tiempo de compilación**: arriba de todo del archivo
hay dos `#define` y se descomenta uno solo.

```cpp
// ELEGI 1
#define ROBOT1
//#define ROBOT2
```
(`arquero.ino:9-11`; en `delantero.ino:9-11` está al revés: `ROBOT2` activo.)

Según cuál esté activo, se compila un bloque de `#define` u otro (`arquero.ino:13-59`). Traducido:

| Rol físico de la rueda | Pines INA/INB/PWM en el **ARQUERO** (`ROBOT1`) | Pines INA/INB/PWM en el **DELANTERO** (`ROBOT2`) | Driver de la Zircon |
|---|---|---|---|
| Delantera **derecha** (motor 1 del código) | 2 / 5 / 3 | 8 / 7 / 6 | U5 en R1, U17 en R2 |
| Delantera **izquierda** (motor 2 del código) | 8 / 7 / 6 | 11 / 12 / 4 | U17 en R1, U7 en R2 |
| **Trasera** (motor 3 del código) | 11 / 12 / 4 | 2 / 5 / 3 | U7 en R1, U5 en R2 |

Mirado desde el driver, que es como lo cuenta `mapa-pines-teensy.md:83-87`:

| Driver (pines) | En el arquero es… | En el delantero es… |
|---|---|---|
| U5 (2/5/3) | Motor 1 | Motor 3 |
| U17 (8/7/6) | Motor 2 | Motor 1 |
| U7 (11/12/4) | Motor 3 | Motor 2 |

Es una **rotación cíclica**: no hay dos motores intercambiados, están los tres corridos un lugar.

### 4.2 Por qué cargar el define equivocado te hace creer que rompiste el robot

Supongamos que agarran el archivo del arquero (`ROBOT1`) y lo cargan en el **delantero**.

El programa manda el comando de "delantera derecha" a los pines 2/5/3. En el delantero esos pines
son el driver U5, que mueve **la rueda trasera**. Y así con las tres:

- lo que el programa cree que es la **delantera derecha** → mueve la **trasera**
- lo que cree que es la **delantera izquierda** → mueve la **delantera derecha**
- lo que cree que es la **trasera** → mueve la **delantera izquierda**

Ahora corré mentalmente `avanzar()`: manda 100 hacia adelante a la "delantera derecha", 100 al
revés a la "delantera izquierda", y 0 a la "trasera". Con la rotación, eso llega como: 100 a la
**trasera**, 100 al revés a la **delantera derecha**, y **0 a la delantera izquierda**. El robot
no avanza: sale en diagonal girando. Y `girar()`, que manda lo mismo a las tres, **sale igual** —
porque el giro no distingue ruedas: da lo mismo cuál es cuál.

Ese es exactamente el cuadro que te hace pensar que rompiste el hardware:

> "Gira bien, pero cuando quiere avanzar sale para cualquier lado. Debe estar quemado un motor."

No hay nada quemado. Es un `#define`.

### 4.3 Chequeo de 30 segundos, antes de tocar un destornillador

1. Abrí el `.ino` que estás por cargar. Mirá las líneas 9-11: ¿cuál `#define` está sin comentar?
2. ¿Ese archivo va al robot que corresponde? `arquero.ino` → `ROBOT1`. `delantero.ino` → `ROBOT2`.
3. Si dudás, agregá esto al `setup()` y leelo por el monitor serie antes de bajar el robot a la
   cancha:

```cpp
#if defined(ROBOT1)
  Serial.println("Compilado para ROBOT1 (ARQUERO): M1=2/5/3  M2=8/7/6  M3=11/12/4");
#elif defined(ROBOT2)
  Serial.println("Compilado para ROBOT2 (DELANTERO): M1=8/7/6  M2=11/12/4  M3=2/5/3");
#else
  Serial.println("PELIGRO: no hay ningun ROBOT definido");
#endif
```

Costo: 6 líneas. Ahorro: una tarde de desarmar el robot buscando un motor que no tiene nada roto.

> **Aviso honesto sobre esta sección:** la tabla de pines está verificada en el código y en el mapa
> de pines. Lo que **no** está verificado en banco es qué rueda física cuelga de cada driver en cada
> robot; eso lo afirma `mapa-pines-teensy.md:78-87` y lo confirma el hecho de que el mismo
> `avanzar()` sirva para los dos, pero **hasta que no lo prueben rueda por rueda (§5.2) es lectura
> de documentación, no medición.**

---

## 5. Signos y ejes: el error número 1, y cómo se caza

En un omni, el 90% de los "se mueve mal" no es la fórmula: es una **convención de signo**. Y hay
una regla que ahorra días:

> **La traslación y la rotación se rompen por separado, y se arreglan por separado.** Un robot
> puede trasladar perfecto y girar al revés. Arreglar uno no arregla el otro. Probalos SIEMPRE
> por separado.

### 5.1 Las tres capas donde puede estar el problema

1. **La fórmula.** Es aritmética cerrada, casi nunca está mal.
2. **Las convenciones.** Qué es +X, qué giro es positivo, en qué marco están los ángulos, y para
   qué lado gira cada rueda cuando le mandás (INA=1, INB=0). **Acá viven casi todos los bugs.**
3. **La planta física.** Cuánto PWM hace falta para que la rueda efectivamente gire (§6). Eso no
   es cinemática, es electromecánica.

### 5.2 Prueba rueda por rueda (es LA prueba, háganla antes que cualquier otra cosa)

Robot **sobre un soporte, con las ruedas en el aire**. Este sketch corto, cargado solo (no el
programa completo).

> **Por qué NO usa `zirconLib`:** hoy la librería del repo **no compila** (llave de más en
> `zirconLib.cpp:355`), y además define su propia variable `bno` (`zirconLib.cpp:4`), que choca con
> la del programa. Ver `bugs-conocidos.md`. Este sketch se pone los pines en OUTPUT **él solo**, con
> `pinMode()`, así que no depende de nada roto y se puede cargar hoy mismo. Es una sola línea por
> pin y hace exactamente lo mismo que `initializePins()` (`zirconLib.cpp:320-328`) para la rama
> Mark1.

```cpp
#include <Arduino.h>

#define ROBOT1              // <-- OJO: el que corresponda al robot que tenes en la mano

#if defined(ROBOT1)
  #define INA1 2
  #define INB1 5
  #define PWM1 3
  #define INA2 8
  #define INB2 7
  #define PWM2 6
  #define INA3 11
  #define INB3 12
  #define PWM3 4
#else                        // ROBOT2 = delantero
  #define INA1 8
  #define INB1 7
  #define PWM1 6
  #define INA2 11
  #define INB2 12
  #define PWM2 4
  #define INA3 2
  #define INB3 5
  #define PWM3 3
#endif

void unaRueda(int ina, int inb, int pwmPin, int pwm, bool sentidoPositivo) {
  digitalWrite(ina, sentidoPositivo ? 1 : 0);
  digitalWrite(inb, sentidoPositivo ? 0 : 1);
  analogWrite(pwmPin, pwm);
}

void todoQuieto() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void pinesDeMotorEnSalida() {
  int pines[9] = { INA1, INB1, PWM1, INA2, INB2, PWM2, INA3, INB3, PWM3 };
  for (int k = 0; k < 9; k++) pinMode(pines[k], OUTPUT);
}

void setup() {
  pinesDeMotorEnSalida();
  Serial.begin(19200);
  todoQuieto();
  delay(2000);
}

void loop() {
  Serial.println("M1 sentido POSITIVO (INA=1, INB=0)");
  unaRueda(INA1, INB1, PWM1, 120, true);  delay(1500); todoQuieto(); delay(1500);
  Serial.println("M1 sentido NEGATIVO (INA=0, INB=1)");
  unaRueda(INA1, INB1, PWM1, 120, false); delay(1500); todoQuieto(); delay(1500);

  Serial.println("M2 sentido POSITIVO");
  unaRueda(INA2, INB2, PWM2, 120, true);  delay(1500); todoQuieto(); delay(1500);
  Serial.println("M2 sentido NEGATIVO");
  unaRueda(INA2, INB2, PWM2, 120, false); delay(1500); todoQuieto(); delay(1500);

  Serial.println("M3 sentido POSITIVO");
  unaRueda(INA3, INB3, PWM3, 120, true);  delay(1500); todoQuieto(); delay(1500);
  Serial.println("M3 sentido NEGATIVO");
  unaRueda(INA3, INB3, PWM3, 120, false); delay(1500); todoQuieto(); delay(1500);
}
```

Con esto anotan **una tabla en papel** (sí, en papel, y se pega adentro del robot):

| Rueda | ¿Qué motor físico se movió? | Con (INA=1, INB=0) el borde de arriba de la rueda va hacia… |
|---|---|---|
| M1 | | |
| M2 | | |
| M3 | | |

Esa tabla es la **única** fuente de verdad de los signos. Todo lo demás es teoría.

### 5.3 Árbol de diagnóstico: "el robot se mueve mal"

Antes de entrar: si **ninguna** rueda gira, no es cinemática — es batería, driver, o pines sin
OUTPUT. Volvé a §3.3.

- **Nivel 0 — ¿qué falla, trasladar o girar?** Probalos separados: `avanzar()` solo, después
  `girar()` solo, después un movimiento lateral solo. Si `girar()` sale bien y avanzar no, ya sabés que
  el problema está en los ángulos/signos de traslación, no en el giro.

- **Nivel 1 — "gira en vez de ir derecho" (le pido traslación y me da vueltas).** Es el síntoma de
  ruedas cambiadas de lugar. Sospechoso número uno: **el `#define` equivocado** (§4). Sospechoso
  dos: la asignación de qué rueda es la trasera. Chequeo: cuando pedís avanzar, **la trasera tiene
  que quedar en cero**. Si la trasera se mueve al avanzar, la trasera no es la que vos creés.

- **Nivel 2 — "traslada, pero para el lado contrario".** Es un signo global de traslación. Los tres
  motores están invertidos respecto de lo que la fórmula asume (por ejemplo, están todos montados
  espejados). Se arregla en **un solo lugar**: invertir el signo de las tres ruedas, o sumarle 180°
  a los tres ángulos. **No toca el giro.**

- **Nivel 3 — "el giro sale al revés".** Es el signo de ω, que es una capa **independiente** de la
  anterior: el término `ω·R` no depende del ángulo, así que invertir la traslación no lo arregla.
  Se corrige aparte, con un signo propio para ω.

- **Nivel 4 — "una sola rueda va para el lado contrario".** Cables del motor invertidos, o la
  fila de esa rueda con el signo cambiado. Se detecta de una con la prueba de §5.2.

- **Nivel 5 — "va derecho pero se va yendo de rumbo".** Eso no es un bug de signos: es **yaw
  parásito** (§7). Es normal en un omni, y es lo que el código del arquero intenta compensar.

- **Nivel 6 — "anda rápido pero lento no arranca / raspa / zumba".** Eso es **piso de PWM** (§6).
  No es cinemática.

- **Nivel 7 — puerta de salida.** Confirmá **a mano, en banco, sin ningún lazo cerrado** los tres
  movimientos puros (adelante, de costado, girar) antes de cerrar cualquier control sobre el
  giroscopio. Un lazo cerrado sobre un signo invertido no oscila: **diverge**, se va derecho hasta
  el tope y parece "mal tuneado" cuando en realidad ninguna ganancia lo va a arreglar.

---

## 6. El piso de PWM

### 6.1 Qué es

Un motor de corriente continua con caja reductora no arranca proporcionalmente. Por debajo de
cierto PWM, el par que genera no alcanza para vencer la **fricción estática** del propio motor, de
la caja y de la rueda contra el piso. Entonces, la forma de la curva es esta (**los números son
inventados para explicar la forma — los de ESTE robot no los sabe nadie todavía, se miden en §6.3**):

- PWM 0 → quieto.
- PWM chico → **quieto igual**, pero el motor está consumiendo corriente y **zumba** (ese zumbido es
  literalmente la frecuencia del PWM haciendo vibrar el bobinado).
- PWM un poco más grande → arranca **de golpe**, y no despacito: salta directo a una velocidad que
  ya es bastante.

Ese umbral es el **piso de PWM**, y tiene tres consecuencias que ordenan todo el diseño de control:

1. **El robot no sabe moverse despacio.** Hay una velocidad mínima física, y abajo de eso no hay
   nada. No existe "corregí 2 grados": o corrige un montón, o no corrige.
2. **El piso es distinto para cada rueda.** Cada motor tiene su propia fricción, su propia caja,
   su propio driver. Y la rueda trasera, en un movimiento lateral, va **alineada** con el
   movimiento mientras las delanteras van en diagonal: ven fricciones diferentes.
3. **Es distinto para cada robot, y cambia con la batería.** Con la batería cargada el piso baja;
   con la batería a medias sube.

Por eso el número `89` de la tabla de strafe no es el `100` que pide la fórmula (§2.3): entre
"velocidad que pide la matriz" y "PWM que hay que mandar" hay una relación que **no es lineal y no
es la misma para las tres ruedas**.

### 6.2 Este robot NO tiene compensación de zona muerta

Verificado leyendo las tres capas (`.ino`, `zirconLib`, y las funciones de movimiento): **no hay
ninguna línea que eleve un PWM chico hasta un piso.** Los valores son fijos por tabla, ajustados a
mano en cancha en 2025.

Pero el código **sí tiene las cicatrices del problema**, que es la mejor evidencia de que existe:

| Dónde | Qué hace | Qué está compensando |
|---|---|---|
| `arquero.ino:1016-1028` (`impulso_inicial`) | primer estado del arquero: manda `1.8*50` = **90** a las dos delanteras y `1.8*85` = **153** a la trasera durante **40 ms**, y recién después pasa a `moverce_derecha` | un **kickstart**: el golpe fuerte para despegar de quieto, porque con el PWM de patrulla (50/50/89) no arranca. Ojo al detalle: la base de la trasera acá es **85**, no el 89 de la tabla de patrulla — nadie dejó escrito por qué |
| `arquero.ino:1126-1148` (`impulso_derecha` / `impulso_izquierda`) | al rebotar en la línea, fuerza **350 ms** de movimiento hacia el otro lado antes de volver a decidir | el comentario del propio código (`arquero.ino:1127`) lo dice: en los costados "se traba con el blanco, porque cambia de moverce izquierda a moverce derecha erraticamente" |
| `arquero.ino:368-372` (`IMPULSO_INICIAL_GIRANDO`) | manda PWM **150** a las tres durante 50 ms (`arquero.ino:374`) antes de pasar a `girar()`, que usa PWM 30 | mismo mecanismo: 30 no alcanza para arrancar de quieto, 150 sí. **Aviso: este estado es del DELANTERO.** Con `ROBOT1` compilado es código inalcanzable; sirve como evidencia de que el equipo se topó con el problema, no como algo que el arquero ejecute |
| `arquero.ino:1047` y `1095` (`pd = 1.5`) | cuando el arquero **ve la pelota** y la tiene desviada (`abs(Yp) >= 5`), multiplica por 1,5 **todos** los PWM de `ai/adproporcional`. Vuelve a `pd = 1` cuando no ve pelota (`1065`, `1113`) | subir de golpe la autoridad, porque no hay forma de pedir "un poco más" |

Los cuatro son la misma idea vestida distinta: **"para arrancar hay que pegar más fuerte que para
seguir"**. Eso es un piso de PWM.

### 6.3 Cómo se MIDE (procedimiento concreto, 20 minutos)

Hay dos pisos distintos y no son el mismo número. Midan los dos:

- **Piso de arranque** (el que importa): el PWM mínimo que hace que la rueda **empiece** a girar
  desde quieto.
- **Piso de sostén**: una vez girando, el PWM mínimo al que **sigue** girando. Siempre es más bajo.

**Setup:** robot **apoyado en la cancha** (no en el aire: la fricción con el piso es parte de lo
que estás midiendo), batería **cargada**, y anotá el voltaje. Repetí con la batería a medias.

```cpp
#include <Arduino.h>

#define ROBOT1              // <-- el que corresponda al robot que tenes en la mano

#if defined(ROBOT1)
  #define INA1 2
  #define INB1 5
  #define PWM1 3
  #define INA2 8
  #define INB2 7
  #define PWM2 6
  #define INA3 11
  #define INB3 12
  #define PWM3 4
#else                        // ROBOT2 = delantero
  #define INA1 8
  #define INB1 7
  #define PWM1 6
  #define INA2 11
  #define INB2 12
  #define PWM2 4
  #define INA3 2
  #define INB3 5
  #define PWM3 3
#endif

// Cambiar estos tres a mano para probar cada rueda:
#define INA_TEST INA3
#define INB_TEST INB3
#define PWM_TEST PWM3

void setup() {
  int pines[9] = { INA1, INB1, PWM1, INA2, INB2, PWM2, INA3, INB3, PWM3 };
  for (int k = 0; k < 9; k++) pinMode(pines[k], OUTPUT);
  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }
  Serial.println("Rampa: PWM sube de 5 en 5, 800 ms cada escalon.");
  Serial.println("Anota el PWM al que la rueda ARRANCA de verdad.");
  digitalWrite(INA_TEST, 1);
  digitalWrite(INB_TEST, 0);
}

void loop() {
  for (int pwm = 0; pwm <= 150; pwm += 5) {
    analogWrite(PWM_TEST, pwm);
    Serial.print("PWM = "); Serial.println(pwm);
    delay(800);                       // tiempo para mirar la rueda y anotar
  }
  analogWrite(PWM_TEST, 0);
  Serial.println("--- fin de la rampa, 5 s de pausa ---");
  delay(5000);
}
```

Para el **piso de sostén**, la misma rampa **al revés**: arrancá en 150 y bajá de 5 en 5 hasta que
se pare.

**Ojo con lo práctico:** con una sola rueda empujando, el robot **se va a mover** (mal, de costado,
arrastrando las otras dos). Dejen espacio libre alrededor y que alguien esté listo para levantarlo.
Y no lo sujeten con la mano apretando: si le agregan fricción, están midiendo otra cosa.

**Planilla (una por robot):**

| Rueda | Piso de arranque, batería llena | Piso de sostén, batería llena | Piso de arranque, batería a medias |
|---|---|---|---|
| M1 (del. derecha) | | | |
| M2 (del. izquierda) | | | |
| M3 (trasera) | | | |

**Criterio de "arrancó":** la rueda da **una vuelta completa** sin ayuda. Que se mueva un poquito
y se pare no cuenta.

> **Los números hay que medirlos en ESTE robot.** No los copien de ningún lado, y menos del robot
> de Incheon: ese es otro robot, con otra electrónica, otros motores, otras ruedas y otro peso.
> Sus pisos medidos no dicen **nada** sobre estos. Lo que sí se transfiere es el **concepto** y el
> **procedimiento**. Además: hay **dos** robots acá, y muy probablemente den distinto entre sí.

### 6.4 Qué se hace con el número, una vez medido

Con el piso medido, la capa de motor puede **elevar** cualquier PWM chico hasta el piso, en vez de
mandar un valor que solo hace zumbar:

```cpp
// PISO[i] va con los valores MEDIDOS de ESTE robot (ver 6.3). En cero hasta que se midan:
// con PISO en cero esta funcion no cambia nada, que es el default seguro.
const int PISO[3] = { 0, 0, 0 };
// RUIDO tambien se elige DESPUES de medir: es "que tan chico es tan chico que no vale la pena".
// El 3 de aca es un valor de arranque inventado para el ejemplo, no medido.
const int RUIDO   = 3;     // por debajo de esto mandamos 0 directamente: no zumbes al pedo

int aplicarPiso(int pwm, int piso) {
  int mag = abs(pwm);
  if (mag <= RUIDO) return 0;               // practicamente cero -> cero de verdad
  if (mag < piso)   mag = piso;             // demasiado chico para arrancar -> subilo al piso
  if (mag > 255)    mag = 255;
  return (pwm < 0) ? -mag : mag;
}
```

Y el otro uso, más importante: **si tu corrección de rumbo es más chica que el piso, el robot no
puede hacerla.** Ahí no sirve subir la ganancia (te vas a pasar): la salida hay que **pulsarla** —
mandar la corrección al mínimo físico durante una fracción del tiempo, y cero el resto. Ese tema
completo excede esta skill; acá alcanza con saber **por qué** un control fino no puede funcionar
mientras no sepan cuál es el piso.

---

## 7. Yaw parásito: por qué el robot gira solo yendo de costado

### 7.1 El mecanismo

En la teoría, un strafe puro (`vx ≠ 0`, `vy = 0`, `ω = 0`) tiene giro exactamente cero. En la
realidad, no. El robot se va girando solo mientras se mueve de costado. Las razones se acumulan:

- **Las ruedas no rinden igual.** Si la trasera debería ir al doble de las delanteras y va al 1,9,
  las velocidades no se cancelan y queda un resto de rotación (acordate de §2.5: si la suma con
  signo no da cero, hay giro).
- **Los pisos de PWM cuantizan.** A velocidad baja, las ruedas se aplastan contra sus pisos y las
  proporciones que pedía la fórmula se pierden.
- **Los rodillos no son simétricos.** Cada rueda omni tiene rodillos discretos; según en qué rodillo
  esté apoyada en ese instante, el punto de contacto y la fricción cambian. Genera un torque chico
  y variable.
- **El peso no está centrado** y la cancha tiene alfombra que no es igual en todas partes.

Todo eso junto da un torque neto que nadie pidió. Y encima **cambia de signo** según para qué lado
estés yendo: yendo a la derecha te gira para un lado, yendo a la izquierda para el otro.

**No es un bug que se arregla: es la planta.** Se compensa, no se elimina.

### 7.2 Cómo lo compensa HOY el arquero (y por qué el nombre miente)

El arquero patrulla llamando a `adproporcional()` desde el estado `moverce_derecha`
(`arquero.ino:1031`) o a `aiproporcional()` desde `moverce_izquierda` (`arquero.ino:1079`). Las
funciones están en `arquero.ino:187-209` y `arquero.ino:211-233`.

(El "derecha"/"izquierda" sale del **nombre** que le puso el equipo 2025 al estado y a la función.
Que el robot efectivamente vaya para ese lado **no está verificado**: se confirma con §5.2 y
mirando el robot.)

Lo primero que hay que decir con todas las letras:

> **Se llaman "proporcional" y no son proporcionales.** No hay ninguna multiplicación por el error.
> Son **tres juegos fijos de PWM**, y el giroscopio solo elige **cuál de los tres** se usa. En
> control se llama a esto *bang-bang de 3 niveles*, no control proporcional.

El error de rumbo viene de `arquero.ino:337-339`: `error = currentYaw - initialYaw`, normalizado a
±180°. Y la selección es literalmente esta (`arquero.ino:188, 193, 201`):

```cpp
if (error > -1 && error < 1) { ... }   // banda muerta de +-1 grado
else if (error > 0)          { ... }
else if (error < 0)          { ... }
```

Las tablas de abajo son con **`pd = 1`**, que es el valor inicial (`arquero.ino:87`). Cuando el
arquero ve la pelota desviada, `pd` pasa a **1,5** (`arquero.ino:1047`) y **todos** estos números se
multiplican por 1,5. Los `//60 //99 //120` que aparecen comentados al lado de cada `analogWrite`
**parecen** una versión anterior de la tabla que quedó como comentario; el compilador no los ve.

**`aiproporcional()` — yendo a la izquierda** (delanteras con dir (0,1), trasera con dir (1,0)):

| Rama | M1 (del. der.) | M2 (del. izq.) | M3 (trasera) | Líneas |
|---|---|---|---|---|
| banda muerta, \|error\| < 1° | 50 | 50 | **89** | 188-192 |
| error > 0 | 50 | 50 | **40** | 193-200 |
| error < 0 | **40** | **65** | **100** | 201-208 |

**`adproporcional()` — yendo a la derecha** (delanteras con dir (1,0), trasera con dir (0,1)):

| Rama | M1 (del. der.) | M2 (del. izq.) | M3 (trasera) | Líneas |
|---|---|---|---|---|
| banda muerta, \|error\| < 1° | 50 | 50 | **89** | 212-216 |
| error > 0 | 50 | 50 | **100** | 217-224 |
| error < 0 | **40** | **65** | **40** | 225-232 |

Las dos funciones son **espejo exacto** una de la otra: mismas magnitudes, todas las direcciones
invertidas. Eso está bien pensado, porque el yaw parásito también cambia de signo al cambiar de
lado.

### 7.3 Qué palanca están moviendo (esto es lo que hay que entender)

Hay dos palancas y las usan las dos:

**Palanca 1 — la trasera.** Acordate de §2.5: `ω·R = (v1+v2+v3)/3`. Si movés el PWM de la trasera
sin tocar las delanteras, la suma cambia y aparece rotación. La trasera es la rueda **más
apalancada** para meter o sacar giro en un strafe, porque es la que va alineada con el movimiento.

**Palanca 2 — desbalancear las delanteras.** En la rama `error < 0` las delanteras dejan de ser
50/50 y pasan a 40/65. Como las dos delanteras aportan a `vy` con **signos opuestos**, hacerlas
distintas también rota el robot.

Poniéndoles número (usando la suma con signo de §2.5, con la convención "dirección (1,0) = positivo"):

| Función / rama | Suma con signo (v1+v2+v3) | Lectura |
|---|---|---|
| `aiproporcional` banda muerta | −11 | casi puro strafe, con un sesgo chico de giro |
| `aiproporcional` error > 0 | **−60** | corrección fuerte hacia un lado |
| `aiproporcional` error < 0 | −5 | prácticamente sin corrección |
| `adproporcional` banda muerta | +11 | espejo del anterior |
| `adproporcional` error > 0 | 0 | prácticamente sin corrección |
| `adproporcional` error < 0 | **+65** | corrección fuerte, hacia el otro lado |

Y acá aparece lo interesante, que se ve en la cuenta y no en el nombre de la función:

- Dentro de `aiproporcional` las tres sumas son **todas negativas o cero**. Dentro de
  `adproporcional`, **todas positivas o cero**. Es decir: **en un mismo estado, el robot solo puede
  corregir el rumbo hacia UN lado**; el error solo elige *cuánto*, nunca *hacia dónde*.
- Eso es coherente con lo que dijimos en §7.1: yendo a la izquierda el yaw parásito siempre tira
  para el mismo lado, así que la corrección también. **Está armado como compensación (feedforward)
  de una deriva conocida, no como realimentación.** Si la deriva alguna vez sale para el otro lado,
  este esquema no la puede corregir; tiene que esperar a que el robot rebote en la línea y cambie
  de estado.
- La rama `error < 0` también mete un `v1 − v2 ≠ 0`, o sea un **componente de avance/retroceso** que
  nadie pidió: mientras corrige el rumbo, el robot también se va un poco para adelante o para atrás.

> **Cuidado con esta tabla de sumas:** trata el PWM como si fuera velocidad, y **no lo es** (§6).
> Sirve para leer **signos y tendencias**, no para predecir grados por segundo. Es aritmética sobre
> los valores del código bajo el modelo de §2.3: **derivado, falta validar en banco.**

### 7.4 Dos cosas más del código que hay que saber

- **`correccion = error * kp` con `kp = 0.3`** (`arquero.ino:340` y `arquero.ino:73`) se calcula en
  **cada** vuelta del loop… y **nunca se escribe a ningún motor**. Verificado: `correccion` aparece
  exactamente dos veces en todo el archivo, la declaración (`arquero.ino:69`) y la asignación
  (`arquero.ino:340`). Lo mismo en el delantero (`delantero.ino:77` y `365`). Hay un control
  proporcional a medio escribir que quedó ahí. No lo borren sin entender que hoy no hace nada.
- **La banda muerta es de ±1°** en el arquero (`arquero.ino:188`) y de ±2° en el delantero
  (`delantero.ino:212`, aunque en el delantero esta función es código inalcanzable). ±1° es
  **muy angosto** para un BNO055 con ruido: es probable que la rama central casi nunca se ejecute
  y el sistema esté saltando entre las otras dos todo el tiempo. **Falta confirmarlo midiendo** —
  imprimir `error` por serie durante una patrulla y ver cuánto tiempo pasa dentro de ±1.

---

## 8. Cómo sería mejorarlo de verdad (y por qué NO es lo primero que hay que hacer)

### 8.1 El orden importa más que la mejora

Un control proporcional de verdad sobre el rumbo es la mejora obvia. **Pero no se puede hacer
todavía**, y el motivo no es técnico-elegante, es de sentido común: un controlador es una máquina
de perseguir un número. Si el número está mal, el controlador persigue mal, más rápido y con más
autoridad que antes.

El orden que sí cierra:

1. **Que compile.** Hoy los programas **no compilan**: hay una llave `}` de más al final de
   `zirconLib.cpp:355`, y la variable `bno` está definida dos veces — en `zirconLib.cpp:4` y otra
   vez en cada programa (`arquero.ino:68`, `delantero.ino:76`). El segundo error queda tapado por
   el primero. Está documentado en `bugs-conocidos.md`, con parches en `correcciones-propuestas.md`.
   Sin esto no hay nada que discutir.
2. **Que el rumbo signifique algo.** `error` se mide contra `initialYaw`, que se lee en el `setup()`
   **inmediatamente** después de `bno.begin()` (`arquero.ino:246-255`), cuando el BNO055 todavía
   puede estar devolviendo 0. Si `initialYaw` vale 0, `error` deja de significar "cuánto me desvié
   de como arranqué" y pasa a significar "para dónde está el norte magnético" — y en el arquero ese
   `error` **mueve las ruedas** (elige la rama de la tabla). Es el clásico "hoy anda y mañana no".
   **Eso va en la skill del giroscopio BNO055**, no acá.
3. **Medir la planta, a lazo abierto:** el piso de PWM de cada rueda (§6.3) y **cuánto se va de
   rumbo el robot solo**, sin ninguna corrección, en 2 segundos de strafe. Esos dos números
   dimensionan todo el control que venga después. Sin ellos, cualquier ganancia es adivinar.
4. **Recién ahí**, un control proporcional.

Y no es tarea trivial: es reemplazar la única parte del robot que **ganó un Nacional**. Tiene que
haber un plan de vuelta atrás (el archivo viejo guardado, sin excusas) y una prueba de banco que
compare las dos versiones en la misma tarde, con la misma batería.

### 8.2 Cómo se vería, si llegan hasta ahí

Idea: en vez de tres tablas fijas, **una** tabla de strafe puro más un **trim proporcional** que se
suma o se resta. Con la trasera como palanca principal, que es lo que el código ya venía haciendo a
mano:

```cpp
// PROPUESTO. No probado en el robot. Requiere haber hecho (1) (2) y (3) de 8.1.
// OJO: los unicos dos numeros de aca que salen del robot son BASE_FRENTE y BASE_TRASERA.
// KP_RUMBO, BANDA_MUERTA y TRIM_MAX son INVENTADOS para que el ejemplo compile.
// No los copien: se sintonizan en banco, con este robot y esta bateria.
const int   BASE_FRENTE = 50;    // de la tabla actual (arquero.ino:189-190)
const int   BASE_TRASERA = 89;   // de la tabla actual (arquero.ino:191)
const float KP_RUMBO   = 3.0;    // INVENTADO. A SINTONIZAR EN BANCO. Arrancar bajo y subir de a poco.
const float BANDA_MUERTA = 3.0;  // INVENTADO. Grados. Mas ancha que el +-1 actual: el BNO tiene ruido.
const int   TRIM_MAX   = 25;     // INVENTADO. Tope de la correccion, en PWM.

int trimDeRumbo(float error) {
  if (fabs(error) < BANDA_MUERTA) return 0;      // no persigas ruido
  float t = KP_RUMBO * error;
  if (t >  TRIM_MAX) t =  TRIM_MAX;              // saturacion simetrica
  if (t < -TRIM_MAX) t = -TRIM_MAX;
  return (int)t;
}

// sentido = +1 para strafe a la derecha, -1 para strafe a la izquierda
void strafeConRumbo(int sentido, float error) {
  int trim = trimDeRumbo(error) * sentido;       // el yaw parasito cambia de signo con el sentido

  int pwm1 =  BASE_FRENTE  * sentido;            // delantera derecha
  int pwm2 =  BASE_FRENTE  * sentido;            // delantera izquierda
  int pwm3 = -BASE_TRASERA * sentido + trim;     // trasera: base + correccion

  rueda1(aplicarPiso(pwm1, PISO[0]));            // rueda1/2/3 de 3.4, aplicarPiso de 6.4
  rueda2(aplicarPiso(pwm2, PISO[1]));
  rueda3(aplicarPiso(pwm3, PISO[2]));
}
```

**Lo que este código NO resuelve, y hay que decirlo:**

- Si `trim` sale más chico que el piso de PWM de la trasera, **no hace nada**. La corrección fina
  contra un actuador con zona muerta necesita pulsar la salida, no bajarle la amplitud.
- `KP_RUMBO = 3.0` es un valor de arranque **inventado para el ejemplo**, no medido en este robot.
  Se sintoniza con el robot andando: si serpentea, se ensancha la banda muerta o se baja la
  ganancia; si se va de rumbo sin corregir, se sube.
- Si el signo del trim está al revés, el robot **no oscila: se va derecho hasta el tope** y parece
  mal tuneado. Antes de tocar ninguna ganancia, verificar el signo con el robot quieto: forzalo a
  20° de error a mano y mirá para dónde intenta corregir.
- Un `P` puro no le gana a una deriva **sistemática** (siempre para el mismo lado): siempre le va a
  quedar un error residual. Ahí entra un término integral, que es otra conversación y necesita
  anti-windup.

**Todo esto es propuesto. Falta validar en banco.**

---

## 9. Lo que este robot NO tiene (para que nadie lo suponga)

Cuando busquen en internet, o le pregunten a una IA, van a aparecer cosas que **acá no existen**.
Que quede claro de una vez:

| Lo que van a leer por ahí | En este robot |
|---|---|
| Encoders en las ruedas | **No hay.** El robot no sabe cuánto giró una rueda. Todo es a lazo abierto y por tiempo (`millis()`). |
| Odometría / saber la posición en la cancha | **No hay.** No hay forma de saber dónde está, más allá de "vi una línea blanca". |
| Sensores ToF, ultrasonido, sensores de distancia | **No hay.** |
| Segunda cámara, cámaras traseras | **No hay.** Una sola OpenMV H7 en Serial1. |
| Solenoide / kicker de verdad | **No hay.** La patada es `avanzar_patear()`: las ruedas a fondo. En el arquero, PWM fijo (`arquero.ino:174-178`); en el delantero, una rampa (`delantero.ino:181-201`). |
| PlatformIO, `platformio.ini`, entornos de compilación | **No.** Es Arduino IDE con archivos `.ino`, y se carga por USB. |
| FreeRTOS, micro-ROS, tareas, prioridades | **No.** Es un `loop()` corrido, un solo hilo. |
| Varias placas (CENTRAL / TOP / DOWN) | **No.** Un solo Teensy 4.1 sobre una sola Zircon. |
| Pisos de PWM ya medidos, ganancias ya validadas | **No existen para este robot.** Los que aparecen en documentos del robot de Incheon son de **otra** máquina: otra electrónica, otros motores, otro peso. Hay que medirlos acá (§6.3). |

Si un día tienen encoders, la fórmula de §2.2 se puede **invertir** para calcular a qué velocidad
se está moviendo el robot a partir de lo que giraron las ruedas (cinemática directa, la base de la
odometría). Hoy no se puede: **no hay de dónde sacar el dato.**

---

## 10. Tabla de síntomas

| Síntoma | Causa más probable | Cómo se confirma | Dónde |
|---|---|---|---|
| Gira bien pero avanzar sale para cualquier lado | `#define` del robot equivocado | mirar las líneas 9-11 del `.ino` cargado | §4 |
| Le pido ir de costado y da vueltas | ruedas mal asignadas (qué es la trasera) | al avanzar, la trasera **tiene** que quedar en 0 | §5.3 nivel 1 |
| Traslada al revés (izq. ↔ der.) | signo global de traslación | invertir las tres, o +180° a los tres ángulos | §5.3 nivel 2 |
| El giro sale al revés pero traslada bien | signo de ω (capa independiente) | invertir solo ω; no toca la traslación | §5.3 nivel 3 |
| Una sola rueda va al revés | cables del motor invertidos | prueba rueda por rueda | §5.2 |
| Una rueda zumba y no gira | PWM por debajo del piso | rampa de PWM y anotar dónde arranca | §6.3 |
| Arranca de quieto solo si le pego fuerte | piso de PWM (arranque > sostén) | medir los dos pisos | §6.3 |
| Se va de rumbo yendo de costado | yaw parásito | medir la deriva a lazo abierto, 2 s de strafe | §7.1 |
| Corrige el rumbo para un solo lado | las tablas actuales son compensación de signo fijo, no realimentación | tabla de sumas de §7.3 | §7.3 |
| Serpentea permanentemente | banda muerta ±1° demasiado angosta para el ruido del BNO | imprimir `error` por serie y ver cuánto tiempo está dentro de ±1 | §7.4 |
| Se movía bien y hoy no | batería a media carga: el piso de PWM subió | repetir la medición con batería a medias | §6.3 |
| Cambio `pasoPateo` (solo delantero) y no pasa nada | puede ser un bug conocido, no el tuning: la rampa nunca vuelve a cero (DEL-04) | ver `bugs-conocidos.md` | — |
| No compila | llave de más + `bno` duplicado | `bugs-conocidos.md` y `correcciones-propuestas.md` | §8.1 |

---

## 11. Archivos de este robot (dónde mirar)

- `futbol-roboliga2026/robots-2025/arquero/arquero.ino` — 1207 líneas, `#define ROBOT1`
  (líneas 9-11). Funciones de movimiento en 140-184, tablas de strafe con rumbo en 187-233.
  Estado inicial `impulso_inicial` (`131`).
- `futbol-roboliga2026/robots-2025/delantero/delantero.ino` — 1214 líneas, `#define ROBOT2`
  (líneas 9-11). Distinto `#define` y distinto estado inicial: `AVANCE_INICIO` (`138`).
  Su `avanzar_patear()` (181-201) no es igual al del arquero.
- `futbol-roboliga2026/robots-2025/libreria-zircon/zirconLib.cpp` — 355 líneas. `initializePins()`
  (234-342) es quien pone los pines de motor en OUTPUT; `motor1/2/3()` (173-232) **no se usan**.
  **Hoy no compila** (llave de más en la 355).
- `futbol-roboliga2026/robots-2025/mapa-pines-teensy.md` — pines por robot y tabla de equivalencia
  (78-87).
- `futbol-roboliga2026/robots-2025/arquero/COMO-FUNCIONA.md` y
  `futbol-roboliga2026/robots-2025/delantero/COMO-FUNCIONA.md` — transcripción fiel de cada programa.
- `futbol-roboliga2026/bugs-conocidos.md` y `correcciones-propuestas.md` — lo que está roto y los
  parches propuestos.

**Aviso final, el mismo del principio:** los dos `.ino` **salieron del mismo programa** y cada uno
contiene la máquina de estados del arquero **y** la del delantero en el mismo `switch`; el
`#define` y el estado inicial deciden cuál corre, y el bloque del otro robot queda
**inalcanzable**. Si están leyendo una función y no logran entender cuándo se ejecuta, chequeen
primero si pertenece al otro robot: puede que la respuesta sea **nunca**.

⚠️ **Pero NO son copias idénticas.** El equipo 2025 los editó por separado y divergieron: hay
estados que existen en uno y no en el otro, y `tolerancia_cercania` es 140.0 en el arquero y 50.0
en el delantero. **Dar vuelta el `#define` de un archivo no te da el firmware del otro robot.**
Cada robot tiene el suyo.
