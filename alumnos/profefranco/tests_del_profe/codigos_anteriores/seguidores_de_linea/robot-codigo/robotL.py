from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.tools import wait, StopWatch
from pybricks.robotics import DriveBase

hub = PrimeHub()

left_motor = Motor(Port.A, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.B)

robot = DriveBase(left_motor, right_motor, 56, 90)

robot.settings(
    straight_speed=900,
    straight_acceleration=1200,
    turn_rate=450,
    turn_acceleration=1000
)

hub.imu.reset_heading(0)

BIAS = 8

def error_angulo(objetivo, actual):
    error = objetivo - actual
    while error > 180:
        error -= 360
    while error < -180:
        error += 360
    return error

def avanzar_recto(distancia_mm, objetivo):
    robot.reset()
    timer = StopWatch()
    timer.reset()

    velocidad_min = 100
    velocidad_max = 850
    rampa_dist = 400

    error_max = 0
    error_total = 0
    muestras = 0

    zonas = {
        "inicio": {"sum": 0, "count": 0},
        "medio": {"sum": 0, "count": 0},
        "final": {"sum": 0, "count": 0}
    }

    while robot.distance() < distancia_mm:
        distancia = robot.distance()
        actual = hub.imu.heading()
        error = error_angulo(objetivo, actual)

        if abs(error) > error_max:
            error_max = abs(error)

        error_total += abs(error)
        muestras += 1

        if distancia < 300:
            zona = "inicio"
        elif distancia < 700:
            zona = "medio"
        else:
            zona = "final"

        zonas[zona]["sum"] += error
        zonas[zona]["count"] += 1

        if distancia < 200:
            Kp = 3.0
        else:
            Kp = 1.6

        correccion = error * Kp
        correccion += BIAS

        if correccion > 100:
            correccion = 100
        if correccion < -100:
            correccion = -100

        if distancia < rampa_dist:
            velocidad = velocidad_min + (velocidad_max - velocidad_min) * (distancia / rampa_dist)
        else:
            velocidad = velocidad_max

        robot.drive(velocidad, correccion)

        wait(10)

    robot.stop()

    error_promedio = error_total / muestras if muestras > 0 else 0

    print("Recta:", timer.time(), "ms")
    print("Desvio max:", round(error_max, 2))
    print("Desvio promedio:", round(error_promedio, 2))

    for nombre in ["inicio", "medio", "final"]:
        if zonas[nombre]["count"] > 0:
            prom = zonas[nombre]["sum"] / zonas[nombre]["count"]
            direccion = "derecha" if prom > 0 else "izquierda"
            print(nombre, "→", round(prom, 2), "°", direccion)

def girar_a(objetivo):
    timer = StopWatch()
    timer.reset()

    while True:
        actual = hub.imu.heading()
        error = error_angulo(objetivo, actual)

        if abs(error) < 3:
            break

        if abs(error) > 20:
            giro = error * 5
        else:
            giro = 120 if error > 0 else -120

        robot.drive(0, giro)

        wait(10)

    robot.stop()
    wait(50)

    print("Giro:", timer.time(), "ms")

    return hub.imu.heading()

print("=== FORMA L: 30cm x 80cm ===")

angulo_actual = 0

print("---- LADO 1: 30cm ----")
avanzar_recto(300, angulo_actual)

angulo_actual = girar_a(angulo_actual + 90)
print("Giro 90° completado")

print("---- LADO 2: 80cm ----")
avanzar_recto(800, angulo_actual)

print("=== FORMA L COMPLETADA ===")
hub.speaker.beep()