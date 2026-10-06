# 2026-10-06 — El destrabe por fin anda, la órbita deja de perder la pelota, y lo probamos: *"es bastante confiable"*

> Robot: **delantero**, Teensy 4.1 sobre placa Zircon, serie `15708680`, compilado con
> `#define ROBOT2`. Cancha de tela.

---

## 📋 RESUMEN DE LA CLASE

**Lo más importante primero: se validó en cancha, y anda.** Las cinco cosas que había que
probar dieron bien, y el equipo lo resumió así: *"es bastante confiable"*.

Eso es la **primera validación completa** de todo lo acumulado desde el 29/09: búsqueda con
giro continuo, centrado por ángulo, destrabe y los dos cambios de órbita de hoy. Hasta esta
clase, nada de eso había jugado.

Además:

1. **El destrabe por fin salta.** Tenía **tres** errores míos encadenados, y los tres eran del
   mismo tipo: condiciones de reinicio que no dejaban que la cuenta llegara nunca.
2. **La órbita dejó de perder la pelota**, y el porqué resultó ser geometría, no ajuste fino.
3. **Las tolerancias de puntería se pudieron apretar** justamente porque se arregló la órbita:
   el 15° flojo era un parche de la órbita vieja.

---

## 1. ✅ Validado en cancha

| qué se probó | resultado |
|---|---|
| encuentra la pelota rápido con el giro continuo | **sí** |
| pega el avance corto cuando se traba (3 s) | **sí** |
| mantiene la pelota mientras orbita | **sí** |
| patea con las tolerancias apretadas | **sí** |
| la pelota llega al arco con el golpe de 500 ms | **sí** |

Todavía **no jugó un partido**, así que para competir sigue valiendo la distinción del
`respaldos/README.md`: esto es *probado en cancha*, no *probado en partido*.

---

## 2. El destrabe: tres relojes mal puestos, uno atrás del otro

El equipo lo reportó **cuatro veces** (*"el avance de 1 segundo no lo hace todavía...!!!"*), y
cada vez encontré una causa distinta. Las tres eran reales y estaban apiladas.

### Error 1 — el cupo sólo se reponía al LLEGAR a la pelota

`nDestrabes = 0` estaba en un único lugar del programa: cuando el robot llega y se pone a
orbitar. Al perder la pelota y volver a buscar, **el cupo no se reponía**. Así que después de 5
destrabes sin haber llegado nunca, el destrabe **no volvía a saltar en todo el partido** — justo
el caso en que más se necesita.

Y el comentario que yo mismo había escrito decía *"hasta 5 veces **por cada ida a buscar**"*. O
sea: el comentario describía lo que yo creía haber hecho, no lo que el código hacía.

### Error 2 — el reloj contaba el tiempo escapando de la línea

`t_sinAcercarse` se ponía en cero al perder la pelota, pero **no al entrar a `!LINEA!`**. Con un
sensor leyendo blanco de más (pasó hoy, ver punto 5), el robot escapa todo el tiempo y ese rato
se sumaba como *"la veo y no me acerco"*. Se quemaba el cupo por el motivo equivocado.

Ahora el reloj se para en `cambiarA()` cuando el nuevo estado es `ESCAPA_LINEA`.

### Error 3 — el que lo rompía de verdad: un parpadeo borraba el reloj

```cpp
const unsigned long MS_GRACIA = 300;
bool laVeo = (millis() - t_ultimaPelota) < MS_GRACIA;
```

`laVeo` se vuelve falso con **300 ms** sin ver la pelota, o sea unos **13 cuadros** de los 45 por
segundo que manda la cámara. Y el 29/09 yo había puesto que al perderla el reloj vuelve a cero.

Cuando la pelota está lejos y el robot se zarandea tratando de centrarse, **la pierde de vista
un instante todo el tiempo**. Cada parpadeo le borraba la cuenta, así que no llegaba nunca a los
3 segundos.

**Arreglo:** una constante nueva, `MS_PERDIDA_REAL = 1000`. Un parpadeo corto ya no borra nada;
recién a **1 segundo** sin verla se da por perdida.

### Y lo que faltaba: *"busque de vuelta"*

Releyendo el pedido original del equipo:

> *"que se frene (mientras la ve en cualquier parte de la cámara), avance un poco, y **busque de
> vuelta**"*

Después del avance yo lo mandaba a `CENTRANDO`, no a buscar. Se quedaba peleando con la misma
pelota desde la misma mala posición. **El "busque de vuelta" me lo había salteado.** Ahora
vuelve a `BUSCANDO`.

