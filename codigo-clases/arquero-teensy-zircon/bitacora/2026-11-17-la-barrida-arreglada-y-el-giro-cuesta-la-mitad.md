# 2026-11-17 (martes) — La barrida arreglada, y girar cuesta la mitad de lo que creíamos

**Quiénes:** el equipo (pruebas en cancha y **las dos observaciones que destaparon todo**)
+ Claude (lectura de bitácoras, diagnóstico, código y carga)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa nuevo:** `pruebas/velocidad-giro` (con `GIRO.bat`)
**Modificado:** `funciona/partido-0921` (la barrida)

---

## 🎯 LOS DOS HALLAZGOS DEL DÍA

**1. La barrida iba sólo a la derecha y se reiniciaba cada tantos segundos. Las dos cosas
eran el mismo bug, y estaba en el límite:** el tercer paso apuntaba a +45°, que *era* el
límite. Cualquier pasada de largo mataba la barrida antes de llegar al primer paso de la
izquierda.

**2. Girar sobre el eje arranca con 35 de PWM, no con 70.** Todo el programa viene suponiendo
*"abajo de ~70 un motor parado no arranca"* — y para **girar con las tres ruedas eso es el
doble de lo necesario**. Nadie lo había medido.

---

| | |
|---|---|
| §1 | Seis semanas de atraso, y cómo me puse al día |
| §2 | Las dos estrategias, probadas una después de la otra |
| §3 | Los "pulsitos" de la estrategia 2: qué eran de verdad |
| §4 | 🎯 **El bug de la barrida** |
| §5 | La barrida nueva: vaivén continuo de ±90° |
| §6 | Un cuadro en el barrido |
| §7 | 🎯 **`velocidad-giro`: el mínimo es 35** |
| §8 | Lo que eso cambia en tres lugares del programa |
| §9 | Mis errores del día |
| §10 | Números de hoy |
| §11 | Qué queda pendiente |
| §12 | Lo que se aprendió del método |

---

## 1. Seis semanas de atraso, y cómo me puse al día

La última vez que hablamos fue el **06/10**. Hoy es el **17/11**: seis semanas y **ocho clases**
en el medio, que yo no tenía.

⚠️ **Y mi primer intento de ponerme al día estuvo mal.** Miré sólo el repositorio local,
no encontré nada posterior al 06/10 y les dije que **no había bitácoras de esas semanas**.
El equipo tuvo que corregirme: *"tenés que leer las bitácoras de GitHub"*. Al traer del
remoto aparecieron **30 commits** y las ocho bitácoras.

**La lección, para no repetirla:** el repo local no es el repo. Antes de decir "no hay nada",
`git fetch`.

Las ocho se leyeron con **tres agentes en paralelo** (12-20/10, 27/10-03/11, 08-10/11), unas
2100 líneas. Lo más importante de lo que me enteré:

| | |
|---|---|
| **La pelota quieta oscilando** (lo que quedó abierto el 06/10) | **Resuelto: era la predicción.** Y el diagnóstico del equipo es mejor que el mío: *"No está corrigiendo brusco a la pelota. Está persiguiendo su propio giro."* La cámara va montada en el robot, 1° de giro corre la pelota 1,75 cm en la imagen a 1 m —más que la zona muerta de 1,4— y el giroscopio al enderezarlo cierra el círculo |
| **La predicción** | **Borrada entera del código** el 03/11, no apagada |
| **Mis dos arreglos del 06/10** | Nunca se probaron. El de la zona muerta quedó anotado como *"número a ojo"* — buen criterio |
| **Se volvió al programa del 21/09** | Porque la línea nueva terminó en una versión que *"despeja muchas menos pelotas que antes"*. Volver atrás fue la lectura correcta |
| **El giroscopio está soldado** | Ya no va con cinta. El problema del cable del 29/09, cerrado |
| **La Roboliga era el 11/11** | Ya pasó, y **no quedó ningún resultado anotado** en ninguna bitácora |

---

## 2. Las dos estrategias, probadas una después de la otra

Se cargó la **estrategia 1** (`funciona/partido-0921`, la **0921k**), después la **estrategia 2**
(`funciona/estrategia-2`, versión 2026-11-10d), y finalmente se volvió a la 1.

