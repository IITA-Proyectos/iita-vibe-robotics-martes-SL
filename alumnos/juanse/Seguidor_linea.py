from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

hub = PrimeHub()

motor_izq = Motor(Port.A)
motor_der = Motor(Port.B)

sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)

vel = 200
vel_atras = 100   # 🔥 más suave hacia atrás

NEGRO = 10  # según tu medición

while True:

    izq = sensor_izq.reflection()
    der = sensor_der.reflection()

    print("IZQ:", izq, "DER:", der)

    # =====================
    # CONTROL POR SENSOR
    # =====================

    # 🔴 SENSOR IZQUIERDO VE NEGRO
    if izq < NEGRO and der > NEGRO:
        motor_izq.run(vel_atras)   # 🔥 hacia atrás
        motor_der.run(vel)         # adelante

    # 🔴 SENSOR DERECHO VE NEGRO
    elif der < NEGRO and izq > NEGRO:
        motor_izq.run(-vel)        # adelante
        motor_der.run(-vel_atras)  # 🔥 hacia atrás

    # 🔴 AMBOS NEGRO → sigue recto lento
    elif izq < NEGRO and der < NEGRO:
        motor_izq.run(-100)
        motor_der.run(100)

    # ⚪ AMBOS BLANCO → recto normal
    else:
        motor_izq.run(-vel)
        motor_der.run(vel)

    wait(10)