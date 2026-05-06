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

motor_izq = Motor(Port.B, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.A)

sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)
sensor_central = ColorSensor(Port.E)

robot = DriveBase(motor_izq, motor_der, wheel_diameter=88, axle_track=90)

# ═══════════════════════════════════════════════════════════════════
# CALIBRACIÓN (valores medidos)
# ═══════════════════════════════════════════════════════════════════
NEGRO_IZQ = 5
BLANCO_IZQ = 26
NEGRO_DER = 5
BLANCO_DER = 26
NEGRO_CEN = 5
BLANCO_CEN = 26

# ═══════════════════════════════════════════════════════════════════
# PARÁMETROS — CAMBIAR ACÁ PARA TUNEAR
# ═══════════════════════════════════════════════════════════════════
VEL_RECTA = 60
VEL_CURVA = 35
VEL_MUY_CERRADA = 20
KP = 1.8
KD = 20.0
BIAS = 0  # Lo dejamos en 0 para ver si eso causaba el desvío a la izquierda

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

def norm_cen(valor):
    n = (valor - NEGRO_CEN) * 100 // (BLANCO_CEN - NEGRO_CEN)
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
ultimo_error_grande = 0

# ═══════════════════════════════════════════════════════════════════
# LOOP PID 
# ═══════════════════════════════════════════════════════════════════
while True:
    izq = norm_izq(sensor_izq.reflection())
    der = norm_der(sensor_der.reflection())
    cen = norm_cen(sensor_central.reflection())

    error_crudo = izq - der

    # Inteligencia para curvas cerradas vs huecos blancos
    if izq > 80 and der > 80 and cen > 80:
        # Los tres ven blanco.
        if abs(ultimo_error_grande) > 50:
            error = ultimo_error_grande  # Se salió doblando fuerte: mantiene el giro
        else:
            error = 0  # Es un hueco en la recta: sigue derecho
    else:
        error = error_crudo
        
        # Guardar en la memoria si estamos doblando fuerte o si estamos centrados
        if error > 50:
            ultimo_error_grande = 100
        elif error < -50:
            ultimo_error_grande = -100
        elif abs(error) < 20 and cen < 60:
            ultimo_error_grande = 0  # Centrado sobre la línea, resetea la memoria

    derivada = error - error_prev
    error_prev = error

    correccion = (KP * error) + (KD * derivada) + BIAS

    # Velocidad adaptativa
    if abs(error) > 50:
        vel = 0  # Gira en el lugar (una rueda avanza, otra retrocede)
    elif abs(error) > 20:
        vel = VEL_CURVA
    else:
        vel = VEL_RECTA

    robot.drive(vel, correccion)