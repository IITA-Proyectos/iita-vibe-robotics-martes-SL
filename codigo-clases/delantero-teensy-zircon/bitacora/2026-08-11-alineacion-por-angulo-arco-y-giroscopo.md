# 2026-08-11 — Alineación por ángulo, elección de arco y giroscopio

**Quiénes:** Gustavo Viollaz (piso) + Claude (código)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa cargado:** `funciona/delantero/delantero.ino` — **cargado y verificado corriendo**

## Qué queríamos probar

Que patee mejor. Gustavo pidió revisar la lógica de alineación usando de guía el delantero
campeón 2025, y de paso dejar preparadas la elección de arco y el giroscopio.

## El hallazgo: la cuenta de alineación estaba mal

El criterio de patada era:

```cpp
abs(Yp - Yarco) <= 12        // centimetros
```

**Eso resta centímetros medidos a distancias distintas.** `Yp` se mide a la distancia de la
pelota (~17-22 cm) y `Yarco` a la del arco (~100 cm o más). Es restar "3 pasos míos" menos
"3 pasos de un gigante".

**El número que lo muestra:** una pelota con `Yp = 12 cm` a 17 cm de distancia está a **35°**.
Un arco con `Yarco = 12 cm` a 100 cm está a **6,8°**. La cuenta vieja da `|12−12| = 0` y canta
"alineado" cuando en realidad están a **28° uno del otro**.

La solución ya estaba en el código campeón 2025 (`delantero.ino:311-313`) y no la habíamos
copiado:

```cpp
anguloPelota    = atan2(Yp,  Xp)  * 180.0 / PI;
anguloArco_Azul = atan2(Yaz, Xaz) * 180.0 / PI;
```

Un ángulo **no depende de la distancia**. Ahora se comparan ángulos con ángulos.

Y se agregó una segunda condición que el 2025 también tenía (`tolerancia_apuntado = 15°`): la
pelota tiene que estar **adelante**. Si la pelota está a 40° y el arco también, la resta da 0 —
pero el robot al avanzar derecho ni la toca.

## Qué se agregó (tres cosas, una sola activa)

| | Qué | Interruptor | Estado |
|---|---|---|---|
| **A** | Patada por ángulo | — | **ACTIVA** |
| **B** | Elegir arco al encender | `ELEGIR_ARCO_AL_ENCENDER` | apagada |
| **C1** | Patear al rumbo 0 si no ve el arco | `USAR_GIROSCOPO` | apagada |
| **C2** | Orbitar por el camino más corto | `ORBITA_CAMINO_CORTO` | apagada |

Se prenden **de a una**, en ese orden. El procedimiento de cada paso está en el bloque
`COMO PROBAR ESTO` dentro del `.ino`.

**B — el ritual:** se apoya el robot **mirando al arco rival** y se lo enciende. Mira 2 s sin
moverse y se queda con el arco más centrado. El mismo gesto define el "cero" del giroscopio: un
solo ritual, dos funciones.

**C — degradación con gracia:** si el BNO055 no contesta, lo dice y sigue andando como hasta
ayer. La detección de "sensor congelado" (10 lecturas de 0.000 exacto seguidas) es prestada del
`cuadrado-giroscopo` del arquero, que la pagó caro el 28/7.

**Perillas de signo, `[SIN VERIFICAR EN BANCO]`:** no sabemos de qué lado es "Y positivo" ni qué
sentido de giro da `sentidoA = true`. Hay dos booleanos para darlo vuelta —
`SENTIDO_ORBITA_INVERTIDO` y `GIRO_RUMBO_INVERTIDO`— y se prueban en dos intentos cada uno.

## Números del monitor serie (9 s, robot andando)

```
>>> AVANZANDO   Xp=140 Yp=-3  (a  -1.2 grados)  arco AZUL: no lo veo
>>> CENTRANDO   Xp=34  Yp=-11 (a -17.9 grados)  arco AZUL: no lo veo
>>> BUSCANDO    Xp=143 Yp=-23 (a  -9.1 grados)  arco AZUL: no lo veo
*** llegue a 17 cm -> a orbitar
>>> ORBITANDO   Xp=17  Yp=7   (a  22.4 grados)  arco AZUL: no lo veo
>>> AVANZANDO   Xp=57  Yp=23  (a  22.0 grados)
... perdi la pelota orbitando
```

**Dos cosas, y ninguna es la lógica nueva:**

