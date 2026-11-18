# 2026-11-17 — Sacarle la vuelta a la órbita: patear rápido, y la lógica ya estaba escrita

> Robot **delantero**, Teensy 4.1 / Zircon, serie `15708680`.
> Primera clase después de la Roboliga.

---

## 📋 RESUMEN

| | |
|---|---|
| 🏆 **Roboliga (11/11): 3 puntos, TERCER PUESTO** | perdimos 4-5 y 3-7, ganamos **10-0** |
| 🎯 **La idea del día** | *"los equipos mejores patean rapidísimo y nosotros nos demoramos orbitando"* |
| ✅ **La lógica que pensó el equipo YA ESTABA PROGRAMADA** | lo único que sobraba era **la espera**: `VUELTAS_PARA_RUMBO0` de 1 a **0** |
| 🎛️ **Tolerancias apretadas** | rumbo 12 → **8°**, pelota 12 → 8 → **10°** |
| 🛑 **Freno antes de patear al cero** | **300 ms**, y al terminar vuelve a mirar si sigue alineado |
| 📷 **La cámara manda sobre el cero** | mientras VEA el arco no patea al cero, por **2 s** |
| 🔴 **Una idea mía que estaba mal** | corregir el cero con la cámara. **La frenó el equipo** |
| 💾 **Respaldo** | `delantero-2026-11-11-ROBOLIGA-2026-tercer-puesto.ino` |
| ⚠️ **Nada probado** | todo compila; la carga y la prueba quedaron para el equipo |

---

## 1. 🏆 La Roboliga, contada por el equipo

| | resultado |
|---|---|
| Partido 1 | perdimos **4-5** |
| Partido 2 | perdimos **3-7** |
| Partido 3 | ganamos **10-0** |

**3 puntos y tercer puesto.** El 4-5 dice que el partido que se escapó estuvo ahí.

No hay bitácora de la competencia, así que **no sabemos qué cargaron, con qué arco, ni qué hizo
mal el robot en cada partido**. Es la tercera vez que queda anotado este agujero (21/09 y 12/10).
Lo que sí quedó claro es el diagnóstico del equipo, y es el que mandó el trabajo de hoy:

> *"Queremos que vaya más rápido para patear y centrarse, porque los equipos mejores lo hacen
> rapidísimo y nosotros nos demoramos orbitando."*

---

## 2. ✅ La lógica nueva, y la sorpresa

**La lógica que planteó el equipo, con sus palabras:**

1. Ve la pelota y se dirige a ella.
2. El robot se alinea al 0 del giroscopio *"(o lo más cerca que pueda, porque si es 0 exacto no va
   a llegar nunca)"* con la pelota al frente.
3. Si ve el arco, gira al lado correspondiente y patea.
4. Si no ve el arco, que patee para adelante.

**Y la sorpresa fue que eso ya estaba programado.** Lo único que hacía el programa de la Roboliga
era **hacerlo esperar**: recién habilitaba el "patear al cero" después de **una vuelta entera**
alrededor de la pelota.

```cpp
if (t_buscaRumbo0 == 0 && fabs(giradoOrbita) >= 360.0 * VUELTAS_PARA_RUMBO0) {
  t_buscaRumbo0 = millis();          // desde aca SI patea al cero
}
```

Así que todo el cambio de concepto se consiguió con **un número**: `VUELTAS_PARA_RUMBO0` de **1 a
0**. Con 0, apenas llega a la pelota ya busca quedar mirando al cero.

**Es la segunda vez en el proyecto que pasa esto** (la primera fue el centrado por ángulo): la
capacidad estaba escrita y lo que fallaba era *cuándo* se la dejaba actuar.

### Una corrección de física al paso 3

*"Gira al lado correspondiente y patea"* — **si gira sobre su propio eje, la pelota se le va del
frente.** Está a 32 cm adelante: cuando el robot rota en el lugar, la pelota queda de costado y ya
no hay patada.

