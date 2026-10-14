# 2026-10-13 (martes) — PC nueva, límite lateral, despeje de 35 cm y la ESTABLE 0.1013

**Quiénes:** Diego y Laureano (cancha y pedidos) y el profe Franco (aclaraciones y pendientes)
+ Claude (instalación, código y carga)
**Robot:** el **ARQUERO** — Teensy con número de serie `13348550`
**Programas nuevos:** `respaldos/estable1013conlimiteslaterales`
**Modificados:** `funciona/seguir-y-despejar`, `pruebas/herramientas/robot.ps1`, `README.md`
**PC:** otra más: la notebook que viaja a Buenos Aires (usuario `sarap`). Ver §1.

---

## 🎯 EL RESUMEN DEL DÍA

**No hay ningún número medido hoy.** Todo lo que dice "anda" es lo que dijo el equipo mirando el
robot en la cancha. La Roboliga es el **11 de noviembre**.

**1. Se armó la PC que viaja** y compila los 27 proyectos, también sin internet (§1).

**2. Se cargaron seis versiones** (§2). La patrulla corregida del 12/10 entró por primera vez al
robot, y el equipo dijo que el despeje y la vuelta al medio andan bien.

**3. Límite lateral nuevo:** siguiendo la pelota, al pisar el lateral frena y vuelve al medio
(§3). El equipo lo probó y dijo que está bien. **Se cargó sin revisión, por el apuro.**

**4. El despeje sale 35 cm en vez de 50** (§4). Sin probar.

**5. El equipo declaró estable la 0.1013** y quedó congelada (§5). Después pidió prender la
predicción: **lo que está en el robot NO es la estable** (§2, fila 6).

**6. El profe dejó una regla de trabajo y tres pendientes para las próximas clases** (§7 y §8).

---

## 1. La PC que viaja, y con qué se compiló

🚨 **Se compiló con versiones más nuevas que el 12/10.** Si algo anda distinto que en la PC del
aula sin haber tocado el código, la diferencia está acá:

| | Hoy (esta PC) | El 12/10 (PC del aula) |
|---|---|---|
| PlatformIO Core | 6.2.0 | 6.2.0 |
| Plataforma `teensy` | **6.0.0** | 5.0.0 |
| `framework-arduinoteensy` | **1.162.0** (Teensyduino 1.62) | 1.159.0 (1.59) |
| Compilador | gcc 15.2.1 | gcc 11.3.1 |
| Adafruit BNO055 / Unified Sensor / BusIO | 1.6.4 / 1.1.15 / 1.17.4 | iguales |

- La plataforma 5.0.0 quedó **también instalada** en esta PC, por si hay que volver sin internet:
  se fija poniendo `platform = teensy@5.0.0` en el `platformio.ini`. Se compiló una copia del
  arquero con ella y compila; **no se cargó**.
- **Los 27 proyectos compilan** por PlatformIO y también por el `arduino-cli` del Arduino IDE
  2.3.10 (core `teensy:avr` 1.62.0, con la librería de DFRobot bajada a mano).
- **Sin internet compila:** probado bloqueando la red con un proxy muerto (3 proyectos).
- **La carga anduvo las seis veces** con `pio run -e teensy41 -t upload`, sin abrir el Teensy
  Loader ni apretar el botón blanco. Después de cada carga se leyó el cartel por USB.
- `pruebas/herramientas/robot.ps1` ya no tiene las rutas del taller escritas: busca el Arduino
  IDE solo. Probado `compilar`; **`cargar` no se probó**.
- Trampas de esta PC: **COM5 es un puerto Bluetooth** (a `serie.py` hay que pasarle `--puerto`),
  y PowerShell tiene los scripts bloqueados (`powershell -ExecutionPolicy Bypass -File ...`).

---

## 2. Lo que se cargó, en orden

El cartel es lo que el robot imprime por USB al arrancar.

