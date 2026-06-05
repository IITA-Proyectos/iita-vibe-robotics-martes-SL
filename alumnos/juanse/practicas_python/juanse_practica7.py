"""
Diario El Tribuno se contactó con cada uno de ustedes para que les realicen el
programa del fixture del Mundial de este año.
Debes realizar un programa para el fixture del Mundial 2022, para que tengas todos
los resultados en Google
Necesitamos un programa que nos ayude a ir siguiendo el Mundial.
Los grupos del año 2022 son:
GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"],
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"],
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"],
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"],
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"],
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"],
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"],
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]
La idea es que primero se le muestre al usuario TODOS los grupos del mundial.
Luego, con un input el usuario debe ingresar cual fue el primero y el segundo de cada grupo.
Por ejemplo:
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
"""
import time

GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"]
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"]
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"]
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"]
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"]
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"]
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"]
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]

print("Los grupos del mundial 2022 son:")
time.sleep(1)

print("Grupo A")
for seleccion in GrupoA:
    print(seleccion)

print("   ")

print("Grupo B")
for seleccion in GrupoB:
    print(seleccion)

print("   ")

print("Grupo C")
for seleccion in GrupoC:
    print(seleccion)

print("   ")

print("Grupo D")
for seleccion in GrupoD:
    print(seleccion)

print("   ")

print("Grupo E")
for seleccion in GrupoE:
    print(seleccion)

print("   ")

print("Grupo F")
for seleccion in GrupoF:
    print(seleccion)

print("   ")

print("Grupo G")
for seleccion in GrupoG:
    print(seleccion)

print("   ")

print("Grupo H")
for seleccion in GrupoH:
    print(seleccion)

time.sleep(2)
print("Grupo A")
print(GrupoA)

indice = input("Ingrese el primer equipo clasificado del grupo A: ")
indice = int(indice)
indice2 = input("Ingrese el segundo equipo clasificado del grupo A: ")
indice2 = int(indice2)

octavos = []
octavos.append(GrupoA[indice])
octavos.append(GrupoA[indice2])
#print(octavos)

print("Grupo B")
print(GrupoB)
indice3 = input("Ingrese el primer equipo clasificado del grupo B: ")
indice3 = int(indice3)
indice4 = input("Ingrese el segundo equipo clasificado del grupo B: ")
indice4 = int(indice4)

octavos.append(GrupoB[indice3])
octavos.append(GrupoB[indice4])

print("Grupo C")
print(GrupoC)
indice5 = input("Ingrese el primer equipo clasificado del grupo C: ")
indice5 = int(indice5)
indice6 = input("Ingrese el segundo equipo clasificado del grupo C: ")
indice6 = int(indice6)

octavos.append(GrupoC[indice5])
octavos.append(GrupoC[indice6])

print("Grupo D")
print(GrupoD)

indice7 = input("Ingrese el primer equipo clasificado del grupo D: ")
indice7 = int(indice7)
indice8 = input("Ingrese el segundo equipo clasificado del grupo D: ")
indice8 = int(indice8)

octavos.append(GrupoD[indice7])
octavos.append(GrupoD[indice8])

print("Grupo E")
print(GrupoE)
indice9 = input("Ingrese el primer equipo clasificado del grupo E: ")
indice9 = int(indice9)
indice10 = input("Ingrese el segundo equipo clasificado del grupo E: ")
indice10 = int(indice10)

octavos.append(GrupoE[indice9])
octavos.append(GrupoE[indice10])

print("Grupo F")
print(GrupoF)
indice11 = input("Ingrese el primer equipo clasificado del grupo F: ")
indice11 = int(indice11)
indice12 = input("Ingrese el segundo equipo clasificado del grupo F: ")
indice12 = int(indice12)

octavos.append(GrupoF[indice11])
octavos.append(GrupoF[indice12])

print("Grupo G")
print(GrupoG)
indice13 = input("Ingrese el primer equipo clasificado del grupo G: ")
indice13 = int(indice13)
indice14 = input("Ingrese el segundo equipo clasificado del grupo G: ")
indice14 = int(indice14)

