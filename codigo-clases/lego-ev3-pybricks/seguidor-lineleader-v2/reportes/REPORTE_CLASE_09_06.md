# Reporte de Clase: 9 de Junio de 2026

## Objetivos de la Clase
1. **Pruebas de Velocidad:** Incrementar la velocidad base del seguidor de línea de manera controlada y evaluar el comportamiento de la amortiguación del PID.
2. **Detección de Marcadores Verdes (RoboCup):** Diseñar e implementar por software el debounce y la alineación física del robot para la detección de verde.
3. **Automatización del Despliegue:** Facilitar el flujo de carga y ejecución del código en el EV3 desde entornos de Windows.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Optimización de Velocidad (Speed Tuning)
* **Ajuste de velocidad:** Se aumentó la velocidad base `BASE_SPEED` de **`130`** a **`145`** en [main.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/main.py).
* **Comportamiento:** El robot avanza de forma más dinámica en rectas largas. En curvas cerradas, gracias a que la velocidad mínima `MIN_SPEED` se mantuvo en `30` y los coeficientes del PID siguen estables (`KP = 4.5` y `KD = 22.0`), el robot frena con la suficiente fuerza en el momento en que se desvía del centro, asegurando que gire sin salirse.

### B. Preparación del Sistema de Marcadores Verdes (Draft en Código)
Se diseñó e integró por software la lógica de marcadores verdes para evitar falsos positivos y asegurar giros exactos, manteniéndola comentada para validar primero la estabilidad de velocidad en la pista:
* **Filtro de Debounce (Tiempo):** Se introdujo una variable `green_debounce_counter`. Para confirmar un marcador verde, el robot ahora exige leer el mismo color/patrón durante al menos **5 ciclos de bucle consecutivos**. Esto evita que ruidos de iluminación del suelo disparen giros erróneos.
* **Alineación Geométrica (`GREEN_ALIGN_DISTANCE = 20`):** Al detectar un verde, el robot avanza recto `20 mm` adicionales antes de iniciar el giro. Esto asegura que el eje de rotación del robot coincida exactamente con la línea de la intersección y no gire antes de tiempo.
* **Calibración Manual de 8 Pasos:**
  Se modificó la función `calibracion_manual()` para intercalar:
  - **Paso 5:** Verde Izquierdo (mide sensores 0 y 1 del LSA).
  - **Paso 6:** Verde Derecho (mide sensores 6 y 7 del LSA).
  Esto desplaza las T a los pasos 7 y 8. Actualmente está comentado para no interferir en pruebas de velocidad y T puras.

### C. Herramientas de Despliegue y Productividad (Windows)
Se crearon archivos auxiliares para simplificar el ciclo de desarrollo en clase:
* **[deploy.bat](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/deploy.bat):** Script en lote (Batch) de un solo clic que realiza la subida de `main.py` vía SCP y lo ejecuta en el EV3 mediante SSH/brickrun automáticamente.
* **[COMO_CARGAR_CODIGO.md](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/COMO_CARGAR_CODIGO.md):** Documentación detallada que describe los comandos manuales, uso del script batch y resolución de problemas comunes (como matar ejecuciones colgadas que devuelven `Text file busy`).

---

## 2. Estado del Repositorio y Archivos
* **[main.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/main.py):** Lógica ajustada y calibración de 8 pasos redactada (draft de verdes comentado).
* **[deploy.bat](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/deploy.bat):** Herramienta de carga lista para usar.
* **[COMO_CARGAR_CODIGO.md](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/COMO_CARGAR_CODIGO.md):** Guía de ayuda.

---

## 3. Plan para la Próxima Clase
1. Descomentar las secciones de calibración y detección de verde en [main.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-ejemplo/movimiento/seguidor-lineleader-v2/main.py).
2. Probar la rutina de calibración de 8 pasos.
3. Evaluar los giros de 90° (un verde) y 180° (doble verde) en pista física ajustando `GREEN_ALIGN_DISTANCE` si es necesario.
