from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor, UltrasonicSensor, ForceSensor
from pybricks.parameters import Button, Color, Direction, Port, Side, Stop
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch
#puertos que se usan
#MOTORES IZQ (D), DER(B)
#SENSORES LUZ IZQ (C), DER(A), FRENTE (E), ULTRASONICO (F)
hub = PrimeHub()
def reiniciar_giroscopio(angulo):
    hub.imu.reset_heading(angulo)
hub.speaker.volume(100)
#Calibracion de los sensores
#Valor sensor izquierdo centro:17
#Valor sensor izquierdo borde:48
#Valor sensor derecho centro:18
#Valor sensor derecho borde:48
#Valor sensor frontal centro:7
#Valor sensor fronta borde:27
#umbrales de negro
centro_negro = 25
cenntro_negro_frente= 24
borde_negro = 35
#umbral linea lateral
#sensor frontal E
negro_frontal = 9
#sensor derecha A
negro_derecha = 13
#sensor izquierda C
negro_izquierda = 13
# Inicializacion de los motores
motor_izquierdo = Motor(Port.D,Direction.COUNTERCLOCKWISE) #motor izquierdo sentido antihorario
motor_derecho = Motor(Port.B,Direction.CLOCKWISE) #motor derecho sentido horario
#definir base de robot 
base_robot = DriveBase(motor_izquierdo, motor_derecho, 56, 140)
# Inicialización de los sensores
distance_sensor = UltrasonicSensor(Port.F)
color_sensor_l = ColorSensor(Port.C)
color_sensor_r = ColorSensor(Port.A)
color_sensor_f = ColorSensor(Port.E)
#funcion para avanzar-retroceder
def movermotores(velocidad_motor_izquierdo,velocidad_motor_derecho,rotacion):
    motor_izquierdo.run_angle(velocidad_motor_izquierdo,rotacion*360,wait=False)
    motor_derecho.run_angle(velocidad_motor_derecho,rotacion*360,wait=False)
    while not motor_izquierdo.done() or not motor_derecho.done():
        wait (10)
#funcion para mover motores de forma indefinida
def mover_motores_indefinido(velocidad_motor_izquierdo,velocidad_motor_derecho):
    motor_izquierdo.run(velocidad_motor_izquierdo)
    motor_derecho.run(velocidad_motor_derecho)
#funcion para correccion proporcional
def recorrer_proporcional(angulo, velocidad):
    lectura_actual = hub.imu.heading()
    kp = 5
    error = angulo - lectura_actual
    correccion_proporcional = error * kp
    base_robot.drive(velocidad, correccion_proporcional)
#funcion para avanzar distancia con correccion proporcional
def recorrer_distancia(cantidad_mm, angulo, velocidad):
    base_robot.reset()
    while True:
        recorrer_proporcional(angulo, velocidad)
        if abs(base_robot.distance()) >= abs(cantidad_mm) : 
            base_robot.brake()
            break
#funcion para parar motores de robot_base
def parar_motores_alt():
    base_robot.brake()
#funcion para parar motores
def parar_motores():
    motor_izquierdo.stop()
    motor_derecho.stop()
def valores_HSV_color(sensor):
    # Obtener los valores de los sensores
    if sensor == 'derecho':
        hsv = color_sensor_r.hsv()
        color = color_sensor_r.color()
        luz = color_sensor_r.reflection()
        print(f"Derecho - HSV: {hsv}, Color: {color}, Luz: {luz}")
    elif sensor == 'izquierdo':
        hsv = color_sensor_l.hsv()
        color = color_sensor_l.color()
        luz = color_sensor_l.reflection()
        print(f"Izquierdo - HSV: {hsv}, Color: {color}, Luz: {luz}")
    elif sensor == 'frontal':
        hsv = color_sensor_f.hsv()
        color = color_sensor_f.color()
        luz = color_sensor_f.reflection()
        print(f"Frontal - HSV: {hsv}, Color: {color}, Luz: {luz}")
#lista de colores a usar, calibrar en cualquier pista
Color.BLACK = Color(h=180, s=20, v=27)
Color.GREEN = Color(h=166, s=68, v=34)
Color.WHITE = Color(h=204, s=17, v=93)
Color.SILVER = Color(h=0, s=0, v=99)

Color.SILVERFRONTAL = Color(h=0, s=0 , v=99)
Color.BLACKFRONTAL = Color(h=180, s=22, v=5)
Color.GREENFRONTAL = Color(h=165, s=68, v=19)
Color.WHITEFRONTAL = Color(h=202, s=19, v=79)

