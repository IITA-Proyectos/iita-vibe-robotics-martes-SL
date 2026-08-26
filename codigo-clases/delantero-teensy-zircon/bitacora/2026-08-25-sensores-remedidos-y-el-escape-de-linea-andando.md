# 2026-08-25 — Sensores re-medidos, el sensor 1 recuperado, y el escape de línea andando

**Quiénes:** Máximo (mesa) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa cargado al cerrar:** el del 18/08 con los umbrales de línea re-medidos hoy

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| ✅ **El sensor 1 se recuperó** | de 3 puntos de separación verde↔blanco a **446** |
| ✅ **Umbrales re-medidos y cargados** | `{1024, 675, 700}` → `{542, 546, 608}` |
| ✅ **EL ESCAPE DE LÍNEA SE EJECUTÓ** | por primera vez desde que existe. Dos veces |
| ✅ **PlatformIO funcionando** | tres problemas encadenados, ninguno era el robot |
| ✅ **La carpeta es un clon de git de verdad** | estaba 3 meses y medio atrasada |
| 🔴 **El segundo escape no se despegó** | 24 s en `!LINEA!` sin salir |
| 🔴 **El giroscopio sigue sin medir** | pero ahora sabemos que el sensor está vivo |
| ⚠️ **Duda sin cerrar** | el verde de hoy no es el verde del 18/08 |
| ✅ **Umbrales RE-medidos EN LA CANCHA** | `{707, 582, 795}` — sección 8 |
| ✅ **Tolerancia de patada adaptativa** | 15° con el arco lejos — sección 9 |
| ✅ **Restaurado el golpe de freno** | el de gviollaz del 18/08 — sección 10 |
| 🔴 **SIGUE SALIÉNDOSE DE LA CANCHA** | al patear. Es lo que queda abierto |
| 🎁 **La otra mesa resolvió las unidades** | factor 2,87 en el arquero — sección 11 |

---

## 1. Los sensores de línea, re-medidos

**Método.** Rotando cuál sensor pisa el blanco (dos en negro, uno en blanco) y al final los
tres en verde. Es el método diferencial del 18/08: preguntar *cuál cambió* en vez de *cuánto
lee éste*. El negro de cada sensor es el **promedio de las dos fases** en que le tocó estar en
negro, no una lectura suelta. Cada fase son ~20 muestras con el robot quieto, capturadas por
serie y promediadas.

| sensor | negro | verde | blanco | separación verde→blanco |
|---|---|---|---|---|
| 1 | 96 | 319 | 765 | **446** |
| 2 | 71 | 330 | 762 | **432** |
| 3 | 95 | 452 | 765 | **313** |

### 🔧 El sensor 1 se recuperó

El 18/08 leía **762 en verde y 765 en blanco** — tres puntos — y hubo que anularlo poniéndole
el umbral en 1024. Hoy tiene **446 puntos** de separación y es **el que mejor separa de los
tres**. Vuelve a entrar en juego con un umbral real.

### Los umbrales nuevos

```cpp
int UMBRAL_LINEA[3] = { 542, 546, 608 };   // era { 1024, 675, 700 }
```

Punto medio **verde↔blanco**, no negro↔blanco. Esto importa y es fácil de errar: el programa
`pruebas/sensores-de-linea/` sugiere el umbral como `(minimo + maximo) / 2` de todo lo que
vio, y si se le mete el negro en la corrida propone ~420 — **por debajo del verde**, con lo
cual el robot escaparía parado sobre la cancha. **Ignorar esa línea del programa cuando se
mide con negro.**

### ⚠️ Lo que NO se confirmó

El negro y el blanco dieron igual que el 18/08, pero el **verde se movió muchísimo**:
762/588/638 → 319/330/452.

Y cuidado con el razonamiento fácil: **que negro y blanco coincidan NO prueba que el montaje
esté igual.** Los dos están en los extremos del rango — el blanco satura en 765 pase lo que
pase, el negro está contra el piso — así que son **insensibles a la altura de los sensores**.
El verde es el único que está en el medio, y por eso es el único que revela un cambio de
altura.

Quedan dos explicaciones sin decidir:

- **A)** se levantaron los sensores (era el pendiente 3 del 18/08) → los números valen y el
  sensor 1 quedó arreglado de raíz;
