# =====================================================================
#  ROBOT SIN POWERSHELL - cargar y leer el DELANTERO en una PC donde
#  Windows PowerShell no arranca
#  IITA - Roboliga 2026 - 2026-10-12
# =====================================================================
#
#  POR QUE EXISTE. El dia del partido del 12/10 se trabajo en otra PC, y
#  en esa PC Windows PowerShell no arranca: le falta un componente de
#  Windows (Microsoft.Management.Infrastructure) y cualquier script
#  termina en "Se han producido uno o varios errores". CARGAR-ROBOT.bat,
#  MEDIR-TUTI-ROBOT.bat y CALIBRAR-ROBOT.bat son los tres de PowerShell,
#  asi que ahi no abre ninguno.
#
#  Esto hace LO MISMO que cargar-robot.ps1 y medir-robot.ps1, pero con el
#  Python que trae PlatformIO adentro (no hay que instalar nada).
#
#  NO REEMPLAZA a los .bat de siempre: en una PC sana se siguen usando
#  esos, que son los que ganaron partidos. Esto es el repuesto.
#
#  COMO SE USA: doble clic en CARGAR-ROBOT-SIN-POWERSHELL.bat (partido)
#  o en MEDIR-ROBOT-SIN-POWERSHELL.bat (pruebas y leer el robot).
#
#  A mano:
#     python robot-sin-powershell.py cargar azul
#     python robot-sin-powershell.py cargar amarillo
#     python robot-sin-powershell.py medir grabar-linea
#     python robot-sin-powershell.py leer            (manda una tecla y lee 25 s)
#     python robot-sin-powershell.py leer 15 sin-tecla
#     python robot-sin-powershell.py ver             (que robot hay enchufado)
# =====================================================================

import os
import re
import subprocess
import sys
import time
from datetime import datetime

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("  XXX  Este Python no tiene pyserial. Hay que correrlo con el Python de")
    print("       PlatformIO: %USERPROFILE%\\.platformio\\penv\\Scripts\\python.exe")
    sys.exit(1)

SERIE_DELANTERO = "15708680"
VID_TEENSY = 0x16C0
PID_SERIE = 0x0483          # la Teensy corriendo un programa
BAUDIOS = 19200

RAIZ = os.path.dirname(os.path.abspath(__file__))
HOME = os.environ.get("USERPROFILE", os.path.expanduser("~"))
PIO = os.path.join(HOME, ".platformio", "penv", "Scripts", "pio.exe")
LOADER = os.path.join(HOME, ".platformio", "packages", "tool-teensy", "teensy.exe")
FIRMWARE = os.path.join(RAIZ, "funciona", "delantero")
PRUEBAS = os.path.join(RAIZ, "pruebas")
MEDICIONES = os.path.join(RAIZ, "mediciones")

# Las pruebas que graban solas con la bateria y se leen despues.
SIN_CABLE = ("grabar-linea", "calibracion-quieto", "calibracion-moviendo")

# La consola de Windows no siempre puede con lo que manda el robot.
try:
    sys.stdout.reconfigure(errors="replace")
except Exception:
    pass


def linea():
    print("=" * 64)


def error_y_salir(msg):
    print()
    print("  XXX  " + msg)
    print()
    sys.exit(1)


def preguntar(texto):
    try:
        return input(texto).strip()
    except EOFError:
        return ""


# ---------------------------------------------------------------------
# Que Teensy hay enchufadas
# ---------------------------------------------------------------------

def serie_decimal(s):
    """En modo carga (PID 0478) la Teensy informa el numero de serie en
    HEXADECIMAL y dividido por 10: 0017F834 -> 1570868 -> x10 = 15708680.
    Misma regla que en cargar-robot.ps1."""
    try:
        n = int(s, 16)
    except ValueError:
        return None
    if n < 10000000:
        n *= 10
    return str(n)


