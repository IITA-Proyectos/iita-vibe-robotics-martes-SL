# Comparación de Modelos de Seguidores de Línea

Este documento compara los diferentes modelos de seguidores de línea implementados por los alumnos en la carpeta `profefranco`.

## Resumen General

Todos los archivos implementan seguidores de línea para robots LEGO Spike Prime utilizando dos sensores de color y dos motores. Sin embargo, difieren en su enfoque de control, calibración y complejidad.

---

## 1. test-bigpickle.py (Modelo: opencode/big-pickle)

### Características principales:
- **Control PID completo** (Proporcional, Integral, Derivativo)
- **Calibración automática** al inicio: mide valores sobre línea y fondo para calcular targets
- **Targets separados** para cada sensor (TARGET_LEFT y TARGET_RIGHT)
- **Límite de corrección** (-150 a 150) para evitar giros bruscos
- **Velocidad base**: 200 mm/s
- **Ganancias PID**: KP=1.5, KI=0.0, KD=10.0

### Ventajas:
- Calibración automática que se adapta a diferentes condiciones de iluminación
- Control preciso gracias al término derivativo alto (KD=10.0)
- Sistema robusto con límites de corrección

### Desventajas:
- Más complejo de entender debido a la calibración y múltiples parámetros
- El término integral está desactivado (KI=0.0)

---

## 2. test-nanogpt5.py (Modelo: nanogpt5)

### Características principales:
- **Control PID** con todos los términos activos
- **Error calculado** como diferencia directa entre sensores (ref_izq - ref_der)
- **Funciones auxiliares** bien definidas (clamp, calcular_error_pid)
- **Velocidad base**: 180 mm/s
- **Ganancias PID**: KP=1.8, KI=0.03, KD=10.0

### Ventajas:
- Código bien organizado con funciones claras y documentación
- Uso efectivo de todos los términos PID
- Límite en el término integral para prevenir acumulación excesiva

### Desventajas:
- No incluye calibración automática (usa TARGET fijo = 45)
- Asume que el valor óptimo es el mismo para ambos sensores

---

## 3. test-minimax.py (Modelo: minimax-m2.5-free)

### Características principales:
- **Enfoque basado en umbrales** (no PID)
- **Calibración manual** requerida (usuario coloca sensores sobre línea y fondo)
- **Máquina de estados** con 4 condiciones:
  - 0: Ambos sensores en línea (intersección)
  - 1: Izquierdo en línea, derecho en fondo
  - 2: Izquierdo en fondo, derecho en línea
  - 3: Ambos en fondo (fuera de línea)
- **Control proporcional simple** basado en estado
- **Velocidad base**: 300 grados/second

### Ventajas:
- Muy intuitivo y fácil de entender
- Robusto frente a ruido gracias al enfoque de umbrales
- Fácil de depurar y modificar el comportamiento por estado

### Desventajas:
- Menos preciso que el control PID continuo
- Requiere calibración manual cada vez
- Puede ser brusco en las transiciones entre estados

---

## 4. test-qwen.py (Modelo: qwen3.6-plus-free)

### Características principales:
- **Control PID completo** (similar a nanogpt5)
- **Excelente documentación** con explicaciones detalladas de cada componente
- **Error calculado** como diferencia entre sensores (ref_izq - ref_der)
- **Velocidad base**: 180 mm/s
- **Ganancias PID**: KP=1.8, KI=0.03, KD=10.0
- **Uso de giroscopio** para estabilizar movimientos

### Ventajas:
- Mejor documentado de todos los archivos
- Explicaciones claras de qué hace cada término del PID
- Buenas prácticas de código (funciones auxiliares, comentarios detallados)
- Incluye estabilización giroscópica

### Desventajas:
- Igual que nanogpt5: no tiene calibración automática
- Practicamente idéntico a nanogpt5 en funcionalidad, solo difiere en documentación

---

## 5. test-nemotron.py (Modelo: nemotron-3-super-free)

### Características principales:
- **Control proporcional simple** (solo término P, sin I ni D)
- **Error calculado** como diferencia entre sensores
- **Corrección** = GANANCIA * error
- **Velocidad base**: 200 mm/s
- **Ganancia**: GANANCIA=2.0
- **Umbral**: 30 (pero no se usa realmente en el algoritmo)
- **Puertos diferentes**: Motores en E/F, sensores en A/B

### Ventajas:
- Más simple de entender e implementar
- Menos parámetros para ajustar
- Código muy conciso y directo

### Desventajas:
- Menos preciso que los controladores PID completos
- Puede oscilar o tener dificultades para mantenerse centrado en la línea
- No compensa errores acumulados ni predice cambios futuros

---

## Comparación Técnica

| Característica | BigPickle | NanoGPT5 | Minimax | Qwen | Nemotron |
|----------------|-----------|----------|---------|------|----------|
| Tipo de Control | PID Completo | PID Completo | Umbral/Estados | PID Completo | Proporcional (P) |
| Calibración | Automática | Fija | Manual | Fija | Fija |
| Targets Separados | Sí | No | N/A | No | No |
| Uso de Giroscopio | No | No | No | Sí | No |
| Límites de Corrección | Sí (±150) | Sí (clamp) | N/A | Sí (clamp) | No |
| Complejidad del Código | Alta | Media | Media | Alta | Baja |
| Documentación | Básica | Buena | Excelente | Excelente | Básica |

---

## Recomendaciones

1. **Para máxima precisión**: Usar BigPickle o Qwen (ambos tienen PID completo con calibración o buena documentación)
2. **Para facilidad de uso**: Minimax es el más intuitivo y fácil de depurar
3. **Para aprendizaje**: NanoGPT5 o Qwen ofrecen buen equilibrio entre complejidad y documentación
4. **Para simplicidad extrema**: Nemotron es el más simple pero menos preciso

## Conclusiones

Los modelos BigPickle y Qwen representan las implementaciones más sofisticadas con control PID completo. BigPickle tiene la ventaja de la calibración automática, mientras que Qwen destaca por su excelente documentación.

Minimax ofrece un enfoque diferente basado en estados que puede ser más robusto en ciertos escenarios, aunque menos preciso que el PID continuo.

NanoGPT5 y Qwen son funcionalmente muy similares, con Qwen teniendo mejor documentación.

Nemotron es el más simple pero sacrifica precisión por simplicidad.