octavos.append(GrupoG[indice13])
octavos.append(GrupoG[indice14])

print("Grupo H")
print(GrupoH)
indice15 = input("Ingrese el primer equipo clasificado del grupo H: ")
indice15 = int(indice15)
indice16 = input("Ingrese el segundo equipo clasificado del grupo H: ")
indice16 = int(indice16)

octavos.append(GrupoH[indice15])
octavos.append(GrupoH[indice16])

time.sleep(1)
print("Los equipos que pasaron a octavos son: ")
print(octavos)

print("   ")
print("Los partidos de octavos son: ")
time.sleep(1)

contador = 0
for i in range (0,8):
    print(octavos[contador] + " vs " + octavos[contador+1])
    contador = contador + 2

cuartos = []

print("   ")
print("Primer partido de octavos:")
print(octavos[0], "vs", octavos[1])

ganador_partido = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido)


print("   ")
print("Segundo partido de octavos:")
print(octavos[2], "vs", octavos[3])

ganador_partido2 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido2)


print("   ")
print("Tercer partido de octavos:")
print(octavos[4], "vs", octavos[5])

ganador_partido3 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido3)


print("   ")
print("Cuarto partido de octavos:")
print(octavos[6], "vs", octavos[7])

ganador_partido4 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido4)


print("   ")
print("Quinto partido de octavos:")
print(octavos[8], "vs", octavos[9])

ganador_partido5 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido5)


print("   ")
print("Sexto partido de octavos:")
print(octavos[10], "vs", octavos[11])

ganador_partido6 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido6)


print("   ")
print("Séptimo partido de octavos:")
print(octavos[12], "vs", octavos[13])

ganador_partido7 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido7)


print("   ")
print("Octavo partido de octavos:")
print(octavos[14], "vs", octavos[15])

ganador_partido8 = input("Ingrese el equipo que ganó: ")
cuartos.append(ganador_partido8)


print("Los equipos que pasaron a cuartos son:")
print(cuartos)

print("   ")
print("Los partidos de cuartos son: ")
time.sleep(1)

contador = 0
for i in range (0,4):
    print(cuartos[contador] + " vs " + cuartos[contador+1])
    contador = contador + 2

semis = []

print("   ")
print("Primer partido de cuartos:")
print(cuartos[0], "vs", cuartos[1])

ganador_partidoB = input("Ingrese el equipo que ganó: ")
semis.append(ganador_partidoB)

print("   ")
print("Segundo partido de cuartos:")
print(cuartos[2], "vs", cuartos[3])

ganador_partidoA = input("Ingrese el equipo que ganó: ")
semis.append(ganador_partidoA)

print("   ")
print("Tercer partido de cuartos:")
print(cuartos[4], "vs", cuartos[5])

ganador_partidoE = input("Ingrese el equipo que ganó: ")
semis.append(ganador_partidoE)

print("   ")
print("Cuarto partido de cuartos:")
print(cuartos[6], "vs", cuartos[7])

ganador_partidoF = input("Ingrese el equipo que ganó: ")
semis.append(ganador_partidoF)


print("Los que pasaron a semifinales son:")
print(semis)

print("   ")
print("Los partidos de semifinales son: ")
time.sleep(1)

contador = 0
for i in range (0,2):
    print(semis[contador] + " vs " + semis[contador+1])
    contador = contador + 2

final =[]

print("   ")
print("Primer partido de semifinal:")
print(semis[0], "vs", semis[1])

ganador_partidoC = input("Ingrese el equipo que ganó: ")
final.append(ganador_partidoC)

print("   ")
print("Segundo partido de semifinal:")
print(semis[2], "vs", semis[3])

ganador_partidoD = input("Ingrese el equipo que ganó: ")
final.append(ganador_partidoD)

print("   ")
print("El partido de la final es: ")
time.sleep(1)

print (final[0] + " vs " + final[1])
print("¿Quién gano?")
ganador_final = input("Ingrese el campeón: ")
print("¡CONGRATULATIONS!")
print("    ")
print("LISTO, HASTA AQUÍ HA SIDO EL FIXTURE DE ESTE MUNDIAL :)")

