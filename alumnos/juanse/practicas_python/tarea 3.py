
#  Pedí una temperatura con input(). Mostrá si es “Frío” (menos de 15), “Agradable” (entre 16 y 25) o “Caluroso” (más de 25).

temperatura = input("Ingrese una temperatura: ")
temperatura = int(temperatura)

if temperatura < 15:
    print("Frio")
elif temperatura > 15 and temperatura < 25:
    print("Agradable")
elif temperatura > 25:
    print("Caluroso")


# Pedí la edad del usuario y si tiene entrada (True/False). Usando and mostrá si puede entrar o no a la fiestita (mayores de 12 con entrada).

edad = input("Ingrese su edad: ")
tiene_entrada = True

if int(edad) > int(17) and tiene_entrada:
    print("Podes entrar")
else:
    print("No podes entrar, lo siento")


# Guardá un número en una variable. Usando while, pedile al usuario que adivine. Avisale también si se quedó corto o si se pasó. Cuando adivine, rompé el bucle.

numero_secreto = 24

while True:
    numerito = input("Decime un numerito: ")
    if int(numerito) < int(numero_secreto):
        print("Te quedaste corto")
    if int(numerito) > int(numero_secreto):
        print("Te pasate")
    if int(numerito) == int(numero_secreto):
        print("¡Adivinaste!")
        break


# Pedile al usuario un número y usa for con range() para mostrar su tabla del 1 al 10.

numero = int(input("Ingrese un número: "))

for i in range(1, 11):
    print(numero, "x", i, "=", numero * i)

