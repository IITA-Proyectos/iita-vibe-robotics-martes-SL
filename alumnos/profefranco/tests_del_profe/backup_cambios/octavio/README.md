# 🤖 Octavio

Mi carpeta de trabajo. Acá guardo mis programas de Pybricks.

## Mis programas

| Archivo | Qué hace | ¿Funciona? |
|---------|----------|:----------:|
| *(agregar programas acá)* | | |

## Notas

*(Anotá lo que vayas descubriendo, tips, errores que resolviste, etc)*
El robot al final de la clase funciono relativamente bien. Aun se deben mejorar los giros, el tambaleo en las rectas etc

## Notas 24/03

Notamos que el robot mejoro desde que le pedimos que haga una medicion de los datos

-----------------------------------------------------------------

Prompt final
Quiero que me ayudes a programar un robot LEGO SPIKE Prime usando Pybricks.

========================
🔧 CONFIGURACIÓN DEL ROBOT
========================
- Entorno: Pybricks
- Hub: PrimeHub
- Motores:
  - Izquierdo → Puerto E → Direction.COUNTERCLOCKWISE
  - Derecho → Puerto F → Direction.CLOCKWISE
- DriveBase:
  - wheel_diameter = 56 mm
  - axle_track = 115 mm
- Ruedas chicas (56x14 mm)
- Superficie: mesa
- Uso de giroscopio (IMU)

========================
🎯 OBJETIVO
========================
El robot debe:
1. Dibujar un cuadrado de 1 metro por lado (1000 mm)
2. Avanzar con velocidad progresiva (aceleración + desaceleración)
3. Mantener dirección recta usando giroscopio
4. NO reiniciar el giroscopio en cada lado (solo al inicio)
5. Girar 90° con alta precisión
6. Evitar zigzag y vibraciones
7. Ser lo más rápido posible sin perder control

========================
📊 TELEMETRÍA (MUY IMPORTANTE)
========================

Quiero medir:

🔹 POR CADA LADO:
- Tiempo total
- Error promedio del giroscopio
- Error máximo

🔹 POR SECTORES DEL LADO (dividir cada lado en 4 partes):
Para cada sector:
- Error promedio
- Error máximo

Ejemplo de salida esperada:

Lado 1:
Tiempo: XXXX ms
Error promedio total: X.X
Error máximo total: X.X

Sector 1: promedio X.X | max X.X
Sector 2: promedio X.X | max X.X
Sector 3: promedio X.X | max X.X
Sector 4: promedio X.X | max X.X

========================
🧠 PROBLEMAS YA DETECTADOS Y SOLUCIONES
========================

1. Robot giraba al iniciar
✔ Solución: velocidad progresiva

2. Robot hacía círculos
✔ Solución: invertir motores correctamente

3. Se pasaba de 90°
✔ Solución: ajustar axle_track

4. Zigzag en esquinas
✔ Solución:
- no frenar a 0
- transición suave al giro

5. Frenado raro a alta velocidad
✔ Causa:
- corrección del giroscopio
✔ Solución:
- reducir corrección
- frenar más tarde

6. Giros imprecisos
✔ Solución:
- giro progresivo (rápido → lento)
- tolerancia final baja

7. Error acumulado del giroscopio
✔ Solución:
- usar variable “objetivo”
- NO resetear cada lado

========================
⚠️ PROBLEMAS ACTUALES A MEJORAR
========================

- Giros todavía no perfectos
- A alta velocidad pierde precisión
- Corrección puede ser agresiva
- Frenado final mejorable
- Falta análisis por sectores

========================
🔧 REQUISITOS TÉCNICOS
========================

- Usar Pybricks
- Usar DriveBase
- Usar StopWatch
- Crear función:
  normalizar_error(-180 a 180)
- Usar variable global “objetivo”
- Implementar:
  ✔ velocidad progresiva
  ✔ corrección con giroscopio
  ✔ giro progresivo preciso
  ✔ telemetría completa
  ✔ análisis por sectores

========================
🚀 NECESITO QUE HAGAS
========================

1. Generar el programa COMPLETO
2. Incluir medición por sectores
3. Optimizar:
   - velocidad
   - precisión
   - giros
4. Mantener código limpio y sin duplicaciones
5. Listo para copiar y pegar

========================
💡 OPCIONAL (SI SABÉS HACERLO)
========================

- Mejorar precisión del giro aún más
- Ajuste automático de parámetros
- Optimización basada en datos

========================
🎯 PRIORIDAD
========================

1. Precisión de giros
2. Estabilidad en recta
3. Velocidad alta
4. Datos útiles

-----------------------------------------------------------------
