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
GREEN_L_H, GREEN_L_S, GREEN_L_V = 145, 70, 25
GREEN_R_H, GREEN_R_S, GREEN_R_V = 130, 70, 25

BLACK_L_H, BLACK_L_S, BLACK_L_V = 99, 74, 38
BLACK_R_H, BLACK_R_S, BLACK_R_V = 87, 84, 27

WHITE_L_H, WHITE_L_S, WHITE_L_V = 192, 30, 81
WHITE_R_H, WHITE_R_S, WHITE_R_V = 173, 21, 71

# ── Funciones de Memoria y Aprendizaje ───────────────────────────

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

def registrar_experiencia(sensor_nombre, hsv_medido, hsv_nuevo):
    """Registra en experiencia_pista.json el aprendizaje progresivo del verde en pista."""
    registro = {
        "sensor": sensor_nombre,
        "hsv_medido": hsv_medido,
        "hsv_nuevo_promedio": hsv_nuevo
    }
    historial = []
    try:
        with open("experiencia_pista.json", "r") as f:
            historial = json.load(f)
    except:
        historial = []
        
    historial.append(registro)
    try:
        with open("experiencia_pista.json", "w") as f:
            json.dump(historial, f)
    except Exception as e:
        print("Error guardando experiencia:", e)

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

# ── Funciones de Interfaz Gráfica (Pantalla EV3 178x128 px) ───────

