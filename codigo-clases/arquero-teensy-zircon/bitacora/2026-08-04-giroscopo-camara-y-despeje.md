# 2026-08-04 — Lazo cerrado con giroscopio, la cámara andando, y el primer despeje

**Quiénes:** Gustavo Viollaz (banco y piso) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas:** `pruebas/cuadrado-giroscopo`, `pruebas/ver-camara`, `pruebas/calibrar-15cm`,
`funciona/despeje-pelota`. Todos compilados con el mapeo del arquero (ROBOT1).

---

## Qué queríamos probar

Seguir donde quedamos: que los giros y los avances se controlen con el giroscopio en vez de con
cronómetro. Terminó siendo un día bastante más largo que eso.

---

## 1. El misterio del giroscopio mudo: era la batería

Veníamos con el BNO055 devolviendo **0.0 en los tres ángulos** y sin recuperarse. Quedó resuelto:

**El BNO055 se alimenta de la batería, no del USB.** Con la batería apagada, `bno.begin()`
devuelve OK — el chip alcanza a contestar su identificación — pero la fusión no corre y todo sale
cero. Es una trampa fea: **el programa imprime "giroscopio OK" mientras devuelve puros ceros.**

Lo del martes pasado fue eso: en el enredo del cable se cortó la batería.

**Arreglo:** el sketch ahora vigila el sensor todo el tiempo, no sólo al arrancar. Si se queda
mudo, corta motores y avisa. El código 2025 no tiene nada de esto: si el BNO falla al arrancar se
cuelga en un `while(1)` mudo.

---

## 2. El lazo cerrado funciona

### Giros

| | Error |
|---|---|
| Por cronómetro (martes pasado) | pedíamos 90°, hacía **160°** |
| Con giroscopio | **0,1°** y **−1,5°** |

El primer giro **se pasó 25° y volvió solo**. Eso es lo que el cronómetro no puede hacer.

**El giroscopio cuenta positivo en sentido horario** — medido girando el robot a mano: 359,9° →
99,7°. Y los dos signos que había que descubrir (`giroInvertido`, `correccionInvertida`) salieron
**los dos en `false`**. No hace falta volver a buscarlos.

### Rectas: proporcional no alcanzaba, hubo que acumular

Con corrección sólo proporcional (KP=3):

| | Desvío |
|---|---|
| Máximo | 11,9° |
| **Final** | **7,2°** |

Corregía — el número bajaba — pero nunca terminaba de enderezarse. **Hay algo que empuja siempre
para el mismo lado** (una rueda que tira más, o roza), y con corrección proporcional eso termina
en empate: corrige lo justo para no torcerse más, y se queda ahí.

Agregando corrección acumulada (KP=5, KI=1.5, con tope anti-windup):

| | Desvío |
|---|---|
| Máximo | 5,2° |
| **Final** | **−1,8°** |

Fijate el signo: pasó de +7,2 a **−1,8**. Cruzó el cero, o sea que se enderezó del todo y se pasó
apenas. Menos de 2 grados.

---

## 3. Los motores tumban al giroscopio cuando el robot se mueve de verdad

Aparecieron caídas del sensor **girando**. Pruebas para aislar la causa:

| Situación | Motores | ¿Se movía? | Giroscopio |
|---|---|---|---|
| Ruedas al aire | 3, más de 30 s | no | ✅ aguantó |
| Agarrado en la mesa, con fuerza | 3, 30 s | no | ✅ aguantó |
| En el piso | 3, ~2 s | **sí** | ❌ mudo |
| En el piso, después de cortar la batería | 3, 5 giros | sí | ❌ mudo |

**Hacer andar los motores por sí solo NO lo tumba.** Se descarta el ruido de los drivers como
causa única.

### El arranque suave ayudó muchísimo

Los tres motores pasaban de quietos a fondo de golpe. Poniendo una rampa de ~300 ms:

| | Aguantó |
|---|---|
| Sin rampa | **5 giros** |
| Con rampa, 1ª corrida | **~10 cuadrados** (unos 40 giros), sin frenarse |
| Con rampa, 2ª corrida | ~17 giros |

### Pero la escena del crimen dice que no es el tirón inicial

El robot ahora anota qué estaba haciendo al quedarse mudo. La última vez:

- **girando**, vuelta 5
- a los **651 ms** de arrancar ese giro
- con la potencia en **38** (de 60 máximo)

La rampa termina de subir a los ~190 ms. O sea que **no murió en el tirón del arranque**, sino
mucho después y con potencia baja.

**Lo que sí explica todo:** la batería. Primera corrida 10 cuadrados, la segunda —
inmediatamente después, batería más gastada — falló a los 4. Y a la mañana, después de cortar y
volver a prender la batería, pasó de aguantar 1 giro a aguantar 5.

⬜ **Falta probarlo con la batería cargada.** Es la prueba que cierra el caso.
⬜ El arreglo de fondo es de electrónica: un capacitor junto al sensor, que le haga de reserva.

---

## 4. La cámara anda, y ve la pelota

Nunca la habíamos probado. Con `pruebas/ver-camara`:

- **~26 paquetes por segundo, cero rotos.** Enlace limpio.
- Tiene su programa adentro y **arranca sola**, sin computadora ni OpenMV IDE.
- Sigue la pelota naranja bien: **19 cm → 31 → 40** mientras la alejábamos, con el desvío pasando
  de 0 a 7 cm. Lecturas estables.

---

