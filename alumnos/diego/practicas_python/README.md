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

if que_falla in fallos_robot:
    print("fallo encontrado y arreglalo")

else:
    print("fallo no encontrado")

