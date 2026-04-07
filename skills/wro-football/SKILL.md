---
name: wro-football
description: Estrategia y patrones para WRO Football (RoboSport Football) — competición de fútbol robótico 2v2 con bola IR pulsada. Usar SIEMPRE que se trabaje en WRO Football, fútbol robótico, soccer robot, IR ball tracking, sensor de bola infrarroja, posicionamiento en cancha de fútbol robótico, roles goalkeeper/striker, omnidirectional drive, kicker mechanism, o se mencione 'WRO Football', 'soccer', 'IR ball', 'pulsed infrared', 'goalkeeper', 'striker', 'kicker', 'goal post', 'penalty area', 'RCJ Soccer'. La bola es siempre IR pulsada según estándar RoboCup. NO usar para WRO RoboMission (eso va en wro-robomission-strategy) ni Future Engineers.
---

# WRO Football / RoboCup Junior Soccer

WRO Football (también conocido como RoboSport Football) y **RoboCup Junior Soccer** son la categoría de fútbol robótico para equipos juveniles. Dos robots autónomos por equipo (2v2) compiten sobre una cancha verde rectangular con una **pelota infrarroja pulsada**, dos arcos coloreados, y los robots deben marcar goles. Esta skill cubre la estrategia, la detección de pelota, y los patrones tácticos.

**RoboCup Junior Soccer** es la versión históricamente más establecida y muchos de sus principios se aplican directamente a WRO Football.

## La pelota IR — el corazón del juego

Las competiciones de soccer robotico estandarizan la pelota como una **bola infrarroja pulsada** que emite IR a una **frecuencia específica** (típicamente 833 Hz para RCJ, ver el rulebook del año). Los robots usan **sensores IR direccionales** que detectan la pelota a 360° alrededor del robot.

**Las dos pelotas estándar son**:
- **Pasivas** (reflectivas): el robot tiene que iluminar la zona y detectar el reflejo. Más difícil.
- **Activas** (emisoras): la pelota tiene un emisor IR pulsado interno. Más fácil de detectar.

Las competiciones internacionales han pasado a usar **pelotas pasivas** para hacerlo más realista, pero a nivel escolar/regional la pelota activa sigue siendo común.

## Setup hardware típico

| Componente | Función |
|---|---|
| **Spike Prime hub** o controller equivalente | Cerebro |
| **Anillo de sensores IR** (8-16 sensores alrededor del robot) | Detección direccional de la pelota |
| **Brújula digital** o IMU con yaw confiable | Orientación absoluta |
| **2-4 motores de tracción** | Diferencial o omnidireccional (mejor para soccer) |
| **Sensor de color** apuntando al piso | Detectar las líneas de la cancha (white border, penalty area) |
| **Kicker mechanism** (opcional) | Solenoide o motor para patear con fuerza |
| **Sensor ultrasónico** o ToF lateral/frontal | Detectar paredes y oponentes |

**Spike Prime no tiene anillo de sensores IR oficial.** Para WRO Football con Spike, hay que usar:
- Sensores IR custom (ej: TSSP4038, módulos de Pololu IR sensor array).
- Un microcontrolador intermedio (Arduino Nano, Raspberry Pi Pico) que lea los IR y publique al hub vía un protocolo simple (digital lines o I2C).
- O usar hardware no-LEGO con compatibilidad similar.

## Decisión estratégica fundamental: omni vs diferencial

### Diferencial (2 ruedas + free wheels)

- **Pros**: simple, barato, encaja con LEGO standard.
- **Contras**: no puede moverse lateralmente sin girar. Tiene que rotar todo el robot para cambiar dirección.

### Omnidireccional (3 o 4 ruedas omnidireccionales)

- **Pros**: puede moverse en cualquier dirección sin rotar. **Crítico para soccer porque la pelota se mueve y vos tenés que reposicionarte rápido sin perder el frente apuntando al arco.**
- **Contras**: 3-4 motores necesarios, más complejo, ruedas omni difíciles de conseguir en LEGO.

