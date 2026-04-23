from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()

left_motor = Motor(Port.A, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.B)

robot = DriveBase(left_motor, right_motor, 56, 90)

robot.settings(
    straight_speed=600,
    straight_acceleration=600,
    turn_rate=450,
    turn_acceleration=1000
)

sensor = ColorSensor(Port.C)

TARGET = 35
KP = 1.8
KI = 0.03
KD = 12.0
VELOCIDAD = 200

def clamp(val, minv, maxv):
    if val < minv:
        return minv
    if val > maxv:
        return maxv
    return val

hub.imu.reset_heading(0)
wait(1500)

integral = 0.0
error_prev = 0.0

hub.light.on(Color.GREEN)

print("=== SEGUIDOR DE LÍNEA INICIADO ===")

while True:
    reflejo = sensor.reflection()
    error = reflejo - TARGET

    integral = clamp(integral + error, -50, 50)
    derivada = error - error_prev
    correccion = KP * error + KI * integral + KD * derivada
    error_prev = error

    robot.drive(VELOCIDAD, correccion)
    wait(10)