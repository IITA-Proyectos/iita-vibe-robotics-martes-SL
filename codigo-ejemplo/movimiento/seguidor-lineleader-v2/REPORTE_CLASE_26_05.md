# Reporte de Clase: 26 de Mayo de 2026

## Objetivos de la Clase
El objetivo principal de hoy fue optimizar el seguidor de línea para curvas cerradas de 90° o más, corregir los problemas dinámicos en la pérdida de línea, y desarrollar un sistema de detección y calibración robusto para intersecciones de T (izquierda y derecha) y cruz, evitando falsos positivos durante el trayecto.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Ajuste de Velocidades y PID para Curvas de 90°
* **Problema:** En curvas de 90° o más cerradas, el robot se pasaba de largo y perdía la línea debido al exceso de velocidad y falta de fuerza en el torque de giro.
* **Solución:**
  * Subimos `KP` de `3.5` a **`4.5`** y `KD` a **`22.0`** para dar una respuesta inmediata y con mejor amortiguamiento en las curvas.
  * Redujimos `BASE_SPEED` a **`130`** e implementamos una velocidad mínima en curvas `MIN_SPEED` muy baja de **`30`** (antes `80`).
  * **Efecto:** Al entrar a un ángulo cerrado y aumentar el error, el robot frena casi a cero avance lineal, lo que le permite pivotar de forma controlada sobre su propio eje físico (necesario ya que el sensor está ubicado en el eje de las ruedas).

### B. Corrección Matemática del Modo Recuperación (Recovery)
* **Problema:** Cuando el robot se salía en curvas rápidas o cerradas y entraba en modo recuperación, quedaba en un ángulo desfavorable (perpendicular a la línea o de espaldas) y perdía la orientación de avance.
* **Solución:**
  1. **Inversión de Giro en Reversa (`-turn`):** Cambiamos el giro al retroceder de `turn` a **`-turn`**. Físicamente, para desandar un camino curvo marcha atrás, la rotación debe ser la opuesta a la que causó la salida. Al invertir el signo, el robot endereza su trayectoria trasera y se realinea en paralelo con la línea.
  2. **Debounce / Filtro de Tiempo:** Añadimos un contador `loss_counter` para ignorar pérdidas menores a **8 ciclos (~80ms)**. Si la pérdida es momentánea por ruido o rebote, el robot continúa con el avance que traía sin tironear. Si supera los 80ms, aplica la marcha atrás controlada a **`-70 mm/s`**.

### C. Detección de Intersecciones en Cruz (8 Sensores)
* **Problema:** La detección previa con `count_on_line >= 7` se activaba por ruido o sombras en las curvas.
* **Solución:** 
  * Definimos un umbral estricto `INTERSECTION_THRESHOLD = 20` (negro puro).
  * Exigimos que **los 8 sensores lean negro simultáneamente** (`count_on_line == 8`). 
  * Dado que la línea en curvas solo puede tapar a lo sumo 2 o 3 sensores a la vez, es físicamente imposible falsear esta detección. Al detectarse, el robot se detiene, emite un tono largo (600Hz, 300ms) y avanza recto `40 mm` para pasar el cruce.

### D. Detección y Calibración de Intersecciones en T
* **Problema:** El robot debía detectar intersecciones en T (izquierda y derecha), avisar sonoramente, e ignorarlas continuando derecho, sin confundirlas con las curvas de la pista.
* **Solución:**
  1. **Calibración de T Izquierda y T Derecha por Software:** Añadimos los pasos 3 y 4 a la `calibracion_manual`. El robot lee los sensores sobre la T, genera una máscara binaria (`T_L_MASK` y `T_R_MASK`) donde `True` representa negro y `False` representa blanco, y la guarda de manera persistente en `calibracion.json`. La pantalla muestra el patrón binario (ej. `1 1 1 1 0 0 0 0`) para depuración en tiempo real.
  2. **Algoritmo de Doble Zona de Coincidencia (`matches_mask`):**
     * En lugar de contar coincidencias totales sueltas, reescribimos la función para validar de manera separada:
       * **Zona Negra Esperada:** Que los sensores que deben ser negros lo sean (tolerancia de -1).
       * **Zona Blanca Esperada:** Que los sensores que deben ser blancos lo sean (tolerancia de -1).
     * **Por qué funciona:** En una curva cerrada, el centro del sensor se vuelve blanco. Con la lógica previa de conteo simple, el robot aún sumaba suficientes aciertos en los extremos como para confundirla con una T. Al exigir de forma estricta que la zona negra esperada (que incluye el centro en una T) sea negra, las curvas son rechazadas exitosamente.
  3. **Comportamiento en T:** Al pasar por una T izquierda emite dos pitidos a `800Hz` y al pasar por una T derecha emite dos pitidos a `1000Hz`, en ambos casos avanzando recto `40 mm` para superar la bifurcación y seguir camino de frente.

---

## 2. Estado de Archivos
* **`main.py`**: Contiene la lógica PID ajustada, velocidad adaptativa extrema (30-130), modo recuperación debounceado con giro de reversa corregido (`-turn`), y filtros matemáticos para intersecciones T y Cruz con calibración manual en pantalla.
* **`calibracion.json`**: Guarda el mapa de calibración: Blanco, Negro, T Izquierda (`T_L_MASK`) y T Derecha (`T_R_MASK`).

---

## 3. Tareas para la Próxima Clase
1. **Reactivación de Marcadores Verdes:** Integrar la detección de color verde utilizando los rangos dinámicos independientes guardados, aplicando la misma filosofía de filtros de tiempo (debounce) implementada hoy para evitar falsos positivos.
2. **Pruebas de Velocidad:** Si la estabilidad en curvas se mantiene sólida, ensayar el incremento de `BASE_SPEED` de forma progresiva.
