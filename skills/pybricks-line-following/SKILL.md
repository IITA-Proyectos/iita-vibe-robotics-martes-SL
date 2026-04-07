---
name: pybricks-line-following
description: Seguidores de línea de precisión con LEGO Spike Prime y Pybricks usando ColorSensor.reflection() y controladores P/PI/PID profesionales. Usar SIEMPRE que se trabaje con line following, seguidor de línea, line follower, calibración de sensor de color para línea, controladores P/PI/PID, intersecciones, T-junctions, gap detection, two-sensor line following, velocidad adaptativa en curvas, o se mencione 'reflection', 'setpoint', 'Kp', 'Ki', 'Kd', 'PID tuning', 'línea negra sobre blanco', 'cross detection'. También usar para RCJ Rescue Line, FLL line missions, WRO line tracking sub-tasks. NO usar para precision driving puro (eso va en pybricks-precision-driving).
---

# Seguidores de línea profesionales con Pybricks

El seguidor de línea es el ejercicio canónico de robótica educativa y aparece en WRO RoboMission, RCJ Rescue Line, FLL y casi todos los certámenes. Esta skill cubre desde el seguidor más simple (P controller con un sensor) hasta técnicas avanzadas (PID con dos sensores, normalización, intersecciones, gap recovery, velocidad adaptativa).

## Hardware setup

Uno o dos `ColorSensor` apuntando al piso, montados al frente del robot:

- **Altura sobre el piso**: 5-10 mm. Más bajo = mejor contraste pero rasca. Más alto = pierde resolución.
- **Distancia desde el axle**: 80-120 mm adelante. Más adelante = anticipación temprana pero más overshoot. Más atrás = más estable pero reacción tardía.
- **Iluminación**: el ColorSensor tiene LED propio, funciona con cualquier luz ambiente, **PERO** pierde precisión bajo luz solar directa o reflejos. Cubrir con falda negra de LEGO.

## El método clave: `reflection()`

```python
sensor = ColorSensor(Port.E)
value = sensor.reflection()  # 0-100
```

- `reflection() = 100` → blanco perfecto
- `reflection() = 0` → negro perfecto
- En un mat WRO típico: blanco real ~85-95, negro real ~5-15, gris medio ~40-50

**No usar `color()` para line following** — clasificación discreta que pierde la información analógica que el PID necesita.

## Calibración — paso obligatorio

Los valores varían entre sensores, entre pisos y entre niveles de batería. Calibrar SIEMPRE antes de competir.

```python
def calibrate():
    hub.display.text('W')  # poner sobre blanco
    while Button.CENTER not in hub.buttons.pressed(): wait(10)
    while Button.CENTER in hub.buttons.pressed(): wait(10)
    white = sensor.reflection()
    
    hub.display.text('B')  # poner sobre negro
    while Button.CENTER not in hub.buttons.pressed(): wait(10)
    while Button.CENTER in hub.buttons.pressed(): wait(10)
    black = sensor.reflection()
    
    setpoint = (white + black) // 2
    hub.display.text(str(white) + ' ' + str(black))
    return white, black, setpoint
```

Para competición, después de calibrar guardar como constantes:

```python
WHITE = 92
BLACK = 8
SETPOINT = (WHITE + BLACK) // 2  # 50
```

## Seguidor con un sensor — método edge-following

Con un solo sensor, **NO se sigue el centro de la línea sino su borde**. El sensor se posiciona sobre el borde derecho (o izquierdo), y el controlador mantiene `reflection()` en el setpoint.

Cuando el sensor se sale al blanco → robot se va al blanco → girar hacia el negro.

```python
KP = 0.5
BASE_SPEED = 150  # mm/s

def follow_line_p(distance_mm):
    """P controller, sigue borde derecho de línea negra."""
    drive.reset()
    while drive.distance() < distance_mm:
        error = sensor.reflection() - SETPOINT
        turn_rate = KP * error
        drive.drive(BASE_SPEED, turn_rate)
        wait(10)
    drive.stop()
```

**Sintonización empírica de Kp**:

1. Empezar con `KP = 0.5`, `BASE_SPEED = 100`.
2. Recta: si oscila visiblemente, **bajar Kp**. Si serpentea suave o se sale en curvas, **subir Kp**.
3. Curva cerrada: si se sale, subir Kp más.
4. Subir `BASE_SPEED` gradualmente hasta perder la línea, después bajar 20-30%.

A mayor velocidad, mayor Kp. No es lineal — pasar de 100 mm/s a 200 mm/s puede requerir ir de Kp=0.5 a Kp=1.0.

## Controlador PID completo

P funciona bien para velocidades bajas. Para velocidades altas o curvas frecuentes, agregar **D (derivativo)** suaviza la respuesta y permite Kp más alto sin oscilación. **I (integral)** raramente es necesario en line following — dejarlo en 0 por default.

```python
KP = 0.8
KI = 0.0
KD = 5.0
BASE_SPEED = 250

def follow_line_pid(distance_mm):
    drive.reset()
    integral = 0
    last_error = 0
    while drive.distance() < distance_mm:
        error = sensor.reflection() - SETPOINT
        integral += error
        derivative = error - last_error
        last_error = error
        # Anti-windup
        if integral > 100: integral = 100
        elif integral < -100: integral = -100
        turn_rate = KP * error + KI * integral + KD * derivative
        drive.drive(BASE_SPEED, turn_rate)
        wait(10)
    drive.stop()
```