def dibujar_pantalla_evento(tipo):
    """Dibuja en la pantalla del EV3 (178x128 px) gráficos descriptivos según el evento detectado en pista."""
    ev3.screen.clear()
    # Marco exterior de pantalla
    ev3.screen.draw_box(1, 1, 176, 126)
    
    if tipo == "VERDE_IZQ":
        ev3.screen.draw_text(15, 8, "[ VERDE IZQ 90° ]")
        # Letra V grande a la derecha
        ev3.screen.draw_text(145, 50, "V")
        # Flecha apuntando a la izquierda (gruesa)
        ev3.screen.draw_line(135, 65, 40, 65)
        ev3.screen.draw_line(135, 66, 40, 66)
        ev3.screen.draw_line(135, 67, 40, 67)
        # Cabeza de flecha
        ev3.screen.draw_line(40, 66, 75, 35)
        ev3.screen.draw_line(40, 66, 75, 97)
        ev3.screen.draw_line(41, 66, 76, 35)
        ev3.screen.draw_line(41, 66, 76, 97)

    elif tipo == "VERDE_DER":
        ev3.screen.draw_text(15, 8, "[ VERDE DER 90° ]")
        # Letra V grande a la izquierda
        ev3.screen.draw_text(15, 50, "V")
        # Flecha apuntando a la derecha (gruesa)
        ev3.screen.draw_line(40, 65, 135, 65)
        ev3.screen.draw_line(40, 66, 135, 66)
        ev3.screen.draw_line(40, 67, 135, 67)
        # Cabeza de flecha
        ev3.screen.draw_line(135, 66, 100, 35)
        ev3.screen.draw_line(135, 66, 100, 97)
        ev3.screen.draw_line(134, 66, 99, 35)
        ev3.screen.draw_line(134, 66, 99, 97)

    elif tipo == "VERDE_DOBLE":
        ev3.screen.draw_text(10, 8, "[ DOBLE VERDE 180° ]")
        ev3.screen.draw_text(70, 75, "V V")
        # Retorno en U (U-Turn)
        ev3.screen.draw_line(55, 110, 55, 55)
        ev3.screen.draw_line(56, 110, 56, 55)
        ev3.screen.draw_line(55, 55, 120, 55)
        ev3.screen.draw_line(55, 54, 120, 54)
        ev3.screen.draw_line(120, 55, 120, 110)
        ev3.screen.draw_line(121, 55, 121, 110)
        # Cabeza de flecha hacia abajo
        ev3.screen.draw_line(120, 110, 100, 90)
        ev3.screen.draw_line(120, 110, 140, 90)
        ev3.screen.draw_line(120, 109, 100, 89)
        ev3.screen.draw_line(120, 109, 140, 89)

    elif tipo == "CRUZ":
        ev3.screen.draw_text(25, 8, "[ CRUZ NEGRA + ]")
        # Cruz grande (+)
        ev3.screen.draw_line(89, 30, 89, 105)
        ev3.screen.draw_line(90, 30, 90, 105)
        ev3.screen.draw_line(91, 30, 91, 105)
        ev3.screen.draw_line(50, 67, 128, 67)
        ev3.screen.draw_line(50, 68, 128, 68)
        ev3.screen.draw_line(50, 69, 128, 69)

    elif tipo == "T_IZQ":
        ev3.screen.draw_text(25, 8, "[ T IZQUIERDA ]")
        # Diagrama T con bifurcación a la izquierda
        ev3.screen.draw_line(115, 30, 115, 105)
        ev3.screen.draw_line(116, 30, 116, 105)
        ev3.screen.draw_line(117, 30, 117, 105)
        ev3.screen.draw_line(45, 67, 115, 67)
        ev3.screen.draw_line(45, 68, 115, 68)
        ev3.screen.draw_line(45, 69, 115, 69)

    elif tipo == "T_DER":
        ev3.screen.draw_text(25, 8, "[ T DERECHA ]")
        # Diagrama T con bifurcación a la derecha
        ev3.screen.draw_line(60, 30, 60, 105)
        ev3.screen.draw_line(61, 30, 61, 105)
        ev3.screen.draw_line(62, 30, 62, 105)
        ev3.screen.draw_line(60, 67, 130, 67)
        ev3.screen.draw_line(60, 68, 130, 68)
        ev3.screen.draw_line(60, 69, 130, 69)

    elif tipo == "CURVA_DESCARTE":
        ev3.screen.draw_text(15, 8, "[ CURVA DE 90° ]")
        ev3.screen.draw_text(20, 50, "Retrocediendo 25mm")
        ev3.screen.draw_text(25, 80, "Resolviendo PID")

    elif tipo == "RECOVERY":
        ev3.screen.draw_text(25, 8, "[ ! PERDIDO ! ]")
        ev3.screen.draw_box(65, 30, 112, 105)
        ev3.screen.draw_text(84, 45, "!")
        ev3.screen.draw_text(84, 80, ".")

    elif tipo == "PAUSA":
        ev3.screen.draw_text(45, 8, "[ PAUSA ]")
        ev3.screen.draw_box(30, 30, 148, 85)
        # Símbolo de Pausa || (barras gruesas)
        ev3.screen.draw_line(75, 40, 75, 75)
        ev3.screen.draw_line(76, 40, 76, 75)
        ev3.screen.draw_line(77, 40, 77, 75)
        ev3.screen.draw_line(100, 40, 100, 75)
        ev3.screen.draw_line(101, 40, 101, 75)
        ev3.screen.draw_line(102, 40, 102, 75)
        ev3.screen.draw_text(15, 95, "CENTRO p/Continuar")

# ── Algoritmo Principal ──────────────────────────────────────────

