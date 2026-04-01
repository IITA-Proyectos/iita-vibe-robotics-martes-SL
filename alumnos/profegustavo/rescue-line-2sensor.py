# ╔══════════════════════════════════════════════════════════════════╗
# ║  RESCUE LINE v1.0 — RoboCup Junior                             ║
# ║  Robot: spike-2wd-basico (E izq, F der) + 2 sensores color     ║
# ║  Sensores: Puerto C (izquierdo), Puerto D (derecho)            ║
# ║  Máquina de estados + PID + detección verde HSV                ║
# ║  Autor: Taller Vibe Robotics IITA — 2026                       ║
# ╚══════════════════════════════════════════════════════════════════╝

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# HARDWARE
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F)
sensor_izq = ColorSensor(Port.C)
sensor_der = ColorSensor(Port.D)

DIAMETRO_RUEDA = 56.0    # mm
DISTANCIA_EJES = 112.0   # mm
MM_PER_DEG = 3.14159265 * DIAMETRO_RUEDA / 360.0

robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)
robot.use_gyro(True)

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# PARÁMETROS DE CALIBRACIÓN — AJUSTAR EN CAMPO
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# ── Umbrales de reflexión ─────────────────────────────────────────
UMBRAL_NEGRO = 20        # Reflexión máxima para considerar "negro"
UMBRAL_BLANCO = 65       # Reflexión mínima para considerar "blanco"

# ── Detección de verde (HSV) ──────────────────────────────────────
HUE_MIN = 100            # Hue mínimo para verde (←CALIBRAR)
HUE_MAX = 200            # Hue máximo para verde (←CALIBRAR)
SAT_MIN = 25             # Saturación mínima (descarta grises)
V_MIN = 12               # Value mínimo (descarta negro)
V_MAX = 80               # Value máximo (descarta blanco brillante)
GREEN_CONFIRM = 3        # Lecturas consecutivas para confirmar verde

# ── PID de line following ─────────────────────────────────────────
KP = 1.5                 # Proporcional
KI = 0.02                # Integral
KD = 8.0                 # Derivativo
INTEGRAL_MAX = 50        # Anti-windup del integral

# ── Velocidades (mm/s) ───────────────────────────────────────────
VEL_MAX = 200            # Velocidad en recta
VEL_MIN = 80             # Velocidad mínima en curvas cerradas
VEL_FACTOR = 2.0         # Factor de reducción por error
VEL_SCAN = 100           # Velocidad al escanear verde
VEL_BUSCAR = 120         # Velocidad al buscar línea

# ── PID de giro ──────────────────────────────────────────────────
GIRO_KP = 6.0
GIRO_KI = 0.15
GIRO_KD = 8.0
GIRO_MAX_RATE = 500
GIRO_SETTLE_ERR = 1.0
GIRO_SETTLE_N = 5
GIRO_TIMEOUT = 2500      # ms

# ── Distancias de maniobra (mm) ──────────────────────────────────
AVANCE_POST_VERDE = 45   # Avanzar tras detectar verde, antes de girar
AVANCE_POST_GIRO = 20    # Avanzar tras girar, buscando línea
GAP_MAX_MM = 250         # Distancia máxima de gap antes de declarar pérdida
BUSCAR_TIMEOUT = 3000    # ms para buscar línea antes de declarar pérdida

# ── Dead end ─────────────────────────────────────────────────────
DEAD_END_CICLOS = 30     # Ciclos con ambos negro = dead end (~300ms)

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# ESTADOS
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

FOLLOW_LINE = 0
DETECTED_GREEN = 1
TURN_LEFT = 2
TURN_RIGHT = 3
U_TURN = 4
GO_STRAIGHT = 5
GAP_CROSS = 6
DEAD_END = 7
SEARCH_LINE = 8
STOP = 9

NOMBRES_ESTADO = [
    "FOLLOW_LINE", "DETECTED_GREEN", "TURN_LEFT", "TURN_RIGHT",
    "U_TURN", "GO_STRAIGHT", "GAP_CROSS", "DEAD_END",
    "SEARCH_LINE", "STOP"
]

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# FUNCIONES AUXILIARES
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def clamp(val, minv, maxv):
    """Limitar un valor entre mínimo y máximo."""
    if val < minv:
        return minv
    if val > maxv:
        return maxv
    return val


