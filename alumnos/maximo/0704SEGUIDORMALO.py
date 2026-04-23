from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

# -----------------------
# INICIALIZACIÓN
# -----------------------
hub = PrimeHub()

left_motor = Motor(Port.A)    # Izquierdo (invertido)
right_motor = Motor(Port.B)   # Derecho

left_sensor = ColorSensor(Port.C)
right_sensor = ColorSensor(Port.D)

# -----------------------
# CONFIGURACIÓN
# -----------------------
vel_avance = 150    # velocidad normal
vel_giro = 100      # velocidad para girar
vel_retro = 200     # velocidad de retroceso (más rápida)
blanco = 90         # valor aproximado blanco
negro = 20          # valor aproximado negro
umbral = (blanco + negro) / 2  # punto medio para decidir
retro_delay = 200   # tiempo de retroceso en ms

# -----------------------
# LOOP PRINCIPAL
# -----------------------
while True:
    # Leer sensores
    l = left_sensor.reflection()
    r = right_sensor.reflection()

    # Mostrar valores en terminal para calibrar
    print("L:", l, "R:", r)

    # Decisión de movimiento
    if l > umbral and r > umbral:
        # Ambos sensores en blanco -> avanzar recto
        left_motor.run(-vel_avance)  # invertido
        right_motor.run(vel_avance)
    elif l < umbral and r > umbral:
        # Izquierdo en negro -> girar derecha
        left_motor.run(-vel_giro)
        right_motor.run(vel_avance)
    elif l > umbral and r < umbral:
        # Derecho en negro -> girar izquierda
        left_motor.run(-vel_avance)
        right_motor.run(vel_giro)
    else:
        # Ambos en negro -> retroceder más rápido un poco
        left_motor.run(vel_retro)   # invertido
        right_motor.run(-vel_retro)
        #wait(retro_delay)            # retroceso breve