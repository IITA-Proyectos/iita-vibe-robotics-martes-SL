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

# Velocidad base de avance (en milímetros por segundo). 
VELOCIDAD = 80  

# =======================================================
# CALIBRACIÓN DE SENSORES
# =======================================================
# Valores obtenidos de la fase 1 de calibración
MIN_IZQ = 10
MAX_IZQ = 59
MIN_DER = 8
MAX_DER = 49

# =======================================================
# CONSTANTES PID
# =======================================================
# Como normalizamos los sensores a un rango de 0-100, ahora el "Error" matemático 
# es el doble de grande (antes iba de 0 a 50, ahora de 0 a 100).
# Por ende, debemos Bajar el Kp a la mitad para compensar.
KP = 2.2        

# Constante Derivativa (Kd). Es el "freno" contra la inercia.
# Lo ajustamos a 5.0 por la misma razón que bajamos el KP.
KD = 9

# =======================================================

print("Modo: Seguidor de Línea Proporcional")
print("Presiona el botón izquierdo del HUB para arrancar.")

# Espera hasta que presiones el botón izquierdo del hub para arrancar y darte tiempo de acomodarlo.
while not hub.buttons.pressed():
    wait(10)

print("¡Arrancando!")
wait(500) # Pequeña pausa antes de arrancar

# Inicializamos el error previo para la derivada
error_previo = 0

# Variables para la lógica de recuperación
cronometro = StopWatch()
perdido = False
UMBRAL_BLANCO = 45 # Si leen mayor a 45, están en el fondo blanco
UMBRAL_NEGRO = 20  # Si lee menor a 20, es la línea negra

while True:
    ref_izq_bruto = sensor_izquierdo.reflection()
    ref_der_bruto = sensor_derecho.reflection()
    # Leemos el central para saber cuándo nos perdimos y cuándo la recuperamos
    ref_cen_bruto = sensor_central.reflection() 
    
    # =========================================================
    # LÓGICA DE RECUPERACIÓN (MARCHA ATRÁS)
    # =========================================================
    if ref_izq_bruto > UMBRAL_BLANCO and ref_der_bruto > UMBRAL_BLANCO and ref_cen_bruto > UMBRAL_BLANCO:
        if not perdido:
            perdido = True
            cronometro.reset() # Empezamos a contar la tolerancia
            
        elif cronometro.time() > 400: # Bajado a 400ms para que no se aleje tanto
            print("¡ME PERDÍ! Activando marcha atrás...")
            
            # Reversa recta y lenta
            robot.drive(-50, 0)
            
            # Nos quedamos en este bucle hasta que CUALQUIER sensor muerda la línea negra
            # (Antes era solo el central, lo que causaba que retrocediera demasiado si entraba de lado)
            while True:
                if sensor_central.reflection() < UMBRAL_NEGRO or sensor_izquierdo.reflection() < UMBRAL_NEGRO or sensor_derecho.reflection() < UMBRAL_NEGRO:
                    break
                wait(10)
                
            print("¡Línea encontrada de vuelta!")
            
            # 1. FRENAMOS TOTALMENTE EL ROBOT. Si no, la inercia de la marcha atrás lo saca otra vez.
            robot.stop()
            wait(200) # Una pequeñísima pausa para estabilizarse físicamente
            
            # 2. ReseteamosPID
            error_previo = 0
            perdido = False
            continue 
    else:
        perdido = False
    # =========================================================
    
    # 0. Normalizar los sensores (mapear de 0 a 100 de forma equitativa)
    # Esto elimina el problema de que el lado derecho corrija más fuerte que el izquierdo.
    val_izq = (ref_izq_bruto - MIN_IZQ) / (MAX_IZQ - MIN_IZQ) * 100
    val_der = (ref_der_bruto - MIN_DER) / (MAX_DER - MIN_DER) * 100
    
    # 1. Calcular el Error simétrico
    error = val_izq - val_der
    
    # 2. Calcular la Derivada (La velocidad a la que cambia el error)
    derivada = error - error_previo
    
    # 3. Calcular la Magnitud de la Corrección (P + D)
    correccion = (error * KP) + (derivada * KD)
    
    # 4. Guardar el error para el siguiente ciclo
    error_previo = error
    
    # 5. Aplicar motores
    robot.drive(VELOCIDAD, correccion)
    
    # Pausa super corta recomendada por Pybricks para no sobrecargar el bus de datos
    wait(10)
