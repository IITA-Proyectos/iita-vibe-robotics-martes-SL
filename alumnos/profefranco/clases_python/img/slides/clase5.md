# Control de Flujo — Clase

---

## 🖥️ Diapositiva 1 — ¿Qué es el flujo de un programa?

Todo programa se ejecuta **de arriba hacia abajo**, línea por línea, como si leyéramos un libro.

```python
print('Primero')
print('Segundo')
print('Tercero')
```

A eso se le llama el **flujo** del programa.

Pero no siempre queremos que vaya recto. Existen tres tipos de flujo:

- **Secuencial** → una instrucción sigue a la otra (lo que veníamos haciendo).
- **Selectivo** → el programa toma una decisión y va por un camino u otro.
- **Repetitivo** → el programa repite un bloque de código varias veces.

---

## 🖥️ Diapositiva 2 — ¿Qué es una condición?

Una **condición** es una pregunta que el programa se hace a sí mismo.
La respuesta siempre es una de dos cosas: **`True`** (verdadero) o **`False`** (falso).

```python
temperatura = 40
temperatura > 35   # → True
temperatura < 10   # → False
```

```python
tiene_permiso = True
tiene_permiso        # → True
```

> A estos valores se los llama **booleanos**. Son la base de toda decisión en programación.

---

## 🖥️ Diapositiva 3 — if / else

Con `if` le decimos al programa: *"si esto es verdad, hacé esto"*.
Con `else` le decimos: *"si no, hacé esto otro"*.

```python
temperatura = 40

if temperatura > 35:
    print('Aviso por alta temperatura')
else:
    print('Parámetros normales')
```

> La línea del `if` siempre termina con `:` y el bloque adentro va con **4 espacios** (indentación).

---

## 🖥️ Diapositiva 4 — elif

¿Qué pasa si tenemos **más de dos casos**? Usamos `elif`.

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

> Python evalúa de arriba hacia abajo y ejecuta **el primero que se cumpla**. El resto los ignora.

---

## 🖥️ Diapositiva 5 — Operadores de comparación

| Operador | Significado | Ejemplo | Resultado |
|:---:|:---:|:---:|:---:|
| `==` | Igual a | `5 == 5` | `True` |
| `!=` | Distinto de | `5 != 3` | `True` |
| `>` | Mayor que | `5 > 3` | `True` |
| `<` | Menor que | `5 < 3` | `False` |
| `>=` | Mayor o igual | `5 >= 5` | `True` |
| `<=` | Menor o igual | `5 <= 3` | `False` |

> ⚠️ `=` **asigna** un valor. `==` **compara** dos valores. ¡No confundirlos!

---

## 🖥️ Diapositiva 6 — Ejemplos con comparadores

```python
edad = 17

if edad >= 18:
    print('Sos mayor de edad')
else:
    print('Sos menor de edad')
```

```python
nota = 6

if nota == 10:
    print('Perfecto')
elif nota >= 6:
    print('Aprobado')
else:
    print('Reprobado')
```

```python
numero = 7

if numero != 0:
    print('El número no es cero')
```

---

## 🖥️ Diapositiva 7 — and / or

Sirven para **combinar condiciones**.

**`and`** → las **DOS** condiciones deben cumplirse:
```python
edad = 20
tiene_dni = True

if edad >= 18 and tiene_dni:
    print('Puede votar')
```

**`or`** → basta con que **una** se cumpla:
```python
es_finde = True
es_feriado = False

if es_finde or es_feriado:
    print('No hay clase')
```

---

## 🖥️ Diapositiva 8 — Ejemplos con and / or

```python
# Para entrar al boliche necesitás las dos cosas
edad = 20
tiene_entrada = True

if edad >= 18 and tiene_entrada:
    print('Pasá')
else:
    print('No podés entrar')
```

```python
# Para que suene la alarma basta con una
hay_humo = False
hay_fuego = True

if hay_humo or hay_fuego:
    print('🚨 ALARMA')
```

```python
# Se pueden combinar
bateria = 80
wifi = True
modo_avion = False

if bateria > 20 and wifi and not modo_avion:
    print('Podés hacer la videollamada')
```

---

## 🖥️ Diapositiva 9 — ¿Para qué sirven los bucles?

Imaginá que querés imprimir los números del 1 al 5. Fácil:

