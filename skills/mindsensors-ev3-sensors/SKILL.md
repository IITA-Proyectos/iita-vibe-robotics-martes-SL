---
name: mindsensors-ev3-sensors
description: Integración de sensores Mindsensors (terceros, NO LEGO) con EV3 usando Pybricks/ev3dev y el bus I²C. Cubre LineLeader-V2 (array de 8 sensores), AbsoluteIMU-ACG, NXTCam5, MultiplexerForEV3, LightSensorArray, AngleSensor (GlideWheel-AS), GlideWheel-M, IRSeeker-V2 y otros sensores I²C de Mindsensors. Usar SIEMPRE que se trabaje con sensores Mindsensors en EV3, programación I²C en Pybricks EV3, terceros sobre puerto S1-S4, o se mencione 'mindsensors', 'LineLeader', 'NXTCam', 'AbsoluteIMU', 'I2CDevice', 'i2c address', 'I2C bus EV3', 'multiplexer NXT/EV3', 'IRSeeker', 'PSP-Nx', 'sensor multiplexor', 'array de 8 sensores luz'. NO usar para sensores LEGO oficiales (eso es pybricks-ev3-fundamentals) ni para sensores con interfaz no-I²C.
---

# Sensores Mindsensors en EV3 con Pybricks

[Mindsensors](http://www.mindsensors.com) fabrica sensores y módulos de tercera parte para LEGO MINDSTORMS NXT/EV3 que **expanden masivamente** lo que el brick puede hacer: arrays de luz, IMU completos (giroscopio + acelerómetro + magnetómetro), cámaras embebidas, multiplexores de sensores y motores, encoders angulares de precisión, sensores ultrasónicos, etc.

Casi todos se conectan al EV3 por el puerto sensor estándar pero hablan **I²C** (no el protocolo EV3-UART), así que requieren un patrón de programación distinto al de los sensores LEGO.

## Cuándo usar / cuándo NO usar

- ✅ Usar para: cualquier sensor Mindsensors enchufado al EV3, lectura I²C custom en Pybricks, conexión de NXTCam5, integración de LineLeader-V2 en seguidores de línea, AbsoluteIMU para reemplazar el gyro LEGO que driftea, multiplexores cuando se necesitan más puertos.
- ❌ No usar para: sensores LEGO oficiales (Color, Ultrasonic, Gyro, Touch — esos son `pybricks-ev3-fundamentals`), sensores I²C de otros fabricantes con protocolo distinto (ej. HiTechnic — buscar docs específicas), Spike Prime (los conectores son distintos y la mayoría de los Mindsensors no son compatibles eléctricamente).

## Cómo funciona el I²C en EV3

El EV3 tiene 4 puertos sensores (S1-S4). Cada uno puede operar en modo **EV3-UART** (sensores LEGO modernos), **NXT-Analog** (sensores LEGO viejos), o **NXT-I²C** (Mindsensors, HiTechnic). El kernel ev3dev autodetecta el tipo del sensor en cada puerto.

Para sensores Mindsensors la conexión es: enchufar al puerto S1-S4 → ev3dev detecta el dispositivo I²C → en Pybricks lo accedemos con `I2CDevice` o con clases dedicadas según el modelo.

**Direcciones I²C típicas de Mindsensors**: 0x02, 0x18, 0x34, 0x46, 0x50, etc. Cada modelo tiene una dirección documentada. Algunos permiten cambiarla por software (importante cuando hay >1 sensor del mismo modelo en distintos puertos).

## Acceso I²C genérico desde Pybricks EV3

Pybricks expone `pybricks.iodevices.I2CDevice` para conversación raw cuando no hay clase dedicada:

```python
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port

# Mindsensors I²C addr en el bus EV3 generalmente es la addr/2 documentada
# porque el EV3 usa direcciones de 7 bits sin el bit R/W.
# LineLeader documentada como 0x02 → en EV3 va como 0x01
device = I2CDevice(Port.S3, 0x01)

# Leer 8 bytes desde el registro 0x49
data = device.read(reg=0x49, length=8)
print(list(data))  # [b0, b1, ..., b7]

# Escribir un byte de comando
device.write(reg=0x41, data=b'\x52')  # 'R' = reset white calibration en LineLeader
```

**Truco clave**: las datasheets de Mindsensors documentan la dirección en formato 8-bit (con bit R/W). Pybricks usa 7-bit. **Dividir la dirección por 2** (o desplazarla `>> 1`) al instanciar.

Ejemplo: LineLeader documentado como `0x02` → en Pybricks va como `0x01`.

## Tabla de sensores Mindsensors más usados en competencia

| Sensor | Modelo | Addr (7-bit) | Para qué sirve |
|---|---|---|---|
| LineLeader-V2 | NXTLineLeader-V2 | 0x01 | Array de 8 sensores de luz para seguidor de línea pro |
| AbsoluteIMU-ACG | AbsoluteIMU-A, -C, -ACG | 0x11 | IMU 9-DOF (gyro+accel+mag) — reemplaza al gyro EV3 que driftea |
| NXTCam5 | NXTCam5 | 0x01 (configurable) | Cámara con detección de blobs y OCR onboard |
| Multiplexer (Sensor) | NXTMMX-v3 / EV3Mux | 0x06 | 4 sensores extra en un puerto |
| Multiplexer (Motor) | NXTMMX | 0x06 | 2 motores extra controlados por I²C |
| GlideWheel-AS | AngleSensor-AS | 0x18 | Encoder absoluto de alta precisión |
| IRSeeker-V2 | IRSeekerV2 | 0x08 | Detecta dirección de pelotas IR (RoboCup soccer) |
| PSP-Nx-v4 | PSP-Nx | 0x01 | Joystick PlayStation 2 vía I²C |
| LightSensorArray | NXTLSA / LSA | 0x14 | Array de 8 sensores **pasivos** (sin LED activo, más barato) |
| DistANCE-Nx | DIST-Nx-v3 | 0x01 | Sharp IR distance, mejor rango que ultrasónico LEGO |
| EV3SensorMux | EV3SensorMux | 0x32 | Permite conectar hasta 3 sensores LEGO al mismo puerto |

Verificar el datasheet actualizado en mindsensors.com — algunos modelos cambiaron de protocolo entre revisiones.

## LineLeader-V2 — el más usado en competencia

Array de 8 LEDs y 8 fototransistores que cubre ~30 mm. Devuelve los 8 valores y además un PID interno (puede correr el seguidor solo, devolviendo `steering` directamente).

### Wrapper de uso

```python
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port

class LineLeader:
    """Wrapper de Mindsensors LineLeader-V2 sobre I²C en Pybricks EV3."""

    ADDR = 0x01           # 0x02 >> 1
    REG_COMMAND  = 0x41
    REG_STEERING = 0x42   # int8 signed, -100..100 (PID interno)
    REG_AVERAGE  = 0x43   # 0-80 (posición ponderada × 10)
    REG_RESULT   = 0x44   # bitmask de los 8 sensores (1 = sobre línea)
    REG_RAW      = 0x49   # 8 bytes raw uncalibrated
    REG_CAL      = 0x59   # 8 bytes calibrated 0-100

    def __init__(self, port):
        self.dev = I2CDevice(port, self.ADDR)

    def _cmd(self, byte):
        self.dev.write(reg=self.REG_COMMAND, data=bytes([byte]))

    # Comandos del firmware (ASCII)
    def calibrate_white(self):  self._cmd(ord('W'))   # 'W'
    def calibrate_black(self):  self._cmd(ord('B'))   # 'B'
    def sleep(self):            self._cmd(ord('P'))   # power down LEDs
    def wake(self):             self._cmd(ord('W'))
    def invert(self):           self._cmd(ord('I'))   # blanco sobre negro
    def reset(self):            self._cmd(ord('R'))

    # Lecturas
    def steering(self):
        """PID interno: -100 (línea muy a la izq) .. 100 (muy a la der), 0 = centrada."""
        b = self.dev.read(reg=self.REG_STEERING, length=1)
        v = b[0]
        return v - 256 if v > 127 else v  # convertir a signed

    def position(self):
        """Posición ponderada 0-80 (centro = 40). Más precisa que steering."""
        return self.dev.read(reg=self.REG_AVERAGE, length=1)[0]

    def result(self):
        """Bitmask: bit N en 1 si sensor N detecta línea. 0b11111111 = todos sobre línea."""
        return self.dev.read(reg=self.REG_RESULT, length=1)[0]

    def raw(self):
        """8 valores raw del ADC (0-255)."""
        return list(self.dev.read(reg=self.REG_RAW, length=8))

    def calibrated(self):
        """8 valores calibrados 0-100 (0=negro, 100=blanco)."""
        return list(self.dev.read(reg=self.REG_CAL, length=8))
```

### Patrón de uso en una corrida

```python
ll = LineLeader(Port.S3)

# Calibración (una vez al inicio)
ev3.screen.print("Pone TODO BLANCO + centro")
while Button.CENTER not in ev3.buttons.pressed(): wait(20)
ll.calibrate_white()
wait(500)

ev3.screen.print("Pone TODO NEGRO + centro")
while Button.CENTER not in ev3.buttons.pressed(): wait(20)
ll.calibrate_black()
wait(500)

# Loop principal — modo POSITION (más control que steering interno)
KP, KD = 1.2, 8.0
BASE_SPEED = 250
last_error = 0
while True:
    pos = ll.position()         # 0-80, 40 = centro
    error = pos - 40
    derivative = error - last_error
    last_error = error
    turn = KP * error + KD * derivative
    drive.drive(BASE_SPEED, turn)
    wait(10)
```

### LineLeader: tres modos de operación

| Modo | Cómo se obtiene | Pros | Contras |
|---|---|---|---|
| **Steering interno** | `ll.steering()` | Sencillo, ya viene con PID | Las ganancias son fijas en firmware |
| **Position (recomendado)** | `ll.position()` (0-80) | Vos sintonizás tu PID externo, control total | Requiere tuning |
| **Raw / calibrated bitmask** | `ll.calibrated()` o `ll.result()` | Detección de intersecciones, ramas, gaps | Más código |

Para competencia seria usar **modo position con PID externo + lectura del bitmask para detectar T-junctions y all-black**.

### Detección de intersecciones con LineLeader

```python
def detect_pattern(ll):
    """
    Devuelve el patrón del array como string para debug y para detectar bifurcaciones.
    Cada char: '#' = sobre línea, '.' = blanco.
    """
    bits = ll.result()
    return ''.join('#' if (bits >> i) & 1 else '.' for i in range(8))

# Casos típicos:
# '........' → toda blanca (gap o salida)
# '...##...' → línea centrada
# '########' → intersección perpendicular (T o cruce)
# '######..' → empieza una bifurcación a la izquierda
# '..######' → empieza una bifurcación a la derecha
```

## AbsoluteIMU-ACG — el reemplazo del gyro LEGO

El gyro de EV3 driftea 1-2°/min y se calibra solo apagando y prendiendo. El AbsoluteIMU es un 9-DOF (gyro de 3 ejes + accel de 3 ejes + magnetómetro de 3 ejes) con drift mucho menor.

### Wrapper

```python
class AbsoluteIMU:
    """Mindsensors AbsoluteIMU-ACG: gyro+accel+mag."""

    ADDR = 0x11  # 0x22 >> 1
    REG_COMMAND = 0x41
    REG_TILT_X   = 0x42  # signed
    REG_ACCEL_X  = 0x45  # 6 bytes, accel xyz int16
    REG_HEADING  = 0x4B  # uint16 compass 0-359
    REG_MAG_X    = 0x4D
    REG_GYRO_X   = 0x53  # 6 bytes, gyro xyz int16 (deg/s × 100)

    def __init__(self, port):
        self.dev = I2CDevice(port, self.ADDR)

    def _read_int16(self, reg, count=3):
        data = self.dev.read(reg=reg, length=2 * count)
        out = []
        for i in range(count):
            v = data[2*i] | (data[2*i+1] << 8)
            if v > 32767: v -= 65536
            out.append(v)
        return out

    def gyro(self):
        """deg/s en x, y, z (escalado /100 según firmware)."""
        return [v / 100.0 for v in self._read_int16(self.REG_GYRO_X)]

    def accel(self):
        """milli-g en x, y, z."""
        return self._read_int16(self.REG_ACCEL_X)

    def heading(self):
        """Compass: 0-359°."""
        d = self.dev.read(reg=self.REG_HEADING, length=2)
        return d[0] | (d[1] << 8)

    def start_calibration_compass(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'C')
    def stop_calibration_compass(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'c')
    def start_calibration_accel(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'X')
    def stop_calibration_accel(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'x')
```

### Uso típico — heading absoluto

```python
imu = AbsoluteIMU(Port.S4)

# Calibración del compass (UNA vez por sesión, lejos de imanes/motores)
imu.start_calibration_compass()
for _ in range(20):
    drive.turn(20)  # girar 360° lento
    wait(100)
imu.stop_calibration_compass()

# Usar heading como referencia absoluta
target = imu.heading()
while running:
    error = target - imu.heading()
    if error > 180: error -= 360
    if error < -180: error += 360
    drive.drive(200, KP_HEADING * error)
    wait(20)
```

**Gotcha del compass**: la cercanía a motores LEGO y al brick mismo genera campos magnéticos que arruinan la lectura. **Montar el AbsoluteIMU lejos de los motors** — un brazo de 8-10 cm hace la diferencia.

## NXTCam5 — visión a bordo

Cámara con procesamiento de blobs en el firmware. Devuelve hasta 8 objetos por frame con sus bounding boxes y colores.

```python
class NXTCam5:
    ADDR = 0x01  # 0x02 >> 1
    REG_COMMAND = 0x41
    REG_NUM_OBJECTS = 0x42
    REG_BLOBS = 0x43  # 5 bytes por blob × 8 blobs = 40 bytes

    def __init__(self, port):
        self.dev = I2CDevice(port, self.ADDR)

    def track_objects(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'B')   # iniciar tracking
    def stop_tracking(self):
        self.dev.write(reg=self.REG_COMMAND, data=b'X')

    def blobs(self):
        """Devuelve lista de (color, left, top, right, bottom)."""
        n = self.dev.read(reg=self.REG_NUM_OBJECTS, length=1)[0]
        if n == 0: return []
        data = self.dev.read(reg=self.REG_BLOBS, length=5 * n)
        result = []
        for i in range(n):
            base = 5 * i
            result.append((data[base], data[base+1], data[base+2],
                          data[base+3], data[base+4]))
        return result
```

Calibrar los **colormaps** desde NXTCam5utility.exe en Windows antes de usar.

## Multiplexer — más sensores en menos puertos

Cuando se necesitan 5+ sensores el `EV3SensorMux` (o `NXTMMX` viejo) divide un puerto físico en 3 lógicos.

```python
# EV3SensorMux en S3, expone 3 sub-puertos: 1, 2, 3
mux = I2CDevice(Port.S3, 0x32)

# Configurar el sub-puerto 1 como ColorSensor
mux.write(reg=0x4E, data=bytes([0x01, 0x29]))  # subport 1, modo color
# ... y leer desde su dirección virtual
```

**Importante**: el multiplexer trae latencia adicional de ~10-20 ms por lectura. Si necesitás un loop a 100 Hz, mantené sensores críticos (LineLeader, IMU) en puertos directos y usá el mux solo para sensores secundarios.

## I²C: gotchas que pasan SIEMPRE en competencia

| Síntoma | Causa | Solución |
|---|---|---|
| `OSError: ENODEV` al instanciar | ev3dev no detectó el sensor como I²C | Desconectar/reconectar el cable, esperar 3 seg, reintentar |
| Lecturas todas en 0 | Dirección I²C mal (sin dividir por 2) | Probar `addr // 2` o `addr >> 1` |
| Lecturas erráticas (saltos al azar) | Cable largo o malo, batería baja, ruido eléctrico | Cable LEGO original más corto, batería ≥7.5 V |
| Funciona uno pero al agregar otro fallan ambos | Conflicto de direcciones | Algunos Mindsensors permiten cambiar addr — ver REG_I2C_ADDR del datasheet |
| LineLeader devuelve siempre 40 | Sin calibrar o calibrado con el mismo color | Re-ejecutar calibrate_white Y calibrate_black con superficies distintas |
| AbsoluteIMU compass apunta siempre al mismo lado | Saturado por motor cercano | Alejar el sensor del motor 10+ cm |
| NXTCam5 no ve nada | Modo apagado, lente sucia, colormap mal calibrado | `track_objects()` antes de leer; calibrar colormap en daylight |
| Mux satura el loop | Demasiadas lecturas por ciclo | Reducir frecuencia: leer sensor lento cada 5 ciclos |
| Programa freezea al desconectar el sensor | I²C bus en estado raro | Power-cycle del EV3 (el bus se reinicia) |

## Patrón "verificar sensor al arrancar"

```python
def safe_sensor_init(port, sensor_class, name):
    """Detecta si el sensor está enchufado. Aborta el programa con mensaje claro si no."""
    try:
        s = sensor_class(port)
        # Read de prueba
        if hasattr(s, 'calibrated'):
            _ = s.calibrated()
        elif hasattr(s, 'heading'):
            _ = s.heading()
        return s
    except OSError as e:
        ev3.screen.clear()
        ev3.screen.print("FALLA")
        ev3.screen.print(name)
        ev3.screen.print(str(port))
        ev3.speaker.beep(200, 1000)
        raise SystemExit("Sensor " + name + " no responde en " + str(port))

ll = safe_sensor_init(Port.S3, LineLeader, "LineLeader")
imu = safe_sensor_init(Port.S4, AbsoluteIMU, "IMU")
```

## Recursos

- Mindsensors product index: http://www.mindsensors.com/ev3-and-nxt
- LineLeader-V2 datasheet (PDF en el sitio del producto)
- ev3dev I²C driver docs: https://docs.ev3dev.org/projects/lego-linux-drivers/en/ev3dev-stretch/sensors.html
- Pybricks `iodevices.I2CDevice`: https://docs.pybricks.com/en/latest/iodevices/i2cdevice.html
- Repos de wrappers Mindsensors en ev3dev-python (referencia): https://github.com/ev3dev/ev3dev-lang-python
