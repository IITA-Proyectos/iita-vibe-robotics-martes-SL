# ╔══════════════════════════════════════════════════════════════════╗
# ║  SEGUIDOR DE LÍNEA SIMPLE                                      ║
# ║  Modelo IA: qwen3.6-plus-free (opencode)                        ║
# ║  Robot: Spike 2WD + 2 sensores de color                        ║
# ║  Sensores: Puerto A (izq), Puerto B (der)                      ║
# ║  Motores: Puerto C (izq), Puerto D (der)                       ║
# ║  Autor: Profe Franco — Taller Vibe Robotics IITA               ║
# ╚══════════════════════════════════════════════════════════════════╝

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# HARDWARE: inicializamos el hub, motores y sensores
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

hub = PrimeHub()

# Motor izquierdo en Puerto C, girando al revés para que avance igual que el derecho
motor_izq = Motor(Port.C, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.D)

# Sensor izquierdo en Puerto A, sensor derecho en Puerto B
sensor_izq = ColorSensor(Port.A)
sensor_der = ColorSensor(Port.B)

# Configuración física del robot
DIAMETRO_RUEDA = 56.0   # mm — diámetro de las ruedas
DISTANCIA_EJES = 90.0   # mm — distancia entre centros de las ruedas

# DriveBase: combina los 2 motores en un objeto que maneja movimiento
robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)

# Usar el giroscopio interno para giros más estables
robot.use_gyro(True)

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# PARÁMETROS DEL PID — AJUSTAR SEGÚN LA PISTA
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# Velocidad base de avance en mm/s (más bajo = más preciso, más alto = más rápido)
VELOCIDAD = 180

# Ganancias del control PID:
#   KP (proporcional): corrige según el error actual. Más alto = más agresivo.
#   KI (integral): corrige el error acumulado a lo largo del tiempo.
#                  Útil para eliminar desvíos constantes.
#   KD (derivativo): corrige según la velocidad del cambio del error.
#                    Ayuda a prevenir oscilaciones y suaviza la respuesta.
KP = 1.8
KI = 0.03
KD = 10.0

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# FUNCIONES AUXILIARES
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

def clamp(valor, minimo, maximo):
    """
    Limita un valor para que no se salga de un rango [minimo, maximo].
    Esto evita que la corrección del PID sea demasiado grande
    y cause giros bruscos o inestables.
    """
    if valor < minimo:
        return minimo
    if valor > maximo:
        return maximo
    return valor


def calcular_error_pid(ref_izq, ref_der):
    """
    Calcula el error de seguimiento usando la diferencia entre
    los dos sensores de color.

    - Si ambos sensores ven lo mismo → error = 0 (robot centrado)
    - Si el izquierdo ve más blanco → error positivo → corregir a la izquierda
    - Si el derecho ve más blanco → error negativo → corregir a la derecha

    Retorna: (error, correccion_pid)
    """
    global integral, error_prev

    # Error = diferencia entre sensor izquierdo y derecho
    error = ref_izq - ref_der

    # Término integral: acumula errores pasados para corregir desvíos constantes
    integral = clamp(integral + error * KI, -50, 50)

    # Término derivativo: mide qué tan rápido cambia el error
    derivada = error - error_prev
    error_prev = error

    # Fórmula completa del PID
    correccion = KP * error + integral + KD * derivada

    return error, correccion


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# INICIALIZACIÓN
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# Resetear el giroscopio y esperar a que esté listo
hub.imu.reset_heading(0)
wait(1500)

# Variables del PID (se mantienen entre ciclos del loop)
integral = 0.0
error_prev = 0.0

# Led verde = robot listo para seguir la línea
hub.light.on(Color.GREEN)

print("=== SEGUIDOR DE LÍNEA SIMPLE INICIADO ===")
print("Modelo: qwen3.6-plus-free (opencode)")
print("Presioná el botón del hub para detener")

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# LOOP PRINCIPAL: lee sensores → calcula PID → ajusta motores
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

while True:
    # 1. Leer la reflexión de ambos sensores (0 = negro puro, 100 = blanco puro)
    reflejo_izq = sensor_izq.reflection()
    reflejo_der = sensor_der.reflection()

    # 2. Calcular la corrección PID basada en la diferencia entre sensores
    error, correccion = calcular_error_pid(reflejo_izq, reflejo_der)

    # 3. Aplicar la corrección: el robot avanza a VELOCIDAD y gira según correccion
    #    correccion positiva → gira a la izquierda
    #    correccion negativa → gira a la derecha
    robot.drive(VELOCIDAD, correccion)

    # 4. Esperar 10ms antes de la próxima lectura (~100 ciclos por segundo)
    wait(10)
