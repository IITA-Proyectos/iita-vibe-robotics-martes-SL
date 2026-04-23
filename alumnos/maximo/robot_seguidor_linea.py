# ╔══════════════════════════════════════════════════════════════════╗
# ║  SEGUIDOR PID — Taller Martes San Lorenzo                      ║
# ║  Motores: A (izq) + B (der, CCW)                               ║
# ║  Sensores: C (tras izq) + D (tras der)                         ║
# ║  Ruedas: 88mm | axle_track: 90mm                               ║
# ║  Calibrado: IZQ negro=15 blanco=99 | DER negro=15 blanco=99    ║
# ╚══════════════════════════════════════════════════════════════════╝

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()

motor_izq = Motor(Port.A)
motor_der = Motor(Port.B, Direction.COUNTERCLOCKWISE)

sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)

robot = DriveBase(motor_izq, motor_der, wheel_diameter=88, axle_track=90)

# ═══════════════════════════════════════════════════════════════════
# CALIBRACIÓN (valores medidos)
# ═══════════════════════════════════════════════════════════════════
NEGRO_IZQ = 15
BLANCO_IZQ = 99
NEGRO_DER = 15
BLANCO_DER = 99

# ═══════════════════════════════════════════════════════════════════
# PARÁMETROS — CAMBIAR ACÁ PARA TUNEAR
# ═══════════════════════════════════════════════════════════════════
VEL_RECTA = 120
VEL_CURVA = 60
VEL_MUY_CERRADA = 40
KP = 2.5
KD = 12.0

# ═══════════════════════════════════════════════════════════════════
# NORMALIZACIÓN — convierte lectura cruda a 0-100
# ═══════════════════════════════════════════════════════════════════
def norm_izq(valor):
    n = (valor - NEGRO_IZQ) * 100 // (BLANCO_IZQ - NEGRO_IZQ)
    if n < 0:
        return 0
    if n > 100:
        return 100
    return n

def norm_der(valor):
    n = (valor - NEGRO_DER) * 100 // (BLANCO_DER - NEGRO_DER)
    if n < 0:
        return 0
    if n > 100:
        return 100
    return n

# ═══════════════════════════════════════════════════════════════════
# ARRANQUE
# ═══════════════════════════════════════════════════════════════════
hub.light.on(Color.GREEN)
hub.speaker.beep(frequency=880, duration=200)

error_prev = 0

# ═══════════════════════════════════════════════════════════════════
# LOOP PID — sin wait()
# ═══════════════════════════════════════════════════════════════════
while True:
    izq = norm_izq(sensor_izq.reflection())
    der = norm_der(sensor_der.reflection())

    # error: positivo = línea a la derecha, negativo = línea a la izq
    error = der - izq

    derivada = error - error_prev
    error_prev = error

    correccion = KP * error + KD * derivada

    # Velocidad adaptativa
    if abs(error) > 40:
        vel = VEL_MUY_CERRADA
    elif abs(error) > 20:
        vel = VEL_CURVA
    else:
        vel = VEL_RECTA

    robot.drive(vel, correccion)