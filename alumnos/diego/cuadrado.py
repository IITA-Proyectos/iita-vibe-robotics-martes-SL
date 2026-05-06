from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# Inicializar el Hub
hub = PrimeHub()

# Configurar motores
motor_izquierdo = Motor(Port.A, Direction.COUNTERCLOCKWISE)
motor_derecho = Motor(Port.B)

# Configurar la base motriz
robot = DriveBase(
    motor_izquierdo, 
    motor_derecho, 
    wheel_diameter=56, 
    axle_track=112
)

# ¡NUEVO! Activar el giroscopio integrado del Hub
# Esto hace que el robot avance perfectamente derecho y que los giros sean súper precisos.
robot.use_gyro(True)

# Avisamos que estamos listos
hub.speaker.beep()

# Hacer un cuadrado (4 lados iguales y 4 giros de 90 grados)
for i in range(4):
    robot.straight(300) # Avanzar 30 cm
    robot.turn(90)      # Girar 90 grados
    wait(200)           # Pequeña pausa para que se estabilice

# Sonido de festejo al terminar
hub.speaker.beep(frequency=1000, duration=100)
wait(100)
hub.speaker.beep(frequency=1200, duration=100)
wait(100)
hub.speaker.beep(frequency=1500, duration=300)