```python
print(1)
print(2)
print(3)
print(4)
print(5)
```

Ahora imaginá que te pido los números del 1 al **100**.
¿Escribirías 100 `print`? 😅

Para eso existen los **bucles**. Hay dos tipos:
- **`while`** → repite *mientras* una condición sea verdadera.
- **`for`** → repite una cantidad *determinada* de veces.

---

## 🖥️ Diapositiva 10 — while

*"Mientras esto sea verdad, hacé esto."*

```python
numero = 1

while numero <= 5:
    print(numero)
    numero += 1
```

Salida: `1  2  3  4  5`

> ⚠️ Si la condición **nunca deja de cumplirse**, el programa corre para siempre. A esto se le llama **bucle infinito**. Para frenarlo: `CTRL + C`.

---

## 🖥️ Diapositiva 11 — while con input

Un uso muy común: seguir preguntándole al usuario hasta que decida parar.

```python
respuesta = 'S'

while respuesta == 'S':
    print('¡Hola, qué tal!')
    respuesta = input('¿Querés otro saludo? [S/N]: ')

print('¡Que tengas un buen día!')
```

> El bucle sigue mientras el usuario responda `S`. Cuando responde `N`, se detiene.

---

## 🖥️ Diapositiva 12 — for y range()

El `for` recorre una secuencia de valores, uno por uno.
`range()` genera esa secuencia de números automáticamente.

```python
for i in range(5):
    print(i)
# Imprime: 0, 1, 2, 3, 4
```

También podemos indicar inicio, fin y paso:
```python
# range(inicio, fin, paso)
for numero in range(2, 11, 2):
    print(numero)
# Imprime: 2, 4, 6, 8, 10
```

> `range(5)` va del `0` al `4`. El número del final **nunca se incluye**.

---

## 🖥️ Diapositiva 13 — Ejemplos con for

```python
# Los 100 números... en 2 líneas 😎
for i in range(1, 101):
    print(i)
```

```python
# Tabla de multiplicar del 7
for i in range(1, 11):
    print(f'7 x {i} = {7 * i}')
```

```python
# Recorrer una palabra letra por letra
for letra in 'Python':
    print(letra)
```

---

## 🖥️ Diapositiva 14 — Ejercicios

1. **La temperatura:** Pedí una temperatura. Mostrá si es "Frío" (menos de 15), "Agradable" (entre 15 y 30) o "Calor" (más de 30).

2. **¿Puedo entrar?:** Pedí la edad del usuario y si tiene entrada (`True`/`False`). Con `and`, mostrá si puede entrar al evento (mayores de 16 con entrada).

3. **El número secreto:** Guardá un número en una variable. Con `while`, pedile al usuario que adivine. Avisale si se pasó o quedó corto. Cuando acierte, salí del bucle.

4. **Tabla de multiplicar:** Pedí un número y usá `for` con `range()` para mostrar su tabla del 1 al 10.

---
---

# 📋 Machete del Profe

---

### Diapositiva 1 — ¿Qué es el flujo de un programa?
Arrancar con la idea de que todo programa se lee de arriba para abajo, como un libro. Escribir los tres tipos en el pizarrón antes de avanzar: secuencial (lo que ya saben hacer), selectivo (el programa elige un camino) y repetitivo (repite cosas, que es lo nuevo de hoy). Es una diapo de contexto, no tiene código complejo, sirve para encuadrar toda la clase.

---

### Diapositiva 2 — ¿Qué es una condición?
Arrancar preguntando: "¿alguien me da un ejemplo de condición de la vida real?" (si llueve agarro paraguas, si tengo hambre como, etc.). Después llevar eso a programación: el programa se hace una pregunta y la respuesta siempre es sí o no, `True` o `False`. Mostrar los ejemplos y ejecutarlos en el editor para que vean cómo Python devuelve el booleano directamente. No profundizar demasiado, es el calentamiento.

---

### Diapositiva 3 — if / else
Explicar la estructura paso a paso: el `if`, la condición, los dos puntos, y los 4 espacios adentro. Hacer énfasis en la indentación porque si no la ponen Python tira error. Mostrar el ejemplo y después cambiar el valor de `temperatura` a 20 para que vean el `else` en acción. El `else` no tiene condición, es simplemente "en todos los demás casos".

---

