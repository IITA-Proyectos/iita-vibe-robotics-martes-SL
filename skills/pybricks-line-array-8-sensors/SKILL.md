---
name: pybricks-line-array-8-sensors
description: Seguidores de línea profesionales con arrays multi-sensor (8 sensores de luz) tipo Mindsensors LineLeader-V2, NXTLightSensorArray, Pololu QTR-8RC o equivalente. Cubre cálculo de posición ponderada (weighted average), calibración por canal, modos binario/análogico, detección de intersecciones desde el patrón del array, recovery de línea perdida (last known position), velocidad adaptativa basada en posición, y diferencias frente al seguidor de 1-2 sensores. Usar SIEMPRE que se trabaje con un array de luz, 8 sensores en línea, LineLeader, QTR-8, lightsensorarray, o se mencione 'weighted position', 'posición ponderada', 'centroid de la línea', 'sensor multiplexado para seguidor', 'detección de T-junction con array', 'seguidor de línea con 8 sensores'. NO usar para seguidor con 1-2 ColorSensor (eso es pybricks-line-following).
---

# Seguidores de línea con array de 8 sensores

Un **array de 8 sensores de luz** transforma el seguidor de línea de un controlador reactivo a uno **predictivo**: en cualquier instante sabés en qué punto exacto del ancho del array está la línea, no solo "estoy más a la izquierda o más a la derecha". Esto permite:

- Velocidades 2-3× superiores que con 1 sensor manteniendo estabilidad.
- Detección de intersecciones, T-junctions, cruces y bifurcaciones desde el patrón del array.
- Recovery robusto cuando se pierde la línea (sabés desde qué lado se fue).
- Curvas cerradas sin overshoot porque el sensor ve la línea antes de pasarla.

Esta skill cubre el modelo de procesamiento, el cálculo de posición ponderada, los patrones de detección de pista y los algoritmos avanzados que separan un seguidor "ok" de uno de competición.

## Cuándo usar / cuándo NO usar

- ✅ Usar para: Mindsensors LineLeader-V2/V1, NXTLightSensorArray (LSA), Pololu QTR-8RC vía analógico, cualquier array de N≥5 sensores donde se obtenga el valor calibrado de cada canal.
- ❌ No usar para: 1 ColorSensor (esquema edge-following — usar `pybricks-line-following`), 2 ColorSensors (esquema diferencial — usar `pybricks-line-following`).

Para integrar específicamente el LineLeader vía I²C en EV3, ver también `mindsensors-ev3-sensors`. Esta skill asume que ya tenés acceso a los 8 valores calibrados.

## Modelo mental: el array como "regla de medir línea"

Imaginá los 8 sensores numerados de 0 a 7, dispuestos en línea perpendicular al avance del robot:

```
  Robot avanza →
  ┌─────────────────────────────────────┐
  │  [0][1][2][3][4][5][6][7]           │ ← array, 30 mm de ancho total
  └─────────────────────────────────────┘
               ↓ línea negra (15 mm ancho)
  ════════════════════════════════
```

Cada sensor devuelve un valor calibrado:
- **0** = sobre negro (sensor encima de la línea)
- **100** = sobre blanco
- (los sensores parcialmente sobre la línea dan valores intermedios — esto es la clave)

La **posición ponderada** es el centroide del peso de la línea visto por el array. Con 8 sensores se obtiene una resolución efectiva mucho mayor que 8 — típicamente **0-70 con 1 unidad ≈ 0.4 mm de precisión**.

## Cálculo de posición ponderada (weighted average)

### Fórmula

```
posicion = Σ(i × peso_i) / Σ(peso_i)
```

Donde `peso_i` es la "blackness" del sensor i: `peso_i = 100 - valor_calibrado_i` (porque queremos que pesen más los sensores que ven negro).

Para 8 sensores, posición ∈ [0, 7] o escalada × 10 ∈ [0, 70]. El centro del array está en 3.5 (o 35 si escalado).

### Implementación en Pybricks

