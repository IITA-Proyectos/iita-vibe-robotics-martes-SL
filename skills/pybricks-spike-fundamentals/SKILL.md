---
name: pybricks-spike-fundamentals
description: Fundamentos de Pybricks (MicroPython) sobre LEGO Spike Prime y Robot Inventor. Usar SIEMPRE que se trabaje con código Pybricks para LEGO Spike Prime/Inventor, programación en MicroPython para robots educativos LEGO, inicialización del PrimeHub, configuración de motores y sensores, estructura básica de un programa Pybricks, debugging con prints en el hub, multitarea con multitask/run_task, o se mencione 'Pybricks', 'Spike Prime', 'Robot Inventor', 'PrimeHub', 'pybricks.hubs', 'pybricks.pupdevices', 'pybricks.parameters', 'StopWatch'. NO usar para LEGO EV3 (es 'pybricks.ev3devices'), LEGO Education SPIKE App (Scratch-based, no Pybricks), Arduino, ni Raspberry Pi.
---

# Pybricks + LEGO Spike Prime — Fundamentos

Pybricks es un firmware MicroPython que reemplaza el firmware oficial de LEGO Education sobre el **Spike Prime hub** y el **Robot Inventor hub** (mismo hardware, distinto naming comercial). Da Python real, APIs limpias, mejor performance que el firmware oficial, y acceso a features que LEGO oculta (gyro raw, control PID custom, multitarea cooperativa).

**Docs oficiales:** https://docs.pybricks.com/ · **Editor web:** https://code.pybricks.com/

## Imports estándar

```python
from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor, UltrasonicSensor, ForceSensor
from pybricks.parameters import Port, Direction, Stop, Color, Button, Side, Axis
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch, multitask, run_task
```

No importar `umath`/`math` salvo necesidad real — Pybricks tiene helpers que cubren los casos comunes y ahorran memoria.

## PrimeHub — el hub

```python
hub = PrimeHub()
```

APIs clave:

- `hub.battery.voltage()` → mV. Chequear al inicio, alertar si <7800 mV.
- `hub.battery.current()` → mA.
- `hub.imu.heading()` → 0-360°, gyro yaw.
- `hub.imu.angular_velocity(Axis.Z)` → deg/s.
- `hub.imu.acceleration()` → mm/s² (X, Y, Z).
- `hub.imu.tilt()` → (pitch, roll) en grados.
- `hub.imu.up()` → cara hacia arriba (Side.TOP, FRONT, etc.).
- `hub.imu.reset_heading(0)` → resetea yaw a 0. **Llamar al inicio de cada run.**
- `hub.display.icon(matrix)` / `.number(42)` / `.text('OK')` / `.char('A')` → display 5×5.
- `hub.light.on(Color.GREEN)` → LED del botón central.
- `hub.speaker.beep(frequency=440, duration=200)`.
- `hub.buttons.pressed()` → set de Button.LEFT, RIGHT, CENTER, BLUETOOTH.

## Motores

```python
left_motor = Motor(Port.A, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.B, Direction.CLOCKWISE)
attachment = Motor(Port.C)
```

**`Direction`**: en un robot diferencial montado simétrico, uno de los dos motores va `COUNTERCLOCKWISE` para que `left.run(speed)` y `right.run(speed)` con el mismo signo muevan recto.

Métodos clave:

| Método | Uso |
|---|---|
| `motor.run(speed)` | gira a velocidad constante en deg/s, no bloqueante |
| `motor.run_time(speed, time, then=Stop.HOLD, wait=True)` | gira por `time` ms |
| `motor.run_angle(speed, angle, then=Stop.HOLD, wait=True)` | gira `angle` grados relativos |
| `motor.run_target(speed, target_angle, then=Stop.HOLD, wait=True)` | gira hasta posición absoluta |
| `motor.run_until_stalled(speed, then=Stop.HOLD, duty_limit=None)` | hasta tope mecánico — usar para attachments |
| `motor.stop()` / `.brake()` / `.hold()` | corta libre / frena pasivo / frena activo |
| `motor.angle()` | grados actuales |
| `motor.reset_angle(0)` | setea ángulo actual como cero |
| `motor.speed()` | deg/s actuales |
| `motor.dc(duty_percent)` | PWM directo, sin PID, fuerza bruta |

**`Stop` actions**: `Stop.COAST` (libre), `Stop.BRAKE` (pasivo), `Stop.HOLD` (activo).

## Sensores

### ColorSensor

