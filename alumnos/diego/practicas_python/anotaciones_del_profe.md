## Anotaciones de las prácticas

![perritoceja](../../profefranco/tests_del_profe/perritoceja.jpg)

### Práctica 5

En el punto 1: Hay un error de índices. Si te fijas, la consigna pide que imprimas el tercer material dentro de la lista. Sabemos que en Python las listas empiezan desde el índice 0, asi que si haces:

→ `print(materiales_robot[3])`

Estas imprimiendo el cuarto material de la lista. Para arreglarlo, deberias hacer:

→ `print(materiales_robot[2])`

----------------------------------------------------

Punto 2: **El registro de un cuadrado**: Empecemos con una lista vacía `acciones = []`. Usando `.append()` agregá todas las acciones que creas necesarias para que un robot realice un cuadrado de 30cm en el piso. Al terminar imprimí la lista.
Después de todo eso, usá `.pop()` para eliminar las últimas 2 acciones y luego imprimí la lista de nuevo.


En el punto 2 hiciste esto:

→ ultimas_acciones = acciones.pop(3)
→ ultimas_acciones = acciones.pop(2)

Quizá sin tener que guardar las acciones eliminadas en una variable hubiese sido más fácil. Algo así:

→ acciones.pop()
→ acciones.pop()

Igualmente está bien y funciona.

----------------------------------------------------

Punto 4: **Registro de fallos**: Tenés una lista de `alertas = ['Batería baja', 'Sensor desconectado', 'Programación con errores', 'Bluetooth no conectado', 'USB no reconocido']`. Hacé un programa que pregunte al usuario un fallo y use `in` para verificar si está en la lista de alertas.

Este ejercicio está incompleto, ya que solo pedis por input al usuario que ingrese el error pero al parecer, nunca usas `in` para verificar si está en la lista de alertas.

→ que_falla = input("ingrese un fallo :")
→ que_falla = str(que_falla)

Deberías hacer algo como así:

alertas = ["Batería baja", "Sensor desconectado", "Programación con errores", "Bluetooth no conectado", "USB no reconocido"]

fallo_usuario = input("Ingrese un fallo: ")

if fallo_usuario in alertas:
    print("El fallo está en la lista de alertas.")
else:
    print("El fallo NO está en la lista de alertas.")

----------------------------------------------------

### Práctica 6

Punto 2: **La lista de Scaloni**: Tenés que crear una tupla y una lista imaginando los convocados al mundial.
   - En la **tupla** tienen que estar los fijos que son los del cuerpo técnico (Scaloni, Aimar, etc.).
   - En la **lista**, el usuario debe ingresar con `input()` los 11 jugadores del equipo titular (imaginando un 4-4-2 o la formación que prefieras).
   Al final, imprimí en la terminal el cuerpo técnico y los jugadores. Bien bonito.

Tu código dice esto: 

```python
for jugador in range(0, 12):
    jugador = input("Ingresá un jugador por posicion: ")
    convocados.append(jugador)
```

El problema viene en que estas haciendo un rango que realiza 12 iteraciones (repite 12 veces el bucle), cuando solo debería realizar 11.

```python
for jugador in range(0, 12):
```

Debería ser:

```python
for jugador in range(0, 11):
```

Por lo demás, está bueno que le pidas al usuario por posición el jugador que va a convocar. Bien futbolero.

----------------------------------------------------

Punto 5: **Tienda LEGO**: Tenemos este inventario de stock guardado en un diccionario simple:
   ```python
   inventario = {
       'spike_hub': 5,
       'motor_grande': 12,
       'sensor_color': 8
   }
   ```
   Agregale un par de items más al inventario (por ejemplo, `'sensor_distancia': 10` y `'cable_usb': 15`) y luego, pedile al usuario con `input()` el nombre del componente que desea buscar. Usá `.get()` para buscarlo en el diccionario. Si lo encuentra, imprimí la cantidad disponible en stock. Si no lo encuentra, imprimí un mensaje amigable que le notifique que no se encuentra registrado ese item en el stock. 

