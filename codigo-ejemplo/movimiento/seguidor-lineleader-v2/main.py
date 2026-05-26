#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port, Direction, Button
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch
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

BASE_SPEED = 130
MIN_SPEED = 30
CENTER = 35
INTERSECTION_THRESHOLD = 20

WHITE = [0]*8
BLACK = [0]*8
GREEN_L_MIN, GREEN_L_MAX = 30, 70
GREEN_R_MIN, GREEN_R_MAX = 30, 70
T_L_MASK = [False]*8
T_R_MASK = [False]*8

# ── Funciones de Memoria ─────────────────────────────────────────

def guardar_calibracion():
    datos = {
        "WHITE": WHITE,
        "BLACK": BLACK,
        "GREEN_L": [GREEN_L_MIN, GREEN_L_MAX],
        "GREEN_R": [GREEN_R_MIN, GREEN_R_MAX],
        "T_L_MASK": T_L_MASK,
        "T_R_MASK": T_R_MASK
    }
    try:
        with open("calibracion.json", "w") as f:
            json.dump(datos, f)
    except Exception as e:
        print("Error guardando:", e)

def cargar_calibracion():
    global WHITE, BLACK, GREEN_L_MIN, GREEN_L_MAX, GREEN_R_MIN, GREEN_R_MAX, T_L_MASK, T_R_MASK
    try:
        with open("calibracion.json", "r") as f:
            datos = json.load(f)
            for i in range(8):
                WHITE[i] = datos["WHITE"][i]
                BLACK[i] = datos["BLACK"][i]
            GREEN_L_MIN, GREEN_L_MAX = datos["GREEN_L"][0], datos["GREEN_L"][1]
            GREEN_R_MIN, GREEN_R_MAX = datos["GREEN_R"][0], datos["GREEN_R"][1]
            if "T_L_MASK" in datos:
                T_L_MASK = datos["T_L_MASK"]
            if "T_R_MASK" in datos:
                T_R_MASK = datos["T_R_MASK"]
        return True
    except:
        return False

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

def is_green_left(v):
    return GREEN_L_MIN <= v <= GREEN_L_MAX

def is_green_right(v):
    return GREEN_R_MIN <= v <= GREEN_R_MAX

def matches_mask(cal, mask, threshold_black=35, threshold_white=65):
    # Si la máscara no está calibrada (todo False), no debe coincidir con nada
    if not any(mask):
        return False
    
    black_expected = 0
    black_matched = 0
    white_expected = 0
    white_matched = 0
    
    for i in range(8):
        if mask[i]:
            black_expected += 1
            if cal[i] < threshold_black:
                black_matched += 1
        else:
            white_expected += 1
            if cal[i] > threshold_white:
                white_matched += 1
                
    # Exigimos que coincidan casi todos los negros esperados y casi todos los blancos esperados por separado.
    # Esto evita que una curva (donde el centro es blanco pero un lado es negro) coincida con la T.
    return (black_matched >= black_expected - 1) and (white_matched >= white_expected - 1)

def detect_green_marker(cal):
    """
    Detecta el patrón de verdes (RoboCup Rescue Line).
    Retorna: 'LEFT', 'RIGHT', 'DOUBLE' o None
    """
    left_zone = cal[0:3]
    center_zone = cal[3:5]
    right_zone = cal[5:8]
    
    # 1. El centro debe estar viendo negro (línea)
    if not any(v < 30 for v in center_zone):
        return None
        
    # 2. Contar grises (verdes) separados por lado
    left_greens = sum(1 for v in left_zone if is_green_left(v))
    right_greens = sum(1 for v in right_zone if is_green_right(v))
    
    # 3. Contar blancos (piso limpio) relativo al límite superior de su propio verde
    left_whites = sum(1 for v in left_zone if v > GREEN_L_MAX)
    right_whites = sum(1 for v in right_zone if v > GREEN_R_MAX)
    
    # Patrón Doble Verde: GRIS - NEGRO - GRIS (exigimos >= 2 sensores grises por lado)
    if left_greens >= 2 and right_greens >= 2:
        return 'DOUBLE'
        
    # Patrón Verde Derecho: BLANCO - NEGRO - GRIS
    # Requiere 2 grises a la derecha, y al menos 1 blanco a la izquierda
    elif right_greens >= 2 and left_whites >= 1:
        return 'RIGHT'
        
    # Patrón Verde Izquierdo: GRIS - NEGRO - BLANCO
    # Requiere 2 grises a la izquierda, y al menos 1 blanco a la derecha
    elif left_greens >= 2 and right_whites >= 1:
        return 'LEFT'
        
    return None

