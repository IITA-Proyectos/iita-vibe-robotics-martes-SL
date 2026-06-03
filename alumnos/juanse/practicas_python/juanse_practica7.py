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

GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"]
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"]
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"]
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"]
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"]
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"]
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"]
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]

print("Los grupos del mundial 2022 son:")

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


print(GrupoA)

indice = input("Ingrese el primer equipo clasificado del grupo A: ")
indice = int(indice)
indice2 = input("Ingrese el segundo equipo clasificado del grupo A: ")
indice2 = int(indice2)

octavos = []
octavos.append(GrupoA[indice])
octavos.append(GrupoA[indice2])
#print(octavos)

print(GrupoB)
indice3 = input("Ingrese el primer equipo clasificado del grupo B: ")
indice3 = int(indice3)
indice4 = input("Ingrese el segundo equipo clasificado del grupo B: ")
indice4 = int(indice4)

octavos.append(GrupoB[indice3])
octavos.append(GrupoB[indice4])

print(GrupoC)
indice5 = input("Ingrese el primer equipo clasificado del grupo C: ")
indice5 = int(indice5)
indice6 = input("Ingrese el segundo equipo clasificado del grupo C: ")
indice6 = int(indice6)

octavos.append(GrupoC[indice5])
octavos.append(GrupoC[indice6])

print(GrupoD)

indice7 = input("Ingrese el primer equipo clasificado del grupo D: ")
indice7 = int(indice7)
indice8 = input("Ingrese el segundo equipo clasificado del grupo D: ")
indice8 = int(indice8)

octavos.append(GrupoD[indice7])
octavos.append(GrupoD[indice8])

print(GrupoE)
indice9 = input("Ingrese el primer equipo clasificado del grupo E: ")
indice9 = int(indice9)
indice10 = input("Ingrese el segundo equipo clasificado del grupo E: ")
indice10 = int(indice10)

octavos.append(GrupoE[indice9])
octavos.append(GrupoE[indice10])

print(GrupoF)
indice11 = input("Ingrese el primer equipo clasificado del grupo F: ")
indice11 = int(indice11)
indice12 = input("Ingrese el segundo equipo clasificado del grupo F: ")
indice12 = int(indice12)

octavos.append(GrupoF[indice11])
octavos.append(GrupoF[indice12])

print(GrupoG)
indice13 = input("Ingrese el primer equipo clasificado del grupo G: ")
indice13 = int(indice13)
indice14 = input("Ingrese el segundo equipo clasificado del grupo G: ")
indice14 = int(indice14)

octavos.append(GrupoG[indice13])
octavos.append(GrupoG[indice14])

print(GrupoH)
indice15 = input("Ingrese el primer equipo clasificado del grupo H: ")
indice15 = int(indice15)
indice16 = input("Ingrese el segundo equipo clasificado del grupo H: ")
indice16 = int(indice16)

octavos.append(GrupoH[indice15])
octavos.append(GrupoH[indice16])

print(octavos)
contador = 0
for i in range (0,8):
    print(octavos[contador] + octavos[contador+1])
    contador = contador + 2
    





