## 5. Calibración de distancia: el robot no puede medir, hay que medirle

No tiene encoders ni sensor de distancia. "Avanzar 30 cm" es en realidad "el tiempo que, con
regla, dio 30 cm".

Tres saltos medidos con regla, potencia 200:

| Tiempo | Distancia |
|---|---|
| 150 ms | 13 cm |
| 250 ms | 19 cm |
| 350 ms | 33 cm |

Ajustando una recta: **1 cm cada 10 ms, con ~33 ms que se pierden al arrancar.**

    distancia_cm = tiempo_ms / 10 − 3,3        →  30 cm = 333 ms

**Por qué tres saltos y no uno:** hay dos incógnitas (velocidad y el retardo de arranque). Con una
sola medición no se separan, y la regla de tres directa se equivoca — el mismo error que nos
comimos con los giros por cronómetro.

⚠️ Los datos tienen ruido: entre el 1er y 2do salto avanzó 6 cm por cada 100 ms, y entre el 2do y
3ro avanzó 14.

### Marcha atrás va mucho más rápido que marcha adelante

Con **el mismo tiempo y la misma potencia** (333 ms), retrocediendo se pasó **23 cm** de la
largada: hizo ~53 cm donde para adelante hacía ~30.

Sospecha: la ida **le pega a la pelota y la empuja**, y eso le come velocidad. La vuelta es libre.

    velocidad atrás = 53 / (333−33) = 0,177 cm/ms   →  30 cm = ~200 ms

---

## 6. El despeje anda

`funciona/despeje-pelota`: se queda vigilando, y cuando la cámara ve la pelota naranja **a 30 cm
o menos y de frente**, sale disparado 30 cm, vuelve, y se acomoda.

Probado en el piso: **funcionó bien**. Detecta, despeja, vuelve.

Detalles de diseño, y por qué:

- **Pide ver la pelota 3 veces seguidas** antes de disparar. Un solo cuadro no alcanza: cualquier
  reflejo naranja lanzaría al robot.
- **Espera 1,5 s** después de cada despeje, si no encadena despejes sin parar.
- **Se acomoda al volver** con el giroscopio: anota el rumbo antes de salir y lo recupera al
  terminar. Un salto a potencia 200 tuerce el robot, y sin esto cada despeje lo deja más ladeado.
- **Si el giroscopio no está, despeja igual** y sólo se saltea el acomodarse. No queríamos que una
  mejora bloquee la función principal.

---

## Números para no volver a medirlos

| Qué | Valor |
|---|---|
| Giroscopio | positivo = **horario** |
| `giroInvertido` / `correccionInvertida` | **false / false** |
| Error de giro con lazo cerrado | 0,1° y −1,5° |
| Corrección en recta | KP=5, KI=1,5, tope 30 |
| Desvío final en recta, 8 s | **−1,8°** |
| Avance, potencia 200 | 1 cm cada 10 ms, +33 ms de arranque |
| 30 cm adelante | **333 ms** |
| 30 cm atrás | **200 ms** |
| Rampa de arranque de giro | +3 de PWM cada 15 ms |
| Cámara | ~26 paquetes/s, cero rotos |
| Despeje dispara si | pelota ≤ 30 cm y desvío ≤ 15 cm, 3 cuadros seguidos |

---

## Qué queda pendiente

### Del despeje

- ⬜ **Verificar la vuelta con los 200 ms nuevos.** Se cargó pero no se llegó a medir.
- ⬜ 🚨 **La cámara tiene zona muerta justo adelante** — lo aprendió la otra mesa: cuando la
  pelota se le mete encima manda `Xp = 0`, que nuestro sketch interpreta como *"no la veo"*.
  **Perder de vista una pelota que tenías pegada no es perderla: es tenerla.** Hay que
  distinguir los dos casos.
- ⬜ El umbral de disparo (30 cm) y el desvío tolerado (15 cm) están puestos a ojo, sin probar
  en cancha.

### Del giroscopio

- ⬜ **Probar con la batería cargada.** Es lo que cierra el caso de las caídas.
- ⬜ Capacitor de reserva junto al sensor.

### Lo que no se arregla con giroscopio

- ⬜ **El cuadrado se corre para la izquierda vuelta a vuelta.** El giroscopio controla hacia
  dónde MIRA el robot, no dónde ESTÁ. Cada vuelta termina apuntando bien pero unos centímetros
  corrido, y eso se suma. Haría falta que el robot supiera su posición, no sólo su orientación.

### Del código 2025

- ⬜ Los umbrales de color de la cámara son de la luz del laboratorio 2025. En cancha hay que
  recalibrarlos.

---

## Cómo se trabajó

Todo el día **sin abrir el Arduino IDE**: compilar, cargar, mandar teclas y leer respuestas por
consola, con los scripts de [`../pruebas/herramientas/`](../pruebas/herramientas/).

Dos trampas que nos costaron tiempo y conviene tener presentes:

**Cargar un sketch reinicia el Teensy.** Los sketches que arrancan solos empiezan la cuenta
regresiva apenas termina la carga. Hay que cargar con la batería apagada, o frenar en el acto.

**El cable USB no llega al piso.** Por eso varios sketches se hicieron autónomos: arrancan solos
con una cuenta regresiva y **el robot se acuerda de lo que midió**, así se lo lleva al piso sin
cable y después se le pregunta con la tecla `m`. Para que eso funcione hay que traerlo **con la
batería prendida** — si se corta, se borra.
