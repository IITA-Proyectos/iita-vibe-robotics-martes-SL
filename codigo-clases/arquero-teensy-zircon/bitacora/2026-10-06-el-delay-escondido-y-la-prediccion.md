# 2026-10-06 (martes) — El `delay` escondido, y la predicción que sí estaba

**Quiénes:** el equipo (pruebas en cancha, y **dos diagnósticos que resolvieron el día**)
+ Claude (código, carga y lectura por USB)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programas nuevos:** `pruebas/ir-al-medio`, `pruebas/ver-prediccion`
**Modificados:** `pruebas/medir-ancho`, `funciona/seguir-y-despejar`

---

## 🎯 EL RESUMEN DEL DÍA

Tres cosas, y las tres salieron de preguntas del equipo:

**1. Se midió el ancho de la cancha, limpio y dos veces.** Pero antes hubo que encontrar por
qué el robot zigzagueaba — y la causa era **un `delay(200)` escondido adentro de una función
de la librería de Adafruit**, que yo había metido en el lazo de control la semana pasada sin
saberlo. El robot manejaba **a ciegas la mitad del tiempo**.

**2. Se escribió `ir-al-medio`**, idea del equipo: volver al centro del arco usando las dos
líneas perpendiculares de la cancha para saber dónde está en los dos ejes antes de cronometrar.
Funciona.

**3. Se destapó que la predicción de la pelota estaba trabajando desde el 08/09... pero tan
poquito que no se veía.** Y arreglarla destapó que había **tres números peleados entre sí**.

---

| | |
|---|---|
| §1 | El zigzagueo: tres intentos míos, todos equivocados |
| §2 | 🎯 **La causa: `getSystemStatus()` termina con `delay(200)`** |
| §3 | Y el piso de corrección daba vuelta las ruedas |
| §4 | 🎯 **El ancho, medido limpio dos veces** |
| §5 | La asimetría que se da vuelta, y qué significa |
| §6 | `ir-al-medio`: volver al centro usando las líneas |
| §7 | `ver-prediccion`: la regla con dos marcas |
| §8 | 🎯 **La predicción: estaba, pero era invisible** |
| §9 | Por qué 250 ms, explicado |
| §10 | El techo de velocidad lateral, y tres números peleados |
| §11 | La pelota quieta y el robot saltando |
| §12 | Errores del día (casi todos míos) |
| §13 | Números de hoy |
| §14 | Qué queda pendiente |
| §15 | Lo que se aprendió del método |

---

## 1. El zigzagueo: tres intentos míos, todos equivocados

Se cargó `medir-ancho` con el arranque del giroscopio rehecho la semana pasada, y el robot
**zigzagueaba de costado**: se torcía, no corregía, se torcía más, y cuando por fin corregía se
pasaba. El equipo lo describió como *"oscila poco al principio y después mucho"*.

Oscilación que **crece** es la firma de un lazo de control inestable, y yo salí a cazarla por el
lado de las ganancias. Tres intentos, todos fallidos:

| Intento | Qué hice | Resultado |
|---|---|---|
| 1 | `VEL_MEDIR` 100 → **160** (teoría de la zona muerta) | arregló el zigzagueo del arranque, **pero el robot llegaba al borde y chocaba contra la madera** |
| 2 | Agregar que se endereza después del golpe (fase `ENDEREZANDO`) | correcto, pero no era la causa |
| 3 | `PWM_MIN_CORRECCION` = **30**, después **70** | ninguno cambió nada; el 70 empeoró |

En el medio también bajé la velocidad a 120 y el equipo me corrigió: **"160 sí arregló el
zigzagueo del arranque"**, así que la volví a subir. Y después fueron ellos los que apuntaron
en la dirección correcta:

> *"160 es mucho, lo que habría que subir sería el piso de corrección o la velocidad mínima."*

Tenían razón en el razonamiento —el problema no era que el robot fuera lento, era que la
corrección no tenía fuerza— pero el piso tampoco lo arregló. **Y eso, en retrospectiva, era la
pista más importante del día:** subir ganancias, poner pisos y ensanchar la zona muerta son
**tres perillas de ganancia**, y ninguna hizo nada.

> **Cuando tres perillas de ganancia no cambian nada, el problema no es de ganancia.**

---

## 2. 🎯 La causa: `getSystemStatus()` termina con `delay(200)`

Lo encontró una revisión de tres agentes comparando `medir-ancho` contra el programa de juego,
que mueve el robot de costado todo el partido **sin zigzaguear**. Dos de las tres lentes
llegaron a lo mismo por caminos distintos.

