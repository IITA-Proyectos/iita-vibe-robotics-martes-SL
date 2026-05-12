
# PRÁCTICA 1

# Punto n1
mensaje = "Hola, estoy aprendiendo Python"
print(mensaje)

mensaje = "Ahora cambié el mensaje"
print(mensaje)


# Punto n2
nombre = "juan"
print("Hola " + nombre + ", ¿te gustaría aprender a programar?")


# Punto n3
print(6 + 7)    
print(7 - 6)
print(6 * 7)    

# Punto n4
entero = 10
decimal = 3.14
string = "hola"
booleano = True

print(type(mi_entero))
print(type(mi_decimal))
print(type(mi_string))
print(type(mi_booleano))


# Punto n5
numero = float(input("Ingresá un número decimal: "))
print(int(numero))

# Punto n6
print(1 != 2)
print(True == 1)
print(False != False)
print(False > 0)
print(1.0 < 1)
print("test" == "test")
print(1.0 >= 1)

------------------------------------------------------------------------------------------------------------------------------------------------------

# PRÁCTICA 2

# Punto n1
numero_cliente = int(input("Ingrese su número de cliente: "))

if numero_cliente == 1000:
    print("¡Ganaste un premio!")


# Punto n2 
edad = int(input("¿Cuál es tu edad?: "))

if edad >= 18:
    print("Eres mayor de edad.")
else:
    print("No eres mayor de edad.")


# Punto n3
numero = int(input("Ingrese un número entero: "))

if numero % 2 == 0:
    print("El número es par.")
else:
    print("El número es impar.")


# Punto n4
num1 = float(input("Ingrese el primer número: "))
num2 = float(input("Ingrese el segundo número: "))

if num1 < num2:
    print("El número menor es " + str(num1))
else:
    print("El número menor es " + str(num2))


# Punto n5
dia = input("Ingrese un día de la semana: ")

if dia == "lunes":
    print("¡Buena semana, a trabajar!")
elif dia == "viernes":
    print("¡Por fin es viernes!")
elif dia == "sábado" or dia == "domingo":
    print("¡A descansar que es fin de semana!")
else:
    print("Es un día de semana normal.")


# Punto n6
print("COMPARADOR DE AÑOS")
anio_actual = int(input("¿En qué año estamos?: "))
anio_cualquiera = int(input("Escriba un año cualquiera: "))

if anio_cualquiera > anio_actual:
    diferencia = anio_cualquiera - anio_actual
    print("Para llegar al año " + str(anio_cualquiera) + " faltan " + str(diferencia) + " años.")
elif anio_cualquiera < anio_actual:
    diferencia = anio_actual - anio_cualquiera
    print("Desde el año " + str(anio_cualquiera) + " han pasado " + str(diferencia) + " años.")
else:
    print("¡Son el mismo año!")

-------------------------------------------------------------------------------------------------------------------------------------------------------
