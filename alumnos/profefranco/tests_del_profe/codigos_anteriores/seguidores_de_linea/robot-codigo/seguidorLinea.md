# Seguidor de Línea - Pybricks

## Descripción

Programa para robot LEGO Spike con Pybricks que sigue una línea negra usando control PID.

## Hardware

- **Hub**: PrimeHub
- **Motores**: Puerto A (izquierdo), Puerto B (derecho)
- **Sensor de color**: Puerto C
- **Ruedas**: 56 mm de diámetro
- **Distancia entre ejes**: 90 mm

## Configuración

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| TARGET | 35 | Reflexión del borde de línea (calibrar) |
| KP | 1.8 | Ganancia proporcional |
| KI | 0.03 | Ganancia integral |
| KD | 12.0 | Ganancia derivativa |
| VELOCIDAD | 200 | mm/s |

## Calibración

1. Colocar el sensor sobre el borde de la línea negra
2. Ajustar TARGET al valor de reflexión observado
3. Ajustar KP, KI, KD según necesidad

## Cómo funciona

1. El sensor de color mide la reflexión de la superficie
2. El PID calcula la corrección basada en el error (reflexión - TARGET)
3. El robot ajusta su trayectoria para mantenerse en el borde
4. El led verde indica funcionamiento normal

---

*Generado por opencode (modelo: minimax-m2.5-free)*