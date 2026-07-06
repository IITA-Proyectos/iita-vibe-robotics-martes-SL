import os
import time
import random

#Pedile un número al usuario e indicá:
#Positivo si es mayor a 0
#Negativo si es menor a 0
#Es cero si vale 0

os.system("cls")

numero= input("Ponga un numero cualquiera: ")
numero = float(numero)

if numero > 0:
    print("Es positivo.")
elif numero < 0:
    print("Es negativo.")
elif numero == 0:
    print("Es cero.")
time.sleep(2)

#Sos el guardia de seguridad de la puerta de PAMI. 
#Tenés que preguntarle al usuario si ya tiene la edad suficiente 
#para jubilarse:
#Si es una doñita: Se jubila siendo mayor de 60 años.
#Si es un señor: Se jubila siendo mayor que 65 años.

os.system("cls")
nombre = input("Ingrese su nombre completo: ")
time.sleep(1)
HoM = input("¿Varón o mujer?: ")
HoM = str(HoM)
time.sleep(1)
edad = input("Ingrese su edad, por favor: ")
edad = int(edad)
os.system("cls")
time.sleep(2)
print("GRACIAS, SOY EL GUARDIA VIRTUAL DEL PAMI")
time.sleep(1)

edad_faltante_H = 65 - edad
edad_faltante_M = 60 - edad

def esta_jubilado ():
    if edad >= 60 and HoM == "mujer":
        print("Ya puedes jubilarte.")

    elif edad >= 65 and HoM == "varón" or "varon":
        print("Señor " + nombre + ", ya puede jubilarse.")

    elif edad < 60 and HoM == "mujer":
        print("Lo siento, " + nombre + ", usted aún no puede jubilarse. Gracias por comunicarte con PAMI.")
        print("Para jubilarse le quedan " + str(edad_faltante_M) + " años.")

    elif edad < 60 and HoM == "varón" or "varon":
        print("Lo siento, " + nombre + ", usted aún no puede jubilarse. Gracias por comunicarte con PAMI.")
        print("Para jubilarse le quedan " + str(edad_faltante_H) + " años.")

esta_jubilado()

# Te contrataron de Facebook en los principios de los 2000 y 
#te pidieron hacer el login/inicio de sesión. Pedile al usuario 
#por consola que ingrese un usuario y contraseña. Si las credenciales 
#son correctas mostrá por consola "Bienvenido", sino un mensaje 
#diciéndole que sus datos son incorrectos.

time.sleep(5)
os.system("cls")
usuario = "Juanse-bike"
usuario = str(usuario)
contraseña = "Caralibro"
contraseña = str(contraseña)

while True:
    print("------ Facebook ------")
    usuario1 = input("Usuario: ")
    usuario1 = str(usuario1)
    contraseña1 = input("Contraseña: ")
    contraseña1 = str(contraseña1)
    os.system("cls")
    time.sleep(2)
    if usuario1 == usuario and contraseña1 == contraseña:
        print("Bienvenido")
        break
    else:
        print("°°°Contraseña o usuario incorrecto.°°°")


#Hagamos una calculadora.
#Pedile dos números al usuario y por consola tenés que mostrar el resultado de:
#La suma
#La resta
#La multiplicación
#La división

time.sleep(2)
os.system("cls")

print("😨CALCULADORA PRO-🛂")
numero1 = input("Escriba un numerito: ")
numero2 = input("Escriba otro numerito: ")
numero1 = float(numero1)
numero2 = float(numero2)

os.system("cls")
print("Pensando...")
time.sleep(1)

suma = numero1 + numero2
resta = numero1 - numero2
multiplicacion = numero1 * numero2
division = numero1 / numero2

print("Suma: " + str(suma))
print("Resta: " + str(resta))
print("Multiplicación: " + str(multiplicacion))
print("División: " + str(division))
time.sleep(5)

#Sabiendo usar la librería de la aleatorización (random) hagamos un juego. 
#El usuario debe elegir su opción y la computadora también elige una opción 
#al azar. Mostrar ambas elecciones y decirle si ganó, perdió o empató.

os.system("cls")
time.sleep(2)

opciones = ["piedra", "papel", "tijera"]

print("PIEDRA💎 PAPEL🎫 TIJERA⚔")
time.sleep(1)

