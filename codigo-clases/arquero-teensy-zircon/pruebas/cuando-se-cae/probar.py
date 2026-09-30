"""Corre la prueba "cuando se cae" varias veces y junta los resultados.

    python probar.py          (3 vueltas)
    python probar.py 5        (5 vueltas)

O directamente doble clic en PROBAR.bat.

QUE HACE
Larga varias vueltas seguidas de lo que el robot tenga cargado en
`cuando-se-cae`, va mostrando todo, y al final junta los resultados.

⚠️ ESTE PROGRAMA NO ELIGE QUE CORRE EL ROBOT: solo le manda la tecla `g` y
lee lo que contesta. Si el robot tiene cargado otro sketch, lo que se va a
ver es lo de ese otro sketch. Es el error mas facil de cometer aca.

HAY DOS MODOS, y lo decide el sketch (MOVER_LOS_MOTORES):

  VIGILANCIA (como esta hoy) — los motores NO se mueven. Mira el giroscopio y
  cuenta cuantas veces se cae. Es para menear el cable y ver el numero
  moverse. 🎯 Si no le meneas el cable, no probaste nada: quieto aguanta.

  RAMPA (con MOVER_LOS_MOTORES = true) — 🚨 LAS RUEDAS GIRAN, el robot tiene
  que estar sujeto. Sube la potencia de a poco y anota en que PWM se cae el
  giroscopio. Si el umbral se repite entre vueltas es CORRIENTE; si esta
  desparramado es VIBRACION.

Se corta con Ctrl+C.
"""
import re
import sys
import time

import serial
from serial.tools import list_ports

VID_PJRC = 0x16C0
BAUD = 19200

# El robot imprime esta linea al terminar cada vuelta, siempre igual.
RESUMEN = re.compile(
    r"RESUMEN pwm2=(-?\d+) pwm3=(-?\d+) tirones=(\d+) volvio2=(\d) volvio3=(\d)")

# En modo vigilancia (motores apagados) el robot manda ademas esta linea.
VIGILANCIA = re.compile(
    r"VIGILANCIA caidas=(\d+) vueltas=(\d+) sanas=(\d+) mudas=(\d+)")


def buscar_teensy():
    for p in list_ports.comports():
        if p.vid == VID_PJRC:
            return p.device
    return None


def abrir():
    """El puerto tarda un momento en estar usable despues de una carga."""
    for intento in range(1, 13):
        p = buscar_teensy()
        if p:
            try:
                s = serial.Serial(p, BAUD, timeout=0.05)
                time.sleep(0.4)
                s.in_waiting
                return s, p
            except Exception:
                pass
        time.sleep(1.0)
    return None, None


def una_vuelta(s, numero, total):
    """Larga una vuelta y devuelve el resumen, o None si no llego."""
    print()
    print("=" * 62)
    print(" VUELTA %d de %d" % (numero, total))
    print("=" * 62)
    s.reset_input_buffer()
    s.write(b"g")
    s.flush()

    linea = ""
    sin_nada = 0
    # 3 minutos de tope: la prueba dura ~50 s, pero si el giroscopio no
    # aparece el robot se queda buscandolo y hay que darle tiempo a que el
    # equipo menee el conector.
    fin = time.time() + 180
    while time.time() < fin:
        n = s.in_waiting
        if not n:
            time.sleep(0.02)
            sin_nada += 1
            continue
        sin_nada = 0
        for ch in s.read(n).decode("utf-8", "replace"):
            if ch == "\n":
                texto = linea.rstrip("\r")
                print("  " + texto)
                v = VIGILANCIA.search(texto)
                if v:
                    return {
                        "modo": "vigilancia",
                        "caidas": int(v.group(1)),
                        "vueltas": int(v.group(2)),
                        "sanas": int(v.group(3)),
                        "mudas": int(v.group(4)),
                    }
                m = RESUMEN.search(texto)
                if m:
                    return {
                        "modo": "rampa",
                        "pwm2": int(m.group(1)),
                        "pwm3": int(m.group(2)),
                        "tirones": int(m.group(3)),
                        "volvio2": m.group(4) == "1",
                        "volvio3": m.group(5) == "1",
                    }
                linea = ""
            else:
                linea += ch
    print("  !! se paso el tiempo sin terminar la vuelta")
    return None


def columna(vals):
    """Los umbrales que existieron de verdad (los -1 son 'no se cayo')."""
    return [v for v in vals if v is not None and v >= 0]