En este punto vos estas haciendo: 

```python
print(el_robot.get(que_comprar))
```

Acá el problema es que estás llamando al diccionario 'el_robot' cuando en realidad deberías estar buscando en 'inventario'.

Luego, en el 'if', estás comparando 'que_comprar' con varias cosas diferentes. Pero ojo, esta condición **siempre** será verdadera.

```python
if que_comprar != "bateria" or "cables" or "sensor_color" or "motor_grande " or "spike_hub":
```

Es como que estás diciendo: "si no es 'bateria' O si no es 'cables' O si no es 'sensor_color' O si no es 'motor_grande' O si no es 'spike_hub'". Siempre se va a cumplir la condición porque preguntas si es diferente.

Lo correcto sería algo como esto:

```python
if que_comprar not in inventario:
    print("Lo sentimos no tenemos " + que_comprar + ",pero puede venir cuando lo tengamos disponible")
else:
    print("Tenemos " + que_comprar + " con stock de " + str(inventario[que_comprar]) + " unidades")
```

----------------------------------------------------

###Practica 7 y 8 

Vos tenes una función:

```python
def separador():
    print("vs")
```

La función está bien definida, pero el problema está cuando la invocas:

```python
print("primer partido de octavos :")
print(clacificadosA[0], separador() ,clacificadosA[1])
```

Tu función separador() ejecuta un print "vs" pero no retorna o devuelve nada. Así que si la metes dentro de ese print (clacificadosA[0], separador() ,clacificadosA[1]), la función SI SE EJECUTA imprimiendo el "vs" POR SEPARADO, pero no se une a los clasificados que querés imprimir.

Algo como esto estaría bien:

```python
print(clacificadosA[0] + " vs " + clacificadosA[1])
```

Pero lo que te mostraría con lo que vos hiciste sería algo así:

```text
Catar None Ecuador
```

El None debido a que la función no retorna nada.

----------------------------------------------------

Por otro lado, en la parte de los cuartos de final:

```python
cuartos.append(indice1)
```

Vos estás guardando acá el índice entero (0 o 1) del ganador, el lugar de acceder a la lista correspondiente para guardar el nombre de la seleccion. Deberías tener algo asi:

```python
clacificadosA[indice1]
```

De acá en más, genera fallos en el programa a partir de los cuartos de final.

----------------------------------------------------

Llegando a las semifinales tenés algunos problemas más:

```python
semifinales.append(indice1)
semifinales.append(indice2)
print(final)
print("LA GRAN FINAL DE MUNDO :")
print(final[0], separador(), final[1])
```

Estás añadiendo los ganadores de cada semifinal a la misma lista 'semifinales' en lugar de guardarlos en la lista final. O sea, 'final' siempre quedará vacía.

Esto te provoca un IndexError al intentar acceder a 'final[0]' y 'final[1]'.

----------------------------------------------------

### Práctica 10

Piedra, Papel y Tijera:

En este ejercicio te faltó programar una lógica de condiciones que te determine si el usuario ganó, perdió o empató. La entrada del usuario, la elección de la PC e incluso tu super barra de cargando está buenísima, pero faltó eso al final.

----------------------------------------------------

Estas son algunas correcciones que te puedo ir haciendo de todos los trabajos que subiste. Quizá me pude haber salteado alguna parte, pero no pasa nada porque lo importante es aprender a programar, y si algo no funciona lo importante es intentar entender el porqué no funciona y nunca quedarse con la duda.

Cometiendo errores es la forma normaaaal de aprender. Si te equivocas o algo no funciona tenete paciencia. Acá no te ponemos ninguna nota, pero el hecho de que hagas las prácticas es una gran señal de que estás teniendo ganas de aprender y eso es lo que más me alegra.

Seguí así Diego que vas bien.

    



