# m_reference — Análisis de rutinas RCJ Rescue Line (Spike Legacy API)

> Documento de referencia interna. Análisis técnico de un sistema de robot
> competitivo para RoboCup Junior Rescue Line, basado en LEGO Spike Prime
> con arquitectura híbrida Spike + Arduino + HuskyLens.

---

## 🔧 Stack tecnológico y diferencias clave con Pybricks

Este robot usa la **API Legacy de LEGO Education Spike** (`from spike import ...`)
en lugar de Pybricks. Son DISTINTAS. Diferencias importantes:

| Concepto | Spike Legacy | Pybricks |
|---|---|---|
| Motores par | `MotorPair('C', 'A')` | `DriveBase(left, right, diam, axle)` |
| Mover par | `motor_pair.move_tank(deg, 'degrees', left, right)` | `drive.straight(mm)` |
| Sensor color | `sensor.get_reflected_light()` | `sensor.reflection()` |
| Color discreto | `sensor.get_color()` | `sensor.color()` |
| Giroscopio | `motion_sensor.get_yaw_angle()` | `hub.imu.heading()` |
| Reset yaw | `motion_sensor.reset_yaw_angle()` | `hub.imu.reset_heading(0)` |
| Timer | `from spike.control import Timer` | `from pybricks.tools import StopWatch` |
| Esperar | `wait_for_seconds(n)` | `wait(ms)` |
| Tiempo actual | `time.ticks_ms()` | `StopWatch().time()` |

---

## 📌 Configuración del hardware (puertos)

```python
# Motores de tracción
right_engine = Motor('A')
left_engine  = Motor('C')
motor_pair   = MotorPair('C', 'A')  # izq primero, der segundo
motor_pair.set_motor_rotation(3.2 * math.pi, 'cm')
# → circunferencia real de rueda en cm. 3.2 * pi ≈ 10.05 cm

# Sensores de color (3 sensores)
left_color    = ColorSensor("F")   # izquierdo al piso
forward_color = ColorSensor("D")   # frente/centro al piso
right_color   = ColorSensor("B")   # derecho al piso

# HuskyLens (cámara IA)
huskyLens = HuskyLensCamera(hub.port.E, baudrate=9600, debug=False)
# → conectada al puerto E del Spike via UART serial

# Umbrales de reflectancia calibrados para la cancha real
basic_blak_color   = 26   # negro para sensores laterales en line following
side_black_color   = 29   # negro para detección de intersecciones laterales
black_color_front  = 20   # negro para sensor frontal
double_blck        = 32   # negro para detección de doble negro (ambos lados)
obstacule_distace  = 40   # distancia de obstáculo (cm, con DistanceSensor)
recue_distance     = 180  # distancia en zona de evacuación
```

---

## 🧠 Loop principal — hilo de ejecución

El programa corre en un **`while True` sin `wait()` entre iteraciones**.
Esto significa que el loop corre tan rápido como el intérprete MicroPython
del Spike lo permite (~100-300 Hz dependiendo de la carga).

### Estructura del loop:

```
Iteración N:
  1. Leer sensores (color_l, color_r, light_l, ligth_f, ligth_r)
  2. Elegir modo de control:
     a. controlPID == True → aplicar PID
     b. controlPID == False → control bang-bang simple
  3. Verificar timer de pérdida de línea
  4. Llamar green()
  5. Llamar doubleblack()
  6. Llamar right_intersection()
  7. Llamar left_intersection()
  8. Llamar cuadrado()
```

> ⚠️ IMPORTANTE: Las funciones del paso 4-8 se llaman en CADA iteración.
> Si detectan su condición, ejecutan y retornan. Si no, retornan inmediatamente.
> NO hay `break` del loop principal dentro de ellas (excepto `cuadrado()` que
> es bloqueante). Esto es una state machine implícita por prioridad de llamadas.

---

## 🎮 Modo 1: Control bang-bang (controlPID = False)

```python
if left_color.get_reflected_light() > basic_blak_color:
    right_engine.start_at_power(30)
else:
    right_engine.start_at_power(-50)

if right_color.get_reflected_light() > basic_blak_color:
    left_engine.start_at_power(-30)
else:
    left_engine.start_at_power(50)
```

