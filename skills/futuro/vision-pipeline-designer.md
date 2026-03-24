# 👁️ Skill Futuro: Vision Pipeline Designer

> **Estado**: 🔮 FUTURO — Para cuando se incorporen cámaras al taller

## Cuándo activar

- Un alumno conecta una cámara (Raspberry Pi Camera, ESP32-CAM, OpenMV, Pixy2)
- El equipo RoboCup pasa a Soccer Open (pelota naranja, visión)
- Competencia WRO Future Engineers (conducción autónoma con visión)

## Hardware necesario

| Plataforma | Cámara | Procesamiento | Costo aprox |
|------------|--------|---------------|:-----------:|
| Raspberry Pi 4/5 | Pi Camera v2/v3 | OpenCV + Python | USD 60-80 |
| ESP32-CAM | OV2640 integrada | Arduino/MicroPython | USD 8-15 |
| OpenMV Cam H7 | Integrada | MicroPython nativo | USD 65 |
| Pixy2 | Integrada | Firmware propio | USD 60 |

## Prompt (para cuando se active)

```
Sos un ingeniero de visión artificial para robots de competencia.
Ayudame a diseñar el pipeline de visión.

HARDWARE: [cámara y procesador]
OBJETIVO: [qué detectar]
COMPETENCIA: [cuál]
RESTRICCIONES: [FPS mínimo, distancia, iluminación]

Necesito:

1. PIPELINE COMPLETO:
   Captura → Preprocesamiento → Detección → Tracking → Output
   Para cada etapa: algoritmo, parámetros, costo de CPU

2. CALIBRACIÓN DE CÁMARA:
   - Procedimiento para calibrar intrínsecos
   - Corrección de distorsión
   - Mapeo píxel → distancia real

3. DETECCIÓN POR COMPETENCIA:
   a) RCJ Soccer Open: pelota naranja
      - Color HSV filtering vs neural network
      - Falsos positivos: camisetas naranja, reflejos
      - Estimación de distancia por tamaño aparente
   b) WRO Future Engineers: señales de tráfico, obstáculos
      - Color detection para señales rojo/verde
      - Contorno/forma para obstáculos
   c) Rescue Line: víctimas (letras/formas)
      - Template matching o clasificador simple

4. ROBUSTEZ:
   - Manejo de cambios de iluminación
   - Filtro temporal (no reaccionar a un solo frame)
   - Fallback cuando la cámara no detecta nada
   - FPS mínimo aceptable para cada aplicación

5. INTEGRACIÓN CON CONTROL:
   - Cómo pasar datos de visión al loop de control
   - Latencia aceptable
   - Qué hacer cuando visión pierde el objeto
```

## Pipelines por competencia

### RCJ Soccer Open — Pelota naranja
```
Captura (30fps) → Resize (320x240) → HSV Convert 
→ Color Filter (H:5-25, S:100-255, V:100-255)
→ Erode/Dilate → Find Contours → Largest Blob
→ Centro + Radio → Ángulo + Distancia estimada
→ Enviar a controlador principal
```

### WRO Future Engineers — Señales y obstáculos
```
Captura (30fps) → ROI (región de interés)
→ Color segmentation (rojo/verde para señales)
→ Contour analysis → Clasificar señal
→ Decision: girar izq/der
En paralelo: detección de paredes por bordes/LiDAR
```

## Métricas a medir

| Métrica | Objetivo | Cómo medir |
|---------|:--------:|------------|
| FPS real | >20 | Timer en loop |
| Detección rate | >95% | Contar frames con/sin detección |
| Falsos positivos | <2% | Log manual |
| Latencia cámara→acción | <100ms | Timestamp |

## Recursos para cuando llegue el momento

- [OpenCV Python Tutorials](https://docs.opencv.org/4.x/d6/d00/tutorial_py_root.html)
- [OpenMV IDE](https://openmv.io/pages/download) — MicroPython con visión
- [Pixy2 Docs](https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:start)
- [RoboCup Junior ir-golf-ball](https://github.com/robocup-junior/ir-golf-ball)