En `Adafruit_BNO055.cpp`:

```c
void Adafruit_BNO055::getSystemStatus(...) {
  ...
  *system_error = read8(BNO055_SYS_ERR_ADDR);
  delay(200);          // ← acá
}
```

**Y el 29/09 yo puse esa llamada adentro de `rumboActual()`, cada 400 ms**, para preguntarle al
chip si su fusión seguía corriendo. Consecuencia:

> El robot manejaba **a ciegas 200 de cada 400 milisegundos** —la mitad del tiempo, unas 11 veces
> por cruce— con la última corrección **clavada en los motores**, porque el PWM lo sigue
> generando el hardware mientras el programa duerme.

Eso da las dos mitades exactas del síntoma: **se tuerce sin corregir** (la ventana ciega) y
**cuando corrige se pasa** (la corrección queda aplicada 200 ms sin control). Y explica por qué
las tres perillas de ganancia no sirvieron: **el problema era el tiempo en que nadie mira.**

### El dato que cerró el caso

Los agentes miraron la versión commiteada de antes de hoy (`git show HEAD`). Tenía el lazo
**idéntico** al del programa de juego —mismas ganancias, misma mezcla, misma rampa, sin piso— y
**ya zigzagueaba**. Con el lazo igual y el síntoma presente, la causa no podía estar en el lazo.

### El arreglo

La misma información se saca leyendo el registro **0x39** (SYS_STATUS) a mano, en un par de
milisegundos y sin dormir a nadie. El programa de juego nunca llamó a `getSystemStatus()`: su
`rumboActual()` es sólo `bno.getEvent()`, y por eso siempre anduvo bien.

⚠️ Y hay una consecuencia que vale anotar: **el `ENDEREZANDO` que agregué hoy también llamaba a
`rumboActual()`**, así que también se congelaba 200 ms girando en el lugar a 75-85 de PWM. Por
eso ese arreglo tampoco enderezaba nada.

---

## 3. Y el piso de corrección daba vuelta las ruedas

El `PWM_MIN_CORRECCION = 70` no sólo no servía: **era activamente malo**, y la cuenta es de
tres líneas.

La corrección de rumbo se suma a las tres ruedas. De costado, con la mezcla 50/50/89 y velocidad
100, las dos de adelante reciben **50**. Entonces:

```
yendo a la derecha, ruedas de adelante en 50
si la correccion pide c = -70   ->   50 - 70 = -20
```

⚠️ **Las ruedas de adelante se daban vuelta.** Con un piso de 70 la corrección deja de ser un
ajuste y pasa a ser un interruptor que **destruye el movimiento lateral** cada vez que dispara.

Y la prueba de que el piso nunca hizo falta: **el programa de juego no tiene piso** —devuelve el
valor crudo, sea 3 o 40— y mueve el robot de costado todo el partido con las ruedas de adelante
en **30**, menos todavía que los 50 de acá.

Quedó escrito en el código, con la cuenta, para que nadie lo vuelva a intentar.

---

## 4. 🎯 El ancho, medido limpio dos veces

Con el `delay` afuera, el piso sacado y la zona muerta de vuelta en 1,0:

| | corrida 1 | corrida 2 |
|---|---|---|
| cruce izquierda → derecha | **4344 ms** | **4405 ms** |
| cruce derecha → izquierda | **4547 ms** | **4615 ms** |
| al medio desde la izquierda | **2213 ms** | **2243 ms** |
| al medio desde la derecha | **2314 ms** | **2348 ms** |
| peor desvío por tramo (grados) | 2,9 / 3,4 / 3,6 / 2,1 | 2,7 / 2,9 / 2,3 / 3,2 |

**El arreglo funcionó:** menos de 4° de desvío en los ocho tramos, contra los ~14 de antes. Y
**nunca hizo falta enderezarlo** — o sea que el golpe contra la madera nunca lo dejó torcido más
de 3°. El choque no era el problema; era el zigzagueo.

**Repetibilidad entre corridas: 1,5%** (61 y 68 ms). La medición ya es sólida.

### 🎯 Los números del 21/09 estaban bien

| | 21/09 (con el cable roto) | Hoy | Diferencia |
|---|---|---|---|
| Cruce, promedio | 4507 ms | **4446 ms** | **1,4%** |
| Al medio, promedio | 2294 ms | **2264 ms** | **1,3%** |

Yo daba por hecho que aquella medición estaba arruinada por el cable del giroscopio, y **no lo
estaba**. Bien que la repetimos: ahora lo sabemos en vez de suponerlo.

---