**Lógica:** Cada motor se controla independientemente según el sensor DEL LADO
OPUESTO. Si el sensor izquierdo ve blanco (>26), el motor derecho avanza.
Si ve negro (<26), el motor derecho frena/retrocede. Esto hace que el robot
"persiga" el borde de la línea.

**Nota sobre los valores:** `start_at_power(30)` vs `start_at_power(-50)`:
la frenada es más agresiva que el avance para lograr giros más rápidos.

---

## 🎮 Modo 2: Control PID (controlPID = True)

```python
velocidad = 10    # velocidad base (potencia %)
kp = 1.2          # ganancia proporcional
ki = 0.0          # integral desactivada (evita windup)
kd = 0.15         # ganancia derivativa (suaviza oscilaciones)

dt = tiempo - tanterior           # tiempo entre iteraciones (ms)
error = ligth_r - light_l         # sensor der - sensor izq
proporcional = error
integral = integral + error * 0.005   # acumulación con factor de escala
derivada = error - error_previo       # cambio de error entre ticks

salida = int(kp * proporcional + ki * integral + kd * derivada)

motor_pair.start_tank(velocidad + salida, velocidad - salida)
```

**Lógica del error:** `error = derecho - izquierdo`
- Si derecho ve más negro que izquierdo → error negativo → robot a la derecha → corrección a la izquierda
- Si izquierdo ve más negro → error positivo → corrección a la derecha

**Por qué ki = 0.0:** En line following, el integral acumula error en curvas
y hace que el robot sobrecompense. Se deja en 0 por defecto y solo se activa
si hay deriva sistemática comprobada.

**El factor 0.005 del integral:** Es un anti-windup manual. Sin él, en una
curva larga el integral puede crecer hasta valores que hacen el robot incontrolable.

---

## ⏱️ Sistema de recuperación por timeout (pérdida de línea)

```python
# Al inicio del loop:
if light_l < 30 or ligth_r < 30 or ligth_f < 40:
    timer.reset()   # hay línea → resetear timer

# Después del control:
if timer.now() > 2.5:   # 2.5 segundos sin ver línea
    motor_pair.stop()
    motor_pair.start_tank(-30, -30)  # retroceder
    while (left_color.get_reflected_light() > 30 and
           right_color.get_reflected_light() > 30 and
           forward_color.get_reflected_light() > 40):
        print('sin linea')         # retroceder hasta ver algo negro
    motor_pair.stop()

    if left_color.get_reflected_light() < 30:
        right_engine.run_for_degrees(100, -45)   # girar a la izquierda
    elif right_color.get_reflected_light() < 30:
        left_engine.run_for_degrees(100, 45)     # girar a la derecha
```

**Flujo completo del timeout:**
1. Timer se resetea en cada tick donde hay línea.
2. Si 2.5 segundos pasan sin línea → el robot perdió la pista.
3. Retrocede lentamente hasta reencontrar negro en cualquier sensor.
4. Corrige la orientación girando sobre el motor del lado que encontró la línea.

**Detalle de `run_for_degrees(100, -45)`:**
Primer argumento = grados a rotar, segundo = velocidad en %.
Esto es bloqueante — el motor gira 100° y luego continúa el loop.

---

## 🟢 Rutina: `green()` — Detección y manejo de marcadores verdes

```python
def green():
    if right_color.get_color() == 'green':
        motor_pair.stop()
        right_engine.run_for_degrees(4)     # avance mínimo para leer bien
        if left_color.get_color() == 'green':
            turn_180_rigth()                # ambos verdes → U-turn
        else:
            motor_pair.move_tank(180, 'degrees', 60, 60)  # avanzar al centro
            turn_90_right()
            motor_pair.move_tank(80, 'degrees', 60, 60)
            look_rigth_line()               # buscar la línea a la derecha

    if left_color.get_color() == 'green':
        motor_pair.stop()
        left_engine.run_for_degrees(3)
        if right_color.get_color() == 'green':
            turn_180_rigth()                # ambos verdes → U-turn
        else:
            motor_pair.move_tank(180, 'degrees', 60, 60)
            turn_90_left()
            motor_pair.move_tank(80, 'degrees', 60, 60)
            look_left_line()
```

