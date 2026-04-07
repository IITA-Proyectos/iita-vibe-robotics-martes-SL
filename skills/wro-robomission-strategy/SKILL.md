---
name: wro-robomission-strategy
description: Estrategia y patrones de programación para WRO RoboMission (Elementary, Junior, Senior) con LEGO Spike Prime y Pybricks. Usar SIEMPRE que se trabaje en estrategia para WRO RoboMission, gestión de attachments, multi-run vs single-run, recovery patterns, time management durante rondas, scoring strategy, color sensor para detectar misiones, uso de paredes para localización, o se mencione 'WRO', 'RoboMission', 'World Robot Olympiad', 'mission', 'attachment', 'home base', 'starting area', 'rondas de competición'. NO usar para line following puro (eso va en pybricks-line-following), precision driving puro (pybricks-precision-driving), Future Engineers (wro-future-engineers) ni Football (wro-football).
---

# Estrategia y patrones para WRO RoboMission

WRO RoboMission es la categoría principal del World Robot Olympiad. Equipos construyen robots autónomos con LEGO que completan misiones puntuables sobre un mat estandarizado dentro de un tiempo límite (típicamente 2 minutos por ronda). Esta skill cubre los **patrones estratégicos** que diferencian un robot que puntúa de uno que solo se mueve.

## Categorías y reglas básicas

| Categoría | Edad | Set | Hub |
|---|---|---|---|
| **Elementary** | 8-12 | SPIKE Essential / WeDo 2.0 | Essential Hub |
| **Junior** | 11-15 | SPIKE Prime | Prime Hub |
| **Senior** | 14-19 | SPIKE Prime | Prime Hub |

**Reglas comunes**:

- Robot 100% autónomo (sin control remoto, sin Bluetooth durante el run).
- Tiempo máx por ronda: 2 minutos.
- Robot arranca y termina dentro del **starting area** (home base).
- Cada vez que el robot vuelve al home, se permite **tocarlo** (cambiar attachments, reposicionar, reiniciar programa).
- Tocar el robot **fuera** del home = **lack of control** = perder los puntos del run.
- **Bonus por tiempo restante** si todas las missions completas.

**Las reglas específicas del año vigente están en**: https://wro-association.org/competition/wro-international/rulebook/

## La decisión arquitectónica más grande: single-run vs multi-run

### Single-run

Un programa que arranca, recorre todas las missions, y vuelve al home. Sin tocar el robot.

**Pros**: maximiza time bonus si funciona, menos manipulación humana.

**Contras**: si falla mission #3, perdés #4-#N. Error acumulado de gyro/odometría sobre 2 min puede ser >10 cm. Difícil de debuggear.

**Conviene**: equipos avanzados, robot muy bien calibrado, missions cercanas, mat con pocas misiones grandes.

### Multi-run

El mat se divide en zonas. N programas distintos, uno por zona. Entre programas, vuelve al home, equipo cambia attachment, arranca el siguiente.

**Pros**: resiliente, fácil de iterar, permite squareo entre programas (resetea drift).

**Contras**: pierde time bonus, presión operativa para el equipo.

**Conviene**: equipos principiantes/intermedios, missions distribuidas.

**Recomendación general para IITA**: **multi-run con 3-5 programas** es el sweet spot. Solo equipos top con >6 meses de práctica deberían intentar single-run.

## Selector de programa en el hub

```python
hub = PrimeHub()

programs = [
    ('1', mission_north),
    ('2', mission_east),
    ('3', mission_south),
    ('4', mission_west),
    ('5', mission_center),
]

selected = 0

def show_selection():
    hub.display.text(programs[selected][0])

def program_selector():
    global selected
    show_selection()
    while True:
        pressed = hub.buttons.pressed()
        if Button.LEFT in pressed:
            selected = (selected - 1) % len(programs)
            show_selection()
            while Button.LEFT in hub.buttons.pressed(): wait(10)
        elif Button.RIGHT in pressed:
            selected = (selected + 1) % len(programs)
            show_selection()
            while Button.RIGHT in hub.buttons.pressed(): wait(10)
        elif Button.CENTER in pressed:
            while Button.CENTER in hub.buttons.pressed(): wait(10)
            hub.light.on(Color.GREEN)
            programs[selected][1]()
            hub.light.on(Color.BLUE)
            selected = (selected + 1) % len(programs)  # auto-avanzar
            show_selection()
        wait(50)

program_selector()
```

**Ventajas**: el equipo no tiene que pensar qué programa subir entre runs — el robot avanza solo.

## Patrón de inicio de cada run

```python
def start_run():
    """Llamar al inicio de cada función de mission."""
    drive.stop()
    drive.reset()
    hub.imu.reset_heading(0)
    
    # Squareo contra la pared trasera del home
    drive.use_gyro(False)
    left_motor.run(-80)
    right_motor.run(-80)
    wait(700)
    left_motor.stop()
    right_motor.stop()
    
    drive.use_gyro(True)
    hub.imu.reset_heading(0)
    drive.reset()
    drive.straight(20)  # despegar de la pared
```

**Por qué**: garantiza que el robot **siempre arranca con el mismo heading absoluto** sin importar cómo el equipo lo posicionó. Elimina la principal fuente de variabilidad.

## Gestión de attachments

### Principios de diseño

1. **Mounting rápido y reproducible.** Pins LEGO con tope, no encaje libre.
2. **Driven por motor central** (Port C). Reservar exclusivamente para attachments.
3. **Auto-calibración al inicio.** Cada attachment debe encontrar su zero con `run_until_stalled()`.
4. **Sin colisión con la geometría base** (no mover IMU ni ruedas).

