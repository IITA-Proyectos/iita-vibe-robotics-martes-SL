
# PRÁCTICA 1

# Punto n1
mensaje = "Hola, estoy aprendiendo Python"
print(mensaje)

mensaje = "Ahora cambié el mensaje"
print(mensaje)


# Punto n2
nombre = "juan"
print("Hola " + nombre + ", ¿te gustaría aprender a programar?")


# Punto n3
print(6 + 7)    
print(7 - 6)
print(6 * 7)    

# Punto n4
entero = 10
decimal = 3.14
string = "hola"
booleano = True

print(type(mi_entero))
print(type(mi_decimal))
print(type(mi_string))
print(type(mi_booleano))


# Punto n5
numero = float(input("Ingresá un número decimal: "))
print(int(numero))

# Punto n6
print(1 != 2)
print(True == 1)
print(False != False)
print(False > 0)
print(1.0 < 1)
print("test" == "test")
print(1.0 >= 1)

------------------------------------------------------------------------------------------------------------------------------------------------------

# PRÁCTICA 2

# Punto n1
numero_cliente = int(input("Ingrese su número de cliente: "))

if numero_cliente == 1000:
    print("¡Ganaste un premio!")


# Punto n2 
edad = int(input("¿Cuál es tu edad?: "))

if edad >= 18:
    print("Eres mayor de edad.")
else:
    print("No eres mayor de edad.")


# Punto n3
numero = int(input("Ingrese un número entero: "))

if numero % 2 == 0:
    print("El número es par.")
else:
    print("El número es impar.")


# Punto n4
num1 = float(input("Ingrese el primer número: "))
num2 = float(input("Ingrese el segundo número: "))

if num1 < num2:
    print("El número menor es " + str(num1))
else:
    print("El número menor es " + str(num2))


# Punto n5
dia = input("Ingrese un día de la semana: ")

if dia == "lunes":
    print("¡Buena semana, a trabajar!")
elif dia == "viernes":
    print("¡Por fin es viernes!")
elif dia == "sábado" or dia == "domingo":
    print("¡A descansar que es fin de semana!")
else:
    print("Es un día de semana normal.")


# Punto n6
print("COMPARADOR DE AÑOS")
anio_actual = int(input("¿En qué año estamos?: "))
anio_cualquiera = int(input("Escriba un año cualquiera: "))

if anio_cualquiera > anio_actual:
    diferencia = anio_cualquiera - anio_actual
    print("Para llegar al año " + str(anio_cualquiera) + " faltan " + str(diferencia) + " años.")
elif anio_cualquiera < anio_actual:
    diferencia = anio_actual - anio_cualquiera
    print("Desde el año " + str(anio_cualquiera) + " han pasado " + str(diferencia) + " años.")
else:
    print("¡Son el mismo año!")

-------------------------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------------------------

# PRACTICA N7 


GrupoA = ["Catar", "Ecuador", "Senegal", "Países Bajos"]
GrupoB = ["Inglaterra", "Irán", "Estados Unidos", "Gales"]
GrupoC = ["Argentina", "Arabia Saudita", "México", "Polonia"]
GrupoD = ["Francia", "Australia", "Dinamarca", "Túnez"]
GrupoE = ["España", "Costa Rica", "Alemania", "Japón"]
GrupoF = ["Bélgica", "Canadá", "Marruecos", "Croacia"]
GrupoG = ["Brasil", "Serbia", "Suiza", "Camerún"]
GrupoH = ["Portugal", "Ghana", "Uruguay", "Corea del Sur"]

print("LOS GRUPOS DEL MUNDIAL SON")

print("Grupo A:", GrupoA)
print("Grupo B:", GrupoB)
print("Grupo C:", GrupoC)
print("Grupo D:", GrupoD)
print("Grupo E:", GrupoE)
print("Grupo F:", GrupoF)
print("Grupo G:", GrupoG)
print("Grupo H:", GrupoH)

octavos = []

# GRUPO A

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo A")
print(GrupoA)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoA[primerlugar])
octavos.append(GrupoA[segundolugar])

# GRUPO B

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo B")
print(GrupoB)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoB[primerlugar])
octavos.append(GrupoB[segundolugar])

# GRUPO C

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo C")
print(GrupoC)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoC[primerlugar])
octavos.append(GrupoC[segundolugar])

# GRUPO D

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo D")
print(GrupoD)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoD[primerlugar])
octavos.append(GrupoD[segundolugar])

# GRUPO E

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo E")
print(GrupoE)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoE[primerlugar])
octavos.append(GrupoE[segundolugar])

# GRUPO F

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo F")
print(GrupoF)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoF[primerlugar])
octavos.append(GrupoF[segundolugar])

# GRUPO G

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo G")
print(GrupoG)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoG[primerlugar])
octavos.append(GrupoG[segundolugar])

# GRUPO H

print("-----------------------------------------")
print("Ingrese el primer y segundo del Grupo H")
print(GrupoH)

primerlugar = int(input("Ingrese el primer: "))
segundolugar = int(input("Ingrese el segundo: "))

octavos.append(GrupoH[primerlugar])
octavos.append(GrupoH[segundolugar])

print("-----------------------------------------")
print("CLASIFICADOS A OCTAVOS")
print(octavos)

# OCTAVOS

cuartos = []

print("-----------------------------------------")
print("OCTAVOS DE FINAL")

print(octavos[0], "vs", octavos[1])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[2], "vs", octavos[3])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[4], "vs", octavos[5])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[6], "vs", octavos[7])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[8], "vs", octavos[9])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[10], "vs", octavos[11])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[12], "vs", octavos[13])
ganador = input("Ganador: ")
cuartos.append(ganador)

print(octavos[14], "vs", octavos[15])
ganador = input("Ganador: ")
cuartos.append(ganador)

print("-----------------------------------------")
print("CLASIFICADOS A CUARTOS")
print(cuartos)

# CUARTOS

semifinal = []

print("-----------------------------------------")
print("CUARTOS DE FINAL")

print(cuartos[0], "vs", cuartos[1])
ganador = input("Ganador: ")
semifinal.append(ganador)

print(cuartos[2], "vs", cuartos[3])
ganador = input("Ganador: ")
semifinal.append(ganador)

print(cuartos[4], "vs", cuartos[5])
ganador = input("Ganador: ")
semifinal.append(ganador)

print(cuartos[6], "vs", cuartos[7])
ganador = input("Ganador: ")
semifinal.append(ganador)

print("-----------------------------------------")
print("CLASIFICADOS A SEMIFINAL")
print(semifinal)

# SEMIFINAL

final = []

print("-----------------------------------------")
print("SEMIFINAL")

print(semifinal[0], "vs", semifinal[1])
ganador = input("Ganador: ")
final.append(ganador)

print(semifinal[2], "vs", semifinal[3])
ganador = input("Ganador: ")
final.append(ganador)

print("-----------------------------------------")
print("FINALISTAS")
print(final)

# FINAL

print("-----------------------------------------")
print("GRAN FINAL")

print(final[0], "vs", final[1])

campeon = input("Ingrese el campeón del mundo: ")

print("-----------------------------------------")
print("CAMPEON DEL MUNDO 2022")
print("El campeón es:", campeon)