**Lógica de decisión:**
| Sensor izq | Sensor der | Acción |
|---|---|---|
| Verde | Verde | U-turn 180° (calle sin salida) |
| Verde | No verde | Girar 90° a la izquierda |
| No verde | Verde | Girar 90° a la derecha |

**Problema conocido:** Usa `get_color() == 'green'` que es clasificación
discreta del sensor. Bajo ciertas luces, puede confundir verde oscuro con negro.
La solución más robusta es usar HSV (como en el sistema de Pybricks del taller actual).

**Los delays implícitos:** `move_tank(180, 'degrees', ...)` es BLOQUEANTE.
El robot no hace nada más hasta completar los 180 grados de rotación de ruedas.
Esto congela todo el loop por ~500ms-1s dependiendo de la velocidad.

---

## ⬛ Rutina: `doubleblack()` — Detección de doble negro (intersección total)

```python
def doubleblack():
    if (right_color.get_reflected_light() < double_blck and
        left_color.get_reflected_light() < double_blck):
        motor_pair.stop()
        motor_pair.move_tank(3, 'degrees', 60, 60)   # micro-avance
        if (right_color.get_reflected_light() < double_blck and
            left_color.get_reflected_light() < double_blck):
            motor_pair.move_tank(100, 'degrees', 60, 60)  # cruzar recto
        else:
            motor_pair.move_tank(-16, 'degrees', 60, 60)  # retroceder
```

**Propósito:** Detectar cuando AMBOS sensores laterales ven negro
simultáneamente, lo que indica una intersección o cruce completo.

**El doble chequeo:** Se avanza 3° (micro-movimiento) y se re-verifica.
Esto filtra falsos positivos causados por ruido o bordes de la línea.

**Si sigue en doble negro:** Avanza 100° para cruzar la intersección recto.
**Si ya no está en doble negro:** Retrocede 16° (era un borde, no intersección).

---

## 🚦 Rutinas: `right_intersection()` y `left_intersection()`

```python
def right_intersection():
    if (right_color.get_reflected_light() < side_black_color and
        forward_color.get_reflected_light() < black_color_front):
        motor_pair.start_tank(0, 0)
        motor_pair.move_tank(3, 'degrees', 30, 30)
        if (right_color.get_reflected_light() < side_black_color and
            forward_color.get_reflected_light() < black_color_front):
            motor_pair.move_tank(100, 'degrees', 60, 60)
        else:
            motor_pair.move_tank(-15, 'degrees', 60, 60)
```

**Propósito:** Detectar T-intersecciones. Cuando el sensor derecho Y el
sensor frontal ven negro simultáneamente = hay una ramificación a la derecha
y la línea principal continúa adelante.

**Comportamiento:** Igual que `doubleblack()` — avanza 3° para confirmar
y luego cruza 100° o retrocede 15° si fue falso positivo.

**`left_intersection()` es simétrica** pero usa umbrales fijos de 32 en
lugar de las constantes, lo que parece un bug de calibración sin terminar.

---

## 🔄 Rutinas de giro: `turn_90_left()` y `turn_90_right()`

```python
def turn_90_left():
    motor_pair.start_tank(0, 0)
    wait_for_seconds(1)                        # PAUSA COMPLETA de 1 segundo
    Primehub.motion_sensor.reset_yaw_angle()   # resetear giroscopio a 0
    motor_pair.start_tank(30, -30)             # girar: rueda izq adelante, der atrás
    while (Primehub.motion_sensor.get_yaw_angle() < 85):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.start_tank(0, 0)
```

**Análisis del `wait_for_seconds(1)`:**
Este delay de 1 segundo es CRÍTICO y tiene razón de ser:
1. Detiene el robot completamente.
2. Deja que el giroscopio se estabilice (el IMU del Spike tiene deriva
   cuando hay vibración mecánica por los motores).
