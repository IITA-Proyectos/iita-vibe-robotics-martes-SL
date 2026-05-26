# Estructuras de Datos: Tuplas y Diccionarios — Clase 6

---

## 🖥️ Diapositiva 1 — Listas superpoderosas pero frágiles

La clase pasada vimos que las **Listas** están buenísimas porque podemos guardar un montón de valores en una sola variable. ¡Como una caja muy grande!

Pero, ¿qué pasa si en esa caja guardamos **datos delicadísimos** que no se pueden modificar por nada del mundo?
Por ejemplo: Las distancias exactas en milímetros donde tiene que frenar tu robot.

Si usás una lista normal:
```python
distancias_frenado = [150, 200]

# Alguien (o vos mismo por error) hace un pop y...
distancias_frenado.pop() 

# ¡El robot no sabe a qué distancia frenar y se choca!
```

Necesitamos una "caja" que venga **cerrada con candado**.

---

## 🖥️ Diapositiva 2 — Las Tuplas al rescate

Las **Tuplas** son exactamente eso: listas inmutables (no se pueden modificar).
Se crean usando paréntesis `()` en lugar de corchetes `[]`.

```python
distancias_frenado = (150, 200)
colores_rgb = (255, 0, 0)
```

Podés leer los datos igual que en una lista, usando los índices (¡acordate de contar desde el 0!):

```python
print("Primera distancia:", distancias_frenado[0])
```

Pero si intentás modificarlas o usar `.append()` / `.pop()`, Python te va a tirar un error y va a frenar el programa para proteger tus datos.

---

## 🖥️ Diapositiva 3 — El problema de los índices

Tanto en las listas como en las tuplas, para buscar un dato tenés que acordarte su **número de posición** (el índice).

Imaginate tener la ficha técnica completa de tu robot:
```python
robot_info = ["Wall-E", 100, "En reposo", 50, 2026]
```
Si querés saber cuánta batería tiene, tenés que pensar: *"A ver, el nombre era el 0, la batería era el 1..."* -> `print(robot_info[1])`.

¡Es un dolor de cabeza! ¿No sería más fácil buscar la palabra "bateria" y que te dé el número?

---

## 🖥️ Diapositiva 4 — Los Diccionarios

¡Para eso existen los **Diccionarios**! Funcionan igual que un diccionario de la vida real:
Buscás la **Clave** (Palabra) y encontrás el **Valor** (Significado).

Se crean usando llaves `{}`.

```python
robot_info = {
    'nombre': 'Wall-E',
    'bateria': 100,
    'estado': 'En reposo'
}
```

¡Ahora es muchísimo más fácil leer los datos!
```python
print("Nivel de batería:", robot_info['bateria'])
```

---

## 🖥️ Diapositiva 5 — ¡Me tiró error! El truquito de .get()

¿Qué pasa si en el diccionario le preguntás por una clave que no existe? 

```python
print(robot_info['velocidad']) # ¡ERROR! KeyError
```

Si hacés eso, el programa se tranca por completo. Para evitar que el robot se apague por un error así, usamos la función **`.get()`**. 

```python
# Si la clave no existe, no tira error, devuelve 'None' (nada)
print(robot_info.get('velocidad')) 

# O mejor aún, ¡le decimos qué responder si no lo encuentra!
print(robot_info.get('velocidad', 'Dato no encontrado, capo')) 
```

---

## 🖥️ Diapositiva 6 — Agregando y modificando Diccionarios

¡Los diccionarios sí se pueden modificar! Es tan fácil como decir "la clave tal, ahora vale esto".

```python
# Cambiamos un valor que ya existía (ej: se gastó la batería)
robot_info['bateria'] = 85

# Agregamos una pareja nueva que no existía
robot_info['velocidad'] = 50

print(robot_info)
```

Ya sea con Listas, Tuplas o Diccionarios, ¡tenemos todas las herramientas para manejar la memoria de nuestros robots!

---

## 🖥️ Diapositiva 7 — Ejercicios Prácticos

1. **Calibrando el Sensor de Luz:** Creá una **tupla** llamada `calibracion_luz` con dos valores de intensidad de luz (el mínimo de la habitación apagada y el máximo con la linterna prendida, por ejemplo: `(20, 95)`). Intentá cambiar el primer valor usando `calibracion_luz[0] = 30`. ¡Fijate cómo Python te frena al toque para proteger tu calibración!
2. **La lista de Scaloni:**
   - Creá una **tupla** llamada `staff_tecnico` con los nombres fijos del cuerpo técnico: `'Scaloni'`, `'Aimar'`, `'Samuel'`.
   - Creá una **lista** vacía llamada `convocados`.
   - Pedile al usuario por `input()` que ingrese los 11 jugadores del equipo titular (esquema 4-4-2):
     - Primero, 1 arquero.
     - Después, 4 defensores (podés usar un ciclo `for` con `range(4)` para pedir cada uno).
     - Después, 4 mediocampistas (ciclo `for` con `range(4)`).
     - Por último, 2 delanteros (ciclo `for` con `range(2)`).
     - Agregá cada jugador a la lista `convocados` usando `.append()`.
   - Al final, imprimí en la terminal el cuerpo técnico (la tupla) y los jugadores titulares (la lista) bien presentados en la pantalla.
