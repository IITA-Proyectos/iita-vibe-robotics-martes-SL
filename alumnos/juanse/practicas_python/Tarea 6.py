"""
Creá una tupla llamada calibracion_luz con dos valores de intensidad 
de luz (el mínimo de la habitación apagada y el máximo con la linterna
prendida, por ejemplo: (20, 95)). Intentá cambiar el primer valor usando
calibracion_luz[0] = 30. 
¡Fijate cómo Python te frena al toque para proteger tu calibración!
"""
calibracion_luz = (24, 90)

#calibracion_luz[0] = 30

"""
Tenés que crear una tupla y una lista imaginando los convocados al mundial.

En la tupla tienen que estar los fijos que son los del cuerpo técnico (Scaloni, Aimar, etc.).
En la lista, el usuario debe ingresar con input() los 11 jugadores del equipo titular
(imaginando un 4-4-2 o la formación que prefieras). Al final, 
imprimí en la terminal el cuerpo técnico y los jugadores. Bien bonito.
"""
cuerpo_tecnico = ("Scaloni", "Aimar", "Walter Samuel", "Roberto Ayala")

convocados = []
print("A continuación, se le pedirá al usuario que ingrese los 11 jugadores de Argentina, o los que sepa.")
for jugador in range(0, 12):
    jugador = input("Ingresá un jugador: ")
    convocados.append(jugador)

print("Los integrantes del cuerpo técnico son: ")
print(cuerpo_tecnico[0])
print(cuerpo_tecnico[1])
print(cuerpo_tecnico[2])
print(cuerpo_tecnico[3])
print("Los jugadores seleccionados son: ")
print(convocados[0])
print(convocados[1])
print(convocados[2])
print(convocados[3])
print(convocados[4])
print(convocados[5])
print(convocados[6])
print(convocados[7])
print(convocados[8])
print(convocados[9])
print(convocados[10])
print(convocados[11])


"""
Creá un diccionario llamado mi_robot con tres claves: 'nombre', 'color' y 'ruedas'.
Poneles los valores que quieras.
Después, imprimí solo el nombre del robot usando la clave.
"""
mi_robot = {
    "nombre": "Stop 2.0",
    "color": "verde oscuro",
    "ruedas": "grandes"
}

print(mi_robot["nombre"])


"""
Usando el diccionario del punto 3, agregale una clave nueva que sea 'bateria' 
con el valor 100. Después, modificalo para que la batería baje a 80. 
Imprimí el diccionario completo para ver los cambios.
"""

mi_robot["bateria"] = 100
mi_robot["bateria"] = 80

print(mi_robot)

"""
Tenemos este inventario de stock guardado en un diccionario simple:

inventario = {
    'spike_hub': 5,
    'motor_grande': 12,
    'sensor_color': 8
}
Agregale un par de items más al inventario 
(por ejemplo, 'sensor_distancia': 10 y 'cable_usb': 15) 
y luego, pedile al usuario con input() el nombre del componente que desea 
buscar. Usá .get() para buscarlo en el diccionario. Si lo encuentra, 
imprimí la cantidad disponible en stock. Si no lo encuentra, imprimí 
un mensaje amigable que le notifique que no se encuentra registrado ese 
item en el stock.
"""
inventario = {
    'spike_hub': 5,
    'motor_grande': 12,
    'sensor_color': 8
}

inventario["bateria"] = 84
inventario["cables"] = 224

a_comprar = input("Buen día, ¿Qué está buscando en nuestra tienda lego?: ")
print("Tenemos de "+ a_comprar)
print(mi_robot.get(a_comprar))
print("unidad/es")

if a_comprar != "bateria" or "cables" or "sensor_color" or "motor_grande " or "spike_hub":
    print("Actualmente, no tenemos " + a_comprar + ", estamos haciendo lo necesario para que mañana ya dipongamos de ese producto. ")






