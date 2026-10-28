# -*- coding: utf-8 -*-
r"""
=====================================================================
  TABLERO DEL DELANTERO - una ventana con botones para mirar y probar
  el robot sin escribir comandos.
  IITA Salta - Roboliga 2026 - 2026-10-27
=====================================================================

QUÉ TIENE
  PELOTA       dibuja, visto desde arriba, dónde ve el robot la pelota y
               los dos arcos.
  SENSORES     los 3 sensores de línea, el giroscopio y la cámara.
  MOVIMIENTOS  prueba los 7 movimientos del partido (patada, avance, los
               3 escapes de la línea, órbita y giro), cambia sus números
               SIN recargar el robot, y dice si salió derecho (con gráfico).
  CONSOLA      todo lo que dice el robot, y una caja para escribirle a mano.

CÓMO SE ABRE
  Doble clic en TABLERO.bat: carga en el robot el programa del tablero
  (tablero.ino, al lado de este archivo) y abre esta ventana.
  Sin robot: TABLERO-DEMO.bat abre la ventana con un robot de mentira.

  A mano (con el Python de PlatformIO):
     python tablero.py                  busca el robot solo
     python tablero.py --puerto COM7    para forzar el puerto
     python tablero.py --demo           robot de mentira (no abre el USB)
     python tablero.py --autoprueba     se prueba a sí mismo, SIN abrir la
                                        ventana ni el USB. Tiene que decir OK.

CÓMO FUNCIONA POR DENTRO (para el que lo quiera tocar)
  El robot y la ventana se hablan por el cable USB con renglones de texto.
  Los renglones para la ventana empiezan con "#": por ejemplo, "#S ..."
  trae los sensores 10 veces por segundo. Los demás son para personas y
  van a la CONSOLA. Las reglas exactas están en EL CONTRATO (abajo) y
  arriba de todo en tablero.ino. Si se cambia de un lado, hay que
  cambiarlo del otro.

  El archivo está en partes, de lo más simple a lo más complicado:
    1. EL CONTRATO       los nombres, los movimientos y los números.
    2. EL TRADUCTOR      pasa cada renglón del robot a datos. Son funciones
                         "puras": no tocan la ventana ni el cable, y por
                         eso se pueden probar solas (--autoprueba).
    3. LA MEMORIA        junta lo que va llegando: el último #S, los
                         números, el resultado de la última prueba.
    4. EL CABLE          un hilo aparte lee el USB, así la ventana nunca
                         se traba esperando al robot.
    5. EL ROBOT DE MENTIRA (--demo)
    6. LA VENTANA        los botones y los dibujos (tkinter).
    7. LA AUTOPRUEBA

  ¿Por qué un hilo aparte? Leer el cable es "esperar a que llegue algo".
  Si la ventana esperara, se congelaría. Entonces un hilo espera y deja
  cada renglón en una COLA; la ventana mira la cola cada 50 ms (con
  after()) y se dibuja con lo que encuentra. tkinter solo se toca desde
  su propio hilo: el del cable NUNCA toca la ventana.
"""

import argparse
import math
import os
import queue
import random
import re
import sys
import threading
import time
import traceback

try:
    import tkinter as tk
    from tkinter import ttk, messagebox
except ImportError:              # sin tkinter igual anda la --autoprueba
    tk = ttk = messagebox = None

try:
    import serial
    from serial.tools import list_ports
except ImportError:              # sin pyserial anda la --autoprueba y la --demo
    serial = list_ports = None

CARPETA = os.path.dirname(os.path.abspath(__file__))
ARCHIVO_FALLAS = os.path.join(CARPETA, "tablero-error.txt")


# =====================================================================
#  1. EL CONTRATO: lo que el robot y la ventana acordaron
# =====================================================================

SERIE_DELANTERO = "15708680"     # el número de serie del Teensy del delantero
VID_TEENSY = 0x16C0              # el "fabricante" de las Teensy (PJRC): no cambia nunca
PID_SERIE = 0x0483               # una Teensy corriendo un programa, con su puerto COM
BAUDIOS = 19200
PROGRAMA = "tablero-delantero"   # lo que dice "#HOLA" si el robot tiene el programa del tablero
FECHA_CONTRATO = "2026-10-27"

ETIQUETAS = ("HOLA", "S", "V", "OK", "ERROR", "CUENTA", "INICIO", "FRENO",
             "M", "R", "CONSEJO", "PANEL", "FINPANEL")
MODOS = ("ESPERA", "CUENTA", "MOVIENDO")
FASES = {"g": "golpe", "r": "retroceso", "m": "movimiento"}
CAMPOS_R = ("pico", "picoms", "pico200", "fin", "fin2", "cruces", "giro", "gps",
            "duro", "hueco", "huecoms", "plantado")
EDAD_VIEJA_MS = 300              # pelota/arco vistos hace más que esto: se dibujan en gris
ANGULO_CAMARA = 34               # grados para cada lado que ve la cámara (más o menos)
MAX_MUESTRAS = 1000              # el robot manda hasta 300; esto es por las dudas

# Las notas: (color de fondo, color de letra, cómo se lee, qué quiere decir)
NOTAS = {
    "PROLIJO":    ("#2e7d32", "white", "PROLIJO",
                   "Fue derecho: se torció 4 grados o menos, casi sin zigzag, y terminó derecho."),
    "CASI":       ("#f9a825", "#212121", "CASI",
                   "Anda, pero se torció un poco. Mirá los números y el consejo."),
    "ZIGZAGUEA":  ("#ef6c00", "white", "ZIGZAGUEA",
                   "Va y viene de un lado al otro: cruzó 4 veces o más."),
    "SE_TUERCE":  ("#c62828", "white", "SE TUERCE",
                   "Se torció 10 grados o más."),
    "GIRA_BIEN":  ("#2e7d32", "white", "GIRA BIEN",
                   "Gira parejo y no se planta."),
    "SE_PLANTA":  ("#c62828", "white", "SE PLANTA",
                   "En algún momento estuvo más de 400 ms casi sin girar."),
    "NO_ARRANCA": ("#c62828", "white", "NO ARRANCA",
                   "Giró menos de 20 grados en total: no arrancó."),
    "NO_VALE":    ("#757575", "white", "NO VALE",
                   "Esta prueba no vale: la frenaron, se pasó del tiempo, o no había giroscopio."),
}

# Los números que se cambian desde la ventana.
#   NOMBRE: (paso de los botones [-] [+], qué hace, opciones)
#   opciones = None para un número; si es un interruptor, los dos textos
#   de sus botones (para el valor 0 y para el 1).
NUMEROS = {
    "VEL_PATADA":         (5, "Fuerza del golpe (60 a 255). Más = llega más lejos, pero se levanta y vuelca más fácil.", None),
    "MS_PATADA":          (50, "Cuánto dura el golpe, en ms (1000 ms = 1 segundo). Más = llega más lejos.", None),
    "TRIM_PATADA":        (1, "Compensación fija. Si se va a la DERECHA: subir. Si se va a la IZQUIERDA: bajar.", None),
    "KP_PATADA":          (0.5, "Corrige por cada grado torcido. Más = corrige más fuerte (y puede zigzaguear).", None),
    "KD_PATADA":          (0.1, "Frena el giro antes de que crezca: baja el pico.", None),
    "VEL_RETROCESO":      (5, "Fuerza de la marcha atrás después del golpe.", None),
    "MS_RETROCESO":       (50, "Cuánto dura la marcha atrás (ms).", None),
    "LAZO":               (None, "SÍ = corrige con el giroscopio durante el golpe (como juega). NO = solo el TRIM: sirve para ajustar el TRIM. Solo de esta prueba.", ("NO", "SÍ")),
    "TRASERA_TRABADA":    (None, "SÍ = la rueda de atrás queda trabada durante el golpe (como juega). Solo de esta prueba.", ("NO", "SÍ")),
    "SIN_ESPERA":         (None, "SÍ = mira si el giroscopio anda SIN clavar el programa 200 ms. Si la lectura rápida no coincide, el robot no lo deja. Solo de esta prueba.", ("NO", "SÍ")),
    "VEL_AVANCE":         (5, "Fuerza yendo hacia la pelota cuando está lejos.", None),
    "MS_PRUEBA_AVANCE":   (100, "Cuánto avanza en esta prueba (ms). Solo de esta prueba.", None),
    "VEL_ESCAPE_FUERTE":  (5, "Fuerza del escape. Más = se despega más rápido, pero se levanta.", None),
    "MS_RETROCESO_LINEA": (10, "Cuánto dura el escape (ms).", None),
    "VEL_ORB_IMPULSO":    (5, "Golpe de la rueda de atrás para arrancar a rodear.", None),
    "MS_ORB_IMPULSO":     (50, "Cuánto dura ese golpe (ms). Si no arranca: subir.", None),
    "VEL_ORB_TRASERA":    (5, "Fuerza de la vuelta. Más = más rápida. Si se planta: subir.", None),
    "VEL_ORB_FRENTE":     (5, "Las dos de adelante, bajito a propósito para que no giren. Nunca cerca de 70.", None),
    "MS_PRUEBA_ORBITA":   (500, "Cuánto rodea en esta prueba (ms). Solo de esta prueba.", None),
    "SENTIDO_ORBITA":     (None, "Para qué lado rodea (0 o 1). Arranca igual que en el partido. Solo de esta prueba.", ("0", "1")),
    "VEL_GIRO_CONTINUO":  (5, "Fuerza del giro buscando la pelota. Si zumba y no gira: subir.", None),
    "RAMPA_GIRO_PASO":    (2, "El giro arranca suave: sube esto de fuerza...", None),
    "RAMPA_GIRO_MS":      (2, "...cada esta cantidad de ms.", None),
    "MS_PRUEBA_GIRO":     (500, "Cuánto gira en esta prueba (ms). Solo de esta prueba.", None),
}

AVISO_LINEA = "En estas pruebas el robot NO mira la línea blanca."
# Lo que hay que hacer si el robot no encuentra el giroscopio: el robot lo
# vuelve a buscar cada vez que se aprieta PROBAR.
AYUDA_SIN_GIRO = "prendé la batería (antes que el USB) y apretá PROBAR: lo busca de nuevo."
# Si el robot NO tiene el programa del tablero, la "x" no lo frena.
NO_OBEDECE_PARAR = "Este programa NO obedece PARAR: levantá el robot o apagá la batería."

# Los 7 movimientos. tipo: "patada" y "derecho" se miden por cuánto se
# tuercen; "giro" por cuántos grados por segundo gira y si se planta.
MOVIMIENTOS = [
    dict(n=1, nombre="PATADA", tipo="patada", boton="1   PATADA",
         titulo="1 · PATADA  (golpe + retroceso)",
         que_hace="El golpe del partido (con la rampa, el TRIM, el corrector del giroscopio y la "
                  "rueda de atrás trabada) y después la marcha atrás. Mide si el golpe va derecho.",
         lugar="Dejale lugar ADELANTE (más de un metro) y ATRÁS.",
         numeros=["VEL_PATADA", "MS_PATADA", "TRIM_PATADA", "KP_PATADA", "KD_PATADA",
                  "VEL_RETROCESO", "MS_RETROCESO", "LAZO", "TRASERA_TRABADA", "SIN_ESPERA"]),
    dict(n=2, nombre="AVANCE", tipo="derecho", boton="2   AVANCE",
         titulo="2 · AVANCE",
         que_hace="Avanza como cuando va a buscar la pelota de lejos. El avance del partido NO "
                  "corrige el rumbo: acá se mide cuánto se tuerce.",
         lugar="Dejale lugar ADELANTE.",
         numeros=["VEL_AVANCE", "MS_PRUEBA_AVANCE"]),
    dict(n=3, nombre="ESCAPE_S1", tipo="derecho", boton="3   ESCAPE  sensor 1",
         titulo="3 · ESCAPE del sensor 1 (DERECHO)",
         que_hace="Escapa como si el sensor 1 (DERECHO) viera la línea: sale para adelante a la "
                  "izquierda. Tendría que alejarse SIN girar: se mide cuánto gira.",
         lugar="Dejale lugar adelante y a la IZQUIERDA.",
         numeros=["VEL_ESCAPE_FUERTE", "MS_RETROCESO_LINEA"]),
    dict(n=4, nombre="ESCAPE_S2", tipo="derecho", boton="4   ESCAPE  sensor 2",
         titulo="4 · ESCAPE del sensor 2 (IZQUIERDO)",
         que_hace="Escapa como si el sensor 2 (IZQUIERDO) viera la línea: sale para adelante a la "
                  "derecha. Tendría que alejarse SIN girar: se mide cuánto gira.",
         lugar="Dejale lugar adelante y a la DERECHA.",
         numeros=["VEL_ESCAPE_FUERTE", "MS_RETROCESO_LINEA"]),
    dict(n=5, nombre="ESCAPE_S3", tipo="derecho", boton="5   ESCAPE  sensor 3",
         titulo="5 · ESCAPE del sensor 3 (DELANTERO)",
         que_hace="Escapa como si el sensor 3 (DELANTERO) viera la línea: sale derecho para atrás. "
                  "Tendría que alejarse SIN girar: se mide cuánto gira.",
         lugar="Dejale lugar ATRÁS.",
         numeros=["VEL_ESCAPE_FUERTE", "MS_RETROCESO_LINEA"]),
    dict(n=6, nombre="ORBITA", tipo="giro", boton="6   ÓRBITA",
         titulo="6 · ÓRBITA  (rodear la pelota)",
         que_hace="Rodea una pelota imaginaria: un golpe de la rueda de atrás para arrancar y "
                  "después la vuelta. Mide cuántos grados por segundo gira y si se planta.",
         lugar="Dejale lugar ALREDEDOR (más o menos medio metro).",
         numeros=["VEL_ORB_IMPULSO", "MS_ORB_IMPULSO", "VEL_ORB_TRASERA", "VEL_ORB_FRENTE",
                  "MS_PRUEBA_ORBITA", "SENTIDO_ORBITA"]),
    dict(n=7, nombre="GIRO", tipo="giro", boton="7   GIRO  (buscar)",
         titulo="7 · GIRO  (buscando la pelota)",
         que_hace="Gira en el lugar como cuando busca la pelota, con su arranque suave (rampa). "
                  "Mide cuántos grados por segundo gira y si arranca.",
         lugar="Gira en el lugar: que no tenga nada pegado.",
         numeros=["VEL_GIRO_CONTINUO", "RAMPA_GIRO_PASO", "RAMPA_GIRO_MS", "MS_PRUEBA_GIRO"]),
]
MOV_POR_NUMERO = {m["n"]: m for m in MOVIMIENTOS}
MOV_POR_NOMBRE = {m["nombre"]: m for m in MOVIMIENTOS}

# El estado del giroscopio (BNO055, registro 0x39 = SYS_STATUS)
ESTADOS_GIRO = {
    0: "quieto, sin arrancar",
    1: "ERROR del sistema",
    2: "arrancando (periféricos)",
    3: "arrancando",
    4: "haciendo su autoprueba",
    5: "fusión OK",
    6: "anda, pero SIN fusión",
}


# =====================================================================
#  2. EL TRADUCTOR: de un renglón del robot a datos
# =====================================================================
#  interpretar(renglon) devuelve SIEMPRE un diccionario con "tipo":
#     "TEXTO"        un renglón para personas (no empieza con #)
#     "HOLA", "S", "V", ...  uno del contrato, ya traducido
#     "ROTA"         empieza con # pero vino mal (cortado, pegado con
#                    otro, un número raro). Se muestra en la consola en
#                    rojo y no se usa.
#     "DESCONOCIDA"  empieza con # pero no es del contrato
#  y "texto" con el renglón tal cual llegó. Nunca tira error.

_RE_NUMERO = re.compile(r"^[-+]?(?:\d+(?:\.\d*)?|\.\d+)$")
_RE_ENTERO = re.compile(r"^[-+]?\d+$")
_RE_NOMBRE = re.compile(r"^[A-Z][A-Z0-9_]*$")
_RE_OTRA_ETIQUETA = re.compile(r"#(?:" + "|".join(ETIQUETAS) + r")(?=\s|$)")
# Lo que imprime Arduino cuando una cuenta da cualquier cosa:
_SIN_NUMERO = {"nan", "-nan", "inf", "-inf", "+inf", "ovf", "-ovf"}
# El número más largo que manda el robot tiene unos 13 caracteres
# ("-4294967040.0"). Uno más largo es basura: con 400 cifras, float()
# ni siquiera puede (tira OverflowError).
LARGO_MAX_NUMERO = 15


def leer_numero(texto):
    """'12' -> 12, '-0.5' -> -0.5. 'nan', 'inf' u 'ovf' -> None (no se sabe).
    Cualquier otra cosa (con coma, con letras, vacío, larguísimo) -> ValueError."""
    t = str(texto)
    if t.lower() in _SIN_NUMERO:
        return None
    if len(t) > LARGO_MAX_NUMERO or not _RE_NUMERO.match(t):
        raise ValueError("no es un número: %r" % t[:20])
    if _RE_ENTERO.match(t):
        return int(t)
    return float(t)


def _numero(texto):
    """Como leer_numero, pero acá 'nan' tampoco vale."""
    v = leer_numero(texto)
    if v is None:
        raise ValueError("número inválido: %r" % texto)
    return v


def _entero(texto):
    t = str(texto)
    if len(t) > LARGO_MAX_NUMERO or not _RE_ENTERO.match(t):
        raise ValueError("no es un número entero: %r" % t[:20])
    return int(t)


def _lista(texto, cuantos):
    """'310,322,705' -> [310, 322, 705]. Tiene que tener justo 'cuantos'."""
    partes = texto.split(",")
    if len(partes) != cuantos:
        raise ValueError("se esperaban %d números en %r" % (cuantos, texto))
    return [leer_numero(p) for p in partes]


def _pares(resto):
    """'t=12 e=ESPERA' -> {'t': '12', 'e': 'ESPERA'}. Un pedazo sin '=' o
    sin valor quiere decir que el renglón vino roto."""
    d = {}
    for pedazo in resto.split():
        clave, igual, valor = pedazo.partition("=")
        if not igual or not clave or not valor:
            raise ValueError("pedazo roto: %r" % pedazo)
        d[clave] = valor
    return d


def _i_hola(resto):
    partes = resto.split()
    if not partes:
        raise ValueError("#HOLA sin nombre")
    return {"nombre": partes[0], "fecha": partes[1] if len(partes) > 1 else ""}


def _i_s(resto):
    d = _pares(resto)
    # t va primero y e va último: si falta alguno, el renglón se cortó.
    if "t" not in d or "e" not in d:
        raise ValueError("faltan t= o e= (renglón cortado)")
    if d["e"] not in MODOS:
        raise ValueError("e= desconocido: %r" % d["e"])
    m = {"t": _entero(d["t"]), "e": d["e"]}
    m["l"] = _lista(d["l"], 3) if "l" in d else None
    m["u"] = _lista(d["u"], 3) if "u" in d else None
    r = leer_numero(d["r"]) if "r" in d else None
    m["r"] = float(r) if (r is not None and r >= 0) else None      # -1 = sin giroscopio
    g = leer_numero(d["g"]) if "g" in d else None
    m["g"] = int(g) if (g is not None and g >= 0) else None        # -1 = sin giroscopio
    for clave in ("p", "am", "az"):
        if clave in d:
            x, y, edad = _lista(d[clave], 3)
            m[clave] = (x, y, -1 if edad is None else edad)       # edad -1 = nunca se vio
        else:
            m[clave] = None
    m["q"] = leer_numero(d["q"]) if "q" in d else None
    return m


def _i_v(resto):
    valores = {}
    for pedazo in resto.split():
        nombre, igual, valor = pedazo.partition("=")
        if not igual or not _RE_NOMBRE.match(nombre):
            raise ValueError("pedazo roto: %r" % pedazo)
        _numero(valor)                    # que sea un número de verdad
        valores[nombre] = valor           # se guarda como lo escribió el robot ("0.6")
    if not valores:
        raise ValueError("#V vacío")
    return {"valores": valores}


def _i_ok(resto):
    partes = resto.split()
    if len(partes) != 1:
        raise ValueError("#OK lleva un solo NOMBRE=valor")
    nombre, igual, valor = partes[0].partition("=")
    if not igual or not _RE_NOMBRE.match(nombre):
        raise ValueError("#OK roto")
    _numero(valor)
    return {"nombre": nombre, "valor": valor}


def _i_error(resto):
    return {"mensaje": resto.strip()}


def _i_cuenta(resto):
    partes = resto.split()
    if len(partes) != 1:
        raise ValueError("#CUENTA lleva un número")
    return {"segundos": float(_numero(partes[0]))}


def _i_inicio(resto):
    partes = resto.split()
    if len(partes) != 2 or not _RE_NOMBRE.match(partes[1]):
        raise ValueError("#INICIO lleva número y NOMBRE")
    return {"n": _entero(partes[0]), "nombre": partes[1]}


def _i_freno(resto):
    return {"motivo": resto.strip()}


def _i_m(resto):
    partes = resto.split()
    if len(partes) != 3:
        raise ValueError("#M lleva 3 datos")
    t = _numero(partes[0])
    grados = _numero(partes[1])
    fase = partes[2]
    if fase not in FASES or t < 0:
        raise ValueError("#M raro")
    return {"t": t, "grados": float(grados), "fase": fase}


def _i_r(resto):
    d = _pares(resto)
    for clave in ("n", "mov", "nota"):
        if clave not in d:
            raise ValueError("falta %s= (renglón cortado)" % clave)
    if d["nota"] not in NOTAS:
        raise ValueError("nota desconocida: %r" % d["nota"])
    m = {"n": _entero(d["n"]), "mov": d["mov"], "nota": d["nota"]}
    for clave in CAMPOS_R:
        m[clave] = leer_numero(d[clave]) if clave in d else None
    return m


def _i_consejo(resto):
    return {"mensaje": resto.strip()}


def _i_panel(resto):
    # Tal cual, con sus espacios: es una línea de C++ para pegar.
    return {"codigo": resto}


def _i_finpanel(resto):
    return {}


_INTERPRETES = {
    "HOLA": _i_hola, "S": _i_s, "V": _i_v, "OK": _i_ok, "ERROR": _i_error,
    "CUENTA": _i_cuenta, "INICIO": _i_inicio, "FRENO": _i_freno, "M": _i_m,
    "R": _i_r, "CONSEJO": _i_consejo, "PANEL": _i_panel, "FINPANEL": _i_finpanel,
}


