
""" Simulador de Cajero Automático: Vamos a crear un programa que
 simule el ingreso y las operaciones de un cajero automático. 
 ¡Este ejercicio requiere pensar bien cómo anidar bucles y condicionales!

Requisitos:

Define un PIN secreto en una variable (por ejemplo, pin_secreto = "1234").
Define un saldo inicial (por ejemplo, saldo = 10000).
Usa un while para darle al usuario un máximo de 3 intentos
 para ingresar el PIN correcto. Si agota los 3 intentos, mostrá un mensaje
  que diga "Cuenta bloqueada" y el programa debe terminar.
Si el usuario adivina el PIN, dale la bienvenida y mostrale un menú 
interactivo usando otro while que se repita hasta que elija "Salir".
El menú debe tener las siguientes opciones (usa if/elif/else para 
la elección):
    Retirar dinero: Pedí el monto a retirar. Si es mayor al saldo, avisa que no
hay fondos suficientes. Si es menor o igual, resta el monto del saldo e imprimí el nuevo saldo.
    Consultar saldo: Mostrale al usuario el valor de su saldo actual. (Ej: "Su saldo actual es de $10000").
    Salir: Termina el bucle del menú y despídete del usuario.
Pistas para resolverlo:

Para contar los intentos, crea una variable intentos = 0 y
súmale 1 en cada vuelta del primer while.
Para que el menú se repita infinitamente hasta que el usuario decida,
puedes usar while True: y cuando elija la opción "3", ejecutas la palabra
clave break para romper el ciclo.
"""
import time

pin_secreto = 1234
pin_secreto = int(pin_secreto)

saldo_inicial = 24000
saldo_inicial = int(saldo_inicial)

contador = 0

while True:
    contraseña = input("Ingrese su contraseña: ")
    contraseña = int(contraseña)

    if contraseña == pin_secreto:
        print("Bienvenido al banco JS")
        while True:
            print("1. Retirar dinero")
            print("2. Consultar saldo")
            print("3. Salir")
            time.sleep(2)

            codigo = input("Elija una opción, por favor. (Por ejemplo: 3): ")
            codigo = int(codigo)
            if codigo == 1:
                print("¿Cuanto desea retirar?")
                dinero_retiro = input("Ingrese el monto a retirar: ")
                dinero_retiro = int(dinero_retiro)
                if dinero_retiro > saldo_inicial:
                    print("No tienes suficiente dinero.")
                else:
                    nuevosaldo = int(saldo_inicial) - int(dinero_retiro)
                    nuevosaldo = int(nuevosaldo)
                    print("Listo, ya puedes retirar")
                    print("Tu nuevo saldo es de" + str(nuevosaldo))
                    time.sleep(2)


            elif codigo == 2:
                print("Tu saldo actual es de" + str(saldo_inicial))
                time.sleep(2)

            elif codigo == 3:
                print("Gracias por elegirnos, hasta pronto.")
                break



    else:
        contador = contador + 1

    if contador == 3:
        print("Cuenta bloqueda. Sr usuario recuerde que tiene 3 intentos.")
        print("Regrese otro día.")
        break

