def buscar_teensys():
    """Devuelve [(pid, serie)] de las Teensy enchufadas, en cualquier modo.
    Usa lo mismo que mira cargar-robot.ps1 (Win32_PnPEntity), pero preguntado
    con wmic. Si wmic no esta, se arregla con los puertos serie: asi no ve una
    Teensy en modo carga, pero si la que esta corriendo un programa."""
    encontradas = []
    patron = re.compile(r"USB\\VID_16C0&PID_([0-9A-F]{4})\\([^\\\s]+)\s*$", re.I)
    try:
        r = subprocess.run(
            ["wmic", "path", "Win32_PnPEntity", "where",
             "PNPDeviceID like '%VID_16C0%'", "get", "PNPDeviceID"],
            capture_output=True, timeout=30)
        crudo = r.stdout
        if crudo.startswith(b"\xff\xfe"):
            texto = crudo.decode("utf-16", errors="replace")
        else:
            texto = crudo.decode("ascii", errors="replace")
        for renglon in texto.splitlines():
            m = patron.search(renglon.strip())
            if m:
                encontradas.append((m.group(1).upper(), m.group(2)))
        return encontradas
    except (OSError, subprocess.SubprocessError):
        pass
    for p in list_ports.comports():
        if p.vid == VID_TEENSY:
            encontradas.append(("%04X" % (p.pid or 0), p.serial_number or "?"))
    return encontradas


def buscar_puerto():
    for p in list_ports.comports():
        if p.vid == VID_TEENSY and p.pid == PID_SERIE:
            return p.device
    return None


def comprobar_delantero(preguntar_si_no_se=True):
    """Corta si no hay UN solo robot o si no es el delantero."""
    teensys = buscar_teensys()
    if len(teensys) == 0:
        error_y_salir("No hay ningun robot enchufado por USB.")
    if len(teensys) > 1:
        error_y_salir("Hay %d robots enchufados. Deja SOLO el delantero: el cargador "
                      "no elige y podria cargar el arquero." % len(teensys))
    pid, serie = teensys[0]
    if pid == "0478":
        dec = serie_decimal(serie)
        if dec == SERIE_DELANTERO:
            print("      OK: es el DELANTERO (serie %s), en modo carga" % SERIE_DELANTERO)
        elif dec:
            error_y_salir("El robot en modo carga tiene serie %s y el delantero es %s. "
                          "Es el ARQUERO? No cargo nada." % (dec, SERIE_DELANTERO))
        else:
            if not preguntar_si_no_se:
                error_y_salir("No pude leer la serie del robot en modo carga.")
            ok = preguntar("      No pude leer la serie. Es el DELANTERO y esta solo? (s/n) ")
            if ok.lower() != "s":
                error_y_salir("Cancelado.")
    elif serie != SERIE_DELANTERO:
        error_y_salir("El robot enchufado tiene serie %s y el delantero es %s. "
                      "Es el ARQUERO? No cargo nada." % (serie, SERIE_DELANTERO))
    else:
        print("      OK: es el DELANTERO (serie %s)" % SERIE_DELANTERO)


# ---------------------------------------------------------------------
# Teensy Loader y carga
# ---------------------------------------------------------------------

def loader_abierto():
    try:
        r = subprocess.run(["tasklist", "/FI", "IMAGENAME eq teensy.exe", "/NH"],
                           capture_output=True, timeout=20)
        return b"teensy.exe" in r.stdout.lower()
    except (OSError, subprocess.SubprocessError):
        return False


def abrir_loader():
    if not loader_abierto():
        subprocess.Popen([LOADER], cwd=os.path.dirname(LOADER))
        time.sleep(2)
    print("      OK: abierto")


def cerrar_loader():
    # Despues de cargar, el Loader queda en modo Auto con el programa del
    # DELANTERO adentro: si en esta PC despues se pone el ARQUERO en modo
    # carga, se lo graba. Por eso se cierra al terminar. [revision 21/09]
    try:
        subprocess.run(["taskkill", "/IM", "teensy.exe", "/F"],
                       capture_output=True, timeout=20)
    except (OSError, subprocess.SubprocessError):
        pass


def compilar_y_cargar(proyecto, entorno=None):
    """Corre pio run -t upload. Devuelve True si el cargador dijo que si.
    OJO: eso NO prueba que el programa llego al robot. Lo prueba el banner."""
    orden = [PIO, "run", "-t", "upload"]
    if entorno:
        orden[2:2] = ["-e", entorno]
    try:
        r = subprocess.run(orden, cwd=proyecto, capture_output=True, timeout=240)
    except subprocess.TimeoutExpired:
        error_y_salir("La carga se colgo (4 min). Apreta el boton blanco de la Teensy "
                      "y proba de nuevo.")
    salida = (r.stdout + r.stderr).decode("utf-8", errors="replace")
    # El cargador puede decir SUCCESS sin haber encontrado ninguna placa. Ya
    # paso dos veces (29/09 y 06/10), textual. Aca se lo agarra.
    if "No Teensy boards were found" in salida:
        error_y_salir("El cargador NO ENCONTRO ninguna Teensy (aunque diga SUCCESS). "
                      "Revisa el cable y proba de nuevo.")
    if r.returncode != 0:
        print()
        print("---- lo ultimo que dijo el cargador ----")
        for renglon in salida.splitlines()[-20:]:
            print(renglon)
        error_y_salir("La carga FALLO (codigo %d). El robot sigue con el programa "
                      "que tenia." % r.returncode)
    return True


