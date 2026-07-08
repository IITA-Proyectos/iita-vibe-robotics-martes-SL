#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor, ColorSensor
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port, Direction, Button, Color
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

# Sensores de Color Frontales
# S2: Izquierdo, S4: Derecho (Colocados a 85mm del eje de las ruedas)
color_izq = ColorSensor(Port.S2)
color_der = ColorSensor(Port.S4)

# ── Variables de Control y Calibración ───────────────────────────
KP = 4.5
KI = 0.0
KD = 22.0

BASE_SPEED = 72
MIN_SPEED = 15
CENTER = 35
INTERSECTION_THRESHOLD = 20

# Distancia de alineación: sensores al eje (46mm) + verde (25mm) + ancho de línea (17mm) = 88mm
GREEN_ALIGN_DISTANCE = 88
PEEK_GREEN_DIST = 15  # Avance corto para comprobar si hay intersección antes del verde (en mm)

WHITE = [0]*8
BLACK = [0]*8
T_L_MASK = [False]*8
T_R_MASK = [False]*8

# Valores de calibración por defecto para Verde, Negro y Blanco (HSV)
GREEN_L_H, GREEN_L_S, GREEN_L_V = 145, 70, 17
GREEN_R_H, GREEN_R_S, GREEN_R_V = 130, 70, 17

BLACK_L_H, BLACK_L_S, BLACK_L_V = 99, 74, 38
BLACK_R_H, BLACK_R_S, BLACK_R_V = 87, 84, 27

WHITE_L_H, WHITE_L_S, WHITE_L_V = 192, 30, 81
WHITE_R_H, WHITE_R_S, WHITE_R_V = 173, 21, 71

# ── Funciones de Memoria ─────────────────────────────────────────

def guardar_calibracion():
    datos = {
        "WHITE": WHITE,
        "BLACK": BLACK,
        "T_L_MASK": T_L_MASK,
        "T_R_MASK": T_R_MASK,
        "GREEN_L_HSV": [GREEN_L_H, GREEN_L_S, GREEN_L_V],
        "GREEN_R_HSV": [GREEN_R_H, GREEN_R_S, GREEN_R_V],
        "BLACK_L_HSV": [BLACK_L_H, BLACK_L_S, BLACK_L_V],
        "BLACK_R_HSV": [BLACK_R_H, BLACK_R_S, BLACK_R_V],
        "WHITE_L_HSV": [WHITE_L_H, WHITE_L_S, WHITE_L_V],
        "WHITE_R_HSV": [WHITE_R_H, WHITE_R_S, WHITE_R_V]
    }
    try:
        with open("calibracion_verdes.json", "w") as f:
            json.dump(datos, f)
    except Exception as e:
        print("Error guardando:", e)

def cargar_calibracion():
    global WHITE, BLACK, T_L_MASK, T_R_MASK
    global GREEN_L_H, GREEN_L_S, GREEN_L_V, GREEN_R_H, GREEN_R_S, GREEN_R_V
    global BLACK_L_H, BLACK_L_S, BLACK_L_V, BLACK_R_H, BLACK_R_S, BLACK_R_V
    global WHITE_L_H, WHITE_L_S, WHITE_L_V, WHITE_R_H, WHITE_R_S, WHITE_R_V
    try:
        with open("calibracion_verdes.json", "r") as f:
            datos = json.load(f)
            for i in range(8):
                WHITE[i] = datos["WHITE"][i]
                BLACK[i] = datos["BLACK"][i]
            if "T_L_MASK" in datos:
                T_L_MASK = datos["T_L_MASK"]
            if "T_R_MASK" in datos:
                T_R_MASK = datos["T_R_MASK"]
            if "GREEN_L_HSV" in datos:
                GREEN_L_H, GREEN_L_S, GREEN_L_V = datos["GREEN_L_HSV"]
            if "GREEN_R_HSV" in datos:
                GREEN_R_H, GREEN_R_S, GREEN_R_V = datos["GREEN_R_HSV"]
            if "BLACK_L_HSV" in datos:
                BLACK_L_H, BLACK_L_S, BLACK_L_V = datos["BLACK_L_HSV"]
            if "BLACK_R_HSV" in datos:
                BLACK_R_H, BLACK_R_S, BLACK_R_V = datos["BLACK_R_HSV"]
            if "WHITE_L_HSV" in datos:
                WHITE_L_H, WHITE_L_S, WHITE_L_V = datos["WHITE_L_HSV"]
            if "WHITE_R_HSV" in datos:
                WHITE_R_H, WHITE_R_S, WHITE_R_V = datos["WHITE_R_HSV"]
        return True
    except:
        return False