3. **Mi primer diccionario:** Creá un diccionario llamado `mi_robot` con tres claves: `'nombre'`, `'color'` y `'ruedas'`. Poneles los valores que quieras. Después, imprimí solo el nombre del robot usando la clave.
4. **Mantenimiento en boxes:** Usando el diccionario del punto 3, agregale una clave nueva que sea `'bateria'` con el valor `100`. Después, modificalo para que la batería baje a `80`. Imprimí el diccionario completo para ver los cambios.
5. **Tienda LEGO Technic:** Tenés este diccionario que representa el inventario de repuestos de tu kit de robótica:
   ```python
   inventario = {
       'spike_hub': {'precio': 200, 'descripcion': 'Controlador LEGO Spike', 'stock': 5},
       'motor_grande': {'precio': 45, 'descripcion': 'Motor de alto torque', 'stock': 12},
       'sensor_color': {'precio': 30, 'descripcion': 'Detector de colores y luz', 'stock': 8}
   }
   ```
   Pedile al usuario por `input()` que ingrese el nombre del componente que busca. Usá la función `.get()` para buscarlo en el diccionario. Si lo encuentra, imprimí su descripción, precio y stock. Si no está registrado en el inventario, mostrá un mensaje amigable como `"Esa pieza no está registrada en el stock, che"`.

---
---

# 📋 Machete del Profe

---

### Diapositiva 1 — Listas superpoderosas pero frágiles
Arrancá la clase felicitándolos por la Práctica 5. Comentales que Juanse, Máximo y Diego vienen súper bien (a Laureano pegale una ayudita en privado). Usá este ejemplo catastrófico: "Imaginen que el robot está por llegar al borde de la mesa y la distancia límite estaba en una lista... y ¡pum! alguien le hizo un pop por accidente. ¡Robot al suelo!". Es la excusa perfecta para presentar las tuplas.

---

### Diapositiva 2 — Las Tuplas al rescate
Remarcá visualmente los paréntesis `()`. Hacé hincapié en la palabra **inmutable**. Preguntales: "¿Para qué sirve algo que no se puede cambiar?". Que se den cuenta que en la programación a veces queremos "proteger" nuestros datos de nuestros propios errores. Es como ponerle "solo lectura" a un archivo.

---

### Diapositiva 3 — El problema de los índices
Acá volvé a plantearles una situación incómoda. Muestrales una lista larguísima de datos (sin nombres) y deciles: "A ver, díganme cuál de esos números es la velocidad y cuál es la temperatura del motor". Que entiendan que buscar por números (0, 1, 2...) es un bajón cuando la información es de distinto tipo.

---

### Diapositiva 4 — Los Diccionarios
Explicá la estructura `Clave: Valor` haciendo el paralelismo directo con un diccionario de inglés-español. Aclarales bien el tema de las llaves `{}`. Dales tiempo para asimilar la sintaxis (comillas para la clave, dos puntos, y luego el valor).

---

### Diapositiva 5 — El truquito de .get()
Este es un "Pro Tip". Mostrales primero el error (KeyError) para que vean cómo explota el programa de forma fea en la consola. Después mostrales cómo `.get()` es la forma elegante y profesional de pedirle datos al diccionario sin que todo explote. A los chicos les encanta sentir que usan comandos "pro".

---

### Diapositiva 6 — Agregando y modificando
Acá mostrales que a diferencia de `.append()` en las listas, en los diccionarios es súper directo: `diccionario['clave_nueva'] = valor`. Si la clave existe, la pisa. Si no existe, la crea. Es magia pura y súper fácil de entender para ellos.

---

### Diapositiva 7 — Ejercicios Prácticos
Son ejercicios desafiantes pero bien guiados, ideales para que asimilen los conceptos y les quede tiempo de programar los robots reales.
- **Ejercicio 1:** Sirve para que choquen de frente contra el error de inmutabilidad de las tuplas. Que vean que Python frena el programa para defender los datos de calibración del sensor.
- **Ejercicio 2 (La lista de Scaloni):** Es la frutilla del postre. Combina la tupla fija con una lista que va creciendo con `input()`. Sugeriles usar ciclos `for` con `range()` para pedir los defensores, mediocampistas y delanteros, así no escriben código repetitivo. Ayudá a Laureano si se tranca con la indentación dentro del ciclo.
- **Ejercicio 3 y 4:** Son los ejercicios clave para que se familiaricen con la sintaxis de claves y valores `{}` de los diccionarios y cómo modificar datos existentes o agregar nuevos (Mantenimiento en boxes).
- **Ejercicio 5:** Aplica diccionarios anidados y el método `.get()`. Es súper útil para que vean cómo organizar un inventario real de LEGO. Mostrales cómo el `.get()` evita que el programa explote si buscan un componente inexistente (como un motor de NXT que no usan).
Pasate por el banco de Laureano para asegurarte que arranque bien, a los demás dejalos volar un poco que ya le agarraron la mano.
