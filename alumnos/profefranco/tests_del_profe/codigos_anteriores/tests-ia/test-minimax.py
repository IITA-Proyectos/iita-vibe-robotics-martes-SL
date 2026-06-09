"""
================================================================================
TEST-MINIMAX - Seguidor de Línea Simple
================================================================================
Modelo utilizado: minimax-m2.5-free

Descripción:
    Robot seguidor de línea para competencia Rescue Line Junior (Robocup).
    Configuración: 2 ruedas + 2 sensores de luz en la parte frontal.
    
Hardware:
    - 2 motores de tracción (ruedas izquierda y derecha)
    - 2 sensores de luz (reflectancia) ubicados adelante de las ruedas
================================================================================
"""

from pybricks.hubs import SpikePrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Stop
from pybricks.tools import wait

# ================================================================================
# CONFIGURACIÓN DEL ROBOT
# ================================================================================

# Definición de puertos y configuración de motores
MOTOR_IZQUIERDO = Port.A
MOTOR_DERECHO = Port.B
SENSOR_IZQUIERDO = Port.S1
SENSOR_DERECHO = Port.S4

# Parámetros del robot
DIAMETRO_RUEDA = 56  # mm
ANCHO_TRACCION = 120  # mm (distancia entre ruedas)

# Velocidades base
VELOCIDAD_BASE = 300  # grados por segundo
VELOCIDAD_MAXIMA = 500  # grados por segundo

# ================================================================================
# INICIALIZACIÓN DEL ROBOT
# ================================================================================

hub = SpikePrimeHub()

motor_izquierdo = Motor(MOTOR_IZQUIERDO, Direction.CLOCKWISE)
motor_derecho = Motor(MOTOR_DERECHO, Direction.CLOCKWISE)

sensor_izquierdo = ColorSensor(SENSOR_IZQUIERDO)
sensor_derecho = ColorSensor(SENSOR_DERECHO)

# ================================================================================
# CALIBRACIÓN
# ================================================================================

def calibrar_sensores():
    """
    Calibra los sensores de luz para determinar los valores de línea y suelo.
    
    Proceso:
        - Colocar sensores sobre la línea negra
        - Colocar sensores sobre el suelo blanco
        - El robot calculará automáticamente los umbrales
    """
    print("Iniciando calibración...")
    print("Coloca ambos sensores sobre la LÍNEA NEGRA y presiona botón central")
    wait(2000)
    
    # Lecturas sobre línea negra
    lectura_linea_i = sensor_izquierdo.reflection()
    lectura_linea_d = sensor_derecho.reflection()
    
    print(f"Línea - Izq: {lectura_linea_i}, Der: {lectura_linea_d}")
    print("Coloca ambos sensores sobre el SUELO BLANCO y presiona botón central")
    wait(2000)
    
    # Lecturas sobre suelo blanco
    lectura_suelo_i = sensor_izquierdo.reflection()
    lectura_suelo_d = sensor_derecho.reflection()
    
    print(f"Suelo - Izq: {lectura_suelo_i}, Der: {lectura_suelo_d}")
    print("Calibración completada!")
    
    # Calcular umbrales (promedio entre línea y suelo)
    umbral_i = (lectura_linea_i + lectura_suelo_i) // 2
    umbral_d = (lectura_linea_d + lectura_suelo_d) // 2
    
    return {
        'umbral_izquierdo': umbral_i,
        'umbral_derecho': umbral_d,
        'linea_izq': lectura_linea_i,
        'linea_der': lectura_linea_d,
        'suelo_izq': lectura_suelo_i,
        'suelo_der': lectura_suelo_d
    }

# ================================================================================
# LECTURA DE SENSORES
# ================================================================================

def leer_sensores(config):
    """
    Lee los valores de反射ancia de ambos sensores y determina su estado.
    
    Retorna:
        - 0: Ambos sensores sobre línea (intersección)
        - 1: Sensor izquierdo sobre línea, derecho sobre suelo
        - 2: Sensor izquierdo sobre suelo, derecho sobre línea
        - 3: Ambos sensores sobre suelo (fuera de línea)
    """
    lectura_i = sensor_izquierdo.reflection()
    lectura_d = sensor_derecho.reflection()
    
    # Determinar si cada sensor está sobre línea o suelo
    sobre_linea_i = lectura_i < config['umbral_izquierdo']
    sobre_linea_d = lectura_d < config['umbral_derecho']
    
    # Codificar estado
    if sobre_linea_i and sobre_linea_d:
        return 0  # Intersección
    elif sobre_linea_i and not sobre_linea_d:
        return 1  # Girar izquierda
    elif not sobre_linea_i and sobre_linea_d:
        return 2  # Girar derecha
    else:
        return 3  # Fuera de línea

# ================================================================================
# CONTROL DE MOVIMIENTO
# ================================================================================

def mover(velocidad_i, velocidad_d):
    """
    Controla la velocidad de cada motor independientemente.
    
    Args:
        velocidad_i: Velocidad del motor izquierdo (°/s)
        velocidad_d: Velocidad del motor derecho (°/s)
    """
    motor_izquierdo.run(velocidad_i)
    motor_derecho.run(velocidad_d)

def stop():
    """Detiene ambos motores."""
    motor_izquierdo.stop()
    motor_derecho.stop()

def seguir_linea(config):
    """
    Algoritmo principal de seguimiento de línea.
    
    Utiliza control proporcional:
    - Si ambos sensores en línea: avanza recto
    - Si sensor izquierdo en línea: gira a la izquierda
    - Si sensor derecho en línea: gira a la derecha
    - Si ambos fuera de línea: busca la línea
    
    Args:
        config: Diccionario con valores de calibración
    """
    estado = leer_sensores(config)
    
    if estado == 0:
        # Intersección - avanzar recto
        mover(VELOCIDAD_BASE, VELOCIDAD_BASE)
        
    elif estado == 1:
        # Sensor izquierdo detecta línea - girar izquierda
        # Reducir velocidad motor izquierdo, mantener derecho
        mover(VELOCIDAD_BASE * 0.3, VELOCIDAD_BASE)
        
    elif estado == 2:
        # Sensor derecho detecta línea - girar derecha
        # Mantener motor izquierdo, reducir velocidad derecho
        mover(VELOCIDAD_BASE, VELOCIDAD_BASE * 0.3)
        
    elif estado == 3:
        # Ambos fuera de línea - buscar línea
        # Opciones: girar en lugar o continuar último movimiento
        mover(VELOCIDAD_BASE * 0.5, VELOCIDAD_BASE * 0.5)

# ================================================================================
# PROGRAMA PRINCIPAL
# ================================================================================

def main():
    """
    Programa principal que ejecuta el seguidor de línea.
    
    Flujo:
        1. Calibrar sensores
        2. Esperar presión de botón
        3. Ejecutar algoritmo de seguimiento
    """
    # Calibrar sensores al inicio
    config = calibrar_sensores()
    
    # Esperar a que se presione el botón central para iniciar
    print("Presiona botón central para iniciar")
    hub.button.center.pressed(wait)
    
    print("Iniciando seguidor de línea...")
    
    # Bucle principal de seguimiento
    while True:
        seguir_linea(config)
        wait(10)  # Pequeña pausa para estabilidad

# Ejecutar programa
main()
