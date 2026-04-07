"""
RCJ Rescue Line — Seguidor de línea competitivo
================================================

LEGO Spike Prime + Pybricks (MicroPython)

Cubre los elementos típicos de la pista RoboCup Junior Rescue Line:
  - Line following PID con dos ColorSensor normalizados
  - Detección de marcadores verdes (HSV) → giro automático
  - Recuperación de gaps con gyro
  - Detección de obstáculos con ultrasonido → esquive rectangular
  - Detección de rampa con IMU tilt → modo lento
  - Detección de cinta plateada (entrada de evacuación) → fin de pista
  - Calibración interactiva de sensores con botones del hub
  - State machine sencilla por prioridad de eventos

NO cubre (queda para programas separados):
  - Búsqueda y depósito de víctimas dentro de la zona de evacuación.
  - Speed bumps (suelen pasar bien sin lógica especial a velocidad moderada).

Hardware asumido (cambiar puertos según tu robot):
  - Port A: motor izquierdo (Direction.COUNTERCLOCKWISE para que +speed = adelante)
  - Port B: motor derecho
  - Port C: motor de attachment (RESERVADO para programa de evacuación, no se usa acá)
  - Port D: UltrasonicSensor frontal
  - Port E: ColorSensor IZQUIERDO mirando al piso
  - Port F: ColorSensor DERECHO mirando al piso

Calibración mínima ANTES del primer run:
  1. Sensores (auto): mantener LEFT presionado al inicio para entrar a calibración.
  2. wheel_diameter / axle_track: calibrar empíricamente con straight(1000) y turn(360).
  3. GREEN_HUE_*: leer sensor.hsv() sobre el marcador verde real y ajustar rango.

Iniciar un run:
  1. Encender el hub. Esperar el icono de flecha.
  2. (Opcional) Mantener LEFT presionado los primeros 1.5 seg para recalibrar sensores.
  3. Posicionar el robot sobre la línea negra, frente a la dirección de marcha.
  4. Presionar CENTER para arrancar.
"""

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor, UltrasonicSensor
from pybricks.parameters import Port, Direction, Stop, Color, Button, Axis
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

# ============================================================================
# CONSTANTES — todas calibrables. Ajustar para tu robot y la cancha real.
# ============================================================================

# --- Geometría del robot (calibrar empíricamente) ---
WHEEL_DIAMETER_MM = 56.0
AXLE_TRACK_MM = 114.0

# --- Velocidades (mm/s) ---
BASE_SPEED = 180          # crucero en line following
SLOW_SPEED = 100          # rampas, esquives
SEARCH_SPEED = 80         # barrido / recuperación de gap

# --- PID line following ---
KP = 1.5
KI = 0.0                  # mantener en 0 para line following (ver skill robotics-control-theory)
KD = 6.0

# --- Calibración de sensores de color (sobreescritos por calibrate()) ---
WHITE_LEFT = 90
BLACK_LEFT = 8
WHITE_RIGHT = 90
BLACK_RIGHT = 8

# --- Detección de marcadores verdes (HSV — más robusto que .color()) ---
GREEN_HUE_MIN = 90
GREEN_HUE_MAX = 150
GREEN_SAT_MIN = 40
GREEN_VAL_MIN = 30

# --- Detección de gap (todo blanco) ---
WHITE_NORM_THRESHOLD = 70   # valor normalizado >70 = piso blanco
LINE_NORM_THRESHOLD = 30    # valor normalizado <30 = línea negra recuperada

# --- Detección de cinta plateada (entrada zona de evacuación) ---
SILVER_VAL_MIN = 80         # plateado tiene alta luminosidad
SILVER_SAT_MAX = 15         # ...y baja saturación HSV

# --- Detección de obstáculos ---
OBSTACLE_DISTANCE_MM = 80
OBSTACLE_BYPASS_LATERAL = 150
OBSTACLE_BYPASS_FORWARD = 400

# --- Recuperación de gap ---
GAP_MAX_RECOVERY_MM = 250