colores = (Color.BLACK, Color.GREEN,Color.NONE,Color.WHITE,Color.SILVER)
coloresf = (Color.BLACKFRONTAL, Color.GREENFRONTAL,Color.NONE,Color.WHITEFRONTAL, Color.SILVERFRONTAL)
color_sensor_l.detectable_colors(colores)
color_sensor_r.detectable_colors(colores)
color_sensor_f.detectable_colors(coloresf)
# Función de actualización de los sensores
def update():
    global color_f, color_l, color_r, light_l, light_f, light_r, distance
    distance = distance_sensor.distance()
    color_l = color_sensor_l.color()
    color_r = color_sensor_r.color()
    color_f = color_sensor_f.color()
    light_l = color_sensor_l.reflection()
    light_r = color_sensor_r.reflection()
    light_f = color_sensor_f.reflection()
#declaro las constantes a utilizar en mi logica de seguidor proporcional integrativo derivativo
velocidad=25
kp=1.6
ki=0
kd=0
error=10
error_anterior = 0
suma_errores = 0
P, I, D = 0,0 ,0
contador = 0.001
ultima_deteccion = StopWatch()
def seguidor_de_linea():
    global error, error_anterior, suma_errores, P, I, D, contador
    tiempo_actual = ultima_deteccion.time()
    error = light_r - light_l
    if light_r < borde_negro or light_l < borde_negro:
        ultima_deteccion.reset()
    if tiempo_actual > 6000:
        parar_motores()
        while True:
            mover_motores_indefinido(-70,-70)
            if color_sensor_r.reflection() < (borde_negro+5) or color_sensor_l.reflection() < (borde_negro+5) or color_sensor_f.reflection() < (negro_frontal+5): 
                parar_motores()
                ultima_deteccion.reset()
                break
    else:
        P = error
        I += error * contador
        D = (error - error_anterior) / contador
        correccion = int(P * kp + I * ki + D * kd)
        potencia_motor_izquierdo = velocidad - correccion
        potencia_motor_derecho = velocidad + correccion
        motor_izquierdo.dc(potencia_motor_izquierdo)
        motor_derecho.dc(potencia_motor_derecho)
        error_anterior = error

def giro_90_grados_derecha():
    hub.imu.reset_heading(0)
    while hub.imu.heading() < 75:
        print(hub.imu.heading())
        mover_motores_indefinido(150,-150)
    parar_motores()
    wait(300)
        
def giro_90_grados_izquierda():
    hub.imu.reset_heading(0)
    while hub.imu.heading() >-65:
        print(hub.imu.heading())
        motor_izquierdo.run(-350)
        motor_derecho.run(350)
    parar_motores()
    wait(300)

def giro_180_grados_derecha():
    hub.imu.reset_heading(0)
    while hub.imu.heading() <177:
        print(hub.imu.heading())
        motor_izquierdo.run(300)
        motor_derecho.run(-300)
    parar_motores()

def obstaculo():
    if -1<distance<100:
        parar_motores()
        hub.speaker.beep(700,30)
        if -1<distance_sensor.distance()<100:
            movermotores(-100,-100,0.1)
            hub.imu.reset_heading(0)
            while hub.imu.heading()>-90:
                mover_motores_indefinido(-100,100)
            parar_motores()
            movermotores(100,100,0.1)
            while color_sensor_l.reflection() > centro_negro:
                mover_motores_indefinido(350,132)
            parar_motores()
            movermotores(100,100,0.08)
            while color_sensor_r.reflection() > centro_negro:
                mover_motores_indefinido(-100,100)
                buscar_linea_izquierda    
            parar_motores()

def buscar_linea_derecha():
    parar_motores()
    hub.imu.reset_heading(0)
    while hub.imu.heading() < 45:
        mover_motores_indefinido(180,-180)
    parar_motores()
    while color_sensor_r.reflection() > (centro_negro+5):
        mover_motores_indefinido(-180,180)
    parar_motores()
    while color_sensor_l.reflection() > (centro_negro+5):
        mover_motores_indefinido(180,-180)
    parar_motores()

def buscar_linea_izquierda():
    parar_motores()
    hub.imu.reset_heading(0)
    while hub.imu.heading() > -45:
        mover_motores_indefinido(-180,180)
    parar_motores()
    while color_sensor_l.reflection() > (centro_negro+5):
        mover_motores_indefinido(180,-180)
    parar_motores()
    while color_sensor_r.reflection() > (centro_negro+5):
        mover_motores_indefinido(-100,100)
    parar_motores()