3. Hace el reset del yaw cuando el sensor está quieto → lecturas más precisas.

**Sin este delay:** El giroscopio puede reportar un valor residual del
movimiento anterior, y el giro terminaría en un ángulo incorrecto.

**Por qué 85° en lugar de 90°:** El momentum del robot hace que siga
girando ~5° después de que los motores paran. El undershoot intencional
de 5° compensa la inercia. Requiere calibración empírica.

**`turn_180_rigth()` = dos turn_90_left seguidos:**
```python
def turn_180_rigth():
    turn_90_left()
    turn_90_left()
    motor_pair.move_tank(0.7, 'cm', 30, -30)  # micro-ajuste de posición
```
Total de delays: 2 segundos de pausa + tiempo de giro x2.

---

## 🔍 Rutinas de búsqueda de línea: `look_rigth_line()` y `look_left_line()`

```python
def look_rigth_line():
    motor_pair.stop()
    wait_for_seconds(1)                         # stabilizar giroscopio
    Primehub.motion_sensor.reset_yaw_angle()
    motor_pair.start_tank(-30, 30)              # girar hacia la derecha
    while (Primehub.motion_sensor.get_yaw_angle() < -10):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.stop()
    motor_pair.start_tank(30, -30)              # girar en sentido contrario
    while (right_color.get_reflected_light() > 30):
        print('sensor derecho')
    motor_pair.stop()
```

**Propósito:** Después de un giro a la derecha, esta rutina busca la línea
haciendo un barrido angular. Gira primero hasta -10° (overshoots levemente)
y luego barre en sentido contrario hasta que el sensor derecho detecta negro.

**El hilo de ejecución aquí:**
1. Para motores → 1 segundo quieto.
2. Primer `while`: gira hasta llegar a -10° de yaw. Bloqueante hasta lograrlo.
3. Para motores.
4. Segundo `while`: gira en sentido opuesto hasta ver negro. También bloqueante.
5. Para motores. → Retorna al loop principal.

**Toda esta función es completamente bloqueante** durante ~2-4 segundos.

---

## 📦 Rutina: `cuadrado()` — Zona de evacuación

```python
def cuadrado():
    if (forward_color.get_green() > 630 and
        forward_color.get_blue() > 700 and
        forward_color.get_red() > 620):
        motor_pair.stop()
        wait_for_seconds(2)
        motor_pair.start_tank(25, 25)
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) < 6000:
            Aproximity()
        turn_90_rigth_rescue()
        # ... repite 4 veces haciendo un cuadrado
```

**Detección de zona plateada/blanca brillante:** El sensor frontal lee los
canales RGB separadamente. Cuando los tres canales son altos (R>620, G>630, B>700)
significa una superficie muy reflectante y neutra = cinta plateada o zona de evacuación.

**El patrón del cuadrado:** El robot recorre el perímetro de la zona de
evacuación en un patrón cuadrado, en cada lado buscando víctimas por 6 segundos.

**`time.ticks_ms()` y `time.ticks_diff()`:**
Estas son funciones de MicroPython para medir tiempo con precisión de milisegundos.
`ticks_diff(actual, inicio)` calcula la diferencia correctamente incluso si
hay overflow del contador de 32 bits (overflow ocurre cada ~49 días).

---

## 👁️ Rutina: `FollowVictim()` — Seguimiento de víctima con HuskyLens

```python
def FollowVictim():
    while True:
        vivo = huskyLens.getBlocksByID(1)     # pedir bloque ID=1 a la cámara
        if len(vivo) == 1:
            x_position = vivo[0].x            # posición X del objeto en pantalla
            h_height   = vivo[0].height       # altura del bounding box

            if x_position < 150:
                motor_pair.move_tank(0.5, "cm", left_speed=0, right_speed=15)
            elif x_position > 180:
                motor_pair.move_tank(0.5, "cm", left_speed=15, right_speed=0)
            elif h_height > 28 and h_height < 30:
                motor_pair.move_tank(3, "cm", 15, 15)   # suficientemente cerca
                motor_pair.stop()
                wait_for_seconds(10)                     # ESPERA 10 segundos
            else:
                # centrado en X, acercarse
                motor_pair.start(10, 10)
        else:
            motor_pair.move_tank(1, "cm", -15, -15)     # no ve víctima → retrocede
```

