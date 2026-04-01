from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()

motor_der = Motor(Port.A)
motor_izq = Motor(Port.B, Direction.COUNTERCLOCKWISE)

robot = DriveBase(motor_izq, motor_der, 56, 90)
robot.use_gyro(True)

hub.imu.reset_heading(0)
wait(500)

# -----------------------------
def error_angulo(obj, act):
    e = obj - act
    while e > 180:
        e -= 360
    while e < -180:
        e += 360
    return e

# -----------------------------
def avanzar_recto(dist, objetivo):
    robot.reset()

    while True:
        d = robot.distance()

        if d >= dist:
            break

        actual = hub.imu.heading()
        error = error_angulo(objetivo, actual)

        # 🔥 CONTROL SIMPLE (ESTABLE)
        Kp = 1.2
        correccion = error * Kp

        # 🔥 límite BAJO → evita que gire en vez de avanzar
        if correccion > 40:
            correccion = 40
        if correccion < -40:
            correccion = -40

        # 🔥 velocidad CONSTANTE (sin rampa para estabilidad)
        robot.drive(400, correccion)

        wait(10)

    robot.stop()
    wait(200)

# -----------------------------
def girar_a(objetivo):
    while True:
        actual = hub.imu.heading()
        error = error_angulo(objetivo, actual)

        if abs(error) < 3:
            break

        # 🔥 giro simple y firme
        giro = error * 3

        if giro > 120:
            giro = 120
        if giro < -120:
            giro = -120

        robot.drive(0, giro)
        wait(10)

    robot.stop()
    wait(200)

# -----------------------------
# CUADRADO

angulo = 0

for i in range(4):
    avanzar_recto(1000, angulo)

    angulo += 90
    girar_a(angulo)

hub.speaker.beep()