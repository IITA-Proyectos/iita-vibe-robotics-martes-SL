#Solicitar al usuario un número de cliente. Si el número es el 1000, imprimir "Ganaste un premio".

numero = input("Ponga un número cualquiera:")
print(numero)
if int(numero) == 1000:
    print("Ganaste un premio")


#Escribir un programa que pregunte al usuario su edad y muestre por pantalla si es mayor de edad o no.

edad = input("Introduzca su edad, por favor: ")

if int(edad) >=18:
    print("Eres mayor de edad")

else:
    print("Eres menor de edad")


#Escribir un programa que pida al usuario un número entero y muestre por pantalla si es par o impar.

numero_entero = input("Introduzca un número entero: ")

if int(numero_entero) % 2 == 0:
    print("El número es par")
else:
    print("El número es impar")


#Solicitar al usuario que ingrese dos números y mostrar cuál de los dos es menor. No considerar el caso en que ambos números son iguales.

primer_numero = input("Ingrese su primer número: ")
segundo_numero = input("Ingrese su segundo número: ")

if int(primer_numero) < int(segundo_numero):
    print("El número menor es: " + primer_numero)
elif int(segundo_numero) < int(primer_numero):
    print("En menor número es: " + segundo_numero)
elif primer_numero == segundo_numero:
    print("Son iguales")


#Requerir al usuario que ingrese un día de la semana e imprimir un mensaje si es lunes, otro mensaje diferente si es viernes, otro mensaje diferente si es sábado o domingo. Si el día ingresado no es ninguno de esos, imprimir otro mensaje.

dia_de_la_semana = input("Ingrese un día de la semana: ")

if dia_de_la_semana == "lunes":
    print("Es el peor día,no lo podes elegir")

elif dia_de_la_semana == "viernes":
    print("Viernes proviene de la diosa Venus, buena elección")

elif dia_de_la_semana in ["sabado", "domingo"]:
    print("A descansar, te lo ganaste")

elif dia_de_la_semana in ["martes", "miércoles", "jueves"]:
    print("Es un día de la semana normal")


#Escriba un programa que pida el año actual y un año cualquiera y que escriba cuántos años han pasado desde ese año o cuántos años faltan para llegar a ese año.

año = input("Ingrese el año actual: ")
año_random = input("Ingrese un año cualquiera: ")

if int(año) > int(año_random):
    print("Desde el año " + str(año_random) + " han pasado " + str(int(año) - int(año_random)))
elif int(año) <= int(año_random):
    print("Faltan" + str(int(año_random) - int(año)) + " para el año " + str(año_random))