| # | Cartel | Qué cambió | Qué dijo el equipo |
|---|---|---|---|
| 1 | `0.1012 EN PRUEBA (ubicacion nueva + patrulla corregida)` | Nada: el archivo del 12/10, que nunca se había cargado | *"Funciona todo bien el despeje y el movimiento para volver al medio del área"* |
| 2 | `0.1013 EN PRUEBA (… sale a los 10 s)` | `MS_QUIETO_PARA_PATRULLAR` 15000 → **10000** | — |
| 3 | `0.1013 EN PRUEBA (patrulla a los 10 s + limite lateral: vuelve al medio)` | Límite lateral (§3) | *"Está bien lo que pusiste"*, *"la versión actual está buena"* |
| 4 | `0.1013 EN PRUEBA (patrulla 10 s + limite lateral + despeje 35 cm)` | `msAdelante` 533 → **383** | — |
| 5 | `ESTABLE 0.1013 con limites laterales (patrulla 10 s + despeje 35 cm)` | Solo el cartel. Es la que se congeló (§5) | — |
| 6 | `0.1013 EN PRUEBA (ESTABLE 0.1013 + prediccion prendida)` | `predecirTrayectoria` false → **true** | 🚨 **LA QUE ESTÁ EN EL ROBOT** |

La revisión del cambio 2 (tres miradas independientes) no encontró nada que cambiar. Una cuenta
que dejó: sin pelota a la vista el ciclo de patrulla pasa de ~22 s a ~17 s, y el robot está fuera
del medio ~28 % del tiempo en vez de ~22 %.

---

## 3. El límite lateral

**El pedido:** siguiendo la pelota el robot se iba de largo por el costado, y si choca los muros
de afuera lo sacan 1 minuto. Que al ver la línea blanca del lateral vuelva al centro.

**Qué hace** (`volverAlMedioEnElLateral`, y la fase nueva `LATERAL_AL_MEDIO`):

1. Solo en `SIGUIENDO`. Cuenta como lateral si ve blanco **el sensor de atrás del lado hacia el
   que va y el otro no**, 3 vueltas de `loop` seguidas. Los dos juntos se toman por la línea del
   área y no cuentan.
2. Frena, y vuelve al medio **por tiempo**: `MS_MEDIO_AL_MEDIO` (2228 ms) a velocidad 100.
   **El mismo tiempo desde los dos laterales, decisión del equipo.** La cuenta del 06/10 para el
   lateral derecho da 2331 ms.
3. Mientras vuelve **no mira la pelota** (~2,7 s, cuenta). Si volviendo pisa el otro lateral,
   frena ahí.
4. En el medio: si ve la pelota la sigue; si no, termina como la patrulla (línea del área,
   enderezarse, avanzar el poquito).
5. **Sin giroscopio no hay límite**: no puede volver por tiempo.

**Lo que NO se sabe.** El equipo dijo que anda bien; no quedó anotado de qué lateral, a qué
velocidad, ni cuántas veces. Esto salió de leer las bitácoras y **no se probó**:

- ⚠️ **Falso lateral.** Corrido ~10 cm para atrás y torcido, un solo sensor puede quedar sobre la
  línea del área. Es la falla que hizo sacar el límite el 18/08, y ahora cuesta más: desde el
  medio, 2228 ms es medio ancho de cancha.
- ⚠️ **Puede llegar tarde.** El 06/10, yendo de costado a 160, el robot chocaba la madera del
  borde; el seguimiento llega a 150 y las líneas laterales solo se vieron a 100 y 110.
- ⚠️ **Al perder la pelota cerca del lateral** el programa suelta los motores (`parar()`) y deja
  de mirar los sensores: puede seguir de largo por inercia.
- ⚠️ **Ida y vuelta:** si la pelota queda del lado del lateral, vuelve al medio, la ve, y sale
  otra vez.

---

## 4. El despeje sale 35 cm

`msAdelante` 533 (~50 cm) → **383** (~35 cm), con la cuenta del 04/08: `(35 + 3,3) × 10`. El
disparo sigue en 30 cm (`umbralCm`), así que quedan **5 cm de empuje** en vez de 20.

🚨 **Sin probar.** El 15/09, con 0 cm de sobra (30 y 30), *"ni llegaba a tocar la pelota"*.
Para volver: 533.

---

## 5. La ESTABLE 0.1013

