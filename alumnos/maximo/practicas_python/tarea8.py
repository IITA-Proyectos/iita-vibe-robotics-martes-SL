def mostrar_bienvenida():
    print("======================================")
    print("¡BIENVENIDO AL FIXTURE DEL MUNDIAL!")
    print("======================================")


def separador():
    print("********** VS **********")


def festejar_campeon():
    print("🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆")
    print("🎉 ¡ATENCIÓN! ¡TENEMOS UN NUEVO CAMPEÓN! 🎉")
    print("⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐")


def mostrar_menu():
    print("--- MENÚ DEL FIXTURE ---")
    print("1. Ver grupos")
    print("2. Definir octavos")
    print("3. Definir cuartos")
    print("4. Salir")
    print("------------------------")

mostrar_bienvenida()
mostrar_menu()

# NOTA:
# Debido al tamaño del programa original, copia aquí el resto de tu código.
# Asegúrate de aplicar estas dos correcciones:
#
# ganadores_grupo_F.append(GrupoF[primerlugarF])
# ganadores_grupo_F.append(GrupoF[segundolugarF])
#
# ganadores_grupo_G.append(GrupoG[primerlugarG])
# ganadores_grupo_G.append(GrupoG[segundolugarG])
#
# Reemplaza print("----------------------------------")
# por separador()
#
# Y antes del print final del campeón:
#
# festejar_campeon()
# print("Ha Ganado el mundial 2022:", ganadoresfinal)
mostrar_bienvenida()
mostrar_menu()
GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"]
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"]
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"]
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"]
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"]
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"]
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"]
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]


print("----------------------------------")
print(GrupoA)
primerlugar = input('ingrese el primer puesto del grupo A: ')
segundolugar = input('ingrese el segundo puesto del grupo A: ')
primerlugar = int(primerlugar)
segundolugar = int(segundolugar)

grupoAGanadores = []
grupoAGanadores.append(GrupoA[primerlugar])
grupoAGanadores.append(GrupoA[segundolugar])

print(grupoAGanadores)

print("----------------------------------")
print(GrupoB)
primerlugarB = input('ingrese el primer puesto del grupo B: ')
segundolugarB = input('ingrese el segundo puesto del grupo B: ')
primerlugarB = int(primerlugarB)
segundolugarB = int(segundolugarB)

grupoBGanadores = []
grupoBGanadores.append(GrupoB[primerlugarB])
grupoBGanadores.append(GrupoB[segundolugarB])

print(grupoBGanadores)

print("----------------------------------")
print(GrupoC)
primerlugarC = input('ingrese el primer lugar del grupo C: ')
segundolugarC = input('ingrese el segundo puesto del grupo C: ')
primerlugarC = int(primerlugarC)
segundolugarC = int(segundolugarC)

ganadores_grupo_C = []

ganadores_grupo_C.append(GrupoC[primerlugarC])
ganadores_grupo_C.append(GrupoC[segundolugarC])
print(ganadores_grupo_C)

print("----------------------------------")
print(GrupoD)
primerlugarD = input('ingrese el primer lugar del grupo D: ')
segundolugarD = input('ingrese el segundo puesto del grupo D: ')
primerlugarD = int(primerlugarD)
segundolugarD = int(segundolugarD)

ganadores_grupo_D = []

ganadores_grupo_D.append(GrupoD[primerlugarD])
ganadores_grupo_D.append(GrupoD[segundolugarD])
print(ganadores_grupo_D)

print("----------------------------------")
print(GrupoE)
primerlugarE = input('ingrese el primer lugar del grupo E: ')
segundolugarE = input('ingrese el segundo puesto del grupo E: ')
primerlugarE = int(primerlugarE)
segundolugarE = int(segundolugarE)

ganadores_grupo_E = []

ganadores_grupo_E.append(GrupoE[primerlugarE])
ganadores_grupo_E.append(GrupoE[segundolugarE])
print(ganadores_grupo_E)
print("----------------------------------")
print(GrupoF)
primerlugarF = input('ingrese el primer lugar del grupo F: ')
segundolugarF = input('ingrese el segundo puesto del grupo F: ')
primerlugarF = int(primerlugarF)
segundolugarF = int(segundolugarF)

ganadores_grupo_F = []

ganadores_grupo_F.append(GrupoF[primerlugarF])
ganadores_grupo_F.append(GrupoF[segundolugarF])
print(ganadores_grupo_F)

print("----------------------------------")
print(GrupoG)
primerlugarG = input('ingrese el primer lugar del grupo G: ')
segundolugarG = input('ingrese el segundo puesto del grupo G: ')
primerlugarG = int(primerlugarG)
segundolugarG = int(segundolugarG)