- **B)** el verde de hoy no es el de la cancha. Estos 319-452 se parecen al verde de la mesa
  del **arquero** (350-468), no al del delantero. Si es una muestra suelta, **estos umbrales
  podrían no transferir**.

**Cómo se cierra:** encender el robot apoyado en el verde **de la cancha** y leer el banner.
No se pudo hacer hoy porque la mesa está lejos de la cancha y el cable USB no llega.

### El verde varía de un punto a otro

Dos lecturas del verde en la misma clase:

| | S1 | S2 | S3 |
|---|---|---|---|
| medición | 319 | 330 | 452 |
| al encender, otro punto | 264 | 259 | 341 |
| diferencia | −55 | −71 | −111 |

**Los márgenes lo absorben sin problema** (quedan 267-287 puntos hasta el umbral), pero
confirma el pendiente 2 del 18/08: el verde no es un número, es un rango.

---

## 2. 🎉 El escape de línea se ejecutó — el pendiente N°1

Nunca se lo había visto correr. Hoy corrió **dos veces**, disparado por **dos sensores
distintos**:

```
>>> CENTRANDO   Xp=131 Yp=-61
!!! LINEA BLANCA (sensores 3) estando en CENTRANDO -> ESCAPO
... ya me despegue de la linea            <-- se despegó ✅
>>> BUSCANDO -> CENTRANDO -> AVANZANDO
!!! LINEA BLANCA (sensores 2) estando en AVANZANDO -> ESCAPO
[!LINEA!] ... 12 reportes seguidos, ~24 segundos ...   <-- NO se despegó 🔴
```

**La primera se despegó y volvió a jugar. La segunda no.** El estado sale 400 ms después de
dejar de ver la línea, así que 24 s adentro significa que el sensor 2 siguió viendo blanco
todo ese tiempo. El log se corta ahí (se desenchufó el USB), así que no se sabe si se despegó
después.

**Sospecha principal:** la mesa. Escapó hacia un lado y encontró más blanco. Sobre una
superficie chica, "alejarse de la línea" no siempre tiene a dónde ir. **No invalida los
umbrales** —el disparo funcionó las dos veces— pero la maniobra completa recién se puede
juzgar en la cancha.

---

## 3. 🛠️ La autoprotección es del ARRANQUE, no continua

Media hora perdida en esto, y la lección vale para toda la temporada.

Después de cargar los umbrales nuevos, el robot **dejó de detectar el blanco**. No era el
umbral: el robot se había encendido apoyado sobre la mesa, la autoprotección vio blanco y
**desactivó el escape para toda la corrida**. Esa decisión se toma **una sola vez, en el
`setup()`**, y no se revierte por más que después lo pongas sobre el verde.

**Cómo se diagnostica en 5 segundos, sin banner:**

| loop | significa |
|---|---|
| **~17.500/s** | está leyendo los 3 sensores → escape **ACTIVADO** |
| **~423.000/s** | ve pelota Y arco, no lee línea → **DESACTIVADO** |
| **~480.000/s** | ve la pelota, no lee línea → **DESACTIVADO** |
| **~631.000/s** | no ve nada, no lee línea → **DESACTIVADO** |

Los tres `analogRead` por vuelta cuestan la diferencia grande. Las diferencias chicas entre
423k, 480k y 631k son los `atan2()` de `anguloDe()`, que sólo se ejecutan cuando hay algo a la
vista — con `X <= 0` la función sale por la primera línea sin calcular nada.

**El ritual, entonces: encender el robot apoyado sobre el verde.** Y alcanza con una muestra
de verde en la mesa — la autoprotección no sabe si es la cancha o un recorte.

> **🚨 EN COMPETENCIA ESTO ES UN PARTIDO PERDIDO.** Si el robot se enciende sobre blanco,
> juega **sin protección de línea** y el único aviso pasa volando en el banner de arranque.
> **Propuesta pendiente:** que la telemetría de cada 2 s imprima `linea: ON/OFF`, y que el LED
> del hub lo muestre — porque en cancha se prueba sin cable y sin serial.

> **Y OJO CON EL FLASHEO:** cargar un programa resetea el Teensy y vuelve a correr la
> autoprotección. Si en ese momento el robot no está sobre el verde, el escape queda apagado
> aunque los umbrales sean correctos. Pasó hoy dos veces.

