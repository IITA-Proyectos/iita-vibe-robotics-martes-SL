# Plan de Clase: Interacción y Operaciones (20 minutos)

## Objetivo de la clase
Dado que la clase dura solo 20 minutos, entrar directamente a **control de flujo (if/else, for, while)** puede ser muy apresurado. Antes de hacer condiciones, los alumnos necesitan saber cómo interactuar con el usuario.

**Propuesta:** Hacer una "clase puente" que refuerce variables y expanda hacia **Entrada/Salida (input/print)**, **conversión de tipos** y **f-strings**. Con esto, la semana que viene estarán listos para usar condicionales con datos reales ingresados por ellos.

---

## Estructura de la Clase (20 min)

### 1. Repaso Activo (5 minutos)
En lugar de hablar vos, hace que ellos te digan:
- Escribí en la pantalla: `edad = "15"` y `vidas = 3`.
- Preguntales: *"¿De qué tipo de dato es `edad` y de cuál es `vidas`?"*
- Mostrales rápido cómo comprobarlo con `print(type(edad))` y `print(type(vidas))`.
- Recordatorio rápido de **nombres válidos** (snake_case, no empezar con números).

### 2. Tema Nuevo: Hablando con la computadora (10 minutos)
**A. Pedir datos por teclado (`input`)**
- Función `input()`: Mostrarles cómo pedir el nombre.
- **Importante:** Destacar que `input()` SIEMPRE devuelve un string (`str`).

```python
nombre = input("¿Cómo te llamás?: ")
print("Hola", nombre)
```

**B. Conversión explícita (Casteo)**
- Si queremos sumar años o hacer operaciones, hay que convertir el texto a número (`int` o `float`).
```python
edad_str = input("¿Cuántos años tenés?: ")
edad_num = int(edad_str)
```

**C. Strings Facheros: f-strings**
- Mostrarles la magia de las f-strings (Python 3.6+) para que no tengan que usar tantas comas y concatenar signos más (`+`).
```python
print(f"Hola {nombre}, el año que viene vas a tener {edad_num + 1} años.")
```

### 3. Mini Desafío en vivo (5 minutos)
Pedirles que escriban un programa de 4 líneas que:
1. Pida el nombre de su robot o mascota.
2. Pida el año de nacimiento (y lo pase a entero).
3. Calcule la edad actual (`2024 - año_nacimiento` o `2026`).
4. Muestre un mensaje con f-strings.

*Ejemplo de resolución esperada:*
```python
robot = input("Nombre del robot: ")
año = int(input("¿Año de fabricación?: "))
edad = 2024 - año
print(f"El robot {robot} tiene {edad} años de antigüedad.")
```

---
## ¿Por qué esto y no control de flujo?
Con `input` y conversiones (`int()`, `float()`), la próxima clase podés enseñar `if/else` usando ejemplos dinámicos:
> *"Si la edad ingresada es > 18, sos mayor."*
Sin `input`, el control de flujo es más estático y aburrido porque las variables las definís a mano por código.
