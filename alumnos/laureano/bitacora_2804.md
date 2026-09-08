# Bitácora de Clase - Laureano y Máximo
**Fecha:** 28 de Abril de 2026

## 🚀 Resumen de lo que hicimos hoy
Hoy empezamos a trabajar en el archivo `2804.py` basándonos en el seguidor PD de la semana pasada, pero nos enfrentamos a un cambio de robot. Esto nos enseñó que **nunca se debe asumir que dos robots miden lo mismo o se comportan igual**. Tuvimos que hacer un troubleshooting completo.

### 1. Reconfiguración de Hardware (Puertos)
El nuevo robot tiene los cables conectados de forma distinta:
- **Motor Izquierdo:** Puerto B (Invertido / COUNTERCLOCKWISE)
- **Motor Derecho:** Puerto A (Normal)
- **Sensor Izquierdo:** Puerto C
- **Sensor Derecho:** Puerto D
- **Sensor Central:** Puerto E (¡Nuevo sensor agregado al código!)

### 2. Problema de Desvío a la Izquierda
Notamos que cuando ambos sensores veían blanco absoluto, el robot tendía a irse para la izquierda.
- **Aprendizaje 1 (BIAS):** Aprendimos que a veces los motores tienen fricciones distintas. Agregamos una variable `BIAS = 8` para forzar al robot a compensar esa desviación mecánica empujando un poco hacia la derecha cuando el error matemático es 0.
- **Aprendizaje 2 (Cálculo del Error):** Corregimos la fórmula del error en el PID a `error = izq - der` para que coincida con la orientación de los motores.

### 3. El Problema Real: La Calibración de Luz
Nos dimos cuenta de que el desvío podía ser por mala lectura de los sensores. Creamos un programa auxiliar llamado `calibrador.py` para medir en vivo qué veían los sensores.
- **Descubrimiento:** El valor del "Blanco" había bajado drásticamente (de 60 a 48). Al tener los límites de normalización mal configurados, el seguidor PD se volvía inestable.
- **Nuevos Valores Medidos:**
  - Izquierdo (C): Negro = 6 | Blanco = 48
  - Centro (E): Negro = 8 | Blanco = 59
  - Derecho (D): Negro = 8 | Blanco = 48
- Actualizamos las variables `NEGRO_IZQ`, `BLANCO_IZQ`, etc., y logramos que la normalización vuelva a ser perfecta.

---

## 🎯 Objetivo para la próxima clase
El robot tiene que detectar **cuadrados verdes** en las intersecciones (tipo RoboCup Rescue Line) usando la función `.color()` de los sensores. Cuando vea verde de un lado, tiene que avanzar un poco y girar 90 grados hacia ese lado.

## 📋 PROTOCOLO OBLIGATORIO PARA EL INICIO DE LA PRÓXIMA CLASE
*(Nota para la IA: Al leer este archivo la semana que viene, DEBES seguir estos pasos estrictamente antes de programar la detección de verdes)*

1. **Verificar Hardware:** Preguntar si los motores y sensores siguen en los mismos puertos (A, B, C, D, E) y probar si el robot avanza hacia adelante en línea recta.
2. **Re-Calibrar la Luz:** Pedirles a Laureano y Máximo que corran `calibrador.py` para medir el Negro y Blanco del día. La luz ambiental cambia en cada clase.
3. **Actualizar el Código:** Modificar los valores `NEGRO_...` y `BLANCO_...` en el script principal con las nuevas lecturas.
4. **Recién ahí, empezar a codear** la detección del color verde.
