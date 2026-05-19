# Estructuras de Datos: Listas — Clase 5

---

## 🖥️ Diapositiva 1 — El problema de las múltiples variables

Imaginá que estás armando un robot y necesitás guardar las lecturas de los últimos 5 sensores de distancia. 
Podríamos hacer esto:

```python
sensor1 = 15
sensor2 = 20
sensor3 = 18
sensor4 = 10
sensor5 = 12
```

¿Qué pasa si el robot tiene **100 sensores** o hace 100 lecturas por segundo? ¡No podemos crear 100 variables a mano!
Necesitamos una "caja" que pueda guardar muchos valores adentro.

---

## 🖥️ Diapositiva 2 — ¿Qué es una Lista?

Una **Lista** es exactamente esa "caja". Nos permite almacenar muchos datos ordenados usando una sola variable.
En Python, las listas se crean usando corchetes `[]` y separando los elementos con comas.

```python
# Una lista de números (ej. lecturas de sensores)
lecturas = [15, 20, 18, 10, 12]

# Una lista de textos (ej. nombres de motores)
motores = ['motor_izquierdo', 'motor_derecho', 'motor_garra']

# ¡Incluso listas vacías para ir llenándolas después!
datos_robot = []
```

---

## 🖥️ Diapositiva 3 — Índices (Contando desde cero)

Todos los elementos de una lista están ordenados y tienen un número de posición llamado **Índice**.
¡La regla de oro de la programación es que **siempre empezamos a contar desde el 0**!

```python
        #   0         1           2
piezas = ['Rueda', 'Batería', 'Cables']

print(piezas[0])  # Imprime: Rueda
print(piezas[2])  # Imprime: Cables
```

También podemos usar el índice para modificar un elemento existente:
```python
piezas[1] = 'Motor'
print(piezas)     # Imprime: ['Rueda', 'Motor', 'Cables']
```

---

## 🖥️ Diapositiva 4 — Agregando elementos: .append() y .insert()

Rara vez nuestras listas se quedan iguales. Los robots constantemente leen datos nuevos.

**`.append()`** → Agrega un elemento siempre al **final** de la lista. (¡Súper usado!)
```python
sensores = []
sensores.append('Ultrasonido')
sensores.append('Infrarrojo')
print(sensores)  # ['Ultrasonido', 'Infrarrojo']
```

**`.insert()`** → Agrega un elemento en una **posición (índice) específica**, moviendo los demás.
```python
nombres = ['Ana', 'Juan']
nombres.insert(1, 'Pedro') 
print(nombres)  # ['Ana', 'Pedro', 'Juan']
```

---

## 🖥️ Diapositiva 5 — Eliminando elementos: .pop() y .remove()

Así como agregamos, también podemos quitar elementos de nuestra lista.

**`.pop()`** → Elimina y te devuelve el elemento de la posición que le digas (por defecto el último).
```python
tareas = ['Avanzar', 'Girar', 'Frenar']
ultima_tarea = tareas.pop()
print(ultima_tarea) # 'Frenar'
print(tareas)       # ['Avanzar', 'Girar']
```

```python
tareas = ['Avanzar', 'Girar', 'Frenar']
primer_tarea = tareas.pop(0)
print(primer_tarea) # 'Avanzar'
print(tareas)       # ['Girar', 'Frenar']
```

**`.remove()`** → Busca un valor específico y elimina su **primera aparición**.
```python
errores = ['Sensor Desconectado', 'Batería Baja', 'Sensor Desconectado']
errores.remove('Sensor Desconectado')
print(errores) # ['Batería Baja', 'Sensor Desconectado']
```

---

## 🖥️ Diapositiva 6 — Otras herramientas útiles: len() e in

**`len()`** → Nos dice cuántos elementos tiene una lista en total ("length").
```python
velocidades = [10, 20, 30, 40]
cantidad = len(velocidades)
print('Tengo', cantidad, 'lecturas registradas.') # Tengo 4 lecturas registradas.
```

**`in`** → Nos permite preguntar si un elemento existe adentro de la lista (devuelve True o False).
```python
piezas_disponibles = ['Tornillo', 'Rueda', 'Sensor']
if 'Batería' in piezas_disponibles:
    print('Tenemos batería para armar el robot.')
else:
    print('¡Falta batería!')
```

---

## 🖥️ Diapositiva 7 — Listas y el ciclo 'for': La pareja perfecta

Las listas y el ciclo `for` están hechos el uno para el otro. El `for` recorre la lista completa, sacando un elemento a la vez.

```python
# Muestra cada tarea del robot, una por una
rutina = ['Avanzar 10cm', 'Girar 90 grados', 'Encender LED']

for paso in rutina:
    print('Ejecutando:', paso)
```

