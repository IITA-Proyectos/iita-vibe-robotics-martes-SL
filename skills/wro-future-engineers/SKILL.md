---
name: wro-future-engineers
description: Estrategia y arquitectura para WRO Future Engineers — la categoría de auto autónomo de WRO con vehículo Ackermann, cámara, y misiones de circuit + obstacle avoidance + parking. Usar SIEMPRE que se trabaje en WRO Future Engineers, FE, autonomous car competition, vehículo Ackermann, steering servo, computer vision con Raspberry Pi/OpenCV para detección de obstáculos por color, parking paralelo autónomo, navegación por circuito cerrado con paredes, o se mencione 'Future Engineers', 'autonomous car', 'Ackermann steering', 'parallel parking', 'pillars', 'red green pillar', 'OpenCV ROI', 'PiCamera'. NO usar para WRO RoboMission (eso va en wro-robomission-strategy) ni Football (wro-football). Esta categoría usa Raspberry Pi + Pi Camera, NO Spike Prime.
---

# WRO Future Engineers — auto autónomo con visión

WRO Future Engineers (FE) es la categoría más sofisticada de WRO. Equipos construyen un **auto autónomo a escala** que recorre un circuito cerrado evitando pilares de colores y completando un parking paralelo. Es la categoría más cercana a la robótica autónoma profesional (auto-driving cars). Esta skill cubre la arquitectura, algoritmos de visión, y patrones de control que usan los equipos top.

## Setup hardware típico

A diferencia del resto de WRO, FE **NO usa Spike Prime**. El setup canónico es:

| Componente | Función |
|---|---|
| **Raspberry Pi 4** o **5** | Cerebro principal, corre Python con OpenCV |
| **Pi Camera v2 / v3** o **USB cam** | Visión frontal |
| **Servo MG996R** o similar | Steering Ackermann |
| **Motor DC con encoder + driver** (L298N, TB6612) | Tracción trasera |
| **IMU** (MPU6050, BNO055) | Yaw para navegación |
| **VL53L0X / VL53L1X** (LiDAR ToF) | Distancia a paredes/obstáculos |
| **Power bank 5V** + **batería motor 7.4V** | Alimentación |

Pybricks **no aplica directamente** acá, pero los conceptos de control y navegación son los mismos.

## Las dos fases de la competición

### Fase 1 — Open Round (sin obstáculos)

El robot recorre 3 vueltas al circuito sin obstáculos en el menor tiempo posible. La dirección de marcha (clockwise/counterclockwise) la determina el juez al inicio del round. Puntos por completar las 3 vueltas + bonus por tiempo.

**Estrategia**: navegación pura por paredes laterales con sensores ToF + IMU. El truco es ir lo más rápido posible sin colisionar.

### Fase 2 — Obstacle Round (con pilares)

El robot recorre 3 vueltas evitando **pilares rojos y verdes** distribuidos en el circuito. La regla:

- **Pilar verde** → pasar por la **derecha**.
- **Pilar rojo** → pasar por la **izquierda**.

Más el parking paralelo al final entre dos pilares magenta. **Esta fase requiere visión por computadora** — los sensores ToF no distinguen color.

## Arquitectura de software recomendada

```
auto/
├── main.py                     ← Loop principal y state machine
├── perception/
│   ├── camera.py               ← Captura de Pi Camera
│   ├── color_detection.py      ← Detección de pilares por HSV
│   └── lane_detection.py       ← Detección de paredes laterales (opcional)
├── control/
│   ├── steering.py             ← PID de steering Ackermann
│   ├── throttle.py             ← Control de velocidad
│   └── parking.py              ← Maniobra de parking paralelo
├── navigation/
│   ├── state_machine.py        ← Estados: lap1, lap2, lap3, parking, done
│   └── obstacle_avoidance.py   ← Lógica rojo/verde
└── sensors/
    ├── imu.py
    └── tof.py
```

## Detección de pilares con OpenCV

El truco es trabajar en **espacio HSV** (no RGB) porque es robusto a cambios de iluminación.

