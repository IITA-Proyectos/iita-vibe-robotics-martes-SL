# 📊 Skill Futuro: Telemetry & Log Analyzer

> **Estado**: 🔮 PARCIALMENTE ACTIVO — El logging básico ya se usa con Pybricks

## Cuándo activar completamente

- Cuando los programas se vuelvan complejos y difíciles de debuggear
- Cuando se necesite grabar datos de una corrida para análisis posterior
- Cuando se quiera comparar runs antes y después de un cambio

## Nivel 1: Logging con Pybricks (YA DISPONIBLE)

```python
# Print básico a consola de Pybricks
print("H:", hub.imu.heading(), "D:", robot.distance(), "V:", motor.speed())

# Con timestamp
from pybricks.tools import StopWatch
timer = StopWatch()
print(timer.time(), hub.imu.heading(), robot.distance())
```

### Patrón: Logger estructurado

```python
def log(msg, *values):
    """Imprimir con timestamp para análisis posterior."""
    t = timer.time()
    vals = " ".join(str(v) for v in values)
    print(f"{t} {msg} {vals}")

# Uso:
log("HEADING", hub.imu.heading())
log("MOTORS", motor_izq.speed(), motor_der.speed())
log("STATE", estado)
log("ERROR", error, "CORRECTION", correction)
```

### Copiar output de consola → análisis

1. Correr el programa con logging activado
2. Copiar el output de la consola de Pybricks
3. Pegarlo a Claude/ChatGPT con este prompt:

```
Analizá estos logs de mi robot. Cada línea tiene:
timestamp_ms variable valor

[pegar logs]

Necesito:
1. Gráfico mental del heading vs tiempo
2. ¿Hubo drift? ¿Cuánto?
3. ¿Los giros llegaron al target?
4. ¿Dónde están los problemas?
5. ¿Qué parámetros ajustar?
```

## Nivel 2: DataLog de Pybricks (DISPONIBLE)

```python
from pybricks.tools import DataLog

# Crear archivo de log en el hub
data = DataLog("time", "heading", "speed_l", "speed_r",
               name="run1", timestamp=False)

# En el loop:
data.log(timer.time(), hub.imu.heading(),
         motor_izq.speed(), motor_der.speed())

# Después: descargar el archivo CSV desde el hub
```

## Nivel 3: Análisis avanzado (FUTURO)

Cuando se use Arduino/RPi con serial logging:

```
Serial.println(String(millis()) + "," + 
               String(heading) + "," + 
               String(speed_left) + "," +
               String(speed_right));
```

### Prompt para análisis de CSV

```
Sos un ingeniero de datos de robótica.
Analizá este CSV de telemetría de mi robot.

Columnas: timestamp_ms, heading, speed_left, speed_right, state

[pegar CSV o adjuntar archivo]

Necesito:
1. Gráficos: heading vs tiempo, velocidades vs tiempo
2. Detección de anomalías (spikes, drift, oscillaciones)
3. Correlación entre estados y comportamiento
4. Para cada giro: error final, tiempo de settle, overshoot
5. Para cada recta: desvío máximo de heading, velocidad promedio
6. Recomendaciones concretas de parámetros a cambiar
```

## Métricas clave a logear

| Métrica | Para qué | Frecuencia |
|---------|----------|:----------:|
| `hub.imu.heading()` | Verificar giros y drift | 100 Hz |
| `motor.speed()` (ambos) | Detectar slip, saturación | 50 Hz |
| `robot.distance()` | Verificar distancias | 50 Hz |
| `sensor.reflection()` | Calibración line following | 50 Hz |
| `hub.battery.voltage()` | Correlación batería-rendimiento | 1 Hz |
| `estado` (state machine) | Debugging de lógica | Cada transición |
| `timer.time()` | Benchmark de tiempo por misión | Cada misión |

## Recursos

- [Pybricks DataLog docs](https://docs.pybricks.com/en/stable/tools.html)
- [Arduino Serial Plotter](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-serial-plotter/)
- [Python matplotlib](https://matplotlib.org/) — Para graficar CSVs
