
# 1. Crea una variable global llamada escuela con el nombre de tu institución educativa. 
#Luego, crea una función que acceda a esta variable global y la muestre en pantalla 
#dentro de un mensaje amigable.

import time
import os
import random

os.system("cls")
escuela = "Bachillerato Humanista Moderno"

def que_escuela ():
    global escuela
    print("Yo voy al colegio " + str(escuela) + " que queda en la Entre Ríos y Mitre, en Salta.")

que_escuela()

time.sleep(2)
os.system("cls")
# 2. Crea una función y declara dentro de ella una variable local llamada materia.
#Intenta imprimir dicha variable fuera de la función (en el flujo principal del programa)
#y observa qué sucede. Deja un comentario en tu código explicando el comportamiento observado.

def horario ():
    materia = "biología"

#print(materia)
# El programa salta error: "NameError: name 'materia' is not defined"y termina el programa. No
# la lee porque es una variable que esta dentro de una función y no puede ser utilizada directamente.

# 3. Crea una variable global llamada vidas inicializada en 5.
#Luego, define una función llamada perder_vida() que reste 1 al valor de vidas utilizando
#la palabra clave global. Llama a la función y muestra por pantalla el valor de la variable
#antes y después de la llamada.

vidas = 5
print(vidas)

def perder_vida():
    global vidas
    vidas = vidas - 1

perder_vida()
print(vidas)

time.sleep(2)
# 4. Crea una función obtener_saludo() que devuelva el texto "Hola Mundo" utilizando la
# instrucción return. Llama a la función, guarda su resultado en una variable y muéstralo
#por pantalla.

os.system("cls")
time.sleep(1)

def obtener_saludo():
    saludo = "Hola mundo"
    return saludo

saludar = obtener_saludo()
print(saludar)

# 5. Crea una función que devuelva el resultado de la operación 25 + 15.
#Llama a la función y muestra su resultado directamente en la pantalla.

time.sleep(2)
def suma ():
    respuesta = 25 + 15
    return respuesta

total = suma()
print(total)

time.sleep(2)
os.system("cls")
# 6. Crea una función que devuelva el nombre de una mascota (por ejemplo, "Firulais").
#Guarda el resultado de la función en una variable externa e imprímelo en pantalla con el formato:

def nombre_mascota ():
    mascota = "Changa"
    return mascota
nombremascota = nombre_mascota()
print("Mi mascota se llama " + nombremascota)

# 7. Crea una función llamada obtener_edad() que retorne un número entero que represente una edad.
#En el programa principal, guarda el valor retornado por esta función en una variable y,
#utilizando una estructura condicional if/else, muestra si esa edad corresponde a un mayor de
#edad (18 años o más) o a un menor de edad.

time.sleep(2)
os.system("cls")
def obtener_edad ():
    edad = input("Ingrese su edad por favor: ")
    edad = int(edad)
    return edad

nueva_edad = obtener_edad()
if nueva_edad >= 18:
    print("Eres mayor de edad.😜")
else:
    print("Eres menor de edad.😒")