# ── Funciones Matemáticas y de Color ─────────────────────────────

def rgb_to_hsv(r, g, b):
    """Convierte valores RGB (0-100) de Pybricks a HSV (Hue: 0-359, Sat: 0-100, Val: 0-100)."""
    r_n = r / 100.0
    g_n = g / 100.0
    b_n = b / 100.0
    
    max_c = max(r_n, g_n, b_n)
    min_c = min(r_n, g_n, b_n)
    delta = max_c - min_c
    
    if delta == 0:
        h = 0
    elif max_c == r_n:
        h = (60 * ((g_n - b_n) / delta) + 360) % 360
    elif max_c == g_n:
        h = (60 * ((b_n - r_n) / delta) + 120) % 360
    else:
        h = (60 * ((r_n - g_n) / delta) + 240) % 360
        
    s = 0 if max_c == 0 else (delta / max_c) * 100.0
    v = max_c * 100.0
    
    return int(h), int(s), int(v)

def es_color_verde(h, s, v, target_g, target_b, target_w, sens_verde=1.1):
    """Evalúa si el color clasifica como VERDE utilizando distancia euclidiana ponderada."""
    gh, gs, gv = target_g
    bh, bs, bv = target_b
    wh, ws, wv = target_w
    
    def diff_hue(h1, h2):
        d = abs(h1 - h2)
        return 360 - d if d > 180 else d

    # Distancia al Negro
    d_black_h = diff_hue(h, bh)
    if s < 22:
        dist_black = (s - bs)**2 + (v - bv)**2
    else:
        dist_black = d_black_h**2 + (s - bs)**2 + (v - bv)**2

    # Distancia al Blanco
    d_white_h = diff_hue(h, wh)
    if s < 22:
        dist_white = (s - ws)**2 + (v - wv)**2
    else:
        dist_white = d_white_h**2 + (s - ws)**2 + (v - wv)**2

    # Distancia al Verde (penalizado si la saturación es muy baja)
    d_green_h = diff_hue(h, gh)
    if s < 22:
        dist_green = ((s - gs)**2 + (v - gv)**2) + 50000
    else:
        dist_green = d_green_h**2 + (s - gs)**2 + (v - gv)**2
        
    # Ponderación del verde (sens_verde > 1.0 hace que sea más estricto)
    dist_green = dist_green * sens_verde
    
    # El verde gana si su distancia es menor que la del negro y la del blanco
    return dist_green < dist_black and dist_green < dist_white

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
    t_cooldown_green = 0
    t_cooldown_t = 0
    viz_counter = 0
    
    while True:
        try:
            if t_cooldown_green > 0:
                t_cooldown_green -= 1
            if t_cooldown_t > 0:
                t_cooldown_t -= 1

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
                t_cooldown_green = 40
                t_cooldown_t = 40
                continue

            # --- DETECCION DE MARCADORES VERDES ---
            if t_cooldown_green == 0:
                # Lectura de sensores de color
                r_l, g_l, b_l = color_izq.rgb()
                h_l, s_l, v_l = rgb_to_hsv(r_l, g_l, b_l)
                
                r_r, g_r, b_r = color_der.rgb()
                h_r, s_r, v_r = rgb_to_hsv(r_r, g_r, b_r)
                
                # Evaluar verdes con respecto a los targets calibrados
                green_l = es_color_verde(h_l, s_l, v_l, 
                                         (GREEN_L_H, GREEN_L_S, GREEN_L_V), 
                                         (BLACK_L_H, BLACK_L_S, BLACK_L_V), 
                                         (WHITE_L_H, WHITE_L_S, WHITE_L_V))
                green_r = es_color_verde(h_r, s_r, v_r, 
                                         (GREEN_R_H, GREEN_R_S, GREEN_R_V), 
                                         (BLACK_R_H, BLACK_R_S, BLACK_R_V), 
                                         (WHITE_R_H, WHITE_R_S, WHITE_R_V))
                
                if green_l or green_r:
                    # 1. Parar de inmediato y esperar un momento para inspección visual
                    drive.stop()
                    ev3.speaker.beep(700, 50)  # Tono rápido de detección inicial
                    wait(50)  # Pausa visual inicial de 50ms
                    
                    # 2. Hacer el avance corto PEEK_GREEN_DIST (15 mm)
                    drive.straight(PEEK_GREEN_DIST)
                    drive.stop()
                    
                    # 3. Esperar otro momento quieto para inspección visual del avance
                    wait(50)  # Pausa visual secundaria de 50ms
                    
                    # Comprobamos si el LSA detectó intersección en este nuevo punto
                    cal_test = normalize_array(ll.raw())
                    es_cruz = sum(1 for v in cal_test if v < INTERSECTION_THRESHOLD) >= 7
                    es_t_l = matches_mask(cal_test, T_L_MASK)
                    es_t_r = matches_mask(cal_test, T_R_MASK)
                    
                    if es_cruz or es_t_l or es_t_r:
                        print("\n[VERDE] Ignorado: intersección primero detectada por LSA.")
                        t_cooldown_green = 40  # Cooldown para pasar la intersección sin re-detectar verde
                        t_cooldown_t = 40
                        continue
                    
                    # Confirmar con 5 muestras rápidas
                    confirm_l = False
                    confirm_r = False
                    for _ in range(5):
                        rl, gl, bl = color_izq.rgb()
                        hl, sl, vl = rgb_to_hsv(rl, gl, bl)
                        if es_color_verde(hl, sl, vl, 
                                          (GREEN_L_H, GREEN_L_S, GREEN_L_V), 
                                          (BLACK_L_H, BLACK_L_S, BLACK_L_V), 
                                          (WHITE_L_H, WHITE_L_S, WHITE_L_V)):
                            confirm_l = True
                        
                        rr, gr, br = color_der.rgb()
                        hr, sr, vr = rgb_to_hsv(rr, gr, br)
                        if es_color_verde(hr, sr, vr, 
                                          (GREEN_R_H, GREEN_R_S, GREEN_R_V), 
                                          (BLACK_R_H, BLACK_R_S, BLACK_R_V), 
                                          (WHITE_R_H, WHITE_R_S, WHITE_R_V)):
                            confirm_r = True
                        wait(10)
                    
                    if confirm_l or confirm_r:
                        ev3.speaker.beep(1200, 150)  # Confirmación sonora
                        
                        # Avanzar la distancia restante para alinear el eje de las ruedas con la intersección
                        drive.straight(GREEN_ALIGN_DISTANCE - PEEK_GREEN_DIST)
                        wait(50)
                        
                        # Decidir acción de giro
                        if confirm_l and confirm_r:
                            print("\n[VERDE] DOBLE VERDE -> Giro de 180°")
                            drive.turn(180)
                        elif confirm_l:
                            print("\n[VERDE] VERDE IZQ -> Giro de 90° Izquierda")
                            drive.turn(90)
                        elif confirm_r:
                            print("\n[VERDE] VERDE DER -> Giro de 90° Derecha")
                            drive.turn(-90)
                            
                        # Limpiar variables post-giro
                        last_err = 0
                        integral = 0
                        turn = 0
                        t_cooldown_green = 5
                        t_cooldown_t = 50
                        continue
            
            # --- DETECCION DE INTERSECCIONES T/L (Brake & Peek) ---
            if t_cooldown_t == 0:
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
                        t_cooldown_t = 40
                    else:
                        # Si el centro se volvió blanco, es una curva (de cualquier ángulo)
                        # Retrocedemos los 25 mm para volver al inicio y resolver con PID
                        ev3.speaker.beep(300, 200)  # Sonido grave
                        print("\nCurva detectada - Retrocediendo 25 mm para resolver con PID")
                        drive.straight(-25)
                        t_cooldown_t = 60  # Cooldown extendido para completar el giro
                    
                    # Reiniciamos variables después de la evaluación
                    last_err = 0
                    integral = 0
                    turn = 0
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
                    info_str = "Pos: {} CD_G:{} CD_T:{}".format(pos, t_cooldown_green, t_cooldown_t)
                
                # Terminal PC
                print("\rLSA: [{}] {}   ".format(viz, info_str), end="")
                
                # Pantalla EV3
                ev3.screen.clear()
                ev3.screen.draw_text(10, 20, "G:{} T:{}".format(t_cooldown_green, t_cooldown_t))
                ev3.screen.draw_text(10, 50, "[" + viz + "]")
                ev3.screen.draw_text(10, 80, info_str)
            
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
            speed = adaptive_speed(error)
            if t_cooldown_green > 0 or t_cooldown_t > 0:
                speed = speed // 2
            drive.drive(speed, turn)
            
        except OSError:
            pass
            
        wait(10)

