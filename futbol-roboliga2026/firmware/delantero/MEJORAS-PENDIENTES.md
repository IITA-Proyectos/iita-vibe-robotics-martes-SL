# Delantero — mejoras pendientes

**Nada de esto está implementado.** Es la lista de lo que sigue, armada al cerrar la clase del
**2026-08-04**, donde la órbita quedó andando (*"orbita bastante bien"*, validado en el piso).

Regla que no se negocia: **un cambio por vez**. Si se tocan tres cosas y empeora, no se sabe
cuál fue.

---

## El orden importa: unas destraban a las otras

```
1. CALIBRAR LA CÁMARA
       │
       ├──► destraba  3. girar por el camino más corto   (necesita ver el arco)
       │
       └──► si aún así no alcanza  ──►  2. patear al "arco 0" del giroscopio
                                             (es el plan B de no ver el arco)

4. LÍNEA BLANCA → dejar de orbitar y patear    (es independiente: seguridad)
```

---

## 1. Calibrar la cámara — 🔴 lo primero

**Qué pasa hoy:** no detecta el arco de lejos, y la pelota da saltos imposibles. El 2026-08-04
el monitor mostró `Xp` yendo de **139 a 41 cm** entre dos cambios de estado — el robot no se
movió 98 cm. Son dos manchas distintas en extremos opuestos de la imagen.

**Por qué está primero:** mientras la cámara mienta, cualquier ajuste de control se hace a
ciegas. Y las mejoras 2 y 3 dependen de ver el arco.

**Cómo se hace:** está todo escrito, con protocolo de 6 pasos y herramienta lista.
→ [`../../vision/README.md`](../../vision/README.md) y
[`../../vision/calibrar-umbrales.py`](../../vision/calibrar-umbrales.py)

**Lo que hay que arreglar antes de mover un solo umbral:** la cámara tiene ganancia y balance
de blancos **en automático**. Se re-acomoda sola según toda la escena, así que cualquier umbral
que se calibre sin congelar eso primero se despega solo. Es medir con una cinta que se estira.

**Tiempo:** ~10 minutos con la cámara, el IDE y la pelota.

---

## 2. Si da una vuelta entera y no ve el arco → patear hacia el "0" del giroscopio

*(pedido de Gustavo, 2026-08-04)*

**La idea:** el robot guarda hacia dónde estaba mirando cuando arrancó. Si orbita una vuelta
completa y no encontró el arco, en vez de rendirse, patea hacia esa dirección inicial. Es un
plan B: mejor patear a un lado razonable que no patear.

**Lo bueno: ya hay código de referencia.** El delantero 2025 hace exactamente eso
(`robots-2025/delantero/delantero.ino`):

```cpp
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);   // :76
bno.getEvent(&event);
initialYaw = event.orientation.x;                  // :278  guarda el 0 al arrancar
...
error = currentYaw - initialYaw;                   // :362  cuánto se desvió
correccion = error * kp;                           //  kp = 0.3
```

**⚠️ Lo que falta confirmar antes de escribir una línea:** el firmware de 2026 **no lee el BNO**
— cero referencias. Que el código de 2025 lo usara **no prueba** que el sensor esté hoy
conectado ni que funcione. Primer paso obligatorio: el test de la sección 5.

**Decisión de diseño a discutir:** al arrancar el robot se lo apoya mirando a algún lado. Si ese
"0" no es la dirección del arco rival, el plan B patea para cualquier parte. Hay que definir el
ritual de arranque: *"al robot se lo apoya mirando al arco contrario"*.

---

## 3. Girar por el camino más corto hacia el arco azul

*(pedido de Gustavo, 2026-08-04)*

**Qué pasa hoy:** `ORBITA_INVERTIDA` es una constante fija. El robot siempre orbita para el
mismo lado, aunque el arco esté a 20° del otro lado. Puede dar casi una vuelta entera para
llegar a algo que tenía al lado.

**Qué habría que hacer:** al entrar en `ORBITANDO`, mirar de qué lado quedó el arco y elegir el
sentido. Con el giroscopio sale más limpio todavía: si se conoce el rumbo del arco, la
diferencia de ángulos más corta dice el sentido directamente.