ganadores_grupo_G = []

ganadores_grupo_G.append(GrupoG[primerlugarG])
ganadores_grupo_G.append(GrupoG[segundolugarG])
print(ganadores_grupo_G)

print("----------------------------------")
print(GrupoH)
primerlugarH = input('ingrese el primer lugar del grupo H: ')
segundolugarH = input('ingrese el segundo puesto del grupo H: ')
primerlugarH = int(primerlugarH)
segundolugarH = int(segundolugarH)

ganadores_grupo_H = []

ganadores_grupo_H.append(GrupoH[primerlugarH])
ganadores_grupo_H.append(GrupoH[segundolugarH])
print(ganadores_grupo_H)

CRUZEOCTAVOS1 = []
CRUZEOCTAVOS2 = []
CRUZEOCTAVOS3 = []
CRUZEOCTAVOS4 = []
CRUZEOCTAVOS5 = []
CRUZEOCTAVOS6 = []
CRUZEOCTAVOS7 = []
CRUZEOCTAVOS8 = []

CRUZEOCTAVOS1.append(GrupoA[primerlugar])
CRUZEOCTAVOS1.append(GrupoB[segundolugarB])
CRUZEOCTAVOS2.append(GrupoC[primerlugarC])
CRUZEOCTAVOS2.append(GrupoD[segundolugarD])
CRUZEOCTAVOS3.append(GrupoE[primerlugarE])
CRUZEOCTAVOS3.append(GrupoF[segundolugarF])
CRUZEOCTAVOS4.append(GrupoG[primerlugarG])
CRUZEOCTAVOS4.append(GrupoH[segundolugarH])
CRUZEOCTAVOS5.append(GrupoB[primerlugarB])
CRUZEOCTAVOS5.append(GrupoA[segundolugar])
CRUZEOCTAVOS6.append(GrupoD[primerlugarD])
CRUZEOCTAVOS6.append(GrupoC[segundolugarC])
CRUZEOCTAVOS7.append(GrupoF[primerlugarF])
CRUZEOCTAVOS7.append(GrupoE[segundolugarE])
CRUZEOCTAVOS8.append(GrupoH[primerlugarH])
CRUZEOCTAVOS8.append(GrupoG[segundolugarG])

separador()
print(CRUZEOCTAVOS1)
ganadorcruzeoctavos1 = input("ingrese el ganador del cruce octavos 1: ")
ganadorcruzeoctavos1 = int(ganadorcruzeoctavos1)
ganadoresoctavos1 = []
ganadoresoctavos1.append(CRUZEOCTAVOS1[ganadorcruzeoctavos1])
print(ganadoresoctavos1)

separador()
print(CRUZEOCTAVOS2)
ganadorcruzeoctavos2 = input("ingrese el ganador del cruce octavos 2: ")
ganadorcruzeoctavos2 = int(ganadorcruzeoctavos2)
ganadoresoctavos2 = []
ganadoresoctavos2.append(CRUZEOCTAVOS2[ganadorcruzeoctavos2])
print(ganadoresoctavos2)

separador()
print(CRUZEOCTAVOS3)    
ganadorcruzeoctavos3 = input("ingrese el ganador del cruce octavos 3: ")
ganadorcruzeoctavos3 = int(ganadorcruzeoctavos3)
ganadoresoctavos3 = []
ganadoresoctavos3.append(CRUZEOCTAVOS3[ganadorcruzeoctavos3])
print(ganadoresoctavos3)

separador()
print(CRUZEOCTAVOS4)
ganadorcruzeoctavos4 = input("ingrese el ganador del cruce octavos 4: ")
ganadorcruzeoctavos4 = int(ganadorcruzeoctavos4)
ganadoresoctavos4 = []
ganadoresoctavos4.append(CRUZEOCTAVOS4[ganadorcruzeoctavos4])
print(ganadoresoctavos4)

separador()
print(CRUZEOCTAVOS5)
ganadorcruzeoctavos5 = input("ingrese el ganador del cruce octavos 5: ")
ganadorcruzeoctavos5 = int(ganadorcruzeoctavos5)    
ganadoresoctavos5 = []
ganadoresoctavos5.append(CRUZEOCTAVOS5[ganadorcruzeoctavos5])
print(ganadoresoctavos5)

separador()
print(CRUZEOCTAVOS6)
ganadorcruzeoctavos6 = input("ingrese el ganador del cruce octavos 6: ")
ganadorcruzeoctavos6 = int(ganadorcruzeoctavos6)
ganadoresoctavos6 = []
ganadoresoctavos6.append(CRUZEOCTAVOS6[ganadorcruzeoctavos6])
print(ganadoresoctavos6)

