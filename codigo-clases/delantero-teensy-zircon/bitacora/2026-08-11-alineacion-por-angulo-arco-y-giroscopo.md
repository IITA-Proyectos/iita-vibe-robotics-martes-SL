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
