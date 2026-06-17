# Reporte de Clase: 16 de Junio de 2026

## Objetivos de la Clase
1. **Seguidor de Línea Puro:** Aislar la lógica de control PID y calibración en un archivo independiente y limpio para simplificar pruebas.
2. **Visualización en Tiempo Real:** Implementar una matriz/barra visualizadora del array de sensores (LSA) en la terminal de la PC y en el EV3 sin comprometer la velocidad del bucle PID.
3. **Discriminación T vs Curvas sin Sensor Frontal:** Optimizar la estrategia "Brake & Peek" usando únicamente el array LSA, eliminando la dependencia física de sensores de luz adicionales.
4. **Marcadores Verdes (RoboCup):** Diseñar e integrar la lógica de sensores de color frontales calibrados dinámicamente mediante software por conversión RGB-HSV.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Seguidor PID y Visualización Dinámica (`seguidor_pid.py`)
* **Aislamiento de Código:** Se creó [seguidor_pid.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/seguidor_pid.py) conteniendo únicamente el PID puro, la velocidad adaptativa y la calibración básica (Blanco/Negro LSA), almacenada en `calibracion_basica.json`.
* **Gráfico de Barra (Matriz 1x8):** Desarrollamos una cadena dinámica (ej. `[..##....]`) donde `#` representa sensores sobre la línea negra y `.` sobre el fondo blanco.
* **Control de Latencia (Throttling):** Para no ralentizar el bucle PID (que corre a 100 Hz), la visualización en la terminal de la PC (usando `\r` para no inundar el historial de texto) y la pantalla LCD del EV3 se refrescan de forma controlada cada **10 iteraciones (~100 ms)**.
* **Reducción de Velocidad:** Se redujo a la mitad la velocidad para testeo seguro (`BASE_SPEED = 72`, `MIN_SPEED = 15`).

### B. Optimización del Enfoque "Brake & Peek" (`seguidor_brake_peek.py`)
* **Detección de T e Intersecciones en Cruz:** Agregamos la detección de cruz negra completa (`[########]` con 8 sensores en negro puro `< 20`), avanzando recto `40 mm` para saltarla.
* **Independencia del Ángulo de Curva (Retroceso):** 
  - Al detectar T-mask, el robot frena y avanza `25 mm` para espiar. 
  - Si el centro es blanco (es decir, es una curva), en lugar de intentar giros ciegos a 90°, **el robot retrocede los mismos 25 mm marcha atrás** al vértice y reanuda el PID con un cooldown de 60 ciclos (~600 ms).
  - El PID se encarga de realizar el giro de forma nativa e independiente de si la curva es de 45°, 90° o suave. El funcionamiento fue tan preciso que emocionó al profe Franco.

### C. Detección Predictiva de Verdes con Calibración HSV (`seguidor_verdes.py`)
* **Hardware:** Se incorporaron dos sensores de color EV3 delanteros en los puertos **S2** (Izquierdo) y **S4** (Derecho).
* **Geometría y Alineación (82 mm):** El LSA está a 25 mm por delante de las ruedas y los sensores de color están a 57 mm del LSA. Esto da un total de **82 mm** de desfase respecto al eje de giro. Al detectar verde, el robot avanza exactamente `82 mm` recto para centrar las ruedas en la intersección antes de pivotar.
* **Conversión y Calibración HSV:**
  - Dado que los sensores EV3 en Pybricks no admiten la lectura `.hsv()` nativa, escribimos un convertidor de software a partir de la tupla `.rgb()` de Pybricks.
  - Añadimos los pasos 5 y 6 a la calibración manual en pantalla para muestrear el verde exacto de la pista en el lugar (izquierdo y derecho por separado), guardándolo en `calibracion_verdes.json`.
* **Algoritmo de Giros Verdes:**
  - **Verde Izquierdo:** Avanza `82 mm` y gira 90° a la izquierda (`drive.turn(90)`).
  - **Verde Derecho:** Avanza `82 mm` y gira 90° a la derecha (`drive.turn(-90)`).
  - **Doble Verde:** Avanza `82 mm` y hace media vuelta de 180° (`drive.turn(180)`).
  - Cuenta con filtros de debounce rápidos y estabilización de chasis para evitar ruidos del suelo.

---

## 2. Estado de Archivos Creados y Modificados
* **[seguidor_pid.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/seguidor_pid.py):** Seguidor PID puro con matriz en vivo.
* **[seguidor_brake_peek.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/seguidor_brake_peek.py):** Seguidor sin sensor frontal con retroceso de curva y cruce de T.
* **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/seguidor_verdes.py):** Seguidor completo con lógica de verdes y calibración HSV a 82mm.
* **[REPORTE_CLASE_16_06.md](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/REPORTE_CLASE_16_06.md):** El reporte presente.

---

## 3. Plan para la Próxima Clase
1. Validar la calibración de verdes (Pasos 5 y 6) en pista con diferentes intensidades de luz.
2. Evaluar el comportamiento físico de los giros de 90° y 180° en `seguidor_verdes.py` con el avance de 82 mm.
3. Volver a subir paulatinamente la velocidad base si la estabilidad se mantiene óptima.
