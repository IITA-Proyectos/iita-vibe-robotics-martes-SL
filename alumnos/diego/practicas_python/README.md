# Practica 5

materiales_robot = ['motor','rueda','sensor','piezas lego','cables']
print(materiales_robot[3])

acciones = []

acciones.append('moverse')
acciones.append('giro')
acciones.append('medir')
acciones.append('detenerse')

ultimas_acciones = acciones.pop(3)
ultimas_acciones = acciones.pop(2)

print(ultimas_acciones)
print(acciones)

nombres_persona = ['profefranco','prfegustavo','laureano','juanse','maximo','diego']

for nombres in nombres_persona:
    print('hola', nombres)

fallos_robot = ['Batería baja', 'Sensor desconectado', 'Programación con errores', 'Bluetooth no conectado','USB no reconocido']

que_falla = input("ingrese un fallo :")
que_falla = str(que_falla)


# Practica 6

calibracion_luz = (21, 90)

'calibracion_luz[0] = 23'



cuerpo_tecnico = ("Scaloni", "Aimar", "Walter Samuel", "Roberto Ayala",)

convocados = []
for jugador in range(0, 12):
    jugador = input("Ingresá un jugador por posicion: ")
    convocados.append(jugador)

print("El cuerpo técnico: ")
print(cuerpo_tecnico[0])
print(cuerpo_tecnico[1])
print(cuerpo_tecnico[2])
print(cuerpo_tecnico[3])
print("Los 11 titulares: ")
print('arquero :',convocados[0])
print('laterar derecho :',convocados[1])
print('central 6 :',convocados[2])
print('central 2 :',convocados[3])
print('laterar izquierdo :',convocados[4])
print('mediocampista defencivo :',convocados[5])
print('mediocampista central :',convocados[6])
print('mediocampista central :',convocados[7])
print('mediocampista ofencivo :',convocados[8])
print('delantaro (extremo) :',convocados[9])
print('delantero 9 :',convocados[10])
print('delantaro (extremo) :',convocados[11])



el_robot = {
    "nombre": "Messi 1.0",
    "color": "celeste , blanco",
    "ruedas": "grandes"
}

print(el_robot["nombre"])



el_robot["bateria"] = 100
el_robot["bateria"] = 1

print(el_robot)



inventario = {
    'spike_hub': 5,
    'motor_grande': 12,
    'sensor_color': 8
}

inventario["sensor_distancia"] = 10
inventario["bateria"] = 50

que_comprar = input("Hola que desea comprar : ")
print(el_robot.get(que_comprar))
print("tenemos",que_comprar)
if que_comprar != "bateria" or "cables" or "sensor_color" or "motor_grande " or "spike_hub":
    print("Lo sentimos no tenemos " + que_comprar + ",pero pude venir cuando lo tengamos disponible. ")




if que_falla in fallos_robot:
    print("fallo encontrado y arreglalo")

else:
    print("fallo no encontrado")

