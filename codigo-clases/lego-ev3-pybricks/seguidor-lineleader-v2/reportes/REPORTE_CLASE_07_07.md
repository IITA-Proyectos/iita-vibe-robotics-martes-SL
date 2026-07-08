# Reporte de Clase: 7 de Julio de 2026

## Objetivos de la Clase
El objetivo de la clase de hoy fue resolver el problema en el circuito de la **rotonda**, donde el robot detectaba correctamente el primer marcador verde para ingresar, pero ignoraba el segundo verde necesario para reincorporarse a la línea principal. Además, se buscó eliminar las pausas prolongadas de detección y calibrar la sensibilidad del color verde.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Separación de Cooldowns (`t_cooldown_green` y `t_cooldown_t`)
* **El Problema:** Descubrimos que al usar un único contador de cooldown, al finalizar el giro de $90^\circ$ el robot reactivaba los sensores, pero el LSA principal todavía estaba sobre la intersección anterior. Al leerla, el LSA la interpretaba como una **falsa T** y le aplicaba al robot un cooldown largo de `40` o `60` ciclos. Durante ese tiempo, el robot quedaba "ciego" y pasaba de largo el segundo verde de reincorporación.
* **La Solución:** Dividimos el cooldown en dos contadores independientes:
  * `t_cooldown_green`: Maneja la ceguera de los sensores frontales ante el verde. Al terminar un giro, se le asigna **`5`** ciclos (unos $50\text{ ms}$), permitiendo detectar el siguiente verde casi de inmediato.
  * `t_cooldown_t`: Maneja el bloqueo de intersecciones del LSA. Al terminar un giro, se le asigna **`50`** ciclos (unos $500\text{ ms}$), asegurando que el LSA ignore por completo el cruce anterior del que acaba de salir.

### B. Optimización del Algoritmo "Peek & Check"
* **El Problema:** El robot tardaba casi un segundo inmóvil confirmando el verde debido a las pausas de depuración visual de $400\text{ ms}$ puestas la clase anterior.
* **La Solución:** Redujimos las dos pausas de parada a **`50 ms`** (`wait(50)`). El avance corto de $15\text{ mm}$ y el chequeo cruzado con el LSA ahora ocurren en milisegundos, haciendo que el paso por el verde sea dinámico y sin detenciones prolongadas.

### C. Calibración de Sensibilidad del Verde (`sens_verde = 1.1`)
* Bajamos el multiplicador del verde por defecto de `1.3` a **`1.1`** en la función [es_color_verde](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py#L151). 
* Esto hace que el algoritmo de distancia Euclidiana en HSV sea menos restrictivo y detecte el verde verdadero mucho más rápido, sin requerir forzar físicamente el robot contra el suelo.

### D. Velocidad de Prueba en Cooldowns (Speed Tuning)
* Se programó que cuando alguno de los cooldowns esté activo (`t_cooldown_green > 0` o `t_cooldown_t > 0`), la velocidad lineal del robot se reduzca a la mitad (`speed // 2`) para facilitar las pruebas visuales y comprobar en cámara lenta la trayectoria de reacomodación.

---

## 2. Estado de Archivos y Repositorio
* **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py):** Código actualizado con el sistema de doble cooldown, pausas reducidas a 50ms, sensibilidad en 1.1 y reducción de velocidad al 50% en cooldowns.

---

## 3. Diagnóstico y Tareas para la Próxima Clase

> [!WARNING]
> **Comportamiento post-giro en rotonda:** Tras completar el giro físico de $90^\circ$ en la rotonda, el chasis no llega a reacomodarse en paralelo con la línea antes de que los sensores frontales alcancen el segundo cuadradito verde. Esto hace que el sensor derecho lee el verde antes de tiempo debido a la desalineación angular del robot.

### Tareas para la próxima clase:
1. **Implementar Búsqueda Activa de Línea (`buscar_linea`):** Programar una rutina de barrido angular post-giro para forzar al robot a realinearse perfectamente con la línea negra antes de permitir que avance.
2. **Ajuste Físico:** Evaluar si es necesario retrasar ligeramente la posición física del faldón o de los sensores frontales para dar más margen de reacomodación al chasis.
