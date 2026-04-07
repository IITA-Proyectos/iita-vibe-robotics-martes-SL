---
name: robotics-control-theory
description: Teoría de control aplicada a robótica de competición — PID tuning con métodos sistemáticos (Ziegler-Nichols), feedforward, filtros de señal (low-pass, complementary, Kalman simple), state machines y máquinas de estado finito para comportamiento robótico. Usar SIEMPRE que se trabaje en sintonización profesional de PID, métodos formales de tuning, fusión de sensores con filtros, diseño de state machines para robots, control teorico aplicado, o se mencione 'PID tuning', 'Ziegler-Nichols', 'feedforward', 'low-pass filter', 'complementary filter', 'Kalman', 'state machine', 'FSM', 'control loop', 'transfer function'. Aplica a Pybricks, Arduino, ROS, Raspberry Pi y cualquier plataforma de robótica. Es transversal a todas las categorías de competición.
---

# Teoría de control aplicada a robots de competición

Esta skill cubre los conceptos de control y procesamiento de señales que separan a un robot que "anda más o menos" de uno que es preciso, repetible y profesional. Es teoría aplicada — no se entra en matemáticas profundas, sino en cómo usar las técnicas en código real de Pybricks (o cualquier plataforma).

## PID — el controlador universal

PID = **P**roporcional + **I**ntegral + **D**erivativo. Es el controlador más usado en robótica porque es simple, predecible, y funciona bien para casi todo.

### Anatomía del PID

```python
class PIDController:
    def __init__(self, kp, ki, kd, setpoint=0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.setpoint = setpoint
        self.integral = 0
        self.last_error = 0
        self.integral_max = 1000  # anti-windup
    
    def update(self, measurement, dt):
        error = self.setpoint - measurement
        self.integral += error * dt
        # Anti-windup
        if self.integral > self.integral_max:
            self.integral = self.integral_max
        elif self.integral < -self.integral_max:
            self.integral = -self.integral_max
        derivative = (error - self.last_error) / dt
        self.last_error = error
        output = self.kp * error + self.ki * self.integral + self.kd * derivative
        return output
    
    def reset(self):
        self.integral = 0
        self.last_error = 0
```

### Qué hace cada término

| Término | Función | Cuándo usar |
|---|---|---|
| **P** (Proporcional) | Reacciona al error actual. Más error = más corrección. | SIEMPRE. Es el caballo de batalla. |
| **I** (Integral) | Acumula el error histórico. Corrige error sistemático constante (ej: una rueda más lenta). | Solo cuando hay un error que P solo no puede eliminar. **Causa overshoot** si está mal usado. |
| **D** (Derivativo) | Reacciona al cambio del error. Anticipa. | Cuando P solo causa oscilación. **Es ruidoso** si hay ruido en la medición. |

**Regla práctica para line following**: empezar con P solo, agregar D si oscila, dejar I en 0.

**Regla práctica para straight gyro**: P + D, sin I.

**Regla práctica para velocidad de motor**: PI, sin D (los encoders son ruidosos).

## Sintonización con Ziegler-Nichols

Método sistemático para encontrar Kp, Ki, Kd. Hay dos versiones, la **closed-loop** es la más práctica para robots.

### Método closed-loop Ziegler-Nichols

1. **Setear Ki = 0, Kd = 0.**
2. **Subir Kp gradualmente** hasta que el sistema **oscile sostenidamente** (ni se acelera ni se amortigua). A ese Kp se le llama **Ku** (ultimate gain).
3. **Medir el período de oscilación Tu** en segundos.
4. **Aplicar las fórmulas**:

| Tipo de controlador | Kp | Ki | Kd |
|---|---|---|---|
| P | 0.5 × Ku | 0 | 0 |
| PI | 0.45 × Ku | 1.2 × Kp / Tu | 0 |
| PID clásico | 0.6 × Ku | 2 × Kp / Tu | Kp × Tu / 8 |
| PID Pessen | 0.7 × Ku | 2.5 × Kp / Tu | 0.15 × Kp × Tu |
| PID poco overshoot | 0.33 × Ku | 2 × Kp / Tu | Kp × Tu / 3 |
| PID sin overshoot | 0.2 × Ku | 2 × Kp / Tu | Kp × Tu / 3 |

**Para robots de competición**, "PID sin overshoot" o "PID poco overshoot" suelen ser los más estables. Los robots no necesitan respuesta rápida — necesitan respuesta **predecible**.

