# 🛟 Seguidor RCJ Rescue Line — guía de uso

Programa: [`rcj_rescue_line_seguidor.py`](rcj_rescue_line_seguidor.py)

Seguidor de línea **competitivo** para Spike Prime + Pybricks pensado para
RoboCup Junior Rescue Line. Cubre line following PID con dos sensores,
detección de marcadores verdes, recuperación de gaps, esquive de obstáculos,
rampas y detección de la cinta plateada que marca la entrada de la zona de
evacuación.

## ¿Qué cubre?

| Elemento del campo | Detección | Manejo |
|---|---|---|
| Línea negra sobre blanco | `ColorSensor.reflection()` normalizado | PD line following con dos sensores |
| Marcador verde (intersección) | `sensor.hsv()` con rango HSV | Giro 90°/180° según cuál sensor lo ve |
| Gap (línea cortada) | Ambos sensores ven blanco | Avance recto con gyro hasta encontrar línea, sweep en abanico si falla |
| Obstáculo físico | `UltrasonicSensor.distance() < 80 mm` | Esquive rectangular y vuelta a la línea |
| Rampa | `hub.imu.tilt()` > 8° | Modo lento con Kp aumentado |
| Cinta plateada (evacuación) | HSV: luminosidad alta, saturación baja | Parar y avisar (fin de pista) |

## ¿Qué NO cubre?

- **Búsqueda y rescate de víctimas dentro de la zona de evacuación.** Eso es
  un programa entero aparte (búsqueda en espiral, clasificación plateada vs
  negra, depósito en el triángulo). Cuando este programa detecta la cinta
  plateada, termina y le pasa el control implícitamente al siguiente programa.
- **Speed bumps**. A velocidades moderadas el robot pasa sin lógica especial.
  Si tu robot tiene problemas, bajá `BASE_SPEED` o agregá detección por
  acelerómetro vertical.
- **Detección de qué lado conviene esquivar el obstáculo.** El programa asume
  que se puede esquivar por la derecha siempre. Para versiones avanzadas,
  agregar dos ultrasonidos laterales y elegir el lado libre.

## Hardware asumido

| Puerto | Componente |
|---|---|
| **A** | Motor izquierdo (`Direction.COUNTERCLOCKWISE`) |
| **B** | Motor derecho |
| **C** | (reservado para attachment de evacuación) |
| **D** | UltrasonicSensor frontal |
| **E** | ColorSensor IZQUIERDO mirando al piso |
| **F** | ColorSensor DERECHO mirando al piso |

Si tus puertos son distintos, cambiá la sección `HARDWARE` arriba en el .py.

Los dos ColorSensor deben ir montados:
- 5-10 mm sobre el piso (más bajo da contraste, más alto evita rasguños)
- Separados unos 30-40 mm entre sí (uno a cada lado del centro de la línea)
- 80-120 mm adelante del eje de las ruedas motrices

## Calibración previa al primer run

### 1. Calibrar geometría del DriveBase (una sola vez por robot)

Probar `drive.straight(1000)` con cinta métrica y ajustar `WHEEL_DIAMETER_MM`
hasta que recorra exactamente 1000 mm. Después probar `drive.turn(360)` y
ajustar `AXLE_TRACK_MM` hasta que el robot vuelva al heading inicial con
error < 1°.

Ver la skill [`pybricks-precision-driving`](../../skills/pybricks-precision-driving/SKILL.md)
para el procedimiento detallado.

### 2. Calibrar sensores de color (cada vez que cambia el mat o la luz)

**Opción A — Auto desde el programa:**

1. Encender el hub. Esperar el splash (estrella).
2. Ver el `?` en pantalla.
3. Mantener **LEFT presionado**.
4. Cuando aparece `W` en pantalla, posicionar los dos sensores sobre el blanco del mat y presionar CENTER.
5. Cuando aparece `B`, posicionar los dos sensores sobre la línea negra y presionar CENTER.
6. Aparece `OK` y los valores se imprimen por consola (visibles si el hub está conectado al editor por USB).

**Opción B — Hardcodear los valores:**

Editar las constantes al tope del .py después de leer manualmente con
`color.reflection()`:

```python
WHITE_LEFT = 92
BLACK_LEFT = 8
WHITE_RIGHT = 90
BLACK_RIGHT = 10
```

### 3. Calibrar detección de verde (una vez por marcador, importante)

Los marcadores verdes del set RCJ pueden tener tono distinto al genérico.
Leer con `sensor.hsv()` sobre un marcador real y ajustar:

```python
GREEN_HUE_MIN = 90       # bajar si los verdes tiran a amarillo
GREEN_HUE_MAX = 150      # subir si los verdes tiran a azul
GREEN_SAT_MIN = 40       # bajar si los marcadores son pálidos
GREEN_VAL_MIN = 30
```

Probar con un programa corto:

```python
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

s = ColorSensor(Port.E)
while True:
    print(s.hsv())
    wait(200)
```

Pasar el sensor sobre el marcador verde y leer los valores reales en consola.

### 4. Verificar detección de plateado

La cinta plateada del RCJ es más reflectante que el blanco normal y tiene
baja saturación HSV. El programa detecta cuando `v > 80` y `s < 15`. Si tu
plateado es distinto, ajustar `SILVER_VAL_MIN` y `SILVER_SAT_MAX`.

## Cómo iniciar un run

1. Encender el hub.
2. (Opcional) Mantener LEFT presionado los primeros 1.5 seg para entrar a calibración.
3. Esperar a que aparezca la flecha en el display.
4. Posicionar el robot **sobre la línea negra**, **mirando en la dirección de marcha**.
5. Presionar **CENTER**.
6. El robot arranca, sigue la línea, y reacciona automáticamente a los
   eventos del campo.

