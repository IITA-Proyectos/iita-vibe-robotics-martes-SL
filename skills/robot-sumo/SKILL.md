---
name: robot-sumo
description: Estrategia y patrones para combate robótico de sumo (mini-sumo y full-sumo) con LEGO Spike Prime y Pybricks. Usar SIEMPRE que se trabaje en sumo robotico, mini-sumo, robo-sumo, lego sumo, edge detection en el dohyo, opponent tracking con sensores ultrasónicos o IR, push strategies, bullying patterns, dohyo, sumo arena, evitar caer del ring, o se mencione 'sumo', 'mini-sumo', 'dohyo', 'opponent', 'edge sensor', 'push', 'bull rush'. NO usar para line following (eso va en pybricks-line-following) ni soccer.
---

# Robot Sumo — combate robótico

Sumo robótico es un combate 1v1 entre dos robots autónomos sobre un **dohyo** (ring circular) donde el objetivo es **empujar al oponente fuera del ring** sin caer uno mismo. Las dos categorías principales son:

| Categoría | Tamaño máx | Peso máx | Dohyo |
|---|---|---|---|
| **Mini-sumo** | 10×10 cm base | 500 g | 77 cm de diámetro |
| **Full-sumo** | 20×20 cm base | 3 kg | 154 cm de diámetro |
| **Lego sumo** (educativo) | varía | varía | varía |

Para LEGO Spike Prime, lo más común es **mini-sumo o lego-sumo educativo**.

## Reglas básicas

- 1v1, mejor de 3 rondas.
- Cada ronda dura ~3 minutos máximo, pero la mayoría termina en <30 segundos.
- **Caer del dohyo o tocar el suelo fuera con cualquier parte del robot** = pierde la ronda.
- Robot 100% autónomo (sin control remoto).
- Después de un comando "Hajime!" (start), el robot debe detectar al oponente y atacar.
- Hay un delay de inicio de ~5 segundos antes de poder empezar a moverse (regla anti-arranque trampa).

## Setup hardware típico para sumo con Spike Prime

- **2 motores de tracción** con la mayor torque/velocidad posible (motores grandes de Spike).
- **2 ColorSensor** apuntando al piso, uno adelante a cada lado, para detectar el **borde blanco del dohyo**.
- **2-4 sensores de oponente**:
  - **UltrasonicSensor** al frente para detectar oponente directo enfrente.
  - **2 sensores IR proximity** o **ToF VL53L0X** a los costados frontales para detectar oponente a 30-45° del frente.
- **Pala/cuña frontal** baja al ras del piso para meter debajo del oponente y elevarlo (ventaja de empuje).
- **Peso máximo permitido** distribuido bajo y centrado para mejor tracción y resistencia al volcado.

## Decisión clave: agresivo vs defensivo

### Agresivo (bull rush)

El robot va al centro del dohyo y embiste al primer contacto detectado. Apuesta por la potencia bruta.

**Pros**: 70% de los matches se ganan en los primeros 5 segundos por bull rush exitoso. Simple de programar.

**Contras**: si el oponente es más rápido o tiene mejor pala, perdés.

### Defensivo / contraataque

El robot espera en el centro, gira buscando al oponente, ataca solo cuando el oponente está identificado y bien encarado.

**Pros**: aprovecha errores del oponente, mejor contra robots agresivos pero mal calibrados.

**Contras**: contra otro defensivo es un standoff aburrido que termina en empate.

### Adaptativo (recomendado para escuela)

Empieza agresivo, si pierde una ronda cambia a defensivo, si pierde otra vuelve a agresivo con un patrón distinto.

## Edge detection — la prioridad #1

**Antes de pensar en empujar, el robot tiene que NO caer del dohyo.** Los dos sensores de color al frente leen el piso constantemente:

```python
color_left = ColorSensor(Port.E)
color_right = ColorSensor(Port.F)

# El dohyo es negro, el borde es blanco
EDGE_THRESHOLD = 50  # >50 = blanco = borde

def at_edge():
    """Devuelve qué borde detectado: 'left', 'right', 'both', None."""
    l = color_left.reflection() > EDGE_THRESHOLD
    r = color_right.reflection() > EDGE_THRESHOLD
    if l and r:
        return 'both'
    elif l:
        return 'left'
    elif r:
        return 'right'
    return None
```

Y la rutina de evasión cuando se detecta el borde **interrumpe TODO**:

```python
def avoid_edge(edge):
    """Reacción inmediata al detectar borde. PRIORIDAD ABSOLUTA."""
    drive.stop()
    
    if edge == 'both':
        # Borde frontal directo → retroceder y girar
        drive.straight(-200)
        drive.turn(180)
    elif edge == 'left':
        # Borde a la izquierda → retroceder y girar a la derecha
        drive.straight(-100)
        drive.turn(45)
    elif edge == 'right':
        drive.straight(-100)
        drive.turn(-45)
    
    # Después de evadir, volver a buscar al oponente
```

## Detección del oponente

El sensor frontal ultrasónico te dice si hay algo enfrente. Los sensores laterales te dicen si está a un costado.

