# Seguidor de Línea simple
# Modelo actual: nanogpt5
# Robot: Spike 2WD + 2 sensores de luz (Front)
# Sensores: Izquierdo en Puerto A, Derecho en Puerto B
# Motores: Izquierdo en Puerto C, Derecho en Puerto D
##
# Este programa sigue una pista de línea simple usando dos sensores
# de reflexión. Si ambos sensores ven la línea, avanza; si el izquierdo
# ve más blanco, gira hacia la izquierda, etc. Es un controlador básico
# PID para mantener la línea en el centro.

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# ------------------------
# HARDWARE: inicialización
# ------------------------

# Motores: Izquierdo (C), Derecho (D)
motor_izq = Motor(Port.C, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.D)

# Sensores: Izquierdo (A), Derecho (B)
sensor_izq = ColorSensor(Port.A)
sensor_der = ColorSensor(Port.B)

# DriveBase para manejo simplificado de movimiento
DIAMETRO_RUEDA = 56.0    # mm
DISTANCIA_EJES = 90.0     # mm
robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)
robot.use_gyro(True)  # para estabilizar movimientos (opcional)

# ----------------------------
# PARÁMETROS DEL CONTROL (PID)
# ----------------------------
VELOCIDAD = 180            # mm/s
# Valor de referencia de reflexión para el borde de la línea
TARGET = 45

# Ganancias del PID (ajustar según la pista)
KP = 1.8
KI = 0.03
KD = 10.0

# Tolerancias y variables de estado
integral = 0.0
error_prev = 0.0

# ----------------------------
# FUNCIONES AUXILIARES
# ----------------------------
def clamp(valor, minimo, maximo):
    """
    Limita un valor a un rango [minimo, maximo].
    Evita acciones demasiado agresivas del controlador.
    """
    if valor < minimo:
        return minimo
    if valor > maximo:
        return maximo
    return valor

def calcular_error_pid(ref_izq, ref_der):
    """
    Calcula el error y la corrección PID a partir de las lecturas
    de los dos sensores.

    Retorna: (error, correccion)
    - error: diferencia entre lecturas (positivo si más blanco en izquierda)
    - correccion: salida de control para DriveBase (mapeado a giro)
    """
    global integral, error_prev

    # Error simple basado en diferencia de reflexiones
    error = ref_izq - ref_der

    # Integral (acumulado del error)
    integral = clamp(integral + error * KI, -50, 50)

    # Derivada (velocidad de cambio del error)
    derivada = error - error_prev
    error_prev = error

    # PID
    correccion = KP * error + integral + KD * derivada
    return error, correccion

# ----------------------------
# INICIALIZACIÓN
# ----------------------------
# Preparar sensores y estado inicial
hub.imu.reset_heading(0)
wait(1500)
print("=== SEGUIDOR DE LÍNEA SIMPLE (nanogpt5) INICIADO ===")

# ----------------------------
# BUCLE PRINCIPAL
# ----------------------------
while True:
    reflejo_izq = sensor_izq.reflection()
    reflejo_der = sensor_der.reflection()

    _, correccion = calcular_error_pid(reflejo_izq, reflejo_der)

    # DriveBase recibe (velocidad_lineal, velocidad_rotacional)
    robot.drive(VELOCIDAD, correccion)

    wait(10)