## 5. La asimetría que se da vuelta, y qué significa

La diferencia entre ir a la izquierda y a la derecha es del **4%** (203 y 210 ms), **tres veces
más grande que el ruido entre corridas (1,5%)**, y **para el mismo lado las dos veces**. Es real,
no es ruido. (Lo dije mal al principio y me corregí: con una sola corrida parecía casualidad.)

Pero ahora mirá esto:

| | izquierda | derecha |
|---|---|---|
| **21/09** | 2350 | 2239 |
| **Hoy** (promedio) | 2228 | **2331** |

**Son los mismos dos números, dados vuelta.** 2350 ≈ 2331 y 2239 ≈ 2228.

La explicación más probable: el 21/09 el robot estaba apoyado **mirando al otro arco**. La
izquierda del robot apuntaba al otro costado de la cancha, así que las etiquetas se invirtieron.

Y eso contesta una pregunta que ni habíamos hecho: **¿la asimetría es del robot o de la cancha?**

- Si fuera **del robot** (una rueda más floja, un motor más fuerte), el tiempo de *su* izquierda
  a *su* derecha sería el mismo siempre, dé para donde dé. **No se daría vuelta.**
- Si fuera **de la cancha** (una leve pendiente, un lado más áspero), **se da vuelta** al girar
  el robot. Es lo que pasó.

⬜ **Y hay una prueba que lo define en un minuto:** correr `medir-ancho` con el robot mirando al
otro arco. Si el lado lento sigue a **la cancha**, es la cancha. Si sigue al **robot**, es el
robot. Importa de verdad, porque si es la cancha, **al cambiar de lado en el entretiempo los dos
números se dan vuelta.**

---

## 6. `ir-al-medio`: volver al centro usando las líneas

**Idea del equipo, y es buena por una razón de fondo:** el robot no tiene encoders, así que
sólo sabe cuánto *tiempo* estuvo moviéndose. Y un tiempo sirve únicamente si se sabe **desde
dónde** se empezó a contar. Los 2228 ms valen si el robot arranca pegado al lateral; desde
cualquier otro lado, lo dejan en cualquier lado.

La solución: usar **las dos líneas perpendiculares** para fijar la posición en los dos ejes, y
recién entonces cronometrar.

```
0. si arrancó sobre una línea, se despega        (sólo si hace falta)
1. de costado hasta la línea LATERAL             -> fija el ANCHO
2. se aleja un poquito del lateral               -> ver (*)
3. retrocede hasta la línea del FONDO            -> fija la PROFUNDIDAD
4. avanza 50 cm                                  -> sale del área, que no sea falta
5. vuelve al MISMO lateral                       -> ver (**)
6. al medio por TIEMPO (2228 ms)                 -> centrado a lo ancho
7. retrocede hasta la línea del ÁREA             -> como arranca el juego
8. un poquito adelante (10 cm)                   -> no quedar sobre el blanco
```

**(\*) ¿Por qué alejarse antes de retroceder?** Porque los sensores que ven el blanco son los de
**atrás**, y si el robot quedó apoyado sobre la línea lateral ya están viendo blanco:
"retroceder hasta la línea del fondo" terminaría al instante sin haberse movido. Hay que
despegarse para que "ver blanco" signifique **una sola cosa**.

**(\*\*) ¿Por qué volver al lateral si ya había estado?** Porque retroceder y avanzar pueden
haberlo corrido de costado unos centímetros. El cronómetro del paso 6 arranca **desde el
lateral**, así que hay que estar seguros de estar ahí. Es barato y saca toda la duda.

**El paso 0 es lo que permite apoyarlo en cualquier lado de la cancha**, incluso encima de una
línea, que fue un pedido explícito del equipo.

### Las dos velocidades de costado, y no es un descuido

| Pasos | Velocidad | Por qué |
|---|---|---|
| 0, 1, 2, 5 | **110** | terminan **cuando ven la línea**: la velocidad cambia cuánto tardan, **no dónde paran** |
| **6** | **100** 🚨 | termina **por tiempo**, y los 2228 ms se midieron a 100 |

Esa distinción —moverse *hasta ver algo* contra moverse *por tiempo*— es la que decide si un
número se puede tocar o no.

### Los números, y cuál es de fe

| | Valor | De dónde sale |
|---|---|---|
| Avance del paso 4 (50 cm) | **943 ms** a potencia 110 | ⚠️ regla de tres |
| Alejarse (paso 2) | **400 ms** | ⚠️ no sabemos a cuántos cm equivale |
| Al medio (paso 6) | **2228 ms** | ✅ medido hoy, dos veces |
| Salir de la línea (paso 8) | **215 ms** = 10 cm a 110 | ⚠️ regla de tres |
| Tope de retroceso | **1200 ms** | ✅ del programa de juego |