---

## 4. La patada: se alinea perfecto, y aun así no patea

Capturado en vivo, en estado `AVANZANDO`:

```
>>> AVANZANDO   Xp=91 Yp=4   arco AZUL: a -0.9 grados
[AVANZANDO]  Xp=94 Yp=7  angPelota=4.3  angArco=3.4  separacion=0.8
```

**`angPelota = 4,3°`** (pide ≤ 8 ✅) y **`separacion = 0,8°`** (pide ≤ 8 ✅). Las dos
condiciones de la patada cumplidas con margen enorme. No pateó.

El motivo es estructural: **el chequeo de patada existe sólo dentro del estado `ORBITANDO`.**
En `AVANZANDO` no hay ningún `if` que patee, por bien alineado que esté.

Acá es defendible —la pelota estaba a Xp=91, lejos— pero deja un dato importante para el
pendiente del 18/08: **el robot se alinea bárbaro**, y el arco se ve perfecto (el arreglo del
`XARCO_MAX` del 11/08 está funcionando). Lo que falla es específicamente la geometría **al
salir de la órbita**, no la capacidad de alinearse.

---

## 5. El giroscopio: el sensor está vivo

El banner sigue diciendo lo mismo que el 18/08, y el número es **exactamente el mismo**:

```
Giroscopo:
   contesta pero da ceros (9/20 lecturas utiles) — no lo doy por bueno
NO CONTESTA. Sigo sin el, como hasta ayer.
```

Ese 9/20 confirma dos cosas: que **el revert del 18/08 está efectivamente en la placa**, y —lo
importante— que **9 de 20 lecturas fueron distintas de cero**. El sensor entrega datos. Lo que
está roto es la verificación.

**El bug, en `arrancarGiroscopo()`:**

```cpp
if (!bno.begin()) return false;
delay(700);                      // espera a que arranque la fusion
bno.setExtCrystalUse(true);      // <-- ESTO LA REINICIA. Los 700 ms se tiraron.
for (int i = 0; i < 20; i++) { ... }   // empieza a medir con el sensor recien reiniciado
if (buenas < 10) return false;   // saca 9. FALLA POR UNA.
```

El `delay(700)` está **antes** del `setExtCrystalUse()`, que reinicia la fusión.

**⚠️ NO alcanza con mover el delay.** El 18/08 se arregló exactamente eso, el giroscopio pasó
a reportar `rumbo=360` congelado, y el robot **empeoró**: creía tener giroscopio, entraba en
`APUNTA_RUMBO0` y giraba en el lugar 6 s contra un rumbo que nunca cambia. Corregir la
verificación sin corregir la fusión es peor que no tener giroscopio.

**Dato nuevo de hoy:** el arquero tiene **exactamente el mismo código** (mismo `delay(700)`
antes del `setExtCrystalUse`, mismo `buenas < 10`) y a él **le anda**. Eso apunta al
**hardware**, no al software — y refuerza la sospecha del cristal externo.

**Próximo paso, y se puede probar sobre la mesa girando el robot a mano:** un programa en
`pruebas/` que arranque el BNO **sin** `setExtCrystalUse(true)`, lea el registro de estado del
sistema (fusión corriendo = 5) y muestre el rumbo en vivo.

---

## 6. 🛠️ PlatformIO: tres problemas encadenados, ninguno era el robot

Se perdió tiempo acá, así que queda anotado para que no vuelva a pasar.

**1. Avast intercepta TLS.** Reemite todos los certificados HTTPS con su raíz
`Avast Web/Mail Shield Root`. Git y `curl` andan (usan el almacén de Windows); **Python no**
(usa `certifi`). PlatformIO lo enmascara como un **`HTTPClientError:` vacío** que no menciona
certificados, así que parece un problema de red. **La red está bien.**

Solución: bundle combinado en `C:\Users\alumnos\.platformio\ca-bundle-avast.pem`. Antes de
cualquier `pio`, exportar `SSL_CERT_FILE` y `REQUESTS_CA_BUNDLE` apuntando ahí.

**2. Faltaba el toolchain entero.** Sólo estaba `contrib-piohome`. Ya está la plataforma
`teensy` + ARM GCC.

