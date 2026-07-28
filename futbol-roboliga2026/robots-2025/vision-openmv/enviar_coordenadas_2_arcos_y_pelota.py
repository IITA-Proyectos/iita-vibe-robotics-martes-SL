from pyb import UART
import sensor, image, time
import math

# ----- Configuración UART -----
uart = UART(3, 19200)

# ----- LEDs ------
import pyb

led_rojo = pyb.LED(1)    # Naranja / Pelota
led_verde = pyb.LED(2)   # Amarillo / Arco derecho
led_azul = pyb.LED(3)    # Azul / Arco izquierdo

# Parpadea el LED verde.
led_verde.on()
time.sleep(0.4)
led_verde.off()
time.sleep(0.4)
led_verde.on()
time.sleep(0.4)
led_verde.off()


# ----- Inicializar cámara -----
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

# Calibrar balance de blancos automáticamente primero
sensor.set_auto_whitebal(True)
sensor.set_auto_gain(True)
#sensor.set_auto_exposure(True)

# Esperar para que se ajusten bien los valores de color
sensor.skip_frames(time=2000)

# Ajustá exposure_us si la imagen queda muy oscura o muy brillante.
#sensor.set_auto_exposure(False, exposure_us=37000)  # (opcional: exposición fija 15ms)

# invertir imagen
sensor.set_hmirror(True)
sensor.set_vflip(True)

clock = time.clock()     # Para medir FPS

# ----- Constantes físicas -----
h = 18.7       # altura cámara (cm)
r = 13.5/(2*math.pi)  # radio pelota (cm)

# ----- Umbrales LAB -----

# threshold([30, 80, -10, 10, 20, 70])
# L (Luminicidad): 30 a 80 → píxeles de brillo medio
# A (Canal de color verde–rojo) : -10 a 10 → colores casi neutros en rojo-verde
# B (Canal de color azul–amarillo) : 20 a 70 → tonos amarillos moderados

naranja_threshold = (21, 67, 18, 79, -32, 127)    # Pelota naranja
amarillo_threshold = (17, 70, -27, 14, 38, 111) # Arco amarillo
azul_threshold = (4, 36, -13, 57, -64, -4)    # Arco azul


# ----- Funciones -----

def transformarcoordenadas(u, v):
    # Matriz de transformación homográfica (ajustar)
    H = [[ 4.49341044e-02, -9.48228474e-01,  7.78932109e+02],
         [-2.39913185e+00, -5.65934886e-02,  3.91128921e+02],
         [-1.81344856e-03,  1.15408531e-01,  1.00000000e+00]]

    # Aplicar transformación homográfica manualmente
    denominator = H[2][0]*u + H[2][1]*v + H[2][2]
    x = (H[0][0]*u + H[0][1]*v + H[0][2]) / denominator
    y = (H[1][0]*u + H[1][1]*v + H[1][2]) / denominator

    return x, y


def procesar_blob(blobs, dibujar_color):
    if not blobs:
        return 0, 0  # Si no hay detección, se manda 0

    # Seleccionar el blob más grande (asumimos que es el arco o la pelota)
    largest_blob = max(blobs, key=lambda b: b.pixels())

    # Dibujar un rectángulo alrededor del blob detectado
    img.draw_rectangle(largest_blob.rect(), color=dibujar_color)

    # Dibujar una cruz en el centro del blob
    img.draw_cross(largest_blob.cx(), largest_blob.cy(), color=(255,255,255))

    # Transformacion geometrica
    u, v = largest_blob.cx(), largest_blob.cy()
    x, y = transformarcoordenadas(u, v)

    X = x * (h - r) / h
    Y = y * (h - r) / h

    # Por si las coordenadas se pasan de la cota
    if X>200:
        X = 200
    if Y+100>200:
        Y = 100
    if Y+100<0:
        Y = -100

    return X, Y  # Retornamos coordenadas reales (cm)


# ----- Bucle principal -----

while(True):
    clock.tick()
    img = sensor.snapshot()
    #img.binary([amarillo_threshold])
    # Línea vertical blanca
    #img.draw_line((160,0,160,240), color=(255,255,255))

    # Detectar blobs
    naranja_blobs = img.find_blobs([naranja_threshold], pixels_threshold=7, area_threshold=7, merge=True)
    azul_blobs = img.find_blobs([azul_threshold], pixels_threshold=300, area_threshold=300, merge=True)
    amarillo_blobs = img.find_blobs([amarillo_threshold], pixels_threshold=600, area_threshold=600, merge=True)

    # Encender LEDs según detección
    led_rojo.off()
    if naranja_blobs:
        led_rojo.on()

    led_azul.off()
    if azul_blobs:
        led_azul.on()

    led_verde.off()
    if amarillo_blobs:
        led_verde.on()

    # Procesar blobs
    Xp, Yp = procesar_blob(naranja_blobs, (255,0,0))
    Xaz, Yaz = procesar_blob(azul_blobs, (0,0,255))
    Xam, Yam = procesar_blob(amarillo_blobs, (0,255,0))

    # Codificar Y
    codedYp = Yp + 100
    codedYam = Yam + 100
    codedYaz = Yaz + 100

    # ----------------------------
    # ⚠️ Armar paquete UART
    packet = [
        201, int(Xp), int(codedYp),
        202, int(Xam), int(codedYam),
        203, int(Xaz), int(codedYaz)
    ]

    uart.write(bytearray(packet))
    print("Enviando:", packet)
