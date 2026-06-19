# TEST DE DISTANCIA — Calibrar diámetro de rueda
# El robot avanza 1 metro. Medí con cinta métrica.
# Si no llega a 1m exacto, ajustá DIAMETRO_RUEDA.
#
# FÓRMULA:
#   nuevo = viejo × (1000 / distancia_real_mm)
#   Ejemplo: midió 985mm → nuevo = 56.0 × (1000/985) = 56.85

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F)

DIAMETRO_RUEDA = 56.0   # ← AJUSTAR ACÁ
DISTANCIA_EJES = 112.0

robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)
robot.use_gyro(True)
robot.settings(straight_speed=200, straight_acceleration=150)

hub.imu.reset_heading(0)
wait(1500)

hub.light.on(Color.YELLOW)
hub.speaker.beep(800, 200)
wait(500)

# Avanzar exactamente 1 metro
robot.straight(1000)

hub.light.on(Color.GREEN)
hub.speaker.beep(1200, 300)

# Ahora medí con cinta métrica la distancia real
# y ajustá DIAMETRO_RUEDA con la fórmula de arriba
