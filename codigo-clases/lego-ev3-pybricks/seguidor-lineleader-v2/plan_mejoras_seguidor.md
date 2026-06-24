# Plan de Mejoras e Ideas para el Seguidor de Línea (Profe Franco)

Este documento analiza la propuesta del clasificador de colores por distancia Euclidiana en HSV y recopila las mejores prácticas e ideas extraídas de los códigos anteriores de Spike/EV3, detallando cómo implementarlas en las próximas clases del taller.

---

## 1. Análisis del Clasificador de Colores HSV por Distancia

### La Propuesta:
El algoritmo calcula la distancia geométrica (Euclidiana) entre la lectura actual $[H, S, V]$ y los perfiles calibrados de **Verde**, **Negro** y **Blanco**:
$$\text{Distancia}^2 = (H - H_{\text{target}})^2 + (S - S_{\text{target}})^2 + (V - V_{\text{target}})^2$$
Aplica un multiplicador de sensibilidad (peso) y elige el color con la menor distancia.

### ¿Es buena idea añadirlo a la clase?
**Sí, es una excelente idea pedagógica y de control**, especialmente para solucionar los falsos positivos del verde.

#### Ventajas:
1.  **Simplificación de umbrales:** En lugar de codificar múltiples condicionales anidados (`if H_min < h < H_max and S > S_min...`), los alumnos solo calibran tres puntos tridimensionales.
2.  **Ponderación (Sensibilidad):** La sensibilidad ajustable es una herramienta muy potente. Al multiplicar la distancia del verde por `1.2`, aumentamos su distancia matemática artificialmente, haciendo que el algoritmo sea "más estricto" para elegir verde, evitando falsas detecciones en los bordes degradados de la línea negra.

#### Adaptación y Correcciones para EV3:
Dado que el EV3 no tiene lectura HSV nativa y usamos un conversor, debemos resolver dos problemas matemáticos importantes:

1.  **El problema del ciclo del Hue (0-359):**
    El tono es circular ($0^\circ = 360^\circ$). Si el verde calibrado es $10^\circ$ y leemos $350^\circ$, la resta da $-340$, que al cuadrado es $115,600$ (una distancia enorme), cuando en realidad están a solo $20^\circ$ de distancia.
    *   *Solución:* Calcular la diferencia absoluta circular antes de elevar al cuadrado:
        ```python
        diff_h = abs(h - target_h)
        if diff_h > 180:
            diff_h = 360 - diff_h
        ```
2.  **La inestabilidad del Hue en Blanco y Negro (Baja Saturación):**
    Cuando el sensor lee blanco o negro, la saturación ($S$) es muy baja (cercana a 0). En estas condiciones, el valor de Hue ($H$) se vuelve sumamente inestable y fluctúa de forma aleatoria. Esto puede disparar distancias falsamente pequeñas hacia el verde si el Hue coincide de casualidad.
    *   *Solución:* Ignorar la distancia de Hue ($H$) si la saturación medida $S < 20$. O bien, usar una distancia modificada donde el peso del Hue dependa de la saturación:
        ```python
        # Si la saturación es baja, no evaluar Hue
        if s < 20:
            dist_verde = ((s - verde_s)**2 + (v - verde_v)**2) * sens_verde
        else:
            dist_verde = (diff_h**2 + (s - verde_s)**2 + (v - verde_v)**2) * sens_verde
        ```

---

## 2. Ideas Clave de Códigos Anteriores para Incorporar en Clase

Analizando los códigos antiguos (`a_main.py`, `m_main.py`, `robotL.py`), se identifican las siguientes ideas de alto impacto:

### A. Algoritmo Activo de "Búsqueda de Línea" (`buscar_linea_derecha` / `izquierda`)
*   **De dónde sale:** De `a_main.py`.
*   **La Idea:** Tras hacer un giro de $90^\circ$ por un marcador verde, en lugar de asumir que el sensor quedó exactamente sobre la línea negra y reanudar el PID, el robot ejecuta una rutina de barrido. Gira hasta un ángulo límite (ej. $45^\circ$) buscando que el sensor de color/LSA vuelva a cruzar el umbral negro antes de continuar.
*   **Por qué sirve:** En competencias, un desajuste en las ruedas o el desgaste de batería hace que los giros de $90^\circ$ queden en $85^\circ$ o $95^\circ$. Con esta rutina de búsqueda, el robot se vuelve inmune a esos pequeños errores de rotación física.

### B. Corrección Angular y Estabilización con Giroscopio (`avanzar_recto`)
*   **De dónde sale:** De `robotL.py` y `a_main.py`.
*   **La Idea:** Usar el giroscopio del hub para avanzar en línea recta con un controlador proporcional simple (`error = angulo_objetivo - yaw`).
*   **Por qué sirve:** 
    *   Al pasar intersecciones en cruz o T, en lugar de hacer un avance ciego `drive.straight(40)`, podemos forzar al robot a avanzar manteniendo el ángulo exacto que traía (`yaw` de entrada).
    *   Prepara a los chicos para la **Zona de Rescate**, donde no hay líneas y el robot debe navegar orientándose únicamente con las paredes y el giroscopio para buscar víctimas.

### C. Rampa de Velocidad Trapezoidal
*   **De dónde sale:** De `robotL.py`.
*   **La Idea:** Ajustar la velocidad lineal en función de la distancia recorrida (`velocidad_min + (velocidad_max - velocidad_min) * (distancia / rampa_dist)`).
*   **Por qué sirve:** Evita que el robot patine por aceleraciones bruscas en las rectas y asegura frenadas progresivas y exactas al llegar a las paredes en la zona de evacuación.

---

## 3. Plan de Trabajo Sugerido para las Próximas Clases

| Clase | Tema | Objetivo Físico / Algorítmico |
| :--- | :--- | :--- |
| **Clase 1** | Implementación del Clasificador HSV | Escribir y validar la fórmula de distancia en HSV en el EV3. Resolver el problema de inestabilidad en blanco/negro. |
| **Clase 2** | Robustez en Giros Verdes | Integrar las rutinas de búsqueda de línea (`buscar_linea`) para corregir desvíos mecánicos después de los giros de $90^\circ$ y $180^\circ$. |
| **Clase 3** | Avance Asistido por Giroscopio | Modificar los avances en T y Cruz para que utilicen la estabilización del giroscopio del EV3, evitando desvíos en el cruce. |
| **Clase 4** | Iniciación en Zona de Rescate | Reutilizar los motores del EV3 y algoritmos de `robotL.py` para programar la navegación sistemática con giroscopio y sensor de ultrasonido dentro del área sin líneas. |