## Protocolo de testing — ladder progresivo

NO probar el programa entero la primera vez. Subir por escalones, validando
cada uno antes de pasar al siguiente. Esto está sacado de la skill
[`rescue-line-course-planner`](../../skills-pack/rescue-line-course-planner/SKILL.md)
del pack externo.

| # | Test | Qué validás | Esperado |
|---|---|---|---|
| 1 | Línea recta 1 metro | Line following base | Sigue sin oscilar visiblemente |
| 2 | Curva suave de 90° | Kp suficiente | Toma la curva sin salirse |
| 3 | Curva cerrada en U | Kp/Kd y velocidad | Toma la U bajando velocidad si hace falta |
| 4 | Recta con un gap de 10 cm | `handle_gap` | Cruza el gap recto y retoma la línea |
| 5 | Recta con un gap de 20 cm | `handle_gap` | Idem |
| 6 | Marcador verde a la izquierda | `handle_green` | Gira 90° a la izquierda |
| 7 | Marcador verde a la derecha | `handle_green` | Gira 90° a la derecha |
| 8 | Marcador verde a ambos lados | `handle_green` | Gira 180° |
| 9 | Obstáculo (cubo) en el medio | `handle_obstacle` | Esquiva por la derecha y retoma |
| 10 | Rampa de subida | `handle_ramp` | Sube sin perder la línea |
| 11 | Rampa de bajada | `handle_ramp` | Idem |
| 12 | Cinta plateada | `handle_silver` | Para, luz azul, beeps, fin del programa |
| 13 | Pista entera con todo lo anterior | Loop completo | Cubre 100% de la pista |

**Regla**: cada test tiene que pasar **8 de 10 corridas** antes de pasar al
siguiente. Si falla, mirar la skill
[`competition-debugger`](../../skills-pack/competition-debugger/SKILL.md) y el
fault tree para diagnosticar.

## Tuning rápido del PID

Del orden de la skill [`line-follower-tuner`](../../skills-pack/line-follower-tuner/SKILL.md):

1. **Threshold** OK (calibración hecha).
2. **`BASE_SPEED`** estable. Empezar con 150 mm/s.
3. **`KP`**: subir hasta que el robot siga la línea con confianza. Si oscila, bajar.
4. **`KD`**: empezar con `5 * KP`. Subir si oscila. Bajar si responde tarde a curvas.
5. Una vez estable, subir gradualmente `BASE_SPEED` hasta perder la línea, después bajar 20%.

**Defaults razonables** para la mayoría de los robots Spike Prime:

```python
BASE_SPEED = 180
KP = 1.5
KD = 6.0
```

## Errores típicos y soluciones

| Síntoma | Causa probable | Solución |
|---|---|---|
| Robot oscila visiblemente sobre la línea recta | Kp muy alto | Bajar Kp 30%, agregar Kd |
| Se sale en curvas cerradas | Kp bajo o velocidad alta | Subir Kp o bajar BASE_SPEED |
| Detecta verde como negro | Rango HSV mal | Leer `sensor.hsv()` real y ajustar |
| Detecta verde donde no hay | Saturación min muy baja | Subir GREEN_SAT_MIN |
| No cruza el gap | GAP_MAX_RECOVERY_MM corto | Subir a 350 |
| Cruza el gap pero pierde heading | Gyro mal calibrado | Verificar `use_gyro(True)` y `reset_heading(0)` al inicio |
| Detecta la rampa donde no hay | Hub no plano en suelo | Recalibrar IMU dejando el robot quieto al encender |
| No detecta la cinta plateada | Threshold mal | Leer `sensor.hsv()` real sobre la cinta y ajustar |
| Falla con buena luz pero anda con sombra | Reflejos en el ColorSensor | Bajar el sensor más cerca del piso o cubrir lateralmente con falda negra |
| Anda con bater al 100%, falla al 70% | BASE_SPEED depende del voltaje | Bajar BASE_SPEED 20% como margen |

## Skills de Claude usadas

Este programa fue construido aplicando estas skills del repo:

- [`pybricks-spike-fundamentals`](../../skills/pybricks-spike-fundamentals/SKILL.md) — imports y estructura
- [`pybricks-precision-driving`](../../skills/pybricks-precision-driving/SKILL.md) — DriveBase + gyro
- [`pybricks-line-following`](../../skills/pybricks-line-following/SKILL.md) — PID con normalización
- [`rcj-rescue-line`](../../skills/rcj-rescue-line/SKILL.md) — elementos de la pista
- [`robotics-control-theory`](../../skills/robotics-control-theory/SKILL.md) — orden de prioridades del state machine
- [`line-follower-tuner`](../../skills-pack/line-follower-tuner/SKILL.md) — orden de tuning
- [`rescue-line-course-planner`](../../skills-pack/rescue-line-course-planner/SKILL.md) — separación modular
- [`competition-debugger`](../../skills-pack/competition-debugger/SKILL.md) — fault tree para troubleshooting

## Próximos pasos

1. **Probarlo en el robot real** y completar el ladder de testing.
2. **Ajustar constantes** según los resultados.
3. **Cuando funcione**, copiarlo a `rutinas-exitosas/` con un nombre que incluya la fecha y el robot.
4. **Programar el módulo de evacuación** como un programa separado que arranca después de detectar la cinta plateada.
5. **Mejorar el esquive de obstáculos** detectando de qué lado conviene esquivar (con ultrasonidos laterales).