⚠️ **La única calibración con regla que tenemos es a potencia 200** (1 cm cada 10 ms, más 33 ms
de arranque, medido el 04/08). Los avances de este programa van a **110**, así que sus tiempos
salen de suponer que la velocidad sube parejo con el PWM — y **no sube parejo**: pesa el roce, y
más cuanto más lento se va. Lo más probable es que **se queden cortos**.

⬜ **Se arregla en dos minutos:** cinta en el piso donde arranca, correr la rutina, y medir con
la regla cuánto avanzó de verdad en el paso 4. De paso queda la conversión ms/cm a potencia 110,
que hoy no tenemos y sirve para cualquier otro movimiento.

**Probado y funciona.** Queda pendiente decidir dónde se implementa (ver §14).

---

## 7. `ver-prediccion`: la regla con dos marcas

La predicción está escrita desde el 08/09 y **nunca se probó**, por un motivo concreto: el
programa de juego no tiene consola, así que la predicción trabaja a ciegas y desde afuera no hay
forma de saber si acierta, si llega tarde, o si apunta para el lado contrario.

🚨 **El riesgo que este programa viene a descartar:** si el **signo** de la velocidad estuviera
invertido, la predicción apuntaría **detrás** de la pelota y sería **peor** que no predecir. Esa
cuenta nunca se verificó contra la realidad.

```
izq ----------------:-----o----P---:---------------- der
    o = donde esta    P = donde se predice
    : = el pasillo de punteria (±7 cm)
```

🎯 Rodando la pelota de **izquierda a derecha**, la `P` tiene que quedar a la **derecha** de la
`o`. Si queda a la izquierda, el signo está invertido.

Muestra además: velocidad de costado y de acercamiento en cm/s, con aviso cuando está debajo del
**piso de ruido de 3,6 cm/s**; cuántos cm adelanta la predicción; y la comparación que importa,
**"¿despejaría? prediciendo SÍ / sin predecir no"**, marcada cuando las dos difieren.

Todos los números son copia exacta del programa de juego, para que lo que se vea sea lo que hace
el robot al jugar. El robot no se mueve.

El equipo lo probó y confirmó que **la cámara lee bien**.

---

## 8. 🎯 La predicción: estaba, pero era invisible

El equipo probó en cancha y reportó:

> *"Vimos que el robot no sigue lateralmente la pelota intentando adelantarse al trayecto de la
> pelota. Sigue parecido a lo que antes."*

Yo dije que el seguimiento no usaba la predicción. **Me equivoqué, y en un minuto me corregí
mirando el código:** sí la usa, desde el 08/09.

```c
// 2026-09-08: ahora se sigue ADONDE VA A ESTAR la pelota, no donde esta.
float desvio = desvioPredicho();
```

### Entonces por qué parecía igual

Por la cuenta. El adelanto es `velocidad × 0,25 s`:

| Velocidad de la pelota | Cuánto se adelantaba |
|---|---|
| 5 cm/s (rodando despacio) | **1,2 cm** — ¡menos que la zona muerta de 1,4! |
| 15 cm/s | 3,8 cm |
| 35 cm/s (lo más rápido medido) | 8,7 cm |

Con pelota normal el adelanto eran **dos o tres centímetros**. Estaba prediciendo — pero tan
poco que a ojo no se veía. Y con pelota lenta el adelanto **caía dentro de la zona muerta y
desaparecía del todo**.

🎯 **Y lo que el equipo pidió ya estaba en la cuenta.** Lo dijeron así:

> *"Si la pelota va lenta tiene que ir siguiéndola igual que el movimiento de la pelota, y si va
> rápido que vaya más adelantado."*

Eso es exactamente lo que hace `velocidad × tiempo`: con pelota lenta adelanta poco y con pelota
rápida adelanta mucho, **solo**. No hacía falta lógica nueva. **Lo que faltaba era ganancia.**

### El cambio: dos anticipaciones distintas

**Seguir y disparar no quieren el mismo adelanto.**

- **Para disparar**, 250 ms es el número correcto: es lo que el robot tarda en reaccionar, así
  que sale hacia donde la pelota va a estar cuando él llegue. Más sería pasarse de largo.
- **Para seguir**, el robot está *siempre* atrás: la pelota se mueve y él la persigue, y nunca
  termina de alcanzarla. Ahí conviene adelantarse más.

