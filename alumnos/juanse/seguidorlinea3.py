from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

hub = PrimeHub()

motor_izq = Motor(Port.A)
motor_der = Motor(Port.B)

sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)

# =====================
# CONFIG
# =====================
vel_base = 250
vel_min = 180
vel_max = 400

Kp = 2.2
Kd = 10.0

NEGRO_UMBRAL = 15

error_anterior = 0
comp = 0

# =====================
# 🔬 CALIBRACIÓN
# =====================
hub.speaker.beep()
print("BLANCO...")
wait(2000)

blanco_izq = sensor_izq.reflection()
blanco_der = sensor_der.reflection()

hub.speaker.beep()
print("NEGRO...")
wait(2000)

negro_izq = sensor_izq.reflection()
negro_der = sensor_der.reflection()

hub.speaker.beep()

# =====================
# NORMALIZACIÓN
# =====================
def norm(valor, negro, blanco):
    return (valor - negro) * 100 / (blanco - negro)

# =====================
# VERDE PRO (HSV)
# =====================
contador_verde_izq = 0
contador_verde_der = 0

def es_verde(sensor):
    h, s, v = sensor.hsv()
    return (90 < h < 160 and s > 40 and v > 20)

def detectar_verde(izq_raw, der_raw):
    global contador_verde_izq, contador_verde_der

    # ❌ ignorar verde si hay negro
    if izq_raw < NEGRO_UMBRAL or der_raw < NEGRO_UMBRAL:
        contador_verde_izq = 0
        contador_verde_der = 0
        return None

    if es_verde(sensor_izq):
        contador_verde_izq += 1
    else:
        contador_verde_izq = 0

    if es_verde(sensor_der):
        contador_verde_der += 1
    else:
        contador_verde_der = 0

    if contador_verde_izq > 3 and contador_verde_der > 3:
        return "doble"
    elif contador_verde_izq > 3:
        return "izq"
    elif contador_verde_der > 3:
        return "der"

    return None

# =====================
# GIROS PRO
# =====================
def girar_hasta_linea_izq():
    while True:
        motor_izq.run(120)
        motor_der.run(250)

        if sensor_izq.reflection() < NEGRO_UMBRAL or sensor_der.reflection() < NEGRO_UMBRAL:
            break

def girar_hasta_linea_der():
    while True:
        motor_izq.run(-250)
        motor_der.run(-120)

        if sensor_izq.reflection() < NEGRO_UMBRAL or sensor_der.reflection() < NEGRO_UMBRAL:
            break

# =====================
# LOOP
# =====================
while True:

    izq_raw = sensor_izq.reflection()
    der_raw = sensor_der.reflection()

    # =====================
    # VERDE
    # =====================
    verde = detectar_verde(izq_raw, der_raw)

    if verde == "izq":
        girar_hasta_linea_izq()
        continue

    elif verde == "der":
        girar_hasta_linea_der()
        continue

    elif verde == "doble":
        motor_izq.run(250)
        motor_der.run(250)
        wait(400)
        continue

    # =====================
    # NORMALIZAR
    # =====================
    izq = norm(izq_raw, negro_izq, blanco_izq)
    der = norm(der_raw, negro_der, blanco_der)

    # =====================
    # MODO AGRESIVO
    # =====================
    if izq_raw < NEGRO_UMBRAL and der_raw > NEGRO_UMBRAL:
        motor_izq.run(120)
        motor_der.run(260)
        continue

    elif der_raw < NEGRO_UMBRAL and izq_raw > NEGRO_UMBRAL:
        motor_izq.run(-260)
        motor_der.run(-120)
        continue

    # =====================
    # PID
    # =====================
    error = der - izq

    derivada = error - error_anterior
    steering = (Kp * error) + (Kd * derivada)

    error_anterior = error

    # =====================
    # VELOCIDAD DINÁMICA
    # =====================
    curva = abs(error)

    if curva < 5:
        vel = vel_max
    elif curva < 15:
        vel = vel_base
    else:
        vel = vel_min

    # =====================
    # COMPENSACIÓN AUTOMÁTICA
    # =====================
    if izq > 80 and der > 80:
        comp += (der - izq) * 0.05

    if comp > 30: comp = 30
    if comp < -30: comp = -30

    print("comp:", comp)

    # =====================
    # CONTROL FINAL
    # =====================
    vel_izq = vel - steering
    vel_der = vel + steering + comp

    motor_izq.run(-vel_izq)
    motor_der.run(vel_der)

    wait(10)