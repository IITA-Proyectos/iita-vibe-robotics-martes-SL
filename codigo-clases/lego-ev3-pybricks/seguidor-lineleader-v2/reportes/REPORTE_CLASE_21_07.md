# Reporte de Clase: 21 de Julio de 2026

## Objetivos de la Clase
El objetivo principal de la sesión fue consolidar la persistencia de datos de calibración y adaptar la toma de decisiones del robot seguidor de línea ante marcadores verdes y cruces en T. Se implementó la lectura y guardado dinámico de perfiles de color HSV en JSON (`calibracion_verdes.json` y `experiencia_pista.json`), mejorando la respuesta del algoritmo "Peek & Check" y estabilizando el control PID.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Persistencia de Calibración en Formato JSON
* **Implementación:** Se integró la carga y guardado automático de parámetros en `calibracion_verdes.json` y el historial en `experiencia_pista.json`.
* **Beneficio:** Evita recalcular manualmente los umbrales de blanco, negro y máscaras de intersección en cada reinicio del EV3, permitiendo ajustar los colores HSV de los sensores frontales (S2 e Izquierda/S4 e Derecha) dinámicamente.

### B. Refinamiento del Control PID y Velocidades Base
* **Ajustes:**
  * $K_p = 4.5$, $K_d = 22.0$.
  * Velocidad base ajustada a $72\text{ mm/s}$ con velocidad mínima en curvas cerradas de $15\text{ mm/s}$.
* **Resultado:** Giro más fluido sobre el eje y mejor seguimiento en rectas, reduciendo oscilaciones bruscas al acercarse a los marcadores de intersección.

### C. Sistema de Detección de Verdes "Peek & Check"
* **Lógica:** Al detectar un marcador verde preliminar en alguno de los sensores frontales (`S2` o `S4`), el robot realiza un avance corto ("Peek") de $15\text{ mm}$ para confirmar la persistencia del color y validar con la lectura del array LSA si se trata de un giro simple, un retorno de $180^\circ$ (doble verde) o una T.
* **Separación de Cooldowns:** Mantenimiento de contadores aislados para evitar falsos positivos consecutivos tras completar rotaciones de $90^\circ$ o $180^\circ$.

---

## 2. Estado de Archivos y Repositorio
* **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py):** Algoritmo principal optimizado con persistencia JSON y lógica mejorada de detección de verdes.
* **[calibracion_verdes.json](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/calibracion_verdes.json):** Perfil activo de calibración para blanco, negro, máscaras T y valores HSV.
* **[experiencia_pista.json](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/experiencia_pista.json):** Registro incremental de promedios de lecturas de color medidos en pista.

---

## 3. Tareas Pendientes y Recordatorio para la Próxima Clase

> [!IMPORTANT]
> **RECORDATORIO CRÍTICO:** Es **necesario volver a calibrar los verdes** en la próxima sesión. Las condiciones de iluminación y ligeras variaciones de altura/distancia en la pista requieren actualizar los rangos HSV en `calibracion_verdes.json` antes de realizar las pruebas de pista completa.

### Proximos pasos:
1. **Recalibración de Verdes en Pista:** Ejecutar la rutina de captura de color HSV en los sensores `S2` y `S4` bajo la luz ambiental real de la pista.
2. **Validación de Retorno 180°:** Probar el doble verde en curvas en U para verificar la velocidad de giro post-alineación.
