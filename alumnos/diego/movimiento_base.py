from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# Inicializar el Hub
hub = PrimeHub()

# 1. Configurar los motores
# A menudo, los motores están enfrentados, por lo que uno debe girar "al revés" para que ambos avancen.
# Si al probarlo el robot gira en su propio eje o va hacia atrás, podés cambiar el "Direction.COUNTERCLOCKWISE" al motor derecho.
motor_izquierdo = Motor(Port.A, Direction.COUNTERCLOCKWISE)
motor_derecho = Motor(Port.B)

# 2. Configurar la base motriz (DriveBase)
# Se necesitan los diámetros de las ruedas y la distancia entre los centros de las ruedas (axle_track)
# Los valores típicos para el robot básico de Spike son 56 y 112 milímetros.
robot = DriveBase(
    motor_izquierdo, 
    motor_derecho, 
    wheel_diameter=56, 
    axle_track=112
)

# Avisamos que estamos listos
hub.speaker.beep()

# 3. Mover el robot
# Avanzar 500 milímetros (50 cm)
robot.straight(500)

# Esperar medio segundo
wait(500)

# Girar 90 grados a la derecha
robot.turn(90)

# Esperar medio segundo
wait(500)

# Avanzar otros 300 milímetros (30 cm)
robot.straight(300)

# Sonido final
hub.speaker.beep(frequency=1000, duration=500)
