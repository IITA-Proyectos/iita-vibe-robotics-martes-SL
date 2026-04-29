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
# raiz_cuadrada = numero_1  numero_2

print(suma, resta, multiplicacion, division, potencia)










