Para girar sin perder la pelota hay que girar **alrededor de ella**, y eso es orbitar. **Lo que
sobraba no era la órbita: era dar la vuelta entera.** Ahora la órbita es la corrección final, corta
y con destino, en vez de una búsqueda.

### Y sí gira por el lado más corto

Ya estaba resuelto, y con el criterio correcto:

```cpp
if (millis() - arcoT() < MS_GRACIA) haciaElPositivo = (anguloDe(arcoX(), arcoY()) > 0);  // ve el arco: para el lado del arco
else if (giroscopoSano())           haciaElPositivo = (diferencia(rumboCero, rumboActual()) > 0);  // si no: para el lado del CERO
```

> ⚠️ **El lado se decide UNA sola vez** al llegar a la pelota, y queda congelado. Antes daba la
> vuelta entera igual, así que no importaba; **ahora que no espera, si elige mal se va por el lado
> largo**. Es el primer lugar donde mirar si se lo ve dudar.

---

## 3. 🔴 Una idea mía que estaba mal, y la frenó el equipo

Propuse que **la cámara le corrigiera el cero al giroscopio**: cada vez que viera el arco rival,
actualizar el cero con esa dirección, para que la deriva no importara.

**El equipo la frenó:**

> *"Si el robot ve el arco de costado va a marcar mal el cero, o si lo ve una parte también. No sé
> si es buena idea hacer eso, yo quiero confiar en el giroscopio."*

**Tenía razón, y el motivo es más de fondo que el que planteó:** el ángulo al arco **depende de
dónde está parado el robot en la cancha**. Desde el córner el arco queda al costado; desde el medio,
al frente. O sea que *"para allá está el arco"* **no es una dirección fija de la cancha** — y el
cero sí lo es. Son dos magnitudes distintas, y yo propuse usar una para corregir la otra. Con cámara
perfecta y arco visto de frente, **igual habría estado mal**: el cero se habría ido moviendo por
toda la cancha según la posición del robot.

Es el mismo error que ya mordió cuatro veces en este proyecto: el número está bien calculado y **la
magnitud que mide no es la que hace falta**.

**El reparto correcto quedó como lo pensó el equipo:**

| | para qué sirve |
|---|---|
| **giroscopio (el cero)** | la **referencia de la cancha**: lo único que sabe "para allá está el arco rival" desde cualquier lado |
| **cámara** | la **puntería final**, cuando llega a ver el arco |

Lo que **sí** se mantiene es `VETO_ARCO_PROPIO`, y es distinto: no intenta *medir* nada, sólo dice
*"si veo NUESTRO arco adelante, no pateo"*. No necesita saber dónde está el robot.

---

## 4. 🎛️ Las tolerancias, y por qué son dos

Son dos cosas distintas y se ajustan por motivos diferentes:

| tolerancia | qué mide | hoy |
|---|---|---|
| `TOL_RUMBO` | hacia dónde **apunta el robot** | 12 → **8°** |
| `TOL_ANG_LEJOS_PELOTA` | dónde está **la pelota** respecto de esa dirección | 12 → 8 → **10°** |

**Las dos tienen que cumplirse a la vez**, porque este robot patea **avanzando**, no tiene
pateador. Si apunta bien pero la pelota quedó al costado, avanza al arco y **deja la pelota atrás**.
Si la pelota está adelante pero apunta mal, la empuja derecho para el lado equivocado.

**Por qué se apretó el rumbo:** con 0 vueltas el robot patea al cero **a ciegas**, así que el ángulo
con el que sale es el ángulo con el que la pelota viaja.

| error de rumbo | desvío a 2 m |
|---|---|
| 12° | 42 cm |
| **8°** | **28 cm** |

El arco mide ~45 cm (unos 22 a cada lado): con 12° el tiro se iba afuera desde lejos.

**Y por qué la de la pelota quedó en 10 y no en 8:** es la más cara de apretar. El 25/08 se midió
que **ésta es la condición que BLOQUEA la patada** — se vieron casos con el arco perfecto (0,3° y
0,8°) y la pelota a **21° y 29°**. A 32 cm, 10° son 5,6 cm de corrimiento y la pelota mide 4,3 cm.