def follow_line(confirmar_verdes=False):
    global GREEN_L_H, GREEN_L_S, GREEN_L_V, GREEN_R_H, GREEN_R_S, GREEN_R_V
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
            # --- BOTÓN DE PAUSA (IZQ o DER) ---
            botones_presionados = ev3.buttons.pressed()
            if Button.LEFT in botones_presionados or Button.RIGHT in botones_presionados:
                drive.stop()
                ev3.speaker.beep(400, 150)
                dibujar_pantalla_evento("PAUSA")
                
                # Esperar a que se suelte el botón IZQ / DER
                while Button.LEFT in ev3.buttons.pressed() or Button.RIGHT in ev3.buttons.pressed():
                    wait(20)
                
                # Esperar a que apreten CENTRO para salir de la pausa
                while Button.CENTER not in ev3.buttons.pressed():
                    wait(20)
                    
                # Esperar a que se suelte el botón CENTRO
                while Button.CENTER in ev3.buttons.pressed():
                    wait(20)
                    
                ev3.speaker.beep(800, 200)
                last_err = 0
                integral = 0
                turn = 0
                continue

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
                dibujar_pantalla_evento("CRUZ")
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
                    # 1. Parar de inmediato sobre el verde
                    drive.stop()
                    det_l = green_l
                    det_r = green_r
                    
                    # 2. Si estamos en Modo Recolección de Datos -> Pedir confirmación manual, actualizar 70/30 y registrar historial
                    if confirmar_verdes:
                        hl_m, sl_m, vl_m = h_l, s_l, v_l
                        hr_m, sr_m, vr_m = h_r, s_r, v_r
                        
                        ev3.screen.clear()
                        ev3.screen.draw_box(1, 1, 176, 126)
                        ev3.screen.draw_text(15, 8, "¿Es VERDE REAL?")
                        if det_l and det_r:
                            ev3.screen.draw_text(15, 30, "DOBLE VERDE")
                            ev3.screen.draw_text(10, 52, "I:{},{},{}".format(hl_m, sl_m, vl_m))
                            ev3.screen.draw_text(10, 72, "D:{},{},{}".format(hr_m, sr_m, vr_m))
                        elif det_l:
                            ev3.screen.draw_text(15, 30, "VERDE IZQ (S2)")
                            ev3.screen.draw_text(10, 60, "HSV:{},{},{}".format(hl_m, sl_m, vl_m))
                        elif det_r:
                            ev3.screen.draw_text(15, 30, "VERDE DER (S4)")
                            ev3.screen.draw_text(10, 60, "HSV:{},{},{}".format(hr_m, sr_m, vr_m))
                        
                        ev3.screen.draw_text(10, 100, "IZQ: SI    DER: NO")
                        ev3.speaker.beep(1000, 200)
                        
                        es_verde_real = None
                        while True:
                            b_pressed = ev3.buttons.pressed()
                            if Button.LEFT in b_pressed:
                                es_verde_real = True
                                break
                            elif Button.RIGHT in b_pressed:
                                es_verde_real = False
                                break
                            wait(20)
                            
                        while any(ev3.buttons.pressed()):
                            wait(20)
                            
                        if not es_verde_real:
                            ev3.speaker.beep(300, 200)
                            ev3.screen.clear()
                            ev3.screen.draw_text(10, 50, "DESCARTADO")
                            wait(500)
                            t_cooldown_green = 40
                            continue

                        # Actualizar promedio 70/30, registrar historial y guardar calibración SOLO en Modo Recolección
                        if det_l:
                            GREEN_L_H = int(0.70 * GREEN_L_H + 0.30 * hl_m)
                            GREEN_L_S = int(0.70 * GREEN_L_S + 0.30 * sl_m)
                            GREEN_L_V = int(0.70 * GREEN_L_V + 0.30 * vl_m)
                            registrar_experiencia("IZQ", [hl_m, sl_m, vl_m], [GREEN_L_H, GREEN_L_S, GREEN_L_V])
                            
                        if det_r:
                            GREEN_R_H = int(0.70 * GREEN_R_H + 0.30 * hr_m)
                            GREEN_R_S = int(0.70 * GREEN_R_S + 0.30 * sr_m)
                            GREEN_R_V = int(0.70 * GREEN_R_V + 0.30 * vr_m)
                            registrar_experiencia("DER", [hr_m, sr_m, vr_m], [GREEN_R_H, GREEN_R_S, GREEN_R_V])
                            
                        guardar_calibracion()
                        
                        ev3.speaker.beep(1200, 150)
                        ev3.screen.clear()
                        ev3.screen.draw_text(10, 40, "Guardado OK!")
                        wait(400)
                    
                    # 3. Avanzar la distancia de alineación (88 mm) para colocar ruedas en intersección y girar
                    drive.straight(GREEN_ALIGN_DISTANCE)
                    wait(50)
                    
                    # Decidir acción de giro
                    if det_l and det_r:
                        print("\n[VERDE] DOBLE VERDE -> Giro de 180°")
                        dibujar_pantalla_evento("VERDE_DOBLE")
                        drive.turn(180)
                    elif det_l:
                        print("\n[VERDE] VERDE IZQ -> Giro de 90° Izquierda")
                        dibujar_pantalla_evento("VERDE_IZQ")
                        drive.turn(90)
                    elif det_r:
                        print("\n[VERDE] VERDE DER -> Giro de 90° Derecha")
                        dibujar_pantalla_evento("VERDE_DER")
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
                            dibujar_pantalla_evento("T_IZQ")
                        else:
                            print("\nT Derecha CONFIRMADA - Siguiendo derecho")
                            dibujar_pantalla_evento("T_DER")
                        
                        # Avanzamos un poco más para superar completamente la bifurcación
                        drive.straight(15)
                        t_cooldown_t = 40
                    else:
                        # Si el centro se volvió blanco, es una curva (de cualquier ángulo)
                        # Retrocedemos los 25 mm para volver al inicio y resolver con PID
                        ev3.speaker.beep(300, 200)  # Sonido grave
                        print("\nCurva detectada - Retrocediendo 25 mm para resolver con PID")
                        dibujar_pantalla_evento("CURVA_DESCARTE")
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
                
                # Pantalla EV3 (Solo actualiza si no hay cooldowns activos de gráficos especiales)
                if t_cooldown_green == 0 and t_cooldown_t == 0:
                    ev3.screen.clear()
                    ev3.screen.draw_text(10, 20, "G:{} T:{}".format(t_cooldown_green, t_cooldown_t))
                    ev3.screen.draw_text(10, 50, "[" + viz + "]")
                    ev3.screen.draw_text(10, 80, info_str)
            
            # --- PID SEGÚN LA LÍNEA ---
            if pos is None:
                loss_counter += 1
                if loss_counter > 8:
                    # Recovery
                    dibujar_pantalla_evento("RECOVERY")
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

