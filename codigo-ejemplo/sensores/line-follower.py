# SEGUIDOR DE LÍNEA PID
# Robot: spike-2wd-basico + sensor de color en Puerto A
# Sigue el borde de una línea negra sobre fondo blanco
#
# CALIBRACIÓN: Ajustar TARGET al valor de reflexión
# en el borde de la línea (entre negro puro y blanco puro)
# Tip: poné el sensor sobre el borde y mirá qué valor da

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F)
robot = DriveBase(motor_izq, motor_der, wheel_diameter=56, axle_track=112)
robot.use_gyro(True)

sensor = ColorSensor(Port.A)  # ← CAMBIAR PUERTO SI ES OTRO

# ── Parámetros ───────────────────────────────────────
VELOCIDAD = 150        # mm/s — más lento = más preciso
TARGET = 50            # Reflexión en el borde (calibrar)
KP = 2.0               # Proporcional
KI = 0.05              # Integral
KD = 10.0              # Derivativo


def clamp(val, minv, maxv):
    if val < minv: return minv
    if val > maxv: return maxv
    return val


# ── Programa principal ───────────────────────────────
hub.imu.reset_heading(0)
wait(1500)

integral = 0.0
error_prev = 0.0

hub.light.on(Color.GREEN)

while True:
    reflejo = sensor.reflection()
    error = reflejo - TARGET

    integral = clamp(integral + error, -100, 100)
    derivada = error - error_prev
    correccion = KP * error + KI * integral + KD * derivada
    error_prev = error

    robot.drive(VELOCIDAD, correccion)
    wait(10)
