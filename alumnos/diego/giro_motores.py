from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

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

# Activar giroscopio
robot.use_gyro(True)

# 1. Avanzar 20 centímetros (200 milímetros)
robot.straight(200)

wait(500) # Esperamos medio segundo

# 2. Controlar cada motor por separado
# Motor derecho en positivo (hacia adelante) a velocidad 300 grados/segundo
# Motor izquierdo en negativo (hacia atrás) a velocidad -300 grados/segundo
motor_derecho.run(300)
motor_izquierdo.run(-300)

# Mantener ese movimiento durante 2 segundos (2000 milisegundos)
wait(2000)

# 3. Frenar los motores
motor_derecho.stop()
motor_izquierdo.stop()

hub.speaker.beep()
