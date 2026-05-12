---
name: pybricks-ev3-fundamentals
description: API de Pybricks específica para LEGO MINDSTORMS EV3 (no Spike Prime, no NXT). Cubre EV3Brick, módulo pybricks.ev3devices, Motor, ColorSensor, GyroSensor, InfraredSensor, UltrasonicSensor, TouchSensor de EV3, puertos 1-4 y A-D, MicroPython embebido y diferencias clave contra Pybricks Spike. Usar SIEMPRE que se trabaje con robots EV3, ev3dev, Pybricks for EV3, programación EV3 con Python, o se mencione 'EV3Brick', 'Port.S1', 'Port.A', 'ev3devices', 'brick.py', 'ev3dev-stretch', 'ev3-pybricks', 'EV3', 'LargeMotor', 'MediumMotor', 'ColorSensor EV3', 'GyroSensor EV3'. NO usar para Spike Prime (eso es pybricks-spike-fundamentals) ni para sensores Mindsensors I2C (eso es mindsensors-ev3-sensors).
---

# Pybricks para EV3 — fundamentos

LEGO MINDSTORMS EV3 es la generación anterior al Spike Prime y sigue siendo la plataforma dominante en competencias educativas (RoboCup Junior, WRO en muchas regiones, FLL hasta 2023, OBR). Pybricks corre sobre el firmware **ev3dev** y expone una API limpia de MicroPython que es muy parecida a la de Spike pero **no idéntica**.

Esta skill cubre las diferencias clave, los gotchas típicos del EV3 y el patrón base de un programa de competición.

## Cuándo usar / cuándo NO usar

- ✅ Usar para: programar EV3 con Pybricks, instalar ev3dev en SD, conectar EV3 vía SSH/Wi-Fi/BT, manejar motors EV3 (LargeMotor, MediumMotor), sensores LEGO oficiales conectados al EV3.
- ❌ No usar para: Spike Prime / Robot Inventor (es `pybricks-spike-fundamentals`), sensores Mindsensors I²C (es `mindsensors-ev3-sensors`), EV3 con el firmware LEGO oficial o EV3-G (este skill asume Pybricks/ev3dev exclusivamente).

## Instalación rápida (resumen)

1. Bajar imagen `ev3dev-stretch-ev3-generic-2020-04-10.img.xz` desde ev3dev.org.
2. Flashear a microSD ≥ 8 GB con balenaEtcher.
3. Insertar SD en EV3, encender — bootea desde la SD sin tocar el firmware interno.
4. Conectar por USB/Wi-Fi y abrir VS Code con extensión **LEGO MINDSTORMS EV3 MicroPython** (de LEGO/ev3dev).
5. Cada programa se sube por SSH y corre con `brickrun`.

Para Pybricks 3.x (versión moderna), se usa el mismo flujo pero el `import` cambia — ver abajo.

## Estructura típica de un programa Pybricks EV3

```python
#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import (Motor, TouchSensor, ColorSensor,
                                 InfraredSensor, UltrasonicSensor, GyroSensor)
from pybricks.parameters import Port, Stop, Direction, Button, Color
from pybricks.tools import wait, StopWatch, DataLog
from pybricks.robotics import DriveBase
from pybricks.media.ev3dev import SoundFile, ImageFile

# Hub
ev3 = EV3Brick()

# Motors — EV3 usa puertos A, B, C, D
left_motor = Motor(Port.B, Direction.CLOCKWISE)
right_motor = Motor(Port.C, Direction.CLOCKWISE)

# Sensors — EV3 usa puertos 1, 2, 3, 4
color_sensor = ColorSensor(Port.S3)
gyro = GyroSensor(Port.S2)
touch = TouchSensor(Port.S1)

# DriveBase — diámetro de rueda y track width en mm
WHEEL_DIAMETER = 56    # rueda balón de fútbol clásica de EV3
AXLE_TRACK = 114       # distancia entre el centro de las ruedas
drive = DriveBase(left_motor, right_motor, WHEEL_DIAMETER, AXLE_TRACK)

# Beep al iniciar
ev3.speaker.beep()

# ... lógica de competición ...
```

**Shebang obligatorio**: `#!/usr/bin/env pybricks-micropython`. Sin él, ev3dev no sabe qué interprete usar.

