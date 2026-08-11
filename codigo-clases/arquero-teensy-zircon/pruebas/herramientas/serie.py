"""Hablarle al robot por el puerto serie, sin monitor grafico.

  python serie.py --escuchar 3
  python serie.py --enviar 3 --escuchar 4
  python serie.py --enviar 0            (parada de emergencia)
"""
import argparse
import sys
import time

import serial

p = argparse.ArgumentParser()
p.add_argument("--puerto", default="COM5")
p.add_argument("--baud", type=int, default=19200)
p.add_argument("--enviar", default=None, help="texto a mandar (se le agrega ENTER)")
p.add_argument("--escuchar", type=float, default=2.0, help="segundos de escucha")
p.add_argument("--espera-previa", type=float, default=0.4,
               help="segundos entre abrir el puerto y mandar")
a = p.parse_args()

try:
    s = serial.Serial(a.puerto, a.baud, timeout=0.1)
except serial.SerialException as e:
    print(f"NO SE PUDO ABRIR {a.puerto}: {e}", file=sys.stderr)
    sys.exit(2)

with s:
    time.sleep(a.espera_previa)
    if a.enviar is not None:
        s.reset_input_buffer()
        s.write((a.enviar + "\n").encode())
        s.flush()
        print(f"--- enviado: {a.enviar!r} ---")

    fin = time.time() + a.escuchar
    vacio = True
    while time.time() < fin:
        n = s.in_waiting
        if n:
            vacio = False
            sys.stdout.write(s.read(n).decode("utf-8", "replace"))
            sys.stdout.flush()
        else:
            time.sleep(0.02)

print()
if vacio:
    print("--- (el robot no dijo nada en esos segundos) ---")
