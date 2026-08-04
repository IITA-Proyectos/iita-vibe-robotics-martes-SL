# 🤖 Guía de Arduino - Curso de Verano "Arte y Robótica" 2023
*Por el Profe Franco - IITA (Instituto de Innovación y Tecnología Aplicada)*

¡Bienvenidos a la guía interactiva de programación con Arduino! Esta guía fue diseñada para acompañarte paso a paso en el aprendizaje de la electrónica y la programación utilizando **Tinkercad** como simulador. 

A continuación, encontrarás cada una de las prácticas explicadas, sus componentes, esquemas de conexión, códigos de programación y explicaciones detalladas.

---

## 📋 Índice de Prácticas
1. [Práctica 1: Encender un LED](#-práctica-1-encender-un-led)
2. [Práctica 2: LED que Parpadea (Blink)](#-práctica-2-led-que-parpadea-blink)
3. [Práctica 3: Semáforo](#-práctica-3-semáforo)
4. [Concepto Clave: Variables](#-concepto-clave-variables)
5. [Herramienta: Monitor Serial](#-herramienta-monitor-serial)
6. [Práctica 4: Servomotor SG90](#-práctica-4-servomotor-sg90)
7. [Práctica 5: Encender un LED con un Pulsador](#-práctica-5-encender-un-led-con-un-pulsador)
8. [Práctica 6: Sensor de Luz (LDR) - Lectura Básica](#-práctica-6-sensor-de-luz-ldr---lectura-básica)
9. [Práctica 7: Sensor de Luz (LDR) - Control de LED](#-práctica-7-sensor-de-luz-ldr---control-de-led)
10. [Práctica 8: Sensor de Ultrasonido (HC-SR04)](#-práctica-8-sensor-de-ultrasonido-hc-sr04)

---

## 💡 Práctica 1: Encender un LED

El primer ejercicio es sencillo para comenzar a entender cómo funcionan las salidas digitales. Vamos a armar el circuito y programarlo para que una salida conectada a un LED se encienda.

### 🔌 Componentes Necesarios
| Cantidad | Componente | Descripción |
| :---: | :--- | :--- |
| 1 | Arduino UNO | Placa de desarrollo principal |
| 1 | Placa de pruebas | Breadboard para prototipado rápido |
| 1 | LED | Cualquier color |
| 1 | Resistencia | $1\text{ k}\Omega$ (Marrón, Negro, Rojo, Dorado) |

### 🗺️ Esquema de Conexión (Tinkercad)
![Esquema de Conexión - LED](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-01.png)

### 👣 Pasos para la Conexión
1. El **pin 13** del Arduino lo conectaremos con uno de los terminales de la resistencia.
2. El **otro terminal** de la resistencia lo conectaremos al **ánodo** (positivo / pata larga) del LED.
3. Finalmente, el **cátodo** (negativo / pata corta) del LED lo conectaremos a **GND** (tierra) del Arduino.

### 💻 Programación
> 📝 **Mensaje del profe:** *Escribí, no copies y pegues 🐱*

```cpp
void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
}
```

### 📖 Explicación del Código
* **`void setup()`**: Definimos nuestra configuración de pines y lo que queremos que haga nuestro Arduino una única vez al encenderse.
  * Nosotros solo tenemos la línea `pinMode(13, OUTPUT);` con la que programamos nuestro pin 13 (donde conectamos el LED) como **salida**.
* **`void loop()`**: Definimos lo que deseamos que haga nuestro Arduino infinitamente (o al menos mientras tenga energía...).
  * Nosotros programamos con la línea `digitalWrite(13, HIGH);` para que a nuestro LED conectado al pin 13 se le escriba el estado **alto (HIGH)**, para que se encienda y se mantenga así.

> 💡 **Pregunta para pensar:**
> ¿Y si queremos apagar nuestro LED? Acordate que para encender un LED usábamos la instrucción `HIGH` (estado ALTO o ENCENDIDO). Por el contrario, si queremos apagarlo, tendríamos que cambiarlo a `LOW` (estado BAJO o APAGADO).
> 
> *Intento:* Reemplazá el estado `HIGH` por `LOW` en tu código: `digitalWrite(13, LOW);`.
> 
> **¿Ya probaste el funcionamiento del circuito? ¿Cómo haríamos para poder encender más de un solo LED?**

![Original Slide 2](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-02.png)

---

## ⏳ Práctica 2: LED que Parpadea (Blink)

Si te pareció fácil el primer ejercicio, el segundo también lo será. Armaremos el mismo circuito, pero cambiaremos la programación para que nuestro LED se apague y se prenda cada cierto tiempo.

### 🔌 Componentes Necesarios
Los mismos que en la Práctica 1.

### 🗺️ Esquema de Conexión
![Esquema de Conexión - Blink](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-03.png)

### 💻 Programación
> 📝 **Mensaje del profe:** *Si te animas, añadí los delay donde correspondan y la instrucción para que se apague nuestro LED 😁*

```cpp
void setup() {
  pinMode(13, OUTPUT); // DECLARAMOS EL PIN 13 COMO SALIDA
}

void loop() {
  digitalWrite(13, HIGH); // ENCENDEMOS EL LED CONECTADO AL PIN 13
  delay(2000);            // ESPERAMOS 2000 milisegundos (2seg)
  digitalWrite(13, LOW);  // APAGAMOS EL LED CONECTADO AL PIN 13
  delay(2000);            // ESPERAMOS 2000 milisegundos (2seg)
}
```

### 📖 Explicación del Código
¿Ves cómo poco a poco vamos añadiendo más instrucciones, alargando así nuestro programa?
* Dentro del `void setup()` solo declaramos el pin 13 como salida (`pinMode(13, OUTPUT);`).
* Dentro del `void loop()` vemos que hay algunos cambios:
  * Luego de encender el LED con `digitalWrite(13, HIGH);`, en la siguiente línea vemos la instrucción `delay(2000);`. Con esta línea hacemos que nuestro programa espere **2000 milisegundos**, que es equivalente a **2 segundos**.
  * Luego de la espera, apagamos nuestro LED con la instrucción `digitalWrite(13, LOW);`.
  * En la última línea hay un segundo `delay(2000);`. Su función es esperar 2 segundos más antes de que el ciclo vuelva a iniciar y encender el LED nuevamente.

> 🧠 **¿Entendés cómo funciona el `void loop()`?**
> Se ejecuta de arriba hacia abajo y cuando llega a la última instrucción, vuelve a repetir la primera instrucción, y así infinitamente.

![Original Slide 4](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-04.png)

---

## 🚦 Práctica 3: Semáforo

Si pensamos en el funcionamiento de un semáforo, podríamos decir que son luces que se encienden y se apagan cada cierto tiempo. Con los dos ejercicios anteriores ya aprendimos a hacer exactamente eso. ¡Manos a la obra! ✍️

### 🔌 Componentes Necesarios
| Cantidad | Componente | Descripción |
| :---: | :--- | :--- |
| 1 | Arduino UNO | Placa principal |
| 1 | Placa de pruebas | Breadboard |
| 3 | LED | 1 Rojo, 1 Amarillo (o Azul/Blanco) y 1 Verde |
| 3 | Resistencia | $1\text{ k}\Omega$ para cada LED |

### 🗺️ Esquema de Conexión
![Esquema de Conexión - Semáforo](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-05.png)

### 👣 Pasos para la Conexión
1. Primero colocamos los LED con las resistencias en la placa de pruebas como se muestra en la imagen.
2. A continuación, los conectamos (el cátodo a GND a través de la resistencia).
3. Finalmente, conectamos los ánodos a los pines digitales que deseemos. En este ejemplo, usaremos los pines **1, 2 y 3**.

### 💻 Programación
> 📝 **Mensaje del profe:** *Sé que puede ser confuso al principio, pero andá acostumbrándote a entender tus programas cada vez más largos 🧑‍💻*

```cpp
void setup()
{
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop()
{
  digitalWrite(1, HIGH); // Encendemos el LED Rojo
  delay(2000);           // Esperamos 2 segundos
  digitalWrite(2, HIGH); // Encendemos el LED Amarillo
  delay(1000);           // Esperamos 1 segundo
  digitalWrite(1, LOW);  // Apagamos el LED Rojo
  digitalWrite(2, LOW);  // Apagamos el LED Amarillo
  digitalWrite(3, HIGH); // Encendemos el LED Verde
  delay(2000);           // Esperamos 2 segundos
  digitalWrite(3, LOW);  // Apagamos el LED Verde
  digitalWrite(2, HIGH); // Encendemos el LED Amarillo
  delay(1000);           // Esperamos 1 segundo
  digitalWrite(2, LOW);  // Apagamos el LED Amarillo
}
```

### 📖 Explicación del Ciclo
Así es la secuencia de nuestro semáforo:
1. **Paso 1:** Solo luz Roja encendida (2 segundos).
2. **Paso 2:** Luz Roja y luz Amarilla encendidas a la vez (1 segundo).
3. **Paso 3:** Se apagan Roja y Amarilla, y se enciende la luz Verde (2 segundos).
4. **Paso 4:** Se apaga la luz Verde y se enciende la luz Amarilla (1 segundo) antes de volver a empezar con la luz Roja en la siguiente iteración.

![Explicación Secuencia](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-07.png)

> 🏆 **Desafío - Semáforo Doble:**
> Si caminas por la calle y llegas a una esquina con semáforo, lo más seguro es que veas que no hay solo un semáforo, **HAY DOS**.
> En ese caso se llaman semáforos de intersección simple. Tu misión es la siguiente: armá un circuito en la placa de pruebas con 2 semáforos, conecta los LED al Arduino y programalos para que funcionen de forma coordinada (cuando uno esté en verde, el otro debe estar en rojo, etc.).
> 
> *¡Mucha suerte! 💪*

---

## 📦 Concepto Clave: Variables

Una **variable** es un espacio en la memoria de la placa al que le ponemos un nombre y le asignamos un valor. Sirven para hacer que nuestro código sea mucho más fácil de leer y modificar.

Imaginemos que nos cuesta recordar el pin al que conectamos nuestro LED. En lugar de escribir el número del pin cada vez, podemos crear una variable:

```cpp
int LED = 13;
```

### ¿Cómo se crea una variable?
1. **Tipo de dato:** Le decimos al Arduino qué tipo de dato guardaremos. En este caso `13` es un número **entero**, por lo que escribiremos `int`.
2. **Nombre:** Le asignamos un nombre a nuestra variable (por ejemplo, `LED`, `LED_ROJO`, etc.).
3. **Valor:** Le asignamos el valor del pin (`= 13;`).

### 💻 Ejemplo 1: Parpadeo usando Variables
![Conexión de LED con Variables](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-08.png)

```cpp
int LED = 13;

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(2000);
  digitalWrite(LED, LOW);
  delay(2000);
}
```

### 💻 Ejemplo 2: Semáforo usando Variables
¡Mirá qué ordenado y fácil de leer queda el código del semáforo si declaramos variables al inicio!

```cpp
int LED_ROJO = 1;
int LED_AMARILLO = 2;
int LED_VERDE = 3;

void setup()
{
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
}

void loop()
{
  digitalWrite(LED_ROJO, HIGH);
  delay(2000);
  digitalWrite(LED_AMARILLO, HIGH);
  delay(1000);
  
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, HIGH);
  delay(2000);
  
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, HIGH);
  delay(1000);
  digitalWrite(LED_AMARILLO, LOW);
}
```
![Original Slide 9](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-09.png)

---

## 🖥️ Herramienta: Monitor Serial

El **monitor serial** es el "cable" o canal de comunicación entre tu computadora y el Arduino. Nos permite recibir mensajes del Arduino en la pantalla (como valores de sensores) o enviarle instrucciones desde la computadora.

### ¿Dónde está el Monitor Serie?
* **En el IDE de Arduino:** Está en la esquina superior derecha, representado por el icono de una lupa (en versiones 1.x) o en la esquina superior derecha / pestaña inferior (en versiones 2.x).
* **En Tinkercad:** Se encuentra en la parte inferior del editor de código, bajo el botón que dice "Monitor en serie".

![Ubicación Monitor Serial](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-10.png)

Dispone de dos zonas principales:
1. **Enviar:** Una barra de texto arriba para mandar datos al Arduino.
2. **Recibir:** Un área grande blanca que muestra los datos recibidos.

![Zonas Monitor](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-11.png)

### 💻 Programación: Hola Mundo Serial
```cpp
void setup() {
  Serial.begin(9600); // Inicializamos la comunicación a 9600 baudios
}

void loop() {
  Serial.println("Hola mundo!");
  delay(1000);
}
```

### 📖 Explicación del Código
* **`Serial.begin(9600);`**: Inicializa la comunicación serial. El número `9600` indica la velocidad de transmisión en **baudios** (símbolos por segundo). ¡Ambos dispositivos deben estar configurados a la misma velocidad para poder entenderse!
* **`Serial.println("Hola mundo!");`**: Le indica al Arduino que envíe el texto entre comillas al monitor serial. El `println` agrega automáticamente un salto de línea al final de cada mensaje, haciendo que la lectura sea mucho más prolija que si usáramos `Serial.print`.

![Resultado Monitor](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-13.png)

---

## 🦾 Práctica 4: Servomotor SG90

Un **servomotor** es un motor especial que, a diferencia de los motores convencionales que giran libremente sin parar, permite controlar con alta precisión su posición angular (rango de **0° a 180°**), aceleración y velocidad. Se usan mucho en articulaciones de brazos robóticos y en robots caminantes (como el robot OTTO).

### 🎨 Servomotor SG90
Este pequeño servomotor cuenta con tres cables de conexión:
1. **Marrón:** GND (Tierra)
2. **Rojo:** 5V (Alimentación)
3. **Naranja:** Señal PWM (Control del movimiento)

> ⚠️ **IMPORTANTE:** Debes conectar el cable de señal a un pin digital que tenga soporte **PWM** (indicados en la placa con el símbolo `~`). Si usas un pin común, el servomotor no funcionará.

![Servomotor Detalle](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-15.png)

* **Calibración inicial:** Antes de colocar las paletas del servomotor, es buena práctica programarlo para que se ubique en la posición central de **90°**. Así nos aseguramos de que tendrá el mismo rango de movimiento tanto a la izquierda como a la derecha.
* **Librerías:** Para programar el servomotor utilizaremos la librería `#include <Servo.h>` que viene incluida en el IDE de Arduino. Las librerías son archivos de código creados por otros programadores que nos facilitan el control de componentes complejos.

---

### 🔌 Componentes Necesarios
* 1x Arduino UNO
* 1x Placa de pruebas
* 1x Servomotor SG90

### 🗺️ Esquema de Conexión
![Esquema Servomotor](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-16.png)

---

### 💻 Programación Básica (Posicionamiento Directo)
```cpp
#include <Servo.h> // Incluimos la librería del servo

Servo mi_servo_1; // Declaramos nuestro objeto servomotor

void setup() {
  mi_servo_1.attach(9); // Indicamos que el servo está conectado al pin PWM 9
}

void loop() {
  mi_servo_1.write(0);   // Mover a la posición 0 grados
  delay(2000);
  mi_servo_1.write(90);  // Mover a la posición 90 grados
  delay(2000);
  mi_servo_1.write(180); // Mover a la posición 180 grados
  delay(2000);
}
```

### 📖 Explicación del Código
* **`#include <Servo.h>`**: Importa todas las instrucciones necesarias para controlar servomotores.
* **`Servo mi_servo_1;`**: Crea una instancia u objeto de tipo `Servo` para que podamos controlarlo en el programa.
* **`mi_servo_1.attach(9);`**: Le indica a la placa que el cable naranja del servomotor está conectado en el **pin 9**.
* **`mi_servo_1.write(angulo);`**: Envía la orden al motor para que se desplace hasta el ángulo indicado (entre 0 y 180).

---

### 💻 Programación Avanzada: Movimiento Suave con Ciclo `for`
Si ejecutamos el código anterior, notarás que el servo se mueve muy rápido de un extremo a otro. En muchos proyectos es preferible que el servo se mueva poco a poco. 

Para lograr esto de forma eficiente y sin escribir cientos de líneas repetitivas, utilizamos un **bucle `for`**.

```cpp
#include <Servo.h>

Servo mi_servo_1;
int posicion; // Variable para almacenar el ángulo actual

void setup() {
  mi_servo_1.attach(9);
  mi_servo_1.write(0); // Calibramos el servo al inicio en 0°
}

void loop() {
  // Movimiento de ida: de 0 a 180 grados de 1 en 1
  for (posicion = 0; posicion <= 180; posicion = posicion + 1) {
    mi_servo_1.write(posicion);
    delay(25); // Pequeña pausa de 25ms para controlar la velocidad
  }
  
  // Movimiento de vuelta: de 179 a 0 grados restando de 1 en 1
  for (posicion = 179; posicion > 0; posicion = posicion - 1) {
    mi_servo_1.write(posicion);
    delay(25);
  }
}
```

#### ⚙️ Explicación del Bucle `for`
La estructura de un bucle `for` es la siguiente:
```cpp
for (expresión_de_inicio; condición; incremento/decremento) {
  // instrucciones a repetir
}
```
* **Inicio:** `posicion = 0`. Indica el valor inicial con el que comenzará la variable.
* **Condición:** `posicion <= 180`. El bucle seguirá ejecutándose e incrementando mientras esta condición sea verdadera.
* **Incremento:** `posicion = posicion + 1`. Le suma 1 a la variable al final de cada iteración del bucle.

![Bucle For Explicación](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-20.png)

---

## 🔘 Práctica 5: Encender un LED con un Pulsador

En esta práctica aprenderemos a usar una **entrada digital**. Utilizaremos un pulsador (botón) para que el Arduino lea su estado físico y decida cuándo encender o apagar el LED.

### 🔌 Componentes Necesarios
| Cantidad | Componente | Descripción |
| :---: | :--- | :--- |
| 1 | Arduino UNO | Placa principal |
| 1 | Placa de pruebas | Breadboard |
| 1 | Pulsador | Botón de 4 pines |
| 1 | LED | Cualquier color |
| 2 | Resistencia | $1\text{ k}\Omega$ (una para el LED y otra para el botón como pull-down) |

### 🗺️ Esquema de Conexión
![Esquema Pulsador](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-21.png)

### 💻 Programación
```cpp
int pin_led = 13;
int pin_pulsador = 8;
int valor_pulsador = 0;

void setup() {
  pinMode(pin_led, OUTPUT);      // Declaramos el LED como SALIDA
  pinMode(pin_pulsador, INPUT);  // Declaramos el pin del pulsador como ENTRADA
}

void loop() {
  // Leemos el estado del pulsador (0 o 1) y lo guardamos en la variable
  valor_pulsador = digitalRead(pin_pulsador);

  if (valor_pulsador == 1) { 
    digitalWrite(pin_led, HIGH); // Si está presionado (1), encendemos el LED
  } 
  else {
    digitalWrite(pin_led, LOW);  // Si no está presionado (0), apagamos el LED
  }
}
```
> ⚠️ **Nota de corrección:** En algunas diapositivas originales el código del loop utiliza directamente la constante no declarada `LED` en lugar de `pin_led`. Asegúrate de escribirlo como `pin_led` en tu código para evitar errores de compilación.

### 📖 Explicación del Código
* **`pinMode(pin_pulsador, INPUT);`**: Configura el pin digital 8 como **entrada**. El Arduino ahora estará atento a recibir energía desde ese pin.
* **`digitalRead(pin_pulsador);`**: Lee el estado eléctrico del pin. Si el pulsador está presionado ingresará una señal de 5V (**HIGH** o **1**). Si está libre, la resistencia lo conectará a tierra obteniendo 0V (**LOW** o **0**).
* **Condicional `if-else`:** Evalúa si se cumple la condición dentro del paréntesis:
  * Si la condición es **verdadera** (`valor_pulsador == 1`), se ejecuta el bloque de código dentro del `if`.
  * Si es **falsa** (el pulsador vale 0), se saltea el primer bloque y se ejecuta lo que está dentro del `else`.

![Explicación Condicional](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-23.png)

---

## ☀️ Práctica 6: Sensor de Luz (LDR) - Lectura Básica

Una **fotocelda o LDR** (resistencia dependiente de la luz) es un sensor que varía su resistencia interna según la cantidad de luz que recibe. Al cambiar su resistencia, cambia el voltaje del pin. En este ejercicio aprenderemos a leer valores analógicos de un sensor.

### 🔌 Componentes Necesarios
| Cantidad | Componente | Descripción |
| :---: | :--- | :--- |
| 1 | Arduino UNO | Placa principal |
| 1 | Placa de pruebas | Breadboard |
| 1 | LDR (Fotocelda) | Sensor de luz |
| 1 | Resistencia | $10\text{ k}\Omega$ (Marrón, Negro, Naranja, Dorado) |

### 🗺️ Esquema de Conexión
![Esquema LDR](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-24.png)

### 👣 Pasos para la Conexión
1. Conectar las líneas de **5V** y **GND** del Arduino a las líneas correspondientes de la placa de pruebas.
2. Como la fotocelda no tiene polaridad, conectar cualquiera de sus terminales a **5V**.
3. El otro terminal de la fotocelda conectarlo a uno de los extremos de la resistencia de $10\text{ k}\Omega$.
4. En ese mismo nodo de unión entre la fotocelda y la resistencia, colocar un cable que vaya al pin analógico **A5** del Arduino.
5. Finalmente, conectar el terminal libre de la resistencia a **GND** (Tierra).

### 💻 Programación
```cpp
int valor_fotocelda; // Variable para guardar la lectura analógica

void setup() {
  pinMode(A5, INPUT);  // Configuramos el pin analógico A5 como entrada
  Serial.begin(9600); // Iniciamos el monitor serial
}

void loop() {
  valor_fotocelda = analogRead(A5); // Leemos el voltaje en A5
  Serial.println(valor_fotocelda);  // Lo mostramos en pantalla
  delay(100);                       // Pausa para estabilizar la lectura
}
```

### 📖 Explicación del Código
* **`analogRead(A5);`**: A diferencia de los pines digitales que solo leen estados de `0` o `1`, el convertidor analógico-digital de Arduino convierte voltajes de 0V a 5V en un número entero que va desde **0 hasta 1023**.
* Al abrir el monitor serial verás números que cambian dinámicamente según la luz. 

> 💡 **Tip en Tinkercad:** 
> Durante la simulación, haz clic sobre el sensor de luz para desplegar una barra deslizable que te permitirá cambiar la intensidad de luz ambiente y ver cómo varían los valores leídos.
> 
> *¿Sabías que...? La iluminación pública de las calles incorpora fotoceldas para detectar de forma automática cuándo es de noche y encender las farolas.*

![Tip Tinkercad](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-27.png)

---

## 🌙 Práctica 7: Sensor de Luz (LDR) - Control de LED

¡Démosle una aplicación práctica a nuestro sensor de luz! Crearemos un circuito de encendido automático: si la fotocelda detecta oscuridad (lecturas de luz bajas), el LED se encenderá de forma automática.

### 🔌 Componentes Necesarios
* 1x Arduino UNO y Placa de pruebas
* 1x LED (cualquier color)
* 1x LDR (Fotocelda)
* 1x Resistencia de $1\text{ k}\Omega$ (para el LED)
* 1x Resistencia de $10\text{ k}\Omega$ (para la fotocelda)

### 🗺️ Esquema de Conexión
![Esquema LDR + LED](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-28.png)

### 💻 Programación
```cpp
int valor_fotocelda; // Variable para almacenar el valor de luz
int LED = 13;        // LED en el pin 13

void setup() {
  pinMode(LED, OUTPUT); // Pin 13 como salida
  pinMode(A5, INPUT);   // Pin analógico A5 como entrada
}

void loop() {
  valor_fotocelda = analogRead(A5); // Leemos el sensor

  // Si el valor medido es menor a 500 (umbral de oscuridad), encendemos el LED
  if (valor_fotocelda < 500) {
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(LED, LOW);
  }
}
```

### 📖 Explicación del Umbral
En este código definimos un **umbral** de `500`. 
* Si la habitación tiene suficiente luz, la lectura del sensor será alta (mayor a 500), por lo que el programa irá al bloque `else` y apagará el LED.
* Si tapamos el sensor con el dedo o simulamos oscuridad en Tinkercad, la lectura bajará de 500, ingresando al bloque `if` y encendiendo el LED.

> 🏆 **Desafío:**
> Intenta combinar las dos prácticas anteriores: haz que el Arduino encienda el LED cuando detecte oscuridad y que, a la vez, esté enviando constantemente el valor de lectura actual del sensor al monitor serie para poder ver en tiempo real en qué valor se realiza la transición.

![Original Slide 30](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-30.png)

---

## 🦇 Práctica 8: Sensor de Ultrasonido (HC-SR04)

El sensor de ultrasonido **HC-SR04** funciona emitiendo una onda ultrasónica inaudible a través de un emisor (Trigger) y esperando que la onda rebote contra un objeto para ser recibida en un receptor (Echo). Midiendo el tiempo transcurrido en el viaje del sonido, calcula de forma precisa la distancia al objeto.

Dispone de 4 pines:
1. **VCC:** Conexión a 5V.
2. **TRIGGER (Trig):** Dispara el pulso ultrasónico.
3. **ECHO (Echo):** Recibe el rebote del pulso.
4. **GND:** Conexión a tierra.

### 🔌 Componentes Necesarios
* 1x Arduino UNO y Placa de pruebas
* 1x Sensor de ultrasonido HC-SR04

### 🗺️ Esquema de Conexión
![Esquema Ultrasonido](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-31.png)

### 👣 Pasos para la Conexión
1. Conectar **VCC** a 5V y **GND** a tierra en la placa de pruebas.
2. Conectar el pin **TRIGGER** del sensor a un pin digital de tu elección (ej. **pin 11**).
3. Conectar el pin **ECHO** del sensor a otro pin digital (ej. **pin 10**).

---

### 💻 Programación utilizando la librería `NewPing`
Para facilitar el uso de este sensor sin lidiar directamente con las matemáticas de conversión del tiempo de vuelo de la onda, utilizaremos la librería **NewPing** creada por Tim Eckel.

> 📥 **¿Cómo instalar la librería NewPing en el IDE de Arduino?**
> 1. Ve al menú superior **Programa** > **Incluir librería** > **Administrar librerías...**
> 2. En el cuadro de búsqueda del Gestor de Librerías escribe **`newping`**.
> 3. Busca la opción **NewPing de Tim Eckel** y haz clic en **Instalar**.

![Libreria Newping](img/GUIA%20ARDUINO%20ARTE%20Y%20ROB%C3%93TICA%202023-32.png)

#### 📝 Código de Ejemplo (NewPingExample)
```cpp
#include <NewPing.h> // Incluimos la librería al programa

#define TRIGGER_PIN 11   // Pin conectado a TRIGGER
#define ECHO_PIN    10   // Pin conectado a ECHO
#define MAX_DISTANCE 200 // Distancia máxima a medir en cm (máx. 400cm)

// Inicializamos el sensor con los parámetros anteriores
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  Serial.begin(115200); // Inicializamos el monitor serie a 115200 baudios
}

void loop() {
  delay(50); // Pausa de 50ms entre mediciones (mínimo recomendado: 29ms)
  
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm()); // Lee la distancia y la convierte a cm
  Serial.println("cm");
}
```

### 📖 Explicación del Código
* **`#define`**: Es una directiva de preprocesamiento que define constantes. A diferencia de las variables, estas constantes consumen menos memoria y no pueden cambiar su valor durante la ejecución.
* **`NewPing sonar(TRIG, ECHO, MAX);`**: Define el objeto `sonar` entregándole tres parámetros obligatorios.
* **`Serial.begin(115200);`**: En este ejemplo inicializamos el monitor serie a una velocidad más rápida de **115200 baudios** para recibir lecturas más veloces.
* **`sonar.ping_cm();`**: Función de la librería que realiza todo el proceso eléctrico de disparar el Trigger, esperar el Echo, calcular la física del sonido y retornar el valor entero final en **centímetros**. Si retorna un `0`, significa que el objeto está fuera del rango máximo configurado.

> 🏆 **Desafío Final:**
> Por si te pica la curiosidad, el sensor de ultrasonido puede programarse perfectamente sin el uso de librerías externas utilizando las funciones integradas `pulseIn()`. Busca en Google cómo funciona la programación de este sensor sin librería, analízalo y compáralo con el código de arriba. ¿Cuál te parece más fácil de entender?