**Sintonizar Kd**: empezar con `KD = 5 * KP`. Subir si oscila pese a Kp bajo. Bajar si responde tarde a curvas.

## Seguidor con dos sensores — método del centro

Con dos sensores, uno a cada lado de la línea, se sigue el centro real. Más estable, permite velocidades altas, pero requiere más tuning porque hay 4 valores calibrados.

```python
color_left = ColorSensor(Port.E)
color_right = ColorSensor(Port.F)

WHITE_L = 92; BLACK_L = 8
WHITE_R = 90; BLACK_R = 10

def normalize(value, white, black):
    """Normaliza reflection a 0-100 considerando calibración del sensor."""
    return max(0, min(100, (value - black) * 100 // (white - black)))

KP = 1.5
BASE_SPEED = 350

def follow_line_two_sensor(distance_mm):
    drive.reset()
    while drive.distance() < distance_mm:
        l = normalize(color_left.reflection(), WHITE_L, BLACK_L)
        r = normalize(color_right.reflection(), WHITE_R, BLACK_R)
        # l > r → robot se va a la izquierda → girar a la izquierda
        error = l - r
        turn_rate = KP * error
        drive.drive(BASE_SPEED, turn_rate)
        wait(10)
    drive.stop()
```

**La normalización es clave** cuando los dos sensores tienen calibraciones distintas, porque si uno lee blanco como 95 y el otro como 88, el error nunca llega a cero y el robot zigzaguea.

## Velocidad adaptativa — más rápido en rectas

Bajar la velocidad cuando |error| es grande (curva), subirla cuando es chico (recta).

```python
def follow_line_adaptive(distance_mm):
    drive.reset()
    last_error = 0
    MAX_SPEED = 400
    MIN_SPEED = 120
    while drive.distance() < distance_mm:
        error = sensor.reflection() - SETPOINT
        derivative = error - last_error
        last_error = error
        speed = MAX_SPEED - (abs(error) * 4)
        speed = max(MIN_SPEED, min(MAX_SPEED, speed))
        turn_rate = KP * error + KD * derivative
        drive.drive(speed, turn_rate)
        wait(10)
    drive.stop()
```

## Detección de intersecciones (T-junctions, cross)

Una intersección es donde un sensor adicional perpendicular al sentido de marcha detecta negro. Agregar un **tercer sensor** apuntando a un costado o al frente.

```python
color_main = ColorSensor(Port.E)        # sigue el borde
color_junction = ColorSensor(Port.F)    # detecta intersecciones (offset lateral)

def follow_until_junction():
    while True:
        if color_junction.reflection() < 20:
            drive.stop()
            return
        error = color_main.reflection() - SETPOINT
        drive.drive(BASE_SPEED, KP * error)
        wait(10)
```

Después de detectar la intersección, avanzar un poco más recto para que el axle quede sobre la intersección antes de girar (compensa la distancia sensor-axle).

## Detección de gaps (RCJ Rescue Line)

```python
def follow_with_gap_recovery():
    while True:
        if sensor.reflection() > 70:  # todo blanco — posible gap
            drive.straight(50)        # avanzar recto
            if sensor.reflection() < 30:
                continue              # ya volvimos
            # Sweep buscando la línea
            drive.turn(30)
            if sensor.reflection() < 30: continue
            drive.turn(-60)
            if sensor.reflection() < 30: continue
            drive.turn(30)  # volver al heading original
        error = sensor.reflection() - SETPOINT
        drive.drive(BASE_SPEED, KP * error)
        wait(10)
```

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| Robot oscila visiblemente | Kp muy alto | Bajar Kp 30%, agregar Kd |
| Se sale en curvas cerradas | Kp bajo o velocidad alta | Subir Kp 50% o bajar BASE_SPEED |
| Zigzaguea suave en recta | Kd alto o setpoint mal | Bajar Kd, recalibrar white/black |
| Funciona con buena luz, falla con sol | Reflejos confunden | Cubrir sensor con falda negra |
| Funciona en un robot pero no en otro idéntico | Calibración distinta | Calibrar cada robot por separado |
| Funciona bater llena, falla bater media | BASE_SPEED depende del voltaje | Bajar 20% como margen |
| Pierde línea en U-turns | Sensor muy adelante del axle | Mover sensor cerca del axle, bajar velocidad |
| Detecta inicial y arranca, después se pierde | Ruido en `reflection()` | Promediar 3 lecturas |

## Patrón: arrancar → seguir hasta intersección → salir

```python
def line_to_intersection_and_exit(exit_turn_angle):
    """Sigue línea hasta intersección, sale girando exit_turn_angle."""
    while color_junction.reflection() > 30:
        error = color_main.reflection() - SETPOINT
        drive.drive(BASE_SPEED, KP * error)
        wait(10)
    drive.stop()
    drive.straight(40)  # centrar axle sobre la intersección
    drive.turn(exit_turn_angle)
```

## Notas finales

- **Loop a >50 Hz.** `wait(10)` da 100 Hz que es suficiente. Sin wait, satura CPU y el watchdog del hub puede matarlo.
- **Probar con la línea real**, no cinta de oficina (reflectividad distinta).
- **Documentar valores calibrados en comentarios al inicio.** Cuando el robot competa en otra cancha, **recalibrar SIEMPRE** — los pisos varían más de lo que parece.