**1. `angArco = --` en TODAS las líneas.** En 9 segundos el arco azul **no se vio ni una vez**.
Mientras eso pase, el robot **no puede patear nunca**, con la cuenta vieja o con la nueva. O sea:
**la mejora A está cargada pero todavía no se pudo evaluar.** Es la calibración de cámara, que
viene postergada desde el 28/7.

**2. Entra a orbitar a 17-20 cm y ahí pierde la pelota.** Las cuatro entradas a `ORBITANDO`
fueron a **17, 20, 19 y 17 cm** — y el piso de la imagen calculado el 28/7 es **17,4 cm**.
Inmediatamente después, la pelota o se pierde o salta a 40-62 cm.

Eso es **consistente** con la sospecha abierta desde la primera clase: la órbita pone la pelota
justo en el borde de lo que la cámara puede ver (`R = 2·L = 17,5 cm` contra un piso de imagen de
17,4 cm). **Sigue sin confirmarse** — falta saber desde dónde mide la homografía, que se
contesta poniendo la pelota a 30 cm exactos y leyendo `Xp`.

## Qué queda pendiente

1. **Calibrar la cámara.** Ya no es "lo primero de la lista": es lo único que destraba lo demás.
   El arco no se ve, y sin arco no hay patada. Herramienta y protocolo en
   [`../vision/`](../vision/).
2. Medir los 30 cm que cierran la duda de la zona ciega.
3. Recién con el arco a la vista, evaluar la patada por ángulo y anotar **a cuántos grados
   pateó**.
4. Después, los pasos 2, 3 y 4 del bloque `COMO PROBAR ESTO`.

**Compila y carga no prueban nada de esto.** Lo único validado hoy es que el firmware corre y
que imprime los ángulos.

---

## Segunda tanda del mismo día: se prendieron B y C

Gustavo probó el ritual (encender mirando al arco amarillo) y **pateó al azul**. No falló nada:
`ELEGIR_ARCO_AL_ENCENDER` estaba en `false`, o sea que la función estaba **apagada por
configuración**. El robot lo decía en el banner: `Arco objetivo: AZUL (fijo por configuracion)`.

También pidió bajar el ángulo: *"es mucho y patea muy mal de dirección"*.

### Qué se cambió

| Perilla | Antes | Ahora |
|---|---|---|
| `TOL_ANG_PELOTA` | 15° | **8°** |
| `TOL_ANG_ALINEADO` | 15° | **8°** |
| `ELEGIR_ARCO_AL_ENCENDER` | false | **true** |
| `USAR_GIROSCOPO` | false | **true** |
| `ORBITA_CAMINO_CORTO` | false | false (sigue apagada: signo sin verificar) |

**Por qué bajaron los dos ángulos y no sólo el del arco:** el robot empuja **derecho**. Si la
pelota está 14° al costado, la toca de refilón y sale para cualquier lado. La dirección la
arruina `TOL_ANG_PELOTA` tanto como la del arco.

A 100 cm, 8° son ~14 cm de desvío. Escalera si ahora **nunca** patea: 8 → 10 → 12. Si sigue
torcido: 6.

### 🔴 El hallazgo: el giroscopio contesta pero da ceros

Primera vez que se lo enciende en este robot. El banner:

```
Giroscopo:    contesta pero da ceros (9/20 lecturas utiles) --- no lo doy por bueno
              NO CONTESTA. Sigo sin el, como hasta ayer.
```

**Qué significa exactamente:** `bno.begin()` **funcionó** — el sensor está en el bus I2C y
responde quién es. Pero de 20 lecturas, **11 devolvieron 0.000 exacto en los tres ángulos**. La
guarda prestada del arquero lo detectó y se negó a darlo por bueno. **El diseño funcionó: el
robot avisó y siguió andando como antes en vez de girar con datos basura.**

**La causa más probable, y no es nuestra:** el README del arquero lo dice textual —
*"El giroscopio se alimenta de la batería, no del USB. Con la batería apagada contesta que
existe pero devuelve puros ceros."*

**Dos señales independientes apuntan a lo mismo:**

1. El giroscopio da ceros.
2. `Xp=89 Yp=-3` quedó **congelado en el mismo valor durante 8 impresiones seguidas** estando en
   BUSCANDO, que es un estado en el que el robot gira. Si girara, la pelota cambiaría de
   posición. **El robot no se estaba moviendo** — o sea, los motores no tenían potencia.