```python
def position_weighted(calibrated_8):
    """
    Calcula la posición ponderada de la línea en el array.
    calibrated_8: lista de 8 valores calibrados 0-100 (0=negro, 100=blanco).
    Devuelve: float 0.0-7.0 representando dónde está el centro de la línea.
              Si no detecta nada, devuelve None.
    """
    weights = [100 - v for v in calibrated_8]  # invertir: ahora 100=línea, 0=blanco
    total_weight = sum(weights)
    
    if total_weight < 80:  # umbral: no hay línea visible
        return None
    
    weighted_sum = sum(i * w for i, w in enumerate(weights))
    return weighted_sum / total_weight
```

### Posición × 10 (entera, evita float)

En microcontroladores la aritmética entera es más rápida y predecible:

```python
def position_x10(calibrated_8):
    """Devuelve posición × 10 (entero) en rango [0, 70]. None si no hay línea."""
    total = 0
    weighted = 0
    for i, v in enumerate(calibrated_8):
        w = 100 - v
        total += w
        weighted += i * 10 * w
    if total < 80:
        return None
    return weighted // total
```

Centro del array = 35 (con escala ×10).

## Loop básico de seguidor PID con 8-array

```python
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait
# Suponiendo wrapper LineLeader del skill mindsensors-ev3-sensors
from line_leader import LineLeader

ev3 = EV3Brick()
left = Motor(Port.B, Direction.CLOCKWISE)
right = Motor(Port.C, Direction.CLOCKWISE)
drive = DriveBase(left, right, wheel_diameter=56, axle_track=114)
ll = LineLeader(Port.S3)

# Calibración previa: ll.calibrate_white() / ll.calibrate_black() con setup manual

KP = 1.5
KI = 0.0
KD = 12.0
BASE_SPEED = 300   # mm/s
CENTER = 35        # posición × 10 del centro del array

def follow_line_array(distance_mm):
    drive.reset()
    integral = 0
    last_error = 0
    last_position = CENTER  # para recovery
    
    while drive.distance() < distance_mm:
        cal = ll.calibrated()
        pos = position_x10(cal)
        
        if pos is None:
            # Línea perdida — usar último side conocido
            pos = 0 if last_position < CENTER else 70
        
        error = pos - CENTER
        integral += error
        if integral > 1000: integral = 1000
        elif integral < -1000: integral = -1000
        derivative = error - last_error
        last_error = error
        last_position = pos
        
        turn_rate = KP * error + KI * integral + KD * derivative
        drive.drive(BASE_SPEED, turn_rate)
        wait(10)
    drive.stop()
```

## Calibración por canal — paso CRÍTICO

Cada uno de los 8 sensores del array tiene **fototransistores ligeramente distintos**, óptica con polvo, y leds con intensidad no uniforme. Si calibrás "globalmente" y promediás, la línea se "ve corrida" porque los sensores de los extremos suelen leer distinto que los del centro.

### Procedimiento de calibración profesional

```python
def calibrate_per_channel(ll):
    """
    Calibración manual por canal. Guarda white_per_channel[8] y black_per_channel[8].
    Devolver tablas para normalización posterior.
    """
    ev3.screen.print("TODO BLANCO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    # Promediar 20 muestras
    white_acc = [0]*8
    for _ in range(20):
        raw = ll.raw()
        for i in range(8): white_acc[i] += raw[i]
        wait(20)
    white = [w // 20 for w in white_acc]
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    
    ev3.screen.print("TODO NEGRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    black_acc = [0]*8
    for _ in range(20):
        raw = ll.raw()
        for i in range(8): black_acc[i] += raw[i]
        wait(20)
    black = [b // 20 for b in black_acc]
    
    ev3.screen.print("Listo")
    print("WHITE:", white)
    print("BLACK:", black)
    return white, black


def normalize_array(raw_8, white, black):
    """Normaliza cada canal a 0-100 usando su propia calibración."""
    out = [0]*8
    for i in range(8):
        span = white[i] - black[i]
        if span < 5:
            out[i] = 50  # canal muerto — neutral
        else:
            n = (raw_8[i] - black[i]) * 100 // span
            out[i] = max(0, min(100, n))
    return out
```

