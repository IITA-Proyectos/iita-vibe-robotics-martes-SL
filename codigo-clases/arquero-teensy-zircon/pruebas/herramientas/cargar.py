"""Compilar y cargar un programa al Teensy del ARQUERO, sin PowerShell.

  python cargar.py <carpeta>            compila, carga y muestra el cartel
  python cargar.py <carpeta> --mirar    lo mismo y despues abre el monitor

<carpeta> es la carpeta del programa (la que tiene el platformio.ini), por
ejemplo  ..\\..\\funciona\\seguir-y-despejar  o  ..\\ajustar-movimientos

Lo usan los .bat: CARGAR-PARTIDO.bat y AJUSTAR.bat. Anda sin internet: usa
el PlatformIO que ya esta instalado en la compu.

🚨 CARGAR REINICIA EL TEENSY. Si el programa arranca solo (el de partido),
el robot sale a moverse apenas termina la carga. Cargar con la bateria
apagada, o con el robot en el piso de la cancha.
"""
import os
import subprocess
import sys
import time

import serial
from serial.tools import list_ports

VID_PJRC = 0x16C0
SERIE_ARQUERO = "13348550"
SERIE_DELANTERO = "15708680"   # el otro robot: a este NO se le carga nada


def buscar_pio():
    pio = os.path.join(os.path.expanduser("~"), ".platformio", "penv",
                       "Scripts", "pio.exe")
    if os.path.exists(pio):
        return pio
    return "pio"   # si esta en el PATH


def teensys_enchufados():
    return [p for p in list_ports.comports() if p.vid == VID_PJRC]


def leer_cartel(segundos=2.5):
    """Espera a que el Teensy vuelva despues de la carga y lee lo que dice."""
    fin = time.time() + 8
    while time.time() < fin:
        puertos = teensys_enchufados()
        if not puertos:
            time.sleep(0.05)
            continue
        try:
            with serial.Serial(puertos[0].device, 19200, timeout=0.1) as s:
                texto = ""
                t2 = time.time() + segundos
                while time.time() < t2:
                    texto += s.read(512).decode("utf-8", "replace")
                return texto
        except serial.SerialException:
            time.sleep(0.1)
    return ""


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    mirar = "--mirar" in sys.argv
    if not args:
        print(__doc__)
        return 2
    carpeta = os.path.abspath(args[0])
    if not os.path.exists(os.path.join(carpeta, "platformio.ini")):
        print("NO ENCUENTRO EL PROGRAMA en:")
        print("   ", carpeta)
        return 2
    nombre = os.path.basename(carpeta)

    # ---- que haya UN Teensy, y que sea el del arquero ----
    puertos = teensys_enchufados()
    if not puertos:
        print("NO HAY NINGUN TEENSY ENCHUFADO. Fijate el cable USB.")
        return 2
    if len(puertos) > 1:
        print("HAY", len(puertos), "TEENSY ENCHUFADOS. Deja uno solo:")
        print("el cargador no elige, y podes cargarle esto al otro robot.")
        return 2
    serie = puertos[0].serial_number or "?"
    if serie == SERIE_DELANTERO:
        print("ESE ES EL TEENSY DEL DELANTERO (serie " + serie + "). No cargo nada.")
        return 2
    if serie != SERIE_ARQUERO:
        print("El Teensy enchufado tiene serie " + serie + ", y el del arquero es "
              + SERIE_ARQUERO + ".")
        if input("¿Es el arquero igual? (s/n): ").strip().lower() != "s":
            return 2

    # ---- que ninguna otra ventana tenga tomado el robot ----
    # Si AJUSTAR, MIRAR, MONITOR o el Monitor Serie del IDE estan abiertos,
    # el cargador no puede reiniciar el Teensy y la carga se queda esperando
    # que apreten el boton. Mejor decirlo claro.
    try:
        serial.Serial(puertos[0].device, 19200).close()
    except serial.SerialException:
        print("OTRA VENTANA ESTA USANDO EL ROBOT (AJUSTAR, MIRAR, MONITOR o el")
        print("Monitor Serie del IDE). Cerrala (Ctrl+C) y volve a probar.")
        return 2

    # ---- el Teensy Loader en modo Auto le puede grabar OTRO programa ----
    # (lo hace igual el cargador del delantero). Si no estaba abierto, no pasa nada.
    subprocess.run(["taskkill", "/IM", "teensy.exe", "/F"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    print("=" * 60)
    print(" Cargando", nombre, "al arquero (" + puertos[0].device + ")")
    print(" 🚨 Si es el programa de partido, el robot va a arrancar solo.")
    print("=" * 60)

    r = subprocess.run([buscar_pio(), "run", "-e", "teensy41", "-t", "upload",
                        "-d", carpeta])
    if r.returncode != 0:
        print()
        print("FALLO. Si es un error de compilacion, no se cargo nada.")
        print("Si dice 'Waiting for Teensy device': desenchufa y enchufa el USB y")
        print("volve a probar (y fijate que no haya otra ventana de monitor abierta).")
        return 1

    print()
    if mirar:
        # Directo al monitor, sin leer nada antes: si no, lo que el programa
        # dice al arrancar (la ayuda) se lo come esta lectura y no se ve.
        # Hay que esperar a que el Teensy vuelva a aparecer: mirar.py lo
        # busca una sola vez y, si todavia no esta, se cierra.
        fin = time.time() + 8
        while not teensys_enchufados() and time.time() < fin:
            time.sleep(0.1)
        aqui = os.path.dirname(os.path.abspath(__file__))
        return subprocess.call([sys.executable, os.path.join(aqui, "mirar.py")])

    print("--- lo que dijo el robot al arrancar ---")
    texto = leer_cartel()
    lineas = [l for l in texto.replace("\x1b[2J", "").replace("\x1b[H", "\n")
              .replace("\x1b[K", "").splitlines() if l.strip()]
    print("\n".join(lineas[:6]) if lineas else "(nada: el programa de partido solo lo dice una vez y a veces no llego)")
    print("----------------------------------------")
    return 0


if __name__ == "__main__":
    sys.exit(main())
