# Recalibrar la cámara — qué encontré y cómo se hace

**2026-08-04.** Escrito antes de tocar la cámara, leyendo el script que hoy corre en ella:
[`../robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py`](../../../futbol-roboliga2026/robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py).

Herramienta de banco: [`calibrar-umbrales.py`](calibrar-umbrales.py) — se abre con el **OpenMV IDE**.

---

## Lo primero: hay un paso que falta ANTES de tocar los umbrales

**CALCULADO leyendo el script — líneas 31 a 33:**

```python
sensor.set_auto_whitebal(True)     # ← automático
sensor.set_auto_gain(True)         # ← automático
#sensor.set_auto_exposure(True)    # ← ni siquiera se toca
```

La cámara está con **ganancia y balance de blancos en automático**. Se re-acomoda sola,
todo el tiempo, mirando **toda la escena**.

### Por qué eso arruina cualquier umbral

Un umbral de color dice "naranja es esto". Pero con el automático prendido, cuando el robot
gira y le entra al cuadro una ventana, una pared blanca o el arco oscuro, la cámara **sube o
baja la ganancia para compensar**. La pelota no se movió, la luz de la sala no cambió — pero
**los números que devuelve la cámara para la pelota sí cambiaron**.

Es como medir a alguien con una cinta métrica que se estira sola según quién más haya en la
habitación. No importa cuánto cuides la medición: el instrumento no es estable.

**Consecuencia práctica:** si hoy calibran los umbrales sin congelar esto primero, los
umbrales nuevos van a durar hasta que el robot dé media vuelta. Van a pensar que calibraron
mal. No van a haber calibrado mal: van a haber calibrado sobre arena.

### El costo de congelar, dicho honestamente

Congelar tiene una contra real: si cambia la luz del lugar (otra sala, otra hora, la cancha
de la Roboliga), **hay que recalibrar**. No se adapta solo.

Pero ese costo se paga **en el box, cuando ustedes quieren, y tarda 5 minutos**. El costo de
no congelar se paga **solo, en el medio de un partido, sin que nadie entienda por qué**.
Por eso se congela.

---

## Lo segundo: el mínimo de tamaño está en 7 píxeles

**CALCULADO — línea 120 del script:**

```python
naranja_blobs = img.find_blobs([naranja_threshold], pixels_threshold=7, area_threshold=7, ...)
```

`pixels_threshold=7` significa: *"cualquier mancha naranja de 7 píxeles o más, es candidata a
ser la pelota"*. Siete píxeles es una mancha de **3 píxeles de ancho**. Una basurita.

¿Cuánto mide la pelota de verdad? Salió de la propia homografía del script
(pelota de 4,3 cm de diámetro, cámara a 18,7 cm):

| Distancia de la pelota | Ancho en la imagen | Área aproximada |
|---|---|---|
| 150 cm (lo más lejos que el firmware cree) | ~8 px | **~50 px** |
| 100 cm | ~13 px | ~135 px |
| 60 cm | ~21 px | ~350 px |
| 22 cm (donde arranca la órbita) | ~48 px | ~1800 px |

O sea: **la pelota más chica que tiene sentido creer mide ~50 px, y el filtro deja pasar de 7**.
Hay un factor 7 de margen regalado al ruido. Cualquier reflejo naranja, una zapatilla, una
silla, la madera del piso con la luz justa — todo eso entra.

> ⚠️ Ese cálculo usa la escala del **piso**, y la pelota es una esfera, así que es una
> aproximación. Además `find_blobs` sólo cuenta los píxeles que pasan el umbral: si media
> pelota queda en sombra, el blob real es **más chico**. Por eso el número final **se mide**,
> no se deduce — la herramienta lo imprime.

---

## El mapa de la imagen: dónde miente y dónde no

Corrí la homografía del script para cada fila de la imagen (columna del medio, `u=160`).
**CALCULADO, no medido.** La imagen es de 320 × 240.

