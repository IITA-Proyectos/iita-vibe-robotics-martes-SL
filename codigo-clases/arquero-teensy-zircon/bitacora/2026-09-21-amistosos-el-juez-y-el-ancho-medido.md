# 2026-09-21 (lunes) — Amistosos: el giroscopio anduvo 5 de 5, el juez nos saco por quieto, y medimos el ancho

**Quiénes:** el equipo (cancha y partidos) + Claude (código, carga y lectura por USB)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas:** `funciona/seguir-y-despejar` (modificado). Nuevos: `pruebas/medir-ancho`,
`pruebas/medir-ancho-sin-giro`

**Resumen del día:** jornada de partidos amistosos. Se simplificó el programa de juego (sin
consola, arranque más corto, sin centrarse con el arco). El giroscopio anduvo **5 de 5**
apoyando el robot quieto **antes** de prenderlo. Un juez nos sacó un minuto porque el robot
**se quedaba quieto** cuando no veía la pelota, y de ahí salió el pedido nuevo: que patrulle.
Para eso se hizo una herramienta que **mide cuánto tarda en cruzar** de costado a costado.

| | |
|---|---|
| §1 | Cambios al programa de juego |
| §2 | El giroscopio: 5 de 5, y lo que dice internet |
| §3 | La comparación con `cuadrado-giroscopo` |
| §4 | El juez, y la herramienta para medir el ancho |
| §5 | 🎯 **EL CAMBIO QUE VIENE: patrullar sin pelota** |
| §6 | Trampas del día |

---

## 1. Cambios al programa de juego (`seguir-y-despejar`)

| Cambio | Antes | Ahora | Por qué |
|---|---|---|---|
| Teclas y mensajes por USB | muchos | **ninguno** | en la cancha no hay cable; nunca se usaban |
| Cuenta de arranque | 10 s | **2 s** | pedido del equipo |
| Espera extra si el giroscopio no da datos | 15 s | **2 s** | pedido del equipo (ver ⚠️ abajo) |
| Centrarse con el arco del rival | sí | **no** | "lo centramos nosotros" |
| Ida del despeje | 433 ms (~40 cm) | **533 ms (~50 cm)** | +10 cm, pedido del equipo |
| Distancia de disparo del despeje | 20 cm | **30 cm** | +10 cm, para que quede parejo con la ida |

Con disparo a 30 y ida de 50 le siguen sobrando **20 cm de empuje**, igual que antes (20 y 40).
Es la regla de siempre: la ida tiene que alcanzar más allá de donde está la pelota.

Ahora, al prender: 2 s de cuenta → atrás hasta la línea → se endereza → espera la pelota.
**Hay que apoyarlo a mano en el centro del arco.**

⚠️ La revisión del código marcó que bajar la espera extra de 15 a 2 s **empeora** el caso de un
giroscopio lento: si tarda más, el robot se arma sin rumbo y juega todo el partido sin
enderezarse. En las corridas sanas esa espera no corre, así que volverla a 15 s no cuesta nada.
**Pendiente decidir.**

Es lo que quedó **cargado para los partidos** (sin patrulla).

---

## 2. El giroscopio: 5 de 5 apoyando el robot quieto antes de prender

En un partido lo prendieron **5 veces y las 5 anduvo el giroscopio**. La única diferencia de
procedimiento: **apoyaban el robot en su lugar y esperaban 10 s o más antes de prenderlo.**
Antes lo prendían y lo acomodaban con la mano durante la cuenta.

Se investigó en internet (con verificación de fuentes):

- ✅ **Lo más firme:** el giroscopio del BNO055 se calibra solo **quedándose quieto unos segundos
  después de prender**, y esa calibración **se pierde en cada apagado**. Hoja de datos de Bosch
  (BST-BNO055-DS000, sección 3.11.2) y guía de Adafruit. Nadie publica cuántos segundos.
- ⚠️ **Otro sospechoso: la brújula.** `bno.begin()` deja el sensor en modo **NDOF**, que usa el
  magnetómetro. Adafruit documenta que en ese modo el rumbo puede **saltar de golpe** cuando la
  brújula se termina de calibrar. Podría explicar los 80° del 01/09. La salida sería el modo
  **IMUPLUS** (sin brújula); las reglas de RCJ piden no depender de la brújula y FTC lo usa por
  defecto. **No se cambió todavía.**
- ❓ **Estar apagado un rato** no tiene respaldo firme: solo relatos de usuarios de que apagar y
  prender muy rápido puede dejar al chip sin contestar.
- ❌ **Ninguna de las dos explica que se caiga a mitad de partido** (el LED que empieza a temblar
  después de moverse). Lo más probable: un bajón de tensión reinicia el chip, que vuelve a modo
  CONFIG y devuelve ceros. **El programa nunca lo vuelve a inicializar.**

> 5 de 5 es un buen indicio, no una prueba: si antes fallaba 1 de cada 3, cinco aciertos
> seguidos salen por suerte un 17% de las veces.

### 🎯 Procedimiento para los partidos

