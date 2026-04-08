# 🤖 Laureano
Mi carpeta de trabajo.


## 07/04

Resumen completo del desarrollo del seguidor de línea
1) Configuración inicial del robot

Primero definimos tu hardware así:

Motor izquierdo: Puerto A
Motor derecho: Puerto B
Sensor izquierdo: Puerto C
Sensor derecho: Puerto D

Después aclaraste un dato muy importante: el motor izquierdo está invertido, así que para avanzar debe recibir velocidad negativa, mientras que el motor derecho avanza con velocidad positiva.

Ese detalle fue clave, porque en robots de LEGO SPIKE o Pybricks la orientación física de los motores cambia totalmente la lógica del programa. Si no se respeta eso, el robot termina girando mal o directamente sobre su propio eje.

2) Primer enfoque: seguidor simple proporcional

Al principio usamos una idea básica:

leer la reflexión de ambos sensores,
calcular la diferencia entre ellos,
usar esa diferencia para corregir la dirección.

La lógica era algo como:

si el sensor izquierdo ve más oscuro que el derecho, girar hacia ese lado,
si el derecho ve más oscuro, girar al otro,
si ambos ven parecido, ir recto.
Qué buscaba ese enfoque

La idea era que el robot siguiera la línea negra sobre fondo blanco con una corrección proporcional, algo muy común para empezar.

Qué pasó

En la práctica, sobre una superficie blanca o negra uniforme, el robot empezaba a girar solo, incluso sin tener línea clara. Eso pasó porque:

los sensores nunca leen exactamente lo mismo,
una mínima diferencia ya se convertía en corrección,
esa corrección, multiplicada por Kp, hacía que un motor avance y el otro retroceda,
y eso terminaba haciendo que el robot girara sobre su eje.
Resultado

Ese primer enfoque no funcionó bien para tu robot.

3) Primera corrección: zona muerta

Para evitar que el robot reaccionara a diferencias mínimas, agregamos una zona muerta o deadband.

La idea era:

si la diferencia entre sensores es muy pequeña, no corregir nada,
si la diferencia supera cierto límite, entonces sí corregir.
Qué salió bien

Eso ayudó un poco a evitar giros por ruido pequeño.

Qué no salió bien

No resolvió el problema principal, porque el robot seguía girando cuando la diferencia era suficiente para activar la corrección. O sea, la zona muerta redujo el ruido, pero no corrigió la lógica base.

4) Segunda corrección: limitar la corrección máxima

Luego limitamos la corrección máxima para evitar que el robot se pasara de giro.

La idea fue:

calcular la corrección,
si es demasiado grande, recortarla a un valor máximo,
así evitar que uno de los motores se invierta demasiado.
Qué salió bien

Esto ayudó a evitar giros bruscos extremos.

Qué no salió bien

Seguía existiendo un problema de fondo: la dirección de la corrección no estaba bien alineada con la orientación real del robot.

5) Tercera corrección: usar un valor objetivo

Después intentamos usar un valor de referencia, o “objetivo”, para comparar cada sensor con un punto medio entre blanco y negro.

La idea era:

medir blanco y negro,
tomar un valor intermedio,
considerar que el robot debía mantenerse alrededor de ese punto.
Qué salió bien

Eso ayudó a pensar en términos de calibración real.

Qué no salió bien

Todavía no resolvía el problema principal del control, porque el robot seguía sin entender correctamente hacia dónde corregir en tu configuración física concreta.

6) Ajuste clave: respetar el motor izquierdo invertido

Después aclaraste algo esencial: el motor izquierdo avanza con velocidad negativa.

Eso cambió completamente la forma de escribir el control.

Qué se corrigió

Se ajustó el cálculo de velocidades para que:

el motor izquierdo siempre use signo negativo para avanzar,
el motor derecho use signo positivo para avanzar.
Qué salió bien

Esta corrección sí fue importante. Sin esa inversión correcta, el robot no podía comportarse de manera coherente.

Qué no salió bien

Todavía podía darse el caso de que la corrección empujara a uno de los motores a cruzar cero y pasar a girar al revés. Y cuando eso pasaba, el robot tendía a girar sobre su propio eje.

7) Debug: impresión de valores de sensores

Después pediste que el programa imprimiera los valores de cada sensor para entender qué estaba detectando realmente.

Se imprimieron muchísimos valores, y eso fue una gran ayuda.

Lo que mostraron tus datos reales

