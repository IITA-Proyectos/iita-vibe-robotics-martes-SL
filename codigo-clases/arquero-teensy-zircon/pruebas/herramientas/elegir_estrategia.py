"""ELEGIR LA ESTRATEGIA del arquero (1 o 2) y cargarla al robot.  [2026-11-13]

  python elegir_estrategia.py               pregunta 1 o 2 y carga esa
  python elegir_estrategia.py --sin-cargar  lo mismo, pero NO compila ni carga:
                                            dice que haria (para probar sin robot)

Lo abre ELEGIR-ESTRATEGIA.bat (carpeta arquero-teensy-zircon), con doble clic.

  1 = ESTRATEGIA 1: el programa de juego               -> funciona\\partido-0921
  2 = ESTRATEGIA 2: el arquero sobre la linea del area -> funciona\\estrategia-2

Carga lo que HAY en esa carpeta. Si con el TABLERO cambiaron valores de la
estrategia 1, carga eso (se nota porque el SELLO deja de decir "fabrica").

Los pasos, y por que:
  1. Elegir 1 o 2. Enter solo o 0 = salir sin cargar nada.
  2. Mirar que robot hay enchufado con "tablero.py --quien" (no abre ningun
     puerto). Solo se sigue si hay UN Teensy y es el ARQUERO (13348550), y
     ninguno en MODO CARGA: un Teensy en modo carga no tiene puerto COM y el
     cargador le carga PRIMERO a ese, aunque sea el DELANTERO (revision del
     13/11; el tablero hace lo mismo desde el 03/11).
  3. Preguntar otra vez: solo carga si escriben S.
  4. Compilar SIN tocar el robot. Si no compila, el robot sigue como estaba.
  5. Mirar el robot de nuevo (pudieron cambiarlo mientras compilaba) y
     recien ahi cargar con cargar.py, que tambien revisa que sea el arquero.

Si el argumento no es --sin-cargar (por ejemplo --sin-carga), no hace nada:
asi un error de tipeo no termina cargando de verdad.

🚨 CARGAR REINICIA EL TEENSY Y LAS DOS ESTRATEGIAS ARRANCAN SOLAS. Cargar con
la bateria APAGADA.
"""
import os
import re
import subprocess
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
ARQUERO = os.path.normpath(os.path.join(AQUI, "..", ".."))
CARGAR = os.path.join(AQUI, "cargar.py")
TABLERO_PY = os.path.join(ARQUERO, "pruebas", "tablero", "tablero.py")

ESTRATEGIAS = {
    "1": {
        "titulo": "ESTRATEGIA 1: el programa de juego",
        "carpeta": os.path.join(ARQUERO, "funciona", "partido-0921"),
        "archivo": "partido-0921.ino",
        "como": "Sigue la pelota de costado delante del arco y la despeja.\n"
                "      Si no la ve en 4 s, gira de a poco buscandola (la barrida).",
    },
    "2": {
        "titulo": "ESTRATEGIA 2: el arquero sobre la linea del area",
        "carpeta": os.path.join(ARQUERO, "funciona", "estrategia-2"),
        "archivo": "estrategia-2.ino",
        "como": "Retrocede hasta la linea del area y sigue la pelota de\n"
                "      costado por esa linea. Despeja y vuelve a la linea.",
    },
}

# Lo que contesta "tablero.py --quien" (sus codigos de salida, CODIGOS_QUIEN)
QUE_ROBOT = {
    3: "Ese es el DELANTERO. No le cargo nada: tiene los pines cruzados.",
    4: "Hay un robot que NO es el arquero, o no pude leer su numero.",
    5: "No veo ningun robot enchufado por USB. Si esta enchufado y no\n"
       "   aparece, desenchufalo y volvelo a enchufar.",
    6: "Hay mas de un Teensy: el cargador no elige a cual le carga.",
    8: "Hay un Teensy en MODO CARGA (sin programa corriendo): no se que\n"
       "   robot es, y el cargador le cargaria a ESE (puede ser el DELANTERO).\n"
       "   Desenchufalo y cerra el Teensy Loader si quedo abierto.\n"
       "   Si el UNICO enchufado es el ARQUERO y sigue asi, quedo sin programa:\n"
       "   cargalo con el Arduino IDE (placa Teensy 4.1) y el boton blanco,\n"
       "   o avisale al profe.",
}