def calibrar_lsa():
    global WHITE, BLACK, T_L_MASK, T_R_MASK
    # 1. Blanco LSA
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ LSA ] 1. Blanco")
    ev3.screen.draw_text(0, 35, "LSA sobre BLANCO")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(400, 100)
    WHITE = [0]*8
    for _ in range(20):
        raw = ll.raw()
        for i in range(8): WHITE[i] += raw[i]
        wait(20)
    for i in range(8): WHITE[i] = WHITE[i] // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Blanco LSA OK")
    ev3.screen.draw_text(0, 35, " ".join(str(v) for v in WHITE[0:4]))
    ev3.screen.draw_text(0, 65, " ".join(str(v) for v in WHITE[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 2. Negro LSA
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ LSA ] 2. Negro")
    ev3.screen.draw_text(0, 35, "LSA sobre NEGRO")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(500, 100)
    BLACK = [0]*8
    for _ in range(20):
        raw = ll.raw()
        for i in range(8): BLACK[i] += raw[i]
        wait(20)
    for i in range(8): BLACK[i] = BLACK[i] // 20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Negro LSA OK")
    ev3.screen.draw_text(0, 35, " ".join(str(v) for v in BLACK[0:4]))
    ev3.screen.draw_text(0, 65, " ".join(str(v) for v in BLACK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 3. T Izquierda
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ LSA ] 3. T Izq")
    ev3.screen.draw_text(0, 35, "Colocar sobre T Izq")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
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
    ev3.screen.draw_text(0, 5, "T Izq OK")
    ev3.screen.draw_text(0, 35, " ".join("1" if b else "0" for b in T_L_MASK[0:4]))
    ev3.screen.draw_text(0, 65, " ".join("1" if b else "0" for b in T_L_MASK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 4. T Derecha
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ LSA ] 4. T Der")
    ev3.screen.draw_text(0, 35, "Colocar sobre T Der")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
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
    ev3.screen.draw_text(0, 5, "T Der OK")
    ev3.screen.draw_text(0, 35, " ".join("1" if b else "0" for b in T_R_MASK[0:4]))
    ev3.screen.draw_text(0, 65, " ".join("1" if b else "0" for b in T_R_MASK[4:8]))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

def calibrar_colores_frontales():
    global GREEN_L_H, GREEN_L_S, GREEN_L_V, GREEN_R_H, GREEN_R_S, GREEN_R_V
    global BLACK_L_H, BLACK_L_S, BLACK_L_V, BLACK_R_H, BLACK_R_S, BLACK_R_V
    global WHITE_L_H, WHITE_L_S, WHITE_L_V, WHITE_R_H, WHITE_R_S, WHITE_R_V

    # 1. Blanco Color (S2/S4)
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ COLOR ] 1. Blanco")
    ev3.screen.draw_text(0, 35, "S2/S4 sobre BLANCO")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(400, 100)
    h_l_sum, s_l_sum, v_l_sum = 0, 0, 0
    h_r_sum, s_r_sum, v_r_sum = 0, 0, 0
    for _ in range(20):
        rl, gl, bl = color_izq.rgb()
        hl, sl, vl = rgb_to_hsv(rl, gl, bl)
        h_l_sum += hl; s_l_sum += sl; v_l_sum += vl
        
        rr, gr, br = color_der.rgb()
        hr, sr, vr = rgb_to_hsv(rr, gr, br)
        h_r_sum += hr; s_r_sum += sr; v_r_sum += vr
        wait(20)
    WHITE_L_H, WHITE_L_S, WHITE_L_V = h_l_sum//20, s_l_sum//20, v_l_sum//20
    WHITE_R_H, WHITE_R_S, WHITE_R_V = h_r_sum//20, s_r_sum//20, v_r_sum//20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Blanco Color OK")
    ev3.screen.draw_text(0, 35, "I: {}, {}, {}".format(WHITE_L_H, WHITE_L_S, WHITE_L_V))
    ev3.screen.draw_text(0, 65, "D: {}, {}, {}".format(WHITE_R_H, WHITE_R_S, WHITE_R_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 2. Negro Color (S2/S4)
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ COLOR ] 2. Negro")
    ev3.screen.draw_text(0, 35, "S2/S4 sobre NEGRO")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(500, 100)
    h_l_sum, s_l_sum, v_l_sum = 0, 0, 0
    h_r_sum, s_r_sum, v_r_sum = 0, 0, 0
    for _ in range(20):
        rl, gl, bl = color_izq.rgb()
        hl, sl, vl = rgb_to_hsv(rl, gl, bl)
        h_l_sum += hl; s_l_sum += sl; v_l_sum += vl
        
        rr, gr, br = color_der.rgb()
        hr, sr, vr = rgb_to_hsv(rr, gr, br)
        h_r_sum += hr; s_r_sum += sr; v_r_sum += vr
        wait(20)
    BLACK_L_H, BLACK_L_S, BLACK_L_V = h_l_sum//20, s_l_sum//20, v_l_sum//20
    BLACK_R_H, BLACK_R_S, BLACK_R_V = h_r_sum//20, s_r_sum//20, v_r_sum//20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Negro Color OK")
    ev3.screen.draw_text(0, 35, "I: {}, {}, {}".format(BLACK_L_H, BLACK_L_S, BLACK_L_V))
    ev3.screen.draw_text(0, 65, "D: {}, {}, {}".format(BLACK_R_H, BLACK_R_S, BLACK_R_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 3. Verde Izquierdo (S2)
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ COLOR ] 3. Verde Izq")
    ev3.screen.draw_text(0, 35, "Sensor IZQ sobre VERDE")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(600, 100)
    h_sum, s_sum, v_sum = 0, 0, 0
    for _ in range(20):
        r, g, b = color_izq.rgb()
        h, s, v = rgb_to_hsv(r, g, b)
        h_sum += h; s_sum += s; v_sum += v
        wait(20)
    GREEN_L_H, GREEN_L_S, GREEN_L_V = h_sum//20, s_sum//20, v_sum//20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Verde Izq OK")
    ev3.screen.draw_text(0, 35, "HSV: {}, {}, {}".format(GREEN_L_H, GREEN_L_S, GREEN_L_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

    # 4. Verde Derecho (S4)
    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "[ COLOR ] 4. Verde Der")
    ev3.screen.draw_text(0, 35, "Sensor DER sobre VERDE")
    ev3.screen.draw_text(0, 65, "Apretar CENTRO")
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    ev3.screen.draw_text(0, 95, "Midiendo...")
    ev3.speaker.beep(600, 100)
    h_sum, s_sum, v_sum = 0, 0, 0
    for _ in range(20):
        r, g, b = color_der.rgb()
        h, s, v = rgb_to_hsv(r, g, b)
        h_sum += h; s_sum += s; v_sum += v
        wait(20)
    GREEN_R_H, GREEN_R_S, GREEN_R_V = h_sum//20, s_sum//20, v_sum//20

    ev3.screen.clear()
    ev3.screen.draw_text(0, 5, "Verde Der OK")
    ev3.screen.draw_text(0, 35, "HSV: {}, {}, {}".format(GREEN_R_H, GREEN_R_S, GREEN_R_V))
    ev3.screen.draw_text(0, 100, "Click p/seguir")
    while Button.CENTER in ev3.buttons.pressed(): wait(20)
    while Button.CENTER not in ev3.buttons.pressed(): wait(20)
    while Button.CENTER in ev3.buttons.pressed(): wait(20)

def calibracion_manual(modo="COMPLETA"):
    ev3.speaker.beep()
    # Cargar valores previos para no sobreescribir con ceros los sensores que no se calibran
    cargar_calibracion()
    
    if modo == "COMPLETA":
        calibrar_lsa()
        calibrar_colores_frontales()
    elif modo == "LSA":
        calibrar_lsa()
    elif modo == "COLOR":
        calibrar_colores_frontales()

    guardar_calibracion()
    ev3.screen.clear()
    ev3.screen.draw_text(0, 40, "Guardado OK!")
    ev3.speaker.beep(1000, 200)
    wait(1000)

# --- MENÚ DE INICIO DE CALIBRACIÓN ---
ev3.screen.clear()
ev3.screen.draw_text(0, 5, "=== CALIBRACION ===")
ev3.screen.draw_text(0, 30, "ARRIBA : Memoria")
ev3.screen.draw_text(0, 55, "ABAJO  : Todo (7 pasos)")
ev3.screen.draw_text(0, 80, "IZQ    : Solo LSA (Piso)")
ev3.screen.draw_text(0, 105, "DER    : Solo Color (S2/S4)")

opcion = None
while True:
    botones = ev3.buttons.pressed()
    if Button.UP in botones:
        opcion = "MEMORIA"
        break
    elif Button.DOWN in botones:
        opcion = "TODO"
        break
    elif Button.LEFT in botones:
        opcion = "LSA"
        break
    elif Button.RIGHT in botones:
        opcion = "COLOR"
        break
    wait(20)

while any(ev3.buttons.pressed()):
    wait(20)

if opcion == "MEMORIA":
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
elif opcion == "TODO":
    calibracion_manual("COMPLETA")
elif opcion == "LSA":
    calibracion_manual("LSA")
elif opcion == "COLOR":
    calibracion_manual("COLOR")

# 3. Espera inicio con debug
ev3.screen.clear()
ev3.screen.draw_text(0, 5, "=== LISTO A PISTA ===")
ev3.screen.draw_text(0, 30, "CENTRO : Modo Normal")
ev3.screen.draw_text(0, 55, "OTRO   : Recolectar (Data)")

confirmar_verdes = False
while True:
    botones = ev3.buttons.pressed()
    if Button.CENTER in botones:
        confirmar_verdes = False
        break
    elif any(b for b in botones if b != Button.CENTER):
        confirmar_verdes = True
        break
        
    try:
        cal = normalize_array(ll.raw())
        pos = pos_x10(cal)
        ev3.screen.draw_text(0, 85, "Posicion: " + str(pos) + "   ")
    except OSError:
        ev3.screen.draw_text(0, 85, "Error I2C...       ")
    wait(50)

while any(ev3.buttons.pressed()):
    wait(20)

ev3.speaker.beep(800, 300)

# 4. Seguir línea
follow_line(confirmar_verdes=confirmar_verdes)

ev3.screen.clear()
ev3.screen.draw_text(10, 50, "LISTO")
ev3.speaker.beep(800, 300)
