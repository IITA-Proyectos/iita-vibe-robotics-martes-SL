# Gran Repaso de Python: Cuestionario de 50 Preguntas — Clase 10

Este documento contiene un banco de 50 preguntas de tipo Multiple Choice agrupadas en 5 categorías temáticas para repasar todos los conceptos vistos en el curso. Al final se incluye el "Machete del Profe" con la clave de respuestas correctas y sugerencias didácticas.

---

## 📂 Categoría 1: Conceptos Básicos, Variables y Tipos de Datos

### 1. ¿Cómo se define correctamente una variable en Python?
*   a) `variable = 10`
*   b) `10 = variable`
*   c) `variable := 10`
*   d) `let variable = 10`

### 2. Si escribís `valor = 8.5`, ¿de qué tipo es la variable `valor`?
*   a) `int` (entero)
*   b) `str` (texto)
*   c) `float` (decimal)
*   d) `bool` (booleano)

### 3. ¿Cuál de los siguientes nombres de variables es INVÁLIDO en Python?
*   a) `motor_derecho`
*   b) `motor2`
*   c) `2motor`
*   d) `_motor`

### 4. ¿Qué valor almacena una variable del tipo `bool`?
*   a) Cualquier número entero o decimal.
*   b) Únicamente texto entre comillas.
*   c) Solo valores de verdadero (`True`) o falso (`False`).
*   d) Una lista de elementos ordenados.

### 5. ¿Qué pasa si intentás ejecutar el siguiente código?
```python
lectura = "25"
total = lectura + 5
```
*   a) `total` pasa a valer `30`.
*   b) Da un error porque no podés sumar un texto (`str`) con un número (`int`).
*   c) `total` pasa a valer `"255"`.
*   d) Se borra la variable `lectura`.

### 6. ¿Cómo se convierte correctamente el texto `"100"` a un número entero?
*   a) `int("100")`
*   b) `str(100)`
*   c) `float("100")`
*   d) `convert("100", int)`

### 7. ¿Cuál es la forma correcta de escribir una f-string para mostrar el valor de la variable `velocidad`?
*   a) `print("La velocidad es: velocidad")`
*   b) `print(f"La velocidad es: [velocidad]")`
*   c) `print(f"La velocidad es: {velocidad}")`
*   d) `print("La velocidad es: " + f{velocidad})`

### 8. ¿Para qué sirve la función `type()` en Python?
*   a) Para escribir un texto en la consola.
*   b) Para averiguar de qué tipo de dato es una variable o valor.
*   c) Para borrar la pantalla.
*   d) Para convertir un decimal a entero.

### 9. Si tenés el siguiente código:
```python
a = 15
b = a
a = 20
```
¿Cuánto vale la variable `b` al final?
*   a) `20`
*   b) `15`
*   c) `35`
*   d) Da un error de ejecución.

### 10. ¿Qué operador aritmético usás para obtener el resto (residuo) de una división?
*   a) `/`
*   b) `//`
*   c) `%`
*   d) `**`

---

## 📂 Categoría 2: Estructuras de Control / Condicionales

### 11. En Python, ¿cómo definimos que una línea de código está "adentro" de un bloque `if`?
*   a) Poniendo llaves `{}` alrededor del bloque.
*   b) Escribiendo la palabra `end` al final.
*   c) Dejando una sangría de 4 espacios (indentación).
*   d) Poniendo un punto y coma al final de cada línea.

### 12. ¿Qué carácter es obligatorio poner al final de la línea del `if` o del `else`?
*   a) Un punto y coma `;`
*   b) Dos puntos `:`
*   c) Un punto `.`
*   d) Ninguno, se deja vacío.

### 13. ¿Cuál es el operador correcto para comparar si dos valores son exactamente iguales?
*   a) `=`
*   b) `==`
*   c) `===`
*   d) `is`

### 14. ¿Cuál es el operador que usamos para verificar si dos valores son distintos (diferentes)?
*   a) `!=`
*   b) `<>`
*   c) `!==`
*   d) `not ==`

### 15. Si usamos el operador lógico `and` para unir dos condiciones, ¿cuándo da `True` el resultado?
*   a) Cuando al menos una de las dos condiciones se cumple.
*   b) Cuando ambas condiciones se cumplen al mismo tiempo.
*   c) Cuando ninguna de las dos condiciones se cumple.
*   d) Cuando la primera condición es falsa y la segunda es verdadera.

### 16. Si usamos el operador lógico `or` para unir dos condiciones, ¿cuándo da `True` el resultado?
*   a) Solo si ambas condiciones se cumplen.
*   b) Si al menos una de las dos condiciones se cumple.
*   c) Solo si la primera condición es falsa.
*   d) Nunca, el operador `or` solo sirve para comparar textos.