def doble_negro():
    if light_l < centro_negro+1 and light_r < centro_negro+1 and light_f < centro_negro+1:
        parar_motores()
        movermotores(70,70,0.04)
        if light_l < centro_negro+1 and light_r < centro_negro+1 and light_f < centro_negro+1:
            hub.display.text("DN")
            movermotores(70,70,0.3)

def deteccion_señal_verde():
    if color_r == Color.GREEN:
        motor_izquierdo.stop()
        motor_derecho.stop()
        movermotores(100,100,0.05)
        if color_sensor_l.color() == Color.GREEN:
            hub.display.text("DV")
            giro_180_grados_derecha()
        else:
            hub.display.text("VD")
            movermotores(150,150,0.15)
            giro_90_grados_derecha()
            movermotores(150,150,0.12)
            buscar_linea_derecha()
    if color_l == Color.GREEN:
        motor_izquierdo.stop()
        motor_derecho.stop()
        movermotores(100,100,0.05)
        if color_sensor_r.color() == Color.GREEN:
            hub.display.text("DV")
            giro_180_grados_derecha()
        elif color_l==  Color.GREEN:
            hub.display.text("VI")
            movermotores(150,150,0.15)
            giro_90_grados_izquierda()
            wait(10)
            movermotores(150,150,0.12)
            buscar_linea_izquierda()
        
def T_derecha():
    if light_r < negro_derecha+5 and light_f < negro_frontal+5:
        parar_motores()
        movermotores(100,100,0.03)
        if color_sensor_r.reflection() < negro_derecha+5 and color_sensor_f.reflection() < negro_frontal+5:
            if color_sensor_f.color() == Color.GREEN or color_sensor_r.color() == Color.GREEN:
                parar_motores()
                hub.speaker.beep(500,50)
                hub.display.text("X")
                parar_motores()
                movermotores(-100,-100,0.2)
            else:
                parar_motores()
                hub.display.text("TD")
                wait(1000)
                movermotores(70,70,0.3)
                buscar_linea_izquierda()

def T_izquierda():
        if light_l < negro_izquierda+5 and light_f < negro_frontal+5:
            parar_motores()
            movermotores(100,100,0.03)
        if color_sensor_l.reflection() < negro_izquierda+5 and color_sensor_f.reflection() < negro_frontal+5:
            if color_sensor_l.color() == Color.GREEN or color_sensor_f.color() == Color.GREEN:
                parar_motores()
                hub.speaker.beep(500,50)
                hub.display.text("X")
                parar_motores()
                movermotores(-100,-100,0.1)
            else:
                parar_motores()
                hub.display.text("TI")
                wait(1000)
                movermotores(120,120,0.1)
                buscar_linea_derecha()

def buscar_pared():
    while hub.imu.heading() < 85:
        mover_motores_indefinido(300,-300)
    parar_motores()
    if distance_sensor.distance() < 500:
        print(distance_sensor.distance())
        while distance_sensor.distance()>52:
            mover_motores_indefinido(150,150)
        parar_motores()
        while hub.imu.heading() > 2:
            mover_motores_indefinido(-300, 300)
        parar_motores()
        return "izquierda"  
    else:
        print(distance_sensor.distance())
        while hub.imu.heading() > -85:
            mover_motores_indefinido(-300, 300)
        parar_motores()
        while distance_sensor.distance()>52:
            mover_motores_indefinido(150,150)
        parar_motores()
        while hub.imu.heading() < 2:
            mover_motores_indefinido(300, -300)
        parar_motores()
        return "derecha"

