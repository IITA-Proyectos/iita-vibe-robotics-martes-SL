# 🔌 Skill Futuro: Platform Adapter

> **Estado**: 🔮 FUTURO — Para cuando el taller escale a múltiples plataformas

## Cuándo activar

- Un alumno quiere usar Arduino en vez de Spike
- Se incorpora Raspberry Pi al taller
- El equipo RoboCup pasa a hardware custom
- Alguien quiere usar ROS

## El problema

Las soluciones de robótica son conceptualmente iguales en todas las plataformas (PID es PID, state machine es state machine), pero la implementación cambia completamente. Este skill traduce de una plataforma a otra.

## Mapa de plataformas

```
Nivel 1: LEGO + Pybricks (actual del taller)
    ↓ escala a
Nivel 2: Arduino + sensores discretos
    ↓ escala a
Nivel 3: Raspberry Pi + cámara + Python
    ↓ escala a
Nivel 4: ROS 2 + múltiples sensores + SLAM
```

## Equivalencias entre plataformas

| Concepto | Pybricks | Arduino | Raspberry Pi | ROS 2 |
|----------|----------|---------|-------------|-------|
| Motor DC | `Motor(Port.A)` | `analogWrite(pin, pwm)` | `RPi.GPIO` / `pigpio` | `ros2_control` |
| Servo | N/A | `Servo.write(angle)` | `pigpio.set_servo` | `ros2_control` |
| Encoder | `motor.angle()` | `attachInterrupt` | `pigpio callback` | `encoder topic` |
| Gyro/IMU | `hub.imu.heading()` | `MPU6050.getAngleZ()` | `smbus2` / `imusensor` | `imu_filter_madgwick` |
| Color sensor | `sensor.reflection()` | `analogRead(pin)` | OpenCV HSV | `image_pipeline` |
| Ultrasonido | `UltrasonicSensor()` | `NewPing` library | `RPi.GPIO` | `range_sensor` |
| PID loop | `wait(10)` loop | `millis()` loop | `time.sleep(0.01)` | `ros2_control` |
| State machine | `if/elif` | `switch/case` | `if/elif` o `smach` | `FlexBE` / `BehaviorTree.CPP` |

## Prompt (para cuando se active)

```
Sos un ingeniero de robótica que ayuda a migrar código entre plataformas.

Tengo este código en [PLATAFORMA_ORIGEN]:
[pegar código]

Necesito portarlo a [PLATAFORMA_DESTINO].
Hardware disponible: [listar]

Necesito:
1. Código equivalente completo para la nueva plataforma
2. Mapa de conexiones (qué pin/puerto para cada sensor/motor)
3. Librerías necesarias (con versiones)
4. Diferencias de comportamiento a tener en cuenta
5. Qué parámetros hay que re-calibrar
6. Limitaciones de la nueva plataforma vs la anterior
```

## Kits sugeridos para escalar desde Spike

### Nivel 2: Arduino
```
- Arduino Mega 2560 (más pines que Uno)
- Motor driver: L298N o TB6612FNG
- IMU: MPU6050 (gyro + accel, I2C)
- Sensores IR: TCRT5000 (line following)
- Ultrasonido: HC-SR04
- Alimentación: pack 7.4V LiPo
- Costo total: ~USD 25-40
```

### Nivel 3: Raspberry Pi
```
- Raspberry Pi 4 (2GB mínimo)
- Pi Camera v2 o v3
- HAT de motores (Adafruit Motor HAT o similar)
- IMU: BNO055 (sensor fusion integrado)
- LiDAR: RPLidar A1 (USD 99) o TFMini (USD 30)
- Alimentación: powerbank 5V 3A
- Costo total: ~USD 100-200
```

## Recursos

- [Arduino Reference](https://www.arduino.cc/reference/en/)
- [MicroPython](https://micropython.org/) — Python en microcontroladores
- [CircuitPython](https://circuitpython.org/) — Adafruit fork de MicroPython
- [ROS 2 Jazzy](https://docs.ros.org/en/jazzy/) — LTS actual
- [PlatformIO](https://platformio.org/) — IDE multi-plataforma
