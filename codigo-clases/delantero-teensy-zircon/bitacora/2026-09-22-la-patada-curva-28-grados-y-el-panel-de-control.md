# 2026-09-22 — La patada curva 28 grados, y un panel para tocar sin depender de nadie

**Quiénes:** Máximo (cancha, mesa y las decisiones) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**Programa cargado al cerrar:** `funciona/delantero/`, entorno `teensy41` (elige el arco mirando)

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🎯 **La patada curva 28,4°** | medido. Y **siempre al mismo lado**: es un sesgo, no azar |
| ⚠️ **El número que importa no es el 4,4** | es el **pico de 12,9°**, porque la pelota se va antes del final |
| ✅ **Cuatro mejoras a la patada** | compensación fija, corrección que acelera, trasera trabada y derivativo |
| ✅ **Panel de control** | los 20 números que se tocan en clase, arriba de todo, con qué hace cada uno |
| ✅ **`MEDIR-ROBOT.bat`** | menú de calibraciones, separado del de partido |
| ✅ **Aflojar al acercarse** | desacomodaba la pelota al llegar. Ahora llega suave |
| 🧹 **7 programas borrados** | `grabar-linea` los reemplazó. Los 4 del giroscopio quedan |
| 🏆 **Partidos 2 y 3 anotados** | 3-1 y 3-2, contados por el equipo (van en la bitácora del 21/09) |
| 🔴 **Nada de esto se vio en cancha** | ni las cuatro de la patada, ni el aflojado |

---

## 1. 🎯 La patada curva, y está medido

El equipo lo reportó así: *"cuando arranca, se desvía y gira un poco a un lado mientras avanza"*.
Y propuso dos causas: potencia distinta entre motores, o que uno arranque antes que el otro.

**La primera era la correcta.** Se corrió `pruebas/patada-derecha/` en el piso:

```
A) SIN correccion  (el robot desnudo):   -28.4 grados   (pico -22.9)
B) CON heading-hold, KP = 4.0:            -4.4 grados   (pico -12.9)
```

**Los dos negativos: siempre se va para la DERECHA.** Eso descarta el azar — es una rueda que
empuja más que la otra, sistemáticamente.

### 🔴 Y algo que no esperábamos

| | 01/09 | hoy |
|---|---|---|
| patada | 240 × **1000 ms** | 215 × **420 ms** |
| se torció | **10,1°** | **28,4°** |

**Casi el triple de desvío en menos de la mitad de tiempo**, y hoy encima con la rampa puesta.
La explicación más probable es **la cancha de tela**: cambió el agarre, y si las dos ruedas de
adelante agarran distinto, el desbalance se amplifica. Es un dato nuevo, no estaba.

### ⚠️ El número que importa no es el 4,4

El heading-hold rescata el final (28,4 → 4,4), pero el **pico de B fue −12,9°**.

> La pelota se va del robot en los primeros ~200 ms. O sea que **la pelota ve el pico, no el
> promedio final**. El 4,4° es la foto de después de que la pelota ya salió.

Por eso las cuatro mejoras apuntan a **bajar el pico**, no el número de cierre.

---

## 2. Las cuatro mejoras de la patada

Todas en `avanzarDerecho()`. **Ninguna probada en cancha.**

### 1 · Compensación fija (`TRIM_PATADA = 15`)

```cpp
int vi = vel - TRIM_PATADA;   // rueda izquierda
int vd = vel + TRIM_PATADA;   // rueda derecha
```

El sesgo es sistemático, así que se anula **de entrada** en vez de esperar a medirlo y
reaccionar. Ataca los 28,4° de raíz.

**El signo se dedujo encadenando dos mediciones**, no a ojo:

```
dif(r0, rumboFinal) = -28,4   ->  el rumbo SUBIO 28,4
rumbo que sube = girar a la DERECHA   [medido el 01/09]
->  se va a la derecha  ->  hay que aflojar la IZQUIERDA
```

