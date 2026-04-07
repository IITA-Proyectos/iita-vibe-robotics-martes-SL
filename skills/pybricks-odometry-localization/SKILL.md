---
name: pybricks-odometry-localization
description: Odometría diferencial, dead reckoning y localización de pose (x, y, θ) en tiempo real para robots LEGO Spike Prime con Pybricks. Usar SIEMPRE que se trabaje con tracking de pose, posición global del robot, dead reckoning, kinematics diferencial, fusión de encoders y giroscopo, navegación por waypoints (x, y), conversión polar/cartesiana, world frame vs robot frame, o se mencione 'odometría', 'odometry', 'pose', 'world frame', 'kinematics', 'dead reckoning', 'localization', 'waypoint navigation', 'fusión sensorial', 'complementary filter'. NO usar para movimiento simple recto/giro (eso va en pybricks-precision-driving) ni para line following (pybricks-line-following).
---

# Odometría y localización con Pybricks

La odometría es el cálculo continuo de la posición y orientación del robot (`x`, `y`, `θ`) a partir de la información de los encoders de las ruedas y opcionalmente del giroscopo. Permite **navegación por waypoints en coordenadas globales** en lugar de la secuencia "avanzar X, girar Y, avanzar Z" de la programación clásica. Es la base de cualquier sistema de localización robótica seria.

## Por qué importa

Sin odometría, el robot solo sabe "estoy haciendo el comando #5". Con odometría, el robot sabe "estoy en (x=823 mm, y=412 mm, θ=87°) en el sistema de coordenadas del campo". Esto permite:

- Replanificación on-the-fly cuando una mission falla.
- Volver al home base desde cualquier punto.
- Saber si llegaste donde querías llegar.
- Detectar slip y compensar.
- Estrategias adaptativas en WRO Future Engineers, Sumo, Football.

## Modelo cinemático diferencial

Un robot diferencial con dos ruedas motrices, separadas por un `axle_track` `L`, donde cada rueda recorre `dL` y `dR` mm en un intervalo `dt`:

- **Distancia recorrida**: `d = (dL + dR) / 2`
- **Cambio de orientación**: `dθ = (dR - dL) / L` (radianes)
- **Cambio en x**: `dx = d * cos(θ + dθ/2)`
- **Cambio en y**: `dy = d * sin(θ + dθ/2)`

Donde `θ` es la orientación al inicio del intervalo. La fórmula con `θ + dθ/2` es el **midpoint method**, más preciso que usar `θ` directo cuando hay rotación durante el intervalo.

## Implementación pura con encoders

```python
from pybricks.tools import StopWatch
from umath import sin, cos, pi

class Odometry:
    def __init__(self, left_motor, right_motor, wheel_diameter_mm, axle_track_mm):
        self.left = left_motor
        self.right = right_motor
        self.wheel_radius = wheel_diameter_mm / 2.0
        self.axle = axle_track_mm
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0  # radianes
        self.last_left_deg = left_motor.angle()
        self.last_right_deg = right_motor.angle()
    
    def update(self):
        """Llamar en cada iteración del loop principal (~50-100 Hz)."""
        left_deg = self.left.angle()
        right_deg = self.right.angle()
        
        delta_left_deg = left_deg - self.last_left_deg
        delta_right_deg = right_deg - self.last_right_deg
        
        self.last_left_deg = left_deg
        self.last_right_deg = right_deg
        
        # mm recorridos por cada rueda
        dL = (delta_left_deg * pi / 180.0) * self.wheel_radius
        dR = (delta_right_deg * pi / 180.0) * self.wheel_radius
        
        # Distancia + cambio angular
        d = (dL + dR) / 2.0
        dtheta = (dR - dL) / self.axle
        
        # Midpoint method
        theta_mid = self.theta + dtheta / 2.0
        self.x += d * cos(theta_mid)
        self.y += d * sin(theta_mid)
        self.theta += dtheta
        
        # Normalizar a [-pi, pi]
        while self.theta > pi: self.theta -= 2*pi
        while self.theta < -pi: self.theta += 2*pi
    
    def reset(self, x=0.0, y=0.0, theta=0.0):
        self.x = x
        self.y = y
        self.theta = theta
        self.last_left_deg = self.left.angle()
        self.last_right_deg = self.right.angle()
    
    def pose(self):
        """Devuelve (x, y, theta_grados)."""
        return (self.x, self.y, self.theta * 180.0 / pi)
```

