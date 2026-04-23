# ============================================================
# MODELO ACTUALMENTE EN USO: opencode/big-pickle
# Prueba de generación de código para seguidor de línea
# Robot: LEGO Spike Prime con 2 sensores de luz (1 por rueda)
# ============================================================

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# ============================================================
# INICIALIZACIÓN DEL HARDWARE
# ============================================================

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

# Sensores de luz: uno a la izquierda de la línea, otro a la derecha
left_sensor = ColorSensor(Port.C)
right_sensor = ColorSensor(Port.D)

# ============================================================
# PARÁMETROS DEL CONTROLADOR PID
# ============================================================

TARGET_LEFT = 40
TARGET_RIGHT = 40

KP = 1.5
KI = 0.0
KD = 10.0

VELOCIDAD_BASE = 200

# ============================================================
# FUNCIONES DE UTILIDAD
# ============================================================

def clamp(valor, minimo, maximo):
    """Limita un valor dentro de un rango definido"""
    if valor < minimo:
        return minimo
    if valor > maximo:
        return maximo
    return valor

# ============================================================
# CALIBRACIÓN INICIAL
# ============================================================

hub.light.on(Color.ORANGE)
print("=== CALIBRACIÓN ===")
print("Coloca el robot sobre la línea y espera...")
wait(2000)
print("Calibrando sensores...")

linea_izq = left_sensor.reflection()
linea_der = right_sensor.reflection()

hub.light.on(Color.YELLOW)
print("Coloca el robot en el espacio libre junto a la línea...")
wait(2000)

fondo_izq = left_sensor.reflection()
fondo_der = right_sensor.reflection()

TARGET_LEFT = (linea_izq + fondo_izq) / 2
TARGET_RIGHT = (linea_der + fondo_der) / 2

print(f"Valores calibrados - Izq: {TARGET_LEFT:.1f}, Der: {TARGET_RIGHT:.1f}")

hub.imu.reset_heading(0)
wait(1000)

hub.light.on(Color.GREEN)
print("=== SEGUIDOR DE LÍNEA INICIADO ===")

# ============================================================
# BUCLE PRINCIPAL DE CONTROL PID
# ============================================================

integral = 0.0
error_prev = 0.0

while True:
    reflejo_izq = left_sensor.reflection()
    reflejo_der = right_sensor.reflection()

    # Error: diferencia entre el valor del sensor y el objetivo
    # Error positivo = el sensor está sobre la línea (más blanco)
    # Error negativo = el sensor está fuera de la línea (más negro)
    error_izq = reflejo_izq - TARGET_LEFT
    error_der = reflejo_der - TARGET_RIGHT

    # Promedio de errores para determinar la corrección
    error = (error_izq + error_der) / 2

    # Término integral: acumula errores pasados (para correcciones suaves)
    integral = clamp(integral + error, -50, 50)

    # Término derivativo: predice el comportamiento basándose en el cambio de error
    derivada = error - error_prev
    error_prev = error

    # Cálculo de la corrección PID
    correccion = KP * error + KI * integral + KD * derivada

    # Limitamos la corrección para evitar giros demasiado bruscos
    correccion = clamp(correccion, -150, 150)

    # Movemos el robot: velocidad base + corrección para girar
    robot.drive(VELOCIDAD_BASE, correccion)
    wait(10)