### Ejemplo aplicado a line following

```python
# Paso 1: Ki=0, Kd=0
KP = 0.1
KI = 0
KD = 0

# Probar. Subir Kp gradualmente. Observar.
# A Kp = 1.5 el robot oscila visiblemente con período de ~0.4 seg.
# Entonces Ku = 1.5, Tu = 0.4

# Aplicar fórmula "PID poco overshoot":
KU = 1.5
TU = 0.4
KP = 0.33 * KU         # = 0.5
KI = 2 * KP / TU       # = 2.5  → en line following dejarlo en 0
KD = KP * TU / 3       # = 0.067

# En line following el integral causa más mal que bien, así que:
KI = 0
```

## Feedforward — el upgrade que la mayoría no usa

PID es **reactivo**: solo corrige errores que YA pasaron. **Feedforward** es **proactivo**: predice qué corrección hace falta antes de que el error aparezca.

### Cuándo usar feedforward

- En curvas de radio conocido: vos sabés que para una curva de radio R necesitás una velocidad angular de v/R. **Setealá directamente** sin esperar a que el PID lo descubra.
- Compensación de gravedad: si tenés un brazo que cae por su propio peso, agregar un offset al motor que compense la gravedad (no esperar a que el PID lo "descubra").

```python
def follow_curve_with_feedforward(radius, distance):
    """Sigue una curva usando feedforward + corrección PID."""
    # Feedforward: la velocidad angular ideal para esta curva
    base_speed = 200  # mm/s
    feedforward_turn_rate = base_speed / radius * 180 / pi  # deg/s
    
    drive.reset()
    while drive.distance() < distance:
        # PID solo para correcciones residuales (drift)
        gyro_error = (target_heading - hub.imu.heading())
        correction = KP * gyro_error
        
        drive.drive(base_speed, feedforward_turn_rate + correction)
        wait(20)
```

## Filtros de señal

Las mediciones de sensores tienen **ruido**. Filtrar el ruido antes de pasarlo al PID hace toda la diferencia.

### Low-pass filter (EWMA — Exponentially Weighted Moving Average)

El más simple y útil. Suaviza valores ruidosos.

```python
class LowPassFilter:
    def __init__(self, alpha=0.3):
        self.alpha = alpha  # 0-1, más bajo = más suavizado
        self.value = None
    
    def update(self, new_value):
        if self.value is None:
            self.value = new_value
        else:
            self.value = self.alpha * new_value + (1 - self.alpha) * self.value
        return self.value
```

**Uso típico**:

```python
sensor_filter = LowPassFilter(alpha=0.3)

while True:
    raw = sensor.reflection()
    smooth = sensor_filter.update(raw)
    error = smooth - SETPOINT
    drive.drive(BASE_SPEED, KP * error)
    wait(20)
```

**Ajuste de alpha**:
- `alpha = 0.5` → suavizado leve, respuesta rápida.
- `alpha = 0.2` → suavizado fuerte, respuesta lenta.
- `alpha = 0.05` → muy suavizado, casi insensible a cambios rápidos.

### Median filter

Mejor que el low-pass cuando el ruido tiene **outliers** (valores muy distintos al resto). Devuelve el valor mediano de los últimos N valores.

```python
class MedianFilter:
    def __init__(self, window=5):
        self.window = window
        self.values = []
    
    def update(self, new_value):
        self.values.append(new_value)
        if len(self.values) > self.window:
            self.values.pop(0)
        return sorted(self.values)[len(self.values) // 2]
```

**Uso típico**: filtrar UltrasonicSensor que devuelve picos espurios cuando detecta ruido.

### Complementary filter

Combina dos sensores que miden lo mismo pero con perfiles de error opuestos (uno preciso a corto plazo, otro a largo plazo). Clásico: gyro + acelerómetro para tilt.

```python
class ComplementaryFilter:
    def __init__(self, alpha=0.98):
        self.alpha = alpha  # peso del gyro
        self.angle = 0
    
    def update(self, gyro_rate, accel_angle, dt):
        self.angle = self.alpha * (self.angle + gyro_rate * dt) + (1 - self.alpha) * accel_angle
        return self.angle
```

**Donde el gyro es preciso a corto plazo (no sufre vibración) y el accel es preciso a largo plazo (no tiene drift)**, alpha ≈ 0.98 da un buen equilibrio.

### Kalman filter (versión simple 1D)

