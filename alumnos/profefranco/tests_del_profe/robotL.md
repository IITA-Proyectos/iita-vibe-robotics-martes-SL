# Robot L - Forma de L

## Descripción

Este programa hace que el robot LEGO Spike con Pybricks trace una forma de **"L"**:
- **Primer lado**: 30 cm (300 mm)
- **Giro**: 90° a la derecha
- **Segundo lado**: 80 cm (800 mm)

## Cómo funciona

El código utiliza el sensor IMU del hub para corregir la trayectoria del robot mientras avanza recto:

1. **Control de trayectoria**: Usa un control proporcional (Kp) que ajusta el ángulo de giro según el error entre el ángulo objetivo y el actual.

2. **Compensación de BIAS**: Agrega una corrección de 8° para compensar la deriva natural de los motores.

3. **Rampa de velocidad**: Acelera gradualmente en los primeros 400mm para evitar saltos.

4. **Giros precisos**: La función `girar_a()` usa control PID para girar exactamente 90°.

## Métricas

Al finalizar cada tramo, el programa imprime:
- Tiempo total del tramo
- Desvío máximo en grados
- Desvío promedio en grados
- Desvío por zona (inicio, medio, final)

## Hardware

- **Hub**: PrimeHub
- **Motores**: Puerto A (izquierdo) y Puerto B (derecho)
- **Ruedas**: 56 mm de diámetro
- **Distancia entre ejes**: 90 mm

---

*Generado por opencode (modelo: minimax-m2.5-free)*