# ── Algoritmo Principal ──────────────────────────────────────────

def follow_line():
    last_err = 0
    last_pos = CENTER
    integral = 0
    turn = 0  # Inicializamos turn para el caso de recovery
    loss_counter = 0
    
    while True:
        try:
            # 1. Leemos raw
            raw_vals = ll.raw()
            # 2. Normalizamos por software
            cal = normalize_array(raw_vals)
            
            # Intersección negra (los 8 sensores deben leer negro puro)
            if count_on_line(cal, threshold=INTERSECTION_THRESHOLD) == 8:
                drive.stop()
                ev3.speaker.beep(600, 300)
                drive.straight(40)  # Avanzar un poco para pasar la intersección
                last_err = 0
                integral = 0
                turn = 0
                continue

            # Intersección en T Izquierda (se ignora y se sigue derecho)
            if matches_mask(cal, T_L_MASK):
                drive.stop()
                ev3.speaker.beep(800, 100)
                wait(50)
                ev3.speaker.beep(800, 100)
                print("T Izquierda detectada - Ignorando")
                drive.straight(40)  # Avanzar para pasar el desvío
                last_err = 0
                integral = 0
                turn = 0
                continue

            # Intersección en T Derecha (se ignora y se sigue derecho)
            if matches_mask(cal, T_R_MASK):
                drive.stop()
                ev3.speaker.beep(1000, 100)
                wait(50)
                ev3.speaker.beep(1000, 100)
                print("T Derecha detectada - Ignorando")
                drive.straight(40)  # Avanzar para pasar el desvío
                last_err = 0
                integral = 0
                turn = 0
                continue
            
            # Intersección verde (RoboCup) - Desactivado temporalmente
            # marker = detect_green_marker(cal)
            # if marker:
            #     drive.stop()
            #     ev3.speaker.beep(1000, 100)
            #     print("Marcador Verde:", marker)
            #     
            #     # Avanza aprox 1cm para quedar alineado
            #     drive.straight(10)
            #     
            #     if marker == 'RIGHT':
            #         drive.turn(90)
            #     elif marker == 'LEFT':
            #         drive.turn(-90)
            #     elif marker == 'DOUBLE':
            #         drive.turn(180)
            #     
            #     # Reiniciamos variables post-giro
            #     last_err = 0
            #     integral = 0
            #     turn = 0
            #     continue
                
            pos = pos_x10(cal)
            
            if pos is None:
                loss_counter += 1
                if loss_counter > 8:  # ~80ms (8 ciclos de 10ms) de pérdida continua
                    # === RECOVERY MODE ===
                    # Si perdemos la línea de verdad, retrocedemos (marcha atrás) a 70 mm/s.
                    # Invertimos la rotación (-turn) para que el robot desande la curva
                    # y enderece su ángulo con la línea en lugar de quedar perpendicular.
                    drive.drive(-70, -turn)
                # Si es una pérdida momentánea (ruido o inicio de curva rápida),
                # no cambiamos la velocidad/giro para darle margen a recuperarse solo.
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

# ── Flujo del Programa (con Calibración por Software) ────────────