También sirve para operaciones matemáticas, como sumar todos los valores de una lista:
```python
voltajes = [5, 4, 3, 5]
suma_total = 0

for v in voltajes:
    suma_total += v

print('El voltaje total es:', suma_total) # 17
```

---

## 🖥️ Diapositiva 8 — Ejercicios Prácticos

1. **Tu primer inventario:** Creá una lista con 5 materiales que necesitas para armar tu robot. Mostrá por pantalla el **tercer** material de la lista (acordate de los índices).
2. **El registro de la misión:** Empezá con una lista vacía `acciones = []`. Usando `.append()`, agregá 3 acciones ("Arrancar", "Acelerar", "Frenar") y luego imprimí la lista entera. Después, usá `.pop()` para eliminar la última acción e imprimí la lista de nuevo.
3. **Pase de lista:** Creá una lista con los nombres de 4 compañeros. Recorré la lista con un ciclo `for` e imprimí un saludo personalizado para cada uno. (Ej: "Hola, Diego!").
4. **Alerta de fallos:** Tenés una lista `alertas = ['Luz', 'Motor', 'Sensor', 'Batería baja']`. Hacé un programa que pregunte al usuario un fallo y use `in` para verificar si está en la lista de alertas.

---
---

# 📋 Machete del Profe

---

### Diapositiva 1 — El problema de las múltiples variables
Arrancá la clase planteando un escenario robótico ridículo: "Imaginen que su robot hace 100 lecturas por segundo. ¿Vamos a escribir `lectura1`, `lectura2`... hasta 100?". Dejalos que se rían o propongan soluciones ineficientes. Mostrar el código de esta diapo para evidenciar que necesitamos una herramienta más inteligente. Este es el anzuelo para engancharlos.

---

### Diapositiva 2 — ¿Qué es una Lista?
Presentá las listas como una "caja contenedora". Es importante hacerles notar los **corchetes `[]`** y que adentro los valores se separan por comas. Mostrá que pueden guardar números, textos, o empezar vacías. Lo de la "lista vacía" es un concepto raro al principio, explicales que es como "comprar una mochila nueva sin haberle puesto los cuadernos todavía".

---

### Diapositiva 3 — Índices
**Punto Crítico.** Acá muchos se pierden. Repetí varias veces que en programación empezamos a contar desde el 0. Dibujá en el pizarrón la lista y los números arriba: 0, 1, 2. Corré el ejemplo. Desafialos en vivo preguntando: "¿Qué pasa si pongo `piezas[3]`?". Que vean el error `IndexError: list index out of range` en vivo para que pierdan el miedo y sepan qué significa cuando les salte a ellos.

---

### Diapositiva 4 — Agregando elementos
El `.append()` va a ser su mejor amigo de acá a que termine el curso, es crucial para la robótica (ej: ir metiendo en la lista cada distancia que el sensor ultrasónico detecta en un ciclo while). Aclarales que `append` solo empuja cosas al **final** del vagón. El `.insert()` es menos común, explicalo rápido como "el que se cuela en la fila".

---

### Diapositiva 5 — Eliminando elementos
Mostrarles la diferencia entre `.pop()` (que trabaja con **posiciones/índices**) y `.remove()` (que trabaja con **valores exactos**). Hacé la analogía: `pop()` es sacar la última carta de un mazo, `remove()` es buscar en el mazo todas las cartas y sacar el primer "Ancho de Espadas" que veas. Aclarales que si usan `.remove()` y el elemento no existe, Python va a tirar error.

---

### Diapositiva 6 — Otras herramientas útiles (len / in)
Dos herramientas de bolsillo súper útiles. `len()` (de *length* / longitud) la van a usar mucho para saber si una lista está vacía (`len(lista) == 0`). Y el operador `in` es mágico: les evita hacer un ciclo `for` a mano solo para buscar si algo existe o no. Mostrar el ejemplo del `if 'Batería' in...`.

---

### Diapositiva 7 — Listas y el ciclo 'for'
La cereza del postre. Si la clase de control de flujo quedó clara, esto les va a resultar natural. Mostrales que con las listas el `for` se vuelve "leíble como el inglés": `for paso in rutina` ("Para cada paso en la rutina"). Recorré el segundo ejemplo paso por paso explicando cómo el acumulador `suma_total` se va incrementando vuelta a vuelta.

---

### Diapositiva 8 — Ejercicios Prácticos
Los primeros 2 ejercicios son de fijación rápida (apuntar, agregar, sacar). El ejercicio 3 combina el tema de hoy con el ciclo `for`. El ejercicio 4 mezcla listas con el condicional `if`. Dejalos programar y pasá por los bancos, seguramente van a haber errores de "Index out of range" o van a olvidar poner las listas vacías antes del `.append()`. Si resuelven estos cuatro sin problemas, están listísimos para aplicarlo en los robots.
