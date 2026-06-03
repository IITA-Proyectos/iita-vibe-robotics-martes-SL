from pybricks.hubs import PrimeHub
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port, Color
from pybricks.tools import wait, StopWatch

hub = PrimeHub()

# Configuración de Sensores
sensor_izquierdo = ColorSensor(Port.B)
sensor_central = ColorSensor(Port.D)
sensor_derecho = ColorSensor(Port.F)

hub.light.on(Color.BLUE)
print("=== PROGRAMA DE CALIBRACIÓN DE SENSORES ===")
print("Arrastrá el robot a mano sobre la pista de modo que")
print("los tres sensores pasen por la línea NEGRA y el fondo BLANCO.")
print("Presioná el BOTÓN IZQUIERDO del hub para empezar la calibración.")

# Esperar a que se presione un botón
while not hub.buttons.pressed():
    wait(10)

print("\n¡Calibrando! Mové el robot de lado a lado por la línea ahora...")
hub.light.on(Color.YELLOW)

reloj = StopWatch()

# Inicializamos los mínimos muy altos y los máximos muy bajos
min_izq = 100
max_izq = 0

min_cen = 100
max_cen = 0

min_der = 100
max_der = 0

# Calibrar durante 8 segundos (8000 ms)
while reloj.time() < 8000:
    val_izq = sensor_izquierdo.reflection()
    val_cen = sensor_central.reflection()
    val_der = sensor_derecho.reflection()
    
    # Actualizar Izquierdo
    if val_izq < min_izq: min_izq = val_izq
    if val_izq > max_izq: max_izq = val_izq

    # Actualizar Central
    if val_cen < min_cen: min_cen = val_cen
    if val_cen > max_cen: max_cen = val_cen

    # Actualizar Derecho
    if val_der < min_der: min_der = val_der
    if val_der > max_der: max_der = val_der
        
    wait(10)

hub.light.on(Color.GREEN)
print("\n=== ¡Calibración Terminada! ===")
print("Reemplazá los valores en tu programa del seguidor de línea por estos:")
print("="*40)
print(f"MIN_IZQ = {min_izq}")
print(f"MAX_IZQ = {max_izq}")
print(f"MIN_CEN = {min_cen}  # Usá esto para ajustar UMBRAL_NEGRO_CEN")
print(f"MAX_CEN = {max_cen}  # Usá esto para ajustar UMBRAL_BLANCO_CEN")
print(f"MIN_DER = {min_der}")
print(f"MAX_DER = {max_der}")
print("="*40)

print("\nTerminado. Revisá la terminal para ver los resultados.")
