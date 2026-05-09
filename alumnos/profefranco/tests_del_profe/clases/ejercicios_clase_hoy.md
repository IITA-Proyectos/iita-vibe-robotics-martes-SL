# Ejercicios Prácticos: Variables, Tipos e Interacción

En este archivo encontrarás ejercicios tanto de los conceptos de la semana pasada (variables, tipos de datos) como los temas nuevos (interacción con el usuario). Podés dárselos a los chicos para que los resuelvan directamente en Python.

---

## PARTE 1: Repaso de Variables y Tipos (Semana Pasada)

### Ejercicio 1: Creando el perfil del Robot
Definí cuatro variables con la información de un robot ficticio y luego imprimí cada variable en la consola usando `print()`. Además, debés asegurarte de imprimir el **tipo de dato** usando `type()`.
*   El nombre del robot (cadena de texto).
*   El nivel de batería (número flotante decimal, por ej: 85.5).
*   La cantidad de motores que tiene (número entero).
*   Si está encendido o apagado (booleano).

### Ejercicio 2: Operaciones Matemáticas Básicas
Escribí un programa en el cual definas dos variables con números (por ejemplo `a = 15` y `b = 4`). El programa debe calcular e imprimir:
1. La suma
2. La resta
3. La multiplicación
4. La división entera (`//`)
5. El resto o módulo (`%`)

### Ejercicio 3: ¿Qué tipo de dato soy?
Sin correr el código, observá las siguientes palabras o números y tratá de pensar de qué tipo de dato son (`int`, `float`, `str`, `bool`). Luego verificalo armando el código en Python.
1. `"73"`
2. `False`
3. `3.1415`
4. `99`
5. `True + 1` *(¡Ojo con este, acordate de la conversión implícita!)*

---

## PARTE 2: Interacción con el usuario (Tema Nuevo)

### Ejercicio 4: ¡Conociendo al usuario!
La computadora de a poco se vuelve más inteligente. Escribí un programa que interactúe de verdad con la persona en el teclado:
1. Pedirle su nombre usando `input()` y guardarlo en una variable.
2. Pedirle su edad sabiendo que `input()` devuelve texto, así que deberemos transformarlo usando `int()`.
3. Informarle en la pantalla cuántos años va a tener dentro de **10 años**, utilizando "f-strings" para un mensaje prolijo.
   *Ejemplo esperado: "Hola Facu, dentro de 10 años vas a tener 25 años".*

### Ejercicio 5: Presupuesto del Proyecto Arduino
Sos el encargado de las compras para armar un proyecto nuevo. Tu programa en Python te va a ayudar a no equivocarte con las calculadoras.
1. Preguntarle al usuario cuánto cuesta el paquete de **luces LED** (tiene que poder ingresar números con coma o decimales, por ejemplo `1200.50`, usar `float()`).
2. Preguntarle qué cantidad de paquetes necesita comprar (usar `int()`).
3. Calcular el **costo total**.
4. Imprimir un recibo final usando f-strings donde aparezca el precio del led, la cantidad y el precio total.

### Ejercicio 6: Buscando el Error (Debugging)
Un alumno de otra clase intentó hacer un pequeño programa para calcular el doble de una distancia ingresada por la pantalla, pero cuando lo ejecuta, en lugar de duplicar el número matemáticamente, ¡lo repite dos veces como si fuera texto! Por ejemplo si ingreso 5, saca 55.
**¿Dónde está el error y cómo lo arreglamos?**

```python
# Código con errores
distancia = input("Ingresá los centímetros que caminó el robot: ")
doble_distancia = distancia * 2
print(f"El robot debería caminar {doble_distancia} cm en total")
```

---
*💡 **Tip para el profe:** En el último ejercicio, la clave está en que los chicos se den cuenta solos de que hay que arropar el `input()` con un `int()` o `float()` para que python deje de tratar al 5 como la letra "5" y pase a multiplicarlo como un número de verdad.*