Y un estado muerto que encontró el diagnóstico: con el cupo en `5/5` el robot se quedaba
**congelado en CENTRANDO para siempre**, viendo la pelota y sin hacer nada. Ahora, si gastó los
destrabes y sigue sin acercarse, **suelta esa pelota y vuelve a buscar**.

### 🔬 Lo que cambió el método: medir en vez de explicar

Después de tres explicaciones mías equivocadas, le agregué al programa un diagnóstico que
imprime una línea por segundo:

```
    [destrabe] Xp=87 mejor=92 reloj=2340/3000 ms  cupo=0/5  CENTRANDO
```

Con eso, **en dos minutos** quedó claro lo que tres razonamientos no habían resuelto. Primero
que el reloj ya contaba bien:

```
[destrabe] Xp=138 mejor=138 reloj=1001/3000 ms  cupo=0/5  CENTRANDO
[destrabe] Xp=138 mejor=138 reloj=2002/3000 ms  cupo=0/5  CENTRANDO
```

Después, el destrabe saltando de verdad:

```
... la veo a 145 cm y no me acerco -> avanzo a lo bruto (destrabe 2 de 5)
>>> destrabando   Xp=145 Yp=70 (a 25.8 grados)
```

Y por último el estado muerto, que nadie había pedido buscar:

```
[destrabe] Xp=131 mejor=119 reloj=7064/3000 ms  cupo=5/5  CENTRANDO
```

**El reloj pasado de largo y el cupo agotado.** Eso no se deduce leyendo: se ve.

---

## 3. 🎯 La órbita perdía la pelota por GEOMETRÍA, no por ajuste

El equipo propuso: *"idear una lógica para que no pierda la pelota en la órbita y vaya
corrigiéndose si pasa un rango de tolerancia (ya que si es exacto no va a llegar nunca)"*. La
intuición de la tolerancia era exactamente la correcta — es la misma lección de la histéresis
del centrado.

**Lo que encontramos al mirar el código:** la órbita es **completamente a ciegas**. Durante
`ORBITANDO` la cámara se usa sólo para tres *salidas* (perdí la pelota, se alejó más de 55 cm,
veo el arco alineado). El movimiento no la mira nunca:

```cpp
orbitar(sentidoOrbita, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
```

**Y acá está el fondo del problema.** La órbita traba las dos ruedas de adelante
(`VEL_ORB_FRENTE = 30`, debajo del piso de PWM a propósito) y empuja sólo con la trasera. Eso
hace girar al robot alrededor de un punto **fijo** a `R = 2*L = 17,5 cm` adelante de su centro.
La órbita se diseñó para la pelota a 17-18 cm.

**Pero `XP_ORBITA` hoy es 32.** El robot gira alrededor de un punto que está **14 cm más cerca
que la pelota**: la pelota no está en el centro del giro, y mientras el robot rota **se le pasea
por el cuadro** hasta salirse.

Y lo creamos nosotros sin darnos cuenta: **subir `XP_ORBITA` (22 → 34) arregló que chocara la
pelota el 15/09, y de paso rompió la órbita.** Las dos cosas tiran para lados opuestos, y con el
centro de giro fijo no hay valor de `XP_ORBITA` que sirva para las dos.

### Lo que se hizo

**a) Un parpadeo no tira la órbita a la basura.** Antes, cualquier 300 ms sin ver la pelota
hacía `cambiarA(BUSCANDO)`, y abandonar cuesta carísimo: vuelve a buscar/centrar/avanzar desde
cero, **y al reentrar repite el golpe de arranque de 120, que empuja la pelota**. También
re-decide el sentido. Ahora, si no la ve, **frena y espera quieto hasta 1 segundo sin salir de
`ORBITANDO`**.

Frena en vez de seguir a ciegas por un dato propio: el 29/09 medimos que con el robot quieto la
cámara ve la pelota en el **98 %** de los cuadros, y moviéndose en el **45 %**. Frenar es lo que
le da una imagen quieta para reencontrarla.

> **Detalle que parece menor y no lo es:** al volver **no se pasa por `cambiarA(ORBITANDO)`**. Se
> queda *dentro* del estado y simplemente no se llama a `orbitar()` durante la pausa. Si pasara
> por `cambiarA()` repetiría el golpe y re-decidiría el sentido — justo lo que se quiere evitar.

**b) Banda de ángulo con histéresis.** Si la pelota se corre más de `TOL_ANG_ORB_ENTRA = 18°`,
se interrumpe el empuje y se la recentra a pulsos hasta tenerla mejor que
`TOL_ANG_ORB_SALE = 9°`. Sin banda el robot oscila, como pasaba con el centrado.

**c) Diagnóstico**, por el mismo criterio de antes:

```
    [orbita] Xp=31 ang=-12.4  orbitando  sin verla=0/1000 ms  arco=47.0 grados
```