def abrir_serie(puerto):
    """Abre con reintentos: Windows a veces muestra el COM un instante antes
    de dejarlo abrir, y eso no es 'un monitor serie abierto'."""
    ultimo = ""
    for _ in range(6):
        try:
            sp = serial.Serial()
            sp.port = puerto
            sp.baudrate = BAUDIOS
            sp.timeout = 0.1
            sp.dtr = True
            sp.open()
            return sp
        except (serial.SerialException, OSError) as e:
            ultimo = str(e)
            time.sleep(0.6)
    error_y_salir("No pude abrir %s (%s). Si hay un monitor serie abierto (Arduino IDE, "
                  "PlatformIO), cerralo y proba de nuevo." % (puerto, ultimo))


def esperar_puerto(segundos):
    fin = time.time() + segundos
    while time.time() < fin:
        puerto = buscar_puerto()
        if puerto:
            return puerto
        time.sleep(0.2)
    return None


# ---------------------------------------------------------------------
# CARGAR: el programa de partido, con el arco fijo
# ---------------------------------------------------------------------

def cargar(arco):
    arco = arco.strip().lower()
    if arco == "1":
        arco = "azul"
    if arco == "2":
        arco = "amarillo"

    linea()
    print("   DELANTERO  -  cargar programa de partido   (sin PowerShell)")
    linea()
    print("   Antes: BATERIA DEL ROBOT APAGADA (si no, sale andando")
    print("   apenas termina de cargar), USB enchufado, y SOLO este robot.")

    while arco not in ("azul", "amarillo"):
        print()
        print("   1)  atacar el ARCO AZUL")
        print("   2)  atacar el ARCO AMARILLO")
        print()
        op = preguntar("   Elegi 1 o 2 y apreta Enter: ")
        if op == "1":
            arco = "azul"
        elif op == "2":
            arco = "amarillo"
        else:
            print("   '%s' no es 1 ni 2. De nuevo." % op)
    nombre = arco.upper()
    print()
    print("   Voy a cargar:   ARCO %s" % nombre)
    print()

    comprobar_herramientas(FIRMWARE)

    print("[1/4] Buscando el robot...")
    comprobar_delantero()

    print("[2/4] Teensy Loader...")
    abrir_loader()

    print("[3/4] Compilando y cargando ARCO %s (la primera vez tarda ~1 min, "
          "despues ~10 s)..." % nombre)
    try:
        compilar_y_cargar(FIRMWARE, arco)
        print("      OK: el cargador dice SUCCESS (falta que lo confirme el robot)")

        print("[4/4] Leyendo lo que dice el robot...")
        puerto = esperar_puerto(15)
        if not puerto:
            error_y_salir("El robot no volvio a aparecer por USB despues de cargar. "
                          "Desenchufa y enchufa, y mira el monitor serie.")

        # Se abre el puerto YA: el robot espera hasta 3 s a que alguien lo
        # escuche antes de imprimir el banner. La segunda barrera (que el que
        # volvio sea el delantero) va despues, para no perder el banner.
        sp = abrir_serie(puerto)
        texto = ""
        fin = time.time() + 12
        while time.time() < fin:
            try:
                texto += sp.read(4096).decode("utf-8", errors="replace")
            except (serial.SerialException, OSError):
                break               # si se corto el USB, uso lo que llego
            if re.search(r"ARCO: \w+[^\r\n]*[\r\n]", texto) and "Arranca en" in texto:
                break
        try:
            sp.close()
        except Exception:
            pass

        despues = buscar_teensys()
        if len(despues) != 1 or despues[0][1] != SERIE_DELANTERO:
            error_y_salir("Despues de cargar, el robot enchufado NO es el delantero (%s). "
                          "Revisar que robot quedo cargado." % SERIE_DELANTERO)
    finally:
        cerrar_loader()

    renglones = texto.splitlines()
    print()
    print("---- el robot dice ----")
    for r in renglones:
        if re.search(r"^ARCO: |^patada: |RETROCEDE YA|^Arranca en|ATACO EL ARCO|^Giroscopo|^Linea: ", r):
            print("   " + r)
    print("-----------------------")
    print()

    linea_arco = next((r for r in renglones if r.startswith("ARCO: ")), None)
    if not linea_arco:
        print("  !!  El robot no dijo que arco tiene cargado. La carga dio SUCCESS,")
        print("      pero NO esta confirmado. Mira el monitor serie (19200) y busca")
        print("      la linea 'ARCO:'.")
        sys.exit(2)
    if not re.match(r"^ARCO: %s\s+\(fijo por programa\)" % nombre, linea_arco):
        error_y_salir("EL ROBOT DICE '%s' Y ELEGISTE ARCO %s. NO JUEGUES ASI: volve a "
                      "cargar." % (linea_arco.strip(), nombre))

    linea()
    print("  LISTO: EL ROBOT ATACA EL ARCO %s  (confirmado por el robot)" % nombre)
    linea()
    print()
    print("  Ahora:")
    print("   1. Desenchufa el USB.")
    print("   2. En la cancha, apoya el robot MIRANDO AL ARCO RIVAL.")
    print("   3. Recien ahi prende la bateria. Si ya estaba prendida: apagala y prendela.")
    print("      (al prender guarda hacia donde mira: es su plan B para patear)")
    print()
    print("  Si cambian de lado en el entretiempo: correr esto de nuevo con el otro arco.")
    print()


