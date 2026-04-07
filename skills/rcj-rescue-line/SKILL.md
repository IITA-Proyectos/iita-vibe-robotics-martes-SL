---
name: rcj-rescue-line
description: Estrategia y arquitectura para RoboCup Junior Rescue Line — el seguidor de línea más complejo del circuito de robótica educativa, con gaps, intersecciones, obstáculos, rampas, intersecciones plateadas, victims y zona de evacuación. Usar SIEMPRE que se trabaje en RCJ Rescue Line, RoboCup Junior Rescue, robot rescate, line rescue robot, evacuación de víctimas, detección de víctimas plateadas/negras, rampa, obstáculos en pista de rescate, o se mencione 'RCJ Rescue Line', 'RoboCup Junior Rescue', 'evacuation zone', 'silver victim', 'black victim', 'red line stop', 'green marker', 'turn marker'. NO usar para line following genérico (eso va en pybricks-line-following) ni Soccer.
---

# RoboCup Junior Rescue Line — robot de rescate

RCJ Rescue Line es probablemente **el ejercicio de seguidor de línea más complejo del circuito juvenil**. Un robot autónomo recorre una pista negra sobre fondo blanco que incluye: curvas cerradas, intersecciones señalizadas con marcadores verdes, gaps en la línea, obstáculos físicos, rampas inclinadas, intersecciones plateadas, y termina en una **zona de evacuación** donde debe encontrar y rescatar **víctimas** (bolas). Esta skill cubre los algoritmos y patrones específicos.

## Anatomía de la pista

| Elemento | Descripción |
|---|---|
| **Línea negra** | Camino principal, ~2 cm de ancho sobre fondo blanco |
| **Gaps** | Tramos sin línea de hasta 20 cm que el robot debe cruzar a ciegas |
| **Curvas** | Cualquier ángulo, incluyendo zigzags y U-turns |
| **Marcador verde** | Cuadrado verde antes de una intersección indica girar (a la izquierda, derecha, o ambos = 180°) |
| **Marcador rojo** | Línea roja perpendicular = stop obligatorio de 5 segundos |
| **Obstáculos** | Cubos o ladrillos físicos sobre la línea, robot debe evitarlos y volver a la línea |
| **Rampas** | Inclinaciones de hasta 25°, robot debe subirlas sin perder la línea |
| **Speed bumps** | Lomos de burro físicos que sacuden el robot |
| **Intersecciones plateadas** | Zona reflectante (cinta plateada) que separa el campo de la evacuación |
| **Zona de evacuación** | Cuarto cerrado al final con paredes, donde el robot busca víctimas |
| **Víctimas** | Bolas de 4 cm. **Plateadas** = vivas (más puntos), **negras** = muertas (menos puntos) |
| **Triángulo de evacuación** | Esquina del cuarto donde depositar las víctimas |

## Setup hardware típico

- **2 ColorSensor** apuntando al piso, separados ~3-4 cm para line following + detección lateral.
- **1 ColorSensor frontal** ligeramente elevado para detectar el marcador plateado de la zona de evacuación.
- **1-2 UltrasonicSensor** o ToF para detectar obstáculos y paredes en la zona de evacuación.
- **1 mecanismo de garra/cuchara** (motor C) para levantar las víctimas.
- **IMU del Prime Hub** para mantener heading después de gaps y obstáculos.

## Arquitectura de software

```
rescue/
├── main.py
├── line_follower.py        ← Edge follower con PID + detección de marcadores
├── junction_handler.py     ← Lógica de intersecciones por color
├── gap_recovery.py         ← Recuperación cuando se pierde la línea
├── obstacle_avoidance.py   ← Esquivar y volver
├── ramp_handler.py         ← Detección y manejo de rampas
└── evacuation/
    ├── victim_search.py    ← Búsqueda de víctimas en el cuarto
    ├── victim_classifier.py ← Plateada vs negra
    └── deposit.py          ← Llevar al triángulo de evacuación
```

## Line following adaptado para RCJ

Diferencia clave con line following genérico: el robot tiene que **detectar simultáneamente** la línea, los marcadores verdes, los gaps, y los obstáculos. La función del loop principal no es solo "seguir la línea" sino "decidir qué hacer en cada tick".