| Fila de la imagen (`v`) | Distancia que reporta | Cuánto vale 1 píxel de error |
|---|---|---|
| 0 a 20 (el techo) | **siempre 200 cm** — es el recorte | — |
| 33 | 150 cm | 4,5 cm |
| 40 | 124 cm | 2,8 cm |
| 60 | 84 cm | 1,4 cm |
| 84 | 60 cm | 0,7 cm |
| 120 | 41 cm | 0,3 cm |
| 180 | 25 cm | 0,17 cm |
| 240 (el piso de la imagen) | **17,4 cm** | 0,12 cm |

Tres cosas se leen de esta tabla:

1. **El techo de la imagen es veneno.** Las 20 filas de arriba siempre dan `Xp = 200`. Ese es
   el famoso "valor de recorte". No es un error: es la cuenta diciendo *"esto está en el
   horizonte"*. El firmware ya las descarta (`XP_MAX = 150`, `delantero.ino`, constante
   `XP_MAX`), y de hecho descarta todo lo que esté arriba de la fila 33.

2. **Arriba, un píxel vale una barbaridad.** En la fila 33, moverse **un solo píxel** cambia la
   distancia en 4,5 cm. Abajo, en la fila 180, un píxel vale 1,7 mm. La misma cámara, la misma
   pelota: **la medición de cerca es 26 veces más firme que la de lejos**. Es geometría, no un
   defecto — pasa lo mismo cuando mirás la ruta y no podés decir si ese auto está a 300 o a 400 m.

3. **El salto de 60 a 146 cm que vieron NO es ruido.** 60 cm es la fila 84; 146 cm es la fila 33.
   Son **50 píxeles de distancia**. Ningún ruido de medición mueve un blob 50 píxeles con la
   pelota quieta. Lo que pasó es otra cosa: o el programa **agarró otra mancha** naranja más
   arriba, o la mancha **se fusionó** con la pelota (`merge=True` en la línea 120) y el centro
   del blob se fue para arriba. En los dos casos, la causa es la misma: **el umbral deja pasar
   cosas que no son la pelota**. Que es justo lo que venimos a arreglar.

---

## El orden de la calibración (el orden IMPORTA)

Si se hace en otro orden, se tira el trabajo.

```
1. CONGELAR exposición, ganancia y balance de blancos
        ↓   (cambiar la exposición cambia TODOS los colores)
2. Medir el color real de la pelota  →  umbral LAB
        ↓   (el umbral define qué blobs aparecen)
3. Medir el tamaño de la pelota lejos →  pixels_threshold
```

**Por qué no al revés:** la exposición es cuánta luz entra. Si calibran el umbral y después
tocan la exposición, todos los colores se corren y el umbral que midieron ya no sirve.
La exposición se elige **una vez**, se anota, y no se toca más.

---

## El protocolo de banco (10 minutos)

Lo que necesitan: la cámara conectada por USB, el OpenMV IDE, la pelota, una regla o cinta
métrica, y la luz **con la que van a jugar** (si van a jugar de noche con los tubos prendidos,
calibren de noche con los tubos prendidos).

### Paso 0 — la cámara mirando la cancha

Enciendan con la cámara apuntando **a la cancha**, no al techo ni a una ventana. Los primeros
3 segundos el programa deja que el automático se acomode, y **congela lo que haya elegido**.
Si en ese momento está mirando una lámpara, congela mal.

### Paso 1 — abrir la herramienta y anotar la exposición

Abrir [`calibrar-umbrales.py`](calibrar-umbrales.py) en el OpenMV IDE y darle play.
En el terminal va a imprimir algo así:

```
--- lo que eligio el automatico ---
   exposicion (us)        = 18342
   ganancia (dB)          = 12.5
   balance RGB (dB)       = (...)
--- CONGELO con la que eligio el automatico: 18342 us
```

**Anoten esos tres números en la bitácora.** Esa es la "receta de luz" de hoy.

Después pongan ese número de exposición en `EXPOSICION_US` arriba del archivo. A partir de
ahí, cada vez que arranque va a congelar **siempre en el mismo lugar**, sin depender de a
dónde estuviera mirando.