```python
import cv2
import numpy as np

# Rangos HSV calibrados empíricamente con los pilares reales
RED_LOWER_1 = np.array([0, 120, 70])
RED_UPPER_1 = np.array([10, 255, 255])
RED_LOWER_2 = np.array([170, 120, 70])  # rojo wrapping
RED_UPPER_2 = np.array([180, 255, 255])

GREEN_LOWER = np.array([40, 70, 70])
GREEN_UPPER = np.array([85, 255, 255])

MAGENTA_LOWER = np.array([140, 100, 100])
MAGENTA_UPPER = np.array([170, 255, 255])

def detect_pillars(frame):
    """Devuelve lista de detecciones [(color, x, y, area), ...]."""
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    
    # Máscaras de color
    mask_red = cv2.inRange(hsv, RED_LOWER_1, RED_UPPER_1) | cv2.inRange(hsv, RED_LOWER_2, RED_UPPER_2)
    mask_green = cv2.inRange(hsv, GREEN_LOWER, GREEN_UPPER)
    
    # Limpiar ruido con morphological ops
    kernel = np.ones((5, 5), np.uint8)
    mask_red = cv2.morphologyEx(mask_red, cv2.MORPH_OPEN, kernel)
    mask_green = cv2.morphologyEx(mask_green, cv2.MORPH_OPEN, kernel)
    
    detections = []
    
    for color, mask in [('red', mask_red), ('green', mask_green)]:
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        for c in contours:
            area = cv2.contourArea(c)
            if area > 500:  # filtrar ruido pequeño
                x, y, w, h = cv2.boundingRect(c)
                cx = x + w // 2
                cy = y + h // 2
                detections.append((color, cx, cy, area))
    
    return detections
```

## ROI — Region of Interest

Procesar el frame entero es lento. **Recortar solo la zona donde pueden aparecer pilares** baja el tiempo de procesamiento de 50ms a 10ms y aumenta el frame rate.

```python
def get_roi(frame):
    """Recorta el ROI: mitad inferior del frame, sin los bordes laterales."""
    h, w = frame.shape[:2]
    roi = frame[h//2:h, w//6:5*w//6]
    return roi
```

## Estrategia de evasión rojo/verde

```python
TARGET_LATERAL_OFFSET = 100  # px del centro del frame
KP_STEER = 0.5

def avoidance_command(detections, frame_width):
    """Devuelve un steering correction basado en pilares detectados."""
    if not detections:
        return 0  # sin pilares, ir recto
    
    # Tomar el pilar más grande (más cercano)
    nearest = max(detections, key=lambda d: d[3])  # max por area
    color, cx, cy, area = nearest
    
    frame_center = frame_width // 2
    
    if color == 'red':
        # Pasar por la izquierda → robot debe estar a la derecha del pilar
        # Pilar debe quedar a la izquierda del frame → cx < center
        target_cx = frame_center + TARGET_LATERAL_OFFSET
    elif color == 'green':
        # Pasar por la derecha → pilar a la derecha del frame
        target_cx = frame_center - TARGET_LATERAL_OFFSET
    
    error = target_cx - cx
    steering = KP_STEER * error
    return max(-30, min(30, steering))  # clip a ±30°
```

## Steering Ackermann con servo

A diferencia de un robot diferencial, un auto Ackermann tiene **steering físico** con un ángulo limitado (típicamente ±30° del centro).

```python
import RPi.GPIO as GPIO
from gpiozero import Servo

steering_servo = Servo(18, min_pulse_width=0.5/1000, max_pulse_width=2.5/1000)

def set_steering_angle(angle_deg):
    """angle_deg en [-30, 30]. 0 = recto."""
    angle_clipped = max(-30, min(30, angle_deg))
    # gpiozero Servo usa -1 a 1
    value = angle_clipped / 30.0
    steering_servo.value = value
```

## Detección de paredes laterales con ToF

Para la fase open round, dos sensores ToF apuntando perpendicular dan el offset lateral del robot al centro del carril.

