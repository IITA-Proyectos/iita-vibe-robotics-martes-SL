
## PARTE 1: Repaso de la semana pasada

### Ejercicio 1: Creando el perfil de tu Robot 🤖
Vas a crear la ficha técnica de un robot usando variables. En tu programa escribí cuatro variables distintas y luego imprimilas en la pantalla usando `print()`. Además, debés asegurarte de imprimir el **tipo de dato** usando `print(type(nombre_de_la_variable))`.

*   El **nombre del robot** (texto).
*   El **separacion de ruedas** (número con decimales, por ej: 85.5).
*   La **cantidad de motores** que tiene (número entero).
*   **Si está encendido o apagado** (verdadero o falso).

### Ejercicio 2: Calculadora Básica 🧮
Escribí un programa en el cual definas dos variables con números (por ejemplo `a = 15` y `b = 4`). El programa debe calcular e imprimir en pantalla el resultado de:
1. La suma (+)
2. La resta (-)
3. La multiplicación (*)
4. La división entera (//)
5. El resto de la división o módulo (%)

### Ejercicio 3: ¿Qué tipo de dato soy? 🕵️‍♂️
Sin correr el código, mirá estas opciones e intentá adivinar qué tipo de dato son (`int`, `float`, `str`, `bool`). Anotá lo que creas y después armá el código en Python usando `print(type(...))` para comprobar si acertaste.
1. `"73"`
2. `False`
3. `3.1415`
4. `99`
5. `True + 1` *(¡Cuidado, esta tiene trampa!)*

---

## PARTE 2: ¡Interactuando con el usuario!

### Ejercicio 4: Conociendo a la persona detrás de la PC 💻
Tu programa de a poco se vuelve más inteligente. Escribí un código que interactúe con la persona en el teclado:
1. Pedile su **nombre** usando `input()` y guardalo en una variable.
2. Pedile su **edad**. Recordá que `input()` siempre devuelve texto, así que deberemos transformarlo usando `int()`.
3. Informale en la pantalla cuántos años va a tener dentro de **10 años**, concatenando para que el mensaje quede prolijo.
   *Ejemplo esperado: "Hola Facu, dentro de 10 años vas a tener 25 años."*

### Ejercicio 5: Presupuesto del Proyecto Arduino 🔌
Sos el encargado de las compras para armar un proyecto nuevo. Tu programa en Python te va a ayudar a no equivocarte con los cálculos de plata.
1. Preguntale al usuario cuánto cuesta un paquete de **luces LED** (tiene que poder ingresar números con coma o decimales, por ejemplo `1200.50`. Usá `float()`).
2. Preguntale qué cantidad de paquetes necesita comprar (usá `int()`).
3. Calculá el **costo total**.
4. Imprimí un recibo final usando f-strings donde aparezca el precio del LED, la cantidad y el costo total a pagar.

### Ejercicio 6: Buscando el Error (Debugging 🦠)
Un compañero intentó hacer un programa para calcular el doble de una distancia. El problema es que a él, si le ingresa `5`, en lugar de decirle `10` matemáticamente, le dice `55` (como texto pegado).
**Copiá el código en tu compu, fijate qué falla y arreglalo para que funcione bien.**

```python
distancia = input("Ingresá los centímetros que caminó el robot: ")
doble_distancia = distancia * 2
print(f"El robot debería caminar {doble_distancia} cm en total")
```
