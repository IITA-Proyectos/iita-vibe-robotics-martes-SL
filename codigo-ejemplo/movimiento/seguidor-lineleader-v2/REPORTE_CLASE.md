# Reporte de Clase: Seguidor de Línea Avanzado
**Fecha:** 12 de Mayo
**Proyecto:** Seguidor de Línea con Sensor Array

## 1. Problemas Encontrados y Soluciones

### A. Error I2C (OSError: [Errno 5] EIO)
*   **Problema:** Al intentar usar el `LineLeader v2` en el puerto 3, el comando `ll.calibrate_white()` causaba un cuelgue total (`EIO`). Esto ocurría porque el comando de escritura interno de hardware fallaba o la versión del firmware del sensor lo rechazaba.
*   **Solución:** Descartamos la calibración por hardware y programamos una **calibración puramente por software**. Leímos los datos crudos (0-255), sacamos el promedio tomando 20 muestras, y usamos una función matemática (`normalize_array`) para pasarlos a la escala ideal de 0 a 100.

### B. Confusión de Sensor (LSA vs LineLeader)
*   **Problema:** Al probar el otro sensor en el **Puerto 1**, nos dimos cuenta de que no era el LineLeader, sino el **LightSensorArray (LSA)**. Esto causaba más errores `EIO` porque las direcciones de memoria son diferentes.
*   **Solución:** Creamos un script **Escáner Inteligente I2C (`test_i2c.py`)** que interrogó físicamente los puertos. 
    *   Descubrimos que la dirección 7-bit del LSA es **`0x0A`** (y no `0x14` que es su formato de 8-bits).
    *   Descubrimos que sus datos calibrados/crudos se alojan en el registro **`0x42`** (y no en el `0x49`).
    *   Actualizamos `main.py` con estos valores mágicos y el sensor empezó a leer de maravilla.

### C. Polaridad de Giro Invertida
*   **Problema:** El robot detectaba la línea a la izquierda, pero el PID mandaba una corrección hacia la derecha (haciendo que el robot se escape de la pista).
*   **Solución:** En lugar de recablear los motores físicamente, invertimos la matemática del error: cambiamos `error = pos - CENTER` por `error = CENTER - pos`.

### D. Dinámica del Sensor en el Eje de las Ruedas
*   **Problema:** Al usar la nueva marcha adelante (marcha atrás invertida), el sensor quedó **casi pegado al eje de rotación de las ruedas**. En esa posición física, el sensor no tiene efecto de "palanca" cuando el robot gira sobre sí mismo.
*   **Solución:** 
    1. Tuvimos que subir la fuerza de corrección (KP de 1.6 a **2.0** y KD a **18.0**).
    2. Mantuvimos la velocidad mínima en curvas alta (`MIN_SPEED = 120`), porque si el robot frena a velocidad casi nula en la curva, el sensor no cruza la línea y se pierde.

### E. Recovery Mode (Pérdida de Línea)
*   **Problema:** En curvas muy cerradas, la velocidad sacaba al robot de la línea (`pos is None`).
*   **Solución:** Creamos una "Memoria de Retroceso". Si el robot se sale, pone marcha atrás (`-100 mm/s`) pero **conservando el último ángulo de giro** del PID. Literalmente deshace la misma curva por la que derrapó hasta que sus sensores vuelven a ver la línea.

---

## 2. Tareas Pendientes (Bugs a arreglar la próxima clase)

**Bug del "LISTO" (Parada repentina):**
*   **Síntoma:** El robot va andando, de la nada frena por completo, dice "LISTO" en la pantalla y termina el programa.
*   **Causa:** Tenemos un bloque de código para detectar intersecciones: `if count_on_line(cal) >= 7: break`. Cuando el sensor tira una lectura sucia, sombra, o detecta mucho negro de golpe al perderse en una curva fea, cree que es una intersección de la pista, rompe el `while True` de seguimiento y termina el programa prematuramente.
*   **Cómo arreglarlo (Próxima clase):** Hay que mejorar ese `if`. Podríamos pedir que detecte la intersección durante *varios ciclos seguidos* antes de frenar, o simplemente comentar ese bloque de intersección hasta que la física de la pista lo amerite.

## 3. Resumen de Archivos
*   `main.py`: Código final del seguidor de línea calibrado para el LSA.
*   `test_i2c.py`: Herramienta de escáner súper útil para diagnosticar cualquier sensor Mindsensors conectado a los puertos.