# ── Flujo de Calibración ─────────────────────────────────────────

def calibracion_manual():
    global T_L_MASK, T_R_MASK
    global GREEN_L_H, GREEN_L_S, GREEN_L_V, GREEN_R_H, GREEN_R_S, GREEN_R_V
    global BLACK_L_H, BLACK_L_S, BLACK_L_V, BLACK_R_H, BLACK_R_S, BLACK_R_V
    global WHITE_L_H, WHITE_L_S, WHITE_L_V, WHITE_R_H, WHITE_R_S, WHITE_R_V
    ev3.speaker.beep()

    # 1. Calibrar Blanco General (LSA + Sensores Color)
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "1. Blanco Gral")
    ev3.screen.draw_text(0, 30, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(0, 55, "Midiendo...")
    ev3.speaker.beep(400, 100)
    h_l_sum, s_l_sum, v_l_sum = 0, 0, 0
    h_r_sum, s_r_sum, v_r_sum = 0, 0, 0
    for _ in range(20):
        raw = ll.raw()
        for i in range(8): WHITE[i] += raw[i]
        
        rl, gl, bl = color_izq.rgb()
        hl, sl, vl = rgb_to_hsv(rl, gl, bl)
        h_l_sum += hl
        s_l_sum += sl
        v_l_sum += vl
        
        rr, gr, br = color_der.rgb()
        hr, sr, vr = rgb_to_hsv(rr, gr, br)
        h_r_sum += hr
        s_r_sum += sr
        v_r_sum += vr
        wait(20)
        
    for i in range(8): WHITE[i] = WHITE[i] // 20
    WHITE_L_H = h_l_sum // 20
    WHITE_L_S = s_l_sum // 20
    WHITE_L_V = v_l_sum // 20
    WHITE_R_H = h_r_sum // 20
    WHITE_R_S = s_r_sum // 20
    WHITE_R_V = v_r_sum // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Blanco OK")
    ev3.screen.draw_text(0, 30, "LSA: " + " ".join(str(v) for v in WHITE[0:4]))
    ev3.screen.draw_text(0, 55, "I: {}, {}, {}".format(WHITE_L_H, WHITE_L_S, WHITE_L_V))
    ev3.screen.draw_text(0, 80, "D: {}, {}, {}".format(WHITE_R_H, WHITE_R_S, WHITE_R_V))
    ev3.screen.draw_text(0, 105, "Click p/seguir")
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

    # 3. Calibrar Negro Color (S2 y S4)
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "3. Negro S2/S4")
    ev3.screen.draw_text(10, 50, "Poner S2/S4 s/NEGRO")
    ev3.screen.draw_text(10, 70, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(10, 90, "Midiendo...")
    ev3.speaker.beep(500, 100)
    h_l_sum, s_l_sum, v_l_sum = 0, 0, 0
    h_r_sum, s_r_sum, v_r_sum = 0, 0, 0
    for _ in range(20):
        rl, gl, bl = color_izq.rgb()
        hl, sl, vl = rgb_to_hsv(rl, gl, bl)
        h_l_sum += hl
        s_l_sum += sl
        v_l_sum += vl
        
        rr, gr, br = color_der.rgb()
        hr, sr, vr = rgb_to_hsv(rr, gr, br)
        h_r_sum += hr
        s_r_sum += sr
        v_r_sum += vr
        wait(20)
        
    BLACK_L_H = h_l_sum // 20
    BLACK_L_S = s_l_sum // 20
    BLACK_L_V = v_l_sum // 20
    BLACK_R_H = h_r_sum // 20
    BLACK_R_S = s_r_sum // 20
    BLACK_R_V = v_r_sum // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Negro Color OK")
    ev3.screen.draw_text(0, 40, "I: {}, {}, {}".format(BLACK_L_H, BLACK_L_S, BLACK_L_V))
    ev3.screen.draw_text(0, 70, "D: {}, {}, {}".format(BLACK_R_H, BLACK_R_S, BLACK_R_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 4. Calibrar T Izquierda
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "4. T IZQUIERDA")
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

    # 5. Calibrar T Derecha
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "5. T DERECHA")
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

    # 6. Calibrar Verde Izquierdo (Sensor Color Puerto 2)
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "6. Verde Izq (S2)")
    ev3.screen.draw_text(10, 50, "Poner sensor IZQ")
    ev3.screen.draw_text(10, 70, "sobre VERDE")
    ev3.screen.draw_text(10, 90, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(10, 110, "Midiendo...")
    ev3.speaker.beep(600, 100)
    h_sum, s_sum, v_sum = 0, 0, 0
    for _ in range(20):
        r, g, b = color_izq.rgb()
        h, s, v = rgb_to_hsv(r, g, b)
        h_sum += h
        s_sum += s
        v_sum += v
        wait(20)
    GREEN_L_H = h_sum // 20
    GREEN_L_S = s_sum // 20
    GREEN_L_V = v_sum // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Verde Izq OK")
    ev3.screen.draw_text(0, 40, "HSV: {}, {}, {}".format(GREEN_L_H, GREEN_L_S, GREEN_L_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 7. Calibrar Verde Derecho (Sensor Color Puerto 4)
    ev3.screen.clear()
    ev3.screen.draw_text(10, 30, "7. Verde Der (S4)")
    ev3.screen.draw_text(10, 50, "Poner sensor DER")
    ev3.screen.draw_text(10, 70, "sobre VERDE")
    ev3.screen.draw_text(10, 90, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed():
        wait(20)

    ev3.screen.draw_text(10, 110, "Midiendo...")
    ev3.speaker.beep(600, 100)
    h_sum, s_sum, v_sum = 0, 0, 0
    for _ in range(20):
        r, g, b = color_der.rgb()
        h, s, v = rgb_to_hsv(r, g, b)
        h_sum += h
        s_sum += s
        v_sum += v
        wait(20)
    GREEN_R_H = h_sum // 20
    GREEN_R_S = s_sum // 20
    GREEN_R_V = v_sum // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 10, "Verde Der OK")
    ev3.screen.draw_text(0, 40, "HSV: {}, {}, {}".format(GREEN_R_H, GREEN_R_S, GREEN_R_V))
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
ev3.screen.draw_text(0, 10, "INICIO (VERDES)")
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
