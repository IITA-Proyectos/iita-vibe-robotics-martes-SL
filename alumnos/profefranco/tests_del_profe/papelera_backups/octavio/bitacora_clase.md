# Bitácora de Desarrollo: Seguidor de Línea (RoboCup)
**Fecha:** 14 de Abril | **Estado:** En calibración avanzada

En esta sesión dimos pasos agigantados pasando de un robot estático a un algoritmo de movimiento avanzado con lógica de control reactivo. A continuación, el resumen de nuestro proceso, los errores que el robot nos presentó y cómo evolucionamos el código para atacarlos.

---

## 🛠️ Evolución del Código y Superación de Errores

### 1. El Nacimiento del Seguidor Proporcional (P)
* **El Objetivo:** Usar el sensor izquierdo (B) y derecho (F) para seguir el centro de una línea.
* **El Problema:** Al probar velocidades altas (`150 mm/s`), el robot no lograba reaccionar a tiempo y la corrección matemática no era suficientemente agresiva, haciendo que se perdiera en curvas suaves.
* **La Solución:** Bajamos la velocidad base y comenzamos a jugar con el multiplicador Proporcional (`KP`), subiéndolo para forzar giros más pronunciados simulando un "volantazo".

### 2. El Efecto Diente de Sierra (Overshoot)
* **El Problema:** En una curva muy extrema (diente de sierra), el robot tomaba la primera curva con mucha fuerza por tener un `KP` alto. Esa fuerza generaba inercia angular, y en la contracurva inmediata el robot "se pasaba de largo" (Overshoot) sin poder regresar a tiempo al centro.
* **La Solución:** Convertimos nuestro Seguidor P en un **PD**. Implementamos la "Derivada" (`KD`) como un freno. Al restarle el error actual al error previo, el robot aprendió a "predecir" si se estaba acercando bruscamente de vuelta a la línea, frenando el giro antes de que se pase de largo hacia el otro lado.

### 3. El Síndrome del Giro Asimétrico
* **El Problema:** Descubrimos que el robot giraba mucho mejor para un lado que para el otro.
* **La Causa (Análisis de Datos):** La telemetría nos delató. Sobre el blanco puro, el sensor izquierdo leía `58` pero el derecho leía `48`. La matemática percibía estas luces como "errores" asimétricos, de forma que sentía el doble de impulso para corregir hacia la derecha.
* **La Solución:** Agregamos una **Normalización Mín-Máx**. Usando los valores de calibración recolectados al inicio (`~10` para negro, `~50/60` para blanco), forzamos al código a "mapear" el porcentaje de 0 a 100 independientemente del lente físico. Esto garantizó una simetría perfecta en las restas. 

### 4. La Recreación del Rescate Físico (Line Recovery)
* **El Problema:** Aún con todo, si entraba demasiado veloz, llegaba a escaparse de la pista. Un robot de rescate no puede rendirse.
* **La Solución Inicial:** Diseñamos un `StopWatch` que al leer que los 3 sensores están en blanco le permite avanzar a ciegas por 500ms (para superar los Gaps permitidos por reglamento). Si el vacío persistía, activaba `robot.drive(-50, 0)` para retroceder hasta encontrar la línea central.
* **El Error Crítico:** Al retroceder, cruzaba la línea pero su fuerte inercia física de marcha atrás lo patinaba sobre la pista blanca nuevamente, entrando en un bucle tonto. 
* **El Ajuste Maestro:** Le agregamos un `robot.stop()` instantáneo al encontrar el negro, obligándolo a matar el derrape físico durante 200ms antes de transferirle el control al PID para avanzar hacia adelante.

---

## ✅ Objetivos Cumplidos (El Plan)

> [!TIP]
> **Base Física Resuelta:** Inicialización en Pybricks exitosa. La inversión de puertos de motor está funcionando perfectamente, se calculó la base motora con el axle_track de `155mm` y tenemos telemetría real.

> [!NOTE]
> **Algoritmo PD Dinámico:** La estructura algorítmica PID asimétrica funciona, está limpiamente escrita e incorpora la recuperación (la mejor táctica para la Rescue Line).

---

## 🏗️ Lo que Falta (Nuestros "To-Dos" Futuros)

> [!WARNING]
> Aún falta lograr el **Equilibrio Dorado del PID**

1. **Ajuste Fino de KP y KD:** Modificaste el `KP` manual a 2.2 y acortaste los tiempos de ráfaga del ciclo en la prueba final (`wait(5)`). El robot "rebota" menos pero sigue teniendo problemas en las contracurvas rápidas para acoplarse con suavidad tras encontrar la línea en marcha atrás. Necesitamos seguir testeando esa reacción.
2. **Uso Completo del Sensor Central:** Hasta ahora lo usamos únicamente como seguro de vida (saber cuándo retroceder). El próximo paso será que el sensor detecte "Marcas de Intersección" (las cuadrículas verdes o doble intersección).
3. **Manejo Dinámico de Velocidad:** Reducir la velocidad `VELOCIDAD` *calculadamente* basándonos en qué tan alto es el nivel de error, haciendo que frene solo en las curvas y acelere solo en las rectas.