Entonces: `msAnticipacionSeguir = 500` para el seguimiento, y `msAnticipacion` para la decisión
del despeje — que a pedido del equipo, al final del día, quedó en **0** (ver §14).

---

## 9. Por qué 250 ms, explicado

Pregunta del equipo. La respuesta estaba escrita en el código:

```c
// Cuanto adelanto. Es EL TIEMPO QUE TARDA EL ROBOT EN REACCIONAR, sumando:
//    ~115 ms  -> los 3 cuadros seguidos que exige antes de creerle
//    ~135 ms  -> arrancar los motores y que la rampa suba
// ⚠️ ESTIMADO.
```

**La idea de fondo no es adivinar el futuro de la pelota: es compensar lo que el robot tarda en
moverse.** Entre que la pelota entra en el pasillo de tiro y que el robot va de verdad hacia
ella pasa un rato, y en ese rato la pelota siguió viajando. Si apuntás a donde está, llegás a
donde estaba.

Así que la pregunta no es *"¿cuánto quiero adelantarme?"* sino **"¿cuánto tardo yo?"** — y a ésa
se la puede contestar con una suma:

| | |
|---|---|
| **~115 ms** | los **3 cuadros seguidos** que exige antes de creerle. La cámara manda ~26 por segundo (38 ms cada uno), y tres son 115. Ese requisito existe para que un reflejo naranja no lo dispare, y **se paga en tiempo** |
| **~135 ms** | arrancar los motores (**33 ms** de retardo mecánico, medido el 04/08) más lo que la rampa tarda en subir |
| **= 250 ms** | desde "decido" hasta "voy en serio" |

Y de ahí salen los dos síntomas de que esté mal, que es lo más útil de todo:

- **Queda del otro lado de la pelota** (se pasó) → el número está **alto**
- **Llega atrás, como antes** → el número está **bajo**

**El síntoma te dice para qué lado moverlo.** No hay que adivinar.

Con pelota lenta nada de esto importa: a 5 cm/s, 250 ms son 1,2 cm, menos que el ruido. Recién
se nota con pelota rápida: a 35 cm/s son **8,7 cm**, más que todo el pasillo de tiro (±7). Ahí
la predicción decide si despeja o no.

---

## 10. El techo de velocidad lateral, y tres números peleados

Otra pregunta del equipo: **"¿y el seguimiento lateral tiene un tope de velocidad?"**

Sí: `pwmMaxLateral = 120`, con piso de 60. Y la cuenta de cuándo satura:

```
120 / kpLateral = 120 / 11,5 = 10,4 cm de desvio
```

🎯 **Con la pelota a más de 10 cm de desvío, el robot ya iba al tope.** De ahí para arriba el
número no cambiaba nada: 10 cm y 30 cm daban lo mismo.

**Y eso significa que subir la anticipación de 250 a 500 ms NO cambiaba nada con pelota rápida**,
porque el desvío ya pasaba los 10,4 cm y la fuerza ya estaba recortada. La predicción cambiaba
*hacia dónde* mira, pero el robot **no podía ir más rápido**.

Los dos números estaban peleados: **mover uno solo no alcanzaba.**

Se subió el techo a **150** (satura a 13 cm). El reparto con la mezcla 50/50/89:

| | |
|---|---|
| Las dos de adelante | **75** — justo arriba del piso de arranque de ~70 |
| La de atrás | **133** |
| Más la corrección de rumbo (hasta 100) | 233, debajo de 255 ✓ |

⚠️ El costo: **más velocidad es más envión**, y al llegar al desvío cero se pasa de largo. Si
oscila de lado a lado alrededor de la pelota, bajar a 135 o 125.

---

## 11. La pelota quieta y el robot saltando

Después del cambio, el equipo reportó:

> *"Ahora cuando la pelota está quieta el robot va de lado a lado. ¿Será que el piso de velocidad
> está muy alto? ¿Y corrige demás?"*

**Dos causas distintas del mismo síntoma, y las dos eran mías.**

### Causa A: el ruido amplificado por la anticipación

El ruido de la cámara con la pelota parada llega a **3,6 cm/s** (medido el 08/09). Con la
anticipación en 500 ms:

```
3,6 cm/s × 0,5 s = 1,8 cm
```

**Y `ZONA_MUERTA_PELOTA` es 1,4 cm.** O sea que el ruido **se pasaba de la zona muerta**: el
robot creía que la pelota se movía, la perseguía, el ruido cambiaba de signo, y la perseguía
para el otro lado.