**Lógica de la cámara HuskyLens:**
La pantalla de la HuskyLens mide 320×240 píxeles. El centro es X=160.
- X < 150 → víctima a la izquierda → girar derecha (rueda der avanza)
- X > 180 → víctima a la derecha → girar izquierda (rueda izq avanza)
- X en zona muerta [150, 180] → centrado → acercarse recto

**La altura como indicador de distancia:**
A mayor altura del bounding box → víctima más cerca.
`h_height > 28 and h_height < 30` es una ventana muy estrecha (~2px).
Cuando la víctima ocupa ese rango de altura, el robot está lo suficientemente
cerca → avanza 3cm más → se detiene 10 segundos (tiempo para "rescatar").

**El `wait_for_seconds(10)` es completamente bloqueante.** Durante esos 10
segundos el robot no hace NADA más. Es el tiempo que se le da al mecanismo
de claw (Arduino) para completar la acción de rescate.

**`Aproximity()` es el wrapper:**
```python
def Aproximity():
    ball = huskyLens.getBlocksByID(1)
    if len(ball) == 1:
        FollowVictim()
```
Solo llama a FollowVictim si la cámara ve algo. Si no hay víctima en cuadro,
retorna inmediatamente al loop de cuadrado.

---

## 📡 Clase HuskyLensCamera — Comunicación UART con la cámara IA

La HuskyLens se comunica por protocolo serial binario propio.
La clase implementa este protocolo desde cero en MicroPython.

### Protocolo de comunicación:
```
Frame de comando:
  55 AA 11          ← header (3 bytes fijos)
  [data_length]     ← 1 byte: largo de los datos
  [command_byte]    ← 1 byte: qué función ejecutar
  [data...]         ← N bytes de datos
  [checksum]        ← 1 byte: suma de todos los bytes anteriores mod 256
```

### Métodos clave:
| Método | Qué hace |
|---|---|
| `knock()` | Verifica que la cámara responde (handshake) |
| `blocks()` | Pide todos los bloques detectados en pantalla |
| `getBlocksByID(n)` | Pide el bloque con ID de aprendizaje N |
| `algorithm(alg)` | Cambia el algoritmo de visión activo |
| `learn(x)` | Enseña un nuevo objeto con ID x |
| `forget()` | Borra todos los objetos aprendidos |

### Por qué el `flush()` antes de cada comando:
El buffer UART puede tener datos residuales de comandos anteriores.
`flush()` vacía el buffer para que la respuesta del próximo comando
sea la primera cosa en el buffer, evitando desincronización del protocolo.

### El `force_read(size, timeout)`:
Lee `size` bytes con un timeout en milisegundos. Si el byte no llega
en `timeout` ms, retorna lo que tenga (puede ser buffer incompleto).
Esto es necesario porque el UART del Spike no es DMA — el programa
tiene que leer activamente byte a byte.

---

## 🔩 Arduino — Sistema de agarre (claw) de víctimas

El Arduino controla físicamente el mecanismo de claw/garra.
Recibe comandos del Spike por comunicación serial (UART entre los dos micros).

### Comandos del protocolo:
| Mensaje recibido | Acción | Función Arduino |
|---|---|---|
| `>` | Agarrar víctima en zona | `PMessage()` |
| `<[>` | Bajar claw y encender LED frontal | `AMessage()` |
| `<>` (var 1) | Subir izquierda | `DMessage()` |
| `<>` (var 2) | Subir derecha | `EMessage()` |
| `<>` (var 3) | Depositar izquierda | `FMessage()` |
| `<>` (var 4) | Depositar derecha | `GMessage()` |

### Por qué dos micros:
El Spike Prime tiene puertos limitados y no puede controlar servos comunes.
El Arduino maneja los servos del claw y los LEDs adicionales, mientras que
el Spike se encarga de la navegación y visión. Se comunican por el puerto
serie del Spike (modo UART en un puerto de sensor).

