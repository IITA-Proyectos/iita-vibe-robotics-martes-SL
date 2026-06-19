# TEST DE GIRO — Calibrar distancia entre ejes (axle track)
# El robot gira 10 vueltas completas (3600°).
# Contá cuántas vueltas reales hizo.
#
# FÓRMULA:
#   nuevo = viejo × (grados_reales / 3600)
#   Ejemplo: giró solo 9.8 vueltas (3528°):
#   nuevo = 112.0 × (3528/3600) = 109.76
#
# TIP: Marcá el frente del robot con cinta y contá las vueltas.

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F)

DIAMETRO_RUEDA = 56.0
DISTANCIA_EJES = 112.0  # ← AJUSTAR ACÁ

robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)
robot.use_gyro(True)
robot.settings(turn_rate=100, turn_acceleration=80)

hub.imu.reset_heading(0)
wait(1500)

hub.light.on(Color.YELLOW)
hub.speaker.beep(800, 200)
wait(500)

# Girar 10 vueltas completas
robot.turn(3600)

hub.light.on(Color.GREEN)
hub.speaker.beep(1200, 300)

# Contá cuántas vueltas reales hizo y usá la fórmula
