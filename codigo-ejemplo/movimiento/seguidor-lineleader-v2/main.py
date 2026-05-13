#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port, Direction, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch

# ── Clase LightSensorArray (Mindsensors I2C) ───────────────────
class LightSensorArray:
    """Wrapper de Mindsensors LightSensorArray (LSA) usando solo lectura cruda."""
    ADDR = 0x0A           # Dirección 7-bit del LSA (0x14 dividido por 2)
    REG_RAW = 0x42        # 8 bytes sensor data en el LSA (comienza en 0x42)

    def __init__(self, port):
        self.dev = I2CDevice(port, self.ADDR)

    def raw(self):
        """Retorna lista de 8 valores raw del sensor (usualmente 0-255)."""
        return list(self.dev.read(reg=self.REG_RAW, length=8))

# ── Hub y Configuración ──────────────────────────────────────────
ev3 = EV3Brick()

# Motores (Invertimos la marcha para que la parte trasera sea la nueva delantera)
# El motor que antes era el Derecho (B) ahora es el Izquierdo de la nueva delantera
motor_izq = Motor(Port.B, Direction.CLOCKWISE)
motor_der = Motor(Port.A, Direction.COUNTERCLOCKWISE)

# Ruedas
DIAMETRO_RUEDA = 43
DISTANCIA_EJES = 200

drive = DriveBase(motor_izq, motor_der, wheel_diameter=DIAMETRO_RUEDA, axle_track=DISTANCIA_EJES)

# Sensor LSA en el puerto 1 (cerca del eje de las ruedas)
ll = LightSensorArray(Port.S1)

# ── Variables de Control y Calibración ───────────────────────────
# Como el sensor está cerca del eje, no tiene "palanca" al girar.
# Necesitamos un KP más alto y no podemos frenar tanto en las curvas.
KP = 2.0
KI = 0.0
KD = 18.0

BASE_SPEED = 200
MIN_SPEED = 120
CENTER = 35

WHITE = [0]*8
BLACK = [0]*8

# ── Funciones Matemáticas ────────────────────────────────────────

def normalize_array(raw_8):
    """Convierte los valores raw usando los promedios de WHITE y BLACK a 0-100."""
    out = [0]*8
    for i in range(8):
        span = WHITE[i] - BLACK[i]
        if span == 0: 
            span = 1 # Evitar división por cero
        
        # Mapeamos: si lee igual que BLACK, da 0. Si lee igual que WHITE, da 100.
        n = (raw_8[i] - BLACK[i]) * 100 // span
        out[i] = max(0, min(100, n))
    return out

def pos_x10(cal):
    total = 0
    w_sum = 0
    for i, v in enumerate(cal):
        w = 100 - v # Queremos que lo negro (0) pese 100
        total += w
        w_sum += i * 10 * w
    
    if total < 80:
        return None
    return w_sum // total

def adaptive_speed(error):
    factor = 1.0 - abs(error) / 35.0
    if factor < 0: 
        factor = 0
    return int(MIN_SPEED + (BASE_SPEED - MIN_SPEED) * factor)

def count_on_line(cal, threshold=30):
    return sum(1 for v in cal if v < threshold)

# ── Algoritmo Principal ──────────────────────────────────────────

def follow_line():
    last_err = 0
    last_pos = CENTER
    integral = 0
    turn = 0  # Inicializamos turn para el caso de recovery
    
    while True:
        try:
            # 1. Leemos raw
            raw_vals = ll.raw()
            # 2. Normalizamos por software
            cal = normalize_array(raw_vals)
            
            if count_on_line(cal) >= 7:
                drive.stop()
                ev3.speaker.beep()
                print("Intersección detectada!")
                break
                
            pos = pos_x10(cal)
            
            if pos is None:
                # === RECOVERY MODE ===
                # Si perdemos la línea, retrocedemos (marcha atrás) a 100 mm/s.
                # Mantener la rotación (turn) del ciclo anterior hace que el robot
                # "deshaga" la curva exacta por donde se salió.
                drive.drive(-100, turn)
                wait(10)
                continue
                
            error = CENTER - pos
            
            integral += error
            if integral > 1000: integral = 1000
            elif integral < -1000: integral = -1000
            
            deriv = error - last_err
            
            last_err = error
            last_pos = pos
            
            turn = KP * error + KI * integral + KD * deriv
            drive.drive(adaptive_speed(error), turn)
            
        except OSError:
            # Si hay un error I2C temporal, ignoramos este ciclo
            pass
            
        wait(10)

# ── Flujo del Programa (con Calibración por Software) ────────────

ev3.speaker.beep()

# 1. Calibrar Blanco
ev3.screen.clear()
ev3.screen.draw_text(10, 30, "1. Fondo BLANCO")
ev3.screen.draw_text(10, 60, "Apretar CENTRO")
while Button.CENTER not in ev3.buttons.pressed():
    wait(20)

ev3.screen.draw_text(10, 90, "Midiendo...")
ev3.speaker.beep(400, 100)
for _ in range(20):
    raw = ll.raw()
    for i in range(8): WHITE[i] += raw[i]
    wait(20)
for i in range(8): WHITE[i] = WHITE[i] // 20

while Button.CENTER in ev3.buttons.pressed():
    wait(20)

# 2. Calibrar Negro
ev3.screen.clear()
ev3.screen.draw_text(10, 30, "2. Todo NEGRO")
ev3.screen.draw_text(10, 60, "Apretar CENTRO")
while Button.CENTER not in ev3.buttons.pressed():
    wait(20)

ev3.screen.draw_text(10, 90, "Midiendo...")
ev3.speaker.beep(500, 100)
for _ in range(20):
    raw = ll.raw()
    for i in range(8): BLACK[i] += raw[i]
    wait(20)
for i in range(8): BLACK[i] = BLACK[i] // 20

while Button.CENTER in ev3.buttons.pressed():
    wait(20)

# 3. Espera inicio con debug
ev3.screen.clear()
ev3.screen.draw_text(10, 20, "Listo! A la linea")
ev3.screen.draw_text(10, 50, "Apretar CENTRO")

while Button.CENTER not in ev3.buttons.pressed():
    try:
        cal = normalize_array(ll.raw())
        pos = pos_x10(cal)
        ev3.screen.draw_text(10, 80, "Posicion: " + str(pos) + "   ")
    except OSError:
        ev3.screen.draw_text(10, 80, "Error I2C...       ")
    wait(50)

while Button.CENTER in ev3.buttons.pressed():
    wait(20)

ev3.speaker.beep(800, 300)

# 4. Seguir línea
follow_line()

ev3.screen.clear()
ev3.screen.draw_text(10, 50, "LISTO")
ev3.speaker.beep(800, 300)