# --- Detección de rampa ---
RAMP_TILT_DEG_ENTER = 8     # threshold para entrar al modo rampa
RAMP_TILT_DEG_EXIT = 3      # threshold más bajo para salir (evita oscilación)
RAMP_KP_MULT = 1.3          # más Kp en rampa porque los motores cargan más

# --- Loop ---
LOOP_MS = 10                # 100 Hz

# ============================================================================
# HARDWARE
# ============================================================================

hub = PrimeHub()
left_motor = Motor(Port.A, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.B)
color_left = ColorSensor(Port.E)
color_right = ColorSensor(Port.F)
us_front = UltrasonicSensor(Port.D)

drive = DriveBase(left_motor, right_motor, WHEEL_DIAMETER_MM, AXLE_TRACK_MM)
drive.use_gyro(True)

# ============================================================================
# HELPERS GENÉRICOS
# ============================================================================

def normalize(value, white, black):
    """Normaliza una lectura de reflection a 0-100 según calibración del sensor."""
    if white == black:
        return 50
    n = (value - black) * 100 // (white - black)
    if n < 0:
        return 0
    if n > 100:
        return 100
    return n

def wait_for_button(button=Button.CENTER):
    """Espera press + release del botón. Bloqueante."""
    while button not in hub.buttons.pressed():
        wait(10)
    while button in hub.buttons.pressed():
        wait(10)

def beep_ok():
    hub.light.on(Color.GREEN)
    hub.speaker.beep(frequency=880, duration=100)

def beep_error():
    hub.light.on(Color.RED)
    hub.speaker.beep(frequency=200, duration=300)

def is_green(sensor):
    """Detecta marcador verde por HSV. Más robusto que .color() porque no
    confunde verde oscuro con negro."""
    h, s, v = sensor.hsv().h, sensor.hsv().s, sensor.hsv().v
    return GREEN_HUE_MIN < h < GREEN_HUE_MAX and s > GREEN_SAT_MIN and v > GREEN_VAL_MIN

def is_silver(sensor):
    """Detecta cinta plateada por HSV: alta luminosidad y baja saturación.
    Esto separa plateado del blanco normal del piso (que tiene saturación cercana a 0
    pero el plateado además tiene mayor luminosidad metálica).
    """
    hsv = sensor.hsv()
    return hsv.v > SILVER_VAL_MIN and hsv.s < SILVER_SAT_MAX

# ============================================================================
# CALIBRACIÓN INTERACTIVA DE SENSORES
# ============================================================================

def calibrate():
    """Calibra WHITE/BLACK de los dos ColorSensor con ayuda de los botones.
    Posicionar el sensor sobre blanco y presionar CENTER, después sobre negro.
    """
    global WHITE_LEFT, BLACK_LEFT, WHITE_RIGHT, BLACK_RIGHT

    # Blanco
    hub.display.text("W")
    hub.light.on(Color.WHITE)
    wait_for_button()
    sl, sr = 0, 0
    for _ in range(5):
        sl += color_left.reflection()
        sr += color_right.reflection()
        wait(50)
    WHITE_LEFT = sl // 5
    WHITE_RIGHT = sr // 5
    beep_ok()

    # Negro
    hub.display.text("B")
    hub.light.on(Color.BLACK)
    wait_for_button()
    sl, sr = 0, 0
    for _ in range(5):
        sl += color_left.reflection()
        sr += color_right.reflection()
        wait(50)
    BLACK_LEFT = sl // 5
    BLACK_RIGHT = sr // 5
    beep_ok()

    # Mostrar y log
    hub.display.text("OK")
    print("WHITE_LEFT={} BLACK_LEFT={}".format(WHITE_LEFT, BLACK_LEFT))
    print("WHITE_RIGHT={} BLACK_RIGHT={}".format(WHITE_RIGHT, BLACK_RIGHT))
    wait(800)

# ============================================================================
# LINE FOLLOWING (módulo principal)
# ============================================================================

