# Reporte de Clase: 30 de Junio de 2026

## Objetivos de la Clase
El objetivo de la clase de hoy fue resolver el desalineamiento físico de los giros verdes y solucionar el problema de secuencia de lectura en intersecciones donde el verde se encuentra posicionado **después** del cruce (lo que hacía que el robot doblara erróneamente en caminos donde debía seguir derecho).

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Corrección Geométrica de la Alineación (88 mm)
* Se actualizó la constante `GREEN_ALIGN_DISTANCE` en [seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py) a **`88`**.
* Esta distancia se dedujo a partir del análisis exacto de las dimensiones físicas del circuito y la ubicación de los sensores:
  * **$46\text{ mm}$:** Distancia física real de los sensores de color al eje de las ruedas.
  * **$+ 25\text{ mm}$:** Longitud del cuadrito verde oficial en la pista.
  * **$+ 17\text{ mm}$:** Ancho de la línea negra transversal sobre la que gira el robot.
  * **Total = $88\text{ mm}$** desde la primera detección del verde hasta que el eje de las ruedas queda alineado con la línea transversal de giro.

### B. Algoritmo de Inspección Corta ("Peek & Check") para Priorizar Intersecciones
* **El Problema:** En el caso de intersecciones que tienen un cuadrito verde después del cruce (por ejemplo, para los robots que vienen del sentido contrario), los sensores frontales (a $46\text{ mm}$) leían el verde antes de que el LSA (a $25\text{ mm}$) pudiera pisar y registrar la T o el Doble Negro (Cruz). El robot frenaba inmediatamente y giraba de manera incorrecta.
* **La Solución:** Implementamos un avance de inspección controlado usando una constante de prueba `PEEK_GREEN_DIST = 15` (15 mm).
  * Al detectar verde, el robot frena e inicializa una pausa de **$400\text{ ms}$** para control visual.
  * Luego, avanza una distancia muy corta de **$15\text{ mm}$** y vuelve a frenar por otros **$400\text{ ms}$**.
  * En esta segunda parada, el LSA ya se encuentra sobre la línea de cruce. El software comprueba si el LSA registra una intersección (T o Cruz).
  * **Si se detecta intersección:** El verde se descarta (`t_cooldown = 40`), el robot ignora el giro y continúa de largo para cruzar la intersección.
  * **Si NO se detecta intersección:** Se confirma el verde mediante las 5 muestras HSV habituales, se avanza la distancia restante ($88 - 15 = 73\text{ mm}$) y se ejecuta el giro de $90^\circ$ o $180^\circ$.

---

## 2. Estado de Archivos y Repositorio
* **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py):** Código actualizado con la constante `GREEN_ALIGN_DISTANCE = 88`, `PEEK_GREEN_DIST = 15`, lógica de pausas visuales de inspección y filtros cruzados LSA/Color.

---

## 3. Resultados Obtenidos
* El robot ahora discrimina correctamente cuándo el verde se encuentra antes del cruce (efectuando el giro alineado en el centro del eje) y cuándo el verde está después (ignorándolo y continuando de largo para respetar el sentido de la pista).
* Las pausas visuales de $400\text{ ms}$ ayudaron a los alumnos y docentes a depurar físicamente el movimiento sobre la pista en tiempo real.