### 17. ¿Qué hace el operador lógico `not`?
*   a) Suma dos valores booleanos.
*   b) Invierte el valor booleano (si es `True` pasa a `False` y viceversa).
*   c) Compara si un número es menor que otro.
*   d) Termina el programa inmediatamente.

### 18. ¿Cuándo se ejecuta el código que está adentro del bloque `else`?
*   a) Siempre que se ejecute el `if`.
*   b) Únicamente cuando la condición del `if` es falsa (`False`).
*   c) Cuando todas las condiciones del programa son verdaderas.
*   d) Si el usuario presiona una tecla en la consola.

### 19. Si tenés varias condiciones excluyentes en cadena, ¿qué palabra clave usás entre el `if` y el `else`?
*   a) `else if`
*   b) `elif`
*   c) `switch`
*   d) `if else`

### 20. ¿Cuál es el resultado de evaluar la expresión: `not (10 < 5)`?
*   a) `True`
*   b) `False`
*   c) `None`
*   d) Da un error de sintaxis.

---

## 📂 Categoría 3: Bucles y Repeticiones (while, for)

### 21. ¿Cuándo es preferible usar un bucle `while` en lugar de un bucle `for`?
*   a) Cuando sabemos exactamente cuántas vueltas va a dar el bucle.
*   b) Cuando queremos repetir un bloque de código mientras se cumpla una condición, sin saber cuántas vueltas tomará.
*   c) Cuando trabajamos únicamente con listas de sensores.
*   d) Cuando queremos definir una función matemática.

### 22. ¿Qué ocurre si la condición de un bucle `while` es siempre verdadera (`True`) y no hay nada que la cambie adentro?
*   a) El programa da un error de sintaxis y no arranca.
*   b) El bucle se ejecuta una sola vez y se detiene.
*   c) Se genera un bucle infinito y el programa no se detiene nunca.
*   d) La computadora se apaga automáticamente.

### 23. ¿Para qué sirve la palabra clave `break` adentro de un bucle?
*   a) Para pausar la ejecución por unos segundos.
*   b) Para romper el bucle inmediatamente y salir de él.
*   c) Para reiniciar el bucle desde la primera vuelta.
*   d) Para saltar a la siguiente función.

### 24. ¿Cuál es la sintaxis correcta para recorrer todos los elementos de una lista llamada `valores` usando un bucle `for`?
*   a) `for valores in x:`
*   b) `for x in valores:`
*   c) `for x ranges valores:`
*   d) `loop x in valores:`

### 25. ¿Qué números genera la expresión `range(5)`?
*   a) `1, 2, 3, 4, 5`
*   b) `0, 1, 2, 3, 4`
*   c) `0, 1, 2, 3, 4, 5`
*   d) `1, 2, 3, 4`

### 26. ¿Qué números genera la expresión `range(2, 6)`?
*   a) `2, 3, 4, 5`
*   b) `2, 3, 4, 5, 6`
*   c) `1, 2, 3, 4, 5`
*   d) `2, 4, 6`

### 27. ¿Qué números genera la expresión `range(0, 10, 2)`?
*   a) `0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10`
*   b) `0, 2, 4, 6, 8, 10`
*   c) `0, 2, 4, 6, 8`
*   d) `2, 4, 6, 8`

### 28. ¿Qué hace la instrucción `intentos += 1` en un programa?
*   a) Compara si `intentos` es igual a 1.
*   b) Le suma 1 al valor actual de la variable `intentos`.
*   c) Borra el valor de la variable `intentos`.
*   d) Multiplica `intentos` por 1.

### 29. ¿Qué estructura usarías para pedirle una clave al usuario y no dejarlo avanzar hasta que escriba la clave correcta?
*   a) Un condicional `if` simple.
*   b) Un bucle `while` controlado por una condición o bandera.
*   c) Un bucle `for` con `range(10)`.
*   d) Una tupla inmutable.

### 30. Si ejecutás este código:
```python
for i in range(3):
    print("Hola")
```
¿Cuántas veces se muestra la palabra "Hola" en la consola?
*   a) 2 veces.
*   b) 3 veces.
*   c) 4 veces.
*   d) Ninguna, da un error.

---

## 📂 Categoría 4: Listas, Tuplas y Diccionarios

### 31. Si tenés la lista `sensores = ["giroscopio", "ultrasonido", "color"]`, ¿cuál es el índice del elemento `"giroscopio"`?
*   a) `1`
*   b) `0`
*   c) `-1`
*   d) `"giroscopio"`