## Diferencias críticas vs Pybricks Spike

| Aspecto | EV3 (ev3dev) | Spike Prime / Inventor |
|---|---|---|
| Import del hub | `from pybricks.hubs import EV3Brick` | `from pybricks.hubs import PrimeHub` |
| Import sensores | `from pybricks.ev3devices import ColorSensor, ...` | `from pybricks.pupdevices import ColorSensor, ...` |
| Puertos motores | `Port.A`, `Port.B`, `Port.C`, `Port.D` (4 puertos) | `Port.A`-`Port.F` (6 puertos) |
| Puertos sensores | `Port.S1`, `Port.S2`, `Port.S3`, `Port.S4` | Mismos puertos que motores |
| Motores | `LargeMotor` (Port A-D), `MediumMotor` (Port A-D) — la clase `Motor()` detecta el tipo | Todos son `Motor()` (medium-angular, large-angular) |
| Velocidad típica útil | LargeMotor ~700 deg/s, MediumMotor ~1200 deg/s | Más rápidos, ~1000-1500 deg/s |
| Encoder | 1 deg de precisión | 1 deg de precisión |
| Loop rate seguro | `wait(10)` = 100 Hz | `wait(10)` = 100 Hz |
| IMU/Gyro | Sensor EXTERNO (`GyroSensor` que se enchufa) | IMU integrado (`hub.imu`) |
| Color sensor | Devuelve `reflection()` 0-100, `color()`, `ambient()` | Igual pero con HSV adicional |
| Multitarea | Solo single-thread (`uasyncio` limitado) | Multitask con `multitask()` |
| Display | 178×128 monocromo, `ev3.screen.draw_*` | 5×5 LED matrix |
| Sonido | Speaker integrado con `ev3.speaker.beep/play_file` | Solo beep |
| Botones | `ev3.buttons.pressed()` devuelve lista de `Button.*` | Igual |
| Tiempo de boot | 30-45 seg (booteo de Linux) | ~3 seg |

## EV3Brick — APIs útiles

```python
ev3 = EV3Brick()

# Screen
ev3.screen.clear()
ev3.screen.draw_text(20, 40, "Hola!", text_color=Color.BLACK)
ev3.screen.print("línea simple")
ev3.screen.load_image(ImageFile.HAPPY)

# Speaker
ev3.speaker.beep(frequency=500, duration=300)
ev3.speaker.play_notes(["E4/4", "D4/4", "C4/2"], tempo=120)
ev3.speaker.say("Robot listo")  # text-to-speech, requiere espeak

# Buttons
pressed = ev3.buttons.pressed()
if Button.CENTER in pressed:
    print("Centro presionado")

# Battery
print("Bater:", ev3.battery.voltage(), "mV")
print("Current:", ev3.battery.current(), "mA")

# Light bar arriba del brick (verde/rojo/naranja, sólido/flash)
ev3.light.on(Color.GREEN)
ev3.light.on(Color.RED)
ev3.light.off()
```

## Motors — particularidades EV3

```python
m = Motor(Port.A, Direction.CLOCKWISE, gears=[12, 36])  # gear reduction 1:3
m.run(500)              # deg/s constante hasta nuevo comando
m.run_time(500, 1000)   # 500 deg/s durante 1000 ms (bloqueante)
m.run_target(500, 90)   # ir a posición absoluta 90° (bloqueante)
m.run_angle(500, 90)    # girar 90° desde posición actual (bloqueante)
m.run_until_stalled(500, then=Stop.HOLD, duty_limit=30)  # útil para attachments

m.stop()                # coast (frenado libre)
m.brake()               # brake (resistencia eléctrica)
m.hold()                # hold (lock activo, drena batería)

# Lectura
m.angle()       # posición absoluta acumulada (deg)
m.speed()       # velocidad actual (deg/s)
m.reset_angle(0)
m.reset_angle()  # resetea al cero del encoder físico

# Control settings
m.control.limits(speed=600, acceleration=2000, torque=100)
m.control.target_tolerances(speed=20, position=10)
```

**Diferencia importante con Spike**: en EV3 los motores LargeMotor saturan ~700-900 deg/s reales. Pedirle 1500 deg/s no acelera más — solo desperdicia control. **Conocé el techo real de tu motor antes de tunear**.