```python
color = ColorSensor(Port.E)

color.color()                    # Color enum (RED, WHITE, ...) — clasificación discreta
color.reflection()               # 0-100, intensidad reflejada — USAR para line following
color.ambient()                  # 0-100, luz ambiente sin emisor
color.hsv()                      # HSV(h, s, v) — h en 0-360, s y v en 0-100
color.rgb()                      # (r, g, b) en 0-100
color.lights.on([100, 0, 0])    # encender los 3 LEDs del sensor
color.detectable_colors([Color.RED, Color.BLUE, Color.WHITE, Color.BLACK])
```

**Crítico**: para line following usar `reflection()` (analógico, alta resolución), NO `color()` (clasificación discreta que pierde info).

### UltrasonicSensor

```python
us = UltrasonicSensor(Port.D)
us.distance()        # mm, hasta ~2000 mm
us.lights.on([100, 100, 100, 100])
```

Detecta mal superficies inclinadas, materiales absorbentes, bordes finos. Para paredes rígidas y planas funciona excelente.

### ForceSensor

```python
force = ForceSensor(Port.F)
force.force()         # 0-10 N
force.distance()      # 0-8 mm de compresión
force.pressed(force=3)  # True si >= 3 N
force.touched()       # True si hay contacto
```

Uso típico: botón virtual de inicio (`while not force.pressed(): wait(10)`).

## Tiempo

```python
wait(500)  # bloquea 500 ms

sw = StopWatch()
sw.reset()
# ... acciones ...
elapsed = sw.time()  # ms desde reset
sw.pause(); sw.resume()
```

## Estructura típica de un programa

```python
# 1. Imports
from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

# 2. Hardware setup (al tope, una sola vez)
hub = PrimeHub()
left_motor = Motor(Port.A, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.B)
color_left = ColorSensor(Port.E)
color_right = ColorSensor(Port.F)
attachment = Motor(Port.C)

# 3. DriveBase con dimensiones medidas a mano
drive = DriveBase(left_motor, right_motor, wheel_diameter=56, axle_track=114)
drive.use_gyro(True)

# 4. Constantes calibradas
WHITE = 95
BLACK = 8
SETPOINT = (WHITE + BLACK) // 2

# 5. Helpers
def wait_for_button():
    while Button.CENTER not in hub.buttons.pressed():
        wait(10)
    while Button.CENTER in hub.buttons.pressed():
        wait(10)

def beep_ok():
    hub.light.on(Color.GREEN)
    hub.speaker.beep(frequency=880, duration=100)

# 6. Main
def main():
    hub.display.icon([
        [0, 100, 0, 100, 0],
        [100, 100, 100, 100, 100],
        [100, 100, 100, 100, 100],
        [0, 100, 100, 100, 0],
        [0, 0, 100, 0, 0],
    ])
    wait_for_button()
    hub.imu.reset_heading(0)
    # ... lógica del run ...
    beep_ok()

main()
```

## Multitarea cooperativa

```python
from pybricks.tools import multitask, run_task

async def drive_forward():
    await drive.straight(500)

async def raise_arm():
    await attachment.run_angle(500, 180)

async def main():
    await multitask(drive_forward(), raise_arm())  # paralelo

run_task(main())
```

**Cuándo usar**: cuando un movimiento del robot puede ocurrir en paralelo con un attachment (avanzar mientras se baja un brazo). **NO abusar**: la mayoría de los programas WRO son secuenciales y más fáciles de debuggear sin async.

## Errores comunes

| Síntoma | Causa | Solución |
|---|---|---|
| `ENODEV` / `OSError [Errno 19]` | Cable desconectado o tipo incorrecto | Verificar puerto y tipo de dispositivo |
| Robot no va recto con `straight()` | `use_gyro(True)` no llamado, o `Direction` mal | Verificar gyro activo y simetría motores |
| Programa termina inmediatamente | Faltó `wait=True` o falta llamar a `main()` | Revisar flujo del archivo |
| `print()` no aparece | Solo se ve por USB en el editor | Usar `hub.display.number()` o `text()` |
| Hub se resetea solo | Bater <7000 mV o loop sin `wait()` (watchdog) | Cargar bater + agregar `wait(10)` en loops |

## Pybricks vs LEGO Education SPIKE App

- **LEGO SPIKE App** (Scratch o Python limitado): para principiantes. Limita precisión y performance.
- **Pybricks**: MicroPython real, APIs limpias, mejor performance. **Para WRO, RoboCup, FLL y cualquier competición seria, Pybricks es muy superior.**
- Flashear: ir a https://code.pybricks.com, conectar el hub por USB, instalar firmware. **Reversible** — se puede volver al firmware oficial de LEGO.

## Recursos

- Docs oficiales: https://docs.pybricks.com/
- GitHub: https://github.com/pybricks/pybricks-micropython
- Foro: https://github.com/orgs/pybricks/discussions
- API reference: https://docs.pybricks.com/en/latest/hubs/primehub.html