### 32. ¿Cómo averiguás cuántos elementos tiene una lista llamada `herramientas`?
*   a) `herramientas.length()`
*   b) `len(herramientas)`
*   c) `count(herramientas)`
*   d) `size(herramientas)`

### 33. ¿Qué método usás para agregar un nuevo elemento al final de una lista?
*   a) `.add()`
*   b) `.append()`
*   c) `.push()`
*   d) `.insert()`

### 34. Si tenés la lista `partes = ["motor", "cable", "rueda"]` y ejecutás `partes.pop()`, ¿qué elemento se elimina de la lista?
*   a) `"motor"`
*   b) `"rueda"`
*   c) `"cable"`
*   d) Se eliminan todos.

### 35. ¿Para qué sirve el operador `in` en una lista?
*   a) Para ordenar los elementos alfabéticamente.
*   b) Para verificar si un elemento específico se encuentra dentro de la lista.
*   c) Para cambiar el valor de un elemento de la lista.
*   d) Para convertir la lista en una tupla.

### 36. ¿Cuál es la diferencia principal entre una Lista y una Tupla en Python?
*   a) Las tuplas usan corchetes `[]` y las listas paréntesis `()`.
*   b) Las listas son mutables (se pueden modificar) y las tuplas son inmutables (no se pueden modificar una vez creadas).
*   c) Las tuplas solo pueden almacenar números decimales.
*   d) Las listas no permiten elementos repetidos y las tuplas sí.

### 37. ¿Qué estructura de datos almacena información mediante parejas de "clave-valor"?
*   a) La Tupla.
*   b) La Lista.
*   c) El Diccionario.
*   d) El Bucle.

### 38. Si tenés el diccionario `robot = {"nombre": "Wall-E", "motores": 2}`, ¿cómo accedés al valor de la clave `"nombre"`?
*   a) `robot[0]`
*   b) `robot["nombre"]`
*   c) `robot.key("nombre")`
*   d) `robot(nombre)`

### 39. ¿Qué ventaja tiene usar el método `.get()` para buscar un elemento en un diccionario en lugar de usar corchetes?
*   a) Hace que el programa corra el doble de rápido.
*   b) Si la clave buscada no existe, no da un error que detenga el programa, sino que devuelve `None`.
*   c) Convierte automáticamente el diccionario en una lista.
*   d) Permite guardar claves duplicadas.

### 40. Si tenés el diccionario `estado = {"bateria": 90}` y querés actualizar el valor de `"bateria"` a 80, ¿qué instrucción usás?
*   a) `estado["bateria"] = 80`
*   b) `estado.update("bateria", 80)`
*   c) `estado("bateria") = 80`
*   d) `estado = {"bateria" : 80}` (borrando el resto del diccionario)

---

## 📂 Categoría 5: Funciones, Parámetros y Librerías

### 41. ¿Con qué palabra reservada empezamos a definir una función en Python?
*   a) `function`
*   b) `def`
*   c) `create`
*   d) `void`

### 42. Para ejecutar (invocar) una función que definiste, ¿qué tenés que poner obligatoriamente al final de su nombre?
*   a) Dos puntos `:`
*   b) Paréntesis `()`
*   c) Llaves `{}`
*   d) La palabra `run`

### 43. Al definir una función en `def configurar_robot(velocidad):`, la variable `velocidad` declarada en los paréntesis es un...
*   a) Argumento.
*   b) Parámetro.
*   c) Módulo.
*   d) Retorno.

### 44. Cuando llamás a la función haciendo `configurar_robot(400)`, el valor real `400` que le enviás es un...
*   a) Parámetro.
*   b) Argumento.
*   c) Operador.
*   d) Variable local.

### 45. Si una función está definida con dos parámetros obligatorios y la llamás pasándole solo un argumento, ¿qué pasa?
*   a) El programa asigna `0` al segundo parámetro y funciona igual.
*   b) Da un error de tipo `TypeError` porque falta un argumento obligatorio.
*   c) El programa se saltea esa línea sin hacer nada.
*   d) Se crea un bucle infinito.

### 46. ¿Qué palabra clave usamos para traer a nuestro programa herramientas externas de una librería?
*   a) `include`
*   b) `import`
*   c) `require`
*   d) `use`

### 47. ¿Qué instrucción usás para borrar por completo la consola de comandos de Windows?
*   a) `os.system('cls')`
*   b) `os.clear()`
*   c) `time.clean()`
*   d) `system.cls()`

