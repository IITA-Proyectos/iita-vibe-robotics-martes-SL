# Modelo: nemotron-3-super-free
# Seguidor de línea simple para robot con 2 ruedas y 2 sensores de luz
# Este código implementa un seguidor de línea básico que detecta una línea negra sobre fondo blanco
# utilizando dos sensores de color posicionados delante de cada rueda.

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# Inicialización del hub y motores
hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)  # Motor izquierdo (giro contrario)
motor_der = Motor(Port.F)                             # Motor derecho
robot = DriveBase(motor_izq, motor_der, wheel_diameter=56, axle_track=112)
robot.use_gyro(True)

# Inicialización de los sensores de luz (uno delante de cada rueda)
sensor_izq = ColorSensor(Port.A)  # Sensor izquierdo
sensor_der = ColorSensor(Port.B)  # Sensor derecho

# Parámetros del seguidor de línea
VELOCIDAD_BASE = 200       # Velocidad básica en mm/s
GANANCIA = 2.0             # Ganancia para corrección proporcional
UMBRAL = 30                # Valor umbral para detectar línea (ajustar según calibración)

def leer_sensores():
    """
    Lee los valores de reflexión de ambos sensores de color.
    Retorna una tupla (valor_izquierdo, valor_derecho)
    donde valores más bajos indican superficie más oscura.
    """
    return sensor_izq.reflection(), sensor_der.reflection()

def calcular_correccion(reflejo_izq, reflejo_der):
    """
    Calcula la corrección necesaria basado en la diferencia entre sensores.
    Retorna un valor de corrección para girar el robot.
    Positivo: girar a la derecha, Negativo: girar a la izquierda
    """
    # Si ambos sensores detectan línea (valores bajos), seguir recto
    # Si solo el izquierdo detecta línea, girar ligeramente a la izquierda
    # Si solo el derecho detecta línea, girar ligeramente a la derecha
    # Si ninguno detecta línea, buscar la línea (girar en búsqueda)
    
    error = reflejo_izq - reflejo_der
    correccion = GANANCIA * error
    return correccion

def main():
    """
    Programa principal del seguidor de línea.
    El robot sigue una línea negra sobre fondo blanco usando dos sensores.
    """
    hub.light.on(Color.YELLOW)  # Indicador de inicio
    wait(1000)
    hub.light.on(Color.GREEN)   # Indicador de operación
    
    while True:
        # Lectura de sensores
        reflejo_izq, reflejo_der = leer_sensores()
        
        # Cálculo de corrección basada en diferencia de sensores
        correccion = calcular_correccion(reflejo_izq, reflejo_der)
        
        # Aplicar movimiento: velocidad constante + corrección de giro
        robot.drive(VELOCIDAD_BASE, correccion)
        
        # Pequeña pausa para estabilidad
        wait(20)

if __name__ == "__main__":
    main()