```python
us_front = UltrasonicSensor(Port.D)

def opponent_distance():
    """Distancia al oponente enfrente, en mm. None si no detectado."""
    d = us_front.distance()
    if d > 1500:
        return None  # nadie a la vista
    return d

def opponent_direction():
    """Devuelve 'front', 'left', 'right', None."""
    front = us_front.distance() < 800
    # Suponiendo que tenemos sensores IR a los lados
    left = ir_left.value() > IR_THRESHOLD
    right = ir_right.value() > IR_THRESHOLD
    
    if front:
        return 'front'
    elif left and not right:
        return 'left'
    elif right and not left:
        return 'right'
    return None
```

## Loop principal del agresivo

```python
SEARCH_SPEED = 200
ATTACK_SPEED = 800  # MÁXIMA velocidad cuando hay contacto inminente
DELAY_START = 5000  # ms

def main():
    # Delay inicial obligatorio
    sw = StopWatch()
    sw.reset()
    while sw.time() < DELAY_START:
        wait(50)
    
    # Combate
    while True:
        # PRIORIDAD 1: edge detection
        edge = at_edge()
        if edge:
            avoid_edge(edge)
            continue
        
        # PRIORIDAD 2: si hay oponente detectado, atacar
        direction = opponent_direction()
        if direction == 'front':
            # Bull rush
            drive.drive(ATTACK_SPEED, 0)
        elif direction == 'left':
            drive.drive(SEARCH_SPEED, -200)  # girar hacia él
        elif direction == 'right':
            drive.drive(SEARCH_SPEED, 200)
        else:
            # PRIORIDAD 3: buscar al oponente girando en el lugar
            drive.drive(0, 150)  # rotar
        
        wait(20)
```

## Loop principal del defensivo

```python
def main_defensive():
    sw = StopWatch()
    sw.reset()
    while sw.time() < DELAY_START:
        wait(50)
    
    while True:
        edge = at_edge()
        if edge:
            avoid_edge(edge)
            continue
        
        direction = opponent_direction()
        distance = opponent_distance()
        
        if direction == 'front' and distance and distance < 200:
            # Solo atacar cuando está MUY cerca y enfrente
            drive.drive(ATTACK_SPEED, 0)
        elif direction in ['left', 'right']:
            # Encararlo pero sin avanzar
            turn_rate = 200 if direction == 'right' else -200
            drive.drive(0, turn_rate)
        else:
            # Quedarse en el centro buscando
            drive.drive(0, 100)
        
        wait(20)
```

## Patrones avanzados

### "El esquive" (dodge)

Cuando detectás al oponente embistiendo de frente, **moverte lateralmente** en el último momento. Si el oponente va a 800 mm/s y vos te corrés 30 cm en 0.3 seg, le esquivás y queda con la inercia mirando hacia el borde.

```python
def dodge_left():
    """Esquive lateral izquierdo de un ataque frontal."""
    drive.use_gyro(False)
    left_motor.run(-800)
    right_motor.run(800)  # rotar 90° rápido a la izquierda
    wait(150)
    drive.straight(150)  # avanzar lateralmente
    drive.turn(90)       # reorientar para contraatacar al oponente que pasó de largo
```

### "El gancho" (hook)

Engañar al oponente acercándose y desviando en el último momento para tomarlo por el costado, donde su pala no protege.

### "El empuje continuo"

Una vez que tu pala está debajo del oponente, **NO PARES de empujar** hasta que cruce el borde del dohyo. Mantener `drive.drive(MAX_SPEED, 0)` hasta detectar que el oponente desapareció (cayó) o que vos estás llegando al borde.

```python
def push_until_win_or_edge():
    while True:
        edge = at_edge()
        if edge:
            # CUIDADO: sos vos el que está por caer
            avoid_edge(edge)
            return
        
        if us_front.distance() > 500:
            # El oponente desapareció = lo empujamos fuera
            drive.stop()
            hub.light.on(Color.GREEN)
            return
        
        # Seguir empujando
        drive.drive(MAX_SPEED, 0)
        wait(20)
```

## Diseño mecánico crítico

**Lo que más importa en sumo después del código es la mecánica:**

1. **Pala al ras del piso.** Si la pala del oponente queda debajo de la tuya, perdés. La tuya tiene que ser más baja (a 1-2 mm del piso).
2. **Centro de gravedad bajo.** Robots altos se vuelcan con un golpe lateral.
3. **Tracción máxima.** Ruedas con el mejor grip + peso sobre el axle motriz.
4. **Robustez frontal.** El frente recibe todos los impactos, tiene que aguantar.

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| Cae del dohyo en los primeros segundos | No hay edge detection prioritaria | `avoid_edge()` antes que cualquier otra lógica |
| Detecta al oponente pero no lo alcanza | Velocidad de búsqueda baja | Subir SEARCH_SPEED |
| Empuja pero el oponente lo levanta | Pala no llega al ras | Rebajar pala mecánicamente |
| Va al centro y se queda esperando | No tiene patrón de búsqueda | Agregar rotación continua cuando no hay oponente detectado |
| Pierde contra robots laterales | Sin sensores laterales | Agregar IR proximity a 30-45° del frente |
| Se atasca empujando contra una pala enemiga | Sin lógica de retroceso | Detectar stall: si el robot no avanza por >500ms, retroceder y reintentar otro ángulo |

## Recursos

- All Japan Robot-Sumo Tournament: la competición de mini-sumo profesional.
- RoboCup Junior tiene categoría "On Stage" pero NO tiene sumo oficial.
- LEGO sumo educativo: cualquier club o escuela puede organizarlo, no hay rulebook oficial único.
- Tutorial mini-sumo Pololu: https://www.pololu.com/docs/0J11/0