Después usás `normalize_array(ll.raw(), WHITE, BLACK)` en vez de `ll.calibrated()` cuando querés más control que el firmware interno del sensor.

Para Mindsensors LineLeader, el firmware ya hace calibración por canal cuando se llama `calibrate_white()` y `calibrate_black()`. **Verificar con 4-5 corridas que los valores son consistentes** — si no, hacé tu propia tabla.

## Velocidad adaptativa basada en error

Cuanto más lejos del centro está la línea, más cerrada es la curva, y más conviene bajar velocidad para no perderla:

```python
MAX_SPEED = 500   # mm/s, rectas
MIN_SPEED = 150   # mm/s, curvas cerradas

def adaptive_speed(error):
    """Speed proporcional a (1 - |error|/35)."""
    factor = 1.0 - abs(error) / 35.0
    if factor < 0: factor = 0
    return int(MIN_SPEED + (MAX_SPEED - MIN_SPEED) * factor)
```

Una mejora todavía mayor: **bajar velocidad también por el derivative** (cuán rápido cambia el error). Curva entrante = derivative grande = futuro error grande.

```python
def adaptive_speed_v2(error, derivative):
    e_factor = 1.0 - abs(error) / 35.0
    d_factor = 1.0 - abs(derivative) / 20.0
    factor = max(0, min(e_factor, d_factor))
    return int(MIN_SPEED + (MAX_SPEED - MIN_SPEED) * factor)
```

## Detección de patrones — el superpoder del array

El bitmask de los 8 sensores te dice exactamente qué forma tiene la pista bajo el robot. Estos patrones son la base para reconocer intersecciones, gaps, esquinas pronunciadas, etc.

```python
def pattern_string(calibrated_8, threshold=30):
    """'########' = todo negro, '...##...' = línea centrada."""
    return ''.join('#' if v < threshold else '.' for v in calibrated_8)


def count_on_line(calibrated_8, threshold=30):
    """Cuántos sensores ven negro."""
    return sum(1 for v in calibrated_8 if v < threshold)
```

### Patrones canónicos y qué significan

| Patrón | Significado | Acción típica |
|---|---|---|
| `...##...` o `....##..` | Línea centrada o casi | Seguir normal |
| `##......` | Línea muy a la izquierda | Curva izquierda cerrada |
| `......##` | Línea muy a la derecha | Curva derecha cerrada |
| `........` | TODO BLANCO — línea perdida | Recovery: volver al último lado conocido |
| `########` | TODO NEGRO — intersección perpendicular o línea ancha | Detectar T/cruz, ejecutar acción de misión |
| `####....` | Bifurcación a la izquierda | Decisión: seguir derecha o izquierda |
| `....####` | Bifurcación a la derecha | Idem |
| `##....##` | Doble línea (carriles paralelos) | Caso raro — generalmente error de pista |
| `#.######` o ruidoso | Sucio, sombra, residuo | Filtrar |

### Detector de intersecciones

```python
def detect_event(cal_8):
    """Devuelve string del evento detectado, o None."""
    n = count_on_line(cal_8)
    
    if n == 0:
        return 'LOST'
    if n >= 7:
        return 'INTERSECTION'  # casi todo negro = cruce o T
    if n >= 5:
        left = sum(1 for v in cal_8[:4] if v < 30)
        right = sum(1 for v in cal_8[4:] if v < 30)
        if left == 4 and right < 3:
            return 'BRANCH_LEFT'
        if right == 4 and left < 3:
            return 'BRANCH_RIGHT'
    return None
```

## Recovery de línea perdida

Cuando `count_on_line == 0`, la línea no está bajo el array. Hay dos estrategias:

### Estrategia A — "sigo en la dirección del último error" (rápida)