**3. `fatal error: SPI.h: No such file or directory`.** Las `lib_deps` no están pinneadas, así
que al bajar todo en limpio entró **Adafruit BusIO 1.17.4**, que trae un
`Adafruit_SPIDevice.h` con `#include <SPI.h>`. El buscador de dependencias de PlatformIO sólo
mira los `#include` de *nuestro* `.ino`, que pide `Wire` y no `SPI`. El 18/08 compilaba porque
había caché con una versión vieja. **Es deriva de versión, no código roto.**

Arreglado agregando `SPI` a `lib_deps`.

> **⚠️ LA MESA DEL ARQUERO TIENE EL MISMO PROBLEMA, SIN ARREGLAR.** Confirmado compilando:
> `arquero-teensy-zircon/pruebas/cuadrado-giroscopo` y `funciona/seguir-y-despejar` fallan
> igual. Es una línea en cada `platformio.ini`. **No se tocaron: hay que avisarles.**

**Y una cosa buena:** el cuelgue de `scons` en `Linking` del 18/08 **no volvió a pasar** —
todos los builds de hoy corrieron derecho, el más largo en 10,3 s. Con el toolchain recién
bajado no se colgó ni una vez. Probable que aquello fuera el toolchain a medio instalar.

---

## 7. La carpeta no era un repo

La carpeta de esta computadora **no era un clon de git**: era una copia congelada del 28/04
con 202 archivos, mientras el repo tenía 590. Faltaba **todo `codigo-clases/`** — el firmware
del delantero, el del arquero y las bitácoras de julio y agosto. Estábamos 3 meses y medio
atrasados.

Ya es un clon de `main` de verdad. Se hizo backup de la copia vieja en
`Documents/Martes 1830/_backup_local_2026-08-25` antes de tocar nada.

De la copia vieja, 38 archivos los había borrado Franco a propósito entre mayo y junio y se
sacaron; **3 nunca se subieron** y quedaron sin tocar: `alumnos/laureano/2804.py`,
`bitacora_2804.md` y `calibrador.py`. Además `alumnos/laureano/robot_seguidor_linea.py` local
tiene la recalibración del 5/5 (negro=5, blanco=26) que en GitHub sigue en 15/99.

---

## Qué queda pendiente

1. **Cerrar la duda A/B:** encender el robot sobre el verde **de la cancha** y leer el banner.
   Si dice `escape ACTIVADO`, los umbrales sirven. Requiere llevar el robot a la cancha —
   el cable USB no llega, así que hay que ir con batería y sin serial.
2. **Ver el escape despegarse de verdad**, en cancha, donde hay a dónde ir. Hoy la segunda vez
   se quedó 24 s adentro.
3. **`linea: ON/OFF` en la telemetría de cada 2 s**, y el LED del hub avisando. Sin esto, un
   robot que se encendió sobre blanco juega sin protección y nadie se entera.
4. **Probar el giroscopio sin `setExtCrystalUse(true)`**, con un programa en `pruebas/` que
   muestre el estado del sistema y el rumbo en vivo. Se puede hacer sobre la mesa.
5. **Avisar a la mesa del arquero del `SPI.h`** — sus dos programas con BNO055 no compilan.
6. **La patada desde la órbita** sigue sin resolverse. Se cierra con cinta métrica: pelota a
   30, 60 y 100 cm y leer `Xp`, para saber si `XP_ORBITA = 22` está en centímetros.
7. **¿Se levantaron los sensores?** Nadie lo confirmó, y es la mitad de la duda A/B.

## Nota de método

Dos veces hoy afirmé algo de más y los datos me corrigieron: dije que la placa tenía un
firmware distinto al del repo (era el `atan2` corriendo con un `XpBueno` viejo), y dije que
las dos veces el escape se había despegado (sólo la primera). **Las dos veces el error fue
sacar conclusión de una sola observación.** El loop a 631.000/s y el "ya me despegue" estaban
los dos en el log; había que leerlos, no inferirlos.


---
---

# SEGUNDA PARTE — lo que siguió después de medir en la mesa

## 8. Los umbrales de la mesa NO servían en la cancha

Con `{542, 546, 608}` el escape andaba **en la mesa** pero **no en la cancha**. Se descartó que
fuera el reset (el robot arranca desde cero, sin energía previa, apoyado en el verde) y que el
binario estuviera viejo (se reflasheó y se verificó el hex).

