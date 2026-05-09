# a_reference — Análisis de rutinas RCJ Rescue Line (Pybricks + zona de rescate adaptativa)

> Documento de referencia interna. Sistema competitivo RoboCup Junior Rescue Line
> con Pybricks, 3 sensores de color, colores HSV personalizados, PID con dc(),
> y algoritmo de rescate adaptativo por paredes con detección de esquina.

---

## 🔧 Hardware y puertos

```
Puerto D → Motor izquierdo (COUNTERCLOCKWISE)
Puerto B → Motor derecho (CLOCKWISE)
Puerto C → ColorSensor izquierdo (al piso)
Puerto A → ColorSensor derecho (al piso)
Puerto E → ColorSensor frontal (al piso)
Puerto F → UltrasonicSensor (distancia frontal)
```

**DriveBase definida pero usada solo para rectas controladas:**
```python
base_robot = DriveBase(motor_izquierdo, motor_derecho, 56, 140)
# wheel_diameter=56mm, axle_track=140mm
```
El seguidor de línea usa `motor.dc()` directamente, NO `DriveBase`.

---

## 🎨 Sistema de colores HSV personalizados — diferencia clave

En lugar de depender de la clasificación automática de Pybricks, este sistema
define exactamente qué es cada color en HSV calibrado para la pista real:

```python
Color.BLACK  = Color(h=180, s=20,  v=27)
Color.GREEN  = Color(h=166, s=68,  v=34)
Color.WHITE  = Color(h=204, s=17,  v=93)
Color.SILVER = Color(h=0,   s=0,   v=99)

# Colores frontales — mismo matiz pero distinta luminosidad
Color.BLACKFRONTAL  = Color(h=180, s=22, v=5)
Color.GREENFRONTAL  = Color(h=165, s=68, v=19)
Color.WHITEFRONTAL  = Color(h=202, s=19, v=79)
Color.SILVERFRONTAL = Color(h=0,   s=0,  v=99)
```

**Por qué colores separados para el frontal:**
El sensor frontal está orientado diferente o a distinta altura que los laterales.
La misma superficie (ej. negro) da distinta `v` (brillo) según el ángulo del sensor.
Con `detectable_colors()` se le dice a cada sensor exactamente qué buscar:

```python
color_sensor_l.detectable_colors(colores)   # lista lateral
color_sensor_r.detectable_colors(colores)
color_sensor_f.detectable_colors(coloresf)  # lista frontal
```

**SILVER y SILVERFRONTAL son iguales** (h=0, s=0, v=99) pero están en listas
distintas para que cada sensor los pueda detectar en su propio contexto.

---

## 📊 Umbrales de reflectancia calibrados

```python
centro_negro       = 25   # umbral negro en sensores laterales (centro de línea)
cenntro_negro_frente = 24 # umbral negro en sensor frontal
borde_negro        = 35   # umbral negro en borde de línea (más laxo)
negro_frontal      = 9    # umbral específico para detección T con sensor frontal
negro_derecha      = 13   # umbral para sensor derecho en detección T
negro_izquierda    = 13   # umbral para sensor izquierdo en detección T
```

**Valores de calibración anotados:**
```
Sensor izq centro: 17   Sensor izq borde: 48
Sensor der centro: 18   Sensor der borde: 48
Sensor frontal centro: 7  Sensor frontal borde: 27
```

**El centro de línea da ~17-18 de reflection** (negro puro ≈ 0).
El borde de la línea da ~48 (mezcla negro/blanco).

---

## 🧵 Loop principal y hilo de ejecución

```python
def main():
    global modo
    while True:
        update()                # 1. Leer todos los sensores
        if SILVER en los 3:
            modo = "rescate"    # 2. Detectar entrada a zona de rescate
        obstaculo()             # 3. Esquivar obstáculos
        deteccion_señal_verde() # 4. Responder a marcadores verdes
        doble_negro()           # 5. Detectar cruce total
        T_derecha()             # 6. Detectar T a la derecha
        T_izquierda()           # 7. Detectar T a la izquierda
        if modo == "rescate":
            rescate()
            break               # 8. Modo rescate → fin del loop de línea
        seguidor_de_linea()     # 9. Control PID normal
```

**Estado de la variable `modo`:** funciona como flag de estado de máquina.
Inicia en `None`. Cuando se detecta plateado → `"rescate"` → se activa
la función de rescate y el `break` termina el loop de línea para siempre.