## DriveBase — el caballo de batalla

```python
drive = DriveBase(left_motor, right_motor, wheel_diameter=56, axle_track=114)

# Settings (mm/s, mm/s², deg/s, deg/s²)
drive.settings(straight_speed=200, straight_acceleration=400,
               turn_rate=120, turn_acceleration=300)

# Movimientos bloqueantes
drive.straight(300)            # avanzar 300 mm
drive.straight(-100)           # retroceder 100 mm
drive.turn(90)                 # girar 90° en el lugar (pivot)
drive.curve(radius=200, angle=90)  # curva de radio 200 mm, 90°

# Movimiento no-bloqueante (loop manual)
drive.drive(speed=200, turn_rate=30)   # mm/s, deg/s
# ... hacer otras cosas ...
drive.stop()

# Estado
drive.distance()    # mm recorridos desde reset()
drive.angle()       # deg girados acumulados
drive.state()       # (distance, drive_speed, angle, turn_rate)
drive.reset()
```

## Sensores LEGO en EV3 — particularidades

### ColorSensor (EV3, puerto S3 típicamente)

```python
cs = ColorSensor(Port.S3)
cs.reflection()  # 0-100, LED rojo, mejor para line following
cs.ambient()     # 0-100, sin LED, mide luz ambiente
cs.color()       # Color.BLACK/WHITE/RED/GREEN/BLUE/YELLOW/BROWN/NONE
cs.rgb()         # (r, g, b) cada uno 0-100
```

**Gotcha**: el ColorSensor de EV3 es ~3-5× más lento que el del Spike. No esperes muestreos de 500 Hz — quédate en ~100 Hz reales.

### GyroSensor (EV3, puerto S2 típicamente)

```python
g = GyroSensor(Port.S2, Direction.CLOCKWISE)
g.angle()  # deg acumulados desde reset()
g.speed()  # deg/s (rate)
g.reset_angle(0)
```

**El gyro EV3 tiene drift**: ~1-2°/min en reposo. Para una corrida de 2-3 minutos no es problema, para una de 10 sí. Recalibrar antes de cada misión, no en el medio.

**No mover el robot durante el `reset_angle()`**. El gyro hace una calibración interna que requiere reposo.

### UltrasonicSensor

```python
us = UltrasonicSensor(Port.S4)
us.distance()  # mm, rango 30-2500
us.presence()  # True si detecta algún otro sensor ultrasónico cercano
```

Rango efectivo confiable: 50-1000 mm. Por debajo de 30 mm es ciego, por arriba de 1500 mm es ruidoso. Filtrar con median filter cuando se uses en loop.

### InfraredSensor (rojo, opcional)

```python
ir = InfraredSensor(Port.S4)
ir.distance()        # 0-100 (proxy de distancia, NO mm)
ir.beacon(channel=1) # (relative_distance, heading) del beacon IR
ir.keypad(channel=1) # botones del control remoto IR
```

Útil para WRO Football (beacon ball) pero no es ranging real.

### TouchSensor

```python
t = TouchSensor(Port.S1)
if t.pressed(): ...
```

Útil como **bumper de seguridad** y para arrancar el robot al apretarse contra una pared.

## Patrón "wait for start" robusto

```python
def wait_for_start():
    """Mostrar 'READY' en pantalla. Arrancar al apretar centro O al tocar bumper."""
    ev3.screen.clear()
    ev3.screen.draw_text(40, 50, "READY")
    ev3.speaker.beep()
    while True:
        if Button.CENTER in ev3.buttons.pressed():
            break
        if touch.pressed():
            break
        wait(20)
    while Button.CENTER in ev3.buttons.pressed():
        wait(20)
    ev3.screen.clear()
    ev3.screen.draw_text(40, 50, "GO!")
```

## Patrón "calibrar gyro" antes de arrancar

```python
def calibrate_gyro():
    """Reset del gyro asegurando que el robot esté quieto."""
    ev3.screen.draw_text(30, 50, "Calibrando")
    wait(500)  # asegurar reposo
    gyro.reset_angle(0)
    wait(200)
    # Doble check — debería seguir en 0
    if abs(gyro.angle()) > 1:
        gyro.reset_angle(0)
        wait(200)
    ev3.screen.draw_text(30, 80, "OK")
```