## Uso típico

```python
odo = Odometry(left_motor, right_motor, wheel_diameter_mm=56, axle_track_mm=114)

# Loop de movimiento con tracking
drive.drive(200, 0)  # adelante a 200 mm/s
sw = StopWatch()
sw.reset()
while sw.time() < 3000:
    odo.update()
    wait(20)  # 50 Hz
drive.stop()

x, y, theta_deg = odo.pose()
print('Pose:', x, y, theta_deg)
```

## Fusión con giroscopo — el upgrade clave

La odometría pura con encoders **acumula error** rápido por slip de ruedas. El giroscopo del IMU **no acumula slip pero tiene drift** lento. Combinarlos da lo mejor de ambos.

### Estrategia 1: gyro absoluto para `θ`, encoders solo para distancia

La más simple y robusta. Para Spike Prime es lo que recomendamos en la mayoría de los casos.

```python
class OdometryGyro:
    def __init__(self, hub, left_motor, right_motor, wheel_diameter_mm):
        self.hub = hub
        self.left = left_motor
        self.right = right_motor
        self.wheel_radius = wheel_diameter_mm / 2.0
        self.x = 0.0
        self.y = 0.0
        self.last_left_deg = left_motor.angle()
        self.last_right_deg = right_motor.angle()
    
    def update(self):
        left_deg = self.left.angle()
        right_deg = self.right.angle()
        delta_left_deg = left_deg - self.last_left_deg
        delta_right_deg = right_deg - self.last_right_deg
        self.last_left_deg = left_deg
        self.last_right_deg = right_deg
        
        dL = (delta_left_deg * pi / 180.0) * self.wheel_radius
        dR = (delta_right_deg * pi / 180.0) * self.wheel_radius
        d = (dL + dR) / 2.0
        
        # θ viene DIRECTO del gyro, no se calcula desde los encoders
        theta_rad = self.hub.imu.heading() * pi / 180.0
        
        self.x += d * cos(theta_rad)
        self.y += d * sin(theta_rad)
    
    def pose(self):
        return (self.x, self.y, self.hub.imu.heading())
```

**Ventaja**: el slip rotacional (la principal fuente de error en `θ` con odometría pura) desaparece completamente. El gyro siempre te da el yaw real.

### Estrategia 2: complementary filter

Combina la mejor parte de cada sensor. El gyro confiable a corto plazo (sin drift inmediato), encoders confiables a largo plazo (sin drift acumulativo si no hay slip).

```python
ALPHA = 0.98  # peso del gyro
def update_theta(self, dt):
    # Gyro: integración instantánea
    gyro_rate = self.hub.imu.angular_velocity(Axis.Z) * pi / 180.0
    theta_gyro = self.theta + gyro_rate * dt
    # Encoders: cálculo de θ por diferencia de ruedas
    theta_enc = self.theta + ((dR - dL) / self.axle)
    # Fusión
    self.theta = ALPHA * theta_gyro + (1 - ALPHA) * theta_enc
```

Para Spike Prime, **la Estrategia 1 es suficiente y más simple**. Solo justifica el complementary filter en robots con encoders muy precisos y mucha vibración.

## Navegación por waypoints

Una vez que tenés odometría confiable, podés navegar a coordenadas globales en lugar de comandos relativos:

