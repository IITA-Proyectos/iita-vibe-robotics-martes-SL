# -*- coding: utf-8 -*-
"""
Machete de Soluciones - Clase 6: Tuplas y Diccionarios
Creado por la IA para el Profe Franco en 'tests_de_la_ia'
"""

print("==================================================")
print(">>> PROBANDO EJERCICIOS DE LA CLASE 6")
print("==================================================\n")

# -----------------------------------------------------------------------------
# EJERCICIO 1: Calibrando el Sensor de Luz
# -----------------------------------------------------------------------------
print("--- Ejercicio 1: Calibración de Sensor (Tuplas) ---")

# Creamos la tupla de calibración con (mínimo, máximo)
calibracion_luz = (20, 95)
print(f"Calibración guardada en tupla: {calibracion_luz}")
print(f"Minimo (oscuridad): {calibracion_luz[0]}%")
print(f"Maximo (linterna): {calibracion_luz[1]}%")

# Intentamos modificar un valor para demostrar que falla y protege el programa
print("\nIntentando modificar la tupla (calibracion_luz[0] = 30)...")
try:
    calibracion_luz[0] = 30
except TypeError as e:
    print("[X] ¡Python te sacó carpiendo! Error esperado:")
    print(f"    TypeError: {e}")
    print("¡Buenísimo! La tupla no se modificó y los datos están a salvo.\n")

print("==================================================\n")

# -----------------------------------------------------------------------------
# EJERCICIO 2: La lista de Scaloni (El 11 Titular - 4-4-2)
# -----------------------------------------------------------------------------
print("--- Ejercicio 2: La Lista de Scaloni (Tupla + Lista) ---")

# Tupla fija con el Staff Técnico
staff_tecnico = ('Scaloni', 'Aimar', 'Samuel')
print(f"Staff Tecnico (fijo): {staff_tecnico}")

# Lista vacía para los convocados titulares
convocados = []

print("\nIngresa los 11 jugadores del equipo titular (Esquema 4-4-2):")

# 1. Arquero (1)
arquero = input("Ingrese el nombre del Arquero: ")
convocados.append(arquero)

# 2. Defensores (4)
print("\n--- Seccion Defensores ---")
for i in range(4):
    defensor = input(f"Ingrese defensor {i+1} de 4: ")
    convocados.append(defensor)

# 3. Mediocampistas (4)
print("\n--- Seccion Mediocampistas ---")
for i in range(4):
    medio = input(f"Ingrese mediocampista {i+1} de 4: ")
    convocados.append(medio)

# 4. Delanteros (2)
print("\n--- Seccion Delanteros ---")
for i in range(2):
    delantero = input(f"Ingrese delantero {i+1} de 2: ")
    convocados.append(delantero)

# Imprimimos de forma linda en la terminal
print("\n=============================================")
print("  PRESENTACION DE LA SELECCION TITULAR  ")
print("=============================================")
print(f"Staff Tecnico Fijo: {', '.join(staff_tecnico)}")
print("---------------------------------------------")
print("El 11 Titular Convocado:")
print(f"(*) Arquero:        {convocados[0]}")
print(f"(*) Defensores:     {', '.join(convocados[1:5])}")
print(f"(*) Mediocampistas: {', '.join(convocados[5:9])}")
print(f"(*) Delanteros:     {', '.join(convocados[9:11])}")
print("=============================================\n")

# -----------------------------------------------------------------------------
# EJERCICIO 3: Mi primer diccionario
# -----------------------------------------------------------------------------
print("--- Ejercicio 3: Mi primer diccionario ---")

mi_robot = {
    'nombre': 'VibeBot',
    'color': 'Amarillo y Negro',
    'ruedas': 2
}

print(f"Diccionario creado: {mi_robot}")
print(f"El nombre del robot es: {mi_robot['nombre']}\n")

print("==================================================\n")

# -----------------------------------------------------------------------------
# EJERCICIO 4: Mantenimiento en boxes
# -----------------------------------------------------------------------------
print("--- Ejercicio 4: Mantenimiento en boxes ---")

print("Agregando bateria al 100%...")
mi_robot['bateria'] = 100
print(f"Estado actual: {mi_robot}")

print("\nSimulando consumo... la bateria baja al 80%...")
mi_robot['bateria'] = 80
print(f"Estado final: {mi_robot}\n")

print("==================================================\n")

# -----------------------------------------------------------------------------
# EJERCICIO 5: Tienda LEGO Technic
# -----------------------------------------------------------------------------
print("--- Ejercicio 5: Tienda LEGO Technic (.get()) ---")

inventario = {
    'spike_hub': {
        'precio': 200, 
        'descripcion': 'Controlador inteligente LEGO Spike Prime', 
        'stock': 5
    },
    'motor_grande': {
        'precio': 45, 
        'descripcion': 'Motor angular grande de alto torque', 
        'stock': 12
    },
    'sensor_color': {
        'precio': 30, 
        'descripcion': 'Sensor detector de colores y luz ambiente', 
        'stock': 8
    }
}

print("Componentes en stock:")
for item in inventario:
    print(f"- {item}")

buscar = input("\n¿Que repuesto de LEGO estas buscando? ").strip().lower()

# Buscamos de forma segura con .get()
pieza_encontrada = inventario.get(buscar)

if pieza_encontrada:
    print("\n[V] Componente encontrado!")
    print(f"[-] Producto:    {buscar.upper().replace('_', ' ')}")
    print(f"[-] Descripcion: {pieza_encontrada['descripcion']}")
    print(f"[-] Precio:      USD {pieza_encontrada['precio']}")
    print(f"[-] Stock:       {pieza_encontrada['stock']} unidades disponibles")
else:
    print(f"\n[X] Esa pieza no esta registrada en el stock, che. Buscaste: '{buscar}'")

print("\n=============================================")
print("🏁 Fin del test de ejercicios. ¡Todo de 10!")
print("=============================================")