Con los 250 ms de antes el ruido daba 0,9 cm y quedaba **adentro** de la zona muerta. **Subir la
anticipación lo sacó afuera.** No corregía de más: se le estaba amplificando el ruido.

🎯 **Y el arreglo ya estaba escrito en el programa:** `PISO_VELOCIDAD = 4.0`, puesto justo apenas
arriba de esos 3,6 cm/s... pero **se usaba sólo para armar la diagonal del despeje**, no para la
predicción. Ahora se aplica también ahí: debajo del piso, la velocidad se toma como **cero** y la
predicción no adelanta nada.

### Causa B: con el piso en 60 no existe la corrección suave

La otra mitad, y la intuición del equipo apuntaba justo acá:

```
desvio de 1,5 cm  ->  1,5 × 11,5 = 17  ->  se sube a 60
desvio de 5,0 cm  ->  5,0 × 11,5 = 58  ->  se sube a 60
```

**El robot pegaba el mismo salto por 1,5 cm que por 5 cm.** Con la pelota quieta y el ruido
entrando y saliendo de la zona muerta, eso es exactamente un robot saltando de lado a lado.

Se bajó `pwmMinLateral` de **60 a 45**.

⚠️ El riesgo: el 60 estaba ahí porque abajo de eso *"no se mueve, solo zumba"*. Con 45 la rueda
más cargada recibe 40 — justo el piso de **rodadura**. Moviéndose alcanza; **desde parado puede
no arrancar**. Si zumba sin moverse con desvíos chicos, subir de a 5.

⚠️ **Aclaración honesta:** `pwmMinLateral` **nunca se cambió hoy**, venía en 60 de antes. Cuando
el equipo pidió "bajalo al que tenía antes" puede que se refiriera a otro número — quedó
anotado en el código y hay que confirmarlo.

### 🔴 Y NO ALCANZÓ: lo probaron y sigue pasando

Con los dos arreglos cargados, el equipo reportó:

> **"Cuando la pelota está quieta sigue corrigiendo de un lado hacia otro."**

Así que el ruido amplificado y el piso de 60 **no eran toda la causa**, o no eran la causa.
Queda abierto, y pensándolo aparece un mecanismo que no habíamos considerado:

**El seguimiento lateral es un lazo de POSICIÓN, y el robot se mueve dentro de él.** Cuando el
robot va a la derecha, la pelota **pasa a verse más a la izquierda** — porque la cámara va
montada en el robot. Entonces el desvío cambia de signo *por el movimiento del propio robot*, no
porque la pelota se haya movido. Eso es un lazo cerrado que puede oscilar **con la pelota
perfectamente quieta y sin nada de ruido**.

Y hay dos cosas que lo hacen oscilar en vez de converger:

**1. No puede hacer una corrección chica.** Con `pwmMinLateral` en 45, cualquier desvío que pase
la zona muerta recibe un empujón de 45. No existe "acercarse despacito".

**2. Cuando llega al centro, no frena: se suelta.** En la zona muerta el código hace `parar()`,
que **sólo corta la corriente** — el robot sigue de largo por inercia. Así que entra a la zona
muerta ya con velocidad, la cruza, y sale del otro lado con desvío suficiente para recibir otro
empujón de 45. Para el otro lado. Y de nuevo.

**O sea que la zona muerta tiene que ser más ancha que lo que el robot patina al soltarse** — y
1,4 cm es poquísimo.

⬜ **Dos arreglos propuestos, sin probar** (ver §14):

- **`frenar()` en vez de `parar()` al entrar en la zona muerta.** `frenar()` cortocircuita los
  motores y el robot se detiene de verdad. 🎯 Hay precedente: el 15/09 el equipo pidió
  exactamente este cambio para el enderezado, por el mismo motivo — el robot se pasaba de largo.
- **Ensanchar `ZONA_MUERTA_PELOTA`** de 1,4 a 3 cm. Es la misma lección que aprendimos hoy con la
  corrección de rumbo: **si hay un piso, la zona muerta tiene que ser más ancha**, o el robot
  salta entre "nada" y "el empujón mínimo" para siempre.

---

## 12. Errores del día (casi todos míos)

Hoy me equivoqué bastante, y vale anotarlo todo porque el patrón enseña más que los aciertos.

**El `delay(200)` que yo mismo metí el 29/09.** Causó el zigzagueo y nos costó media clase de
hoy. Lo puse para *mejorar* la detección del giroscopio caído, sin leer qué hacía la función de
la librería por dentro. **Agregar una llamada a una librería es agregar todo lo que esa llamada
hace, no sólo lo que uno quería.**

