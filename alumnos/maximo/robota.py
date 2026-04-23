# ╔══════════════════════════════════════════════════════════════════╗
# ║  SEGUIDOR PID BASE — para tunear paso a paso                    ║
# ║  Sin gap recovery, sin velocidad adaptativa, sin nada extra     ║
# ║  Ruedas 88mm, motores A/B, sensores C/D                          ║
# ╚══════════════════════════════════════════════════════════════════╝
#
# MÉTODO DE TUNING (hacé esto en ORDEN estricto):
#
#   PASO 1 - VELOCIDAD
#   Probá con VELOCIDAD=120, KP=1.0, KD=0.
#   Va a seguir mal pero sin zigzag. Si va muy lento, subí VELOCIDAD
#   hasta 150. Si se sale en curvas, quedate en 120.
#
#   PASO 2 - KP (reactivo)
#   Dejá KD=0. Subí KP de a 0.3 hasta que siga la línea con
#   confianza. Cuando EMPIECE A ZIGZAGUEAR, bajá 30%.
#   Valor típico final: KP entre 1.5 y 2.5.
#
#   PASO 3 - KD (amortiguador)
#   Con KP fijo, empezá KD = KP * 3.
#   Subí KD de a 2 hasta que el zigzag desaparezca.
#   Si se vuelve "blando" y responde tarde, bajá.
#   Valor típico final: KD entre 5 y 15.
#
#   PASO 4 - Subir velocidad
#   Una vez estable, subí VELOCIDAD de a 20. Probablemente haya que
#   subir un toque KP y KD también.

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait

# ═══════════════════════════════════════════════════════════════════
# HARDWARE
# ═══════════════════════════════════════════════════════════════════
hub = PrimeHub()

motor_izq = Motor(Port.A, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.B)

sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)

robot = DriveBase(motor_izq, motor_der, wheel_diameter=88, axle_track=90)

# ═══════════════════════════════════════════════════════════════════
# PARÁMETROS — EMPEZAR AQUÍ, seguir el método de arriba
# ═══════════════════════════════════════════════════════════════════
VELOCIDAD = 120    # mm/s — constante, sin adaptativa

KP = 5.0
KD = 25.0           # empezar en 0, subir solo cuando KP esté listo

# ═══════════════════════════════════════════════════════════════════
# OFFSET automático (ambos sensores sobre blanco al arrancar)
# ═══════════════════════════════════════════════════════════════════

hub.light.on(Color.YELLOW)
hub.speaker.beep(frequency=500, duration=150)
print("Midiendo offset — ambos sensores sobre BLANCO, quieto 1 seg")

suma_izq = 0
suma_der = 0
for i in range(10):
    suma_izq = suma_izq + sensor_izq.reflection()
    suma_der = suma_der + sensor_der.reflection()
    wait(100)

prom_izq = suma_izq // 10
prom_der = suma_der // 10
OFFSET = prom_izq - prom_der

print("Blanco IZQ =", prom_izq, " DER =", prom_der, " OFFSET =", OFFSET)
hub.speaker.beep(frequency=880, duration=150)
wait(500)

# ═══════════════════════════════════════════════════════════════════
# LOOP PID BASE
# ═══════════════════════════════════════════════════════════════════

hub.light.on(Color.GREEN)

error_prev = 0

while True:
    ref_izq = sensor_izq.reflection()
    ref_der = sensor_der.reflection()

    error = (ref_izq - ref_der) - OFFSET

    derivada = error - error_prev
    error_prev = error

    correccion = KP * error + KD * derivada

    robot.drive(VELOCIDAD, correccion)
    wait(10)