def es_verde(sensor):
    """Detectar si el sensor está sobre un cuadrado verde usando HSV."""
    h, s, v = sensor.hsv()
    return (HUE_MIN <= h <= HUE_MAX and
            s >= SAT_MIN and
            V_MIN <= v <= V_MAX)


def es_negro(reflexion):
    """True si la reflexión indica superficie negra."""
    return reflexion < UMBRAL_NEGRO


def es_blanco(reflexion):
    """True si la reflexión indica superficie blanca."""
    return reflexion > UMBRAL_BLANCO


def confirmar_verde_izq():
    """Confirmar que el sensor izquierdo ve verde (múltiples lecturas)."""
    count = 0
    for _ in range(GREEN_CONFIRM + 2):
        if es_verde(sensor_izq):
            count += 1
        wait(8)
    return count >= GREEN_CONFIRM


def confirmar_verde_der():
    """Confirmar que el sensor derecho ve verde (múltiples lecturas)."""
    count = 0
    for _ in range(GREEN_CONFIRM + 2):
        if es_verde(sensor_der):
            count += 1
        wait(8)
    return count >= GREEN_CONFIRM


def avanzar_mm(distancia, velocidad=150):
    """Avanzar una distancia fija manteniendo heading actual."""
    heading_fijo = hub.imu.heading()
    motor_izq.reset_angle(0)
    motor_der.reset_angle(0)
    while True:
        dist = (abs(motor_izq.angle()) + abs(motor_der.angle())) / 2.0 * MM_PER_DEG
        if dist >= distancia:
            break
        h_err = heading_fijo - hub.imu.heading()
        robot.drive(velocidad, h_err * 3.0)
        wait(10)
    robot.stop()


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# GIRO PID (sobre giroscopio)
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def girar_pid(heading_objetivo):
    """Girar hasta heading_objetivo usando PID sobre el gyro."""
    err_prev = heading_objetivo - hub.imu.heading()
    integral = 0.0
    settled = 0
    reloj = StopWatch()

    while True:
        h = hub.imu.heading()
        err = heading_objetivo - h

        # Proporcional
        p = GIRO_KP * err

        # Integral (solo cerca del target)
        if abs(err) < 15:
            integral = clamp(integral + GIRO_KI * err, -30, 30)
        else:
            integral *= 0.3

        # Derivativo
        d = GIRO_KD * (err - err_prev)
        err_prev = err

        # Output
        out = clamp(p + integral + d, -GIRO_MAX_RATE, GIRO_MAX_RATE)
        if abs(err) < GIRO_SETTLE_ERR and abs(out) < 5:
            out = 0

        robot.drive(0, out)

        # Verificar si se estabilizó
        if abs(err) < GIRO_SETTLE_ERR:
            settled += 1
            if settled >= GIRO_SETTLE_N:
                break
        else:
            settled = 0

        # Timeout de seguridad
        if reloj.time() > GIRO_TIMEOUT:
            print("GIRO: timeout!")
            break

        wait(10)

    robot.stop()


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# BÚSQUEDA DE LÍNEA POST-GIRO
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def buscar_linea_adelante(timeout_ms=2000):
    """Avanzar lento hasta encontrar la línea."""
    reloj = StopWatch()
    heading_fijo = hub.imu.heading()

    while reloj.time() < timeout_ms:
        ref_i = sensor_izq.reflection()
        ref_d = sensor_der.reflection()

        if es_negro(ref_i) or es_negro(ref_d):
            robot.stop()
            return True

        h_err = heading_fijo - hub.imu.heading()
        robot.drive(VEL_BUSCAR, h_err * 2.5)
        wait(10)

    robot.stop()
    return False


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# PROCESAMIENTO DE INTERSECCIÓN
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def procesar_interseccion():
    """
    Al detectar verde, frenar, escanear, y decidir giro.
    Retorna el nuevo estado: TURN_LEFT, TURN_RIGHT, U_TURN, o FOLLOW_LINE.
    """
    robot.stop()
    wait(50)

    # Escanear verde en ambos lados
    verde_izq = confirmar_verde_izq()
    verde_der = confirmar_verde_der()

    print("VERDE: izq=", verde_izq, "der=", verde_der)

    if verde_izq and verde_der:
        return U_TURN
    elif verde_izq:
        return TURN_LEFT
    elif verde_der:
        return TURN_RIGHT
    else:
        # Falsa alarma: vimos verde pero no se confirma
        return FOLLOW_LINE