def veredicto_vigilancia(res):
    print()
    print("=" * 62)
    print(" VIGILANCIA — %d vuelta(s), los motores nunca se movieron" % len(res))
    print("=" * 62)
    print(" vuelta   caidas   vueltas   lecturas sanas   mudas")
    print(" ------   ------   -------   --------------   -----")
    for i, r in enumerate(res, 1):
        print("   %2d       %-6d   %-7d   %-14d   %d"
              % (i, r["caidas"], r["vueltas"], r["sanas"], r["mudas"]))
    caidas = sum(r["caidas"] for r in res)
    sanas = sum(r["sanas"] for r in res)
    mudas = sum(r["mudas"] for r in res)
    print()
    if caidas == 0 and mudas == 0:
        print(" 🎯 NO SE CAYO NI UNA VEZ, con el robot quieto.")
        print("    Si mientras corria le meneaste el cable y aguanto,")
        print("    el cable quedo bien. Si no lo tocaste, no probaste nada:")
        print("    hay que menearlo, porque quieto y sin tocar aguanta igual.")
    else:
        total = sanas + mudas
        pct = (100 * mudas // total) if total else 0
        print(" Se cayo %d vez/veces. Estuvo mudo el %d%% del tiempo." % (caidas, pct))
        print(" Si se caia justo cuando movias el cable, ES ESE CABLE.")
        print(" Si se cae solo, sin tocarlo, esta peor que falso contacto.")
    print("=" * 62)


def veredicto(res):
    if res and res[0].get("modo") == "vigilancia":
        return veredicto_vigilancia(res)
    print()
    print("=" * 62)
    print(" RESUMEN DE LAS %d VUELTAS" % len(res))
    print("=" * 62)
    print(" vuelta   pwm 3 ruedas   pwm 2 ruedas   caidas en tirones")
    print(" ------   ------------   ------------   -----------------")
    for i, r in enumerate(res, 1):
        def m(v):
            return "no se cayo" if v < 0 else str(v)
        print("   %2d         %-12s   %-12s   %d"
              % (i, m(r["pwm3"]), m(r["pwm2"]), r["tirones"]))

    tres = columna([r["pwm3"] for r in res])
    dos = columna([r["pwm2"] for r in res])
    volvio = sum(1 for r in res if r["volvio2"] or r["volvio3"])

    print()
    if not tres and not dos:
        print(" 🎯 NUNCA SE CAYO, en ninguna vuelta.")
        print("    O el problema se arreglo, o hoy el contacto quedo bien")
        print("    apoyado. Vale la pena repetirlo despues de mover el robot.")
        print("=" * 62)
        return

    if tres:
        print(" con 3 ruedas se cayo en %d de %d vueltas, a pwm %s"
              % (len(tres), len(res), tres))
        print("    -> menor %d, mayor %d, diferencia %d"
              % (min(tres), max(tres), max(tres) - min(tres)))
    if dos:
        print(" con 2 ruedas se cayo en %d de %d vueltas, a pwm %s"
              % (len(dos), len(res), dos))
        print("    -> menor %d, mayor %d, diferencia %d"
              % (min(dos), max(dos), max(dos) - min(dos)))
    print(" volvio solo al parar los motores en %d de %d vueltas"
          % (volvio, len(res)))

    print()
    print(" QUE DICE ESTO:")
    # El criterio: si los umbrales caen dentro de una ventana angosta, hay
    # umbral. Si estan desparramados por todo el rango de la rampa (60 a 220),
    # no lo hay. 40 de PWM es un cuarto del recorrido: mas que eso ya no se
    # puede llamar "el mismo punto".
    for nombre, vals in (("3 ruedas", tres), ("2 ruedas", dos)):
        if len(vals) < 2:
            continue
        rango = max(vals) - min(vals)
        if rango <= 40:
            print("  - con %s el umbral SE REPITE (varia %d de pwm)."
                  % (nombre, rango))
            print("    Eso es CORRIENTE: hay un consumo a partir del cual la")
            print("    tension no alcanza y el chip se cae.")
        else:
            print("  - con %s el umbral NO se repite (varia %d de pwm)."
                  % (nombre, rango))
            print("    Eso apunta a VIBRACION, o sea al conector.")

    if tres and dos and min(tres) < min(dos):
        print("  - con 3 ruedas se cae ANTES que con 2, y 3 ruedas consumen")
        print("    mas. Otra vez apunta a la corriente.")
    elif tres and dos and min(dos) < min(tres):
        print("  - con 2 ruedas se cae antes que con 3, y eso es raro si fuera")
        print("    corriente (2 ruedas consumen menos). Mirar el conector.")

    if volvio:
        print("  - volvio solo al parar los motores: el chip no se")
        print("    desconecto, se reinicio. Eso es tension.")
    print("=" * 62)


def main():
    vueltas = 3
    if len(sys.argv) > 1:
        try:
            vueltas = int(sys.argv[1])
        except ValueError:
            pass

    s, puerto = abrir()
    if not s:
        print("NO ENCONTRE EL TEENSY. ¿Esta el cable USB puesto?")
        return 2

    print("=" * 62)
    print(" CUANDO SE CAE — %d vueltas    (puerto %s)" % (vueltas, puerto))
    print("=" * 62)
    print(" 🚨 LAS RUEDAS GIRAN. El robot tiene que estar SUJETO.")
    print(" 🚨 La bateria tiene que estar prendida.")
    print()
    print(" Si el giroscopio no aparece, el robot se queda buscandolo:")
    print(" MENEA EL CONECTOR y va a arrancar solo.")
    print()
    print(" Ctrl+C para cortar.")

    res = []
    try:
        with s:
            for i in range(1, vueltas + 1):
                r = una_vuelta(s, i, vueltas)
                if r:
                    res.append(r)
                else:
                    print("  (esta vuelta no dio resultado, sigo)")
                time.sleep(1.0)
    except KeyboardInterrupt:
        print()
        print("--- cortado a mano ---")
    except serial.SerialException as e:
        print()
        print("--- se corto la conexion: %s ---" % e)
        print("--- ¿se desenchufo el cable? ---")

    if res:
        veredicto(res)
    else:
        print()
        print("No se completo ninguna vuelta.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
