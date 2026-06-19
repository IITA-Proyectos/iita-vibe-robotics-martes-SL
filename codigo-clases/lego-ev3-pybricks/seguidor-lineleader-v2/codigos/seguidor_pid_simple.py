#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port, Direction, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait
import json

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
KP = 4.5
KI = 0.0
KD = 22.0

BASE_SPEED = 72  # Reducida a la mitad (de 145)
MIN_SPEED = 15   # Reducida a la mitad (de 30)
CENTER = 35

WHITE = [0]*8
BLACK = [0]*8

# ── Funciones de Memoria ─────────────────────────────────────────

def guardar_calibracion():
    datos = {
        "WHITE": WHITE,
        "BLACK": BLACK
    }
    try:
        with open("calibracion_basica.json", "w") as f:
            json.dump(datos, f)
    except Exception as e:
        print("Error guardando:", e)

def cargar_calibracion():
    global WHITE, BLACK
    try:
        with open("calibracion_basica.json", "r") as f:
            datos = json.load(f)
            for i in range(8):
                WHITE[i] = datos["WHITE"][i]
                BLACK[i] = datos["BLACK"][i]
        return True
    except:
        return False

# ── Funciones Matemáticas y de Control ───────────────────────────

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

# ── Algoritmo Principal ──────────────────────────────────────────

def follow_line():
    last_err = 0
    last_pos = CENTER
    integral = 0
    turn = 0  # Inicializamos turn para el caso de recovery
    loss_counter = 0
    viz_counter = 0
    
    while True:
        try:
            # 1. Leemos raw
            raw_vals = ll.raw()
            # 2. Normalizamos por software
            cal = normalize_array(raw_vals)
            
            # Buscamos la posición de la línea
            pos = pos_x10(cal)
            
            # Visualización (Throttling a 10 iteraciones = ~100ms)
            viz_counter += 1
            if viz_counter >= 10:
                viz_counter = 0
                viz = "".join("#" if v < 40 else "." for v in cal)
                if pos is None:
                    if loss_counter > 8:
                        info_str = "[ RECOVERY ]"
                    else:
                        info_str = "[ PERDIDO ]"
                else:
                    info_str = "Pos: {}".format(pos)
                
                # Terminal PC (vía SSH)
                print("\rLSA: [{}] {}   ".format(viz, info_str), end="")
                
                # Pantalla EV3
                ev3.screen.clear()
                ev3.screen.draw_text(10, 20, "SEGUIDOR PID")
                ev3.screen.draw_text(10, 50, "[" + viz + "]")
                ev3.screen.draw_text(10, 80, info_str)
            
            if pos is None:
                loss_counter += 1
                if loss_counter > 8:  # ~80ms (8 ciclos de 10ms) de pérdida continua
                    # === RECOVERY MODE ===
                    # Si perdemos la línea de verdad, retrocedemos (marcha atrás) a -70 mm/s.
                    # Invertimos la rotación (-turn) para desandar la curva
                    drive.drive(-70, -turn)
                # Si es una pérdida momentánea, mantenemos velocidad/giro
                wait(10)
                continue
            
            loss_counter = 0
                
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

# ── Flujo de Calibración ─────────────────────────────────────────

def calibracion_manual():
    ev3.speaker.beep()

    # 1. Calibrar Blanco LSA
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "1. Blanco LSA")
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

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Blanco OK")
    ev3.screen.draw_text(0, 40, " ".join(str(v) for v in WHITE[0:4]))
    ev3.screen.draw_text(0, 70, " ".join(str(v) for v in WHITE[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 2. Calibrar Negro LSA
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "2. Negro LSA")
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

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Negro OK")
    ev3.screen.draw_text(0, 40, " ".join(str(v) for v in BLACK[0:4]))
    ev3.screen.draw_text(0, 70, " ".join(str(v) for v in BLACK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    guardar_calibracion()
    ev3.screen.clear()
    ev3.screen.draw_text(0, 40, "Guardado OK!")
    ev3.speaker.beep(1000, 200)
    wait(1000)

# --- MENÚ DE INICIO ---
ev3.screen.clear()
ev3.screen.draw_text(0, 10, "INICIO (PID)")
ev3.screen.draw_text(0, 40, "ARRIBA: Memoria")
ev3.screen.draw_text(0, 70, "ABAJO: Calibrar")

usar_guardada = False
while True:
    botones = ev3.buttons.pressed()
    if Button.UP in botones:
        usar_guardada = True
        break
    elif Button.DOWN in botones:
        usar_guardada = False
        break
    wait(20)

if usar_guardada:
    if cargar_calibracion():
        ev3.screen.clear()
        ev3.screen.draw_text(0, 40, "Memoria OK!")
        ev3.speaker.beep(1000, 200)
        wait(1000)
    else:
        ev3.screen.clear()
        ev3.screen.draw_text(0, 40, "No hay memoria")
        ev3.screen.draw_text(0, 70, "Calibre manual")
        ev3.speaker.beep(200, 500)
        wait(2000)
        calibracion_manual()
else:
    calibracion_manual()

# 3. Espera inicio con debug de posición en vivo
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
