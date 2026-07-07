numero = int(input("Ingresá un número: "))

if numero > 0:
    print("Positivo")
elif numero < 0:
    print("Negativo")
else:
    print("Es cero")

usuario = input("Usuario: ")
contraseña = input("Contraseña: ")

if usuario == "maxim" and contraseña == "1234":
    print("Bienvenido")
else:
    print("Sus datos son incorrectos")

    sexo = input("¿Sos señor o señora?: ")
edad = int(input("Ingresá tu edad: "))

if sexo == "señora":
    if edad > 60:
        print("Ya se puede jubilar.")
    else:
        print("Todavia no se puede jubilar.")
elif sexo == "señor":
    if edad > 65:
        print("Ya se puede jubilar.")
    else:
        print("Todavia no se puede jubilar.")

        num1 = float(input("Primer número: "))
num2 = float(input("Segundo número: "))

print("Suma:", num1 + num2)
print("Resta:", num1 - num2)
print("Multiplicacion:", num1 * num2)
print("División:", num1 / num2)

import random

opciones = ["piedra", "papel", "tijera"]

usuario = input("Elegi piedra, papel o tijera: ")
pc = random.choice(opciones)

print("Computadora:", pc)

if usuario == pc:
    print("Empate")
elif (usuario == "piedra" and pc == "tijera") or (usuario == "papel" and pc == "piedra") or (usuario == "tijera" and pc == "papel"):
    print("Ganaste")
else:
    print("Perdiste")

    numero = int(input("Ingresá un número: "))

if numero >= 10 and numero <= 20:
    print("Esta entre 10 y 20.")
else:
    print("Esta fuera del rango.")

    partido = input("Ingresa el partido que queres predecir: ")

print("El ganador del partido sera: Paraguay")

import random

jugador1 = 0
jugador2 = 0

while jugador1 < 3 and jugador2 < 3:

    input("Jugador 1: presioná ENTER para tirar el dado.")
    dado1 = random.randint(1, 6)
    print("Jugador 1 saco:", dado1)

    input("Jugador 2: presioná ENTER para tirar el dado.")
    dado2 = random.randint(1, 6)
    print("Jugador 2 saco:", dado2)

    if dado1 > dado2:
        jugador1 += 1
        print("Gana la ronda el Jugador 1")
    elif dado2 > dado1:
        jugador2 += 1
        print("Gana la ronda el Jugador 2")
    else:
        print("Empate")

    print("Marcador:", jugador1, "-", jugador2)

if jugador1 == 3:
    print("Ganó el Jugador 1")
else:
    print("Ganó el Jugador 2")