Las dos cosas se explican con **la batería apagada**. `[HIPÓTESIS — se confirma en 1 minuto:
prender la batería y volver a resetear]`.

### El otro dato: la cámara no vio NINGÚN arco

```
amarillo: 0 muestras
azul:     0 muestras
NO VI NINGUN ARCO -> me quedo con el de siempre: AZUL
```

En 2 segundos mirando, **cero muestras de los dos arcos**. La elección de arco funcionó
perfecto: no vio nada, lo dijo, y se quedó con el de siempre. **El problema no es la lógica
nueva: es que la cámara no ve los arcos.** Tercera confirmación del día.

### Lo primero de la próxima vez

1. **Prender la batería** y resetear. Ver si el giroscopio pasa a `OK`. Un minuto.
2. **Calibrar la cámara.** Van tres mediciones distintas hoy diciendo lo mismo.

---

## 🔴 Tercera tanda: EL bug. El arco se tiraba a la basura por lejos

Gustavo conectó la cámara al OpenMV IDE y reportó: **"no parece estar mal, incluso lo ve de
lejos al arco"**. Y propuso una hipótesis: que el programa leyera lento y se quedara con datos
viejos del buffer.

### El bug

```cpp
const int XP_MAX = 150;   // "arriba de esto no le creo" -> pensado para la PELOTA

if ((Xaz > 0) && (Xaz <= XP_MAX) && (abs(Yaz) < 100)) {   // <-- aplicado AL ARCO
```

**El arco a más de 150 cm se descartaba.** Y el arco casi siempre está lejos. Por eso
`angArco = --` en todos los logs del día y `0 muestras` de los dos arcos al arrancar: la cámara
lo veía y **el firmware lo tiraba**.

El campeón 2025 no tenía techo para el arco (`delantero.ino:335-345`): le alcanzaba con
`Xam != 0`. Le habíamos copiado al arco el criterio de la pelota sin preguntarnos si tenía
sentido. **Una pelota lejos es sospechosa** (es chiquita, se confunde con cualquier mancha
naranja). **Un arco lejos es simplemente un arco lejos** — es grande, y la cámara ya le exige
300-600 píxeles. Y para *alinear* no importa a qué distancia está: importa en qué **dirección**.

**El arreglo:** techo propio, `XARCO_MAX = 200` (todo lo que la cámara puede mandar).

**El resultado, inmediato:**

```
*** ATACO EL ARCO AMARILLO ***
>>> CENTRANDO  Xp=55 Yp=18 (a 18.1 grados)   arco AMARILLO: a -3.3 grados
```

### La hipótesis del buffer: medida, no discutida

Se instrumentó el enlace en vez de opinar. Cada 2 s el robot imprime:

```
camara: 46 paq/s   0 bytes tirados/s   loop: 415350 /s
```

| Medición | Valor | Qué dice |
|---|---|---|
| Paquetes válidos | **46 /s** | la cámara corre a ~46 fps |
| Bytes descartados resincronizando | **0 /s** | no hay desincronización ni buffer desbordado |
| Vueltas del `loop()` | **415.000 /s** | leemos ~9.000 veces más rápido de lo que llegan los datos |

**La hipótesis queda refutada con números.** Pero mandó a mirar el camino de los datos, que es
donde estaba el bug — dos líneas más adelante que donde se sospechaba.

### Un error propio que el mismo log delató

La primera versión imprimió `amarillo: 4613996 muestras` en 2 segundos. Contaba **vueltas del
loop**, no cuadros de cámara: con un solo vistazo fugaz el contador se llenaba igual y
`MUESTRAS_MINIMAS_ARCO` no filtraba nada. Corregido a contar sólo cuando llega un dato **nuevo**
(cambia `t_ultimoAmarillo`). Ahora imprime **95 muestras en 2 s**, que es consistente con los
46 paq/s medidos.

### Estado al cierre

- ✅ El arco se ve y se sigue: `angArco` entre -1,6° y -3,3°.
- ✅ La elección de arco al encender funciona: eligió **AMARILLO** con 95 muestras a 3,3°.
- ⏸️ La patada todavía no se pudo ver: el robot no se estaba moviendo (`Xp=55 Yp=18` congelado),
  así que nunca llegó a orbitar. La pelota a 18,1° con tolerancia de 8° **no debe** patear:
  ahí el firmware está haciendo lo correcto.