**Lo que hay que seguir mirando:** recentrar **interrumpe** el avance de la órbita. Si recentrara
todo el tiempo, el robot giraría en el lugar sin rodear la pelota y se rendiría a los 20 s. La
perilla es subir los 18°.

---

## 4. 🎯 Apretar la puntería, y por qué recién ahora se podía

El equipo pidió: *"ahora que se corrige bien, estaría bueno que bajemos la tolerancia para
apuntar y patear al arco"*. Dos cosas aparecieron al mirarlo.

**Primero: el número que mandaba no era el 8, era el 15.** La tolerancia adaptativa pasa las dos
condiciones a `TOL_ANG_LEJOS = 15°` cuando el arco está entre 50 y 200 cm — y 200 es el techo
que manda la cámara. En un partido el arco casi siempre está a más de 50 cm, así que **la
tolerancia en juego era casi siempre 15°**. Bajar el 8 no habría cambiado nada.

**Segundo, y es lo bueno:** el comentario del 25/08 dejó medido que **la condición que bloqueaba
era la de la PELOTA, no la del arco** — se vieron separaciones de arco de **0,3° y 0,8°**
(perfectas) con `angPelota` en **21° y 29°**. Y cerraba diciendo:

> *"Si aun asi sigue sin patear, el problema no es la tolerancia: es que la orbita termina donde
> no debe."*

**Eso es exactamente lo que se arregló hoy.** El 15° era un parche para que la órbita no dejara
la pelota de costado. Con la banda de ángulo manteniéndola adelante, el parche sobra.

Así que la tolerancia de lejos, que era **una sola para las dos condiciones**, se partió en dos:

| condición | qué controla | antes | ahora |
|---|---|---|---|
| pelota adelante | que la pelota esté al frente | 15° | **12°** |
| arco alineado | **la puntería del tiro** | 15° | **8°** |

Lo que gana, en centímetros de desvío:

| arco a | con 15° | con 8° |
|---|---|---|
| 100 cm | 27 cm | **14 cm** |
| 200 cm | 54 cm | **28 cm** |

El arco mide ~45 cm, o sea ~22 cm a cada lado. Con 15° a 100 cm **el tiro ya se iba afuera**;
ahora entra con margen. A 200 cm sigue justo, pero es la mitad del error.

**Si orbita y no patea nunca, la perilla es subir `TOL_ANG_LEJOS_ARCO` a 10 o 12.** Queda
anotado al lado de la constante.

---

## 5. ⚠️ Los umbrales de línea dependen de lo que el robot tenga debajo

Pasó dos veces en la misma clase y es una lección para la competencia.

| momento | S1 (der) | S2 (izq) | S3 (del) | qué pasaba |
|---|---|---|---|---|
| primera lectura | 277 | **753** | 106 | S2 leía blanco, 21 escapes en 7 s |
| al rato, movido | 105 | 296 | 71 | **los tres OK**, cero escapes |
| más tarde | 207 | 131 | **688** | ahora el que leía blanco era S3 |
| umbral | 390 | 427 | 413 | |

Con el robot sobre la mesa, **según lo que tenga debajo, cualquiera de los tres puede leer
blanco** y disparar el escape, que anula todo lo demás. Mientras eso pasa el robot no busca, no
orbita y no patea: no se puede evaluar nada.

### Y acá me equivoqué, anotado

Cuando vi que S2 leía 753 dos veces seguidas mientras los otros dos cambiaban, dije que
**parecía un problema del sensor o del cable**. No lo era: al moverse el robot, S2 bajó a 296.
Era lo que tenía debajo. Si hubiéramos ido a buscar el cable, era media clase perdida.

**La lección para la competencia:** los umbrales 390/427/413 son **de esta tela**. En la cancha
de la competencia hay que remedirlos en el lugar, con `MEDIR-ROBOT.bat`. Son 10 minutos y sin
eso el robot puede escapar de líneas que no existen.

---

## 6. ⚠️ El `SUCCESS` del cargador mintió, otra vez y textual

```
No Teensy boards were found on any USB ports of your computer.
Please press the PROGRAM MODE BUTTON on your Teensy to upload your sketch.
========================= [SUCCESS] Took 2.91 seconds =========================
```

Dice "no encontré ningún Teensy" y abajo **`SUCCESS`**. El robot estaba desenchufado y no se
cargó nada. Es la segunda clase seguida que esto aparece: **el `SUCCESS` no prueba nada, se
confirma leyendo el banner de arranque.**

---

## 7. Números de hoy