**El veredicto del equipo sobre la estrategia 2**, y la razón es buena:

> *"No funciona muy bien que digamos, prioriza el seguir la línea en vez de la pelota."*

Y es exactamente el problema de diseño de esa estrategia: su regla es *no despegarse nunca del
arco*, y para cumplirla sacrifica llegar a la pelota. **Un arquero que no llega a la pelota no
sirve aunque esté perfectamente ubicado.**

⚠️ Antes de cargarla se verificó la serie del Teensy (**13348550** = arquero; el delantero es
15708680). Si hubiera sido el otro robot, no se cargaba nada.

---

## 3. Los "pulsitos" de la estrategia 2: qué eran de verdad

El equipo reportó, de la estrategia 2:

> *"Al hacer la patrulla nueva que le pusimos hace un pulso hacia la derecha, luego otro, y
> vuelve con un gran pulso hacia el frente, y no usa la patrulla para la izquierda."*

⚠️ **La estrategia 2 no tiene patrulla ni barrida.** Lo que vieron es la **secuencia de chequeo
de punta**, que leída en el código es esto:

```
va de costado
   -> un sensor de atras deja de ver blanco
   -> CHEQUEO_PUNTA: frena y hace un PULSITO PARA ATRAS (PWM 100, hasta 250 ms)
         si vuelve a ver blanco -> "me habia corrido", sigue como si nada
         si NO vuelve a ver blanco -> es LA PUNTA del area
   -> VOLVER_ADELANTE: deshace el pulsito, para adelante   <- el "gran pulso al frente"
   -> marca ese lado como limite: NO SIGUE MAS PARA AHI
```

Y el lado bloqueado **sólo se libera cuando la pelota lo lleva 0,3 s para el otro lado**
(`MS_SOLTAR_PUNTA = 300`). De ahí el "no va para la izquierda".

### 🔴 Quedó una pregunta sin contestar, y define el diagnóstico

**¿Eso pasó en el BORDE del área (la esquina) o en el MEDIO?**

- **En la esquina** → el programa hace lo que fue diseñado para hacer.
- **En el medio** → es una **punta falsa**, y es exactamente el **problema 3** de la revisión
  del 09/11 que quedó sin arreglar: *"puntas falsas por frenar justo en el borde de la línea"*.
  Y el gran pulso al frente sería una corrección innecesaria que además lo saca de lugar
  (**problema 1**).

Si fue en el medio hay un sospechoso concreto y medible: **los tres sensores usan el mismo
umbral (425)**. Si un sensor de atrás confunde verde con blanco antes que el otro, inventa
puntas. `pruebas/calibrar-linea` lo mide en cinco minutos, y ya estaba anotado como pendiente de
esa misma revisión.

---

## 4. 🎯 El bug de la barrida

El equipo lo describió así:

> *"Va hacia la derecha pero no hacia la izquierda, y en vez de hacerlo constantemente para unos
> segundos para reiniciar."*

La barrida hacía 7 pasitos de 15°: `{+15, +30, +45, −15, −30, −45, 0}`, frenando y mirando
200 ms en cada uno. Y la causa de los **dos** síntomas está en una sola línea:

```c
if (fabs(rel) > GRADOS_MAX_BARRIDA + GRADOS_MARGEN_BARRIDA) return GIRO_FUERA_DE_LIMITE;
//                      45          +        15           =  60
```

🎯 **El tercer paso apuntaba a +45°, que ERA el límite.** Girando a PWM 80 con una tolerancia de
3° es normal pasarse unos cuantos grados; al cruzar los 60 el programa daba
`GIRO_FUERA_DE_LIMITE`, llamaba a `cortarBarridaYEnderezar()`, se enderezaba y **esperaba 7
segundos** antes de reintentar.

O sea que moría **justo antes del cuarto paso, que era el primero de la izquierda**. Hacía los
tres de la derecha, se cortaba, y volvía a empezar. Las dos cosas que vieron eran el mismo bug.

