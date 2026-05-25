pin = "6767"
saldo = 100

ingreso = input("Ingrese su PIN: ")

if ingreso == pin:

    while True:

        print("1. Retirar dinero")
        print("2. Consultar dinero")
        print("3. Salir")

        opcion = input("elija una opcion: ")

        if opcion == "1":

            monto = int(input("cuanto quiere sacar?: "))

            if monto <= saldo:
                saldo = saldo - monto
                print("retiro exitoso")
                print("dinero actual:", saldo)

            else:
                print("dinero no suficiente")

        elif opcion == "2":

            print("Su saldo es:", saldo)

        elif opcion == "3":

            print("Gracias por usar el cajero automatico de maxi")
            break

        else:

            print("Opcion incorrecta")

else:

    print("PIN incorrecto")