```python
if pos is None:
    if last_position < CENTER:
        turn_rate = -100   # girar a la izquierda fuerte
    else:
        turn_rate = 100    # girar a la derecha fuerte
    drive.drive(80, turn_rate)
    wait(20)
    continue
```

Funciona bien cuando la pérdida es por overshoot leve. Falla en gaps grandes (la línea reaparece después de un hueco).

### Estrategia B — "search sweep" (más lenta pero confiable)

```python
def search_line():
    """Sweep buscando la línea: derecha 30°, izquierda 60°, derecha 30°."""
    drive.stop()
    for angle in (45, -90, 45):
        drive.turn(angle / 3)  # girando lento, chequeo cada 1/3 del barrido
        for _ in range(int(abs(angle/3) * 10)):
            cal = ll.calibrated()
            if count_on_line(cal) >= 2:
                return True
            wait(20)
    return False
```

### Estrategia C — "memoria de pose"

Si tenés gyro+encoders, podés recordar la pose donde se perdió la línea y avanzar en línea recta una distancia razonable, asumiendo que la pista continúa más adelante. Útil en RCJ Rescue Line con gaps marcados.

## Anti-bias por sombra del robot

El robot proyecta sombra sobre el array, especialmente bajo luz lateral. Síntoma: los sensores 0-1 (o 6-7) leen sistemáticamente más oscuro **aunque no haya línea**, y la posición ponderada se sesga.

**Solución**: agregar una constante de corrección por canal después de la calibración, basada en un test "todo blanco" con la iluminación real de la competencia.

```python
# Después de calibrar bajo la luz de la pista:
SHADOW_OFFSET = [0, 0, 0, 0, 0, 0, 0, 0]  # rellenar con offsets observados

def normalize_with_shadow(raw_8, white, black):
    out = []
    for i in range(8):
        adjusted = raw_8[i] - SHADOW_OFFSET[i]
        span = white[i] - black[i]
        n = (adjusted - black[i]) * 100 // span if span > 5 else 50
        out.append(max(0, min(100, n)))
    return out
```

## Modo binario vs analógico — cuándo cada uno

**Modo analógico** (lo que vimos arriba): usa el valor 0-100 de cada sensor, posición ponderada continua. Resolución máxima, ideal para PID puro y velocidades altas.

**Modo binario** (umbralizado): cada sensor es 0 o 1. Posición se calcula con los índices de los sensores en 1. Más simple, menos sensible a ruido en las transiciones, pero pierde resolución.

```python
def position_binary(cal_8, threshold=30):
    indices = [i for i, v in enumerate(cal_8) if v < threshold]
    if not indices:
        return None
    return sum(indices) * 10 // len(indices)  # × 10
```

Recomendación: **arrancar binario para tener algo funcionando rápido, después migrar a analógico para subir velocidad de competición**.

## Comparación de rendimiento típica

Con un robot EV3, 56 mm de rueda, en pista WRO estándar:

| Configuración | Velocidad máxima estable | Tiempo en circuito 5 m | Comentario |
|---|---|---|---|
| 1 ColorSensor edge | ~250 mm/s | ~22 s | Sirve para arrancar |
| 2 ColorSensor diferencial | ~350 mm/s | ~16 s | Mejor estabilidad |
| LineLeader binario | ~400 mm/s | ~13 s | Fácil setup |
| LineLeader analógico + PID | ~600 mm/s | ~9 s | Profesional |
| LineLeader + PID + feedforward por curva | ~750 mm/s | ~7 s | Top-tier |

## Tuning empírico del PID con array

El array de 8 hace el tuning **mucho más fácil** porque el rango de error es conocido y simétrico (-35 a +35), y tenés telemetría rica.

Procedimiento:
1. **Setear KI=0, KD=0**. Empezar con KP=1.0, BASE_SPEED=200.
2. **Recta**: subir KP hasta que oscile, después bajar 30%.
3. **Curva cerrada de la pista**: si se sale, subir KP otro poquito; si overshootea pero vuelve, agregar KD.
4. **Empezar con KD = 5 × KP**. Subir hasta que las curvas sean limpias sin sobrecorregir.
5. **Subir BASE_SPEED gradualmente** hasta que se pierda la línea en alguna curva. Bajar 15%.
6. **Loggear** con `DataLog` y mirar las curvas error/derivative/output — un ojo entrenado ve el tuning en el gráfico.