| constante | antes | ahora |
|---|---|---|
| `XP_ORBITA` | 34 | **32** (pasó por 28 y 30 en la clase) |
| `MS_PATADA` | 420 | **500** (la pelota llegaba corta al arco) |
| `MS_LIMITE_CENTRAR` | 5000 | **3000** |
| `MS_AVANCE_DESTRABE` | 1000 | **500** |
| `VEL_DESTRABE` | 100 | **150** |
| `MEJORA_DESTRABE` | — | **10 cm** |
| `MS_PERDIDA_REAL` | — | **1000 ms** |
| `MS_AGUANTE_ORBITA` | — | **1000 ms** |
| `TOL_ANG_ORB_ENTRA` / `SALE` | — | **18° / 9°** |
| `TOL_ANG_LEJOS` | 15 (las dos) | **pelota 12 / arco 8** |

---

## ⚠️ Qué queda por VER

- **Un partido.** Esto está probado en cancha, no en partido.
- Si la órbita **recentra demasiado** y por eso no termina de rodear la pelota.
- Si con el arco a **200 cm** la puntería de 8° alcanza (28 cm de desvío contra ~22 de medio
  arco: sigue justo).
- Si con el golpe de 500 ms el robot **se pasa más** y queda demasiado cerca de la línea.
- Si a 32 cm vuelve a **chocar la pelota** al empezar a orbitar.

## 🔧 Qué queda por HACER — en orden

1. **La cinta métrica contra `Xp`** — 2 minutos, pendiente desde el 04/08. Pelota a 30 cm
   exactos del centro del robot y leer `Xp`. Ahora es más importante que nunca: desde el 29/09
   todas las distancias se tratan como centímetros, y esto lo confirma o lo tira abajo.
2. **El piso de PWM en la tela.** Seguimos usando ~70 desde quieto, medido en la cancha vieja.
   Y `pruebas/piso-de-pwm/` todavía tiene **los pines del arquero** (usa el pin 2 para la rueda
   izquierda; el delantero usa el 8), así que movería los motores equivocados.
3. **Calibrar ganancia y balance de blancos de la cámara** (punto 1 de
   `MEJORAS-PENDIENTES.md`, nunca hecho).
4. Arreglar el informe contradictorio de `pruebas/calibracion-busqueda/`, que dice
   `NUNCA LA VIO` y `100% de los cuadros` a la vez.
5. Falta la bitácora del 08/09.

## 📋 Lista de competencia

- **Cargar con `CARGAR-ROBOT.bat`** (arco fijo `azul` o `amarillo`), **no** con el entorno
  `teensy41` que se usa para probar: ése elige el arco mirando al encender, y en un partido los
  rivales lo tapan. Esto ganó el partido 2.
- **Remedir los umbrales de línea en la cancha de la competencia** (ver punto 5).
- **Llevar `respaldos/partido1-2026-09-21/`**, la única versión que jugó y ganó.
- La computadora tiene que poder cargar: Teensy Loader abierto y PlatformIO sano (el antivirus
  rompe el SSL; hay un arreglo de CA bundle).
- **Revisar el cable del giroscopio.** El de este robot contesta bien (`SYS_STATUS = 5`), pero la
  otra mesa descubrió que su problema era el cable.

---

## Correcciones a cosas que yo había dicho mal

1. **`sentidoParaOrbitar()` no tiene el bug que le atribuí.** Venía diciendo que sus dos ramas
   elegían lados opuestos. Lo leí: las dos calculan `haciaElPositivo` de forma coherente. Queda
   como **sospecha sin confirmar** que midan cosas distintas (una el ángulo del arco, la otra
   cuánto giró desde el rumbo de arranque) y que los signos no coincidan. Sospecha, no bug.
2. **`MEJORAS-PENDIENTES.md` está vencido.** Dice arriba *"Nada de esto está implementado"* y ya
   está casi todo hecho: patear al rumbo 0, camino más corto, escape de línea, el interruptor de
   arco, y `VEL_AVANCE` arreglado. Hoy esa lista engaña a quien la lea.
3. **El sensor 2 no estaba roto** (ver punto 5).

---

## Nota de método

Tres clases seguidas el mismo patrón: el número estaba bien y **la magnitud medida estaba mal**.
El reloj del destrabe midió tiempo de buscar, después tiempo seguido girando, después se dejó
borrar por parpadeos. Y las tres veces lo detectó el equipo **mirando el robot**, no leyendo el
código.

Lo que cambió hoy fue el método: en vez de dar una cuarta explicación, le puse al programa un
diagnóstico que imprime lo que decide. En dos minutos mostró lo que tres razonamientos no
habían resuelto, **y además encontró un estado muerto que nadie estaba buscando** (cupo agotado
y robot congelado). Esa línea por segundo costó diez minutos de trabajo y vale más que las tres
explicaciones juntas.