```python
from umath import atan2, sqrt

def go_to_point(target_x, target_y, tolerance=20):
    """Navega al punto (target_x, target_y) en coordenadas globales."""
    while True:
        odo.update()
        x, y, theta_deg = odo.pose()
        
        dx = target_x - x
        dy = target_y - y
        distance = sqrt(dx*dx + dy*dy)
        
        if distance < tolerance:
            drive.stop()
            return
        
        # Heading deseado (radianes → grados)
        target_heading = atan2(dy, dx) * 180.0 / pi
        heading_error = target_heading - theta_deg
        while heading_error > 180: heading_error -= 360
        while heading_error < -180: heading_error += 360
        
        # Si el error de heading es grande, girar primero
        if abs(heading_error) > 15:
            drive.drive(0, heading_error * 3)  # solo rotar
        else:
            # Avanzar con corrección de heading proporcional
            drive.drive(200, heading_error * 5)
        
        wait(20)
```

## Detección de slip por discrepancia

Comparando lo que los encoders dicen contra lo que el gyro dice, podés detectar slip:

```python
def check_slip():
    # Theta esperado por encoders
    theta_enc = ((right_motor.angle() - left_motor.angle()) * pi / 180.0 * wheel_radius) / axle_track
    theta_enc_deg = theta_enc * 180.0 / pi
    
    # Theta real por gyro
    theta_gyro = hub.imu.heading()
    
    discrepancy = theta_enc_deg - theta_gyro
    if abs(discrepancy) > 10:
        hub.light.on(Color.RED)
        return True
    return False
```

Si hay slip detectado, **trustear el gyro y NO los encoders** para `θ`.

## Reset por landmarks (squareo)

La odometría siempre acumula error eventualmente. La solución es **resetear contra landmarks conocidos del campo**: paredes, líneas, marcas. Cada vez que el robot toca una pared en `x = 0`, podés resetear `odo.x = 0` con confianza absoluta.

```python
def square_and_reset_x_zero():
    """Squarea contra la pared izquierda del campo y resetea x = 0."""
    drive.use_gyro(False)
    left_motor.run(-80)
    right_motor.run(-80)
    wait(700)
    left_motor.stop()
    right_motor.stop()
    drive.use_gyro(True)
    
    odo.x = 0  # estamos contra la pared izquierda del campo
    # Aprovechar también para resetear theta
    hub.imu.reset_heading(0)
    odo.last_left_deg = left_motor.angle()
    odo.last_right_deg = right_motor.angle()
```

Esta combinación de **odometría + reset por landmarks** es lo que usan los robots ganadores de WRO y FLL: tienen pose tracking continuo, pero recalibran contra la geometría conocida del campo cada 30-60 segundos para que el error no explote.

## Errores comunes

| Síntoma | Causa | Solución |
|---|---|---|
| Pose se desvía rápido al rotar | Slip rotacional | Usar Estrategia 1 (gyro absoluto para θ) |
| Pose se desvía linealmente | `wheel_diameter` mal | Calibrar empíricamente |
| Pose explota después de squareo | `last_left_deg`/`last_right_deg` no se resetearon | Llamar `odo.reset()` después de squareo |
| `update()` no llamado seguido | Loop sin `wait` o `wait` muy largo | Garantizar 50+ Hz |
| Robot va al waypoint pero pasa de largo | Tolerancia muy baja o velocidad alta | Subir tolerance o bajar velocidad cerca del target |

## Cuándo NO usar odometría

- En programas single-shot muy cortos (<10 segundos) donde la secuencia rígida es más confiable.
- En categorías donde el campo no tiene coordenadas relevantes (Sumo).
- Cuando el robot se golpea/empuja con frecuencia (Football, Sumo) — el slip rompe la odometría irrecuperablemente.

## Recursos

- Probabilistic Robotics, Thrun et al. — el libro canónico de localización robótica.
- ROS REP 105 (Coordinate Frames) — convenciones de world frame y robot frame.
- Pybricks IMU API: https://docs.pybricks.com/en/latest/hubs/primehub.html#imu
