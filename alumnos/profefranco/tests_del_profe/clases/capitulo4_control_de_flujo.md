# Control de Flujo

## Índice

1. [¿Qué es el control de flujo?](#qué-es-el-control-de-flujo)
2. [Definición de bloques e indentación](#definición-de-bloques-e-indentación)
3. [Comentarios](#comentarios)
4. [Operadores de comparación](#operadores-de-comparación)
5. [Operadores lógicos](#operadores-lógicos)
6. [Condicionales](#condicionales)
   - [if / else](#if--else)
   - [elif](#elif)
   - [Asignación condicional (en una línea)](#asignación-condicional-en-una-línea)
   - [match-case](#match-case)
7. [Bucles](#bucles)
   - [while](#while)
   - [for y range()](#for-y-range)
   - [break y continue](#break-y-continue)
   - [Bucles anidados](#bucles-anidados)

---

## ¿Qué es el control de flujo?

Todo programa se ejecuta de **arriba hacia abajo**, línea por línea, como si leyéramos un libro. A eso se le llama **flujo** del programa.

El **control de flujo** son las herramientas que nos permiten modificar ese orden: que el programa tome decisiones, repita instrucciones o se salte partes según ciertas condiciones.

Existen tres tipos:
- **Secuencial:** una instrucción sigue a la otra (lo que veníamos haciendo).
- **Selectiva:** el programa toma decisiones (condicionales).
- **Repetitiva:** el programa repite instrucciones (bucles/ciclos).

---

## Definición de bloques e indentación

En otros lenguajes de programación los bloques de código se definen con llaves `{}`. Python decidió no usarlas y en su lugar usa **espacios en blanco**, preferiblemente **4 espacios** (o la tecla `TAB`).

A esto se le llama **indentación** y es fundamental: si no indentás bien, el programa falla o hace cosas que no esperamos.

```python
if temperatura > 35:
    print('Hace mucho calor')  # esto está dentro del if (4 espacios)
print('Esto siempre se ejecuta')  # esto está fuera del if
```

---

## Comentarios

Los comentarios son anotaciones en el código que el intérprete de Python **ignora completamente**. Sirven para explicar qué hace el código, dejar notas o desactivar líneas temporalmente.

En Python tenemos dos formas:
- **`#`** → Comenta una sola línea.
- **`''' '''`** → Comenta múltiples líneas.

```python
# Esto es un comentario de una línea
edad_universo = 13800 * (10 ** 6) * 365

'''
Esto es un comentario
que ocupa varias líneas
'''
```

> **Consejo:** Un buen comentario explica el *por qué* de algo, no el *qué*. Si el código es claro, no hace falta comentar cada línea.

---

## Operadores de comparación

Para tomar decisiones necesitamos **comparar valores**. Estas comparaciones siempre devuelven `True` o `False`.

| Operador | Descripción | Ejemplo | Resultado |
|:---:|:---:|:---:|:---:|
| `==` | Igual a | `5 == 5` | `True` |
| `!=` | Distinto de | `5 != 3` | `True` |
| `>` | Mayor que | `5 > 3` | `True` |
| `<` | Menor que | `5 < 3` | `False` |
| `>=` | Mayor o igual que | `5 >= 5` | `True` |
| `<=` | Menor o igual que | `5 <= 3` | `False` |

> **Importante:** No confundir `=` (asignación, le damos valor a una variable) con `==` (comparación, preguntamos si dos cosas son iguales). ¡Es un error muy común al principio!

Python también permite encadenar comparaciones de forma directa:

```python
>>> 4 <= x <= 12  # equivale a x >= 4 and x <= 12
True
```

---

## Operadores lógicos

Cuando necesitamos combinar varias condiciones usamos los **operadores lógicos**: `and`, `or` y `not`.

```python
>>> x = 8
>>> x > 4 and x < 12   # las dos deben cumplirse → True
True
>>> x < 4 or x > 12    # basta con que una se cumpla → False
False
>>> not(x != 8)        # niega el resultado → True
True
```

**Cortocircuito lógico:** Python es inteligente y no evalúa toda la expresión si ya sabe el resultado:
- En un `and`, si la primera condición es `False`, ya sabe que el resultado es `False` y no sigue evaluando.
- En un `or`, si la primera condición es `True`, ya sabe que el resultado es `True` y no sigue evaluando.

---

## Condicionales

### if / else

La sentencia `if` nos permite ejecutar un bloque de código **solo si se cumple una condición**. Se escribe con la condición seguida de dos puntos `:` y el bloque indentado.

```python
temperatura = 40
if temperatura > 35:
    print('Aviso por alta temperatura')
else:
    print('Parámetros normales')
```

Si la condición del `if` no se cumple, el programa ejecuta lo que esté en el `else`.

### elif

Cuando necesitamos evaluar **más de dos casos**, usamos `elif` (abreviatura de "else if"). Podemos poner todos los `elif` que necesitemos entre el `if` y el `else`.

```python
temperatura = 28
if temperatura < 10:
    print('Nivel azul')
elif temperatura < 20:
    print('Nivel verde')
elif temperatura < 30:
    print('Nivel naranja')
else:
    print('Nivel rojo')
```

Python evalúa las condiciones **de arriba hacia abajo** y ejecuta el primer bloque cuya condición se cumpla. El resto los ignora.

### Asignación condicional (en una línea)

Hay una forma compacta de escribir un `if/else` cuando lo usamos para asignar un valor a una variable:

```python
# Forma clásica
if temperatura < 30:
    riesgo = 'BAJO'
else:
    riesgo = 'ALTO'

# Forma compacta (una sola línea)
riesgo = 'BAJO' if temperatura < 30 else 'ALTO'
```

### match-case

Desde Python 3.10 existe la sentencia `match-case`, que funciona como un conjunto de `if` encadenados pero mucho más ordenado. Es ideal cuando queremos comparar una variable contra muchos valores posibles.

```python
color = '#FF0000'
match color:
    case '#FF0000':
        print('Rojo')
    case '#00FF00':
        print('Verde')
    case '#0000FF':
        print('Azul')
    case _:           # el guión bajo actúa como "en cualquier otro caso"
        print('Color desconocido')
```

> **Nota:** El `case _` es el equivalente al `else`: se ejecuta si ningún otro caso coincide.

---

## Bucles

### while

El bucle `while` repite un bloque de código **mientras una condición se siga cumpliendo**. La idea es: *"Mientras esto sea verdad, hacé esto"*.

```python
quiere_saludo = 'S'
while quiere_saludo == 'S':
    print('¡Hola, qué tal!')
    quiere_saludo = input('¿Querés otro saludo? [S/N]: ')
print('¡Que tengas un buen día!')
```

> **Cuidado con el bucle infinito:** Si la condición nunca deja de cumplirse, el programa corre para siempre. Para frenarlo podés usar `CTRL + C` en la terminal.

```python
# Bucle infinito intencional (útil a veces)
while True:
    nota = float(input('Ingresá una nota: '))
    if nota < 0 or nota > 10:
        print('Nota fuera de rango')
        break   # salimos del bucle
    print(nota)
```

### for y range()

El bucle `for` recorre los elementos de algo **iterable** (una cadena de texto, una lista, un rango de números, etc.) y ejecuta el bloque una vez por cada elemento.

```python
# Recorrer una cadena de texto
for letra in 'Python':
    print(letra)
```

Para repetir algo una cantidad determinada de veces usamos `range()`:

```python
# range(stop) → del 0 al stop-1
for i in range(5):
    print(i)   # imprime 0, 1, 2, 3, 4

# range(inicio, fin, paso)
for numero in range(2, 11, 2):
    print(numero)   # imprime 2, 4, 6, 8, 10
```

`range()` acepta tres parámetros:
- **inicio** (opcional, por defecto es `0`)
- **fin** (obligatorio, llega hasta `fin - 1`)
- **paso** (opcional, por defecto es `1`)

> **Nota:** Recordá que en programación siempre empezamos a contar desde `0`. Por eso `range(5)` nos da los números del `0` al `4`.

Si no necesitamos usar la variable del bucle (solo queremos repetir algo N veces), la convención es usar `_` como nombre:

```python
for _ in range(3):
    print('Repetido 3 veces')
```

### break y continue

Dos herramientas para controlar el flujo dentro de un bucle:

- **`break`** → Rompe el bucle y sale de él inmediatamente.
- **`continue`** → Salta el resto del código en la iteración actual y pasa a la siguiente.

```python
# break: para cuando encuentra la letra 't'
for letra in 'Python':
    if letra == 't':
        break
    print(letra)   # imprime P, y

# continue: se salta las vocales
for letra in 'Python':
    if letra in 'aeiou':
        continue
    print(letra)   # imprime P, y, t, h, n
```

### Bucles anidados

Es posible poner un bucle **dentro de otro bucle**. Por cada iteración del bucle externo, el bucle interno completa todas sus iteraciones.

```python
# Tablas de multiplicar del 1 al 3
for tabla in range(1, 4):
    for factor in range(1, 10):
        resultado = tabla * factor
        print(f'{tabla} x {factor} = {resultado}')
```

> **Tené en cuenta:** Cada nivel de anidamiento que agregás hace que el código sea más lento y más difícil de leer. Usarlo con criterio.

---

## Ejercicios Prácticos

1. **La temperatura:** Pedí una temperatura por teclado. Si es mayor a 35 mostrá "Calor extremo", si está entre 20 y 35 mostrá "Temperatura agradable", si es menor a 20 mostrá "Frío".
2. **El número secreto:** Hacé un programa que tenga un número secreto guardado en una variable. Con un `while`, pedile al usuario que adivine el número. Mostrá si se pasó, si quedó corto o si acertó. Cuando acierte, salí del bucle.
3. **Contar vocales:** Dada una cadena de texto, usá un `for` para contar cuántas vocales tiene e imprimí el resultado.
4. **Tabla de multiplicar:** Pedí un número y usá un `for` con `range()` para mostrar su tabla de multiplicar completa (del 1 al 10).
