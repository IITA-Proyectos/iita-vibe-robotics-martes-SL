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
print(clacificadosA[0], 'vs' ,clacificadosA[1])

print("segundo partido de octavos :")
print(clacificadosB[0], 'vs' ,clacificadosB[1])

print("tercer partido de octavos :")
print(clacificadosC[0], 'vs' ,clacificadosC[1])

print("cuarto partido de octavos :")
print(clacificadosD[0], 'vs' ,clacificadosD[1])

print("quinto partido de octavos :")
print(clacificadosE[0], 'vs' ,clacificadosE[1])

print("sexto partido de octavos :")
print(clacificadosF[0], 'vs' ,clacificadosF[1])