_RE_VERSION = re.compile(r'const\s+char\s*\*\s*VERSION_PARTIDO\s*=\s*"([^"]*)"')
_RE_SELLO = re.compile(r'const\s+char\s*\*\s*SELLO\s*=\s*"([^"]*)"')
_RE_ESTRATEGIA_2 = re.compile(r'ESTRATEGIA 2 version (\S+) \(([^:)]*)')


def que_version(clave, texto):
    """Lo que dice el archivo de su version, en una linea."""
    if clave == "1":
        v = _RE_VERSION.search(texto)
        s = _RE_SELLO.search(texto)
        version = v.group(1) if v else "?"
        if not s:
            sello = "sin sello"
        elif s.group(1) == "fabrica":
            sello = "con los valores de fabrica"
        else:
            sello = 'con valores cambiados en el TABLERO (sello "%s")' % s.group(1)
        return "version %s, %s" % (version, sello)
    m = _RE_ESTRATEGIA_2.search(texto)
    if m:
        return "version %s %s" % (m.group(1), m.group(2).strip())
    return "version ?"


def leer(clave):
    """El texto de la version, o None si no esta el programa."""
    e = ESTRATEGIAS[clave]
    ino = os.path.join(e["carpeta"], e["archivo"])
    ini = os.path.join(e["carpeta"], "platformio.ini")
    if not (os.path.exists(ino) and os.path.exists(ini)):
        return None
    with open(ino, encoding="utf-8", errors="replace") as f:
        return que_version(clave, f.read())


def preguntar(texto):
    """input() que, si se cierra la ventana, no hay teclado o apretan
    Ctrl+C, devuelve None (= salir sin cargar).
    (El replace saca la marca BOM que mete PowerShell al pasarle texto.)"""
    try:
        return input(texto).replace("﻿", "").strip()
    except (EOFError, KeyboardInterrupt):
        print()
        return None


def correr(orden):
    """Corre una orden sin darle el teclado (asi una S de mas no contesta
    nada por nosotros). -> su codigo de salida; Ctrl+C o error = 1."""
    sys.stdout.flush()     # que lo nuestro salga ANTES que lo de la orden
    try:
        return subprocess.call(orden, stdin=subprocess.DEVNULL)
    except KeyboardInterrupt:
        print()
        print("(cortado con Ctrl+C)")
        return 1
    except OSError as e:
        print("No pude correr:", orden[0], "-", e)
        return 1


def que_robot_hay():
    """tablero.py --quien: 0 = hay UN Teensy y es el arquero."""
    print()
    print("  Que robot hay enchufado:")
    return correr([sys.executable, TABLERO_PY, "--quien"])