Tu secuencia de valores dejó algo clarísimo:

En superficie blanca, los sensores marcaban aproximadamente entre 63 y 78.
En zonas oscuras o negras, bajaban aproximadamente a 2 a 12.
En algunos tramos intermedios aparecían valores como 47, 49, 41, que probablemente eran bordes, transición o zonas grises.
Qué significó eso

Eso confirmó que:

los sensores sí estaban funcionando,
sí veían diferencias reales entre blanco y negro,
el problema no era de lectura,
el problema era de control y lógica.
Qué salió bien

Esto fue de las mejores partes del proceso, porque finalmente tuvimos evidencia real para ajustar el programa.

8) Corrección equivocada: hacer que ambos sensores negros causen giro fijo

En un momento se intentó hacer que, si ambos sensores veían negro, el robot girara para buscar la línea.

Qué salió bien

La intención era buena: no dejar al robot perdido cuando no distinguía la línea.

Qué no salió bien

Eso no era correcto para un seguidor de línea bien hecho, porque un seguidor no debería girar siempre hacia un solo lado por defecto. Eso lo vuelve sesgado y poco inteligente.

Tú mismo lo notaste: un seguidor de línea no tiene que “irse” a la derecha o a la izquierda siempre.

9) Corrección mejorada: memoria de dirección

Para resolver eso se agregó una idea mejor:

recordar cuál fue la última dirección en la que estaba la línea,
si se pierde la línea, buscar hacia ese lado.
Qué salió bien

Eso fue una mejora real. Ya no giraba siempre para un lado fijo, sino que usaba una memoria simple.

Qué no salió bien

Aunque ayudaba a recuperarse cuando perdía la línea, seguía sin ser la mejor solución para el centrado fino.

10) Corrección del centrado de la línea

Después notaste que el robot seguía la línea, pero no se centraba bien.

Ahí vimos que usar solo la diferencia directa entre sensores no era suficiente para centrar finamente.

Qué hicimos

Se intentó comparar cada sensor con un valor medio entre blanco y negro, para acercar la lógica al centro real de la línea.

Qué salió bien

Eso mejoró un poco la intención del control.

Qué no salió bien

Todavía había un problema de orientación: el robot a veces corregía hacia el lado contrario del que debía. Es decir, si la línea estaba a la izquierda, el robot se iba a la derecha, y viceversa.

11) Corrección importante: invertir el signo de la corrección

Ese fue uno de los hallazgos más importantes.

Te diste cuenta de que el robot estaba corrigiendo al revés:

cuando debía girar hacia donde no estaba la línea, hacía lo contrario.

Entonces invertimos el cálculo del error.

Qué salió bien

Esto sí corrigió la dirección de giro. Fue una corrección muy importante porque alineó la lógica con el comportamiento físico del robot.

Qué no salió bien

Aun con la dirección bien, el robot podía tardar en reaccionar en curvas cerradas.

12) Mejora en curvas: agregar derivada (control PD)

Después de eso, el robot seguía la línea, pero en curvas pronunciadas tardaba en girar.

Eso es normal cuando usás solo control proporcional:

el robot reacciona al error actual,
pero no “ve venir” la curva.

Por eso se agregó la derivada, formando un control PD.

Qué hace la derivada

La derivada mide qué tan rápido cambia el error.

Si el error está aumentando rápido, el robot entiende que viene una curva fuerte y corrige antes.

Qué salió bien

Esto mejoró la anticipación en curvas.

Qué no salió bien

A veces hacía el robot demasiado sensible o demasiado brusco, dependiendo de los valores de Kp y Kd.

13) Velocidad progresiva según el error

Después observaste algo muy importante: el robot aumentaba velocidad en negro, y eso te parecía innecesario.

Entonces se propuso una velocidad progresiva:

recta: más rápido,
curva leve: velocidad media,
curva fuerte: más lento.
Qué salió bien

Esta es una de las ideas más correctas para un seguidor de línea rápido y estable. Reducir velocidad en curvas ayuda muchísimo a no perder la línea.

Qué no salió bien

En la práctica, con tu robot, esto no fue la solución que más te convenció. Funcionaba conceptualmente, pero tú viste que el robot iba mejor con un comportamiento más simple y agresivo.

14) Tu gran descubrimiento: Kp = 5 y Kd = 0

Después hiciste una prueba decisiva:

Kp = 5
Kd = 0