def interpretar(renglon):
    """Traduce UN renglón del robot. Ver la explicación de arriba."""
    texto = "" if renglon is None else str(renglon)
    texto = texto.rstrip("\r\n")
    limpio = texto.rstrip()
    if not limpio.startswith("#"):
        return {"tipo": "TEXTO", "texto": texto}
    cabeza, _, resto = limpio[1:].partition(" ")
    if "#" in cabeza:
        return {"tipo": "ROTA", "texto": texto, "motivo": "dos renglones pegados"}
    if cabeza not in ETIQUETAS:
        return {"tipo": "DESCONOCIDA", "texto": texto}
    # Si se cortó un renglón por la mitad, el que sigue queda pegado:
    #   "#S t=12 l=31#S t=112 l=..."  -> no se usa ninguno de los dos.
    if _RE_OTRA_ETIQUETA.search(resto):
        return {"tipo": "ROTA", "texto": texto, "motivo": "dos renglones pegados"}
    try:
        m = _INTERPRETES[cabeza](resto)
    except (ValueError, KeyError, IndexError, TypeError, OverflowError) as e:
        return {"tipo": "ROTA", "texto": texto, "motivo": str(e)}
    m["tipo"] = cabeza
    m["texto"] = texto
    return m


# =====================================================================
#  3. LA MEMORIA: lo que se sabe del robot
# =====================================================================

class Estado:
    """Junta lo que va llegando. aplicar() devuelve qué cambió, para que la
    ventana redibuje solo eso:
       "hola" "valores" "sensores" "modo" "resultado" "freno" "ok" "error"
       "panel_listo" "rota"
    No toca la ventana: por eso la autoprueba la puede probar sola."""

    def __init__(self):
        self.valores = {}            # NOMBRE -> "valor" como lo mandó el robot
        self.mov_n = None            # el movimiento del último resultado
        self.mov_nombre = None
        self.freno = None            # motivo del último #FRENO
        self.muestras = []           # [(t_ms, grados, fase)] de las #M
        self.informe = None          # el último #R
        self.consejos = []
        self.panel = []              # las #PANEL juntadas
        self.rotas = 0
        self.ultimo_ok = None
        self.ultimo_error = None
        self.reiniciar_conexion()

    def reiniciar_conexion(self):
        """Se enchufó (o se reenchufó) el robot: lo que dependía de él se
        olvida. El último resultado queda, para no perder el gráfico."""
        self.hola = None             # (nombre del programa, fecha)
        self.sensores = None         # el último #S
        self.t_sensores = None       # cuándo llegó (reloj de la PC)
        self.modo = "ESPERA"
        self.cuenta = None
        self.panel_abierto = False

    def _nuevo_resultado(self):
        self.freno = None
        self.muestras = []
        self.informe = None
        self.consejos = []

    def aplicar(self, m, ahora=0.0):
        tipo = m.get("tipo")
        cambios = set()
        if tipo == "S":
            self.sensores = m
            self.t_sensores = ahora
            cambios.add("sensores")
            if m["e"] != self.modo:
                self.modo = m["e"]
                if self.modo == "ESPERA":
                    self.cuenta = None
                cambios.add("modo")
        elif tipo == "HOLA":
            # El robot arrancó de nuevo (o contestó a "h"): está en espera.
            self.hola = (m["nombre"], m["fecha"])
            self.panel_abierto = False
            if self.modo != "ESPERA":
                self.modo = "ESPERA"
                self.cuenta = None
                cambios.add("modo")
            cambios.add("hola")
        elif tipo == "V":
            self.valores.update(m["valores"])
            cambios.add("valores")
        elif tipo == "OK":
            self.valores[m["nombre"]] = m["valor"]
            self.ultimo_ok = (m["nombre"], m["valor"])
            cambios |= {"valores", "ok"}
        elif tipo == "ERROR":
            self.ultimo_error = m["mensaje"]
            cambios.add("error")
        elif tipo == "CUENTA":
            if self.modo != "CUENTA":          # empieza una prueba nueva
                self._nuevo_resultado()
                self.mov_n = self.mov_nombre = None
                cambios.add("resultado")
            self.modo = "CUENTA"
            self.cuenta = m["segundos"]
            cambios.add("modo")
        elif tipo == "INICIO":
            self._nuevo_resultado()
            self.modo = "MOVIENDO"
            self.cuenta = None
            self.mov_n = m["n"]
            self.mov_nombre = m["nombre"]
            cambios |= {"modo", "resultado"}
        elif tipo == "FRENO":
            # Frenado en la cuenta, o "sin giroscopio" (ese llega en ESPERA,
            # sin ningún #CUENTA antes): no se llegó a mover y no viene
            # informe. Lo de la prueba anterior se borra, si no quedaría a
            # la vista una nota vieja como si fuera de esta.
            # Moviéndose, después vienen las #M y el #R.
            if self.modo != "MOVIENDO":
                self._nuevo_resultado()
                self.mov_n = self.mov_nombre = None
            self.freno = m["motivo"]
            cambios |= {"freno", "resultado"}
            if self.modo == "CUENTA":
                self.modo = "ESPERA"
                self.cuenta = None
                cambios.add("modo")
        elif tipo == "M":
            if self.informe is not None:
                # Una #M después de un informe completo es de una prueba
                # NUEVA cuyo #INICIO no se vio (se reconectó el cable en
                # medio del movimiento): no se mezcla con el gráfico viejo.
                self._nuevo_resultado()
                self.mov_n = self.mov_nombre = None
            if len(self.muestras) < MAX_MUESTRAS:
                self.muestras.append((m["t"], m["grados"], m["fase"]))
                cambios.add("resultado")
        elif tipo == "R":
            self.informe = m
            self.mov_n = m["n"]
            self.mov_nombre = m["mov"]
            self.modo = "ESPERA"
            self.cuenta = None
            cambios |= {"resultado", "modo"}
        elif tipo == "CONSEJO":
            self.consejos.append(m["mensaje"])
            cambios.add("resultado")
        elif tipo == "PANEL":
            if not self.panel_abierto:         # la primera de una tanda nueva
                self.panel = []
                self.panel_abierto = True
            self.panel.append(m["codigo"])
        elif tipo == "FINPANEL":
            if not self.panel_abierto:         # no había nada para copiar
                self.panel = []
            self.panel_abierto = False
            cambios.add("panel_listo")
        elif tipo == "ROTA":
            self.rotas += 1
            cambios.add("rota")
        return cambios


# ---------- cuentas chicas que usa la ventana (puras: se prueban solas) ----------

def tipo_de(nombre_mov):
    m = MOV_POR_NOMBRE.get(nombre_mov or "")
    return m["tipo"] if m else "derecho"


def lado(grados):
    """-3.2 -> '3.2 grados a la DERECHA'. Signo medido el 01/09:
    NEGATIVO = se fue a la DERECHA, POSITIVO = a la IZQUIERDA."""
    if grados is None:
        return "? grados"
    if abs(grados) < 0.05:
        return "0 grados (derecho)"
    return "%.1f grados a la %s" % (abs(grados), "IZQUIERDA" if grados > 0 else "DERECHA")


def visible(dato):
    """¿Se vio alguna vez? dato = (x, y, edad) de un #S."""
    return (dato is not None and dato[0] is not None and dato[1] is not None
            and dato[2] is not None and dato[2] >= 0)


def describir_objeto(nombre, dato, femenino=True):
    """'Pelota a 60 cm, 24 grados a la izquierda'. Devuelve (frase, como),
    con como = 'fresca', 'vieja' (hace más de 0.3 s) o 'nunca'.
    X = para adelante, Y = al costado, Y POSITIVO = IZQUIERDA (01/09)."""
    no = "No la veo" if femenino else "No lo veo"
    if not visible(dato):
        return no, "nunca"
    x, y, edad = dato
    distancia = math.hypot(x, y)
    angulo = math.degrees(math.atan2(y, x))
    if abs(angulo) < 2:
        hacia = "derecho adelante"
    else:
        hacia = "%d grados a la %s" % (round(abs(angulo)), "izquierda" if angulo > 0 else "derecha")
    if edad > EDAD_VIEJA_MS:
        return "%s. La última vez (hace %.1f s): a %d cm, %s" % (
            no, edad / 1000.0, round(distancia), hacia), "vieja"
    return "%s a %d cm, %s" % (nombre, round(distancia), hacia), "fresca"


def cm_a_pantalla(x_cm, y_cm, cx, cy, escala):
    """Del robot (cm) a la pantalla (píxeles), visto desde arriba con el
    robot mirando para ARRIBA:
      X (para adelante) -> para arriba: la y de la pantalla BAJA.
      Y positivo (a la IZQUIERDA del robot) -> a la izquierda de la pantalla."""
    return cx - y_cm * escala, cy - x_cm * escala


def _decimales(texto):
    t = str(texto)
    return len(t.split(".", 1)[1]) if "." in t else 0


def nuevo_valor(texto_actual, paso, signo):
    """Lo que manda [+] o [-]: nuevo_valor('0.6', 0.1, +1) -> '0.7'.
    Respeta los decimales que ya tenía ('4.0' + 0.5 -> '4.5')."""
    actual = _numero(texto_actual)
    dec = max(_decimales(texto_actual), _decimales(repr(float(paso))) if paso != int(paso) else 0)
    v = round(actual + signo * paso, dec) + 0.0          # + 0.0 saca el "-0"
    if dec == 0:
        return str(int(round(v)))
    return "%.*f" % (dec, v)


def normalizar_entrada(texto):
    """Lo que escriben en la cajita: ' 0,7 ' -> '0.7'. Si no es un número: None."""
    t = str(texto).strip().replace(",", ".")
    if not t or len(t) > LARGO_MAX_NUMERO or not _RE_NUMERO.match(t):
        return None
    return t


def desenrollar(grados):
    """Para el gráfico: junta los saltos de 180 a -180 [170, -170] -> [170, 190].
    En ÓRBITA y GIRO el robot ya manda lo girado EN TOTAL (pasa de 180), y
    eso no se toca: los saltos de más de 180 grados en 10 ms no existen.
    Esto queda por las dudas, si alguna vez llega enrollado."""
    salida = []
    previo = None
    suma = 0.0
    for g in grados:
        if previo is None:
            suma = g
        else:
            d = g - previo
            while d > 180:
                d -= 360
            while d < -180:
                d += 360
            suma += d
        salida.append(suma)
        previo = g
    return salida


def escala_grados(maximo):
    """Hasta cuántos grados mostrar en el gráfico (un número redondo)."""
    for tope in (5, 10, 15, 20, 30, 45, 60, 90, 120, 180, 270, 360, 540, 720, 1080, 1440, 2160, 2880, 3600):
        if tope >= maximo * 1.1:
            return tope
    return int(math.ceil(maximo / 360.0)) * 360


def paso_grilla(tope):
    for paso in (1, 2, 5, 10, 15, 30, 45, 90, 180, 360, 720):
        if tope / paso <= 5:
            return paso
    return tope


def paso_tiempo(tmax):
    for paso in (50, 100, 200, 250, 500, 1000, 2000, 5000, 10000):
        if tmax / paso <= 8:
            return paso
    return tmax


def puede_mover(conectado, es_delantero, programa_ok, modo, pendiente=False):
    """¿Se puede pedir un movimiento (o cambiar un número)? -> (sí/no, por qué no)."""
    if not conectado:
        return False, "No hay robot conectado."
    if not es_delantero:
        return False, "Este robot NO es el delantero: la ventana no le manda nada."
    if not programa_ok:
        return False, "El robot no tiene cargado el programa del tablero: usá TABLERO.bat."
    if modo != "ESPERA":
        return False, "Esperá a que termine el movimiento."
    if pendiente:
        return False, "Esperando que el robot conteste..."
    return True, ""


def explicar_informe(inf):
    """Pasa el #R a frases en criollo: [(frase, estilo)], estilo = '',
    'malo' o 'bueno'."""
    mov = inf.get("mov") or ""
    tipo = tipo_de(mov)

    def num(clave):
        v = inf.get(clave)
        return 0 if v is None else v

    frases = []
    if tipo in ("patada", "derecho"):
        que = "el golpe" if tipo == "patada" else "el movimiento"
        frases.append(("Lo más torcido durante %s: %s, a los %d ms." % (que, lado(num("pico")), num("picoms")), ""))
        if tipo == "patada":
            frases.append(("En los primeros 200 ms (lo que ve la pelota): %s." % lado(num("pico200")), ""))
        frases.append(("Al terminar %s: %s." % (que, lado(num("fin"))), ""))
        if tipo == "patada":
            frases.append(("Después del retroceso quedó: %s." % lado(num("fin2")), ""))
        cruces = int(num("cruces"))
        frases.append(("Cruzó de un lado al otro %d %s (4 o más = zigzag)." % (
            cruces, "vez" if cruces == 1 else "veces"), "malo" if cruces >= 4 else ""))
        if mov.startswith("ESCAPE") and max(abs(num("pico")), abs(num("fin"))) > 10:
            frases.append(("El escape GIRÓ más de 10 grados: tendría que alejarse de la línea sin girar.", "malo"))
    else:
        giro = abs(num("giro"))
        frases.append(("Giró en total %d grados (%.1f vueltas)." % (round(giro), giro / 360.0),
                       "malo" if giro < 20 else ""))
        gps = abs(num("gps"))
        if gps > 0:
            frases.append(("Velocidad: %d grados por segundo (una vuelta cada %.1f s)." % (round(gps), 360.0 / gps), ""))
        else:
            frases.append(("Velocidad: 0 grados por segundo.", "malo"))
        plantado = num("plantado")
        frases.append(("El rato más largo casi sin girar: %d ms (más de 400 = se planta)." % plantado,
                       "malo" if plantado > 400 else ""))
    if tipo == "patada":
        # En la PATADA el robot manda lo que duró solo el GOLPE (sin el
        # retroceso), para compararlo con MS_PATADA.
        frases.append(("El golpe duró %d ms. Tendría que durar lo que dice MS_PATADA: "
                       "si duró mucho más, el programa se clavó." % num("duro"), ""))
    else:
        frases.append(("Duró %d ms." % num("duro"), ""))
    hueco = num("hueco")
    if hueco > 50:
        frases.append(("El programa se CLAVÓ %d ms (a los %d ms): en ese rato el robot no corrige nada." % (
            hueco, num("huecoms")), "malo"))
    else:
        frases.append(("El programa no se clavó (la vuelta más larga: %d ms)." % hueco, "bueno"))
    return frases


# ---------- las notas, con los criterios del contrato (los usa el robot de mentira) ----------

def contar_cruces(grados, banda=2.0):
    """Un cruce es pasar de más de +2 grados a menos de -2, o al revés."""
    lado_actual = 0
    cruces = 0
    for g in grados:
        nuevo = 1 if g > banda else (-1 if g < -banda else 0)
        if nuevo != 0:
            if lado_actual != 0 and nuevo != lado_actual:
                cruces += 1
            lado_actual = nuevo
    return cruces


def nota_derecho(pico, cruces, fin):
    if abs(pico) <= 4 and cruces <= 2 and abs(fin) <= 3:
        return "PROLIJO"
    if cruces >= 4:
        return "ZIGZAGUEA"
    if abs(pico) >= 10:
        return "SE_TUERCE"
    return "CASI"


def nota_giro(giro_total, plantado_ms):
    if abs(giro_total) < 20:
        return "NO_ARRANCA"
    if plantado_ms > 400:
        return "SE_PLANTA"
    return "GIRA_BIEN"


# =====================================================================
#  4. EL CABLE: un hilo que lee el USB y otro que escribe
# =====================================================================

class Conexion:
    """Habla con el robot de verdad. Deja en la cola:
        ("buscando",)                     no hay ningún robot enchufado
        ("otro", puerto, serie)           hay una Teensy, pero NO es el delantero:
                                          su puerto NO se abre (sigue buscando)
        ("ocupado", puerto, error)        está, pero otro programa tiene el puerto
        ("conectado", puerto, serie)
        ("linea", texto)                  un renglón que mandó el robot
        ("desconectado", puerto)
    y se reconecta sola si se desenchufa el cable."""

    def __init__(self, cola, puerto_forzado=None):
        self.cola = cola
        self.puerto_forzado = puerto_forzado
        self._sp = None
        self._salida = queue.Queue()
        self._seguir = True

    def arrancar(self):
        for tarea, nombre in ((self._leer, "lector-usb"), (self._escribir, "escritor-usb")):
            threading.Thread(target=tarea, name=nombre, daemon=True).start()

    def enviar(self, texto):
        """Deja un comando para mandar. Lo manda el hilo escritor, así la
        ventana nunca se queda esperando al cable."""
        if self._sp is None:
            return False
        self._salida.put(texto)
        return True

    def enviar_ya(self, texto):
        """Manda en el momento (solo al cerrar la ventana)."""
        sp = self._sp
        if sp is None:
            return False
        try:
            sp.write((texto + "\n").encode("ascii", "replace"))
            sp.flush()
            return True
        except Exception:
            return False

    def cerrar(self):
        self._seguir = False

    def _vaciar_salida(self):
        # Lo que se pidió antes de desenchufar NO se manda después: un "!1"
        # viejo haría arrancar el robot cuando nadie lo espera.
        while True:
            try:
                self._salida.get_nowait()
            except queue.Empty:
                return

    def _escribir(self):
        while self._seguir:
            try:
                texto = self._salida.get(timeout=0.5)
            except queue.Empty:
                continue
            sp = self._sp
            if sp is None:
                continue
            try:
                sp.write((texto + "\n").encode("ascii", "replace"))
            except Exception:
                pass                 # si se desenchufó, el lector se entera y avisa

    def _elegir_puerto(self):
        """-> (abrir, otro). abrir = (puerto, serie) del delantero, o None.
        otro = (puerto, serie) de una Teensy que NO es el delantero, o None.
        A esa otra NO se le abre el puerto: si la ventana se lo quedara, el
        otro equipo (por ejemplo el del arquero) no podría usar su monitor
        serie ni cargarle un programa. El número de serie ya viene en la
        lista de puertos de Windows, sin abrir nada."""
        try:
            puertos = list(list_ports.comports())
        except Exception:
            return None, None
        if self.puerto_forzado:          # --puerto: lo pidieron a mano, se abre ese
            for p in puertos:
                if p.device.upper() == self.puerto_forzado.upper():
                    return (p.device, (p.serial_number or "")), None
            return None, None
        teensys = [(p.device, p.serial_number or "") for p in puertos
                   if p.vid == VID_TEENSY and p.pid == PID_SERIE]
        for puerto, serie in teensys:
            if serie == SERIE_DELANTERO:
                return (puerto, serie), None
        return None, (teensys[0] if teensys else None)

    def _leer(self):
        avisado = None
        while self._seguir:
            elegido, otro = self._elegir_puerto()
            if elegido is None:
                if otro is not None:
                    aviso = "otro %s %s" % otro
                    if avisado != aviso:
                        self.cola.put(("otro", otro[0], otro[1]))
                        avisado = aviso
                elif avisado != "buscando":
                    self.cola.put(("buscando",))
                    avisado = "buscando"
                time.sleep(0.5)
                continue
            puerto, serie = elegido
            try:
                sp = serial.Serial(puerto, BAUDIOS, timeout=0.05, write_timeout=1)
            except Exception as e:
                # Recién enchufado, Windows a veces no lo suelta enseguida.
                # Si sigue así, es otro monitor serie abierto.
                if avisado != "ocupado " + puerto:
                    self.cola.put(("ocupado", puerto, str(e)))
                    avisado = "ocupado " + puerto
                time.sleep(1.0)
                continue
            avisado = None
            self._vaciar_salida()
            self._sp = sp
            self.cola.put(("conectado", puerto, serie))
            resto = b""
            try:
                while self._seguir:
                    datos = sp.read(max(1, sp.in_waiting))
                    if not datos:
                        continue
                    resto += datos
                    if b"\n" in resto:
                        *renglones, resto = resto.split(b"\n")
                        for crudo in renglones:
                            self.cola.put(("linea", crudo.decode("utf-8", "replace").rstrip("\r")))
                    if len(resto) > 4000:          # basura sin fin de línea
                        self.cola.put(("linea", resto.decode("utf-8", "replace")))
                        resto = b""
            except Exception:
                pass                               # se desenchufó o se reinició
            self._sp = None
            self._vaciar_salida()
            try:
                sp.close()
            except Exception:
                pass
            if self._seguir:
                self.cola.put(("desconectado", puerto))
            time.sleep(0.5)


# =====================================================================
#  5. EL ROBOT DE MENTIRA (--demo): para ver la ventana sin robot
# =====================================================================
#  Contesta como el robot de verdad, con el mismo idioma. TODO lo que
#  "mide" es inventado: sirve para practicar con la ventana, no para
#  ajustar nada. Con TRIM_PATADA = 7 la patada de mentira sale derecha.

VALORES_DE_MENTIRA = {
    # Los mismos con los que arranca tablero.ino (PANEL DE CONTROL del principal, 27/10)
    "VEL_PATADA": "150", "MS_PATADA": "500", "TRIM_PATADA": "5", "KP_PATADA": "4.0",
    "KD_PATADA": "0.6", "VEL_RETROCESO": "110", "MS_RETROCESO": "700", "LAZO": "1",
    "TRASERA_TRABADA": "1", "SIN_ESPERA": "0", "VEL_AVANCE": "95", "MS_PRUEBA_AVANCE": "800",
    "VEL_ESCAPE_FUERTE": "135", "MS_RETROCESO_LINEA": "210", "VEL_ORB_IMPULSO": "120",
    "MS_ORB_IMPULSO": "300", "VEL_ORB_TRASERA": "67", "VEL_ORB_FRENTE": "30",
    "MS_PRUEBA_ORBITA": "3000", "SENTIDO_ORBITA": "0", "VEL_GIRO_CONTINUO": "40",
    "RAMPA_GIRO_PASO": "10", "RAMPA_GIRO_MS": "10", "MS_PRUEBA_GIRO": "2000",
}
# (mínimo, máximo, con decimales, cómo se declara en el principal; "" = solo de la prueba)
RANGOS_DE_MENTIRA = {
    "VEL_PATADA": (60, 255, False, "const int"), "MS_PATADA": (100, 1000, False, "const int"),
    "TRIM_PATADA": (-60, 60, False, "const int"), "KP_PATADA": (0, 20, True, "const float"),
    "KD_PATADA": (0, 5, True, "const float"), "VEL_RETROCESO": (0, 255, False, "const int"),
    "MS_RETROCESO": (0, 1500, False, "const int"), "LAZO": (0, 1, False, ""),
    "TRASERA_TRABADA": (0, 1, False, ""), "SIN_ESPERA": (0, 1, False, ""),
    "VEL_AVANCE": (0, 255, False, "const int"), "MS_PRUEBA_AVANCE": (100, 3000, False, ""),
    "VEL_ESCAPE_FUERTE": (0, 255, False, "const int"),
    # hasta 300 = MS_CIEGO_LINEA del principal (nunca menos que MS_RETROCESO_LINEA)
    "MS_RETROCESO_LINEA": (50, 300, False, "const unsigned long"),
    "VEL_ORB_IMPULSO": (0, 255, False, "const int"), "MS_ORB_IMPULSO": (0, 2000, False, "const int"),
    "VEL_ORB_TRASERA": (0, 255, False, "const int"), "VEL_ORB_FRENTE": (0, 69, False, "const int"),
    # órbita y giro hasta 3000: el robot guarda 300 muestras, una cada 10 ms
    "MS_PRUEBA_ORBITA": (500, 3000, False, ""), "SENTIDO_ORBITA": (0, 1, False, ""),
    "VEL_GIRO_CONTINUO": (0, 255, False, "const int"), "RAMPA_GIRO_PASO": (1, 255, False, "const int"),
    "RAMPA_GIRO_MS": (1, 200, False, "const unsigned long"), "MS_PRUEBA_GIRO": (300, 3000, False, ""),
}