> **Si el IDE tira un error rojo en alguna de las líneas de `set_auto_...`:** puede pasar. Yo
> escribí esas llamadas según la API de OpenMV, pero **no las probé contra esta cámara** — no la
> tengo. Si falla, el IDE dice exactamente en qué línea. Anoten el mensaje en la bitácora y
> saquen el argumento que se queja (por ejemplo, dejar `sensor.set_auto_gain(False)` sin el
> `gain_db=...`): congelar sin especificar el valor sigue sirviendo, sólo que congela en lo que
> la cámara tuviera puesto. Los tres `leer(...)` del arranque ya están protegidos y avisan
> solos si no pueden leer un valor.

### Paso 2 — medir el color de la pelota

En pantalla hay un **recuadro blanco chico** en el medio. Acerquen la pelota hasta que
**tape el recuadro entero** — que no se vea nada de piso adentro.

> Si le entra piso, el umbral sale más ancho de lo que corresponde y van a terminar
> detectando el piso. Esa es exactamente la enfermedad que vinimos a curar.

El terminal imprime:

```
  UMBRAL ANCHO    (24, 71, 22, 68, 15, 90)      <-- EMPEZÁ ACÁ
  UMBRAL APRETADO (35, 58, 34, 51, 41, 72)      <-- si ve manchas, pasá a este
```

Copien el **ANCHO** a la variable `NARANJA` de la herramienta y vuelvan a darle play.

### Paso 3 — la pregunta que decide todo: ¿cuántas ve?

Con la pelota sola en el piso, en el medio de la cancha, el terminal dice:

```
NARANJAS: 4 con el minimo de hoy (7 px)  |  1 con el candidato (40 px)   <-- QUEREMOS 1
```

**El objetivo es que el primer número sea 1.** No "que se vea bien", no "que parezca":
**uno**. Contar es fácil y no admite discusión; opinar si un umbral "está bien" no se puede
anotar en la bitácora.

- ¿Ve **más de una**? El umbral es demasiado ancho → pasen al **APRETADO**.
- ¿Ve **cero**? Es demasiado angosto → vuelvan al ANCHO y agranden un poco el rango de `L`
  (los dos primeros números), que es el que más se mueve con la luz.
- **Un cambio por vez.** Cambien un número, miren, anoten. Si cambian tres y empeora, no van
  a saber cuál fue.

Cada mancha que ve la imprime con su tamaño y su posición, así que se puede ver **qué** es lo
que está confundiendo:

```
   PELOTA? px= 1832  en (u=161, v=203)  ->  Xp=  21.6 cm  Yp=  -0.3 cm
   mancha  px=   11  en (u= 47, v= 28)  ->  Xp= 200.0 cm  Yp= -78.4 cm  <-- ARRIBA DE LA ROJA
```

Con las líneas dibujadas en pantalla se ve solo: lo que aparece **arriba de la línea roja** el
firmware ya lo tira. Lo que está **abajo de la amarilla** es donde la cámara mide firme.

### Paso 4 — medir el tamaño de la pelota lejos

Pongan la pelota a **1,50 m** (medida con la cinta, no a ojo) y anoten el `px=` que imprime.
Después a 1 m, a 60 cm y a 22 cm. Esa tabla es la que decide `pixels_threshold`:

> **Regla:** `pixels_threshold` = más o menos **la mitad** del tamaño de la pelota a la
> distancia más lejos que quieran creerle. Mitad, no el valor exacto: si media pelota queda
> en sombra el blob se achica, y no queremos perder la pelota de verdad por apretar de más.

Anoten los cuatro números en la bitácora. Es la primera vez que el equipo va a tener medida
la relación tamaño ↔ distancia, y sirve para mucho más que esto.

### Paso 5 — pasar los números al script de competencia

Recién ahora se toca
[`../robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py`](../../../futbol-roboliga2026/robots-2025/vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py):

| Dónde | Qué va | Hoy dice |
|---|---|---|
| líneas 31-33 | congelar los tres automáticos, con los valores del Paso 1 | los tres en `True` |
| línea 58 | `naranja_threshold` = el umbral del Paso 3 | `(21, 67, 18, 79, -32, 127)` |
| línea 120 | `pixels_threshold` y `area_threshold` = el del Paso 4 | `7` |