> **Si el robot orbita y NO patea, la primera que hay que aflojar es la de la pelota (volver a 12),
> no la del rumbo.** Queda anotado al lado de la constante.

---

## 5. 🛑 El freno de 300 ms antes de patear al cero

Pedido del equipo: *"cuando llega al 0 grados que se pare por 300 ms y recién se acomode a patear o
patee derecho"*.

**Por qué hace falta:** llega al cero **orbitando**, o sea empujando de costado con la rueda
trasera. Si patea desde ahí, el golpe arranca con velocidad lateral y la pelota sale torcida.

Y tiene un efecto extra que viene bien: **al terminar de frenar vuelve a mirar si sigue alineado.**
Si el envión lo pasó de los 8°, no patea y sigue acomodándose. O sea que el freno hace las dos cosas
del pedido: se para, y recién entonces se acomoda **o** patea.

Es un **estado**, no un `delay()`: durante esos 300 ms **sigue mirando la línea blanca**. Con
`MS_FRENO_PRE_PATADA = 0` se saca.

### 🐛 Un reloj mal reseteado, cazado antes de que muerda

La primera versión tenía el error de siempre: si el robot empezaba a frenar y **en medio del freno
la pelota se corría** de los 10°, la condición dejaba de cumplirse pero **el reloj quedaba con la
hora vieja**. La próxima vez que se alineara, el programa iba a creer que ya frenó y **patearía sin
frenar**.

Arreglado poniendo la condición en un `bool` y reiniciando el reloj en cuanto deja de cumplirse:

```cpp
if (!listoParaCero) t_frenoPrePatada = 0;
```

**Quinta vez que aparece este mismo error en este programa.** La regla ya está escrita en el
código: *la cuenta tiene que medir tiempo SEGUIDO*.

---

## 6. 📷 La cámara manda sobre el cero, por 2 segundos

Con 0 vueltas, las dos condiciones quedaron vivas a la vez, y **ganaba la que se cumplía primero**.
O sea que si el robot pasaba por el cero antes de alinear el arco, **pateaba al cero aunque estuviera
viendo el arco**. Eso es al revés de lo que conviene: el arco es el dato real, el cero es una
estimación a ciegas.

**Cómo quedó:** mientras VEA el arco, no patea al cero y sigue acomodándose.

**Pero con límite de 2 segundos**, y no por capricho: con prioridad absoluta, un caso como el del
25/08 (arco perfecto, pelota a 29°) dejaría al robot orbitando hasta que se le acaben los 15 s de
`MS_RUMBO0_MAX` y **volvería a buscar sin patear** — peor que patear al cero.

**El orden de prioridades al llegar a la pelota, entonces:**

1. ¿Ve el arco **alineado** con la pelota? → patea al arco.
2. ¿Ve el arco pero **no alineado**? → se acomoda, y **no patea al cero** por 2 s.
3. ¿No ve el arco (o pasaron los 2 s)? → se alinea al cero, frena 300 ms y patea.
4. ¿Ve **nuestro** arco adelante? → no patea al cero, sigue orbitando.

El diagnóstico de la órbita ahora imprime `MANDA EL ARCO` o `BUSCA EL CERO`, así se ve cuál de las
dos está decidiendo.

---

## 7. ⚠️ `partido-con-saque` es otra copia, y quedó atrasada

`PARTIDO-CON-SAQUE.bat` **no carga `funciona/delantero`**: carga `partido-con-saque/`, que es una
**copia completa** del programa (1561 líneas contra 1262) con el saque agregado.

| | vueltas | TOL_RUMBO | pelota | freno | prioridad |
|---|---|---|---|---|---|
| `funciona/delantero` | **0** | **8°** | **10°** | **300 ms** | **2 s** |
| `partido-con-saque` | 1 | 12° | 12° | — | — |

**Si se juega con `PARTIDO-CON-SAQUE.bat`, no hay NADA de lo de hoy.**