Para una variable única, el Kalman 1D es solo:

```python
class Kalman1D:
    def __init__(self, process_variance=0.01, measurement_variance=1.0):
        self.q = process_variance       # incertidumbre del modelo
        self.r = measurement_variance   # incertidumbre del sensor
        self.x = 0                      # estimación
        self.p = 1                      # incertidumbre de la estimación
    
    def update(self, measurement):
        # Predicción
        self.p += self.q
        # Corrección
        k = self.p / (self.p + self.r)
        self.x += k * (measurement - self.x)
        self.p *= (1 - k)
        return self.x
```

**Cuándo usar**: cuando tenés un sensor ruidoso pero querés estimar la variable real con teoría sólida. Para la mayoría de los robots de competición educativa, **el low-pass filter es suficiente** y el Kalman es overkill.

## State machines (FSM) — comportamiento estructurado

Un robot complejo tiene **estados**: "buscando", "atacando", "huyendo", "esquivando borde". Una **State Machine** organiza el código de forma que cada estado sabe qué hacer y cómo transicionar al siguiente.

```python
class State:
    SEARCHING = 0
    APPROACHING = 1
    ATTACKING = 2
    AVOIDING_EDGE = 3

class SumoFSM:
    def __init__(self):
        self.state = State.SEARCHING
    
    def transition_to(self, new_state):
        print('Transition:', self.state, '->', new_state)
        self.state = new_state
    
    def update(self):
        # PRIORIDAD MÁXIMA — interrupciones globales
        if at_edge():
            if self.state != State.AVOIDING_EDGE:
                self.transition_to(State.AVOIDING_EDGE)
        
        # Lógica por estado
        if self.state == State.SEARCHING:
            self.handle_searching()
        elif self.state == State.APPROACHING:
            self.handle_approaching()
        elif self.state == State.ATTACKING:
            self.handle_attacking()
        elif self.state == State.AVOIDING_EDGE:
            self.handle_avoiding_edge()
    
    def handle_searching(self):
        drive.drive(0, 150)  # rotar buscando
        if opponent_visible():
            self.transition_to(State.APPROACHING)
    
    def handle_approaching(self):
        direction = opponent_direction()
        drive.drive(300, direction * 5)
        if opponent_distance() < 200:
            self.transition_to(State.ATTACKING)
        elif not opponent_visible():
            self.transition_to(State.SEARCHING)
    
    def handle_attacking(self):
        drive.drive(800, 0)  # max speed forward
        if not opponent_visible():
            self.transition_to(State.SEARCHING)
    
    def handle_avoiding_edge(self):
        edge = at_edge()
        if edge == 'both':
            drive.straight(-200)
            drive.turn(180)
        elif edge == 'left':
            drive.straight(-100)
            drive.turn(45)
        elif edge == 'right':
            drive.straight(-100)
            drive.turn(-45)
        self.transition_to(State.SEARCHING)
```

**Ventajas**:
- Cada estado es fácil de testear individualmente.
- Las transiciones están explícitas, fácil debug.
- Agregar un estado nuevo no rompe los existentes.
- Logging de transiciones da un trace claro de qué pasó en una corrida.

## Errores típicos en control

| Síntoma | Causa | Solución |
|---|---|---|
| PID oscila | Kp muy alto, sin Kd | Bajar Kp o agregar Kd |
| PID responde lento | Kp muy bajo | Subir Kp |
| Error residual constante | No hay I | Agregar I pequeño con anti-windup |
| Sobrecorrige y rebota | I sin anti-windup | Agregar `max(min(integral, MAX), -MAX)` |
| Filtro low-pass muy lento | alpha muy bajo | Subir alpha |
| Filtro low-pass no suaviza | alpha muy alto | Bajar alpha |
| State machine tiene bugs raros | Transiciones acopladas | Centralizar transiciones en un solo método |
| Kp óptimo cambia con velocidad | PID no es lineal con velocidad | Tuning separado por rango de velocidad o ganancias dependientes de velocidad |

## Recursos

- "Feedback Systems" (Astrom & Murray) — libro libre online sobre control: https://fbsbook.org/
- "Probabilistic Robotics" (Thrun, Burgard, Fox) — el libro canónico de filtros y localización.
- Pybricks PID internals: https://docs.pybricks.com/en/latest/parameters/control.html
- "PID Without a PhD" (Tim Wescott) — paper práctico clásico.
