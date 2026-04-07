---
name: pybricks-precision-driving
description: Movimiento preciso y predecible con DriveBase de Pybricks sobre LEGO Spike Prime usando gyro-assisted driving, calibración empírica de geometría y squaring contra paredes. Usar SIEMPRE que se trabaje con DriveBase, drive.straight, drive.turn, drive.curve, drive.drive, calibración de wheel_diameter o axle_track, gyro heading, IMU del Prime Hub, squaring del robot, drift correction, slip detection, o se mencione 'precision driving', 'movimiento preciso', 'gyro straight', 'manejar drift', 'calibrar el robot', 'WRO RoboMission movement', 'FLL navigation', 'campo de juego'. NO usar para line following puro (eso va en pybricks-line-following) ni para odometría avanzada con tracking de pose (eso va en pybricks-odometry-localization).
---

# Movimiento preciso con DriveBase

La precisión del movimiento es lo más crítico en WRO RoboMission, FLL y cualquier ejercicio de robótica de competición. Un robot que recorre 1 metro recto con error de 5 cm pierde puntos. Esta skill cubre las técnicas que llevan el error a <1 cm en distancias de 2 metros.

## DriveBase — la abstracción central

```python
from pybricks.robotics import DriveBase

drive = DriveBase(
    left_motor=left_motor,
    right_motor=right_motor,
    wheel_diameter=56,    # mm — MEDIR EMPÍRICAMENTE
    axle_track=114,       # mm — distancia entre centros de rueda, MEDIR EMPÍRICAMENTE
)

drive.use_gyro(True)  # CRÍTICO: activa corrección de drift con IMU
```

## Métodos del DriveBase

```python
drive.straight(distance)               # mm, +adelante / -atrás
drive.straight(500, then=Stop.HOLD)
drive.turn(angle)                      # grados, +CW / -CCW
drive.curve(radius, angle)             # radio en mm, ángulo en grados
drive.drive(speed, turn_rate)          # continuo no bloqueante: mm/s + deg/s
drive.stop()                           # libre
drive.brake()                          # frenado pasivo
drive.distance()                       # mm acumulados desde último reset
drive.angle()                          # grados acumulados de rotación
drive.reset()                          # distance + angle = 0
drive.state()                          # (distance, drive_speed, angle, turn_rate)
```

## Settings — clave de la repetibilidad

```python
drive.settings(
    straight_speed=200,             # mm/s
    straight_acceleration=400,      # mm/s²
    turn_rate=180,                  # deg/s
    turn_acceleration=360,          # deg/s²
)
```

**Regla de oro**: a mayor velocidad, menor precisión. Baseline para WRO:

- `straight_speed=200` (lento confiable). Subir a 400-500 solo en tramos largos sin missions cerca.
- `turn_rate=120` para giros precisos. Subir a 200-300 solo si el giro está calibrado y validado.
- **Aceleraciones nunca >2× la velocidad** para evitar slip en arranque.

## Calibración de geometría — el paso CRÍTICO

Los valores nominales de LEGO **NO son exactos**. Hay variación por desgaste del rubber, compresión bajo el peso del robot, slack en los ejes, diferencias entre lotes. **Medir empíricamente** una vez por robot.

### `wheel_diameter`

```python
# Programa de calibración
drive = DriveBase(left_motor, right_motor, wheel_diameter=56, axle_track=114)
drive.use_gyro(True)
drive.straight(1000)

# Medir con cinta métrica. Supongamos midió 985 mm.
# Nuevo wheel_diameter = 56 * (1000 / 985) = 56.85
# Reemplazar y volver a probar hasta que straight(1000) recorra exactamente 1000 mm.
```

Repetir 3 veces y promediar. Si los 3 difieren mucho, hay otro problema (slip, batería, mecánica suelta).

### `axle_track`

```python
# Programa de calibración
hub.imu.reset_heading(0)
drive.turn(360)
wait(500)
error = hub.imu.heading()
print('Error:', error, 'grados')
# Si gyro reporta -5° (faltó 5°), AUMENTAR axle_track ~1-3 mm.
# Si gyro reporta +5° (se pasó 5°), DISMINUIR axle_track ~1-3 mm.
# Repetir hasta error <1°.
```

## Gyro-assisted driving — el game changer

Con `drive.use_gyro(True)`, DriveBase usa el IMU para detectar y corregir drift en tiempo real durante `straight()`. **Esto diferencia un robot que va recto 30 cm de uno que va recto 3 metros.**

Verificaciones importantes:

1. **Hub montado plano** (eje Z perpendicular al piso). Si está inclinado, gyro yaw mide mal.
2. **No mover el hub durante los primeros 2 segundos** post power-on (calibración del IMU).
3. **Resetear heading al inicio de cada run**: `hub.imu.reset_heading(0)`.
4. **Drift acumulativo** ~1°/min en hubs nuevos, hasta 5°/min en viejos. Para runs >2 min, agregar correcciones por squaring contra paredes.

