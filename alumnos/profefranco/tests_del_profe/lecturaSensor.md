# Lectura de Sensor de Color

## Descripción

Programa de diagnóstico que lee continuamente el sensor de color y muestra el valor de reflexión en la pantalla del hub PrimeHub.

## Hardware

- **Hub**: PrimeHub
- **Sensor de color**: Puerto C

## Uso

1. Ejecutar el programa en el robot
2. Colocar el sensor sobre diferentes superficies
3. Observar los valores en pantalla

## Valores de Referencia

| Superficie | Reflexión típica |
|------------|------------------|
| Negro | 5 - 15 |
| Borde de línea | 25 - 45 |
| Blanco | 60 - 80 |

## Notas

- El valor de reflexión se actualiza cada 100ms
- Usar estos valores para calibrar el TARGET del seguidor de línea

---

*Generado por opencode (modelo: minimax-m2.5-free)*