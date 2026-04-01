from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port

hub = PrimeHub()

right_motor = Motor(Port.C)
left_motor = Motor(Port.D)

# Motores girando indefinidamente
right_motor.run(500)
left_motor.run(500)

# Mantener el programa vivo
while True:
    pass