```python
from pybricks.parameters import Color

color_left = ColorSensor(Port.E)   # primary line follower
color_right = ColorSensor(Port.F)  # detecta marcadores verdes y gaps

KP = 0.8
KD = 4.0
BASE_SPEED = 200

WHITE = 90
BLACK = 8
GREEN_HUE_RANGE = (90, 150)  # rango HSV del verde del marcador

def is_green(sensor):
    """Detecta marcador verde por HSV (más robusto que color())."""
    h, s, v = sensor.hsv()
    return GREEN_HUE_RANGE[0] < h < GREEN_HUE_RANGE[1] and s > 40 and v > 30

def main_loop():
    last_error = 0
    while True:
        l_refl = color_left.reflection()
        r_refl = color_right.reflection()
        
        # 1. Check rampa
        pitch = hub.imu.tilt()[0]
        if abs(pitch) > 10:
            handle_ramp(pitch)
            continue
        
        # 2. Check obstáculo enfrente
        if us_front.distance() < 80:
            avoid_obstacle()
            continue
        
        # 3. Check marcador verde (intersección señalizada)
        if is_green(color_left) or is_green(color_right):
            handle_green_marker()
            continue
        
        # 4. Check intersección plateada (zona de evacuación)
        if is_silver(color_front):
            enter_evacuation_zone()
            return
        
        # 5. Check gap (todo blanco)
        if l_refl > 70 and r_refl > 70:
            recover_from_gap()
            continue
        
        # 6. Line following normal
        error = l_refl - SETPOINT
        derivative = error - last_error
        last_error = error
        turn_rate = KP * error + KD * derivative
        drive.drive(BASE_SPEED, turn_rate)
        wait(20)
```

## Manejo de marcadores verdes

Los marcadores verdes preceden las intersecciones e indican qué dirección tomar:

- **Verde a la izquierda** → girar a la izquierda
- **Verde a la derecha** → girar a la derecha
- **Verde a ambos lados** → girar 180°

```python
def handle_green_marker():
    """Detecta cuántos verdes hay y gira en consecuencia."""
    drive.straight(30)  # avanzar para confirmar la intersección
    
    left_green = is_green(color_left)
    right_green = is_green(color_right)
    
    drive.straight(50)  # avanzar al centro de la intersección
    
    if left_green and right_green:
        drive.turn(180)
    elif left_green:
        drive.turn(-90)
    elif right_green:
        drive.turn(90)
    
    # Después del giro, retomar line following
    # El robot está sobre o cerca de la nueva línea
```

## Recuperación de gaps

Cuando los dos sensores ven blanco simultáneamente, hay un gap. El robot debe avanzar a ciegas en línea recta hasta encontrar la línea de nuevo.

```python
def recover_from_gap():
    """Avanza recto hasta GAP_MAX_DISTANCE buscando la línea."""
    GAP_MAX_DISTANCE = 250  # mm
    drive.use_gyro(True)
    initial_heading = hub.imu.heading()
    drive.reset()
    
    while drive.distance() < GAP_MAX_DISTANCE:
        # Mantener heading absoluto
        error = hub.imu.heading() - initial_heading
        drive.drive(150, -error * 2)
        
        # Check si recuperamos la línea
        if color_left.reflection() < 30 or color_right.reflection() < 30:
            drive.stop()
            return True
        wait(20)
    
    # Gap demasiado largo o el camino se desvió → buscar con sweep
    drive.stop()
    return sweep_for_line()

def sweep_for_line():
    """Hace un barrido en abanico buscando la línea."""
    for angle in [30, -60, 90, -120]:
        drive.turn(angle)
        if color_left.reflection() < 30:
            return True
    return False
```

## Esquivar obstáculos

Los obstáculos son cubos LEGO físicos sobre o cerca de la línea. El robot debe pasar al costado y volver.

```python
def avoid_obstacle():
    """Patrón clásico: girar 90, avanzar, girar -90, avanzar, girar -90, avanzar, girar 90."""
    drive.turn(90)         # girar a la derecha
    drive.straight(150)    # avanzar lateralmente
    drive.turn(-90)        # apuntar hacia adelante
    
    # Avanzar hasta pasar el obstáculo, buscando la línea
    drive.reset()
    while drive.distance() < 400:
        drive.drive(150, 0)
        if color_left.reflection() < 30:
            # Encontramos la línea de nuevo
            drive.stop()
            return
        wait(20)
    
    drive.stop()
    drive.turn(-90)        # girar a la izquierda
    drive.straight(150)    # cerrar el rectángulo
    drive.turn(90)
```

