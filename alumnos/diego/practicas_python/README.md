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

-----------------------------------------------------------------------------------------------------------------------

# Pracrtica 7

'''
Diario El Tribuno se contactó con cada uno de ustedes para que les realicen el
programa del fixture del Mundial de este año.
Debes realizar un programa para el fixture del Mundial 2022, para que tengas todos
los resultados en Google
Necesitamos un programa que nos ayude a ir siguiendo el Mundial.
Los grupos del año 2022 son:
'''
'''GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"],
   GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"],
   GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"],
   GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"],
   GrupoE = ["España", "Costa Rica", "Alemania", "Japón"],
   GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"],
   GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"],
   GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]'''
'''La idea es que primero se le muestre al usuario TODOS los grupos del mundial.
Luego, con un input el usuario debe ingresar cual fue el primero y el segundo de cada grupo.
Por ejemplo:'''

'''
→ GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"],
→ Ingrese el indice del primer equipo del grupo A: → Ecuador
→ Ingrese el indice del segundo equipo del grupo A: → Países Bajos
Después, en una lista vacia, tienen que añadir o agregar cada equipo que pasa a la
siguiente ronda.
octavos = ['Ecuador', 'Países Bajos']
Ejemplo:
print("Primer partido de octavos")
print(octavos[0], "vs", octavos[1])
cuartos = []
ganador_partido = input("Ingrese el equipo que ganó:") → Por ejemplo: Ecuador
cuartos.append(ganador_partido)
El programa debe repetir esta acción de pedir ganador del partido y guardar
a los ganadores de los 8 partidos de octavos en la lista "cuartos".
Y así hasta llegar a la final
Que quede bonito
Suerte
'''

def mostrar_bienvenida():
    print("============================================")
    print("    ¡BIENVENIDO AL FIXTURE DEL MUNDIAL!")
    print("============================================")

print(mostrar_bienvenida)

def separador():
    print("vs")

def festejar_campeon():
    print("🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆")
    print("🎉¡ATENCION! ¡TENEMOS UN NUEVO CAMPEON!🎉")
    print("👏👏👏👏👏👏💥💥💥💥💥💥🎇🎇🎇🎇🎇🎇")

def mostrar_menu():
    print('--- MENÚ DEL FIXTURE ---')
    print('1. Ver grupos')
    print("2. Definir octavos")
    print("3. Definir cuartos")
    print("4. Salir")
    print("------------------------")


GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"]
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"]
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"]
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"]
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"]
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"]
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"]
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]

print("Grupo A ")
print(GrupoA)
print("Grupo B ")
print(GrupoB)
print("Grupo C ")
print(GrupoC)
print("Grupo D ")
print(GrupoD)
print("Grupo E ")
print(GrupoE)
print("Grupo F ")
print(GrupoF)
print("Grupo G ")
print(GrupoG)
print("Grupo H ")
print(GrupoH)

print("-------------------------------------------------")
print(GrupoA)

clacificadosA = []

indice1 = input("ingrese el indice del primer equipo del grupo A :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo A :")
indice2 = int(indice2)

clacificadosA.append(GrupoA[indice1])
clacificadosA.append(GrupoA[indice2])

print(clacificadosA)

print("-------------------------------------------------")
print(GrupoB)

clacificadosB = []

indice1 = input("ingrese el indice del primer equipo del grupo B :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo B :")
indice2 = int(indice2)

clacificadosB.append(GrupoB[indice1])
clacificadosB.append(GrupoB[indice2])

print(clacificadosB)


print("-------------------------------------------------")
print(GrupoC)

clacificadosC = []

indice1 = input("ingrese el indice del primer equipo del grupo C :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo C :")
indice2 = int(indice2)

clacificadosC.append(GrupoC[indice1])
clacificadosC.append(GrupoC[indice2])

print(clacificadosC)


print("-------------------------------------------------")
print(GrupoD)

clacificadosD = []

indice1 = input("ingrese el indice del primer equipo del grupo D :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo D :")
indice2 = int(indice2)

clacificadosD.append(GrupoD[indice1])
clacificadosD.append(GrupoD[indice2])

print(clacificadosA)


print("-------------------------------------------------")
print(GrupoE)

clacificadosE = []

indice1 = input("ingrese el indice del primer equipo del grupo E :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo E :")
indice2 = int(indice2)

clacificadosE.append(GrupoE[indice1])
clacificadosE.append(GrupoE[indice2])

print(clacificadosE)

print("-------------------------------------------------")
print(GrupoF)

clacificadosF = []

indice1 = input("ingrese el indice del primer equipo del grupo F :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo F :")
indice2 = int(indice2)

clacificadosF.append(GrupoF[indice1])
clacificadosF.append(GrupoF[indice2])

print(clacificadosF)

print("-------------------------------------------------")
print(GrupoG)

clacificadosG = []