Copia congelada en [`respaldos/estable1013conlimiteslaterales/`](../respaldos/estable1013conlimiteslaterales/)
(el nombre lo eligió el equipo). Es la versión 5 de la tabla: patrulla a los 10 s, límite
lateral, despeje de 35 cm y **predicción apagada**. Su cartel dice
`RESPALDO ESTABLE 0.1013 con limites laterales (copia congelada)`.

**Qué tan estable es:** el equipo dijo *"la versión actual está buena"* **antes** de bajar el
despeje a 35 cm. Ese último cambio se congeló sin que quedara anotado si se probó. No jugó
ningún partido. La 0.0929 sigue en `respaldos/estable-0.0929/`.

---

## 6. Errores del día (míos)

1. **Cargué el límite lateral sin revisarlo**, por el apuro de la clase. Compilaba sin avisos y
   el equipo lo vio andar, pero nadie leyó ese código buscándole fallas. Es lo mismo que pasó
   con la patrulla el 12/10.
2. **Dos comentarios míos del límite estaban mal** y se cargaron así: decían que el límite del
   11/08 miraba cualquiera de los dos sensores (miraba el del lado hacia el que iba) y que el
   tiempo desde el lateral derecho no estaba medido (la cuenta del 06/10 da 2331 ms). Los
   corregí después, cuando llegó la lectura de las bitácoras.
3. **Se cargaron cuatro cambios de comportamiento en una clase** (10 s, límite lateral, despeje,
   predicción). La regla del equipo es uno por vez. Si algo anda raro, los cuatro tienen su
   interruptor o su número de vuelta en el encabezado del programa.

---

## 7. Regla de trabajo nueva (del profe Franco)

Cuando Diego o Laureano piden un cambio, **Claude primero les dice qué entendió**, les muestra qué
hace hoy el programa en ese punto, y les pregunta lo que no cierre. Recién después cambia el
código. Salió de un pedido de hoy (*"el despeje cambialo a 30cm de la distancia en la que detecta
la pelota"*) que tenía tres lecturas, una ya probada y fallida, y además el programa ya
disparaba a 30 cm.

---

## 8. Qué queda pendiente

### 🔴 Lo primero, en la cancha

- 🔴 **El despeje de 35 cm:** ¿llega a tocar la pelota y la saca? Pelota quieta a 30 cm, de frente.
- 🔴 **La predicción prendida:** ¿oscila con la pelota lejos? Pelota quieta a 1 m, 15–20 cm al
  costado, contar cuántas veces cambia de lado. Es la pregunta que quedó sin contestar el 12/10.
- 🔴 **El límite lateral desde el lateral DERECHO:** ¿termina en el medio con 2228 ms?
- 🔴 **El falso lateral:** con el robot en el medio, moverle la pelota de lado a lado. No tiene que
  salir disparado para un costado sin haber pisado ningún lateral.

### ⬜ Pedidos del profe para las próximas clases (13/10)

- ⬜ **Preparación de cancha sin internet:** un `.bat` que lleve a los chicos paso a paso por cada
  parte del robot con las pruebas de `pruebas/` (despeje, laterales, sensores, cámara…) y les dé
  los valores para que ellos los escriban en el programa de partido.
- ⬜ **Todo lo ajustable arriba:** en el programa de partido, un solo bloque al principio con todas
  las variables y los interruptores. Es mover declaraciones sin cambiar ningún valor.
- ⬜ **Una herramienta simple para ajustar la corrección de rumbo en el avance**, de a poco, para
  que no oscile ni zigzaguee.

### ⬜ Arrastres

- ⬜ **Revisar el código del límite lateral** antes de la Roboliga (§6, error 1).
- ⬜ **Medir con los motores nuevos:** sigue sin hacerse nada de la lista del 12/10 (`medir-ancho`,
  el tablero con los sensores de línea, `probar-ruedas`, la regla).
- ⬜ **El script de la cámara** (`futbol-roboliga2026/robots-2025/vision-openmv/`) tiene un cambio
  en esta PC que **no se subió**: el umbral naranja pasó de `(21, 67, 18, 79, -32, 127)` a
  `(…, 78, …)`, guardado el 13/10 a las 19:59. No es de esta sesión de código y ese script lo
  comparte el delantero: falta saber si es lo que está cargado en la cámara.
