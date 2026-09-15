# 2026-09-15 — Cancha nueva: se recalibró todo, y el giroscopio quedó como el problema de fondo

**Quiénes:** los chicos (mesa y cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa:** `funciona/seguir-y-despejar`. Nuevos: `pruebas/mapa-linea`, `pruebas/buscar-i2c`,
`pruebas/probar-giroscopo`

> 🚨 **CAMBIÓ LA CANCHA.** Es **más chica**, el verde es **más oscuro**, las líneas blancas son
> **más angostas**, y los bordes tienen una **rampita** para que la pelota vuelva al campo.
> Eso invalidó varios números medidos y fue el tema del día.

**Resumen:** se recalibró la cancha y se ajustó el despeje a la nueva medida. El giroscopio
apareció como la causa de fondo de casi todo lo demás — es **intermitente**, y quedó pendiente
revisar su conector. Quedó programado y listo para el amistoso que el robot elija solo entre el
arco azul y el amarillo.

| | |
|---|---|
| §1 | Qué números murieron con la cancha nueva y cuáles sobrevivieron |
| §2 | La calibración de la línea, y un sensor que parecía roto y no lo estaba |
| §3 | El despeje ajustado a la cancha chica |
| §4 | 🚨 El giroscopio: cinco hipótesis descartadas |
| §5 | "Siempre mirar al frente" — lo que encontró la auditoría |
| §6 | Listo para el amistoso: elegir el arco solo |

---

## 1. Qué sobrevive y qué no

Lo primero fue separar los números **del robot** de los números **de la cancha**:

| Sobrevive ✅ | Murió ❌ |
|---|---|
| Conversión de la cámara (2,87 unidades = 1 cm) | El umbral de blanco (620) |
| Mapa de ruedas y de sensores | Las distancias del despeje |
| Piso de arranque de los motores (~70 de PWM) | |
| Que el arco sirva para centrarse | |

La conversión de la cámara se salva porque es **geometría de la cámara**, no del piso. El umbral
de blanco se muere porque es puro color de cancha.

---

## 2. La línea: umbral 425, y el sensor que no estaba roto

### El problema de medir sin cable ni LED

En la cancha no llega el USB, y el LED del robot está casi tapado por la batería. Se escribió
[`pruebas/mapa-linea`](../pruebas/mapa-linea/), que en vez de guardar mínimo y máximo guarda un
**histograma**: cuenta cuántas veces vio cada valor.

> Las superficies donde el robot estuvo apoyado un rato forman **montones**; lo que vio de paso
> —el piso, la mesa, los dedos mientras lo llevan— queda como **pelitos sueltos**. No importa
> *cuándo* estuvo sobre cada cosa: el montón se forma solo.

Al final se usó el `calibrar-linea` de siempre, porque el equipo avisó que el LED **sí se ve un
poquito**. `mapa-linea` queda para cuando no alcance.

### La primera medición: un sensor "roto"

| Sensor | Verde | Blanco | Separación |
|---|---|---|---|
| A13 atrás-izq | 92 | 540 | 443 ✅ |
| **A11 atrás-der** | 59 | **87** | **22** ❌ |
| A12 adelante | 60 | 565 | 500 ✅ |

El programa dijo *"se superponen, no hay umbral que sirva"* — pero mezclaba los dos de atrás y
**el peor arrastraba al otro**. Dos de tres sensores andaban perfecto.

**El A11 no estaba roto: no estaba sobre la línea.** Lo dijo el equipo:

> *"únicamente 2 sensores estaban sobre el blanco porque la línea no es tan ancha"*

⚠️ **Consecuencia que conviene tener presente:** la línea nueva es **más angosta que la
separación entre los dos sensores de atrás**, así que los dos **nunca van a estar sobre la línea
al mismo tiempo**. Para el robot no es problema —el regreso del despeje frena con *cualquiera* de
los dos— pero para medir hay que medir cada uno por separado.

### La segunda medición, con el A11 sobre la línea

| Sensor | Verde | Blanco (máximo) |
|---|---|---|
| A13 | 95 | **763** |
| A11 | 81 | **753** |
| A12 | 56 | **759** |

Los promedios salieron sucios (rangos de 279 a 763: el robot estuvo a ratos adentro y a ratos
afuera de la línea mientras medía), pero **los máximos son limpios y coinciden los tres**.

### 🎯 UMBRAL: 425

```
verde más claro:   97
blanco de verdad: 753
punto medio:      425     ← queda a 328 de cada lado
```

**Y hay un motivo más fuerte que la cuenta para bajarlo tanto.** El mismo sensor, sobre la misma
línea, dio **557 o 763 según qué tan centrado estuviera**. Y el robot, cuando vuelve del despeje,
cruza la línea **en movimiento** y muchas veces en diagonal: nunca perfectamente centrado.

> **Con el umbral viejo (620) el robot nunca habría encontrado la línea nueva.** Se le rompía el
> regreso del despeje *y* la ubicación inicial, y habríamos perdido clases buscando el problema
> en el código.

### Comparación de las dos canchas

| | Vieja | Nueva |
|---|---|---|
| Verde | 356 / 465 | **60 a 95** |
| Blanco | ~760 | **~755** |

El blanco quedó igual y **el verde se oscureció muchísimo**: el contraste **mejoró**.

---

## 3. El despeje, ajustado a la cancha chica

Tres pasadas en el día:

| | Ida | Disparo | Qué pasó |
|---|---|---|---|
| Al empezar | 533 ms (~50 cm) | 30 cm | de la cancha vieja |
| Primer ajuste | 333 ms (~30 cm) | 30 cm | **quedó corto: "ni llegaba a tocar la pelota"** |
| Final | **433 ms (~40 cm)** | **20 cm** | |

**Por qué quedaba corto, y por qué ahora cierra.** Con la ida en 30 y el disparo también en 30, el
robot llegaba **justo** a donde estaba la pelota y no le sobraba ni un centímetro para empujarla.
Ahora dispara a 20 y avanza 40: **le sobran 20 cm de empuje**.

Es la regla vieja del programa —`umbralCm` nunca puede ser mayor que el alcance de la ida— que
esta vez se cumplía *justo*, y "justo" no alcanzaba.

---

## 4. 🚨 El giroscopio: cinco hipótesis descartadas

El síntoma: el robot no se enderezaba, y a veces sí. **Es intermitente.** En la misma tarde:

```
16:xx  >> giroscopio DANDO DATOS despues de 561 ms     ✅
17:xx  >> giroscopio: NUNCA APARECIO al encender       ❌
```

### Lo que se descartó, y con qué

| Hipótesis | Cómo se descartó |
|---|---|
| Batería floja | El equipo midió **8,23 V** — más que la nominal |
| Tarda en calentar | No llega ni a saludar: `bno.begin()` falla 10 veces |
| **Bus equivocado** | La placa es **Mark1** (pin 32 en bajo), y los **tres** buses están vacíos |
| Cable de datos cortado | SDA y SCL los dos en alto: ninguno pegado a masa |
| Falta algo del código 2025 | Es **idéntico** al nuestro |

### El bus equivocado: una buena idea que no era

El equipo trajo el código con el que compitieron en 2025 y encontró esto en `zirconLib`:

```cpp
if (ZirconVersion == "Mark1")   bno = Adafruit_BNO055(55, 0x28, &Wire);    // 18/19
else if (ZirconVersion == "Naveen1") bno = Adafruit_BNO055(55, 0x28, &Wire2); // 24/25
```

**El Teensy 4.1 tiene tres buses I2C**, y según la versión de la placa el giroscopio va en uno o
en otro. Valía la pena mirarlo. Se escaneraon los tres: **todos vacíos**, y el pin 32 dice que
esta placa es Mark1, o sea que el bus que ya usábamos era el correcto.

Descartada, pero bien descartada: ahora queda escrito para siempre qué placa es.

### Herramientas que quedaron

- [`pruebas/buscar-i2c`](../pruebas/buscar-i2c/) — escanea **los tres buses** cada 2 segundos,
  para poder **menear el conector** y ver si el sensor aparece. Además mira el estado eléctrico de
  SDA y SCL antes de hablar, y dice qué versión de placa es.
- [`pruebas/probar-giroscopo`](../pruebas/probar-giroscopo/) — el ejemplo oficial de Adafruit con
  tres arreglos: **no se cuelga** si no lo encuentra (el original hace `while(1)`, justo lo que no
  sirve para diagnosticar), busca en 0x28 **y** 0x29, y baja el bus a 100 kHz porque Adafruit
  avisa que el BNO055 es quisquilloso con los tiempos. Y pregunta la identificación cruda
  (registro 0x00 = 0xA0) **sin ninguna librería**, para sacar la librería de sospechosos.

### Lo que el robot cuenta ahora

Se agregó que el robot se acuerde y lo diga con la tecla `i`:

```
el giroscopio tardo 561 ms en dar el primer dato despues de encender
giroscopio: NUNCA APARECIO al encender
me enderece esperando: 0 veces
```

Ese último número resultó clave (ver §5).

---

## 5. "Siempre mirar al frente"

### El pedido del equipo

> *"El robot cuando no ve la pelota no vuelve a mirar al frente. Tiene que mirar al frente
> cuando: 1- cuando arranca. 2- después de retroceder, luego del despeje. 3- cuando pierde la
> pelota o no esté en su campo de visión. 4- seguramente otra ocasión, pero se entiende la
> importancia."*

Y aclararon el mecanismo: se tuerce **moviéndose rápido de costado** detrás de la pelota, y si la
pierde ahí, queda mirando a una pared.

### Primer intento, y por qué no alcanzó

Se agregó que en `ESPERANDO` el robot se enderece solo si se pasa de 8 grados. **No funcionó.** El
robot lo explicó él mismo:

```
giroscopio: NUNCA APARECIO al encender
!! y NUNCA lo dio: me arme sin rumbo
me enderece esperando: 0 veces
```

En esa corrida no había giroscopio, así que **no había con qué enderezarse**. El chequeo exige un
rumbo de referencia y el robot se armó sin ninguno. **El equipo tenía razón** al sospechar del
giroscopio — y esta vez lo confirmó el robot, no una deducción.

### 🎯 Lo que encontró la auditoría del código

El mecanismo de fondo, en una línea:

> `correccionDeRumbo()` **no maneja los motores**: devuelve un número que **otro le suma a las
> ruedas**. Y se suma en cinco lugares, **todos movimientos**.

```
ROBOT QUIETO = NADIE SUMA NADA = EL RUMBO NO SE SOSTIENE
```

El censo encontró **siete lugares** donde el robot se queda quieto. El parche tapaba **uno**.

**El peor es la zona muerta de `SIGUIENDO`:** cuando la pelota queda casi enfrente pero lejos, el
código hace `parar()` y **no cambia de fase**. El robot queda con los motores sueltos, viendo la
pelota, sin corregir nada.

> Y acá está lo lindo del hallazgo: **desvío ≈ 0 es exactamente el estado de ÉXITO que toda la
> fase persigue.** Cada vez que el robot alinea bien una pelota lejana, aterriza en la zona muerta
> y se congela. **El estado de éxito de la fase es el único donde no se corrige el rumbo.**

Ese es el *"4 - seguramente otra ocasión"* que el equipo intuyó y no pudo nombrar.

### La regla general propuesta (⬜ sin implementar)

> **Si el robot está quieto y está torcido, se endereza.** Sin importar en qué fase esté.

Con un truco para que sea estructural y no otro parche: **"¿estoy quieto?" se contesta con un solo
dato — hace cuánto que alguien llamó a `aplicar()`**. Todo movimiento pasa por ahí; `parar()` y
`frenar()` escriben los pines por su cuenta y no. Así **cualquier fase futura hereda la regla**. Y
las pausas cortas (150, 200, 300 ms) quedan excluidas solas por durar menos que el umbral.

### Una regresión que se metió y se arregló

El parche podía dejar al robot en un lazo: si el enderezado no converge, vuelve a `ESPERANDO`
torcido, mide, dispara la misma maniobra que acaba de fracasar. Y como corta **antes** de mirar la
pelota, **un robot atrapado ahí no ataja**. Hasta 14 segundos girando y ciego.

Se le puso un **descanso obligatorio de 6 segundos** entre intentos.

---

## 6. Listo para el amistoso: el robot elige el arco solo

**Pedido del equipo:** en el amistoso el lado se sortea, así que el arco del rival a veces va a
ser **azul** y a veces **amarillo**. No se puede recompilar entre partido y partido.

**Cómo lo resuelve:** durante los 10 segundos de la cuenta de arranque —cuando ya está apoyado,
quieto y mirando la cancha— el robot **cuenta cuántos cuadros ve cada arco**. El que más vio es el
del rival, porque **el propio lo tiene a la espalda** y la cámara no lo puede ver. Al armarse se
queda con ese y no lo cambia más.

No hay que configurar nada ni acordarse de apretar ninguna tecla: se apoya el robot mirando la
cancha y listo. Y **no cambia de opinión a mitad del partido**, que sería peor que equivocarse.

⚠️ **El amarillo es más difícil de ver:** el programa de la cámara le exige una mancha de **600
píxeles** al arco amarillo y solo **300** al azul. Si toca el lado amarillo y no lo reconoce, ese
es el primer sospechoso — y la salida es forzarlo con la tecla **`b`** (cicla auto → azul →
amarillo).

**Compilado y sin probar:** el Teensy quedó desenchufado al final de la clase.

---

## Números de hoy

| Qué | Valor |
|---|---|
| **Umbral de blanco** | 620 → **425** |
| Verde de la cancha nueva | 56 a 95 (la vieja: 356 / 465) |
| Blanco de la cancha nueva | **~755** (la vieja: ~760) |
| El mismo sensor sobre la misma línea | **557 o 763** según el centrado |
| **Ida del despeje** | 533 → 333 → **433 ms (~40 cm)** |
| **Disparo del despeje** | 30 → **20 cm reales** |
| Placa Zircon | **Mark1** (pin 32 en bajo) → giroscopio en `Wire`, 18/19 |
| Batería medida | **8,23 V** |
| El giroscopio tarda en dar datos | **561 ms** después de saludar |
| Velocidad lateral máxima de la pelota vista | **34,8 cm/s** (el ruido es 3,6) |
| Descanso entre intentos de enderezarse | 6 s |

---

## Qué queda pendiente

### 🔴 Lo primero, y viene de tres clases

- ⬜ **El conector del giroscopio.** SDA 18, SCL 19, VIN y GND. Los cables de datos están sanos,
  así que **los primeros sospechosos son VIN y GND**. Con `buscar-i2c` corriendo se puede menear
  el conector y ver si aparece. Y mirar si la plaquita del sensor tiene LED de encendido.
- ⬜ **Que `bno.begin()` se reintente DURANTE el juego**, no solo al encender. Hoy, si falla al
  arrancar, el robot queda ciego toda la corrida aunque el sensor aparezca dos segundos después.
  Con un sensor intermitente, eso convierte una falla pasajera en una corrida perdida.
  Queda por decidir qué toma como "frente" si aparece a mitad de partido.

### Para el amistoso

- ⬜ **Probar que elija bien el arco**, sobre todo del lado **amarillo**, que es el más difícil de
  detectar. Tecla `b` si se equivoca.
- ⬜ **Probar el centrado con el arco**, que se volvió a prender y nunca se probó.

### De la regla del frente

- ⬜ **Implementar la regla general** (§5). Hoy solo está tapado `ESPERANDO`, y el agujero más
  grande es la zona muerta de `SIGUIENDO`.
- ⬜ Bajar `TOLERANCIA_ESPERA` de 8 grados, que es mucho para quedarse quieto.

### De antes

- ⬜ **La diagonal del despeje está apagada** (`D`), escrita y nunca probada. Necesita el
  giroscopio: sin él sale girando.
- ⬜ **La predicción nunca se probó en cancha** con pelota rápida.
- ⬜ **La rampita de los bordes**: no se investigó. ¿Se le puede subir el robot? Inclinado
  confunde al giroscopio y a los sensores de línea.
- ⬜ **Un LED que se vea** — pines 9 y 10 libres, el código ya está preparado con `PIN_AVISO`.
- ⬜ Medir la conversión del eje Y de la cámara.

---

## Lo que se aprendió del método

**1. Un programa que junta dos sensores puede mentir sobre los dos.**
`calibrar-linea` dijo "se superponen, no hay umbral que sirva" cuando **dos de tres sensores
andaban perfecto**. Mezclaba los dos de atrás y el peor arrastraba al otro. Antes de creerle al
veredicto, conviene mirar los números de a uno.

**2. "No funciona" no dice cuál de las dos cosas falló.**
El robot no se enderezaba. Podía ser que no se diera cuenta, o que no pudiera girar. Son arreglos
distintos. Se agregó un contador —*"me enderecé esperando: N veces"*— y ese número solo separa los
dos casos. **Cuando algo no anda, la primera pregunta es qué medición distinguiría las causas.**

**3. Descartar bien vale tanto como acertar.**
De las cinco hipótesis del giroscopio, las cinco se cayeron. No es tiempo perdido: cada una se
cayó **con un dato**, y ahora sabemos que el problema no está en el software. Sin eso estaríamos
tocando código para siempre.

**4. La mejor pista del día la trajo el equipo.**
Traer el código de 2025 fue la idea correcta, y el hallazgo de los tres buses I2C era
perfectamente razonable. Que haya dado negativo no le quita valor: **una hipótesis que se puede
descartar en cinco minutos vale más que una que suena bien y no se puede probar.**