contador_homo = 1
contador_pc = 1

os.system("cls")
print("Pensando...")
time.sleep(3)
os.system("cls")

def quien_gana ():
    global contador_homo
    global contador_pc

    if opcion_homo == "piedra" and opcion_pc == "tijera":
        print("¡GANASTE!✨")
        contador_homo += 1
    elif opcion_homo == "tijera" and opcion_pc == "piedra":
        print("¡TE GANÓ LA PC!, JAJA 💻")
        contador_pc += 1
    elif opcion_homo == "piedra" and opcion_pc == "piedra":
        print("EMPATE, OTRA RONDA")
    elif opcion_homo == "tijera" and opcion_pc == "tijera":
        print("EMPATE, OTRA RONDA")
    elif opcion_homo == "papel" and opcion_pc == "tijera":
        print("¡TE GANÓ LA PC!, JAJA 💻")
        contador_pc += 1
    elif opcion_homo == "tijera" and opcion_pc == "papel":
        print("¡GANASTE!✨")
        contador_homo += 1
    elif opcion_homo == "papel" and opcion_pc == "papel":
        print("EMPATE, OTRA RONDA")
    elif opcion_homo == "piedra" and opcion_pc == "papel":
        print("¡TE GANÓ LA PC!, JAJA 💻")
        contador_pc += 1
    elif opcion_homo == "papel" and opcion_pc == "piedra":
        print("¡GANASTE!✨")
        contador_homo += 1

while True :
    opcion_homo = input("Ingrese su jugada: ")
    opcion_pc = random.choice(opciones)
    print("Tu jugada es: " + str(opcion_homo))
    print("La jugada de pc es: " + str(opcion_pc))
    quien_gana()
    if contador_homo == 3:
        print("GANASTE LA RONDA!! YUHUUUU")
        break
    if contador_pc == 3:
        print("Te ganó la compu, que mal que jugás. 😝")
        break


#Pedile un número al usuario. 
#Si el número que elige el usuario está dentro del rango del 10 al 20 mostrá por consola:

os.system("cls")
numero_del_usuario = input("Ingrese un número: ")
numero_del_usuario = float(numero_del_usuario)

if 10 <= numero_del_usuario <= 20:
    print("Está entre el 10 y el 20.")
else:
    print("Está fuera del rango.")
time.sleep(3)

#Este pulpo fue muy popular en la Copa del Mundo Sudáfrica 2010 por predecir resultados. 
#Pedile al usuario que ingrese el partido que desea predecir de las eliminatorias y mostrá por 
#consola al ganador:

os.system("cls")
partido = []

print("⚽⚽PREDICCIÓN DE PARTIDOS😎🤑")

equipo1 = input("Ingrese el primer equipo: ")
equipo2 = input("Ingrese el segundo equipo: ")

partido.append(equipo1)
partido.append(equipo2)

os.system("cls")
print("El partido es: " + partido[0] + " vs " + partido[1])

ganador = random.choice(partido)
time.sleep(2)
print("El ganador del partido será: " + str(ganador))

time.sleep(3)

#Hagamos otro juego, pero ahora para dos jugadores. 
#Cada uno va a tirar un dado y el que saque un número más alto gana. 
#El jugador que gane 3 rondas gana.

os.system("cls")
print("🎲JUEGO DE DADOS🎲")
contador_jugador1 = 0
contador_jugador2 = 0

while True:
    print("Jugador 1")
    input("Presiona ENTER para tirar el dado: ")
    dado1 = random.randint(1, 6)

    print("Jugador 2")
    input("Presiona ENTER para tirar el dado: ")
    dado2 = random.randint(1, 6)

    os.system("cls")
    print("El jugador 1 sacó: " + str(dado1))
    print("El jugador 2 sacó: " + str(dado2))

    if dado1 > dado2:
        print("¡Jugador 1 gana!🎀")
        contador_jugador1 += 1
    elif dado2 > dado1:
        print("¡Jugador 2 gana!🎉")
        contador_jugador2 += 1
    else:
        print("¡Empate!")

    if contador_jugador1 == 3:
        print("¡GANÓ EL JUGADOR 1🎀🎀🎀!")
        break
    if contador_jugador2 == 3:
        print("¡GANÓ EL JUGADOR 2🎉🎉🎉!")
        break
time.sleep(2)
