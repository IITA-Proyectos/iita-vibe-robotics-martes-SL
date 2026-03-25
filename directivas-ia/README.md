# 🧠 Directivas para IAs

Prompts y directivas que le damos a las IAs para que escriban buen código.

| Archivo | Para qué |
|---------|----------|
| `system-prompts/pybricks-expert.md` | Prompt experto para Pybricks + Spike |
| `tips.md` | Consejos generales para vibe coding |

## ¿Cómo contribuir?

Si encontrás un prompt que funciona bien, guardalo acá con:
- Nombre descriptivo
- Para qué IA es (Claude, ChatGPT, etc)
- Ejemplo de resultado

------------------------------------------------

## Prompts Juanse-Laureano 24/03 
→ Con ChatGPT - Lo que mas ayudo fue implementar la medicion de datos 

Estoy trabajando con un robot LEGO SPIKE Prime usando Pybricks (DriveBase con giroscopio).

Quiero que tengas en cuenta TODOS estos detalles técnicos y ajustes ya probados antes de sugerir cambios:

CONFIGURACIÓN DEL ROBOT:

* Motores:

  * Izquierdo: Port A (COUNTERCLOCKWISE)
  * Derecho: Port B
* Ruedas: diámetro 56 mm
* Distancia entre ruedas: 90 mm
* Uso DriveBase con IMU (giroscopio)
* Se usa hub.imu.heading() como referencia absoluta
* Se resetea el heading al inicio

OBJETIVO:

* Movimiento en cuadrados (rectas de 1000 mm + giros de 90°)
* Alta precisión + consistencia (nivel competencia tipo FLL)
* Minimizar desviación angular y tiempo

PROBLEMAS YA DETECTADOS Y SOLUCIONADOS:

1. ❌ Desviación al empezar la recta
   ✔ Solución:

* Implementar rampa de aceleración real
* Velocidad inicial muy baja (~100)
* Rampa larga (~400 mm)

2. ❌ Corrección demasiado brusca
   ✔ Solución:

* Limitar corrección (±100)
* Ajustar Kp dinámico:

  * Inicio: Kp alto (~3.0)
  * Resto: Kp más bajo (~1.6)

3. ❌ Acumulación de error tras giros
   ✔ Solución:

* Usar SIEMPRE el ángulo real del giroscopio después del giro
* No usar ángulos teóricos acumulados

4. ❌ Giros muy lentos y con oscilación
   ✔ Solución:

* Control en 2 zonas:

  * Lejos (>20°): giro rápido (error * 5)
  * Cerca: velocidad constante (±120)
* Tolerancia de salida: ±3°
* Evitar corrección fina (elimina tembleque)

5. ❌ Robot “se traba” al final del giro
   ✔ Solución:

* NO buscar precisión extrema (±1°)
* Salida anticipada (±3°)
* Micro pausa (50 ms)

6. ❌ Desviación constante hacia un lado (bias mecánico)
   ✔ Diagnóstico:

* El robot se desvía consistentemente hacia la izquierda
  ✔ Solución:
* Agregar compensación fija:
  correccion += BIAS
* BIAS inicial recomendado: 8 (ajustable entre 5–10)

7. ❌ Rectas lentas por sobrecorrección
   ✔ Solución:

* Limitar corrección máxima
* Mejorar Kp inicial
* Compensar bias correctamente

SISTEMA DE MEDICIÓN IMPLEMENTADO:

* Tiempo por recta y giro (ms)
* Desvío máximo (grados)
* Desvío promedio (grados)
* División de recta en 3 zonas:

  * Inicio (0–300 mm)
  * Medio (300–700 mm)
  * Final (700–1000 mm)
* Medición de dirección:

  * Positivo → derecha
  * Negativo → izquierda

INTERPRETACIÓN DE DATOS:

* Desvío máximo:

  * > 5° → problema fuerte
  * 2–5° → aceptable
  * <2° → ideal

* Desvío por zonas:

  * Inicio alto → problema de arranque
  * Medio crece → corrección débil
  * Cambia de signo → sobrecorrección

* Si todos los valores tienen mismo signo → bias mecánico

REQUISITOS PARA RESPUESTAS:

* SIEMPRE devolver el programa COMPLETO (no fragmentos)
* Mantener estructura con:

  * avanzar_recto()
  * girar_a()
  * error_angulo()
* No simplificar el control
* Explicar SOLO cambios relevantes
* Priorizar estabilidad sobre perfección absoluta

OBJETIVO FINAL:

* Desvío máximo < 3°
* Desvío promedio < 1.5°
* Tiempo de recta ~2300 ms
* Giros consistentes (~800–1200 ms)
* Movimiento repetible y confiable

Si propones mejoras:

* Deben basarse en estos datos reales
* No reiniciar desde cero
* Ajustar sobre este sistema ya optimizado

------------------------------------------------