**`update()` se llama ANTES que todo:** garantiza que las variables globales
`color_l`, `color_r`, `color_f`, `light_l`, `light_r`, `light_f`, `distance`
siempre tienen los datos frescos del tick actual.

**CÓDIGO DUPLICADO (BUG):** El código desde `valores_HSV_color` hasta el
segundo `main()` al final del archivo es código muerto — Python nunca lo
ejecuta porque está después del primer `main()`. Fue pegado dos veces.

---

## 🎮 Seguidor PID — uso de `motor.dc()`

```python
velocidad = 25    # potencia base en % de duty cycle
kp = 1.6          # proporcional
ki = 0            # integral (desactivada)
kd = 0            # derivativa (desactivada) → es solo un P controller

error = light_r - light_l   # derecho - izquierdo

P = error
I += error * 0.001           # contador=0.001, acumulación muy lenta
D = (error - error_anterior) / 0.001

correccion = int(P * kp + I * ki + D * kd)  # con ki=kd=0 → solo P*kp

potencia_izquierdo = velocidad - correccion
potencia_derecho   = velocidad + correccion

motor_izquierdo.dc(potencia_izquierdo)
motor_derecho.dc(potencia_derecho)
```

**`motor.dc(n)`** aplica `n` % de duty cycle directamente al motor,
sin control de velocidad (no tiene encoder feedback). Es más rápido que
`motor.run()` pero la velocidad varía con la batería.

**Efectivamente es un P controller:** ki=0 y kd=0 anulan la I y la D.
El `contador=0.001` hace que aunque I y D estén definidas, su impacto
es prácticamente cero incluso si se activaran.

**Timeout de pérdida de línea (6 segundos):**
```python
ultima_deteccion = StopWatch()

# Dentro del seguidor:
if light_r < borde_negro or light_l < borde_negro:
    ultima_deteccion.reset()   # hay línea → resetear

if ultima_deteccion.time() > 6000:  # 6 segundos sin línea
    parar_motores()
    while True:
        mover_motores_indefinido(-70, -70)   # retroceder lento
        if algún_sensor < umbral:
            parar_motores()
            ultima_deteccion.reset()
            break
```

---

## 🔄 Rutinas de movimiento base

### `movermotores(vel_izq, vel_der, rotaciones)` — bloqueante
```python
motor_izquierdo.run_angle(vel_izq, rotaciones*360, wait=False)
motor_derecho.run_angle(vel_der, rotaciones*360, wait=False)
while not motor_izquierdo.done() or not motor_derecho.done():
    wait(10)
```
Ambos motores arrancan simultáneamente (`wait=False`) y luego el programa
espera activamente en el `while`. Esto es bloqueante para el resto del código.
`rotaciones * 360` convierte vueltas a grados de motor.

### `mover_motores_indefinido(vel_izq, vel_der)` — no bloqueante
```python
motor_izquierdo.run(vel_izq)
motor_derecho.run(vel_der)
```
Solo arranca los motores y retorna inmediatamente. Se usa dentro de
`while` propios para control continuo.

### `recorrer_proporcional(angulo, velocidad)` — un tick de corrección
```python
lectura_actual = hub.imu.heading()
kp = 5
error = angulo - lectura_actual
correccion_proporcional = error * kp
base_robot.drive(velocidad, correccion_proporcional)
```
NO es bloqueante. Aplica un solo tick de corrección y retorna.
Se llama repetidamente dentro de un `while` externo para lograr el efecto.

### `recorrer_distancia(mm, angulo, velocidad)` — bloqueante con corrección
```python
base_robot.reset()
while True:
    recorrer_proporcional(angulo, velocidad)
    if abs(base_robot.distance()) >= abs(cantidad_mm):
        base_robot.brake()
        break
```
Avanza una distancia usando DriveBase con corrección de heading en cada tick.
Es bloqueante hasta alcanzar la distancia.

---

## 🔄 Giros con giroscopio

### `giro_90_grados_derecha()` — gira hasta 75° (no 90°)
```python
hub.imu.reset_heading(0)
while hub.imu.heading() < 75:
    mover_motores_indefinido(150, -150)
parar_motores()
wait(300)
```
**Undershoot intencional a 75°:** el momentum hace que el robot siga
girando ~15° más después de parar → llega a ~90° real.
El `wait(300)` deja que el robot se estabilice antes de continuar.