- ⏸️ Giroscopio sigue en ceros → **la batería sigue apagada**. Es la misma causa que explica que
  el robot no se mueva.

---

## Cuarta tanda: escape de la línea blanca

Gustavo confirmó dos cosas del dibujo: la rueda de abajo-izquierda es **DD** (delantera derecha,
M2, pines 11/12/4), y la regla de comportamiento: **si detecta línea blanca se anula todo lo que
esté haciendo y se escapa para el lado que corresponda.**

### La geometría, y por qué sale gratis

Cada sensor vigila **un lado** del triángulo, y enfrente de cada lado hay una rueda:

| Sensor | Escapa hacia | Qué hacen los motores |
|---|---|---|
| 1 | rueda **DI** (M1, izquierda) | **IZQ apagada**, DER y TRA opuestas |
| 2 | rueda **DD** (M2, derecha) | **DER apagada**, IZQ y TRA opuestas |
| 3 | rueda **T** (M3, trasera) | **TRA apagada**, IZQ y DER opuestas |

Para moverse en la dirección de una rueda, **esa rueda no tiene que girar** (su empuje es
perpendicular a ese movimiento) y las otras dos hacen todo. Es el mismo truco que la órbita: la
rueda que no gira define la geometría, y **no hace falta calibrar nada**.

No lo inventamos: es exactamente `retroceder1/2/3` del campeón 2025
(`delantero.ino:164-178`), PWM 100 durante 400 ms.

**Las esquinas salen solas.** Si saltan dos sensores, las dos direcciones se **suman**. Y como
las tres direcciones suman cero, sensor1 + sensor2 da exactamente lo contrario de sensor3:
alejarse de la rueda T. Sin medir un solo ángulo.

Si saltan **los tres**, las direcciones se cancelan y no hay hacia dónde ir: el robot **para** y
lo dice. Salir para un lado elegido al azar sería inventar.

### Prioridad absoluta

La comprobación de línea va **antes** de la máquina de estados y anula el estado en curso,
**incluida la patada** —que hasta hoy era lo único ininterrumpible—. Sale de la línea, sigue
400 ms más después de dejar de verla, y vuelve a BUSCANDO.

### Los pines: medidos, no adivinados

La librería Zircon **autodetecta** la versión de placa leyendo el pin 32
(`zirconLib.cpp:52-60`), y cada versión usa pines distintos:

| Versión | Sensor 1 | Sensor 2 | Sensor 3 |
|---|---|---|---|
| Mark1 | A11 | A13 | A12 |
| Naveen1 | A8 | A9 | A12 |

Se replicó la misma detección en el firmware. **Resultado leído del robot: `Mark1`**, sensores
en los pines físicos **25 / 27 / 26**.

### La autoprotección se activó — y hay que leerla con cuidado

```
Linea: sensores leen 743 / 642 / 758   umbrales 650 / 650 / 750
!!! YA LEE BLANCO ESTANDO EN EL VERDE -> el umbral esta mal.
!!! ESCAPE DE LINEA DESACTIVADO.
```

La idea: el robot se enciende apoyado en el verde, nunca sobre una línea. Si un sensor ya dice
"blanco" al arrancar, el umbral está mal para la luz de hoy — y con el umbral mal el robot
**escaparía para siempre**. Mejor desactivar y avisar.

⚠️ **PERO el robot estaba sobre el ESCRITORIO, no sobre la cancha.** Esos 743/642/758 son del
escritorio, no del verde. **No se puede concluir todavía que los umbrales estén mal para la
cancha.** Hay que repetirlo con el robot apoyado en el verde.

### Qué hacer la próxima vez, en orden

1. Robot **sobre el verde**, resetear, y mirar qué dice el chequeo.
2. Cargar `pruebas/sensores-de-linea/` (no mueve motores) y anotar: cuánto lee cada sensor sobre
   el **verde** y cuánto sobre la **línea blanca**. El programa calcula solo el umbral (el punto
   medio) e indica **cuál sensor saltó** al pasar cada lado por la línea — que es el dato que
   confirma cuál índice es cuál lado.
3. Poner esos tres números en `UMBRAL_LINEA[3]` del firmware.
4. Recién ahí probar el escape de verdad.

> Nota: mientras `lineaHabilitada` sea false, los `analogRead` no se ejecutan y el `loop()`
> sigue a ~490.000 vueltas/s. Cuando se active va a bajar, y eso es normal.