**La hipótesis la puso Máximo:** que la autoprotección del arranque estuviera anulando el
escape en cada encendido. **Los números se la confirmaron.**

### Cómo se midió en la cancha sin cable

El cable USB no llega a la cancha, y el banner del arranque **se pierde**: el Teensy descarta
lo que manda por USB si no hay ningún host escuchando. Se agregó instrumentación que guarda
las lecturas en RAM y las reimprime cada 2 s:

```
linea: ON   arranque 625/381/740  visto 369..789 311..783 397..850  umbrales 707/582/795
```

Se prende el robot en la cancha, se lo deja andar, y se lo trae a la mesa **SIN APAGAR LA
BATERÍA** para engancharle el USB. Si no se corta la energía, el programa sigue corriendo y los
números siguen ahí.

### El resultado

| sensor | verde de la CANCHA | umbral que tenía | |
|---|---|---|---|
| 1 | **625** | 542 | lo superaba por 83 |
| 2 | 381 | 546 | ok |
| 3 | **740** | 608 | lo superaba por 132 |

**Dos de los tres sensores leían el verde de la cancha como si fuera blanco.** La autoprotección
hacía exactamente lo que debía con esa información, y anulaba el escape en cada arranque — **en
silencio**, porque el aviso sale por serie y en la cancha no hay cable.

El verde de la cancha es **mucho más reflectante** que la muestra de la mesa: 625/381/740 contra
319/330/452. Los umbrales de la mesa nunca iban a servir allá.

### Lo que quedó cargado

```cpp
int UMBRAL_LINEA[3] = { 707, 582, 795 };   // punto medio verde-blanco, EN CANCHA
const bool PROTECCION_ARRANQUE = false;    // apagada a pedido de Máximo
```

⚠️ **El sensor 3 quedó con 55 puntos de margen a cada lado**, y el verde ya varió más que eso
entre dos puntos. Es el de ADELANTE. Primer sospechoso si dispara en falso o no dispara.

⚠️ **El blanco no está limpio.** Los máximos vienen del acumulador min/max, que sigue acumulando
después de volver a la mesa: se vio a S3 trepar de 814 a 850 ya estando en la mesa. El VERDE sí
está limpio, porque sale de `arranque`, que se congela al encenderse. **Para cerrarlo:** congelar
el min/max a los ~90 s del arranque y repetir una pasada.

---

## 9. Tolerancia de patada adaptativa por distancia al arco

**Idea de Máximo**, a partir de una observación propia: *de lejos no patea*. Orbita, no cumple
nunca la condición, y se rinde a los 20 s sin tirar.

```cpp
const bool  TOLERANCIA_ADAPTATIVA = true;
const int   ARCO_LEJOS_MIN = 50;
const int   ARCO_LEJOS_MAX = 200;
const float TOL_ANG_LEJOS  = 15.0;    // arrancó en 10, se subió a 15
```

Dentro de esa banda **las dos** tolerancias pasan a 15 grados; fuera quedan en 8.

**Por qué las dos y no sólo la del arco:** se midió en vivo que la que bloquea es la de la
PELOTA. Se vieron separaciones con el arco de **0,3 y 0,8 grados** (perfectas) con `angPelota` en
**21 y 29 grados**. Aflojar sola la del arco no cambiaría nada.

⚠️ **La geometría va al revés de la intuición, y quedó dicho en el código.** Un ángulo fijo se
ABRE con la distancia: 15 grados son 27 cm de desvío a 1 m y **54 cm a 2 m**. Aflojar de lejos
afloja justo donde ya se es más impreciso. **Se hizo igual por OPORTUNIDAD, no por puntería:** un
tiro torcido al lado correcto de la cancha le gana a orbitar 20 s y rendirse. Es una apuesta
consciente.

La telemetría ahora imprime `arcoX=` y `tol=` para poder ajustarlo con datos.

---

## 10. Restaurado el golpe de freno (era de gviollaz, 18/08)

Máximo volvió a ver el síntoma: *"cuando patea y está la línea, la atraviesa y se sale de la
cancha"*. Es **exactamente** el problema que gviollaz ya había resuelto el 18/08 en `5c38605`,
revertido ese mismo día como **daño colateral** — se dieron de baja los cuatro cambios del día
juntos, y el sospechoso principal era el arreglo del giroscopio, no éste. **El freno nunca se
demostró culpable de nada.**