1. Apoyar el robot en su lugar, quieto, **antes** de prenderlo.
2. Prender la batería y **no tocarlo** hasta que arranque.
3. Si se apaga, esperar ~10 s antes de volver a prender.

Con la cuenta de 2 s esto es **obligatorio**: el rumbo de referencia se fija ~4 s después de
prender. Si lo están acomodando en ese momento, sostiene un "frente" torcido todo el partido.

---

## 3. ¿Por qué en `cuadrado-giroscopo` "anda mejor"?

El equipo notó que el giroscopio parecía andar mucho mejor en `cuadrado-giroscopo`. La comparación
(verificada línea por línea) dice que **no es el sensor, es el programa**:

1. `cuadrado-giroscopo` **no se mueve** si no tiene giroscopio → nunca se lo ve "andar mal".
   `seguir-y-despejar` juega igual sin él.
2. `seguir-y-despejar` pregunta por el giroscopio **una sola vez**, al prender. Si se cae
   jugando, no lo vuelve a buscar.
3. El enderezado se abandona con **una sola** lectura mala, y gira a 75-85 de PWM (se pasa).
4. Tirones sin arranque suave al empezar el retroceso del despeje y el empujoncito.
5. En la zona muerta de `SIGUIENDO` queda quieto sin corregir el rumbo (ya estaba anotado el 15/09).

---

## 4. El juez, y la herramienta para medir el ancho

**Lo que pasó:** cuando el robot no veía la pelota **se quedaba quieto**, y el juez lo tomó como
robot que no funciona y **lo sacó de la cancha 1 minuto**.

**Idea del equipo:** que cuando no vea la pelota vaya a un costado y vuelva a la mitad. Para eso
tiene que saber **cuánto tiempo tarda en cruzar** (no tiene encoders: mide tiempo, no distancia).

### `pruebas/medir-ancho` — solo la herramienta de medir

Se apoya en el medio y se prende. Solo, sin cámara:

1. espera que el giroscopio se calibre (quieto);
2. de costado a la **izquierda** hasta la línea (sensor de atrás izquierdo);
3. cruza a la **derecha cronometrando** → ida;
4. vuelve a la **izquierda cronometrando** → vuelta;
5. va **a la mitad** con la cuenta del medio cruce (si termina en el medio, el número sirve);
6. guarda los tiempos en la **EEPROM** (no se borra al apagar ni al cargar otro programa) y los
   informa **por USB** cada 2 s.

La cuenta del medio no es la mitad justa: cada tramo arranca desde quieto y el arranque rinde
menos (rampa + retardo del motor), igual que con el empujoncito del 18/08:

    al medio = cruce / 2 + 41 ms        (a VEL 100: rampa 100 ms, retardo 33 ms)

Versiones del día:
- **v1:** la que midió los números de abajo.
- **v2:** agrega topes de tiempo a la medida (antes eran 6 s fijos, casi 2 m) y **controla que los
  números tengan sentido** antes de guardar (cruce ≈ 2 veces la ida del medio; ida y vuelta
  parecidas). Si no: 5 destellos y no guarda.
- **`medir-ancho-sin-giro`:** igual, pero llama al giroscopio **8 veces** y, si no contesta, **mide
  igual sin él** (latido doble al terminar; la EEPROM anota que fue sin giroscopio).

### 🎯 LA MEDICIÓN (v1, con giroscopio)

| Qué | Tiempo |
|---|---|
| Cruce izquierda → derecha | **4618 ms** |
| Cruce derecha → izquierda | **4396 ms** |
| Diferencia | 222 ms (**4%**): va parejo para los dos lados |
| **Al medio desde la IZQUIERDA** (yendo a la derecha) | **2350 ms** |
| **Al medio desde la DERECHA** (yendo a la izquierda) | **2239 ms** |
| Velocidad de costado | **100** (mezcla 50/50/89) |

**Decisión del equipo: se usan estos números.** No hace falta medir en cada partido.

⚠️ Valen **solo para velocidad 100** y esta forma de moverse (misma rampa, misma mezcla, misma
corrección de rumbo). Si se cambia algo de eso, hay que volver a medir.

---

## 5. 🎯 EL CAMBIO QUE VIENE: patrullar sin pelota

**Pedido del equipo, para el programa de juego:**

> Cuando el robot pierde la pelota, **espera 8 segundos** a ver si vuelve a entrar en su campo
> de visión. Si no, **va hacia un lateral** (hasta la línea) y **de ahí directo al medio de la
> cancha**, usando los tiempos medidos.

    sin pelota 8 s  ->  de costado hasta la linea del lateral (frena en la linea)
                    ->  de vuelta al medio por tiempo:
                            2350 ms si salio de la linea IZQUIERDA
                            2239 ms si salio de la linea DERECHA
                    ->  espera la pelota (y si pasan otros 8 s, repite)

**No se llegó a hacer hoy**: el partido empezaba en 3 minutos y se cargó lo que funciona.

### Ya hay un borrador, y ya se revisó

Se escribió una primera versión (con medición del ancho al arrancar, que ahora sobra) y se le
hizo una revisión adversarial. Encontró **8 problemas reales**. Los que hay que resolver antes
de usarla en un partido:

