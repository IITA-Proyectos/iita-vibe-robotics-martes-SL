from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction
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

# Umbrales para la lógica de huecos (Gaps)
UMBRAL_BLANCO = 40  # Si todos leen mayor a esto, no hay línea
UMBRAL_NEGRO = 20   # Si alguno lee menor a esto, encontró la línea

# =======================================================

print("Seguidor con Búsqueda de Huecos (20cm)")
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
    # LÓGICA DE RECUPERACIÓN Y GAPS (HUECOS)
    # =========================================================
    if ref_izq_bruto > UMBRAL_BLANCO and ref_der_bruto > UMBRAL_BLANCO and ref_cen_bruto > UMBRAL_BLANCO:
        print("¡Línea Perdida! Iniciando búsqueda (Gap de 20 cm)")
        
        # Reseteamos los medidores de distancia de los motores a cero
        robot.reset()
        encontrada = False
        
        # 1. AVANZAR BUSCANDO POR EL GAP A LA MISMA VELOCIDAD
        robot.drive(VELOCIDAD, 0) # Avanzamos a la velocidad normal del robot
        
        while robot.distance() < 200:
            if sensor_izquierdo.reflection() < UMBRAL_NEGRO or sensor_derecho.reflection() < UMBRAL_NEGRO or sensor_central.reflection() < UMBRAL_NEGRO:
                encontrada = True
                print("Línea cruzando el hueco... ¡Reanudando!")
                break
            wait(5)
            
        # 2. SI NO LA ENCONTRÓ TRAS 20 CM, RETROCEDER
        if not encontrada:
            print("No se encontró línea al frente. ¡Retrocediendo a la última conocida!")
            robot.drive(-VELOCIDAD, 0) # Marcha atrás a la velocidad normal
            
            while True:
                if sensor_izquierdo.reflection() < UMBRAL_NEGRO or sensor_derecho.reflection() < UMBRAL_NEGRO or sensor_central.reflection() < UMBRAL_NEGRO:
                    print("Línea detectada en la marcha atrás.")
                    break
                wait(5)
                
        # 3. AL ENCONTRARLA, ESTABILIZARSE
        robot.stop()
        wait(150) # Pausa para matar la inercia
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
