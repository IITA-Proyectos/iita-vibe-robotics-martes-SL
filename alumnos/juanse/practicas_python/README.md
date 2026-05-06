# Práctica 1

---------------------------------------------------------------------------------
#Almacene un mensaje en una variable e imprímalo en pantalla. 
# Después cambie el valor del mensaje e imprímalo nuevamente.

mensaje = "Hola, soy juanse"
print(mensaje)

mensaje = "Che, mirame"
print(mensaje)

# Almacene el nombre de una persona en una variable, imprima un mensaje para esa person
nombre = "Juan"
print("Hola, " + nombre + " que estás haciendo")

# El número ocho: Escriba una suma, resta, multiplicación y división que resulten cada una en el número ocho. 
# Asegúrese de utilizar la función print() para ver los resultados en pantalla.

print(6+2)
print(10-2)
print(4*2)
print(16/2)

# Cree cuatro variables llamadas mi_entero, mi_decimal, mi_string y mi_booleano. 
# Asigne a cada variable un valor del tipo que le corresponda. 
# Luego muestre en pantalla el tipo de cada variable usando la función type() combinando todo con la función print():

mi_entero = 24
mi_decimal = 3.14
mi_string = "Python de robótica"
mi_booleano = True

print(type(mi_entero))
print(type(mi_decimal))
print(type(mi_string))
print(type(mi_booleano))

# Escriba un programa que acepte un numero decimal como entrada y lo imprima sin lugares decimales.

numero_decimal = 24.67
numero_entero = int(numero_decimal)
print(numero_entero)

# (Opcional) Escriba un programa que le pida al usuario que ingrese nombre y edad.
#Luego muestre un mensaje donde le informe el año en que va a cumplir 100.

identidad = input("Introduzca su nombre")
print (identidad)
edad = input("Introduzca su edad")
print(edad)
cuenta = 100 - int(edad)
edad_que_tendrá_100 = "En " + str(cuenta) + " tendrás 100 años"
print(edad_que_tendrá_100)

# (Opcional) Escriba un programa que permita convertir una temperatura
#en Celsius a la escala Farenheit.

Celsius = 25
Fahrenheit = (9.0/5.0) * Celsius + 32
print(Fahrenheit)

# (Opcional) Calculadora simple: Cree un programa capaz de pedir dos números
# al usuario y devolver el resultado de la suma, resta, multiplicación y división entre los mismos. 

numero_1 = input("Ingrese un número")
print(numero_1)
numero_2 = input("Ingrese otro número")
print(numero_2)

suma =  float(numero_1) + float(numero_2)
resta = float(numero_1) - float(numero_2)
multiplicacion = float(numero_1) * float(numero_2)
division = float(numero_1) / float(numero_2)
potencia = float(numero_1) ** float(numero_2)

print(suma, resta, multiplicacion, division, potencia)
---------------------------------------------------------------------------------

# Práctica 2

--------------------------------------------------------------------------------

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

if int(numero_entero) / 2 == 0:
    print("Es un número par")
elif int(numero_entero) / 2 != 0:
    print("Es un número impar")


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

if str(dia_de_la_semana) == "lunes":
    print("Es el peor día,no lo podes elegir")
elif str(dia_de_la_semana) == "viernes":
    print("Viernes proviene de la diosa Venus, buena elección")
elif str(dia_de_la_semana) == "sabado" or "domingo":
    print("A descansar, te lo ganaste")
elif str(dia_de_la_semana) =="martes" or "miércoles" or "jueves":
    print("Es un día de semana normal")


#Escriba un programa que pida el año actual y un año cualquiera y que escriba cuántos años han pasado desde ese año o cuántos años faltan para llegar a ese año.

año = input("Ingrese el año actual: ")
año_random = input("Ingrese un año cualquiera: ")

if int(año) > int(año_random):
    print("Desde el año " + str(año_random) + " han pasado " + str(int(año) - int(año_random)))
elif int(año) <= int(año_random):
    print("Faltan" + str(int(año_random) - int(año)) + " para el año " + str(año_random))