**Para soccer competitivo, el setup ganador es omnidireccional.** Para escuela / iniciación, diferencial está bien.

## Detección direccional de la pelota

El anillo de sensores IR devuelve un array donde cada sensor reporta la intensidad recibida. Por ejemplo con 12 sensores:

```python
def read_ir_ring():
    """Lee los 12 sensores y devuelve [intensidades_0_a_11]."""
    return [ir_sensors[i].value() for i in range(12)]
```

Para encontrar la **dirección de la pelota** se calcula el centroide angular:

```python
from umath import sin, cos, atan2, pi

def ball_direction(ir_values):
    """Devuelve (angle_rad, magnitude). Angle 0 = enfrente, +CCW."""
    n = len(ir_values)
    sum_x = 0.0
    sum_y = 0.0
    for i, v in enumerate(ir_values):
        angle = (i / n) * 2 * pi
        sum_x += v * cos(angle)
        sum_y += v * sin(angle)
    
    magnitude = (sum_x*sum_x + sum_y*sum_y) ** 0.5
    if magnitude < 5:  # threshold de ruido
        return None  # no se detecta pelota
    
    angle = atan2(sum_y, sum_x)
    return (angle, magnitude)
```

`magnitude` te da una idea de qué tan **cerca** está la pelota — más alto = más cerca.

## Patrón básico del striker

```python
def striker_loop():
    while True:
        result = ball_direction(read_ir_ring())
        
        if result is None:
            # Sin pelota visible — buscar
            search_for_ball()
            continue
        
        angle_rad, distance = result
        angle_deg = angle_rad * 180.0 / pi
        
        # 1. Si la pelota está enfrente y cerca → patear
        if abs(angle_deg) < 15 and distance > 50:
            kick_and_advance()
            continue
        
        # 2. Si la pelota está enfrente lejos → avanzar derecho
        if abs(angle_deg) < 30:
            move_toward_goal_with_ball()
            continue
        
        # 3. Pelota a un lado → rotar para encararla
        rotate_toward_ball(angle_deg)
        
        wait(20)
```

## El "ball follower naive" no funciona

**Error clásico de equipos principiantes**: hacer que el robot siga la pelota directamente.

```python
# ❌ MAL — robot va directo a la pelota
def naive_follow():
    angle, _ = ball_direction(read_ir_ring())
    move_in_direction(angle)
```

**Por qué no funciona**: la pelota está sobre el piso. Si el robot va directo a ella, se topa con ella **por el costado equivocado** y patea la pelota lejos del arco rival, no hacia él. O peor, hacia el propio arco (autogol).

## El patrón correcto: ball-aware positioning

El robot debe **posicionarse DETRÁS de la pelota** (relativo al arco rival) y EMPUJARLA hacia adelante. Esto se llama **"approach from behind"** o **"ball capture path"**.

```python
def approach_ball_from_behind():
    """Calcula un punto detrás de la pelota (entre el propio arco y la pelota)
    y se dirige a ese punto. Después, una vez detrás, empuja la pelota."""
    
    ball_x, ball_y = estimate_ball_position()  # de odometría + IR
    own_goal_x, own_goal_y = OWN_GOAL_CENTER
    
    # Vector del propio arco a la pelota
    dx = ball_x - own_goal_x
    dy = ball_y - own_goal_y
    norm = (dx*dx + dy*dy) ** 0.5
    
    # Punto a 200 mm detrás de la pelota (en dirección del propio arco)
    behind_x = ball_x - 200 * dx / norm
    behind_y = ball_y - 200 * dy / norm
    
    if not at_position(behind_x, behind_y, tolerance=50):
        navigate_to(behind_x, behind_y)
    else:
        # Estamos detrás de la pelota, empujar hacia el arco rival
        push_ball_toward_enemy_goal()
```

## Goalkeeper

El portero tiene una lógica más simple pero muy distinta:

1. **Quedarse cerca del propio arco**.
2. **Moverse lateralmente** para interponer su cuerpo entre la pelota y el arco.
3. **Solo atacar si la pelota viene clara** y nadie del propio equipo está más cerca.

