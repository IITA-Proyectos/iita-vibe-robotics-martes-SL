# ╔══════════════════════════════════════════════════════════════════╗
# ║  CALIBRACIÓN RESCUE LINE — Leer valores de sensores            ║
# ║  Posar el robot sobre cada superficie y anotar los valores     ║
# ║  Sensores: Puerto C (izquierdo), Puerto D (derecho)            ║
# ╚══════════════════════════════════════════════════════════════════╝
#
# PROCEDIMIENTO:
#   1. Correr el programa
#   2. Posar sensores sobre BLANCO → anotar Ref y HSV
#   3. Posar sensores sobre NEGRO → anotar Ref y HSV
#   4. Posar sensores sobre VERDE → anotar Ref y HSV (¡el más importante!)
#   5. Posar sensores sobre GRIS (si hay) → verificar que NO da verde
#   6. Usar estos valores para ajustar los parámetros en rescue-line-2sensor.py
#
# TIPS:
#   - Calibrar en la superficie REAL de competencia
#   - Probar verde desde distintos ángulos (los valores cambian un poco)
#   - El verde debería tener: H entre 100-200, S > 25, V entre 15-75
#   - El negro debería tener: Ref < 20, V < 15
#   - El blanco debería tener: Ref > 65, S < 20

from pybricks.hubs import PrimeHub
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port, Color, Button
from pybricks.tools import wait

hub = PrimeHub()
sensor_izq = ColorSensor(Port.D)
sensor_der = ColorSensor(Port.C)

hub.light.on(Color.CYAN)
hub.speaker.beep(1000, 200)

print("=== CALIBRACIÓN RESCUE LINE ===")
print("Posar sensores sobre cada superficie")
print("Apretar botón central para capturar muestra")
print("")

muestra = 0

while True:
    # Lectura continua
    ref_i = sensor_izq.reflection()
    ref_d = sensor_der.reflection()
    hi, si, vi = sensor_izq.hsv()
    hd, sd, vd = sensor_der.hsv()

    # Mostrar en consola
    print("IZQ: Ref=", ref_i, "H=", hi, "S=", si, "V=", vi,
          " | DER: Ref=", ref_d, "H=", hd, "S=", sd, "V=", vd)

    # Si aprietan el botón, capturar muestra formal
    if Button.CENTER in hub.buttons.pressed():
        muestra += 1
        print("")
        print("══ MUESTRA", muestra, "══")
        print("  IZQ: Ref=", ref_i, "H=", hi, "S=", si, "V=", vi)
        print("  DER: Ref=", ref_d, "H=", hd, "S=", sd, "V=", vd)
        
        # Indicación visual
        if si > 25 and 100 < hi < 200 and vi > 12:
            print("  → Parece VERDE ✓")
            hub.light.on(Color.GREEN)
        elif ref_i < 20:
            print("  → Parece NEGRO")
            hub.light.on(Color.RED)
        elif ref_i > 65:
            print("  → Parece BLANCO")
            hub.light.on(Color.WHITE)
        else:
            print("  → INTERMEDIO (¿gris?)")
            hub.light.on(Color.YELLOW)
        
        print("")
        hub.speaker.beep(1500, 100)
        wait(800)  # Debounce
        hub.light.on(Color.CYAN)

    wait(200)