# ---------------------------------------------------------------------
# MEDIR: cargar una de las pruebas
# ---------------------------------------------------------------------

def comprobar_herramientas(proyecto):
    if not os.path.isfile(PIO):
        error_y_salir("No encuentro PlatformIO en " + PIO)
    if not os.path.isfile(LOADER):
        error_y_salir("No encuentro el Teensy Loader en " + LOADER)
    if not os.path.isfile(os.path.join(proyecto, "platformio.ini")):
        error_y_salir("No encuentro el proyecto en " + proyecto)


def lista_de_pruebas():
    if not os.path.isdir(PRUEBAS):
        return []
    return sorted(d for d in os.listdir(PRUEBAS)
                  if os.path.isfile(os.path.join(PRUEBAS, d, "platformio.ini")))


def medir(carpeta):
    proyecto = os.path.join(PRUEBAS, carpeta)
    comprobar_herramientas(proyecto)

    linea()
    print("   DELANTERO  -  MEDIR (no juega): %s" % carpeta)
    linea()
    print("   Antes: BATERIA APAGADA si la prueba NO mueve motores.")
    print("   Y SOLO este robot enchufado: el cargador no elige.")
    print()

    print("[1/3] Buscando el robot...")
    comprobar_delantero()
    print("[2/3] Teensy Loader...")
    abrir_loader()
    print("[3/3] Compilando y cargando (la primera vez tarda ~1 min)...")
    try:
        compilar_y_cargar(proyecto)
        # No cerrar el Loader hasta que el robot vuelva: si se lo mata mientras
        # todavia esta grabando, la Teensy queda sin programa.
        volvio = esperar_puerto(15)
    finally:
        cerrar_loader()
    if not volvio:
        error_y_salir("El robot no volvio a aparecer por USB despues de cargar. "
                      "No se sabe si quedo cargado: desenchufa, enchufa y proba de nuevo.")
    print("      OK: cargado, y el robot volvio a aparecer en " + volvio)

    print()
    linea()
    if carpeta in SIN_CABLE:
        print("   LISTO. Esta prueba GRABA SOLA, sin cable.")
        print()
        print("   1. PRENDE LA BATERIA")
        print("   2. recien ahi DESENCHUFA el USB")
        print("      (si se corta la alimentacion se reinicia y se pierde todo)")
        print("   3. hace la medicion en la cancha")
        print("   4. volve, enchufa el USB SIN APAGAR LA BATERIA, y usa LEER")
    else:
        print("   LISTO. Esta prueba NECESITA EL CABLE: usa LEER para ver lo que dice.")
    linea()
    print()


