
calibracion_luz = (20, 95)

"calibracion_luz[0] = 30"
# 2

cuerpo_tecnico = ("Scaloni", "Aimar", "Samuel", "Ayala")

jugadores = []

for i in range(11):
    jugador = input("Ingrese un jugador: ")
    jugadores.append(jugador)
print("Los convocados de Scaloni son:")
print(cuerpo_tecnico)
print(jugadores)
# 3

mi_robot = {
    "nombre": "Robotito",
    "color": "Rojo",
    "ruedas": 4
}

print(mi_robot["nombre"])
# 4

mi_robot = {
    "nombre": "Robotito",
    "color": "Rojo",
    "ruedas": 4
}

mi_robot["bateria"] = 100

mi_robot["bateria"] = 80

print(mi_robot)
# 5
print("buenos dias que piezas tenes en stock ??")

inventario = {
    "spike_hub": 5,
    "motor_grande": 12,
    "sensor_color": 8,
    "sensor_distancia": 10,
    "cable_usb": 15
}
print(inventario)
componente = input()

cantidad = inventario.get(componente)

if cantidad:
    print(cantidad)
else:
    print("No se encuentra registrado ese item en el stock.")