---

## 🏁 Prototipo PID puro (versión simplificada)

El archivo `PID.py` es la versión más simple del seguidor, usada para
entender el concepto antes de integrar todo:

```python
base = 20    # potencia base
kp = 1       # solo proporcional

while True:
    diferencia = left_color.get_reflected_light() - right_color.get_reflected_light()
    potencia_izquierda = int(base + diferencia * kp)
    potencia_derecha   = int(base - diferencia * kp)
    motor_pair.start_tank(potencia_derecha, potencia_izquierda)
```

**Por qué funciona:**
- `diferencia > 0` → izquierdo más blanco → robot yendo a la derecha
  → aumentar potencia izquierda, reducir potencia derecha → corrección a derecha
- `diferencia < 0` → derecho más blanco → robot yendo a la izquierda
  → aumentar potencia derecha, reducir potencia izquierda → corrección a izquierda
- `diferencia ≈ 0` → robot centrado → ambos motores a velocidad `base`

**Sin `wait()` en el loop:** El loop corre a la máxima velocidad del intérprete.
En el Spike Legacy esto es ~100-200 Hz, lo que da un control bastante suave.

---

## 🧵 Modelo mental del hilo de ejecución completo

```
INICIO
  │
  ├── Inicializar hardware
  ├── Conectar HuskyLens (knock)
  │
  └── while True:  ← LOOP INFINITO (sin wait entre iteraciones)
        │
        ├── Leer sensores (4 lecturas de reflection + 2 de color)
        │   → ~10-20ms de latencia UART por lectura
        │
        ├── Control de motores (PID o bang-bang)
        │   → inmediato, ~1ms
        │
        ├── Timer check
        │   → si timer > 2.5s: BLOQUEA hasta reencontrar línea
        │      (puede durar 1-5 segundos)
        │
        ├── green()
        │   → si detecta verde: BLOQUEA 2-8 segundos (giros + búsqueda)
        │   → si no: retorna en ~5ms
        │
        ├── doubleblack()
        │   → si detecta: BLOQUEA ~500ms
        │   → si no: retorna en ~5ms
        │
        ├── right_intersection() / left_intersection()
        │   → si detecta: BLOQUEA ~500ms
        │   → si no: retorna en ~5ms
        │
        └── cuadrado()
              → si detecta zona: BLOQUEA INDEFINIDAMENTE
                (el robot queda en el modo evacuación para siempre)
              → si no: retorna en ~5ms
```

**Consecuencia importante:** Como no hay RTOS ni threads, cuando una función
bloqueante está corriendo (como `turn_90_left()` con su delay de 1 segundo),
el robot NO puede detectar eventos nuevos. Si hay un marcador verde durante
un giro, no se va a ver hasta que el giro termine.

Este es el trade-off del modelo de programación secuencial vs event-driven.
Para competición esto funcionó porque los elementos de la pista están
espaciados lo suficiente para no superponerse temporalmente.

---

## 📝 Valores de calibración documentados

Para replicar este sistema en otro robot, los valores críticos a calibrar son:

```python
# Reflectancia (0-100)
basic_blak_color  = 26   # negro en sensores laterales (line following)
side_black_color  = 29   # negro para detección de T-intersección
black_color_front = 20   # negro para sensor frontal
double_blck       = 32   # negro para doble negro (ambos sensores)

# PID
velocidad = 10    # potencia base %
kp = 1.2          # proporcional (ajustar empíricamente)
ki = 0.0          # integral (mantener en 0 salvo deriva comprobada)
kd = 0.15         # derivativo (suaviza sin agregar latencia)

# Giros (compensación de inercia)
# Usar 85° en lugar de 90° para compensar momentum post-frenado

# HuskyLens zona muerta centrado
# x < 150 → girar derecha
# x > 180 → girar izquierda
# [150, 180] → acercarse recto

# Altura de víctima para "suficientemente cerca"
# h_height entre 28 y 30 px
```

---

*Referencia técnica — IITA Vibe Robotics — Uso interno*