def line_follow_step(last_error):
    """Un tick del PD line follower con dos sensores.
    Devuelve el nuevo last_error para encadenar en el loop.

    Lógica del error:
      l > r  → izq ve más blanco que der → robot se está yendo a la izquierda
             → error positivo → turn_rate positivo → girar a la derecha (compensar)
    """
    l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
    r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)
    error = l - r
    derivative = error - last_error
    turn_rate = KP * error + KD * derivative
    drive.drive(BASE_SPEED, turn_rate)
    return error

# ============================================================================
# DETECTOR DE EVENTOS
# ============================================================================

def detect_event():
    """Devuelve el evento detectado o None. Las prioridades importan:
    rampa primero (afecta toda la dinámica), después obstáculos físicos,
    después marcadores especiales, después gaps.
    """
    # Prioridad 1: rampa (cambia el régimen de control)
    pitch = hub.imu.tilt()[0]
    if abs(pitch) > RAMP_TILT_DEG_ENTER:
        return 'ramp'

    # Prioridad 2: obstáculo físico al frente
    if us_front.distance() < OBSTACLE_DISTANCE_MM:
        return 'obstacle'

    # Prioridad 3: cinta plateada (entrada de evacuación = fin de pista)
    if is_silver(color_left) or is_silver(color_right):
        return 'silver'

    # Prioridad 4: marcador verde (intersección señalizada)
    if is_green(color_left) or is_green(color_right):
        return 'green'

    # Prioridad 5: gap (todo blanco bajo ambos sensores)
    l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
    r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)
    if l > WHITE_NORM_THRESHOLD and r > WHITE_NORM_THRESHOLD:
        return 'gap'

    return None

# ============================================================================
# MANEJADORES DE EVENTOS
# ============================================================================

def handle_green():
    """Marcador verde detectado: leer cuántos verdes hay y girar en consecuencia.
    - verde a la izquierda → girar 90° a la izquierda
    - verde a la derecha → girar 90° a la derecha
    - verde a ambos lados → girar 180°
    """
    drive.stop()
    drive.straight(20)  # avanzar un toque para leer ambos sensores sobre la marca

    left_green = is_green(color_left)
    right_green = is_green(color_right)

    drive.straight(50)  # avanzar al centro de la intersección antes de girar

    if left_green and right_green:
        drive.turn(180)
    elif left_green:
        drive.turn(-90)
    elif right_green:
        drive.turn(90)
    # Si en el segundo check ya no hay verde, continuar recto (falsa alarma).

def sweep_for_line():
    """Barrido en abanico buscando la línea cuando el avance recto no alcanza."""
    for angle in [30, -60, 90, -120]:
        drive.turn(angle)
        l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
        r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)
        if l < LINE_NORM_THRESHOLD or r < LINE_NORM_THRESHOLD:
            return True
    return False

def handle_gap():
    """Avanza recto manteniendo heading hasta encontrar la línea.
    Si después de GAP_MAX_RECOVERY_MM no la encontró, hace un sweep en abanico.
    Devuelve True si recuperó la línea, False si no.
    """
    initial_heading = hub.imu.heading()
    drive.reset()

    while drive.distance() < GAP_MAX_RECOVERY_MM:
        # Mantener heading absoluto durante el cruce a ciegas
        heading_error = hub.imu.heading() - initial_heading
        drive.drive(SEARCH_SPEED, -heading_error * 2)

        l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
        r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)

        if l < LINE_NORM_THRESHOLD or r < LINE_NORM_THRESHOLD:
            drive.stop()
            return True

        wait(LOOP_MS)

    # No encontramos en línea recta — barrer en abanico
    drive.stop()
    return sweep_for_line()