# ---------------------------------------------------------------------
# LEER: lo que el robot tiene para decir. Guarda SIEMPRE en archivo antes
# de mostrar: ya se perdio una medicion entera por mostrarla recortada sin
# guardarla. [08/09]
# ---------------------------------------------------------------------

def leer(segundos=25, mandar_tecla=True):
    puerto = buscar_puerto()
    if not puerto:
        error_y_salir("El robot no aparece por USB. Enchufa el cable.")
    print("      leyendo %s durante %d s..." % (puerto, segundos))
    sp = abrir_serie(puerto)
    time.sleep(0.25)
    if mandar_tecla:
        try:
            sp.write(b"\r\n")
        except (serial.SerialException, OSError):
            pass
    trozos = []
    fin = time.time() + segundos
    while time.time() < fin:
        try:
            trozos.append(sp.read(4096))
        except (serial.SerialException, OSError):
            break
    try:
        sp.close()
    except Exception:
        pass
    texto = b"".join(trozos).decode("utf-8", errors="replace")

    os.makedirs(MEDICIONES, exist_ok=True)
    sello = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    archivo = os.path.join(MEDICIONES, "medicion-%s.txt" % sello)
    with open(archivo, "w", encoding="utf-8", newline="") as f:
        f.write(texto)

    print()
    linea()
    if not texto.strip():
        print("  El robot no dijo nada.")
        print("  Si la prueba esperaba una tecla, proba de nuevo.")
        print("  Si se corto la bateria al desenchufar, la grabacion se perdio.")
    else:
        print(texto)
    linea()
    print("  Guardado en: " + archivo)
    return texto


def ver():
    teensys = buscar_teensys()
    if not teensys:
        print("No hay ninguna Teensy enchufada.")
        return
    for pid, serie in teensys:
        if pid == "0478":
            dec = serie_decimal(serie) or "?"
            modo = "modo CARGA"
        else:
            dec = serie
            modo = "corriendo un programa"
        quien = "DELANTERO" if dec == SERIE_DELANTERO else "NO es el delantero"
        print("Teensy serie %s  (%s)  ->  %s" % (dec, modo, quien))
    print("Puerto serie: %s" % (buscar_puerto() or "ninguno"))


# ---------------------------------------------------------------------

def menu_medir():
    pruebas = lista_de_pruebas()
    while True:
        print()
        linea()
        print("   DELANTERO  -  MEDIR (no juega)   (sin PowerShell)")
        linea()
        print("   Ninguno de estos programas juega: solo miden.")
        print("   Para el partido es CARGAR-ROBOT-SIN-POWERSHELL.bat, no este.")
        print()
        for i, p in enumerate(pruebas, 1):
            marca = "*" if p in SIN_CABLE else " "
            print("  %3d) %s %s" % (i, marca, p))
        print()
        print("    L)   LEER lo que el robot tiene para decir (volviste de la cancha)")
        print("    V)   VER que robot hay enchufado")
        print("    S)   salir")
        print()
        print("   * = no necesita el cable: graba con la bateria")
        print()
        op = preguntar("   Que hacemos: ").upper()
        if op in ("S", ""):
            return
        if op == "L":
            leer(25, True)
            preguntar("   Enter para volver al menu ")
            continue
        if op == "V":
            ver()
            continue
        if op.isdigit() and 1 <= int(op) <= len(pruebas):
            carpeta = pruebas[int(op) - 1]
            if preguntar("   Cargo '%s'. Sigo? (s/n) " % carpeta).lower() == "s":
                medir(carpeta)
                if preguntar("   Queres leer el robot ahora? (s/n) ").lower() == "s":
                    leer(20, carpeta == "grabar-linea")
            continue
        print("   '%s' no esta en la lista. De nuevo." % op)


def main():
    args = sys.argv[1:]
    orden = args[0].lower() if args else "menu"
    if orden == "cargar":
        cargar(args[1] if len(args) > 1 else "")
    elif orden == "medir":
        if len(args) > 1:
            medir(args[1])
        else:
            menu_medir()
    elif orden == "leer":
        segundos = 25
        tecla = True
        for a in args[1:]:
            if a.isdigit():
                segundos = int(a)
            elif a.lower() == "sin-tecla":
                tecla = False
        leer(segundos, tecla)
    elif orden == "ver":
        ver()
    else:
        menu_medir()


if __name__ == "__main__":
    main()