> 🔴 **El problema de fondo no son los números: son las dos copias.** Cada cambio hay que hacerlo
> dos veces, y la segunda se olvida — y lo que se desincroniza es justo **lo que juega**. Ya pasó con
> la patada: medirla en dos lugares distintos con configuraciones distintas hizo sacar conclusiones
> equivocadas toda una tarde.
>
> **Propuesta pendiente:** que el saque sea un interruptor dentro del principal (`SAQUE_AL_AZAR`, que
> ya existe en la copia) en vez de una carpeta aparte. Un programa, un lugar donde cambiar las cosas.

---

## 8. 🧰 Cosas de método que cambiaron hoy

**La carga la hace el equipo.** Instrucción nueva: *"de ahora en más no cargues vos automáticamente
el programa, lo hago yo con el .bat"*. Claude **compila** y avisa; el equipo carga y **confirma por
el banner**, que ahora imprime las cuatro reglas nuevas.

**El respaldo de la Roboliga** quedó en `respaldos/delantero-2026-11-11-ROBOLIGA-2026-tercer-puesto.ino`,
verificado idéntico al programa que jugó. Para volver a esa lógica alcanza con
`VUELTAS_PARA_RUMBO0 = 1.0`.

**`rumbo-vivo` ahora compara los dos modos del giroscopio** (con brújula y sin), porque las 8 pruebas
que usan el giroscopio lo arrancaban en el modo **viejo**: cargarlas para "probar el modo nuevo" era
revisar los frenos de otro auto. Medido el 13/10, con reservas: `quieto en 10 s → modo A 0,1° / modo
B 20,8°`. **Ese 20,8 está contaminado** (se mide justo después de que alguien zamarrea el robot a
mano), y está anotado así. El repo siguió en **modo B**, que es con lo que se jugó la Roboliga.

---

## ⚠️ Qué queda por VER

1. 🔴 **Si de verdad gana tiempo.** Nada de hoy se probó: todo compila y nada se cargó.
2. 🔴 **Si patea hacia nuestro arco alguna vez.** Es el riesgo que trae patear al cero sin esperar:
   antes, con el cero mal, el robot **no pateaba**; ahora **patea convencido**.
3. **Si elige mal el lado** y se va por la vuelta larga (ver punto 2).
4. **Si los 300 ms de freno alcanzan** o si todavía sale torcida.
5. **Si los 2 s de prioridad de la cámara son mucho o poco.**
6. **La patada medida:** el robot guarda el desvío de las últimas 5 y nunca se leyó después de un
   partido.

## 🔧 Qué queda por HACER

1. 🔴 **Decidir qué hacer con `partido-con-saque`**: sincronizarlo o unificarlo (punto 7).
2. 🔴 **Medir `L`** con una regla. El robot se achicó el 12/10 y los `17,5 cm` del código quedaron
   viejos; de ese número cuelga toda la geometría de la órbita.
3. **Escribir qué pasó en la Roboliga**, mientras alguien se acuerde: qué se cargó en cada partido y
   qué hizo el robot.
4. Siguen de antes: `calibracion-quieto` y `calibracion-moviendo` **nunca se corrieron**, el piso de
   PWM sin medir, y `MEJORAS-PENDIENTES.md` vencido.

---

## Nota de método

**Lo mejor del día no lo escribió nadie hoy.** La lógica que el equipo pensó desde cero ya estaba en
el programa, escrita en agosto como plan B. Lo que la volvía lenta era una espera de una vuelta
entera, puesta cuando ese plan B casi nunca se usaba. **Cuando un respaldo pasa a ser el camino
principal, hay que revisarle las esperas** — es la misma lección que dejó el 12/10 por el lado malo.

**Y lo segundo mejor fue una idea mía rechazada.** Propuse corregir el cero con la cámara y el equipo
dijo "no, yo quiero confiar en el giroscopio". Tenía razón: las dos cosas miden magnitudes distintas
y mi idea habría movido el cero por toda la cancha. **La pregunta "¿esto mide lo que hace falta?"
sigue siendo la que más errores encuentra en este proyecto** — y esta vez la hizo el equipo, no yo.