## Detección de rampa

```python
from pybricks.parameters import Axis

def handle_ramp(pitch):
    """En la rampa, mantener line following pero con velocidad reducida."""
    while abs(hub.imu.tilt()[0]) > 5:
        # Mismo line following pero más lento (motores cargan más)
        error = color_left.reflection() - SETPOINT
        drive.drive(120, KP * 1.3 * error)  # más lento + más Kp
        wait(20)
    # Salimos de la rampa
```

## Zona de evacuación — el endgame

El cuarto de evacuación es un cuarto cerrado con paredes. El robot debe:

1. Entrar.
2. Buscar las víctimas (bolas plateadas y negras).
3. Tomar cada víctima.
4. Llevarlas al triángulo de evacuación.
5. Depositarlas.

### Búsqueda en espiral o en zigzag

```python
def search_for_victims():
    """Recorre el cuarto en patrón de espiral hasta detectar una víctima."""
    while not victim_detected():
        drive.straight(200)
        if us_front.distance() < 100:
            drive.turn(90)  # esquivar pared
        if us_left.distance() < 100:
            drive.turn(45)
        wait(20)

def victim_detected():
    """Una víctima cerca dispara el ultrasonido frontal a una distancia específica."""
    return 30 < us_front.distance() < 80
```

### Clasificación plateada vs negra

```python
def classify_victim():
    """Después de detectar una víctima al frente, acercarse y clasificarla."""
    drive.straight(50)  # acercar
    
    refl = color_front.reflection()
    if refl > 60:
        return 'silver'  # más reflectante = víctima viva
    else:
        return 'black'   # menos reflectante = víctima muerta
```

### Depositar en el triángulo

El triángulo de evacuación está en una **esquina del cuarto**. Identificarlo por la geometría: dos paredes formando 90°.

```python
def find_evacuation_triangle():
    """Navega a la esquina del cuarto donde dos paredes se encuentran."""
    while True:
        front = us_front.distance()
        left = us_left.distance()
        if front < 100 and left < 100:
            # Esquina encontrada
            return
        if front > 100:
            drive.straight(50)
        elif left > 100:
            drive.turn(-90)
            drive.straight(50)
        wait(20)
```

## Estrategia de scoring

El scoring de RCJ Rescue Line es complejo y privilegia **completar la pista entera** sobre completar tareas individuales. Lecciones tácticas:

1. **Las víctimas plateadas valen más que las negras.** Si solo hay tiempo para una, priorizar plateada.
2. **Un robot que termina la pista sin víctimas vale más que uno que rescata todas pero queda atascado en la rampa.**
3. **Lack of progress** (LoP): si el robot se queda atascado, el equipo puede pedir un LoP para reposicionarlo al último checkpoint, pagando una penalización menor. **Es crítico saber cuándo pedir LoP** vs seguir intentando.

## Errores típicos

| Error | Causa | Solución |
|---|---|---|
| Detecta verde como negro | `color()` clasificación discreta falla | Usar HSV: hue 90-150, saturation > 40 |
| Pierde la línea en curvas cerradas | Velocidad alta + sensor lejos del axle | Bajar velocidad en curvas, mover sensor cerca del axle |
| No recupera después de gap largo | `GAP_MAX_DISTANCE` muy corto | Subir a 300-400 mm con gyro firme |
| No identifica intersección plateada | Sensor mal calibrado al plateado | Calibrar sensor frontal específicamente con cinta plateada |
| Rampa hace que el robot rebote | `straight_acceleration` muy alta | Bajar aceleración + agregar peso al frente |
| Confunde víctima negra con sombra | Threshold de reflexión muy alto | Bajar threshold + mejorar iluminación del sensor |
| Trae la víctima fuera del triángulo | Detección de esquina mal | Usar 2 sensores ToF perpendiculares |

## Recursos

- RCJ Rescue Line rulebook: https://junior.robocup.org/rcjrescue/
- Repo IITA: `IITA-Proyectos/rcj-2026-rescue-line-iita-salta-robocup`
- RoboCup 2026 Incheon: junio 30-julio 6.
- Comunidad: Discord oficial de RCJ.