def _valor_de(valores, nombre):
    try:
        return float(valores.get(nombre, "0"))
    except ValueError:
        return 0.0


def duracion_de_mentira(n, valores):
    V = lambda nombre: _valor_de(valores, nombre)
    ms = {1: V("MS_PATADA") + V("MS_RETROCESO"), 2: V("MS_PRUEBA_AVANCE"),
          3: V("MS_RETROCESO_LINEA"), 4: V("MS_RETROCESO_LINEA"), 5: V("MS_RETROCESO_LINEA"),
          6: V("MS_PRUEBA_ORBITA"), 7: V("MS_PRUEBA_GIRO")}[n]
    return max(10, int(ms))


def _enrollar(g):
    while g > 180:
        g -= 360
    while g <= -180:
        g += 360
    return g


def _plantado(puntos, desde_ms):
    """El rato más largo (ms) girando menos de 3 grados, después de desde_ms."""
    mejor = 0
    inicio = None
    for t, g, _ in puntos:
        if t <= desde_ms:
            continue
        if inicio is None or abs(g - inicio[1]) >= 3.0:
            inicio = (t, g)
        mejor = max(mejor, t - inicio[0])
    return int(mejor)


def _consejos_de_mentira(n, inf, nota, valores):
    V = lambda nombre: valores.get(nombre, "?")
    c = []
    if nota == "NO_VALE":
        return ["Esta no vale: lo frenaron antes de terminar. Probar de nuevo."]
    if n == 1:
        if inf["cruces"] >= 4:
            c.append("Zigzaguea (%d cruces): bajar KP_PATADA (esta en %s) o subir KD_PATADA." % (inf["cruces"], V("KP_PATADA")))
        elif inf["pico"] <= -4:
            c.append("Se fue a la DERECHA: subir TRIM_PATADA (esta en %s)." % V("TRIM_PATADA"))
        elif inf["pico"] >= 4:
            c.append("Se fue a la IZQUIERDA: bajar TRIM_PATADA (esta en %s)." % V("TRIM_PATADA"))
        if abs(inf["pico"]) >= 6 and abs(inf["fin"]) <= 2:
            c.append("Pico grande y final chico: subir KD_PATADA (esta en %s)." % V("KD_PATADA"))
        if nota == "PROLIJO":
            c.append("Va derecho. Repetirlo 3 veces: si sale igual, Copiar al panel.")
    elif n == 2:
        if abs(inf["fin"]) > 3 or abs(inf["pico"]) > 3:
            c.append("El avance se torcio. El avance del partido no corrige el rumbo: anotarlo en la bitacora.")
        else:
            c.append("El avance va derecho.")
    elif n in (3, 4, 5):
        if max(abs(inf["pico"]), abs(inf["fin"])) > 10:
            c.append("El escape del sensor %d giro mas de 10 grados: tendria que trasladarse sin girar. Avisarlo." % (n - 2))
        else:
            c.append("El escape del sensor %d sale sin girar casi nada." % (n - 2))
    elif n == 6:
        if nota == "NO_ARRANCA":
            c.append("No arranco: subir MS_ORB_IMPULSO (esta en %s)." % V("MS_ORB_IMPULSO"))
        elif nota == "SE_PLANTA":
            c.append("Se planto %d ms: subir VEL_ORB_TRASERA (esta en %s)." % (inf["plantado"], V("VEL_ORB_TRASERA")))
        else:
            c.append("Rodea bien, a %.0f grados por segundo." % inf["gps"])
    elif n == 7:
        if nota == "NO_ARRANCA":
            c.append("No arranco: subir VEL_GIRO_CONTINUO (esta en %s)." % V("VEL_GIRO_CONTINUO"))
        else:
            c.append("Gira bien, a %.0f grados por segundo." % inf["gps"])
    if inf["hueco"] > 50:
        c.append("El programa se clavo %d ms: probar SIN_ESPERA = 1." % inf["hueco"])
    return c


def demo_resultado(n, valores, rng, cortado_ms=None):
    """El robot de mentira 'mide' el movimiento n. Devuelve los renglones
    #M, #R y #CONSEJO con el formato exacto del contrato.
    cortado_ms = a los cuántos ms lo frenaron (None = terminó solo)."""
    V = lambda nombre: _valor_de(valores, nombre)
    mov = MOV_POR_NUMERO[n]
    dur = duracion_de_mentira(n, valores)
    puntos = []                       # (t, grados sin enrollar, fase)
    arranque = 0                      # para "plantado": después del golpe de arranque
    pasos = range(10, dur + 1, 10)
    if n == 1:
        golpe = V("MS_PATADA")
        sesgo = (V("TRIM_PATADA") - 7) * 1.8                 # con TRIM 7 va derecho (de mentira)
        if V("LAZO") >= 1:
            sesgo /= (1 + V("KP_PATADA") / 3.0)
        osc = max(0.4, 0.8 + max(0.0, V("KP_PATADA") - 4) * 1.6 - V("KD_PATADA") * 0.8)
        for t in pasos:
            fase = "g" if t <= golpe else "r"
            g = sesgo * (1 - math.exp(-t / 150.0)) + osc * 2.5 * math.sin(t / 40.0) * math.exp(-t / 350.0)
            if fase == "r":
                g += sesgo * (t - golpe) / 600.0
            puntos.append((t, g + rng.uniform(-0.3, 0.3), fase))
    elif n == 2:
        for t in pasos:
            g = -7.0 * (t / float(dur)) ** 1.3 * V("VEL_AVANCE") / 95.0
            puntos.append((t, g + rng.uniform(-0.3, 0.3), "m"))
    elif n in (3, 4, 5):
        total = {3: 4.0, 4: -6.5, 5: 12.0}[n] * V("VEL_ESCAPE_FUERTE") / 135.0
        for t in pasos:
            puntos.append((t, total * (1 - math.exp(-t / 70.0)) + rng.uniform(-0.3, 0.3), "m"))
    elif n == 6:
        signo = 1 if V("SENTIDO_ORBITA") >= 1 else -1
        arranque = V("MS_ORB_IMPULSO")
        crucero = max(0.0, (V("VEL_ORB_TRASERA") - 45) * 5.0)    # grados por segundo
        acumulado = 0.0
        for t in pasos:
            if t <= arranque:
                vel = 140.0 if V("VEL_ORB_IMPULSO") >= 90 else 5.0
            elif V("VEL_ORB_TRASERA") < 60 and t > arranque + 300:
                vel = 1.0                                         # se planta
            else:
                vel = crucero
            acumulado += signo * vel * 0.01
            puntos.append((t, acumulado + rng.uniform(-0.05, 0.05), "m"))
    else:
        fuerza = V("VEL_GIRO_CONTINUO")
        crucero = 0.0 if fuerza < 38 else (fuerza - 30) * 14.0
        arranque = fuerza / max(1.0, V("RAMPA_GIRO_PASO")) * V("RAMPA_GIRO_MS")
        acumulado = 0.0
        for t in pasos:
            acumulado += crucero * min(1.0, t / max(arranque, 1.0)) * 0.01
            puntos.append((t, acumulado + rng.uniform(-0.05, 0.05), "m"))

    if cortado_ms is not None:
        puntos = [p for p in puntos if p[0] <= cortado_ms]
        dur = max(int(cortado_ms), 0)
    puntos = puntos[:300]                 # el robot guarda hasta 300

    inf = dict.fromkeys(CAMPOS_R, 0)
    # En la PATADA el robot manda lo que duró el GOLPE solo (no el retroceso).
    inf["duro"] = min(dur, int(V("MS_PATADA"))) if n == 1 else dur
    inf["hueco"], inf["huecoms"] = rng.randint(2, 9), rng.randint(0, max(dur, 1))
    if n == 1 and V("SIN_ESPERA") < 1 and dur > 60:
        # El chequeo del giroscopio de la librería clava el programa 200 ms
        # (bitácora 13/10). Cae en un momento cualquiera del golpe.
        inf["hueco"], inf["huecoms"] = 201, rng.randint(50, max(51, min(dur, int(V("MS_PATADA")))))
    if mov["tipo"] in ("patada", "derecho"):
        moviendo = [p for p in puntos if p[2] in ("g", "m")]
        if moviendo:
            pico = max(moviendo, key=lambda p: abs(p[1]))
            inf["pico"], inf["picoms"] = round(pico[1], 1), pico[0]
            primeros = [p for p in moviendo if p[0] <= 200]
            inf["pico200"] = round(max(primeros, key=lambda p: abs(p[1]))[1], 1) if primeros else 0
            inf["fin"] = round(moviendo[-1][1], 1)
            inf["cruces"] = contar_cruces([p[1] for p in moviendo])
        retroceso = [p for p in puntos if p[2] == "r"]
        if retroceso:
            inf["fin2"] = round(retroceso[-1][1], 1)
        nota = nota_derecho(inf["pico"], inf["cruces"], inf["fin"])
    else:
        total = abs(puntos[-1][1]) if puntos else 0.0
        inf["giro"] = round(total, 1)
        inf["gps"] = round(total * 1000.0 / max(dur, 1), 1)
        inf["plantado"] = _plantado(puntos, arranque)
        nota = nota_giro(total, inf["plantado"])
    if cortado_ms is not None:
        nota = "NO_VALE"

    # Como el robot de verdad: en ÓRBITA y GIRO, lo girado EN TOTAL (pasa de
    # 180); en la patada y los derechos, el desvío entre -180 y 180.
    renglones = ["#M %d %.1f %s" % (t, (g if mov["tipo"] == "giro" else _enrollar(g)), f)
                 for t, g, f in puntos]
    renglones.append(
        "#R n=%d mov=%s nota=%s pico=%.1f picoms=%d pico200=%.1f fin=%.1f fin2=%.1f cruces=%d "
        "giro=%.1f gps=%.1f duro=%d hueco=%d huecoms=%d plantado=%d" % (
            n, mov["nombre"], nota, inf["pico"], inf["picoms"], inf["pico200"], inf["fin"],
            inf["fin2"], inf["cruces"], inf["giro"], inf["gps"], inf["duro"], inf["hueco"],
            inf["huecoms"], inf["plantado"]))
    renglones += ["#CONSEJO " + c for c in _consejos_de_mentira(n, inf, nota, valores)]
    return renglones