**Tres intentos de arreglo, los tres equivocados.** Velocidad a 160 (que además hizo chocar al
robot), piso de corrección en 30, y después en 70 (que daba vuelta las ruedas de adelante). Los
tres son **perillas de ganancia**, y el problema era de **retardo**. La señal de que iba mal
estaba a la vista desde el segundo intento: **nada cambiaba**.

**Me desdije demasiado rápido.** Cuando el equipo contó del choque, abandoné la teoría de los
160 en el mismo mensaje — y resultó que esa parte **sí** había funcionado. Tuvieron que
corregirme ellos. Una observación nueva no invalida automáticamente lo anterior.

**Dije que el seguimiento no usaba la predicción, y era falso.** Me corregí en un minuto
mirando el código, pero lo afirmé antes de verificarlo.

**Dije que la asimetría del 4% era ruido**, con una sola corrida. Con dos quedó claro que se
repite y es real.

**Casi reporté un bug que no existía.** Iba a decir que el término acumulado del control daba un
salto después de una caída del sensor; fui a mirar y el tope de `dt` ya estaba puesto. **Un bug
inventado manda a arreglar lo que no está roto.**

**Rompí un string partiéndolo en dos líneas**, y tuve **dos errores del preprocesador de
Arduino** por definir funciones y enums en el orden equivocado (el IDE genera los prototipos y
los mete antes de la primera función que encuentra, así que un tipo declarado más abajo no
existe todavía). Los tres se vieron al compilar, pero son tiempo perdido.

---

## 13. Números de hoy

| Qué | Valor |
|---|---|
| Cruce de la cancha, promedio de 2 corridas | **4446 ms** |
| Al medio desde la izquierda / derecha | **2228 / 2331 ms** |
| Repetibilidad entre corridas | **1,5%** |
| Asimetría izquierda/derecha | **4%**, repetible |
| Peor desvío por tramo (antes / después del arreglo) | ~14° → **menos de 4°** |
| Diferencia con la medición del 21/09 | **1,4%** |
| `delay` escondido en `getSystemStatus()` | **200 ms**, cada 400 → **la mitad del tiempo ciego** |
| Satura el seguimiento lateral (120 / 11,5) | a **10,4 cm** de desvío → ahora **13 cm** |
| Ruido de la cámara con pelota quieta | **3,6 cm/s** → ×0,5 s = **1,8 cm** > zona muerta 1,4 |

### Lo que quedó cargado en el robot

| | Antes de hoy | Ahora |
|---|---|---|
| `pwmMaxLateral` | 120 | **150** |
| `pwmMinLateral` | 60 | **45** |
| `msAnticipacionSeguir` | (no existía: usaba 250) | **500** |
| `msAnticipacion` (despeje) | 250 | **0** — a pedido del equipo |
| `PISO_VELOCIDAD` aplicado a la predicción | no | **sí** |

🧊 **Los valores viejos están anotados en el encabezado del programa**, y la copia completa
congelada en `respaldos/estable-0.0929/`.

---

## 14. Qué queda pendiente

### 🔴 Lo primero

- 🔴 **LA PELOTA QUIETA Y EL ROBOT CORRIGIENDO DE LADO A LADO SIGUE PASANDO.** Es lo único que
  quedó abierto del día, y los dos arreglos de hoy no lo resolvieron (§11). La sospecha nueva:
  el lazo oscila solo, porque la cámara va montada en el robot y moverse cambia el desvío; y
  cuando llega al centro hace `parar()`, que suelta los motores en vez de frenarlos, así que
  patina y sale del otro lado. **Los dos arreglos a probar: `frenar()` en vez de `parar()` en la
  zona muerta, y ensanchar `ZONA_MUERTA_PELOTA` de 1,4 a 3 cm.** Uno por vez, para saber cuál fue.
- ⬜ **Probar en cancha lo que quedó cargado.** Son cinco números cambiados y ninguno probado con
  pelota rápida. Qué mirar: si **oscila de lado a lado** alrededor de la pelota, el techo de 150
  es mucho (bajar a 135). Si **se pasa de largo**, la anticipación de 500 es mucha (bajar a 400).
  Si **zumba sin moverse** con desvíos chicos, el piso de 45 es poco (subir de a 5).
- ⬜ **Decidir si el despeje vuelve a predecir.** Hoy quedó en `msAnticipacion = 0` para probar
  que "despeje siempre". Los 250 ms tienen una justificación física (§9), así que conviene
  volver a probarlos una vez que el seguimiento esté afinado.
