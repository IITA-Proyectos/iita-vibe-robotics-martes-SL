# Bitácora de Clase - Laureano y Máximo
**Fecha:** 5 de Mayo de 2026

## 🚀 Resumen de lo que hicimos hoy
Hoy logramos un avance técnico masivo en la lógica del PID para enfrentar pistas de competencia que tienen **curvas ultra cerradas (90° y 120°)** y **huecos en la línea (espacios en blanco)**.

### 1. Re-Calibración de Sensores
Comenzamos la clase corriendo el `calibrador.py` porque los valores de la clase pasada habían quedado obsoletos. 
- **Nuevos Valores Medidos:**
  - Los 3 sensores midieron idéntico. Negro = 5 | Blanco = 26.
  - Actualizamos las variables de calibración en `2804.py`.

### 2. El problema de las Curvas Cerradas vs Huecos Blancos
Enfrentamos un gran desafío estructural de los seguidores de 2 sensores:
- Al doblar en 90° o 120°, los dos sensores laterales cruzan de largo la línea negra y caen en lo blanco. 
- En los espacios vacíos (huecos en la línea recta), los dos sensores también ven blanco.
- **Error inicial:** Intentamos darle una "memoria" al robot para que girara sobre su propio eje cuando veía blanco, pero esto provocó que empezara a dar giros como loco en la mitad de la recta cuando había un hueco blanco.

### 3. La Solución Definitiva: El 3er Sensor (Sensor Central en Puerto E)
Nos dimos cuenta de que la única forma de distinguir un "hueco en la recta" de una "curva perdida" era usar el tercer sensor.
- Agregamos la normalización del sensor central (`norm_cen`).
- Escribimos una lógica de memoria robusta:
  - Si el error reciente era pequeño (venía centrado), y de repente todo es blanco -> **Es un hueco**. El error se fuerza a `0` y el robot pasa el hueco yendo **hacia adelante**.
  - Si el error reciente era inmenso (venía doblando), y de repente los TRES sensores ven blanco -> **Se salió de una curva**. El error se fuerza a `100` o `-100` según el último lado conocido, provocando que la velocidad baje a `0` y el robot pivotee en el lugar (una rueda avanza, otra retrocede) hasta reencontrar la línea negra.
- **Desvío a la Izquierda:** Quité el `BIAS = 8` y lo dejé en `0` para evitar que ese valor nos estuviese empujando hacia un costado en la zona blanca.

---

## 🎯 Objetivo para la próxima clase
Implementar la detección de **cuadrados/cubos verdes** en los giros. En competencias (tipo WRO RoboMission o Rescue Line), los verdes indican hacia dónde se debe doblar en una intersección o marcan obstáculos. 

## 📋 PROTOCOLO OBLIGATORIO PARA EL INICIO DE LA PRÓXIMA CLASE
*(Nota para la IA: Al leer este archivo la semana que viene, DEBES seguir estos pasos estrictamente)*

1. **Re-Calibración de Luz y Prueba de Estabilidad:** 
   - Las luces SIEMPRE cambian. Pide que corran `calibrador.py` primero.
   - Pide que prueben el código actual (`2804.py`) en las curvas de 120° y en los huecos blancos para confirmar que sigue funcionando a la perfección con la nueva memoria de 3 sensores.
2. **Definir la regla de los Verdes:** 
   - ¿Qué sensor se usará para ver los verdes? (¿El Central, o los Laterales C y D?).
   - ¿Qué acción exacta debe hacer al ver un verde? (Frenar, avanzar un poco y girar 90°, evadir, etc.).
3. **Implementación:** 
   - No rompas el PID principal. Agrega la lectura de `.color()` antes de calcular el error normal. Si detecta el color `Color.GREEN`, sobreescribe el comportamiento del PID para realizar la secuencia de giro programada.