class RobotDeMentira:
    """Se hace pasar por la Conexion: misma forma de usarlo."""

    def __init__(self, cola, semilla=2026):
        self.cola = cola
        self.valores = dict(VALORES_DE_MENTIRA)
        self._entrada = queue.Queue()
        self._seguir = True
        self._rng = random.Random(semilla)
        self._t0 = time.monotonic()
        self._pelota = None
        self._t_pelota = None

    def arrancar(self):
        threading.Thread(target=self._correr, name="robot-de-mentira", daemon=True).start()

    def enviar(self, texto):
        self._entrada.put(texto)
        return True

    def enviar_ya(self, texto):
        return self.enviar(texto)

    def cerrar(self):
        self._seguir = False

    def _decir(self, renglon):
        self.cola.put(("linea", renglon))

    def _correr(self):
        time.sleep(0.4)
        self.cola.put(("conectado", "DEMO", SERIE_DELANTERO))
        self._decir("TABLERO DEL DELANTERO  (ROBOT DE MENTIRA: todo lo que mide es inventado)")
        self._decir("#HOLA %s %s" % (PROGRAMA, FECHA_CONTRATO))
        proximo = 0.0
        while self._seguir:
            try:
                orden = self._entrada.get(timeout=0.02)
            except queue.Empty:
                orden = None
            if orden is not None:
                self.comando(orden)
            if time.monotonic() >= proximo:
                self._decir(self.linea_S())
                proximo = time.monotonic() + 0.1

    def linea_S(self):
        rng = self._rng
        t = int((time.monotonic() - self._t0) * 1000)
        # Línea: verde ~300; cada 7 s uno de los tres "pisa" blanco un rato.
        l = [300 + rng.randint(-12, 12), 312 + rng.randint(-12, 12), 325 + rng.randint(-12, 12)]
        if t % 7000 < 1200:
            l[(t // 7000) % 3] = 610 + rng.randint(-20, 20)
        rumbo = (90 + 25 * math.sin(t / 6000.0)) % 360
        # La pelota anda en un ocho. La cámara la ve solo dentro de su ángulo,
        # y cada 12 s se "esconde" 2.5 s (para ver cómo queda en gris).
        x = 75 + 45 * math.sin(t / 3100.0)
        y = 40 * math.sin(t / 1900.0)
        dentro = abs(math.degrees(math.atan2(y, x))) < ANGULO_CAMARA
        if dentro and t % 12000 > 2500:
            self._pelota = (int(round(x)), int(round(y)))
            self._t_pelota = t - rng.randint(0, 50)
        if self._pelota is None:
            p = "0,0,-1"
        else:
            p = "%d,%d,%d" % (self._pelota[0], self._pelota[1], max(0, t - self._t_pelota))
        am = "%d,%d,%d" % (160, -25 + rng.randint(-1, 1), rng.randint(0, 60))
        return ("#S t=%d l=%d,%d,%d u=424,434,440 r=%.1f g=5 p=%s am=%s az=0,0,-1 q=%d e=ESPERA"
                % (t, l[0], l[1], l[2], rumbo, p, am, 18 + rng.randint(-2, 2)))

    def linea_V(self):
        return "#V " + " ".join("%s=%s" % (k, v) for k, v in self.valores.items())

    def comando(self, orden):
        c = orden.strip()
        if not c:
            return
        if c.startswith("="):
            self._cambiar(c[1:])
        elif c.startswith("!"):
            try:
                n = int(c[1:])
            except ValueError:
                n = 0
            if n in MOV_POR_NUMERO:
                self._mover(n)
            else:
                self._decir("#ERROR no hay movimiento %s: son del !1 al !7" % c[1:])
        elif c == "h":
            self._decir("#HOLA %s %s" % (PROGRAMA, FECHA_CONTRATO))
        elif c == "?":
            self._decir(self.linea_V())
            self._decir("------------------- TABLERO DEL DELANTERO (de mentira) -------------------")
            self._decir("!1 a !7 = un movimiento   x = parar   =NOMBRE valor = cambiar un numero")
            self._decir("v = copiar al principal   0 = volver a los del principal   h = saludo")
        elif c == "x":
            self._decir("PARADO (frena 0,4 s y suelta las ruedas)")
        elif c == "v":
            for nombre, valor in self.valores.items():
                declarado = RANGOS_DE_MENTIRA[nombre][3]
                if declarado and valor != VALORES_DE_MENTIRA[nombre]:
                    self._decir("#PANEL %s %s = %s;" % (declarado, nombre, valor))
            self._decir("#FINPANEL")
        elif c == "0":
            self.valores = dict(VALORES_DE_MENTIRA)
            self._decir("Volvieron los numeros del PANEL DE CONTROL del principal (27/10).")
            self._decir(self.linea_V())
        else:
            self._decir("#ERROR no entiendo '%s'. ? = ayuda" % c)

    def _cambiar(self, texto):
        nombre, _, valor = texto.strip().partition(" ")
        valor = valor.strip()
        if nombre not in self.valores:
            self._decir("#ERROR no existe el numero %s. Con ? salen todos los nombres" % nombre)
            return
        if "," in valor:
            self._decir("#ERROR %s: usar punto decimal (4.5, no 4,5)" % nombre)
            return
        try:
            v = _numero(valor)
        except ValueError:
            self._decir("#ERROR %s: no es un numero: %s" % (nombre, valor))
            return
        minimo, maximo, decimales, _ = RANGOS_DE_MENTIRA[nombre]
        if not decimales and v != int(v):
            self._decir("#ERROR %s va sin decimales" % nombre)
            return
        if not minimo <= v <= maximo:
            self._decir("#ERROR %s fuera de rango: va de %s a %s" % (nombre, minimo, maximo))
            return
        texto_valor = ("%.2f" % v).rstrip("0") if decimales else str(int(v))
        if texto_valor.endswith("."):
            texto_valor += "0"
        self.valores[nombre] = texto_valor
        self._decir("#OK %s=%s" % (nombre, texto_valor))
        self._decir(self.linea_V())

    def _esperar(self, segundos):
        """Espera. Si llega cualquier cosa (menos un Enter suelto), lo frenan:
        devuelve a los cuántos ms. Si no, None."""
        t0 = time.monotonic()
        fin = t0 + segundos
        while self._seguir:
            falta = fin - time.monotonic()
            if falta <= 0:
                return None
            try:
                orden = self._entrada.get(timeout=min(falta, 0.02))
            except queue.Empty:
                continue
            if orden.strip() == "":
                continue
            return int((time.monotonic() - t0) * 1000)
        return 0

    def _mover(self, n):
        for segundos in ("1.5", "1.0", "0.5"):
            self._decir("#CUENTA " + segundos)
            if self._esperar(0.5) is not None:
                self._decir("#FRENO tecla")        # en la cuenta: no se movió, no hay informe
                return
        self._decir("#INICIO %d %s" % (n, MOV_POR_NUMERO[n]["nombre"]))
        cortado = self._esperar(duracion_de_mentira(n, self.valores) / 1000.0)
        if cortado is not None:
            self._decir("#FRENO tecla")
        for renglon in demo_resultado(n, self.valores, self._rng, cortado):
            self._decir(renglon)


# =====================================================================
#  6. LA VENTANA
# =====================================================================

FONDO = "#f3f4f6"
LETRA = "Segoe UI"
F_CHICO = (LETRA, 10)
F_NORMAL = (LETRA, 12)
F_NORMAL_B = (LETRA, 12, "bold")
F_MEDIO = (LETRA, 14)
F_MEDIO_B = (LETRA, 14, "bold")
F_GRANDE = (LETRA, 18, "bold")
F_ENORME = (LETRA, 32, "bold")
F_CONSOLA = ("Consolas", 11)
VERDE = "#2e7d32"
ROJO = "#c62828"
NARANJA = "#e65100"
GRIS = "#757575"
AZUL = "#1565c0"
COLOR_FASE = {"g": "#e65100", "r": "#1565c0", "m": "#6a1b9a"}
# Para rodear o soltar la pelota el robot NO usa la distancia: usa X, lo
# que está para ADELANTE (XP_ORBITA = 32 y XP_SUELTA = 55 en el principal).
# Por eso son RAYAS de lado a lado y no círculos: una pelota a 40 cm y 45
# grados tiene X = 28, y el robot ya la está rodeando.
RAYAS_X = (                     # (X en cm, qué es, color)
    (32, "X = 32 cm: ahí empieza a rodearla", "#ffcc80"),
    (55, "X = 55 cm: si se aleja más, la suelta y vuelve a buscarla", "#fff59d"),
)
CIRCULOS = (                    # (cm, qué es, color): solo para tener una idea de la distancia
    (100, "1 m", "#c8e6c9"),
    (150, "1.5 m", "#c8e6c9"),
)
PANEL_COPIADO = "Ya está copiado (y abajo se ve)."
PANEL_NO_COPIADO = "No pude copiarlo solo: seleccionalo abajo y Ctrl+C."
INSTRUCCIONES_PANEL = (
    "NO lo pegues todo junto: quedaría repetido y no compila. Para cada línea: buscá el "
    "nombre con Ctrl+F en el PANEL DE CONTROL y cambiá SOLO el número por el de acá.\n"
    "1. Abrí funciona\\delantero\\delantero.ino y buscá el PANEL DE CONTROL (arriba de todo).\n"
    "2. Buscá cada nombre (Ctrl+F) y cambiá SOLO el número de esa línea (el comentario queda).\n"
    "3. UN CAMBIO POR VEZ, y anotalo en la bitácora: qué, de cuánto a cuánto y qué se vio.\n"
    "4. CERRÁ EL TABLERO (la ventana grande, no solo este cartel): si queda abierto, el "
    "cargador no puede hablar con el robot. Recién ahí cargá el de partido con "
    "CARGAR-ROBOT.bat y leé el cartel del robot.\n"
    "(Si el robot dejó algún aviso más, está en la pestaña CONSOLA.)"
)


def guardar_falla(texto):
    """Anota una falla de la ventana en tablero-error.txt (al lado de este
    archivo), para mostrársela al profe. Con pythonw no hay consola."""
    try:
        with open(ARCHIVO_FALLAS, "a", encoding="utf-8") as f:
            f.write("==== %s ====\n%s\n" % (time.strftime("%Y-%m-%d %H:%M:%S"), texto))
    except Exception:
        pass


class Ventana:
    def __init__(self, raiz, conexion, cola, demo=False, estricto=False):
        self.raiz = raiz
        self.conexion = conexion
        self.cola = cola
        self.demo = demo
        self.estricto = estricto        # en la autoprueba: una falla corta todo
        self.estado = Estado()
        self.conectado = False
        self.situacion = "buscando"     # buscando / otro / ocupado / conectado
        self.puerto = None
        self.serie = None
        self.es_delantero = False
        # ¿El robot tiene el programa del tablero? Se sabe por lo que MANDA
        # (#S, o el #HOLA del tablero), sin preguntarle nada: al conectar la
        # ventana no manda nada, porque otras pruebas del delantero toman
        # cualquier tecla como una orden (grabar-linea vuelca su grabación,
        # identificar-sensores se saltea pasos).
        self.es_tablero = False
        self.pidio_numeros = False      # ya mandó "?" en esta conexión
        self.t_pide_hola = None         # cuándo mandó "h" por última vez
        self.ultimo_aviso = None        # (texto, color) del último aviso (lo mira la autoprueba)
        self.t_conexion = None
        self.t_ultima_linea = None
        self.t_pedido = None            # cuándo se mandó "!n" (para no mandarlo dos veces)
        self.t_pedido_panel = None      # cuándo se mandó "v"
        self.aviso_programa = False     # ya pasó el rato sin #HOLA
        self.elegido = 1
        self.filas = {}
        self.botones_mov = {}
        self.lineas_consola = 0
        self.pendientes = set()
        self._armar_estilos()
        self._armar()
        self._elegir(1)
        self._refrescar({"conexion", "valores", "sensores", "modo", "resultado"})
        self.raiz.protocol("WM_DELETE_WINDOW", self._cerrar)
        self.raiz.bind("<Escape>", lambda e: self._parar())
        self.raiz.report_callback_exception = self._falla_tk
        self.raiz.after(50, self._revisar_cola)
        self.raiz.after(250, self._latido)

    # ------------------------------------------------------------ armado

    def _armar_estilos(self):
        estilo = ttk.Style(self.raiz)
        if "clam" in estilo.theme_names():
            estilo.theme_use("clam")    # "clam" deja pintar los botones de colores
        estilo.configure(".", background=FONDO, font=F_NORMAL)
        estilo.configure("TFrame", background=FONDO)
        estilo.configure("TNotebook", background=FONDO)
        estilo.configure("TNotebook.Tab", font=(LETRA, 13, "bold"), padding=(20, 8))
        estilo.map("TNotebook.Tab", background=[("selected", "white")])
        estilo.configure("TButton", font=F_NORMAL, padding=(10, 6))
        estilo.configure("TCheckbutton", background=FONDO, font=F_NORMAL)
        # Botones de los movimientos bajitos, para que la columna entre en
        # una notebook de 1366x768.
        estilo.configure("Mov.TButton", font=F_MEDIO_B, padding=(12, 5), anchor="w")
        estilo.configure("Elegido.TButton", font=F_MEDIO_B, padding=(12, 5), anchor="w",
                         background="#90caf9")
        estilo.map("Elegido.TButton", background=[("disabled", "#cfd8dc"), ("active", "#64b5f6")])
        estilo.configure("Probar.TButton", font=(LETRA, 17, "bold"), padding=(18, 10),
                         background=VERDE, foreground="white")
        estilo.map("Probar.TButton", background=[("disabled", "#a5d6a7"), ("active", "#388e3c")],
                   foreground=[("disabled", "#f1f8e9")])
        estilo.configure("Paso.TButton", font=(LETRA, 14, "bold"), padding=(4, 2))

    def _armar(self):
        r = self.raiz
        r.title("Tablero del DELANTERO" + ("   (ROBOT DE MENTIRA)" if self.demo else ""))
        r.configure(bg=FONDO)
        r.minsize(1000, 640)
        # Siempre maximizada: así nunca queda más alta que la pantalla (con
        # la escala de Windows al 125%, una de 1920x1080 es como 1536x864)
        # ni tapada por la barra de tareas.
        try:
            r.state("zoomed")
        except Exception:
            try:
                r.geometry("1280x720")  # si el sistema no sabe maximizar
            except Exception:
                pass

        # --- arriba: con quién está hablando, y el botón PARAR ---
        arriba = tk.Frame(r, bg=FONDO)
        arriba.pack(side="top", fill="x", padx=10, pady=(8, 2))
        self.btn_parar = tk.Button(
            arriba, text="■  PARAR   (Esc)", font=(LETRA, 20, "bold"), bg=ROJO, fg="white",
            activebackground="#8e0000", activeforeground="white", relief="raised", bd=4,
            padx=28, pady=4, cursor="hand2", command=self._parar)
        self.btn_parar.pack(side="right", padx=(10, 0))
        izquierda = tk.Frame(arriba, bg=FONDO)
        izquierda.pack(side="left", fill="x", expand=True)
        self.lbl_conexion = tk.Label(izquierda, text="", font=F_GRANDE, anchor="w", padx=12, pady=4)
        self.lbl_conexion.pack(side="top", fill="x")
        self.lbl_programa = tk.Label(izquierda, text="", font=F_NORMAL_B, anchor="w", justify="left",
                                     bg=FONDO, padx=12, wraplength=900)
        self.lbl_programa.pack(side="top", fill="x")

        # --- abajo: el último aviso ---
        self.lbl_aviso = tk.Label(r, text="", font=F_MEDIO, anchor="w", bg="#e0e0e0", padx=12, pady=4)
        self.lbl_aviso.pack(side="bottom", fill="x")

        # --- las pestañas ---
        self.cuaderno = ttk.Notebook(r)
        self.cuaderno.pack(side="top", fill="both", expand=True, padx=8, pady=4)
        self.tab_pelota = ttk.Frame(self.cuaderno)
        self.tab_sensores = ttk.Frame(self.cuaderno)
        self.tab_mov = ttk.Frame(self.cuaderno)
        self.tab_consola = ttk.Frame(self.cuaderno)
        self.cuaderno.add(self.tab_pelota, text="  PELOTA  ")
        self.cuaderno.add(self.tab_sensores, text="  SENSORES  ")
        self.cuaderno.add(self.tab_mov, text="  MOVIMIENTOS  ")
        self.cuaderno.add(self.tab_consola, text="  CONSOLA  ")
        self._armar_pelota()
        self._armar_sensores()
        self._armar_movimientos()
        self._armar_consola()
        self.cuaderno.select(self.tab_mov)
        self.cuaderno.bind("<<NotebookTabChanged>>", lambda e: self._refrescar({"sensores", "resultado"}))

    def _etiqueta(self, padre, **kw):
        kw.setdefault("bg", FONDO)
        kw.setdefault("anchor", "w")
        kw.setdefault("justify", "left")
        return tk.Label(padre, **kw)

    def _armar_pelota(self):
        t = self.tab_pelota
        self.cv_pelota = tk.Canvas(t, bg="#2e7d32", highlightthickness=0)
        self.cv_pelota.pack(side="left", fill="both", expand=True, padx=(4, 8), pady=4)
        self.cv_pelota.bind("<Configure>", lambda e: self._dibujar_pelota())
        der = tk.Frame(t, bg=FONDO, width=410)
        der.pack(side="right", fill="y", padx=4, pady=4)
        der.pack_propagate(False)
        self.lbl_p_frase = self._etiqueta(der, font=(LETRA, 20, "bold"), wraplength=395)
        self.lbl_p_frase.pack(fill="x", pady=(10, 4))
        self.lbl_p_detalle = self._etiqueta(der, font=F_NORMAL, fg="#455a64", wraplength=395)
        self.lbl_p_detalle.pack(fill="x")
        tk.Frame(der, height=2, bg="#bdbdbd").pack(fill="x", pady=12)
        self.lbl_amarillo = self._etiqueta(der, font=F_MEDIO_B, wraplength=395)
        self.lbl_amarillo.pack(fill="x", pady=2)
        self.lbl_azul = self._etiqueta(der, font=F_MEDIO_B, wraplength=395)
        self.lbl_azul.pack(fill="x", pady=2)
        tk.Frame(der, height=2, bg="#bdbdbd").pack(fill="x", pady=12)
        self.lbl_p_camara = self._etiqueta(der, font=F_MEDIO_B, wraplength=395)
        self.lbl_p_camara.pack(fill="x")
        self._etiqueta(
            der, font=F_CHICO, fg="#616161", wraplength=395,
            text="Visto desde arriba: el robot mira para ARRIBA. Lo que está a la izquierda del "
                 "robot se ve a la izquierda de la pantalla.\n\n"
                 "Naranja = la pelota. Gris = la última vez que se vio (hace más de 0,3 s).\n"
                 "Las dos líneas en V: lo que alcanza a ver la cámara (unos 34 grados para cada lado).\n"
                 "Las rayas de lado a lado: X = 32 cm para adelante = ahí empieza a rodearla; "
                 "X = 55 cm = si se aleja más, la suelta y vuelve a buscarla.\n"
                 "Círculos: 1 m y 1,5 m.\n\n"
                 "Acá no hay predicción: es lo que la cámara vio de verdad."
        ).pack(side="bottom", fill="x", pady=6)

    def _armar_sensores(self):
        t = self.tab_sensores
        t.columnconfigure(0, weight=3)
        t.columnconfigure(1, weight=2)
        t.columnconfigure(2, weight=2)
        t.rowconfigure(0, weight=1)

        f1 = tk.LabelFrame(t, text=" LÍNEA BLANCA ", font=F_MEDIO_B, bg=FONDO, padx=8, pady=8)
        f1.grid(row=0, column=0, sticky="nsew", padx=6, pady=6)
        self.cv_lineas = tk.Canvas(f1, bg="white", highlightthickness=0)
        self.cv_lineas.pack(fill="both", expand=True)
        self.cv_lineas.bind("<Configure>", lambda e: self._dibujar_lineas())
        self._etiqueta(
            f1, font=F_NORMAL, wraplength=480,
            text="Cada barra es lo que lee el sensor (0 a 1023). La raya roja es su umbral: "
                 "de ahí para arriba dice BLANCO. Para probar: pasá el robot por la línea con la mano."
        ).pack(fill="x", pady=(8, 0))

        f2 = tk.LabelFrame(t, text=" GIROSCOPIO ", font=F_MEDIO_B, bg=FONDO, padx=8, pady=8)
        f2.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        self.cv_brujula = tk.Canvas(f2, bg=FONDO, highlightthickness=0, height=280)
        self.cv_brujula.pack(fill="x")
        self.cv_brujula.bind("<Configure>", lambda e: self._dibujar_brujula())
        self.lbl_rumbo = tk.Label(f2, font=F_ENORME, bg=FONDO)
        self.lbl_rumbo.pack()
        self.lbl_fusion = tk.Label(f2, font=F_MEDIO_B, bg=FONDO, wraplength=320)
        self.lbl_fusion.pack(pady=4)
        self._etiqueta(
            f2, font=F_NORMAL, wraplength=320,
            text="Girá el robot con la mano: el número tiene que cambiar. "
                 "'fusión OK' = el giroscopio anda bien. Sin eso los movimientos no arrancan. "
                 "Si dice SIN GIROSCOPIO: " + AYUDA_SIN_GIRO
        ).pack(fill="x", pady=6)

        f3 = tk.LabelFrame(t, text=" CÁMARA ", font=F_MEDIO_B, bg=FONDO, padx=8, pady=8)
        f3.grid(row=0, column=2, sticky="nsew", padx=6, pady=6)
        self.lbl_q = self._etiqueta(f3, font=F_GRANDE, wraplength=330)
        self.lbl_q.pack(fill="x")
        self.lbl_q_txt = self._etiqueta(f3, font=F_NORMAL, wraplength=330, fg="#455a64")
        self.lbl_q_txt.pack(fill="x", pady=(2, 12))
        tabla = tk.Frame(f3, bg=FONDO)
        tabla.pack(fill="x")
        for col, titulo in enumerate(("", "X", "Y", "visto hace")):
            tk.Label(tabla, text=titulo, font=F_NORMAL_B, bg=FONDO).grid(row=0, column=col, padx=6, sticky="w")
        self.celdas_cam = {}
        for fila, (clave, nombre) in enumerate((("p", "pelota"), ("am", "arco amarillo"), ("az", "arco azul")), 1):
            tk.Label(tabla, text=nombre, font=F_NORMAL_B, bg=FONDO).grid(row=fila, column=0, padx=6, pady=3, sticky="w")
            celdas = []
            for col in (1, 2, 3):
                lbl = tk.Label(tabla, text="-", font=F_MEDIO, bg=FONDO)
                lbl.grid(row=fila, column=col, padx=6, sticky="w")
                celdas.append(lbl)
            self.celdas_cam[clave] = celdas
        self._etiqueta(
            f3, font=F_CHICO, fg="#616161", wraplength=330,
            text="X = cm para adelante. Y = cm al costado (POSITIVO = a la IZQUIERDA del robot). "
                 "Es la última posición BUENA de cada cosa."
        ).pack(fill="x", pady=10)

    def _armar_movimientos(self):
        t = self.tab_mov
        t.columnconfigure(0, weight=0)
        t.columnconfigure(1, weight=0)
        t.columnconfigure(2, weight=1)
        t.rowconfigure(0, weight=1)

        # --- izquierda: los 7 movimientos ---
        # (todo bien juntito: tiene que entrar en una pantalla de 1366x768)
        izq = tk.Frame(t, bg=FONDO)
        izq.grid(row=0, column=0, sticky="ns", padx=(6, 4), pady=6)
        self._etiqueta(izq, text="Elegí un movimiento:", font=F_MEDIO_B).pack(fill="x", pady=(0, 4))
        for m in MOVIMIENTOS:
            b = ttk.Button(izq, text=m["boton"], style="Mov.TButton",
                           command=lambda n=m["n"]: self._elegir(n))
            b.pack(fill="x", pady=2)
            self.botones_mov[m["n"]] = b
        tk.Frame(izq, bg=FONDO, height=10).pack()
        self.btn_copiar = ttk.Button(izq, text="Copiar al panel", command=self._copiar_panel)
        self.btn_copiar.pack(fill="x", pady=2)
        self._etiqueta(izq, font=F_CHICO, fg="#616161",
                       text="Qué números cambiar en el PANEL\nDE CONTROL del programa de partido."
                       ).pack(fill="x")
        self.btn_partido = ttk.Button(izq, text="Volver a los números\ndel partido",
                                      command=self._volver_partido)
        self.btn_partido.pack(fill="x", pady=(8, 2))

        # --- medio: qué hace, PROBAR, y sus números ---
        med = tk.Frame(t, bg=FONDO)
        med.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        self.lbl_mov_titulo = self._etiqueta(med, font=F_GRANDE)
        self.lbl_mov_titulo.pack(fill="x")
        self.lbl_mov_desc = self._etiqueta(med, font=F_NORMAL, wraplength=520)
        self.lbl_mov_desc.pack(fill="x")
        self.lbl_mov_lugar = self._etiqueta(med, font=F_NORMAL_B, fg="#bf360c", wraplength=520)
        self.lbl_mov_lugar.pack(fill="x", pady=(4, 8))
        fila = tk.Frame(med, bg=FONDO)
        fila.pack(fill="x", pady=(0, 8))
        self.btn_probar = ttk.Button(fila, text="▶  PROBAR", style="Probar.TButton", command=self._probar)
        self.btn_probar.pack(side="left")
        self.lbl_cuenta = self._etiqueta(fila, font=F_MEDIO_B, wraplength=250)
        self.lbl_cuenta.pack(side="left", padx=12, fill="x", expand=True)
        self._etiqueta(med, font=F_NORMAL_B,
                       text="Sus números (se cambian SIN recargar el robot):").pack(fill="x")
        caja = tk.Frame(med, bg=FONDO)
        caja.pack(fill="both", expand=True)
        self.cv_tabla = tk.Canvas(caja, bg=FONDO, highlightthickness=0, width=520)
        barra = ttk.Scrollbar(caja, orient="vertical", command=self.cv_tabla.yview)
        self.cv_tabla.configure(yscrollcommand=barra.set)
        barra.pack(side="right", fill="y")
        self.cv_tabla.pack(side="left", fill="both", expand=True)
        self.tabla = tk.Frame(self.cv_tabla, bg=FONDO)
        self._id_tabla = self.cv_tabla.create_window(0, 0, window=self.tabla, anchor="nw")
        self.tabla.bind("<Configure>",
                        lambda e: self.cv_tabla.configure(scrollregion=self.cv_tabla.bbox("all")))
        self.cv_tabla.bind("<Configure>",
                           lambda e: self.cv_tabla.itemconfigure(self._id_tabla, width=e.width))
        self.cv_tabla.bind("<Enter>", lambda e: self.cv_tabla.bind_all("<MouseWheel>", self._rueda))
        self.cv_tabla.bind("<Leave>", lambda e: self.cv_tabla.unbind_all("<MouseWheel>"))

        # --- derecha: el resultado ---
        der = tk.Frame(t, bg=FONDO)
        der.grid(row=0, column=2, sticky="nsew", padx=(6, 6), pady=6)
        self.lbl_res_titulo = self._etiqueta(der, font=F_MEDIO_B, text="RESULTADO")
        self.lbl_res_titulo.pack(fill="x")
        self.lbl_nota = tk.Label(der, text="-", font=F_ENORME, bg="#bdbdbd", fg="white")
        self.lbl_nota.pack(fill="x", pady=2)
        self.lbl_nota_txt = self._etiqueta(der, font=F_NORMAL, wraplength=560)
        self.lbl_nota_txt.pack(fill="x")
        der.bind("<Configure>", lambda e: self.lbl_nota_txt.configure(wraplength=max(200, e.width - 20)))
        self.cv_grafico = tk.Canvas(der, bg="white", height=240, highlightthickness=1,
                                    highlightbackground="#bdbdbd")
        self.cv_grafico.pack(fill="x", pady=6)
        self.cv_grafico.bind("<Configure>", lambda e: self._dibujar_grafico())
        marco = tk.Frame(der, bg=FONDO)
        marco.pack(fill="both", expand=True)
        self.txt_detalle = tk.Text(marco, font=F_NORMAL, wrap="word", height=8, bg="white",
                                   relief="flat", padx=8, pady=6)
        barra2 = ttk.Scrollbar(marco, orient="vertical", command=self.txt_detalle.yview)
        self.txt_detalle.configure(yscrollcommand=barra2.set)
        barra2.pack(side="right", fill="y")
        self.txt_detalle.pack(side="left", fill="both", expand=True)
        self.txt_detalle.tag_configure("titulo", font=F_NORMAL_B)
        self.txt_detalle.tag_configure("malo", foreground=ROJO, font=F_NORMAL_B)
        self.txt_detalle.tag_configure("bueno", foreground=VERDE)
        self.txt_detalle.tag_configure("consejo", foreground=AZUL, font=F_MEDIO_B)
        self.txt_detalle.configure(state="disabled")

    def _armar_consola(self):
        t = self.tab_consola
        arriba = tk.Frame(t, bg=FONDO)
        arriba.pack(fill="x", padx=6, pady=6)
        self.ver_s = tk.BooleanVar(value=False)
        ttk.Checkbutton(arriba, text="Mostrar también los #S (llegan 10 por segundo)",
                        variable=self.ver_s).pack(side="left")
        ttk.Button(arriba, text="Borrar", command=self._consola_borrar).pack(side="right")
        ttk.Button(arriba, text="Ayuda del robot  (?)",
                   command=lambda: self._mandar("?")).pack(side="right", padx=6)
        abajo = tk.Frame(t, bg=FONDO)
        abajo.pack(side="bottom", fill="x", padx=6, pady=(4, 6))
        self._etiqueta(
            t, font=F_CHICO, fg="#616161", wraplength=1200,
            text="Comandos:  ?  ayuda    h  saludo   =NOMBRE valor  cambia un número    !1 a !7  "
                 "un movimiento    x  parar    v  panel    0  números del partido.    "
                 "OJO: cualquier cosa que mandes mientras se mueve lo FRENA.    "
                 "Si el robot NO tiene el programa del tablero, solo se puede mandar ? y h."
        ).pack(side="bottom", fill="x", padx=6)
        self._etiqueta(abajo, text="Mandar al robot:", font=F_NORMAL_B).pack(side="left")
        self.ent_consola = ttk.Entry(abajo, font=F_CONSOLA)
        self.ent_consola.pack(side="left", fill="x", expand=True, padx=6)
        self.ent_consola.bind("<Return>", lambda e: self._consola_mandar())
        ttk.Button(abajo, text="Mandar", command=self._consola_mandar).pack(side="left")
        marco = tk.Frame(t)
        marco.pack(fill="both", expand=True, padx=6)
        self.txt_consola = tk.Text(marco, font=F_CONSOLA, wrap="word", bg="#fafafa", height=10)
        barra = ttk.Scrollbar(marco, orient="vertical", command=self.txt_consola.yview)
        self.txt_consola.configure(yscrollcommand=barra.set)
        barra.pack(side="right", fill="y")
        self.txt_consola.pack(side="left", fill="both", expand=True)
        self.txt_consola.tag_configure("humano", foreground="#212121")
        self.txt_consola.tag_configure("maquina", foreground="#607d8b")
        self.txt_consola.tag_configure("rota", foreground=ROJO)
        self.txt_consola.tag_configure("mandado", foreground=VERDE, font=(F_CONSOLA[0], F_CONSOLA[1], "bold"))
        self.txt_consola.tag_configure("sistema", foreground="#6a1b9a")
        self.txt_consola.configure(state="disabled")

    # ------------------------------------------------------------ la cola

    def _revisar_cola(self):
        """Cada 50 ms: lo que dejó el hilo del cable. Nunca espera."""
        try:
            lote = []
            for _ in range(600):
                try:
                    lote.append(self.cola.get_nowait())
                except queue.Empty:
                    break
            self._procesar_lote(lote)
        finally:
            self.raiz.after(50, self._revisar_cola)

    def _procesar_lote(self, lote):
        for msg in lote:
            try:
                self._procesar(msg)
            except Exception:
                self._falla()
        if self.pendientes:
            cambios, self.pendientes = self.pendientes, set()
            try:
                self._refrescar(cambios)
            except Exception:
                self._falla()

    def _procesar(self, msg):
        tipo = msg[0]
        if tipo == "linea":
            self._llego_linea(msg[1])
        elif tipo == "conectado":
            _, self.puerto, self.serie = msg
            self.conectado = True
            self.situacion = "conectado"
            self.es_delantero = (self.serie == SERIE_DELANTERO)
            self.estado.reiniciar_conexion()
            self.t_conexion = time.monotonic()
            self.aviso_programa = False
            self.t_pedido = self.t_pedido_panel = None
            self.es_tablero = self.pidio_numeros = False
            self.t_pide_hola = None
            self._consola_agregar("--- conectado: %s (serie %s) ---" % (self.puerto, self.serie or "?"), "sistema")
            # NO se le manda nada todavía: recién cuando llega algo que solo
            # manda el tablero (ver _reconocer_tablero).
            if not self.es_delantero:
                self._aviso("Este robot NO es el delantero: la ventana no le manda nada.", ROJO)
            self.pendientes |= {"conexion", "valores", "sensores", "modo", "resultado"}
        elif tipo == "desconectado":
            self.conectado = False
            self.situacion = "buscando"
            self.estado.modo = "ESPERA"
            self.estado.cuenta = None
            self.t_pedido = self.t_pedido_panel = None
            self.es_tablero = self.pidio_numeros = False
            self._consola_agregar("--- se desenchufó (o se reinició el robot) ---", "sistema")
            self._aviso("Se cortó la conexión con el robot. Cuando vuelva, se reconecta solo.", NARANJA)
            self.pendientes |= {"conexion", "modo", "sensores"}
        elif tipo == "buscando":
            self.situacion = "buscando"
            self.pendientes.add("conexion")
        elif tipo == "otro":
            # Una Teensy que NO es el delantero: su puerto ni se abrió.
            self.situacion = "otro"
            self.puerto, self.serie = msg[1], msg[2]
            self._consola_agregar("--- en %s hay una Teensy que NO es el delantero (serie %s): "
                                  "no la abro ---" % (msg[1], msg[2] or "?"), "sistema")
            self._aviso("Hay otra Teensy enchufada (NO es el delantero): la ventana no la toca. "
                        "Enchufá el delantero.", ROJO)
            self.pendientes.add("conexion")
        elif tipo == "ocupado":
            self.situacion = "ocupado"
            self.puerto = msg[1]
            self._consola_agregar("--- %s ocupado: %s ---" % (msg[1], msg[2]), "sistema")
            self.pendientes.add("conexion")

    def _llego_linea(self, texto):
        ahora = time.monotonic()
        self.t_ultima_linea = ahora
        m = interpretar(texto)
        tipo = m["tipo"]
        if tipo == "TEXTO":
            self._consola_agregar(texto, "humano")
        elif tipo == "ROTA":
            self._consola_agregar("[renglón roto, no se usa: %s]  %s" % (m.get("motivo", ""), texto), "rota")
        elif tipo != "S" or bool(self.ver_s.get()):
            self._consola_agregar(texto, "maquina")
        if tipo in ("CUENTA", "INICIO", "FRENO", "ERROR", "R", "HOLA"):
            self.t_pedido = None        # el robot contestó al "!n"
        self.pendientes |= self.estado.aplicar(m, ahora)
        if tipo == "HOLA":
            self.pendientes.add("conexion")
        if tipo in ("S", "HOLA") and self.conectado and self.es_delantero:
            self._reconocer_tablero(tipo, ahora)

    def _reconocer_tablero(self, tipo, ahora):
        """Llegó un #S o un #HOLA. Al conectar la ventana NO mandó nada; recién
        ahora que sabe que es el tablero le pregunta:
           "h" = qué programa tenés (si todavía no lo dijo)
           "?" = cuánto valen tus números (una vez por conexión)
        El robot manda #S SOLO en espera: la pregunta no puede frenar un
        movimiento. Si llegan #S pero el #HOLA no (por ejemplo, el cable se
        reconectó en medio de un movimiento y el robot se comió la "h"),
        vuelve a pedir "h", como mucho cada 2 s."""
        if tipo == "HOLA" and not self._programa_ok():
            return                      # el #HOLA de otro programa: no es el tablero
        if not self.es_tablero:
            self.es_tablero = True
            self.pendientes.add("conexion")
        if self.estado.modo != "ESPERA" or self._pedido_pendiente():
            return                      # por las dudas: nada que lo pueda frenar
        if self.estado.hola is None and (self.t_pide_hola is None or ahora - self.t_pide_hola >= 2.0):
            self.t_pide_hola = ahora
            self._mandar("h")
        if not self.pidio_numeros:
            self.pidio_numeros = True
            self._mandar("?")

    # ------------------------------------------------------------ refrescar

    def _refrescar(self, cambios):
        if "conexion" in cambios or "hola" in cambios:
            self._pintar_conexion()
        if "valores" in cambios:
            self._pintar_valores()
        if "sensores" in cambios:
            if self._visible(self.tab_pelota):
                self._dibujar_pelota()
                self._textos_pelota()
            if self._visible(self.tab_sensores):
                self._dibujar_lineas()
                self._dibujar_brujula()
                self._textos_sensores()
        if cambios & {"modo", "resultado", "freno", "conexion", "hola"}:
            self._pintar_cuenta()       # ahí se ve POR QUÉ PROBAR está apagado
        if "resultado" in cambios:
            self._pintar_resultado()
        if "ok" in cambios and self.estado.ultimo_ok:
            self._aviso("El robot confirmó: %s = %s" % self.estado.ultimo_ok, VERDE)
        if "error" in cambios and self.estado.ultimo_error is not None:
            self._aviso("El robot dice: " + self.estado.ultimo_error, ROJO)
        if "freno" in cambios and self.estado.freno:
            self._aviso("Se frenó: " + self.estado.freno, ROJO)
        if "panel_listo" in cambios and self.t_pedido_panel is not None:
            self.t_pedido_panel = None
            self._mostrar_panel(list(self.estado.panel))
        self._actualizar_habilitados()

    def _visible(self, pestana):
        try:
            return str(self.cuaderno.select()) == str(pestana)
        except Exception:
            return False

    def _programa_ok(self):
        return self.estado.hola is not None and self.estado.hola[0] == PROGRAMA

    def _habla_tablero(self):
        """¿El robot tiene el programa del tablero? (mandó #S o su #HOLA).
        Solo ese obedece PARAR y entiende lo que se escribe en la consola."""
        return self.conectado and self.es_delantero and (self.es_tablero or self._programa_ok())

    def _pedido_pendiente(self):
        return self.t_pedido is not None and time.monotonic() - self.t_pedido < 3.0

    def _puede(self):
        if not self.conectado and self.situacion == "otro":
            return False, "La Teensy enchufada NO es el delantero: enchufá el delantero."
        if not self.conectado and self.situacion == "ocupado":
            return False, "El puerto del robot está ocupado: cerrá el monitor serie u otra ventana del tablero."
        if self.conectado and self.es_delantero and self.es_tablero and self.estado.hola is None:
            return False, "Esperando que el robot diga qué programa tiene..."
        return puede_mover(self.conectado, self.es_delantero, self._programa_ok(),
                           self.estado.modo, self._pedido_pendiente())

    @staticmethod
    def _habilitar(widget, si):
        widget.state(["!disabled"] if si else ["disabled"])

    def _actualizar_habilitados(self):
        se_puede, _ = self._puede()
        quieto = self.estado.modo == "ESPERA" and not self._pedido_pendiente()
        for b in self.botones_mov.values():
            self._habilitar(b, quieto)
        self._habilitar(self.btn_probar, se_puede)
        self._habilitar(self.btn_copiar, se_puede)
        self._habilitar(self.btn_partido, se_puede)
        for fila in self.filas.values():
            for b in fila["botones"]:
                self._habilitar(b, se_puede)
            if fila["entrada"] is not None:
                self._habilitar(fila["entrada"], se_puede)

    def _aviso(self, texto, color="#212121"):
        self.ultimo_aviso = (texto, color)
        self.lbl_aviso.configure(text=texto, fg=color)

    def _pintar_conexion(self):
        if self.conectado and self.demo:
            texto, fondo, letra = "●  ROBOT DE MENTIRA (--demo): no hay ningún robot de verdad", "#6a1b9a", "white"
        elif self.conectado and self.es_delantero:
            texto, fondo, letra = "●  CONECTADO  ·  %s  ·  DELANTERO" % self.puerto, VERDE, "white"
        elif self.conectado:
            texto = "✖  ESTE NO ES EL DELANTERO  (serie %s, en %s): la ventana no le manda nada" % (
                self.serie or "desconocida", self.puerto)
            fondo, letra = ROJO, "white"
        elif self.situacion == "otro":
            texto = "✖  ESA TEENSY NO ES EL DELANTERO  (serie %s): enchufá el delantero" % (
                self.serie or "desconocida")
            fondo, letra = ROJO, "white"
        elif self.situacion == "ocupado":
            texto = ("El robot está en %s, pero el puerto está OCUPADO: cerrá el monitor serie "
                     "(PlatformIO o Arduino) u OTRA VENTANA DEL TABLERO" % self.puerto)
            fondo, letra = NARANJA, "white"
        else:
            texto, fondo, letra = "○  Buscando el robot...  enchufá el cable USB", "#e0e0e0", "#424242"
        self.lbl_conexion.configure(text=texto, bg=fondo, fg=letra)
        programa, color = self._texto_programa()
        self.lbl_programa.configure(text=programa, fg=color)

    def _texto_programa(self):
        if not self.conectado or not self.es_delantero:
            return "", "#424242"
        hola = self.estado.hola
        if hola is None:
            if self.es_tablero:
                return "Preguntándole al robot qué programa tiene...", GRIS
            if self.aviso_programa:
                # Lo más importante primero, por si no entra todo.
                return ("⚠  " + NO_OBEDECE_PARAR + "  (No contesta como tablero: "
                        "¿cargaste su programa con TABLERO.bat?)"), ROJO
            return "Escuchando al robot para saber qué programa tiene...", GRIS
        if hola[0] != PROGRAMA:
            return ("⚠  " + NO_OBEDECE_PARAR + "  (Tiene otro programa: %s. "
                    "El del tablero se carga con TABLERO.bat.)" % hola[0]), ROJO
        return "Programa del tablero cargado  (%s %s)" % hola, VERDE

    def _texto_sin_datos(self):
        """Por qué no hay datos frescos para dibujar (o None si los hay)."""
        if not self.conectado:
            return "Sin conexión con el robot"
        if self.estado.modo != "ESPERA":
            return "El robot se está moviendo: mientras mide no manda datos"
        if self.estado.sensores is None:
            if self.aviso_programa:
                return "El robot no manda datos: ¿tiene el programa del tablero?"
            return "Esperando datos del robot..."
        viejo = time.monotonic() - (self.estado.t_sensores or 0)
        if viejo > 1.5:
            return "Los datos son de hace %.0f s: el robot dejó de mandar" % viejo
        return None

    # ------------------------------------------------------------ PELOTA

    def _dibujar_pelota(self):
        c = self.cv_pelota
        c.delete("all")
        w = max(int(c.winfo_width()), 300)
        h = max(int(c.winfo_height()), 300)
        abajo = 70
        escala = max(0.5, min((h - abajo - 30) / 205.0, (w / 2.0 - 20) / 140.0))   # píxeles por cm
        cx, cy = w / 2.0, h - abajo

        # lo que alcanza a ver la cámara
        for signo in (1, -1):
            a = math.radians(ANGULO_CAMARA * signo)
            x2, y2 = cm_a_pantalla(205 * math.cos(a), 205 * math.sin(a), cx, cy, escala)
            c.create_line(cx, cy, x2, y2, fill="#a5d6a7", dash=(8, 5), width=2)
        # los círculos de distancia (solo para tener una idea)
        for radio, nombre, color in CIRCULOS:
            r = radio * escala
            c.create_oval(cx - r, cy - r, cx + r, cy + r, outline=color, dash=(3, 5), width=1)
            a = math.radians(-55)
            tx, ty = cm_a_pantalla(radio * math.cos(a), radio * math.sin(a), cx, cy, escala)
            c.create_text(tx + 4, ty, text=nombre, anchor="w", fill=color, font=F_CHICO)
        # las rayas de X: lo que el robot usa de verdad para rodearla o soltarla
        for x_cm, nombre, color in RAYAS_X:
            x1, y1 = cm_a_pantalla(x_cm, 140, cx, cy, escala)      # 140 cm a la izquierda
            x2, y2 = cm_a_pantalla(x_cm, -140, cx, cy, escala)     # 140 cm a la derecha
            c.create_line(x1, y1, x2, y2, fill=color, dash=(8, 4), width=2)
            c.create_text(x1 + 6, y1 - 3, text=nombre, anchor="sw", fill=color, font=F_NORMAL_B)
        c.create_text(14, 12, anchor="nw", text="◄ IZQUIERDA del robot", fill="white", font=F_NORMAL_B)
        c.create_text(w - 14, 12, anchor="ne", text="DERECHA del robot ►", fill="white", font=F_NORMAL_B)

        # el robot, con el frente marcado
        rr = max(11 * escala, 14)
        c.create_oval(cx - rr, cy - rr, cx + rr, cy + rr, fill="#37474f", outline="white", width=2)
        c.create_polygon(cx, cy - rr - 14, cx - 10, cy - rr + 3, cx + 10, cy - rr + 3,
                         fill="#ff5252", outline="white")
        c.create_text(cx, cy + rr + 16, text="ROBOT  (el triángulo es el frente)", fill="white", font=F_NORMAL_B)

        s = self.estado.sensores
        motivo = self._texto_sin_datos()
        if motivo:
            c.create_text(w / 2.0, 44, text=motivo, fill="#fff59d", font=F_MEDIO_B)
        if s is None:
            return

        for clave, color, nombre in (("am", "#fdd835", "amarillo"), ("az", "#1e88e5", "azul")):
            dato = s.get(clave)
            if not visible(dato):
                continue
            x, y, edad = dato
            viejo = edad > EDAD_VIEJA_MS
            px, py = cm_a_pantalla(x, y, cx, cy, escala)
            opciones = {"fill": "#9e9e9e" if viejo else color, "outline": color if viejo else "black", "width": 2}
            if viejo:
                opciones["dash"] = (4, 2)
            c.create_rectangle(px - 26, py - 9, px + 26, py + 9, **opciones)
            c.create_text(px, py + 20, text="arco %s  %d cm%s" % (nombre, round(math.hypot(x, y)),
                                                                   "  (viejo)" if viejo else ""),
                          fill="white", font=F_NORMAL_B)

        dato = s.get("p")
        if visible(dato):
            x, y, edad = dato
            viejo = edad > EDAD_VIEJA_MS
            px, py = cm_a_pantalla(x, y, cx, cy, escala)
            c.create_line(cx, cy, px, py, fill="#bdbdbd" if viejo else "#ffe0b2", width=2, dash=(2, 4))
            rb = max(3.5 * escala, 11)
            c.create_oval(px - rb, py - rb, px + rb, py + rb, fill="#9e9e9e" if viejo else "#ff8f00",
                          outline="white", width=2)
            c.create_text(px, py - rb - 12,
                          text=("última vez vista" if viejo else "%d cm" % round(math.hypot(x, y))),
                          fill="white", font=F_MEDIO_B)

    def _textos_pelota(self):
        s = self.estado.sensores
        if s is None:
            self.lbl_p_frase.configure(text="Sin datos del robot", fg=GRIS)
            self.lbl_p_detalle.configure(text="")
            self.lbl_amarillo.configure(text="")
            self.lbl_azul.configure(text="")
            self.lbl_p_camara.configure(text="")
            return
        frase, como = describir_objeto("Pelota", s.get("p"), True)
        self.lbl_p_frase.configure(text=frase, fg={"fresca": NARANJA, "vieja": GRIS}.get(como, "#424242"))
        p = s.get("p")
        if visible(p):
            x, y, _ = p
            costado = "a la izquierda" if y > 0 else ("a la derecha" if y < 0 else "justo adelante")
            self.lbl_p_detalle.configure(text="X = %s cm para adelante,  Y = %s cm (%s)" % (x, y, costado))
        else:
            self.lbl_p_detalle.configure(text="")
        for lbl, clave, nombre, color in ((self.lbl_amarillo, "am", "Arco amarillo", "#8d6e00"),
                                          (self.lbl_azul, "az", "Arco azul", AZUL)):
            frase, como = describir_objeto(nombre, s.get(clave), False)
            if como == "nunca":
                frase = "%s: no lo veo" % nombre
            lbl.configure(text=frase, fg=color if como == "fresca" else GRIS)
        texto, color = self._texto_camara(s.get("q"))
        self.lbl_p_camara.configure(text=texto, fg=color)

    @staticmethod
    def _texto_camara(q):
        if q is None:
            return "Cámara: ?", GRIS
        if q <= 0:
            return "Cámara: 0 paquetes por segundo. No manda nada.", ROJO
        return "Cámara: %g paquetes por segundo" % q, (VERDE if q >= 5 else NARANJA)

    # ------------------------------------------------------------ SENSORES

    def _dibujar_lineas(self):
        c = self.cv_lineas
        c.delete("all")
        w = max(int(c.winfo_width()), 360)
        h = max(int(c.winfo_height()), 240)
        s = self.estado.sensores
        nombres = ("1  DERECHO", "2  IZQUIERDO", "3  DELANTERO")
        izq, der = 150, 120
        ancho = w - izq - der
        alto_fila = (h - 20) / 3.0
        for i in range(3):
            y0 = 10 + i * alto_fila + alto_fila * 0.2
            y1 = y0 + alto_fila * 0.4
            medio = (y0 + y1) / 2.0
            c.create_text(10, medio, anchor="w", text=nombres[i], font=F_MEDIO_B, fill="#212121")
            c.create_rectangle(izq, y0, izq + ancho, y1, fill="#37474f", outline="")
            lecturas = s.get("l") if s else None
            umbrales = s.get("u") if s else None
            v = lecturas[i] if lecturas else None
            u = umbrales[i] if umbrales else None
            if v is None:
                c.create_text(izq + ancho + 12, medio, anchor="w", text="sin dato", font=F_NORMAL, fill=GRIS)
                continue
            blanco = u is not None and v >= u
            c.create_rectangle(izq, y0, izq + min(max(v / 1023.0, 0.0), 1.0) * ancho, y1,
                               fill="white" if blanco else "#66bb6a", outline="")
            if u is not None:
                ux = izq + min(max(u / 1023.0, 0.0), 1.0) * ancho
                c.create_line(ux, y0 - 8, ux, y1 + 8, fill="#e53935", width=3)
                c.create_text(ux, y1 + 10, anchor="n", text="umbral %s" % u, fill="#e53935", font=F_CHICO)
            c.create_text(izq + ancho + 12, medio - 11, anchor="w", text=str(v), font=F_MEDIO_B, fill="#212121")
            c.create_text(izq + ancho + 12, medio + 12, anchor="w", text="BLANCO" if blanco else "VERDE",
                          font=F_MEDIO_B, fill="#212121" if blanco else VERDE)
        motivo = self._texto_sin_datos()
        if motivo:
            c.create_text(w / 2.0, h - 6, anchor="s", text=motivo, font=F_NORMAL_B, fill=NARANJA)

    def _dibujar_brujula(self):
        c = self.cv_brujula
        c.delete("all")
        w = max(int(c.winfo_width()), 220)
        h = max(int(c.winfo_height()), 220)
        r = min(w, h) / 2.0 - 26
        cx, cy = w / 2.0, h / 2.0
        c.create_oval(cx - r, cy - r, cx + r, cy + r, outline="#455a64", width=3, fill="white")
        for grado in range(0, 360, 30):
            a = math.radians(grado)
            c.create_line(cx + (r - 10) * math.sin(a), cy - (r - 10) * math.cos(a),
                          cx + r * math.sin(a), cy - r * math.cos(a), fill="#455a64", width=2)
            if grado % 90 == 0:
                c.create_text(cx + (r + 15) * math.sin(a), cy - (r + 15) * math.cos(a),
                              text=str(grado), font=F_CHICO, fill="#455a64")
        s = self.estado.sensores
        rumbo = s.get("r") if s else None
        if rumbo is None:
            c.create_text(cx, cy, text=("SIN\nGIROSCOPIO" if s else "sin datos"), justify="center",
                          fill=ROJO if s else GRIS, font=F_MEDIO_B)
            return
        a = math.radians(rumbo)
        c.create_line(cx, cy, cx + (r - 18) * math.sin(a), cy - (r - 18) * math.cos(a),
                      width=5, fill=ROJO, arrow="last", arrowshape=(16, 20, 6))
        c.create_oval(cx - 7, cy - 7, cx + 7, cy + 7, fill="#455a64", outline="")

    def _textos_sensores(self):
        s = self.estado.sensores
        rumbo = s.get("r") if s else None
        self.lbl_rumbo.configure(text=("%.1f°" % rumbo) if rumbo is not None else "---")
        g = s.get("g") if s else None
        if s is None:
            texto, color = "sin datos", GRIS
        elif g is None:
            texto, color = "SIN GIROSCOPIO: " + AYUDA_SIN_GIRO, ROJO
        elif g == 5:
            texto, color = "fusión OK  (estado 5)", VERDE
        else:
            texto, color = "estado %d: %s" % (g, ESTADOS_GIRO.get(g, "?")), ROJO
        self.lbl_fusion.configure(text=texto, fg=color)
        q = s.get("q") if s else None
        if q is None:
            self.lbl_q.configure(text="? paquetes por segundo", fg=GRIS)
            self.lbl_q_txt.configure(text="")
        else:
            texto, color = self._texto_camara(q)
            self.lbl_q.configure(text="%g paquetes por segundo" % q, fg=color)
            self.lbl_q_txt.configure(
                text=("La cámara no manda nada: ¿está prendida? ¿el cable está bien?" if q <= 0
                      else "La cámara le manda al robot lo que ve, por un cable."))
        for clave, celdas in self.celdas_cam.items():
            dato = s.get(clave) if s else None
            if not visible(dato):
                for lbl, texto in zip(celdas, ("-", "-", "nunca")):
                    lbl.configure(text=texto, fg=GRIS)
                continue
            x, y, edad = dato
            color = GRIS if edad > EDAD_VIEJA_MS else "#212121"
            celdas[0].configure(text=str(x), fg=color)
            celdas[1].configure(text=str(y), fg=color)
            celdas[2].configure(text="%.1f s" % (edad / 1000.0), fg=color)

    # ------------------------------------------------------------ MOVIMIENTOS

    def _elegir(self, n):
        self.elegido = n
        m = MOV_POR_NUMERO[n]
        for k, b in self.botones_mov.items():
            b.configure(style="Elegido.TButton" if k == n else "Mov.TButton")
        self.lbl_mov_titulo.configure(text=m["titulo"])
        self.lbl_mov_desc.configure(text=m["que_hace"])
        self.lbl_mov_lugar.configure(text="⚠ " + AVISO_LINEA + " " + m["lugar"] + " Y el cable flojo.")
        self.btn_probar.configure(text="▶  PROBAR  " + m["nombre"].replace("_", " "))
        self._armar_tabla(m)
        self._pintar_valores()
        self._actualizar_habilitados()

    def _armar_tabla(self, m):
        for hijo in self.tabla.winfo_children():
            hijo.destroy()
        self.filas = {}
        for i, nombre in enumerate(m["numeros"]):
            paso, explicacion, opciones = NUMEROS[nombre]
            fondo = "white" if i % 2 == 0 else "#e8eef3"
            f = tk.Frame(self.tabla, bg=fondo, padx=6, pady=4)
            f.pack(fill="x", pady=1)
            arriba = tk.Frame(f, bg=fondo)
            arriba.pack(fill="x")
            tk.Label(arriba, text=nombre, font=F_NORMAL_B, bg=fondo, anchor="w", width=18).pack(side="left")
            if opciones is None:
                menos = ttk.Button(arriba, text="−", style="Paso.TButton", width=3,
                                   command=lambda n=nombre: self._paso(n, -1))
                mas = ttk.Button(arriba, text="+", style="Paso.TButton", width=3,
                                 command=lambda n=nombre: self._paso(n, +1))
            else:
                menos = ttk.Button(arriba, text=opciones[0], style="Paso.TButton", width=4,
                                   command=lambda n=nombre: self._mandar_valor(n, "0"))
                mas = ttk.Button(arriba, text=opciones[1], style="Paso.TButton", width=4,
                                 command=lambda n=nombre: self._mandar_valor(n, "1"))
            valor = tk.Label(arriba, text="?", font=F_GRANDE, width=5, bg=fondo)
            menos.pack(side="left")
            valor.pack(side="left", padx=4)
            mas.pack(side="left")
            botones = [menos, mas]
            entrada = None
            if opciones is None:
                entrada = ttk.Entry(arriba, width=6, font=F_MEDIO)
                entrada.pack(side="left", padx=(14, 4))
                entrada.bind("<Return>", lambda e, n=nombre: self._poner(n))
                poner = ttk.Button(arriba, text="Poner", width=6, command=lambda n=nombre: self._poner(n))
                poner.pack(side="left")
                botones.append(poner)
            tk.Label(f, text=explicacion, font=F_CHICO, fg="#546e7a", bg=fondo, anchor="w",
                     justify="left", wraplength=480).pack(fill="x")
            self.filas[nombre] = {"valor": valor, "entrada": entrada, "botones": botones, "opciones": opciones}
        self.cv_tabla.yview_moveto(0)

    def _rueda(self, evento):
        pasos = int(-evento.delta / 120) or (-1 if evento.delta > 0 else 1)
        self.cv_tabla.yview_scroll(pasos, "units")

    def _pintar_valores(self):
        for nombre, fila in self.filas.items():
            texto = self.estado.valores.get(nombre)
            if texto is None:
                mostrar = "?"
            elif fila["opciones"]:
                try:
                    mostrar = fila["opciones"][1] if float(texto) != 0 else fila["opciones"][0]
                except ValueError:
                    mostrar = texto
            else:
                mostrar = texto
            fila["valor"].configure(text=mostrar)

    def _paso(self, nombre, signo):
        actual = self.estado.valores.get(nombre)
        if actual is None:
            self._aviso("Todavía no sé cuánto vale %s: el robot no lo mandó." % nombre, NARANJA)
            return
        try:
            nuevo = nuevo_valor(actual, NUMEROS[nombre][0], signo)
        except ValueError:
            self._aviso("%s vale '%s' y no lo entiendo." % (nombre, actual), ROJO)
            return
        self._mandar_valor(nombre, nuevo)

    def _poner(self, nombre):
        fila = self.filas.get(nombre)
        if not fila or fila["entrada"] is None:
            return
        escrito = str(fila["entrada"].get())
        valor = normalizar_entrada(escrito)
        if valor is None:
            self._aviso("'%s' no es un número. Escribí por ejemplo 7 o 0.5" % escrito.strip(), ROJO)
            return
        fila["entrada"].delete(0, "end")
        self._mandar_valor(nombre, valor)

    def _mandar_valor(self, nombre, valor):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if self._mandar("=%s %s" % (nombre, valor)):
            self._aviso("Mandé %s = %s. Esperando que el robot confirme..." % (nombre, valor), GRIS)

    def _mandar(self, texto):
        """El único lugar por donde sale algo hacia el robot."""
        if not self.conectado:
            self._aviso("No hay robot conectado.", NARANJA)
            return False
        if not self.es_delantero:
            self._aviso("Este robot NO es el delantero: la ventana no le manda nada.", ROJO)
            return False
        if not self.conexion.enviar(texto):
            self._aviso("No se pudo mandar: ¿se desenchufó el cable?", ROJO)
            return False
        self._consola_agregar("> " + texto, "mandado")
        return True

    def _probar(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        n = self.elegido
        if self._mandar("!%d" % n):
            self.t_pedido = time.monotonic()
            self._aviso("Mandé !%d (%s): arranca en 1.5 s. Para frenarlo: PARAR o Esc." % (
                n, MOV_POR_NUMERO[n]["nombre"]), NARANJA)
            self._pintar_cuenta()
            self._actualizar_habilitados()

    def _parar(self):
        if (self.conectado and not self.es_delantero) or (not self.conectado and self.situacion == "otro"):
            self._aviso("Este no es el delantero: la ventana no le manda nada. Para pararlo, apagá la batería.", ROJO)
            return
        # La "x" sale SIEMPRE (si es el tablero, lo frena). Pero si el robot
        # no tiene el programa del tablero, la "x" no hace nada: hay que
        # decirlo, para que nadie crea que el robot está frenado.
        if self._mandar("x"):
            if self._habla_tablero():
                self._aviso("PARAR: mandé x.", ROJO)
            else:
                self._aviso("Mandé x, pero OJO: " + NO_OBEDECE_PARAR, ROJO)

    def _copiar_panel(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if self._mandar("v"):
            self.t_pedido_panel = time.monotonic()
            self._aviso("Pidiendo las líneas para el panel...", GRIS)

    def _volver_partido(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if not messagebox.askyesno(
                "Volver a los números del partido",
                "¿Volver TODOS los números a los del programa de partido?\n"
                "Se pierde lo que cambiaron acá (si sirvió, primero 'Copiar al panel').",
                parent=self.raiz):
            return
        if self._mandar("0"):
            self._aviso("Pedí volver a los números del partido.", GRIS)

    def _mostrar_panel(self, lineas):
        texto = "\n".join(lineas)
        if lineas:
            copiado = self._copiar_texto(texto + "\n")
            mensaje = (PANEL_COPIADO if copiado else PANEL_NO_COPIADO) + " " + INSTRUCCIONES_PANEL
            self._aviso("Copié %d línea%s para el PANEL DE CONTROL." % (len(lineas), "" if len(lineas) == 1 else "s"), VERDE)
        else:
            mensaje = ("No hay nada para copiar: los números que existen en el programa de partido "
                       "están todos igual que allá. (Los que son solo de esta prueba no se copian.)")
            self._aviso("Nada para copiar al panel.", GRIS)
        v = tk.Toplevel(self.raiz)
        v.title("Para pegar en el PANEL DE CONTROL")
        v.configure(bg=FONDO)
        v.transient(self.raiz)
        self._etiqueta(v, text=mensaje, font=F_MEDIO, wraplength=840).pack(fill="x", padx=14, pady=(12, 6))
        caja = tk.Text(v, font=F_CONSOLA, height=max(3, min(20, len(lineas) + 1)), width=90, bg="white")
        caja.insert("1.0", texto)
        caja.pack(fill="both", expand=True, padx=14)
        fila = tk.Frame(v, bg=FONDO)
        fila.pack(fill="x", padx=14, pady=10)
        if lineas:
            ttk.Button(fila, text="Copiar de nuevo",
                       command=lambda: self._copiar_texto(texto + "\n")).pack(side="left")
        ttk.Button(fila, text="Cerrar", command=v.destroy).pack(side="right")

    def _copiar_texto(self, texto):
        try:
            self.raiz.clipboard_clear()
            self.raiz.clipboard_append(texto)
            return True
        except Exception:
            return False

    def _pintar_cuenta(self):
        e = self.estado
        if e.modo == "CUENTA":
            texto, color = "Arranca en %.1f s...\n(PARAR o Esc para cancelar)" % (e.cuenta or 0), NARANJA
        elif e.modo == "MOVIENDO":
            texto, color = "¡SE ESTÁ MOVIENDO!\n(PARAR o Esc para frenarlo)", ROJO
        elif self._pedido_pendiente():
            texto, color = "Pedido... esperando que el robot cuente.", GRIS
        else:
            se_puede, motivo = self._puede()
            if not se_puede:
                # Un botón apagado no explica nada al apretarlo: el motivo va acá.
                texto, color = motivo, NARANJA
            elif e.freno and e.informe is None:
                texto, color = "Se frenó: %s" % e.freno, ROJO
                if "giroscopio" in e.freno:
                    texto += ". " + AYUDA_SIN_GIRO[0].upper() + AYUDA_SIN_GIRO[1:]
            else:
                texto, color = "Listo. Al apretar PROBAR cuenta 1.5 s y arranca.", "#37474f"
        self.lbl_cuenta.configure(text=texto, fg=color)

    def _pintar_resultado(self):
        e = self.estado
        inf = e.informe
        nombre = (inf or {}).get("mov") or e.mov_nombre
        self.lbl_res_titulo.configure(text="RESULTADO" + ("  ·  " + nombre.replace("_", " ") if nombre else ""))
        if inf is not None:
            fondo, letra, leida, explicacion = NOTAS[inf["nota"]]
        elif e.modo in ("CUENTA", "MOVIENDO"):
            fondo, letra, leida, explicacion = "#9e9e9e", "white", "midiendo...", "El gráfico y la nota llegan cuando termina."
        elif e.freno:
            fondo, letra, leida, explicacion = "#9e9e9e", "white", "FRENADO", "Se frenó (%s) antes de moverse: no hay informe." % e.freno
            if "giroscopio" in e.freno:
                explicacion += " Sin giroscopio: " + AYUDA_SIN_GIRO
        else:
            fondo, letra, leida, explicacion = "#bdbdbd", "white", "-", "Todavía no hay resultado. Elegí un movimiento y apretá PROBAR."
        self.lbl_nota.configure(text=leida, bg=fondo, fg=letra)
        self.lbl_nota_txt.configure(text=explicacion)

        t = self.txt_detalle
        t.configure(state="normal")
        t.delete("1.0", "end")
        if e.freno:
            t.insert("end", "Se frenó antes de terminar: %s\n" % e.freno, "malo")
        if inf is not None:
            t.insert("end", "Lo que midió:\n", "titulo")
            for frase, estilo in explicar_informe(inf):
                t.insert("end", "•  " + frase + "\n", estilo)
        if e.consejos:
            t.insert("end", "\nQué tocar:\n", "titulo")
            for consejo in e.consejos:
                t.insert("end", "→  " + consejo + "\n", "consejo")
        t.configure(state="disabled")
        self._dibujar_grafico()

    def _dibujar_grafico(self):
        c = self.cv_grafico
        c.delete("all")
        w = max(int(c.winfo_width()), 320)
        h = max(int(c.winfo_height()), 160)
        muestras = self.estado.muestras
        if not muestras:
            c.create_text(w / 2.0, h / 2.0, fill=GRIS, font=F_NORMAL,
                          text="Acá va el gráfico de cuánto se torció, cuando termine una prueba.")
            return
        ts = [float(m[0]) for m in muestras]
        gs = desenrollar([float(m[1]) for m in muestras])
        fases = [m[2] for m in muestras]
        tmax = max(max(ts), 1.0)
        tope = escala_grados(max(abs(g) for g in gs))
        izq, der, arr, aba = 62, 14, 16, 26
        ancho, alto = w - izq - der, h - arr - aba

        def X(t):
            return izq + t / tmax * ancho

        def Y(g):
            return arr + (tope - g) / (2.0 * tope) * alto

        tipo = tipo_de(self.estado.mov_nombre or (self.estado.informe or {}).get("mov"))
        if tipo != "giro":
            # la franja "prolija": hasta 4 grados para cada lado
            c.create_rectangle(izq, Y(4), izq + ancho, Y(-4), fill="#e8f5e9", outline="")
        paso = paso_grilla(tope)
        for i in range(-int(tope // paso), int(tope // paso) + 1):
            g = i * paso
            c.create_line(izq, Y(g), izq + ancho, Y(g), fill="#424242" if g == 0 else "#e0e0e0",
                          width=2 if g == 0 else 1)
            c.create_text(izq - 6, Y(g), anchor="e", text=("%+d°" % g) if g else "0°", font=F_CHICO, fill="#616161")
        pt = paso_tiempo(tmax)
        for i in range(0, int(tmax // pt) + 1):
            x = X(i * pt)
            c.create_line(x, arr + alto, x, arr + alto + 4, fill="#616161")
            c.create_text(x, arr + alto + 5, anchor="n", text="%d" % (i * pt), font=F_CHICO, fill="#616161")
        c.create_text(w - 4, h - 3, anchor="se", text="ms", font=F_CHICO, fill="#616161")
        c.create_text(izq + 6, arr + 2, anchor="nw", text="▲ izquierda", font=F_CHICO, fill="#616161")
        c.create_text(izq + 6, arr + alto - 2, anchor="sw", text="▼ derecha", font=F_CHICO, fill="#616161")

        # la curva, un tramo de color por fase
        tramo = [X(ts[0]), Y(gs[0])]
        fase = fases[0]
        for i in range(1, len(ts)):
            tramo += [X(ts[i]), Y(gs[i])]
            if fases[i] != fase or i == len(ts) - 1:
                c.create_line(*tramo, fill=COLOR_FASE.get(fase, "black"), width=3)
                tramo = [X(ts[i]), Y(gs[i])]
                fase = fases[i]
        if len(ts) == 1:
            c.create_oval(X(ts[0]) - 3, Y(gs[0]) - 3, X(ts[0]) + 3, Y(gs[0]) + 3,
                          fill=COLOR_FASE.get(fase, "black"), outline="")

        # la leyenda, arriba a la derecha
        x = izq + ancho - 8
        for f in ("m", "r", "g"):
            if f not in fases:
                continue
            c.create_text(x, arr + 8, anchor="e", text=FASES[f], font=F_NORMAL_B, fill=COLOR_FASE[f])
            x -= 9 * len(FASES[f]) + 12
            c.create_line(x, arr + 8, x + 18, arr + 8, fill=COLOR_FASE[f], width=4)
            x -= 16

    # ------------------------------------------------------------ CONSOLA

    def _consola_agregar(self, texto, estilo):
        t = self.txt_consola
        al_final = float(t.yview()[1]) >= 0.999
        t.configure(state="normal")
        t.insert("end", texto + "\n", estilo)
        self.lineas_consola += 1
        if self.lineas_consola > 3000:              # que no crezca para siempre
            t.delete("1.0", "1001.0")
            self.lineas_consola -= 1000
        t.configure(state="disabled")
        if al_final:
            t.see("end")

    def _consola_borrar(self):
        self.txt_consola.configure(state="normal")
        self.txt_consola.delete("1.0", "end")
        self.txt_consola.configure(state="disabled")
        self.lineas_consola = 0

    def _consola_mandar(self):
        texto = str(self.ent_consola.get()).strip()
        if not texto:
            return
        # Con otro programa cargado, lo que se escribe puede ser una orden de
        # ese programa (en motores-a-mano, "!1" prende una rueda, y la "x" no
        # la frena). Solo se deja preguntar: "h" y "?".
        if texto not in ("h", "?") and not self._habla_tablero():
            self._aviso("La consola solo le habla al programa del tablero. Con otro programa, "
                        "solo se puede mandar ? y h.", NARANJA)
            return
        if self._mandar(texto):
            self.ent_consola.delete(0, "end")
            if texto.startswith("!"):
                self.t_pedido = time.monotonic()

    # ------------------------------------------------------------ latido y cierre

    def _latido(self):
        """Cada 250 ms: lo que depende del reloj."""
        try:
            ahora = time.monotonic()
            cambios = set()
            if (self.conectado and self.es_delantero and self.estado.hola is None
                    and not self.aviso_programa and self.t_conexion is not None
                    and ahora - self.t_conexion > 3.0):
                self.aviso_programa = True
                cambios |= {"conexion", "sensores"}
            if self.t_pedido is not None and ahora - self.t_pedido >= 3.0:
                self.t_pedido = None
                cambios.add("modo")
                if self.estado.modo == "ESPERA":
                    self._aviso("El robot no arrancó la cuenta. ¿Tiene el programa del tablero?", NARANJA)
            if self.t_pedido_panel is not None and ahora - self.t_pedido_panel > 4.0:
                self.t_pedido_panel = None
                self._aviso("No llegó el panel completo. Probá de nuevo.", NARANJA)
            if (self.estado.modo in ("CUENTA", "MOVIENDO") and self.t_ultima_linea is not None
                    and ahora - self.t_ultima_linea > 20.0):
                self.estado.modo = "ESPERA"
                self.estado.cuenta = None
                cambios.add("modo")
                self._aviso("El robot no avisó que terminó. Si se sigue moviendo: PARAR o apagar la batería.", ROJO)
            if self.conectado and self.estado.t_sensores is not None and ahora - self.estado.t_sensores > 1.5:
                cambios.add("sensores")          # para que se vea "los datos son de hace..."
            if cambios:
                self._refrescar(cambios)
        except Exception:
            self._falla()
        finally:
            self.raiz.after(250, self._latido)

    def _cerrar(self):
        # Si se cierra a mitad de un movimiento, que frene.
        try:
            if self._habla_tablero():
                self.conexion.enviar_ya("x")
        except Exception:
            pass
        try:
            self.conexion.cerrar()
        except Exception:
            pass
        self.raiz.destroy()

    def _falla_tk(self, tipo, valor, tb):
        self._registrar_falla("".join(traceback.format_exception(tipo, valor, tb)))

    def _falla(self):
        self._registrar_falla(traceback.format_exc())

    def _registrar_falla(self, texto):
        if self.estricto:
            raise RuntimeError(texto)
        guardar_falla(texto)
        try:
            self._consola_agregar("!!! FALLA DE LA VENTANA (mostrársela al profe):\n" + texto, "rota")
            self._aviso("La ventana tuvo una falla (está en la CONSOLA y en tablero-error.txt). Sigue andando.", ROJO)
        except Exception:
            pass


# =====================================================================
#  7. LA AUTOPRUEBA (--autoprueba): sin ventana y sin USB
# =====================================================================

class _CableDePrueba:
    """Un cable de mentira que anota lo que la ventana manda."""

    def __init__(self):
        self.mandados = []

    def enviar(self, texto):
        self.mandados.append(texto)
        return True

    def enviar_ya(self, texto):
        self.mandados.append(texto)
        return True

    def arrancar(self):
        pass

    def cerrar(self):
        pass


S_BUENA = ("#S t=12345 l=310,322,705 u=424,434,440 r=93.4 g=5 p=55,24,40 "
           "am=150,-20,120 az=0,0,-1 q=18 e=ESPERA")
# (en la PATADA, duro = lo que duró el GOLPE solo, como manda el robot)
R_BUENA = ("#R n=1 mov=PATADA nota=CASI pico=-6.2 picoms=180 pico200=-5.9 fin=-4.1 fin2=-7.5 "
           "cruces=1 giro=0 gps=0 duro=500 hueco=201 huecoms=340 plantado=0")
V_BUENA = "#V " + " ".join("%s=%s" % kv for kv in VALORES_DE_MENTIRA.items())


def _autoprueba_traductor(verificar):
    def tipo(renglon):
        return interpretar(renglon)["tipo"]

    # --- #S ---
    m = interpretar(S_BUENA)
    verificar(m["tipo"] == "S", "S buena")
    verificar(m["t"] == 12345 and m["e"] == "ESPERA", "S: t y e")
    verificar(m["l"] == [310, 322, 705] and m["u"] == [424, 434, 440], "S: línea y umbrales")
    verificar(m["r"] == 93.4 and m["g"] == 5 and m["q"] == 18, "S: rumbo, estado, cámara")
    verificar(m["p"] == (55, 24, 40) and m["am"] == (150, -20, 120) and m["az"] == (0, 0, -1), "S: pelota y arcos")
    verificar(tipo(S_BUENA + "\r") == "S", "S con \\r al final")
    verificar(tipo(S_BUENA + "\r\n") == "S", "S con \\r\\n al final")
    m = interpretar("#S t=1 l=1,2,3 u=4,5,6 r=-1 g=-1 p=0,0,-1 am=0,0,-1 az=0,0,-1 q=0 e=ESPERA")
    verificar(m["tipo"] == "S" and m["r"] is None and m["g"] is None, "S sin giroscopio")
    verificar(m["p"] == (0, 0, -1) and not visible(m["p"]), "S: pelota nunca vista")
    verificar(interpretar(S_BUENA.replace("r=93.4", "r=-1.0"))["r"] is None, "S: r=-1.0")
    verificar(interpretar(S_BUENA.replace("r=93.4", "r=nan"))["tipo"] == "S", "S: r=nan no rompe el renglón")
    verificar(interpretar(S_BUENA.replace("q=18", "q=17.5"))["q"] == 17.5, "S: q con decimales")
    verificar(interpretar(S_BUENA.replace("e=ESPERA", "e=CUENTA"))["e"] == "CUENTA", "S: e=CUENTA")
    verificar(tipo(S_BUENA + " zz=9") == "S", "S con un campo de más (se ignora)")
    for k in range(len(S_BUENA)):
        verificar(tipo(S_BUENA[:k]) != "S", "S cortada en %d se tomó como buena" % k)
    verificar(tipo(S_BUENA.replace(" e=ESPERA", "")) == "ROTA", "S sin e=")
    verificar(tipo(S_BUENA.replace("e=ESPERA", "e=ESP")) == "ROTA", "S con e= cortado")
    verificar(tipo(S_BUENA.replace("l=310,322,705", "l=310,322")) == "ROTA", "S con 2 sensores")
    verificar(tipo(S_BUENA.replace("l=310", "l=31x")) == "ROTA", "S con basura en un número")
    verificar(tipo(S_BUENA.replace("p=55,24,40", "p=55,24")) == "ROTA", "S con la pelota cortada")
    verificar(tipo(S_BUENA.replace("r=93.4", "r=93,4")) == "ROTA", "S con coma decimal")
    verificar(tipo(S_BUENA[:40] + S_BUENA) == "ROTA", "dos #S pegados")
    verificar(tipo("#S t=1 basura e=ESPERA") == "ROTA", "S con un pedazo sin =")
    verificar(tipo("#S") == "ROTA", "#S vacío")
    verificar(tipo("#S#INICIO 1 PATADA") == "ROTA", "pegados en la etiqueta")

    # --- #HOLA ---
    m = interpretar("#HOLA tablero-delantero 2026-10-27")
    verificar(m["tipo"] == "HOLA" and m["nombre"] == PROGRAMA and m["fecha"] == "2026-10-27", "HOLA")
    verificar(interpretar("#HOLA tablero-delantero")["fecha"] == "", "HOLA sin fecha")
    verificar(tipo("#HOLA") == "ROTA", "HOLA sin nada")

    # --- #V, #OK, #ERROR ---
    m = interpretar("#V VEL_PATADA=150 KP_PATADA=4.0 KD_PATADA=0.60 TRIM_PATADA=-3 LAZO=1")
    verificar(m["tipo"] == "V" and m["valores"] == {"VEL_PATADA": "150", "KP_PATADA": "4.0",
              "KD_PATADA": "0.60", "TRIM_PATADA": "-3", "LAZO": "1"}, "V")
    verificar(len(interpretar(V_BUENA)["valores"]) == len(NUMEROS), "V con todos los números")
    for malo in ("#V", "#V VEL_PATADA=", "#V VEL_PATADA=1,5", "#V vel=3", "#V VEL_PATADA=nan",
                 "#V VEL_PATADA", "#V =3"):
        verificar(tipo(malo) == "ROTA", "V mala: %r" % malo)
    m = interpretar("#OK TRIM_PATADA=7")
    verificar(m["tipo"] == "OK" and m["nombre"] == "TRIM_PATADA" and m["valor"] == "7", "OK")
    verificar(interpretar("#OK KD_PATADA=0.65")["valor"] == "0.65", "OK con decimales")
    for malo in ("#OK TRIM_PATADA", "#OK A=1 B=2", "#OK", "#OK TRIM_PATADA=siete"):
        verificar(tipo(malo) == "ROTA", "OK malo: %r" % malo)
    m = interpretar("#ERROR VEL_PATADA fuera de rango: va de 60 a 255")
    verificar(m["tipo"] == "ERROR" and m["mensaje"] == "VEL_PATADA fuera de rango: va de 60 a 255", "ERROR")
    verificar(interpretar("#ERROR")["mensaje"] == "", "ERROR vacío")

    # --- la prueba: #CUENTA #INICIO #FRENO #M #R #CONSEJO ---
    verificar(interpretar("#CUENTA 1.5")["segundos"] == 1.5, "CUENTA 1.5")
    verificar(interpretar("#CUENTA 0.5")["segundos"] == 0.5, "CUENTA 0.5")
    for malo in ("#CUENTA", "#CUENTA uno", "#CUENTA 1.5 2"):
        verificar(tipo(malo) == "ROTA", "CUENTA mala: %r" % malo)
    m = interpretar("#INICIO 1 PATADA")
    verificar(m["tipo"] == "INICIO" and m["n"] == 1 and m["nombre"] == "PATADA", "INICIO")
    verificar(interpretar("#INICIO 6 ORBITA")["nombre"] == "ORBITA", "INICIO 6")
    for malo in ("#INICIO PATADA", "#INICIO 1", "#INICIO uno PATADA", "#INICIO 1 PATADA X"):
        verificar(tipo(malo) == "ROTA", "INICIO malo: %r" % malo)
    for motivo in ("tecla", "sin giroscopio", "tope de tiempo"):
        verificar(interpretar("#FRENO " + motivo)["motivo"] == motivo, "FRENO " + motivo)
    m = interpretar("#M 10 -0.4 g")
    verificar(m["tipo"] == "M" and m["t"] == 10 and m["grados"] == -0.4 and m["fase"] == "g", "M golpe")
    verificar(interpretar("#M 520 3.1 r")["fase"] == "r", "M retroceso")
    verificar(interpretar("#M 30 12 m")["grados"] == 12.0, "M sin decimales")
    for malo in ("#M 10 -0.4", "#M 10 -0.4 z", "#M 10 nan g", "#M diez 1 g", "#M 10 -0.4 g x", "#M"):
        verificar(tipo(malo) == "ROTA", "M mala: %r" % malo)
    # en ÓRBITA y GIRO el robot manda lo girado en total: pasa de 180
    verificar(interpretar("#M 2990 -1085.4 m")["grados"] == -1085.4, "M de un giro de varias vueltas")
    # un número de 400 cifras (basura): float() no puede, y no tiene que romper nada
    largo = "9" * 400
    for malo in ("#M 10 %s g" % largo, "#M %s 1.0 g" % largo, "#CUENTA " + largo,
                 S_BUENA.replace("r=93.4", "r=" + largo), S_BUENA.replace("t=12345", "t=" + largo),
                 S_BUENA.replace("p=55,24,40", "p=%s,24,40" % largo), "#INICIO %s PATADA" % largo,
                 "#OK TRIM_PATADA=" + largo, "#V VEL_PATADA=" + largo,
                 R_BUENA.replace("pico=-6.2", "pico=-" + largo), R_BUENA.replace("n=1", "n=" + largo)):
        verificar(tipo(malo) == "ROTA", "número de 400 cifras: %r" % malo[:40])
    try:
        leer_numero(largo)
        verificar(False, "leer_numero aceptó 400 cifras")
    except ValueError:
        pass
    verificar(leer_numero("-4294967040.0") == -4294967040.0, "el número más largo del robot")
    m = interpretar(R_BUENA)
    verificar(m["tipo"] == "R" and m["n"] == 1 and m["mov"] == "PATADA" and m["nota"] == "CASI", "R")
    verificar(m["pico"] == -6.2 and m["picoms"] == 180 and m["pico200"] == -5.9, "R: picos")
    verificar(m["fin"] == -4.1 and m["fin2"] == -7.5 and m["cruces"] == 1, "R: fin y cruces")
    verificar(m["duro"] == 500 and m["hueco"] == 201 and m["huecoms"] == 340 and m["plantado"] == 0, "R: tiempos")
    verificar(interpretar(R_BUENA.replace("CASI", "SE_TUERCE"))["nota"] == "SE_TUERCE", "R SE_TUERCE")
    verificar(tipo(R_BUENA[:30]) == "ROTA", "R cortada")
    verificar(tipo(R_BUENA.replace(" nota=CASI", "")) == "ROTA", "R sin nota")
    verificar(tipo(R_BUENA.replace("CASI", "GENIAL")) == "ROTA", "R con nota desconocida")
    verificar(tipo(R_BUENA.replace("pico=-6.2", "pico=abc")) == "ROTA", "R con número roto")
    m = interpretar("#CONSEJO Se fue a la DERECHA: subir TRIM_PATADA de 5 a 6")
    verificar(m["tipo"] == "CONSEJO" and m["mensaje"].startswith("Se fue a la DERECHA"), "CONSEJO")

    # --- #PANEL ---
    codigo = "const int TRIM_PATADA                  = 6;               // compensacion"
    m = interpretar("#PANEL " + codigo)
    verificar(m["tipo"] == "PANEL" and m["codigo"] == codigo, "PANEL respeta los espacios")
    verificar(interpretar("#PANEL const float KD_PATADA = 0.7;")["codigo"] == "const float KD_PATADA = 0.7;", "PANEL")
    verificar(interpretar("#PANEL")["codigo"] == "", "PANEL vacío")
    verificar(tipo("#FINPANEL") == "FINPANEL", "FINPANEL")

    # --- lo que no es del contrato ---
    for humano in ("Hola, soy el tablero", "", "  #S t=1 e=ESPERA", "S t=1 l=1,2,3 e=ESPERA",
                   "=400,410,420 u=424", "PARADO (frena 0,4 s y suelta las ruedas)"):
        verificar(tipo(humano) == "TEXTO", "renglón para personas: %r" % humano)
    verificar(tipo(None) == "TEXTO", "None")
    verificar(tipo("#ZZZ 1 2") == "DESCONOCIDA" and tipo("#") == "DESCONOCIDA", "etiqueta desconocida")
    verificar(tipo("#CONSEJO subir TRIM#S t=1 e=ESPERA") == "ROTA", "CONSEJO pegado con un #S")

    # --- a lo bruto: pedazos al azar de renglones buenos, pegados y cortados ---
    rng = random.Random(7)
    buenos = [S_BUENA, R_BUENA, V_BUENA, "#HOLA tablero-delantero 2026-10-27", "#M 10 -0.4 g",
              "#CUENTA 1.5", "#INICIO 1 PATADA", "#FRENO tecla", "#OK TRIM_PATADA=7",
              "#PANEL const int TRIM_PATADA = 6;", "#FINPANEL", "#CONSEJO subir KD_PATADA",
              "#ERROR no entiendo 'q'", "PARADO", "�� basura"]
    tipos_validos = set(ETIQUETAS) | {"TEXTO", "ROTA", "DESCONOCIDA"}
    for _ in range(3000):
        a, b = rng.choice(buenos), rng.choice(buenos)
        renglon = a[rng.randint(0, len(a)):] + b[:rng.randint(0, len(b))]
        verificar(interpretar(renglon)["tipo"] in tipos_validos, "a lo bruto: %r" % renglon)


def _autoprueba_memoria(verificar):
    e = Estado()
    verificar(e.modo == "ESPERA" and e.hola is None, "memoria nueva")
    c = e.aplicar(interpretar("#HOLA tablero-delantero 2026-10-27"))
    verificar("hola" in c and e.hola == (PROGRAMA, "2026-10-27"), "memoria: HOLA")
    e.aplicar(interpretar(V_BUENA))
    verificar(e.valores["TRIM_PATADA"] == VALORES_DE_MENTIRA["TRIM_PATADA"] and e.valores["KP_PATADA"] == VALORES_DE_MENTIRA["KP_PATADA"], "memoria: V")
    c = e.aplicar(interpretar(S_BUENA), 10.0)
    verificar("sensores" in c and e.t_sensores == 10.0 and e.sensores["q"] == 18, "memoria: S")
    verificar(e.aplicar(interpretar("#S t=1 basura")) == {"rota"} and e.rotas == 1, "memoria: S rota no pisa")
    verificar(e.sensores["t"] == 12345, "memoria: la S rota no se usó")
    # una prueba completa
    c = e.aplicar(interpretar("#CUENTA 1.5"))
    verificar(e.modo == "CUENTA" and e.cuenta == 1.5 and "modo" in c, "memoria: CUENTA")
    e.aplicar(interpretar("#CUENTA 1.0"))
    e.aplicar(interpretar("#CUENTA 0.5"))
    verificar(e.cuenta == 0.5, "memoria: cuenta baja")
    e.aplicar(interpretar("#INICIO 1 PATADA"))
    verificar(e.modo == "MOVIENDO" and e.mov_nombre == "PATADA", "memoria: INICIO")
    for renglon in ("#M 10 -0.4 g", "#M 20 -1.2 g", "#M 510 -3.0 r"):
        e.aplicar(interpretar(renglon))
    verificar(e.muestras == [(10, -0.4, "g"), (20, -1.2, "g"), (510, -3.0, "r")], "memoria: M")
    c = e.aplicar(interpretar(R_BUENA))
    verificar(e.modo == "ESPERA" and e.informe["nota"] == "CASI" and "resultado" in c, "memoria: R")
    e.aplicar(interpretar("#CONSEJO subir TRIM_PATADA"))
    e.aplicar(interpretar("#CONSEJO subir KD_PATADA"))
    verificar(e.consejos == ["subir TRIM_PATADA", "subir KD_PATADA"], "memoria: CONSEJO")
    # otra cuenta borra el resultado viejo; frenada en la cuenta vuelve a espera
    e.aplicar(interpretar("#CUENTA 1.5"))
    verificar(e.muestras == [] and e.informe is None and e.consejos == [], "memoria: cuenta nueva borra")
    c = e.aplicar(interpretar("#FRENO tecla"))
    verificar(e.modo == "ESPERA" and e.freno == "tecla" and "modo" in c, "memoria: FRENO en la cuenta")
    # frenado moviéndose: sigue MOVIENDO hasta el #R
    e.aplicar(interpretar("#CUENTA 1.5"))
    e.aplicar(interpretar("#INICIO 7 GIRO"))
    e.aplicar(interpretar("#FRENO tecla"))
    verificar(e.modo == "MOVIENDO" and e.freno == "tecla", "memoria: FRENO moviéndose")
    e.aplicar(interpretar("#R n=7 mov=GIRO nota=NO_VALE pico=0 picoms=0 pico200=0 fin=0 fin2=0 cruces=0 "
                          "giro=12.0 gps=40.0 duro=300 hueco=3 huecoms=10 plantado=0"))
    verificar(e.modo == "ESPERA" and e.informe["nota"] == "NO_VALE", "memoria: R después del FRENO")
    # un #S en espera también saca de MOVIENDO (si se perdió el #R)
    e.modo = "MOVIENDO"
    e.aplicar(interpretar(S_BUENA))
    verificar(e.modo == "ESPERA", "memoria: S en espera")
    # #OK
    c = e.aplicar(interpretar("#OK TRIM_PATADA=7"))
    verificar(e.valores["TRIM_PATADA"] == "7" and e.ultimo_ok == ("TRIM_PATADA", "7") and "ok" in c, "memoria: OK")
    c = e.aplicar(interpretar("#ERROR no entiendo 'q'"))
    verificar(e.ultimo_error == "no entiendo 'q'" and "error" in c, "memoria: ERROR")
    # #PANEL ... #FINPANEL, dos tandas
    e.aplicar(interpretar("#PANEL const int TRIM_PATADA = 7;"))
    e.aplicar(interpretar("#PANEL const float KD_PATADA = 0.7;"))
    c = e.aplicar(interpretar("#FINPANEL"))
    verificar("panel_listo" in c and e.panel == ["const int TRIM_PATADA = 7;", "const float KD_PATADA = 0.7;"], "memoria: PANEL")
    e.aplicar(interpretar("#PANEL const int VEL_PATADA = 160;"))
    e.aplicar(interpretar("#FINPANEL"))
    verificar(e.panel == ["const int VEL_PATADA = 160;"], "memoria: PANEL nuevo pisa al viejo")
    c = e.aplicar(interpretar("#FINPANEL"))
    verificar("panel_listo" in c and e.panel == [], "memoria: FINPANEL solo = nada para copiar")
    # HOLA en medio de un movimiento: el robot se reinició
    e.modo = "MOVIENDO"
    e.aplicar(interpretar("#HOLA tablero-delantero 2026-10-27"))
    verificar(e.modo == "ESPERA", "memoria: HOLA vuelve a espera")
    e.reiniciar_conexion()
    verificar(e.hola is None and e.sensores is None and e.informe is not None, "memoria: reconectar guarda el resultado")

    # "#FRENO sin giroscopio" llega EN ESPERA, sin #CUENTA antes: borra la prueba vieja
    e = Estado()
    for renglon in ("#CUENTA 1.5", "#INICIO 2 AVANCE", "#M 10 -0.4 m", "#M 20 -1.0 m",
                    "#R n=2 mov=AVANCE nota=PROLIJO pico=-1.0 picoms=20 pico200=-1.0 fin=-1.0 fin2=0 "
                    "cruces=0 giro=0 gps=0 duro=800 hueco=3 huecoms=10 plantado=0",
                    "#CONSEJO El avance va derecho."):
        e.aplicar(interpretar(renglon))
    verificar(e.informe["nota"] == "PROLIJO" and e.modo == "ESPERA", "memoria: un AVANCE prolijo")
    c = e.aplicar(interpretar("#FRENO sin giroscopio"))
    verificar(e.informe is None and e.muestras == [] and e.consejos == [] and e.mov_nombre is None
              and e.freno == "sin giroscopio" and e.modo == "ESPERA" and "resultado" in c,
              "memoria: FRENO sin giroscopio en espera borra la nota vieja")
    # se reconectó en medio de un movimiento: llegan #M y #R sin #INICIO
    e = Estado()
    for renglon in ("#CUENTA 1.5", "#INICIO 1 PATADA", "#M 10 -0.4 g", R_BUENA):
        e.aplicar(interpretar(renglon))
    e.reiniciar_conexion()
    e.aplicar(interpretar("#M 10 5.0 m"))
    verificar(e.informe is None and e.muestras == [(10, 5.0, "m")], "memoria: #M sin #INICIO no se mezcla con lo viejo")
    e.aplicar(interpretar("#M 20 6.0 m"))
    e.aplicar(interpretar("#R n=7 mov=GIRO nota=NO_ARRANCA pico=0 picoms=0 pico200=0 fin=0 fin2=0 "
                          "cruces=0 giro=6.0 gps=300.0 duro=20 hueco=3 huecoms=10 plantado=0"))
    verificar(len(e.muestras) == 2 and e.mov_nombre == "GIRO", "memoria: #R sin #INICIO")


def _autoprueba_cuentas(verificar):
    verificar(describir_objeto("Pelota", (55, 24, 40)) == ("Pelota a 60 cm, 24 grados a la izquierda", "fresca"),
              "frase de la pelota: %r" % (describir_objeto("Pelota", (55, 24, 40)),))
    verificar(describir_objeto("Pelota", (55, -24, 40))[0] == "Pelota a 60 cm, 24 grados a la derecha", "pelota a la derecha")
    verificar(describir_objeto("Pelota", (60, 0, 10))[0] == "Pelota a 60 cm, derecho adelante", "pelota adelante")
    frase, como = describir_objeto("Pelota", (55, 24, 1200))
    verificar(como == "vieja" and frase.startswith("No la veo") and "1.2 s" in frase, "pelota vieja: %r" % frase)
    verificar(describir_objeto("Pelota", (0, 0, -1)) == ("No la veo", "nunca"), "pelota nunca vista")
    verificar(describir_objeto("Pelota", None) == ("No la veo", "nunca"), "sin dato")
    verificar(describir_objeto("Arco azul", None, False)[0] == "No lo veo", "arco: masculino")
    px, py = cm_a_pantalla(100, 50, 400, 500, 2)
    verificar((px, py) == (300, 300), "cm a pantalla")
    verificar(cm_a_pantalla(0, 10, 400, 500, 2)[0] < 400, "Y positivo = IZQUIERDA en la pantalla")
    verificar(cm_a_pantalla(10, 0, 400, 500, 2)[1] < 500, "X = para ARRIBA en la pantalla")
    for actual, paso, signo, esperado in (("0.6", 0.1, 1, "0.7"), ("0.60", 0.1, 1, "0.70"), ("5", 1, -1, "4"),
                                          ("4.0", 0.5, 1, "4.5"), ("150", 5, 1, "155"), ("0", 1, -1, "-1"),
                                          ("0.1", 0.1, -1, "0.0"), ("0.65", 0.1, -1, "0.55"),
                                          ("3000", 500, 1, "3500"), ("4", 0.5, -1, "3.5")):
        verificar(nuevo_valor(actual, paso, signo) == esperado,
                  "nuevo_valor(%r, %r, %r) = %r" % (actual, paso, signo, nuevo_valor(actual, paso, signo)))
    for escrito, esperado in (("0,7", "0.7"), (" 12 ", "12"), ("-3", "-3"), (".5", ".5"),
                              ("abc", None), ("", None), ("1.2.3", None), ("7;", None), ("1 2", None),
                              ("9" * 400, None)):
        verificar(normalizar_entrada(escrito) == esperado, "normalizar_entrada(%r)" % escrito[:20])
    verificar(desenrollar([170, -170, -150]) == [170, 190, 210], "desenrollar una vuelta")
    verificar(desenrollar([0, 100, 200, 300, 400]) == [0, 100, 200, 300, 400]
              and desenrollar([-1000.0, -1050.0, -1085.4]) == [-1000.0, -1050.0, -1085.4],
              "desenrollar no toca lo girado en total")
    verificar(desenrollar([0, 1, -1]) == [0, 1, -1], "desenrollar sin vuelta")
    verificar(desenrollar([-170, 170]) == [-170, -190], "desenrollar para el otro lado")
    verificar(escala_grados(0) == 5 and escala_grados(3) == 5 and escala_grados(9.5) == 15, "escala del gráfico")
    verificar(escala_grados(700) == 1080 and escala_grados(5000) == 5040, "escala del gráfico, muchas vueltas")
    verificar(paso_grilla(5) == 1 and paso_grilla(15) == 5 and paso_tiempo(1200) == 200, "pasos del gráfico")
    verificar(puede_mover(True, True, True, "ESPERA") == (True, ""), "se puede mover")
    verificar(not puede_mover(False, True, True, "ESPERA")[0], "sin conexión no se mueve")
    verificar(not puede_mover(True, False, True, "ESPERA")[0], "si no es el delantero no se mueve")
    verificar(not puede_mover(True, True, False, "ESPERA")[0], "sin el programa no se mueve")
    verificar(not puede_mover(True, True, True, "CUENTA")[0], "en la cuenta no se pide otro")
    verificar(not puede_mover(True, True, True, "MOVIENDO")[0], "moviéndose no se pide otro")
    verificar(not puede_mover(True, True, True, "ESPERA", True)[0], "con un pedido pendiente no se pide otro")
    verificar(lado(-3.24) == "3.2 grados a la DERECHA" and lado(2) == "2.0 grados a la IZQUIERDA", "lado")
    frases = explicar_informe(interpretar(R_BUENA))
    todo = " ".join(f for f, _ in frases)
    verificar("DERECHA" in todo and "retroceso" in todo and "200 ms" in todo, "explicar la patada")
    verificar("El golpe duró 500 ms" in todo and "MS_PATADA" in todo, "explicar la patada: duro es el golpe")
    verificar(any(estilo == "malo" and "CLAVÓ" in f for f, estilo in frases), "explicar el hueco")
    giro = interpretar("#R n=6 mov=ORBITA nota=SE_PLANTA pico=0 picoms=0 pico200=0 fin=0 fin2=0 cruces=0 "
                       "giro=540.0 gps=180.0 duro=3000 hueco=4 huecoms=20 plantado=620")
    todo = " ".join(f for f, _ in explicar_informe(giro))
    verificar("540 grados" in todo and "180 grados por segundo" in todo and "620 ms" in todo, "explicar la órbita")
    verificar("Duró 3000 ms" in todo and "golpe" not in todo, "explicar la órbita: duro es todo")
    verificar(contar_cruces([0, 3, -3, 3, 1, -1]) == 2 and contar_cruces([1, -1, 1.9]) == 0, "cruces")
    verificar(nota_derecho(3, 1, 2) == "PROLIJO" and nota_derecho(5, 1, 2) == "CASI", "nota PROLIJO / CASI")
    verificar(nota_derecho(3, 4, 2) == "ZIGZAGUEA" and nota_derecho(12, 0, 12) == "SE_TUERCE", "nota ZIGZAG / TUERCE")
    verificar(nota_derecho(3, 1, 5) == "CASI" and nota_derecho(-10, 1, -2) == "SE_TUERCE", "nota con final torcido")
    verificar(nota_giro(10, 0) == "NO_ARRANCA" and nota_giro(300, 500) == "SE_PLANTA"
              and nota_giro(300, 100) == "GIRA_BIEN", "nota de los giros")
    # la tabla del contrato
    for m in MOVIMIENTOS:
        for nombre in m["numeros"]:
            verificar(nombre in NUMEROS and nombre in VALORES_DE_MENTIRA and nombre in RANGOS_DE_MENTIRA,
                      "falta %s en las tablas" % nombre)
    verificar(set(NUMEROS) == set(VALORES_DE_MENTIRA) == set(RANGOS_DE_MENTIRA), "tablas iguales")


def _autoprueba_robot_de_mentira(verificar):
    # Lo que inventa para cada movimiento cumple el contrato.
    for n in MOV_POR_NUMERO:
        for cortado in (None, 150):
            renglones = demo_resultado(n, VALORES_DE_MENTIRA, random.Random(n), cortado)
            tipos = [interpretar(r)["tipo"] for r in renglones]
            verificar("ROTA" not in tipos and set(tipos) <= {"M", "R", "CONSEJO"},
                      "mentira %d: renglones fuera del contrato %r" % (n, renglones[-3:]))
            verificar(tipos.count("R") == 1 and tipos.count("M") <= 300, "mentira %d: un solo #R" % n)
            r = interpretar([x for x in renglones if x.startswith("#R")][0])
            verificar(r["mov"] == MOV_POR_NUMERO[n]["nombre"], "mentira %d: nombre" % n)
            verificar((r["nota"] == "NO_VALE") == (cortado is not None), "mentira %d: NO_VALE si la frenan" % n)
    # Con TRIM 7 la patada de mentira sale PROLIJA; con TRIM 0 se va a la derecha.
    valores = dict(VALORES_DE_MENTIRA, TRIM_PATADA="7", SIN_ESPERA="1")
    r = interpretar([x for x in demo_resultado(1, valores, random.Random(1)) if x.startswith("#R")][0])
    verificar(r["nota"] == "PROLIJO" and r["hueco"] < 50, "mentira: patada prolija %r" % r)
    renglones = demo_resultado(1, dict(VALORES_DE_MENTIRA, TRIM_PATADA="0"), random.Random(1))
    r = interpretar([x for x in renglones if x.startswith("#R")][0])
    verificar(r["pico"] < -4 and any("subir TRIM_PATADA" in x for x in renglones), "mentira: TRIM bajo")
    r = interpretar([x for x in demo_resultado(7, dict(VALORES_DE_MENTIRA, VEL_GIRO_CONTINUO="30"),
                                               random.Random(1)) if x.startswith("#R")][0])
    verificar(r["nota"] == "NO_ARRANCA", "mentira: giro sin fuerza")
    r = interpretar([x for x in demo_resultado(6, dict(VALORES_DE_MENTIRA, VEL_ORB_TRASERA="50"),
                                               random.Random(1)) if x.startswith("#R")][0])
    verificar(r["nota"] == "SE_PLANTA", "mentira: órbita que se planta %r" % r)
    renglones = demo_resultado(6, VALORES_DE_MENTIRA, random.Random(1))
    r = interpretar([x for x in renglones if x.startswith("#R")][0])
    verificar(r["nota"] == "GIRA_BIEN", "mentira: órbita normal %r" % r)
    # como el robot de verdad: la órbita manda lo girado EN TOTAL, sin enrollar
    ultima = interpretar([x for x in renglones if x.startswith("#M")][-1])
    verificar(r["giro"] > 180 and abs(abs(ultima["grados"]) - r["giro"]) < 1,
              "mentira: la órbita manda el total (%r, %r)" % (ultima["grados"], r["giro"]))
    # en la patada, duro = lo que duró el GOLPE (MS_PATADA), no golpe + retroceso
    r = interpretar([x for x in demo_resultado(1, VALORES_DE_MENTIRA, random.Random(1)) if x.startswith("#R")][0])
    verificar(r["duro"] == int(VALORES_DE_MENTIRA["MS_PATADA"]), "mentira: duro de la patada %r" % r["duro"])

    # Las órdenes, sin hilos.
    cola = queue.Queue()
    robot = RobotDeMentira(cola)

    def dijo(orden):
        robot.comando(orden)
        salida = []
        while not cola.empty():
            salida.append(cola.get()[1])
        return salida

    verificar(dijo("h") == ["#HOLA tablero-delantero 2026-10-27"], "mentira: h")
    salida = dijo("?")
    verificar(interpretar(salida[0])["valores"] == VALORES_DE_MENTIRA, "mentira: ? manda #V")
    salida = dijo("=TRIM_PATADA 7")
    verificar(salida[0] == "#OK TRIM_PATADA=7" and interpretar(salida[1])["valores"]["TRIM_PATADA"] == "7", "mentira: =")
    verificar(dijo("=KP_PATADA 4.5")[0] == "#OK KP_PATADA=4.5", "mentira: = con decimales")
    for orden in ("=NO_EXISTE 3", "=VEL_PATADA 999", "=KD_PATADA 0,7", "=TRIM_PATADA 1.5", "=TRIM_PATADA", "zz", "!9",
                  "=VEL_PATADA 40", "=MS_RETROCESO_LINEA 350", "=MS_PRUEBA_ORBITA 5000", "=MS_PRUEBA_GIRO 3500"):
        verificar(interpretar(dijo(orden)[0])["tipo"] == "ERROR", "mentira: %r tendría que dar #ERROR" % orden)
    salida = dijo("v")
    verificar(salida == ["#PANEL const int TRIM_PATADA = 7;", "#PANEL const float KP_PATADA = 4.5;", "#FINPANEL"],
              "mentira: v %r" % salida)
    salida = dijo("0")
    verificar(interpretar(salida[-1])["valores"] == VALORES_DE_MENTIRA, "mentira: 0")
    verificar(dijo("v") == ["#FINPANEL"], "mentira: v sin cambios")
    verificar(interpretar(robot.linea_S())["tipo"] == "S", "mentira: #S")
    # una prueba de verdad (dura ~1.7 s) y una frenada en la cuenta
    salida = dijo("!3")
    tipos = [interpretar(x)["tipo"] for x in salida]
    verificar(tipos[:4] == ["CUENTA", "CUENTA", "CUENTA", "INICIO"] and "R" in tipos and "ROTA" not in tipos,
              "mentira: !3 %r" % tipos[:6])
    robot.enviar("x")
    salida = dijo("!1")
    verificar(salida == ["#CUENTA 1.5", "#FRENO tecla"], "mentira: frenado en la cuenta %r" % salida)


def _autoprueba_ventana(verificar):
    """La ventana entera con un tkinter DE MENTIRA (unittest.mock): no se
    abre nada en la pantalla. Sirve para encontrar errores de tipeo en la
    parte de la ventana, que si no solo aparecerían con el robot enchufado."""
    from unittest import mock
    g = globals()
    guardados = {k: g[k] for k in ("tk", "ttk", "messagebox")}
    try:
        g["tk"] = mock.MagicMock(name="tk")
        g["ttk"] = mock.MagicMock(name="ttk")
        g["messagebox"] = mock.MagicMock(name="messagebox")
        g["messagebox"].askyesno.return_value = True
        raiz = mock.MagicMock(name="raiz")
        cable = _CableDePrueba()
        v = Ventana(raiz, cable, queue.Queue(), estricto=True)

        def llegan(*renglones):
            v._procesar_lote([("linea", r) for r in renglones])

        v._procesar_lote([("buscando",)])
        v._procesar_lote([("ocupado", "COM9", "Acceso denegado")])
        v._pintar_cuenta()           # (todos los Label de mentira son el mismo: se mira el último)
        verificar("está ocupado" in str(v.lbl_cuenta.configure.call_args),
                  "ventana: al lado de PROBAR dice por qué está apagado (ocupado)")
        v._parar()
        verificar(cable.mandados == [], "ventana: sin conexión no manda nada")
        # otra Teensy (su puerto ni se abrió): cartel rojo y no le manda nada
        v._procesar_lote([("otro", "COM3", "16000000")])
        v._pintar_conexion()
        v._probar()
        v._parar()
        verificar(cable.mandados == [] and v.situacion == "otro" and v.ultimo_aviso[1] == ROJO,
                  "ventana: otra Teensy, cartel rojo y nada mandado %r" % (v.ultimo_aviso,))
        # AL CONECTAR NO MANDA NADA: no sabe qué programa tiene el robot
        v._procesar_lote([("conectado", "COM9", SERIE_DELANTERO)])
        verificar(cable.mandados == [], "ventana: al conectar no manda nada %r" % cable.mandados)
        llegan("algo para personas", "#ZZZ", "#S t=1 basura")
        verificar(cable.mandados == [], "ventana: sin #S ni #HOLA sigue sin mandar nada %r" % cable.mandados)
        v._pintar_cuenta()
        verificar("tablero" in str(v.lbl_cuenta.configure.call_args),
                  "ventana: al lado de PROBAR dice por qué está apagado (sin programa)")
        v._probar()
        verificar(cable.mandados == [], "ventana: sin #HOLA no mueve")
        v.t_conexion -= 10
        v._latido()
        verificar(v.aviso_programa, "ventana: avisa que no contesta como tablero")
        verificar(NO_OBEDECE_PARAR in v._texto_programa()[0], "ventana: arriba avisa que PARAR no lo frena")
        # sin el programa del tablero: PARAR manda x igual, pero avisa en rojo
        v._parar()
        verificar(cable.mandados == ["x"] and v.ultimo_aviso == ("Mandé x, pero OJO: " + NO_OBEDECE_PARAR, ROJO),
                  "ventana: PARAR sin tablero avisa %r" % (v.ultimo_aviso,))
        # ...y la consola solo deja preguntar (h y ?)
        for escrito in ("!1", "=TRIM_PATADA 7", "x", "0", "v"):
            v.ent_consola.get.return_value = escrito
            v._consola_mandar()
        verificar(cable.mandados == ["x"], "ventana: la consola sin tablero no manda órdenes %r" % cable.mandados)
        v.ent_consola.get.return_value = "h"
        v._consola_mandar()
        v.ent_consola.get.return_value = "?"
        v._consola_mandar()
        verificar(cable.mandados == ["x", "h", "?"], "ventana: la consola sin tablero deja h y ? %r" % cable.mandados)
        del cable.mandados[:]
        # el primer #S (solo lo manda el tablero, y solo en espera): recién ahí pregunta
        llegan(S_BUENA)
        verificar(cable.mandados == ["h", "?"] and v.es_tablero, "ventana: con el primer #S manda h y ? %r" % cable.mandados)
        llegan(S_BUENA, S_BUENA)
        verificar(cable.mandados == ["h", "?"], "ventana: no pregunta h a cada rato %r" % cable.mandados)
        v.t_pide_hola -= 2.5
        llegan(S_BUENA)
        verificar(cable.mandados == ["h", "?", "h"], "ventana: sin #HOLA vuelve a pedir h a los 2 s %r" % cable.mandados)
        llegan("#HOLA tablero-delantero 2026-10-27", V_BUENA, S_BUENA, "algo para personas")
        verificar(v._programa_ok(), "ventana: programa OK")
        v.t_pide_hola -= 10
        llegan(S_BUENA)
        verificar(cable.mandados == ["h", "?", "h"], "ventana: con #HOLA no pregunta más %r" % cable.mandados)
        v._pintar_cuenta()
        verificar("Listo" in str(v.lbl_cuenta.configure.call_args), "ventana: con todo bien dice Listo")
        v._probar()
        verificar(cable.mandados[-1] == "!1", "ventana: PROBAR manda !1")
        v._probar()
        verificar(cable.mandados.count("!1") == 1, "ventana: doble clic no lo manda dos veces")
        llegan("#CUENTA 1.5", "#CUENTA 1.0")
        v._probar()
        v._paso("TRIM_PATADA", +1)
        verificar(cable.mandados.count("!1") == 1 and "=TRIM_PATADA 6" not in cable.mandados,
                  "ventana: en la cuenta no manda nada más")
        v._parar()
        verificar(cable.mandados[-1] == "x", "ventana: PARAR manda x")
        llegan("#CUENTA 0.5", "#INICIO 1 PATADA")
        llegan(*demo_resultado(1, VALORES_DE_MENTIRA, random.Random(3)))
        verificar(v.estado.modo == "ESPERA" and v.estado.informe is not None, "ventana: terminó la patada")
        v._paso("TRIM_PATADA", +1)
        verificar(cable.mandados[-1] == "=TRIM_PATADA 6", "ventana: [+] %r" % cable.mandados[-1])
        v._paso("KD_PATADA", -1)
        verificar(cable.mandados[-1] == "=KD_PATADA 0.5", "ventana: [-] con decimales %r" % cable.mandados[-1])
        llegan("#OK TRIM_PATADA=6", "#ERROR VEL_PATADA fuera de rango: va de 60 a 255")
        v.filas["KP_PATADA"]["entrada"].get.return_value = "4,5"
        v._poner("KP_PATADA")
        verificar(cable.mandados[-1] == "=KP_PATADA 4.5", "ventana: la cajita con coma %r" % cable.mandados[-1])
        v.filas["KP_PATADA"]["entrada"].get.return_value = "cuatro"
        antes = len(cable.mandados)
        v._poner("KP_PATADA")
        verificar(len(cable.mandados) == antes, "ventana: la cajita con letras no manda")
        v._mandar_valor("LAZO", "0")
        verificar(cable.mandados[-1] == "=LAZO 0", "ventana: interruptor")
        for n in MOV_POR_NUMERO:
            v._elegir(n)
            renglones = demo_resultado(n, VALORES_DE_MENTIRA, random.Random(n))
            llegan("#CUENTA 1.5", "#INICIO %d %s" % (n, MOV_POR_NUMERO[n]["nombre"]), *renglones)
            v._dibujar_grafico()
        v._elegir(7)
        v._paso("VEL_GIRO_CONTINUO", -1)
        verificar(cable.mandados[-1] == "=VEL_GIRO_CONTINUO 35", "ventana: [-] del giro")
        v._copiar_panel()
        verificar(cable.mandados[-1] == "v", "ventana: Copiar al panel manda v")
        llegan("--- PARA COPIAR ---", "#PANEL const int TRIM_PATADA = 6;", "#FINPANEL")
        verificar(raiz.clipboard_append.called, "ventana: copió al portapapeles")
        v._copiar_panel()
        llegan("#FINPANEL")
        v._volver_partido()
        verificar(cable.mandados[-1] == "0", "ventana: volver a los del partido manda 0")
        v.ent_consola.get.return_value = "?"
        v._consola_mandar()
        verificar(cable.mandados[-1] == "?", "ventana: la consola manda")
        # los dibujos, con datos y sin datos
        llegan(S_BUENA, S_BUENA.replace("p=55,24,40", "p=55,24,2000"),
               "#S t=1 l=1,2,3 u=4,5,6 r=-1 g=-1 p=0,0,-1 am=0,0,-1 az=0,0,-1 q=0 e=ESPERA",
               "#S t=1 l=1", "#ZZZ", "#M basura")
        for estado_s in (v.estado.sensores, interpretar(S_BUENA), None,
                         interpretar(S_BUENA.replace("g=5", "g=3").replace("am=150,-20,120", "am=150,-20,900"))):
            v.estado.sensores = estado_s
            v._dibujar_pelota()
            v._textos_pelota()
            v._dibujar_lineas()
            v._dibujar_brujula()
            v._textos_sensores()
        v.estado.muestras = [(10, 1.0, "g")]
        v._dibujar_grafico()
        v.estado.muestras = []
        v._pintar_resultado()
        llegan("#CUENTA 1.5", "#FRENO tecla")
        llegan("#CUENTA 1.5", "#INICIO 2 AVANCE")
        v._pintar_resultado()
        v.t_ultima_linea -= 30
        v._latido()
        verificar(v.estado.modo == "ESPERA", "ventana: si el robot no contesta, vuelve sola")
        v._refrescar({"conexion", "hola", "valores", "sensores", "modo", "resultado", "freno", "ok",
                      "error", "panel_listo", "rota"})
        v._consola_borrar()
        # se desenchufa: no manda nada
        v._procesar_lote([("desconectado", "COM9")])
        antes = len(cable.mandados)
        v._parar()
        v._probar()
        verificar(len(cable.mandados) == antes, "ventana: desenchufado no manda")
        # se reconecta EN MEDIO de un movimiento: no le manda nada (el robot
        # sigue hasta terminar). Cuando termina y vuelve el #S, pregunta "h".
        v._procesar_lote([("conectado", "COM9", SERIE_DELANTERO)])
        llegan(*demo_resultado(6, VALORES_DE_MENTIRA, random.Random(6)))
        verificar(len(cable.mandados) == antes and v.estado.informe["mov"] == "ORBITA"
                  and v.estado.mov_nombre == "ORBITA",
                  "ventana: reconectado en medio de un movimiento, no manda nada %r" % cable.mandados[antes:])
        llegan(S_BUENA)
        verificar(cable.mandados[antes:] == ["h", "?"], "ventana: reconectado, pide h con el #S %r" % cable.mandados[antes:])
        llegan("#HOLA tablero-delantero 2026-10-27")
        verificar(v._puede()[0], "ventana: reconectado, los botones vuelven sin recargar nada")
        v._procesar_lote([("desconectado", "COM9")])
        antes = len(cable.mandados)
        # otro robot: NUNCA le manda nada (ni h ni ?): en el arquero "1" arranca un movimiento
        v._procesar_lote([("conectado", "COM3", "16000000")])
        llegan("#HOLA tablero-delantero 2026-10-27")
        v._parar()
        v._probar()
        v._paso("TRIM_PATADA", 1)
        v.ent_consola.get.return_value = "!1"
        v._consola_mandar()
        verificar(len(cable.mandados) == antes, "ventana: al que no es el delantero no le manda nada")
        v._pintar_conexion()
        v._procesar_lote([("conectado", "COM9", SERIE_DELANTERO)])
        llegan("#HOLA tablero-delantero 2026-10-27")
        v._cerrar()
        verificar(cable.mandados[-1] == "x" and raiz.destroy.called, "ventana: al cerrar manda x")
        # la del robot de mentira, y una falla que NO tiene que tirar abajo la ventana
        v2 = Ventana(mock.MagicMock(name="raiz2"), _CableDePrueba(), queue.Queue(), demo=True)
        v2._procesar_lote([("conectado", "DEMO", SERIE_DELANTERO)])
        anotar = g["guardar_falla"]
        g["guardar_falla"] = lambda texto: None
        try:
            v2._procesar_lote([("tipo-que-no-existe",), ("linea",)])     # el segundo rompe
        finally:
            g["guardar_falla"] = anotar
    finally:
        g.update(guardados)


def _autoprueba_cable(verificar):
    """El hilo del USB con un puerto DE MENTIRA (no abre ningún USB): elige
    al delantero aunque haya otro robot, corta los renglones, se da cuenta
    cuando se desenchufa y avisa si el puerto está ocupado."""

    class Puerto:
        def __init__(self, device, serie):
            self.device, self.vid, self.pid, self.serial_number = device, VID_TEENSY, PID_SERIE, serie

    class SerieDeMentira:
        aperturas = []

        def __init__(self, puerto, baudios, timeout=None, write_timeout=None):
            SerieDeMentira.aperturas.append(puerto)
            if len(SerieDeMentira.aperturas) > 1:
                raise OSError("Acceso denegado")          # la segunda vez: "ocupado"
            self.por_leer = (b"basura del medio\n#HOLA tablero-delantero 2026-10-27\r\n"
                             + S_BUENA.encode() + b"\n#M 10 -0.4 g\n#M 20 -0")   # el último quedó cortado
            self.escrito = b""
            SerieDeMentira.ultima = self

        @property
        def in_waiting(self):
            return len(self.por_leer)

        def read(self, n):
            if not self.por_leer:
                time.sleep(0.05)
                if self.escrito:                          # ya le mandaron algo: "se desenchufa"
                    raise OSError("se desenchufó")
                return b""
            trozo, self.por_leer = self.por_leer[:min(n, 7)], self.por_leer[min(n, 7):]   # de a pedacitos
            return trozo

        def write(self, datos):
            self.escrito += datos

        def flush(self):
            pass

        def close(self):
            pass

    class ModuloSerie:
        Serial = SerieDeMentira

    class ModuloPuertos:
        @staticmethod
        def comports():
            return [Puerto("COM3", "16000000"), Puerto("COM9", SERIE_DELANTERO)]

    g = globals()
    guardados = {k: g[k] for k in ("serial", "list_ports")}
    g["serial"], g["list_ports"] = ModuloSerie, ModuloPuertos
    cola = queue.Queue()
    cable = Conexion(cola)
    try:
        cable.enviar("!1")
        verificar(cable._salida.empty(), "cable: sin conexión no guarda órdenes")
        cable.arrancar()
        recibido = []
        fin = time.monotonic() + 5
        while time.monotonic() < fin and not any(m[0] == "ocupado" for m in recibido):
            try:
                m = cola.get(timeout=0.1)
            except queue.Empty:
                continue
            recibido.append(m)
            if m[0] == "linea" and m[1].startswith("#M 10"):
                verificar(cable.enviar("h"), "cable: enviar conectado")
    finally:
        cable.cerrar()
        g.update(guardados)
    verificar(recibido[0] == ("conectado", "COM9", SERIE_DELANTERO), "cable: elige al delantero %r" % (recibido[:1],))
    verificar("COM3" not in SerieDeMentira.aperturas, "cable: NUNCA abre el otro robot %r" % SerieDeMentira.aperturas)
    renglones = [m[1] for m in recibido if m[0] == "linea"]
    verificar(renglones == ["basura del medio", "#HOLA tablero-delantero 2026-10-27", S_BUENA, "#M 10 -0.4 g"],
              "cable: renglones %r" % renglones)
    verificar(SerieDeMentira.ultima.escrito == b"h\n", "cable: escribió %r" % SerieDeMentira.ultima.escrito)
    tipos = [m[0] for m in recibido]
    verificar("desconectado" in tipos and tipos[-1] == "ocupado", "cable: desenchufe y ocupado %r" % tipos)
    cable._salida.put("!1")
    cable._vaciar_salida()
    verificar(cable._salida.empty(), "cable: lo pendiente se tira al desenchufar")

    # Solo hay OTRA Teensy (por ejemplo el arquero): avisa, NO le abre el
    # puerto (el otro equipo lo necesita para su monitor y para cargar) y
    # sigue buscando al delantero.
    class SoloElOtro:
        @staticmethod
        def comports():
            return [Puerto("COM3", "16000000")]

    SerieDeMentira.aperturas = []
    g["serial"], g["list_ports"] = ModuloSerie, SoloElOtro
    cola = queue.Queue()
    cable = Conexion(cola)
    try:
        verificar(cable._elegir_puerto() == (None, ("COM3", "16000000")), "cable: la otra Teensy no se elige")
        cable.arrancar()
        recibido = []
        fin = time.monotonic() + 3
        while time.monotonic() < fin and len(recibido) < 1:
            try:
                recibido.append(cola.get(timeout=0.1))
            except queue.Empty:
                pass
        time.sleep(1.2)                         # que busque un par de veces más
    finally:
        cable.cerrar()
        g.update(guardados)
    while not cola.empty():
        recibido.append(cola.get())
    verificar(recibido == [("otro", "COM3", "16000000")], "cable: avisa una sola vez del otro %r" % recibido)
    verificar(SerieDeMentira.aperturas == [], "cable: al otro no le abre el puerto %r" % SerieDeMentira.aperturas)
    verificar(cable._sp is None and not cable.enviar("!1"), "cable: al otro no le manda nada")
    # con --puerto se abre el que pidieron (lo eligió una persona, a mano)
    g["list_ports"] = SoloElOtro
    try:
        verificar(Conexion(queue.Queue(), "com3")._elegir_puerto() == (("COM3", "16000000"), None),
                  "cable: --puerto abre el que pidieron")
    finally:
        g.update(guardados)


def autoprueba():
    try:
        sys.stdout.reconfigure(errors="replace")
    except Exception:
        pass
    print("AUTOPRUEBA del tablero: no abre la ventana ni el USB.")
    cuantas = [0]

    def verificar(condicion, que):
        cuantas[0] += 1
        if not condicion:
            raise AssertionError(que)

    partes = (("el traductor", _autoprueba_traductor), ("la memoria", _autoprueba_memoria),
              ("las cuentas", _autoprueba_cuentas), ("el robot de mentira", _autoprueba_robot_de_mentira),
              ("el cable (con un USB de mentira)", _autoprueba_cable),
              ("la ventana (con un tkinter de mentira)", _autoprueba_ventana))
    for nombre, prueba in partes:
        antes = cuantas[0]
        try:
            prueba(verificar)
        except Exception:
            print("  FALLA en %s:" % nombre)
            traceback.print_exc(file=sys.stdout)
            return 1
        print("  %-42s %5d verificaciones bien" % (nombre, cuantas[0] - antes))
    print("OK (%d verificaciones)" % cuantas[0])
    return 0


# =====================================================================

def main():
    ap = argparse.ArgumentParser(description="Tablero del DELANTERO: una ventana para mirar y probar el robot.")
    ap.add_argument("--demo", action="store_true", help="sin robot: un robot de mentira")
    ap.add_argument("--autoprueba", action="store_true", help="se prueba solo, sin abrir la ventana")
    ap.add_argument("--puerto", default=None, help="forzar el puerto (ej: COM7)")
    a = ap.parse_args()
    if a.autoprueba:
        return autoprueba()
    if tk is None:
        print("Este Python no tiene tkinter. Usar el de PlatformIO (TABLERO.bat).")
        return 1
    if not a.demo and serial is None:
        print("Este Python no tiene pyserial. Usar el de PlatformIO (TABLERO.bat).")
        return 1
    cola = queue.Queue()
    cable = RobotDeMentira(cola) if a.demo else Conexion(cola, a.puerto)
    try:
        raiz = tk.Tk()
        Ventana(raiz, cable, cola, demo=a.demo)
    except Exception:
        texto = traceback.format_exc()
        guardar_falla(texto)
        print(texto)
        try:
            messagebox.showerror("Tablero del delantero", "No pude abrir la ventana:\n\n" + texto[-1500:])
        except Exception:
            pass
        return 1
    cable.arrancar()
    raiz.mainloop()
    cable.cerrar()
    return 0


if __name__ == "__main__":
    sys.exit(main())