Se restauró el diseño original sin cambios: `VEL_FRENO = 240`, `MS_FRENO = 150`.

> **La regla, del reglamento nuevo:** cruzar la línea con más de medio cuerpo es **GOL EN
> CONTRA**. Cancelar la patada no alcanza; hay que matar el envión.

El `cherry-pick` no aplicó limpio (el archivo cambió mucho y duplicaba medio fuente), así que se
aplicó a mano.

### 🔴 Y AUN ASÍ SE SIGUE SALIENDO

Es lo que queda abierto al cierre de la clase. Ideas ordenadas en "Qué queda pendiente".

---

## 11. 🎁 La otra mesa resolvió las unidades de la cámara

Mientras trabajábamos, la mesa del **arquero** pusheó el hallazgo que cerraba el pendiente que
veníamos arrastrando: **la cámara exagera por un factor PAREJO de 2,87**, y la causa es que quedó
montada a **~8 cm del piso** en vez de los 18,7 para los que fue calibrada.

Ver `arquero-teensy-zircon/bitacora/2026-08-25-tabla-de-conversion-de-la-camara.md`. Vale la pena
leerla entera: **predicen la altura a partir del factor (7,9 cm) y después la confirman con regla
(7-8 cm)**.

⚠️ **ESE 2,87 ES DEL ARQUERO Y NO SE PUEDE ASUMIR ACÁ.** Otro robot, otra cámara. Ya falló dos
veces transferir datos entre mesas. Lo que SÍ transfiere es el método y el programa.

### Por qué nos importa tanto

Si acá diera algo parecido:

| Nuestro parámetro | Dice | Serían, en cm reales |
|---|---|---|
| `XP_ORBITA = 22` | "orbita a 22 cm" | **7,7 cm** |
| `ARCO_LEJOS_MIN = 50` | queríamos 50 cm | **17 cm** |
| `ARCO_LEJOS_MAX = 200` | queríamos 200 cm | **70 cm** |

**Explicaría la patada de una vez.** La órbita tiene radio **fijo de 17,5 cm** por geometría del
chasis (`R = 2L`, con L = 8,75 cm). Pero el robot empieza a orbitar cuando cree que la pelota
está a 22, que serían 7,7 cm reales: **orbita alrededor de un punto que cae detrás de la
pelota.** La pelota no queda en el centro del círculo y sale descentrada — que es exactamente el
síntoma de 22-34 grados que se viene midiendo hace dos clases. `XP_ORBITA` debería ser
~17,5 × factor, o sea **~50 y no 22**.

Y la banda `50..200` que se cargó hoy **no significa lo que se quiso**: 200 cm reales serían 574
unidades de cámara, y la cámara **satura en 200**. Era inalcanzable.

### Ya está portada

`pruebas/tabla-camara/` está copiada al delantero y **compilando**. Mide **sin cable**: guía con
destellos del LED, el robot se acuerda, y se le pregunta después en la mesa con la tecla `m`.
Seis posiciones, ~1 min 30. Es portable tal cual porque toca los 9 pines de motor sólo para
ponerlos en CERO, y los dos robots usan los mismos 9 pines.

**Se le arregló el bug que a ellos les corrió toda la tabla un lugar:** el anuncio de la posición
1 salía pegado al parpadeo del final de la cuenta inicial. Se agregaron 1500 ms de LED apagado
antes de contar. **Si a la otra mesa le sirve, que se lo lleven.**

---

## 12. Infraestructura que quedó arreglada

- **PlatformIO andando.** Tres problemas encadenados: Avast intercepta TLS (rompe HTTPS desde
  Python, y PlatformIO lo enmascara como un `HTTPClientError:` vacío), faltaba el toolchain
  entero, y `SPI.h` por deriva de versión de Adafruit BusIO.
- **La carpeta es un clon de git de verdad.** Estaba congelada en el 28/04 con 202 archivos
  contra 590 del repo.
- ⚠️ **LA MESA DEL ARQUERO TIENE EL PROBLEMA DEL `SPI.h` SIN ARREGLAR.** Confirmado compilando:
  `pruebas/cuadrado-giroscopo` y `funciona/seguir-y-despejar` fallan. **Es una línea en cada
  `platformio.ini`. Hay que avisarles.**

