# =====================================================================
#  CALIBRAR UMBRALES — herramienta de banco para el OpenMV IDE
#  IITA Salta · fútbol Roboliga 2026 · 2026-08-04
# =====================================================================
#
#  ESTO NO ES EL PROGRAMA DEL ROBOT. Es una herramienta para mirar.
#  No manda nada por el cable serie: mientras corra esto, el Teensy va a
#  decir "NO LLEGAN DATOS DE LA CAMARA". Es normal, no está roto.
#
#  Se abre con el OpenMV IDE, se aprieta play, y se mira el terminal.
#
#  QUÉ CONTESTA, EN ESTE ORDEN (el orden importa — ver README.md):
#    1. Con qué exposición está mirando   -> y la CONGELA
#    2. Qué color es de verdad la pelota  -> te da el umbral LAB sugerido
#    3. Cuántas manchas naranjas ve       -> tiene que ver UNA
#    4. Cuántos píxeles mide la pelota    -> de ahí sale pixels_threshold
#
#  La pregunta que guía todo es "¿CUÁNTAS VEO?", no "¿está bien el umbral?".
#  Contar es fácil y no se presta a discusión. Juzgar si un umbral "está
#  bien" es opinión, y la opinión no se puede anotar en la bitácora.
# =====================================================================

import sensor, image, time, math
import pyb

# ============================ PERILLAS ===============================

# --- exposición ---
# 0  = dejo que el automático elija, leo el valor, y lo congelo ahí.
#      ASÍ SE ARRANCA. El número que imprima se anota y se pone acá abajo.
# >0 = congelo en este valor (microsegundos).
EXPOSICION_US = 0

# --- el recuadro donde se mide el color de la pelota ---
# Se dibuja blanco en pantalla. Es CHICO a propósito: la pelota tiene que
# TAPARLO ENTERO. Si le entra piso, el umbral sale demasiado ancho y vas a
# terminar detectando el piso — que es exactamente el problema que venimos
# a arreglar. Acercá la pelota hasta que no se vea nada del recuadro.
ROI_X, ROI_Y, ROI_W, ROI_H = 150, 110, 20, 20

# --- el umbral que estás probando ---
# Arranca con el de 2025 para ver cuánto se equivoca con la luz de hoy.
# Después lo reemplazás por el que sugiere la herramienta y volvés a mirar.
NARANJA = (21, 67, 18, 79, -32, 127)

# --- los dos tamaños mínimos que se comparan ---
PIX_HOY   = 7     # el que tiene el script de competencia hoy
PIX_NUEVO = 40    # el candidato. Se ajusta con lo que mida la pelota LEJOS.

# Cada cuánto imprime, en segundos. Más rápido que esto no se alcanza a leer.
CADA_SEG = 0.5

# =====================================================================


# --------------------- arranque de la cámara -------------------------

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)              # 320 x 240

# Igual que el script de competencia. Si esto no coincide, la homografía
# de más abajo devuelve cualquier cosa.
sensor.set_hmirror(True)
sensor.set_vflip(True)

print("")
print("=====================================================")
print(" CALIBRAR UMBRALES  ---  apuntá la camara A LA CANCHA")
print("=====================================================")
print("Dejo 3 segundos que el automatico se acomode...")

sensor.set_auto_gain(True)
sensor.set_auto_whitebal(True)
sensor.set_auto_exposure(True)
sensor.skip_frames(time=3000)


# ------------------- congelar: el paso que faltaba --------------------
#
# POR QUÉ SE CONGELA. Con ganancia y balance de blancos en automático la
# cámara se re-acomoda sola a TODA la escena. Cuando el robot gira y le
# entra una ventana, o el arco oscuro, o una pared blanca, cambia la
# ganancia — y el color de la pelota CAMBIA EN LOS NÚMEROS aunque la
# pelota y la luz no hayan cambiado. Un umbral calibrado así no dura ni
# una vuelta del robot.
#
# El costo de congelar: si cambia la luz del lugar, hay que recalibrar.
# Ese costo se paga en el box, cuando vos querés, y tarda 5 minutos.
# El otro se paga solo, en el medio de un partido.

def leer(nombre, funcion):
    """Lee un valor de la camara y AVISA FUERTE si no se puede."""
    try:
        v = funcion()
        print("   %-22s = %s" % (nombre, v))
        return v
    except Exception as e:
        print("   !!! NO PUDE LEER %s -> %s" % (nombre, e))
        print("   !!! anotalo en la bitacora y segui: sigue sirviendo")
        return None

print("")
print("--- lo que eligio el automatico ---")
exp_auto = leer("exposicion (us)",  sensor.get_exposure_us)
gan_auto = leer("ganancia (dB)",    sensor.get_gain_db)
wb_auto  = leer("balance RGB (dB)", sensor.get_rgb_gain_db)

if EXPOSICION_US > 0:
    exp_usar = EXPOSICION_US
    print("--- CONGELO con la exposicion puesta a mano: %d us" % exp_usar)
else:
    exp_usar = exp_auto if exp_auto else 20000
    print("--- CONGELO con la que eligio el automatico: %d us" % int(exp_usar))
    print("    ANOTALA y ponela arriba en EXPOSICION_US.")

sensor.set_auto_exposure(False, exposure_us=int(exp_usar))

if gan_auto is not None:
    sensor.set_auto_gain(False, gain_db=gan_auto)
else:
    sensor.set_auto_gain(False)

