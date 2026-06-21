#Vamos a inventar nuestro primer menú interactivo por consola usando:
#os para limpiar la pantalla
#ime para agregar pausas
##La idea es que el programa quede funcionando constantemente hasta que el usuario decida salir.

import os
import time
import random

os.system("cls")
print("Bienvenido!")
time.sleep(2)

print("1 - Elegir opción 1")
print("2 - Elegir opción 2")
print("3 - Elegir opción 3")

time.sleep(0.5)

numero = input("Ingrese el número de la opción que desea elegir: ")
numero = int(numero)

if numero == 1:
    print("Elegiste la 1")

if numero == 2:
    print("Elegiste la 2")

if numero == 3:
    print("Elegiste la 3")
time.sleep(1.9)

"""
Ahora simulemos la apertura de un paquete de 3 figuritas.
 Usaremos 3 librerías ahora:
random para elegir figuritas al azar
time para agregar suspenso
os para limpiar la pantalla
Tenemos que simular la apertura del paquete, que nos de 3 figuritas al
 azar y nos las muestre una por una con suspenso.
"""
jugadores = [ "Geronimo Rulli", "Juan Musso", "Emiliano Martinez", "Marcos Senesi", "Nicolas Tagliafico", "Gonzalo Montiel", 
             "Lisandro Martinez", "Cristian Romero", "Nicolas Otamendi", "Facundo Medina", "Nahuel Molina", "Leandro Paredes", 
             "Rodrigo De Paul", "Valentin Barco", "Giovani Lo Celso", "Exequiel Palacios", "Alexis Mac Allister", 
             "Enzo Fernandez", "Julian Alvarez", "Lionel Messi", "Nicolas Gonzalez", "Thiago Almada", "Giuliano Simeone", 
             "Nicolas Paz", "Jose Manuel Lopez", "Lautaro Martinez" ]

def separador():
    print("---------------***---------------")

os.system("cls")
print("Ahora, ¡Vamos a abrir paquetes del mundial!")
time.sleep(1)
print("Tengo para que abras solamente 10 paquetes...")
print("¿Cuántos deseas abrir?")
time.sleep(0.5)
numero_paquetes = input("Ingrese el número de paquetes: ")
numero_paquetes = int(numero_paquetes)

tengo_figuritas = []
os.system("cls")
contador = 1
for i in range(0, numero_paquetes):
    print ("ABRIENDO PAQUETE " + str(contador) + "...")
    time.sleep(2)
    print("Primera figurita⚽")
    figurita_nueva = random.choice(jugadores)
    tengo_figuritas.append(figurita_nueva)
    print("->"+ figurita_nueva)
    if figurita_nueva == "Lionel Messi":
        print("🎈Te tocó Messi!!!!💥")
    time.sleep(1)
    print("Segunda figurita⚽")
    figurita_nueva = random.choice(jugadores)
    tengo_figuritas.append(figurita_nueva)
    print("->"+ figurita_nueva)
    if figurita_nueva == "Lionel Messi":
        print("🎈Te tocó Messi!!!!💥")
    time.sleep(1)
    print("Tercera figurita⚽")
    figurita_nueva = random.choice(jugadores)
    tengo_figuritas.append(figurita_nueva)
    print("->"+ figurita_nueva)
    if figurita_nueva == "Lionel Messi":
        print("🎈Te tocó Messi!!!!💥")
    time.sleep(2)
    os.system("cls")
    contador = contador + 1

print("Te tocaron:")
for jugador in tengo_figuritas:
    print(jugador)

time.sleep(2)
repetidas = []
separador()

for figurita in tengo_figuritas:
    if tengo_figuritas.count(figurita) > 1 and figurita not in repetidas:
        repetidas.append(figurita)

print("Figuritas repetidas:")
for jugador in repetidas:
    print(jugador)