def main():
    try:   # que los acentos y el 🚨 no rompan la ventana
        sys.stdout.reconfigure(errors="replace")
    except (AttributeError, ValueError):
        pass
    otros = [a for a in sys.argv[1:] if a != "--sin-cargar"]
    if otros:
        print('No entiendo "%s". No hago nada.' % " ".join(otros))
        print("Para probar la ventana sin cargar: ELEGIR-ESTRATEGIA.bat --sin-cargar")
        return 2
    sin_cargar = "--sin-cargar" in sys.argv

    print("=" * 66)
    print("  ELEGIR LA ESTRATEGIA DEL ARQUERO")
    if sin_cargar:
        print("  (modo --sin-cargar: NO se compila ni se carga nada)")
    print("=" * 66)
    for clave in ("1", "2"):
        e = ESTRATEGIAS[clave]
        version = leer(clave)
        print()
        print("  %s = %s" % (clave, e["titulo"]))
        print("      Carpeta: funciona\\%s" % os.path.basename(e["carpeta"]))
        if version is None:
            print("      XXX NO ENCUENTRO ESTE PROGRAMA: no se puede elegir.")
        else:
            print("      Tiene la %s." % version)
            print("      " + e["como"])
    print()
    print("  0 o Enter solo = salir sin cargar nada")
    print()
    print("  OJO con el TABLERO: le carga OTRO programa al robot, y su boton")
    print("  CARGAR EL PARTIDO carga la estrategia 1. Si usaron el TABLERO y")
    print("  quieren jugar con la 2, vuelvan aca y elijan 2.")
    print("=" * 66)

    # ---- 1. cual ----
    while True:
        elegida = preguntar("Escribi 1 o 2 y apreta Enter: ")
        if elegida in (None, "", "0"):
            print("No cargo nada. El robot sigue con lo que tenia.")
            return 0
        if elegida not in ESTRATEGIAS:
            print('"%s" no es 1 ni 2. Proba de nuevo (0 o Enter solo = salir).' % elegida)
            continue
        if leer(elegida) is None:
            print("Esa no esta en la compu: no la puedo cargar. Elegi otra o 0 para salir.")
            continue
        break
    e = ESTRATEGIAS[elegida]

    # ---- 2. que sea el arquero ----
    while True:
        codigo = que_robot_hay()
        if codigo == 0:
            break
        print()
        print("   XXX " + QUE_ROBOT.get(codigo, "No pude confirmar que sea el arquero."))
        if sin_cargar:
            print("   (--sin-cargar: sigo igual, para probar la ventana)")
            break
        print()
        print("   Deja enchufado SOLO el ARQUERO, espera unos segundos y")
        otra = preguntar("   apreta Enter para mirar de nuevo (0 = salir sin cargar): ")
        if otra is None or otra == "0":
            print("No cargo nada.")
            return 2

    # ---- 3. preguntar otra vez (con la version de AHORA) ----
    version = leer(elegida)
    if version is None:
        print("El programa de la %s ya no esta en la compu. No cargo nada." % e["titulo"])
        return 2
    print()
    print("-" * 66)
    print("  Vas a cargar la %s" % e["titulo"])
    print("  (%s)" % version)
    print()
    print("  🚨 Cargar REINICIA el robot y ARRANCA SOLO: bateria APAGADA.")
    print("  Si la ventana del TABLERO esta abierta, cerrala antes.")
    print("-" * 66)
    ok = preguntar("Para cargarla escribi S y Enter (otra cosa = no cargo nada): ")
    if (ok or "").lower() not in ("s", "si", "sí"):
        print("No cargo nada. El robot sigue con lo que tenia.")
        return 0

    try:
        from cargar import buscar_pio      # (cargar.py esta en esta misma carpeta)
    except ImportError as err:
        print("No puedo usar cargar.py (%s). Abrilo con ELEGIR-ESTRATEGIA.bat," % err)
        print("que usa el Python de PlatformIO. No cargo nada.")
        return 1
    compilar = buscar_pio() + ["run", "-e", "teensy41", "-d", e["carpeta"]]
    cargar = [sys.executable, CARGAR, e["carpeta"]]
    if sin_cargar:
        print()
        print("(--sin-cargar) Aca compilaria con:")
        print("   " + " ".join(compilar))
        print("(--sin-cargar) miraria el robot otra vez, y cargaria con:")
        print("   " + " ".join(cargar))
        return 0

    # ---- 4. compilar sin tocar el robot ----
    print()
    print("Compilando la %s (no toca el robot; la primera vez tarda ~1 min)..." % e["titulo"])
    if correr(compilar) != 0:
        print()
        print("=" * 66)
        print("  NO COMPILA (o se corto). NO toque el robot: sigue con lo que tenia.")
        print("  Lee el error de arriba. Si cambiaron valores con el TABLERO,")
        print("  abrilo y fijate que no haya nada en rojo.")
        print("=" * 66)
        return 1

    # ---- 5. mirar de nuevo y cargar ----
    codigo = que_robot_hay()
    if codigo != 0:
        print()
        print("   XXX " + QUE_ROBOT.get(codigo, "No pude confirmar que sea el arquero."))
        print("   NO CARGUE NADA. Deja enchufado SOLO el arquero y volve a abrir")
        print("   ELEGIR-ESTRATEGIA.")
        return 2
    print()
    r = correr(cargar)
    print()
    print("=" * 66)
    if r == 0:
        print("  LISTO: el robot quedo con la %s." % e["titulo"])
        print("  Para jugar: si la bateria quedo prendida, apagala. Desenchufa el")
        print("  USB, apoyalo quieto en la cancha y recien ahi prende la bateria")
        print("  (asi arranca de cero).")
    else:
        print("  NO SE PUDO CARGAR. Lee el mensaje de arriba.")
        print("  Si fallo antes de grabar (no hay Teensy, otra ventana usa el")
        print("  robot), el robot sigue con lo que tenia: arreglalo y volve a")
        print("  abrir ELEGIR-ESTRATEGIA.")
        print("  Si se corto A LA MITAD de la grabacion, el robot puede quedar SIN")
        print("  programa y en MODO CARGA (y ya no se ve): desenchufa y enchufa el")
        print("  USB y volve a probar; si dice MODO CARGA o NO HAY NINGUN TEENSY,")
        print("  cargalo con el Arduino IDE (placa Teensy 4.1), con SOLO el arquero")
        print("  enchufado, y el boton blanco del Teensy. O avisale al profe.")
    print("=" * 66)
    return r


if __name__ == "__main__":
    sys.exit(main())