| | Problema | Arreglo propuesto |
|---|---|---|
| 1 | **Sin giroscopio** la patrulla lo hace girar y se va de la cancha (de costado, la mezcla 50/50/89 gira si nadie corrige) | sin rumbo, **no** patrullar de costado (o solo ir y volver a la línea de atrás) |
| 2 | Si el seguimiento lo dejó **afuera del área**, pisa la línea desde afuera y "vuelve al medio" hacia el borde | contar los cruces de línea para saber de qué lado está |
| 3 | Si no encuentra la línea (tope), volvía el tiempo entero → de pared a pared | al llegar al tope, volver igual solo el tiempo del medio |
| 4 | Una pelota que **aparece de a ratos** reinicia la espera y lo deja quieto igual | pedir varios cuadros seguidos para cortar la patrulla |
| 5 | Con la **pelota pegada** al frente la cámara manda Xp=0 → se iría de costado en plena disputa | si la última vez la vio muy cerca, no patrullar (los 8 s ya ayudan mucho) |
| 6 | La profundidad se va corriendo patrulla tras patrulla (nada lo vuelve a apoyar en la línea de atrás) | cada 2 patrullas, volver a la línea de atrás |

Con los números fijos desaparecen los problemas de la **medición al arrancar** (que la pelota la
interrumpa, que mida mal y lo guarde todo el partido).

⚠️ También: con **velocidad 100** las ruedas de adelante reciben 50, debajo del piso de ~70 para
arrancar. Si en vez de deslizarse de costado el robot gira, ese es el primer número a mirar (pero
si se cambia, **hay que volver a medir**).

---

## 6. Trampas del día

**Con el USB enchufado, prender la batería NO reinicia el Teensy.** El Teensy ya estaba prendido
por el USB, arrancó sin batería (sin giroscopio) y se quedó en ese error: el robot quieto con
**3 destellos**. Para medir: desenchufar el USB y después prender.

**Cargar un programa con la batería prendida lo hace arrancar solo.** Pasó con `medir-ancho`:
anduvo 6 s de costado en la mesa buscando una línea que no había.

**La forma segura de leer la EEPROM:** batería apagada, USB enchufado, `mirar.bat`. Sin batería
los motores no tienen corriente y el robot no se mueve.

---

## Qué quedó cargado en el robot

**`funciona/seguir-y-despejar`**, la versión de los partidos: sin consola, cuenta de 2 s, sin
centrarse con el arco, despeje a 30 cm con ida de 50 cm. **Sin patrulla** (eso es §5).

Los números del ancho siguen en la EEPROM (el programa de juego no la toca).

---

## Números de hoy

| Qué | Valor |
|---|---|
| Giroscopio apoyando quieto antes de prender | **5 de 5** |
| Cruce izq → der / der → izq | **4618 / 4396 ms** |
| Al medio desde izq / desde der | **2350 / 2239 ms** |
| Velocidad de costado de la medición | 100 |
| Cuenta de arranque del juego | 10 → **2 s** |
| Espera extra del giroscopio | 15 → **2 s** (⚠️ revisar) |
| Espera sin pelota antes de patrullar (pedido) | **8 s** |
| Despeje: disparo / ida | 20 / 40 → **30 / 50 cm** (533 ms) |

---

## Qué queda pendiente

### 🔴 Lo primero

- ⬜ **Hacer la patrulla sin pelota** (§5) con los números medidos y los arreglos de la revisión.
  Probarla fuera de partido antes de usarla.
- ⬜ **Volver la espera extra del giroscopio a 15 s** (no cuesta nada cuando anda).

### Del giroscopio

- ⬜ **Que el programa de juego lo vuelva a buscar si se cae** jugando (hoy nunca lo reinicia).
- ⬜ **Probar el modo IMUPLUS** (sin brújula). Prueba barata antes: con `cuadrado-giroscopo`, acercar
  un destornillador al sensor y ver si el rumbo salta.
- ⬜ **Esperar la calibración del giroscopio** (`getCalibration`, giro = 3) antes de fijar el rumbo.
  `medir-ancho` ya lo hace.
- ⬜ El conector (VIN/GND) sigue siendo el sospechoso del "NUNCA APARECIÓ" (viene del 15/09).

### De antes

- ⬜ La regla general "si está quieto y torcido, se endereza" (15/09).
- ⬜ Un LED que se vea (pines 9 o 10 libres).

---

## Lo que se aprendió del método

**1. Anotar el procedimiento, no solo el programa.** El 5 de 5 no vino de un cambio de código:
vino de **cómo** prendían el robot. Si no lo hubieran contado, lo habríamos atribuido al programa.

**2. Separar la herramienta de medir del programa de juego.** Medir es una tarea aparte, fuera
del partido. Mezclarla con el juego complicaba todo (qué pasa si aparece la pelota mientras mide,
qué pasa si mide mal y lo usa todo el partido). Medir una vez, controlar el número, y usarlo fijo.

**3. Con 3 minutos, se carga lo que funciona.** Un cambio sin probar en un partido es una apuesta;
lo nuevo se prueba afuera.
