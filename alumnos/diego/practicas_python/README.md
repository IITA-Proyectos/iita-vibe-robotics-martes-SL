# Práctica 3
# La temperatura
temp = float(input("Ingrese la temperatura (°C): "))

if temp < 15:
    print("Frío")
elif temp <= 25:
    print("Agradable")
else:
    print("Caluroso")
# Puedo pasar
edad = int(input("¿Cuántos años tenés? "))
tiene_entrada = input("¿Tenés entrada? (True/False): ") == "True"

if edad > 12 and tiene_entrada:
    print(" ¡Sí podés entrar a la fiestita!")
else:
    print(" No podés entrar.")
# El número secreto
secreto = 15

while True:
    numero = int(input("Decime un numerito: "))
    if numero > secreto:
        print("→ Te pasaste.")
    elif numero < secreto:
        print("→ Te quedaste cortina.")
    else:
        print("¡Correcto! Adivinaste el número.")
        break
# Multiplicaciones
numero = int(input("Ingresá el número del cual querés ver la tabla: "))
numero = str(numero)
print("Tabla del " + (numero))
numero = int(numero)
for i in range(1, 11):
    resultado = (numero * i )
    print(f"{numero} * {i} = {resultado}")

# Práctica 4
# --- CAJERO AUTOMÁTICO ---

PIN_CORRECTO = "1234"
saldo = 10000.0
intentos = 0

print("--- CAJERO AUTOMÁTICO ---")

while intentos < 3:
    pin = input("Ingrese su PIN: ")
    if pin == PIN_CORRECTO:
        print("¡Bienvenido!")
        break
    else:
        intentos += 1
        print(f"PIN incorrecto. Intento {intentos}/3")

if intentos == 3:
    print("Tarjeta blqueada.")
    exit()

# Menú principal
while True:
    print("¿Qué desea hacer?")
    print("1. Retirar dinero")
    print("2. Consultar saldo")
    print("3. Salir")
    opcion = input("Opción: ")

    if opcion == "1":
        monto = float(input("¿Cuánto dinero desea retirar?: "))
        if monto > saldo:
            print(f"Fondos insuficientes. Su saldo es ${saldo}")
        else:
            saldo -= monto
            print(f"Retiro exitoso. Su nuevo saldo es ${saldo}")
    elif opcion == "2":
        print(f"Su saldo actual es de ${saldo}")
    elif opcion == "3":
        print("Gracias por usar el cajero automático. ¡Adiós!")
        break
    else:
        print("Opción inválida.")



