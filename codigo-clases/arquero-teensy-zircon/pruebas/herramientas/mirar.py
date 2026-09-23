"""Monitor del robot en vivo: muestra lo que dice y deja escribirle teclas.

A diferencia de serie.py, que escucha unos segundos y se va, este se queda
abierto hasta que lo cierres. Es lo mas parecido al Monitor Serie del
Arduino IDE, pero sin abrir el IDE.

  python mirar.py                 (busca el Teensy solo)
  python mirar.py --puerto COM7   (si queres forzarlo)

Se cierra con Ctrl+C.

Las teclas que escribas se le mandan al robot tal cual. Cuales sirven
depende del programa que tenga cargado: casi todos contestan con ? la
lista de las que entienden.

Si el programa dibuja un tablero que se refresca en su lugar (como
`monitor-robot`), esto ya le pide a Windows que entienda los codigos de
pantalla. Si aun asi se ve basura con corchetes, ese programa tiene una
tecla para pasar a lista simple.
"""
import argparse
import os
import sys
import time

import serial
from serial.tools import list_ports

# El Teensy no siempre cae en el mismo COM: cambia si se enchufa en otro
# conector USB. En vez de adivinar, se lo busca por el identificador del
# fabricante (PJRC = 0x16C0), que ese no cambia nunca.
VID_PJRC = 0x16C0


def habilitar_ansi():
    """Windows NO interpreta los codigos ANSI hasta que un programa se lo pide.

    `monitor-robot` dibuja un tablero que se queda quieto y se refresca en su
    lugar; para eso manda codigos como ESC[H ("volve arriba") y ESC[K ("borra
    el resto de la linea"). Sin esta funcion, la consola los muestra como
    basura con corchetes en vez de obedecerlos.

    Devuelve True si quedo habilitado. Los programas que solo imprimen texto
    normal no se enteran de nada.
    """
    if os.name != "nt":
        return True
    try:
        import ctypes
        k = ctypes.windll.kernel32
        h = k.GetStdHandle(-11)                 # STD_OUTPUT_HANDLE
        modo = ctypes.c_uint32()
        if not k.GetConsoleMode(h, ctypes.byref(modo)):
            return False
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
        return bool(k.SetConsoleMode(
            h, modo.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    except Exception:
        return False


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

    habilitar_ansi()

    print("=" * 60)
    print(f" MONITOR DEL ARQUERO  —  {puerto} a {a.baud}")
    print("=" * 60)
    print(" Lo que escribas se le manda al robot tal cual.")
    print(" Cada programa tiene sus propias teclas: casi todos contestan")
    print(" con ? la lista de las que entienden.")
    print()
    print(" Para salir: Ctrl+C")
    if not habilitar_ansi():
        print()
        print(" (esta consola no entiende los codigos de pantalla: si el")
        print("  monitor se ve con basura, apreta la tecla l)")
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