### `giro_90_grados_izquierda()` — gira hasta -65°
```python
hub.imu.reset_heading(0)
while hub.imu.heading() > -65:
    motor_izquierdo.run(-350)
    motor_derecho.run(350)
parar_motores()
wait(300)
```
**Velocidad asimétrica:** usa -350/350 rpm en lugar de ±150.
Más agresivo que el giro a la derecha — sugiere que el robot tiene
tendencia a quedarse corto girando a la izquierda.

### `giro_180_grados_derecha()` — gira hasta 177°
```python
hub.imu.reset_heading(0)
while hub.imu.heading() < 177:
    motor_izquierdo.run(300)
    motor_derecho.run(-300)
parar_motores()
```
Sin `wait()` al final. Menos margen de undershoot (177° vs 180°).

---

## 🟢 `deteccion_señal_verde()` — con doble chequeo

```python
if color_r == Color.GREEN:          # datos del update() anterior
    parar_motores()
    movermotores(100, 100, 0.05)    # avanzar para centrar el robot sobre el verde
    if color_sensor_l.color() == Color.GREEN:   # re-leer en tiempo real
        giro_180_grados_derecha()   # ambos verdes → U-turn
    else:
        movermotores(150, 150, 0.15)
        giro_90_grados_derecha()
        movermotores(150, 150, 0.12)
        buscar_linea_derecha()
```

**Diferencia entre `color_r` (global) y `color_sensor_l.color()` (directo):**
- `color_r` viene del `update()` al inicio del tick — puede tener un tick de lag.
- `color_sensor_l.color()` es lectura inmediata — más fresca pero más lenta.

Esta es la lógica de decisión verde:
| Der | Izq | Acción |
|-----|-----|--------|
| Verde | Verde | U-turn 180° |
| Verde | No verde | Giro 90° derecha |
| No verde | Verde | Giro 90° izquierda |

**BUG en el bloque izquierdo:** La segunda condición usa `elif color_l == Color.GREEN`
(variable global) en lugar de re-leer el sensor. Si llegó al `elif` es porque
`color_l` ya era verde (fue la condición del `if` externo), así que siempre es True.
Nunca se puede llegar al `elif` y que sea False.

---

## ⬛ `doble_negro()` — con doble chequeo

```python
if light_l < centro_negro+1 and light_r < centro_negro+1 and light_f < centro_negro+1:
    parar_motores()
    movermotores(70, 70, 0.04)     # avanzar un pelo
    if light_l < ... and light_r < ... and light_f < ...:  # re-verificar
        hub.display.text("DN")
        movermotores(70, 70, 0.3)  # cruzar recto la intersección
```

**Por qué los 3 sensores:** En RCJ, el doble negro (fin de línea o cruce total)
se confirma cuando todos los sensores ven negro simultáneamente.
El sensor frontal es el desempate — si solo los laterales ven negro, puede ser
una T-intersección, no un doble negro real.

---

## 🚦 `T_derecha()` y `T_izquierda()` — con anti-verde

```python
def T_derecha():
    if light_r < negro_derecha+5 and light_f < negro_frontal+5:
        parar_motores()
        movermotores(100, 100, 0.03)
        if color_sensor_r.reflection() < negro_derecha+5 and ...:
            # Anti-confusión verde/negro:
            if color_sensor_f.color() == Color.GREEN or color_sensor_r.color() == Color.GREEN:
                movermotores(-100, -100, 0.2)  # era verde, retroceder
            else:
                wait(1000)                      # confirmado: es T
                movermotores(70, 70, 0.3)       # avanzar al centro de la T
                buscar_linea_izquierda()        # buscar la línea izquierda
```

**`wait(1000)` de 1 segundo antes de cruzar:** Le da tiempo al robot de
estabilizarse completamente antes de ejecutar el cruce. Es un "respiro"
operativo que evita que el momentum del frenado afecte el giro siguiente.

**Anti-confusión verde/negro:** Los umbrales de reflection de verde oscuro
y negro son similares. Por eso después de detectar por reflection, se
verifica con `.color()` (clasificación HSV) para filtrar los falsos positivos.

**BUG de indentación en `T_izquierda()`:** El segundo `if` está al mismo
nivel que el primero (fuera del bloque). Esto significa que la segunda
verificación siempre se ejecuta, independientemente del resultado del
primer `if`. Reduce la protección del doble chequeo.

---

## 🔍 `buscar_linea_derecha()` y `buscar_linea_izquierda()`