Y me dijiste que mejoró.

Qué significa eso

Eso quiere decir que tu robot necesitaba una corrección proporcional mucho más fuerte y directa.

En palabras simples:

el robot antes corregía muy suave,
ahora corrige fuerte y rápido,
y eso le da mejor respuesta en curvas.
Qué salió bien

Esto fue una de las mejores configuraciones que probaste.

Qué no salió bien

Al eliminar la derivada, el sistema perdió suavizado y anticipación. O sea, se volvió más agresivo, aunque en tu robot eso resultó positivo.

15) Explicación simple de Kp y Kd

Después te expliqué esto de forma simple:

Kp

Es la fuerza con la que el robot corrige el error actual.

Kp bajo: corrige suave, pero tarda.
Kp alto: corrige fuerte, pero puede zigzaguear.
Kd

Mira cómo cambia el error.

ayuda a anticipar curvas,
suaviza cambios bruscos,
evita sobrecorrección.
Lo que entendiste de eso

Con Kp = 5 el robot reaccionaba más rápido. Y con Kd = 0 se volvía más directo.

16) Idea avanzada: Kp variable o dinámico

Después propusiste algo muy bueno:

en curvas pronunciadas, subir el Kp,
luego bajarlo para estabilizar.

Eso es totalmente válido. Se llama control adaptativo o ganancia variable.

Qué intentamos

Se propuso un programa donde:

si el error es pequeño, Kp es bajo,
si el error es grande, Kp sube.
Qué salió bien

Es una idea muy inteligente y muy útil para robots de competencia.

Qué no salió bien

No fue algo que terminara consolidándose como la solución final, pero la idea es correcta y muy potente.

Correcciones que salieron bien

Estas fueron las que realmente aportaron mejoras:

1. Tener en cuenta el motor izquierdo invertido

Fue indispensable.

2. Imprimir valores de sensores

Sirvió muchísimo para diagnosticar.

3. Invertir el signo de la corrección

Arregló la dirección del giro.

4. Subir Kp a 5

Le dio reacción mucho más rápida.

5. Dejar Kd = 0

En tu caso concreto, simplificó el control y mejoró la respuesta.

6. Pensar en Kp dinámico

Muy buena idea conceptual para curvas.

Correcciones que no salieron bien o no alcanzaron

Estas no resolvieron del todo el problema:

1. Solo usar diferencia entre sensores sin calibración

Provocaba giros indebidos.

2. Zona muerta sola

Ayudó un poco, pero no arregló la lógica.

3. Limitar corrección sin revisar dirección

No solucionó el fondo del problema.

4. Hacer que ambos sensores negros hagan giro fijo

Sesgaba el comportamiento del robot.

5. Memoria de dirección como solución principal

Sirvió como apoyo, pero no como solución central.

6. Velocidad progresiva como única respuesta

Conceptualmente buena, pero no fue la mejora que más te funcionó en tu robot.

7. Kp bajo con Kd alto

No le daba la agresividad que necesitaba tu configuración.

Qué aprendimos de todo esto

Este proceso dejó varias conclusiones muy claras:

1. El hardware manda

Si el motor izquierdo está invertido, eso cambia toda la lógica.

2. Los sensores hay que medirlos de verdad

No basta con suponer valores. Tus lecturas reales mostraron exactamente cómo se comportaba el robot.

3. Un seguidor de línea necesita corregir rápido

En tu caso, una corrección agresiva con Kp alto funcionó mejor que un control muy fino.

4. El signo de la corrección importa muchísimo

A veces el problema no es la magnitud, sino la dirección.

5. En robótica real, probar y medir vale más que adivinar

Tus datos de sensor fueron la clave para entender el comportamiento.

Conclusión final

El camino completo fue este:

empezamos con un seguidor simple,
vimos que giraba mal incluso en blanco o negro,
detectamos que el motor izquierdo estaba invertido,
imprimimos lecturas reales de sensores,
entendimos que los sensores sí funcionaban,
notamos que la corrección estaba al revés,
mejoramos la respuesta en curvas con PD,
luego descubriste que Kp = 5 y Kd = 0 te daba mejor resultado,
y finalmente llegamos a la idea de que la corrección debe ser rápida, directa y coherente con la orientación real del robot.

En tu robot, la mejora más importante no fue hacer el control más sofisticado, sino hacerlo coherente con el hardware y suficientemente agresivo para responder a tiempo.