Para tuning sistemático (Ziegler-Nichols, gain scheduling), ver `advanced-pid-optimization`.

## Patrón completo de uso en competencia (template)

```python
#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.parameters import Port, Direction, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch
from line_leader import LineLeader

ev3 = EV3Brick()
left = Motor(Port.B, Direction.CLOCKWISE)
right = Motor(Port.C, Direction.CLOCKWISE)
drive = DriveBase(left, right, 56, 114)
ll = LineLeader(Port.S3)
watch = StopWatch()

# Constantes calibradas para esta pista
KP, KI, KD = 1.6, 0.0, 14.0
BASE_SPEED, MIN_SPEED = 450, 180
CENTER = 35

def pos_x10(cal):
    total = 0; w_sum = 0
    for i, v in enumerate(cal):
        w = 100 - v
        total += w
        w_sum += i * 10 * w
    return None if total < 80 else w_sum // total

def adaptive_speed(error):
    f = 1.0 - abs(error) / 35.0
    return int(MIN_SPEED + (BASE_SPEED - MIN_SPEED) * max(0, f))

def count_on_line(cal, th=30):
    return sum(1 for v in cal if v < th)

def follow_until_intersection():
    last_err = 0
    last_pos = CENTER
    while True:
        cal = ll.calibrated()
        if count_on_line(cal) >= 7:    # intersección
            drive.stop()
            return 'INTERSECTION'
        pos = pos_x10(cal)
        if pos is None:
            pos = 0 if last_pos < CENTER else 70
        error = pos - CENTER
        deriv = error - last_err
        last_err = error
        last_pos = pos
        turn = KP * error + KD * deriv
        drive.drive(adaptive_speed(error), turn)
        wait(10)

# Espera a empezar
ev3.screen.print("Listo")
while Button.CENTER not in ev3.buttons.pressed(): wait(20)

ev3.speaker.beep()
follow_until_intersection()
ev3.speaker.beep(800, 300)
```

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| Robot oscila a alta velocidad | KD muy alto o ruido en señal | Suavizar con low-pass por canal o bajar KD |
| Robot se sesga a un lado en rectas | Sombra propia genera bias en canales laterales | SHADOW_OFFSET por canal |
| Pierde línea en curvas cerradas | KP bajo o velocidad demasiado alta | Adaptive speed más agresivo, subir KP |
| `position()` salta entre extremos | Línea fuera del array — `pos is None` se ignora | Manejar `None` con recovery explícito |
| Detecta intersección dentro de una curva | Curva cerrada da `count_on_line` alto temporalmente | Requerir N>=7 durante 3 ciclos consecutivos |
| Funciona el primer día, falla al segundo | Pista distinta, luz distinta, batería distinta | Re-calibrar **siempre** antes de cada sesión |
| Mismo programa, distinto LineLeader, no anda | Cada array tiene calibración propia | Guardar WHITE/BLACK como constantes por sensor, no por modelo |
| Detector de eventos disparado por ruido | Threshold demasiado alto, sin debounce | Bajar threshold a 25, requerir N ciclos consecutivos |
| Loop rate cae a <30 Hz | Logging excesivo o I²C saturado | Quitar prints, leer todos los registros en una sola transacción I²C |

## Recursos

- LineLeader-V2 manual: http://www.mindsensors.com/ev3-and-nxt/47-line-sensor-array
- Pololu QTR-8RC user's guide (incluye fórmula de posición ponderada): https://www.pololu.com/docs/0J19
- Paper "Line Following Robot Using PID" (Khan et al., 2017) — análisis comparativo de N sensores.
- Charla "World Class Line Following" (RoboCup Junior archive).
