"""Monitor del robot en vivo: muestra lo que dice y deja escribirle teclas.

A diferencia de serie.py, que escucha unos segundos y se va, este se queda
abierto hasta que lo cierres. Es lo mas parecido al Monitor Serie del
Arduino IDE, pero sin abrir el IDE.

  python mirar.py                 (busca el Teensy solo)
  python mirar.py --puerto COM7   (si queres forzarlo)

Se cierra con Ctrl+C.

Las teclas que escribas se le mandan al robot tal cual. Las utiles:

  M = prender/apagar el monitor en vivo (y frena el robot)
  T = predecir adonde va la pelota, si/no
  0 = PARAR             g = arrancar (avisa 10 s)
  i = resumen           ?  = la ayuda completa del robot
"""
import argparse
import sys
import time

import serial
from serial.tools import list_ports

# El Teensy no siempre cae en el mismo COM: cambia si se enchufa en otro
# conector USB. En vez de adivinar, se lo busca por el identificador del
# fabricante (PJRC = 0x16C0), que ese no cambia nunca.
VID_PJRC = 0x16C0


def buscar_teensy():
    for p in list_ports.comports():
        if p.vid == VID_PJRC:
            return p.device
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--puerto", default=None)
    ap.add_argument("--baud", type=int, default=19200)
    a = ap.parse_args()

    puerto = a.puerto or buscar_teensy()
    if not puerto:
        print("NO ENCONTRE NINGUN TEENSY ENCHUFADO.")
        print("Fijate que el cable USB este puesto, o pasa el puerto a mano:")
        print("    python mirar.py --puerto COM5")
        return 2

    try:
        s = serial.Serial(puerto, a.baud, timeout=0.05)
    except serial.SerialException as e:
        print(f"NO SE PUDO ABRIR {puerto}: {e}")
        return 2

    print("=" * 60)
    print(f" MONITOR DEL ARQUERO  —  {puerto} a {a.baud}")
    print("=" * 60)
    print(" Escribi una tecla y se le manda al robot. Las utiles:")
    print("   M = monitor en vivo (y frena el robot)")
    print("   T = predecir adonde va la pelota, si/no")
    print("   i = resumen        ? = ayuda completa")
    print("   0 = PARAR          g = arrancar")
    print()
    print(" Para salir: Ctrl+C")
    print("=" * 60)
    print()

    # Para leer el teclado sin frenar la lectura del puerto. Solo Windows,
    # que es donde corre esto.
    try:
        import msvcrt
        hay_teclado = True
    except ImportError:
        hay_teclado = False
        print("(sin teclado interactivo en este sistema: solo lectura)")

    try:
        with s:
            while True:
                n = s.in_waiting
                if n:
                    datos = s.read(n).decode("utf-8", "replace")
                    sys.stdout.write(datos)
                    sys.stdout.flush()

                if hay_teclado and msvcrt.kbhit():
                    tecla = msvcrt.getwch()
                    if tecla in ("\r", "\n"):
                        continue
                    s.write((tecla + "\n").encode())
                    s.flush()

                if not n:
                    time.sleep(0.01)
    except KeyboardInterrupt:
        print()
        print("--- cerrado ---")
        return 0
    except serial.SerialException as e:
        print()
        print(f"--- se corto la conexion: {e} ---")
        print("--- ¿se desenchufo el cable? ---")
        return 3


if __name__ == "__main__":
    sys.exit(main())
