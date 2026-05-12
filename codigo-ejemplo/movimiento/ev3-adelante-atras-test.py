#!/usr/bin/env pybricks-micropython
# ╔══════════════════════════════════════════════════════════════╗
# ║  TEST BÁSICO: ADELANTE 2 SEG → ATRÁS 2 SEG                  ║
# ║  Robot: EV3 diferencial                                      ║
# ║    Motores medianos: Puerto A (izq), Puerto B (der)          ║
# ║    Sensores Mindsensors (array luz): S1 (izq), S3 (der)      ║
# ║    Giroscopio LEGO: S2                                       ║
# ║  Pybricks for EV3 (ev3dev)                                   ║
# ║  Autor: Taller Vibe Robotics IITA — 2026-05-12              ║
# ╚══════════════════════════════════════════════════════════════╝

from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor, GyroSensor
from pybricks.parameters import Port, Direction, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# ── Hub ──────────────────────────────────────────────────
ev3 = EV3Brick()

# ── Motores ──────────────────────────────────────────────
# IMPORTANTE: los motores están enfrentados (uno a cada lado del robot),
# así que uno tiene que ir invertido para que ambos avancen "hacia adelante"
# cuando les pedimos velocidad positiva. Si el robot gira en vez de avanzar
# recto, swappear COUNTERCLOCKWISE ↔ CLOCKWISE en uno de los dos.
motor_izq = Motor(Port.A, Direction.CLOCKWISE)
motor_der = Motor(Port.B, Direction.COUNTERCLOCKWISE)

# ── Giroscopio (declarado para uso futuro, no se usa en este test) ──
gyro = GyroSensor(Port.S2)

# ── DriveBase ────────────────────────────────────────────
# AJUSTAR ESTOS VALORES MIDIENDO TU ROBOT CON CALIBRE / REGLA
DIAMETRO_RUEDA = 56    # mm — diámetro de tu rueda real
DISTANCIA_EJES = 120   # mm — distancia centro-a-centro entre las dos ruedas

robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)

# ── Parámetros del test ──────────────────────────────────
VELOCIDAD = 150        # mm/s — empezar lento para verificar dirección
TIEMPO_MS = 2000       # 2 segundos

# ── Espera al botón centro para arrancar ─────────────────
ev3.screen.clear()
ev3.screen.draw_text(10, 40, "TEST adelante/atras")
ev3.screen.draw_text(10, 70, "Apretar CENTRO")
while Button.CENTER not in ev3.buttons.pressed():
    wait(20)
while Button.CENTER in ev3.buttons.pressed():
    wait(20)
ev3.speaker.beep()

# ── 1) ADELANTE 2 segundos ───────────────────────────────
ev3.screen.clear()
ev3.screen.draw_text(40, 50, "ADELANTE")
robot.drive(VELOCIDAD, 0)   # speed mm/s, turn_rate deg/s (0 = recto)
wait(TIEMPO_MS)
robot.stop()

# Pausa breve para que se vea el cambio
wait(500)

# ── 2) ATRÁS 2 segundos ──────────────────────────────────
ev3.screen.clear()
ev3.screen.draw_text(40, 50, "ATRAS")
robot.drive(-VELOCIDAD, 0)
wait(TIEMPO_MS)
robot.stop()

# ── Fin ──────────────────────────────────────────────────
ev3.screen.clear()
ev3.screen.draw_text(40, 50, "LISTO")
ev3.speaker.beep(800, 300)
