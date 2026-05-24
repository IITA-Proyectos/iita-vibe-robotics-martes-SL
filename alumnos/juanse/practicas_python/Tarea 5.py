import time

#  Crea una lista con 5 materiales que necesitaríamos para armar
# nuestro robot. Después, imprimí por pantalla el tercer material
# de nuestra lista usando los índices.

materiales = ["motor", "ruedas", "piezas lego", "sensores", "hub"]
print(materiales[2])

"""
Empecemos con una lista vacía acciones = []. Usando .append() 
agregá todas las acciones que creas necesarias para que un robot 
realice un cuadrado de 30cm en el piso. Al terminar imprimí la lista
. Después de todo eso, usá .pop() para eliminar las últimas 2 acciones
 y luego imprimí la lista de nuevo.
"""

acciones = []

acciones.append("avanzar 30cm")
acciones.append("girar 90°")
acciones.append("avanzar 30cm")
acciones.append("girar 90°")
acciones.append("avanzar 30cm")
acciones.append("girar 90°")
acciones.append("avanzar 30cm")

print(acciones)
time.sleep(1)

ultimas_dos_acciones = acciones.pop(5)
time.sleep(1)
ultimas_dos_acciones = acciones.pop(5)

print ("Lista nueva: " + str(acciones))

"""
Crea una lista con los nombres de todas las personas en el aula.
Luego, recorre la lista con el ciclo for para imprimir un saludo
personalizado para cada uno, como si recién llegaras a la clase.
"""

personas = ["Máximo", "Laureano", "Diego", "Franco", "Gustavo", "Juanse"]

for nombre in personas:
    print("Hola " + nombre + ", como estás.")

"""
Tenés una lista de
alertas = ['Batería baja', 'Sensor desconectado', 
'Programación con errores', 'Bluetooth no conectado',
'USB no reconocido']. Hacé un programa que pregunte al usuario
un fallo y use in para verificar si está en la lista de alertas.
"""

alertas = ['Batería baja', 'Sensor desconectado', 'Programación con errores', 'Bluetooth no conectado','USB no reconocido']

que_pasa = input("¿Que está fallando? (Ingrese un fallo): ")
que_pasa = str(que_pasa)

if que_pasa in alertas:
    print("Alerta detectada, prosiga con el protocolo adecuado.")

else:
    print("Fallo no encontrado.")

