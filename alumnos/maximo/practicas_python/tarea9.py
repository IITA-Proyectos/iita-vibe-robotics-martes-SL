import os
import time
import random

jugadores = [
    "Geronimo Rulli", "Juan Musso", "Emiliano Martinez",
    "Marcos Senesi", "Nicolas Tagliafico", "Gonzalo Montiel",
    "Lisandro Martinez", "Cristian Romero", "Nicolas Otamendi",
    "Facundo Medina", "Nahuel Molina", "Leandro Paredes",
    "Rodrigo De Paul", "Valentin Barco", "Giovani Lo Celso",
    "Exequiel Palacios", "Alexis Mac Allister",
    "Enzo Fernandez", "Julian Alvarez", "Lionel Messi",
    "Nicolas Gonzalez", "Thiago Almada", "Giuliano Simeone",
    "Nicolas Paz", "Jose Manuel Lopez", "Lautaro Martinez"
]

while True:
    os.system("cls")
    print("Bienvenido")
    print("1 - Abrir un paquete")
    print("2 - Abrir varios paquetes")
    print("3 - Salir")

    opcion = input("Ingrese una opción: ")

    if opcion == "1":
        cantidad = 1

    elif opcion == "2":
        cantidad = int(input("¿Cuántos paquetes quiere abrir? "))

    elif opcion == "3":
        break

    else:
        print("Opción incorrecta.")
        time.sleep(2)
        continue

    for paquete in range(cantidad):
        print("Abriendo paquete...")
        time.sleep(1)

        salio_messi = False

        for i in range(3):
            print("Figurita", i + 1, "...")
            time.sleep(1)

            figurita = random.choice(jugadores)
            print("->", figurita)
            time.sleep(1)

            if figurita == "Lionel Messi":
                salio_messi = True

        if salio_messi:
            print("🔥 ¡Te salió Messi!")

    input("Presione ENTER para continuar...")