- ⬜ **La prueba del signo de la predicción**, con `ver-prediccion`: rodar la pelota de izquierda
  a derecha y confirmar que la `P` queda a la derecha de la `o`. Nunca se verificó, y si está
  invertido la predicción empeora las cosas.

### De las mediciones

- ⬜ **La conversión ms/cm a potencia 110**, con cinta métrica. Hoy los avances de `ir-al-medio`
  son reglas de tres y probablemente se queden cortos.
- ⬜ **Medir el ancho de la cancha en centímetros.** Con eso y los 4446 ms sale **cuántos cm/s
  hace el robot de costado a velocidad 100** — un número que no tenemos y que sirve para todo,
  incluso para detectar si la batería baja lo está frenando.
- ⬜ **`medir-ancho` mirando al otro arco**, para saber si la asimetría es de la cancha o del
  robot (§5).

### De `ir-al-medio`

- ⬜ **Decidir dónde se implementa.** El lugar natural es **la patrulla sin pelota**, pendiente
  desde el 21/09 (cuando el juez sacó al robot un minuto por quedarse quieto). La idea original
  era "sin pelota 8 s → al lateral → al medio"; `ir-al-medio` **es eso, pero bien hecho**, con
  las dos líneas en vez de una sola referencia.

### De antes

- ⬜ **El heading-hold en el despeje.** El delantero midió 10,1° → 4,2°. Nuestro despeje sale a
  200 con la trasera suelta: el mismo caso. `derecho-y-vuelta` está escrito para medirlo y
  **todavía no corrió con el giroscopio sano**.
- ⬜ **`SYS_STATUS == 5` en el programa de juego** — pero leyendo el registro **a mano**, nunca
  con `getSystemStatus()`. Hoy aprendimos por qué.
- ⬜ **`INTENTOS_GIRO` de vuelta a 10** (se bajó a 1 cuando el cable estaba roto).
- ⬜ **Mover `setExtCrystalUse` antes del `delay`** en el programa de juego: un segundo regalado.
- ⬜ Probar la versión IMU (`funciona/seguir-y-despejar-imu`, compila desde el 22/09).
- ⬜ Anotar los partidos 2 y 3 del 21/09, que siguen sin quedar en ninguna bitácora.
- ⬜ Un LED que se vea (pines 9 o 10 libres).

---

## 15. Lo que se aprendió del método

**1. Cuando tres perillas de ganancia no cambian nada, el problema no es de ganancia.** Subí la
velocidad, puse un piso, ensanché la zona muerta — y el síntoma quedó idéntico. Eso **ya era la
respuesta**, y tardé en escucharla: el retardo no se arregla tocando ganancias.

**2. Llamar a una función de una librería es ejecutar todo lo que esa función hace.** El
`delay(200)` estaba a tres líneas de distancia, en un archivo que teníamos en la máquina. Nunca
lo miré. Una línea de código ajeno puede traer medio segundo de ceguera.

**3. Comparar contra algo que funciona es más rápido que teorizar.** El programa de juego mueve
el robot de costado todo el partido sin zigzaguear. La pregunta correcta no era "¿qué está mal
en mi lazo?" sino **"¿qué hace distinto el que anda?"**. Y la pista decisiva fue descubrir que
la versión de ayer tenía el lazo **idéntico** al del juego y ya zigzagueaba.

**4. Medir dos veces cambia la conclusión.** Con una corrida dije que la asimetría del 4% era
ruido. Con dos quedó claro que se repite — y comparándola con el 21/09 apareció algo que no
buscábamos: que la asimetría es probablemente **de la cancha**.

**5. A veces lo que falta no es lógica nueva sino ganancia.** La predicción hacía exactamente lo
que el equipo pedía —adelantar poco con pelota lenta y mucho con pelota rápida— desde el 08/09.
El problema era que adelantaba dos centímetros. **Antes de escribir algo nuevo, conviene
preguntarse si lo que ya está sólo necesita ser más grande.**

**6. Los números peleados entre sí no se arreglan de a uno.** Anticipación, techo de velocidad y
zona muerta se tapaban mutuamente: subir la anticipación no hacía nada porque el techo recortaba,
y el techo solo no servía porque el robot miraba a donde la pelota estaba. **Hay que ver el
conjunto o se prueba de a uno para siempre.**

**7. Las preguntas del equipo resolvieron dos cosas hoy.** "¿Lo que habría que subir no es el
piso de corrección?" apuntó en la dirección correcta. Y "¿el seguimiento lateral tiene un tope de
velocidad?" destapó el techo de 120 que estaba anulando todo lo demás. **Preguntar por un número
que nadie miró vale más que tres intentos de ajustarlo a ojo.**