## Patrón: ir recto manteniendo heading absoluto

```python
def straight_corrected(target_distance, target_heading=0, speed=200):
    """Va recto manteniendo heading absoluto (no relativo al robot)."""
    drive.settings(straight_speed=speed)
    error = hub.imu.heading() - target_heading
    if abs(error) > 1:
        drive.turn(-error)
    drive.straight(target_distance)
```

## Patrón: girar a heading absoluto

```python
def turn_to_heading(target_heading, speed=120):
    """Más predecible que turn(angle) porque no acumula error."""
    current = hub.imu.heading()
    delta = target_heading - current
    while delta > 180: delta -= 360
    while delta < -180: delta += 360
    drive.settings(turn_rate=speed)
    drive.turn(delta)
```

## Squaring contra paredes

Las paredes del campo son rígidas y rectas. **Apoyar el robot contra una pared resetea su heading** y elimina todo drift acumulado. Es la técnica de **localización por contacto** más confiable.

```python
def square_against_wall(speed=80, hold_time=700):
    """Avanza contra pared, deja que las dos ruedas se topen y se alineen."""
    drive.use_gyro(False)  # CRÍTICO: durante squaring NO querés corrección de gyro
    left_motor.run(speed)
    right_motor.run(speed)
    wait(hold_time)
    left_motor.stop()
    right_motor.stop()
    drive.use_gyro(True)
    hub.imu.reset_heading(0)
    drive.reset()
```

**Cuándo squarear**:
- Inicio de run para tener heading base confiable.
- Después de un giro grande con error acumulado.
- Antes de mission crítica que requiera precisión absoluta.

**Cuándo NO**:
- Pared sucia o con relieves.
- Robot en zona de scoring (puede tirar piezas).
- Batería baja (motores no vencen fricción).

## Detección de slip

Slip = ruedas patinan, motores reportan más distancia/giro de la real. Pasa con aceleraciones agresivas, pisos pulidos, robots pesados.

```python
hub.imu.reset_heading(0)
drive.turn(90)
gyro_change = hub.imu.heading()
if abs(gyro_change - 90) > 5:
    print('SLIP:', 90 - gyro_change, 'grados perdidos')
```

Mitigación:
- Bajar `turn_acceleration` y `straight_acceleration` a la mitad.
- Agregar peso sobre las ruedas motrices.
- Cambiar a ruedas con grip (negras con relieve > grises lisas).

## Voltaje de batería y reproducibilidad

```python
voltage = hub.battery.voltage()
if voltage < 7800:
    hub.light.on(Color.RED)
    hub.speaker.beep(frequency=200, duration=500)
    hub.display.text('BAT')
```

**El voltaje afecta DIRECTAMENTE a los motores**. Un robot calibrado con batería al 95% se comporta distinto al 50%.

- **Calibrar siempre con bater al 90-100%.**
- **Cargar entre rondas si baja del 80%.**
- **No usar baterías genéricas** — la marca LEGO mantiene voltaje estable.

## Reset completo entre runs

```python
def reset_for_run():
    drive.stop()
    drive.reset()
    left_motor.reset_angle(0)
    right_motor.reset_angle(0)
    hub.imu.reset_heading(0)
    wait(100)  # IMU se estabiliza
```

## Errores típicos

| Síntoma | Causa probable | Solución |
|---|---|---|
| Robot torcido pese a `use_gyro(True)` | Hub no plano o `Direction` mal | Verificar nivel del hub y simetría motores |
| `straight(1000)` recorre 950 mm | `wheel_diameter` mal | Aumentar proporcionalmente |
| `turn(90)` gira 85° | `axle_track` mal | Disminuir |
| Giros repetibles, rectos no | Slip en arranque | Bajar `straight_acceleration` |
| Vuelve a posición inicial pero rotado X° | Drift gyro durante run | Squarear a mitad de run |
| `straight(100)` ok, `straight(1000)` falla | Gyro desactivado | Verificar `drive.use_gyro(True)` |
| Acelera y rebota | Aceleración default alta | `straight_acceleration=200` |

## Ejemplo: navegación entre 3 waypoints

```python
def navigate_three_points():
    reset_for_run()
    drive.settings(straight_speed=300, turn_rate=150)
    drive.straight(500)        # WP1
    drive.turn(90)
    drive.straight(300)        # WP2
    drive.turn(-45)
    drive.straight(200)        # WP3
    final_heading = hub.imu.heading()
    expected = 45  # 90 - 45
    error = final_heading - expected
    if abs(error) > 3:
        hub.light.on(Color.RED)
        hub.display.text('ERR ' + str(int(error)))
    else:
        hub.light.on(Color.GREEN)
```
