# Reporte de Clase: 23 de Junio de 2026

## Objetivos de la Clase
El objetivo principal de hoy fue implementar un **Clasificador de Colores por Distancia Euclidiana en HSV** para los sensores de color frontales S2 y S4 en el EV3, con el fin de resolver de manera definitiva los falsos verdes que se producían en los bordes degradados blanco-negro de la pista.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Algoritmo de Distancia Euclidiana Tridimensional en HSV
*   Se reescribió la función `es_color_verde()` para clasificar la lectura actual en base a su distancia geométrica tridimensional a los perfiles calibrados de **Verde**, **Negro** y **Blanco**:
    $$\text{Distancia}^2 = w_h \cdot (\text{diff\_h})^2 + (S - S_{\text{target}})^2 + (V - V_{\text{target}})^2$$
*   **Ajuste del Hue Circular:** Se implementó una función interna `diff_hue()` que calcula la distancia circular en el tono ($0^\circ - 359^\circ$) para evitar distancias gigantescas erróneas en los extremos del espectro.
*   **Estabilización de Baja Saturación:** Si el sensor mide una saturación menor a $22$ (como en el blanco y negro puros), el Hue se vuelve inestable. El código ahora detecta esto, ignora la diferencia de Hue en las distancias de blanco y negro, y penaliza drásticamente al verde sumando $50000$ a su distancia, descartando falsos verdes.
*   **Ponderación del Verde:** Se multiplicó la distancia del verde por un factor de sensibilidad (`sens_verde = 1.3`) para hacer que el algoritmo sea más selectivo y riguroso antes de declarar que ve verde.

### B. Calibración Manual de 7 Pasos
Se extendió el proceso de calibración manual para capturar las lecturas tridimensionales HSV de Blanco y Negro de los sensores frontales:
*   **Paso 1 (Blanco General):** Calibra Blanco en el array LSA y paralelamente registra los valores de Blanco HSV para el sensor izquierdo (S2) y el derecho (S4).
*   **Paso 2 (Negro LSA):** Calibra Negro en el array LSA.
*   **Paso 3 (Negro Color S2/S4):** *Paso nuevo* donde el usuario posiciona los sensores frontales sobre la línea negra para registrar su firma HSV de Negro.
*   **Pasos 4 a 7:** Calibración de máscaras de intersección T y color Verde.

### C. Corrección Visual de Pantalla en el EV3
*   **Problema:** El texto de depuración `"Izq H:{} S:{} V:{}"` superaba los 178 píxeles de ancho de la pantalla del EV3, lo que cortaba la visualización del valor `V`.
*   **Solución:** Se compactó la visualización a `"I: H, S, V"` y `"D: H, S, V"` para Blanco, Negro y Verde, permitiendo visualizar los datos de calibración en tiempo real de forma completa.

---

## 2. Estado de Archivos y Repositorio
*   **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py):** Código modificado con las ecuaciones de distancia Euclidiana ponderada, calibración de 7 pasos, persistencia en JSON corregida, y layout de pantalla acortado.
*   **Valores guardados por defecto en el código:**
    *   **Verde:** Izq `145, 70, 17` | Der `130, 70, 17`
    *   **Blanco:** Izq `192, 30, 81` | Der `173, 21, 71`
    *   **Negro:** Izq `99, 74, 38` | Der `87, 84, 27`

---

## 3. Problemas Detectados y Plan para la Próxima Clase

> [!WARNING]
> **Comportamiento en Pista:** El robot aún no se comporta del todo bien bajo la lógica de distancia Euclidiana implementada. 

### Diagnóstico y tareas para la próxima clase:
1.  **Ajuste del multiplicador de Verde (`sens_verde`):** Si el valor es de `1.3`, puede estar penalizando demasiado el verde verdadero frente a sombras o brillos. Habrá que probar bajándolo a `1.1` o `1.05`, o bien subiéndolo si siguen pasando verdes falsos.
2.  **Calibración en vivo:** Comprobar si las lecturas de los sensores en movimiento difieren mucho de las lecturas estáticas de calibración.
3.  **Filtrado de datos en tiempo (Debounce):** Considerar expandir el debounce (las muestras de confirmación de 5 a 8 ciclos) en caso de que ruidos esporádicos en el piso confundan al algoritmo.