### Diapositiva 4 — elif
Preguntar: "¿qué pasa si en vez de dos casos tenemos cuatro?". Mostrar que con solo `if/else` se puede pero queda muy anidado y feo. El `elif` lo hace limpio y ordenado. Remarcar bien que Python evalúa de arriba hacia abajo y en cuanto encuentra una condición verdadera ejecuta ese bloque y se olvida del resto. Cambiar la temperatura en vivo para mostrar los distintos casos.

---

### Diapositiva 5 — Operadores de comparación
Repasar la tabla rápido, la mayoría ya los conoce de matemática. El que hay que clavar fuerte es `==` vs `=`. Decirles: "uno asigna, el otro pregunta". Si querés podés mostrar en el editor qué pasa si usan `=` dentro de un `if` para que vean el error y no lo olviden.

---

### Diapositiva 6 — Ejemplos con comparadores
Diapo práctica, correr los tres ejemplos en vivo y cambiar los valores para mostrar distintos resultados. Antes de ejecutar cada uno, preguntar a la clase qué creen que va a imprimir. Así se enganchan. El del `!=` es buen momento para recordar que significa "distinto de".

---

### Diapositiva 7 — and / or
Antes del código, explicarlo con palabras. "Para votar necesitás tener 18 **y** tener DNI: si falta una sola cosa, no podés" → `and`. "Si es finde **o** es feriado, no hay clase: con una sola alcanza" → `or`. Después mostrar el código. La idea es que lo entiendan en castellano antes de verlo en Python.

---

### Diapositiva 8 — Ejemplos con and / or
Correr los tres ejemplos. El tercero, el de la videollamada con `not`, es el más interesante: aprovechar para explicar que `not` invierte el booleano. Si `modo_avion` es `False`, `not modo_avion` es `True`. Que lo digan con palabras: "si hay batería, hay wifi y NO está en modo avión, puede llamar". Muy natural.

---

### Diapositiva 9 — ¿Para qué sirven los bucles?
Este es el gancho de la clase. Mostrar los 5 `print` y preguntar cómo harían para imprimir del 1 al 100. Dejar que alguno proponga escribir 100 `print`. Asentir con cara seria y decir "sí, podría funcionar". Después decirles que con un bucle son 2 líneas. No mostrar el código todavía, eso va en la siguiente diapo. Esta es solo para despertarles la necesidad.

---

### Diapositiva 10 — while
Mostrar la estructura: condición al inicio, bloque adentro, y algo que haga que en algún momento la condición deje de cumplirse (el `numero += 1`). Correr el ejemplo. Después, intencionalmente, sacar el `numero += 1` y mostrar el bucle infinito corriendo. Frenarlo con `CTRL + C` y explicar qué pasó. Que lo vean en vivo para que no se asusten si les pasa en casa.

---

### Diapositiva 11 — while con input
Este ejemplo es más real y más divertido. Correrlo en vivo: responder `S` varias veces y después `N`. Que vean cómo el programa espera al usuario en cada vuelta. Buen momento para que ellos lo prueben solos y después preguntar: "¿qué pasaría si pongo una letra que no es S ni N?". Dejar que piensen.

---

### Diapositiva 12 — for y range()
Explicar que el `for` es ideal cuando sabemos de antemano cuántas veces queremos repetir. Mostrar `range(5)` y remarcar que empieza en 0 y que el 5 no se incluye ("llega hasta uno menos que el número que le ponés"). Después mostrar el `range(inicio, fin, paso)` con el ejemplo de los pares. Que prueben cambiar los valores en sus computadoras.

---

### Diapositiva 13 — Ejemplos con for
Arrancar con el ejemplo de los 100 números: "¿se acuerdan que antes hablamos de escribir 100 print? Acá va la solución". Correrlo. El efecto es bueno. Después mostrar la tabla del 7 y el recorrido de la palabra. El de `'Python'` es bueno para mostrar que el `for` no solo funciona con números sino con cualquier cosa que se pueda recorrer.

---

### Diapositiva 14 — Ejercicios
Darles los cuatro ejercicios y que arranquen. El orden de dificultad es el correcto: el primero solo `if/elif/else`, el segundo agrega `and`, el tercero mezcla `while` con condicionales adentro, el cuarto es `for` puro. Ir pasando por los lugares. Si alguno termina el 4 rápido, desafiarlos a modificar el ejercicio 3 para que cuente cuántos intentos tardó en adivinar.