⚠️ **Nota de método:** primero culpé al salto de 60° entre +45 y −15 (la inercia disparando el
chequeo de "giró para el lado contrario"). Esa era una hipótesis razonable y **había dos
candidatos en el código**; leer `girarHaciaObjetivo()` completa lo definió. Vale la pena anotar
que no era obvio cuál de los dos era.

---

## 5. La barrida nueva: vaivén continuo de ±90°

Pedido del equipo: *"sacale los pulsitos y que lo haga sin parar hasta llegar a los 90 grados, y
así también para la izquierda"*.

| | Antes | Ahora |
|---|---|---|
| Recorrido | 7 pasitos de 15° hasta ±45 | **de corrido hasta ±90** |
| Paradas para mirar | 200 ms por paso (500 en el 0) | **ninguna** |
| Pasos | `{1,2,3,-1,-2,-3,0}` | **`{+6,-6}`** (×15° = ±90) |
| `GRADOS_MAX_BARRIDA` | 45 | **90** |
| `GRADOS_MARGEN_BARRIDA` | 15 | **30** |
| Freno en las puntas | — | **250 ms** |

**Tres cosas sobre esos números:**

**🚨 El margen de 15 → 30 es EL ARREGLO DEL BUG, no un ajuste de gusto.** El objetivo tiene que
quedar lejos del límite. Si hubiera puesto el barrido en ±90 dejando el límite en 90+15, habría
**reproducido el mismo bug en el borde nuevo**. Ahora el objetivo es ±90 y el límite real son
120: quedan **30 grados de aire** para pasarse de largo. Y sigue protegiendo de lo que importa
—que alguien lo levante o lo empuje— porque pasarse 30° de largo girando solo no pasa.

**El freno de 250 ms en las puntas no es una parada para mirar.** Al llegar a +90 tiene que
girar para el otro lado, y si arranca con la inercia encima el rumbo sigue yendo para el lado
viejo un instante — y `girarHaciaObjetivo()` lee eso como `GIRO_AL_REVES` y puede cortar la
barrida otra vez. Un cuarto de segundo mata la inercia. Desde afuera sigue siendo de corrido:
90 grados seguidos, un pestañeo, y 180 seguidos para el otro lado.

**No hizo falta tocar cómo se mira la cámara:** `BARRIDA_GIRAR` ya la miraba **en cada vuelta
del lazo mientras gira**. Eso ya estaba bien hecho.

La implementación fue mínima a propósito: se cambió la lista de pasos, dos constantes, y **una
línea** en `BARRIDA_MIRAR` (su espera pasó de los 200/500 ms de "mirar" a los 250 del freno).
No se reestructuró la máquina de estados — es el programa de partido.

---

## 6. Un cuadro en el barrido

Pedido del equipo: `CUADROS_PARA_VER_BARRIDA` de **3 → 1**.

Y es coherente con el cambio anterior: girando de corrido, pedir 3 cuadros seguidos era muy
exigente. **La cámara ve la pelota en ~45% de los cuadros girando contra el 98% frenado**
(medido por la otra mesa el 29/09). Para eso existían los pasitos.

⚠️ **El riesgo es real: con 1 cuadro, un reflejo naranja suelto alcanza para que salga para ese
lado.**

🎯 **Lo que lo hace aceptable, y estaba ya en el código:** un error acá **no termina en un
despeje malo**. Al ver la pelota el robot vuelve a 0° y el despeje **junta sus cuadros de nuevo
desde cero** (`vecesSeguidas = 0` en `llegarACeroBarrida`), justamente para no despejar nunca
torcido. Así que un reflejo cuesta **un viaje al vacío, no una pelota pateada para el lado
equivocado**.

El despeje sigue pidiendo sus propios cuadros (`VECES_PARA_CREERLE`), sin tocar.

---

## 7. 🎯 `velocidad-giro`: el mínimo es 35

Pedido del equipo: un programa de consola que gire sobre el eje y con teclas poder subir y bajar
la velocidad, para encontrar la mínima.

Se le agregó **una sola cosa** que no estaba pedida, y era el punto: **que mida los grados por
segundo con el giroscopio**, así `>> GIRA` o `>> zumba, no gira` lo decide un número y no el
ojo. Y lleva solo la cuenta del mínimo **para cada sentido por separado**.

