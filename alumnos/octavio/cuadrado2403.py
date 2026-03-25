from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.robotics import DriveBase
from pybricks.parameters import Port, Direction
from pybricks.tools import wait, StopWatch

hub = PrimeHub()

motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F, Direction.CLOCKWISE)

robot = DriveBase(motor_izq, motor_der, wheel_diameter=56, axle_track=115)

hub.imu.reset_heading(0)

objetivo = 0

cronometro = StopWatch()

# 🔧 normalizar ángulo
def normalizar_error(e):
    while e > 180:
        e -= 360
    while e < -180:
        e += 360
    return e

def avanzar_con_pid(distancia, lado):
    global objetivo
    
    robot.reset()
    cronometro.reset()

    suma_error = 0
    max_error = 0
    muestras = 0

    # 🔥 PID variables
    integral = 0
    error_anterior = 0

    # 🔧 constantes PID (ajustables)
    Kp = 2.0
    Ki = 0.02
    Kd = 1.2

    while robot.distance() < distancia:
        d = robot.distance()

        # 🚀 velocidad progresiva optimizada
        if d < distancia * 0.2:
            velocidad = 150 + (d * 1.8)
        elif d > distancia * 0.9:
            velocidad = 220 + (distancia - d) * 0.4
        else:
            velocidad = 420   # 🔥 más velocidad

        # 🎯 error
        error = normalizar_error(hub.imu.heading() - objetivo)

        # 🧠 PID
        integral += error
        derivada = error - error_anterior
        correccion = (Kp * error) + (Ki * integral) + (Kd * derivada)

        error_anterior = error

        # 📊 datos
        suma_error += abs(error)
        if abs(error) > max_error:
            max_error = abs(error)
        muestras += 1

        robot.drive(velocidad, -correccion)
        wait(5)

    robot.stop()

    tiempo = cronometro.time()
    promedio_error = suma_error / muestras

    print("Lado:", lado)
    print("Tiempo (ms):", tiempo)
    print("Error promedio:", promedio_error)
    print("Error máximo:", max_error)
    print("------------------------")

def girar_snap(angulo):
    global objetivo
    
    objetivo += angulo

    while True:
        error = normalizar_error(hub.imu.heading() - objetivo)

        if abs(error) < 1.5:
            break

        robot.drive(0, -error * 6)
        wait(5)

    robot.stop()

# 🔲 CUADRADO PRO
for i in range(4):
    avanzar_con_pid(1000, i + 1)
    girar_snap(90)