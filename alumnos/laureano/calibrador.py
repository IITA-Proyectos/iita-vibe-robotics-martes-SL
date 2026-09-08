from pybricks.hubs import PrimeHub
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

hub = PrimeHub()

# Los sensores en los puertos que me indicaron
sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)
sensor_central = ColorSensor(Port.E)

print("=== PROGRAMA DE CALIBRACIÓN ===")
print("Poné el robot sobre el NEGRO y anotá los números estables.")
print("Luego ponelo sobre el BLANCO y anotá los números estables.")
print("---------------------------------")

while True:
    izq = sensor_izq.reflection()
    cen = sensor_central.reflection()
    der = sensor_der.reflection()
    
    # Imprime los 3 sensores para que tengan la data completa
    print("IZQ (C):", izq, " | CENTRO (E):", cen, " | DER (D):", der)
    
    wait(200) # Espera 0.2 segundos para no volvernos locos con los números