separador()
print(CRUZEOCTAVOS7)
ganadorcruzeoctavos7 = input("ingrese el ganador del cruce octavos 7: ")
ganadorcruzeoctavos7 = int(ganadorcruzeoctavos7)
ganadoresoctavos7 = []
ganadoresoctavos7.append(CRUZEOCTAVOS7[ganadorcruzeoctavos7])
print(ganadoresoctavos7)

separador()
print(CRUZEOCTAVOS8)
ganadorcruzeoctavos8 = input("ingrese el ganador del cruce octavos 8: ")
ganadorcruzeoctavos8 = int(ganadorcruzeoctavos8)
ganadoresoctavos8 = []
ganadoresoctavos8.append(CRUZEOCTAVOS8[ganadorcruzeoctavos8])
print(ganadoresoctavos8)

CRUZECUARTOS1 = []
CRUZECUARTOS2 = []
CRUZECUARTOS3 = []
CRUZECUARTOS4 = []
CRUZECUARTOS1.append(ganadoresoctavos1[0])
CRUZECUARTOS1.append(ganadoresoctavos2[0])
CRUZECUARTOS2.append(ganadoresoctavos3[0])
CRUZECUARTOS2.append(ganadoresoctavos4[0])
CRUZECUARTOS3.append(ganadoresoctavos5[0])
CRUZECUARTOS3.append(ganadoresoctavos6[0])
CRUZECUARTOS4.append(ganadoresoctavos7[0])
CRUZECUARTOS4.append(ganadoresoctavos8[0])

separador()
print("----------------------------------")
print(CRUZECUARTOS1)
ganadorcruzecuartos1 = input("ingrese el ganador del cruce cuartos 1: ")
ganadorcruzecuartos1 = int(ganadorcruzecuartos1)
ganadorescuartos1 = []
ganadorescuartos1.append(CRUZECUARTOS1[ganadorcruzecuartos1])
print(ganadorescuartos1)

separador()
print(CRUZECUARTOS2)    
ganadorcruzecuartos2 = input("ingrese el ganador del cruce cuartos 2: ")
ganadorcruzecuartos2 = int(ganadorcruzecuartos2)
ganadorescuartos2 = []
ganadorescuartos2.append(CRUZECUARTOS2[ganadorcruzecuartos2])
print(ganadorescuartos2)

separador()
print(CRUZECUARTOS3)
ganadorcruzecuartos3 = input("ingrese el ganador del cruce cuartos 3: ")
ganadorcruzecuartos3 = int(ganadorcruzecuartos3)
ganadorescuartos3 = []
ganadorescuartos3.append(CRUZECUARTOS3[ganadorcruzecuartos3])
print(ganadorescuartos3)

separador()
print(CRUZECUARTOS4)
ganadorcruzecuartos4 = input("ingrese el ganador del cruce cuartos 4: ")
ganadorcruzecuartos4 = int(ganadorcruzecuartos4)
ganadorescuartos4 = []
ganadorescuartos4.append(CRUZECUARTOS4[ganadorcruzecuartos4])
print(ganadorescuartos4)    

CRUZESEMIS1 = []
CRUZESEMIS2 = []
CRUZESEMIS1.append(ganadorescuartos1[0])
CRUZESEMIS1.append(ganadorescuartos2[0])
CRUZESEMIS2.append(ganadorescuartos3[0])
CRUZESEMIS2.append(ganadorescuartos4[0])

separador()
print(CRUZESEMIS1)
ganadorcruzemis1 = input("ingrese el ganador del cruce semis 1: ")
ganadorcruzemis1 = int(ganadorcruzemis1)
ganadoressemis1 = []
ganadoressemis1.append(CRUZESEMIS1[ganadorcruzemis1])
print(ganadoressemis1)

separador()
print(CRUZESEMIS2)
ganadorcruzemis2 = input("ingrese el ganador del cruce semis 2: ")
ganadorcruzemis2 = int(ganadorcruzemis2)
ganadoressemis2 = []
ganadoressemis2.append(CRUZESEMIS2[ganadorcruzemis2])
print(ganadoressemis2)

LAGRANFINAL = []
LAGRANFINAL.append(ganadoressemis1[0])
LAGRANFINAL.append(ganadoressemis2[0])

separador()
print(LAGRANFINAL)
ganadorfinal = input("ingrese el ganador de la final: ")
ganadorfinal = int(ganadorfinal)
ganadoresfinal = []
ganadoresfinal.append(LAGRANFINAL[ganadorfinal])
print(ganadoresfinal)
festejar_campeon()
print("Ha Ganado el mundial 2022: ", ganadoresfinal)