def escanear_verde_avanzando():
    """
    Avanzar lentamente sobre la zona verde, escaneando ambos sensores.
    Retorna (verde_izq, verde_der) acumulados.
    
    NOTA: El verde puede estar ligeramente antes o después. Avanzamos
    ~AVANCE_POST_VERDE mm escaneando continuamente para no perderlo.
    """
    verde_izq_visto = False
    verde_der_visto = False
    lecturas_izq = 0
    lecturas_der = 0

    heading_fijo = hub.imu.heading()
    motor_izq.reset_angle(0)
    motor_der.reset_angle(0)

    while True:
        dist = (abs(motor_izq.angle()) + abs(motor_der.angle())) / 2.0 * MM_PER_DEG
        if dist >= AVANCE_POST_VERDE + 30:  # Escanear un poco más allá
            break

        # Leer verde continuamente mientras avanzamos
        if es_verde(sensor_izq):
            lecturas_izq += 1
            if lecturas_izq >= GREEN_CONFIRM:
                verde_izq_visto = True
        
        if es_verde(sensor_der):
            lecturas_der += 1
            if lecturas_der >= GREEN_CONFIRM:
                verde_der_visto = True

        h_err = heading_fijo - hub.imu.heading()
        robot.drive(VEL_SCAN, h_err * 2.5)
        wait(10)

    robot.stop()
    return verde_izq_visto, verde_der_visto


def ejecutar_giro(angulo):
    """Ejecutar giro completo: avanzar al centro, girar, buscar línea."""
    heading_actual = hub.imu.heading()
    heading_objetivo = heading_actual + angulo

    print("GIRO:", angulo, "° → heading objetivo:", heading_objetivo)

    # Avanzar un poco para centrar el robot en la intersección
    avanzar_mm(AVANCE_POST_GIRO, VEL_SCAN)

    # Girar con PID sobre gyro
    girar_pid(heading_objetivo)

    # Buscar línea después del giro
    encontro = buscar_linea_adelante(2000)
    if not encontro:
        print("ADVERTENCIA: no encontró línea post-giro")
    
    return encontro


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# CRUCE DE GAP
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def cruzar_gap():
    """Avanzar recto con heading fijo hasta reencontrar la línea."""
    heading_fijo = hub.imu.heading()
    motor_izq.reset_angle(0)
    motor_der.reset_angle(0)

    print("GAP: cruzando con heading", heading_fijo)

    while True:
        dist = (abs(motor_izq.angle()) + abs(motor_der.angle())) / 2.0 * MM_PER_DEG

        if dist > GAP_MAX_MM:
            print("GAP: excedió máximo, línea no encontrada")
            robot.stop()
            return False

        ref_i = sensor_izq.reflection()
        ref_d = sensor_der.reflection()

        if es_negro(ref_i) or es_negro(ref_d):
            print("GAP: línea encontrada a", int(dist), "mm")
            robot.stop()
            return True

        h_err = heading_fijo - hub.imu.heading()
        robot.drive(VEL_BUSCAR, h_err * 2.5)
        wait(10)

    robot.stop()
    return False


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# BÚSQUEDA DE LÍNEA (patrón zig-zag)
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


def buscar_linea_zigzag():
    """Buscar línea con patrón de giro alternado creciente."""
    heading_base = hub.imu.heading()
    reloj = StopWatch()
    angulos = [30, -60, 90, -120, 150, -180]

    for ang in angulos:
        if reloj.time() > BUSCAR_TIMEOUT:
            print("BUSCAR: timeout, no encontró línea")
            return False

        heading_target = heading_base + ang
        # Girar hacia el ángulo
        girar_pid(heading_target)

        # Avanzar un poco en esa dirección
        motor_izq.reset_angle(0)
        motor_der.reset_angle(0)
        while True:
            dist = (abs(motor_izq.angle()) + abs(motor_der.angle())) / 2.0 * MM_PER_DEG
            if dist > 60:
                break

            ref_i = sensor_izq.reflection()
            ref_d = sensor_der.reflection()
            if es_negro(ref_i) or es_negro(ref_d):
                print("BUSCAR: línea encontrada!")
                robot.stop()
                return True

            robot.drive(100, 0)
            wait(10)

        robot.stop()

    print("BUSCAR: no encontró línea después de zigzag")
    return False


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# LOOP PRINCIPAL — MÁQUINA DE ESTADOS
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# Inicialización
hub.light.on(Color.RED)
motor_izq.stop()
motor_der.stop()