def rescate():
    esquina=0
    parar_motores()
    hub.display.text("r")
    hub.imu.reset_heading(0)
    movermotores(200,200,1)
    orientacion=buscar_pared()
    print(orientacion)
    if orientacion == "izquierda":
        while True:
            while distance_sensor.distance()>59:
                recorrer_proporcional(0,300)
            parar_motores()
            if (color_sensor_l.color() == Color.BLACK or
                color_sensor_r.color() == Color.BLACK or 
                color_sensor_f.color() == Color.BLACK):
                esquina=1
                break
            while hub.imu.heading() > -90:
                mover_motores_indefinido(-200, 200)
            parar_motores()
            wait(10)
            hub.imu.reset_heading(0)
            recorrer_distancia(850,0,300)
            if (color_sensor_l.color() == Color.BLACK or
                color_sensor_r.color() == Color.BLACK or 
                color_sensor_f.color() == Color.BLACK):
                esquina=2
                break
            hub.imu.reset_heading(0)    
            while hub.imu.heading() > 85:
                mover_motores_indefinido(-200,200)
            parar_motores()
            hub.imu.reset_heading(0)
            while distance_sensor.distance()>100:
                recorrer_proporcional(0,300)
            parar_motores()
            esquina=3
            break
        reiniciar_giroscopio(0)
        while distance_sensor.distance() < 128:
            recorrer_proporcional(0,-300)
        parar_motores()
        while hub.imu.heading() > -44:
            mover_motores_indefinido(-200,200)
        parar_motores()
        movermotores(300,300,1.4)
        while hub.imu.heading() > -89:
            mover_motores_indefinido(-200,200)
        parar_motores()
        if esquina == 1 or esquina == 3:
            while distance_sensor.distance()>70:
                recorrer_proporcional(-90,300)
            parar_motores()
            while hub.imu.heading() > -178:
                mover_motores_indefinido(-200,200)
            parar_motores()
            wait(100)
            reiniciar_giroscopio(0)
            while distance_sensor.distance()>70:
                recorrer_proporcional(0,300)
            parar_motores()
            while hub.imu.heading() > -125:
                mover_motores_indefinido(-200,200)
            parar_motores()    
            while distance_sensor.distance()>65:
                recorrer_proporcional(-125,300)
            parar_motores()
        if esquina == 2:
            while distance_sensor.distance()>70:
                recorrer_proporcional(-90,300)
            parar_motores()
            while hub.imu.heading() > -178:
                mover_motores_indefinido(-200,200)
            parar_motores()
            wait(100)
            reiniciar_giroscopio(0)
            while distance_sensor.distance()>70:
                recorrer_proporcional(0,300)
            parar_motores()
            while hub.imu.heading() > -135:
                mover_motores_indefinido(-200,200)
            parar_motores()    
            while distance_sensor.distance()>70:
                recorrer_proporcional(-135,300)
            parar_motores()
    else:
        while True:
            while distance_sensor.distance()>70:
                recorrer_proporcional(0,300)
            parar_motores()
            if (color_sensor_l.color() == Color.BLACK or
                color_sensor_r.color() == Color.BLACK or 
                color_sensor_f.color() == Color.BLACK):
                esquina=1
                break
            while hub.imu.heading() < 88:
                    mover_motores_indefinido(200,-200)
            parar_motores()
            wait(10)
            hub.imu.reset_heading(0)
            recorrer_distancia(850,0,300)
            if (color_sensor_l.color() == Color.BLACK or
                color_sensor_r.color() == Color.BLACK or 
                color_sensor_f.color() == Color.BLACK):
                esquina=2
                break
            hub.imu.reset_heading(0)    
            while hub.imu.heading() < 85:
                    mover_motores_indefinido(200,-200)
            parar_motores()
            hub.imu.reset_heading(0)
            while distance_sensor.distance()>70:
                recorrer_proporcional(0,300)
            parar_motores()
            esquina=3
            break
        reiniciar_giroscopio(0)
        while distance_sensor.distance() < 128:
            recorrer_proporcional(0,-300)
        parar_motores()
        while hub.imu.heading() < 44:
            mover_motores_indefinido(200,-200)
        parar_motores()
        movermotores(300,300,1.4)
        while hub.imu.heading() < 89:
            mover_motores_indefinido(200,-200)
        parar_motores()
        if esquina == 1 or esquina == 3:
            while distance_sensor.distance()>70:
                recorrer_proporcional(90,300)
            parar_motores()
            while hub.imu.heading() < 179:
                mover_motores_indefinido(200,-200)
            parar_motores()
            while distance_sensor.distance()>70:
                recorrer_proporcional(180,300)
            parar_motores()
            reiniciar_giroscopio(0)
            while hub.imu.heading() < 125:
                mover_motores_indefinido(200,-200)
            parar_motores()    
            while distance_sensor.distance()>70:
                recorrer_proporcional(125,300)
            parar_motores()
        if esquina == 2:
            while distance_sensor.distance()>70:
                recorrer_proporcional(90,300)
            parar_motores()
            while hub.imu.heading() < 179:
                mover_motores_indefinido(200,-200)
            parar_motores()
            while distance_sensor.distance()>70:
                recorrer_proporcional(180,300)
            parar_motores()
            while hub.imu.heading() < 320:
                mover_motores_indefinido(200,-200)
            parar_motores()    
            while distance_sensor.distance()>70:
                recorrer_proporcional(320,300)
            parar_motores()

# Bucle principal
modo = None           
def main():
    global modo
    while True:
        update()
        if color_l == Color.SILVER and color_r == Color.SILVER and color_f == Color.SILVERFRONTAL:
            parar_motores()
            modo = "rescate"
        obstaculo()
        deteccion_señal_verde()
        doble_negro()
        T_derecha()
        T_izquierda()
        if modo == "rescate":
            rescate()
            break
        seguidor_de_linea()

main()