if wb_auto is not None:
    sensor.set_auto_whitebal(False, rgb_gain_db=wb_auto)
else:
    sensor.set_auto_whitebal(False)

sensor.skip_frames(time=500)
print("--- congelado. De aca en adelante los numeros son comparables.")
print("")
print("TAPÁ EL RECUADRO BLANCO CON LA PELOTA.")
print("")


# ------------------ la misma cuenta que hace el robot -----------------
# Copiada tal cual de robots-2025/vision-openmv/
# enviar_coordenadas_2_arcos_y_pelota.py, para que los centimetros que ves
# aca sean LOS MISMOS que le llegan al Teensy.

H = [[ 4.49341044e-02, -9.48228474e-01,  7.78932109e+02],
     [-2.39913185e+00, -5.65934886e-02,  3.91128921e+02],
     [-1.81344856e-03,  1.15408531e-01,  1.00000000e+00]]

ALTURA_CAM   = 18.7                  # cm
RADIO_PELOTA = 13.5 / (2 * math.pi)  # cm  (13,5 cm de contorno)
FACTOR = (ALTURA_CAM - RADIO_PELOTA) / ALTURA_CAM

def a_centimetros(u, v):
    den = H[2][0]*u + H[2][1]*v + H[2][2]
    x = (H[0][0]*u + H[0][1]*v + H[0][2]) / den
    y = (H[1][0]*u + H[1][1]*v + H[1][2]) / den
    return x * FACTOR, y * FACTOR


# Dos filas de la imagen que conviene tener dibujadas:
#   v = 33  (ROJA)     ahi la cuenta da 150 cm = XP_MAX del firmware.
#                      Todo lo que este MAS ARRIBA el robot ya lo tira.
#   v = 84  (AMARILLA) ahi da 60 cm.
# Entre las dos lineas UN SOLO PIXEL vale entre 0,7 y 4,5 cm: es la zona
# donde una manchita miente muchisimo. Abajo de la amarilla, un pixel vale
# menos de 0,7 cm y la medicion es mucho mas firme.
FILA_150CM = 33
FILA_60CM  = 84

clock = time.clock()
t_ultimo = time.ticks_ms()

while True:
    clock.tick()
    img = sensor.snapshot()

    # ---- qué ve con el umbral que estás probando ----
    #   pixels_threshold BAJO a proposito: quiero VER la basura, no
    #   esconderla. Lo que no ves, no sabes que esta.
    blobs = img.find_blobs([NARANJA],
                           pixels_threshold=PIX_HOY,
                           area_threshold=PIX_HOY,
                           merge=True)
    grandes = [b for b in blobs if b.pixels() >= PIX_NUEVO]

    # ---- dibujo (cada cuadro, para que el video se vea fluido) ----
    img.draw_rectangle((ROI_X, ROI_Y, ROI_W, ROI_H), color=(255,255,255))
    img.draw_line((0, FILA_150CM, 320, FILA_150CM), color=(255,0,0))
    img.draw_line((0, FILA_60CM,  320, FILA_60CM),  color=(255,255,0))
    for b in blobs:
        c = (0,255,0) if b.pixels() >= PIX_NUEVO else (255,0,255)
        img.draw_rectangle(b.rect(), color=c)
        img.draw_cross(b.cx(), b.cy(), color=c)

    # ---- imprimo cada tanto, no cada cuadro ----
    if time.ticks_diff(time.ticks_ms(), t_ultimo) < int(CADA_SEG * 1000):
        continue
    t_ultimo = time.ticks_ms()

    st = img.get_statistics(roi=(ROI_X, ROI_Y, ROI_W, ROI_H))

    print("---------------------------------------------- %.0f fps" % clock.fps())
    print("EN EL RECUADRO   L: %3d..%3d   A: %4d..%4d   B: %4d..%4d"
          % (st.l_min(), st.l_max(),
             st.a_min(), st.a_max(),
             st.b_min(), st.b_max()))
    print("  UMBRAL ANCHO    (%d, %d, %d, %d, %d, %d)      <-- EMPEZÁ ACÁ"
          % (st.l_min(), st.l_max(), st.a_min(), st.a_max(), st.b_min(), st.b_max()))
    try:
        print("  UMBRAL APRETADO (%d, %d, %d, %d, %d, %d)      <-- si ve manchas, pasá a este"
              % (st.l_lq(), st.l_uq(), st.a_lq(), st.a_uq(), st.b_lq(), st.b_uq()))
    except Exception as e:
        print("  (no pude calcular el apretado: %s)" % e)

    print("NARANJAS: %d con el minimo de hoy (%d px)  |  %d con el candidato (%d px)   <-- QUEREMOS 1"
          % (len(blobs), PIX_HOY, len(grandes), PIX_NUEVO))

    if not blobs:
        print("   (no veo nada naranja)")
    for b in sorted(blobs, key=lambda z: -z.pixels()):
        X, Y = a_centimetros(b.cx(), b.cy())
        marca = "PELOTA?" if b.pixels() >= PIX_NUEVO else "mancha "
        alto = "  <-- ARRIBA DE LA ROJA: el firmware lo descarta" if b.cy() < FILA_150CM else ""
        print("   %s px=%5d  en (u=%3d, v=%3d)  ->  Xp=%6.1f cm  Yp=%6.1f cm%s"
              % (marca, b.pixels(), b.cx(), b.cy(), X, Y, alto))