⚠️ **El valor 15 es tanteo.** Se eligió a ojo. Lo bueno es que el próximo síntoma lo corrige
solo: si sigue curvando a la derecha, subirlo; si ahora curva a la izquierda, se pasó.

### 2 · La corrección ahora acelera, no sólo frena

```cpp
if (mando > 0) { vd -= mitad; vi += mitad; }
else           { vi -= mitad; vd += mitad; }
```

Antes sólo restaba, y estaba escrito el motivo en el código: *"a 240 no hay lugar para subir"*.
**Pero el 01/09 la patada bajó a 215 y sobran 40 puntos.** Era una regla que seguía viva
después de que desapareció su motivo.

Con el pico de 12,9°, la corrección vieja le sacaba **51 puntos de PWM a una rueda** justo
cuando la pelota necesita el golpe: enderezaba, pero igualando para abajo. Repartiendo, el
**promedio de las dos ruedas se mantiene en 215**.

### 3 · La trasera queda TRABADA durante la patada

Antes `avanzar()` la dejaba **suelta** (las dos patas en 0), y suelta no se opone a nada: el
robot pivotea alrededor de ella sin resistencia.

Ahora se le aplica el freno eléctrico (`frenar()`, portado del arquero el 15/09).

> **Y trabarla no debería frenar el avance**, por cómo funciona una rueda omni: para ir derecho
> hacia adelante la trasera se mueve **de costado sobre sus rodillos libres**, que el motor no
> toca. El motor sólo interviene cuando la rueda tiene que girar — o sea cuando el robot rota.
>
> ⚠️ **Es una hipótesis.** Si al probarlo la pelota llega notablemente más corta, estaba mal:
> los rodillos rozan más de lo que pensamos, y hay que soltar la trasera de nuevo.

### 4 · Término derivativo (`KD_PATADA = 0.3`)

```cpp
float mando = KP_PATADA * err + KD_PATADA * derivPatada;
```

`KP` mira **cuánto** se torció; `KD` mira **qué tan rápido está girando**. El segundo llega
antes: reacciona a que *empezó* a girar sin esperar a que el error se acumule. Es justo lo que
hace falta para bajar el pico.

Y de paso ataca el rebote que quedó anotado el 01/09 (corrigió 33° y **se pasó a −9,4°**).

> 🚨 **La trampa del derivativo en este robot, y cómo se esquivó.** El loop corre a
> **17.300 vueltas/s** pero el BNO055 actualiza su fusión a **~100/s**. Calculando la derivada
> en cada vuelta, la mayoría de las veces se leería **el mismo número repetido** → derivada
> cero, y cuando el sensor por fin cambia, un salto enorme. El robot pegaría tirones.
> Se muestrea cada **10 ms** (`MS_DERIVADA_PATADA`), que es el ritmo real del sensor.

### Y la compensación sobrevive sin giroscopio

```cpp
if (giroscopoSano()) avanzarDerecho(vel, rumboAlPatear);
else                 motoresPatada(vel - TRIM_PATADA, vel + TRIM_PATADA);
```

El P y el D **necesitan el giroscopio**: sin él no hay lazo. Pero el trim es un número, no una
realimentación, así que se aplica igual. Antes, sin giroscopio, la patada volvía a los 28,4°
pelados.

---

## 3. Aflojar al acercarse a la pelota

Reportado: *"cuando se dirige a la pelota, la toca a veces por la inercia y la desacomoda"*.

**La causa:** `avanzar(VEL_AVANCE)` mandaba **95 fijo** desde lejos hasta cruzar `XP_ORBITA`.
Llegaba a toda velocidad y recién ahí cambiaba de estado — pero el envión no lo frena un cambio
de estado.

Apareció al subir `VEL_AVANCE` de 55 a 95 el 15/09: a 55 casi no arrancaba, pero tampoco
llegaba con inercia. **Le arreglamos el arranque y le creamos el frenado.**