```
  fuerza (PWM) : 75        sentido: DERECHA
  estado       : GIRANDO
  grados/seg   : 42.3   >> GIRA
  🎯 LA FUERZA MINIMA A LA QUE GIRO:
       a la derecha  : 35   (ahi hacia 18.4 grados/seg)
       a la izquierda: 35
```

Teclas: **espacio** girar/parar, **s/a** ±5, **S/A** ±1, **d** sentido, **0** borrar, **?** ayuda.
Arranca **parado**. Gira sobre su eje, **no se traslada**.

### EL RESULTADO: **35 para los dos lados**

Y es **la mitad** de lo que todo el programa venía suponiendo.

**Por qué:** el famoso *"abajo de ~70 un motor parado no arranca"* (medido el 01/09) es de un
motor **solo**. Girando sobre el eje **las tres ruedas empujan juntas en el mismo sentido**, así
que los tres torques se suman y a cada rueda le alcanza con mucho menos.

> **Girar en el lugar es el movimiento MÁS FÁCIL que tiene este robot — y nadie lo había
> medido.**

Que diera **igual para los dos sentidos** también es buena noticia: no hay una rueda floja ni un
motor más fuerte.

---

## 8. Lo que eso cambia en tres lugares del programa

| Dónde | Hoy | El mínimo real | Cuántas veces de más |
|---|---|---|---|
| `PWM_BARRIDA_TRES` (la barrida) | 80 | 35 | **2,3×** |
| `PWM_MIN/MAX_ACOMODO` (enderezarse) | 75 / 85 | 35 | **2,1 / 2,4×** |
| El enderezado del despeje | 70-85 | 35 | 2 a 2,4× |

🎯 **Y eso ataca de frente el riesgo que nos quedó con el barrido continuo.** El peligro era
barrer por delante de la pelota sin verla, porque girando la cámara ve mucho peor. **Girar más
despacio da más tiempo para mirar cada pedazo de cancha** — sin tocar la cámara ni los cuadros.

**Propuesta, SIN APLICAR todavía: `PWM_BARRIDA_TRES` de 80 a 55.** Es 1,6 veces el mínimo
—margen de sobra para no trabarse si una rueda roza o la batería baja— y hace el barrido casi la
mitad más lento. **No se pone en 35**: correr un lazo justo en el mínimo es pedir que se trabe
con cualquier variación.

Y si eso funciona, probablemente se pueda **volver `CUADROS_PARA_VER_BARRIDA` a 2** —más seguro
contra reflejos— porque girando despacio sí va a alcanzar a juntar dos cuadros. Pero eso después,
de a uno.

⚠️ **El enderezado también tiene lugar para bajar.** El 15/09 el equipo se quejó de que
*"el giro es demasiado rápido... oscila varias veces antes de quedarse quieto"* y se bajó de 110
a 85. Con el mínimo real en 35, **85 sigue siendo 2,4 veces lo necesario**: hay bastante margen
para que deje de pasarse. Es otro número y conviene tocarlo aparte.

---

## 9. Mis errores del día

**Dije que no había bitácoras de las seis semanas.** Miré sólo el repo local. El equipo tuvo que
decirme que las buscara en GitHub, y aparecieron 30 commits. **Antes de afirmar que algo no
existe, traer del remoto.**

**Dije que la "técnica challenge" era una prueba de la Roboliga.** No: es el nombre que el
equipo le puso al programa `pruebas/tecnica-challenge` (antes `recorrido-arcos`). No hay
reglamento ni fecha asociada.

**Dije que el despeje en diagonal venía escrito y apagado desde el 08/09.** Falso: eso era
`perseguirEnElDespeje`. **La diagonal es nueva, del 27/10.** Confundí las dos cosas.

**Culpé al salto de 60° antes de encontrar el límite.** La hipótesis no era absurda —había dos
candidatos reales en el código— pero la dije antes de leer `girarHaciaObjetivo()` completa.

---

## 10. Números de hoy

