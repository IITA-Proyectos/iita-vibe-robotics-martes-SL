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

# Umbrales
UMBRAL_BLANCO = 40  
UMBRAL_NEGRO = 20   

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
        print("¡Intersección detectada algo cruzada! Acomodándose...")
        # BEEP 1 (Tono grave): Avisa que vio una cruz o T por primera vez
        hub.speaker.beep(400, 100) 
        
        # A. Retroceder un poco para salir de la barra horizontal de la "T"
        robot.reset()
        robot.drive(-60, 0) # 6 cm hacia atrás (como modificaste vos)
        while robot.distance() > -60:
            wait(5)
            
        # BEEP 2 (Tono medio): Fin de la marcha atrás, arranca el mini-PID para alinearse
        hub.speaker.beep(600, 100)
            
        # B. Volver a avanzar muy lentamente "siguiendo la línea" para obligarse a enderezarse.
        while True:
            r_izq = sensor_izquierdo.reflection()
            r_cen = sensor_central.reflection()
            r_der = sensor_derecho.reflection()
            
            # Si volvemos a tocar la T de frente salimos porque ya estamos rectos
            if r_cen < UMBRAL_NEGRO and (r_izq < UMBRAL_NEGRO or r_der < UMBRAL_NEGRO):
                # BEEP 3 (Tono agudo): Se alineó y está tocando de nuevo la T de frente
                hub.speaker.beep(800, 100) 
                break
                
            # Hacemos un mini PID para seguir la línea lentamente y corregir la postura
            v_i = (r_izq - MIN_IZQ) / (MAX_IZQ - MIN_IZQ) * 100
            v_d = (r_der - MIN_DER) / (MAX_DER - MIN_DER) * 100
            mini_error = v_i - v_d
            robot.drive(30, mini_error * KP)
            wait(5)
            
        # C. Una vez perfectamente acomodado y de frente a la T, cruzamos con confianza
        print("Acomodado perfectamente. Cruzando intersección recta...")
        robot.reset()
        robot.drive(VELOCIDAD, 0)
        
        # Avanzamos unos 6 cm (60 mm) para atravesar firmemente la línea de casi 2 cm
        while robot.distance() < 60:
            wait(5)
            
        # D. Una vez sorteado el cruce termina y sigue su curso normal
        # BEEP 4 (Tono más agudo y de éxito): T superada
        hub.speaker.beep(1000, 150) 
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