```python
def goalkeeper_loop():
    while True:
        result = ball_direction(read_ir_ring())
        
        # Mantenerse en el penalty area
        my_x, my_y, _ = odo.pose()
        if my_y < GOAL_DEFENSE_LINE:
            move_to(GOAL_X, GOAL_DEFENSE_LINE)
            continue
        
        if result is None:
            # Sin pelota — quedarse en el centro del arco
            move_to(GOAL_X, GOAL_DEFENSE_LINE)
            continue
        
        angle_rad, magnitude = result
        ball_x_estimated = estimate_ball_x_from_angle(angle_rad)
        
        # Moverse lateralmente para tapar el ángulo de tiro
        target_x = clamp(ball_x_estimated, GOAL_LEFT, GOAL_RIGHT)
        move_to(target_x, GOAL_DEFENSE_LINE)
        
        # Si la pelota está MUY cerca → salir a despejar
        if magnitude > 100 and abs(angle_rad) < pi/4:
            sortie_and_clear()
        
        wait(20)
```

## Detección de la línea blanca del borde

La cancha de soccer tiene un **borde blanco** alrededor del campo verde. Si el robot detecta blanco bajo sus ruedas, **está saliendo de la cancha** y debe retroceder inmediatamente.

```python
color_ground = ColorSensor(Port.E)

def check_out_of_bounds():
    if color_ground.reflection() > 70:  # blanco
        drive.straight(-100)  # retroceder
        return True
    return False

# En el loop principal
if check_out_of_bounds():
    continue  # skipear comportamiento normal este tick
```

## Mecanismo de pateo (kicker)

Hay 3 enfoques:

1. **Empuje frontal**: el robot tiene una pala/cuchara fija al frente. La fuerza viene del motor de tracción. Simple pero débil.
2. **Solenoide eléctrico**: un solenoide se dispara al detectar la pelota cerca del frente. Fuerza grande pero requiere componente no-LEGO.
3. **Motor con resorte**: un motor LEGO comprime un resorte y lo libera para patear. Fuerza media, todo LEGO.

```python
kicker_motor = Motor(Port.D)

def kick():
    """Liberar el resorte cargado."""
    kicker_motor.run_angle(speed=1000, angle=90)  # liberar
    wait(100)
    # Recargar
    kicker_motor.run_angle(speed=300, angle=-90)
```

## Estrategia de equipo 2v2

Con dos robots, **uno es striker y otro es goalkeeper** (o ambos atacan en formación 2-0 con uno más adelantado). NO ambos atacan la pelota al mismo tiempo: chocan entre sí y dejan el arco descubierto.

Comunicación entre robots: **WRO no permite comunicación directa entre robots durante el partido**, así que los roles tienen que ser **estáticos** (uno SIEMPRE es portero, otro SIEMPRE es delantero) o **basados en posición** (el más cercano a la pelota ataca, el otro defiende — esto se determina por la posición percibida con sensores).

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| Robot autogol | Empuja la pelota desde el lado equivocado | Implementar approach-from-behind |
| Pierde la pelota constantemente | Sensor IR de baja calidad | Cambiar a TSSP4038 o módulo Pololu |
| Robots del mismo equipo chocan | Sin diferenciación de roles | Asignar roles fijos |
| Sale de la cancha | Sin detección de borde blanco | ColorSensor mirando al piso, retroceder si > 70 |
| Pelota lateral nunca detectada | Anillo IR con cobertura insuficiente | Mínimo 8 sensores, idealmente 12-16 |

## Recursos

- RoboCup Junior Soccer rulebook: https://junior.robocup.org/rcjsoccer/
- WRO RoboSport rulebook: https://wro-association.org/competition/wro-international/rulebook/
- IR ball spec (RCJ): https://junior.robocup.org/wp-content/uploads/2024/01/RCJSoccer_Rules_2024.pdf
- Sensor IR Pololu: https://www.pololu.com/category/154/ir-sensors
- Repos referencia: buscar "robocup junior soccer" en GitHub.