indice1 = input("ingrese el indice del primer equipo del grupo G :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo G :")
indice2 = int(indice2)

clacificadosG.append(GrupoG[indice1])
clacificadosG.append(GrupoG[indice2])

print(clacificadosG)

print("-------------------------------------------------")
print(GrupoH)

clacificadosH = []

indice1 = input("ingrese el indice del primer equipo del grupo H :")
indice1 = int(indice1)
indice2 = input("Ingrese el indice del segundo equipo del grupo H :")
indice2 = int(indice2)

clacificadosH.append(GrupoH[indice1])
clacificadosH.append(GrupoH[indice2])

print(clacificadosH)

print("primer partido de octavos :")
print(clacificadosA[0], separador() ,clacificadosA[1])

print("segundo partido de octavos :")
print(clacificadosB[0], separador() ,clacificadosB[1])

print("tercer partido de octavos :")
print(clacificadosC[0], separador() ,clacificadosC[1])

print("cuarto partido de octavos :")
print(clacificadosD[0], separador() ,clacificadosD[1])

print("quinto partido de octavos :")
print(clacificadosE[0], separador() ,clacificadosE[1])

print("sexto partido de octavos :")
print(clacificadosF[0], separador() ,clacificadosF[1])

print("septimo partido de octavos :")
print(clacificadosG[0], separador() ,clacificadosG[1])

print("octavo partido de octavos :")
print(clacificadosH[0], separador() ,clacificadosH[1])

print('-----------------------------------------------------------')

cuartos = []

indice1 = input("ingrese el indice del ganador del primer partido de octavos :")
indice1 = int(indice1)

cuartos.append(indice1)

print(cuartos)

print('-----------------------------------------------------------')

indice2 = input("ingrese el indice del segundo ganador del partido de octavos :")
indice2 = int(indice2)

cuartos.append(indice2)

print(cuartos)

print('-----------------------------------------------------------')

indice3 = input("ingrese el indice del tercer ganador del partido de octavos :")
indice3 = int(indice3)

cuartos.append(indice3)

print(cuartos)

print('-----------------------------------------------------------')

indice4 = input("ingrese el indice del cuarto ganador del partido de octavos :")
indice4 = int(indice4)

cuartos.append(indice4)

print(cuartos)


print('-----------------------------------------------------------')

indice5 = input("ingrese el indice del quinto ganador del partido de octavos :")
indice5 = int(indice5)

cuartos.append(indice5)


print(cuartos)
print('-----------------------------------------------------------')

indice6 = input("ingrese el indice del sexto ganador del partido de octavos :")
indice6 = int(indice6)

cuartos.append(indice6)

print(cuartos)


print('-----------------------------------------------------------')

indice7 = input("ingrese el indice del septimo ganador del partido de octavos :")
indice7 = int(indice7)

cuartos.append(indice7)

print(cuartos)

print('-----------------------------------------------------------')

indice8 = input("ingrese el indice del octavo ganador del partido de octavos :")
indice8 = int(indice8)

cuartos.append(indice8)

print(cuartos)

print(cuartos[0], separador() ,cuartos[1])
print(cuartos[2], separador() ,cuartos[3])
print(cuartos[4], separador() ,cuartos[5])
print(cuartos[6], separador() ,cuartos[7])

print('-----------------------------------------------------------')

semifinales = []

indice1 = input("ingrese el primer ganador del partido de cuartos :")
indice1 = int(indice1)

semifinales.append(indice1)

print('-----------------------------------------------------------')

indice2 = input("ingrese el segundo ganador del partido de cuartos :")
indice2 = int(indice2)

semifinales.append(indice2)

print('-----------------------------------------------------------')

indice3 = input("ingrese el tercer ganador del partido de cuartos :")
indice3 = int(indice3)

semifinales.append(indice3)

print('-----------------------------------------------------------')


indice4 = input("ingrese el cuarto ganador del partido de cuartos :")
indice4 = int(indice4)

semifinales.append(indice4)

print(semifinales)

print(cuartos[0], separador() ,cuartos[1])
print(cuartos[2], separador() ,cuartos[3])

print('-----------------------------------------------------------')

final = []

indice1 = input("ingrese el primer ganador del partido de semifinales :")
indice1 = int(indice1)

semifinales.append(indice1)

print('-----------------------------------------------------------')

indice2 = input("ingrese el segundo ganador del partido de semifinales :")
indice2 = int(indice2)

semifinales.append(indice2)

print(final)

print(final[0], separador() ,final[1])

campeon = input("INGRESE EL INDICE DE CAMPEON :")

festejar_campeon()


mostrar_menu()


# Practica 8

def reportar_motores():
    print("Motores inicializados correctamente")
    print("Estado: Activos y listos")

# Llamamos a la función
reportar_motores()


if que_falla in fallos_robot:
    print("fallo encontrado y arreglalo")

else:
    print("fallo no encontrado")

