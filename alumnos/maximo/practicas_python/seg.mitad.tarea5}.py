personas_en_clase = ["Juanse", "Diego", "Maximo", "Laureano", "profe Franco y profe Gustavo"]

for nombre in personas_en_clase:
    print("Hola", nombre)

alertas = ['Batería baja','Sensor desconectado','Programación con errores','Bluetooth no conectado','USB no reconocido']

fallo = input("Ingrese un fallo: ")

if fallo in alertas:
    print("El fallo está en la lista")

else:
    print("El fallo no está en la lista")