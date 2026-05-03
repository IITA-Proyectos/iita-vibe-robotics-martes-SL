# Estado de Entregas

| Alumno | Práctica 1 | Práctica 2 |
| :--- | :---: | :---: |
| Juanse | ✅ | ❌ |
| Laureano | ⏳ | ❌ |
| Máximo | ✅ | ❌ |
| Octavio | ❌ | ❌ |

*✅ Entregado | ⏳ Incompleto | ❌ Pendiente*

# Índice

- [Practica 1](#practica-1)
- [Practica 2](#practica-2)

# Practica 1

1. **Mensaje simple**: Almacene un mensaje en una variable e imprímalo en pantalla. Después cambie el valor del mensaje e imprímalo nuevamente. 

2. Almacene el nombre de una persona en una variable, imprima un mensaje para esa persona. Por ejemplo *"Hola Fede, ¿te gustaría aprender a programar?"*

3. **El número ocho**: Escriba una suma, resta, multiplicación y división que resulten cada una en el número ocho. Asegúrese de utilizar la función `print()` para ver los resultados en pantalla. Un ejemplo de línea es el siguiente: 
   ```python
   print(5 + 3)
   ```
   La salida debería mostrar el número ocho tantas veces como líneas haya escrito. 

4. Cree cuatro variables llamadas `mi_entero`, `mi_decimal`, `mi_string` y `mi_booleano`. Asigne a cada variable un valor del tipo que le corresponda. Luego muestre en pantalla el tipo de cada variable usando la función `type()` combinando todo con la función `print()`: 
   ```python
   print(type(mi_entero))
   print(type(mi_booleano))
   ```
   La salida final debería contener las siguientes líneas (no importa el orden): 
   ```text
   <class 'int'> 
   <class 'float'> 
   <class 'str'> 
   <class 'bool'>
   ```

5. Escriba un programa que acepte un numero decimal como entrada y lo imprima sin lugares decimales. 

6. ¿Cuál es la salida de las siguientes expresiones? 
   - `1 != 2` -> Salida: 
   - `True == 1` -> Salida: 
   - `False != False` -> Salida: 
   - `False > 0` -> Salida: 
   - `1.0 < 1` -> Salida: 
   - `"test" == "test"` -> Salida: 
   - `1.0 >= 1` -> Salida: 

7. **(Opcional)** Escriba un programa que le pida al usuario que ingrese nombre y edad. Luego muestre un mensaje donde le informe el año en que va a cumplir 100. 

8. **(Opcional)** Escriba un programa que permita convertir una temperatura en Celsius a la escala Farenheit. La fórmula es: 
   `Fahrenheit = (9.0/5.0) * Celsius + 32`

9. **(Opcional) Calculadora simple**: Cree un programa capaz de pedir dos números al usuario y devolver el resultado de la suma, resta, multiplicación y división entre los mismos. Por ejemplo, si los números son 3 y 5, la calculadora nos devolvería: `3+5`; `3-5`; `3*5` y `3/5`. Pruebe también expandir su calculadora y agregar nuevas operaciones, tales como la potenciación o la división entera.

# Practica 2

1. Solicitar al usuario un número de cliente. Si el número es el 1000, imprimir "Ganaste un premio".

   **Ejemplos de ejecución:**
   ```text
   Ingrese su número de cliente: 1000
   ¡Ganaste un premio!
   
   Ingrese su número de cliente: 123
   ```

2. Escribir un programa que pregunte al usuario su edad y muestre por pantalla si es mayor de edad o no.

   **Ejemplos de ejecución:**
   ```text
   ¿Cuál es tu edad?: 20
   Eres mayor de edad.
   
   ¿Cuál es tu edad?: 15
   No eres mayor de edad.
   ```

3. Escribir un programa que pida al usuario un número entero y muestre por pantalla si es par o impar.

   **Ejemplos de ejecución:**
   ```text
   Ingrese un número entero: 4
   El número es par.
   
   Ingrese un número entero: 7
   El número es impar.
   ```

4. Solicitar al usuario que ingrese dos números y mostrar cuál de los dos es menor. No considerar el caso en que ambos números son iguales.

   **Ejemplo de ejecución:**
   ```text
   Ingrese el primer número: 8
   Ingrese el segundo número: 5
   El número menor es 5.
   ```

5. Requerir al usuario que ingrese un día de la semana e imprimir un mensaje si es lunes, otro mensaje diferente si es viernes, otro mensaje diferente si es sábado o domingo. Si el día ingresado no es ninguno de esos, imprimir otro mensaje.

   **Ejemplos de ejecución:**
   ```text
   Ingrese un día de la semana: lunes
   ¡Buena semana, a trabajar!
   
   Ingrese un día de la semana: viernes
   ¡Por fin es viernes!
   
   Ingrese un día de la semana: domingo
   ¡A descansar que es fin de semana!
   
   Ingrese un día de la semana: martes
   Es un día de semana normal.
   ```

6. Escriba un programa que pida el año actual y un año cualquiera y que escriba cuántos años han pasado desde ese año o cuántos años faltan para llegar a ese año.

   **Ejemplos de ejecución:**
   ```text
   COMPARADOR DE AÑOS
   ¿En qué año estamos?: 2019
   Escriba un año cualquiera: 2024
   Para llegar al año 2024 faltan 5 años.
   
   COMPARADOR DE AÑOS
   ¿En qué año estamos?: 2019
   Escriba un año cualquiera: 1997
   Desde el año 1997 han pasado 22 años.
   
   COMPARADOR DE AÑOS
   ¿En qué año estamos?: 2019
   Escriba un año cualquiera: 2019
   ¡Son el mismo año!
   ```
