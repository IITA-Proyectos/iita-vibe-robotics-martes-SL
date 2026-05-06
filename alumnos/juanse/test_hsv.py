from pybricks.hubs import PrimeHub
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

hub = PrimeHub()

# Configuración de Sensores
sensor_izquierdo = ColorSensor(Port.B)
sensor_derecho = ColorSensor(Port.F)

print("=========================================")
print("       TEST DE LECTURA HSV (VERDES)      ")
print("=========================================")
print("Instrucciones:")
print("1. Colocá los sensores sobre el verde.")
print("2. Presioná cualquier botón del Hub para iniciar.")
print("=========================================")

# Esperar a que presionen un botón para arrancar
while not hub.buttons.pressed():
    wait(10)

print("¡Arrancando lectura en vivo!")
print("Movelo por la pista para ver los valores.")
print("(Presioná nuevamente cualquier botón para salir)")
wait(1000) # Pausa para que suelten el botón

while True:
    # Si presionan un botón, sale del programa
    if hub.buttons.pressed():
        break
        
    # Leemos el HSV de ambos sensores
    hsv_i = sensor_izquierdo.hsv()
    hsv_d = sensor_derecho.hsv()
    
    # Imprimimos Matiz (H), Saturación (S) y Valor (V) de manera prolija
    # H: Hue (Matiz) - Es el número clave para saber el "tipo" de color.
    # S: Saturation - Qué tan "fuerte" o lavado es el color.
    # V: Value - Qué tan iluminado/brillante está.
    print(f"IZQ -> H:{hsv_i.h:3} S:{hsv_i.s:3} V:{hsv_i.v:3}  ||  DER -> H:{hsv_d.h:3} S:{hsv_d.s:3} V:{hsv_d.v:3}")
    
    # Pausa para que no sature la consola y se pueda leer
    wait(300)
    
print("Test finalizado.")