⚠️ **Ese archivo lo comparten los dos robots.** Antes de tocarlo, avisen a la sesión del
arquero. Y ojo: **las dos cámaras van a necesitar números distintos** (son dos sensores
distintos, montados distinto), así que tarde o temprano ese archivo se va a tener que partir
en uno por robot. Es una decisión del equipo, no la tomo yo acá.

### Paso 6 — la prueba que dice si sirvió

Con el script nuevo cargado y el Teensy conectado, **pelota quieta**, mirar el monitor serie a
19200 durante 30 segundos y anotar el `Xp` mínimo y el máximo.

- **Antes:** saltaba de 60 a 146 cm.
- **Bien:** varía pocos centímetros y no aparecen ni `Xp = 200` ni `Yp = ±100`.

Ese es el criterio de éxito. Está en centímetros y se anota. Si sigue saltando, **la
calibración no alcanzó** y hay que volver al Paso 3 — no seguir para adelante tocando el
firmware.

---

## Dos cosas que quedan aclaradas de paso

### Descartado: el problema NO es que se desincronice el cable

El [README de visión](../../../futbol-roboliga2026/robots-2025/vision-openmv/README.md) dice que el protocolo *"no tiene
forma de recuperarse si se desincroniza"*. **Eso no es correcto**, y conviene saberlo porque
si no van a sospechar del cable cuando el problema está en la vista.

Barrí los 320 × 240 píxeles de la imagen pasándolos por la cuenta del script: por los recortes
que ya tiene, **`Xp` nunca puede pasar de 200 y `Yp+100` tampoco**. Los datos viven en 0..200.
Y las marcas son 201, 202 y 203. O sea: **ningún byte de datos puede hacerse pasar por una
marca, nunca.**

Los dos firmwares aprovechan eso: buscan el 201 y verifican que después vengan el 202 y el 203
(en el delantero de hoy, la función `leerCamara()`; en el de 2025, `delantero.ino` alrededor de
la línea 287). Si se pierde un byte, el paquete no valida y se descarta, y al siguiente ya está
alineado de nuevo. **Se resincroniza solo.**

→ Hay que corregir ese párrafo del README de 2025. Queda anotado.

### Abierto y hay que medirlo: la órbita puede caer justo en la zona ciega

**Esto NO está confirmado. Es una sospecha con dos cuentas firmes y un eslabón sin verificar.**

- La fila más baja de la imagen (`v = 240`) da **17,4 cm**. Más cerca que eso, la pelota se sale
  del cuadro por abajo y la cámara manda `Xp = 0` — la zona ciega que ya conocían. *(CALCULADO
  con la homografía del script.)*
- La órbita gira alrededor de un punto a **R = 2·L = 17,5 cm justo adelante del robot**. Verifiqué
  la cinemática por mi cuenta: con las dos ruedas de adelante trabadas, el centro instantáneo de
  giro queda en (0, 2L) sobre el eje que va hacia adelante. La cuenta del firmware está bien.
  *(CALCULADO.)*

**17,4 y 17,5.** Si esos dos números están en el mismo sistema de referencia, la órbita deja la
pelota **exactamente en el borde de lo que la cámara puede ver** — y eso explicaría el
`... perdi la pelota orbitando` y el "tiende a alejarse de la pelota".

**El eslabón que falta:** no sé desde **dónde** mide la homografía. ¿Los 17,4 cm son desde el
centro del robot, desde la cámara, o desde el borde de adelante? Nadie lo anotó. Si el origen
no es el centro del robot, los dos números no se comparan y la sospecha se cae.

**Se contesta en 2 minutos**, y conviene hacerlo en la misma sesión de banco:

> Poné la pelota a **exactamente 30 cm del centro del robot**, medidos con la cinta, justo
> adelante. Leé el `Xp` que imprime la herramienta.
> - ¿Dice ~30? El origen es el centro del robot → **la sospecha se confirma** y hay que subir
>   el radio de la órbita o bajar la cámara.
> - ¿Dice ~20? El origen está ~10 cm adelante → los números no se comparaban, sospecha
>   descartada.
>
> Repetir a 50 cm para confirmar que la diferencia es constante y no un error de escala.

Anoten los dos pares de números. Con eso queda cerrado, para un lado o para el otro.