| Qué | Valor |
|---|---|
| 🎯 **Fuerza mínima para girar sobre el eje** | **35**, igual para los dos lados |
| Lo que se venía suponiendo | ~70 (de un motor solo, medido el 01/09) |
| `PWM_BARRIDA_TRES` hoy | 80 — **2,3 veces el mínimo** |
| Barrido nuevo | **±90°**, de corrido |
| Límite de la barrida | 90 + 30 de margen = **120°** (era 45 + 15 = 60) |
| Freno en las puntas | **250 ms** |
| Cuadros para creerle en el barrido | **1** (era 3) |
| Ve la pelota: frenado / girando | 98% / 45% (medido 29/09 por el delantero) |
| Serie del Teensy del arquero | 13348550 |

---

## 11. Qué queda pendiente

### 🔴 Lo primero, y todo esto está cargado sin probar

- 🔴 **El barrido de ±90° en cancha.** Tres cosas: que **llegue a los 90 y cambie de lado sin
  cortarse** (si todavía se corta, el margen de 30° es poco → 40); que **80 alcance** para girar
  de corrido (ahora sabemos que sí: el mínimo es 35); y que **no salga disparado por un reflejo**
  ahora que le cree a un solo cuadro (si pasa → 2).
- 🔴 **Decidir `PWM_BARRIDA_TRES` 80 → 55** (§8). Está propuesto y **no aplicado**.
- 🔴 **Contestar si los pulsitos de la estrategia 2 fueron en el medio o en la esquina** (§3).
  De eso depende si hay una punta falsa que arreglar.

### De la estrategia 2

- ⬜ Los **tres problemas de la revisión del 09/11** siguen sin arreglar: salirse por la esquina,
  meterse al arco si lo empujan, y las puntas falsas.
- ⬜ **Medir los sensores con `pruebas/calibrar-linea`** y darle a cada uno su umbral. Hoy los
  tres usan 425.

### Lo que puede bajar ahora que sabemos el mínimo

- ⬜ **El enderezado: 75-85 → probar más abajo.** Es el arreglo de fondo de la oscilación que el
  equipo reportó el 15/09.

### Arrastres de las seis semanas (de las bitácoras leídas hoy)

- ⬜ **`TABLERO.bat` carga con Enter solo** — ya dejó al robot con el programa del tablero el
  10/11 ("prende y no hace nada"). Sigue así, y es una trampa esperando un día de partido.
- ⬜ **`partido-0921` no escribe nada por el cable**: sin diagnóstico en cancha. El registro de
  despejes en EEPROM quedó sólo en la 0.1103, que se descartó.
- ⬜ **`MS_MEDIO_AL_MEDIO` = 2228 ms** se midió el 06/10, **antes de mover los motores**.
- ⬜ El umbral naranja de la cámara **79 → 78**, cambiado sólo en una PC y sin subir.
- ⬜ **Anotar los resultados de la Roboliga del 11/11** y de los partidos del 8, 9 y 10. No
  quedó ninguno escrito.

---

## 12. Lo que se aprendió del método

**1. El repo local no es el repo.** Dije que seis semanas de trabajo no existían porque no hice
`git fetch`. Un "no hay nada" es una afirmación fuerte y hay que ganársela.

**2. Un objetivo pegado a su límite es un bug esperando.** El tercer paso apuntaba exactamente
al límite, y pasarse de largo es lo normal en cualquier cosa que se mueva por inercia. **Entre
el objetivo y el límite tiene que haber aire** — y cuánto, se calcula con cuánto se puede pasar
de largo, no a ojo.

**3. Un número heredado puede estar al doble.** El "~70" se arrastró meses en comentarios que lo
citaban como justificación, y era de **un motor solo**: para girar con tres, 35 alcanza.
**Medir lo que se da por sabido pagó el doble hoy** — arregló un número y de paso resolvió el
riesgo de la cámara en el barrido.

**4. Cuando dos síntomas raros aparecen juntos, sospechar de UNA causa.** "Va sólo a la derecha"
y "se reinicia cada tantos segundos" parecían dos problemas. Era uno.

**5. Medir antes de opinar, también en lo chico.** La herramienta de hoy existe sólo para que
"giró" o "zumbó" deje de ser una impresión. Costó veinte minutos y dio el número que más valor
tuvo del día.
