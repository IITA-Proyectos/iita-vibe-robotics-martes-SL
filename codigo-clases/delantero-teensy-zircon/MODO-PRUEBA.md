# Cómo quedó el robot el 2026-09-15 — y qué hay que tocar la clase que viene

**Lo primero, en una línea: el robot NO está en condiciones de jugar tal como quedó.**
La patada está a la mitad de potencia porque la bajamos para poder mirarla.

Este archivo existe para que nadie tenga que buscar en la bitácora cuál era el valor original.
Todos los números están también escritos **al lado de cada constante** en
[`funciona/delantero/delantero.ino`](funciona/delantero/delantero.ino).

---

## 🔴 1. HAY QUE VOLVER ESTO ANTES DE JUGAR

| Constante | Línea | **Está en** | **Tiene que ir a** |
|---|---|---|---|
| `VEL_PATADA` | `delantero.ino:312` | **110** | **215** |

**Por qué está en 110:** el equipo pidió bajar mucho la velocidad de la patada para poder *ver*
qué hace — si detecta el blanco y si no hace cualquier cosa. A 215 el golpe dura 420 ms y no se
llega a mirar nada.

**Ojo con una cosa:** se bajó **sólo la velocidad**, el tiempo quedó en 420 ms. Como la distancia
es velocidad × tiempo, hoy la patada recorre **alrededor del 40 %** de lo que recorría. Es lento
**y corto**, y eso es a propósito.

`MS_PATADA = 420` y `VEL_FRENO = 240` **no se tocaron**: son los de juego.

---

## ⚙️ 2. Valores nuevos de hoy — están bien, pero NINGUNO se validó en jugada completa

No hay que cambiarlos. Hay que **mirarlos andar** y anotar qué pasó.

| Constante | Línea | Valor | Recorrido de hoy |
|---|---|---|---|
| `UMBRAL_LINEA[3]` | `:908` | `390 / 427 / 413` | eran `663/661/757` — **medidos en la tela** |
| `MS_RETROCESO_LINEA` | `:654` | 300 ms | 🔴 **nunca se probó**, se cargó al cerrar |
| `MS_CIEGO_LINEA` | `:655` | 1000 ms | 🔴 nunca se probó |
| `VEL_ESCAPE_FUERTE` | `:630` | 200 | |
| `XP_ORBITA` | `:116` | 34 | 22 → 34 → 38 → **34** |
| `VEL_ORB_IMPULSO` | `:240` | 120 | 99 → 130 → **120** |
| `VEL_ORB_TRASERA` | `:242` | 67 | 48 → 75 → **67** |
| `VEL_AVANCE` | `:162` | 95 | era 55, **debajo del piso de arranque** |
| `MS_LINEA_CONFIRMA` | `:553` | 5 ms | 🔴 nunca se probó (es del 08/09) |
| `RAMPA_PATADA_PASO` | rampa de la patada | +15 cada 5 ms | 🔴 nunca se probó (es del 08/09) |

> ⚠️ **`XP_ORBITA = 34` es tanteo, no cálculo.** El factor de escala de la cámara nunca se midió,
> así que **no sabemos cuántos centímetros son 34**. Se corrió a ojo hasta que dejó de chocar la
> pelota. Lo que cierra esto de verdad es `pruebas/tabla-camara/`, pendiente desde el 25/08.

---

## 🚫 3. Constantes que quedaron SIN USO

No las borré: si hay que volver atrás, están.

| Constante | Línea | Por qué quedó sin uso |
|---|---|---|
| `VEL_FRENO` | `:701` | lo reemplazó el freno **eléctrico** (`frenar()`) |
| `MS_FRENO` | `:702` | ídem |
| `MS_ESPERA_LINEA` | `:594` | era el freno de 1 s antes de retroceder. **Se sacó**: dejaba el robot quieto y después no llegaba a moverse |
| `MS_ESCAPE_CIEGO` | `:629` | lo reemplazó `MS_RETROCESO_LINEA` |

---

## 💾 4. Cómo volver a la versión anterior del escape

Si el retroceso inmediato de 300 ms **no funciona**, la versión anterior (freno eléctrico 1 s +
retroceso ciego 200 ms) está guardada entera:

```
respaldos/delantero-2026-09-15-freno1s-antes-de-retroceso300.ino
```

Para volver, copiarla encima de `funciona/delantero/delantero.ino` y recargar.

> ⚠️ **No la copien AL LADO del original.** PlatformIO compila todo `.ino` que encuentre bajo
> `src_dir`, así que dos en la misma carpeta rompen el build con
> *"multiple definition of setup()"*. Por eso vive en `respaldos/`.

---

## 📋 5. Orden sugerido para la clase que viene

1. **Volver `VEL_PATADA` a 215.** Un cambio, un número.
2. **Probar una jugada completa** y anotar qué pasa. Nada de esto se vio de punta a punta.
3. **Correr `pruebas/piso-de-pwm/`** sobre la tela. Existe desde julio y nunca se corrió: ese
   *"~70 desde quieto / ~40 rodando"* que usamos como base para **todo** nunca se midió, y encima
   es de otra superficie.
4. **Correr `pruebas/tabla-camara/`** y cerrar lo de `XP_ORBITA` con un número real.
5. **Verificar el giroscopio girando el robot a mano** — es un minuto, y ahora la telemetría por
   fin lee en vivo.

---

## 🧭 6. Cómo saber qué está cargado, sin adivinar

El robot lo dice solo. Monitor a 19200, y en el arranque imprime:

```
orbita si Xp<34
orbita: impulso 120 x 300 ms  ->  crucero 67   (max 20 s)
   S1=DERECHO   S2=IZQUIERDO   S3=DELANTERO  (medido 15/09)
Linea: sensores leen ... umbrales 390 / 427 / 413   confirma 5 ms
   al ver linea: RETROCEDE YA 300 ms a 200  (sin mirar la linea hasta los 1000 ms)
Giroscopo: (fusion corriendo, SYS_STATUS=5) OK
```

**El `SUCCESS` del cargador no prueba que el programa llegó a la placa** — está anotado como
trampa desde el 11/08, y hoy volvió a pasar. El banner sí.
