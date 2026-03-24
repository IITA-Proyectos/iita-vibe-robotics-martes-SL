from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction, Color
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

# ╔══════════════════════════════════════════════════════════════╗
# ║  CUADRADO PERFECTO v3 — 1 metro de lado × 5 vueltas        ║
# ║  Robot: spike-2wd-basico (Puerto E izq, Puerto F der)       ║
# ║  Control de tracción + PID giros + perfil trapezoidal       ║
# ║  Heading absoluto acumulado (sin reset)                     ║
# ║  Autor: Taller Vibe Robotics IITA — 2026-03-24             ║
# ╚══════════════════════════════════════════════════════════════╝

# ── TEST MODE: poner True para calibración ───────────
MODO_TEST = False

hub = PrimeHub()
motor_izq = Motor(Port.E, Direction.COUNTERCLOCKWISE)
motor_der = Motor(Port.F)

DIAMETRO_RUEDA = 56.0   # mm — MEDIR CON CALIBRE
DISTANCIA_EJES = 112.0  # mm — MEDIR CENTRO A CENTRO
MM_PER_DEG = 3.14159265 * DIAMETRO_RUEDA / 360.0

robot = DriveBase(motor_izq, motor_der,
                  wheel_diameter=DIAMETRO_RUEDA,
                  axle_track=DISTANCIA_EJES)
robot.use_gyro(True)

# ── Velocidad y tracción ─────────────────────────────
VEL_MAX = 470; ACCEL = 350; DECEL = 600; VEL_MIN = 25
SLIP_THRESHOLD = 80; SLIP_FACTOR = 0.6

# ── PID de giro ──────────────────────────────────────
KP = 6.0; KI = 0.15; KD = 8.0
MAX_RATE = 550; INTEGRAL_MAX = 30; ZONA_INTEGRAL = 15
SETTLE_ERROR = 0.7; SETTLE_CYCLES = 5; TURN_TIMEOUT = 2000

# ── Carrera ──────────────────────────────────────────
LADO_MM = 1000; VUELTAS = 5; TOTAL_LADOS = VUELTAS * 4
COLORES = [Color.GREEN, Color.BLUE, Color.YELLOW,
           Color.MAGENTA, Color.CYAN]


def clamp(val, minv, maxv):
    if val < minv: return minv
    if val > maxv: return maxv
    return val


def detectar_slip():
    diff = abs(abs(motor_izq.speed()) - abs(motor_der.speed()))
    return SLIP_FACTOR if diff > SLIP_THRESHOLD else 1.0


def avanzar(distancia_mm, heading_target):
    motor_izq.reset_angle(0)
    motor_der.reset_angle(0)
    while True:
        dist = (abs(motor_izq.angle()) + abs(motor_der.angle())) / 2.0 * MM_PER_DEG
        restante = distancia_mm - dist
        if restante <= 2.0:
            break
        v_a = max((2.0 * ACCEL * max(dist, 0.5)) ** 0.5, VEL_MIN)
        v_d = max((2.0 * DECEL * max(restante, 0.5)) ** 0.5, VEL_MIN)
        vel = min(v_a, v_d, VEL_MAX) * detectar_slip()
        robot.drive(vel, 0)
        wait(10)
    robot.stop()
    if abs(heading_target - hub.imu.heading()) > 1.5:
        for _ in range(15):
            err = heading_target - hub.imu.heading()
            if abs(err) < 0.8: break
            robot.drive(0, clamp(err * 4.0, -80, 80))
            wait(10)
        robot.stop()


def girar_pid(heading_objetivo):
    h = hub.imu.heading()
    err_prev = heading_objetivo - h
    integral = 0.0; settled = 0; reloj = StopWatch()
    while True:
        h = hub.imu.heading()
        err = heading_objetivo - h
        p = KP * err
        if abs(err) < ZONA_INTEGRAL:
            integral = clamp(integral + KI * err, -INTEGRAL_MAX, INTEGRAL_MAX)
        else:
            integral *= 0.3
        d = KD * (err - err_prev); err_prev = err
        out = clamp(p + integral + d, -MAX_RATE, MAX_RATE)
        if abs(err) < SETTLE_ERROR and abs(out) < 5: out = 0
        robot.drive(0, out)
        if abs(err) < SETTLE_ERROR:
            settled += 1
            if settled >= SETTLE_CYCLES: break
        else:
            settled = 0
        if reloj.time() > TURN_TIMEOUT: break
        wait(10)
    robot.stop()


# ── PROGRAMA ─────────────────────────────────────────
hub.light.on(Color.WHITE)
hub.imu.reset_heading(0)
wait(1500)

for c in [Color.RED, Color.YELLOW, Color.GREEN]:
    hub.light.on(c); hub.speaker.beep(600, 150); wait(350)
hub.speaker.beep(1200, 200)

for i in range(TOTAL_LADOS):
    hub.light.on(COLORES[(i // 4) % len(COLORES)])
    avanzar(LADO_MM, i * 90)
    girar_pid((i + 1) * 90)

robot.stop()
hub.light.on(Color.GREEN)
hub.speaker.beep(880, 150); wait(80)
hub.speaker.beep(1320, 300)