# Esperar calibración del gyro
while not hub.imu.ready():
    wait(100)

hub.imu.reset_heading(0)
wait(500)

hub.light.on(Color.GREEN)
hub.speaker.beep(800, 200)
wait(300)
hub.speaker.beep(1200, 200)

# Variables de estado
estado = FOLLOW_LINE
estado_anterior = -1
heading_acumulado = 0    # Tracking de heading absoluto

# Variables PID
integral = 0.0
error_prev = 0.0

# Contadores
ciclos_ambos_negro = 0   # Para detectar dead end
ciclos_ambos_blanco = 0  # Para detectar gap

print("=== RESCUE LINE INICIADO ===")

while estado != STOP:

    # ── Debug: mostrar cambios de estado ─────────────────────────
    if estado != estado_anterior:
        print("ESTADO:", NOMBRES_ESTADO[estado])
        estado_anterior = estado

        # Color del LED según estado
        if estado == FOLLOW_LINE:
            hub.light.on(Color.GREEN)
        elif estado == DETECTED_GREEN:
            hub.light.on(Color.YELLOW)
        elif estado in (TURN_LEFT, TURN_RIGHT, U_TURN):
            hub.light.on(Color.MAGENTA)
        elif estado == GAP_CROSS:
            hub.light.on(Color.CYAN)
        elif estado == SEARCH_LINE:
            hub.light.on(Color.RED)
        elif estado == DEAD_END:
            hub.light.on(Color.ORANGE)

    # ══════════════════════════════════════════════════════════════
    # ESTADO: SEGUIR LÍNEA (PID)
    # ══════════════════════════════════════════════════════════════
    if estado == FOLLOW_LINE:
        ref_i = sensor_izq.reflection()
        ref_d = sensor_der.reflection()

        # ── Verificar verde ──────────────────────────────────────
        if es_verde(sensor_izq) or es_verde(sensor_der):
            estado = DETECTED_GREEN
            integral = 0.0
            error_prev = 0.0
            continue

        # ── Verificar dead end (ambos negro mucho tiempo) ────────
        if es_negro(ref_i) and es_negro(ref_d):
            ciclos_ambos_negro += 1
            if ciclos_ambos_negro > DEAD_END_CICLOS:
                estado = DEAD_END
                ciclos_ambos_negro = 0
                continue
        else:
            ciclos_ambos_negro = 0

        # ── Verificar gap (ambos blanco mucho tiempo) ────────────
        if es_blanco(ref_i) and es_blanco(ref_d):
            ciclos_ambos_blanco += 1
            if ciclos_ambos_blanco > 15:  # ~150ms sin línea
                estado = GAP_CROSS
                ciclos_ambos_blanco = 0
                continue
        else:
            ciclos_ambos_blanco = 0

        # ── PID line following ───────────────────────────────────
        error = ref_i - ref_d  # Positivo = desviado a la derecha

        # Integral con anti-windup
        integral = clamp(integral + error * KI, -INTEGRAL_MAX, INTEGRAL_MAX)

        # Derivativo
        derivada = error - error_prev
        error_prev = error

        # Corrección PID
        correccion = KP * error + integral + KD * derivada

        # Velocidad adaptativa (más lento en curvas)
        vel = VEL_MAX - abs(error) * VEL_FACTOR
        vel = clamp(vel, VEL_MIN, VEL_MAX)

        robot.drive(vel, correccion)
        wait(10)

    # ══════════════════════════════════════════════════════════════
    # ESTADO: VERDE DETECTADO — Escanear y decidir
    # ══════════════════════════════════════════════════════════════
    elif estado == DETECTED_GREEN:
        robot.stop()
        wait(30)

        # Escanear ambos sensores mientras avanzamos lento
        verde_izq, verde_der = escanear_verde_avanzando()

        print("SCAN VERDE: izq=", verde_izq, "der=", verde_der)

        if verde_izq and verde_der:
            estado = U_TURN
        elif verde_izq:
            estado = TURN_LEFT
        elif verde_der:
            estado = TURN_RIGHT
        else:
            # Falsa alarma o verde pasó muy rápido
            # Verificar una vez más en posición actual
            if confirmar_verde_izq():
                estado = TURN_LEFT
            elif confirmar_verde_der():
                estado = TURN_RIGHT
            else:
                print("VERDE: falsa alarma, retomando línea")
                estado = FOLLOW_LINE

    # ══════════════════════════════════════════════════════════════
    # ESTADO: GIRAR A LA IZQUIERDA
    # ══════════════════════════════════════════════════════════════
    elif estado == TURN_LEFT:
        heading_acumulado = hub.imu.heading()
        ejecutar_giro(-90)  # Negativo = izquierda
        estado = FOLLOW_LINE
        integral = 0.0
        error_prev = 0.0
        ciclos_ambos_negro = 0
        ciclos_ambos_blanco = 0

    # ══════════════════════════════════════════════════════════════
    # ESTADO: GIRAR A LA DERECHA
    # ══════════════════════════════════════════════════════════════
    elif estado == TURN_RIGHT:
        heading_acumulado = hub.imu.heading()
        ejecutar_giro(90)  # Positivo = derecha
        estado = FOLLOW_LINE
        integral = 0.0
        error_prev = 0.0
        ciclos_ambos_negro = 0
        ciclos_ambos_blanco = 0

    # ══════════════════════════════════════════════════════════════
    # ESTADO: U-TURN (180°)
    # ══════════════════════════════════════════════════════════════
    elif estado == U_TURN:
        heading_acumulado = hub.imu.heading()
        ejecutar_giro(180)  # Media vuelta
        estado = FOLLOW_LINE
        integral = 0.0
        error_prev = 0.0
        ciclos_ambos_negro = 0
        ciclos_ambos_blanco = 0

    # ══════════════════════════════════════════════════════════════
    # ESTADO: CRUZAR GAP
    # ══════════════════════════════════════════════════════════════
    elif estado == GAP_CROSS:
        encontro = cruzar_gap()
        if encontro:
            estado = FOLLOW_LINE
        else:
            estado = SEARCH_LINE
        integral = 0.0
        error_prev = 0.0

    # ══════════════════════════════════════════════════════════════
    # ESTADO: DEAD END (baldosa negra)
    # ══════════════════════════════════════════════════════════════
    elif estado == DEAD_END:
        print("DEAD END: dando U-turn")
        robot.stop()
        wait(100)

        # Retroceder un poco para salir de la baldosa negra
        robot.straight(-60)

        # U-turn
        heading_acumulado = hub.imu.heading()
        girar_pid(heading_acumulado + 180)

        # Buscar línea
        encontro = buscar_linea_adelante(2000)
        if encontro:
            estado = FOLLOW_LINE
        else:
            estado = SEARCH_LINE
        
        integral = 0.0
        error_prev = 0.0
        ciclos_ambos_negro = 0

    # ══════════════════════════════════════════════════════════════
    # ESTADO: BUSCAR LÍNEA (perdido)
    # ══════════════════════════════════════════════════════════════
    elif estado == SEARCH_LINE:
        encontro = buscar_linea_zigzag()
        if encontro:
            estado = FOLLOW_LINE
        else:
            # Último recurso: avanzar lento y esperar
            print("PERDIDO: avanzando lento...")
            avanzar_mm(100, 80)
            ref_i = sensor_izq.reflection()
            ref_d = sensor_der.reflection()
            if es_negro(ref_i) or es_negro(ref_d):
                estado = FOLLOW_LINE
            else:
                # Seguir intentando (no detenerse)
                estado = SEARCH_LINE

        integral = 0.0
        error_prev = 0.0

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# FIN
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

robot.stop()
hub.light.on(Color.GREEN)
hub.speaker.beep(880, 150)
wait(80)
hub.speaker.beep(1320, 300)
print("=== RESCUE LINE FINALIZADO ===")