def calibracion_manual():
    global GREEN_L_MIN, GREEN_L_MAX, GREEN_R_MIN, GREEN_R_MAX
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

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Blanco OK")
    ev3.screen.draw_text(0, 40, " ".join(str(v) for v in WHITE[0:4]))
    ev3.screen.draw_text(0, 70, " ".join(str(v) for v in WHITE[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

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

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Negro OK")
    ev3.screen.draw_text(0, 40, " ".join(str(v) for v in BLACK[0:4]))
    ev3.screen.draw_text(0, 70, " ".join(str(v) for v in BLACK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 3. Calibrar Verde Izquierdo (Desactivado temporalmente)
    # ev3.screen.clear()
    # ev3.screen.draw_text(10, 30, "3. VERDE IZQ")
    # ev3.screen.draw_text(10, 60, "Apretar CENTRO")
    # while Button.CENTER not in ev3.buttons.pressed():
    #     wait(20)
    # 
    # ev3.screen.draw_text(10, 90, "Midiendo...")
    # ev3.speaker.beep(600, 100)
    # 
    # green_l = []
    # for _ in range(20):
    #     cal = normalize_array(ll.raw())
    #     # Tomamos muestras de los sensores izquierdos (0 y 1)
    #     green_l.extend([cal[0], cal[1]])
    #     wait(20)
    # 
    # if green_l:
    #     avg_l = sum(green_l) // len(green_l)
    #     GREEN_L_MIN = max(0, avg_l - 15)
    #     GREEN_L_MAX = min(100, avg_l + 15)
    # 
    #     ev3.screen.clear()
    #     ev3.screen.draw_text(0, 10, "V. Izq: " + str(GREEN_L_MIN) + "-" + str(GREEN_L_MAX))
    #     ev3.screen.draw_text(0, 40, " ".join(str(v) for v in cal[0:4]))
    #     ev3.screen.draw_text(0, 70, " ".join(str(v) for v in cal[4:8]))
    #     ev3.screen.draw_text(0, 100, "Click p/seguir")
    # 
    # while Button.CENTER in ev3.buttons.pressed(): wait(20)
    # while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    # while Button.CENTER in ev3.buttons.pressed(): wait(20)
    # 
    # # 4. Calibrar Verde Derecho (Desactivado temporalmente)
    # ev3.screen.clear()
    # ev3.screen.draw_text(10, 30, "4. VERDE DER")
    # ev3.screen.draw_text(10, 60, "Apretar CENTRO")
    # while Button.CENTER not in ev3.buttons.pressed():
    #     wait(20)
    # 
    # ev3.screen.draw_text(10, 90, "Midiendo...")
    # ev3.speaker.beep(600, 100)
    # 
    # green_r = []
    # for _ in range(20):
    #     cal = normalize_array(ll.raw())
    #     # Tomamos muestras de los sensores derechos (6 y 7)
    #     green_r.extend([cal[6], cal[7]])
    #     wait(20)
    # 
    # if green_r:
    #     avg_r = sum(green_r) // len(green_r)
    #     GREEN_R_MIN = max(0, avg_r - 15)
    #     GREEN_R_MAX = min(100, avg_r + 15)
    # 
    #     ev3.screen.clear()
    #     ev3.screen.draw_text(0, 10, "V. Der: " + str(GREEN_R_MIN) + "-" + str(GREEN_R_MAX))
    #     ev3.screen.draw_text(0, 40, " ".join(str(v) for v in cal[0:4]))
    #     ev3.screen.draw_text(0, 70, " ".join(str(v) for v in cal[4:8]))
    #     ev3.screen.draw_text(0, 100, "Click p/seguir")
    # 
    # while Button.CENTER in ev3.buttons.pressed(): wait(20)
    # while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    # while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 3. Calibrar T Izquierda
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "3. T IZQUIERDA")
    ev3.screen.draw_text(10, 60, "Colocar sobre T Izq")
    ev3.screen.draw_text(10, 80, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(10, 100, "Midiendo...")
    ev3.speaker.beep(600, 100)
    samples_tl = [0]*8
    for _ in range(20):
        cal = normalize_array(ll.raw())
        for i in range(8): samples_tl[i] += cal[i]
        wait(20)
    for i in range(8):
        avg = samples_tl[i] // 20
        T_L_MASK[i] = avg < 40

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "T Izq OK")
    ev3.screen.draw_text(0, 40, " ".join("1" if b else "0" for b in T_L_MASK[0:4]))
    ev3.screen.draw_text(0, 70, " ".join("1" if b else "0" for b in T_L_MASK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 4. Calibrar T Derecha
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "4. T DERECHA")
    ev3.screen.draw_text(10, 60, "Colocar sobre T Der")
    ev3.screen.draw_text(10, 80, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(10, 100, "Midiendo...")
    ev3.speaker.beep(600, 100)
    samples_tr = [0]*8
    for _ in range(20):
        cal = normalize_array(ll.raw())
        for i in range(8): samples_tr[i] += cal[i]
        wait(20)
    for i in range(8):
        avg = samples_tr[i] // 20
        T_R_MASK[i] = avg < 40

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "T Der OK")
    ev3.screen.draw_text(0, 40, " ".join("1" if b else "0" for b in T_R_MASK[0:4]))
    ev3.screen.draw_text(0, 70, " ".join("1" if b else "0" for b in T_R_MASK[4:8]))
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
ev3.screen.draw_text(0, 10, "INICIO")
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