### 48. ¿Cómo pausás la ejecución de tu programa por 3 segundos usando la librería `time`?
*   a) `time.pause(3)`
*   b) `time.sleep(3)`
*   c) `time.wait(3000)`
*   d) `sleep.time(3)`

### 49. ¿Qué pasa con una variable que creás adentro de una función?
*   a) Se puede usar en cualquier parte del archivo sin problemas.
*   b) Solo existe adentro de esa función y se destruye cuando la función termina (ámbito local).
*   c) Se guarda automáticamente en GitHub.
*   d) Se convierte en una constante.

### 50. Si definís una función que recibe dos parámetros, ¿cómo los separás dentro de los paréntesis?
*   a) Con un punto y coma `;`
*   b) Con una coma `,`
*   c) Con la palabra `and`
*   d) Con una barra `/`

---
---

# 📋 Machete del Profe

## Clave de Respuestas Correctas

### Categoría 1
1. **a** | 2. **c** | 3. **c** (los nombres no pueden arrancar con números) | 4. **c** | 5. **b** (error de tipos) | 6. **a** | 7. **c** (sintaxis de f-string) | 8. **b** | 9. **b** (b copia el valor de a antes de que a cambie a 20) | 10. **c** (operador módulo)

### Categoría 2
11. **c** (indentación de 4 espacios) | 12. **b** (dos puntos obligatorios) | 13. **b** | 14. **a** | 15. **b** | 16. **b** | 17. **b** | 18. **b** | 19. **b** | 20. **a** (10 < 5 es False, y not False es True)

### Categoría 3
21. **b** | 22. **c** (bucle infinito) | 23. **b** | 24. **b** | 25. **b** (genera del 0 al 4) | 26. **a** (genera del 2 al 5) | 27. **c** (genera 0, 2, 4, 6, 8. El 10 queda afuera por límite exclusivo) | 28. **b** | 29. **b** | 30. **b** (del 0 al 2)

### Categoría 4
31. **b** (los índices arrancan en 0) | 32. **b** | 33. **b** | 34. **b** (elimina el último elemento) | 35. **b** | 36. **b** | 37. **c** | 38. **b** | 39. **b** | 40. **a**

### Categoría 5
41. **b** | 42. **b** (los paréntesis obligatorios para llamar) | 43. **b** | 44. **b** | 45. **b** (TypeError) | 46. **b** | 47. **a** | 48. **b** (los segundos van directamente como argumento) | 49. **b** | 50. **b**

---

## Sugerencias Didácticas para la Clase (Gamificación)

*   **Opción A (Kahoot/Quizziz):** Si hay acceso a internet y celulares/compus, podés cargar estas preguntas en una plataforma de cuestionarios en vivo. A los chicos les encanta competir por puntos de velocidad.
*   **Opción B (Trivia por Equipos):** Dividí la clase en dos equipos (por ejemplo, el equipo "R2-D2" y el equipo "Wall-E"). Proyectá el archivo en la pantalla. Alternando turnos, cada equipo elige una categoría y una pregunta.
    *   Si responden bien: +10 puntos.
    *   Si responden mal: el otro equipo tiene posibilidad de "robo".
*   **Opción C (Tarjetas de Colores):** Repartí a cada alumno 4 papeles de colores (Rojo = A, Azul = B, Verde = C, Amarillo = D). Leés la pregunta y a la cuenta de tres todos levantan su color. Esto te permite ver al instante quién está perdido y quién lo entendió sin que nadie se sienta expuesto.

### Plan de Acción por Alumno (Basado en el Diagnóstico)
*   **Laureano:** Este cuestionario es clave para él. Prestale mucha atención a cómo responde las preguntas de la **Categoría 3 (Bucles)** y **Categoría 5 (Funciones)**. Si ves que duda con `range()` o la sintaxis del `def`, aprovechá las explicaciones rápidas en la pizarra para consolidar esos cimientos antes de pasar a la práctica.
*   **Diego:** Observá su rendimiento en preguntas de sintaxis (como las comillas, las llaves `{}` de las f-strings o los dos puntos). Esto le va a servir para corregir la desprolijidad que a veces tiene al codificar.
*   **Máximo:** Es bueno midiendo lógica general pero a veces se olvida de detalles. Desafialo a responder las preguntas de tipo de dato implícito/conversiones (como la pregunta 5 y la 20).
*   **Juanse:** Al venir muy avanzado, podés usarlo como "validador" o "árbitro" cuando surjan dudas. Si un equipo se equivoca, pedile a Juanse que intente explicar *por qué* la opción elegida no era correcta. Esto lo mantiene enganchado y ayuda a sus compañeros.