## Multi-archivo: estructura recomendada

```
robot/
├── main.py                 # entry point, lógica de misiones
├── robot_config.py         # constantes (puertos, dimensiones, ganancias PID)
├── drive_helpers.py        # straight_with_gyro(), turn_to_heading(), etc.
├── sensor_helpers.py       # calibración, normalización
├── missions/
│   ├── mission_1.py
│   └── mission_2.py
└── tests/
    └── test_calibration.py
```

`main.py` carga las constantes:

```python
import robot_config as cfg
left_motor = Motor(cfg.PORT_LEFT, cfg.DIR_LEFT)
```

## Watchdog y `wait()` — manejo del loop rate

```python
from pybricks.tools import StopWatch

watch = StopWatch()
LOOP_PERIOD_MS = 20  # 50 Hz objetivo

while running:
    loop_start = watch.time()
    
    # ... lógica del PID ...
    
    elapsed = watch.time() - loop_start
    if elapsed < LOOP_PERIOD_MS:
        wait(LOOP_PERIOD_MS - elapsed)
    # si elapsed > período, ya nos pasamos — log y continuar
```

Pybricks EV3 NO tiene watchdog hardware que mate el programa, pero **sin un `wait()` en el loop**, el scheduler de ev3dev congela responsividad y el USB se desconecta.

## DataLog — telemetría para tuning

```python
from pybricks.tools import DataLog

log = DataLog('time', 'error', 'kp_term', 'kd_term', 'output', name='run',
              extension='csv', timestamp=False)

start = watch.time()
while running:
    error = sensor.reflection() - SETPOINT
    kp_term = KP * error
    kd_term = KD * (error - last_error)
    output = kp_term + kd_term
    log.log(watch.time() - start, error, kp_term, kd_term, output)
    drive.drive(BASE_SPEED, output)
    last_error = error
    wait(20)
```

El CSV queda en el EV3 en `/home/robot/`. Bajarlo por SCP y graficarlo en pandas para identificar oscilaciones.

## Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| `OSError: ENODEV` al arrancar | Sensor no detectado en el puerto | Verificar cable y puerto, reiniciar EV3 |
| Gyro lee valores raros (±1000°) | No se reseteó o se movió durante reset | `gyro.reset_angle(0)` con el robot quieto 500 ms |
| Motor "ronronea" sin moverse | Stall por carga excesiva o `duty_limit` muy bajo | Subir `duty_limit` en `run_until_stalled` o quitar carga mecánica |
| Programa cuelga 30 seg al arrancar | Es normal — booteo de Linux | Esperar, ver LED verde sólido |
| `OSError: Permission denied` al escribir CSV | Programa corriendo sin write permission | Correr con `brickrun` o asegurar path `/home/robot/` |
| Wi-Fi se desconecta solo | EV3 ahorra energía con dongle barato | Usar dongle Edimax EW-7811Un v1 o RT5370 confirmado |
| DriveBase va torcido | `wheel_diameter` o `axle_track` incorrectos | Medir con calibre. Test: pedí `straight(1000)` y medí lo real |
| Color sensor lee distinto en cada robot | Variación entre sensores | Calibración por robot guardada como constantes |
| ColorSensor saturado bajo sol | Luz ambiente penetra la falda | Falda de cartulina negra alrededor del sensor |
| Latencia notable en `reflection()` | El loop está pidiendo demasiados sensores | Reducir a lo mínimo, o leer en variables al inicio del loop |

## Comandos útiles desde la PC (terminal)

```bash
ssh robot@ev3dev.local       # password: maker
brickrun /home/robot/my_program/main.py
scp -r robot/ robot@ev3dev.local:/home/robot/
journalctl -f                # logs en vivo
free -h                       # ver RAM disponible (típico: 64 MB libre)
```

## Recursos

- Pybricks EV3 docs: https://pybricks.com/ev3-micropython/
- ev3dev.org: https://www.ev3dev.org/
- LEGO MINDSTORMS EV3 MicroPython (LEGO oficial, basado en ev3dev): https://education.lego.com/en-us/product-resources/mindstorms-ev3/teacher-resources/python-for-ev3/
- Foro Pybricks (EV3 channel): https://github.com/orgs/pybricks/discussions
- Repo de ejemplos: https://github.com/pybricks/pybricks-projects