```python
def buscar_linea_derecha():
    hub.imu.reset_heading(0)
    while hub.imu.heading() < 45:     # girar hacia la derecha 45°
        mover_motores_indefinido(180, -180)
    parar_motores()
    while color_sensor_r.reflection() > (centro_negro+5):  # buscar negro con sensor derecho
        mover_motores_indefinido(-180, 180)   # girar de vuelta
    parar_motores()
    while color_sensor_l.reflection() > (centro_negro+5):  # alinear con sensor izquierdo
        mover_motores_indefinido(180, -180)   # micro-ajuste final
    parar_motores()
```

**Secuencia de 3 fases:**
1. Girar 45° hacia el lado esperado (overshooting la posición de la línea)
2. Girar de vuelta hasta que el sensor del lado detecte negro (encuentra la línea)
3. Micro-ajuste con el otro sensor para quedar centrado en la línea

**El código comentado en `buscar_linea_izquierda()`:**
```python
"""while color_sensor_f.reflection() > cenntro_negro_frente:
    mover_motores_indefinido(60,-60)"""
```
Había una 4ta fase con el sensor frontal para ajuste fino que fue eliminada.
Probablemente causaba overshooting en el ajuste final.

---

## 🧱 `buscar_pared()` — sondeo para planear la zona de rescate

Esta es la función más original del sistema. Antes de entrar a la zona
de rescate, el robot necesita saber hacia dónde está orientado para
elegir el recorrido correcto.

```python
def buscar_pared():
    # Giro 90° a la derecha
    while hub.imu.heading() < 85:
        mover_motores_indefinido(300, -300)
    parar_motores()

    if distance_sensor.distance() < 500:  # hay pared a la derecha
        # Acercarse hasta 52mm de la pared
        while distance_sensor.distance() > 52:
            mover_motores_indefinido(150, 150)
        parar_motores()
        # Volver al heading original (0°)
        while hub.imu.heading() > 2:
            mover_motores_indefinido(-300, 300)
        parar_motores()
        return "izquierda"   # la pared cercana está a la derecha → se recorre por izquierda

    else:  # no hay pared a la derecha → buscar a la izquierda
        while hub.imu.heading() > -85:
            mover_motores_indefinido(-300, 300)
        parar_motores()
        while distance_sensor.distance() > 52:
            mover_motores_indefinido(150, 150)
        parar_motores()
        while hub.imu.heading() < 2:
            mover_motores_indefinido(300, -300)
        parar_motores()
        return "derecha"
```

**Lógica:** La zona de evacuación tiene forma rectangular con 4 paredes.
El robot entra por una apertura en una de las paredes cortas.
Girando 90° y midiendo distancia, detecta si está cerca de una pared lateral
→ eso le dice cuál es su orientación → elige el plan de recorrido.

**Por qué `< 500`:** El sensor ultrasónico retorna ~2000mm cuando no hay
pared detectada. Cualquier valor menor a 500mm indica pared cercana.

**El retorno de la función:** `"izquierda"` o `"derecha"` determina en qué
dirección se hacen los giros dentro de `rescate()`.

---

## 🏁 `rescate()` — algoritmo adaptativo por esquinas

Esta es la función más compleja del sistema. Implementa un recorrido
de la zona de evacuación que se adapta según en qué pared encuentre
el color negro (indicador de víctima).

### Variables de estado:
```python
esquina = 0   # 1, 2 o 3 según en qué pared encontró negro
orientacion   # "izquierda" o "derecha" desde buscar_pared()
```

### Estructura general (caso orientacion == "izquierda"):

```
INICIO
  │
  ├── movermotores(200,200,1)  ← avanzar 1 vuelta para alejarse de la entrada
  ├── buscar_pared()           ← determinar orientación
  │
  └── while True:  ← loop de exploración de paredes
        │
        ├── Avanzar hasta pared frontal (dist < 59mm)
        │     → si negro: esquina=1, break
        │
        ├── Girar 90° a la izquierda
        ├── Recorrer 850mm (pared larga)
        │     → si negro: esquina=2, break
        │
        ├── Girar 90° a la derecha
        ├── Avanzar hasta pared frontal
        │     esquina=3, break (siempre termina aquí si llegó)
```

**Después del loop:** Independientemente de en qué esquina se encontró
negro, el robot se reposiciona para recorrer las otras 2 paredes:

```
Retroceder hasta distancia > 128mm de la pared actual
→ Girar 45° en diagonal
→ Avanzar para acercarse a la tercera pared
→ Girar hasta quedar paralelo a la tercera pared
→ Recorrer tercera pared hasta encontrarla
→ Girar hasta cuarta pared
→ Recorrer cuarta pared hasta encontrarla
→ Girar en diagonal y avanzar hacia el centro
```

### Por qué los ángulos diagonales (125°, 135°):
El robot no puede simplemente girar 90° entre paredes porque está posicionado
en una esquina. Los giros diagonales (~45° adicionales) son atajos geométricos
para cruzar de una pared a la otra pasando por el centro de la zona.

### Detección de víctima en zona:
```python
if (color_sensor_l.color() == Color.BLACK or
    color_sensor_r.color() == Color.BLACK or
    color_sensor_f.color() == Color.BLACK):
    esquina = N
    break
```
Usa color BLACK porque en la zona de evacuación las víctimas negras están
sobre fondo blanco — al acercarse a una pared, los sensores detectan negro
cuando están sobre o muy cerca de una víctima.

---

## 🚧 `obstaculo()` — esquive por la izquierda

```python
def obstaculo():
    if -1 < distance < 100:        # obstáculo a menos de 10cm (mm reales)
        hub.speaker.beep(700, 30)
        if -1 < distance_sensor.distance() < 100:   # doble chequeo
            movermotores(-100, -100, 0.1)   # retroceder
            hub.imu.reset_heading(0)
            while hub.imu.heading() > -90:
                mover_motores_indefinido(-100, 100)  # girar 90° izquierda
            parar_motores()
            movermotores(100, 100, 0.1)
            while color_sensor_l.reflection() > centro_negro:
                mover_motores_indefinido(350, 132)   # avanzar sesgado buscando línea
            parar_motores()
```

**El doble chequeo anti-falso-positivo:** El ultrasónico puede dar lecturas
espurias. Se verifica dos veces antes de ejecutar el esquive.

**`mover_motores_indefinido(350, 132)`:** Velocidades asimétricas hacen que
el robot avance en diagonal, barriendo hacia la línea.

**BUG:** La última línea `buscar_linea_izquierda` (sin paréntesis) es una
referencia a la función, no una llamada. Nunca se ejecuta.

---

## 🧵 Modelo de ejecución: bloqueante vs no bloqueante

| Función | Tipo | Duración estimada |
|---|---|---|
| `update()` | Bloqueante corto | ~20-50ms (6 lecturas sensor) |
| `seguidor_de_linea()` | No bloqueante | ~1ms |
| `giro_90_grados_derecha()` | Bloqueante | ~800-1200ms |
| `giro_180_grados_derecha()` | Bloqueante | ~1500-2000ms |
| `buscar_linea_derecha()` | Bloqueante | ~1-3 segundos |
| `movermotores(v,v,0.3)` | Bloqueante | ~300-800ms |
| `wait(1000)` en T_derecha | Bloqueante puro | 1000ms exactos |
| `rescate()` | Bloqueante total | ~30-120 segundos |
| `buscar_pared()` | Bloqueante | ~3-5 segundos |

**Consecuencia del wait(1000) en T_derecha/T_izquierda:**
Durante ese segundo de pausa, NADA se ejecuta. Si hay un marcador verde
justo en la intersección, no se va a detectar durante ese tiempo.

---

## 📝 Valores clave para replicar

```python
# Geometría del robot
wheel_diameter = 56      # mm
axle_track     = 140     # mm (más ancho que el robot MARIVIR)

# PID (efectivamente solo P)
velocidad = 25           # duty cycle base %
kp        = 1.6          # proporcional
# ki = kd = 0 → control proporcional puro

# Timeout línea
timeout_ms = 6000        # 6 segundos sin línea → retroceder

# Giros (undershoot intencional)
giro_der_target = 75     # en lugar de 90° (momentum ≈ 15°)
giro_izq_target = -65    # en lugar de -90° (momentum ≈ 25°, motores más rápidos)
giro_180_target = 177    # en lugar de 180°

# Rescate
dist_pared_cerca    = 59   # mm — cuando parar frente a una pared
dist_recorrido_largo = 850  # mm — lado largo de la zona de evacuación
dist_retroceso_repo  = 128  # mm — retroceder para reposicionarse entre paredes
```

---

*Referencia técnica — IITA Vibe Robotics — Uso interno*
