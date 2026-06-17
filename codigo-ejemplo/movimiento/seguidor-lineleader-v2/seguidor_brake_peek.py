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
motor_izq = Motor(Port.B, Direction.CLOCKWISE)
motor_der = Motor(Port.A, Direction.COUNTERCLOCKWISE)

# Ruedas
DIAMETRO_RUEDA = 43
DISTANCIA_EJES = 200

drive = DriveBase(motor_izq, motor_der, wheel_diameter=DIAMETRO_RUEDA, axle_track=DISTANCIA_EJES)

# Sensor LSA en el puerto 1
ll = LightSensorArray(Port.S1)

# ── Variables de Control y Calibración ───────────────────────────
KP = 4.5
KI = 0.0
KD = 22.0

BASE_SPEED = 72
MIN_SPEED = 15
CENTER = 35
INTERSECTION_THRESHOLD = 20

WHITE = [0]*8
BLACK = [0]*8
T_L_MASK = [False]*8
T_R_MASK = [False]*8

# ── Funciones de Memoria ─────────────────────────────────────────

def guardar_calibracion():
    datos = {
        "WHITE": WHITE,
        "BLACK": BLACK,
        "T_L_MASK": T_L_MASK,
        "T_R_MASK": T_R_MASK
    }
    try:
        with open("calibracion_brake_peek.json", "w") as f:
            json.dump(datos, f)
    except Exception as e:
        print("Error guardando:", e)

def cargar_calibracion():
    global WHITE, BLACK, T_L_MASK, T_R_MASK
    try:
        with open("calibracion_brake_peek.json", "r") as f:
            datos = json.load(f)
            for i in range(8):
                WHITE[i] = datos["WHITE"][i]
                BLACK[i] = datos["BLACK"][i]
            if "T_L_MASK" in datos:
                T_L_MASK = datos["T_L_MASK"]
            if "T_R_MASK" in datos:
                T_R_MASK = datos["T_R_MASK"]
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
            span = 1
        n = (raw_8[i] - BLACK[i]) * 100 // span
        out[i] = max(0, min(100, n))
    return out

def pos_x10(cal):
    total = 0
    w_sum = 0
    for i, v in enumerate(cal):
        w = 100 - v
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

def matches_mask(cal, mask, threshold_black=35, threshold_white=65):
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
                
    return (black_matched >= black_expected - 1) and (white_matched >= white_expected - 1)

# ── Algoritmo Principal ──────────────────────────────────────────

def follow_line():
    last_err = 0
    last_pos = CENTER
    integral = 0
    turn = 0
    loss_counter = 0
    t_cooldown = 0
    viz_counter = 0
    
    while True:
        try:
            if t_cooldown > 0:
                t_cooldown -= 1

            # 1. Leemos raw y normalizamos
            raw_vals = ll.raw()
            cal = normalize_array(raw_vals)
            
            # Buscamos posición
            pos = pos_x10(cal)
            
            # --- DETECCION DE INTERSECCION EN CRUZ (Doble negro) ---
            if sum(1 for v in cal if v < INTERSECTION_THRESHOLD) == 8:
                drive.stop()
                ev3.speaker.beep(600, 300)
                drive.straight(40)  # Avanzar un poco para pasar la intersección
                last_err = 0
                integral = 0
                turn = 0
                t_cooldown = 40
                continue
            
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
                    info_str = "Pos: {} CD: {}".format(pos, t_cooldown)
                
                # Terminal PC
                print("\rLSA: [{}] {}   ".format(viz, info_str), end="")
                
                # Pantalla EV3
                ev3.screen.clear()
                ev3.screen.draw_text(10, 20, "PEEK M: CD:" + str(t_cooldown))
                ev3.screen.draw_text(10, 50, "[" + viz + "]")
                ev3.screen.draw_text(10, 80, info_str)
            
            # --- DETECCION DE INTERSECCIONES T/L (Brake & Peek) ---
            if t_cooldown == 0:
                match_l = matches_mask(cal, T_L_MASK)
                match_r = matches_mask(cal, T_R_MASK)
                
                if match_l or match_r:
                    drive.stop()
                    ev3.speaker.beep(500, 80)  # Alerta inicial
                    wait(100)  # Estabilizar chasis
                    
                    # Avanzamos recto 25 mm para verificar qué hay por delante de la intersección
                    drive.straight(25)
                    wait(50)
                    
                    # Leemos el LSA en el nuevo punto
                    new_cal = normalize_array(ll.raw())
                    
                    # Si el centro de la línea sigue negro, es una intersección en T (sigue recto)
                    if new_cal[3] < 50 or new_cal[4] < 50:
                        ev3.speaker.beep(800, 100)
                        wait(50)
                        ev3.speaker.beep(800, 100)
                        if match_l:
                            print("\nT Izquierda CONFIRMADA - Siguiendo derecho")
                        else:
                            print("\nT Derecha CONFIRMADA - Siguiendo derecho")
                        
                        # Avanzamos un poco más para superar completamente la bifurcación
                        drive.straight(15)
                        t_cooldown = 40
                    else:
                        # Si el centro se volvió blanco, es una curva (de cualquier ángulo)
                        # Retrocedemos los 25 mm para volver al inicio y resolver con PID
                        ev3.speaker.beep(300, 200)  # Sonido grave
                        print("\nCurva detectada - Retrocediendo 25 mm para resolver con PID")
                        drive.straight(-25)
                        t_cooldown = 60  # Cooldown extendido para completar el giro
                    
                    # Reiniciamos variables después de la evaluación
                    last_err = 0
                    integral = 0
                    turn = 0
                    continue
            
            # --- PID SEGÚN LA LÍNEA ---
            if pos is None:
                loss_counter += 1
                if loss_counter > 8:
                    # Recovery
                    drive.drive(-70, -turn)
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
            pass
            
        wait(10)

# ── Flujo de Calibración ─────────────────────────────────────────

def calibracion_manual():
    global T_L_MASK, T_R_MASK
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
ev3.screen.draw_text(0, 10, "INICIO (PEEK)")
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
