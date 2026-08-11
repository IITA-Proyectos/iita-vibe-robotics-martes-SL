# 2026-08-04 — La órbita anda: impulso de arranque del campeón 2025

**Quiénes:** Gustavo Viollaz (banco y piso) + Claude (análisis y código)
**Robot:** delantero (`ROBOT2`) · Teensy serie **15708680** (COM18)
**Programa cargado:** `firmware/delantero/delantero.ino`, cargado y **verificado corriendo**

## Qué queríamos probar

Que la órbita vaya más lento, para que la cámara alcance a ver el arco mientras el robot barre.
El problema: el motor necesita ~70 de PWM para arrancar desde quieto, así que "mandar 48" no
arranca nunca.

## Qué hicimos

**Un solo cambio en el robot:** el perfil de velocidad de la órbita.

Gustavo se acordó de que el equipo ya resolvía esto antes con un impulso inicial, y pidió
buscarlo en los repos. **Estaba, y estaba justo en la órbita.** En
`futbol-roboliga2026/robots-2025/delantero/delantero.ino` (bloque `ROBOT2`, `c = 0.4`, `ic = 0.55`) la órbita del
campeón son **dos estados encadenados**, no uno:

| Estado 2025 | izq | der | trasera | dura |
|---|---|---|---|---|
| `IMPULSO_CENTRANDO_horario` (:744) | 33 | 33 | **99** | **300 ms** |
| `CENTRANDO_horario` (:613) | 24 | 24 | **72** | el resto |
| `IMPULSO_CENTRANDO_antihorario` (:673) | 33 | 33 | **99** | **500 ms** |
| `IMPULSO_INICIAL_GIRANDO` (:393) — girar en el eje | 150 | 150 | 150 | **70 ms** |

Nosotros habíamos copiado el **segundo** estado y no el primero: veníamos corriendo la trasera
a 120 fijo, sin impulso.

**Por qué el impulso de la órbita es más suave que el de girar en el eje** (99 contra 150): con
la pelota a ~17 cm, un golpe bruto **la empuja**. Si la pelota se corre, deja de estar en el
centro de la órbita y el robot se descentra solo.

## Qué pasó de verdad

✅ **La órbita anda.** *"Orbita bastante bien"* — Gustavo, con el robot en el piso.
**Validado por el humano, no por el compilador.**

❌ **No detecta el arco de lejos.** Ese es ahora el cuello de botella, y es de la cámara.

Además, 7 segundos de monitor serie dieron la evidencia en vivo de que la cámara miente:

```
>>> BUSCANDO    Xp=0    Yp=0     arco: Yaz=-6
>>> CENTRANDO   Xp=130  Yp=-23   arco: Yaz=-6
>>> BUSCANDO    Xp=130  Yp=-23   arco: no lo veo
>>> CENTRANDO   Xp=147  Yp=-25
>>> AVANZANDO   Xp=139  Yp=4
>>> CENTRANDO   Xp=41   Yp=21
>>> BUSCANDO    Xp=30   Yp=15
```

**`Xp` salta de 139 a 41 cm** entre dos cambios de estado. El robot no se movió 98 cm. En filas
de la imagen, 139 cm es la fila ~36 y 41 cm es la fila ~120: **son dos manchas distintas, en
extremos opuestos de la imagen**, no la misma cosa con ruido.

Y la máquina de estados **tiembla**: siete cambios de estado en unos segundos, porque la mancha
aparece y desaparece en menos de los 300 ms de `MS_GRACIA`. El robot no persigue una pelota,
persigue parpadeos.

## Números

### Lo que cambió en el firmware

| Constante | Antes | Ahora | De dónde sale |
|---|---|---|---|
| `VEL_ORB_IMPULSO` | (no existía) | **99** | el `180 × ic` del campeón 2025 |
| `MS_ORB_IMPULSO` | (no existía) | **300** ms | el 2025 usaba 300 y 500 ms según el sentido |
| `VEL_ORB_TRASERA` | 120 | **48** | pedido de Gustavo: "por debajo de 50" |
| `MS_ORBITA_MAX` | 9000 ms | **20000** ms | tiene que alcanzar para ~2 vueltas |
| `VEL_ORB_FRENTE` | 30 | 30 (sin cambio) | debajo del piso, no deben girar |

**Cómo volver exacto a lo de antes:** `VEL_ORB_IMPULSO = VEL_ORB_TRASERA = 120` y
`MS_ORBITA_MAX = 9000`.

**Escalera si algún día se planta:** 48 → 55 → 65 → 72. Entre 40 (piso rodando, medido) y 72
(lo que usó el campeón) no hay ningún dato: 48 anduvo, pero está en tierra de nadie.

### Verificación de la carga

`pio run -e teensy41 -t upload` → SUCCESS. **Pero el SUCCESS es del cargador, no del robot.**
Se confirmó leyendo COM18, con la línea que sólo existe en el firmware nuevo:

```
orbita: impulso 99 x 300 ms  ->  crucero 48   (max 20 s)
```

### La cámara, calculado (no medido)

Corriendo la homografía del script fuera de la cámara. Detalle completo y el protocolo de
calibración en [`../vision/README.md`](../vision/README.md).

| fila de la imagen | distancia que reporta | 1 px de error = |
|---|---|---|
| 0 a 20 | siempre 200 cm (recorte) | — |
| 33 | 150 cm | 4,5 cm |
| 84 | 60 cm | 0,7 cm |
| 180 | 25 cm | 0,17 cm |
| 240 (piso de la imagen) | **17,4 cm** | 0,12 cm |

Y el tamaño de la pelota: a 150 cm mide **~50 px**, pero el script acepta blobs desde **7 px**.
Factor 7 de margen regalado al ruido.

## Lo que quedó descartado

**El cable NO se desincroniza.** Barrí los 320×240 píxeles por la cuenta del script: los datos
viven en 0..200 y las marcas son 201/202/203, así que **ningún byte de datos puede hacerse
pasar por una marca**. Los dos firmwares buscan el 201 y validan 202/203: se realinean solos.
El README de `futbol-roboliga2026/robots-2025/vision-openmv/` dice lo contrario y **está mal**.

## Hardware confirmado para las mejoras de la próxima clase

- **Giroscopio:** el delantero 2025 usa un `BNO055` en I2C `0x28`, y ya guardaba `initialYaw` al
  arrancar (`delantero.ino:76-80`, `:270-278`). La idea del "arco 0" tiene código de referencia.
- **Sensores de línea:** `readLine(1/2/3)` = izquierdo / centro / derecho, umbrales del
  delantero **650 / 650 / 750** (`:26-28`).
- ⚠️ **El firmware de 2026 no lee ninguno de los dos.** Cero referencias al BNO. Que el código
  viejo los usara **no prueba** que hoy funcionen en esta placa.

## Qué queda pendiente

Todo en [`../firmware/delantero/MEJORAS-PENDIENTES.md`](../MEJORAS-PENDIENTES.md).
Lo primero de la próxima clase: **calibrar la cámara** con
[`../vision/calibrar-umbrales.py`](../vision/calibrar-umbrales.py).

## Dato para guardar

El Teensy del delantero tiene número de serie **`15708680`** (`USB\VID_16C0&PID_0483\15708680`).
No cambia nunca y no depende del puerto. Con los dos robots enchufados:

```
Get-CimInstance Win32_PnPEntity | Where-Object PNPDeviceID -like "*VID_16C0*"
```

y sabés cuál es cuál sin desenchufar nada. Cierra el pendiente del 28/07 de etiquetar los robots.