```
Xp > 70   ->  95        Xp = 52  ->  82        Xp = 34  ->  70
```

No se tocó `XP_ORBITA`: ese número ya estaba tanteado hasta que la órbita andaba bien
(22 → 34 → 38 → 34). El problema era de velocidad, no de distancia.

---

## 4. 🎛️ Panel de control

**Los 20 números que se tocan en clase están ahora arriba de todo del `delantero.ino`**,
agrupados por lo que hacen y cada uno con una línea de qué hace:

```cpp
// ---------- IR A BUSCAR LA PELOTA ----------
const int VEL_AVANCE       = 95;   // potencia yendo hacia la pelota, de LEJOS
const int VEL_AVANCE_CERCA = 70;   // ...y con cuanta llega, ya CERCA
```

**Los comentarios largos quedaron donde estaban.** En el lugar de cada constante hay un cartel
que dice que se mudó. La idea: el que va a cambiar un número lo encuentra rápido; el que quiere
saber por qué vale eso, también.

*(Pedido del equipo: "así en caso de necesitar cambiarlas lo hacemos más rápido, en caso de que
no te tengamos a vos".)*

---

## 5. 🖱️ `MEDIR-ROBOT.bat`

Menú de calibraciones, **separado del `CARGAR-ROBOT.bat`** que ganó partidos y no se toca.

- Las 13 pruebas agrupadas: línea, giroscopio, motores, cámara.
- Marca con `*` las que **no necesitan cable**.
- **Opción `L`: leer el robot** al volver de la cancha.
- **Guarda cada medición en `mediciones/`** antes de mostrarla.
- Verifica que el robot sea el delantero y que esté solo. Si es el arquero, **no carga**.

**No escribe los valores medidos en el programa principal** — eso quedó para más adelante, por
decisión del equipo.

> **Se llamaba `CALIBRAR-ROBOT` y se renombró:** en la carpeta se confundía con `CARGAR-ROBOT`
> (las dos empiezan con "CA"). `MEDIR` vs `CARGAR` no se parecen.

---

## 6. 🧹 Limpieza

**Borrados (7):** `grabar-verde`, `medir-verde`, `calibrar-linea`, `sensores-de-linea`,
`adelante-atras`, `tres-ruedas`, `buscar-pelota`.

Los cuatro primeros los reemplazó `grabar-linea`, que hace verde + blanco + negro, clasifica
solo y propone los umbrales. `adelante-atras` y `tres-ruedas` los cubre `diagnostico-motores`.
`buscar-pelota` era una foto congelada del firmware del 28/07.

**Los 4 del giroscopio quedan** (`giroscopo-crudo`, `rumbo-vivo`, `giroscopo-recupera`,
`signos`): se habían borrado dos y el equipo pidió que no, y se restauraron.

**Dos cosas que aparecieron al limpiar:**

- 🐛 **`grabar-linea` comparaba contra los umbrales viejos** (`663/661/757`). Su aviso
  *"CRUZA EL UMBRAL DE HOY"* estaba mintiendo desde el 15/09. Ya apunta a `390/427/413`.
- Borrar dejó **enlaces rotos**, incluido uno que **el robot imprimía por el monitor**
  (`Corre pruebas/sensores-de-linea/`, una carpeta que ya no existe).

Y `patada-derecha` se actualizó: medía `240 × 1000 ms` sin rampa, o sea **una patada que el
robot ya no pega**. Ahora mide `215 × 420 ms` con rampa.

---

## 7. Números de hoy

```
PATADA, medida en el piso con patada-derecha (215 x 420 ms, con rampa):
   sin correccion .... -28.4 grados   (pico -22.9)   siempre a la DERECHA
   con heading-hold ...  -4.4 grados   (pico -12.9)

PANEL DE CONTROL al cerrar:
   avance        95, afloja hasta 70 desde Xp<70
   orbita        Xp<34 · impulso 120 x 300 ms -> crucero 67 · max 20 s
   patada        215 x 420 ms · rampa +15 cada 5 ms
   patada recta  trim 15 · KP 4.0 · KD 0.3 · trasera trabada
   linea         umbrales 390/427/413 · confirma 5 ms
                 al verla: retrocede 210 ms a 170, ciego hasta 300 ms
```

---

## ⚠️ Qué queda por VER

1. 🔴 **Las cuatro mejoras de la patada.** Ninguna se vio en cancha. **Lo primero a mirar es
   para qué lado curva ahora:** si sigue a la derecha el trim quedó corto; si ahora va a la
   izquierda, se pasó.
2. 🔴 **Que la pelota no llegue más corta**, por la trasera trabada. Si llega corta, la
   hipótesis de los rodillos estaba mal.
3. 🔴 **Que no pegue tirones**, que sería el derivativo con el muestreo mal.
4. 🔴 **El aflojado al acercarse** tampoco se probó.
5. **`MEDIR-ROBOT.bat` no lo usó todavía una persona.** Apareció y se arregló un bug al
   probarlo (ver abajo), pero el menú completo no se recorrió.

## 🔧 Qué queda por HACER

6. **Llevar las cuatro mejoras a `patada-derecha`.** Hoy su patada B usa el corrector **viejo**
   (sólo frena, sin trim, sin D, trasera suelta), así que no sirve para verificar lo nuevo.
   Con eso se mide si el trim 15 quedó corto, justo o pasado.
7. 🔴 **Correr `pruebas/piso-de-pwm/`** sobre la tela. Sigue sin correrse desde julio, y
   🐛 **tiene los pines del ARQUERO**: hay que corregirlo antes o mueve las ruedas equivocadas.
8. 🔴 **Correr `pruebas/tabla-camara/`** y cerrar `XP_ORBITA` con centímetros de verdad.
9. **Anotar por qué se perdió el partido 2** (3-1). Se salió menos de la cancha, así que fue
   por otra cosa, y no quedó escrito.
10. 🐛 **`sentidoParaOrbitar()`: las dos ramas eligen lados opuestos.** Sigue abierto.
11. 📓 **La bitácora del 08/09 sigue sin escribirse.**

## ❓ Una anomalía, anotada sin explicar

En **un** arranque el giroscopio contestó `SYS_STATUS=57, SYS_ERR=58`. **Esos valores no
existen** — `SYS_STATUS` va de 0 a 5. En el arranque siguiente volvió a dar 5 y todo normal.
Huele a lectura I2C corrupta en el arranque. Si se repite, `giroscopo-crudo` es el que lo mira.

---

## Nota de método

**Lo mejor del día fue una pregunta del equipo, otra vez.** *"¿Podría ser causado por la
potencia de los motores o que alguno haya empezado antes que el otro? Pero lo dudo."* La
primera parte era exactamente la causa, y la medición la confirmó con número y signo.

**Y una advertencia sobre el 28,4°**, que el propio equipo marcó: **ese número es sin
corrector**. El robot en juego nunca pateó así — siempre tuvo el heading-hold. Sirve para
dimensionar el trim, no para describir cómo juega.

**Dos errores míos, los dos del mismo tipo: repetir una trampa que ya conocía.**

- Puse la función nueva `velAcercarse()` entre las constantes y **rompí el build**: el
  preprocesador de Arduino mete los prototipos justo antes de la **primera** función del
  archivo, y quedaron arriba del `enum Estado`. Ya está el aviso escrito en el código.
- En `medir-robot.ps1` llamé `$pruebas` a una ruta y `$PRUEBAS` a la lista del menú. **En
  PowerShell no se distinguen mayúsculas: son la misma variable**, y la lista pisó la ruta. Es
  **la segunda vez en la misma semana** que me como ese error — la primera fue con
  `$enviar`/`$Enviar`. Ahora está el aviso en el script.

> La lección que se repite: **las trampas conocidas no se evitan solas por estar anotadas.**
> Hay que mirarlas antes de escribir, no después de que el build falla.
