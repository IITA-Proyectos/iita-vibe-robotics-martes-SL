from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

hub = PrimeHub()

# Configuración de Sensores
sensor_izquierdo = ColorSensor(Port.B)
sensor_central = ColorSensor(Port.D)
sensor_derecho = ColorSensor(Port.F)

# Configuración de Motores
motor_izquierdo = Motor(Port.A, Direction.COUNTERCLOCKWISE)
motor_derecho = Motor(Port.E)
robot = DriveBase(motor_izquierdo, motor_derecho, wheel_diameter=56, axle_track=155)

# =======================================================
# PARÁMETROS DEL SEGUIDOR DE LÍNEA
# =======================================================
VELOCIDAD = 80  

# Valores de calibración actualizados
MIN_IZQ = 4
MAX_IZQ = 50
MIN_CEN = 4  
MAX_CEN = 53 
MIN_DER = 4
MAX_DER = 58

# Constantes PID
KP = 2.2        
KD = 9

# Umbrales
UMBRAL_BLANCO = 40  
UMBRAL_NEGRO = 30   # Aumentado de 20 a 30 para que detecte el negro en movimiento rápido


# =======================================================

print("Seguidor con Cruces (T) y Huecos")
print("Presiona el botón izquierdo del HUB para arrancar.")

while not hub.buttons.pressed():
    wait(10)

print("¡Arrancando!")
wait(500) 

error_previo = 0

while True:
    ref_izq_bruto = sensor_izquierdo.reflection()
    ref_der_bruto = sensor_derecho.reflection()
    ref_cen_bruto = sensor_central.reflection() 

    # =========================================================
    # 1. LÓGICA DE INTERSECCIONES EN "T" O CRUCES (+)
    # =========================================================
    # Un cruce en T o de 90° ocurre cuando el sensor CENTRAL ve negro Y, 
    # AL MISMO TIEMPO, alguno de los laterales (Izquierdo o Derecho) también lo ve.
    if ref_cen_bruto < UMBRAL_NEGRO and (ref_izq_bruto < UMBRAL_NEGRO or ref_der_bruto < UMBRAL_NEGRO):
        robot.stop()
        
        # Verificar si es Cruz (los 3 ven negro) o T (solo 2 ven negro)
        es_cruz = (ref_izq_bruto < UMBRAL_NEGRO and ref_der_bruto < UMBRAL_NEGRO)
        
        if es_cruz:
            print("¡Intersección PERPENDICULAR (+) / Doble Negro detectada!")
            robot.stop() # Asegurar freno antes del beep
            hub.speaker.beep(1200, 50) # Tono agudo para la cruz
        else:
            print("¡Intersección en T detectada!")
            robot.stop() # Asegurar freno antes del beep
            hub.speaker.beep(800, 50) # Tono medio para la T

            
        # 1. Retroceder un poco (3 cm) usando straight para garantizar que sea recto y rápido
        robot.settings(straight_speed=VELOCIDAD, straight_acceleration=200)
        robot.straight(-30) 
        
        # 2. Volver a avanzar siguiendo la línea para obligarse a quedar perfectamente de frente
        while True:
            r_izq = sensor_izquierdo.reflection()
            r_cen = sensor_central.reflection()
            r_der = sensor_derecho.reflection()
            
            # Si toca la T de nuevo, ya está alineado
            if r_cen < UMBRAL_NEGRO and (r_izq < UMBRAL_NEGRO or r_der < UMBRAL_NEGRO):
                break
                
            # Mini PID más rápido que antes (velocidad 50 en vez de 30)
            v_i = (r_izq - MIN_IZQ) / (MAX_IZQ - MIN_IZQ) * 100
            v_d = (r_der - MIN_DER) / (MAX_DER - MIN_DER) * 100
            mini_error = v_i - v_d
            robot.drive(50, mini_error * KP)
            wait(5)
            
        robot.stop()
        
        # 3. Lectura de Verdes (Marcadores de RoboCup)
        color_i = sensor_izquierdo.color()
        color_d = sensor_derecho.color()
        
        if color_i == Color.GREEN and color_d == Color.GREEN:
            print("¡Doble Verde! Media vuelta (180°)")
            robot.stop() # Freno explícito antes del sonido
            hub.speaker.beep(2000, 200)
            robot.straight(40) # Centrar el robot en la intersección
            robot.turn(180)    # Media vuelta
            
        elif color_i == Color.GREEN:
            print("¡Verde Izquierdo! Giro de 90° Izquierda")
            robot.stop() # Freno explícito antes del sonido
            hub.speaker.beep(1500, 100)
            robot.straight(40) 
            robot.turn(-90)    # Pybricks: giro negativo es hacia la izquierda
            robot.straight(30) # Avanzar un poquito para reenganchar la línea
            
        elif color_d == Color.GREEN:
            print("¡Verde Derecho! Giro de 90° Derecha")
            robot.stop() # Freno explícito antes del sonido
            hub.speaker.beep(1500, 100)
            robot.straight(40)
            robot.turn(90)     # Pybricks: giro positivo es hacia la derecha
            robot.straight(30)
            
        else:
            # 4. Sin verdes, cruzar la intersección recto (5 cm)
            print("Sin verdes. Cruzando recto...")
            robot.straight(50)
        
        error_previo = 0
        continue

        
    # =========================================================
    # 2. LÓGICA DE RECUPERACIÓN Y GAPS (HUECOS)
    # =========================================================
    if ref_izq_bruto > UMBRAL_BLANCO and ref_der_bruto > UMBRAL_BLANCO and ref_cen_bruto > UMBRAL_BLANCO:
        print("¡Línea Perdida! Iniciando búsqueda (Gap de 20 cm)")
        
        robot.reset()
        encontrada = False
        
        robot.drive(VELOCIDAD, 0) 
        
        while robot.distance() < 200:
            if sensor_izquierdo.reflection() < UMBRAL_NEGRO or sensor_derecho.reflection() < UMBRAL_NEGRO or sensor_central.reflection() < UMBRAL_NEGRO:
                encontrada = True
                print("Línea cruzando el hueco... ¡Reanudando!")
                break
            wait(5)
            
        if not encontrada:
            print("No se encontró línea al frente. ¡Retrocediendo a la última conocida!")
            robot.drive(-VELOCIDAD, 0) 
            
            while True:
                if sensor_izquierdo.reflection() < UMBRAL_NEGRO or sensor_derecho.reflection() < UMBRAL_NEGRO or sensor_central.reflection() < UMBRAL_NEGRO:
                    print("Línea detectada en la marcha atrás.")
                    break
                wait(5)
                
        robot.stop()
        wait(150)
        error_previo = 0
        continue
    # =========================================================
    
    # 0. Normalizar los sensores
    val_izq = (ref_izq_bruto - MIN_IZQ) / (MAX_IZQ - MIN_IZQ) * 100
    val_der = (ref_der_bruto - MIN_DER) / (MAX_DER - MIN_DER) * 100
    
    # 1. Calcular el Error simétrico
    error = val_izq - val_der
    
    # 2. Calcular la Derivada
    derivada = error - error_previo
    
    # 3. Calcular la Magnitud de la Corrección
    correccion = (error * KP) + (derivada * KD)
    
    # 4. Velocidad Dinámica: Frenar en las curvas
    reduccion = abs(error) * 0.6
    velocidad_actual = VELOCIDAD - reduccion
    if velocidad_actual < 25:
        velocidad_actual = 25
        
    # 5. Aplicar motores
    robot.drive(velocidad_actual, correccion)
    
    error_previo = error
