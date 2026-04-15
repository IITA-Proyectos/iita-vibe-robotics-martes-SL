# Plan de Implementación: PD y Sistema de Recuperación

## 1. Ajuste de Reactividad (PD)

Para hacer que el robot sea más reactivo en esas curvas en "diente de sierra", vamos a ajustar los valores nuevamente:
*   Subiremos el **KP a 1.8** o **2.0** (Aumentando la reacción al error).
*   Subiremos de forma proporcional el **KD a 8.0** o **10.0** (Frenando la inercia para no pasarnos).

## 2. Nuevo Sistema: Lógica de Recuperación (Line Recovery)

Nos has pedido que si el robot se sale, retroceda y la busque. Esto es tremendamente útil en el circuito de rescate (y puede salvarte la vida en una curva a 90° o sierra extrema).

### ¿Cómo sabremos que se perdió verdaderamente?
Actualizaremos el bucle infinito para consultar los **3 sensores** (Izquierdo, Central, Derecho).
*   Si los **TRES sensores leen valores muy cercanos al blanco puro** (por ejemplo, más de 45 en bruto), significa que el robot está navegando a ciegas sobre la pista blanca.

### > [!IMPORTANT]
### Pregunta de Diseño (RoboCup Rules)
En la Rescue Line Junior suele haber "Gaps" o huecos (partes donde la línea se corta intencionalmente durante 10-15 cm).
Si el robot retrocede Inmediatamente apenas lee todo blanco, **jamás superará el Hueco** (llegará, leerá blanco, y retrocederá eternamente).

**Solución Propuesta para tu aprobación:**
1.  **Detección:** Si los 3 sensores leen blanco, activamos un cronómetro (`StopWatch` de Pybricks).
2.  **Tolerancia a Huecos:** El robot seguirá avanzando recto por **0.5 a 1 segundo** (suficiente tiempo para cruzar un hueco en la pista). 
3.  **Marcha Atrás:** Si pasa ese tiempo y el sensor central aún no ha tocado el negro de la línea, ¡Asumimos que nos perdimos en una curva!
4.  **Ejecución del Rescate:** Los motores accionan `robot.drive(-VELOCIDAD, 0)` (marcha atrás recta) hasta que cualquiera de los sensores, especialmente el central, vuelva a pisar el negro de la línea. Se reinicia el PID.

¿Estás de acuerdo con añadir el **tiempo de tolerancia (para los huecos)** antes de ejecutar la marcha atrás? ¿O en tu pista actual no hay huecos e implementamos la marcha atrás inmediata? Espero tu respuesta para actualizar el código.