### Auto-calibración con `run_until_stalled()`

```python
attachment = Motor(Port.C)

def calibrate_attachment():
    """Busca su tope mecánico y resetea ángulo a cero."""
    attachment.run_until_stalled(speed=-200, then=Stop.HOLD, duty_limit=50)
    attachment.reset_angle(0)
    wait(100)

def raise_arm():
    attachment.run_target(speed=400, target_angle=90, then=Stop.HOLD)

def lower_arm():
    attachment.run_target(speed=400, target_angle=0, then=Stop.HOLD)
```

`duty_limit=50` evita que el motor empuje con full torque contra el tope (puede romper mecánica o frenar el robot entero).

## Detección de objetos de mission con ColorSensor

```python
def detect_cube_color():
    """Avanza, detecta color del cubo, vuelve."""
    drive.straight(150)
    
    # Promediar 5 lecturas
    readings = []
    for _ in range(5):
        readings.append(color_front.color())
        wait(20)
    
    counts = {}
    for c in readings:
        counts[c] = counts.get(c, 0) + 1
    return max(counts, key=counts.get)
```

**Tips**:
- Restringir colores: `color_front.detectable_colors([Color.RED, Color.GREEN, Color.BLUE, Color.YELLOW])`.
- Distancia al objeto: 5-10 mm. A 20+ mm la lectura se degrada.
- Probar contra los **colores reales del set WRO**, no papel impreso.

## Recovery patterns

### Timeout en attachment

```python
def close_gripper_with_timeout():
    sw = StopWatch()
    sw.reset()
    attachment.run(-300)
    while attachment.speed() > -50 or sw.time() < 200:
        if sw.time() > 2000:
            attachment.stop()
            hub.light.on(Color.RED)
            return False
        wait(20)
    attachment.hold()
    return True
```

### Verificación con sensor antes de actuar

```python
def pickup_only_if_present():
    if us_front.distance() < 80:
        close_gripper_with_timeout()
        return True
    else:
        hub.light.on(Color.YELLOW)  # marcador visual de skip
        return False
```

### Abort y volver al home

```python
def return_home():
    """Volver por la ruta más directa, ignorando obstáculos.
    Mejor perder 1 mission que perder TODAS por touch penalty."""
    drive.use_gyro(True)
    drive.settings(straight_speed=400)
    drive.turn(-hub.imu.heading())
    drive.straight(-2000)  # hasta golpear pared trasera
```

## Time management durante una ronda

Distribución óptima en 2 minutos:

| Tiempo | Acción |
|---|---|
| 0:00-0:10 | Posicionar robot, seleccionar programa #1, arrancar |
| 0:10-0:30 | Programa #1 corre |
| 0:30-0:40 | Cambio attachment, programa #2 |
| 0:40-1:00 | Programa #2 |
| 1:00-1:10 | Cambio |
| 1:10-1:30 | Programa #3 |
| 1:30-1:40 | Cambio |
| 1:40-1:55 | Programa #4 |
| 1:55-2:00 | Buffer |

**Cada programa tiene ~20 segundos máximo de tiempo útil**. CRÍTICO para diseñar las missions.

## Pre-round checklist programático

```python
def pre_round_check():
    errors = []
    if hub.battery.voltage() < 7500:
        errors.append('BAT')
    try:
        left_motor.angle()
        right_motor.angle()
        attachment.angle()
    except:
        errors.append('MOT')
    try:
        r = color_main.reflection()
        if r < 5 or r > 95:
            errors.append('COL')
    except:
        errors.append('COL')
    
    if errors:
        hub.light.on(Color.RED)
        hub.display.text(' '.join(errors))
        hub.speaker.beep(frequency=200, duration=500)
    else:
        hub.light.on(Color.GREEN)
        hub.speaker.beep(frequency=880, duration=100)
```

## Errores típicos en competición

| Error | Prevención |
|---|---|
| Programa equivocado subido | Selector con número visible en display |
| Attachment mal montado | Mounting con pins de tope, no encaje libre |
| Robot mal posicionado en home | Squareo automático contra pared al inicio |
| Mat distinto al de práctica | Recalibrar pre-ronda, dejar margen en distancias |
| Batería bajando | 2 baterías cargadas, swap entre rondas |
| Equipo toca robot fuera home | Practicar handoff en simulacros |
| Cable desconectado | Pre-checklist visual de cables |
| Sensor con suciedad | Limpiar lente con paño seco antes de cada ronda |

## Filosofía general

1. **Reliability > speed.** Un robot que completa 7 missions confiables puntúa más que uno que intenta 12 y completa 4.
2. **Test 10 veces antes de declarar que funciona.** Si funciona 8 de 10 veces, NO funciona.
3. **Documentar las calibraciones en el código.**
4. **Iterar el robot, no el código.** Si una mission requiere código heroico, el problema es el dise del attachment.
5. **Practicar handoffs.** Cambio + reposicionamiento + selección debe tomar <8 segundos.
6. **Llevar dos hubs cargados a competición.** Si uno falla, el segundo entra. Reflashear Pybricks toma 30 seg.

## Recursos

- Rulebook oficial WRO: https://wro-association.org/competition/wro-international/rulebook/
- Repos de WRO 2026 de IITA: `gviollaz/wro2026-robomission-junior`, `gviollaz/wro2026-robomission-elementary`
- Workshop Pybricks: `IITA-Proyectos/iita-vibe-robotics-martes-SL`
- Pybricks docs: https://docs.pybricks.com/
