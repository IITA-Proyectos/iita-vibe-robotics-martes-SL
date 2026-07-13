import time
time.sleep(7)
escuela = "Colegio San marcos"

def wescuela():
    print("Yo estudio en:", escuela)

wescuela()
print("-------------------------------------------------------------------")
time.sleep(7)
def wmateria():
    materia = "matematicas"
wmateria()

print(materia)
"marca error por que la variable esta adentro de la funcion"
print("-------------------------------------------------------------------")
time.sleep(7)
vidas = 5

def perder_vida():
    global vidas
    vidas = vidas - 1

print("Vidas antes:", vidas)

perder_vida()

print("Vidas despues:", vidas)
print("-------------------------------------------------------------------")
time.sleep(7)
def saludo():
    return "Hola Mundo"

saludo = saludo()

print(saludo)
print("-------------------------------------------------------------------")
time.sleep(7)
def suma():
    return 25 + 15

print(suma())
print("-------------------------------------------------------------------")
time.sleep(7)
def mascota():
    return "milanesa"

print("Mi mascota se llama:", mascota())
print("-------------------------------------------------------------------")
time.sleep(7)
def edad():
    return 20

edad = edad()

if edad >= 18:
    print("Es mayor de edad.")
else:
    print("Es menor de edad.")