def handle_obstacle():
    """Esquiva el obstáculo en patrón rectangular y vuelve a la línea.
    Asume que se puede esquivar por la derecha. Para versiones avanzadas,
    detectar primero de qué lado conviene esquivar.
    """
    drive.stop()
    drive.turn(90)                              # 1) girar 90° a la derecha
    drive.straight(OBSTACLE_BYPASS_LATERAL)     # 2) avanzar lateralmente
    drive.turn(-90)                             # 3) apuntar de nuevo hacia adelante

    # 4) avanzar paralelo al obstáculo hasta encontrar la línea de nuevo
    drive.reset()
    while drive.distance() < OBSTACLE_BYPASS_FORWARD:
        drive.drive(SLOW_SPEED, 0)
        l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
        r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)
        if l < LINE_NORM_THRESHOLD or r < LINE_NORM_THRESHOLD:
            drive.stop()
            return
        wait(LOOP_MS)

    # Si llegamos al máximo sin encontrar la línea, cerrar el rectángulo
    drive.stop()
    drive.turn(-90)
    drive.straight(OBSTACLE_BYPASS_LATERAL)
    drive.turn(90)

def handle_ramp():
    """En rampa: line following más lento y con Kp un poco más alto.
    Sale del modo rampa cuando el tilt vuelve a ser pequeño.
    El threshold de salida es más bajo que el de entrada para evitar oscilación.
    """
    last_error = 0
    while abs(hub.imu.tilt()[0]) > RAMP_TILT_DEG_EXIT:
        l = normalize(color_left.reflection(), WHITE_LEFT, BLACK_LEFT)
        r = normalize(color_right.reflection(), WHITE_RIGHT, BLACK_RIGHT)
        error = l - r
        derivative = error - last_error
        last_error = error
        turn_rate = (KP * RAMP_KP_MULT) * error + KD * derivative
        drive.drive(SLOW_SPEED, turn_rate)
        wait(LOOP_MS)

def handle_silver():
    """Cinta plateada detectada → entrada de la zona de evacuación = fin de pista.
    Acá termina este programa. La lógica de evacuación (búsqueda de víctimas,
    clasificación plateada/negra, depósito en triángulo) va en otro programa.
    """
    drive.stop()
    hub.light.on(Color.BLUE)
    hub.display.text("EV")
    hub.speaker.beep(frequency=440, duration=200)
    hub.speaker.beep(frequency=660, duration=200)
    hub.speaker.beep(frequency=880, duration=200)

# ============================================================================
# LOOP PRINCIPAL
# ============================================================================

def main():
    # Splash
    hub.display.icon([
        [0, 100, 0, 100, 0],
        [100, 100, 100, 100, 100],
        [100, 100, 100, 100, 100],
        [0, 100, 100, 100, 0],
        [0, 0, 100, 0, 0],
    ])

    # Verificación de batería
    if hub.battery.voltage() < 7500:
        beep_error()
        hub.display.text("BAT")
        wait(2000)

    # Si LEFT está presionado durante el splash → calibrar sensores
    hub.display.text("?")
    wait(1500)
    if Button.LEFT in hub.buttons.pressed():
        calibrate()

    # Esperar arranque del run
    hub.display.icon([
        [0, 0, 100, 0, 0],
        [0, 100, 100, 100, 0],
        [100, 100, 100, 100, 100],
        [0, 100, 100, 100, 0],
        [0, 0, 100, 0, 0],
    ])
    wait_for_button()
    hub.imu.reset_heading(0)
    drive.reset()
    beep_ok()

    # Loop principal con state machine por evento
    last_error = 0
    while True:
        event = detect_event()

        if event == 'ramp':
            handle_ramp()
            last_error = 0
        elif event == 'obstacle':
            handle_obstacle()
            last_error = 0
        elif event == 'silver':
            handle_silver()
            return  # fin del programa, entra el módulo de evacuación
        elif event == 'green':
            handle_green()
            last_error = 0
        elif event == 'gap':
            recovered = handle_gap()
            if not recovered:
                hub.light.on(Color.RED)
                hub.display.text("X")
                drive.stop()
                return
            last_error = 0
        else:
            # Line following normal
            last_error = line_follow_step(last_error)

        wait(LOOP_MS)


main()
