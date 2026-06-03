from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# -----------------------------
# CONFIGURACIÓN
# -----------------------------
hub = PrimeHub()

# Motores
left_motor = Motor(Port.A)
right_motor = Motor(Port.B, Direction.COUNTERCLOCKWISE)

robot = DriveBase(left_motor, right_motor, 56, 90)

# Velocidad moderada y estable
robot.settings(
    straight_speed=200,
    straight_acceleration=300,
    turn_rate=120,
    turn_acceleration=200
)

# NOTA: NO reiniciamos giroscopio
# hub.imu.reset_heading(0) ❌

# -----------------------------
# FUNCIONES
# -----------------------------
def error_angulo(objetivo, actual):
    error = objetivo - actual
    while error > 180:
        error -= 360
    while error < -180:
        error += 360
    return error

def avanzar_recto(distancia_mm, angulo_objetivo):
    robot.reset()
    while robot.distance() < distancia_mm:
        actual = hub.imu.heading()
        error = error_angulo(angulo_objetivo, actual)
        correccion = error * 1.5
        if correccion > 50: correccion = 50
        if correccion < -50: correccion = -50
        robot.drive(150, correccion)  # 🔥 velocidad asegurada para que se mueva
        wait(10)
    robot.stop()
    wait(50)

def girar_a(objetivo):
    while True:
        actual = hub.imu.heading()
        error = error_angulo(objetivo, actual)
        if abs(error) < 4:
            break
        if abs(error) > 20:
            giro = error * 3
        else:
            giro = 100 if error > 0 else -100
        robot.drive(0, giro)
        wait(10)
    robot.stop()
    wait(50)
    return hub.imu.heading()

# -----------------------------
# RUTINA PRINCIPAL
# -----------------------------
angulo_actual = hub.imu.heading()  # comenzar desde heading actual

for i in range(4):
    avanzar_recto(300, angulo_actual)   # 30 cm
    angulo_actual = girar_a(angulo_actual + 90)

hub.speaker.beep()