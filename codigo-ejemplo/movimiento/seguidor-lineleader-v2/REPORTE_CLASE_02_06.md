# Reporte de Clase: 2 de Junio de 2026

## Objetivos de la Clase
El principal objetivo de hoy fue resolver definitivamente la confusión que el robot presentaba entre **curvas de 90°** e **intersecciones en T** (izquierda y derecha). Para ello, implementamos una propuesta física y algorítmica: añadir un sensor frontal de luz para realizar una lectura predictiva del camino por delante.

---

## 1. Trabajo Realizado y Soluciones Implementadas

### A. Integración de Hardware (Sensor Frontal de Luz)
* **Configuración:** Colocamos un sensor de color de Lego EV3 en el **Puerto 4** (`sensor_frontal = ColorSensor(Port.S4)`).
* **Posicionamiento:** El sensor quedó ubicado al medio y por delante del array de sensores principal (LSA).
* **Modo de Operación:** Se configuró en modo de luz reflejada (`ColorSensor.reflection()`), el cual devuelve un valor porcentual del 0% (negro absoluto) al 100% (blanco puro).

### B. Calibración Manual Expandida (6 Pasos Separados)
Para no mezclar lecturas ni introducir ruido, se separó la calibración manual en 6 pasos independientes en pantalla:
1. **Calibrar Blanco LSA:** Mapeo de la reflexión de luz blanca en el array LSA (0-8).
2. **Calibrar Negro LSA:** Mapeo de la reflexión de la línea negra en el LSA.
3. **Calibrar Blanco Frontal:** Lectura y promediado de 20 muestras sobre el fondo blanco para el nuevo sensor de color.
4. **Calibrar Negro Frontal:** Lectura y promediado sobre la línea negra. Con esto se calcula de forma dinámica:
   $$\text{FRONT\_THRESHOLD} = \frac{\text{FRONT\_WHITE} + \text{FRONT\_BLACK}}{2}$$
5. **Calibrar T Izquierda:** Genera el mapa binario esperado (`T_L_MASK`) posicionando el LSA en la bifurcación izquierda.
6. **Calibrar T Derecha:** Genera el mapa binario esperado (`T_R_MASK`) posicionando el LSA en la bifurcación derecha.

* **Persistencia:** Se expandió `guardar_calibracion` y `cargar_calibracion` para persistir `FRONT_WHITE` y `FRONT_BLACK` en `calibracion.json`, permitiendo saltarse todo el proceso de calibración manual en encendidos posteriores mediante la carga de memoria (botón ARRIBA).

### C. Algoritmo de Detección Activa en Intersecciones (Brake and Evaluate)
Cambiamos la lógica de detección instantánea por un flujo ordenado en la función `follow_line()` cuando coincide la máscara binaria del LSA:
1. **Frenado instantáneo:** El robot detiene la marcha de golpe (`drive.stop()`) al leer la máscara de T.
2. **Primer Sonido (Alerta):** Emite un tono rápido a $500\text{ Hz}$ informando que detectó una T sospechosa.
3. **Pausa de Estabilización:** Espera `100ms` completamente quieto para amortiguar cualquier cabeceo físico del chasis y asegurar lecturas analógicas limpias.
4. **Decisión Física (Frente vs Array):**
   * **SI ES UNA T** (El sensor frontal lee negro, es decir, $\text{reflection} < \text{FRONT\_THRESHOLD}$):
     * Emite pitidos agudos de confirmación (dos a $800\text{ Hz}$ en T Izq o dos a $1000\text{ Hz}$ en T Der).
     * Imprime en pantalla e historial de debug la confirmación.
     * Avanza recto `40 mm` para superar la bifurcación de la T y reanuda el PID de línea.
   * **NO ES UNA T / ES CURVA DE 90°** (El sensor frontal lee blanco, es decir, $\text{reflection} \ge \text{FRONT\_THRESHOLD}$):
     * Emite un pitido grave de descarte a $300\text{ Hz}$ por 200 ms.
     * Continúa el PID normal para tomar la curva cerrada.

### D. Sistema de Cooldown
* **Problema solucionado:** Al reanudar la marcha tras descartar una T, el robot volvía a evaluar la misma curva en la siguiente iteración (que ocurre milisegundos después), lo que generaba un temblequeo de frenados infinitos.
* **Solución:** Añadimos un contador `t_cooldown`. Al descartar (o confirmar) una T, se ajusta a `40` ciclos (~400 ms) durante los cuales el chequeo de intersecciones queda suspendido. Esto da tiempo suficiente para que el robot gire o avance lejos de la línea conflictiva antes de reactivar la lógica de detección.

---

## 2. Resultados de las Pruebas
Las pruebas en pista física fueron exitosas:
* **Curvas cerradas:** El robot frenó brevemente, emitió el pitido grave de descarte y dobló la curva con total fluidez gracias al PID.
* **T-Intersecciones:** Al entrar en la intersección, el robot frenó, sonó la alerta, confirmó inmediatamente con el sensor de adelante (pitido agudo doble), avanzó de frente la distancia de seguridad (`40 mm`) y siguió adelante de manera robusta.

---

## 3. Tareas Pendientes para la Próxima Clase
1. **Reactivación de Marcadores Verdes:** Integrar los verdes de RoboCup utilizando el debounce y el menú de carga por software.
2. **Tuning de Velocidad:** Intentar elevar gradualmente `BASE_SPEED` por encima de 130 si el comportamiento mecánico actual se mantiene estable.
