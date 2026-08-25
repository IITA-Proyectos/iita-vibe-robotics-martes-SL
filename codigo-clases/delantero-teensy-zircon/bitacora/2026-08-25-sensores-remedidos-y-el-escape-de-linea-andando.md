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