---

## 13. El giroscopio: indicación del profe Gustavo

Guardada para cuando se retome. **No tocar el giroscopio sin esto primero:**

1. **El giroscopio original SE QUEMÓ** (les pasó a los robots que compitieron en 2025).
2. **El actual es un reemplazo y no se sabe si es el mismo modelo** que el del arquero.
3. **El programa del 2025 SÍ usaba el giroscopio y funcionaba.** Leer ese código y aprender de
   ahí, en vez de inventar.

**Deducción del 25/08, sin verificar en hardware:** el chip **se presenta como un BNO055**.
`Adafruit_BNO055::begin()` devuelve `true` sólo si lee el chip ID `0xA0`, y el banner dice
"contesta pero da ceros (9/20 lecturas útiles)" — mensaje que está *después* del `begin()`.
Entonces responde por I2C y se identifica bien, pero **la fusión no corre**. Encaja con un **clon
barato de BNO055**: se presentan con el ID correcto y traen el firmware de fusión roto.

Eso explicaría por qué el arquero anda con el MISMO código y éste no. **Si es hardware, ninguna
corrección de software lo va a arreglar** — y el 18/08 ya se perdió una clase persiguiendo
`setExtCrystalUse(true)` como culpable.

---

## Qué queda pendiente — ORDENADO POR PRIORIDAD

### 🔴 1. SE SIGUE SALIENDO DE LA CANCHA AL PATEAR

Ideas, de mayor a menor impacto esperado:

**a) Acortar la patada. `MS_PATADA = 1000` → ~350 ms.** Es la sospecha más fuerte y el cambio más
barato. La pelota se va del robot en los primeros ~200 ms; los otros 800 ms de 240 de PWM **ya no
empujan la pelota: empujan al robot**. Es envión puro, generado después de que la jugada terminó.
Ataca la causa en vez del síntoma.

**b) Alargar el freno. `MS_FRENO = 150` → 250-300 ms.** 150 ms de contra-empuje contra 1000 ms de
aceleración es poco. Va de la mano con (a): si se acorta la patada, quizá no haga falta.

**c) Subir `VEL_ESCAPE` de 100.** La huida en sí es lenta. Una vez frenado, cuanto antes se
despegue, mejor.

**d) MEDIR CUÁNTO SE PASA, en centímetros.** Sigue pendiente desde el 18/08 y **sin ese número
todo lo anterior es a ciegas**. Marcar dónde está la línea, dejarlo patear hacia ella, medir
cuánto la cruzó. Con y sin freno. Si se pasa 2 cm no valía la pena; si se pasa 15, es gol en
contra.

**El encuadre que conviene tener:** un sensor de línea es una señal de *"ya estás ahí"*, no de
*"estás por llegar"*. **Contra la física no se puede reaccionar** — hay que llegar más despacio o
no generar esa velocidad. Por eso (a) es mejor que (b) y (c): las dos últimas reaccionan, la
primera evita.

### 2. Correr `tabla-camara` en el delantero
Ya está portada y compilando. Da el factor propio y probablemente cierra el problema de la
patada (ver sección 11).

### 3. Congelar el min/max a los 90 s
Para tener un blanco de cancha limpio y confirmar los umbrales.

### 4. Avisar a la mesa del arquero del `SPI.h`
Sus dos programas con BNO055 no compilan.

### 5. El giroscopio
Con la indicación de la sección 13. No antes.

### 6. Sensor 3: 55 puntos de margen
Vigilarlo.

## Nota de método

Tres veces hoy afirmé algo de más y los datos me corrigieron: dije que la placa tenía un firmware
distinto al del repo (era el `atan2` corriendo con un `XpBueno` viejo), dije que las dos veces el
escape se había despegado (sólo la primera), y propuse el orden de encendido como causa de que
fallara en cancha (no era). **Las tres veces el error fue sacar conclusión de una sola
observación.**

Y una que salió bien: **la hipótesis que destrabó la clase la puso Máximo**, no yo. Yo venía
detrás de la explicación eléctrica y de las superficies; él dijo "es la función del arranque que
lo anula" y los números le dieron la razón.