**Depende de la mejora 1:** para elegir el lado más corto hay que **ver** el arco. Si no se lo
ve de lejos, no hay nada que comparar.

---

## 4. Línea blanca → dejar de orbitar y patear al arco

*(pedido de Gustavo, 2026-08-04 — **"la clase que viene te explicamos mejor"**)*

**La idea, como la dijo Gustavo:** si los sensores de luz detectan la línea blanca, hay que
**dejar de orbitar y patear la pelota en dirección al arco**, para no salirse de la cancha.

**El hardware existe en el código viejo:** `readLine(1/2/3)` = izquierdo / centro / derecho, con
umbrales propios del delantero **650 / 650 / 750** (`robots-2025/delantero/delantero.ino:26-28`,
`:369-371`).

**⚠️ Mismo aviso que arriba:** el firmware de 2026 no los lee. Hay que confirmar que funcionan.

**Queda pendiente de charlar:** qué pasa si al detectar la línea el robot **no** está alineado
con el arco. ¿Patea igual? ¿Retrocede primero? Salir de la cancha es peor que un mal pase, así
que probablemente convenga patear igual — pero lo define el equipo, no el código.

---

## 5. El test que destraba las mejoras 2 y 4 — hacerlo primero

**Un programa de diagnóstico que no mueve nada:** lee el BNO055 y los 3 sensores de línea y los
imprime por el monitor serie. Nada más.

Contesta de una sola vez:

- ¿El giroscopio responde? ¿`bno.begin()` da bien?
- ¿El yaw cambia cuando girás el robot con la mano, y vuelve al mismo número?
- ¿Los 3 sensores de línea dan valores distintos sobre el verde y sobre la línea blanca?
- ¿Los umbrales 650 / 650 / 750 siguen siendo los correctos con la luz de hoy?

**Sin esto, las mejoras 2 y 4 se escriben a ciegas.** Es media hora y de ahí salen números para
la bitácora.

---

## Lo que encontró Claude leyendo el código (no lo pidió nadie)

### 6. `VEL_AVANCE = 55` está por debajo del piso de arranque (~70)

Y a `AVANZANDO` se entra desde `CENTRANDO`, que usa `rotarPulsado()` y por lo tanto deja el
robot **parado** entre pulso y pulso. O sea: arranca a avanzar desde quieto con 55, que no
debería alcanzarle. **Es exactamente el mismo mecanismo que arreglamos en la órbita**, y el
siguiente lugar natural para el impulso de arranque.

### 7. Medir si la órbita cae justo en la zona ciega de la cámara — 2 minutos

La fila más baja de la imagen da **17,4 cm**: más cerca, la pelota se sale por abajo y llega
`Xp = 0`. La órbita gira alrededor de un punto a **R = 2·L = 17,5 cm** justo adelante. Los dos
números se tocan — pero **no sabemos desde dónde mide la homografía**, así que puede ser
casualidad.

**Se contesta así:** pelota a **30 cm exactos** del centro del robot, leer `Xp`. Si dice ~30 el
origen es el centro y el problema es real; si dice ~20, no se comparaban. Repetir a 50 cm.

### 8. Interruptor para elegir el arco

Hoy el arco azul está hardcodeado (`leerCamara()` tira los bytes del amarillo). En la Roboliga
**se cambia de lado entre tiempos**, así que hay que poder cambiarlo sin reescribir el parser.
Propuesta: una constante `ATACAR_ARCO_AMARILLO`, ~10 líneas.

> Ojo con "el amarillo se detecta mejor": el script pide **600 px** para el amarillo y **300**
> para el azul, o sea ~30 % menos de alcance. El color del amarillo es más limpio, pero el
> filtro de tamaño juega en contra. Hay que medirlo, no opinarlo.

### 9. Cosas chicas

- **Cronometrar una vuelta de la órbita** y poner `MS_ORBITA_MAX` en el doble. Hoy está en 20 s
  por estimación, no por medición.
- **Dibujar el círculo de la órbita con tiza** y confirmar los ~35 cm de diámetro que predice
  `R = 2·L`. Nunca se midió.
- **Corregir el README de `robots-2025/vision-openmv/`**: dice que el protocolo no se puede
  resincronizar, y eso es falso — ningún byte de datos puede valer 201/202/203, así que se
  realinea solo.
