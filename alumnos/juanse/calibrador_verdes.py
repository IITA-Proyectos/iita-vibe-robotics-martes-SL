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

# Umbrales de Línea
UMBRAL_BLANCO = 40  
UMBRAL_NEGRO = 30   # Aumentado de 20 a 30 para que detecte el negro en movimiento rápido

# =======================================================
# PARÁMETROS DE COLOR (HSV PARA VERDES)
# =======================================================
# Calibrados en base a las lecturas en pista
HUE_MIN = 155
HUE_MAX = 185
SAT_MIN = 55
VAL_MIN = 15

def es_verde(hsv):
    """Evalúa si un color HSV entra en el rango de verde configurado."""
    return (HUE_MIN <= hsv.h <= HUE_MAX and 
            hsv.s >= SAT_MIN and 
            hsv.v >= VAL_MIN)

# =======================================================

print("Calibrador de Verdes con Seguidor de Línea")
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
    # 1. LÓGICA DE VERDES (PRIORIDAD ABSOLUTA)
    # =========================================================
    hsv_i = sensor_izquierdo.hsv()
    hsv_d = sensor_derecho.hsv()
    
    verde_izq = es_verde(hsv_i)
    verde_der = es_verde(hsv_d)
    
    if verde_izq or verde_der:
        robot.stop()
        # --- MINI VERIFICACIÓN PARA DOBLE VERDE ---
        if verde_izq and not verde_der:
            print("Verificando posible doble verde (avanzando motor derecho)...")
            ang_ini = motor_derecho.angle()
            motor_derecho.run(150)
            while motor_derecho.angle() - ang_ini < 60:
                hsv_d_nuevo = sensor_derecho.hsv()
                if es_verde(hsv_d_nuevo):
                    verde_der = True
                    hsv_d = hsv_d_nuevo # Actualizamos para el print
                    break
                wait(5)
            motor_derecho.stop()
                
        elif verde_der and not verde_izq:
            print("Verificando posible doble verde (avanzando motor izquierdo)...")
            ang_ini = motor_izquierdo.angle()
            motor_izquierdo.run(150)
            while motor_izquierdo.angle() - ang_ini < 60:
                hsv_i_nuevo = sensor_izquierdo.hsv()
                if es_verde(hsv_i_nuevo):
                    verde_izq = True
                    hsv_i = hsv_i_nuevo # Actualizamos para el print
                    break
                wait(5)
            motor_izquierdo.stop()
        # -------------------------------------------
        
        if verde_izq and verde_der:
            print(f"¡Doble Verde! H_i:{hsv_i.h} H_d:{hsv_d.h} -> Media vuelta (180°)")
            hub.speaker.beep(2000, 200)
            robot.straight(40) # Centrar el robot
            robot.turn(180)    # Media vuelta
            robot.straight(40) # Avanzar un poco más después de los 180 grados
            
        elif verde_izq:
            print(f"¡Verde Izquierdo! H_i:{hsv_i.h} -> Giro de 90° Izquierda")
            hub.speaker.beep(1500, 100)
            robot.straight(40) 
            robot.turn(-90)
            robot.straight(30) # Avanzar para reenganchar la línea
            
        elif verde_der:
            print(f"¡Verde Derecho! H_d:{hsv_d.h} -> Giro de 90° Derecha")
            hub.speaker.beep(1500, 100)
            robot.straight(40)
            robot.turn(90)
            robot.straight(30)
            
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