```python
import board
import busio
import adafruit_vl53l1x

i2c = busio.I2C(board.SCL, board.SDA)
tof_left = adafruit_vl53l1x.VL53L1X(i2c, address=0x29)
tof_right = adafruit_vl53l1x.VL53L1X(i2c, address=0x30)

def lane_offset_mm():
    """Offset del centro del carril. Positivo = robot a la derecha del centro."""
    left_dist = tof_left.distance * 10  # cm → mm
    right_dist = tof_right.distance * 10
    return (left_dist - right_dist) / 2.0
```

Y el control:

```python
KP_LANE = 0.3

def follow_center_of_lane():
    offset = lane_offset_mm()
    steering = -KP_LANE * offset  # negativo porque + offset → girar a la izquierda
    set_steering_angle(steering)
```

## State machine del run

```python
class State:
    INIT = 0
    LAP_1 = 1
    LAP_2 = 2
    LAP_3 = 3
    PARKING = 4
    DONE = 5

state = State.INIT
laps_completed = 0
last_lap_marker = 0

def main_loop():
    global state, laps_completed
    
    while True:
        frame = camera.capture()
        detections = detect_pillars(frame)
        
        if state == State.INIT:
            # Detectar dirección del circuito
            state = State.LAP_1
        
        elif state in [State.LAP_1, State.LAP_2, State.LAP_3]:
            # Navegación con evasión
            steering = avoidance_command(detections, frame.shape[1])
            if steering == 0:
                follow_center_of_lane()
            else:
                set_steering_angle(steering)
            
            # Detección de fin de vuelta (línea naranja/azul en el suelo)
            if detect_lap_marker(frame) and time.time() - last_lap_marker > 3:
                laps_completed += 1
                last_lap_marker = time.time()
                if laps_completed == 3:
                    state = State.PARKING
        
        elif state == State.PARKING:
            execute_parking_maneuver()
            state = State.DONE
        
        elif state == State.DONE:
            stop()
            break
```

## Parking paralelo

La maniobra clásica de parking paralelo se programa como una secuencia hardcoded de comandos en función de la geometría del slot:

```python
def execute_parking_maneuver():
    """Asume que el robot está alineado con el slot a su derecha."""
    set_throttle(0.3)  # avanzar lento
    drive_forward(distance_mm=100)
    
    set_steering_angle(30)  # giro completo a la derecha
    drive_backward(distance_mm=200)
    
    set_steering_angle(-30)  # giro completo a la izquierda
    drive_backward(distance_mm=200)
    
    set_steering_angle(0)
    drive_forward(distance_mm=50)  # centrar en el slot
    stop()
```

Las distancias exactas se ajustan empíricamente sobre el slot real.

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| Detección de color falla con luz cambiante | Rangos HSV mal calibrados | Recalibrar en la cancha real con la luz real |
| Frame rate <15 fps | Resolución muy alta, sin ROI | Bajar a 320×240, usar ROI |
| Robot oscila entre paredes | Kp lateral muy alto | Bajar Kp, agregar D |
| Detecta sombras como pilares | Threshold de saturación bajo | Subir saturation min en HSV |
| Parking sale corto | Distancias hardcoded sin slip | Calibrar emp con slot real |
| Robot pasa pilar por el lado equivocado | Lógica rojo/verde invertida | Revisar el signo del error en avoidance_command |

## Tips finales

- **Frame rate >20 fps es crítico.** Si bajás de eso, la latencia entre detección y comando se nota y el robot oscila.
- **Recortar el ROI agresivamente** y trabajar a 320×240 o incluso 240×180.
- **Calibrar HSV con el set real** bajo la iluminación de la cancha de competición. Los pilares de práctica pueden tener tonos distintos.
- **Logging visual**: durante development guardar frames con las detecciones dibujadas para debug post-run.
- **Encoder de tracción crítico** para distancias precisas en parking.
- **No confiar ciegamente en visión** — combinar con ToF lateral siempre como sanity check.

## Recursos

- WRO Future Engineers rulebook: https://wro-association.org/competition/wro-international/rulebook/
- OpenCV color detection tutorial: https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html
- Repos referencia top: buscar "WRO Future Engineers" en GitHub, equipos top de Indonesia/Rusia/Polonia tienen repos públicos.
