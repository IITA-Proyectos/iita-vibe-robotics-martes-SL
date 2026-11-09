# -*- coding: utf-8 -*-
r"""
=====================================================================
  TABLERO DEL ARQUERO - una ventana con botones para mirar y probar
  el robot ARQUERO sin escribir comandos.
  IITA Salta - Roboliga 2026 (11/11) - 2026-11-03
=====================================================================

QUÉ TIENE
  PELOTA       dibuja, visto desde arriba, dónde ve el robot la pelota, la
               zona donde se frenaría a MIRAR, la zona donde SALE, el tope
               de la diagonal y para dónde saldría "si saliera ahora".
  SENSORES     los 3 sensores de línea (en su lugar en el robot), el
               giroscopio y la cámara.
  MOVIMIENTOS  los 10 movimientos del arquero, copiados del programa de
               PARTIDO 0.1103 (con MIRAR ANTES DE SALIR): cambia sus
               números SIN recargar el robot y dice si salió derecho
               (con gráfico). Se pueden probar SIN CABLE.
  CANCHA       preparar una cancha nueva SIN internet: la línea, el ancho,
               el fondo, la cámara, cuánto sale el despeje y los signos.
               Da la línea exacta para copiar al PANEL DE CONTROL y anota
               todo en una HOJA DE CANCHA (canchas/AAAA-MM-DD_nombre.txt).
  PARTIDO      (2026-11-08) los valores del programa con el que JUEGAN
               (funciona\partido-0921, la 0921): arriba lo de cada cancha
               (los sensores, el tamaño), abajo lo demás. Se cambian, se
               guardan (con copia) y se CARGAN al robot con un botón; el
               robot confirma con su línea #PARTIDO. También "Cargar el
               TABLERO" (para MOVIMIENTOS y CANCHA).
  CONSOLA      todo lo que dice el robot, y una caja para escribirle a mano.

CÓMO SE ABRE
  Doble clic en TABLERO.bat (en arquero-teensy-zircon): mira que el robot
  enchufado sea el ARQUERO, le carga el programa del tablero (tablero.ino,
  al lado de este archivo) y abre esta ventana.
  Sin robot: TABLERO-DEMO.bat abre la ventana con un robot de mentira.
  PARTIDO.bat abre la ventana directo en la pestaña PARTIDO (no carga nada
  solo: la carga es con el botón verde, que avisa antes).

  A mano (con el Python de PlatformIO):
     python tablero.py                  busca el arquero solo
     python tablero.py --puerto COM7    elegir el COM (solo si ESE es el arquero)
     python tablero.py --demo           robot de mentira (no abre el USB)
     python tablero.py --pestana partido   arranca en esa pestaña (pelota,
                                        sensores, movimientos, cancha,
                                        partido o consola)
     python tablero.py --quien          dice qué Teensy hay enchufada, SIN
                                        abrir ningún puerto (lo usa TABLERO.bat;
                                        también ve una en MODO CARGA)
     python tablero.py --autoprueba     se prueba a sí mismo, SIN abrir la
                                        ventana ni el USB. Tiene que decir OK.

SIN CABLE (en la cancha no hay USB)
  Con el tilde SIN CABLE, el robot cuenta 10 s: en ese rato se DESENCHUFA el
  USB. El robot hace la prueba solo, frena solo y guarda el resultado en su
  memoria (EEPROM). El LED dice la nota con destellos. Después se enchufa SIN
  APAGAR LA BATERÍA: la ventana se reconecta sola, le pide el resultado y lo
  muestra. 🚨 SIN CABLE, PARAR NO LLEGA: se frena levantándolo o con la llave
  de la batería.

CÓMO FUNCIONA POR DENTRO (para el que lo quiera tocar)
  El robot y la ventana se hablan por el cable USB con renglones de texto.
  Los renglones para la ventana empiezan con "#": por ejemplo, "#S ..."
  trae los sensores 10 veces por segundo. Los demás son para personas y
  van a la CONSOLA. Las reglas exactas son EL CONTRATO (abajo, y arriba de
  todo en tablero.ino, y en DISENO.md sección 2).

  El archivo está en partes, de lo más simple a lo más complicado (igual
  que el tablero del delantero, del 27/10):
    1. EL CONTRATO       los nombres, los movimientos y los 46 números.
    2. EL TRADUCTOR      pasa cada renglón del robot a datos. Son funciones
                         "puras": no tocan la ventana ni el cable, y por
                         eso se pueden probar solas (--autoprueba).
    3. LA MEMORIA        junta lo que va llegando (class Estado) + las
                         cuentas (el umbral, el ancho, la recta, la
                         diagonal...), también puras y probadas.
    4. EL CABLE          un hilo aparte lee el USB, así la ventana nunca
                         se traba esperando al robot. SOLO abre el arquero.
    5. EL ROBOT DE MENTIRA (--demo)
    6. LA VENTANA        los botones y los dibujos (tkinter).
    7. LA AUTOPRUEBA

  ¿Por qué un hilo aparte? Leer el cable es "esperar a que llegue algo".
  Si la ventana esperara, se congelaría. Entonces un hilo espera y deja
  cada renglón en una COLA; la ventana mira la cola cada 50 ms (con
  after()) y se dibuja con lo que encuentra. tkinter solo se toca desde
  su propio hilo: el del cable NUNCA toca la ventana.

=====================================================================
  EL CONTRATO (robot <-> ventana), copia de DISENO.md sección 2.
  🚨 NO CAMBIARLO SIN CAMBIAR EL OTRO LADO TAMBIÉN (tablero.ino).
=====================================================================
TRANSPORTE
  USB serie a 19200. Cada renglón termina en '\n' (la ventana tolera \r\n).
  La PC manda ASCII, una orden por renglón, de 71 caracteres como mucho.
  Un renglón es para la ventana solo si empieza con '#'. Campos NOMBRE=valor
  separados por UN espacio, listas con comas, decimales con punto. Nombres
  como en el panel del partido (umbralCm, KP_RUMBO): ^[A-Za-z][A-Za-z0-9_]*$
  (el robot los compara sin distinguir mayúsculas).
  Las 27 etiquetas: HOLA S V DEF OK ERROR GIRO CUENTA INICIO PASO FRENO M
  MIRE R USADOS CONSEJO FIN REPITE GUARDADO LIN ANCHO FONDO CAM MEDIO PANEL
  FINPANEL PARTIDO (la PARTIDO la manda el programa de PARTIDO, desde la
  0921g, al arrancar: NO tablero.ino). Otra = DESCONOCIDA. Otra #ETIQUETA adentro del renglón = ROTA.
  El robot habla solo si hay alguien escuchando (if (Serial)), y MIENTRAS
  SE MUEVE NO MANDA NADA (salvo #PASO de A e I, con el robot frenado).

CÓDIGOS  1 COSTADO_IZQ  2 COSTADO_DER  3 RAPIDO_IZQ  4 RAPIDO_DER
         5 IDA_Y_VUELTA  6 DESPEJE  7 UBICARSE  8 SEGUIR  9 ATAJAR  10 IDA
         L LINEA_VERDE/LINEA_BLANCO_IZQ/LINEA_BLANCO_DER/LINEA_GUIADA
         A ANCHO   F FONDO/FONDO_COMPROBAR   C CAMARA   I IR_AL_MEDIO

PC -> ROBOT (solo se leen en ESPERA; en CUENTA o MOVIENDO cualquier byte
             que no sea \r ni \n FRENA y no se interpreta)
  !1..!10 [c]        un movimiento (c = SIN CABLE: cuenta 10 s en vez de 3)
  !L v|i|d|c         la línea: verde / blanco IZQ / blanco DER / guiada sin cable
  !A [c]  !F [c]  !C [c]  !I [c]     ancho, fondo (t = TOCÓ), cámara, ir al medio
  x                  en ESPERA: frena 400 ms y suelta
  =NOMBRE valor      cambiar un número en la RAM   -> #OK + #V, o #ERROR
  ?  ayuda (#V #DEF)   v  líneas para el panel (#PANEL... #FINPANEL)
  0  volver a los números del partido   h/H  saludo (#HOLA + 2 #GUARDADO)
  g  buscar el giroscopio de nuevo      r  repetir el último MOVIMIENTO
  rc repetir la última MEDICIÓN DE CANCHA

ROBOT -> PC (ejemplos exactos)
  #HOLA tablero-arquero 2026-11-03 serie=13348550 partido=0.1103
  #GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210
  #GUARDADO que=cancha hay=0
  #GIRO buscando  /  #GIRO ok=1 estado=5
  #S t=12345 l=88,92,61 u=425 r=93.4 g=5 cal=3,3,0,0 p=86,-12,40 d=30.0 y=4.2
     vy=0.0 va=0.0 ve=1 n=3 dg=1,0,193,33,396 am=150,-20,120 az=0,0,-1 q=24 e=ESPERA
     (un solo renglón; cada 100 ms y solo en ESPERA. l = A13 atrás IZQ, A11
     atrás DER, A12 adelante. u = UN número. p/am/az CRUDOS (x,y,edad).
     d,y en cm con y + = DERECHA. dg = la diagonal "si saliera ahora":
     sale,motivo,avance,lateral,msIda. t primero y e último.)
  #V ... / #DEF ...   los 46 números en el orden de la tabla (bool 0/1)
  #OK umbralCm=32.0   /   #ERROR <texto fijo>
  #CUENTA 3.0 cable (3 s, cada 0,5 s)   #CUENTA 10.0 sincable (10 s, cada 1 s)
  #INICIO 6 DESPEJE tope=20000 sincable=0
  #PASO A tramo=0 ms=2180 desvio=2.9 endereza=0
  #FRENO tecla | tope de tiempo | sin giroscopio | sin camara | sobre la linea
         | recien parado | no es el arquero
  #M t grados fase   (grados + = IZQUIERDA; fases l s m i p a f e c v q)
  #MIRE k= t= res= miro= cuadros= d= y= vy= va= av= lat= ms= ang= recto=   (solo la 9)
  #R n mov nota como pico picoms fin medio cruces fuera3 corr tope mudas racha
     hueco huecoms duro lineams idams atrasms acomodo laterales idas arranques
     conpelota zonamuerta velmedia intentos sale      (los 29, siempre, en orden)
  #USADOS NOMBRE=valor ...   #CONSEJO texto   #FIN codigo (el ÚLTIMO de toda tanda)
  #REPITE 9 ATAJAR sincable=1 apagado=0
  #LIN fase=v n= izq=min,max,prom der= ade=
  #ANCHO ok= error= medioizq= izqder= derizq= almedioizq= almedioder=
         desvio=a,b,c,d endereza= golpe= vel= rampa= umbral= calgiro=
  #FONDO ms= como=TOCO|LINEA|TOPE|TIEMPO|PARADO desvio= umbral= probado=
  #CAM n= cero= val= xp=min,max,prom yp= am= az=
  #MEDIO ok= pasos=p1..p8 como=c1..c8 desvio= acomodo=
  #PANEL <declaración del panel> nombre = valor;  ...  #FINPANEL
  #PARTIDO version=0921g sello=fabrica giro=1 umbralBlanco=425 ... interruptores=1111111110101
     (la manda el programa de PARTIDO al arrancar, cada 1/2 s mientras
     parpadea; version PRIMERO e interruptores ÚLTIMO. DISENO sección 16.)

SECUENCIAS
  (a) normal: >>> -> #CUENTA... -> #INICIO -> (silencio) -> #M... -> (#MIRE en
      la 9) -> #R -> #USADOS -> #CONSEJO... -> RESULTADO: ... -> #FIN n
  (b) tecla en la cuenta: #CUENTA... -> #FRENO tecla (sin informe)
  (c) tecla moviéndose: #INICIO -> #FRENO tecla -> la tanda con nota=NO_VALE
  (d) no arranca: #FRENO motivo, sin #CUENTA   (e) sin giroscopio al terminar
      la cuenta: #CUENTA... -> #FRENO sin giroscopio   (f) tope: #FRENO tope
      de tiempo -> la tanda con NO_VALE
  SIN CABLE: #CUENTA 10.0 sincable ... -> se desenchufa (es lo esperado) ->
      el robot hace todo solo y lo guarda (nuevo=1) -> se enchufa SIN apagar
      -> #HOLA + #GUARDADO nuevo=1 -> la ventana manda r -> #REPITE + la tanda
      + #FIN. (03/11: sin cable queda nuevo=1 aunque la tanda haya salido: el
      Teensy no sabe si lo desenchufaron. La ventana no pide lo que ya vio.)

RECONOCIMIENTO
  La ventana abre SOLO la Teensy con serie 13348550 (el arquero). Nunca la
  del delantero (15708680) ni una desconocida, ni con --puerto. El programa
  es el tablero solo con "#HOLA tablero-arquero" (un #S no alcanza). Al
  conectar no manda nada; con el primer #S manda h, y después del #HOLA, ?.
"""

import argparse
import copy
import hashlib
import math
import os
import queue
import random
import re
import subprocess
import sys
import tempfile
import threading
import time
import traceback

try:
    import tkinter as tk
    from tkinter import ttk, messagebox, filedialog
except ImportError:              # sin tkinter igual anda la --autoprueba
    tk = ttk = messagebox = filedialog = None

try:
    import serial
    from serial.tools import list_ports
except ImportError:              # sin pyserial anda la --autoprueba y la --demo
    serial = list_ports = None

CARPETA = os.path.dirname(os.path.abspath(__file__))
ARCHIVO_FALLAS = os.path.join(CARPETA, "tablero-error.txt")
# Lo del partido se LEE (nunca se escribe): para mostrar el "valor de hoy" de
# cada número y avisar si las copias del robot quedaron viejas.
PARTIDO_INO = os.path.normpath(os.path.join(CARPETA, "..", "..", "funciona", "seguir-y-despejar",
                                            "seguir-y-despejar.ino"))
CANCHAS = os.path.join(CARPETA, "canchas")      # las HOJAS DE CANCHA
# ---- la pestaña PARTIDO (2026-11-08): el programa con el que JUEGAN, la 0921 ----
# Este SÍ se escribe (solo los números, con copia antes) y se carga desde la
# ventana. Todas son globales que se leen EN EL MOMENTO (x or PARTIDO_CARPETA):
# así la autoprueba las cambia por copias en una carpeta temporal.
PARTIDO_CARPETA = os.path.normpath(os.path.join(CARPETA, "..", "..", "funciona", "partido-0921"))
NOMBRE_PARTIDO_INO = "partido-0921.ino"
PARTIDO_0921_INO = os.path.join(PARTIDO_CARPETA, NOMBRE_PARTIDO_INO)
COPIAS_PARTIDO = os.path.join(CARPETA, "copias-partido")     # las copias de seguridad (.txt, NUNCA .ino)
CARGAR_PY = os.path.normpath(os.path.join(CARPETA, "..", "herramientas", "cargar.py"))
_PENV = os.path.join(os.path.expanduser("~"), ".platformio", "penv", "Scripts")
PIO = os.path.join(_PENV, "pio.exe") if os.path.exists(os.path.join(_PENV, "pio.exe")) else "pio"
if os.path.exists(os.path.join(_PENV, "python.exe")):
    PY = os.path.join(_PENV, "python.exe")
else:                            # con pythonw no hay consola: cargar.py va con python.exe
    PY = re.sub(r"pythonw(\.exe)?$", r"python\1", sys.executable or "python", flags=re.IGNORECASE)
# Los archivos que NUNCA se escriben (la autoprueba pone acá el partido de verdad).
RUTAS_QUE_NO_SE_ESCRIBEN = set()
# Dónde la DEMO hace su copia de práctica del partido (None = la carpeta temporal de Windows).
CARPETA_DEMO = None


# =====================================================================
#  1. EL CONTRATO: lo que el robot y la ventana acordaron
# =====================================================================

SERIE_ARQUERO = "13348550"       # el número de serie del Teensy del ARQUERO
SERIE_DELANTERO = "15708680"     # el del DELANTERO: a este NUNCA se le abre el puerto
VID_TEENSY = 0x16C0              # el "fabricante" de las Teensy (PJRC): no cambia nunca
PID_SERIE = 0x0483               # una Teensy corriendo un programa, con su puerto COM
BAUDIOS = 19200
PROGRAMA = "tablero-arquero"     # lo que dice "#HOLA" si el robot tiene ESTE programa
PROGRAMA_DELANTERO = "tablero-delantero"   # el del otro equipo: pines cruzados
FECHA_CONTRATO = "2026-11-03"
PARTIDO_COPIADO = "0.1103"       # la versión del partido de la que son las copias
LARGO_MAX_ORDEN = 71             # el búfer del robot es de 72 (con el '\n')

ETIQUETAS = ("HOLA", "S", "V", "DEF", "OK", "ERROR", "GIRO", "CUENTA", "INICIO", "PASO", "FRENO",
             "M", "MIRE", "R", "USADOS", "CONSEJO", "FIN", "REPITE", "GUARDADO",
             "LIN", "ANCHO", "FONDO", "CAM", "MEDIO", "PANEL", "FINPANEL",
             "PARTIDO")          # la #PARTIDO la manda el programa de PARTIDO (0921g), NO tablero.ino
# ---- la línea de versión del partido (0921g, 2026-11-08). Es un CONTRATO con
# avisarVersion() del .ino: version PRIMERO e interruptores ÚLTIMO (si no, el
# renglón vino cortado). Si se agrega un campo, va antes de interruptores y
# se agrega también allá.
CAMPOS_PARTIDO = ("version", "sello", "giro", "umbralBlanco", "CAMARA_POR_CM", "MS_MEDIO_AL_MEDIO",
                  "MS_MEDIO_FONDO_POR_TIEMPO", "MS_MAX_RETROCESO", "msAdelante", "umbralCm", "umbralDesvio",
                  "potenciaDespeje", "pwmMinLateral", "pwmMaxLateral", "kpLateral", "MS_QUIETO_PARA_PATRULLAR",
                  "interruptores")
NUMEROS_LINEA_PARTIDO = CAMPOS_PARTIDO[3:-1]
# Los 13 interruptores de "interruptores=", en ESTE orden (1 = SÍ)
ORDEN_INTERRUPTORES = ("volverAlMedioEnElLateral", "patrullarSinPelota", "patrullaPorElLateral",
                       "irAlMedioSiNoHayLinea", "avanzarAlUbicarse", "perseguirEnElDespeje",
                       "esperarPlanoAntesDeVolver", "rampaAlVolver", "empujonFinal", "despejeEnDiagonal",
                       "enderezarActivado", "lateralInvertido", "camaraYInvertida")
LINEA_PARTIDO_FABRICA = ("#PARTIDO version=0921g sello=fabrica giro=1 umbralBlanco=425 CAMARA_POR_CM=2.87 "
                         "MS_MEDIO_AL_MEDIO=2228 MS_MEDIO_FONDO_POR_TIEMPO=2000 MS_MAX_RETROCESO=1500 msAdelante=533 "
                         "umbralCm=30.0 umbralDesvio=20.0 potenciaDespeje=200 pwmMinLateral=70 pwmMaxLateral=160 "
                         "kpLateral=11.50 MS_QUIETO_PARA_PATRULLAR=10000 interruptores=1111111110101")
MODOS = ("ESPERA", "CUENTA", "MOVIENDO")
CODIGOS = ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "L", "A", "F", "C", "I")
CODIGOS_CANCHA = ("L", "A", "F", "C", "I")
NOMBRES_CANCHA = {
    "L": ("LINEA_VERDE", "LINEA_BLANCO_IZQ", "LINEA_BLANCO_DER", "LINEA_GUIADA"),
    "A": ("ANCHO",), "F": ("FONDO", "FONDO_COMPROBAR"), "C": ("CAMARA",), "I": ("IR_AL_MEDIO",),
}
# Las fases de las muestras #M: (cómo se lee, color en el gráfico)
FASES = {
    "l": ("de costado", "#6a1b9a"),
    "s": ("siguiendo", "#00838f"),
    "m": ("MIRANDO", "#f9a825"),
    "i": ("ida", "#e65100"),
    "p": ("pausa", "#8d6e63"),
    "a": ("atrás a la línea", "#1565c0"),
    "f": ("frenando", "#c62828"),
    "e": ("enderezándose", "#2e7d32"),
    "c": ("empujoncito", "#5d4037"),
    "v": ("volviendo al medio", "#3949ab"),
    "q": ("quieto al final", "#9e9e9e"),
}
# Los 29 campos del #R, en orden. "mov", "nota" y "como" son palabras; el resto, números.
CAMPOS_R = ("n", "mov", "nota", "como", "pico", "picoms", "fin", "medio", "cruces", "fuera3", "corr",
            "tope", "mudas", "racha", "hueco", "huecoms", "duro", "lineams", "idams", "atrasms",
            "acomodo", "laterales", "idas", "arranques", "conpelota", "zonamuerta", "velmedia",
            "intentos", "sale")
CAMPOS_R_TEXTO = ("n", "mov", "nota", "como")
COMOS_R = {
    "NORMAL": "terminó normal",
    "LATERAL": "frenó al pisar el lateral",
    "SIN_LINEA": "volvió hasta el tope SIN ver la línea",
    "NO_SALIO": "no salió a despejar",
    "TECLA": "lo frenaron con una tecla",
    "TOPE": "se cortó por el tope de tiempo",
}
CAMPOS_MIRE = ("k", "t", "res", "miro", "cuadros", "d", "y", "vy", "va", "av", "lat", "ms", "ang", "recto")
# El enum Resultado de la 0.1103, en palabras
RESULTADOS_MIRE = ("SALE_MIRANDO", "SALE_SIN_MIRAR", "MUY_ABIERTO", "NO_ALCANZA", "NO_VIENE", "POCOS_CUADROS")
PALABRAS_MIRE = {
    "SALE_MIRANDO": "SALIÓ (miró antes)",
    "SALE_SIN_MIRAR": "SALIÓ (sin mirar: ya estaba quieto)",
    "MUY_ABIERTO": "NO SALIÓ: muy abierto",
    "NO_ALCANZA": "NO SALIÓ: el avance no alcanza",
    "NO_VIENE": "NO SALIÓ: la pelota no venía",
    "POCOS_CUADROS": "NO SALIÓ: pocos cuadros",
}
RECTO_MIRE = {0: "", 1: "derecho: costado chico", 2: "derecho: despejeEnDiagonal apagado",
              3: "derecho: sin giroscopio", 4: "derecho: no la veía"}
MAX_MUESTRAS = 2000              # el robot manda hasta 1200; esto es por las dudas
EDAD_VIEJA_MS = 300              # pelota/arco vistos hace más que esto: se dibujan en gris

# Las notas: (color de fondo, color de letra, cómo se lee, qué quiere decir)
NOTAS = {
    "PROLIJO":   ("#2e7d32", "white", "PROLIJO",
                  "Fue derecho: 4° o menos, casi sin zigzag, y terminó derecho."),
    "CASI":      ("#f9a825", "#212121", "CASI",
                  "Anda, pero se torció un poco. Mirá el consejo."),
    "ZIGZAGUEA": ("#ef6c00", "white", "ZIGZAGUEA",
                  "Va y viene: cruzó 4 veces o más."),
    "SE_TUERCE": ("#c62828", "white", "SE TUERCE",
                  "Se torció más de 10°."),
    "NO_SALIO":  ("#6d4c41", "white", "NO SALIÓ",
                  "En msSeguir no salió a despejar. Mirá los intentos: dicen por qué."),
    "NO_VALE":   ("#757575", "white", "NO VALE",
                  "La frenaron, se pasó del tiempo, o el giroscopio no contestó."),
}

# ---------------------------------------------------------------------
# LOS 46 NÚMEROS (DISENO.md sección 3), EN ESTE ORDEN: es el orden de #V,
# #DEF y de ajustes[] en el robot. El defecto es el del PANEL DE CONTROL de
# la 0.1103, escrito como lo manda el robot. Tipo: i int, u unsigned long,
# f float, b bool (0/1). El paso es el de los botones [-] [+]. La
# declaración es la del panel ("" = solo de esta prueba: no va al panel).
# De esta tabla salen NUMEROS, VALORES_DE_MENTIRA y RANGOS_DE_MENTIRA, así
# no hay tres listas que se puedan desparejar.
#   (nombre, tipo, defecto, mínimo, máximo, paso, declaración, explicación)
# ---------------------------------------------------------------------
TABLA = (
    ("umbralBlanco", "i", "425", 100, 1000, 5, "int",
     "Desde qué lectura un sensor de atrás dice BLANCO. Se MIDE en cada cancha (pestaña CANCHA, paso 1)."),
    ("CAMARA_POR_CM", "f", "2.87", 2.00, 4.00, 0.01, "const float",
     "Unidades de cámara por cm real. Es de la CÁMARA: cambiarlo cambia TODOS los cm (y kpLateral está pensado para 2.87)."),
    # Hasta 3200 y no 6000 (2026-11-03, revisión): en !I el paso 6 va de
    # costado ESE tiempo sin mirar la línea, y un cruce entero son ~4400 ms.
    # !A nunca mide más de ~3150. Igual que en tablero.ino.
    ("MS_MEDIO_AL_MEDIO", "u", "2228", 500, 3200, 10, "const unsigned long",
     "Del lateral izquierdo al medio a velocidad 100 (ms). Se MIDE (CANCHA, paso 2)."),
    ("MS_MEDIO_FONDO_POR_TIEMPO", "u", "2000", 500, 6000, 100, "const unsigned long",
     "Ir al medio: cuánto retrocede buscando el fondo si no ve la línea (ms). CANCHA, paso 3."),
    ("kpLateral", "f", "11.5", 5, 20, 0.5, "float",
     "Siguiendo: fuerza por cada cm que la pelota está al costado."),
    ("pwmMinLateral", "i", "45", 30, 80, 5, "int",
     "Siguiendo: lo mínimo que empuja. Si zumba sin moverse: subir de a 5."),
    ("pwmMaxLateral", "i", "150", 80, 170, 10, "int",
     "Siguiendo: lo máximo = la velocidad de costado. Más de 150 nunca se probó."),
    ("ZONA_MUERTA_PELOTA", "f", "1.4", 1, 4, 0.2, "const float",
     "cm al costado que deja pasar sin moverse. Si tiembla con la pelota quieta: subir."),
    ("predecirTrayectoria", "b", "0", 0, 1, None, "bool",
     "SÍ = sigue adonde VA a estar la pelota. Apagada desde el 20/10 (iba y venía)."),
    ("msAnticipacionSeguir", "f", "500.0", 0, 800, 50, "float",
     "Con la predicción prendida: cuánto adelanta (ms)."),
    ("LADO_TRASERA", "i", "89", 80, 100, 1, "const int",
     "De costado, fuerza de la rueda de atrás (%). Si de costado gira solo: 2 más o 2 menos. Cambia MS_MEDIO_AL_MEDIO."),
    ("volverAlMedioEnElLateral", "b", "1", 0, 1, None, "bool",
     "SÍ = al pisar el lateral siguiendo la pelota, vuelve al medio (como juega)."),
    ("umbralCm", "f", "30.0", 20, 45, 1, "float",
     "Sale a despejar con la pelota a esto o menos (cm). ATADO a msAdelante: la ida tiene que llegar más lejos."),
    ("umbralDesvio", "f", "20.0", 3, 30, 1, "float",
     "...y a esto o menos al costado (cm). Saliendo DERECHO, más de 7 no la alcanza."),
    ("potenciaDespeje", "i", "200", 140, 230, 10, "int",
     "Fuerza de la ida. Cambia cuánto sale (msAdelante se calculó a 200)."),
    ("msAdelante", "i", "383", 100, 800, 10, "int",
     "Cuánto dura la ida (ms). 383 ≈ 35 cm (es una CUENTA, sin probar). Se mide en CANCHA, paso 5."),
    ("frenarAntesDeVolver", "b", "1", 0, 1, None, "bool",
     "SÍ = al terminar la ida frena y espera (como juega desde el 20/10). NO = suelta 150 ms."),
    ("MS_ESPERA_ANTES_DE_VOLVER", "u", "400", 0, 800, 25, "const unsigned long",
     "Cuánto se queda frenado antes de volver (ms)."),
    ("potenciaRetroceso", "i", "110", 80, 140, 10, "int",
     "Fuerza de la vuelta hasta la línea (también al ubicarse)."),
    ("msFreno", "u", "200", 100, 400, 25, "unsigned long",
     "Cuánto sostiene el freno eléctrico cada vez que frena (ms). Lo usan TODOS los frenos."),
    ("empujonFinal", "b", "1", 0, 1, None, "bool",
     "SÍ = después de volver del despeje avanza el poquito (~10 cm)."),
    ("potenciaEmpujon", "i", "100", 60, 150, 10, "int",
     "Fuerza del empujoncito. Atada a msEmpujonFinal."),
    ("msEmpujonFinal", "i", "320", 0, 600, 20, "int",
     "Cuánto dura el empujoncito (ms). 320 ≈ 10 cm a 100."),
    ("avanzarAlUbicarse", "b", "1", 0, 1, None, "bool",
     "SÍ = al encontrar la línea del área avanza el poquito."),
    ("despejeEnDiagonal", "b", "1", 0, 1, None, "bool",
     "SÍ = despeja en diagonal. NO = sale derecho, como hasta el 20/10."),
    ("predecirEnDiagonal", "b", "1", 0, 1, None, "bool",
     "SÍ = apunta adonde VA a estar la pelota (solo si estaba quieto o miró)."),
    ("rendimientoCostado", "f", "0.8", 0.40, 1.20, 0.05, "float",
     "⚠ VA AL REVÉS: más chico = MÁS costado. Si no llega de costado: BAJAR de a 0.05. Si se pasa: SUBIR."),
    ("MAX_ANGULO_DIAGONAL", "f", "30.0", 10, 45, 5, "const float",
     "Si tiene que salir más abierto que esto (grados), NO SALE."),
    ("medirAntesDeDespejar", "b", "1", 0, 1, None, "bool",
     "SÍ = MIRA antes de salir (frena, mide quieto, sale a umbralCm). NO = sale apenas la ve a umbralCm."),
    ("distanciaParaMirar", "f", "40.0", 25, 60, 1, "float",
     "A esta distancia (cm) empieza a mirar. Más = ve venir pelotas más rápidas, pero frena más seguido."),
    ("MS_ASENTAR_MIRADA", "u", "60", 0, 300, 10, "const unsigned long",
     "Primero frena esto (ms) sin medir: todavía se está frenando."),
    ("MS_MIRAR", "u", "120", 40, 600, 20, "const unsigned long",
     "Después mide quieto esto (ms) como mínimo, y 3 cuadros como mínimo."),
    ("MS_MAX_MIRANDO", "u", "500", 200, 1500, 50, "const unsigned long",
     "Tope de la mirada (ms). Si la pelota no viene, vuelve a seguirla."),
    ("MS_DESCANSO_MIRADA", "u", "500", 0, 2000, 50, "const unsigned long",
     "Después de una mirada que no salió, no vuelve a mirar hasta que pase esto (ms)."),
    ("KP_RUMBO", "f", "5.0", 1, 10, 0.5, "const float",
     "Corrección por cada grado torcido. Si zigzaguea: bajar de a 0.5. Si se le escapa: subir."),
    ("KI_RUMBO", "f", "1.5", 0, 3, 0.25, "const float",
     "Corrige la torcida que queda fija para un lado."),
    ("LIMITE_INTEGRAL", "f", "30.0", 10, 40, 5, "const float",
     "Tope de lo que acumula KI_RUMBO."),
    ("MAX_CORRECCION_RUMBO", "i", "100", 40, 120, 10, "const int",
     "Tope de la corrección, en fuerza."),
    ("ZONA_MUERTA_RUMBO", "f", "1.0", 0.25, 3, 0.25, "const float",
     "Grados que deja pasar sin corregir."),
    ("RAMPA_MOV_PASO", "i", "10", 2, 20, 1, "const int",
     "Arranque suave: sube esto cada 10 ms. Menos = más suave, pero cambia msAdelante y MS_MEDIO_AL_MEDIO."),
    ("lateralInvertido", "b", "0", 0, 1, None, "bool",
     "Tocar SOLO si el movimiento 2 (costado DERECHA) va a la IZQUIERDA."),
    ("camaraYInvertida", "b", "1", 0, 1, None, "bool",
     "Tocar SOLO si el 2 va bien y aun así, siguiendo la pelota, se aleja de ella."),
    ("velLenta", "i", "70", 40, 150, 5, "",
     "De costado LENTO (1 y 2). Solo de esta prueba."),
    ("msCostado", "i", "1000", 300, 1500, 100, "",
     "Cuánto dura cada tramo de costado (ms). Solo de esta prueba."),
    ("msSeguir", "i", "10000", 3000, 20000, 1000, "",
     "Cuánto dura SEGUIR y cuánto espera ATAJAR (ms). Solo de esta prueba."),
    ("msPruebaIda", "i", "383", 100, 800, 10, "",
     "Cuánto dura la IDA del movimiento 10 (ms). Solo de esta prueba."),
)
ORDEN_NUMEROS = tuple(f[0] for f in TABLA)
TIPO_NUMERO = {f[0]: f[1] for f in TABLA}
DECLARACION = {f[0]: f[6] for f in TABLA}
# Los números que se cambian desde la ventana.
#   NOMBRE: (paso de los botones [-] [+], qué hace, opciones)
#   opciones = None para un número; si es un interruptor, los dos textos
#   de sus botones (para el valor 0 y para el 1).
NUMEROS = {f[0]: (f[5], f[7], ("NO", "SÍ") if f[1] == "b" else None) for f in TABLA}

# Las 7 líneas del panel de la 0.1103 que se MIDEN en la cancha (las 79, 80,
# 81, 82, 101, 189 y 190). Son una COPIA, por si no se encuentra el archivo
# del partido: la ventana las lee del archivo de verdad cuando está.
LINEAS_PANEL_0_1103 = (
    (79, "int umbralBlanco                                 = 425;   // [MEDIDO 15/09] desde que lectura un sensor de linea dice blanco. En cada cancha: medirlo con calibrar-linea y copiarlo TAMBIEN en las pruebas (ver arriba)"),
    (80, "const float CAMARA_POR_CM                        = 2.87;  // [MEDIDO 25/08] unidades de camara por cm real, pelota enfrente. Es de la CAMARA, no de la cancha: se mide de nuevo solo si se movio la camara (tabla-camara). Cambiarlo cambia TODOS los cm"),
    (81, "const unsigned long MS_MEDIO_AL_MEDIO            = 2228;  // [MEDIDO 06/10, antes de mover los motores] del lateral izquierdo al medio a velocidad 100 (medir-ancho). Lo usan ir al medio, la patrulla y el limite lateral. Vale con los numeros de mezcla, rampa y rumbo de hoy: si se tocan, medirlo de nuevo"),
    (82, "const unsigned long MS_MEDIO_FONDO_POR_TIEMPO    = 2000;  // [a ojo, no medido] ir al medio: cuanto retrocede buscando el fondo antes de darlo por encontrado (la rampita no lo deja pisar la linea)"),
    (101, "int msAdelante                                   = 383;   // cuanto dura la ida (ms). [CUENTA, SIN PROBAR] para X cm: ms = (X + 3.3) x 10, a potencia 200 (04/08, antes de la rampa y del freno: puede salir menos). Si no toca la pelota: subir y medir con regla"),
    (189, "bool lateralInvertido                            = false; // [MEDIDO 11/08] se prueba SIN pelota: ajustar-movimientos, movimiento 2 (costado DERECHA). Si va a la IZQUIERDA, cambiar esto"),
    (190, "bool camaraYInvertida                            = true;  // [MEDIDO 11/08] cambiar SOLO si el movimiento 2 de ajustar-movimientos va bien a la derecha y aun asi, siguiendo la pelota, se aleja de ella"),
)
# Los comentarios de esas líneas (para armar la línea si falta el archivo)
COMENTARIOS_PANEL = {
    "umbralBlanco": "[MEDIDO 15/09] desde que lectura un sensor de linea dice blanco. En cada cancha: medirlo con calibrar-linea y copiarlo TAMBIEN en las pruebas (ver arriba)",
    "CAMARA_POR_CM": "[MEDIDO 25/08] unidades de camara por cm real, pelota enfrente. Es de la CAMARA, no de la cancha: se mide de nuevo solo si se movio la camara (tabla-camara). Cambiarlo cambia TODOS los cm",
    "MS_MEDIO_AL_MEDIO": "[MEDIDO 06/10, antes de mover los motores] del lateral izquierdo al medio a velocidad 100 (medir-ancho). Lo usan ir al medio, la patrulla y el limite lateral. Vale con los numeros de mezcla, rampa y rumbo de hoy: si se tocan, medirlo de nuevo",
    "MS_MEDIO_FONDO_POR_TIEMPO": "[a ojo, no medido] ir al medio: cuanto retrocede buscando el fondo antes de darlo por encontrado (la rampita no lo deja pisar la linea)",
    "msAdelante": "cuanto dura la ida (ms). [CUENTA, SIN PROBAR] para X cm: ms = (X + 3.3) x 10, a potencia 200 (04/08, antes de la rampa y del freno: puede salir menos). Si no toca la pelota: subir y medir con regla",
    "lateralInvertido": "[MEDIDO 11/08] se prueba SIN pelota: ajustar-movimientos, movimiento 2 (costado DERECHA). Si va a la IZQUIERDA, cambiar esto",
    "camaraYInvertida": "[MEDIDO 11/08] cambiar SOLO si el movimiento 2 de ajustar-movimientos va bien a la derecha y aun asi, siguiendo la pelota, se aleja de ella",
}
# Los que tienen copia escrita A MANO en otras pruebas (la ventana NO toca
# esos archivos: solo los lista para que los cambien a mano).
COPIAS_A_MANO = {
    "umbralBlanco": "ajustar-movimientos (umbralBlanco), monitor-robot, ir-al-medio, medir-ancho y medir-ancho-sin-giro (UMBRAL_BLANCO)",
    "CAMARA_POR_CM": "ajustar-movimientos, monitor-robot y ver-prediccion (CAMARA_POR_CM)",
    "MS_MEDIO_AL_MEDIO": "ir-al-medio (MS_AL_MEDIO)",
    "MS_MEDIO_FONDO_POR_TIEMPO": "ir-al-medio (MS_FONDO_POR_TIEMPO)",
}

# Lo fijo del partido que usa la ventana (copias de la 0.1103, no se cambian desde acá)
FIJOS = {
    "VECES_PARA_CREERLE": 3, "PISO_VELOCIDAD": 4.0, "PWM_POR_CM_S": 2.0, "PRESUPUESTO_RUEDA": 210,
    "PWM_MIN_AVANCE_DESPEJE": 140, "MS_MAX_ADELANTE": 800, "COSTADO_MINIMO_UTIL": 20,
    "MS_MAX_RETROCESO": 1200, "MS_MEDIO_TOPE_LATERAL": 6000, "RAMPA_MOV_MS": 10,
    "MS_RETARDO_ARRANQUE": 33, "VEL_MEDIO_MEDIR": 100,
}

AVISO_SIN_CABLE = "⚠ El rumbo que sostiene es el de cuando termina la cuenta. Y el cable flojo, o SIN CABLE."
AYUDA_SIN_GIRO = "prendé la batería ANTES que el USB, y apretá \"Buscar el giroscopio de nuevo\"."
NO_OBEDECE_PARAR = "Este programa NO obedece PARAR: levantá el robot o apagá la batería."
SIN_CABLE_NO_LLEGA = "Sin cable, PARAR no llega: levantalo o apagá la batería."
# El tope más largo de una prueba (el de !A, topeDe() en tablero.ino): si se
# corta el cable contando, todavía no se sabe el tope de ESA prueba.
TOPE_MAS_LARGO = 45000
# Paso 5 (IDA): cuánto se espera el #OK de msPruebaIda para mandar !10 solo.
ESPERA_OK_IDA = 3.0
# Al PROBAR sin giroscopio el robot lo busca ANTES de contar: bno.begin(),
# 1 s y hasta 5 s esperando un dato (buscarGiroscopio en tablero.ino). Ese
# rato no es "no arrancó": se le dan 7 s más a la espera del #CUENTA.
ESPERA_GIRO = 7.0
# Después de CARGAR EL PARTIDO, cuánto se espera su #PARTIDO por la conexión
# nueva (2026-11-08, revisión). Con la batería APAGADA el setup() busca el
# giroscopio 10 veces (~11,5 s) antes de decir la primera; después la repite
# cada 1/2 s mientras parpadea (~4 s). Si llega más tarde, confirma igual.
ESPERA_VERSION = 25
# Los resultados de un #MIRE que traen un ángulo de verdad (n.ang en tablero.ino).
RES_CON_ANGULO = ("SALE_MIRANDO", "SALE_SIN_MIRAR", "MUY_ABIERTO")
# CANCHA mide con los números de la RAM del robot (2026-11-03, revisión). Si
# estos no son los del partido, lo medido vale para ESOS números y no para el
# partido (el panel lo dice: "Vale con los numeros de mezcla, rampa y rumbo de
# hoy"). Los de ir de costado / para atrás (pasos 2, 2c y 3) y los de la ida (5).
NUMEROS_DEL_MOVIMIENTO = ("RAMPA_MOV_PASO", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO", "LIMITE_INTEGRAL",
                          "MAX_CORRECCION_RUMBO", "ZONA_MUERTA_RUMBO")
NUMEROS_DE_LA_IDA = NUMEROS_DEL_MOVIMIENTO + ("potenciaDespeje", "frenarAntesDeVolver", "msFreno")
# Números del TABLERO que la 0921 NO tiene, pero que se sabe cómo hace (para
# que CANCHA mida como juega). (2026-11-08, revisión) Al terminar la ida, la
# 0921 hace parar() (SUELTA los motores y sigue un poco de largo, ver su
# PAUSA_MEDIO); el tablero, con frenarAntesDeVolver = 1, frena.
COMO_LA_0921 = {"frenarAntesDeVolver": ("false", "(la 0921 suelta)")}

# Los 10 movimientos. tipo: costado (1 a 5), despeje (6 y 7: la cadena del
# despeje del partido), seguir (8), atajar (9) e ida (10).
MOVIMIENTOS = [
    dict(n=1, nombre="COSTADO_IZQ", tipo="costado", boton="1   COSTADO  izq  (lento)",
         titulo="1 · COSTADO a la IZQUIERDA (lento)",
         que_hace="De costado a la IZQUIERDA a fuerza fija (velLenta), con la corrección de rumbo del "
                  "partido. Frena al pisar el lateral. Mide cuánto se tuerce.",
         lugar="Lugar a la IZQUIERDA (~50 cm). Sensores de atrás FUERA de la línea.",
         numeros=["velLenta", "msCostado", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO", "LIMITE_INTEGRAL",
                  "MAX_CORRECCION_RUMBO", "ZONA_MUERTA_RUMBO", "RAMPA_MOV_PASO"]),
    dict(n=2, nombre="COSTADO_DER", tipo="costado", boton="2   COSTADO  der  (lento)",
         titulo="2 · COSTADO a la DERECHA (lento)",
         que_hace="Igual a la DERECHA. Es la prueba de lateralInvertido: tiene que ir a la DERECHA.",
         lugar="Lugar a la DERECHA.",
         numeros=["velLenta", "msCostado", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO", "LIMITE_INTEGRAL",
                  "MAX_CORRECCION_RUMBO", "ZONA_MUERTA_RUMBO", "RAMPA_MOV_PASO", "lateralInvertido"]),
    dict(n=3, nombre="RAPIDO_IZQ", tipo="costado", boton="3   COSTADO  izq  (rápido)",
         titulo="3 · COSTADO a la IZQUIERDA (rápido)",
         que_hace="Como el 1 a pwmMaxLateral (la velocidad tope del seguimiento).",
         lugar="Lugar a la IZQUIERDA.",
         numeros=["pwmMaxLateral", "msCostado", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO",
                  "MAX_CORRECCION_RUMBO", "RAMPA_MOV_PASO"]),
    dict(n=4, nombre="RAPIDO_DER", tipo="costado", boton="4   COSTADO  der  (rápido)",
         titulo="4 · COSTADO a la DERECHA (rápido)",
         que_hace="Como el 2 a pwmMaxLateral.",
         lugar="Lugar a la DERECHA.",
         numeros=["pwmMaxLateral", "msCostado", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO",
                  "MAX_CORRECCION_RUMBO", "RAMPA_MOV_PASO"]),
    dict(n=5, nombre="IDA_Y_VUELTA", tipo="costado", boton="5   IDA Y VUELTA  (rápido)",
         titulo="5 · IDA Y VUELTA de costado (rápido)",
         que_hace="Rápido a la izquierda y se da vuelta SIN frenar, como cuando la pelota cruza "
                  "siguiéndola.",
         lugar="Lugar a los DOS costados.",
         numeros=["pwmMaxLateral", "msCostado", "LADO_TRASERA", "KP_RUMBO", "KI_RUMBO",
                  "MAX_CORRECCION_RUMBO", "RAMPA_MOV_PASO"]),
    dict(n=6, nombre="DESPEJE", tipo="despeje", boton="6   DESPEJE  (derecho)",
         titulo="6 · DESPEJE derecho (el del partido, sin pelota)",
         que_hace="El despeje del partido entero, sin pelota: ida, freno, vuelta hasta la línea, "
                  "enderezarse y el empujoncito.",
         lugar="Delante de la línea del área, como juega. Lugar adelante (~40 cm).",
         numeros=["potenciaDespeje", "msAdelante", "frenarAntesDeVolver", "MS_ESPERA_ANTES_DE_VOLVER",
                  "potenciaRetroceso", "msFreno", "empujonFinal", "potenciaEmpujon", "msEmpujonFinal",
                  "KP_RUMBO", "KI_RUMBO", "MAX_CORRECCION_RUMBO"]),
    dict(n=7, nombre="UBICARSE", tipo="despeje", boton="7   UBICARSE  (arranque)",
         titulo="7 · UBICARSE (el arranque del partido)",
         que_hace="Lo que hace al prender la batería en el partido: atrás hasta la línea, enderezarse "
                  "y avanzar el poquito. Acá NO hace la rutina del medio si no ve la línea.",
         lugar="A ~20 cm delante de la línea del área.",
         numeros=["potenciaRetroceso", "avanzarAlUbicarse", "potenciaEmpujon", "msEmpujonFinal",
                  "msFreno", "KP_RUMBO", "KI_RUMBO"]),
    dict(n=8, nombre="SEGUIR", tipo="seguir", boton="8   SEGUIR  la pelota",
         titulo="8 · SEGUIR la pelota de costado",
         que_hace="Sigue la pelota de costado como el partido, sin despejar, durante msSeguir. Si "
                  "pisa el lateral vuelve al medio como el partido.",
         lugar="En su lugar de juego. Mové la pelota de un lado al otro, a ~40-60 cm.",
         numeros=["kpLateral", "pwmMinLateral", "pwmMaxLateral", "ZONA_MUERTA_PELOTA",
                  "predecirTrayectoria", "msAnticipacionSeguir", "volverAlMedioEnElLateral",
                  "LADO_TRASERA", "msSeguir", "KP_RUMBO", "KI_RUMBO"]),
    dict(n=9, nombre="ATAJAR", tipo="atajar", boton="9   ATAJAR  (mirar y despejar)",
         titulo="9 · ATAJAR (seguir, MIRAR y despejar en diagonal)",
         que_hace="Como el partido: la sigue, a distanciaParaMirar frena y MIRA, y sale en diagonal "
                  "a umbralCm. Si no sale, reintenta. Termina al primer despeje (o a los msSeguir). "
                  "Ojo: si lo frenás con PARAR, el robot guarda ESTE (NO_VALE) y el anterior ya no se "
                  "puede pedir con r.",
         lugar="En su lugar de juego. Después de la cuenta, tirá la pelota hacia el arco.",
         numeros=["umbralCm", "umbralDesvio", "distanciaParaMirar", "medirAntesDeDespejar",
                  "MS_ASENTAR_MIRADA", "MS_MIRAR", "MS_MAX_MIRANDO", "MS_DESCANSO_MIRADA",
                  "despejeEnDiagonal", "predecirEnDiagonal", "rendimientoCostado",
                  "MAX_ANGULO_DIAGONAL", "potenciaDespeje", "msAdelante", "msSeguir"]),
    dict(n=10, nombre="IDA", tipo="ida", boton="10  IDA  (para medir)",
         titulo="10 · IDA (solo la ida del despeje, para medir)",
         que_hace="Solo la ida del despeje (msPruebaIda) y se queda donde llegó (con frenarAntesDeVolver 1 "
                  "frena; con 0 suelta, como la 0921). Para medir con regla cuánto sale.",
         lugar="Cinta en el frente del chasis. Lugar adelante.",
         numeros=["msPruebaIda", "potenciaDespeje", "frenarAntesDeVolver"]),
]
MOV_POR_NUMERO = {m["n"]: m for m in MOVIMIENTOS}
MOV_POR_NOMBRE = {m["nombre"]: m for m in MOVIMIENTOS}
# Los que no arrancan si un sensor de atrás ve blanco (DISENO 4.3, paso 4)
MIRAN_LA_LINEA = (1, 2, 3, 4, 5, 8, 9)

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
#  1b. EL PROGRAMA DE PARTIDO (la 0921): la pestaña PARTIDO
# =====================================================================
#  Pedido del equipo (03/11): "una pestaña nueva para lo que es chequear o
#  cambiar los valores y cargarlo con esos nuevos de forma rápida". Arriba lo
#  fundamental (los sensores y el tamaño de la cancha) y abajo lo demás.
#  De ESTA tabla sale todo (como de TABLA): las filas, los rangos, los de
#  fábrica y la autoprueba. "Fábrica" = el texto TAL CUAL en la 0921f.
#    (nombre, declaración en el .ino, tipo, FÁBRICA, unidad, mínimo, máximo,
#     paso, grupo, qué es, para qué lado, atado)
#    tipo: "i" int · "u" unsigned long · "f" float · "b" bool · "l" lado (-1/+1)
#          · "v" solo para ver (no se escribe nunca)
#  Los mínimos y máximos son SEGUROS: ninguna fuerza pasa de 255 y ningún
#  tiempo deja al robot clavado o yéndose de la cancha.

_CUL = "const unsigned long"
TABLA_PARTIDO = (
    # ---- 1 · LA CANCHA ----
    ("umbralBlanco", "int", "i", "425", "lectura 0-1023", 100, 1000, 5, "cancha",
     "Desde qué lectura un sensor de atrás dice BLANCO (15/09: verde 60-95, blanco ~755). Lo usan TODAS las "
     "líneas: la vuelta del despeje, ubicarse, el límite lateral, ir al medio y la patrulla.",
     "Se pasa de la línea sin verla → BAJAR. Ve blanco en el verde (frena antes, \"pisa\" laterales que no "
     "hay) → SUBIR. Mejor medirlo: CANCHA paso 1.",
     "Con CANCHA medida: ojo si queda a menos de 80 del peor verde o del peor blanco. Copias a mano en otras "
     "pruebas: " + COPIAS_A_MANO["umbralBlanco"] + "."),
    ("MS_MEDIO_AL_MEDIO", _CUL, "u", "2228", "ms", 500, 3200, 10, "cancha",
     "Cuánto va de costado, a fuerza 100, desde el lateral IZQUIERDO hasta el medio. Lo usan ir al medio, la "
     "patrulla y el límite lateral.",
     "No llega al medio (queda del lado del lateral del que salió) → SUBIR. Se pasa del medio → BAJAR. "
     "Miralo en ir al medio o la patrulla: salen del lateral de LADO_MEDIO (−1 = el IZQUIERDO, la izquierda "
     "del robot mirando para donde mira él). El límite lateral sale del que pisó. Se mide: CANCHA paso 2.",
     "ATADO a VEL_MEDIO_MEDIR (100), LADO_TRASERA, RAMPA_MOV_PASO y el rumbo: si se toca uno, medirlo de nuevo. "
     "Desde el lateral DERECHO daba 2331."),
    ("MS_MEDIO_FONDO_POR_TIEMPO", _CUL, "u", "2000", "ms", 500, 4000, 100, "cancha",
     "Ir al medio, paso 3: cuánto retrocede buscando el fondo cuando la rampita del borde no lo deja pisar la "
     "línea. Está A OJO.",
     "No llega a apoyarse en el fondo → SUBIR. Se queda empujando la rampa → BAJAR. Se mide: CANCHA paso 3 "
     "(lo que tarda + 0,5 s).",
     "A fuerza 110 hace 1 cm cada 18 a 24 ms."),
    ("MS_MAX_RETROCESO", _CUL, "u", "1500", "ms", 800, 2500, 100, "cancha",
     "Tope de TODOS los retrocesos hasta la línea del área: la vuelta del despeje, el del arranque y el último "
     "paso de ir al medio. Si ve la línea antes, frena ahí.",
     "La vuelta se corta antes de la línea y queda adelantado → SUBIR. Al arrancar sin línea se va contra el "
     "arco → BAJAR.",
     "Ojo si es menos que 2,4 × msAdelante: la vuelta, a 110, es unas 2,4 veces más lenta que la ida a 200. "
     "Atado a potenciaRetroceso."),
    ("MS_MEDIO_ADELANTE", _CUL, "u", "943", "ms", 300, 1500, 50, "cancha",
     "Ir al medio, paso 4: cuánto avanza desde el fondo para salir del área (~50 cm a fuerza 110).",
     "El área de esta cancha es más honda y queda adentro → SUBIR.",
     "Atado a VEL_MEDIO_RECTO."),
    ("MS_MEDIO_TOPE_LATERAL", _CUL, "u", "6000", "ms", 3000, 9000, 500, "cancha",
     "Ir al medio y patrulla: si yendo de costado no ve el lateral en este tiempo, da por hecho que llegó al "
     "borde.",
     "Cancha más ancha → SUBIR.",
     "Ojo si es menos que 2 × MS_MEDIO_AL_MEDIO, o menos que 1,3 × el cruce medido en CANCHA paso 2."),
    ("CAMARA_POR_CM", "const float", "f", "2.87", "unid. de cámara por cm", 2.00, 4.00, 0.01, "cancha",
     "Traduce lo que ve la cámara a cm de verdad (cámara a ~8 cm del piso, 25/08). Es de la CÁMARA, no de la "
     "cancha.",
     "Tocarlo SOLO si movieron la cámara (CANCHA paso 4). Más grande = cree que la pelota está más cerca.",
     "Cambia TODO lo que está en cm (umbralCm, umbralDesvio, ZONA_MUERTA_PELOTA), y kpLateral está pensado para "
     "2,87: ojo si cambia."),
    # ---- 2 · INTERRUPTORES (en bool, "para qué lado" dice qué pasa con NO) ----
    ("volverAlMedioEnElLateral", "bool", "b", "true", "", None, None, None, "interruptores",
     "LÍMITE LATERAL: siguiendo la pelota, si pisa la línea del costado frena, vuelve al medio (~2,7 s sin mirar "
     "la pelota) y retrocede a la línea del área.",
     "NO = sin límite, como el 21/09: se puede ir de largo y chocar la madera.",
     "Necesita giroscopio. Ojo si NO y pwmMaxLateral 150 o más."),
    ("patrullarSinPelota", "bool", "b", "true", "", None, None, None, "interruptores",
     "PATRULLA: sin pelota y quieto 10 s, va al lateral, vuelve al medio y a la línea del área. Así el juez ve "
     "que está vivo (el 21/09 y el 12/10 lo sacaron por quieto).",
     "NO = quieto para siempre.",
     "Ojo con NO: el juez lo puede sacar por quieto."),
    ("patrullaPorElLateral", "bool", "b", "true", "", None, None, None, "interruptores",
     "Cómo patrulla. SÍ = hasta el lateral y de vuelta al medio (~8 s, ~5 con el arco solo).",
     "NO = PLAN B: solo atrás a la línea, enderezarse y el poquito (~1,3 s; no descubre el arco, pero tampoco "
     "se vuelve a centrar).",
     "Gris si patrullarSinPelota = NO."),
    ("irAlMedioSiNoHayLinea", "bool", "b", "true", "", None, None, None, "interruptores",
     "IR AL MEDIO AL ARRANCAR: si al ubicarse no encuentra la línea del área, hace la rutina (lateral, fondo, "
     "lateral, medio, área).",
     "NO = se queda donde frenó.",
     "Atado a MS_MAX_RETROCESO y a la rutina de ir al medio (grupo 1)."),
    ("avanzarAlUbicarse", "bool", "b", "true", "", None, None, None, "interruptores",
     "Al encontrar la línea del área (al arrancar, después de patrullar o del límite lateral) se endereza y "
     "avanza el poquito (~10 cm).",
     "NO = se queda parado sobre la línea (y la patrulla da por hecho que NO arranca sobre el blanco).",
     "Atado a potenciaEmpujon y msEmpujonFinal."),
    ("perseguirEnElDespeje", "bool", "b", "true", "", None, None, None, "interruptores",
     "En la ida del despeje sigue mirando la pelota y se va corriendo hacia ella.",
     "NO = va derecho a ciegas (la \"b\"). Si hace eses, primero bajar kpPersecucion o PWM_MAX_PERSECUCION.",
     "Atado a kpPersecucion y PWM_MAX_PERSECUCION."),
    ("esperarPlanoAntesDeVolver", "bool", "b", "true", "", None, None, None, "interruptores",
     "Al terminar la ida espera a que el giroscopio diga que quedó PLANO (no hamacándose) y recién ahí vuelve.",
     "NO = espera MS_PAUSA_MEDIO justo y vuelve.",
     "Atado a MS_PAUSA_MEDIO, MS_MAX_ESPERA_PLANO, GRADOS_PLANO y MS_PLANO_SOSTENIDO."),
    ("rampaAlVolver", "bool", "b", "true", "", None, None, None, "interruptores",
     "La vuelta del despeje arranca suave (con rampa), no de un tirón.",
     "NO = como el 21/09 (el 20/10 se midió que así se tuerce 15 a 20°).",
     ""),
    ("empujonFinal", "bool", "b", "true", "", None, None, None, "interruptores",
     "Al volver del despeje y enderezarse, avanza el poquito para no quedar pegado a la línea.",
     "NO = se queda sobre la línea.",
     "Atado a potenciaEmpujon y msEmpujonFinal."),
    ("despejeEnDiagonal", "bool", "b", "false", "", None, None, None, "interruptores",
     "⚠ ESCRITO Y SIN PROBAR. SÍ = calcula un ángulo y sale en diagonal; si la cuenta dice que no llega, NO sale.",
     "NO = sale derecho (como hoy).",
     "Atado a rendimientoCostado y a AVANZADO › Diagonal."),
    # ---- 3 · EL DESPEJE ----
    ("umbralCm", "float", "f", "30.0", "cm", 15, 45, 1, "despeje",
     "Sale a despejar cuando la pelota está a esto o menos.",
     "Que salga antes → SUBIR. Que la espere más cerca → BAJAR.",
     "ATADO a msAdelante: la ida tiene que llegar MÁS LEJOS que esto."),
    ("msAdelante", "int", "i", "533", "ms", 200, 800, 10, "despeje",
     "Cuánto dura la ida. 533 ≈ 50 cm a fuerza 200 (cm ≈ ms/10 − 3,3, medido el 04/08).",
     "No llega a tocar la pelota → SUBIR de a 20. Deja el arco muy solo → BAJAR. Se mide: CANCHA paso 5.",
     "ATADO a umbralCm, a potenciaDespeje (la cuenta de cm es a 200) y a MS_MAX_RETROCESO."),
    ("umbralDesvio", "float", "f", "20.0", "cm", 3, 30, 1, "despeje",
     "Y al costado tiene que estar a esto o menos para salir.",
     "Sale de gusto y deja el arco solo → BAJAR (10 o 12). Duda y no sale → SUBIR.",
     "Con despejeEnDiagonal NO sale siempre DERECHO: con la pelota a 20 cm al costado, derecho no la toca."),
    ("VECES_PARA_CREERLE", "const int", "i", "3", "cuadros", 2, 8, 1, "despeje",
     "Cuántos cuadros SEGUIDOS de la cámara tiene que verla cerca y de frente para salir (un reflejo suelto no "
     "lo dispara).",
     "Sale por reflejos naranjas → SUBIR. Sale tarde → BAJAR.",
     "La cámara manda unos 26 cuadros por segundo."),
    ("potenciaDespeje", "int", "i", "200", "fuerza", 140, 230, 10, "despeje",
     "Fuerza de la ida.",
     "Más = sale más rápido.",
     "msAdelante se calculó a 200. Con persecución, la rueda más cargada recibe esto + el volantazo + la "
     "corrección: pasado 255 se queda corta."),
    ("potenciaRetroceso", "int", "i", "110", "fuerza", 80, 150, 10, "despeje",
     "Fuerza de la vuelta hasta la línea (y del retroceso al arrancar).",
     "Más = vuelve antes, pero frena más pasado de la línea.",
     "Atado a MS_MAX_RETROCESO."),
    ("kpPersecucion", "float", "f", "6.0", "fuerza por cm", 0, 12, 0.5, "despeje",
     "Con perseguirEnElDespeje: cuánto se corre hacia la pelota en la ida, por cada cm que está al costado. "
     "Puesto a ojo.",
     "Hace eses → BAJAR (6 → 4). No corrige → SUBIR.",
     "Gris si perseguirEnElDespeje = NO."),
    ("PWM_MAX_PERSECUCION", "const int", "i", "60", "fuerza", 0, 100, 10, "despeje",
     "Tope de ese volantazo de costado.",
     "Hace eses → BAJAR (60 → 40).",
     "Gris si perseguirEnElDespeje = NO."),
    ("MS_PAUSA_MEDIO", _CUL, "u", "300", "ms", 0, 800, 50, "despeje",
     "Lo MÍNIMO que se queda quieto entre la ida y la vuelta.",
     "Que vuelva más prolijo → SUBIR. Que vuelva antes al arco → BAJAR.",
     "Tiene que ser menos o igual que MS_MAX_ESPERA_PLANO."),
    ("MS_MAX_ESPERA_PLANO", _CUL, "u", "800", "ms", 300, 2000, 50, "despeje",
     "Lo MÁXIMO que espera a quedar plano; a los 800 vuelve igual (no se traba).",
     "Si siempre llega al tope, subir GRADOS_PLANO, no esto.",
     "Tiene que ser más o igual que MS_PAUSA_MEDIO. Gris si no espera plano."),
    ("GRADOS_PLANO", "const float", "f", "1.5", "grados", 0.5, 5.0, 0.25, "despeje",
     "Cuánto puede estar inclinado (comparado con quieto en el piso) y contar como plano.",
     "Espera siempre hasta el tope → SUBIR (2,0). Vuelve hamacándose → BAJAR.",
     "Gris si no espera plano."),
    ("MS_PLANO_SOSTENIDO", _CUL, "u", "100", "ms", 0, 500, 25, "despeje",
     "...y quedarse plano este rato seguido (si se hamaca, pasa por el plano y se va).",
     "",
     "Gris si no espera plano."),
    ("pisoRampaIda", "int", "i", "0", "fuerza", 0, 120, 10, "despeje",
     "La ida arranca desde esta fuerza en vez de 0. 0 = como la b.",
     "Hace un saltito al salir → probar 70, 80. Patina → bajar.",
     ""),
    ("msFreno", "unsigned long", "u", "200", "ms", 100, 400, 25, "despeje",
     "Cuánto sostiene el freno eléctrico cada vez que frena. Lo usan TODOS los frenos.",
     "Se pasa de la línea al frenar → SUBIR (250). Más de 300 calienta el driver.",
     "En el empujoncito el robot queda trabado ese rato (delay)."),
    ("potenciaEmpujon", "int", "i", "100", "fuerza", 70, 150, 10, "despeje",
     "Fuerza del empujoncito final. Despacio a propósito: rápido patina.",
     "",
     "ATADO a msEmpujonFinal."),
    ("msEmpujonFinal", "int", "i", "320", "ms", 0, 600, 20, "despeje",
     "Cuánto dura el empujoncito. 320 ≈ 10 cm a 100 (18/08).",
     "Queda pegado a la línea → SUBIR. Queda muy adelante → BAJAR.",
     "ATADO a potenciaEmpujon."),
    ("MS_ENFRIAMIENTO", _CUL, "u", "1500", "ms", 300, 3000, 100, "despeje",
     "Después de un despeje se queda quieto esto, sin volver a salir.",
     "Sale dos veces seguidas de gusto → SUBIR. Se queda mirando una pelota que vuelve → BAJAR.",
     ""),
    # ---- 4 · SEGUIR LA PELOTA ----
    ("pwmMaxLateral", "int", "i", "160", "fuerza", 80, 170, 10, "seguir",
     "La velocidad MÁXIMA de costado siguiendo la pelota.",
     "Llega tarde a las cruzadas → SUBIR de a 10. Más de 150 casi no se probó: a 160 una vez chocó la madera.",
     "Tiene que ser más o igual que pwmMinLateral."),
    ("pwmMinLateral", "int", "i", "70", "fuerza", 40, 120, 5, "seguir",
     "Lo MÍNIMO que empuja de costado. Con menos las ruedas zumban y no se mueve.",
     "Zumba sin moverse → SUBIR de a 5. Se pasa de la pelota → BAJAR.",
     "Tiene que ser menos o igual que pwmMaxLateral."),
    ("kpLateral", "float", "f", "11.5", "fuerza por cm", 4, 25, 0.5, "seguir",
     "Cuánto más empuja por cada cm que la pelota está al costado.",
     "Llega tarde → SUBIR. Va y viene → BAJAR. NO \"corregirlo\" a 4: está en cm reales (4 × 2,87 = 11,5).",
     "Atado a CAMARA_POR_CM."),
    ("ZONA_MUERTA_PELOTA", "const float", "f", "1.4", "cm", 0.4, 5.0, 0.2, "seguir",
     "Con la pelota a menos de esto del frente, se queda quieto, para no temblar con el ruido de la cámara. "
     "También la usa la ida.",
     "Tiembla con la pelota quieta → SUBIR. No se acomoda con la pelota apenas corrida → BAJAR.",
     ""),
    # ---- 5 · PATRULLA Y LÍMITE LATERAL ----
    ("MS_QUIETO_PARA_PATRULLAR", _CUL, "u", "10000", "ms", 5000, 60000, 1000, "patrulla",
     "Si pasa este rato sin moverse (y sin pelota), sale a patrullar.",
     "Si el juez aguanta más quieto → SUBIR: cada patrulla deja el arco solo ~5 s. Preguntarle al juez.",
     "Gris si patrullarSinPelota = NO."),
    ("MS_SIN_PELOTA_PARA_PATRULLAR", _CUL, "u", "3000", "ms", 1000, 10000, 500, "patrulla",
     "...y además tiene que llevar este rato SEGUIDO sin ver la pelota (si un rival se la tapa un instante, "
     "no sale).",
     "",
     "Gris si patrullarSinPelota = NO."),
    ("CUADROS_PARA_CORTAR_PATRULLA", "const int", "i", "5", "cuadros", 2, 15, 1, "patrulla",
     "Si patrullando ve la pelota estos cuadros seguidos (~0,2 s), corta la patrulla y la sigue.",
     "Corta por reflejos → SUBIR. Tarda en reaccionar → BAJAR.",
     "Gris si patrullarSinPelota = NO."),
    ("LECTURAS_PARA_CREER_LATERAL", "const int", "i", "3", "lecturas", 1, 10, 1, "patrulla",
     "Límite lateral: cuántas vueltas seguidas pisando el lateral necesita para creerle.",
     "Vuelve al medio sin haber llegado al lateral → SUBIR. Se pasa de la línea → BAJAR.",
     "Gris si volverAlMedioEnElLateral = NO."),
    # ---- 6 · AVANZADO (plegado: casi nunca se toca) ----
    ("lateralInvertido", "bool", "b", "false", "", None, None, None, "avanzado",
     "Tocar SOLO si, cuando el programa dice \"derecha\", el robot va a la IZQUIERDA (MOVIMIENTOS, mov. 2).",
     "NO = como se midió el 11/08 (el mov. 2 fue a la DERECHA).",
     "¡CAMBIA UN SIGNO! Pide confirmar."),
    ("camaraYInvertida", "bool", "b", "true", "", None, None, None, "avanzado",
     "Tocar SOLO si el mov. 2 va bien y aun así, siguiendo la pelota, se ALEJA de ella.",
     "SÍ = como se midió el 11/08 (pelota a la derecha, Yp = −24).",
     "¡CAMBIA UN SIGNO! Pide confirmar."),
    ("enderezarActivado", "bool", "b", "true", "", None, None, None, "avanzado",
     "Apaga TODA la corrección del giroscopio: sin esto no hay límite lateral, ni patrulla, ni ir al medio, ni "
     "se endereza. Solo para pruebas.",
     "NO = sin giroscopio: solo para pruebas.",
     "Pide confirmar."),
    ("KP_RUMBO", "const float", "f", "5.0", "por grado", 1, 10, 0.5, "avanzado",
     "Corrección por cada grado torcido.",
     "Zigzaguea → BAJAR de a 0,5. Se le escapa → SUBIR.",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("KI_RUMBO", "const float", "f", "1.5", "", 0, 3, 0.25, "avanzado",
     "Corrige la torcida que queda fija para un lado.",
     "",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("LIMITE_INTEGRAL", "const float", "f", "30.0", "", 10, 40, 5, "avanzado",
     "Tope de lo que acumula KI_RUMBO.",
     "",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("MAX_CORRECCION_RUMBO", "const int", "i", "100", "fuerza", 40, 120, 10, "avanzado",
     "Tope de la corrección.",
     "",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("ZONA_MUERTA_RUMBO", "const float", "f", "1.0", "grados", 0.25, 3, 0.25, "avanzado",
     "Grados que deja pasar sin corregir.",
     "",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("TOLERANCIA_ACOMODO", "const float", "f", "3.0", "grados", 1, 6, 0.5, "avanzado",
     "Se endereza hasta quedar a esto o menos.",
     "",
     "Tiene que ser MENOS que TOLERANCIA_ESPERA."),
    ("TOLERANCIA_ESPERA", "const float", "f", "8.0", "grados", 4, 20, 1, "avanzado",
     "Esperando la pelota, si está más torcido que esto se endereza solo. Son distintos a propósito: si fueran "
     "iguales tiembla.",
     "",
     "Tiene que ser MÁS que TOLERANCIA_ACOMODO."),
    ("KP_ACOMODO", "const float", "f", "1.5", "fuerza por grado", 0.5, 4, 0.25, "avanzado",
     "Enderezándose: cuánta fuerza por cada grado torcido.",
     "",
     ""),
    ("PWM_MIN_ACOMODO", "const int", "i", "75", "fuerza", 70, 120, 5, "avanzado",
     "Lo mínimo que empuja para enderezarse. NO bajar de 70: debajo las ruedas zumban y no gira (el bug del "
     "01/09).",
     "",
     "Tiene que ser menos o igual que PWM_MAX_ACOMODO."),
    ("PWM_MAX_ACOMODO", "const int", "i", "85", "fuerza", 70, 150, 5, "avanzado",
     "Lo máximo que empuja para enderezarse.",
     "Gira muy rápido y se pasa (oscila) → BAJAR.",
     "Tiene que ser más o igual que PWM_MIN_ACOMODO."),
    ("MS_ASENTAR_ACOMODO", _CUL, "u", "300", "ms", 100, 1000, 50, "avanzado",
     "Espera que pase la inercia antes de volver a medir.",
     "",
     "Ojo si es menos que msFreno."),
    ("MS_TIMEOUT_ACOMODO", _CUL, "u", "3000", "ms", 1000, 6000, 250, "avanzado",
     "Tope de cada intento de enderezarse.",
     "",
     ""),
    ("MAX_REINTENTOS_ACOMODO", "const int", "i", "3", "veces", 0, 6, 1, "avanzado",
     "Cuántas veces reintenta enderezarse.",
     "",
     ""),
    ("MS_DESCANSO_ENDEREZAR", _CUL, "u", "6000", "ms", 2000, 20000, 500, "avanzado",
     "Esperando: descanso obligatorio entre intentos (mientras tanto mira la pelota).",
     "",
     ""),
    ("LADO_TRASERA", "const int", "i", "89", "%", 80, 100, 1, "avanzado",
     "De costado, cuánto empuja la rueda de atrás (50/50/89).",
     "Si de costado gira solo: 2 más o 2 menos.",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO."),
    ("RAMPA_MOV_PASO", "const int", "i", "10", "fuerza cada 10 ms", 2, 20, 1, "avanzado",
     "Arranque suave: cuánto sube la fuerza cada 10 ms.",
     "",
     "Cambia lo medido de MS_MEDIO_AL_MEDIO y de msAdelante."),
    ("LADO_MEDIO", "const int", "l", "-1", "−1 izq / +1 der", -1, 1, None, "avanzado",
     "Para qué lateral va a buscar la línea.",
     "Cambiar SOLO si el lateral IZQUIERDO de esta cancha no tiene línea. Pide confirmar.",
     "MS_MEDIO_AL_MEDIO se midió desde el IZQUIERDO."),
    ("VEL_MEDIO_BUSCAR", "const int", "i", "110", "fuerza", 80, 160, 10, "avanzado",
     "De costado, hasta ver una línea.",
     "",
     ""),
    ("VEL_MEDIO_MEDIR", "const int", "v", "100", "fuerza", None, None, None, "avanzado",
     "SOLO PARA VER: MS_MEDIO_AL_MEDIO se midió a esta fuerza, y CANCHA paso 2 siempre mide a 100.",
     "",
     "No se edita."),
    ("VEL_MEDIO_RECTO", "const int", "i", "110", "fuerza", 80, 150, 10, "avanzado",
     "Atrás y adelante en la rutina de ir al medio.",
     "",
     "Atado a MS_MEDIO_ADELANTE."),
    ("MS_MEDIO_ALEJARSE", _CUL, "u", "400", "ms", 100, 1000, 50, "avanzado",
     "Cuánto se despega del lateral antes de ir al fondo.",
     "",
     ""),
    ("MS_MEDIO_QUIETO", _CUL, "u", "300", "ms", 0, 1000, 50, "avanzado",
     "Quieto entre paso y paso.",
     "",
     ""),
    ("rendimientoCostado", "float", "f", "0.80", "", 0.40, 1.20, 0.05, "avanzado",
     "VA AL REVÉS: más chico = MÁS costado.",
     "Le falta costado → BAJAR de a 0,05.",
     "Gris sin diagonal."),
    ("PWM_MIN_AVANCE_DESPEJE", "const int", "i", "140", "fuerza", 100, 200, 10, "avanzado",
     "Si para entrar en el motor el avance baja de esto, no sale.",
     "",
     "Gris sin diagonal."),
    ("COSTADO_MINIMO_UTIL", "const int", "i", "20", "fuerza", 0, 60, 5, "avanzado",
     "Con menos costado que esto, sale derecho.",
     "",
     "Gris sin diagonal."),
    ("MS_MAX_ADELANTE", _CUL, "u", "800", "ms", 400, 1200, 50, "avanzado",
     "Tope de la ida cuando la diagonal la estira.",
     "",
     "Ojo si es menos que msAdelante. Gris sin diagonal."),
    ("MS_AVISO_ARMADO", _CUL, "u", "2000", "ms", 1000, 10000, 500, "avanzado",
     "Al prender, cuánto parpadea antes de empezar a moverse.",
     "Para tener más tiempo de soltarlo → SUBIR.",
     ""),
    ("PWM_CUENTA_COMO_MOVERSE", "const int", "i", "60", "fuerza", 30, 100, 5, "avanzado",
     "Para el reloj de la patrulla: con menos que esto en las tres ruedas, cuenta como que \"no se movió\".",
     "",
     ""),
)
FILA_PARTIDO = {f[0]: f for f in TABLA_PARTIDO}
ORDEN_PARTIDO = tuple(f[0] for f in TABLA_PARTIDO)
TIPO_PARTIDO = {f[0]: f[2] for f in TABLA_PARTIDO}
VALORES_DE_FABRICA = {f[0]: f[3] for f in TABLA_PARTIDO}      # "Volver a los de fábrica"
# (clave, título, arranca plegado)
GRUPOS_PARTIDO = (
    ("cancha", "1 · LA CANCHA (lo de cada cancha)", False),
    ("interruptores", "2 · INTERRUPTORES (SÍ / NO)", False),
    ("despeje", "3 · EL DESPEJE", False),
    ("seguir", "4 · SEGUIR LA PELOTA", False),
    ("patrulla", "5 · PATRULLA Y LÍMITE LATERAL", False),
    ("avanzado", "6 · AVANZADO (cerrado: casi nunca se toca)", True),
)
AVISO_AVANZADO = "Casi nunca se toca. Cambiarlo cambia lo medido."
# Los subtítulos de AVANZADO: van ANTES de ese nombre.
SUBTITULOS_PARTIDO = {"lateralInvertido": "Signos", "enderezarActivado": "Giroscopio",
                      "TOLERANCIA_ACOMODO": "Enderezarse", "LADO_TRASERA": "Movimiento",
                      "LADO_MEDIO": "Ir al medio", "rendimientoCostado": "Diagonal (solo con despejeEnDiagonal = SÍ)",
                      "MS_AVISO_ARMADO": "Arranque"}
# La fila se ve GRIS (no se usa) si TODOS estos interruptores están en NO.
GRIS_SI_PARTIDO = {
    "patrullaPorElLateral": ("patrullarSinPelota",), "MS_QUIETO_PARA_PATRULLAR": ("patrullarSinPelota",),
    "MS_SIN_PELOTA_PARA_PATRULLAR": ("patrullarSinPelota",), "CUADROS_PARA_CORTAR_PATRULLA": ("patrullarSinPelota",),
    "LECTURAS_PARA_CREER_LATERAL": ("volverAlMedioEnElLateral",),
    "kpPersecucion": ("perseguirEnElDespeje",), "PWM_MAX_PERSECUCION": ("perseguirEnElDespeje",),
    "MS_MAX_ESPERA_PLANO": ("esperarPlanoAntesDeVolver",), "GRADOS_PLANO": ("esperarPlanoAntesDeVolver",),
    "MS_PLANO_SOSTENIDO": ("esperarPlanoAntesDeVolver",),
    "rendimientoCostado": ("despejeEnDiagonal",), "PWM_MIN_AVANCE_DESPEJE": ("despejeEnDiagonal",),
    "COSTADO_MINIMO_UTIL": ("despejeEnDiagonal",), "MS_MAX_ADELANTE": ("despejeEnDiagonal",),
    "potenciaEmpujon": ("empujonFinal", "avanzarAlUbicarse"), "msEmpujonFinal": ("empujonFinal", "avanzarAlUbicarse"),
}
# Los que piden confirmar en una ventanita: nombre -> (con qué valor pregunta, o None = con cualquier cambio; texto)
CONFIRMAR_PARTIDO = {
    "despejeEnDiagonal": ("true", "despejeEnDiagonal está ESCRITO Y SIN PROBAR: con SÍ calcula un ángulo y sale "
                                  "en diagonal, y si la cuenta dice que no llega, NO sale.\n\n¿Seguro?"),
    "lateralInvertido": (None, "¡CAMBIA UN SIGNO! Tocalo SOLO si, en MOVIMIENTOS, el movimiento 2 (costado "
                               "DERECHA) fue a la IZQUIERDA. Si no, el robot se aleja de la pelota.\n\n¿Seguro?"),
    "camaraYInvertida": (None, "¡CAMBIA UN SIGNO! Tocalo SOLO si el movimiento 2 va bien y aun así, siguiendo la "
                               "pelota, se ALEJA de ella.\n\n¿Seguro?"),
    "enderezarActivado": ("false", "Con NO se apaga TODA la corrección del giroscopio: no hay límite lateral, ni "
                                   "patrulla, ni ir al medio, ni se endereza. Es solo para pruebas.\n\n¿Seguro?"),
    "LADO_MEDIO": (None, "Cambiar SOLO si el lateral IZQUIERDO de esta cancha no tiene línea. OJO: "
                         "MS_MEDIO_AL_MEDIO está medido desde el IZQUIERDO (desde el derecho daba 2331).\n\n¿Seguro?"),
}
# Variables de ESTADO de la 0921: NUNCA van en la tabla aunque parezcan
# valores (el programa las pisa en cada despeje). Lo mira la autoprueba.
ESTADO_DEL_PARTIDO = ("rumboBase", "hayGiroscopo", "giroscopoRespondiendo", "msIdaDespeje", "pwmAvanceDespeje",
                      "lateralDelDespeje", "ubicandose", "encontroLaLinea", "empujonDeArranque", "patrullando",
                      "cuadrosConPelota", "lecturasDeLateral", "ladoDelLateral", "vecesSeguidas", "cuantos",
                      "sincronizado", "Xp", "Yp", "integralRumbo", "pwmAcomodoAplicado", "reintentosAcomodo",
                      "enderezandoEnEspera", "incBaseY", "incBaseZ", "hayIncBase", "potenciaRampa",
                      "t_ultimaVersion", "msAdelanteChico", "t_fase", "VERSION_PARTIDO", "SELLO")
# Los que la línea #PARTIDO imprime con decimales (los demás van enteros)
DECIMALES_LINEA_PARTIDO = {"CAMARA_POR_CM": 2, "umbralCm": 1, "umbralDesvio": 1, "kpLateral": 2}



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
# 2026-11-03: el delantero solo aceptaba MAYÚSCULAS. El arquero usa los
# nombres del panel tal cual (umbralCm, MS_MIRAR), así que acá van las dos.
_RE_NOMBRE = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
_RE_MAYUS = re.compile(r"^[A-Z][A-Z0-9_]*$")
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


def _bit(texto):
    """'0' -> False, '1' -> True. Otra cosa: el renglón vino roto."""
    v = _entero(texto)
    if v not in (0, 1):
        raise ValueError("se esperaba 0 o 1: %r" % texto)
    return v == 1


def _lista(texto, cuantos):
    """'310,322,705' -> [310, 322, 705]. Tiene que tener justo 'cuantos'.
    Un 'nan' adentro queda None."""
    partes = texto.split(",")
    if len(partes) != cuantos:
        raise ValueError("se esperaban %d números en %r" % (cuantos, texto[:30]))
    return [leer_numero(p) for p in partes]


def _lista_num(texto, cuantos):
    """Como _lista, pero sin 'nan': todos números de verdad."""
    partes = texto.split(",")
    if len(partes) != cuantos:
        raise ValueError("se esperaban %d números en %r" % (cuantos, texto[:30]))
    return [_numero(p) for p in partes]


def _pares(resto):
    """'t=12 e=ESPERA' -> {'t': '12', 'e': 'ESPERA'} (en el orden en que
    vinieron). Un pedazo sin '=' o sin valor quiere decir que el renglón
    vino roto."""
    d = {}
    for pedazo in resto.split():
        clave, igual, valor = pedazo.partition("=")
        if not igual or not clave or not valor:
            raise ValueError("pedazo roto: %r" % pedazo[:30])
        d[clave] = valor
    return d


def _i_hola(resto):
    partes = resto.split()
    if not partes:
        raise ValueError("#HOLA sin nombre")
    m = {"nombre": partes[0], "fecha": "", "serie": None, "partido": None}
    resto_p = partes[1:]
    if resto_p and "=" not in resto_p[0]:
        m["fecha"] = resto_p[0]
        resto_p = resto_p[1:]
    for pedazo in resto_p:
        clave, igual, valor = pedazo.partition("=")
        if igual and clave in ("serie", "partido") and valor:
            m[clave] = valor
    return m


def _i_s(resto):
    d = _pares(resto)
    claves = list(d)
    # t va PRIMERO y e va ÚLTIMO: si no, el renglón se cortó (o se pegó).
    if not claves or claves[0] != "t" or claves[-1] != "e":
        raise ValueError("t= tiene que ir primero y e= último (renglón cortado)")
    if d["e"] not in MODOS:
        raise ValueError("e= desconocido: %r" % d["e"][:20])
    m = {"t": _entero(d["t"]), "e": d["e"]}
    m["l"] = _lista(d["l"], 3) if "l" in d else None
    # u es UN número en el arquero (el delantero manda tres: ese #S es ROTA acá)
    m["u"] = _numero(d["u"]) if "u" in d else None
    r = leer_numero(d["r"]) if "r" in d else None
    m["r"] = float(r) if (r is not None and r >= 0) else None      # -1 = mudo o sin giroscopio
    g = leer_numero(d["g"]) if "g" in d else None
    m["g"] = int(g) if (g is not None and g >= 0) else None        # -1 = no contesta
    if "cal" in d:
        cal = _lista(d["cal"], 4)
        m["cal"] = None if any(c is None or c < 0 for c in cal) else [int(c) for c in cal]
    else:
        m["cal"] = None
    for clave in ("p", "am", "az"):
        if clave in d:
            x, y, edad = _lista(d[clave], 3)
            m[clave] = (x, y, -1 if edad is None else edad)       # edad -1 = nunca se vio
        else:
            m[clave] = None
    dd = leer_numero(d["d"]) if "d" in d else None
    m["d"] = float(dd) if (dd is not None and dd >= 0) else None   # d < 0: nunca la vio
    for clave in ("y", "vy", "va"):
        v = leer_numero(d[clave]) if clave in d else None
        m[clave] = float(v) if v is not None else None
    m["ve"] = _bit(d["ve"]) if "ve" in d else None
    m["n"] = _entero(d["n"]) if "n" in d else None
    if "dg" in d:
        partes = d["dg"].split(",")
        if len(partes) != 5:
            raise ValueError("dg= lleva 5 números")
        m["dg"] = tuple(_entero(p) for p in partes)     # sale, motivo, avance, lateral, msIda
    else:
        m["dg"] = None
    m["q"] = leer_numero(d["q"]) if "q" in d else None
    return m


def _i_valores(resto):
    valores = {}
    for pedazo in resto.split():
        nombre, igual, valor = pedazo.partition("=")
        if not igual or not _RE_NOMBRE.match(nombre):
            raise ValueError("pedazo roto: %r" % pedazo[:30])
        _numero(valor)                    # que sea un número de verdad
        valores[nombre] = valor           # se guarda como lo escribió el robot ("0.8")
    return valores


def _i_v(resto):
    valores = _i_valores(resto)
    if not valores:
        raise ValueError("#V vacío")
    return {"valores": valores}


def _i_def(resto):
    valores = _i_valores(resto)
    if not valores:
        raise ValueError("#DEF vacío")
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


def _i_giro(resto):
    partes = resto.split()
    if partes == ["buscando"]:
        return {"buscando": True, "ok": None, "estado": None}
    d = _pares(resto)
    ok = _bit(d["ok"])
    estado = _entero(d["estado"]) if "estado" in d else -1
    return {"buscando": False, "ok": ok, "estado": estado if estado >= 0 else None}


def _i_cuenta(resto):
    partes = resto.split()
    if len(partes) != 2 or partes[1] not in ("cable", "sincable"):
        raise ValueError("#CUENTA lleva segundos y cable/sincable")
    return {"segundos": float(_numero(partes[0])), "modo": partes[1], "sincable": partes[1] == "sincable"}


def _i_inicio(resto):
    partes = resto.split()
    if len(partes) < 2 or partes[0] not in CODIGOS or not _RE_MAYUS.match(partes[1]):
        raise ValueError("#INICIO lleva código y NOMBRE")
    d = _pares(" ".join(partes[2:]))
    return {"codigo": partes[0], "nombre": partes[1], "tope": _entero(d["tope"]),
            "sincable": _bit(d["sincable"])}


def _i_paso(resto):
    partes = resto.split()
    if not partes or partes[0] not in CODIGOS:
        raise ValueError("#PASO lleva el código")
    palabras, datos = [], {}
    for pedazo in partes[1:]:
        if "=" in pedazo:
            clave, _, valor = pedazo.partition("=")
            if not clave or not valor:
                raise ValueError("pedazo roto: %r" % pedazo[:30])
            if _RE_NUMERO.match(valor):
                datos[clave] = _numero(valor)          # 400 cifras: ValueError
            elif len(valor) <= LARGO_MAX_NUMERO:
                datos[clave] = valor                   # como=L
            else:
                raise ValueError("valor larguísimo")
        else:
            palabras.append(pedazo)
    return {"codigo": partes[0], "palabras": palabras, "datos": datos}


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


def _i_mire(resto):
    d = _pares(resto)
    if d["res"] not in RESULTADOS_MIRE:
        raise ValueError("res= desconocido: %r" % d["res"][:20])
    m = {"k": _entero(d["k"]), "res": d["res"]}
    for clave in CAMPOS_MIRE:
        if clave not in m:
            m[clave] = leer_numero(d[clave]) if clave in d else None
    return m


def _i_r(resto):
    d = _pares(resto)
    for clave in ("n", "mov", "nota"):
        if clave not in d:
            raise ValueError("falta %s= (renglón cortado)" % clave)
    if d["nota"] not in NOTAS:
        raise ValueError("nota desconocida: %r" % d["nota"][:20])
    n = _entero(d["n"])
    if not 1 <= n <= 10 or not _RE_MAYUS.match(d["mov"]):
        raise ValueError("#R de un movimiento que no existe")
    como = d.get("como")
    if como is not None and not _RE_MAYUS.match(como):
        raise ValueError("como= raro")
    m = {"n": n, "mov": d["mov"], "nota": d["nota"], "como": como}
    for clave in CAMPOS_R:
        if clave not in CAMPOS_R_TEXTO:
            m[clave] = leer_numero(d[clave]) if clave in d else None
    return m


def _i_usados(resto):
    return {"valores": _i_valores(resto)}           # puede venir vacío: "#USADOS"


def _i_consejo(resto):
    return {"mensaje": resto.strip()}


def _i_fin(resto):
    partes = resto.split()
    if len(partes) != 1 or partes[0] not in CODIGOS:
        raise ValueError("#FIN lleva el código")
    return {"codigo": partes[0]}


def _i_repite(resto):
    partes = resto.split()
    if len(partes) < 2 or partes[0] not in CODIGOS or not _RE_MAYUS.match(partes[1]):
        raise ValueError("#REPITE lleva código y NOMBRE")
    d = _pares(" ".join(partes[2:]))
    return {"codigo": partes[0], "nombre": partes[1], "sincable": _bit(d["sincable"]),
            "apagado": _bit(d["apagado"])}


def _i_guardado(resto):
    d = _pares(resto)
    if d["que"] not in ("mov", "cancha"):
        raise ValueError("que= tiene que ser mov o cancha")
    m = {"que": d["que"], "hay": _bit(d["hay"]), "n": None, "nombre": None, "nota": None,
         "sincable": False, "nuevo": False, "mismo": False, "hace": None}
    if m["hay"]:
        if d["n"] not in CODIGOS:
            raise ValueError("n= desconocido")
        m["n"] = d["n"]
        m["nombre"] = d.get("nombre")
        m["nota"] = d.get("nota")
        m["sincable"] = _bit(d.get("sincable", "0"))
        m["nuevo"] = _bit(d.get("nuevo", "0"))
        m["mismo"] = _bit(d.get("mismo", "0"))
        hace = _entero(d.get("hace", "-1"))
        m["hace"] = hace if hace >= 0 else None
    return m


def _i_lin(resto):
    d = _pares(resto)
    if d["fase"] not in ("v", "i", "d"):
        raise ValueError("fase= tiene que ser v, i o d")
    m = {"fase": d["fase"], "n": _entero(d["n"])}
    for clave in ("izq", "der", "ade"):
        m[clave] = _lista_num(d[clave], 3)
    return m


def _i_ancho(resto):
    d = _pares(resto)
    m = {"ok": _bit(d["ok"]), "error": _entero(d["error"])}
    for clave in ("medioizq", "izqder", "derizq", "almedioizq", "almedioder", "endereza",
                  "vel", "rampa", "umbral", "calgiro"):
        m[clave] = _entero(d[clave]) if clave in d else None
    m["desvio"] = _lista_num(d["desvio"], 4) if "desvio" in d else None
    m["golpe"] = _numero(d["golpe"]) if "golpe" in d else None
    return m


def _i_fondo(resto):
    d = _pares(resto)
    if d["como"] not in ("TOCO", "LINEA", "TOPE", "TIEMPO", "PARADO"):
        raise ValueError("como= desconocido")
    m = {"ms": _entero(d["ms"]), "como": d["como"]}
    m["desvio"] = _numero(d["desvio"]) if "desvio" in d else None
    m["umbral"] = _entero(d["umbral"]) if "umbral" in d else None
    m["probado"] = _entero(d["probado"]) if "probado" in d else 0
    return m


def _i_cam(resto):
    d = _pares(resto)
    m = {"n": _entero(d["n"]), "cero": _entero(d["cero"]), "val": _entero(d["val"]),
         "xp": _lista_num(d["xp"], 3)}
    m["yp"] = _numero(d["yp"]) if "yp" in d else None
    m["am"] = _entero(d["am"]) if "am" in d else 0
    m["az"] = _entero(d["az"]) if "az" in d else 0
    return m


def _i_medio(resto):
    d = _pares(resto)
    pasos = [int(p) for p in _lista_num(d["pasos"], 8)]
    como = d["como"].split(",")
    if len(como) != 8 or any(c not in ("L", "T", "-") for c in como):
        raise ValueError("como= lleva 8 letras L, T o -")
    m = {"ok": _bit(d["ok"]), "pasos": pasos, "como": como}
    m["desvio"] = _numero(d["desvio"]) if "desvio" in d else None
    m["acomodo"] = _entero(d["acomodo"]) if "acomodo" in d else 0
    return m


def _i_panel(resto):
    # Tal cual, con sus espacios: es una línea de C++ para pegar.
    return {"codigo": resto}


def _i_finpanel(resto):
    return {}


# Para la #PARTIDO: [0-9] y no \d (en Python \d acepta también dígitos árabes).
_RE_NUMERO_ASCII = re.compile(r"^-?[0-9]+(?:\.[0-9]+)?$")


def _i_partido(resto):
    """La línea de versión del programa de PARTIDO (0921g), NO de tablero.ino:
    #PARTIDO version=0921g sello=fabrica giro=1 umbralBlanco=425 ... interruptores=1111111110101
    version PRIMERO e interruptores ÚLTIMO: si no, vino cortada (o pegada).
    Las claves de más se aceptan y se ignoran (para el futuro)."""
    d = _pares(resto)
    claves = list(d)
    if not claves or claves[0] != "version" or claves[-1] != "interruptores":
        raise ValueError("version= tiene que ir primero e interruptores= último (renglón cortado)")
    if not re.match(r"^[0-9A-Za-z.]{1,12}$", d["version"]):
        raise ValueError("version= rara")
    if not re.match(r"^[A-Za-z0-9-]{1,20}$", d["sello"]):
        raise ValueError("sello= raro")
    if not re.match(r"^[01]{%d}$" % len(ORDEN_INTERRUPTORES), d["interruptores"]):
        raise ValueError("interruptores= lleva %d ceros o unos" % len(ORDEN_INTERRUPTORES))
    m = {"version": d["version"], "sello": d["sello"], "giro": _bit(d["giro"]), "valores": {}, "decimales": {}}
    for nombre in NUMEROS_LINEA_PARTIDO:
        t = d[nombre]
        if len(t) > LARGO_MAX_NUMERO or not _RE_NUMERO_ASCII.match(t):
            raise ValueError("%s= no es un número" % nombre)
        m["valores"][nombre] = _numero(t)
        m["decimales"][nombre] = _decimales(t)
    m["interruptores"] = [c == "1" for c in d["interruptores"]]
    m["llaves"] = dict(zip(ORDEN_INTERRUPTORES, m["interruptores"]))
    return m


_INTERPRETES = {
    "HOLA": _i_hola, "S": _i_s, "V": _i_v, "DEF": _i_def, "OK": _i_ok, "ERROR": _i_error,
    "GIRO": _i_giro, "CUENTA": _i_cuenta, "INICIO": _i_inicio, "PASO": _i_paso, "FRENO": _i_freno,
    "M": _i_m, "MIRE": _i_mire, "R": _i_r, "USADOS": _i_usados, "CONSEJO": _i_consejo,
    "FIN": _i_fin, "REPITE": _i_repite, "GUARDADO": _i_guardado, "LIN": _i_lin, "ANCHO": _i_ancho,
    "FONDO": _i_fondo, "CAM": _i_cam, "MEDIO": _i_medio, "PANEL": _i_panel, "FINPANEL": _i_finpanel,
    "PARTIDO": _i_partido,
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


def reconocer_programa(texto):
    """Los renglones para PERSONAS que dicen qué programa tiene el robot:
       'PARTIDO 0.1103 EN PRUEBA (...)'  el de partido (o 'PARTIDO' solo,
                                         si salió el registro de despejes)
       'AJUSTAR'                         ajustar-movimientos
       'TABLERO_ARQUERO'                 el cartel de setup() de este tablero
       None                              cualquier otra cosa."""
    t = (texto or "").strip()
    if not t.startswith("==="):
        return None
    cuerpo = t.lstrip("=").strip()
    m = re.match(r"ARQUERO\s+version\s*(.*)$", cuerpo)
    if m:
        version = m.group(1).rstrip("=").strip()
        return ("PARTIDO " + version).strip()
    if cuerpo.startswith("REGISTRO DE DESPEJES"):
        return "PARTIDO"
    if cuerpo.startswith("AJUSTAR-MOVIMIENTOS"):
        return "AJUSTAR"
    if cuerpo.startswith("TABLERO DEL ARQUERO"):
        return "TABLERO_ARQUERO"
    return None


# =====================================================================
#  3. LA MEMORIA: lo que se sabe del robot
# =====================================================================

# Los renglones que son parte de una TANDA de resultado (lo que manda el
# robot al terminar, o al repetirlo con r / rc).
_DE_LA_TANDA = ("M", "MIRE", "R", "USADOS", "CONSEJO", "LIN", "ANCHO", "FONDO", "CAM", "MEDIO")
_CODIGO_DE_LA_MEDICION = {"LIN": "L", "ANCHO": "A", "FONDO": "F", "CAM": "C", "MEDIO": "I"}


class Estado:
    """Junta lo que va llegando. aplicar() devuelve qué cambió, para que la
    ventana redibuje solo eso:
       "hola" "valores" "defectos" "sensores" "modo" "resultado" "freno" "ok"
       "error" "panel_listo" "rota" "giro" "guardado" "pasos" "cancha"
       "cancha_lista" "fin" "sin_cable_perdido" "partido"
    No toca la ventana: por eso la autoprueba la puede probar sola."""

    def __init__(self):
        self.valores = {}            # NOMBRE -> "valor" como lo mandó el robot (#V)
        self.defectos = {}           # NOMBRE -> "valor" de las copias del partido (#DEF)
        self.guardado = {"mov": None, "cancha": None}    # los últimos #GUARDADO
        # SIN CABLE: la prueba se pidió sin cable y todavía no llegó su #FIN.
        # Mientras tanto el desenchufe y el silencio son LO ESPERADO.
        self.esperando_sin_cable = False
        self.que_sin_cable = None    # "mov" o "cancha" (lo anota la ventana al pedirla)
        self.t_sin_cable = None      # cuándo empezó esa cuenta SIN CABLE (reloj de la PC)
        # Los resultados que ya llegaron EN VIVO en esta ventana: (que, código,
        # cuándo llegó el #FIN). Una prueba SIN CABLE queda nuevo=1 en el robot
        # aunque la tanda haya salido en vivo (el Teensy no sabe si lo
        # desenchufaron); con esto no se pide de nuevo lo que ya se vio.
        self.vistos = []
        self.cancha = {}             # la última de cada medición: "L" -> {fase: #LIN}, "A" -> #ANCHO...
        self.cancha_lista = None     # (código, [mediciones]) de la última tanda de cancha completa
        self.panel = []              # las #PANEL juntadas
        self.rotas = 0
        self.ultimo_ok = None
        self.ultimo_error = None
        self._nuevo_resultado()
        self.reiniciar_conexion()

    def reiniciar_conexion(self):
        """Se enchufó (o se reenchufó) el robot: lo que dependía de él se
        olvida. El último resultado queda (para no perder el gráfico), y
        también esperando_sin_cable: en la cancha se desenchufa A PROPÓSITO."""
        self.hola = None             # (programa, fecha, serie, partido)
        self.sensores = None         # el último #S
        self.t_sensores = None       # cuándo llegó (reloj de la PC)
        self.modo = "ESPERA"
        self.cuenta = None
        self.cuenta_modo = None
        self.panel_abierto = False
        self.giro = None             # el último #GIRO
        self.repetir = []            # "r" / "rc" que hay que pedir (por un #GUARDADO nuevo=1)
        self.tanda_abierta = False   # llegó el principio de una tanda y todavía no su #FIN
        self.partido_robot = None    # la última #PARTIDO (el programa de partido 0921g o posterior)

    def _nuevo_resultado(self):
        self.codigo = None           # "1".."10" o "L" "A" "F" "C" "I"
        self.mov_nombre = None
        self.freno = None            # motivo del último #FRENO
        self.muestras = []           # [(t_ms, grados, fase)] de las #M
        self.informe = None          # el último #R
        self.intentos = []           # los #MIRE (solo la 9)
        self.usados = None           # el #USADOS: {nombre: valor}
        self.consejos = []
        self.pasos = []              # los #PASO (A e I)
        self.tanda_cancha = []       # las #LIN #ANCHO #FONDO #CAM #MEDIO de esta tanda
        self.completo = False        # llegó el #FIN
        self.de_guardado = False     # vino de un #REPITE (pedido con r / rc)
        self.sincable = False
        self.apagado = False         # #REPITE apagado=1: salió de la EEPROM
        self.tope = None             # el tope= del #INICIO (ms)

    @property
    def mov_n(self):
        return int(self.codigo) if self.codigo and self.codigo.isdigit() else None

    @staticmethod
    def _fin_segun_guardado(m, ahora):
        """Cuándo terminó (reloj de la PC) lo que dice un #GUARDADO, o None
        si no se sabe (mismo=0: es de antes de apagar)."""
        if not (m.get("hay") and m.get("mismo")) or m.get("hace") is None:
            return None
        return ahora - m["hace"] / 1000.0

    def _ya_visto(self, m, ahora):
        """¿Ese #GUARDADO es un resultado que ya llegó EN VIVO a esta ventana?
        (mismo código y terminó a menos de 3 s de cuando llegó su #FIN)."""
        fin = self._fin_segun_guardado(m, ahora)
        if fin is None:
            return False
        if self.esperando_sin_cable and self.t_sin_cable is not None and fin >= self.t_sin_cable - 0.5:
            return False                 # terminó después de la cuenta SIN CABLE que se espera: es ESA
        return any(que == m["que"] and codigo == m["n"] and abs(t - fin) < 3.0 for que, codigo, t in self.vistos)

    def _es_de_esta_prueba(self, m, ahora):
        """¿Ese #GUARDADO terminó DESPUÉS de que empezó la cuenta SIN CABLE
        que se está esperando? Entonces es el resultado de esa prueba."""
        fin = self._fin_segun_guardado(m, ahora)
        if fin is None or self.t_sin_cable is None:
            return False
        return fin >= self.t_sin_cable - 0.5 and not self._ya_visto(m, ahora)

    def aplicar(self, m, ahora=0.0):
        tipo = m.get("tipo")
        cambios = set()
        if tipo in _DE_LA_TANDA:
            # Un renglón de tanda después de una tanda COMPLETA es de una
            # prueba NUEVA cuyo #INICIO no se vio (se reconectó el cable en
            # medio, o terminó sin cable): no se mezcla con lo viejo.
            if self.completo or (tipo == "M" and self.informe is not None):
                self._nuevo_resultado()
                cambios.add("resultado")
            self.tanda_abierta = True
        if tipo == "S":
            self.sensores = m
            self.t_sensores = ahora
            cambios.add("sensores")
            if m["e"] != self.modo:
                self.modo = m["e"]
                if self.modo == "ESPERA":
                    self.cuenta = None
                cambios.add("modo")
            if m["e"] == "ESPERA" and self.tanda_abierta:
                self.tanda_abierta = False       # se perdió el #FIN: el robot ya está en espera
        elif tipo == "HOLA":
            # El robot arrancó de nuevo, contestó a "h", o vio que se abrió el
            # puerto: está en espera. Después vienen sus dos #GUARDADO.
            self.hola = (m["nombre"], m["fecha"], m.get("serie"), m.get("partido"))
            self.panel_abierto = False
            self.repetir = []
            self.tanda_abierta = False
            if self.modo != "ESPERA":
                self.modo = "ESPERA"
                self.cuenta = None
                cambios.add("modo")
            cambios.add("hola")
        elif tipo == "V":
            self.valores.update(m["valores"])
            cambios.add("valores")
        elif tipo == "DEF":
            self.defectos.update(m["valores"])
            cambios.add("defectos")
        elif tipo == "OK":
            self.valores[m["nombre"]] = m["valor"]
            self.ultimo_ok = (m["nombre"], m["valor"])
            cambios |= {"valores", "ok"}
        elif tipo == "ERROR":
            self.ultimo_error = m["mensaje"]
            cambios.add("error")
        elif tipo == "GIRO":
            self.giro = m
            cambios.add("giro")
        elif tipo == "GUARDADO":
            self.guardado[m["que"]] = m
            pedido = "r" if m["que"] == "mov" else "rc"
            if m["hay"] and m["nuevo"]:
                if not self._ya_visto(m, ahora) and pedido not in self.repetir:
                    self.repetir.append(pedido)
            elif self.esperando_sin_cable and self.que_sin_cable == m["que"]:
                if self._es_de_esta_prueba(m, ahora):
                    # nuevo=0, pero terminó DESPUÉS de que empezó la cuenta: es
                    # el de esta prueba. (Un tablero.ino de antes del 03/11 lo
                    # marcaba visto al mandarlo AL VACÍO: el Teensy no se
                    # entera de que lo desenchufaron.) Se pide igual.
                    if pedido not in self.repetir:
                        self.repetir.append(pedido)
                else:
                    # Se pidió sin cable, volvió, y el robot NO tiene nada nuevo:
                    # lo apagaron antes de que terminara (o se reinició).
                    self.esperando_sin_cable = False
                    cambios.add("sin_cable_perdido")
            cambios.add("guardado")
        elif tipo == "CUENTA":
            nueva = self.modo != "CUENTA"
            if nueva:                          # empieza una prueba nueva
                self._nuevo_resultado()
                cambios.add("resultado")
            self.modo = "CUENTA"
            self.cuenta = m["segundos"]
            self.cuenta_modo = m["modo"]
            if m["sincable"]:
                if nueva or self.t_sin_cable is None:
                    self.t_sin_cable = ahora
                self.esperando_sin_cable = True
            cambios.add("modo")
        elif tipo == "INICIO":
            self._nuevo_resultado()
            self.modo = "MOVIENDO"
            self.cuenta = None
            self.codigo = m["codigo"]
            self.mov_nombre = m["nombre"]
            self.tope = m["tope"]
            self.sincable = m["sincable"]
            self.tanda_abierta = True
            if m["sincable"]:
                if not self.esperando_sin_cable or self.t_sin_cable is None:
                    self.t_sin_cable = ahora
                self.esperando_sin_cable = True
            cambios |= {"modo", "resultado"}
        elif tipo == "REPITE":
            # Lo guardado, repetido con r / rc. El modo NO cambia (sigue ESPERA).
            self._nuevo_resultado()
            self.codigo = m["codigo"]
            self.mov_nombre = m["nombre"]
            self.de_guardado = True
            self.sincable = m["sincable"]
            self.apagado = m["apagado"]
            self.tanda_abierta = True
            cambios.add("resultado")
        elif tipo == "PASO":
            self.pasos.append(m)
            cambios.add("pasos")
        elif tipo == "FRENO":
            # Frenado en la cuenta, o no arrancó (llega en ESPERA, sin #CUENTA
            # antes): no se llegó a mover y no viene informe. Lo de la prueba
            # anterior se borra, si no quedaría a la vista una nota vieja como
            # si fuera de esta. Moviéndose, después viene la tanda completa.
            if self.modo != "MOVIENDO":
                self._nuevo_resultado()
            self.freno = m["motivo"]
            cambios |= {"freno", "resultado"}
            if self.modo == "CUENTA":
                self.modo = "ESPERA"
                self.cuenta = None
                self.esperando_sin_cable = False   # no arrancó: no hay nada que esperar
                cambios.add("modo")
        elif tipo == "M":
            if len(self.muestras) < MAX_MUESTRAS:
                self.muestras.append((m["t"], m["grados"], m["fase"]))
            cambios.add("resultado")
        elif tipo == "MIRE":
            if len(self.intentos) < 20:
                self.intentos.append(m)
            cambios.add("resultado")
        elif tipo == "R":
            # El #R ya NO cambia el modo (lo hace el #FIN, que es el último).
            self.informe = m
            self.codigo = str(m["n"])
            self.mov_nombre = m["mov"]
            cambios.add("resultado")
        elif tipo == "USADOS":
            self.usados = dict(m["valores"])
            cambios.add("resultado")
        elif tipo == "CONSEJO":
            self.consejos.append(m["mensaje"])
            cambios.add("resultado")
        elif tipo in _CODIGO_DE_LA_MEDICION:
            codigo = _CODIGO_DE_LA_MEDICION[tipo]
            if self.codigo is None:
                self.codigo = codigo
            self.tanda_cancha.append(m)
            if tipo == "LIN":
                self.cancha.setdefault("L", {})[m["fase"]] = m
            else:
                self.cancha[codigo] = m
            cambios |= {"cancha", "resultado"}
        elif tipo == "FIN":
            self.modo = "ESPERA"
            self.cuenta = None
            self.completo = True
            self.tanda_abierta = False
            self.esperando_sin_cable = False
            if self.codigo is None:
                self.codigo = m["codigo"]
            if not self.de_guardado:           # llegó en vivo: ya se vio (ver _ya_visto)
                que = "cancha" if m["codigo"] in CODIGOS_CANCHA else "mov"
                self.vistos = (self.vistos + [(que, m["codigo"], ahora)])[-10:]
            if m["codigo"] in CODIGOS_CANCHA:
                self.cancha_lista = (m["codigo"], list(self.tanda_cancha))
                cambios.add("cancha_lista")
            cambios |= {"modo", "resultado", "fin"}
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
        elif tipo == "PARTIDO":
            # La manda el programa de PARTIDO al arrancar (no tablero.ino).
            self.partido_robot = m
            cambios.add("partido")
        elif tipo == "ROTA":
            self.rotas += 1
            cambios.add("rota")
        return cambios


# ---------- cuentas chicas que usa la ventana (puras: se prueban solas) ----------

def tipo_de(nombre_mov):
    m = MOV_POR_NOMBRE.get(nombre_mov or "")
    return m["tipo"] if m else "costado"


def defecto_de(nombre):
    for f in TABLA:
        if f[0] == nombre:
            return f[2]
    return None


def rango_de(nombre):
    for f in TABLA:
        if f[0] == nombre:
            return f[3], f[4]
    return None


def valor_num(valores, nombre):
    """El número que tiene el robot AHORA (del #V), o el del partido si
    todavía no llegó."""
    try:
        return float((valores or {}).get(nombre, defecto_de(nombre)))
    except (TypeError, ValueError):
        try:
            return float(defecto_de(nombre))
        except (TypeError, ValueError):
            return 0.0


def formatear_valor(v, tipo):
    """Como lo imprime el robot: enteros sin punto, float con 1 o 2
    decimales (2 solo si hacen falta: 30.0, 0.8, 0.75, 2.87). Sin '-0.0'."""
    if tipo == "f":
        t = "%.2f" % (round(float(v), 2) + 0.0)
        if t.endswith("0"):
            t = t[:-1]
        if t in ("-0.0", "-0.00"):
            t = "0.0"
        return t
    return str(int(round(float(v))))


def lado(grados):
    """-3.2 -> '3.2° a la DERECHA'. Signo del giroscopio (contrato):
    NEGATIVO = giró a la DERECHA, POSITIVO = a la IZQUIERDA."""
    if grados is None:
        return "? grados"
    if abs(grados) < 0.05:
        return "0° (derecho)"
    return "%.1f° a la %s" % (abs(grados), "IZQUIERDA" if grados > 0 else "DERECHA")


def lado_pelota(y):
    """4.2 -> '4.2 cm a la DERECHA'. ⚠ En la CÁMARA del arquero (el y= del
    #S, ya en cm) es al revés que en el giroscopio: + = DERECHA."""
    if y is None:
        return "?"
    if abs(y) < 0.05:
        return "justo enfrente"
    return "%.1f cm a la %s" % (abs(y), "DERECHA" if y > 0 else "IZQUIERDA")


def visible(dato):
    """¿Se vio alguna vez? dato = (x, y, edad) de un #S."""
    return (dato is not None and dato[0] is not None and dato[1] is not None
            and dato[2] is not None and dato[2] >= 0 and dato[0] > 0)


def cm_a_pantalla(x_cm, y_cm, cx, cy, escala):
    """Del robot (cm) a la pantalla (píxeles), visto desde arriba con el
    robot mirando para ARRIBA:
      X (para adelante) -> para arriba: la y de la pantalla BAJA.
      ⚠ Y positivo = a la DERECHA del robot (así lo da el arquero: y= del
        #S) -> a la derecha de la pantalla. Es AL REVÉS que en el tablero
        del delantero: no copiar el signo de allá."""
    return cx + y_cm * escala, cy - x_cm * escala


def _decimales(texto):
    t = str(texto)
    return len(t.split(".", 1)[1]) if "." in t else 0


def nuevo_valor(texto_actual, paso, signo):
    """Lo que manda [+] o [-]: nuevo_valor('0.8', 0.05, -1) -> '0.75'.
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
    En el arquero el robot manda el desvío entre -180 y 180, y un salto de
    más de 180 grados en 10 ms no existe: es la vuelta del número."""
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


def puede_mover(conectado, es_arquero, programa_ok, modo, pendiente=False, serie="",
                serie_robot=None, sin_cable=False, buscando_giro=False, cartel=None):
    """¿Se puede pedir un movimiento (o cambiar un número)? -> (sí/no, por qué no).
    Los motivos van en el orden de DISENO.md 8.7."""
    if not conectado:
        return False, "No hay robot conectado."
    if not es_arquero:
        return False, "Este robot NO es el arquero (serie %s): el tablero no le manda nada." % (serie or "?")
    if not programa_ok:
        return False, cartel or "El robot no tiene cargado el programa del tablero: usá TABLERO.bat."
    if serie_robot is not None and serie_robot != SERIE_ARQUERO:
        return False, "El robot dice que NO es el arquero (serie %s): no se mueve." % serie_robot
    if sin_cable:
        return False, "SIN CABLE: el robot está haciendo la prueba solo. Enchufalo cuando termine."
    if buscando_giro:
        return False, "El robot está buscando el giroscopio..."
    if modo != "ESPERA":
        return False, "Esperá a que termine."
    if pendiente:
        return False, "Esperando que el robot conteste..."
    return True, ""


# ---------- la pelota: qué haría el partido (con el robot quieto) ----------

def angulo_salida(avance, lateral, rendimiento):
    """Para dónde sale de verdad la diagonal, en grados (+ = DERECHA): por
    cada parte de avance, lateral x rendimientoCostado de costado."""
    if not avance:
        return 0.0
    return math.degrees(math.atan(lateral * rendimiento / float(avance)))


def calcular_diagonal(d, y, vy, va, hay_mirada, quieto, valores):
    """COPIA en Python de calcularDiagonal() del partido 0.1103, SOLO para
    el robot de mentira (el de verdad la hace él y la manda en el dg=).
    d, y = la pelota en cm (y + = DERECHA); vy, va = cm/s (la de la mirada
    si miró); quieto = no se movió de costado en MS_QUIETO_PARA_PREDECIR.
    -> (sale 1/0, motivo 0/1/2, avance, lateral, msIda, ángulo que necesitaba)."""
    V = lambda n: valor_num(valores, n)
    potencia = int(V("potenciaDespeje"))
    ms_adelante = int(V("msAdelante"))
    avance, lateral, ms_ida = potencia, 0, ms_adelante
    if V("despejeEnDiagonal") < 0.5:
        return 1, 0, avance, 0, ms_ida, 0.0          # modo viejo: sale derecho
    d = max(float(d), 1.0)                           # no dividir por cero
    piso = FIJOS["PISO_VELOCIDAD"]
    vy = float(vy or 0.0)
    if abs(vy) < piso:
        vy = 0.0                                     # eso es ruido, no movimiento
    va = float(va or 0.0) if hay_mirada else 0.0     # acercamiento: solo si lo midió quieto
    if va < piso:
        va = 0.0                                     # (alejándose no suma)
    predecir = V("predecirEnDiagonal") >= 0.5
    if not predecir or (not hay_mirada and not quieto):
        vy = 0.0
    if not predecir:
        va = 0.0
    k = float(y) / d                                 # motivo 1: la pelota está corrida
    m = vy * FIJOS["PWM_POR_CM_S"]                   # motivo 2: se está corriendo sola
    m += k * va * FIJOS["PWM_POR_CM_S"]              # motivo 3: se viene encima
    r = V("rendimientoCostado")
    denom = 1.0 + 0.5 * abs(k) / r
    techo = (FIJOS["PRESUPUESTO_RUEDA"] - 0.5 * abs(m) / r) / denom
    if techo < avance:
        avance = int(techo)
    if avance < FIJOS["PWM_MIN_AVANCE_DESPEJE"]:
        return 0, 1, avance, 0, ms_adelante, 0.0     # no llego: no salgo
    costado = (k * avance + m) / r
    angulo = math.degrees(math.atan(costado * r / avance))
    if abs(costado * r / avance) > math.tan(math.radians(V("MAX_ANGULO_DIAGONAL"))):
        return 0, 2, avance, 0, ms_adelante, angulo  # muy abierto: no salgo
    if abs(costado) < FIJOS["COSTADO_MINIMO_UTIL"]:
        costado = 0.0                                # sale derecho
    lateral = int(costado)
    ms_ida = int(float(ms_adelante) * potencia / avance)
    if ms_ida > FIJOS["MS_MAX_ADELANTE"]:
        ms_ida = FIJOS["MS_MAX_ADELANTE"]
    return 1, 0, avance, lateral, ms_ida, angulo


def que_haria(s, valores):
    """Con el robot QUIETO y la pelota donde la ve el #S, qué haría el
    programa de partido. -> (código, frase)."""
    if not s or not s.get("ve") or s.get("d") is None:
        return "NO_LA_VE", "No ve la pelota."
    V = lambda n: valor_num(valores, n)
    d = s["d"]
    y = s.get("y") or 0.0
    umbral, desvio = V("umbralCm"), V("umbralDesvio")
    dist_disparo = V("distanciaParaMirar") if V("medirAntesDeDespejar") >= 0.5 else umbral
    n = s.get("n") or 0
    # El partido compara X e Y POR SEPARADO (no es un círculo).
    if abs(y) > desvio:
        return "MUY_AL_COSTADO", "Muy al costado (%.0f cm): la sigue de costado." % abs(y)
    if d > dist_disparo:
        return "LEJOS", "Está a %.0f cm: la sigue de costado." % d
    if n < FIJOS["VECES_PARA_CREERLE"]:
        return "JUNTANDO", "En la zona, juntando cuadros (%d de 3)." % n
    if d > umbral:
        return "MIRARIA", "Se frenaría a MIRAR (está a %.0f cm, sale a %.0f)." % (d, umbral)
    dg = s.get("dg")
    if dg and dg[0] == 1:
        ang = angulo_salida(dg[2], dg[3], V("rendimientoCostado"))
        if dg[3] == 0 or abs(ang) < 0.5:
            return "SALDRIA", "SALDRÍA YA, derecho."
        return "SALDRIA", "SALDRÍA YA, en diagonal %d° a la %s." % (
            int(round(abs(ang))), "DERECHA" if ang > 0 else "IZQUIERDA")
    motivo = dg[1] if dg else 1
    if motivo == 2:
        return "NO_SALDRIA", "NO saldría: muy abierto (más de %.0f°)." % V("MAX_ANGULO_DIAGONAL")
    return "NO_SALDRIA", "NO saldría: el avance no alcanza."


def estado_linea(l, u):
    """'-' (nada), 'IZQ' (pisa el lateral IZQUIERDO: el de atrás izquierdo
    en blanco y el otro no), 'DER', o 'AREA' (los dos: la línea del área)."""
    if not l or u is None or l[0] is None or l[1] is None:
        return "-"
    izq, der = l[0] >= u, l[1] >= u
    if izq and der:
        return "AREA"
    if izq:
        return "IZQ"
    if der:
        return "DER"
    return "-"


# ---------- las cuentas de la CANCHA ----------

def calcular_umbral(mediciones):
    """El umbralBlanco con las #LIN medidas (la lógica de calibrar-linea):
      peorVerde  = el MÁXIMO de izq y der juntando TODAS las fases v
      peorBlanco = min(izq.max de la fase i, der.max de la fase d): el
                   MÁXIMO del blanco, porque el sensor se sale un rato de la
                   línea; y separados, porque la línea de la escuela es más
                   angosta que la separación de los sensores (15/09)
      umbral     = (peorVerde + peorBlanco) // 2
    El de adelante (A12) se muestra, pero no entra en la cuenta."""
    verdes = [m for m in mediciones if m.get("fase") == "v"]
    blancos_i = [m for m in mediciones if m.get("fase") == "i"]
    blancos_d = [m for m in mediciones if m.get("fase") == "d"]
    r = {"umbral": None, "peor_verde": None, "peor_blanco": None, "margen_verde": None,
         "margen_blanco": None, "avisos": [], "falta": []}
    if not verdes:
        r["falta"].append("VERDE")
    if not blancos_i:
        r["falta"].append("BLANCO IZQ")
    if not blancos_d:
        r["falta"].append("BLANCO DER")
    if verdes:
        r["peor_verde"] = max(max(m["izq"][1], m["der"][1]) for m in verdes)
    if blancos_i and blancos_d:
        r["peor_blanco"] = min(blancos_i[-1]["izq"][1], blancos_d[-1]["der"][1])
    if r["falta"]:
        return r
    pv, pb = r["peor_verde"], r["peor_blanco"]
    if pb <= pv:
        r["avisos"].append(("SE SUPERPONEN, no hay umbral que sirva: revisar la altura de los "
                            "sensores, la suciedad, el sol, y que la línea sea blanca.", "rojo"))
        return r
    u = int((pv + pb) // 2)
    r["umbral"] = u
    r["margen_verde"] = u - pv
    r["margen_blanco"] = pb - u
    if min(r["margen_verde"], r["margen_blanco"]) < 100:
        r["avisos"].append(("Frágil: queda a menos de 100 de un lado (%d del verde, %d del blanco). "
                            "Repetirlo, y si da igual, anotarlo." % (r["margen_verde"], r["margen_blanco"]),
                            "amarillo"))
    for m, clave, nombre in ((blancos_i[-1], "izq", "IZQUIERDO"), (blancos_d[-1], "der", "DERECHO")):
        if m[clave][2] < (m[clave][1] + pv) / 2.0:
            r["avisos"].append(("Muestra sucia (blanco %s): el sensor estuvo afuera de la línea buena "
                                "parte del rato: el máximo igual sirve, pero repetilo bien centrado." % nombre,
                                "amarillo"))
    return r


def ms_hasta_el_medio(cruce, vel=100, rampa=10):
    """msHastaElMedio() de medir-ancho: cruce/2 + perdida/2, con
    perdida = tiempo de la rampa / 2 + 33 (el retardo del motor).
    A 100 con la rampa 10/10: cruce/2 + 41. (4344 -> 2213)"""
    ms_rampa = (int(vel) // int(rampa)) * FIJOS["RAMPA_MOV_MS"]
    perdida = ms_rampa // 2 + FIJOS["MS_RETARDO_ARRANQUE"]
    return int(cruce) // 2 + perdida // 2


def numeros_creibles(medioizq, izqder, derizq=None):
    """numerosCreibles() de medir-ancho: el cruce entre 1,4 y 3 veces la
    ida (tramo 1), y la vuelta entre 0,7 y 1,43 veces el cruce (tramo 2).
    -> (bien el 1, bien el 2 o None si no hay tramo 2)."""
    c, i = int(izqder), int(medioizq)
    uno = c * 10 >= i * 14 and c * 10 <= i * 30
    dos = None
    if derizq is not None:
        v = int(derizq)
        dos = v * 10 >= c * 7 and v * 7 <= c * 10
    return uno, dos


def recta(xs, ys):
    """La recta que mejor pasa por los puntos (cuadrados mínimos): y = a·x + b.
    -> (a, b), o None si no se puede (menos de 2 puntos, o todos en la misma x)."""
    n = len(xs)
    if n < 2 or n != len(ys):
        return None
    mx = sum(xs) / float(n)
    my = sum(ys) / float(n)
    sxx = sum((x - mx) ** 2 for x in xs)
    if sxx < 1e-12:
        return None
    a = sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx
    return a, my - a * mx


def ms_para_cm(a, b, cm):
    """Con la recta cm = a·ms + b: cuántos ms para llegar a 'cm'."""
    if not a:
        return None
    return int(round((cm - b) / a))


def sugerido_ancho(corridas, rampa=None):
    """MS_MEDIO_AL_MEDIO sugerido = el PROMEDIO de almedioizq de las
    corridas buenas (así se eligió 2228 el 06/10: 2213 y 2243).
    rampa: el RAMPA_MOV_PASO del PARTIDO. Si se da, solo cuentan las
    corridas medidas con esa rampa (el rampa= del #ANCHO): con otra, el
    arranque rinde distinto y el número no vale para el partido
    (2026-11-03, revisión).
    -> (sugerido, diferencia en %, aviso o '')."""
    todas = [c for c in corridas if c.get("ok") and c.get("almedioizq")]
    otras = [c for c in todas if rampa is not None and c.get("rampa") is not None and c["rampa"] != int(rampa)]
    buenas = [c["almedioizq"] for c in todas if c not in otras]
    if otras and not buenas:
        return None, None, ("Se midió con RAMPA_MOV_PASO %s y el partido usa %d: no sirve para el partido. "
                            "Apretá '%s' (en MOVIMIENTOS) y medí de nuevo." % (
                                otras[-1]["rampa"], int(rampa), BOTON_NUMEROS_PARTIDO))
    if not buenas:
        return None, None, ""
    promedio = sum(buenas) / float(len(buenas))
    dif = (max(buenas) - min(buenas)) / promedio * 100.0 if len(buenas) > 1 else 0.0
    aviso = ""
    if len(buenas) < 2:
        aviso = "Hacé una segunda corrida: el sugerido es el promedio de 2."
    elif dif > 3.0:
        aviso = "Las corridas difieren %.1f %% (más de 3 %%): hacé una tercera." % dif
    if otras:
        aviso = ("%d corrida%s con otra RAMPA_MOV_PASO (no la %d del partido) no entra%s. " % (
            len(otras), "" if len(otras) == 1 else "s", int(rampa), "" if len(otras) == 1 else "n") + aviso).strip()
    return int(round(promedio)), dif, aviso


def numeros_fuera_del_partido(valores, partido, nombres):
    """Los números que el robot tiene AHORA en la RAM (#V) distintos de los
    del PARTIDO: [(nombre, el de ahora, el del partido)]. partido: nombre ->
    texto del panel ('10', 'true'...). Lo que no se puede comparar, se saltea."""
    salida = []
    for nombre in nombres:
        if nombre not in (valores or {}) or (partido or {}).get(nombre) is None:
            continue
        try:
            a = float(valores[nombre])
            b = _numero_del_panel(partido[nombre])
        except (TypeError, ValueError):
            continue
        if abs(a - b) > 0.001:
            salida.append((nombre, valores[nombre], partido[nombre]))
    return salida


def sugerido_fondo(corridas):
    """MS_MEDIO_FONDO_POR_TIEMPO sugerido = el máximo de las corridas que
    terminaron con TOCÓ + 500 ms, redondeado para arriba a 50 (la regla
    del panel). 1730 y 1690 -> 2250."""
    tocos = [c["ms"] for c in corridas if c.get("como") == "TOCO"]
    if not tocos:
        return None
    return int(math.ceil((max(tocos) + 500) / 50.0)) * 50


def consejo_pelota(respuesta, valores):
    """La pregunta de la 9: '¿qué pasó con la pelota?'. Contesta qué tocar
    (sin cambiar nada solo). ⚠ rendimientoCostado va AL REVÉS: si le faltó
    costado, se BAJA."""
    V = lambda n: valor_num(valores, n)
    r = V("rendimientoCostado")
    minimo, maximo = rango_de("rendimientoCostado")
    if respuesta == "DESPEJO":
        return "¡Bien! Anotalo. Repetí 3 veces antes de cambiar algo."
    if respuesta == "FALTO_COSTADO":
        if r <= minimo + 1e-6:
            return ("rendimientoCostado ya está en el mínimo (%.2f): no se puede bajar más. "
                    "Anotalo en la bitácora y avisá." % minimo)
        return ("BAJÁ rendimientoCostado de a 0.05 (ahora %.2f → %.2f). Va al revés: más chico = "
                "MÁS costado." % (r, r - 0.05))
    if respuesta == "SE_PASO":
        if r >= maximo - 1e-6:
            return ("rendimientoCostado ya está en el máximo (%.2f): no se puede subir más. "
                    "Anotalo en la bitácora y avisá." % maximo)
        return "SUBÍ rendimientoCostado de a 0.05 (ahora %.2f → %.2f)." % (r, r + 0.05)
    if respuesta == "TARDE":
        return ("Subí umbralCm de a 2 (ahora %g): sale antes. OJO: atado a msAdelante, la ida tiene "
                "que llegar más lejos que umbralCm." % V("umbralCm"))
    if respuesta == "CORTO":
        return "Subí msAdelante de a 20 (ahora %d) y medilo con regla (CANCHA, paso 5)." % int(V("msAdelante"))
    return ""


# ---------- el PANEL DE CONTROL del partido (solo se LEE) ----------

_RE_PANEL = re.compile(r"^((?:const )?(?:unsigned long|int|float|bool)) ([A-Za-z_][A-Za-z0-9_]*)\s*= "
                       r"([^;]*);\s*// (.*)$")
_RE_VERSION = re.compile(r'const char\*\s*VERSION\s*=\s*"([^"]*)"')


def leer_panel(texto):
    """Las líneas del PANEL DE CONTROL del partido, entre 'PANEL DE CONTROL'
    y 'fin del panel de control':
    -> {nombre: (declaración, valor como texto, comentario, número de línea)}."""
    salida = {}
    dentro = False
    for i, renglon in enumerate((texto or "").splitlines(), 1):
        r = renglon.rstrip("\r")
        if not dentro:
            if "PANEL DE CONTROL" in r:
                dentro = True
            continue
        if "fin del panel de control" in r:
            break
        m = _RE_PANEL.match(r)
        if m:
            salida[m.group(2)] = (m.group(1), m.group(3).strip(), m.group(4), i)
    return salida


def version_del_partido(texto):
    """'0.1103' de const char* VERSION = "0.1103 EN PRUEBA (...)"."""
    m = _RE_VERSION.search(texto or "")
    return m.group(1).split()[0] if m and m.group(1).split() else None


def linea_del_panel(decl, nombre, valor, comentario, etiqueta=None):
    """Una línea del panel, con el mismo formato que las de verdad: la
    declaración y el nombre en 49 columnas, '= ', el valor con ';' en 7 y
    '// ' + el comentario. Con etiqueta ('MEDIDO 11/11 Roboliga') cambia el
    primer [...] del comentario, o lo agrega adelante si no hay."""
    com = comentario or ""
    if etiqueta:
        m = re.search(r"\[[^\]]*\]", com)
        if m:
            com = com[:m.start()] + "[" + etiqueta + "]" + com[m.end():]
        else:
            com = "[" + etiqueta + "] " + com
    return (decl + " " + nombre).ljust(49) + "= " + (valor + ";").ljust(7) + "// " + com


def valor_para_panel(nombre, valor):
    """El valor como se escribe en el .ino: los bool como true/false."""
    tipo = TIPO_NUMERO.get(nombre, "i")
    v = float(valor)
    if tipo == "b":
        return "true" if v >= 0.5 else "false"
    if tipo == "f":
        return "%.2f" % v if abs(round(v, 1) - v) > 1e-9 else "%.1f" % v
    return str(int(round(v)))


def _numero_del_panel(texto):
    t = str(texto).strip()
    if t == "true":
        return 1.0
    if t == "false":
        return 0.0
    return float(t)


def copias_viejas(defectos, panel):
    """Los números en que las COPIAS del robot (#DEF) no coinciden con el
    archivo del partido: [(nombre, el del tablero, el del partido)]. Si hay,
    el partido cambió y hay que actualizar tablero.ino."""
    salida = []
    for nombre in ORDEN_NUMEROS:
        if not DECLARACION[nombre] or nombre not in panel or nombre not in (defectos or {}):
            continue
        try:
            a = _numero_del_panel(panel[nombre][1])
            b = float(defectos[nombre])
        except ValueError:
            continue
        if abs(a - b) > 0.001:
            salida.append((nombre, defectos[nombre], panel[nombre][1]))
    return salida


# ---------- la HOJA DE CANCHA (un .txt que se lee con el Bloc de notas) ----------

SECCIONES_HOJA = ("DATOS", "LINEA", "ANCHO", "FONDO", "CAMARA", "DESPEJE", "SIGNOS", "REVISADO",
                  "COPIADO AL PANEL", "NOTAS")
CABECERA_HOJA = "# HOJA DE CANCHA del ARQUERO - la escribe el tablero. Se puede leer con el Bloc de notas."
# Qué número sale de qué renglón de la hoja (para COMPARAR y "Volver a esta cancha")
CLAVES_HOJA = (("umbralBlanco", "LINEA", "umbral"), ("MS_MEDIO_AL_MEDIO", "ANCHO", "sugerido"),
               ("MS_MEDIO_FONDO_POR_TIEMPO", "FONDO", "sugerido"), ("CAMARA_POR_CM", "CAMARA", "sugerido"),
               ("msAdelante", "DESPEJE", "sugerido"), ("lateralInvertido", "SIGNOS", "lateralInvertido"),
               ("camaraYInvertida", "SIGNOS", "camaraYInvertida"))


def _limpio_hoja(texto, es_clave=False):
    t = str(texto).replace("\r", " ").replace("\n", " ").replace("   # ", "  - ")
    if es_clave:
        t = t.replace(" = ", "=").strip() or "?"
    return t


def hoja_nueva(cancha="", fecha="", partido=PARTIDO_COPIADO):
    hoja = {s: [] for s in SECCIONES_HOJA}
    for clave, valor in (("cancha", cancha), ("fecha", fecha), ("midieron", ""),
                         ("partido_cargado", partido), ("bateria", ""), ("piso", ""),
                         ("luz", ""), ("rampa", ""), ("ancho_linea_mm", ""), ("ancho_cancha_cm", ""),
                         ("mira_al_arco", "")):
        hoja["DATOS"].append((clave, _limpio_hoja(valor), ""))
    return hoja


def hoja_poner(hoja, seccion, clave, valor, nota=""):
    """Pone (o reemplaza) 'clave = valor   # nota' en esa sección."""
    clave = _limpio_hoja(clave, True)
    entradas = hoja.setdefault(seccion, [])
    nuevo = (clave, _limpio_hoja(valor), _limpio_hoja(nota))
    for i, (k, _, _) in enumerate(entradas):
        if k == clave:
            entradas[i] = nuevo
            return clave
    entradas.append(nuevo)
    return clave


def hoja_agregar(hoja, seccion, base, valor, nota=""):
    """Agrega 'base_1', 'base_2'... (para las corridas que se repiten)."""
    entradas = hoja.setdefault(seccion, [])
    usados = {k for k, _, _ in entradas}
    i = 1
    while "%s_%d" % (base, i) in usados:
        i += 1
    return hoja_poner(hoja, seccion, "%s_%d" % (base, i), valor, nota)


def hoja_valor(hoja, seccion, clave, defecto=None):
    for k, v, _ in (hoja or {}).get(seccion, []):
        if k == clave:
            return v
    return defecto


def hoja_a_texto(hoja):
    renglones = [CABECERA_HOJA]
    for seccion, entradas in hoja.items():
        renglones.append("[%s]" % seccion)
        for clave, valor, nota in entradas:
            r = "%s = %s" % (clave, valor)
            if nota:
                r += "   # " + nota
            renglones.append(r)
    return "\n".join(renglones) + "\n"


def texto_a_hoja(texto):
    hoja = {}
    actual = None
    for renglon in (texto or "").splitlines():
        r = renglon.rstrip("\r")
        if not r.strip() or r.lstrip().startswith("#"):
            continue
        if r.startswith("[") and r.rstrip().endswith("]"):
            actual = r.strip()[1:-1]
            hoja.setdefault(actual, [])
            continue
        if actual is None:
            continue
        clave, igual, resto = r.partition(" = ")
        if not igual:
            continue
        valor, _, nota = resto.partition("   # ")
        hoja[actual].append((clave, valor, nota))
    return hoja


def hoja_valores_panel(hoja):
    """Los números que decidió esa cancha: lo copiado al panel (lo de la
    derecha de 'a -> b'), o si no, lo sugerido de cada sección."""
    salida = {}
    for nombre, seccion, clave in CLAVES_HOJA:
        v = hoja_valor(hoja, seccion, clave)
        copiado = hoja_valor(hoja, "COPIADO AL PANEL", nombre)
        if copiado is not None and "->" in copiado:
            v = copiado.split("->", 1)[1].strip()
        if v not in (None, ""):
            v = v.split()[0]
            try:
                float(v)
                salida[nombre] = v
            except ValueError:
                pass
    return salida


def comparar_hojas(hojas):
    """hojas = [(nombre, hoja)] -> [(número, [valor de cada hoja o '-'])]."""
    valores = [hoja_valores_panel(h) for _, h in hojas]
    return [(nombre, [v.get(nombre, "-") for v in valores]) for nombre, _, _ in CLAVES_HOJA]


# La hoja de la cancha de la ESCUELA, con lo que ya estaba medido en las
# bitácoras (NO medido con el tablero). La ventana la escribe en canchas/
# la primera vez, para poder COMPARAR.
HOJA_ESCUELA = """# HOJA DE CANCHA del ARQUERO - la escribe el tablero. Se puede leer con el Bloc de notas.
# origen = de las bitacoras, no medido con el tablero (la arma el tablero, 2026-11-03)
[DATOS]
cancha = escuela IITA (la de las bitacoras)
fecha = 2026-11-03
midieron = el equipo, en varias clases
partido_cargado = 0.1103
origen = de las bitacoras, no medido con el tablero
[LINEA]
datos = A13 95/763, A11 81/753, A12 56/759 (verde maximo / blanco maximo)   # 15/09
umbral = 425   # 15/09: peorVerde 97, peorBlanco 753, margen 328/328
[ANCHO]
corrida_1 = medioizq=? izqder=4344 derizq=4547 almedioizq=2213 almedioder=2314   # 06/10
corrida_2 = medioizq=? izqder=4405 derizq=4615 almedioizq=2243 almedioder=2348   # 06/10
sugerido = 2228   # promedio de 2213 y 2243 (repetibilidad 1,5 %, asimetria 4 %)
[FONDO]
sugerido = 2000   # [a ojo, no medido]
[CAMARA]
tabla = 10->28 20->58 30->88.9 40->117 50->142   # 25/08
sugerido = 2.87   # recta: a = 2.87, b = 0.68
[DESPEJE]
idas = 150ms->13cm 250ms->19cm 350ms->33cm   # 04/08, a potencia 200
sugerido = 383   # para 35 cm (recta: a = 0.1, b = -3.33)
[SIGNOS]
lateralInvertido = 0   # 11/08: el movimiento 2 fue a la DERECHA
camaraYInvertida = 1   # 11/08: pelota a la derecha, Yp = -24
[REVISADO]
[COPIADO AL PANEL]
[NOTAS]
"""
NOMBRE_HOJA_ESCUELA = "escuela-iita-bitacoras.txt"


def avisos_de_numeros(valores, sensores=None, codigo=None):
    """Los avisos que la ventana pone sola, en naranja, SIN bloquear nada
    (el robot no tiene controles cruzados entre números)."""
    V = lambda n: valor_num(valores, n)
    avisos = []
    if V("medirAntesDeDespejar") >= 0.5 and V("distanciaParaMirar") <= V("umbralCm"):
        avisos.append("distanciaParaMirar (%g) no es mayor que umbralCm (%g): mira recién cuando ya "
                      "podría salir." % (V("distanciaParaMirar"), V("umbralCm")))
    if V("MS_MAX_MIRANDO") < V("MS_ASENTAR_MIRADA") + V("MS_MIRAR"):
        avisos.append("MS_MAX_MIRANDO (%d) es menor que MS_ASENTAR_MIRADA + MS_MIRAR (%d): nunca llega "
                      "a medir." % (V("MS_MAX_MIRANDO"), V("MS_ASENTAR_MIRADA") + V("MS_MIRAR")))
    if V("msAdelante") / 10.0 - 3.3 <= V("umbralCm"):
        avisos.append("Con msAdelante %d la ida llega a ~%.0f cm: no pasa de umbralCm (%g). La ida "
                      "no llega a la pelota." % (V("msAdelante"), V("msAdelante") / 10.0 - 3.3, V("umbralCm")))
    if V("pwmMaxLateral") > 150:
        avisos.append("pwmMaxLateral más de 150 nunca se probó (a 160 chocó la madera).")
    if V("umbralDesvio") > 7 and V("despejeEnDiagonal") < 0.5:
        avisos.append("umbralDesvio %g con despejeEnDiagonal = NO: saliendo derecho, más de 7 no la "
                      "alcanza." % V("umbralDesvio"))
    if sensores and codigo is not None and (codigo in MIRAN_LA_LINEA or codigo in ("A", "F", "I")):
        if estado_linea(sensores.get("l"), sensores.get("u")) != "-":
            avisos.append("Un sensor de atrás está sobre una línea blanca: no va a arrancar "
                          "(corranlo ~10 cm para adelante).")
    return avisos


# ---------- las notas, con los criterios del contrato (DISENO 4.4) ----------

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
    """La nota de un movimiento que valió (pasos 3 a 6 de DISENO 4.4)."""
    if cruces >= 4:
        return "ZIGZAGUEA"
    if abs(pico) > 10 or abs(fin) > 10:
        return "SE_TUERCE"
    if abs(pico) <= 4 and cruces <= 2 and abs(fin) <= 3:
        return "PROLIJO"
    return "CASI"


def calcular_nota(como, racha, mudas, muestras, en_movimiento, n, sale, pico, cruces, fin):
    """La calcularNota() de ajustar-movimientos, con nombres (DISENO 4.4)."""
    if (como in ("TECLA", "TOPE") or racha >= 3 or mudas * 50 > muestras + mudas
            or en_movimiento < 5):
        return "NO_VALE"
    if n == 9 and not sale:
        return "NO_SALIO"
    return nota_derecho(pico, cruces, fin)


def _fase_en(muestras, t):
    """La fase de la muestra más cercana (antes) a t ms."""
    fase = None
    for tm, _, f in muestras or ():
        if tm > t:
            break
        fase = f
    return fase


def explicar_informe(inf, intentos=None, valores=None, muestras=None):
    """Pasa el #R a frases en criollo: [(frase, estilo)], estilo = '',
    'malo' o 'bueno'."""
    n = inf.get("n") or 0

    def num(clave):
        v = inf.get(clave)
        return 0 if v is None else v

    frases = []
    como = inf.get("como")
    if como in COMOS_R and como != "NORMAL":
        frases.append(("Cómo terminó: %s." % COMOS_R[como],
                       "malo" if como in ("TECLA", "TOPE", "SIN_LINEA", "NO_SALIO") else ""))
    # Frenado con una tecla o por el tope (2026-11-03, revisión): no llegó a
    # volver ni a quedarse quieto, así que no se dice "no vio la línea" ni
    # "se enderezó" ni "al final, quieto" (el robot tampoco da ese consejo).
    cortado = como in ("TECLA", "TOPE")
    fase = _fase_en(muestras, num("picoms"))
    donde = (", en la fase '%s'" % FASES[fase][0]) if fase in FASES else ""
    desde = " (contado desde que arrancó la ida)" if n == 9 and num("sale") else ""
    frases.append(("Lo más torcido%s: %s a los %d ms%s." % (desde, lado(num("pico")), num("picoms"), donde), ""))
    fin = num("fin")
    if cortado:
        frases.append(("Cuando lo cortaron (no llegó a quedarse quieto): %s." % lado(fin), ""))
    else:
        frases.append(("Al final, quieto: %s." % lado(fin), "bueno" if abs(fin) <= 3 else "malo"))
    cruces = int(num("cruces"))
    frases.append(("Cruzó de un lado al otro %d %s (4 o más = zigzag)." % (
        cruces, "vez" if cruces == 1 else "veces"), "malo" if cruces >= 4 else ""))
    medio = num("medio")
    frases.append(("En promedio, moviéndose: %s (más de 2 = torcido siempre para el mismo lado)." % lado(medio),
                   "malo" if abs(medio) > 2 else ""))
    frases.append(("Más de 3° el %d %% del tiempo. La corrección de rumbo promedio: %d (estuvo al tope el %d %%)."
                   % (num("fuera3"), num("corr"), num("tope")), "malo" if num("tope") > 20 else ""))
    if num("mudas") > 0:
        frases.append(("El giroscopio no contestó %d veces (la peor racha: %d seguidas)." % (
            num("mudas"), num("racha")), "malo" if num("racha") >= 3 else ""))
    hueco = num("hueco")
    if hueco > 40:
        frases.append(("Una vuelta del programa tardó %d ms (a los %d ms): eso es un RETARDO." % (
            hueco, num("huecoms")), "malo"))
    else:
        frases.append(("El programa no se clavó (la vuelta más larga: %d ms)." % hueco, "bueno"))
    frases.append(("Duró %d ms." % num("duro"), ""))
    volvio = n in (6, 7) or (n == 9 and num("sale"))
    if volvio:
        acomodo = int(num("acomodo"))
        if num("lineams") > 0:
            frases.append(("Vio la línea a los %d ms de empezar a volver." % num("lineams"), "bueno"))
        elif not cortado:
            frases.append(("NO vio la línea al volver: frenó por tiempo (%d ms)." % FIJOS["MS_MAX_RETROCESO"],
                           "malo"))
        if cortado and acomodo == 0:
            frases.append(("Lo cortaron antes de terminar: no se sabe si se enderezaba." if num("lineams") > 0 else
                           "Lo cortaron antes de terminar la vuelta: no se sabe si veía la línea ni si se "
                           "enderezaba.", ""))
        else:
            frases.append(("Se enderezó al primer intento." if acomodo == 0 else
                           "Para enderezarse usó %d reintento%s." % (acomodo, "" if acomodo == 1 else "s"),
                           "malo" if acomodo >= 3 else ""))
    if n in (8, 9):
        frases.append(("Siguiéndola: cambió de lado %d veces, arrancó de quieto %d veces, pisó el lateral "
                       "%d veces." % (num("idas"), num("arranques"), num("laterales")), ""))
        frases.append(("Vio la pelota el %d %% del tiempo; quieto por la zona muerta el %d %%." % (
            num("conpelota"), num("zonamuerta")), "malo" if num("conpelota") < 50 else ""))
        va_y_viene = num("idas") >= 3 and num("velmedia") < 4
        frases.append(("La pelota se movía, en promedio, a %.1f cm/s%s." % (
            num("velmedia"), ": VA Y VIENE con la pelota casi quieta" if va_y_viene else ""),
            "malo" if va_y_viene else ""))
    if n == 9:
        cuantos = int(num("intentos"))
        frases.append(("Intentos de despeje: %d. %s" % (
            cuantos, "Salió a despejar." if num("sale") else "NO salió a despejar."),
            "bueno" if num("sale") else "malo"))
        for i in intentos or ():
            frases.append(("   #%s a los %s ms: %s  (pelota a %s cm, %s)" % (
                i.get("k"), i.get("t"), PALABRAS_MIRE.get(i.get("res"), i.get("res")),
                "?" if i.get("d") is None else "%.0f" % i["d"], lado_pelota(i.get("y"))), ""))
    return frases



# ---------- el programa de PARTIDO (la 0921): leerlo y escribirlo (2026-11-08) ----------
#  La pestaña PARTIDO cambia SOLO los números de las declaraciones de arriba
#  de todo (las que empiezan en la columna 0, afuera de las funciones), con
#  una copia de seguridad ANTES. Todo esto es puro (salvo guardar_partido y
#  escribir_bytes_atomico, que usan archivos, pero nunca tkinter) y se prueba
#  en la --autoprueba con COPIAS en una carpeta temporal.
#  🚨 Dos trampas de Python que acá NO se usan:
#    - escribir_atomico() (el de las hojas) pasa los fines de línea a \r\n:
#      cambiaría el archivo ENTERO. Acá se escriben BYTES.
#    - texto.splitlines() corta también en \x0b, \x1c, \x85 y  : acá se
#      usa texto.split("\n") y "\n".join(...), que es exacto byte a byte.

class CambioPorAfuera(Exception):
    """El programa cambió desde que la ventana lo leyó (¿Arduino IDE?): no se escribe."""


class NoSeEscribe(Exception):
    """Algo no da y el archivo NO se tocó. El texto dice por qué (para la ventana)."""


# Las extensiones que compila PlatformIO (src_dir = . compila TODO lo de la
# carpeta, y pruebas/tablero también las subcarpetas): una copia con una de
# estas rompería la compilación (setup() repetido). Las copias van en .txt.
EXTENSIONES_QUE_COMPILAN = (".ino", ".cpp", ".c", ".cc", ".h", ".hpp", ".s", ".pde")
ESPERA_REINTENTO = 0.3           # entre intento e intento de os.replace (la autoprueba lo pone en 0)
RE_NOMBRE_COPIA = re.compile(r"^partido-0921_([0-9]{4}-[0-9]{2}-[0-9]{2}_[0-9]{2}-[0-9]{2}-[0-9]{2})(?:_([0-9]+))?\.txt$")


def huella_de(datos):
    """El sha256 de los bytes: si cambia un solo byte, cambia la huella."""
    return hashlib.sha256(datos).hexdigest()


def mapa_de_lineas(texto):
    """Para cada renglón de texto.split("\\n"): (profundidad de llaves al
    empezar, si empieza adentro de un /* */). Saltea lo que hay en los
    comentarios, en las "cadenas" y en los 'caracteres' (con su \\).
    -> (mapa, problema). problema es None, o un texto si las llaves no
    cierran: entonces no se toca nada (todo queda solo para ver)."""
    mapa = [(0, False)]
    prof = 0
    estado = "normal"            # normal / bloque /* */ / linea // / cadena " / caracter '
    problema = None
    i, n = 0, len(texto)
    while i < n:
        c = texto[i]
        if c == "\n":
            if estado == "linea":
                estado = "normal"
            elif estado in ("cadena", "caracter"):
                problema = problema or "hay una cadena sin cerrar en el renglón %d" % len(mapa)
                estado = "normal"
            mapa.append((prof, estado == "bloque"))
            i += 1
            continue
        if estado == "normal":
            if c == "/" and texto.startswith("/*", i):
                estado = "bloque"
                i += 2
                continue
            if c == "/" and texto.startswith("//", i):
                estado = "linea"
                i += 2
                continue
            if c == '"':
                estado = "cadena"
            elif c == "'":
                estado = "caracter"
            elif c == "{":
                prof += 1
            elif c == "}":
                prof -= 1
                if prof < 0 and problema is None:
                    problema = "sobra una } en el renglón %d" % len(mapa)
        elif estado == "bloque":
            if c == "*" and texto.startswith("*/", i):
                estado = "normal"
                i += 2
                continue
        elif estado in ("cadena", "caracter"):
            if c == "\\":
                i += 1 if texto.startswith("\n", i + 1) else 2     # (el \n lo cuenta la vuelta de arriba)
                continue
            if (c == '"' and estado == "cadena") or (c == "'" and estado == "caracter"):
                estado = "normal"
        i += 1
    if problema is None:
        if estado == "bloque":
            problema = "un /* que no se cierra"
        elif prof != 0:
            problema = "las { y las } no dan parejo (quedan %d abiertas)" % prof
    return mapa, problema


_CACHE_DECLARACION = {}


def _re_declaracion(nombre):
    """La declaración de UN nombre, en UN renglón entero. [ \\t] y no \\s (para
    no cruzar renglones); re.escape(nombre) seguido de [ \\t]*= (así
    umbralBlancoX no es umbralBlanco)."""
    r = _CACHE_DECLARACION.get(nombre)
    if r is None:
        r = re.compile(
            r"^(?P<tipo>(?:const[ \t]+)?(?:unsigned[ \t]+long|int|float|bool))[ \t]+"
            + re.escape(nombre) +
            r"[ \t]*=[ \t]*(?P<valor>[^;\n]*?)[ \t]*;(?P<resto>[^\n]*)$")
        _CACHE_DECLARACION[nombre] = r
    return r


# El valor tiene que ser un número simple ([0-9] y no \d: en Python, \d
# acepta también dígitos árabes y otros).
_VALOR_SEGUN_TIPO = {"i": r"-?[0-9]+", "l": r"-?[0-9]+", "u": r"[0-9]+", "v": r"[0-9]+",
                     "f": r"-?[0-9]+(?:\.[0-9]+)?", "b": r"true|false"}
_RE_VALOR_SEGUN_TIPO = {k: re.compile(r"^(?:%s)$" % v) for k, v in _VALOR_SEGUN_TIPO.items()}
# Un renglón que empieza como declaración: para ver de qué nombre es.
_RE_EMPIEZA_DECLARACION = re.compile(r"^(?:const[ \t]+)?(?:unsigned[ \t]+long|int|float|bool)[ \t]+"
                                     r"([A-Za-z_][A-Za-z0-9_]*)")
# Lo que la pestaña escribe (cajita, [+], CANCHA): solo cifras ASCII.
_RE_ENTRADA_PARTIDO = re.compile(r"^-?[0-9]+(?:\.[0-9]+)?$")


def _re_cadena(nombre):
    # VERSION_PARTIDO y SELLO: const char* NOMBRE = "texto";
    return re.compile(r'^const[ \t]+char[ \t]*\*[ \t]*' + nombre +
                      r'[ \t]*=[ \t]*"(?P<valor>[A-Za-z0-9._-]{1,20})"[ \t]*;(?P<resto>[^\n]*)$')


_RE_VERSION_PARTIDO = _re_cadena("VERSION_PARTIDO")
_RE_SELLO = _re_cadena("SELLO")


def _tipo_base(declaracion):
    """'const unsigned long' -> 'unsigned long' (el const no importa)."""
    t = re.sub(r"[ \t]+", " ", declaracion.strip())
    return t[6:] if t.startswith("const ") else t


def leer_partido(texto):
    """Lee los valores de TABLA_PARTIDO en el texto del .ino. Solo valen los
    renglones que empiezan con profundidad de llaves 0 y fuera de un /* */.
    -> {"valores": {nombre: "425"} (el texto TAL CUAL), "donde": {nombre:
    (renglón, ini, fin)} (las columnas del valor), "tipos", "restos" (lo que
    sigue al ';', el comentario), "faltan", "repetidos", "raros":
    [(nombre, motivo)], "version", "sello", "donde_sello", "problemas",
    "huella", "texto"}. Lo que cae en faltan, repetidos o raros NO se escribe."""
    lineas = texto.split("\n")
    mapa, problema = mapa_de_lineas(texto)
    r = {"valores": {}, "donde": {}, "tipos": {}, "restos": {}, "faltan": [], "repetidos": [], "raros": [],
         "version": None, "sello": None, "donde_sello": None, "problemas": [],
         "huella": huella_de(texto.encode("utf-8")), "texto": texto}
    if problema:
        r["problemas"].append("No entiendo las llaves { } del programa (%s): no toco nada." % problema)
    vistos = {}
    cadenas = {"version": [], "sello": []}
    for n, linea in enumerate(lineas):
        prof, en_bloque = mapa[n] if n < len(mapa) else (1, True)
        if prof != 0 or en_bloque:
            continue
        m = _RE_EMPIEZA_DECLARACION.match(linea)
        if m:
            nombre = m.group(1)
            if nombre in FILA_PARTIDO:
                d = _re_declaracion(nombre).match(linea)
                vistos.setdefault(nombre, []).append((n, d))
            continue
        if linea.startswith("const"):
            for clave, expresion in (("version", _RE_VERSION_PARTIDO), ("sello", _RE_SELLO)):
                c = expresion.match(linea)
                if c:
                    cadenas[clave].append((n, c))
    for nombre in ORDEN_PARTIDO:
        apariciones = vistos.get(nombre, [])
        if not apariciones:
            r["faltan"].append(nombre)
            continue
        if len(apariciones) > 1:
            r["repetidos"].append(nombre)
            continue
        n, d = apariciones[0]
        fila = FILA_PARTIDO[nombre]
        if d is None:
            r["raros"].append((nombre, "está declarado, pero no con 'tipo nombre = valor;' en un solo renglón"))
            continue
        tipo = re.sub(r"[ \t]+", " ", d.group("tipo"))
        valor = d.group("valor")
        if _tipo_base(tipo) != _tipo_base(fila[1]):
            r["raros"].append((nombre, "es %s y tenía que ser %s" % (tipo, fila[1])))
        elif not _RE_VALOR_SEGUN_TIPO[fila[2]].match(valor):
            r["raros"].append((nombre, "el valor '%s' no es un número simple" % valor[:20]))
        else:
            r["valores"][nombre] = valor
            r["donde"][nombre] = (n, d.start("valor"), d.end("valor"))
            r["tipos"][nombre] = tipo
            r["restos"][nombre] = d.group("resto")
    for clave, lista in cadenas.items():
        if len(lista) == 1:
            n, c = lista[0]
            r[clave] = c.group("valor")
            if clave == "sello":
                r["donde_sello"] = (n, c.start("valor"), c.end("valor"))
        elif len(lista) > 1:
            r["problemas"].append("%s está repetido en el programa: no toco nada." % (
                "VERSION_PARTIDO" if clave == "version" else "SELLO"))
    return r


def leer_partido_archivo(ruta):
    """Lee el .ino en BYTES y lo decodifica UTF-8 estricto. La huella es la
    de los bytes. NoSeEscribe si no es UTF-8 (OSError si no se puede abrir)."""
    with open(ruta, "rb") as f:
        datos = f.read()
    try:
        texto = datos.decode("utf-8")
    except UnicodeDecodeError:
        raise NoSeEscribe("El archivo no es UTF-8: no lo toco.")
    r = leer_partido(texto)
    r["huella"] = huella_de(datos)
    r["ruta"] = ruta
    return r


def _decimales_paso(paso):
    if paso is None or float(paso) == int(paso):
        return 0
    return _decimales(repr(float(paso)))


def _g(x):
    """Un número lindo para un texto: 1000 -> '1000', 0.5 -> '0.5'."""
    return "%g" % x if isinstance(x, float) else str(x)


def valor_de_texto(nombre, texto):
    """'430' -> 430, '2,9' -> 2.9, 'true' -> True. Solo cifras ASCII: nada de
    letras, 'e', '0x', 'nan' ni 'inf'. ValueError con el motivo."""
    fila = FILA_PARTIDO.get(nombre)
    if fila is None:
        raise ValueError("%s no está en la lista del partido" % nombre)
    tipo = fila[2]
    if isinstance(texto, bool):
        if tipo != "b":
            raise ValueError("%s es un número, no SÍ/NO" % nombre)
        return texto
    t = str(texto).strip()
    if tipo == "b":
        if t in ("true", "1"):
            return True
        if t in ("false", "0"):
            return False
        raise ValueError("%s es SÍ o NO" % nombre)
    t = t.replace(",", ".")
    if not t or len(t) > LARGO_MAX_NUMERO or not _RE_ENTRADA_PARTIDO.match(t):
        raise ValueError("'%s' no es un número (para %s). Escribí por ejemplo 430 o 2.9" % (str(texto).strip()[:20], nombre))
    v = float(t)
    if tipo in ("i", "u", "l", "v"):
        if v != int(v):
            raise ValueError("%s va sin coma: tiene que ser un número entero" % nombre)
        return int(v)
    return v


def mismo_valor(nombre, a, b):
    """¿Son el mismo valor? (2.870 y 2.87 sí; 'true' y True sí)."""
    try:
        x = a if not isinstance(a, str) else valor_de_texto(nombre, a)
        y = b if not isinstance(b, str) else valor_de_texto(nombre, b)
    except ValueError:
        return False
    if isinstance(x, bool) or isinstance(y, bool):
        return bool(x) == bool(y) and isinstance(x, bool) and isinstance(y, bool)
    return abs(float(x) - float(y)) <= 1e-9 * max(1.0, abs(float(x)))


def validar_valor_partido(nombre, texto):
    """-> (valor, None) si sirve, o (None, motivo) si no. Mira el tipo, el
    rango (los mínimos y máximos SEGUROS de la tabla) y que no sea 'solo ver'."""
    fila = FILA_PARTIDO.get(nombre)
    if fila is None:
        return None, "%s no está en el programa de partido" % nombre
    if fila[2] == "v":
        return None, "%s es solo para ver: no se cambia desde acá" % nombre
    try:
        v = valor_de_texto(nombre, texto)
    except ValueError as e:
        return None, str(e)
    if fila[2] == "b":
        return v, None
    if fila[2] == "l" and v not in (-1, 1):
        return None, "%s es -1 (lateral IZQUIERDO) o 1 (DERECHO)" % nombre
    minimo, maximo = fila[5], fila[6]
    if not minimo - 1e-9 <= v <= maximo + 1e-9:
        return None, "%s va de %s a %s" % (nombre, _g(minimo), _g(maximo))
    return v, None


def texto_para_ino(nombre, valor, texto_viejo):
    """Cómo se escribe el valor en el .ino. Si es el MISMO valor que el
    viejo (2.870 y 2.87), devuelve el viejo tal cual: así, sin cambios, el
    archivo queda idéntico byte a byte. Los float con los decimales que ya
    tenían, o con los del paso si son más (12 -> '12.0', 2.9 -> '2.90')."""
    fila = FILA_PARTIDO[nombre]
    tipo = fila[2]
    v = valor_de_texto(nombre, valor)
    if texto_viejo is not None and mismo_valor(nombre, v, texto_viejo):
        return texto_viejo
    if tipo == "b":
        return "true" if v else "false"
    if tipo in ("i", "u", "l"):
        return str(int(v))
    if tipo == "f":
        pedido = str(valor).strip().replace(",", ".") if isinstance(valor, str) else ""
        dec = max(_decimales(texto_viejo or ""), _decimales_paso(fila[7]), _decimales(pedido), 1)
        t = "%.*f" % (dec, v)
        if float(t) == 0:
            t = "%.*f" % (dec, 0.0)          # sin "-0.0"
        return t
    raise ValueError("%s es solo para ver" % nombre)


def _reglas_con_codigo(valores, valores_del_archivo=None, medido=None):
    """Las reglas que atan valores (DISENO sección 16). -> [(nivel, texto,
    [nombres], código)]. nivel: "error" (rojo: no deja guardar ni cargar),
    "ojo" (naranja: deja, pero lo repite al cargar) o "nota" (gris)."""
    salida = []
    arch = valores_del_archivo or {}
    medido = medido or {}
    v = {}
    for nombre in ORDEN_PARTIDO:
        if nombre not in valores:
            continue
        if FILA_PARTIDO[nombre][2] == "v":
            try:
                v[nombre] = valor_de_texto(nombre, valores[nombre])
            except ValueError:
                pass
            continue
        valor, motivo = validar_valor_partido(nombre, valores[nombre])
        if motivo:
            salida.append(("error", motivo + ".", [nombre], "E6"))
        else:
            v[nombre] = valor

    def hay(*nombres):
        return all(n in v for n in nombres)

    def cambio(nombre):
        return nombre in v and nombre in arch and not mismo_valor(nombre, v[nombre], arch[nombre])

    def error(cod, texto, *nombres):
        salida.append(("error", texto, list(nombres), cod))

    def ojo(cod, texto, *nombres):
        salida.append(("ojo", texto, list(nombres), cod))

    def nota(cod, texto, *nombres):
        salida.append(("nota", texto, list(nombres), cod))

    if hay("pwmMinLateral", "pwmMaxLateral") and v["pwmMinLateral"] > v["pwmMaxLateral"]:
        error("E1", "pwmMinLateral (%d) es más que pwmMaxLateral (%d): tiene que ser menos o igual." % (
            v["pwmMinLateral"], v["pwmMaxLateral"]), "pwmMinLateral", "pwmMaxLateral")
    if hay("PWM_MIN_ACOMODO", "PWM_MAX_ACOMODO") and v["PWM_MIN_ACOMODO"] > v["PWM_MAX_ACOMODO"]:
        error("E2", "PWM_MIN_ACOMODO (%d) es más que PWM_MAX_ACOMODO (%d): tiene que ser menos o igual." % (
            v["PWM_MIN_ACOMODO"], v["PWM_MAX_ACOMODO"]), "PWM_MIN_ACOMODO", "PWM_MAX_ACOMODO")
    if hay("TOLERANCIA_ACOMODO", "TOLERANCIA_ESPERA") and v["TOLERANCIA_ACOMODO"] >= v["TOLERANCIA_ESPERA"]:
        error("E3", "TOLERANCIA_ACOMODO (%s) tiene que ser MENOS que TOLERANCIA_ESPERA (%s): si son iguales, "
              "tiembla." % (_g(v["TOLERANCIA_ACOMODO"]), _g(v["TOLERANCIA_ESPERA"])),
              "TOLERANCIA_ACOMODO", "TOLERANCIA_ESPERA")
    if hay("MS_PAUSA_MEDIO", "MS_MAX_ESPERA_PLANO") and v["MS_PAUSA_MEDIO"] > v["MS_MAX_ESPERA_PLANO"]:
        error("E4", "MS_PAUSA_MEDIO (%d) es más que MS_MAX_ESPERA_PLANO (%d): lo mínimo no puede ser más que "
              "lo máximo." % (v["MS_PAUSA_MEDIO"], v["MS_MAX_ESPERA_PLANO"]), "MS_PAUSA_MEDIO", "MS_MAX_ESPERA_PLANO")
    if hay("msAdelante", "umbralCm", "potenciaDespeje") and v["potenciaDespeje"] == 200:
        alcance = v["msAdelante"] / 10.0 - 3.3        # medido el 04/08, a fuerza 200
        if alcance <= v["umbralCm"]:
            error("E5", "La ida llega a %.0f cm y sale con la pelota a %s: no la toca nunca. Subí msAdelante o "
                  "bajá umbralCm." % (alcance, _g(v["umbralCm"])), "msAdelante", "umbralCm")
        elif alcance - v["umbralCm"] < 10:
            ojo("O1", "La ida llega a %.0f cm, apenas %.0f más que umbralCm (%s): si la pelota se corre, no la "
                "toca." % (alcance, alcance - v["umbralCm"], _g(v["umbralCm"])), "msAdelante", "umbralCm")
    if hay("MS_MAX_RETROCESO", "msAdelante") and v["MS_MAX_RETROCESO"] < 2.4 * v["msAdelante"]:
        ojo("O2", "MS_MAX_RETROCESO (%d) es menos que 2,4 × msAdelante (%d): la vuelta (a 110) es unas 2,4 veces "
            "más lenta que la ida y se puede cortar antes de la línea." % (v["MS_MAX_RETROCESO"], 2.4 * v["msAdelante"]),
            "MS_MAX_RETROCESO", "msAdelante")
    if hay("pwmMaxLateral") and v["pwmMaxLateral"] > 160:
        ojo("O3", "pwmMaxLateral %d: más de 160 casi no se probó (a 160 una vez chocó la madera)." % v["pwmMaxLateral"],
            "pwmMaxLateral")
    if hay("volverAlMedioEnElLateral", "pwmMaxLateral") and not v["volverAlMedioEnElLateral"] \
            and v["pwmMaxLateral"] >= 150:
        ojo("O4", "Sin límite lateral y con pwmMaxLateral %d: se puede ir de largo y chocar la madera." %
            v["pwmMaxLateral"], "volverAlMedioEnElLateral", "pwmMaxLateral")
    if hay("MS_MEDIO_TOPE_LATERAL", "MS_MEDIO_AL_MEDIO"):
        tope = v["MS_MEDIO_TOPE_LATERAL"]
        cruce = medido.get("cruce")
        if tope < 2 * v["MS_MEDIO_AL_MEDIO"]:
            ojo("O5", "MS_MEDIO_TOPE_LATERAL (%d) es menos que 2 × MS_MEDIO_AL_MEDIO (%d): puede creer que llegó al "
                "borde sin llegar." % (tope, 2 * v["MS_MEDIO_AL_MEDIO"]), "MS_MEDIO_TOPE_LATERAL", "MS_MEDIO_AL_MEDIO")
        elif cruce and tope < 1.3 * cruce:
            ojo("O5", "MS_MEDIO_TOPE_LATERAL (%d) es menos que 1,3 × el cruce medido en CANCHA (%d ms)." % (
                tope, cruce), "MS_MEDIO_TOPE_LATERAL")
    movidos = [n for n in NUMEROS_DEL_MOVIMIENTO if cambio(n)]
    if movidos:
        ojo("O6", "Cambiaste %s: MS_MEDIO_AL_MEDIO se midió con los de antes. Medilo de nuevo (CANCHA paso 2)." %
            ", ".join(movidos), "MS_MEDIO_AL_MEDIO", *movidos)
    if cambio("potenciaDespeje") or cambio("RAMPA_MOV_PASO"):
        ojo("O7", "msAdelante se midió a fuerza 200 y con la rampa de antes: medilo de nuevo (CANCHA paso 5).",
            "msAdelante", "potenciaDespeje", "RAMPA_MOV_PASO")
    if cambio("CAMARA_POR_CM"):
        ojo("O8", "Cambiaste CAMARA_POR_CM: cambian TODOS los cm (umbralCm, umbralDesvio, ZONA_MUERTA_PELOTA), y "
            "kpLateral está pensado para 2,87.", "CAMARA_POR_CM")
    if cambio("LADO_MEDIO"):
        ojo("O9", "Cambiaste LADO_MEDIO: MS_MEDIO_AL_MEDIO está medido desde el lateral IZQUIERDO (desde el derecho "
            "daba 2331). Medilo de nuevo.", "LADO_MEDIO", "MS_MEDIO_AL_MEDIO")
    if hay("despejeEnDiagonal") and v["despejeEnDiagonal"]:
        ojo("O10", "despejeEnDiagonal está en SÍ: está ESCRITO Y SIN PROBAR.", "despejeEnDiagonal")
    if hay("enderezarActivado") and not v["enderezarActivado"]:
        ojo("O11", "enderezarActivado está en NO: sin límite lateral, ni patrulla, ni ir al medio, ni se endereza.",
            "enderezarActivado")
    signos = [n for n in ("lateralInvertido", "camaraYInvertida") if cambio(n)]
    if signos:
        ojo("O12", "¡Cambiaste un SIGNO (%s)! Si estaba bien, ahora el robot se aleja de la pelota." %
            ", ".join(signos), *signos)
    if hay("patrullarSinPelota") and not v["patrullarSinPelota"]:
        ojo("O13", "Sin patrulla: si queda quieto mucho rato, el juez lo puede sacar (pasó el 21/09 y el 12/10).",
            "patrullarSinPelota")
    if hay("potenciaDespeje") and v["potenciaDespeje"] != 200:
        ojo("O14", "potenciaDespeje no es 200: la cuenta de cuántos cm llega la ida ya no vale (no puedo revisar "
            "si la ida llega a la pelota).", "potenciaDespeje", "msAdelante")
    if hay("MS_ASENTAR_ACOMODO", "msFreno") and v["MS_ASENTAR_ACOMODO"] < v["msFreno"]:
        ojo("O15", "MS_ASENTAR_ACOMODO (%d) es menos que msFreno (%d): mide antes de que termine de frenar." % (
            v["MS_ASENTAR_ACOMODO"], v["msFreno"]), "MS_ASENTAR_ACOMODO", "msFreno")
    if hay("umbralBlanco") and (medido.get("peor_verde") is not None or medido.get("peor_blanco") is not None):
        u = v["umbralBlanco"]
        pv, pb = medido.get("peor_verde"), medido.get("peor_blanco")
        if (pv is not None and u - pv < 80) or (pb is not None and pb - u < 80):
            ojo("O16", "umbralBlanco %d queda a menos de 80 de lo medido en CANCHA (peor verde %s, peor blanco %s)." % (
                u, "?" if pv is None else _g(pv), "?" if pb is None else _g(pb)), "umbralBlanco")
    if hay("despejeEnDiagonal", "MS_MAX_ADELANTE", "msAdelante") and v["despejeEnDiagonal"] \
            and v["MS_MAX_ADELANTE"] < v["msAdelante"]:
        ojo("O17", "Con la diagonal: MS_MAX_ADELANTE (%d) es menos que msAdelante (%d)." % (
            v["MS_MAX_ADELANTE"], v["msAdelante"]), "MS_MAX_ADELANTE", "msAdelante")
    if hay("umbralDesvio", "despejeEnDiagonal") and v["umbralDesvio"] > 7 and not v["despejeEnDiagonal"]:
        nota("N1", "Con despejeEnDiagonal en NO sale siempre DERECHO: con la pelota a %s cm al costado, derecho no "
             "la toca." % _g(v["umbralDesvio"]), "umbralDesvio")
    if hay("pisoRampaIda", "potenciaDespeje") and v["pisoRampaIda"] > v["potenciaDespeje"]:
        nota("N2", "pisoRampaIda (%d) es más que potenciaDespeje (%d): la ida sale de una con toda la fuerza." % (
            v["pisoRampaIda"], v["potenciaDespeje"]), "pisoRampaIda")
    if hay("MS_QUIETO_PARA_PATRULLAR") and v["MS_QUIETO_PARA_PATRULLAR"] < 10000:
        nota("N3", "MS_QUIETO_PARA_PATRULLAR menos de 10 s: más patrullas, más rato con el arco solo.",
             "MS_QUIETO_PARA_PATRULLAR")
    return salida


def reglas_partido(valores, valores_del_archivo=None, medido=None):
    """-> [(nivel, texto, [nombres])]. Ver _reglas_con_codigo."""
    return [r[:3] for r in _reglas_con_codigo(valores, valores_del_archivo, medido)]


def escribir_partido(texto, lectura, cambios, sello=None):
    """El texto nuevo con los cambios {nombre: valor}. Solo cambia lo de
    adentro del valor (el comentario queda igual). -> (texto_nuevo,
    [(nombre, viejo, nuevo)] con los cambios DE VERDAD). Con sello, y si hubo
    cambios, también cambia el SELLO. NoSeEscribe si algo no se puede."""
    if lectura["problemas"]:
        raise NoSeEscribe(lectura["problemas"][0])
    no_usables = set(lectura["faltan"]) | set(lectura["repetidos"]) | {r[0] for r in lectura["raros"]}
    errores, reales = [], []
    for nombre in sorted(cambios, key=lambda n: ORDEN_PARTIDO.index(n) if n in FILA_PARTIDO else 999):
        if nombre in no_usables:
            errores.append("%s no está bien en el programa (falta, está repetido o es raro): no lo escribo" % nombre)
            continue
        _, motivo = validar_valor_partido(nombre, cambios[nombre])
        if motivo:
            errores.append(motivo)
            continue
        viejo = lectura["valores"][nombre]
        nuevo = texto_para_ino(nombre, cambios[nombre], viejo)
        if nuevo != viejo:
            reales.append((nombre, viejo, nuevo))
    if errores:
        raise NoSeEscribe("; ".join(errores) + ".")
    lineas = texto.split("\n")
    for nombre, viejo, nuevo in reales:
        n, ini, fin = lectura["donde"][nombre]
        linea = lineas[n]
        if linea[ini:fin] != viejo:
            raise NoSeEscribe("El programa no es el que leí (%s): no lo toco." % nombre)
        lineas[n] = linea[:ini] + nuevo + linea[fin:]
    if sello is not None and reales and lectura["donde_sello"] is not None:
        n, ini, fin = lectura["donde_sello"]
        lineas[n] = lineas[n][:ini] + sello + lineas[n][fin:]
    return "\n".join(lineas), reales


def verificar_escritura(viejo, nuevo, reales, lectura_vieja, sello=None):
    """La 3ra capa: antes de escribir, que el texto nuevo sea el viejo con
    SOLO esos números cambiados. NoSeEscribe si algo no da."""
    a, b = viejo.split("\n"), nuevo.split("\n")
    if len(a) != len(b):
        raise NoSeEscribe("El texto nuevo no tiene los mismos renglones: no lo escribo.")
    permitidas = {}
    for nombre, _, _ in reales:
        n, ini, fin = lectura_vieja["donde"][nombre]
        permitidas[n] = (ini, fin)
    if sello is not None and reales and lectura_vieja["donde_sello"] is not None:
        n, ini, fin = lectura_vieja["donde_sello"]
        permitidas[n] = (ini, fin)
    for n, (x, y) in enumerate(zip(a, b)):
        if x == y:
            continue
        if n not in permitidas:
            raise NoSeEscribe("Iba a cambiar el renglón %d, que no es de un número: no lo escribo." % (n + 1))
        ini, fin = permitidas[n]
        largo = len(x) - fin
        if x[:ini] != y[:ini] or x[fin:] != y[len(y) - largo:]:
            raise NoSeEscribe("Iba a cambiar el comentario del renglón %d: no lo escribo." % (n + 1))
    nueva = leer_partido(nuevo)
    pedidos = {nombre: n for nombre, _, n in reales}
    for nombre in ORDEN_PARTIDO:
        if nombre in lectura_vieja["valores"]:
            esperado = pedidos.get(nombre, lectura_vieja["valores"][nombre])
            if nueva["valores"].get(nombre) != esperado:
                raise NoSeEscribe("Leyendo el texto nuevo, %s no da lo pedido: no lo escribo." % nombre)
    if (set(nueva["faltan"]) - set(lectura_vieja["faltan"])) or \
            (set(nueva["repetidos"]) - set(lectura_vieja["repetidos"])) or nueva["problemas"]:
        raise NoSeEscribe("El texto nuevo tiene nombres que faltan o repetidos: no lo escribo.")
    if sello is not None and reales and lectura_vieja["donde_sello"] is not None and nueva["sello"] != sello:
        raise NoSeEscribe("El SELLO nuevo no quedó bien: no lo escribo.")
    return nueva


def nombre_copia(ahora):
    """partido-0921_AAAA-MM-DD_HH-MM-SS.txt (.txt: PlatformIO no lo compila)."""
    return time.strftime("partido-0921_%Y-%m-%d_%H-%M-%S.txt", ahora)


def _adentro_de(carpeta, otra):
    a = os.path.normcase(os.path.abspath(carpeta))
    b = os.path.normcase(os.path.abspath(otra))
    return a == b or a.startswith(b.rstrip("\\/") + os.sep)


def hacer_copia(carpeta_copias, carpeta_programa, datos, ahora):
    """La copia de seguridad con los bytes EXACTOS de antes. Nunca adentro
    de la carpeta del programa y nunca con una extensión que se compile.
    Si ya existe ese nombre: _2, _3... -> la ruta de la copia."""
    if _adentro_de(carpeta_copias, carpeta_programa):
        raise NoSeEscribe("La carpeta de las copias está adentro de la del programa: no lo toco.")
    base = nombre_copia(ahora)
    if os.path.splitext(base)[1].lower() in EXTENSIONES_QUE_COMPILAN:
        raise NoSeEscribe("La copia tendría una extensión que se compila: no lo toco.")
    os.makedirs(carpeta_copias, exist_ok=True)
    ruta, k = os.path.join(carpeta_copias, base), 2
    while True:
        try:
            with open(ruta, "xb") as f:
                f.write(datos)
                f.flush()
                os.fsync(f.fileno())
            break
        except FileExistsError:
            ruta = os.path.join(carpeta_copias, base[:-4] + "_%d.txt" % k)
            k += 1
    with open(ruta, "rb") as f:
        if f.read() != datos:
            raise NoSeEscribe("La copia de seguridad no quedó bien: no toco el programa.")
    return ruta


def copias_del_partido(carpeta_copias):
    """Las copias, de la MÁS NUEVA a la más vieja."""
    try:
        nombres = os.listdir(carpeta_copias)
    except OSError:
        return []
    copias = []
    for nombre in nombres:
        m = RE_NOMBRE_COPIA.match(nombre)
        if m:
            copias.append(((m.group(1), int(m.group(2) or 1)), os.path.join(carpeta_copias, nombre)))
    return [ruta for _, ruta in sorted(copias, reverse=True)]


def elegir_copia_para_volver(carpeta_copias, valores_archivo):
    """'Volver a como estaba': la copia más nueva cuyos VALORES (los de la
    tabla) son distintos de los del archivo. -> (ruta, [(nombre, archivo,
    copia)]) o None. Vuelve valores y no bytes, para no borrar cambios de
    código que alguien haya hecho afuera."""
    for ruta in copias_del_partido(carpeta_copias):
        try:
            with open(ruta, "rb") as f:
                lectura = leer_partido(f.read().decode("utf-8"))
        except (OSError, UnicodeDecodeError):
            continue
        difs = [(n, valores_archivo[n], lectura["valores"][n]) for n in ORDEN_PARTIDO
                if n in valores_archivo and n in lectura["valores"] and FILA_PARTIDO[n][2] != "v"
                and not mismo_valor(n, valores_archivo[n], lectura["valores"][n])]
        if difs:
            return ruta, difs
    return None


def escribir_bytes_atomico(ruta, datos):
    """Escribe BYTES de una: primero un .tmp al lado (eso no lo compila
    nadie), y después os.replace, con hasta 3 intentos (Windows a veces lo
    tiene agarrado un ratito). Si no puede, borra el .tmp y avisa."""
    if os.path.normcase(os.path.abspath(ruta)) in RUTAS_QUE_NO_SE_ESCRIBEN:
        raise NoSeEscribe("Ese archivo no se escribe (lo cuida la autoprueba).")
    carpeta = os.path.dirname(os.path.abspath(ruta))
    fd, temporal = tempfile.mkstemp(prefix=".partido-", suffix=".tmp", dir=carpeta)
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(datos)
            f.flush()
            os.fsync(f.fileno())
        for intento in range(3):
            try:
                os.replace(temporal, ruta)
                return
            except PermissionError:
                if intento == 2:
                    raise
                time.sleep(ESPERA_REINTENTO)
    except Exception as e:
        try:
            os.remove(temporal)
        except OSError:
            pass
        raise NoSeEscribe("No pude escribir el programa: ¿está abierto en otro programa (Arduino IDE, "
                          "antivirus)? Cerralo y probá de nuevo. (%s)" % e.__class__.__name__)


def anotar_registro(carpeta_copias, ahora, que, cambios, origenes=None):
    """Un renglón en copias-partido/registro.txt (se lee con el Bloc de notas)."""
    origenes = origenes or {}
    try:
        os.makedirs(carpeta_copias, exist_ok=True)
        with open(os.path.join(carpeta_copias, "registro.txt"), "a", encoding="utf-8", newline="\r\n") as f:
            for nombre, viejo, nuevo in cambios:
                f.write("%s %s %s %s -> %s%s\n" % (time.strftime("%Y-%m-%d %H:%M", ahora), que, nombre, viejo, nuevo,
                                                 " (de %s)" % origenes[nombre] if origenes.get(nombre) else ""))
    except OSError:
        pass


def guardar_partido(ruta, carpeta_copias, cambios, huella_esperada, ahora=None, origenes=None):
    """GUARDAR (y el paso 2 de CARGAR): escribe los cambios {nombre: valor}
    en el .ino, con copia antes. En este orden: ruta prohibida, la huella
    (si cambió por afuera: CambioPorAfuera), las 3 validaciones, la copia,
    la escritura de bytes y volver a leerlo. Sin cambios de verdad no
    escribe nada ni hace copia. -> {"sin_cambios", "copia", "huella_nueva",
    "lectura_nueva", "cambios": [(nombre, viejo, nuevo)], "sello"}."""
    if os.path.normcase(os.path.abspath(ruta)) in RUTAS_QUE_NO_SE_ESCRIBEN:
        raise NoSeEscribe("Ese archivo no se escribe (lo cuida la autoprueba).")
    with open(ruta, "rb") as f:
        datos = f.read()
    if huella_de(datos) != huella_esperada:
        raise CambioPorAfuera("El programa cambió por afuera (¿Arduino IDE?) desde que lo leí.")
    try:
        texto = datos.decode("utf-8")
    except UnicodeDecodeError:
        raise NoSeEscribe("El archivo no es UTF-8: no lo toco.")
    lectura = leer_partido(texto)
    lectura["huella"] = huella_de(datos)
    if lectura["problemas"]:
        raise NoSeEscribe(lectura["problemas"][0])
    valores = dict(lectura["valores"])
    valores.update(cambios)
    errores = [t for nivel, t, _ in reglas_partido(valores, lectura["valores"]) if nivel == "error"]
    if errores:
        raise NoSeEscribe(errores[0])
    ahora = ahora or time.localtime()
    sello = time.strftime("%Y%m%d-%H%M%S", ahora)
    nuevo_texto, reales = escribir_partido(texto, lectura, cambios, sello)
    if not reales:
        return {"sin_cambios": True, "copia": None, "huella_nueva": lectura["huella"], "lectura_nueva": lectura,
                "cambios": [], "sello": lectura["sello"]}
    verificar_escritura(texto, nuevo_texto, reales, lectura, sello)
    nuevos = nuevo_texto.encode("utf-8")
    copia = hacer_copia(carpeta_copias, os.path.dirname(os.path.abspath(ruta)), datos, ahora)
    escribir_bytes_atomico(ruta, nuevos)
    with open(ruta, "rb") as f:
        quedo = f.read()
    if quedo != nuevos:
        try:
            escribir_bytes_atomico(ruta, datos)
        except NoSeEscribe:
            pass
        raise NoSeEscribe("Lo que quedó escrito no es lo que mandé: volví a poner lo de antes (la copia es %s)."
                          % os.path.basename(copia))
    anotar_registro(carpeta_copias, ahora, "GUARDADO", reales, origenes)
    nueva = leer_partido(nuevo_texto)
    nueva["huella"] = huella_de(nuevos)
    nueva["ruta"] = ruta
    return {"sin_cambios": False, "copia": copia, "huella_nueva": nueva["huella"], "lectura_nueva": nueva,
            "cambios": reales, "sello": nueva["sello"]}


def linea_partido_de(valores, version="0921g", sello="fabrica", giro=1):
    """La línea #PARTIDO que imprimiría avisarVersion() con esos valores
    (textos del .ino). La usa la autoprueba para el contrato."""
    partes = ["#PARTIDO version=%s sello=%s giro=%d" % (version, sello, 1 if giro else 0)]
    for nombre in NUMEROS_LINEA_PARTIDO:
        v = valor_de_texto(nombre, valores[nombre])
        dec = DECIMALES_LINEA_PARTIDO.get(nombre)
        partes.append("%s=%s" % (nombre, "%.*f" % (dec, v) if dec else str(int(v))))
    bits = "".join("1" if valor_de_texto(n, valores[n]) else "0" for n in ORDEN_INTERRUPTORES)
    partes.append("interruptores=" + bits)
    return " ".join(partes)


def comparar_con_robot(m, lectura):
    """La #PARTIDO que dijo el robot contra el archivo. -> (estado,
    [(nombre, robot, archivo)]). estado: "IGUAL", "DISTINTO" u
    "OTRA_VERSION". Los números con la tolerancia de los decimales que
    imprimió (11.50 contra 11.5 es IGUAL); los interruptores bit por bit; el
    sello igual al del archivo (con eso se confirman TODOS los valores)."""
    difs = []
    valores = lectura.get("valores", {})
    for nombre in NUMEROS_LINEA_PARTIDO:
        if nombre not in valores or nombre not in m["valores"]:
            continue
        archivo = _numero(valores[nombre])
        robot = m["valores"][nombre]
        tolerancia = 0.5 * 10 ** -m["decimales"].get(nombre, 0) + 1e-6
        if abs(float(robot) - float(archivo)) > tolerancia:
            difs.append((nombre, robot, archivo))
    for nombre, bit in (m.get("llaves") or {}).items():
        if nombre in valores and bit != (valores[nombre] == "true"):
            difs.append((nombre, bit, valores[nombre] == "true"))
    if lectura.get("sello") is not None and m.get("sello") != lectura["sello"]:
        difs.append(("sello", m.get("sello"), lectura["sello"]))
    if lectura.get("version") is None or m.get("version") != lectura["version"]:
        return "OTRA_VERSION", difs
    return ("DISTINTO" if difs else "IGUAL"), difs


# ---------- cargar desde la ventana: las órdenes (puras, se prueban sin ejecutarlas) ----------

def orden_compilar(carpeta):
    """Compilar SOLO (sin -t: el robot sigue conectado y no se toca)."""
    return [PIO, "run", "-e", "teensy41", "-d", carpeta]


def orden_cargar(carpeta):
    """cargar.py tal cual (NUNCA --mirar): él controla que haya UN Teensy y
    sea el arquero, que el puerto esté libre, cierra el Teensy Loader, carga
    y lee el cartel."""
    return [PY, CARGAR_PY, os.path.abspath(carpeta)]


def entorno_carga():
    """cargar.py imprime emojis: con la salida a un tubo y otra codificación
    revienta. Y sin UNBUFFERED la salida llega toda junta al final."""
    env = dict(os.environ)
    env.update(PYTHONIOENCODING="utf-8", PYTHONUTF8="1", PYTHONUNBUFFERED="1")
    return env


def opciones_de_proceso(cwd):
    """stdin=DEVNULL: si cargar.py llegara a su input() ('¿Es el arquero
    igual?'), tira EOFError y termina con error en vez de quedar colgado."""
    return {"stdin": subprocess.DEVNULL, "stdout": subprocess.PIPE, "stderr": subprocess.STDOUT, "cwd": cwd,
            "env": entorno_carga(), "creationflags": getattr(subprocess, "CREATE_NO_WINDOW", 0x08000000)}


SIGUE_CON_LO_QUE_TENIA = "El robot sigue con el programa que tenía."
# (2026-11-08, revisión) Si la grabación YA había empezado y falló (o se cortó
# por el tope), el Teensy 4 puede quedar BORRADO o a medias, en MODO CARGA y
# sin COM: ni esta ventana ni cargar.py lo ven más. NO decir "sigue con lo que
# tenía" (le pasó a este robot el 12/10: "error writing to Teensy").
PUEDE_A_MEDIAS = "PUEDE HABER QUEDADO A MEDIAS (sin programa)"
COMO_SALIR_A_MEDIAS = ("Desenchufá y enchufá el USB: si la ventana lo vuelve a ver, apretá CARGAR de nuevo. Si no lo "
                       "ve más (queda en MODO CARGA): Arduino IDE (placa Teensy 4.1), con SOLO el arquero enchufado, "
                       "y cargá partido-0921.ino desde ahí (si no arranca la carga, apretá el botón blanco del "
                       "Teensy).")
# Renglones de pio / teensy_loader_cli que dicen que la grabación ya empezó.
MARCAS_DE_GRABAR = ("Uploading", "upload protocol", "Rebooting", "HalfKay", "Programming", "teensy_loader",
                    "teensy_reboot", "Waiting for Teensy", "[upload]", "writing to Teensy")


def motivo_de_carga(lineas, codigo, tope=False):
    """Lo que dijo cargar.py, en castellano. -> (salió bien, texto, nivel)
    con nivel "bien", "malo" (rojo) u "ojo" (naranja). "Sigue con lo que
    tenía" SOLO cuando la salida muestra que no se empezó a grabar (los
    rechazos de cargar.py, o que no compiló); si no, PUEDE_A_MEDIAS."""
    todo = "\n".join(lineas or [])
    if tope:
        return False, ("Se colgó más de 4 min y la corté: " + PUEDE_A_MEDIAS + ". " + COMO_SALIR_A_MEDIAS), "malo"
    if codigo == 0:
        return True, "SALIÓ BIEN", "bien"
    esperando = any("Waiting for Teensy" in l and "Si dice" not in l for l in (lineas or []))
    grabando = any(m in l and "Si dice" not in l for l in (lineas or []) for m in MARCAS_DE_GRABAR)
    no_compilo = (not grabando) and any(": error:" in l or "fatal error" in l for l in (lineas or []))
    if "NO HAY NINGUN TEENSY" in todo:
        texto, nivel = "No veo ningún robot enchufado. No se cargó nada.", "ojo"
    elif "TEENSY ENCHUFADOS" in todo:
        texto, nivel = "Hay más de un Teensy: dejá SOLO el arquero.", "ojo"
    elif "DEL DELANTERO" in todo:
        texto, nivel = "¡Ese es el DELANTERO! No se cargó nada.", "malo"
    elif "tiene serie" in todo or "EOFError" in todo:
        texto, nivel = "Ese Teensy no es el arquero. No se cargó nada.", "malo"
    elif "OTRA VENTANA ESTA USANDO" in todo:
        texto, nivel = ("Otro programa tiene el puerto (Monitor Serie del IDE, MIRAR, otra ventana del tablero): "
                        "cerralo.", "ojo")
    elif esperando:
        return False, ("El Teensy no entró a cargar: " + PUEDE_A_MEDIAS + ". " + COMO_SALIR_A_MEDIAS), "malo"
    elif "NO ENCUENTRO EL PROGRAMA" in todo:
        texto, nivel = "No encuentro la carpeta del programa.", "malo"
    elif no_compilo:
        texto, nivel = "No compiló (al cargar): no se grabó nada. Mirá el error en el log.", "malo"
    else:
        # No se sabe hasta dónde llegó: puede que ya haya borrado el Teensy.
        return False, ("Falló (código %s) mientras cargaba: %s. %s Lo que dijo está en el log." % (
            codigo, PUEDE_A_MEDIAS, COMO_SALIR_A_MEDIAS)), "malo"
    return False, texto + " " + SIGUE_CON_LO_QUE_TENIA, nivel


def lineas_partido_de_la_salida(lineas):
    """Las #PARTIDO que cargar.py leyó del robot al arrancar (ya interpretadas)."""
    lineas = list(lineas or [])
    marca = next((i for i, l in enumerate(lineas) if "lo que dijo el robot al arrancar" in l), None)
    salida = []
    for l in lineas[(marca + 1) if marca is not None else 0:]:
        m = interpretar(l)
        if m["tipo"] == "PARTIDO":
            salida.append(m)
    return salida


# =====================================================================
#  4. EL CABLE: un hilo que lee el USB y otro que escribe
# =====================================================================

class Conexion:
    """Habla con el robot de verdad. Deja en la cola:
        ("buscando",)                     no hay ningún arquero enchufado
        ("otro", puerto, serie)           hay una Teensy, pero NO es el arquero
                                          (el delantero u otra): su puerto NO
                                          se abre (sigue buscando)
        ("ocupado", puerto, error)        está, pero otro programa tiene el puerto
        ("conectado", puerto, serie)
        ("linea", texto)                  un renglón que mandó el robot
        ("desconectado", puerto)
    y se reconecta sola si se desenchufa el cable (en la cancha se
    desenchufa A PROPÓSITO: es el modo SIN CABLE)."""

    def __init__(self, cola, puerto_forzado=None):
        self.cola = cola
        self.puerto_forzado = puerto_forzado
        self._sp = None
        self._salida = queue.Queue()
        self._seguir = True
        self._hilos = []

    def arrancar(self):
        for tarea, nombre in ((self._leer, "lector-usb"), (self._escribir, "escritor-usb")):
            h = threading.Thread(target=tarea, name=nombre, daemon=True)
            h.start()
            self._hilos.append(h)

    def enviar(self, texto):
        """Deja una orden para mandar. La manda el hilo escritor, así la
        ventana nunca se queda esperando al cable."""
        if self._sp is None:
            return False
        self._salida.put(texto)
        return True

    def enviar_ya(self, texto):
        """Manda en el momento (al cerrar la ventana y en VOLVER AL PARTIDO)."""
        sp = self._sp
        if sp is None:
            return False
        try:
            sp.write((texto + "\n").encode("ascii", "replace"))
            sp.flush()
            return True
        except Exception:
            return False

    def cerrar(self, esperar=False):
        """Para los hilos. Con esperar=True además SUELTA EL PUERTO y espera
        a que terminen (VOLVER AL PARTIDO: el cargador necesita el puerto)."""
        self._seguir = False
        if not esperar:
            return
        sp = self._sp
        if sp is not None:
            try:
                sp.close()
            except Exception:
                pass
        for h in self._hilos:
            try:
                h.join(1.5)
            except Exception:
                pass

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
        """-> (abrir, otro). abrir = (puerto, serie) del ARQUERO, o None.
        otro = (puerto, serie) de una Teensy que NO es el arquero, o None.
        A esa otra NO se le abre el puerto: si la ventana se lo quedara, el
        otro equipo (el delantero) no podría usar su monitor serie ni
        cargarle un programa. El número de serie ya viene en la lista de
        puertos de Windows, sin abrir nada.
        Con --puerto se abre ESE puerto, pero solo si la lista dice que es
        una Teensy con la serie del arquero (sirve cuando Windows cambió el
        COM, no para hablarle a otro robot)."""
        try:
            puertos = list(list_ports.comports())
        except Exception:
            return None, None
        if self.puerto_forzado:
            for p in puertos:
                if (p.device or "").upper() == self.puerto_forzado.upper():
                    serie = p.serial_number or ""
                    if p.vid == VID_TEENSY and serie == SERIE_ARQUERO:
                        return (p.device, serie), None
                    return None, (p.device, serie)       # "ese puerto no es el arquero"
            return None, None
        teensys = [(p.device, p.serial_number or "") for p in puertos if p.vid == VID_TEENSY]
        for puerto, serie in teensys:
            if serie == SERIE_ARQUERO:
                return (puerto, serie), None
        for puerto, serie in teensys:
            if serie == SERIE_DELANTERO:                 # lo más importante de avisar
                return None, (puerto, serie)
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


# ---------- --quien: qué Teensy hay enchufada (lo usa TABLERO.bat) ----------

AVISO_QUIEN = ("(Un Teensy en modo carga no tiene puerto COM: se busca aparte, en la lista de "
               "dispositivos de Windows.)")
CODIGOS_QUIEN = {"ARQUERO": 0, "DELANTERO": 3, "OTRO": 4, "NINGUNO": 5, "VARIOS": 6, "NOSE": 7, "CARGA": 8}
# Un Teensy en MODO CARGA (el "HalfKay": se apretó el botón, o una carga quedó
# a medias) se presenta con este PID y SIN número de serie a la vista.
PID_MODO_CARGA = "VID_16C0&PID_0478"
AVISO_SIN_PNPUTIL = ("(No pude mirar si hay un Teensy en MODO CARGA: fijate que el OTRO robot NO este "
                     "enchufado a esta PC.)")


def teensys_en_modo_carga():
    """Los Teensy en MODO CARGA que ve Windows, SIN abrir nada (pnputil solo
    lista los dispositivos). -> lista de ids, o None si no se pudo mirar.
    POR QUÉ (2026-11-03, revisión): teensy-cli (el cargador de cargar.py)
    programa PRIMERO cualquier Teensy que ya esté en modo carga, y recién si
    no hay ninguno reinicia el del puerto COM. Si el DELANTERO quedó en modo
    carga en esta misma PC, el programa del arquero entraría en el delantero.
    Ese Teensy no tiene puerto COM: list_ports no lo ve."""
    if os.name != "nt":
        return None
    windows = os.environ.get("SystemRoot") or r"C:\Windows"
    # (Sysnative: si algún día el Python fuera de 32 bits, System32 no tiene pnputil)
    programa = next((c for c in (os.path.join(windows, "Sysnative", "pnputil.exe"),
                                 os.path.join(windows, "System32", "pnputil.exe")) if os.path.exists(c)), "pnputil")
    try:
        r = subprocess.run([programa, "/enum-devices", "/connected"], capture_output=True, timeout=20,
                           creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    except Exception:
        return None
    if r.returncode != 0 or not r.stdout:
        return None
    texto = r.stdout.decode("latin-1", "replace")        # los ids son ASCII: cualquier idioma de Windows sirve
    return ids_en_modo_carga(texto)


def ids_en_modo_carga(texto):
    """Los ids de Teensy en MODO CARGA en lo que listó pnputil. Un mismo
    Teensy aparece DOS veces (USB\\... y su hijo HID\\..., 2026-11-08): si
    hay USB\\..., se cuentan solo esos, para no decir "dos" cuando es uno."""
    ids = sorted(set(re.findall(r"\S*" + re.escape(PID_MODO_CARGA) + r"\S*", texto, re.IGNORECASE)))
    usb = [i for i in ids if i.upper().startswith("USB\\")]
    return usb or ids


def quien(puertos, en_carga=()):
    """Mira la lista de puertos SIN abrir ninguno. -> (renglones, código de salida):
       0 ARQUERO (exactamente una Teensy y es 13348550, y ninguna en modo
       carga), 3 DELANTERO (alguna es 15708680), 4 OTRO (una sola, con otra
       serie), 5 NINGUNO, 6 VARIOS, 8 CARGA (hay una en MODO CARGA: no se
       sabe cuál es, y el cargador le cargaría a ESA).
    en_carga: lo de teensys_en_modo_carga() (None = no se pudo mirar).
    Los renglones van en ASCII (la consola negra de Windows)."""
    teensys = [p for p in puertos if getattr(p, "vid", None) == VID_TEENSY]
    renglones = []
    for p in teensys:
        serie = getattr(p, "serial_number", None) or ""
        if serie == SERIE_ARQUERO:
            que = "ARQUERO"
        elif serie == SERIE_DELANTERO:
            que = "DELANTERO (NO es el arquero)"
        else:
            que = "NO es el arquero"
        renglones.append("Teensy serie %s en %s  ->  %s" % (serie or "?", p.device, que))
    if not teensys:
        renglones.append("No hay ninguna Teensy enchufada (con un programa corriendo).")
    for _ in (en_carga or ()):
        renglones.append("Teensy en MODO CARGA (sin programa corriendo: no se sabe que robot es). "
                         "El cargador le cargaria a ESE. Desenchufalo.")
    renglones.append(AVISO_QUIEN)
    if en_carga is None:
        renglones.append(AVISO_SIN_PNPUTIL)
    series = [getattr(p, "serial_number", None) or "" for p in teensys]
    if SERIE_DELANTERO in series:
        resultado = "DELANTERO"
    elif en_carga:
        resultado = "CARGA"
    elif not teensys:
        resultado = "NINGUNO"
    elif len(teensys) == 1 and series[0] == SERIE_ARQUERO:
        resultado = "ARQUERO"
    elif len(teensys) == 1:
        resultado = "OTRO"
    else:
        resultado = "VARIOS"
    renglones.append("QUIEN=" + resultado)
    return renglones, CODIGOS_QUIEN[resultado]


def main_quien(salida=None, mirar_carga=None):
    salida = salida or sys.stdout
    if list_ports is None:
        print("No tengo pyserial: no puedo mirar los puertos. Usar el Python de PlatformIO.", file=salida)
        print("QUIEN=NOSE", file=salida)
        return CODIGOS_QUIEN["NOSE"]
    try:
        puertos = list(list_ports.comports())
    except Exception as e:
        print("No pude leer la lista de puertos: %s" % e, file=salida)
        print("QUIEN=NOSE", file=salida)
        return CODIGOS_QUIEN["NOSE"]
    renglones, codigo = quien(puertos, (mirar_carga or teensys_en_modo_carga)())
    for r in renglones:
        print(r, file=salida)
    return codigo



# ---------- EL CARGADOR: compilar y cargar desde la ventana (2026-11-08) ----------
#  Pedido del equipo (03/11): "poder cargar todo facil y cambiar todo facil
#  es la prioridad mas grande". Corre en un hilo aparte (compilar tarda ~1
#  min la primera vez) y, como el hilo del cable, NUNCA toca la ventana:
#  todo lo que pasa lo deja en la cola.

# Lo que dice --quien, para la ventana (el código de CODIGOS_QUIEN)
MOTIVO_QUIEN = {3: "¡Ese es el DELANTERO!", 4: "Ese Teensy no es el arquero.",
                5: "No veo ningún robot enchufado por USB.", 6: "Hay más de un Teensy: dejá SOLO el arquero.",
                7: "No pude mirar qué Teensy hay enchufada.",
                8: "Hay un Teensy en MODO CARGA (puede ser el DELANTERO): desenchufalo."}
# En la --autoprueba es True: si alguien se olvidó de darle un Popen de
# mentira, NO se ejecuta nada de verdad (ni pio ni cargar.py).
SIN_PROCESOS = False


class Cargador:
    """Compila y carga UN programa al arquero. Deja en la cola:
        ("carga", "etapa", n, total, texto)   el paso en que va
        ("carga", "linea", texto)              lo que dicen pio / cargar.py
        ("carga", "guardado", info)            ya escribió los valores (con copia)
        ("carga", "revertido", texto)          no compiló: volvió el archivo a como estaba
        ("carga", "puerto_cerrado",)           soltó el USB (la ventana se vuelve a conectar al final)
        ("carga", "fin", resultado)            SIEMPRE, al final (pase lo que pase)
    cerrar_conexion, mirar_carga y listar_puertos vienen de la ventana (la
    autoprueba les pone de mentira, y también a ejecutar)."""

    TOPE_COMPILAR = 300
    TOPE_CARGAR = 240

    def __init__(self, cola, ejecutar=None, cerrar_conexion=None, mirar_carga=None, listar_puertos=None,
                 dormir=None):
        self.cola = cola
        self.ejecutar = ejecutar
        self.cerrar_conexion = cerrar_conexion or (lambda: None)
        self.mirar_carga = mirar_carga or teensys_en_modo_carga
        self.listar_puertos = listar_puertos or (lambda: list(list_ports.comports()) if list_ports else [])
        self.dormir = dormir or time.sleep

    def _dice(self, *msg):
        self.cola.put(("carga",) + msg)

    def _correr(self, orden, tope, cwd):
        """Corre una orden y manda cada renglón a la cola. -> (código,
        renglones, se pasó del tope). Si se pasa del tope: taskkill /T
        (mata también a teensy_loader_cli)."""
        if self.ejecutar is None and SIN_PROCESOS:
            raise RuntimeError("autoprueba: no se ejecuta nada de verdad (%s)" % orden[:2])
        ejecutar = self.ejecutar or subprocess.Popen
        p = ejecutar(orden, **opciones_de_proceso(cwd))
        se_paso = [False]

        def matar():
            se_paso[0] = True
            try:
                subprocess.run(["taskkill", "/T", "/F", "/PID", str(p.pid)], stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            except Exception:
                pass
            try:
                p.kill()
            except Exception:
                pass
        reloj = threading.Timer(tope, matar)
        reloj.daemon = True
        reloj.start()
        renglones = []
        try:
            for crudo in iter(p.stdout.readline, b""):
                t = crudo.decode("utf-8", "replace").rstrip("\r\n").replace("\r", "")
                renglones.append(t)
                self._dice("linea", t)
            codigo = p.wait()
        finally:
            reloj.cancel()
        return codigo, renglones, se_paso[0]

    def _revertir(self, ruta, info):
        """No compiló: vuelve a poner los bytes de la copia, SOLO si el archivo
        sigue siendo el que escribió el tablero."""
        try:
            with open(ruta, "rb") as f:
                if huella_de(f.read()) != info["huella_nueva"]:
                    return False
            with open(info["copia"], "rb") as f:
                antes = f.read()
            escribir_bytes_atomico(ruta, antes)
        except (OSError, NoSeEscribe):
            return False
        self._dice("revertido", "Volví el archivo a como estaba (la copia %s)." % os.path.basename(info["copia"]))
        return True

    def trabajo(self, que, carpeta, guardar=None):
        """que: "partido" o "tablero". guardar: los argumentos de
        guardar_partido (o None: se carga como está)."""
        total = 5 if guardar is not None else 4
        r = {"ok": False, "que": que, "motivo": "", "nivel": "malo", "codigo": None, "lineas": [],
             "cargo": False, "partido": [], "afuera": False, "revertido": False, "a_medias": False}
        try:
            n = 1
            self._dice("etapa", n, total, "Mirando que sea SOLO el arquero...")
            renglones, codigo = quien(self.listar_puertos(), self.mirar_carga())
            for t in renglones:
                self._dice("linea", t)
            if codigo != CODIGOS_QUIEN["ARQUERO"]:
                r.update(motivo=MOTIVO_QUIEN.get(codigo, "No pude confirmar que sea el arquero.") +
                         " No se cargó nada.", codigo=codigo, lineas=renglones)
                return r
            info = None
            if guardar is not None:
                n += 1
                self._dice("etapa", n, total, "Guardando los valores (con copia)...")
                try:
                    info = guardar_partido(**guardar)
                except CambioPorAfuera:
                    r.update(motivo="El programa cambió mientras editabas (¿Arduino IDE?): lo vuelvo a leer. "
                                    "Revisá y apretá de nuevo. No se cargó nada.", afuera=True)
                    return r
                except NoSeEscribe as e:
                    r.update(motivo="%s No se cargó nada." % e)
                    return r
                self._dice("guardado", info)
            n += 1
            self._dice("etapa", n, total, "Compilando (la primera vez ~1 min)...")
            codigo, lineas, tope = self._correr(orden_compilar(carpeta), self.TOPE_COMPILAR, carpeta)
            if codigo != 0 or tope:
                errores = [t for t in lineas if "error" in t.lower()][:8]
                motivo = "NO COMPILÓ: el robot sigue con lo que tenía."
                if info is not None and info.get("copia"):
                    if self._revertir(guardar["ruta"], info):
                        r["revertido"] = True
                        motivo = ("NO COMPILÓ: volví el archivo a como estaba; el robot sigue con lo que tenía. "
                                  "Tus valores quedaron en el editor.")
                    else:
                        motivo = ("NO COMPILÓ, y NO pude volver el archivo a como estaba (¿lo cambió otro "
                                  "programa?). El robot sigue con lo que tenía. Usá 'Volver a como estaba'.")
                r.update(motivo=motivo + (" Se colgó compilando más de 5 min." if tope else ""), codigo=codigo,
                         lineas=errores)
                return r
            n += 1
            self._dice("etapa", n, total, "Soltando el cable de la ventana...")
            self.cerrar_conexion()
            self._dice("puerto_cerrado")
            self.dormir(0.8)
            n += 1
            self._dice("etapa", n, total, "Cargando al robot (~10 s). ¡VA A ARRANCAR SOLO!")
            cwd = os.path.dirname(CARGAR_PY)
            codigo, lineas, tope = self._correr(orden_cargar(carpeta), self.TOPE_CARGAR, cwd)
            if codigo != 0 and not tope and any("OTRA VENTANA ESTA USANDO" in t for t in lineas):
                # Windows a veces tarda en soltar el COM de la ventana: UNA vez más.
                self._dice("linea", "(Windows todavía no soltó el puerto: pruebo una vez más en 2 s)")
                self.dormir(2.0)
                codigo, lineas, tope = self._correr(orden_cargar(carpeta), self.TOPE_CARGAR, cwd)
            ok, motivo, nivel = motivo_de_carga(lineas, codigo, tope)
            r.update(ok=ok, motivo=motivo, nivel=nivel, codigo=codigo, lineas=lineas, cargo=True,
                     partido=lineas_partido_de_la_salida(lineas), a_medias=(not ok and PUEDE_A_MEDIAS in motivo))
            return r
        except Exception:
            r.update(ok=False, nivel="malo",
                     motivo="Falla del tablero (mostrásela al profe; está en el log): " + traceback.format_exc()[-700:])
            return r
        finally:
            self._dice("fin", r)


# =====================================================================
#  5. EL ROBOT DE MENTIRA (--demo): para ver la ventana sin robot
# =====================================================================
#  Contesta como el tablero del ARQUERO, con el mismo idioma (el
#  contrato). TODO lo que "mide" es inventado: sirve para practicar con la
#  ventana, no para ajustar nada. Las mediciones de CANCHA dan los números
#  de las BITÁCORAS (15/09, 06/10, 25/08), así se ve qué tendría que dar.
#  En la demo la cámara (!C) da, en orden: la pelota a 30 cm, y después
#  10, 20, 30, 40 y 50 cm (como si se hiciera la tabla 4b en ese orden).

# Los valores de la demo son LOS DEL PARTIDO 0.1103 (la tabla de arriba).
VALORES_DE_MENTIRA = {f[0]: f[2] for f in TABLA}
# (mínimo, máximo, tipo, cómo se declara en el panel; "" = solo de la prueba)
RANGOS_DE_MENTIRA = {f[0]: (f[3], f[4], f[1], f[6]) for f in TABLA}
TOPES = {"costado": 3000, "despeje": 20000, "ida": 3000}
LINEAS_DE_MENTIRA = {        # 15/09: verde izq 97 / der 81; blanco izq 763 / der 753 -> 425
    "v": "#LIN fase=v n=3920 izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1",
    "i": "#LIN fase=i n=3900 izq=310,763,702.5 der=80,95,88.0 ade=50,60,55.0",
    "d": "#LIN fase=d n=3910 izq=82,96,89.1 der=300,753,690.3 ade=50,61,55.2",
}
ANCHOS_DE_MENTIRA = (        # 06/10: (medioizq, izqder, derizq, desvíos)
    (2180, 4344, 4547, "2.9,3.4,3.6,2.1"),
    (2210, 4405, 4615, "3.1,2.8,3.9,2.4"),
)
FONDOS_DE_MENTIRA = (1730, 1690)
CAMARAS_DE_MENTIRA = (86.9, 28.0, 58.0, 88.9, 117.0, 142.0)    # 30 cm, y la tabla del 25/08
MEDIO_DE_MENTIRA = ((1830, "L"), (400, "T"), (2000, "T"), (943, "T"), (610, "L"), (2228, "T"),
                    (520, "L"), (320, "T"))


def _g1(v):
    """Un decimal, sin '-0.0' (el contrato no lo tiene)."""
    t = "%.1f" % (float(v) + 0.0)
    return "0.0" if t == "-0.0" else t


def tope_de(n, valores):
    """El tope duro de cada prueba (DISENO 4.3), en ms."""
    V = lambda x: int(valor_num(valores, x))
    if n in (1, 2, 3, 4):
        return V("msCostado") + 3000
    if n == 5:
        return 2 * V("msCostado") + 3000
    if n in (6, 7):
        return 20000
    if n in (8, 9):
        return V("msSeguir") + 20000
    return V("msPruebaIda") + 3000


def _plan_de_mentira(n, valores):
    """[(fase, ms)]: lo que 'hace' el robot de mentira en la prueba n."""
    V = lambda x: valor_num(valores, x)
    freno = V("msFreno")
    pausa = [("p", V("MS_ESPERA_ANTES_DE_VOLVER") if V("frenarAntesDeVolver") >= 0.5 else 150)]
    vuelta = [("a", 520 + V("msAdelante")), ("f", freno), ("e", 380)]
    empujon = [("c", V("msEmpujonFinal")), ("f", freno)] if V("empujonFinal") >= 0.5 else []
    if n in (1, 2, 3, 4):
        return [("l", V("msCostado")), ("f", freno), ("q", 500)]
    if n == 5:
        return [("l", 2 * V("msCostado")), ("f", freno), ("q", 500)]
    if n == 6:
        return [("i", V("msAdelante"))] + pausa + vuelta + empujon + [("q", 500)]
    if n == 7:
        chico = [("c", V("msEmpujonFinal")), ("f", freno)] if V("avanzarAlUbicarse") >= 0.5 else []
        return [("a", 905), ("f", freno), ("e", 300)] + chico + [("q", 500)]
    if n == 8:
        return [("s", V("msSeguir")), ("f", freno), ("q", 500)]
    if n == 9:
        return None                       # la 9 depende de los intentos: ver _intentos_de_mentira
    return [("i", V("msPruebaIda"))] + pausa + [("q", 500)]


def _intentos_de_mentira(valores):
    """Los intentos de la 9 (los #MIRE) y el plan. Con los números del
    partido: el 1ro mira y la pelota no viene; el 2do sale en diagonal."""
    V = lambda x: valor_num(valores, x)
    mira = V("medirAntesDeDespejar") >= 0.5
    intentos = []
    plan = []
    if mira:
        plan += [("s", 1760), ("m", V("MS_MAX_MIRANDO") * 0.76)]
        intentos.append(dict(k=1, t=2140, res="NO_VIENE", miro=1, cuadros=4, d=38.2, y=-3.1, vy=0.0,
                             va=2.5, av=0, lat=0, ms=0, ang=0.0, recto=0))
        plan += [("s", 1350), ("m", 380)]
        d, y, vy, va, t = 29.6, -7.4, -10.0, 30.0, 3870
        sale, motivo, av, lat, ms, ang = calcular_diagonal(d, y, vy, va, True, True, valores)
    else:
        plan += [("s", 2600)]
        d, y, vy, va, t = 29.0, -7.0, -6.0, 0.0, 2600
        sale, motivo, av, lat, ms, ang = calcular_diagonal(d, y, vy, 0.0, False, False, valores)
    if sale:
        res = "SALE_MIRANDO" if mira else "SALE_SIN_MIRAR"
        recto = 2 if V("despejeEnDiagonal") < 0.5 else (1 if lat == 0 else 0)
    else:
        res = "MUY_ABIERTO" if motivo == 2 else "NO_ALCANZA"
        recto = 0
        av, lat, ms = 0, 0, 0
        if motivo == 1:
            ang = 0.0                     # con NO_ALCANZA no se llega a calcular
    intentos.append(dict(k=len(intentos) + 1, t=t, res=res, miro=1 if mira else 0, cuadros=4 if mira else 0,
                         d=d, y=y, vy=vy, va=va if mira else 0.0, av=av, lat=lat, ms=ms, ang=ang, recto=recto))
    if sale:
        freno = V("msFreno")
        plan += [("i", ms)]
        plan += [("p", V("MS_ESPERA_ANTES_DE_VOLVER") if V("frenarAntesDeVolver") >= 0.5 else 150)]
        plan += [("a", 520 + ms), ("f", freno), ("e", 380)]
        if V("empujonFinal") >= 0.5:
            plan += [("c", V("msEmpujonFinal")), ("f", freno)]
        plan += [("q", 500)]
    else:
        ya = sum(p[1] for p in plan)
        plan += [("s", max(0.0, V("msSeguir") - ya)), ("f", V("msFreno")), ("q", 500)]
    return plan, intentos, bool(sale)


def linea_mire(i):
    return ("#MIRE k=%d t=%d res=%s miro=%d cuadros=%d d=%s y=%s vy=%s va=%s av=%d lat=%d ms=%d ang=%s recto=%d"
            % (i["k"], i["t"], i["res"], i["miro"], i["cuadros"], _g1(i["d"]), _g1(i["y"]), _g1(i["vy"]),
               _g1(i["va"]), i["av"], i["lat"], i["ms"], _g1(i["ang"]), i["recto"]))


def consejos_del_contrato(n, inf, intentos, valores, tope=None):
    """Los consejos de DISENO 4.5, con los textos exactos (sin tildes,
    como los imprime el robot). Los usa el robot de mentira."""
    V = lambda x: valores.get(x, VALORES_DE_MENTIRA.get(x, "?"))
    c = []
    como, nota = inf["como"], inf["nota"]
    if como == "TECLA":
        c.append("Lo frenaron antes de terminar: este resultado no vale.")
    elif como == "TOPE":
        c.append("Se corto por el tope de tiempo (%d ms): no deberia pasar. Avisar." % (tope or 0))
    if nota == "NO_VALE" and (inf["racha"] >= 3 or inf["mudas"] > 0):
        c.append("El giroscopio no contesto (%d lecturas mudas, la peor racha %d seguidas): bateria prendida "
                 "ANTES del USB, y el cable del giroscopio (29/09)." % (inf["mudas"], inf["racha"]))
    if inf["hueco"] > 40:
        c.append("Una vuelta del programa tardo %d ms (a los %d ms): eso es un RETARDO y con eso zigzaguea "
                 "igual, toquen lo que toquen. Avisar antes de cambiar numeros." % (inf["hueco"], inf["huecoms"]))
    pico_rel = inf["picoms"] - (inf["idams_ini"] if n == 9 else 0)
    if nota == "PROLIJO":
        c.append("Nada que tocar: anotar los numeros en la bitacora. Probar tambien para el otro lado.")
    elif nota == "CASI":
        if pico_rel < 300 and n in (1, 2, 3, 4, 5, 8):
            c.append("La peor torcida fue al ARRANCAR: bajar RAMPA_MOV_PASO de a 1 (ahora %s). OJO: cambia "
                     "msAdelante y MS_MEDIO_AL_MEDIO (hay que medirlos de nuevo)." % V("RAMPA_MOV_PASO"))
        elif abs(inf["medio"]) > 2:
            c.append("Queda torcido siempre para el mismo lado: subir KI_RUMBO de a 0.25 (ahora %s)." % V("KI_RUMBO"))
        else:
            c.append("Esta cerca. Repetir 2 o 3 veces sin tocar nada: si da parecido, es asi.")
    elif nota == "ZIGZAGUEA":
        c.append("Corrige de mas y se pasa: bajar KP_RUMBO de a 0.5 (ahora %s). Si sigue, bajar KI_RUMBO de "
                 "a 0.25 (ahora %s)." % (V("KP_RUMBO"), V("KI_RUMBO")))
    elif nota == "SE_TUERCE":
        if inf["tope"] > 20:
            c.append("La correccion estuvo al tope el %d%% del tiempo y no alcanzo: subir MAX_CORRECCION_RUMBO "
                     "de a 10 (ahora %s), o ir mas despacio." % (inf["tope"], V("MAX_CORRECCION_RUMBO")))
        elif n in (6, 9) and inf["idams"] <= inf["picoms"] < inf["atrasms"]:
            if valor_num(valores, "frenarAntesDeVolver") < 0.5:
                c.append("Se torcio en la PAUSA, frenado al terminar la ida (ahi no corrige): probar "
                         "frenarAntesDeVolver = 1")
            else:
                c.append("Se torcio en la PAUSA, frenado al terminar la ida (ahi no corrige): probar "
                         "MS_ESPERA_ANTES_DE_VOLVER de a 25 (ahora %s)." % V("MS_ESPERA_ANTES_DE_VOLVER"))
        elif n in (6, 9) and inf["atrasms"] <= inf["picoms"] < inf["atrasms"] + 300:
            c.append("Se torcio al ARRANCAR PARA ATRAS: el partido no reinicia la rampa ahi. No hay numero "
                     "para eso: anotarlo en la bitacora.")
        elif inf.get("pico_fase") == "f":
            c.append("Se tuerce al FRENAR: el freno es igual al del partido, no hay numero para tocar.")
        else:
            c.append("Corrige flojo y se le escapa: subir KP_RUMBO de a 0.5 (ahora %s)." % V("KP_RUMBO"))
    if n in (1, 2, 3, 4, 5) and abs(inf["corr"]) >= 15:
        c.append("Corrigio en promedio %d todo el tiempo: de costado tiende a girar solo. Correr el MISMO "
                 "movimiento para el otro lado: si la correccion cambia de signo es la mezcla (LADO_TRASERA 2 "
                 "mas o 2 menos, ahora %s; OJO: cambia MS_MEDIO_AL_MEDIO); si tiene el mismo signo, es un "
                 "motor que tira mas (KI_RUMBO)." % (inf["corr"], V("LADO_TRASERA")))
    if n in (6, 7, 9) and (como == "SIN_LINEA" or (inf["atrasms"] > 0 and inf["lineams"] == 0)):
        c.append("No vio la linea al volver: freno por tiempo (1200 ms). Mirar umbralBlanco (CANCHA, paso 1) y "
                 "que haya linea atras.")
    if inf["acomodo"] == 3:
        c.append("No se pudo enderezar en 3 intentos: mirar la bateria y el piso.")
    if n in (8, 9) and inf["idas"] >= 3 and inf["velmedia"] < 4:
        c.append("VA Y VIENE con la pelota casi quieta. De a uno: subir ZONA_MUERTA_PELOTA de a 0.2 (ahora %s), "
                 "o probar predecirTrayectoria = 0, o bajar pwmMinLateral de a 5 (ahora %s)."
                 % (V("ZONA_MUERTA_PELOTA"), V("pwmMinLateral")))
    if n in (8, 9) and inf["conpelota"] < 50:
        c.append("Vio la pelota solo el %d%% del tiempo: ponerla de frente, a menos de 60 cm." % inf["conpelota"])
    if n == 9:
        cuenta = {}
        for i in intentos:
            cuenta[i["res"]] = cuenta.get(i["res"], 0) + 1
        if cuenta.get("MUY_ABIERTO"):
            c.append("%d veces no salio por MUY ABIERTO (tenia que salir a mas de %s grados). La perilla es "
                     "MAX_ANGULO_DIAGONAL (ahora %s), de a 5. OJO: salir abierto y no llegar es peor que no salir."
                     % (cuenta["MUY_ABIERTO"], V("MAX_ANGULO_DIAGONAL").split(".")[0], V("MAX_ANGULO_DIAGONAL")))
        if cuenta.get("NO_ALCANZA"):
            c.append("%d veces no salio porque el avance no alcanzaba (muy al costado o muy rapida). No se "
                     "arregla con rendimientoCostado: ese numero tiene que ser el de verdad. Anotarlo."
                     % cuenta["NO_ALCANZA"])
        if cuenta.get("NO_VIENE"):
            c.append("%d veces miro y la pelota no venia (o no llegaba a umbralCm a tiempo). Con la pelota quieta "
                     "lejos es normal. Si pasa con pelotas que SI vienen: subir MS_MAX_MIRANDO de a 50 (ahora %s)."
                     % (cuenta["NO_VIENE"], V("MS_MAX_MIRANDO")))
        if cuenta.get("POCOS_CUADROS"):
            c.append("%d veces no junto 3 cuadros a tiempo: la camara manda pocos (mirar q en SENSORES). Revisar "
                     "la camara antes de tocar MS_MIRAR." % cuenta["POCOS_CUADROS"])
        salidos = [i for i in intentos if i["res"] in ("SALE_MIRANDO", "SALE_SIN_MIRAR")]
        if any(i["recto"] == 3 for i in salidos):
            c.append("Salio DERECHO porque el giroscopio no contestaba.")
        if any(i["recto"] == 2 for i in salidos):
            c.append("Salio DERECHO porque despejeEnDiagonal esta apagado.")
        if inf["sale"]:
            c.append("Si NO LLEGO de costado (la pelota le quedo mas afuera): BAJAR rendimientoCostado de a 0.05 "
                     "(ahora %.2f). Si se PASO de costado: SUBIRLO. Va al reves: mas chico = MAS costado."
                     % valor_num(valores, "rendimientoCostado"))
        if nota == "NO_SALIO":
            c.append("En msSeguir (%s ms) no salio a despejar. Mirar los intentos: dicen por que." % V("msSeguir"))
    if n == 10:
        c.append("Medir con regla desde la cinta hasta el frente del chasis y anotarlo en CANCHA (paso 5).")
    return c


def linea_usados(valores):
    return " ".join(["#USADOS"] + ["%s=%s" % (k, v) for k, v in valores.items()
                                   if k in VALORES_DE_MENTIRA and abs(float(v) - float(VALORES_DE_MENTIRA[k])) > 0.001])


def demo_resultado(n, valores, rng, cortado_ms=None, por_tope=False):
    """El robot de mentira 'mide' el movimiento n. Devuelve la TANDA con el
    formato exacto del contrato: #M..., (#MIRE...), #R, #USADOS,
    #CONSEJO..., el RESULTADO para personas y #FIN n (sin el #FRENO).
    cortado_ms = a los cuántos ms lo frenaron (None = terminó solo)."""
    V = lambda x: valor_num(valores, x)
    mov = MOV_POR_NUMERO[n]
    intentos, sale = [], False
    if n == 9:
        plan, intentos, sale = _intentos_de_mentira(valores)
    else:
        plan = _plan_de_mentira(n, valores)
    tramos = []                           # (desde, hasta, fase)
    t = 0.0
    for fase, ms in plan:
        tramos.append((t, t + ms, fase))
        t += ms
    total = int(t)
    paso_ms = 20 if n in (8, 9) else 10
    # Cuánto se tuerce (inventado, pero responde a los números):
    kp, ki, tope_c = V("KP_RUMBO"), V("KI_RUMBO"), V("MAX_CORRECCION_RUMBO")
    sesgo = (V("LADO_TRASERA") - 89) * 0.9 if mov["tipo"] == "costado" else 1.2
    sesgo += (1.5 - ki) * 1.4
    if n in (3, 4, 5) and tope_c < 70:
        sesgo += (70 - tope_c) * 0.25    # rápido y con poca corrección: se le escapa
    osc = max(0.3, 0.6 + max(0.0, kp - 6.0) * 1.3)
    arranque = max(0.0, (V("RAMPA_MOV_PASO") - 12) * 0.8)
    inicio_ida = next((a for a, _, f in tramos if f == "i"), 0.0)
    muestras = []
    for k in range(0, total + 1, paso_ms):
        fase = next((f for a, b, f in tramos if a <= k < b), tramos[-1][2] if tramos else "q")
        desde = next((a for a, b, f in tramos if a <= k < b), 0.0)
        rel = k - desde
        if fase in ("l", "i", "a", "c", "v", "s"):
            g = sesgo * (1 - math.exp(-rel / 180.0)) + osc * 2.2 * math.sin(k / 55.0)
            g += arranque * math.exp(-rel / 120.0)
        elif fase == "m":
            g = 0.6 * math.sin(k / 90.0)
        elif fase == "q":
            g = sesgo * 0.35
        else:                             # f, p, e
            g = sesgo * 0.6 + 0.4 * math.sin(k / 70.0)
        muestras.append((k, g + rng.uniform(-0.3, 0.3), fase))
    if cortado_ms is not None:
        muestras = [p for p in muestras if p[0] <= cortado_ms]
        total = max(int(cortado_ms), 0)
    muestras = muestras[:1200]            # el robot guarda hasta 1200
    moviendo = ("l", "s", "i", "a", "c", "v")
    desde_nota = inicio_ida if n == 9 else 0.0
    cuenta = [p for p in muestras if p[0] >= desde_nota]
    en_mov = [p for p in cuenta if p[2] in moviendo]
    inf = dict.fromkeys(CAMPOS_R, 0)
    inf.update(n=n, mov=mov["nombre"], como="NORMAL")
    if cuenta:
        pico = max(cuenta, key=lambda p: abs(p[1]))
        inf["pico"], inf["picoms"], inf["pico_fase"] = round(pico[1], 1), pico[0], pico[2]
        inf["cruces"] = contar_cruces([p[1] for p in cuenta])
    quietos = [p[1] for p in muestras if p[2] == "q"]
    if quietos:
        ultimos = [p[1] for p in muestras if p[2] == "q" and p[0] >= muestras[-1][0] - 150]
        inf["fin"] = round(sum(ultimos) / len(ultimos), 1)
    if en_mov:
        inf["medio"] = round(sum(p[1] for p in en_mov) / len(en_mov), 1)
        inf["fuera3"] = int(round(100.0 * sum(1 for p in en_mov if abs(p[1]) > 3) / len(en_mov)))
        corr = [max(-tope_c, min(tope_c, -kp * p[1] * 2.0)) for p in en_mov]
        inf["corr"] = int(round(sum(corr) / len(corr)))
        inf["tope"] = int(round(100.0 * sum(1 for x in corr if abs(x) >= tope_c) / len(corr)))
    inf["hueco"], inf["huecoms"] = rng.randint(8, 13), rng.randint(0, max(total, 1))
    inf["duro"] = total
    fin_ida = next((b for a, b, f in tramos if f == "i"), 0.0)
    ini_atras = next((a for a, b, f in tramos if f == "a"), 0.0)
    dur_atras = next((b - a for a, b, f in tramos if f == "a"), 0.0)
    if n in (6, 9, 10):
        inf["idams"] = int(fin_ida) if fin_ida else 0
        inf["atrasms"] = int(ini_atras) if n != 10 else 0
    if n == 7:
        inf["atrasms"] = 0
    if n in (6, 7) or (n == 9 and sale):
        inf["lineams"] = int(dur_atras)
        inf["acomodo"] = rng.choice((0, 0, 1))
    inf["idams_ini"] = int(inicio_ida)
    if n in (8, 9):
        inf.update(idas=rng.randint(1, 3), arranques=rng.randint(2, 4), conpelota=rng.randint(84, 95),
                   zonamuerta=rng.randint(28, 45), velmedia=round(rng.uniform(8.0, 12.5), 1))
    if n == 9:
        inf["intentos"] = len(intentos)
        inf["sale"] = 1 if sale else 0
        if not sale:
            inf["como"] = "NO_SALIO"
    if cortado_ms is not None:
        inf["como"] = "TOPE" if por_tope else "TECLA"
        if n == 9:
            intentos = [i for i in intentos if i["t"] <= cortado_ms]
            inf["intentos"] = len(intentos)
            inf["sale"] = 1 if (sale and cortado_ms >= inicio_ida) else 0
    inf["nota"] = calcular_nota(inf["como"], inf["racha"], inf["mudas"], len(muestras), len(en_mov), n,
                                inf["sale"], inf["pico"], inf["cruces"], inf["fin"])
    renglones = ["#M %d %s %s" % (t_, _g1(g), f) for t_, g, f in muestras]
    renglones += [linea_mire(i) for i in intentos]
    renglones.append(
        "#R n=%d mov=%s nota=%s como=%s pico=%s picoms=%d fin=%s medio=%s cruces=%d fuera3=%d corr=%d tope=%d "
        "mudas=%d racha=%d hueco=%d huecoms=%d duro=%d lineams=%d idams=%d atrasms=%d acomodo=%d laterales=%d "
        "idas=%d arranques=%d conpelota=%d zonamuerta=%d velmedia=%s intentos=%d sale=%d" % (
            n, mov["nombre"], inf["nota"], inf["como"], _g1(inf["pico"]), inf["picoms"], _g1(inf["fin"]),
            _g1(inf["medio"]), inf["cruces"], inf["fuera3"], inf["corr"], inf["tope"], inf["mudas"],
            inf["racha"], inf["hueco"], inf["huecoms"], inf["duro"], inf["lineams"], inf["idams"],
            inf["atrasms"], inf["acomodo"], inf["laterales"], inf["idas"], inf["arranques"], inf["conpelota"],
            inf["zonamuerta"], _g1(inf["velmedia"]), inf["intentos"], inf["sale"]))
    renglones.append(linea_usados(valores))
    renglones += ["#CONSEJO " + c for c in consejos_del_contrato(n, inf, intentos, valores, tope_de(n, valores))]
    renglones.append("RESULTADO: %d %s -> %s  pico %+.1f grados a los %d ms, final %+.1f, cruces %d, duro %d ms, "
                     "vuelta mas larga %d ms" % (n, mov["nombre"], inf["nota"], inf["pico"], inf["picoms"],
                                                 inf["fin"], inf["cruces"], inf["duro"], inf["hueco"]))
    renglones.append("Grados: NEGATIVO = a la DERECHA, POSITIVO = a la IZQUIERDA.   ? = ayuda")
    renglones.append("#FIN %d" % n)
    return renglones


def linea_ancho(medioizq, izqder, derizq, desvio, ok=1, error=0, valores=None):
    vel, rampa = FIJOS["VEL_MEDIO_MEDIR"], int(valor_num(valores, "RAMPA_MOV_PASO"))
    al_izq = ms_hasta_el_medio(izqder, vel, rampa) if izqder else 0
    al_der = ms_hasta_el_medio(derizq, vel, rampa) if derizq else 0
    return ("#ANCHO ok=%d error=%d medioizq=%d izqder=%d derizq=%d almedioizq=%d almedioder=%d desvio=%s "
            "endereza=0 golpe=2.4 vel=%d rampa=%d umbral=%d calgiro=3" % (
                ok, error, medioizq, izqder, derizq, al_izq, al_der, desvio, vel, rampa,
                int(valor_num(valores, "umbralBlanco"))))


class RobotDeMentira:
    """Se hace pasar por la Conexion: misma forma de usarlo.
    escala_tiempo < 1 lo hace correr más rápido (la autoprueba usa 0.001)."""

    def __init__(self, cola, semilla=2026, escala_tiempo=1.0):
        self.cola = cola
        self.valores = dict(VALORES_DE_MENTIRA)
        self.escala = escala_tiempo
        self._entrada = queue.Queue()
        self._seguir = True
        self._rng = random.Random(semilla)
        self._t0 = time.monotonic()
        self.enchufado = True
        self.forzar_linea = None          # None: cada 7 s pisa blanco un rato; True/False: siempre/nunca
        self.guardado = {"mov": None, "cancha": None}
        self._t_freno = -1e9
        self._corridas = {"A": 0, "F": 0, "C": 0}
        self._pelota = None               # (Xp, Yp, t) crudos
        self._prev = None                 # para las velocidades
        self._vy = self._va = 0.0
        self._n = 0

    # ---- la misma cara que Conexion ----
    def arrancar(self):
        threading.Thread(target=self._correr, name="robot-de-mentira", daemon=True).start()

    def enviar(self, texto):
        if not self.enchufado:
            return False                  # sin cable no llega nada
        self._entrada.put(texto)
        return True

    def enviar_ya(self, texto):
        return self.enviar(texto)

    def cerrar(self, esperar=False):
        self._seguir = False

    # ---- por dentro ----
    def _decir(self, renglon):
        if self.enchufado:                # sin nadie escuchando, el robot no imprime
            self.cola.put(("linea", renglon))

    def _ms(self):
        return int((time.monotonic() - self._t0) * 1000 / max(self.escala, 1e-6))

    def _correr(self):
        time.sleep(0.4)
        self.cola.put(("conectado", "DEMO", SERIE_ARQUERO))
        self.saludo_de_arranque()
        proximo = 0.0
        while self._seguir:
            try:
                orden = self._entrada.get(timeout=0.02)
            except queue.Empty:
                orden = None
            if orden is not None:
                self.comando(orden)
            if time.monotonic() >= proximo and self.enchufado:
                self._decir(self.linea_S())
                proximo = time.monotonic() + 0.1

    def saludo_de_arranque(self):
        self._decir(self.linea_hola())
        self._decir("=== TABLERO DEL ARQUERO %s (copias del partido %s) - NO ES EL DE PARTIDO ===" % (
            FECHA_CONTRATO, PARTIDO_COPIADO))
        self._decir("(ROBOT DE MENTIRA: todo lo que mide es inventado)")
        self._decir("Se mueve SOLO al pedirlo, despues de la cuenta. Cualquier tecla lo frena.")
        self._decir("#GIRO buscando")
        self._decir("#GIRO ok=1 estado=5")
        self._decir(self.linea_V())
        self._decir(self.linea_DEF())
        for r in self.lineas_guardado():
            self._decir(r)

    def linea_hola(self):
        return "#HOLA %s %s serie=%s partido=%s" % (PROGRAMA, FECHA_CONTRATO, SERIE_ARQUERO, PARTIDO_COPIADO)

    def linea_V(self):
        return "#V " + " ".join("%s=%s" % (k, v) for k, v in self.valores.items())

    def linea_DEF(self):
        return "#DEF " + " ".join("%s=%s" % (k, v) for k, v in VALORES_DE_MENTIRA.items())

    def lineas_guardado(self):
        salida = []
        for que in ("mov", "cancha"):
            g = self.guardado[que]
            if g is None:
                salida.append("#GUARDADO que=%s hay=0" % que)
            else:
                salida.append("#GUARDADO que=%s hay=1 n=%s nombre=%s nota=%s sincable=%d nuevo=%d mismo=1 hace=%d"
                              % (que, g["codigo"], g["nombre"], g["nota"], g["sincable"], g["nuevo"],
                                 max(0, self._ms() - g["t"])))
        return salida

    def pisando(self):
        if self.forzar_linea is not None:
            return bool(self.forzar_linea)
        return self._ms() % 7000 < 1200

    def linea_S(self):
        rng = self._rng
        t = self._ms()
        l = [88 + rng.randint(-6, 6), 92 + rng.randint(-6, 6), 61 + rng.randint(-5, 5)]
        if self.pisando():
            l[0] = 610 + rng.randint(-20, 20)        # el de atrás IZQUIERDO pisa blanco un rato
        rumbo = (90 + 25 * math.sin(t / 6000.0)) % 360
        cpc = valor_num(self.valores, "CAMARA_POR_CM")
        # La pelota va y viene delante del arco; cada 12 s se "esconde" 2,5 s.
        x_cm = 42 + 22 * math.sin(t / 3700.0)
        y_cm = 13 * math.sin(t / 2300.0)                  # + = a la DERECHA
        if t % 12000 > 2500:
            yp = -int(round(y_cm * cpc)) if valor_num(self.valores, "camaraYInvertida") >= 0.5 else int(round(y_cm * cpc))
            self._pelota = (int(round(x_cm * cpc)), yp, t - rng.randint(0, 40))
        ve = self._pelota is not None and t - self._pelota[2] < 500
        if self._pelota is None:
            p, d, y = "0,0,-1", -1.0, 0.0
        else:
            xp, yp, tp = self._pelota
            p = "%d,%d,%d" % (xp, yp, max(0, t - tp))
            d = xp / cpc
            y = (-yp if valor_num(self.valores, "camaraYInvertida") >= 0.5 else yp) / cpc
        if ve and self._prev is not None and t > self._prev[2]:
            dt = (t - self._prev[2]) / 1000.0
            self._vy = 0.7 * self._vy + 0.3 * (y - self._prev[1]) / dt
            self._va = 0.7 * self._va + 0.3 * (self._prev[0] - d) / dt
        elif not ve:
            self._vy = self._va = 0.0
        self._prev = (d, y, t) if ve else None
        mira = valor_num(self.valores, "medirAntesDeDespejar") >= 0.5
        disparo = valor_num(self.valores, "distanciaParaMirar") if mira else valor_num(self.valores, "umbralCm")
        if ve and d <= disparo and abs(y) <= valor_num(self.valores, "umbralDesvio"):
            self._n = min(3, self._n + 1)
        else:
            self._n = 0
        if ve:
            dg = calcular_diagonal(d, y, self._vy, 0.0, False, True, self.valores)[:5]
        else:
            dg = (1, 0, int(valor_num(self.valores, "potenciaDespeje")), 0, int(valor_num(self.valores, "msAdelante")))
        am = "150,%d,%d" % (-20 + rng.randint(-1, 1), rng.randint(0, 60)) if t % 9000 < 6000 else "0,0,-1"
        return ("#S t=%d l=%d,%d,%d u=%s r=%.1f g=5 cal=3,3,0,0 p=%s d=%s y=%s vy=%s va=%s ve=%d n=%d "
                "dg=%d,%d,%d,%d,%d am=%s az=0,0,-1 q=%d e=ESPERA"
                % (t, l[0], l[1], l[2], self.valores["umbralBlanco"], rumbo, p, _g1(d), _g1(y), _g1(self._vy),
                   _g1(self._va), 1 if ve else 0, self._n, dg[0], dg[1], dg[2], dg[3], dg[4], am,
                   24 + rng.randint(-2, 2)))

    def comando(self, orden):
        c = orden.strip()
        if not c:
            return
        if len(c) > LARGO_MAX_ORDEN:
            self._decir("#ERROR orden demasiado larga")
        elif c.startswith("="):
            self._cambiar(c[1:])
        elif c.startswith("!"):
            self._orden_de_mover(c[1:].strip())
        elif c in ("h", "H"):
            self._decir(self.linea_hola())
            for r in self.lineas_guardado():
                self._decir(r)
        elif c == "?":
            self._decir(self.linea_V())
            self._decir(self.linea_DEF())
            self._decir("------------------- TABLERO DEL ARQUERO (de mentira) -------------------")
            self._decir("!1 a !10 = un movimiento (con c al final: SIN CABLE)   !L v|i|d|c !A !F !C !I = cancha")
            self._decir("x = parar   =NOMBRE valor = cambiar un numero   v = panel   0 = los del partido")
            self._decir("h = saludo   g = buscar el giroscopio   r / rc = repetir el ultimo resultado")
        elif c == "x":
            self._decir("PARADO (frena 0,4 s y suelta las ruedas)")
        elif c == "v":
            self._decir("--- PARA COPIAR AL PANEL DE CONTROL (funciona/seguir-y-despejar) ---")
            self._decir("Buscar cada nombre (Ctrl+F) y cambiar SOLO el numero.")
            cambiados = [k for k, v in self.valores.items()
                         if abs(float(v) - float(VALORES_DE_MENTIRA[k])) > 0.001]
            del_panel = [k for k in cambiados if RANGOS_DE_MENTIRA[k][3]]
            if not del_panel:
                self._decir("(nada para copiar: estan todos como en el partido)")
            for k in del_panel:
                v = self.valores[k]
                if RANGOS_DE_MENTIRA[k][2] == "b":
                    v = "true" if float(v) >= 0.5 else "false"
                self._decir("#PANEL %s %s = %s;" % (RANGOS_DE_MENTIRA[k][3], k, v))
            solo = [k for k in cambiados if not RANGOS_DE_MENTIRA[k][3]]
            if solo:
                self._decir("(no se copian porque son solo de esta prueba: %s)" % " ".join(solo))
            self._decir("#FINPANEL")
        elif c == "0":
            self.valores = dict(VALORES_DE_MENTIRA)
            self._decir("Volvieron los numeros del partido (las copias de la %s)." % PARTIDO_COPIADO)
            self._decir(self.linea_V())
        elif c == "g":
            self._decir("#GIRO buscando")
            self._esperar(0.5)
            self._decir("#GIRO ok=1 estado=5")
        elif c in ("r", "rc"):
            self._repetir("mov" if c == "r" else "cancha")
        else:
            self._decir("#ERROR no entiendo '%s'. ? = ayuda" % c)

    def _cambiar(self, texto):
        t = texto.replace("\t", " ").strip()
        nombre, _, valor = t.replace("=", " ", 1).partition(" ")
        nombre, valor = nombre.strip(), valor.strip()
        if not nombre:
            self._decir("#ERROR falta el nombre. Se escribe: =NOMBRE valor   (ej: =umbralCm 32)")
            return
        reales = [k for k in self.valores if k.lower() == nombre.lower()]
        if not reales:
            self._decir("#ERROR no existe el numero %s. Con ? salen todos los nombres" % nombre)
            return
        nombre = reales[0]
        minimo, maximo, tipo, _ = RANGOS_DE_MENTIRA[nombre]
        if not valor:
            self._decir("#ERROR falta el valor de %s. Se escribe: =NOMBRE valor" % nombre)
            return
        if "," in valor:
            self._decir("#ERROR %s: usar punto decimal (4.5, no 4,5)" % nombre)
            return
        try:
            v = _numero(valor)
        except ValueError:
            self._decir("#ERROR %s: no es un numero: %s" % (nombre, valor))
            return
        if tipo == "b":
            if v not in (0, 1):
                self._decir("#ERROR %s es si o no: va 0 o 1" % nombre)
                return
        elif tipo != "f" and v != int(v):
            self._decir("#ERROR %s va sin decimales" % nombre)
            return
        if tipo == "f":
            v = round(v, 2)
        if not minimo - 1e-6 <= v <= maximo + 1e-6:
            self._decir("#ERROR %s fuera de rango: va de %s a %s" % (
                nombre, formatear_valor(minimo, tipo), formatear_valor(maximo, tipo)))
            return
        self.valores[nombre] = formatear_valor(v, tipo)
        self._decir("#OK %s=%s" % (nombre, self.valores[nombre]))
        self._decir(self.linea_V())

    def _esperar(self, segundos):
        """Espera (con el reloj de la demo). Si llega cualquier cosa (menos
        un Enter suelto), lo frenan: devuelve (a los cuántos ms, qué llegó).
        Si no, None."""
        t0 = time.monotonic()
        fin = t0 + segundos * self.escala
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
            return int((time.monotonic() - t0) * 1000 / max(self.escala, 1e-6)), orden.strip()
        return 0, ""

    def _orden_de_mover(self, resto):
        if not resto:
            self._decir("#ERROR falta el numero del movimiento: !1 a !10, o !L !A !F !C !I")
            return
        m = re.match(r"^(\d+|[LAFCIlafci])\s*(.*)$", resto)
        if not m:
            self._decir("#ERROR no hay movimiento %s: son del !1 al !10, y los de cancha !L !A !F !C !I" % resto)
            return
        codigo, arg = m.group(1).upper(), m.group(2).strip().lower()
        if codigo.isdigit() and not 1 <= int(codigo) <= 10:
            self._decir("#ERROR no hay movimiento %s: son del !1 al !10, y los de cancha !L !A !F !C !I" % codigo)
            return
        if codigo == "L":
            if arg not in ("v", "i", "d", "c"):
                self._decir("#ERROR !L necesita la fase: !L v (verde), !L i (blanco izq), !L d (blanco der) "
                            "o !L c (sin cable)")
                return
        elif arg not in ("", "c"):
            self._decir("#ERROR no entiendo '!%s'. ? = ayuda" % resto)
            return
        sincable = arg == "c"
        # Los chequeos de antes de arrancar (DISENO 4.3), en orden
        if (time.monotonic() - self._t_freno) < 1.5 * self.escala:
            self._decir("#FRENO recien parado")
            self._decir("lo acaban de frenar: espera un segundo y proba de nuevo")
            return
        mira_linea = codigo in ("A", "I", "F") or (codigo.isdigit() and int(codigo) in MIRAN_LA_LINEA)
        if mira_linea and self.pisando():
            self._decir("#FRENO sobre la linea")
            self._decir("un sensor de atras esta sobre una linea blanca: corranlo ~10 cm para adelante")
            return
        if codigo.isdigit():
            self._mover(int(codigo), sincable)
        else:
            self._medir(codigo, arg, sincable)

    def _cuenta(self, titulo, sincable):
        """La cuenta: 3 s con cable (cada 0,5 s), 10 s sin cable (cada 1 s).
        Sin cable, a los 2 s 'se desenchufa'. -> True si se cumplió."""
        if sincable:
            self._decir(">>> %s SIN CABLE: arranca en 10 s. DESENCHUFEN EL USB. Al terminar, el LED dice la nota."
                        % titulo)
            marcas, paso = [10.0 - k for k in range(10)], 1.0
        else:
            self._decir(">>> %s en 3 s. Saca la mano. Cualquier tecla lo frena." % titulo)
            marcas, paso = [3.0 - 0.5 * k for k in range(6)], 0.5
        for k, s in enumerate(marcas):
            self._decir("#CUENTA %.1f %s" % (s, "sincable" if sincable else "cable"))
            if sincable and k == 2 and self.enchufado:
                self.enchufado = False                       # lo esperado: se desenchufa
                self.cola.put(("desconectado", "DEMO"))
            if self._esperar(paso) is not None:
                self._t_freno = time.monotonic()
                self._decir("#FRENO tecla")
                self._decir("cancelado: no se movio")
                return False
        return True

    def _terminar(self, que, codigo, nombre, nota, sincable, tanda, freno=None):
        """Guarda (siempre) y lo manda si hay alguien escuchando. Si se
        desenchufó, 'se vuelve a enchufar' sin apagar la batería."""
        estaba = self.enchufado
        if freno and estaba:
            self._decir(freno)
        if estaba:
            for r in tanda:
                self._decir(r)
        # Como tablero.ino (2026-11-03): una prueba SIN CABLE queda nuevo=1
        # aunque la tanda haya salido (el Teensy no sabe si lo desenchufaron).
        self.guardado[que] = dict(codigo=codigo, nombre=nombre, nota=nota, sincable=1 if sincable else 0,
                                  nuevo=0 if (estaba and not sincable) else 1, tanda=list(tanda), t=self._ms())
        if not estaba:
            self.enchufado = True
            self.cola.put(("conectado", "DEMO", SERIE_ARQUERO))
            self._decir(self.linea_hola())
            for r in self.lineas_guardado():
                self._decir(r)

    def _repetir(self, que):
        g = self.guardado[que]
        if g is None:
            self._decir("#ERROR no hay nada guardado")
            return
        self._decir("#REPITE %s %s sincable=%d apagado=0" % (g["codigo"], g["nombre"], g["sincable"]))
        for r in g["tanda"]:
            self._decir(r)
        g["nuevo"] = 0

    def _mover(self, n, sincable):
        mov = MOV_POR_NUMERO[n]
        if not self._cuenta("%d %s" % (n, mov["nombre"]), sincable):
            return
        tope = tope_de(n, self.valores)
        self._decir("#INICIO %d %s tope=%d sincable=%d" % (n, mov["nombre"], tope, 1 if sincable else 0))
        completa = demo_resultado(n, self.valores, random.Random(self._rng.random()))
        duro = 0
        for r in completa:
            if r.startswith("#R "):
                duro = interpretar(r).get("duro") or 0
        cortado = self._esperar(duro / 1000.0) if self.enchufado else None
        if cortado is not None:
            self._t_freno = time.monotonic()
            tanda = demo_resultado(n, self.valores, random.Random(1), cortado[0])
            freno = "#FRENO tecla"
        else:
            if not self.enchufado:
                self._esperar(duro / 1000.0)               # se mueve solo, sin cable
            tanda, freno = completa, None
        nota = next((interpretar(r)["nota"] for r in tanda if r.startswith("#R ")), "NO_VALE")
        self._terminar("mov", str(n), mov["nombre"], nota, sincable, tanda, freno)

    def _medir(self, codigo, arg, sincable):
        """Las mediciones de CANCHA, con los números de las bitácoras."""
        V = lambda x: valor_num(self.valores, x)
        if codigo == "L":
            sincable = arg == "c"
            nombre = {"v": "LINEA_VERDE", "i": "LINEA_BLANCO_IZQ", "d": "LINEA_BLANCO_DER", "c": "LINEA_GUIADA"}[arg]
            if sincable and not self._cuenta("L %s" % nombre, True):
                return
            self._decir("#INICIO L %s tope=%d sincable=%d" % (nombre, 50000 if sincable else 5000, 1 if sincable else 0))
            fases = ("v", "i", "d") if arg == "c" else (arg,)
            self._esperar((12.0 * len(fases)) if sincable else 4.0)
            tanda = [LINEAS_DE_MENTIRA[f] for f in fases]
            tanda.append("LINEA: %s medido (%s)" % (nombre, ", ".join(fases)))
            tanda.append("#FIN L")
            self._terminar("cancha", "L", nombre, "OK", sincable, tanda)
        elif codigo == "C":
            if sincable and not self._cuenta("C CAMARA", True):
                return
            self._decir("#INICIO C CAMARA tope=4000 sincable=%d" % (1 if sincable else 0))
            self._esperar(3.0)
            prom = CAMARAS_DE_MENTIRA[self._corridas["C"] % len(CAMARAS_DE_MENTIRA)]
            self._corridas["C"] += 1
            tanda = ["#CAM n=76 cero=0 val=76 xp=%d,%d,%s yp=-1.2 am=0 az=0" % (
                int(prom) - 3, int(prom) + 3, _g1(prom)), "#FIN C"]
            self._terminar("cancha", "C", "CAMARA", "OK", sincable, tanda)
        elif codigo == "A":
            if not self._cuenta("A ANCHO", sincable):
                return
            self._decir("#INICIO A ANCHO tope=45000 sincable=%d" % (1 if sincable else 0))
            medioizq, izqder, derizq, desvio = ANCHOS_DE_MENTIRA[self._corridas["A"] % 2]
            self._corridas["A"] += 1
            for giro in (2, 3):
                self._decir("#PASO A calibrando giro=%d" % giro)
                if self._esperar(1.0) is not None:
                    return self._ancho_frenado(sincable)
            desvios = desvio.split(",")
            for tramo, ms in enumerate((medioizq, izqder, derizq, ms_hasta_el_medio(izqder))):
                if self._esperar(ms / 1000.0 + 0.5) is not None:
                    return self._ancho_frenado(sincable, medioizq if tramo else 0)
                self._decir("#PASO A tramo=%d ms=%d desvio=%s endereza=0" % (tramo, ms, desvios[tramo]))
            tanda = [linea_ancho(medioizq, izqder, derizq, desvio, valores=self.valores),
                     "ANCHO: al medio desde la izquierda %d ms" % ms_hasta_el_medio(izqder), "#FIN A"]
            self._terminar("cancha", "A", "ANCHO", "OK", sincable, tanda)
        elif codigo == "F":
            if not self._cuenta("F FONDO" if not sincable else "F FONDO_COMPROBAR", sincable):
                return
            if sincable:
                probar = int(V("MS_MEDIO_FONDO_POR_TIEMPO"))
                self._decir("#INICIO F FONDO_COMPROBAR tope=8000 sincable=1")
                self._esperar(probar / 1000.0)
                tanda = ["#FONDO ms=%d como=TIEMPO desvio=1.2 umbral=%d probado=%d" % (
                    probar, V("umbralBlanco"), probar), "#FIN F"]
                self._terminar("cancha", "F", "FONDO_COMPROBAR", "OK", True, tanda)
                return
            self._decir("#INICIO F FONDO tope=6000 sincable=0")
            auto = FONDOS_DE_MENTIRA[self._corridas["F"] % 2]     # "aprieta TOCÓ" solo, si nadie lo hace
            self._corridas["F"] += 1
            llego = self._esperar(auto / 1000.0)
            freno = None
            if llego is None:
                ms, como = auto, "TOCO"
            elif llego[1].lower() == "t":
                ms, como = llego[0], "TOCO"
            else:
                ms, como, freno = llego[0], "PARADO", "#FRENO tecla"
                self._t_freno = time.monotonic()
            tanda = ["#FONDO ms=%d como=%s desvio=1.8 umbral=%d probado=0" % (ms, como, V("umbralBlanco")), "#FIN F"]
            self._terminar("cancha", "F", "FONDO", "OK" if como == "TOCO" else "ERROR", False, tanda, freno)
        elif codigo == "I":
            if not self._cuenta("I IR_AL_MEDIO", sincable):
                return
            self._decir("#INICIO I IR_AL_MEDIO tope=40000 sincable=%d" % (1 if sincable else 0))
            for k, (ms, como) in enumerate(MEDIO_DE_MENTIRA, 1):
                if self._esperar(ms / 1000.0 + 0.3) is not None:
                    self._t_freno = time.monotonic()
                    hechos = MEDIO_DE_MENTIRA[:k - 1] + tuple((0, "-") for _ in range(9 - k))
                    tanda = ["#MEDIO ok=0 pasos=%s como=%s desvio=3.1 acomodo=0" % (
                        ",".join(str(p[0]) for p in hechos), ",".join(p[1] for p in hechos)), "#FIN I"]
                    self._terminar("cancha", "I", "IR_AL_MEDIO", "ERROR", sincable, tanda, "#FRENO tecla")
                    return
                self._decir("#PASO I paso=%d ms=%d como=%s" % (k, ms, como))
            tanda = ["#MEDIO ok=1 pasos=%s como=%s desvio=3.1 acomodo=0" % (
                ",".join(str(p[0]) for p in MEDIO_DE_MENTIRA), ",".join(p[1] for p in MEDIO_DE_MENTIRA)), "#FIN I"]
            self._terminar("cancha", "I", "IR_AL_MEDIO", "OK", sincable, tanda)

    def _ancho_frenado(self, sincable, medioizq=0):
        self._t_freno = time.monotonic()
        tanda = [linea_ancho(medioizq, 0, 0, "2.9,0.0,0.0,0.0", ok=0, error=6, valores=self.valores), "#FIN A"]
        self._terminar("cancha", "A", "ANCHO", "ERROR", sincable, tanda, "#FRENO tecla")



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
VIOLETA = "#6a1b9a"
AMARILLO = "#f9a825"
COLOR_SEMAFORO = {"gris": "#9e9e9e", "verde": VERDE, "amarillo": AMARILLO, "rojo": ROJO}
PANEL_COPIADO = "Ya está copiado (y abajo se ve)."
PANEL_NO_COPIADO = "No pude copiarlo solo: seleccionalo abajo y Ctrl+C."
# 2026-11-08: para JUGAR se usa la 0921 (funciona\partido-0921), y sus valores
# se cambian en la pestaña PARTIDO (con copia), no pegando a mano en la 0.1103.
INSTRUCCIONES_PANEL = (
    "OJO: estos números son del TABLERO (copias de la 0.1103). Para JUGAR se usa la 0921 "
    "(funciona\\partido-0921), y sus valores se cambian en la pestaña PARTIDO, no a mano.\n"
    "1. Pestaña PARTIDO: buscá cada nombre y poné el número (cajita + Poner). Si un nombre no está "
    "ahí, la 0921 no lo usa.\n"
    "2. UN CAMBIO POR VEZ, y anotalo en la bitácora: qué, de cuánto a cuánto y qué se vio.\n"
    "3. Apretá CARGAR EL PARTIDO (guarda con copia, compila y carga; el robot confirma con su línea "
    "de versión). ⚠ Apenas termina la carga el robot ARRANCA SOLO.\n"
    "(Si el robot dejó algún aviso más, está en la pestaña CONSOLA.)"
)
INSTRUCCION_COPIAR_CANCHA = ("Ctrl+F con el nombre y cambiá SOLO el número y la etiqueta de ESA línea. "
                             "No pegues todo junto: queda repetido y no compila.")
# (2026-11-08, revisión) El botón de MOVIMIENTOS que vuelve los números del
# robot a los del PARTIDO: ahora los de la 0921 (antes, las copias de la 0.1103).
BOTON_NUMEROS_PARTIDO = "Volver a los números del PARTIDO (0921)"
# (2026-11-08, revisión) COPIAR AL PANEL arma líneas para el PANEL de la 0.1103:
# la 0921 (la de jugar) no tiene panel; sus valores van por PASAR A PARTIDO →.
PARA_JUGAR_NO_HACE_FALTA = ("PARA JUGAR (la 0921) ESTO NO HACE FALTA: usá PASAR A PARTIDO → (pone lo medido en la "
                            "pestaña PARTIDO) y CARGAR EL PARTIDO. Estas líneas son solo para el PANEL de la 0.1103 "
                            "(seguir-y-despejar, que ya no es la de jugar).")
# (En la ventana solo van caracteres hasta U+FFFF: un Tk viejo no puede
# mostrar los emojis de más arriba, y tiraría un error.)
# 2026-11-08: VOLVER AL PARTIDO ya NO carga la 0.1103 (seguir-y-despejar): va
# a la pestaña PARTIDO y carga la 0921 (partido-0921), la de jugar, con los
# valores que estén escritos ahí. Es el mismo CARGAR EL PARTIDO (con su ventanita).
TEXTO_VOLVER_AL_PARTIDO = ("Pestaña PARTIDO: revisá los valores y apretá CARGAR EL PARTIDO. Carga la 0921 "
                           "(funciona\\partido-0921), la de jugar, con los valores que están escritos ahí.")
TEXTO_CARGAR_PARTIDO = (
    "Esto GUARDA estos valores en el programa de partido (funciona\\partido-0921), lo COMPILA y lo CARGA "
    "al arquero.\n\n"
    "⚠ APENAS TERMINA LA CARGA, EL ROBOT ARRANCA SOLO: a los 2 s retrocede buscando la línea\n"
    "(y si no la encuentra, sale a buscar el medio).\n"
    "Batería APAGADA (lo mejor) o el robot EN EL PISO con lugar alrededor.\n"
    "Dejá enchufado SOLO el arquero.")
TEXTO_CARGAR_TABLERO = (
    "Esto carga el programa del TABLERO (pruebas\\tablero) al arquero. Ese no arranca solo, pero el robot "
    "deja de tener el de PARTIDO: al terminar, volvé a la pestaña PARTIDO y CARGAR EL PARTIDO.\n\n"
    "Dejá enchufado SOLO el arquero.")
TEXTO_NO_CERRAR = ("Se está cargando un programa al robot: esperá que termine (si cerrás ahora, la carga puede "
                   "quedar a medias). La primera vez tarda ~1 min.")
# Los pasos de la pestaña CANCHA: (clave, título, número que mide, se bloquea sin el paso 1)
PASOS_CANCHA = (
    ("0", "0 · ¿ANDA TODO?", None, False),
    ("1", "1 · La LÍNEA", "umbralBlanco", False),
    ("2", "2 · El ANCHO", "MS_MEDIO_AL_MEDIO", True),
    ("2c", "2c · Comprobar ir al medio", None, False),
    ("3", "3 · El FONDO", "MS_MEDIO_FONDO_POR_TIEMPO", True),
    ("4", "4 · La CÁMARA", "CAMARA_POR_CM", False),
    ("5", "5 · Cuánto sale el DESPEJE", "msAdelante", True),
    ("6", "6 · Los SIGNOS", "lateralInvertido", False),
    ("R", "Revisar a ojo", None, False),
)
REVISAR_A_OJO = (
    "MS_MAX_RETROCESO 1500 (la 0921): ¿lo apoyan más lejos de la línea del área?",
    "MS_MEDIO_TOPE_LATERAL 6000: tiene que ser 1,3 veces el cruce o más",
    "MS_MEDIO_ADELANTE 943: ¿el área es más honda?",
    "msEmpujonFinal 320 y MS_MEDIO_ALEJARSE 400: ¿el piso patina distinto?",
    "umbralCm y msAdelante: ¿el tamaño de la cancha es otro?",
    "LADO_MEDIO -1: ¿el lateral IZQUIERDO tiene línea?",
    "El ancho de la línea contra la separación de los sensores de atrás",
    "Que no haya otra línea entre el lateral izquierdo y el fondo",
    "¿Hay rampa o pared en los bordes?",
    "La luz y las cosas naranjas cerca de la cancha",
    "MS_QUIETO_PARA_PATRULLAR 10000: preguntarle al juez",
    "La batería: la de jugar, cargada",
)
RESPUESTAS_PELOTA = (
    ("DESPEJO", "LA DESPEJÓ"),
    ("FALTO_COSTADO", "LE FALTÓ COSTADO (la pelota quedó más afuera)"),
    ("SE_PASO", "SE PASÓ DE COSTADO (la pelota quedó más adentro)"),
    ("TARDE", "LLEGÓ TARDE (ya había pasado)"),
    ("CORTO", "SE QUEDÓ CORTO (no llegó adelante)"),
)


def guardar_falla(texto):
    """Anota una falla de la ventana en tablero-error.txt (al lado de este
    archivo), para mostrársela al profe. Con pythonw no hay consola."""
    try:
        with open(ARCHIVO_FALLAS, "a", encoding="utf-8") as f:
            f.write("==== %s ====\n%s\n" % (time.strftime("%Y-%m-%d %H:%M:%S"), texto))
    except Exception:
        pass


def escribir_atomico(ruta, texto):
    """Escribe un archivo de una: primero uno temporal y después lo cambia de
    nombre. Si se corta la luz en el medio, queda el viejo entero, no uno a
    medias."""
    carpeta = os.path.dirname(os.path.abspath(ruta))
    os.makedirs(carpeta, exist_ok=True)
    fd, temporal = tempfile.mkstemp(prefix=".hoja-", suffix=".tmp", dir=carpeta)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\r\n") as f:
            f.write(texto)
        os.replace(temporal, ruta)
    except Exception:
        try:
            os.remove(temporal)
        except OSError:
            pass
        raise


class Ventana:
    def __init__(self, raiz, conexion, cola, demo=False, estricto=False, carpeta_canchas=None,
                 lanzador=None, partido_ino=None, mirar_carga=None, partido_carpeta=None, pestana=None,
                 ejecutar=None, correr_en_hilo=None, fabrica_conexion=None, listar_puertos=None, dormir=None,
                 puerto_forzado=None):
        self.raiz = raiz
        self.conexion = conexion
        self.cola = cola
        self.demo = demo
        self.estricto = estricto        # en la autoprueba: una falla corta todo
        self.carpeta_canchas = carpeta_canchas or CANCHAS
        self.lanzador = lanzador        # para la autoprueba: un subprocess.Popen de mentira
        self.partido_ino = partido_ino or PARTIDO_INO
        # ---- la pestaña PARTIDO y el CARGADOR (2026-11-08) ----
        self.pestana_inicial = pestana or "movimientos"
        self.pestana_elegida = None
        self.ejecutar = ejecutar        # para la autoprueba: un Popen de mentira (None = subprocess.Popen)
        self.correr_en_hilo = correr_en_hilo or self._correr_en_hilo
        self.fabrica_conexion = fabrica_conexion or self._fabrica_conexion
        self.listar_puertos = listar_puertos
        self.dormir_carga = dormir
        self.puerto_forzado = puerto_forzado
        self.estado = Estado()
        self.conectado = False
        self.situacion = "buscando"     # buscando / otro / ocupado / conectado
        self.puerto = None
        self.serie = None
        self.es_arquero = False
        # ¿El robot tiene el programa del tablero? Solo un "#HOLA
        # tablero-arquero" lo asegura. Un #S solo sirve para animarse a
        # preguntar "h": al conectar la ventana no manda nada, porque otros
        # programas toman cualquier tecla como una orden.
        self.es_tablero = False
        self.programa_detectado = None  # TABLERO_ARQUERO / PARTIDO ... / AJUSTAR / TABLERO_DELANTERO / OTRO ...
        self.pidio_numeros = False      # ya mandó "?" en esta conexión
        self.t_pide_hola = None         # cuándo mandó "h" por última vez
        self.ultimo_aviso = None        # (texto, color) del último aviso (lo mira la autoprueba)
        self.t_conexion = None
        self.t_ultima_linea = None
        self.t_pedido = None            # cuándo se mandó "!n" (para no mandarlo dos veces)
        self.t_pedido_panel = None      # cuándo se mandó "v"
        self.t_pedido_repetir = None    # cuándo se mandó "r" / "rc"
        self.rc_a_mano = False          # el último "rc" lo escribieron en la CONSOLA (no lo pidió la ventana sola)
        # Se cortó el cable CON CABLE, contando o andando: hasta cuándo puede
        # seguir moviéndose solo (reloj de la PC), o None.
        self.t_cortado_andando = None
        self.mirar_carga = mirar_carga or teensys_en_modo_carga
        self.aviso_programa = False     # ya pasó el rato sin #HOLA
        self.sin_cable = False          # el tilde SIN CABLE
        self.elegido = 1
        self.filas = {}
        self.botones_mov = {}
        self.lineas_consola = 0
        self.pendientes = set()
        self.ultimo_mov = None          # copia del último resultado de un MOVIMIENTO (1 a 10)
        self.respuesta_pelota = None
        self.ultima_linea_copiada = None
        self.cerrada = False
        # ---- la pestaña CANCHA ----
        self.panel_partido = {}         # el PANEL DE CONTROL del partido (leído, nunca escrito)
        self.version_archivo = None
        self._leer_partido()
        self.hoja = None
        self.ruta_hoja = None
        self.paso_cancha = "0"
        self.pedido_cancha = None       # qué se pidió (para saber de qué es la medición que llega)
        self.botones_cancha = []
        self.w = {}                     # los widgets del paso de CANCHA elegido
        self.cancha = {"lineas": [], "anchos": [], "medios": [], "fondos": [], "comprobar": [],
                       "cam_4a": {}, "cam_4b": {}, "idas": [], "ida_espera_ok": None, "ida_en_curso": None,
                       "signo_en_curso": False, "lateral": None, "yp": None, "umbral_hecho": False,
                       "umbral_confirmado": False, "medido": {}, "revisado": set(), "bateria": False}
        self._iniciar_partido(partido_carpeta)
        self._armar_estilos()
        self._armar()
        self._elegir(1)
        self._elegir_paso("0")
        self._refrescar({"conexion", "valores", "sensores", "modo", "resultado", "partido"})
        self.raiz.protocol("WM_DELETE_WINDOW", self._cerrar)
        self.raiz.bind("<Escape>", lambda e: self._parar())
        self.raiz.report_callback_exception = self._falla_tk
        self.raiz.after(50, self._revisar_cola)
        self.raiz.after(250, self._latido)

    def _leer_partido(self):
        try:
            with open(self.partido_ino, encoding="utf-8", errors="replace") as f:
                texto = f.read()
        except OSError:
            return
        self.panel_partido = leer_panel(texto)
        self.version_archivo = version_del_partido(texto)

    # ------------------------------------------------------------ armado

    def _armar_estilos(self):
        estilo = ttk.Style(self.raiz)
        if "clam" in estilo.theme_names():
            estilo.theme_use("clam")    # "clam" deja pintar los botones de colores
        estilo.configure(".", background=FONDO, font=F_NORMAL)
        estilo.configure("TFrame", background=FONDO)
        estilo.configure("TNotebook", background=FONDO)
        estilo.configure("TNotebook.Tab", font=(LETRA, 13, "bold"), padding=(18, 8))
        estilo.map("TNotebook.Tab", background=[("selected", "white")])
        estilo.configure("TButton", font=F_NORMAL, padding=(10, 6))
        estilo.configure("TCheckbutton", background=FONDO, font=F_NORMAL)
        # Botones de los movimientos bajitos, para que la columna entre en
        # una notebook de 1366x768 (los 10).
        estilo.configure("Mov.TButton", font=F_NORMAL_B, padding=(10, 3), anchor="w")
        estilo.configure("Elegido.TButton", font=F_NORMAL_B, padding=(10, 3), anchor="w",
                         background="#90caf9")
        estilo.map("Elegido.TButton", background=[("disabled", "#cfd8dc"), ("active", "#64b5f6")])
        estilo.configure("Probar.TButton", font=(LETRA, 17, "bold"), padding=(18, 10),
                         background=VERDE, foreground="white")
        estilo.map("Probar.TButton", background=[("disabled", "#a5d6a7"), ("active", "#388e3c")],
                   foreground=[("disabled", "#f1f8e9")])
        estilo.configure("Paso.TButton", font=(LETRA, 14, "bold"), padding=(4, 2))
        estilo.configure("Partido.TButton", font=F_NORMAL_B, padding=(8, 6), foreground=ROJO)
        estilo.configure("Tablero.TButton", font=F_NORMAL_B, padding=(8, 6), foreground=AZUL)
        estilo.configure("Toco.TButton", font=(LETRA, 22, "bold"), padding=(30, 14),
                         background=NARANJA, foreground="white")
        estilo.map("Toco.TButton", background=[("disabled", "#ffccbc"), ("active", "#bf360c")])
        estilo.configure("Usar.TButton", font=F_NORMAL_B, padding=(10, 6), background="#c8e6c9")

    def _armar(self):
        r = self.raiz
        r.title("Tablero del ARQUERO" + ("   (ROBOT DE MENTIRA)" if self.demo else ""))
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
        # PARAR es un tk.Button (no ttk) rojo que NUNCA se deshabilita.
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
        self.tab_cancha = ttk.Frame(self.cuaderno)
        self.tab_partido = ttk.Frame(self.cuaderno)
        self.tab_consola = ttk.Frame(self.cuaderno)
        self.cuaderno.add(self.tab_pelota, text="  PELOTA  ")
        self.cuaderno.add(self.tab_sensores, text="  SENSORES  ")
        self.cuaderno.add(self.tab_mov, text="  MOVIMIENTOS  ")
        self.cuaderno.add(self.tab_cancha, text="  CANCHA  ")
        self.cuaderno.add(self.tab_partido, text="  ★ PARTIDO  ")
        self.cuaderno.add(self.tab_consola, text="  CONSOLA  ")
        self.pestanas = {"pelota": self.tab_pelota, "sensores": self.tab_sensores, "movimientos": self.tab_mov,
                         "cancha": self.tab_cancha, "partido": self.tab_partido, "consola": self.tab_consola}
        self.var_sin_cable = tk.BooleanVar(value=False)
        self._armar_pelota()
        self._armar_sensores()
        self._armar_movimientos()
        self._armar_cancha()
        self._armar_partido()
        self._armar_consola()
        self._ir_a_pestana(self.pestana_inicial if self.pestana_inicial in self.pestanas else "movimientos")
        self.cuaderno.bind("<<NotebookTabChanged>>", self._cambio_de_pestana)

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
        self._etiqueta(der, text="Qué haría el partido AHORA (con el robot quieto):", font=F_NORMAL_B,
                       wraplength=395).pack(fill="x", pady=(8, 0))
        self.lbl_p_frase = self._etiqueta(der, font=(LETRA, 20, "bold"), wraplength=395)
        self.lbl_p_frase.pack(fill="x", pady=(4, 6))
        self.lbl_p_detalle = self._etiqueta(der, font=F_MEDIO_B, wraplength=395)
        self.lbl_p_detalle.pack(fill="x")
        self.lbl_p_vel = self._etiqueta(der, font=F_NORMAL, fg="#455a64", wraplength=395)
        self.lbl_p_vel.pack(fill="x", pady=(4, 0))
        self.lbl_p_n = self._etiqueta(der, font=F_NORMAL, fg="#455a64", wraplength=395)
        self.lbl_p_n.pack(fill="x")
        self.lbl_p_dg = self._etiqueta(der, font=F_NORMAL, fg="#455a64", wraplength=395)
        self.lbl_p_dg.pack(fill="x")
        tk.Frame(der, height=2, bg="#bdbdbd").pack(fill="x", pady=10)
        self.lbl_p_camara = self._etiqueta(der, font=F_MEDIO_B, wraplength=395)
        self.lbl_p_camara.pack(fill="x")
        self._etiqueta(
            der, font=F_CHICO, fg="#616161", wraplength=395,
            text="CÓMO SE LEE. Visto desde arriba: el robot mira para ARRIBA y el arco propio queda "
                 "abajo. Lo que está a la DERECHA del robot se ve a la DERECHA de la pantalla.\n"
                 "Amarillo = acá se frena a MIRAR (distanciaParaMirar). Naranja = acá SALE "
                 "(umbralCm). Son rectángulos porque el partido compara lo de adelante y lo del "
                 "costado por separado (umbralDesvio).\n"
                 "Las rayas en V: el tope de la diagonal (MAX_ANGULO_DIAGONAL). Las dos rayitas "
                 "delante del robot: la zona muerta. La raya de lado a lado: hasta dónde llega la ida.\n"
                 "La flecha de la pelota: dónde estaría en medio segundo. La flecha verde: para "
                 "dónde saldría si saliera ahora.\n"
                 "Los arcos: solo la DIRECCIÓN (la distancia no está medida y el partido no los usa)."
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
            text="Cada barra es lo que lee el sensor (0 a 1023). La raya roja es umbralBlanco: de ahí "
                 "para arriba dice BLANCO. Para probar: pasá el robot por la línea con la mano."
        ).pack(fill="x", pady=(8, 0))

        f2 = tk.LabelFrame(t, text=" GIROSCOPIO ", font=F_MEDIO_B, bg=FONDO, padx=8, pady=8)
        f2.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        self.cv_brujula = tk.Canvas(f2, bg=FONDO, highlightthickness=0, height=260)
        self.cv_brujula.pack(fill="x")
        self.cv_brujula.bind("<Configure>", lambda e: self._dibujar_brujula())
        self.lbl_rumbo = tk.Label(f2, font=F_ENORME, bg=FONDO)
        self.lbl_rumbo.pack()
        self.lbl_fusion = tk.Label(f2, font=F_MEDIO_B, bg=FONDO, wraplength=320)
        self.lbl_fusion.pack(pady=4)
        self.lbl_cal = tk.Label(f2, font=F_NORMAL, bg=FONDO, wraplength=320)
        self.lbl_cal.pack()
        self.btn_giro = ttk.Button(f2, text="Buscar el giroscopio de nuevo", command=self._buscar_giro)
        self.btn_giro.pack(pady=6)
        self._etiqueta(
            f2, font=F_NORMAL, wraplength=320,
            text="Girá el robot con la mano: el número tiene que cambiar. 'fusión OK' = el giroscopio "
                 "anda bien. Sin eso los movimientos no arrancan. Si dice MUDO: " + AYUDA_SIN_GIRO
        ).pack(fill="x", pady=6)

        f3 = tk.LabelFrame(t, text=" CÁMARA ", font=F_MEDIO_B, bg=FONDO, padx=8, pady=8)
        f3.grid(row=0, column=2, sticky="nsew", padx=6, pady=6)
        self.lbl_q = self._etiqueta(f3, font=F_GRANDE, wraplength=330)
        self.lbl_q.pack(fill="x")
        self.lbl_q_txt = self._etiqueta(f3, font=F_NORMAL, wraplength=330, fg="#455a64")
        self.lbl_q_txt.pack(fill="x", pady=(2, 8))
        self.lbl_cam_pelota = self._etiqueta(f3, font=F_MEDIO_B, wraplength=330)
        self.lbl_cam_pelota.pack(fill="x", pady=2)
        tabla = tk.Frame(f3, bg=FONDO)
        tabla.pack(fill="x", pady=6)
        for col, titulo in enumerate(("", "X crudo", "Y crudo", "visto hace")):
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
            text="X e Y CRUDOS: como los manda la cámara (Y ya con el -100, sin dar vuelta). El robot "
                 "los pasa a cm con CAMARA_POR_CM, y con camaraYInvertida el + queda a la DERECHA."
        ).pack(fill="x", pady=10)

    def _armar_movimientos(self):
        t = self.tab_mov
        t.columnconfigure(0, weight=0)
        t.columnconfigure(1, weight=0)
        t.columnconfigure(2, weight=1)
        t.rowconfigure(0, weight=1)

        # --- izquierda: los 10 movimientos (bien juntito: 1366x768) ---
        izq = tk.Frame(t, bg=FONDO)
        izq.grid(row=0, column=0, sticky="ns", padx=(6, 4), pady=6)
        self._etiqueta(izq, text="Elegí un movimiento:", font=F_MEDIO_B).pack(fill="x", pady=(0, 2))
        for m in MOVIMIENTOS:
            b = ttk.Button(izq, text=m["boton"], style="Mov.TButton",
                           command=lambda n=m["n"]: self._elegir(n))
            b.pack(fill="x", pady=1)
            self.botones_mov[m["n"]] = b
        tk.Frame(izq, bg=FONDO, height=6).pack()
        self.btn_copiar = ttk.Button(izq, text="Copiar al panel", command=self._copiar_panel)
        self.btn_copiar.pack(fill="x", pady=1)
        self.btn_numeros = ttk.Button(izq, text=BOTON_NUMEROS_PARTIDO, command=self._volver_numeros)
        self.btn_numeros.pack(fill="x", pady=1)
        self.btn_ultimo = ttk.Button(izq, text="Pedir el último resultado (r)", command=self._pedir_ultimo)
        self.btn_ultimo.pack(fill="x", pady=1)
        tk.Frame(izq, bg="#bdbdbd", height=2).pack(fill="x", pady=8)
        self.btn_partido = ttk.Button(izq, text="VOLVER AL PARTIDO\n(cargar la 0921, la de jugar)",
                                      style="Partido.TButton", command=self._volver_al_partido)
        self.btn_partido.pack(fill="x", pady=1)

        # --- medio: qué hace, PROBAR, y sus números ---
        med = tk.Frame(t, bg=FONDO)
        med.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        self.lbl_mov_titulo = self._etiqueta(med, font=F_GRANDE, wraplength=520)
        self.lbl_mov_titulo.pack(fill="x")
        self.lbl_mov_desc = self._etiqueta(med, font=F_NORMAL, wraplength=520)
        self.lbl_mov_desc.pack(fill="x")
        self.lbl_mov_lugar = self._etiqueta(med, font=F_NORMAL_B, fg="#bf360c", wraplength=520)
        self.lbl_mov_lugar.pack(fill="x", pady=(4, 6))
        fila = tk.Frame(med, bg=FONDO)
        fila.pack(fill="x", pady=(0, 2))
        self.btn_probar = ttk.Button(fila, text="▶  PROBAR", style="Probar.TButton", command=self._probar)
        self.btn_probar.pack(side="left")
        self.chk_sin_cable = ttk.Checkbutton(fila, text="SIN CABLE\n(10 s para desenchufar)",
                                             variable=self.var_sin_cable, command=self._cambio_sin_cable)
        self.chk_sin_cable.pack(side="left", padx=10)
        self.lbl_cuenta = self._etiqueta(med, font=F_MEDIO_B, wraplength=520)
        self.lbl_cuenta.pack(fill="x", pady=(2, 4))
        self.lbl_mov_avisos = self._etiqueta(med, font=F_CHICO, fg=NARANJA, wraplength=520)
        self.lbl_mov_avisos.pack(fill="x")
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
        self.lbl_guardado = self._etiqueta(der, font=F_NORMAL_B, fg=VIOLETA)
        self.lbl_guardado.pack(fill="x")
        self.lbl_nota = tk.Label(der, text="-", font=F_ENORME, bg="#bdbdbd", fg="white")
        self.lbl_nota.pack(fill="x", pady=2)
        self.lbl_nota_txt = self._etiqueta(der, font=F_NORMAL, wraplength=560)
        self.lbl_nota_txt.pack(fill="x")
        der.bind("<Configure>", lambda e: self.lbl_nota_txt.configure(wraplength=max(200, e.width - 20)))
        self.cv_grafico = tk.Canvas(der, bg="white", height=220, highlightthickness=1,
                                    highlightbackground="#bdbdbd")
        self.cv_grafico.pack(fill="x", pady=4)
        self.cv_grafico.bind("<Configure>", lambda e: self._dibujar_grafico())
        # Solo en la 9: los intentos y la pregunta "¿qué pasó con la pelota?"
        self.marco_9 = tk.Frame(der, bg=FONDO)
        self.txt_intentos = tk.Text(self.marco_9, font=("Consolas", 10), height=5, wrap="none", bg="white",
                                    relief="flat")
        self.txt_intentos.pack(fill="x")
        self._etiqueta(self.marco_9, text="¿Qué pasó con la pelota?", font=F_NORMAL_B).pack(fill="x", pady=(4, 0))
        fila9 = tk.Frame(self.marco_9, bg=FONDO)
        fila9.pack(fill="x")
        self.botones_pelota = []
        for i, (clave, texto) in enumerate(RESPUESTAS_PELOTA):
            b = ttk.Button(fila9, text=texto, command=lambda c=clave: self._pregunta_pelota(c))
            b.grid(row=i // 2, column=i % 2, sticky="ew", padx=2, pady=1)
            self.botones_pelota.append(b)
        self.lbl_respuesta = self._etiqueta(self.marco_9, font=F_NORMAL_B, fg=AZUL, wraplength=560)
        self.lbl_respuesta.pack(fill="x")
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
                   command=lambda: self._consola_mandar("?")).pack(side="right", padx=6)
        abajo = tk.Frame(t, bg=FONDO)
        abajo.pack(side="bottom", fill="x", padx=6, pady=(4, 6))
        self._etiqueta(
            t, font=F_CHICO, fg="#616161", wraplength=1200,
            text="Órdenes:  ?  ayuda    h  saludo    =NOMBRE valor  cambia un número    !1 a !10  un "
                 "movimiento (con c: SIN CABLE)    !L v|i|d|c  !A  !F  !C  !I  cancha    x  parar    "
                 "v  panel    0  números del partido    g  giroscopio    r / rc  repetir lo guardado.    "
                 "Mientras se mueve solo se manda PARAR. Si el robot NO tiene el programa del tablero, "
                 "solo se puede mandar ? y h."
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
        self.txt_consola.tag_configure("sistema", foreground=VIOLETA)
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
            if not self.cerrada:
                self.raiz.after(50, self._revisar_cola)

    def _procesar_lote(self, lote):
        for msg in lote:
            try:
                self._procesar(msg)
            except Exception:
                self._falla()
        try:
            self._pedir_repetir()
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
            self.es_arquero = (self.serie == SERIE_ARQUERO)
            self.estado.reiniciar_conexion()
            self.t_conexion = time.monotonic()
            self.aviso_programa = False
            self.t_pedido = self.t_pedido_panel = self.t_pedido_repetir = None
            self.es_tablero = self.pidio_numeros = False
            # Recién cargado (hace menos de 30 s): lo que dijo al arrancar sigue valiendo.
            reciente = self.t_fin_carga is not None and time.monotonic() - self.t_fin_carga < 30.0
            if not reciente:
                self.ultimo_partido = None
            self.programa_detectado = ("PARTIDO " + self.ultimo_partido["version"]) if (
                reciente and self.ultimo_partido) else None
            self.t_pide_hola = None
            self._olvidar_ida()
            self._consola_agregar("--- conectado: %s (serie %s) ---" % (self.puerto, self.serie or "?"), "sistema")
            # NO se le manda nada todavía: recién cuando llega algo que solo
            # manda el tablero (ver _reconocer_tablero).
            if not self.es_arquero:
                self._aviso("Este robot NO es el arquero: la ventana no le manda nada.", ROJO)
            elif self.estado.esperando_sin_cable:
                self._aviso("Volvió el cable. Esperando que el robot diga qué guardó...", VIOLETA)
            self.pendientes |= {"conexion", "valores", "sensores", "modo", "resultado", "cancha"}
        elif tipo == "desconectado":
            modo_antes = self.estado.modo
            self.conectado = False
            self.situacion = "buscando"
            self.estado.modo = "ESPERA"
            self.estado.cuenta = None
            self.t_pedido = self.t_pedido_panel = self.t_pedido_repetir = None
            self.es_tablero = self.pidio_numeros = False
            self.programa_detectado = None
            self.ultimo_partido = None
            self._olvidar_ida()
            self._consola_agregar("--- se desenchufó (o se reinició el robot) ---", "sistema")
            if self.estado.esperando_sin_cable:
                # SIN CABLE: es LO ESPERADO, no una falla.
                self._aviso("SIN CABLE: el robot hace la prueba solo. Enchufalo cuando termine (sin apagar "
                            "la batería). " + SIN_CABLE_NO_LLEGA, VIOLETA)
            elif modo_antes in ("CUENTA", "MOVIENDO"):
                # CON cable, pero se soltó contando o andando (2026-11-03,
                # revisión): el robot NO se entera de que se desenchufó, así
                # que SIGUE solo hasta terminar o hasta su tope. Decir
                # "ESPERA" acá sería mentir justo cuando hay que levantarlo.
                tope = self.estado.tope if modo_antes == "MOVIENDO" else None
                self.t_cortado_andando = time.monotonic() + (tope or TOPE_MAS_LARGO) / 1000.0 + 5.0
                self._aviso(self._texto_cortado_andando(), ROJO)
            else:
                self._aviso("Se cortó la conexión con el robot. Cuando vuelva, se reconecta solo.", NARANJA)
            self.pendientes |= {"conexion", "modo", "sensores", "cancha"}
        elif tipo == "buscando":
            if not self.conectado:
                self.situacion = "buscando"
            self.pendientes.add("conexion")
        elif tipo == "otro":
            # Una Teensy que NO es el arquero: su puerto ni se abrió.
            self.situacion = "otro"
            self.puerto, self.serie = msg[1], msg[2]
            self._consola_agregar("--- en %s hay una Teensy que NO es el arquero (serie %s): no la abro ---"
                                  % (msg[1], msg[2] or "?"), "sistema")
            if msg[2] == SERIE_DELANTERO:
                self._aviso("Ese es el DELANTERO: la ventana no le habla. Enchufá el arquero.", ROJO)
            else:
                self._aviso("Hay otra Teensy enchufada (NO es el arquero): la ventana no la toca.", ROJO)
            self.pendientes.add("conexion")
        elif tipo == "ocupado":
            self.situacion = "ocupado"
            self.puerto = msg[1]
            self._consola_agregar("--- %s ocupado: %s ---" % (msg[1], msg[2]), "sistema")
            self.pendientes.add("conexion")
        elif tipo == "carga":
            self._llego_carga(msg)

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
        if tipo == "TEXTO" and self.conectado and self.es_arquero:
            programa = reconocer_programa(texto)
            if programa:
                self._programa_por_texto(programa)
        pedido_en_el_aire = self.t_pedido is not None
        if tipo in ("CUENTA", "INICIO", "FRENO", "ERROR", "HOLA", "REPITE"):
            self.t_pedido = None        # el robot contestó
        if tipo == "FIN" or (tipo == "ERROR" and "guardado" in m.get("mensaje", "")):
            self.t_pedido_repetir = None
        if tipo in ("S", "HOLA", "FIN", "CUENTA", "INICIO") and self.t_cortado_andando is not None:
            self.t_cortado_andando = None       # el robot habla de nuevo: ya no anda suelto
            self.pendientes.add("modo")
        modo_antes = self.estado.modo
        cambios = self.estado.aplicar(m, ahora)
        self.pendientes |= cambios
        if tipo == "FRENO":
            # No arrancó (o lo frenaron): lo que estaba en el aire ya no vale.
            self._olvidar_ida()
            if modo_antes != "MOVIENDO":
                self.pedido_cancha = None       # no se movió: no va a llegar ninguna medición de eso
        if tipo == "GIRO" and m.get("buscando") and pedido_en_el_aire:
            # 2026-11-03 (revisión): al PROBAR sin giroscopio, el robot lo busca
            # hasta ~6 s ANTES de contar. Si lo encuentra, ARRANCA solo.
            self.t_pedido = ahora + ESPERA_GIRO
            self._aviso("Buscando el giroscopio (hasta 6 s). Si aparece, ARRANCA la cuenta y se mueve: no te "
                        "acerques. Para que no arranque: PARAR.", NARANJA)
        if tipo == "HOLA":
            if m["nombre"] == PROGRAMA:
                self.programa_detectado = "TABLERO_ARQUERO"
            elif m["nombre"] == PROGRAMA_DELANTERO:
                self.programa_detectado = "TABLERO_DELANTERO"
                self._aviso("Este robot tiene el TABLERO DEL DELANTERO: los pines están cruzados. NO probar "
                            "nada.", ROJO)
            else:
                self.programa_detectado = "OTRO " + m["nombre"]
            if self.conectado and self.es_arquero:
                # El robot dijo que NO tiene el de jugar (cartel de PARTIDO, arriba).
                self.estado_archivo = "tablero" if self.programa_detectado == "TABLERO_ARQUERO" else "otro"
            self.pendientes |= {"conexion", "partido"}
        if tipo in ("S", "HOLA") and self.conectado and self.es_arquero:
            self._reconocer_tablero(tipo, ahora)
        if tipo == "OK":
            self._ok_llego(m)
        if tipo == "ERROR":
            self._olvidar_ida()          # no se cambió msPruebaIda (o no arrancó): no se manda !10
        if "cancha_lista" in cambios and self.estado.cancha_lista:
            self._llego_cancha(*self.estado.cancha_lista)
        if "fin" in cambios:
            self._llego_fin(m["codigo"])
        if tipo == "PARTIDO" and self.conectado and self.es_arquero:
            # (2026-11-08) La manda el programa de PARTIDO (0921g) al arrancar,
            # NO tablero.ino: no se le manda nada (ni h ni ?).
            self.programa_detectado = "PARTIDO " + m["version"]
            self.estado.hola = None
            self.ultimo_partido = m
            self.t_partido_robot = ahora
            self.pendientes |= {"conexion", "partido"}
            if self.esperando_version_hasta is not None or self.estado_archivo == "cargado_sin_confirmar":
                # (2026-11-08, revisión) también si llega TARDE (pasado el
                # tiempo, o después de desenchufar y enchufar): confirma igual.
                self._confirmar_con(m)
            else:
                self._estado_por_partido(m)
        if "sin_cable_perdido" in cambios:
            self._olvidar_ida()          # (2026-11-08) si era el IDA del paso 5, ya no llega
            self._aviso("Volvió el cable, pero el robot NO tiene un resultado nuevo: ¿lo apagaron antes de "
                        "que terminara? Probalo de nuevo.", NARANJA)

    def _programa_por_texto(self, programa):
        """Un renglón para personas dijo qué programa tiene el robot."""
        if programa == "TABLERO_ARQUERO":
            if self.programa_detectado is None:
                self.programa_detectado = programa
            self.estado_archivo = "tablero"
        else:
            # El de partido o ajustar-movimientos: NO es el tablero. No se le
            # manda h (ni nada): ajustar toma las teclas como órdenes.
            self.programa_detectado = programa
            self.estado.hola = None
            self.ultimo_partido = None  # (un cartel de otro programa: ya no es la 0921g)
            self.estado_archivo = "otro"        # (la 0921g no imprime carteles: solo su #PARTIDO)
        self.pendientes |= {"conexion", "partido"}

    def _reconocer_tablero(self, tipo, ahora):
        """Llegó un #S o un #HOLA. Al conectar la ventana NO mandó nada; recién
        ahora le pregunta:
           "h" = qué programa tenés (si todavía no lo dijo; cada 2 s)
           "?" = cuánto valen tus números (una vez por conexión, después del #HOLA)
        El robot manda #S SOLO en espera: la pregunta no puede frenar un
        movimiento."""
        if self.programa_detectado not in (None, "TABLERO_ARQUERO"):
            return                      # el partido, ajustar o el del delantero: nada
        if tipo == "HOLA" and not self._programa_ok():
            return
        if tipo == "S" and not self.es_tablero:
            self.es_tablero = True
            self.pendientes.add("conexion")
        if self.estado.modo != "ESPERA" or self._pedido_pendiente():
            return                      # por las dudas: nada que lo pueda frenar
        if self.estado.hola is None:
            if self.t_pide_hola is None or ahora - self.t_pide_hola >= 2.0:
                self.t_pide_hola = ahora
                self._mandar("h")
            return
        if not self.pidio_numeros:
            self.pidio_numeros = True
            self._mandar("?")

    def _pedir_repetir(self):
        """Si un #GUARDADO dijo nuevo=1 (nadie vio ese resultado: se hizo SIN
        CABLE), lo pide UNA vez con r o rc. Nunca con el robot moviéndose
        ni con otro pedido en el aire."""
        e = self.estado
        if not e.repetir or not (self.conectado and self.es_arquero and self._programa_ok()):
            return
        if e.modo != "ESPERA" or self._pedido_pendiente():
            return
        pedido = e.repetir.pop(0)
        if self._mandar(pedido):
            self.t_pedido_repetir = time.monotonic()
            self.rc_a_mano = False
            self._aviso("El robot tiene guardado un resultado (SIN CABLE) que esta ventana no vio: se lo pido "
                        "(%s)..." % pedido, VIOLETA)

    # ------------------------------------------------------------ refrescar

    def _refrescar(self, cambios):
        if cambios & {"conexion", "hola", "defectos"}:
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
        if cambios & {"modo", "resultado", "freno", "conexion", "hola", "giro", "valores", "sensores"}:
            self._pintar_cuenta()       # ahí se ve POR QUÉ PROBAR está apagado
        if "resultado" in cambios:
            self._pintar_resultado()
        # CANCHA se redibuja cuando cambia algo suyo; con los #S (10 por
        # segundo) solo en los pasos que los muestran (el 0 y el 6).
        if cambios & {"cancha", "cancha_lista"} or (
                self._visible(self.tab_cancha)
                and (cambios & {"conexion", "hola", "valores", "giro"}
                     or ("sensores" in cambios and self.paso_cancha in ("0", "6")))):
            self._pintar_cancha()
        if "ok" in cambios and self.estado.ultimo_ok:
            self._aviso("El robot confirmó: %s = %s" % self.estado.ultimo_ok, VERDE)
        if "error" in cambios and self.estado.ultimo_error is not None:
            self._aviso("El robot dice: " + self.estado.ultimo_error, ROJO)
        if "freno" in cambios and self.estado.freno:
            self._aviso("Se frenó: " + self.estado.freno, ROJO)
        if "panel_listo" in cambios and self.t_pedido_panel is not None:
            self.t_pedido_panel = None
            self._mostrar_panel(list(self.estado.panel))
        if "partido" in cambios:
            self._pintar_partido()
        elif cambios & {"conexion", "hola"}:
            self._pintar_robot_partido()
        self._actualizar_habilitados()

    def _visible(self, pestana):
        try:
            return str(self.cuaderno.select()) == str(pestana)
        except Exception:
            return False

    def _programa_ok(self):
        h = self.estado.hola
        return h is not None and h[0] == PROGRAMA

    def _habla_tablero(self):
        """¿El robot tiene el programa del tablero? Solo ese obedece PARAR
        (x) y entiende lo que se escribe en la consola."""
        return (self.conectado and self.es_arquero and self.programa_detectado in (None, "TABLERO_ARQUERO")
                and (self.es_tablero or self._programa_ok()))

    def _pedido_pendiente(self):
        ahora = time.monotonic()
        if self.t_pedido is not None and ahora - self.t_pedido < 3.0:
            return True
        if self.t_pedido_repetir is not None and ahora - self.t_pedido_repetir < 5.0:
            return True
        return self.estado.tanda_abierta and self.estado.modo == "ESPERA"

    def _puede(self):
        e = self.estado
        if not self.conectado and e.esperando_sin_cable:
            return False, "SIN CABLE: el robot está haciendo la prueba solo. Enchufalo cuando termine."
        if self._cortado_andando():
            return False, self._texto_cortado_andando()
        if not self.conectado and self.situacion == "otro":
            if self.serie == SERIE_DELANTERO:
                return False, "Ese es el DELANTERO: la ventana no le habla. Enchufá el arquero."
            return False, "La Teensy enchufada NO es el arquero: enchufá el arquero."
        if not self.conectado and self.situacion == "ocupado":
            return False, "El puerto del robot está ocupado: cerrá el monitor serie u otra ventana del tablero."
        cartel = None
        if self.conectado and self.es_arquero and not self._programa_ok():
            texto = self._texto_programa()[0]
            cartel = texto or None
            if (self.es_tablero or self.programa_detectado == "TABLERO_ARQUERO") and e.hola is None:
                cartel = "Esperando que el robot diga qué programa tiene..."
        serie_robot = e.hola[2] if e.hola else None
        buscando = bool(e.giro and e.giro.get("buscando"))
        return puede_mover(self.conectado, self.es_arquero, self._programa_ok(), e.modo,
                           self._pedido_pendiente(), self.serie, serie_robot, e.esperando_sin_cable,
                           buscando, cartel)

    @staticmethod
    def _habilitar(widget, si):
        try:
            widget.state(["!disabled"] if si else ["disabled"])
        except (AttributeError, Exception):
            try:
                widget.configure(state="normal" if si else "disabled")
            except Exception:
                pass

    def _actualizar_habilitados(self):
        se_puede, _ = self._puede()
        e = self.estado
        quieto = e.modo == "ESPERA" and not self._pedido_pendiente()
        for b in self.botones_mov.values():
            self._habilitar(b, quieto)
        for b in (self.btn_probar, self.btn_copiar, self.btn_numeros, self.btn_ultimo, self.btn_giro):
            self._habilitar(b, se_puede)
        # VOLVER AL PARTIDO va a la pestaña PARTIDO y dice POR QUÉ no se puede cargar.
        self._habilitar(self.btn_partido, not self.cargando)
        self._actualizar_habilitados_partido()
        for fila in self.filas.values():
            for b in fila["botones"]:
                self._habilitar(b, se_puede)
            if fila["entrada"] is not None:
                self._habilitar(fila["entrada"], se_puede)
        for b, paso, robot in self.botones_cancha:
            self._habilitar(b, (not robot) or (se_puede and not self._bloqueado(paso)))
        toco = self.w.get("toco")
        if toco is not None:
            self._habilitar(toco, self.conectado and e.modo == "MOVIENDO" and e.codigo == "F"
                            and e.mov_nombre == "FONDO")

    def _aviso(self, texto, color="#212121"):
        self.ultimo_aviso = (texto, color)
        self.lbl_aviso.configure(text=texto, fg=color)

    def _pintar_conexion(self):
        e = self.estado
        if self.cargando and not self.conectado:
            texto, fondo, letra = "CARGANDO un programa al robot... no desenchufes el USB", VIOLETA, "white"
        elif self.conectado and self.demo:
            texto, fondo, letra = "●  ROBOT DE MENTIRA (--demo): no hay ningún robot de verdad", VIOLETA, "white"
        elif self.conectado and self.es_arquero:
            texto, fondo, letra = "●  CONECTADO  ·  %s  ·  ARQUERO" % self.puerto, VERDE, "white"
        elif self.conectado:
            texto = "✖  ESA TEENSY NO ES EL ARQUERO (serie %s): la ventana no le manda nada" % (
                self.serie or "desconocida")
            fondo, letra = ROJO, "white"
        elif e.esperando_sin_cable:
            texto = "SIN CABLE: el robot hace la prueba solo. Enchufalo cuando termine (sin apagar la batería)"
            fondo, letra = VIOLETA, "white"
        elif self.situacion == "otro" and self.serie == SERIE_DELANTERO:
            texto, fondo, letra = "✖  ESE ES EL DELANTERO: no le hablo. Enchufá el arquero.", ROJO, "white"
        elif self.situacion == "otro":
            texto = "✖  ESA TEENSY NO ES EL ARQUERO (serie %s)" % (self.serie or "desconocida")
            fondo, letra = ROJO, "white"
        elif self.situacion == "ocupado":
            texto = ("El robot está en %s, pero el puerto está OCUPADO: cerrá el monitor serie "
                     "(MIRAR.bat, PlatformIO o Arduino) u OTRA VENTANA DEL TABLERO" % self.puerto)
            fondo, letra = NARANJA, "white"
        else:
            texto, fondo, letra = "○  Buscando el robot...  enchufá el cable USB", "#e0e0e0", "#424242"
        self.lbl_conexion.configure(text=texto, bg=fondo, fg=letra)
        programa, color = self._texto_programa()
        self.lbl_programa.configure(text=programa, fg=color)

    def _texto_programa(self):
        if not self.conectado or not self.es_arquero:
            return "", "#424242"
        e = self.estado
        prog = self.programa_detectado
        if prog == "TABLERO_DELANTERO":
            return ("⛔ Este robot tiene el TABLERO DEL DELANTERO: los pines están cruzados. NO probar nada. "
                    "Cargá el del arquero con TABLERO.bat. (PARAR igual le llega.)"), ROJO
        if prog and prog.startswith("PARTIDO"):
            version = prog[len("PARTIDO"):].strip() or "?"
            if self.ultimo_partido is not None:
                return ("Tiene el programa de PARTIDO %s (el de jugar). Con la batería prendida se mueve solo y "
                        "NO obedece PARAR. Para usar MOVIMIENTOS o CANCHA: pestaña PARTIDO → Cargar el TABLERO."
                        % version), ROJO
            return ("⚠ Tiene el programa de PARTIDO (versión %s): si es la 0.1103, NO es la de jugar (pestaña "
                    "PARTIDO → CARGAR EL PARTIDO). Con la batería prendida se mueve solo y NO obedece PARAR. Para "
                    "usar MOVIMIENTOS o CANCHA: pestaña PARTIDO → Cargar el TABLERO." % version), ROJO
        if prog == "AJUSTAR":
            return "Tiene AJUSTAR-MOVIMIENTOS: no habla con esta ventana. Usá TABLERO.bat.", NARANJA
        if prog and prog.startswith("OTRO"):
            return ("⚠ " + NO_OBEDECE_PARAR + "  (Tiene otro programa: %s. El del tablero se carga con "
                    "TABLERO.bat.)" % prog[5:]), ROJO
        hola = e.hola
        if hola is None:
            if self.es_tablero or prog == "TABLERO_ARQUERO":
                return "Preguntándole al robot qué programa tiene...", GRIS
            if self.aviso_programa:
                if e.esperando_sin_cable:
                    return "El robot todavía se está moviendo solo (SIN CABLE): esperá.", VIOLETA
                return "⚠ No sé qué programa tiene: puede NO obedecer PARAR. Levantalo o apagá la batería.", ROJO
            return "Escuchando al robot para saber qué programa tiene...", GRIS
        if hola[2] is not None and hola[2] != SERIE_ARQUERO:
            return ("⛔ El robot dice que NO es el arquero (serie %s): no se mueve. ¿Le cargaron el tablero del "
                    "arquero al otro robot?" % hola[2]), ROJO
        viejas = copias_viejas(e.defectos, self.panel_partido)
        if viejas:
            detalle = ", ".join("%s: tablero %s, partido %s" % v for v in viejas[:3])
            return ("⚠ COPIAS VIEJAS: el partido cambió (%s%s). Avisar: hay que actualizar tablero.ino."
                    % (detalle, "..." if len(viejas) > 3 else "")), NARANJA
        if self.version_archivo and hola[3] and self.version_archivo != hola[3]:
            return ("⚠ El archivo del partido es la %s y el tablero tiene copias de la %s. Avisar: hay que "
                    "actualizar tablero.ino." % (self.version_archivo, hola[3])), NARANJA
        return "Programa del tablero cargado  (%s, copias del partido %s)" % (hola[1] or "?", hola[3] or "?"), VERDE

    def _texto_sin_datos(self):
        """Por qué no hay datos frescos para dibujar (o None si los hay)."""
        if not self.conectado:
            if self.estado.esperando_sin_cable:
                return "SIN CABLE: el robot está haciendo la prueba solo"
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
        V = lambda n: valor_num(self.estado.valores, n)
        # la cámara ve hasta ~70 cm adelante y ~35 al costado
        escala = max(0.5, min((h - 130) / 85.0, (w / 2.0 - 30) / 50.0))     # píxeles por cm
        cx, cy = w / 2.0, h - 90.0
        P = lambda x, y: cm_a_pantalla(x, y, cx, cy, escala)

        desvio, umbral, mirar = V("umbralDesvio"), V("umbralCm"), V("distanciaParaMirar")
        if V("medirAntesDeDespejar") >= 0.5:
            x1, y1 = P(mirar, -desvio)
            x2, y2 = P(0, desvio)
            c.create_rectangle(x1, y1, x2, y2, fill="#fff59d", outline="#fbc02d", width=2)
            c.create_text(x1 + 6, y1 + 4, anchor="nw", fill="#5d4037", font=F_NORMAL_B,
                          text="acá se frena a MIRAR (%g cm)" % mirar)
        x1, y1 = P(umbral, -desvio)
        x2, y2 = P(0, desvio)
        c.create_rectangle(x1, y1, x2, y2, fill="#ffb74d", outline="#e65100", width=2)
        c.create_text(x1 + 6, y1 + 4, anchor="nw", fill="#3e2723", font=F_NORMAL_B,
                      text="acá SALE (%g cm)" % umbral)
        # el cono de la diagonal (con la pelota QUIETA el ángulo de salida es el de la pelota)
        tope = math.radians(V("MAX_ANGULO_DIAGONAL"))
        for signo in (1, -1):
            xa, ya = P(80 * math.cos(tope), signo * 80 * math.sin(tope))
            c.create_line(cx, cy, xa, ya, fill="white", dash=(6, 5), width=2)
        xa, ya = P(80 * math.cos(tope), 80 * math.sin(tope))
        c.create_text(min(xa, w - 10), max(ya - 4, 30), anchor="se", fill="white", font=F_CHICO,
                      text="tope de la diagonal (%g°): con la pelota QUIETA,\nafuera de esto NO SALE"
                           % V("MAX_ANGULO_DIAGONAL"))
        # la zona muerta del seguimiento: dos rayitas delante del robot
        zm = V("ZONA_MUERTA_PELOTA")
        for signo in (1, -1):
            xa, ya = P(14, signo * zm)
            xb, yb = P(26, signo * zm)
            c.create_line(xa, ya, xb, yb, fill="#e0f2f1", width=1)
        # hasta dónde llega la ida (una CUENTA del 04/08, a potencia 200)
        llega = V("msAdelante") / 10.0 - 3.3
        color = "#ff5252" if llega <= umbral else "white"
        xa, ya = P(llega, -desvio - 6)
        xb, yb = P(llega, desvio + 6)
        c.create_line(xa, ya, xb, yb, fill=color, width=3, dash=(10, 4))
        c.create_text(xb + 6, yb, anchor="w", fill=color, font=F_NORMAL_B,
                      text="llega la ida ≈ %.0f cm\n(una cuenta del 04/08, a 200)" % llega)
        c.create_text(14, 12, anchor="nw", text="◄ IZQUIERDA del robot", fill="white", font=F_NORMAL_B)
        c.create_text(w - 14, 12, anchor="ne", text="DERECHA del robot ►", fill="white", font=F_NORMAL_B)

        # el robot, con el frente marcado, y el arco propio atrás
        rr = max(11 * escala, 14)
        abajo = min(cy + rr + 6, h - 20.0)          # que la franja no se salga del dibujo
        c.create_rectangle(cx - 30 * escala, abajo, cx + 30 * escala, abajo + 16, fill="#9e9e9e",
                           outline="white")
        c.create_text(cx, abajo + 8, text="ARCO PROPIO (atrás)", fill="white", font=F_NORMAL_B)
        c.create_oval(cx - rr, cy - rr, cx + rr, cy + rr, fill="#37474f", outline="white", width=2)
        c.create_polygon(cx, cy - rr - 14, cx - 10, cy - rr + 3, cx + 10, cy - rr + 3,
                         fill="#ff5252", outline="white")

        s = self.estado.sensores
        motivo = self._texto_sin_datos()
        if motivo:
            c.create_text(w / 2.0, 44, text=motivo, fill="#fff59d", font=F_MEDIO_B)
        if s is None:
            return
        # los arcos: solo la DIRECCIÓN, en el borde del dibujo
        for clave, color, nombre in (("am", "#fdd835", "amarillo"), ("az", "#1e88e5", "azul")):
            dato = s.get(clave)
            if not visible(dato) or dato[2] > EDAD_VIEJA_MS:
                continue
            x, y_crudo = dato[0], dato[1]
            y_der = -y_crudo if V("camaraYInvertida") >= 0.5 else y_crudo
            ang = math.atan2(y_der, x)
            radio = min(cx - 40, cy - 40)
            px, py = cx + radio * math.sin(ang), cy - radio * math.cos(ang)
            c.create_rectangle(px - 26, py - 9, px + 26, py + 9, fill=color, outline="black", width=2)
            c.create_text(px, py + 20, text="arco %s (dirección; la distancia no está medida,\ny el partido "
                                            "no los usa)" % nombre, fill="white", font=F_CHICO)
        d, y = s.get("d"), s.get("y")
        if d is None or y is None or not visible(s.get("p")):
            return
        viejo = (not s.get("ve")) or s["p"][2] > EDAD_VIEJA_MS
        px, py = P(d, y)
        c.create_line(cx, cy, px, py, fill="#bdbdbd" if viejo else "#ffe0b2", width=2, dash=(2, 4))
        rb = max(3.5 * escala, 11)
        c.create_oval(px - rb, py - rb, px + rb, py + rb, fill="#9e9e9e" if viejo else "#ff8f00",
                      outline="white", width=2)
        c.create_text(px, py - rb - 12, text=("última vez vista" if viejo else "%.0f cm" % d),
                      fill="white", font=F_MEDIO_B)
        vy, va = s.get("vy") or 0.0, s.get("va") or 0.0
        if not viejo and (abs(vy) > 4 or abs(va) > 4):
            qx, qy = P(d - va * 0.5, y + vy * 0.5)
            c.create_line(px, py, qx, qy, fill="#ffeb3b", width=3, arrow="last", arrowshape=(12, 14, 5))
        if viejo:
            return
        codigo, frase = que_haria(s, self.estado.valores)
        dg = s.get("dg")
        if codigo == "SALDRIA" and dg:
            ang = math.radians(angulo_salida(dg[2], dg[3], V("rendimientoCostado")))
            largo = max(llega, 10.0)
            qx, qy = P(largo * math.cos(ang), largo * math.sin(ang))
            c.create_line(cx, cy - rr, qx, qy, fill="#00e676", width=5, arrow="last", arrowshape=(18, 22, 7))
        elif codigo == "NO_SALDRIA":
            c.create_text(cx, cy - rr - 30, text=frase, fill="#ff8a80", font=F_MEDIO_B)

    def _textos_pelota(self):
        s = self.estado.sensores
        if s is None:
            for lbl in (self.lbl_p_detalle, self.lbl_p_vel, self.lbl_p_n, self.lbl_p_dg, self.lbl_p_camara):
                lbl.configure(text="")
            self.lbl_p_frase.configure(text="Sin datos del robot", fg=GRIS)
            return
        codigo, frase = que_haria(s, self.estado.valores)
        color = {"SALDRIA": VERDE, "MIRARIA": "#f57f17", "NO_SALDRIA": ROJO, "JUNTANDO": NARANJA}.get(codigo, "#424242")
        self.lbl_p_frase.configure(text=frase, fg=color)
        if s.get("d") is not None and visible(s.get("p")):
            edad = s["p"][2]
            if s.get("ve") and edad <= EDAD_VIEJA_MS:
                self.lbl_p_detalle.configure(text="Pelota a %.0f cm, %s" % (s["d"], lado_pelota(s.get("y"))),
                                             fg=NARANJA)
            else:
                self.lbl_p_detalle.configure(text="No la veo. La última vez (hace %.1f s): a %.0f cm, %s" % (
                    edad / 1000.0, s["d"], lado_pelota(s.get("y"))), fg=GRIS)
        else:
            self.lbl_p_detalle.configure(text="No la vio nunca (desde que arrancó)", fg=GRIS)
        vy, va = s.get("vy"), s.get("va")
        if vy is not None and va is not None:
            costado = "quieta de costado" if abs(vy) < 0.05 else "%.1f cm/s hacia la %s" % (
                abs(vy), "DERECHA" if vy > 0 else "IZQUIERDA")
            acerca = "se acerca a %.1f cm/s" % va if va > 0 else ("se aleja a %.1f cm/s" % -va if va < 0 else "no se acerca")
            self.lbl_p_vel.configure(text="Velocidad: %s, %s (con el robot moviéndose, la cámara se confunde)."
                                     % (costado, acerca))
        else:
            self.lbl_p_vel.configure(text="")
        n = s.get("n")
        self.lbl_p_n.configure(text="" if n is None else "%d de 3 cuadros seguidos en la zona" % n)
        dg = s.get("dg")
        if dg and s.get("ve"):
            if dg[0]:
                self.lbl_p_dg.configure(text="Si saliera ahora: avance %d, costado %+d, ida %d ms (%s)." % (
                    dg[2], dg[3], dg[4], "derecho" if dg[3] == 0 else "%.0f° a la %s" % (
                        abs(angulo_salida(dg[2], dg[3], valor_num(self.estado.valores, "rendimientoCostado"))),
                        "DERECHA" if dg[3] > 0 else "IZQUIERDA")))
            else:
                self.lbl_p_dg.configure(text="Si saliera ahora: NO sale (%s)." % (
                    "muy abierto" if dg[1] == 2 else "el avance no alcanza"))
        else:
            self.lbl_p_dg.configure(text="")
        texto, color = self._texto_camara(s.get("q"))
        self.lbl_p_camara.configure(text=texto, fg=color)

    @staticmethod
    def _texto_camara(q):
        if q is None:
            return "Cámara: ?", GRIS
        if q <= 0:
            return "Cámara: 0 paquetes por segundo. No manda nada.", ROJO
        return "Cámara: %g paquetes por segundo" % q, (VERDE if q >= 16 else NARANJA)

    # ------------------------------------------------------------ SENSORES

    def _dibujar_lineas(self):
        c = self.cv_lineas
        c.delete("all")
        w = max(int(c.winfo_width()), 420)
        h = max(int(c.winfo_height()), 280)
        s = self.estado.sensores
        u = s.get("u") if s else None
        l = s.get("l") if s else None
        # el robot visto desde arriba, mirando para arriba
        r = min(w * 0.18, h * 0.36)
        cx, cy = r + 30, h / 2.0
        c.create_oval(cx - r, cy - r, cx + r, cy + r, fill="#eceff1", outline="#455a64", width=3)
        c.create_polygon(cx, cy - r - 12, cx - 10, cy - r + 4, cx + 10, cy - r + 4, fill="#ff5252", outline="")
        c.create_text(cx, cy - r - 20, text="frente", font=F_CHICO, fill="#455a64")
        sensores = (("ATRÁS IZQ (A13)", 0, cx - r * 0.55, cy + r * 0.6),
                    ("ATRÁS DER (A11)", 1, cx + r * 0.55, cy + r * 0.6),
                    ("ADELANTE (A12)", 2, cx, cy - r * 0.7))
        izq = cx + r + 40
        ancho = max(80, w - izq - 120)
        for fila, (nombre, i, sx, sy) in enumerate(sensores):
            v = l[i] if l else None
            blanco = v is not None and u is not None and v >= u
            c.create_oval(sx - 9, sy - 9, sx + 9, sy + 9, fill=("white" if blanco else "#66bb6a") if v is not None
                          else "#bdbdbd", outline="#212121", width=2)
            c.create_text(sx, sy + 16, text=nombre.split(" (")[0], font=F_CHICO, fill="#212121")
            y0 = 20 + fila * (h - 40) / 3.0
            y1 = y0 + 22
            c.create_text(izq, y0 - 2, anchor="sw", text=nombre + ("   (el partido NO lo usa)" if i == 2 else ""),
                          font=F_NORMAL_B, fill="#212121")
            c.create_rectangle(izq, y0, izq + ancho, y1, fill="#37474f", outline="")
            if v is None:
                c.create_text(izq + ancho + 10, (y0 + y1) / 2, anchor="w", text="sin dato", font=F_NORMAL, fill=GRIS)
                continue
            c.create_rectangle(izq, y0, izq + min(max(v / 1023.0, 0.0), 1.0) * ancho, y1,
                               fill="white" if blanco else "#66bb6a", outline="")
            if u is not None:
                ux = izq + min(max(u / 1023.0, 0.0), 1.0) * ancho
                c.create_line(ux, y0 - 6, ux, y1 + 6, fill="#e53935", width=3)
                c.create_text(ux, y1 + 8, anchor="n", text="umbral %d" % u, fill="#e53935", font=F_CHICO)
            c.create_text(izq + ancho + 10, (y0 + y1) / 2 - 9, anchor="w", text=str(v), font=F_MEDIO_B, fill="#212121")
            c.create_text(izq + ancho + 10, (y0 + y1) / 2 + 10, anchor="w", text="BLANCO" if blanco else "VERDE",
                          font=F_NORMAL_B, fill="#212121" if blanco else VERDE)
        estado = estado_linea(l, u)
        frase = {"IZQ": "pisa el lateral IZQUIERDO", "DER": "pisa el lateral DERECHO",
                 "AREA": "los dos: la línea del ÁREA", "-": "-"}[estado]
        c.create_text(cx, cy + r + 18, text=frase, font=F_MEDIO_B, fill=AZUL if estado != "-" else GRIS)
        if l and u is not None and all(x is not None and x >= u for x in l):
            c.create_text(w / 2.0, h - 26, text="¿Está sobre la MESA? Así no se mueve bien (ir-al-medio leyó "
                                                "505/651/601 en la mesa).", font=F_NORMAL_B, fill=ROJO)
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
            c.create_text(cx, cy, text=("MUDO /\nSIN GIROSCOPIO" if s else "sin datos"), justify="center",
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
        giro = self.estado.giro
        if giro and giro.get("buscando"):
            texto, color = "Buscando el giroscopio...", NARANJA
        elif s is None:
            texto, color = "sin datos", GRIS
        elif rumbo is None:
            texto, color = "MUDO / SIN GIROSCOPIO: " + AYUDA_SIN_GIRO, ROJO
        elif g == 5:
            texto, color = "fusión OK  (estado 5)", VERDE
        elif g is None:
            texto, color = "el estado no contesta", NARANJA
        else:
            texto, color = "estado %d: %s" % (g, ESTADOS_GIRO.get(g, "?")), ROJO
        self.lbl_fusion.configure(text=texto, fg=color)
        cal = s.get("cal") if s else None
        if cal:
            self.lbl_cal.configure(text="calibración: giro %d/3 (medir-ancho espera 3), sistema %d, acel %d, mag %d"
                                   % (cal[1], cal[0], cal[2], cal[3]), fg=VERDE if cal[1] == 3 else NARANJA)
        else:
            self.lbl_cal.configure(text="calibración: -", fg=GRIS)
        q = s.get("q") if s else None
        if q is None:
            self.lbl_q.configure(text="? paquetes por segundo", fg=GRIS)
            self.lbl_q_txt.configure(text="")
        else:
            texto, color = self._texto_camara(q)
            self.lbl_q.configure(text="%g paquetes por segundo" % q, fg=color if q > 0 else ROJO)
            self.lbl_q_txt.configure(
                text=("La cámara no manda nada: ¿está prendida? ¿el cable está bien?" if q <= 0
                      else "Tiene que dar de 16 a 26."))
        if s and s.get("d") is not None and visible(s.get("p")):
            self.lbl_cam_pelota.configure(text="Pelota: %.1f cm adelante, %s (hace %.1f s)" % (
                s["d"], lado_pelota(s.get("y")), s["p"][2] / 1000.0),
                fg="#212121" if s.get("ve") else GRIS)
        else:
            self.lbl_cam_pelota.configure(text="Pelota: no la vio nunca", fg=GRIS)
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

    def _buscar_giro(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if self._mandar("g"):
            self._aviso("Le pedí al robot que busque el giroscopio de nuevo (hasta ~6 s)...", GRIS)


    # ------------------------------------------------------------ MOVIMIENTOS

    def _elegir(self, n):
        self.elegido = n
        m = MOV_POR_NUMERO[n]
        for k, b in self.botones_mov.items():
            b.configure(style="Elegido.TButton" if k == n else "Mov.TButton")
        self.lbl_mov_titulo.configure(text=m["titulo"])
        self.lbl_mov_desc.configure(text=m["que_hace"])
        self.lbl_mov_lugar.configure(text="⚠ " + m["lugar"] + " " + AVISO_SIN_CABLE[2:])
        self._texto_probar()
        self._armar_tabla(m)
        self._pintar_valores()
        self._pintar_avisos()
        self._pintar_resultado()
        self._actualizar_habilitados()

    def _texto_probar(self):
        m = MOV_POR_NUMERO[self.elegido]
        if self.sin_cable:
            self.btn_probar.configure(text="▶  PROBAR SIN CABLE")
        else:
            self.btn_probar.configure(text="▶  PROBAR  " + m["nombre"].replace("_", " "))

    def _cambio_sin_cable(self):
        """El tilde SIN CABLE (está en MOVIMIENTOS y en CANCHA: es el mismo)."""
        try:
            self._poner_sin_cable(bool(self.var_sin_cable.get()))
        except Exception:
            self._poner_sin_cable(not self.sin_cable)

    def _poner_sin_cable(self, si):
        self.sin_cable = bool(si)
        try:
            self.var_sin_cable.set(self.sin_cable)
        except Exception:
            pass
        self._texto_probar()
        self._pintar_cuenta()
        if self.sin_cable:
            self._aviso("SIN CABLE: el robot cuenta 10 s para que lo desenchufen. " + SIN_CABLE_NO_LLEGA, VIOLETA)

    def _armar_tabla(self, m):
        for hijo in self.tabla.winfo_children():
            hijo.destroy()
        self.filas = {}
        for i, nombre in enumerate(m["numeros"]):
            paso, explicacion, opciones = NUMEROS[nombre]
            fondo = "white" if i % 2 == 0 else "#e8eef3"
            f = tk.Frame(self.tabla, bg=fondo, padx=6, pady=3)
            f.pack(fill="x", pady=1)
            arriba = tk.Frame(f, bg=fondo)
            arriba.pack(fill="x")
            tk.Label(arriba, text=nombre, font=F_NORMAL_B, bg=fondo, anchor="w", width=24).pack(side="left")
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
            valor = tk.Label(arriba, text="?", font=F_GRANDE, width=6, bg=fondo)
            menos.pack(side="left")
            valor.pack(side="left", padx=4)
            mas.pack(side="left")
            botones = [menos, mas]
            entrada = None
            if opciones is None:
                entrada = ttk.Entry(arriba, width=7, font=F_MEDIO)
                entrada.pack(side="left", padx=(12, 4))
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
        self._pintar_avisos()

    def _pintar_avisos(self):
        avisos = avisos_de_numeros(self.estado.valores, self.estado.sensores, self.elegido)
        self.lbl_mov_avisos.configure(text="\n".join("⚠ " + a for a in avisos))

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
        minimo, maximo = rango_de(nombre)
        if not minimo - 1e-6 <= float(nuevo) <= maximo + 1e-6:
            self._aviso("%s va de %g a %g: ya está en la punta." % (nombre, minimo, maximo), NARANJA)
            return
        self._mandar_valor(nombre, nuevo)

    def _poner(self, nombre):
        fila = self.filas.get(nombre)
        if not fila or fila["entrada"] is None:
            return
        escrito = str(fila["entrada"].get())
        valor = normalizar_entrada(escrito)
        if valor is None:
            self._aviso("'%s' no es un número. Escribí por ejemplo 7 o 0.5" % escrito.strip()[:20], ROJO)
            return
        fila["entrada"].delete(0, "end")
        self._mandar_valor(nombre, valor)

    def _mandar_valor(self, nombre, valor):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return False
        if self._mandar("=%s %s" % (nombre, valor)):
            self._aviso("Mandé %s = %s. Esperando que el robot confirme..." % (nombre, valor), GRIS)
            return True
        return False

    def _mandar(self, texto, urgente=False):
        """El ÚNICO lugar por donde sale algo hacia el robot. Solo al arquero,
        solo ASCII de 71 caracteres o menos, y mientras se mueve solo lo
        urgente (PARAR, TOCÓ). Mientras se CARGA un programa, nada."""
        if self.cargando:
            self._aviso("Se está cargando un programa: no le mando nada al robot. Si se mueve, levantalo o "
                        "apagá la batería.", NARANJA)
            return False
        if not self.conectado:
            self._aviso("No hay robot conectado.", NARANJA)
            return False
        if not self.es_arquero:
            self._aviso("Este robot NO es el arquero: la ventana no le manda nada.", ROJO)
            return False
        try:
            texto.encode("ascii")
        except UnicodeEncodeError:
            self._aviso("Al robot solo se le manda sin tildes ni eñes.", ROJO)
            return False
        if len(texto) > LARGO_MAX_ORDEN or "\n" in texto or "\r" in texto:
            self._aviso("Esa orden es demasiado larga para el robot (%d caracteres como mucho)." % LARGO_MAX_ORDEN, ROJO)
            return False
        if self.estado.modo in ("CUENTA", "MOVIENDO") and not urgente:
            self._aviso("Mientras cuenta o se mueve, solo se manda PARAR (Esc).", NARANJA)
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
        orden = "!%d%s" % (n, " c" if self.sin_cable else "")
        if self._mandar(orden):
            self.t_pedido = time.monotonic()
            # Una prueba de MOVIMIENTOS no es el IDA del paso 5 (2026-11-08):
            # si quedó uno viejo en el aire, que su #FIN 10 no se anote con esos ms.
            self.cancha["ida_en_curso"] = None
            if self.sin_cable:
                self.estado.que_sin_cable = "mov"
                self._aviso("Mandé %s (%s) SIN CABLE: cuenta 10 s. DESENCHUFÁ EL USB. %s" % (
                    orden, MOV_POR_NUMERO[n]["nombre"], SIN_CABLE_NO_LLEGA), VIOLETA)
            else:
                self._aviso("Mandé %s (%s): arranca en 3 s. Para frenarlo: PARAR o Esc." % (
                    orden, MOV_POR_NUMERO[n]["nombre"]), NARANJA)
            self._pintar_cuenta()
            self._actualizar_habilitados()

    def _parar(self):
        if (self.conectado and not self.es_arquero) or (not self.conectado and self.situacion == "otro"):
            self._aviso("Este no es el arquero: la ventana no le manda nada. Para pararlo, apagá la batería.", ROJO)
            return
        if not self.conectado:
            self._aviso(SIN_CABLE_NO_LLEGA, ROJO)
            return
        # En AJUSTAR-MOVIMIENTOS la x NO frena (vuelve sus números a los del
        # partido y escribe la EEPROM): ahí frena el 0. En el tablero, la x.
        # Si NO se sabe qué programa es (2026-11-03, revisión): un ESPACIO.
        # Ajustar lo toma como PARAR y no toca sus números; este tablero y el
        # del delantero frenan con cualquier tecla mientras se mueven (y en
        # espera el espacio no hace nada); el partido no lee el cable. Una x
        # acá, si era ajustar, le pisaba los números y su EEPROM.
        # (Si volvió el cable con el robot andando suelto, es este tablero: x.)
        es_tablero = (self._habla_tablero() or self._cortado_andando()
                      or self.programa_detectado in ("TABLERO_ARQUERO", "TABLERO_DELANTERO"))
        if self.programa_detectado == "AJUSTAR":
            tecla = "0"
        elif es_tablero:
            tecla = "x"
        else:
            tecla = " "
        if self._mandar(tecla, urgente=True):
            if es_tablero or self.programa_detectado == "AJUSTAR":
                self._aviso("PARAR: mandé %s." % tecla, ROJO)
            else:
                self._aviso("Mandé un espacio (frena sin tocar ningún número), pero OJO: " + NO_OBEDECE_PARAR, ROJO)

    def _copiar_panel(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if self._mandar("v"):
            self.t_pedido_panel = time.monotonic()
            self._aviso("Pidiendo las líneas para el panel...", GRIS)

    def _numeros_de_la_0921(self):
        """Lo que hay que mandarle al TABLERO, después del '0', para que tenga
        los números de la 0921 (la de jugar): [(nombre, texto para el robot)].
        '0' lo deja con lo compilado en tablero.ino (copias de la 0.1103); acá
        van los que la 0921 (el ARCHIVO, no el editor) tiene distinto, y los
        de COMO_LA_0921. Solo los que entran en el rango del tablero."""
        if self.lectura is None:
            return []
        salida = []
        for f in TABLA:
            nombre = f[0]
            if nombre not in self.lectura["valores"] and nombre not in COMO_LA_0921:
                continue
            try:
                hoy = _numero_del_panel(self._valor_de_hoy(nombre)[0])
                compilado = _numero_del_panel(self.estado.defectos.get(nombre, f[2]))
            except (TypeError, ValueError):
                continue
            if abs(hoy - compilado) <= 1e-6 or not f[3] - 1e-9 <= hoy <= f[4] + 1e-9:
                continue
            salida.append((nombre, ("1" if hoy >= 0.5 else "0") if f[1] == "b" else _g(float(hoy))))
        return salida

    def _volver_numeros(self):
        """(2026-11-08, revisión) Antes mandaba solo '0', que deja las copias
        de la 0.1103 (pwmMinLateral 45, msAdelante 383...) y los avisos de
        CANCHA no se iban nunca. Ahora, después del '0', le manda los de la
        0921 que son distintos."""
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        cambian = self._numeros_de_la_0921()
        lista = (": " + ", ".join("%s %s" % c for c in cambian)) if cambian else ""
        if not messagebox.askyesno(
                BOTON_NUMEROS_PARTIDO,
                "¿Volver TODOS los números del robot a los del PARTIDO (la 0921, como está guardada en el "
                "archivo)?\nPrimero vuelven a los del TABLERO y después le mando los de la 0921 que son "
                "distintos%s.\nSe pierde lo que cambiaron acá (si sirvió, pasalo a la pestaña PARTIDO)." % lista,
                parent=self.raiz):
            return
        if not self._mandar("0"):
            return
        for nombre, valor in cambian:
            if not self._mandar("=%s %s" % (nombre, valor)):
                break
        self._aviso("Pedí volver a los números del partido (la 0921)%s." % lista, GRIS)

    def _pedir_ultimo(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        if self._mandar("r"):
            self.t_pedido_repetir = time.monotonic()
            self._aviso("Le pedí al robot el último resultado que guardó.", GRIS)

    def _volver_al_partido(self):
        """VOLVER AL PARTIDO (2026-11-08): ya NO lanza CARGAR-PARTIDO.bat (ese
        cargaba la 0.1103 de seguir-y-despejar, que el equipo ya no usa). Va
        a la pestaña PARTIDO y hace lo mismo que CARGAR EL PARTIDO: carga la
        0921 con los valores que están escritos ahí, con su ventanita antes.
        El control del Teensy en MODO CARGA (y del delantero) lo hace el
        cargador en su paso 1 (quien)."""
        self._ir_a_pestana("partido")
        if self.demo:
            self._aviso("En la demo no se carga nada (mirá la pestaña PARTIDO).", VIOLETA)
            return False
        puede, motivo = self._puede_cargar("partido")
        if not puede:
            self._aviso(TEXTO_VOLVER_AL_PARTIDO + "  Ahora no se puede: " + motivo, NARANJA)
            return False
        self._aviso(TEXTO_VOLVER_AL_PARTIDO, AZUL)
        return self._cargar_partido()

    def _mostrar_lineas(self, titulo, mensaje, lineas):
        texto = "\n".join(lineas)
        v = tk.Toplevel(self.raiz)
        v.title(titulo)
        v.configure(bg=FONDO)
        v.transient(self.raiz)
        self._etiqueta(v, text=mensaje, font=F_MEDIO, wraplength=900).pack(fill="x", padx=14, pady=(12, 6))
        caja = tk.Text(v, font=F_CONSOLA, height=max(3, min(22, len(lineas) + 1)), width=110, bg="white",
                       wrap="none")
        caja.insert("1.0", texto)
        caja.pack(fill="both", expand=True, padx=14)
        fila = tk.Frame(v, bg=FONDO)
        fila.pack(fill="x", padx=14, pady=10)
        if lineas:
            ttk.Button(fila, text="Copiar de nuevo",
                       command=lambda: self._copiar_texto(texto + "\n")).pack(side="left")
        ttk.Button(fila, text="Cerrar", command=v.destroy).pack(side="right")

    def _mostrar_panel(self, lineas):
        if lineas:
            copiado = self._copiar_texto("\n".join(lineas) + "\n")
            mensaje = (PANEL_COPIADO if copiado else PANEL_NO_COPIADO) + " " + INSTRUCCIONES_PANEL
            self._aviso("Copié %d línea%s para el PANEL DE CONTROL." % (len(lineas), "" if len(lineas) == 1 else "s"),
                        VERDE)
        else:
            mensaje = ("No hay nada para copiar: los números que existen en el programa de partido "
                       "están todos igual que allá. (Los que son solo de esta prueba no se copian.)")
            self._aviso("Nada para copiar al panel.", GRIS)
        self._mostrar_lineas("Para pegar en el PANEL DE CONTROL", mensaje, lineas)

    def _copiar_texto(self, texto):
        try:
            self.raiz.clipboard_clear()
            self.raiz.clipboard_append(texto)
            return True
        except Exception:
            return False

    def _pintar_cuenta(self):
        e = self.estado
        fuente = F_MEDIO_B
        if e.modo == "CUENTA":
            if e.cuenta_modo == "sincable":
                texto, color, fuente = "DESENCHUFÁ EL USB AHORA: arranca en %.0f s" % (e.cuenta or 0), VIOLETA, F_GRANDE
            else:
                texto, color = "Arranca en %.1f s...  (PARAR o Esc para cancelar)" % (e.cuenta or 0), NARANJA
        elif e.modo == "MOVIENDO":
            texto, color = "¡SE ESTÁ MOVIENDO!  (PARAR o Esc para frenarlo)", ROJO
        elif self._cortado_andando():
            texto, color, fuente = self._texto_cortado_andando(), ROJO, F_GRANDE
        elif not self.conectado and e.esperando_sin_cable:
            texto, color = ("SIN CABLE: el robot está haciendo la prueba solo. Enchufalo cuando termine "
                            "(sin apagar la batería). " + SIN_CABLE_NO_LLEGA), VIOLETA
        elif self._pedido_pendiente():
            texto, color = "Pedido... esperando que el robot cuente.", GRIS
        else:
            se_puede, motivo = self._puede()
            if not se_puede:
                # Un botón apagado no explica nada al apretarlo: el motivo va acá.
                texto, color = motivo, NARANJA
            elif e.freno and e.informe is None and not e.tanda_cancha:
                texto, color = "Se frenó: %s" % e.freno, ROJO
                if "giroscopio" in e.freno:
                    texto += ". " + AYUDA_SIN_GIRO[0].upper() + AYUDA_SIN_GIRO[1:]
            elif self.sin_cable:
                texto, color = "Listo. Al apretar PROBAR cuenta 10 s para desenchufar.", VIOLETA
            else:
                texto, color = "Listo. Al apretar PROBAR cuenta 3 s y arranca.", "#37474f"
        en_cancha = self.w.get("cuenta") if hasattr(self, "w") else None
        if en_cancha is not None:
            if texto.startswith("Listo"):
                en_cancha.configure(text="Listo para medir." + (" SIN CABLE: cuenta 10 s para desenchufar."
                                                                if self.sin_cable else ""), fg="#37474f")
            else:
                en_cancha.configure(text=texto, fg=color)
        self.lbl_cuenta.configure(text=texto, fg=color, font=fuente)

    def _cortado_andando(self):
        return self.t_cortado_andando is not None and time.monotonic() < self.t_cortado_andando

    def _texto_cortado_andando(self):
        """Se cortó el cable CON CABLE mientras contaba o andaba: el robot no
        se entera y SIGUE solo (no hay PARAR que llegue sin cable)."""
        quedan = max(0, int(round((self.t_cortado_andando or 0) - 5.0 - time.monotonic())))
        if self.conectado:
            return ("Volvió el cable, pero el robot puede SEGUIR ANDANDO solo (hasta %d s más): si se mueve, "
                    "PARAR ya llega. Cuando termine, 'Pedir el último resultado' muestra cómo le fue." % quedan)
        return ("¡SE CORTÓ EL CABLE CON EL ROBOT ANDANDO! No se entera: SIGUE SOLO hasta terminar (hasta %d s). "
                "PARAR no llega: levantalo o apagá la batería." % quedan)

    def _resultado_mov(self):
        """El resultado que se muestra en MOVIMIENTOS: el de ahora si es de un
        movimiento, o el último movimiento si lo último fue de CANCHA."""
        e = self.estado
        if e.codigo in CODIGOS_CANCHA:
            return self.ultimo_mov
        return e

    def _llego_fin(self, codigo):
        e = self.estado
        if codigo.isdigit():
            self.ultimo_mov = copy.copy(e)
            n = int(codigo)
            if n == 10 and self.cancha.get("ida_en_curso") is not None:
                ms = self.cancha["ida_en_curso"]
                self.cancha["ida_en_curso"] = None
                if e.informe is not None and e.informe.get("nota") != "NO_VALE":
                    self.cancha["idas"].append([ms, None])
                    self._asegurar_hoja()
                    hoja_agregar(self.hoja, "DESPEJE", "ida", "ms=%d cm=?" % ms, time.strftime("%H:%M"))
                    self._escribir_hoja()
                    self._aviso("Medí con regla cuánto llegó la ida de %d ms y anotalo en CANCHA (paso 5)." % ms, AZUL)
                self.pendientes.add("cancha")
            if n == 2:
                # Con qué lateralInvertido corrió (para el 6a): el de #USADOS si
                # lo cambiaron, si no el del partido (#DEF). Sin #USADOS, el de ahora.
                if e.usados is not None:
                    v = e.usados.get("lateralInvertido", e.defectos.get("lateralInvertido",
                                                                        defecto_de("lateralInvertido")))
                    try:
                        self.cancha["lateral_probado"] = 1 if float(v) >= 0.5 else 0
                    except (TypeError, ValueError):
                        self.cancha["lateral_probado"] = self._lateral_de_ahora()
                else:
                    self.cancha["lateral_probado"] = self._lateral_de_ahora()
                if self.cancha.get("signo_en_curso"):
                    self.cancha["signo_en_curso"] = False
                self.pendientes.add("cancha")
            if n == 9:
                self.respuesta_pelota = None

    def _pintar_resultado(self):
        r = self._resultado_mov()
        if r is None:
            self.lbl_res_titulo.configure(text="RESULTADO")
            self.lbl_guardado.configure(text="")
            self.lbl_nota.configure(text="-", bg="#bdbdbd", fg="white")
            self.lbl_nota_txt.configure(text="La última fue una medición de CANCHA (mirala en esa pestaña).")
            self._mostrar_marco_9(False)
            self._dibujar_grafico()
            return
        inf = r.informe
        nombre = (inf or {}).get("mov") or r.mov_nombre
        self.lbl_res_titulo.configure(text="RESULTADO" + ("  ·  " + nombre.replace("_", " ") if nombre else ""))
        guardado = ""
        if r.de_guardado:
            guardado = "Resultado guardado (SIN CABLE)" if r.sincable else "Resultado guardado (pedido con r)"
            if r.apagado:
                guardado += "   ⚠ el robot se apagó: muestras raleadas y consejos con los números de ahora"
        self.lbl_guardado.configure(text=guardado)
        if inf is not None:
            fondo, letra, leida, explicacion = NOTAS[inf["nota"]]
        elif r.modo in ("CUENTA", "MOVIENDO") and r is self.estado:
            fondo, letra, leida, explicacion = "#9e9e9e", "white", "midiendo...", "El gráfico y la nota llegan cuando termina."
        elif r.freno:
            fondo, letra, leida, explicacion = "#9e9e9e", "white", "FRENADO", "Se frenó (%s) antes de moverse: no hay informe." % r.freno
            if "giroscopio" in r.freno:
                explicacion += " Sin giroscopio: " + AYUDA_SIN_GIRO
        else:
            fondo, letra, leida, explicacion = "#bdbdbd", "white", "-", "Todavía no hay resultado. Elegí un movimiento y apretá PROBAR."
        self.lbl_nota.configure(text=leida, bg=fondo, fg=letra)
        self.lbl_nota_txt.configure(text=explicacion)
        es_9 = (inf or {}).get("n") == 9 or r.codigo == "9"
        self._mostrar_marco_9(es_9)
        if es_9:
            self._pintar_intentos(r.intentos)

        t = self.txt_detalle
        t.configure(state="normal")
        t.delete("1.0", "end")
        if r.freno:
            t.insert("end", "Se frenó: %s\n" % r.freno, "malo")
        if r.usados:
            partes = ["%s=%s (el partido: %s)" % (k, v, (r.defectos or {}).get(k) or defecto_de(k) or "?")
                      for k, v in r.usados.items()]
            t.insert("end", "Se corrió con: " + ", ".join(partes) + "\n", "titulo")
        if inf is not None:
            t.insert("end", "Lo que midió:\n", "titulo")
            for frase, estilo in explicar_informe(inf, r.intentos, r.valores, r.muestras):
                t.insert("end", "•  " + frase + "\n", estilo)
        if r.consejos:
            t.insert("end", "\nQué tocar:\n", "titulo")
            for consejo in r.consejos:
                t.insert("end", "→  " + consejo + "\n", "consejo")
        t.configure(state="disabled")
        self._dibujar_grafico()

    def _mostrar_marco_9(self, si):
        if si:
            self.marco_9.pack(fill="x", pady=2, before=self.txt_detalle.master)
        else:
            self.marco_9.pack_forget()

    def _pintar_intentos(self, intentos):
        t = self.txt_intentos
        t.configure(state="normal")
        t.delete("1.0", "end")
        t.insert("end", "%-2s %6s  %-36s %5s %6s %6s %6s %7s %7s\n" % (
            "k", "t ms", "resultado", "d", "y", "vy", "va", "ángulo", "costado"))
        for i in intentos or ():
            # El robot solo calcula el ángulo si sale o si quedó MUY_ABIERTO
            # (en lo demás manda ang=0.0, que no es "salir derecho").
            ang = ("%+.1f°" % i["ang"] if i.get("res") in RES_CON_ANGULO and i.get("ang") is not None else "-")
            fmt = lambda v: "-" if v is None else "%.1f" % v
            t.insert("end", "%-2s %6s  %-36s %5s %6s %6s %6s %7s %7s\n" % (
                i.get("k"), i.get("t"), PALABRAS_MIRE.get(i.get("res"), i.get("res"))[:36], fmt(i.get("d")),
                fmt(i.get("y")), fmt(i.get("vy")), fmt(i.get("va")), ang,
                "-" if not i.get("lat") else "%+d" % i["lat"]))
        if not intentos:
            t.insert("end", "(todavía no hubo intentos)\n")
        t.configure(state="disabled")

    def _pregunta_pelota(self, clave):
        """La pregunta de la 9: '¿qué pasó con la pelota?'. Solo dice qué
        tocar: NO cambia ningún número sola."""
        texto = consejo_pelota(clave, self.estado.valores)
        nombre = dict(RESPUESTAS_PELOTA).get(clave, clave)
        self.respuesta_pelota = (clave, texto)
        self.lbl_respuesta.configure(text=texto)
        self._consola_agregar("[ATAJAR] ¿qué pasó con la pelota? %s -> %s" % (nombre, texto), "sistema")
        if self.hoja is not None:
            hoja_agregar(self.hoja, "NOTAS", "pelota_9", nombre, texto)
            self._escribir_hoja()
        self._aviso(texto, AZUL)

    def _dibujar_grafico(self):
        c = self.cv_grafico
        c.delete("all")
        w = max(int(c.winfo_width()), 320)
        h = max(int(c.winfo_height()), 160)
        r = self._resultado_mov()
        muestras = r.muestras if r is not None else []
        if not muestras:
            c.create_text(w / 2.0, h / 2.0, fill=GRIS, font=F_NORMAL,
                          text="Acá va el gráfico de cuánto se torció, cuando termine una prueba.")
            return
        ts = [float(m[0]) for m in muestras]
        gs = desenrollar([float(m[1]) for m in muestras])
        fases = [m[2] for m in muestras]
        tmax = max(max(ts), 1.0)
        tope = escala_grados(max(abs(g) for g in gs))
        izq, der, arr, aba = 62, 14, 22, 26
        ancho, alto = w - izq - der, h - arr - aba

        def X(t):
            return izq + t / tmax * ancho

        def Y(g):
            return arr + (tope - g) / (2.0 * tope) * alto

        # la franja "prolija": hasta 4 grados para cada lado
        c.create_rectangle(izq, Y(min(4, tope)), izq + ancho, Y(-min(4, tope)), fill="#e8f5e9", outline="")
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
        # una línea vertical punteada donde cambia de fase
        for i in range(1, len(fases)):
            if fases[i] != fases[i - 1]:
                c.create_line(X(ts[i]), arr, X(ts[i]), arr + alto, fill="#b0bec5", dash=(3, 3))
        # la curva, un tramo de color por fase
        tramo = [X(ts[0]), Y(gs[0])]
        fase = fases[0]
        for i in range(1, len(ts)):
            tramo += [X(ts[i]), Y(gs[i])]
            if fases[i] != fase or i == len(ts) - 1:
                c.create_line(*tramo, fill=FASES.get(fase, ("", "black"))[1], width=3)
                tramo = [X(ts[i]), Y(gs[i])]
                fase = fases[i]
        if len(ts) == 1:
            c.create_oval(X(ts[0]) - 3, Y(gs[0]) - 3, X(ts[0]) + 3, Y(gs[0]) + 3,
                          fill=FASES.get(fase, ("", "black"))[1], outline="")
        # los intentos de la 9: un triangulito arriba
        for i in (r.intentos or ()):
            if i.get("t") is not None and 0 <= i["t"] <= tmax:
                x = X(i["t"])
                salio = i.get("res") in ("SALE_MIRANDO", "SALE_SIN_MIRAR")
                c.create_polygon(x, arr, x - 5, arr - 8, x + 5, arr - 8, fill=VERDE if salio else ROJO, outline="")
        # la leyenda, arriba a la derecha (las fases que aparecen)
        x = izq + ancho - 8
        vistas = []
        for f in fases:
            if f not in vistas:
                vistas.append(f)
        for f in reversed(vistas):
            nombre, color = FASES.get(f, (f, "black"))
            c.create_text(x, arr + 8, anchor="e", text=nombre, font=F_CHICO, fill=color)
            x -= 7 * len(nombre) + 8
            c.create_line(x, arr + 8, x + 14, arr + 8, fill=color, width=4)
            x -= 14

    # ------------------------------------------------------------ CANCHA

    def _armar_cancha(self):
        t = self.tab_cancha
        t.columnconfigure(0, weight=0)
        t.columnconfigure(1, weight=1)
        t.rowconfigure(0, weight=1)
        izq = tk.Frame(t, bg=FONDO)
        izq.grid(row=0, column=0, sticky="ns", padx=(6, 4), pady=6)
        self._etiqueta(izq, text="Preparar una cancha nueva (sin internet):", font=F_MEDIO_B).pack(fill="x")
        self._etiqueta(izq, text="paso                          HOY            MEDIDO AHORA", font=F_CHICO,
                       fg="#616161").pack(fill="x")
        self.filas_cancha = {}
        for clave, titulo, nombre, _ in PASOS_CANCHA:
            fila = tk.Frame(izq, bg=FONDO)
            fila.pack(fill="x", pady=1)
            luz = tk.Label(fila, text="●", font=F_GRANDE, bg=FONDO, fg=COLOR_SEMAFORO["gris"])
            luz.pack(side="left")
            b = ttk.Button(fila, text=titulo, width=24, command=lambda c=clave: self._elegir_paso(c))
            b.pack(side="left")
            hoy = tk.Label(fila, text="", font=F_CHICO, bg=FONDO, width=16, anchor="w", justify="left")
            hoy.pack(side="left", padx=4)
            medido = tk.Label(fila, text="", font=F_NORMAL_B, bg=FONDO, width=8, anchor="w")
            medido.pack(side="left")
            self.filas_cancha[clave] = {"luz": luz, "boton": b, "hoy": hoy, "medido": medido}
        self.der_cancha = tk.Frame(t, bg=FONDO)
        self.der_cancha.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
        abajo = tk.Frame(t, bg="#e3f2fd")
        abajo.grid(row=1, column=0, columnspan=2, sticky="ew", padx=6, pady=(0, 6))
        self._etiqueta(abajo, text="HOJA DE CANCHA:", font=F_NORMAL_B, bg="#e3f2fd").pack(side="left", padx=6)
        self._etiqueta(abajo, text="nombre", font=F_CHICO, bg="#e3f2fd").pack(side="left")
        self.ent_cancha = ttk.Entry(abajo, width=22, font=F_NORMAL)
        self.ent_cancha.pack(side="left", padx=4)
        ttk.Button(abajo, text="Nueva", command=self._hoja_nueva).pack(side="left", padx=2)
        ttk.Button(abajo, text="Abrir", command=self._hoja_abrir).pack(side="left", padx=2)
        ttk.Button(abajo, text="Comparar", command=self._comparar).pack(side="left", padx=2)
        self.lbl_hoja = self._etiqueta(abajo, text="(ninguna abierta: se crea sola con la primera medición)",
                                       font=F_CHICO, bg="#e3f2fd")
        self.lbl_hoja.pack(side="left", padx=8, fill="x", expand=True)
        self.btn_reponer = ttk.Button(abajo, text="Volver a poner los números de la hoja", command=self._reponer_de_la_hoja)
        self.btn_reponer.pack(side="left", padx=4)
        ttk.Button(abajo, text="TERMINAR", style="Usar.TButton", command=self._terminar_cancha).pack(side="right", padx=6)
        ttk.Button(abajo, text="PASAR A PARTIDO →", style="Usar.TButton", command=self._partido_pasar).pack(
            side="right", padx=6)

    def _bloqueado(self, paso):
        bloquea = next((p[3] for p in PASOS_CANCHA if p[0] == paso), False)
        return bloquea and not (self.cancha["umbral_hecho"] or self.cancha["umbral_confirmado"])

    def _boton_cancha(self, padre, texto, comando, paso, robot=True, estilo=None, **pack):
        b = ttk.Button(padre, text=texto, command=comando, style=estilo) if estilo else \
            ttk.Button(padre, text=texto, command=comando)
        b.pack(**(pack or {"side": "left", "padx": 3, "pady": 2}))
        self.botones_cancha.append((b, paso, robot))
        return b

    def _elegir_paso(self, clave):
        self.paso_cancha = clave
        for c, fila in self.filas_cancha.items():
            fila["boton"].configure(style="Elegido.TButton" if c == clave else "TButton")
        for hijo in self.der_cancha.winfo_children():
            hijo.destroy()
        self.botones_cancha = []
        self.w = {}
        d = self.der_cancha
        titulo = next(p[1] for p in PASOS_CANCHA if p[0] == clave)
        self._etiqueta(d, text=titulo, font=F_GRANDE).pack(fill="x")
        self.w["que"] = self._etiqueta(d, font=F_MEDIO, wraplength=820)
        self.w["que"].pack(fill="x", pady=(2, 6))
        self.w["botones"] = tk.Frame(d, bg=FONDO)
        self.w["botones"].pack(fill="x")
        # La cuenta y "¡SE ESTÁ MOVIENDO!" también acá: se mide desde esta pestaña.
        self.w["cuenta"] = self._etiqueta(d, font=F_MEDIO_B, wraplength=820)
        self.w["cuenta"].pack(fill="x", pady=(2, 2))
        getattr(self, "_armar_paso_" + clave.lower())(d)
        if self._bloqueado(clave):
            fila = tk.Frame(d, bg="#fff3e0")
            fila.pack(fill="x", pady=4)
            self._etiqueta(fila, bg="#fff3e0", fg=NARANJA, font=F_NORMAL_B,
                           text="Bloqueado hasta hacer el paso 1 (la línea): los pasos 2, 3 y 5 buscan las líneas "
                                "con umbralBlanco.").pack(side="left", padx=6)
            ttk.Button(fila, text="Usar el umbral de hoy (%s)" % self._valor_de_hoy("umbralBlanco")[0],
                       command=self._confirmar_umbral_de_hoy).pack(side="left", padx=6)
        self.w["resultado"] = tk.Text(d, font=F_NORMAL, wrap="word", height=9, bg="white", relief="flat",
                                      padx=8, pady=6)
        self.w["resultado"].pack(fill="both", expand=True, pady=4)
        for tag, color in (("bueno", VERDE), ("malo", ROJO), ("amarillo", "#f57f17"), ("titulo", "#212121")):
            self.w["resultado"].tag_configure(tag, foreground=color, font=F_NORMAL_B)
        fila = tk.Frame(d, bg=FONDO)
        fila.pack(fill="x", pady=4)
        self.w["sugerido"] = self._etiqueta(fila, font=F_GRANDE, fg=AZUL)
        self.w["sugerido"].pack(side="left")
        self.w["usar"] = self._boton_cancha(fila, "USAR ESTE", self._usar_sugerido, clave, estilo="Usar.TButton",
                                            side="left", padx=8)
        self.w["copiar"] = self._boton_cancha(fila, "COPIAR AL PANEL", self._copiar_sugerido, clave, robot=False,
                                              side="left", padx=4)
        self._pintar_cancha()
        self._pintar_cuenta()
        self._actualizar_habilitados()

    def _fila_sin_cable(self, padre):
        ttk.Checkbutton(padre, text="SIN CABLE (10 s para desenchufar)", variable=self.var_sin_cable,
                        command=self._cambio_sin_cable).pack(side="left", padx=10)

    def _armar_paso_0(self, d):
        self.w["que"].configure(text="Con el robot en el piso de la cancha, batería prendida ANTES que el USB. "
                                     "Mirá que todo diga SÍ.")
        b = self.w["botones"]
        self._boton_cancha(b, "Buscar el giroscopio de nuevo (g)", self._buscar_giro, "0")
        self.var_bateria = tk.BooleanVar(value=self.cancha["bateria"])
        ttk.Checkbutton(b, text="Prendí la batería ANTES que el USB", variable=self.var_bateria,
                        command=self._tilde_bateria).pack(side="left", padx=10)

    def _armar_paso_1(self, d):
        self.w["que"].configure(text="1) Apoyalo en el VERDE y MEDIR VERDE (se puede repetir en 2 o 3 lugares).\n"
                                     "2) El sensor de atrás IZQUIERDO bien sobre la línea blanca: MEDIR BLANCO IZQ.\n"
                                     "3) El DERECHO sobre la línea: MEDIR BLANCO DER.   (O todo SIN CABLE, guiado "
                                     "por el LED: 1, 2 y 3 destellos; fijo = midiendo.)")
        b = self.w["botones"]
        self._boton_cancha(b, "MEDIR VERDE", lambda: self._medir_cancha("!L v", ("L", "v")), "1")
        self._boton_cancha(b, "MEDIR BLANCO IZQ", lambda: self._medir_cancha("!L i", ("L", "i")), "1")
        self._boton_cancha(b, "MEDIR BLANCO DER", lambda: self._medir_cancha("!L d", ("L", "d")), "1")
        self._boton_cancha(b, "TODO SIN CABLE (guiado por el LED)", lambda: self._medir_cancha("!L c", ("L", "c")), "1")
        self._boton_cancha(b, "Borrar lo medido", self._borrar_lineas, "1", robot=False)
        self.w["cv"] = tk.Canvas(d, bg="white", height=120, highlightthickness=0)
        self.w["cv"].pack(fill="x", pady=4)

    def _armar_paso_2(self, d):
        self.w["que"].configure(text="Apoyalo en la MITAD de la cancha, mirando la cancha, a la altura en que juega "
                                     "(justo delante de la línea del área), con los sensores fuera de las líneas, y "
                                     "apretá MEDIR. Hacé 2 corridas. (Opcional: lo mismo mirando al OTRO arco.)")
        b = self.w["botones"]
        self._boton_cancha(b, "MEDIR", lambda: self._medir_cancha("!A", ("A",), True), "2")
        self._fila_sin_cable(b)
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        self._etiqueta(f, text="¿Quedó en el medio?", font=F_NORMAL_B).pack(side="left")
        self._boton_cancha(f, "SÍ", lambda: self._en_el_medio("anchos", "SI"), "2", robot=False)
        self.w["cm"] = ttk.Entry(f, width=5, font=F_NORMAL)
        self.w["cm"].pack(side="left", padx=4)
        self._etiqueta(f, text="cm corrido a la").pack(side="left")
        self._boton_cancha(f, "IZQ", lambda: self._en_el_medio("anchos", "IZQ"), "2", robot=False)
        self._boton_cancha(f, "DER", lambda: self._en_el_medio("anchos", "DER"), "2", robot=False)

    def _armar_paso_2c(self, d):
        self.w["que"].configure(text="Para comprobar MS_MEDIO_AL_MEDIO: el robot hace la rutina de ir al medio del "
                                     "partido (lateral, fondo, adelante, lateral, al medio, atrás al área). Apoyalo "
                                     "delante del área, con lugar, y apretá COMPROBAR.")
        b = self.w["botones"]
        self._boton_cancha(b, "COMPROBAR (ir al medio)", lambda: self._medir_cancha("!I", ("I",), True), "2c")
        self._fila_sin_cable(b)
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        self._etiqueta(f, text="¿Quedó en el medio?", font=F_NORMAL_B).pack(side="left")
        self._boton_cancha(f, "SÍ", lambda: self._en_el_medio("medios", "SI"), "2c", robot=False)
        self.w["cm"] = ttk.Entry(f, width=5, font=F_NORMAL)
        self.w["cm"].pack(side="left", padx=4)
        self._etiqueta(f, text="cm corrido a la").pack(side="left")
        self._boton_cancha(f, "IZQ", lambda: self._en_el_medio("medios", "IZQ"), "2c", robot=False)
        self._boton_cancha(f, "DER", lambda: self._en_el_medio("medios", "DER"), "2c", robot=False)

    def _armar_paso_3(self, d):
        self.w["que"].configure(text="Apoyalo junto al lateral IZQUIERDO, en el lugar MÁS ADELANTADO donde lo podrían "
                                     "apoyar, apretá MEDIR y apretá TOCÓ cuando toque la rampa o la pared. 2 veces. "
                                     "(COMPROBAR SIN CABLE: retrocede el MS_MEDIO_FONDO_POR_TIEMPO de ahora.)")
        b = self.w["botones"]
        self._boton_cancha(b, "MEDIR (con cable)", lambda: self._medir_cancha("!F", ("F", "medir")), "3")
        self._boton_cancha(b, "COMPROBAR SIN CABLE", lambda: self._medir_cancha("!F c", ("F", "comprobar")), "3")
        self.w["toco"] = ttk.Button(b, text="TOCÓ", style="Toco.TButton", command=self._toco)
        self.w["toco"].pack(side="left", padx=12)
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        self._etiqueta(f, text="Al comprobar: ¿llegó a tocar la rampa?", font=F_NORMAL_B).pack(side="left")
        self._boton_cancha(f, "SÍ", lambda: self._toco_comprobar("SI"), "3", robot=False)
        self._boton_cancha(f, "NO", lambda: self._toco_comprobar("NO"), "3", robot=False)

    def _armar_paso_4(self, d):
        self.w["que"].configure(text="4a CONTROL RÁPIDO: pelota en una cinta a 30 cm del FRENTE del chasis y MEDIR; "
                                     "después sin pelota (no tiene que ver nada naranja).\n4b TABLA: apretá cada "
                                     "botón con la pelota YA puesta a esa distancia (así no se corre la tabla, 25/08). "
                                     "Ojo: cambiarlo cambia TODOS los cm del programa, y kpLateral está pensado "
                                     "para 2.87.")
        b = self.w["botones"]
        self._boton_cancha(b, "4a: PELOTA A 30 cm", lambda: self._medir_cancha("!C", ("C", "4a", 30)), "4")
        self._boton_cancha(b, "4a: SIN PELOTA", lambda: self._medir_cancha("!C", ("C", "4a", 0)), "4")
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        for cm in (10, 20, 30, 40, 50):
            self._boton_cancha(f, "4b: %d cm" % cm, lambda c=cm: self._medir_cancha("!C", ("C", "4b", c)), "4")
        self._boton_cancha(f, "Borrar la tabla", self._borrar_camara, "4", robot=False)

    def _armar_paso_5(self, d):
        self.w["que"].configure(text="Cinta de largada en el frente del chasis. Después de cada ida, medí con regla "
                                     "hasta el frente del chasis, escribí los cm, ANOTAR, y volvé a ponerlo en la "
                                     "cinta. (Atajo si no hay tiempo: pelota quieta a 30 cm: ¿la toca y la saca? "
                                     "Si no, subir msAdelante de a 30.) La ida termina SOLTANDO los motores, como "
                                     "la 0921 (la ventana le pone frenarAntesDeVolver 0).")
        b = self.w["botones"]
        for ms in (250, 383, 500):
            self._boton_cancha(b, "IDA %d ms" % ms, lambda x=ms: self._ida(x), "5")
        self._fila_sin_cable(b)
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        self._etiqueta(f, text="Llegó a", font=F_NORMAL_B).pack(side="left")
        self.w["cm"] = ttk.Entry(f, width=6, font=F_NORMAL)
        self.w["cm"].pack(side="left", padx=4)
        self._etiqueta(f, text="cm").pack(side="left")
        self._boton_cancha(f, "ANOTAR", self._anotar_ida, "5", robot=False)
        self._etiqueta(f, text="   Quiero que llegue a", font=F_NORMAL_B).pack(side="left")
        self.w["quiero"] = ttk.Entry(f, width=6, font=F_NORMAL)
        self.w["quiero"].pack(side="left", padx=4)
        self._etiqueta(f, text="cm").pack(side="left")
        self._boton_cancha(f, "Recalcular", self._pintar_cancha, "5", robot=False)

    def _armar_paso_6(self, d):
        self.w["que"].configure(text="6a: el movimiento 2 (costado DERECHA): ¿fue a la DERECHA?\n6b: poné la pelota "
                                     "a la DERECHA del robot y apretá ANOTAR: el Yp crudo tiene que ser negativo "
                                     "(camaraYInvertida = 1, como el 11/08 con Yp = -24).")
        b = self.w["botones"]
        self._boton_cancha(b, "6a: PROBAR el 2 (DERECHA)", self._signo_probar, "6")
        self._boton_cancha(b, "Fue a la DERECHA", lambda: self._signo_lateral(True), "6", robot=False)
        self._boton_cancha(b, "Fue a la IZQUIERDA", lambda: self._signo_lateral(False), "6", robot=False)
        f = tk.Frame(d, bg=FONDO)
        f.pack(fill="x")
        self._boton_cancha(f, "6b: ANOTAR (la pelota está a la DERECHA)", self._signo_camara, "6", robot=False)
        self.w["yp"] = self._etiqueta(f, font=F_MEDIO_B)
        self.w["yp"].pack(side="left", padx=8)

    def _armar_paso_r(self, d):
        self.w["que"].configure(text="Lo que NO se mide con el tablero: mirarlo a ojo en la cancha. Los tildes se "
                                     "guardan en la hoja.")
        self.vars_revisado = []
        for i, texto in enumerate(REVISAR_A_OJO):
            var = tk.BooleanVar(value=i in self.cancha["revisado"])
            ttk.Checkbutton(d, text=texto, variable=var, command=lambda k=i: self._tilde_revisado(k)).pack(anchor="w")
            self.vars_revisado.append(var)

    # ---- lo que pasa al apretar ----

    def _confirmar_umbral_de_hoy(self):
        hoy = self._valor_de_hoy("umbralBlanco")[0]
        if messagebox.askyesno("Usar el umbral de hoy",
                               "¿Seguir sin medir la línea, con umbralBlanco = %s?\nSi la cancha es otra, los "
                               "pasos 2 y 3 pueden no ver las líneas." % hoy, parent=self.raiz):
            self.cancha["umbral_confirmado"] = True
            self._elegir_paso(self.paso_cancha)

    def _medir_cancha(self, orden, pedido, admite_sin_cable=False):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return False
        if self._bloqueado(self.paso_cancha):
            self._aviso("Primero el paso 1 (la línea), o 'Usar el umbral de hoy'.", NARANJA)
            return False
        if admite_sin_cable and self.sin_cable:
            orden += " c"
        if not self._mandar(orden):
            return False
        self.t_pedido = time.monotonic()
        self.pedido_cancha = pedido
        if orden.endswith(" c"):
            self.estado.que_sin_cable = "cancha"
            self._aviso("Mandé %s SIN CABLE: cuenta 10 s. DESENCHUFÁ EL USB. %s" % (orden, SIN_CABLE_NO_LLEGA), VIOLETA)
        else:
            self._aviso("Mandé %s." % orden, GRIS)
        return True

    def _toco(self):
        """TOCÓ: en el fondo, la letra t frena y anota el tiempo."""
        e = self.estado
        if not (self.conectado and e.modo == "MOVIENDO" and e.codigo == "F"):
            self._aviso("TOCÓ sirve solo mientras retrocede midiendo el fondo.", NARANJA)
            return
        if self._mandar("t", urgente=True):
            self._aviso("TOCÓ: mandé t.", AZUL)

    def _ida(self, ms):
        """Paso 5: manda =msPruebaIda X y, cuando llega el #OK de ESE número,
        manda !10 (ver _ok_llego)."""
        if self._bloqueado("5"):
            self._aviso("Primero el paso 1 (la línea), o 'Usar el umbral de hoy'.", NARANJA)
            return
        if valor_num(self.estado.valores, "frenarAntesDeVolver") >= 0.5:
            # (2026-11-08, revisión) La 0921 termina la ida SOLTANDO los
            # motores (parar(), sigue un poco de largo); el tablero, frenando.
            # Si se mide frenando, el msAdelante que sale es de más.
            se_puede, motivo = self._puede()
            if not se_puede:
                self._aviso(motivo, NARANJA)
                return
            if not self._mandar("=frenarAntesDeVolver 0"):
                return
        if self._mandar_valor("msPruebaIda", str(ms)):
            # Con la HORA (2026-11-03, revisión): un pedido viejo, cuyo
            # =msPruebaIda se perdió con el cable, no puede mandar !10 solo
            # mucho después, cuando alguien toque msPruebaIda con + y -.
            self.cancha["ida_espera_ok"] = (ms, time.monotonic())
            self.cancha["ida_sin_cable"] = self.sin_cable

    def _olvidar_ida(self):
        """El IDA del paso 5 que estaba en el aire ya no vale (se cortó o
        volvió el cable, llegó un #FRENO o un #ERROR, o no contestó): que no
        mande !10 solo, ni anote la ida con ms viejos. Si se espera una
        prueba SIN CABLE, la ida en curso SÍ sigue (es esa)."""
        self.cancha["ida_espera_ok"] = None
        if not self.estado.esperando_sin_cable:
            self.cancha["ida_en_curso"] = None

    def _ok_llego(self, m):
        espera = self.cancha.get("ida_espera_ok")
        if espera is None or m["nombre"] != "msPruebaIda":
            return
        self.cancha["ida_espera_ok"] = None
        ms, t_pedido = espera
        # Solo el #OK que contesta ESE pedido: reciente y con el paso 5 a la vista.
        if time.monotonic() - t_pedido > ESPERA_OK_IDA or self.paso_cancha != "5":
            return
        try:
            igual = abs(float(m["valor"]) - ms) < 0.5
        except ValueError:
            igual = False
        if not igual:
            return
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        sin = self.cancha.get("ida_sin_cable")
        orden = "!10 c" if sin else "!10"
        if self._mandar(orden):
            self.t_pedido = time.monotonic()
            self.cancha["ida_en_curso"] = ms
            if sin:
                self.estado.que_sin_cable = "mov"

    def _anotar_ida(self):
        cm = normalizar_entrada(str(self.w["cm"].get())) if "cm" in self.w else None
        if cm is None:
            self._aviso("Escribí los cm que llegó la ida (por ejemplo 33).", ROJO)
            return
        sin_cm = [i for i in self.cancha["idas"] if i[1] is None]
        if not sin_cm:
            self._aviso("No hay ninguna ida sin anotar: primero apretá IDA.", NARANJA)
            return
        sin_cm[0][1] = float(cm)
        try:
            self.w["cm"].delete(0, "end")
        except Exception:
            pass
        self._asegurar_hoja()
        hoja_agregar(self.hoja, "DESPEJE", "medida", "ms=%d cm=%s" % (sin_cm[0][0], cm), time.strftime("%H:%M"))
        self._actualizar_sugeridos_hoja()
        self._escribir_hoja()
        self._pintar_cancha()

    def _signo_probar(self):
        se_puede, motivo = self._puede()
        if not se_puede:
            self._aviso(motivo, NARANJA)
            return
        orden = "!2 c" if self.sin_cable else "!2"
        if self._mandar(orden):
            self.cancha["signo_en_curso"] = True
            self.cancha["lateral_probado"] = self._lateral_de_ahora()    # con este corre (el #FIN lo confirma)
            self.t_pedido = time.monotonic()
            if self.sin_cable:
                self.estado.que_sin_cable = "mov"

    def _lateral_de_ahora(self):
        return 1 if valor_num(self.estado.valores, "lateralInvertido") >= 0.5 else 0

    def _signo_lateral(self, a_la_derecha):
        # Contra el lateralInvertido CON EL QUE CORRIÓ el 2 (2026-11-03,
        # revisión): moverDeCostado lo da vuelta si está en 1. Si fue a la
        # derecha, ese valor está bien; si fue a la izquierda, va el otro.
        # Antes daba siempre 0 = derecha: al repetirlo con el 1 ya puesto,
        # sugería volver al valor malo.
        con = self.cancha.get("lateral_probado")
        if con is None:
            con = self._lateral_de_ahora()
        self.cancha["lateral"] = con if a_la_derecha else 1 - con
        self.cancha["lateral_con"] = con
        self.cancha["lateral_derecha"] = bool(a_la_derecha)
        self._asegurar_hoja()
        hoja_poner(self.hoja, "SIGNOS", "lateralInvertido", str(self.cancha["lateral"]),
                   "el 2 fue a la %s con lateralInvertido=%d" % ("DERECHA" if a_la_derecha else "IZQUIERDA", con))
        self._escribir_hoja()
        self._pintar_cancha()

    def _signo_camara(self):
        s = self.estado.sensores
        if not s or not visible(s.get("p")) or not s.get("ve"):
            self._aviso("La cámara no ve la pelota: ponela a la DERECHA del robot, a ~40 cm.", NARANJA)
            return
        yp = s["p"][1]
        self.cancha["yp"] = yp
        self._asegurar_hoja()
        hoja_poner(self.hoja, "SIGNOS", "camaraYInvertida", "1" if yp < 0 else "0",
                   "pelota a la derecha: Yp = %d" % yp)
        self._escribir_hoja()
        self._pintar_cancha()

    def _en_el_medio(self, lista, respuesta):
        corridas = self.cancha[lista]
        if not corridas:
            self._aviso("Primero medí.", NARANJA)
            return
        if respuesta == "SI":
            texto = "SI"
        else:
            cm = normalizar_entrada(str(self.w["cm"].get())) if "cm" in self.w else None
            if cm is None:
                self._aviso("Escribí cuántos cm quedó corrido.", ROJO)
                return
            texto = "%s_%scm" % (respuesta, cm)
        corridas[-1]["en_el_medio"] = texto
        self._asegurar_hoja()
        seccion = "ANCHO"
        base = "corrida" if lista == "anchos" else "ir_al_medio"
        clave = "%s_%d_en_el_medio" % (base, len(corridas))
        hoja_poner(self.hoja, seccion, clave, texto, time.strftime("%H:%M"))
        self._escribir_hoja()
        self._pintar_cancha()

    def _toco_comprobar(self, respuesta):
        if not self.cancha["comprobar"]:
            self._aviso("Primero COMPROBAR SIN CABLE.", NARANJA)
            return
        self.cancha["comprobar"][-1]["toco"] = respuesta
        self._asegurar_hoja()
        hoja_agregar(self.hoja, "FONDO", "comprobado", "ms=%d toco=%s" % (self.cancha["comprobar"][-1]["ms"], respuesta),
                     time.strftime("%H:%M"))
        self._escribir_hoja()
        self._pintar_cancha()

    def _borrar_lineas(self):
        self.cancha["lineas"] = []
        self._pintar_cancha()

    def _borrar_camara(self):
        self.cancha["cam_4b"] = {}
        self._pintar_cancha()

    def _tilde_bateria(self):
        try:
            self.cancha["bateria"] = bool(self.var_bateria.get())
        except Exception:
            self.cancha["bateria"] = not self.cancha["bateria"]
        if self.hoja is not None:
            hoja_poner(self.hoja, "REVISADO", "bateria_antes_que_el_usb", "SI" if self.cancha["bateria"] else "NO")
            self._escribir_hoja()
        self._pintar_cancha()

    def _tilde_revisado(self, k):
        if k in self.cancha["revisado"]:
            self.cancha["revisado"].discard(k)
        else:
            self.cancha["revisado"].add(k)
        if self.hoja is not None:
            hoja_poner(self.hoja, "REVISADO", "a_ojo_%d" % (k + 1), "SI" if k in self.cancha["revisado"] else "NO",
                       REVISAR_A_OJO[k])
            self._escribir_hoja()
        self._pintar_pasos_lista()

    # ---- llegó una medición de cancha (en vivo o repetida con rc) ----

    def _llego_cancha(self, codigo, tanda):
        """Llegó una tanda de cancha completa. PRIMERO se escribe la hoja, y
        recién después se muestra (regla del delantero del 08/09: que lo
        medido no se pierda nunca)."""
        pedido = self.pedido_cancha
        if pedido is not None and pedido[0] == codigo:
            self.pedido_cancha = None
        else:
            pedido = None                            # (una vieja, repetida con rc: lo pedido sigue esperando)
        e = self.estado
        a_mano, self.rc_a_mano = self.rc_a_mano, False
        nombre = e.mov_nombre or codigo
        if e.de_guardado and e.apagado and not a_mano:
            # 2026-11-03 (revisión): guardada de ANTES DE APAGAR el robot. Puede
            # ser de otro día y de otra cancha (la de la escuela): mezclada con
            # la de hoy tuerce los sugeridos sin que nadie se entere. No se usa.
            self._consola_agregar("[%s: medición VIEJA, de antes de apagar el robot: NO se suma a la cancha]"
                                  % nombre, "sistema")
            self._aviso("Llegó una medición de %s guardada de ANTES DE APAGAR el robot (puede ser de otra "
                        "cancha): NO la sumo. Si es de hoy, medila de nuevo." % nombre, NARANJA)
            return
        if e.de_guardado and pedido is None and not a_mano:
            # Repetida sola con rc, pero esta ventana NO la pidió: es de antes
            # de abrirla (o ya se vio en vivo en otra ventana). No se cuenta
            # dos veces; si hace falta, se suma escribiendo rc en la CONSOLA.
            self._consola_agregar("[%s: medición guardada que esta ventana no pidió: NO se suma (para sumarla, "
                                  "escribí rc)]" % nombre, "sistema")
            self._aviso("Llegó una medición de %s guardada de antes de abrir la ventana: NO la sumo, para no "
                        "contarla dos veces. Si no está en la hoja, escribí rc en la CONSOLA." % nombre, NARANJA)
            return
        if e.de_guardado and e.apagado:
            self._aviso("Sumé una medición de %s de ANTES DE APAGAR el robot (la pediste con rc): fijate que "
                        "sea de esta cancha." % nombre, NARANJA)
        hora = time.strftime("%H:%M")
        self._asegurar_hoja()
        h = self.hoja
        for m in tanda:
            tipo = m["tipo"]
            if tipo == "LIN":
                self.cancha["lineas"].append(m)
                texto = "izq=%s der=%s ade=%s n=%d" % (",".join(_g1(x) if isinstance(x, float) else str(x) for x in m["izq"]),
                                                      ",".join(_g1(x) if isinstance(x, float) else str(x) for x in m["der"]),
                                                      ",".join(_g1(x) if isinstance(x, float) else str(x) for x in m["ade"]), m["n"])
                if m["fase"] == "v":
                    hoja_agregar(h, "LINEA", "verde", texto, hora)
                else:
                    hoja_poner(h, "LINEA", "blanco_izq" if m["fase"] == "i" else "blanco_der", texto, hora)
            elif tipo == "ANCHO":
                self.cancha["anchos"].append({"datos": m, "en_el_medio": None})
                texto = ("ok=%d error=%d medioizq=%s izqder=%s derizq=%s almedioizq=%s almedioder=%s desvio=%s "
                         "endereza=%s vel=%s rampa=%s umbral=%s" % (
                             m["ok"], m["error"], m["medioizq"], m["izqder"], m["derizq"],
                             m["almedioizq"], m["almedioder"],
                             ",".join(_g1(x) for x in (m["desvio"] or [])), m["endereza"],
                             m.get("vel"), m.get("rampa"), m.get("umbral")))
                hoja_agregar(h, "ANCHO", "corrida", texto, hora)
            elif tipo == "MEDIO":
                self.cancha["medios"].append({"datos": m, "en_el_medio": None})
                hoja_agregar(h, "ANCHO", "ir_al_medio", "ok=%d pasos=%s como=%s" % (
                    m["ok"], ",".join(str(p) for p in m["pasos"]), ",".join(m["como"])), hora)
            elif tipo == "FONDO":
                if m["como"] == "TIEMPO" or m.get("probado"):
                    self.cancha["comprobar"].append({"ms": m["ms"], "como": m["como"], "toco": None})
                    hoja_agregar(h, "FONDO", "comprobar", "ms=%d como=%s probado=%d" % (m["ms"], m["como"], m["probado"]), hora)
                else:
                    self.cancha["fondos"].append(m)
                    hoja_agregar(h, "FONDO", "corrida", "ms=%d como=%s desvio=%s" % (
                        m["ms"], m["como"], _g1(m["desvio"] or 0)), hora)
            elif tipo == "CAM":
                cual = pedido[1:] if pedido and len(pedido) >= 3 else ("4a", 30)
                if cual[0] == "4a":
                    self.cancha["cam_4a"]["con" if cual[1] else "sin"] = m
                    hoja_poner(h, "CAMARA", "a30cm" if cual[1] else "sin_pelota",
                               "val=%d prom=%s n=%d" % (m["val"], _g1(m["xp"][2]), m["n"]), hora)
                else:
                    self.cancha["cam_4b"][cual[1]] = m
                    hoja_poner(h, "CAMARA", "cm_%d" % cual[1], "val=%d prom=%s n=%d" % (m["val"], _g1(m["xp"][2]), m["n"]), hora)
        self._actualizar_sugeridos_hoja()
        self._escribir_hoja()                        # ANTES de mostrar
        self.pendientes.add("cancha")

    def _sugeridos(self):
        """{número: (valor sugerido como texto, avisos [(texto, nivel)])} con lo medido hasta ahora."""
        V = lambda n: valor_num(self.estado.valores, n)
        c = self.cancha
        salida = {}
        r = calcular_umbral(c["lineas"])
        salida["umbralBlanco"] = (str(r["umbral"]) if r["umbral"] is not None else None, r["avisos"], r)
        anchos = [a["datos"] for a in c["anchos"]]
        try:
            rampa_partido = int(_numero_del_panel(self._valor_de_hoy("RAMPA_MOV_PASO")[0]))
        except (TypeError, ValueError):
            rampa_partido = None
        sug, dif, aviso = sugerido_ancho(anchos, rampa_partido)
        avisos = [(aviso, "amarillo")] if aviso else []
        fuera = self._aviso_numeros(NUMEROS_DEL_MOVIMIENTO)
        if fuera:
            avisos.append(fuera)
        for a in anchos:
            if a.get("izqder") and FIJOS["MS_MEDIO_TOPE_LATERAL"] < 1.3 * a["izqder"]:
                avisos.append(("MS_MEDIO_TOPE_LATERAL (6000) es menos que 1,3 veces el cruce (%d): avisar." % a["izqder"],
                               "amarillo"))
                break
        salida["MS_MEDIO_AL_MEDIO"] = (str(sug) if sug else None, avisos, dif)
        fondo = sugerido_fondo(c["fondos"])
        avisos = []
        fuera = self._aviso_numeros(NUMEROS_DEL_MOVIMIENTO)
        if fuera:
            avisos.append(fuera)
        if any(f["como"] == "LINEA" for f in c["fondos"] + c["comprobar"]):
            avisos.append(("Esta cancha tiene línea de fondo alcanzable.", "amarillo"))
            tocos = [f["ms"] for f in c["fondos"] if f["como"] == "TOCO"]
            if tocos and any(f["como"] == "LINEA" and f["ms"] < max(tocos) for f in c["fondos"]):
                avisos.append(("Frenó por una línea ANTES del fondo: hay otra línea en el camino y la rutina del "
                               "partido cortaría ahí.", "malo"))
        salida["MS_MEDIO_FONDO_POR_TIEMPO"] = (str(fondo) if fondo else None, avisos, None)
        avisos = []
        con = c["cam_4a"].get("con")
        sin = c["cam_4a"].get("sin")
        if con is not None:
            prom = con["xp"][2]
            cm, factor = prom / V("CAMARA_POR_CM"), prom / 30.0
            nivel = "bueno" if 27 <= cm <= 33 else "malo"
            avisos.append(("4a: a 30 cm ve %.1f (son %.1f cm con CAMARA_POR_CM %.2f; tiene que dar 27 a 33). "
                           "Factor %.2f." % (prom, cm, V("CAMARA_POR_CM"), factor), nivel))
            if not 2.6 <= factor <= 3.2:
                avisos.append(("El factor está fuera de 2,6..3,2: hacé la tabla 4b.", "malo"))
        if sin is not None:
            avisos.append(("Sin pelota: %s" % ("no ve nada naranja, bien." if sin["val"] == 0 else
                                               "VE ALGO NARANJA (%d paquetes): sacarlo o taparlo." % sin["val"]),
                           "bueno" if sin["val"] == 0 else "malo"))
        tabla = sorted((cm, m["xp"][2]) for cm, m in c["cam_4b"].items() if cm and m["val"] > 0)
        sugerido_cam = None
        if len(tabla) >= 2:
            ab = recta([t[0] for t in tabla], [t[1] for t in tabla])
            if ab:
                sugerido_cam = "%.2f" % ab[0]
                avisos.append(("4b: recta cámara = %.2f·cm %+.2f." % ab, "bueno"))
                factores = [p / cm for cm, p in tabla]
                if factores[-1] - factores[0] > 0.1 * (sum(factores) / len(factores)):
                    avisos.append(("El factor crece con la distancia: inclinación, no alcanza con dividir. Avisar.", "amarillo"))
        elif con is not None and 2.6 <= con["xp"][2] / 30.0 <= 3.2:
            sugerido_cam = None                      # con la 4a sola no se cambia: está bien
        salida["CAMARA_POR_CM"] = (sugerido_cam, avisos, tabla)
        avisos = []
        idas = [(ms, cm) for ms, cm in c["idas"] if cm is not None]
        quiero = None
        if "quiero" in self.w:
            quiero = normalizar_entrada(str(self.w["quiero"].get()))
        quiero = float(quiero) if quiero else V("umbralCm") + 5
        sugerido_ida = None
        if len(idas) >= 2:
            ab = recta([i[0] for i in idas], [i[1] for i in idas])
            if ab:
                ms = ms_para_cm(ab[0], ab[1], quiero)
                if ms is not None and 100 <= ms <= 800:
                    sugerido_ida = str(ms)
                avisos.append(("Recta: cm = %.3f·ms %+.2f. Para llegar a %g cm: %s ms." % (
                    ab[0], ab[1], quiero, ms if ms is not None else "?"), "bueno"))
                llega = ab[0] * V("msAdelante") + ab[1]
                if llega <= V("umbralCm"):
                    avisos.append(("Con msAdelante %d la ida llega a %.0f cm: NO pasa de umbralCm (%g)." % (
                        V("msAdelante"), llega, V("umbralCm")), "malo"))
        if any(cm is None for _, cm in c["idas"]):
            avisos.append(("Falta anotar los cm de una ida.", "amarillo"))
        fuera = self._aviso_numeros(NUMEROS_DE_LA_IDA)
        if fuera:
            avisos.append(fuera)
        salida["msAdelante"] = (sugerido_ida, avisos, idas)
        salida["lateralInvertido"] = (None if c["lateral"] is None else str(c["lateral"]), [], None)
        salida["camaraYInvertida"] = (None if c["yp"] is None else ("1" if c["yp"] < 0 else "0"), [], None)
        return salida

    def _actualizar_sugeridos_hoja(self):
        if self.hoja is None:
            return
        s = self._sugeridos()
        r = s["umbralBlanco"][2]
        if r["umbral"] is not None:
            hoja_poner(self.hoja, "LINEA", "umbral", str(r["umbral"]), "peorVerde %s, peorBlanco %s, margen %s/%s" % (
                r["peor_verde"], r["peor_blanco"], r["margen_verde"], r["margen_blanco"]))
        for nombre, seccion in (("MS_MEDIO_AL_MEDIO", "ANCHO"), ("MS_MEDIO_FONDO_POR_TIEMPO", "FONDO"),
                                ("CAMARA_POR_CM", "CAMARA"), ("msAdelante", "DESPEJE")):
            if s[nombre][0] is not None:
                hoja_poner(self.hoja, seccion, "sugerido", s[nombre][0])

    def _aviso_numeros(self, nombres):
        """(aviso, 'amarillo') si el robot tiene AHORA alguno de esos números
        distinto del partido (con eso mide CANCHA), o None."""
        fuera = numeros_fuera_del_partido(self.estado.valores, {n: self._valor_de_hoy(n)[0] for n in nombres},
                                          nombres)
        if not fuera:
            return None
        return ("El robot tiene AHORA números que NO son los del partido (la 0921): %s. Lo que midas vale para "
                "ESOS números: apretá '%s' (en MOVIMIENTOS: le pone los de la 0921) y medí de nuevo." % (
                    ", ".join("%s %s (partido %s)" % f for f in fuera), BOTON_NUMEROS_PARTIDO), "amarillo")

    def _valor_de_hoy(self, nombre):
        """El VALOR DE HOY: PRIMERO el del programa con el que se juega (la
        0921, el de la pestaña PARTIDO; 2026-11-08), después el del panel de
        la 0.1103 (con su etiqueta [MEDIDO ...]), el de #DEF o el de la tabla."""
        lectura = getattr(self, "lectura", None)
        if lectura is not None and nombre in lectura["valores"]:
            m = re.search(r"\[[^\]]*\]", lectura["restos"].get(nombre, ""))
            return lectura["valores"][nombre], (m.group(0) if m else "(la 0921)")
        if lectura is not None and nombre in COMO_LA_0921:
            return COMO_LA_0921[nombre]
        if nombre in self.panel_partido:
            decl, valor, comentario, _ = self.panel_partido[nombre]
            m = re.search(r"\[[^\]]*\]", comentario)
            return valor, (m.group(0) if m else "")
        if nombre in self.estado.defectos:
            return self.estado.defectos[nombre], "(copia del tablero)"
        return defecto_de(nombre) or "?", "(la tabla)"

    def _estado_paso(self, clave, sugeridos):
        c = self.cancha
        if clave == "0":
            chequeos = self._chequeos()
            if not chequeos or all(ok is None for _, ok in chequeos):
                return "gris"
            if any(ok is False for _, ok in chequeos):
                return "rojo"
            return "amarillo" if any(ok == "amarillo" or ok is None for _, ok in chequeos) else "verde"
        if clave == "R":
            return "gris" if not c["revisado"] else ("verde" if len(c["revisado"]) == len(REVISAR_A_OJO) else "amarillo")
        if clave == "2c":
            if not c["medios"]:
                return "gris"
            return "verde" if c["medios"][-1]["datos"]["ok"] else "rojo"
        nombre = next(p[2] for p in PASOS_CANCHA if p[0] == clave)
        valor, avisos, _ = sugeridos[nombre]
        hay_datos = {"1": c["lineas"], "2": c["anchos"], "3": c["fondos"] + c["comprobar"],
                     "4": list(c["cam_4a"].values()) + list(c["cam_4b"].values()), "5": c["idas"],
                     "6": [x for x in (c["lateral"], c["yp"]) if x is not None]}[clave]
        if not hay_datos:
            return "gris"
        niveles = {a[1] for a in avisos}
        if "rojo" in niveles or "malo" in niveles:
            return "rojo" if valor is None else "amarillo"
        if clave == "2" and c["anchos"] and not c["anchos"][-1]["datos"]["ok"]:
            return "rojo"
        if valor is None and clave != "4":
            return "amarillo"
        return "amarillo" if "amarillo" in niveles else "verde"

    def _chequeos(self):
        """El paso 0, ¿anda todo?: [(qué, True / False / 'amarillo' / None)]."""
        s = self.estado.sensores
        e = self.estado
        ok = []
        ok.append(("Programa tablero-arquero", self._programa_ok() if self.conectado else None))
        serie_robot = e.hola[2] if e.hola else None
        ok.append(("Serie %s (el arquero)" % SERIE_ARQUERO,
                   (self.es_arquero and serie_robot in (None, SERIE_ARQUERO)) if self.conectado else None))
        ok.append(("El giroscopio contesta", (s.get("r") is not None) if s else None))
        ok.append(("Fusión OK (estado 5)", (s.get("g") == 5) if s else None))
        cal = s.get("cal") if s else None
        ok.append(("Calibración del giro %s/3" % (cal[1] if cal else "?"),
                   None if not cal else (True if cal[1] == 3 else "amarillo")))
        q = s.get("q") if s else None
        ok.append(("Cámara: %s paquetes por segundo (16 o más)" % ("?" if q is None else "%g" % q),
                   None if q is None else (True if q >= 16 else ("amarillo" if q > 0 else False))))
        ok.append(("Ve la pelota", None if not s else bool(s.get("ve"))))
        estado = estado_linea(s.get("l"), s.get("u")) if s else None
        ok.append(("Sobre el verde: los 2 de atrás en VERDE (si no, el paso 1 es obligatorio)",
                   None if estado is None else estado == "-"))
        if s and s.get("l") and s.get("u") is not None and all(x is not None and x >= s["u"] for x in s["l"]):
            ok.append(("Los 3 en blanco: ¿está sobre la MESA?", False))
        if e.valores:
            fuera = numeros_fuera_del_partido(e.valores, {n: self._valor_de_hoy(n)[0] for n in NUMEROS_DE_LA_IDA},
                                              NUMEROS_DE_LA_IDA)
            ok.append(("Los números de moverse (rampa, mezcla, rumbo, ida) son los del partido" +
                       ("" if not fuera else ": NO, " + ", ".join("%s %s (partido %s)" % f for f in fuera) +
                        " → '%s' (en MOVIMIENTOS)" % BOTON_NUMEROS_PARTIDO),
                       True if not fuera else "amarillo"))
        ok.append(("Prendí la batería ANTES que el USB (a mano)", True if self.cancha["bateria"] else None))
        return ok

    def _pintar_pasos_lista(self, sugeridos=None):
        sugeridos = sugeridos or self._sugeridos()
        for clave, _, nombre, _ in PASOS_CANCHA:
            fila = self.filas_cancha[clave]
            fila["luz"].configure(fg=COLOR_SEMAFORO[self._estado_paso(clave, sugeridos)])
            if nombre:
                valor, etiqueta = self._valor_de_hoy(nombre)
                fila["hoy"].configure(text="%s %s" % (valor, etiqueta[:14]))
                medido = sugeridos[nombre][0]
                fila["medido"].configure(text=medido or "-")
            else:
                fila["hoy"].configure(text="")
                fila["medido"].configure(text="")

    def _pintar_cancha(self):
        sugeridos = self._sugeridos()
        self._pintar_pasos_lista(sugeridos)
        if self.hoja is not None and self.ruta_hoja:
            self.lbl_hoja.configure(text="abierta: %s" % os.path.basename(self.ruta_hoja))
        distintos = self._numeros_de_la_hoja_distintos()
        self.btn_reponer.configure(text="Volver a poner los %d números de la hoja" % len(distintos))
        se_puede = self._puede()[0]
        self._habilitar(self.btn_reponer, bool(distintos) and se_puede)
        if "resultado" not in self.w:
            return
        t = self.w["resultado"]
        t.configure(state="normal")
        t.delete("1.0", "end")
        clave = self.paso_cancha
        nombre = next(p[2] for p in PASOS_CANCHA if p[0] == clave)
        if clave == "0":
            for que, ok in self._chequeos():
                marca, tag = {True: ("SÍ ", "bueno"), False: ("NO ", "malo"), "amarillo": ("¿? ", "amarillo"),
                              None: (" - ", "")}[ok]
                t.insert("end", "%s  %s\n" % (marca, que), tag)
        elif clave == "1":
            r = sugeridos["umbralBlanco"][2]
            for m in self.cancha["lineas"]:
                t.insert("end", "%s: izq máx %s, der máx %s, adelante máx %s (n=%d)\n" % (
                    {"v": "VERDE", "i": "BLANCO IZQ", "d": "BLANCO DER"}[m["fase"]], m["izq"][1], m["der"][1],
                    m["ade"][1], m["n"]), "")
            if r["falta"]:
                t.insert("end", "Falta medir: %s\n" % ", ".join(r["falta"]), "amarillo")
            if r["umbral"] is not None:
                t.insert("end", "peorVerde %s, peorBlanco %s → umbral %d (margen %d del verde y %d del blanco)\n" % (
                    r["peor_verde"], r["peor_blanco"], r["umbral"], r["margen_verde"], r["margen_blanco"]), "bueno")
            self._dibujar_barras_linea(r)
        elif clave == "2":
            for k, a in enumerate(self.cancha["anchos"], 1):
                m = a["datos"]
                if not m["ok"]:
                    t.insert("end", "Corrida %d: NO SIRVE (error %d: %s)\n" % (k, m["error"], {
                        2: "arrancó sobre una línea", 3: "sin giroscopio", 4: "no encontró la línea",
                        5: "números raros", 6: "lo frenaron", 7: "tope total"}.get(m["error"], "?")), "malo")
                    continue
                peor = max(abs(x) for x in (m["desvio"] or [0]))
                tag = "bueno" if peor < 5 else ("amarillo" if peor <= 10 else "malo")
                asim = ((m["almedioder"] - m["almedioizq"]) / float(m["almedioizq"]) * 100) if m["almedioizq"] else 0
                t.insert("end", "Corrida %d: %d / %d / %d ms → al medio desde la IZQ %d ms (desde la DER %d, solo "
                                "dato; asimetría %.1f %%). Peor desvío %.1f°, enderezadas %s. En el medio: %s\n" % (
                                    k, m["medioizq"], m["izqder"], m["derizq"], m["almedioizq"], m["almedioder"],
                                    asim, peor, m["endereza"], a["en_el_medio"] or "?"), tag)
            dif = sugeridos["MS_MEDIO_AL_MEDIO"][2]
            if dif is not None and len([a for a in self.cancha["anchos"] if a["datos"]["ok"]]) > 1:
                t.insert("end", "Repetibilidad entre corridas: %.1f %%\n" % dif, "bueno" if dif <= 3 else "amarillo")
        elif clave == "2c":
            fuera = self._aviso_numeros(NUMEROS_DEL_MOVIMIENTO + ("MS_MEDIO_AL_MEDIO", "MS_MEDIO_FONDO_POR_TIEMPO"))
            if fuera:
                t.insert("end", "⚠ " + fuera[0] + "\n", "amarillo")
            for k, a in enumerate(self.cancha["medios"], 1):
                m = a["datos"]
                t.insert("end", "Vez %d: %s. Pasos %s (%s). En el medio: %s\n" % (
                    k, "terminó en la línea del área" if m["ok"] else "NO terminó bien",
                    ", ".join(str(p) for p in m["pasos"]), ",".join(m["como"]), a["en_el_medio"] or "?"),
                    "bueno" if m["ok"] else "malo")
                if m["pasos"][0] and m["pasos"][4] > 0.6 * m["pasos"][0]:
                    t.insert("end", "   El paso 5 (al lateral de nuevo) no tardó mucho menos que el 1: tendría que "
                                    "estar ya cerca del lateral.\n", "amarillo")
        elif clave == "3":
            for k, f in enumerate(self.cancha["fondos"], 1):
                t.insert("end", "Corrida %d: %d ms, %s\n" % (k, f["ms"], f["como"]), "bueno" if f["como"] == "TOCO" else "amarillo")
            for k, f in enumerate(self.cancha["comprobar"], 1):
                t.insert("end", "Comprobar %d: retrocedió %d ms (%s). ¿Tocó la rampa? %s\n" % (
                    k, f["ms"], f["como"], f.get("toco") or "?"), "")
        elif clave == "4":
            for cm, prom in sugeridos["CAMARA_POR_CM"][2]:
                t.insert("end", "4b: a %d cm ve %.1f (factor %.2f)\n" % (cm, prom, prom / cm), "")
        elif clave == "5":
            for ms, cm in self.cancha["idas"]:
                t.insert("end", "Ida de %d ms: %s\n" % (ms, "%g cm" % cm if cm is not None else "FALTA ANOTAR LOS cm"),
                         "" if cm is not None else "amarillo")
            if "quiero" in self.w and not str(self.w["quiero"].get()).strip():
                try:
                    self.w["quiero"].insert(0, "%g" % (valor_num(self.estado.valores, "umbralCm") + 5))
                except Exception:
                    pass
        elif clave == "6":
            s = self.estado.sensores
            lat = self.cancha["lateral"]
            if lat is not None:
                derecha = self.cancha.get("lateral_derecha", lat == 0)
                con = self.cancha.get("lateral_con", 0)
                t.insert("end", "6a: el 2 fue a la %s corriendo con lateralInvertido = %d → lateralInvertido = %d%s\n" % (
                    "DERECHA" if derecha else "IZQUIERDA", con, lat,
                    " (el que tenía: está bien)" if derecha else " (el otro: cambiarlo y probar el 2 de nuevo)"),
                    "bueno" if derecha else "amarillo")
            if s and visible(s.get("p")):
                yp = s["p"][1]
                self.w["yp"].configure(text="Yp crudo ahora: %d (%s)" % (
                    yp, "negativo: con la pelota a la derecha, camaraYInvertida = 1 está bien" if yp < 0 else
                    "positivo: con la pelota a la derecha, sería camaraYInvertida = 0"))
            if self.cancha["yp"] is not None:
                t.insert("end", "6b: pelota a la derecha, Yp = %d → camaraYInvertida = %s\n" % (
                    self.cancha["yp"], "1" if self.cancha["yp"] < 0 else "0"), "bueno")
        if nombre:
            _, avisos, _ = sugeridos[nombre]
            for texto, nivel in avisos:
                t.insert("end", "⚠ " + texto + "\n" if nivel in ("malo", "rojo", "amarillo") else texto + "\n",
                         {"rojo": "malo"}.get(nivel, nivel))
            valor = sugeridos[nombre][0]
            hoy = self._valor_de_hoy(nombre)
            if clave == "6":
                otro = sugeridos["camaraYInvertida"][0]
                texto = "lateralInvertido = %s   camaraYInvertida = %s" % (valor or "?", otro or "?")
            else:
                texto = "SUGERIDO: %s = %s   (hoy: %s %s)" % (nombre, valor or "?", hoy[0], hoy[1])
            self.w["sugerido"].configure(text=texto)
        else:
            self.w["sugerido"].configure(text="")
        t.configure(state="disabled")

    def _dibujar_barras_linea(self, r):
        c = self.w.get("cv")
        if c is None:
            return
        c.delete("all")
        w = max(int(c.winfo_width()), 500)
        h = max(int(c.winfo_height()), 110)
        izq, der = 120, 20
        ancho = w - izq - der
        X = lambda v: izq + min(max(v / 1023.0, 0.0), 1.0) * ancho
        verdes = [m for m in self.cancha["lineas"] if m["fase"] == "v"]
        filas = (("atrás IZQ", "izq", "i"), ("atrás DER", "der", "d"))
        for k, (nombre, clave, fase) in enumerate(filas):
            y0 = 12 + k * 40
            c.create_text(10, y0 + 10, anchor="w", text=nombre, font=F_NORMAL_B)
            c.create_rectangle(izq, y0, izq + ancho, y0 + 20, fill="#eceff1", outline="")
            if verdes:
                v = max(m[clave][1] for m in verdes)
                c.create_rectangle(izq, y0, X(v), y0 + 20, fill="#66bb6a", outline="")
            blancos = [m for m in self.cancha["lineas"] if m["fase"] == fase]
            if blancos:
                b = blancos[-1][clave][1]
                c.create_line(X(b), y0 - 3, X(b), y0 + 23, fill="#212121", width=3)
                c.create_text(X(b), y0 + 22, anchor="n", text="blanco %s" % b, font=F_CHICO)
        if r.get("umbral") is not None:
            c.create_line(X(r["umbral"]), 4, X(r["umbral"]), h - 4, fill="#e53935", width=3)
            c.create_text(X(r["umbral"]) + 4, h - 6, anchor="sw", text="umbral %d" % r["umbral"], fill="#e53935",
                          font=F_NORMAL_B)

    def _usar_sugerido(self):
        clave = self.paso_cancha
        nombre = next(p[2] for p in PASOS_CANCHA if p[0] == clave)
        if not nombre:
            return
        sugeridos = self._sugeridos()
        valor = sugeridos[nombre][0]
        if clave == "6" and valor is None:
            nombre, valor = "camaraYInvertida", sugeridos["camaraYInvertida"][0]
        if valor is None:
            self._aviso("Todavía no hay un valor medido para usar.", NARANJA)
            return
        self._usar_este(nombre, valor)
        if clave == "6" and sugeridos["camaraYInvertida"][0] is not None and nombre == "lateralInvertido":
            self._usar_este("camaraYInvertida", sugeridos["camaraYInvertida"][0])

    def _copiar_sugerido(self):
        clave = self.paso_cancha
        nombre = next(p[2] for p in PASOS_CANCHA if p[0] == clave)
        if not nombre:
            return
        sugeridos = self._sugeridos()
        valor = sugeridos[nombre][0]
        lineas = []
        if valor is not None:
            lineas.append(self._linea_para(nombre, valor))
        if clave == "6" and sugeridos["camaraYInvertida"][0] is not None:
            lineas.append(self._linea_para("camaraYInvertida", sugeridos["camaraYInvertida"][0]))
        if not lineas:
            self._aviso("Todavía no hay un valor medido para copiar.", NARANJA)
            return
        self._copiar_lineas(lineas, [nombre] + (["camaraYInvertida"] if clave == "6" else []))

    def _usar_este(self, nombre, valor):
        """USAR ESTE: manda =NOMBRE valor (en la RAM del robot) y lo anota en la hoja."""
        antes = self.estado.valores.get(nombre, defecto_de(nombre))
        if not self._mandar_valor(nombre, valor):
            return False
        if nombre == "umbralBlanco":
            self.cancha["umbral_hecho"] = True       # los pasos 2 y 3 buscan las líneas con el NUEVO
        self._asegurar_hoja()
        hoja_poner(self.hoja, "COPIADO AL PANEL", nombre, "%s -> %s" % (antes, valor), time.strftime("%d/%m %H:%M"))
        self._escribir_hoja()
        self.pendientes.add("cancha")
        return True

    def _etiqueta_medido(self):
        nombre = self._nombre_cancha() or (hoja_valor(self.hoja, "DATOS", "cancha") if self.hoja else "") or "cancha"
        nombre = re.sub(r"[\[\]\r\n]", "", nombre).strip()[:30]
        return "MEDIDO %s %s" % (time.strftime("%d/%m"), nombre)

    def _linea_para(self, nombre, valor, etiqueta=None):
        if nombre in self.panel_partido:
            decl, _, comentario, _ = self.panel_partido[nombre]
        else:
            decl, comentario = DECLARACION[nombre], COMENTARIOS_PANEL.get(nombre, "")
        return linea_del_panel(decl, nombre, valor_para_panel(nombre, valor), comentario,
                               etiqueta or self._etiqueta_medido())

    def _copiar_lineas(self, lineas, nombres):
        self.ultima_linea_copiada = lineas[-1]
        copiado = self._copiar_texto("\n".join(lineas) + "\n")
        a_mano = ["   %s: %s" % (n, COPIAS_A_MANO[n]) for n in nombres if n in COPIAS_A_MANO]
        # (2026-11-08, revisión) Para JUGAR esto no hace falta: la 0921 no tiene PANEL.
        mensaje = (PARA_JUGAR_NO_HACE_FALTA + "\n" + (PANEL_COPIADO if copiado else PANEL_NO_COPIADO) + "\n" +
                   INSTRUCCION_COPIAR_CANCHA)
        if a_mano:
            mensaje += "\nCOPIAS A MANO (la ventana NO toca esos archivos):\n" + "\n".join(a_mano)
        self._aviso("Copié la línea para el PANEL de la 0.1103 (NO es la de jugar). Para JUGAR: PASAR A PARTIDO →.",
                    AZUL)
        self._mostrar_lineas("COPIAR AL PANEL (0.1103)", mensaje, lineas)

    def _numeros_de_la_hoja_distintos(self):
        if self.hoja is None or not self.estado.valores:
            return []
        salida = []
        for nombre, valor, _ in self.hoja.get("COPIADO AL PANEL", []):
            if nombre not in NUMEROS or "->" not in valor:
                continue
            nuevo = valor.split("->", 1)[1].strip()
            try:
                if abs(float(nuevo) - valor_num(self.estado.valores, nombre)) > 0.001:
                    salida.append((nombre, nuevo))
            except ValueError:
                pass
        return salida

    def _reponer_de_la_hoja(self):
        """Al reconectar la ventana NO le manda sola los números al robot: los
        vuelve a poner solo si alguien aprieta este botón."""
        distintos = self._numeros_de_la_hoja_distintos()
        if not distintos:
            self._aviso("Los números del robot ya son los de la hoja.", GRIS)
            return
        for nombre, valor in distintos:
            if not self._mandar_valor(nombre, valor):
                break

    # ---- la hoja ----

    def _nombre_cancha(self):
        try:
            return str(self.ent_cancha.get()).strip()
        except Exception:
            return ""

    def _asegurar_hoja(self):
        if self.hoja is None:
            self._hoja_nueva()

    def _escribir_escuela(self):
        ruta = os.path.join(self.carpeta_canchas, NOMBRE_HOJA_ESCUELA)
        if not os.path.exists(ruta):
            escribir_atomico(ruta, HOJA_ESCUELA)

    def _hoja_nueva(self):
        nombre = self._nombre_cancha() or "cancha"
        limpio = re.sub(r"[^A-Za-z0-9_-]+", "-", nombre).strip("-")[:40] or "cancha"
        os.makedirs(self.carpeta_canchas, exist_ok=True)
        try:
            self._escribir_escuela()
        except OSError:
            pass
        base = os.path.join(self.carpeta_canchas, "%s_%s" % (time.strftime("%Y-%m-%d"), limpio))
        ruta, k = base + ".txt", 2
        while os.path.exists(ruta):                  # nunca se pisa una hoja vieja
            ruta, k = "%s_%d.txt" % (base, k), k + 1
        self.hoja = hoja_nueva(nombre, time.strftime("%Y-%m-%d %H:%M"), self.version_archivo or PARTIDO_COPIADO)
        self.ruta_hoja = ruta
        self._escribir_hoja()
        self._aviso("Hoja de cancha nueva: %s" % os.path.basename(ruta), AZUL)

    def _escribir_hoja(self):
        if self.hoja is None or not self.ruta_hoja:
            return
        try:
            escribir_atomico(self.ruta_hoja, hoja_a_texto(self.hoja))
        except OSError as e:
            self._aviso("No pude escribir la hoja de cancha: %s" % e, ROJO)
            return
        self.lbl_hoja.configure(text="abierta: %s" % os.path.basename(self.ruta_hoja))

    def _hoja_abrir(self):
        try:
            ruta = filedialog.askopenfilename(parent=self.raiz, initialdir=self.carpeta_canchas,
                                              title="Abrir una hoja de cancha",
                                              filetypes=[("Hojas de cancha", "*.txt")])
        except Exception:
            ruta = ""
        if not ruta:
            return
        if os.path.basename(str(ruta)) == NOMBRE_HOJA_ESCUELA:
            self._aviso("Esa es la de la escuela (de las bitácoras): solo para COMPARAR.", NARANJA)
            return
        try:
            with open(ruta, encoding="utf-8") as f:
                self.hoja = texto_a_hoja(f.read())
        except (OSError, UnicodeDecodeError) as e:
            self._aviso("No pude abrir esa hoja: %s" % e, ROJO)
            return
        self.ruta_hoja = str(ruta)
        self._aviso("Abrí la hoja %s" % os.path.basename(self.ruta_hoja), AZUL)
        self.pendientes.add("cancha")
        self._pintar_cancha()

    def _hojas_guardadas(self):
        hojas = [("escuela (bitácoras)", texto_a_hoja(HOJA_ESCUELA))]
        try:
            nombres = sorted(os.listdir(self.carpeta_canchas))
        except OSError:
            nombres = []
        for n in nombres:
            if not n.endswith(".txt") or n == NOMBRE_HOJA_ESCUELA:
                continue
            try:
                with open(os.path.join(self.carpeta_canchas, n), encoding="utf-8") as f:
                    hojas.append((n[:-4], texto_a_hoja(f.read())))
            except (OSError, UnicodeDecodeError):
                pass
        return hojas

    def _comparar(self):
        hojas = self._hojas_guardadas()
        filas = comparar_hojas(hojas)
        cabeza = "%-27s %-8s " % ("número", "HOY") + " ".join("%-14s" % n[:14] for n, _ in hojas)
        lineas = [cabeza, "-" * len(cabeza)]
        for nombre, valores in filas:
            hoy = self._valor_de_hoy(nombre)[0]
            lineas.append("%-27s %-8s " % (nombre, hoy) + " ".join("%-14s" % v for v in valores))
        v = tk.Toplevel(self.raiz)
        v.title("COMPARAR las canchas")
        v.configure(bg=FONDO)
        v.transient(self.raiz)
        caja = tk.Text(v, font=F_CONSOLA, height=len(lineas) + 2, width=120, bg="white", wrap="none")
        caja.insert("1.0", "\n".join(lineas))
        caja.pack(fill="both", expand=True, padx=12, pady=8)
        fila = tk.Frame(v, bg=FONDO)
        fila.pack(fill="x", padx=12, pady=6)
        for nombre, hoja in hojas:
            ttk.Button(fila, text="Volver a esta cancha: %s" % nombre[:24],
                       command=lambda h=hoja, n=nombre: self._volver_a_cancha(n, h)).pack(side="left", padx=2)
        ttk.Button(fila, text="Cerrar", command=v.destroy).pack(side="right")
        self.ultima_comparacion = lineas

    def _volver_a_cancha(self, nombre, hoja):
        """'Volver a esta cancha: X' (COMPARAR). (2026-11-08, revisión) Para
        JUGAR: abre la misma ventanita que PASAR A PARTIDO → con los números
        de ESA hoja (van al EDITOR de PARTIDO; después CARGAR EL PARTIDO). Ya
        no deja líneas para pegar a mano en el panel de la 0.1103."""
        fecha = (hoja_valor(hoja, "DATOS", "fecha") or "")[:10]
        fuente = "cancha %s%s" % (re.sub(r"[\[\]\r\n]", "", nombre)[:30], (" (" + fecha + ")") if fecha else "")
        if self.lectura is None:
            self._aviso("No pude leer el programa de partido: %s" % (self.error_lectura or "?"), ROJO)
            return None
        filas = self._medido_para_partido(hoja=hoja, fuente=fuente)
        if not filas:
            self._aviso("Esa hoja no tiene números para la 0921.", NARANJA)
            return None
        return self._dialogo_pasar(filas, titulo="Volver a la cancha %s (a PARTIDO)" % nombre[:30], origen="hoja")

    def _terminar_cancha(self):
        """TERMINAR: todas las líneas para el panel, las copias a mano y el último paso."""
        lineas, nombres = [], []
        for nombre, valor, _ in (self.hoja or {}).get("COPIADO AL PANEL", []):
            if nombre in NUMEROS and "->" in valor:
                lineas.append(self._linea_para(nombre, valor.split("->", 1)[1].strip()))
                nombres.append(nombre)
        a_mano = ["   %s: %s" % (n, COPIAS_A_MANO[n]) for n in nombres if n in COPIAS_A_MANO]
        mensaje = ("PARA JUGAR (la 0921): botón PASAR A PARTIDO →, revisá en la pestaña PARTIDO y CARGAR EL "
                   "PARTIDO (guarda con copia y el robot confirma con su línea de versión).\n"
                   "Abajo: las líneas de lo que se mandó con USAR ESTE, para el PANEL de la 0.1103 "
                   "(seguir-y-despejar, que ya no es la de jugar). " + INSTRUCCION_COPIAR_CANCHA)
        if a_mano:
            mensaje += "\nCOPIAS A MANO (la ventana NO toca esos archivos):\n" + "\n".join(a_mano)
        mensaje += ("\nÚLTIMO PASO: PASAR A PARTIDO → y, en la pestaña PARTIDO, CARGAR EL PARTIDO. Fijate que "
                    "diga LISTO (el robot dijo su versión y sus valores).")
        if lineas:
            self._copiar_texto("\n".join(lineas) + "\n")
        self._mostrar_lineas("TERMINAR la cancha", mensaje, lineas)

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

    def _consola_mandar(self, texto=None):
        escrito = texto is None
        texto = str(self.ent_consola.get() if escrito else texto).strip()
        if not texto:
            return
        # Con otro programa cargado, lo que se escribe puede ser una orden de
        # ese programa (en ajustar-movimientos, "1" arranca un movimiento y la
        # "x" pisa sus números). Solo se deja preguntar: "h" y "?".
        if texto not in ("h", "?") and not self._habla_tablero():
            self._aviso("La consola solo le habla al programa del tablero. Con otro programa, "
                        "solo se puede mandar ? y h.", NARANJA)
            return
        if self._mandar(texto):
            if escrito:
                self.ent_consola.delete(0, "end")
            if texto.startswith("!"):
                self.t_pedido = time.monotonic()
                if texto[1:2].upper() in ("L", "A", "F", "C", "I"):
                    self.pedido_cancha = (texto[1:2].upper(),)   # lo pidieron acá: cuando llegue, se suma
                if texto.rstrip().endswith("c") and len(texto) > 2:
                    self.estado.que_sin_cable = "cancha" if texto[1:2].upper() in "LAFCI" else "mov"
            if texto in ("r", "rc"):
                self.t_pedido_repetir = time.monotonic()
                self.rc_a_mano = (texto == "rc")

    # ------------------------------------------------------------ PARTIDO (2026-11-08)
    #  La pestaña para "chequear o cambiar los valores y cargarlo con esos
    #  nuevos de forma rápida" (pedido del equipo, 03/11). El EDITOR son los
    #  cambios pendientes (self.pendientes_partido, nombre -> texto): el
    #  archivo recién se escribe con GUARDAR o CARGAR, y siempre con copia.

    def _iniciar_partido(self, partido_carpeta):
        self.partido_carpeta = os.path.abspath(partido_carpeta or PARTIDO_CARPETA)
        self.copias_partido = COPIAS_PARTIDO
        if self.demo:
            # La demo trabaja sobre una COPIA de práctica: el de verdad no se toca.
            self.carpeta_demo = tempfile.mkdtemp(prefix="tablero-demo-partido-", dir=CARPETA_DEMO)
            real = os.path.join(self.partido_carpeta, NOMBRE_PARTIDO_INO)
            copia = os.path.join(self.carpeta_demo, "partido-0921")
            os.makedirs(copia, exist_ok=True)
            try:
                with open(real, "rb") as f:
                    datos = f.read()
                with open(os.path.join(copia, NOMBRE_PARTIDO_INO), "wb") as f:
                    f.write(datos)
            except OSError:
                pass
            self.partido_carpeta = copia
            self.copias_partido = os.path.join(self.carpeta_demo, "copias-partido")
        self.partido_ruta = os.path.join(self.partido_carpeta, NOMBRE_PARTIDO_INO)
        self.lectura = None
        self.error_lectura = None
        self.stat_partido = None
        self.hay_copias = False
        self.pendientes_partido = {}    # el EDITOR: nombre -> texto (solo lo que difiere del archivo)
        self.origen_partido = {}        # nombre -> de dónde vino ("CANCHA", "copia", "fábrica")
        # Lo último que se sabe del archivo y el robot (ver _texto_estado_partido):
        # sin_cambios / guardado / cargado / cargado_sin_confirmar / no_coincide /
        # a_medias / tablero / otro
        self.estado_archivo = "sin_cambios"
        self.estado_antes_de_carga = "sin_cambios"
        self.aviso_afuera = None        # "cambió por afuera" (naranja, arriba de la lista)
        self.t_mirar_partido = 0.0
        self.grupo_plegado = {k: plegado for k, _, plegado in GRUPOS_PARTIDO}
        self.filas_partido = {}
        self.cargando = False
        self.que_carga = None
        self.puerto_cerrado_por_carga = False
        self.cambios_en_carga = {}
        self.etapa_carga = ""
        self.log_carga = []
        self.resultado_carga = None     # (texto, color)
        self.esperando_version_hasta = None
        self.ultimo_partido = None      # la última #PARTIDO de ESTA conexión (o de la carga)
        self.t_fin_carga = None
        self._clave_habilitados = None
        self._releer_partido()

    def _stat_partido(self):
        try:
            s = os.stat(self.partido_ruta)
            return (s.st_mtime_ns, s.st_size)
        except OSError:
            return None

    def _releer_partido(self, por_afuera=False):
        """Lee el .ino (solo leer). por_afuera: cambió sin que lo tocara la
        ventana (¿Arduino IDE?): se dejan encima los cambios del editor cuyos
        nombres sigan existiendo y se avisa."""
        anterior = self.lectura
        try:
            lectura = leer_partido_archivo(self.partido_ruta)
            self.error_lectura = None
        except NoSeEscribe as e:
            lectura, self.error_lectura = None, str(e)
        except OSError as e:
            lectura, self.error_lectura = None, "no lo encuentro o no lo puedo abrir (%s)" % e.__class__.__name__
        self.lectura = lectura
        self.stat_partido = self._stat_partido()
        self.hay_copias = bool(copias_del_partido(self.copias_partido))
        if lectura is not None:
            for nombre in list(self.pendientes_partido):
                if nombre not in lectura["valores"]:
                    del self.pendientes_partido[nombre]
                elif mismo_valor(nombre, self.pendientes_partido[nombre], lectura["valores"][nombre]):
                    del self.pendientes_partido[nombre]
        if por_afuera:
            cambiados = []
            if anterior is not None and lectura is not None:
                cambiados = ["%s %s → %s" % (n, anterior["valores"][n], lectura["valores"].get(n, "?"))
                             for n in ORDEN_PARTIDO if n in anterior["valores"]
                             and anterior["valores"][n] != lectura["valores"].get(n)]
            detalle = (" (" + ", ".join(cambiados[:4]) + ("..." if len(cambiados) > 4 else "") + ")") if cambiados else ""
            if self.pendientes_partido:
                self.aviso_afuera = "Cambió mientras editabas%s: revisá y apretá de nuevo." % detalle
            else:
                self.aviso_afuera = "El programa cambió por afuera (¿Arduino IDE?): lo volví a leer%s." % detalle
            self.estado_archivo = "sin_cambios"
            self._aviso(self.aviso_afuera, NARANJA)
        self.pendientes.add("partido")

    def _releer_si_cambio(self):
        if self.cargando:
            return
        if self._stat_partido() != self.stat_partido:
            self._releer_partido(por_afuera=True)

    def _valores_partido(self):
        """Lo del archivo con los cambios del editor encima."""
        valores = dict(self.lectura["valores"]) if self.lectura else {}
        valores.update(self.pendientes_partido)
        return valores

    def _medido_de_cancha(self):
        """Lo medido en CANCHA que usan las reglas (O5, O16)."""
        medido = {}
        try:
            r = calcular_umbral(self.cancha["lineas"])
            if r.get("umbral") is not None:
                medido["peor_verde"], medido["peor_blanco"] = r.get("peor_verde"), r.get("peor_blanco")
        except Exception:
            pass
        cruces = [a["datos"].get("izqder") for a in self.cancha["anchos"] if a["datos"].get("izqder")]
        if cruces:
            medido["cruce"] = max(cruces)
        return medido

    def _reglas_de_ahora(self):
        if self.lectura is None:
            return []
        return _reglas_con_codigo(self._valores_partido(), self.lectura["valores"], self._medido_de_cancha())

    def _no_usables(self):
        lec = self.lectura
        if lec is None or lec["problemas"]:
            return set(ORDEN_PARTIDO)
        return set(lec["faltan"]) | set(lec["repetidos"]) | {r[0] for r in lec["raros"]}

    def _cambios_reales(self):
        """[(nombre, archivo, editor)] de lo que va a cambiar."""
        if self.lectura is None:
            return []
        return [(n, self.lectura["valores"][n], self.pendientes_partido[n]) for n in ORDEN_PARTIDO
                if n in self.pendientes_partido and n in self.lectura["valores"]]

    def _ruta_para_mostrar(self):
        if self.demo:
            return "COPIA DE PRÁCTICA (demo) en " + self.partido_carpeta
        raiz = os.path.normpath(os.path.join(CARPETA, "..", ".."))
        try:
            rel = os.path.relpath(self.partido_ruta, raiz)
            if not rel.startswith(".."):
                return rel
        except ValueError:
            pass
        return self.partido_ruta

    def _partido_editable(self, nombre=None):
        if self.cargando or self.lectura is None:
            return False
        return nombre is None or nombre not in self._no_usables()

    # ---- armado ----

    def _armar_partido(self):
        t = self.tab_partido
        t.columnconfigure(0, weight=1)
        t.columnconfigure(1, weight=0, minsize=440)
        t.rowconfigure(1, weight=1)
        arriba = tk.Frame(t, bg=FONDO)
        arriba.grid(row=0, column=0, columnspan=2, sticky="ew", padx=6, pady=(4, 2))
        self.lbl_partido_programa = self._etiqueta(arriba, font=F_NORMAL_B, wraplength=1250)
        self.lbl_partido_programa.pack(fill="x")
        fila = tk.Frame(arriba, bg=FONDO)
        fila.pack(fill="x")
        self.lbl_partido_estado = tk.Label(fila, text="", font=F_MEDIO_B, bg=FONDO, padx=8)
        self.lbl_partido_estado.pack(side="left")
        self._etiqueta(fila, font=F_CHICO, fg="#616161", wraplength=760,
                       text="Si tenés el partido abierto en el Arduino IDE, cerralo: si guardás ahí, pisás lo del "
                            "tablero. Cada vez que el tablero guarda, hace una copia en pruebas\\tablero\\copias-partido."
                       ).pack(side="left", padx=10)
        if self.demo:
            self._etiqueta(arriba, font=F_NORMAL_B, fg=VIOLETA,
                           text="DEMO: es una COPIA de práctica, el programa de verdad no se toca.").pack(fill="x")

        # --- izquierda: los avisos y la lista ---
        izq = tk.Frame(t, bg=FONDO)
        izq.grid(row=1, column=0, sticky="nsew", padx=(6, 4), pady=4)
        self.txt_avisos_partido = tk.Text(izq, font=F_NORMAL, wrap="word", height=3, bg="white", relief="flat",
                                          padx=6, pady=3)
        self.txt_avisos_partido.pack(fill="x", pady=(0, 4))
        self.txt_avisos_partido.tag_configure("error", foreground=ROJO, font=F_NORMAL_B)
        self.txt_avisos_partido.tag_configure("ojo", foreground=NARANJA)
        self.txt_avisos_partido.tag_configure("nota", foreground="#616161")
        self.txt_avisos_partido.tag_configure("bien", foreground=VERDE)
        self.txt_avisos_partido.configure(state="disabled")
        caja = tk.Frame(izq, bg=FONDO)
        caja.pack(fill="both", expand=True)
        self.cv_partido = tk.Canvas(caja, bg=FONDO, highlightthickness=0)
        barra = ttk.Scrollbar(caja, orient="vertical", command=self.cv_partido.yview)
        self.cv_partido.configure(yscrollcommand=barra.set)
        barra.pack(side="right", fill="y")
        self.cv_partido.pack(side="left", fill="both", expand=True)
        self.lista_partido = tk.Frame(self.cv_partido, bg=FONDO)
        self._id_lista_partido = self.cv_partido.create_window(0, 0, window=self.lista_partido, anchor="nw")
        self.lista_partido.bind("<Configure>",
                                lambda e: self.cv_partido.configure(scrollregion=self.cv_partido.bbox("all")))
        self._wrap_partido = 820
        self.cv_partido.bind("<Configure>", self._ancho_lista_partido)
        self.cv_partido.bind("<Enter>", lambda e: self.cv_partido.bind_all("<MouseWheel>", self._rueda_partido))
        self.cv_partido.bind("<Leave>", lambda e: self.cv_partido.unbind_all("<MouseWheel>"))
        self.grupos_partido = {}
        for clave, titulo, _ in GRUPOS_PARTIDO:
            contenedor = tk.Frame(self.lista_partido, bg=FONDO)
            contenedor.pack(fill="x", pady=(4, 0))
            cabeza = tk.Button(contenedor, text=titulo, font=F_MEDIO_B, relief="flat", anchor="w", bg="#cfd8dc",
                               activebackground="#b0bec5", bd=0, padx=8, pady=3, cursor="hand2",
                               command=lambda k=clave: self._plegar_grupo(k))
            cabeza.pack(fill="x")
            cuerpo = tk.Frame(contenedor, bg=FONDO)
            if clave == "avanzado":
                self._etiqueta(cuerpo, text=AVISO_AVANZADO, font=F_NORMAL_B, fg=ROJO).pack(fill="x", padx=6)
            nombres = [f[0] for f in TABLA_PARTIDO if f[8] == clave]
            for i, nombre in enumerate(nombres):
                if nombre in SUBTITULOS_PARTIDO:
                    self._etiqueta(cuerpo, text=SUBTITULOS_PARTIDO[nombre], font=F_NORMAL_B, fg=AZUL).pack(
                        fill="x", padx=6, pady=(6, 0))
                self._armar_fila_partido(cuerpo, nombre, i)
            self.grupos_partido[clave] = {"cabeza": cabeza, "cuerpo": cuerpo, "titulo": titulo}
            if not self.grupo_plegado[clave]:
                cuerpo.pack(fill="x")

        # --- derecha (fija): el robot, CARGAR y los otros botones ---
        der = tk.Frame(t, bg=FONDO, width=440)
        der.grid(row=1, column=1, sticky="nsew", padx=(4, 6), pady=4)
        self._etiqueta(der, text="EL ROBOT TIENE:", font=F_NORMAL_B).pack(fill="x")
        self.lbl_robot_partido = self._etiqueta(der, font=F_NORMAL_B, wraplength=420)
        self.lbl_robot_partido.pack(fill="x", pady=(0, 4))
        self.btn_cargar_partido = ttk.Button(der, text="▶  CARGAR EL PARTIDO\n     CON ESTOS VALORES",
                                             style="Probar.TButton", command=self._cargar_partido)
        self.btn_cargar_partido.pack(fill="x", pady=2)
        self.lbl_motivo_carga = self._etiqueta(der, font=F_CHICO, fg=NARANJA, wraplength=420)
        self.lbl_motivo_carga.pack(fill="x")
        fila = tk.Frame(der, bg=FONDO)
        fila.pack(fill="x", pady=(4, 0))
        self.btn_guardar_partido = ttk.Button(fila, text="Guardar sin cargar", command=self._partido_guardar)
        self.btn_guardar_partido.pack(side="left", fill="x", expand=True, padx=(0, 2))
        self.btn_descartar_partido = ttk.Button(fila, text="Descartar", command=self._partido_descartar)
        self.btn_descartar_partido.pack(side="left", fill="x", expand=True, padx=(2, 0))
        fila = tk.Frame(der, bg=FONDO)
        fila.pack(fill="x", pady=(2, 0))
        self.btn_volver_copia = ttk.Button(fila, text="Volver a como estaba", command=self._partido_volver_copia)
        self.btn_volver_copia.pack(side="left", fill="x", expand=True, padx=(0, 2))
        self.btn_volver_fabrica = ttk.Button(fila, text="Volver a los de fábrica", command=self._partido_volver_fabrica)
        self.btn_volver_fabrica.pack(side="left", fill="x", expand=True, padx=(2, 0))
        self.btn_pasar_partido = ttk.Button(der, text="Pasar lo medido en CANCHA →", command=self._partido_pasar)
        self.btn_pasar_partido.pack(fill="x", pady=1)
        tk.Frame(der, bg="#bdbdbd", height=2).pack(fill="x", pady=6)
        self.btn_cargar_tablero = ttk.Button(der, text="Cargar el TABLERO\n(para MOVIMIENTOS y CANCHA)",
                                             style="Tablero.TButton", command=self._cargar_tablero)
        self.btn_cargar_tablero.pack(fill="x", pady=1)
        self.lbl_etapa_carga = self._etiqueta(der, font=F_MEDIO_B, wraplength=420)
        self.lbl_etapa_carga.pack(fill="x", pady=(4, 0))
        self.lbl_resultado_carga = self._etiqueta(der, font=F_NORMAL_B, wraplength=420)
        self.lbl_resultado_carga.pack(fill="x")
        marco = tk.Frame(der, bg=FONDO)
        marco.pack(fill="both", expand=True, pady=(2, 0))
        self.txt_log_carga = tk.Text(marco, font=("Consolas", 9), wrap="word", height=5, bg="#fafafa", relief="flat")
        barra2 = ttk.Scrollbar(marco, orient="vertical", command=self.txt_log_carga.yview)
        self.txt_log_carga.configure(yscrollcommand=barra2.set)
        barra2.pack(side="right", fill="y")
        self.txt_log_carga.pack(side="left", fill="both", expand=True)
        self.txt_log_carga.configure(state="disabled")
        self.botones_editar_partido = [self.btn_guardar_partido, self.btn_descartar_partido, self.btn_volver_fabrica,
                                       self.btn_pasar_partido]

    def _armar_fila_partido(self, padre, nombre, i):
        f = FILA_PARTIDO[nombre]
        tipo = f[2]
        fondo = "white" if i % 2 == 0 else "#e8eef3"
        marco = tk.Frame(padre, bg=fondo, padx=6, pady=2)
        marco.pack(fill="x", pady=1)
        arriba = tk.Frame(marco, bg=fondo)
        arriba.pack(fill="x")
        lbl_nombre = tk.Label(arriba, text=nombre, font=F_NORMAL_B, bg=fondo, anchor="w", width=27)
        lbl_nombre.pack(side="left")
        botones, entrada, valor, opciones = [], None, None, None
        if tipo in ("b", "l"):
            opciones = (("NO", "false"), ("SÍ", "true")) if tipo == "b" else (("IZQUIERDO", "-1"), ("DERECHO", "1"))
            for texto, que in opciones:
                b = ttk.Button(arriba, text=texto, width=11 if tipo == "l" else 5,
                               command=lambda n=nombre, q=que: self._partido_elegir(n, q))
                b.pack(side="left", padx=2)
                botones.append(b)
        elif tipo == "v":
            valor = tk.Label(arriba, text="?", font=F_GRANDE, width=7, bg=fondo)
            valor.pack(side="left", padx=4)
            tk.Label(arriba, text="(solo para ver)", font=F_CHICO, bg=fondo, fg=GRIS).pack(side="left")
        else:
            menos = ttk.Button(arriba, text="−", style="Paso.TButton", width=3,
                               command=lambda n=nombre: self._partido_paso(n, -1))
            valor = tk.Label(arriba, text="?", font=F_GRANDE, width=7, bg=fondo)
            mas = ttk.Button(arriba, text="+", style="Paso.TButton", width=3,
                             command=lambda n=nombre: self._partido_paso(n, +1))
            menos.pack(side="left")
            valor.pack(side="left", padx=4)
            mas.pack(side="left")
            entrada = ttk.Entry(arriba, width=8, font=F_MEDIO)
            entrada.pack(side="left", padx=(12, 4))
            entrada.bind("<Return>", lambda e, n=nombre: self._partido_poner(n))
            poner = ttk.Button(arriba, text="Poner", width=6, command=lambda n=nombre: self._partido_poner(n))
            poner.pack(side="left")
            botones = [menos, mas, poner]
        lbl_unidad = tk.Label(arriba, text=f[4], font=F_CHICO, bg=fondo, fg="#546e7a", anchor="w")
        lbl_unidad.pack(side="left", padx=6)
        lbl_explica = tk.Label(marco, text=f[9], font=F_CHICO, fg="#546e7a", bg=fondo, anchor="w", justify="left",
                               wraplength=820)
        lbl_explica.pack(fill="x")
        fondos = [marco, arriba, lbl_nombre, lbl_unidad, lbl_explica]
        if valor is not None:
            fondos.append(valor)
        lbl_lado = None
        if f[10]:
            lbl_lado = tk.Label(marco, text=f[10], font=F_CHICO, fg=AZUL, bg=fondo, anchor="w", justify="left",
                                wraplength=820)
            lbl_lado.pack(fill="x")
            fondos.append(lbl_lado)
        # La línea de ATADO (naranja) solo se ve si tiene algo que decir.
        lbl_atado = tk.Label(marco, text=f[11], font=F_CHICO, fg=NARANJA, bg=fondo, anchor="w", justify="left",
                             wraplength=820)
        if f[11]:
            lbl_atado.pack(fill="x")
        fondos.append(lbl_atado)
        self.filas_partido[nombre] = {"tipo": tipo, "fondo": fondo, "fondos": fondos, "nombre": lbl_nombre,
                                      "valor": valor, "unidad": lbl_unidad, "explica": lbl_explica,
                                      "atado": lbl_atado, "atado_visible": bool(f[11]), "botones": botones,
                                      "entrada": entrada, "opciones": opciones,
                                      "textos": [x for x in (lbl_explica, lbl_lado, lbl_atado) if x is not None]}

    def _ancho_lista_partido(self, evento):
        """La lista ocupa todo el ancho, y los textos se doblan a ESE ancho
        (con la ventana más angosta no se cortan)."""
        self.cv_partido.itemconfigure(self._id_lista_partido, width=evento.width)
        ancho = max(300, int(evento.width) - 40)
        if abs(ancho - self._wrap_partido) >= 20:
            self._wrap_partido = ancho
            for fila in self.filas_partido.values():
                for w in fila["textos"]:
                    w.configure(wraplength=ancho)

    def _rueda_partido(self, evento):
        pasos = int(-evento.delta / 120) or (-1 if evento.delta > 0 else 1)
        self.cv_partido.yview_scroll(pasos, "units")

    def _plegar_grupo(self, clave):
        g = self.grupos_partido[clave]
        self.grupo_plegado[clave] = not self.grupo_plegado[clave]
        if self.grupo_plegado[clave]:
            g["cuerpo"].pack_forget()
        else:
            g["cuerpo"].pack(fill="x")
        self._pintar_partido()

    def _ir_a_pestana(self, nombre):
        """Elige una pestaña por su nombre (pelota, sensores, movimientos,
        cancha, partido, consola)."""
        self.pestana_elegida = nombre
        try:
            self.cuaderno.select(self.pestanas[nombre])
        except Exception:
            pass

    def _cambio_de_pestana(self, evento=None):
        if self._visible(self.tab_partido):
            self._releer_si_cambio()
            self.pendientes.add("partido")
        self._refrescar({"sensores", "resultado", "cancha"} | ({"partido"} if self._visible(self.tab_partido) else set()))

    # ---- pintar ----

    @staticmethod
    def _extra_unidad(nombre, texto):
        try:
            v = float(texto)
        except (TypeError, ValueError):
            return ""
        if nombre == "msAdelante":
            return "  ≈ %.0f cm" % (v / 10.0 - 3.3)
        if FILA_PARTIDO[nombre][4] == "ms" and v >= 1000:
            return "  = %s s" % ("%.1f" % (v / 1000.0)).rstrip("0").rstrip(".")
        return ""

    def _pintar_partido(self):
        if not self.filas_partido:
            return
        lec = self.lectura
        valores = self._valores_partido()
        archivo = lec["valores"] if lec else {}
        reglas = self._reglas_de_ahora()
        por_nombre = {}
        for nivel, texto, nombres, _ in reglas:
            for n in nombres:
                por_nombre.setdefault(n, []).append((nivel, texto))
        no_usables = self._no_usables()
        # arriba: el programa y si hay cambios
        if lec is None:
            self.lbl_partido_programa.configure(text="Programa: %s · NO LO PUDE LEER: %s" % (
                self._ruta_para_mostrar(), self.error_lectura or "?"), fg=ROJO)
        else:
            self.lbl_partido_programa.configure(fg="#212121", text="Programa: %s · versión %s · sello %s" % (
                self._ruta_para_mostrar(), lec["version"] or "(no dice: 0921f o anterior)", lec["sello"] or "-"))
        texto, fondo, color = self._texto_estado_partido()
        self.lbl_partido_estado.configure(text=texto, bg=fondo, fg=color)
        # los avisos
        avisos = []
        if lec is None:
            avisos.append(("error", "No pude leer el programa de partido (%s): %s" % (self.partido_ruta,
                                                                                     self.error_lectura or "?")))
        else:
            for p in lec["problemas"]:
                avisos.append(("error", p))
            if lec["faltan"]:
                avisos.append(("error", "Faltan en el programa: %s. No los adivino: ¿es otra versión? Avisale al "
                                        "profe." % ", ".join(lec["faltan"])))
            if lec["repetidos"]:
                avisos.append(("error", "Están repetidos (no los toco): %s." % ", ".join(lec["repetidos"])))
            for nombre, motivo in lec["raros"]:
                avisos.append(("error", "%s: %s (no lo toco)." % (nombre, motivo)))
        if self.aviso_afuera:
            avisos.append(("ojo", self.aviso_afuera))
        for nivel, texto, _, _ in reglas:
            avisos.append((nivel, texto))
        if not avisos:
            avisos.append(("bien", "✔ Ningún aviso: los valores están dentro de lo seguro."))
        marca = {"error": "✖ ", "ojo": "⚠ ", "nota": "· ", "bien": ""}
        t = self.txt_avisos_partido
        t.configure(state="normal")
        t.delete("1.0", "end")
        for nivel, texto in avisos:
            t.insert("end", marca[nivel] + texto + "\n", nivel)
        t.configure(state="disabled", height=max(2, min(4, len(avisos))))
        # los grupos y las filas
        for clave, titulo, _ in GRUPOS_PARTIDO:
            g = self.grupos_partido[clave]
            cuantos = sum(1 for f in TABLA_PARTIDO if f[8] == clave and f[0] in self.pendientes_partido)
            g["cabeza"].configure(text="%s %s%s" % ("▶" if self.grupo_plegado[clave] else "▼", titulo,
                                                    "      %d cambio%s" % (cuantos, "s" if cuantos != 1 else "")
                                                    if cuantos else ""),
                                  fg="#bf360c" if cuantos else "#212121")
        for nombre, fila in self.filas_partido.items():
            f = FILA_PARTIDO[nombre]
            texto = valores.get(nombre)
            errores = [x for x in por_nombre.get(nombre, ()) if x[0] == "error"]
            ojos = [x for x in por_nombre.get(nombre, ()) if x[0] == "ojo"]
            if nombre in no_usables:
                fondo = "#eeeeee"
            elif errores:
                fondo = "#ffcdd2"
            elif nombre in self.pendientes_partido:
                fondo = "#fff59d"
            else:
                fondo = fila["fondo"]
            for w in fila["fondos"]:
                w.configure(bg=fondo)
            gris = (nombre in no_usables) or self._fila_no_aplica(nombre, valores)
            fila["nombre"].configure(fg=GRIS if gris else "#212121")
            if fila["valor"] is not None:
                fila["valor"].configure(text="?" if texto is None else texto, fg=GRIS if gris else "#212121")
            if fila["opciones"]:
                for b, (_, que) in zip(fila["botones"], fila["opciones"]):
                    elegido = texto is not None and mismo_valor(nombre, que, texto)
                    b.configure(style="Elegido.TButton" if elegido else "TButton")
            fila["unidad"].configure(text=f[4] + self._extra_unidad(nombre, texto))
            if nombre in no_usables:
                explica = "NO ESTÁ en el programa (o no lo entiendo): no lo toco."
            elif nombre in self.pendientes_partido:
                explica = "archivo: %s · %s" % (archivo.get(nombre, "?"), f[9])
            else:
                explica = f[9]
            fila["explica"].configure(text=explica)
            linea = f[11]
            dinamicos = ["⚠ " + x[1] for x in errores + ojos]
            if dinamicos:
                linea = (linea + "\n" if linea else "") + "\n".join(dinamicos)
            fila["atado"].configure(text=linea, fg=ROJO if errores else NARANJA)
            if bool(linea) != fila["atado_visible"]:
                fila["atado_visible"] = bool(linea)
                if linea:
                    fila["atado"].pack(fill="x")
                else:
                    fila["atado"].pack_forget()
        self._pintar_robot_partido()
        self._clave_habilitados = None

    def _texto_estado_partido(self):
        """El cartel grande de arriba: (texto, fondo, color). (2026-11-08,
        revisión) estado_archivo es LO ÚLTIMO QUE SE SABE del archivo y el
        robot, y se actualiza en CADA cosa que pasa (guardar, cada carga, cada
        #PARTIDO o #HOLA que llega): ya no queda pegado un 'cargado' viejo."""
        n = len(self._cambios_reales())
        if n:
            return "%d CAMBIO%s SIN GUARDAR" % (n, "S" if n != 1 else ""), "#fff59d", "#212121"
        if self.cargando:
            return "CARGANDO: no desenchufes el USB", FONDO, VIOLETA
        texto, color = {
            "guardado": ("Guardado, sin cargar: el robot todavía tiene lo de antes", AZUL),
            "cargado": ("Cargado y confirmado por el robot", VERDE),
            "cargado_sin_confirmar": ("Cargado, pero el robot todavía no dijo su versión (mirá abajo)", NARANJA),
            "no_coincide": ("El robot dice OTROS valores que el archivo: CARGAR EL PARTIDO", ROJO),
            "a_medias": ("La carga FALLÓ: el robot puede haber quedado SIN programa (mirá abajo)", ROJO),
            "tablero": ("El robot tiene el TABLERO (no juega): para jugar, CARGAR EL PARTIDO", NARANJA),
            "otro": ("El robot tiene OTRO programa (no la 0921): para jugar, CARGAR EL PARTIDO", NARANJA),
        }.get(self.estado_archivo, ("Sin cambios", GRIS))
        return texto, FONDO, color

    def _estado_por_partido(self, m):
        """Llegó una #PARTIDO sin estar esperando una carga: lo que dice el
        robot pasa a ser lo que se sabe (el SELLO confirma todos los valores)."""
        if self.lectura is None:
            return
        estado, _ = comparar_con_robot(m, self.lectura)
        self.estado_archivo = "cargado" if estado == "IGUAL" else "no_coincide"

    def _fila_no_aplica(self, nombre, valores):
        llaves = GRIS_SI_PARTIDO.get(nombre)
        if not llaves:
            return False
        return all(valores.get(k) == "false" for k in llaves)

    def _texto_robot_partido(self):
        """El recuadro EL ROBOT TIENE: (texto, color)."""
        m = self.ultimo_partido
        if self.cargando:
            return "CARGANDO un programa... no desenchufes el USB.", VIOLETA
        if m is not None and self.lectura is not None:
            estado, difs = comparar_con_robot(m, self.lectura)
            cabeza = "PARTIDO %s · sello %s · giroscopio %s" % (m["version"], m["sello"], "OK" if m["giro"] else "NO")
            if estado == "IGUAL":
                return cabeza + "\n✔ coincide con el archivo", VERDE
            if estado == "OTRA_VERSION":
                return cabeza + "\n✖ Es otra versión: el archivo es la %s. Cargalo de nuevo." % (
                    self.lectura["version"] or "0921f"), ROJO
            return cabeza + "\n✖ NO coincide: " + self._texto_difs(difs), ROJO
        if not self.conectado:
            return "Sin robot conectado (enchufá el arquero por USB).", GRIS
        if not self.es_arquero:
            return "Esa Teensy NO es el arquero.", ROJO
        prog = self.programa_detectado
        if prog == "TABLERO_ARQUERO" or self._programa_ok():
            return "El programa del TABLERO (para MOVIMIENTOS y CANCHA). Para jugar: CARGAR EL PARTIDO.", AZUL
        if prog and prog.startswith("PARTIDO"):
            return ("Tiene la %s, NO la de jugar: CARGAR EL PARTIDO." % (prog[len("PARTIDO"):].strip() or
                                                                         "versión vieja")), ROJO
        if prog == "AJUSTAR":
            return "AJUSTAR-MOVIMIENTOS (no es el de jugar): CARGAR EL PARTIDO.", NARANJA
        if prog == "TABLERO_DELANTERO":
            return "El TABLERO DEL DELANTERO (pines cruzados): CARGAR EL PARTIDO o el TABLERO.", ROJO
        return ("No sé (no dijo su versión: ¿es la 0921f o anterior?). Para que la repita: batería APAGADA, "
                "desenchufá y enchufá el USB."), NARANJA

    @staticmethod
    def _texto_difs(difs):
        def bonito(x):
            if isinstance(x, bool):
                return "SÍ" if x else "NO"
            return _g(x)
        return "; ".join("%s robot %s / archivo %s" % (n, bonito(a), bonito(b)) for n, a, b in difs[:4]) + (
            "..." if len(difs) > 4 else "")

    def _pintar_robot_partido(self):
        if not self.filas_partido:
            return
        texto, color = self._texto_robot_partido()
        self.lbl_robot_partido.configure(text=texto, fg=color)
        self.lbl_etapa_carga.configure(text=self.etapa_carga if self.cargando else "")
        texto, color = self.resultado_carga or ("", "#212121")
        self.lbl_resultado_carga.configure(text=texto, fg=color)

    def _log_carga(self, texto):
        # cargar.py imprime emojis: un Tk viejo no muestra lo de más de U+FFFF (y tira un error).
        texto = "".join(c if ord(c) <= 0xFFFF else "(!)" for c in str(texto))
        self.log_carga.append(texto)
        if len(self.log_carga) > 400:
            del self.log_carga[:100]
        try:
            t = self.txt_log_carga
            t.configure(state="normal")
            t.insert("end", texto + "\n")
            t.see("end")
            t.configure(state="disabled")
        except Exception:
            pass

    def _actualizar_habilitados_partido(self):
        if not self.filas_partido:
            return
        puede_p, motivo_p = self._puede_cargar("partido")
        puede_t, motivo_t = self._puede_cargar("tablero")
        libre = self._partido_editable()
        clave = (puede_p, motivo_p, puede_t, libre, self.hay_copias, bool(self.pendientes_partido),
                 frozenset(self._no_usables()))
        if clave == self._clave_habilitados:
            return
        self._clave_habilitados = clave
        self._habilitar(self.btn_cargar_partido, puede_p)
        self._habilitar(self.btn_cargar_tablero, puede_t)
        self.lbl_motivo_carga.configure(text="" if puede_p else "No se puede cargar: " + motivo_p)
        for b in self.botones_editar_partido:
            self._habilitar(b, libre)
        self._habilitar(self.btn_volver_copia, libre and self.hay_copias)
        no_usables = self._no_usables()
        for nombre, fila in self.filas_partido.items():
            si = libre and nombre not in no_usables
            for b in fila["botones"]:
                self._habilitar(b, si)
            if fila["entrada"] is not None:
                self._habilitar(fila["entrada"], si)

    # ---- cambiar valores (solo el EDITOR: el archivo no se toca) ----

    def _partido_poner_valor(self, nombre, texto, origen=None, confirmar=True):
        """Pone un valor en el EDITOR (amarillo). -> True si quedó."""
        if not self._partido_editable():
            self._aviso("Se está cargando: esperá que termine para cambiar valores." if self.cargando else
                        "No pude leer el programa de partido: no se puede cambiar nada.", NARANJA)
            return False
        if nombre in self._no_usables():
            self._aviso("%s NO ESTÁ (bien) en el programa: no lo toco." % nombre, ROJO)
            return False
        valor, motivo = validar_valor_partido(nombre, texto)
        if motivo:
            self._aviso(motivo + ".", ROJO)
            return False
        archivo = self.lectura["valores"][nombre]
        canonico = texto_para_ino(nombre, texto, archivo)
        if confirmar and nombre in CONFIRMAR_PARTIDO and not mismo_valor(nombre, canonico, archivo):
            con, pregunta = CONFIRMAR_PARTIDO[nombre]
            if con is None or mismo_valor(nombre, canonico, con):
                if not messagebox.askyesno(nombre, pregunta, parent=self.raiz):
                    return False
        if mismo_valor(nombre, canonico, archivo):
            self.pendientes_partido.pop(nombre, None)
            self.origen_partido.pop(nombre, None)
        else:
            self.pendientes_partido[nombre] = canonico
            if origen:
                self.origen_partido[nombre] = origen
            else:
                self.origen_partido.pop(nombre, None)
        self.aviso_afuera = None
        # (2026-11-08, revisión) Editar NO cambia lo que tiene el robot: el
        # cartel de arriba dice "N CAMBIOS SIN GUARDAR" mientras haya, y si se
        # descartan vuelve a decir lo que se sabía.
        self._pintar_partido()
        self._actualizar_habilitados()
        return True

    def _partido_paso(self, nombre, signo):
        if not self._partido_editable(nombre):
            return
        f = FILA_PARTIDO[nombre]
        if f[2] not in ("i", "u", "f"):
            self._aviso("%s no se cambia con [+] y [−]%s." % (nombre, " (es solo para ver)" if f[2] == "v" else ""),
                        GRIS)
            return
        actual = self._valores_partido().get(nombre)
        try:
            nuevo = nuevo_valor(actual, f[7], signo)
        except (ValueError, TypeError):
            self._aviso("%s vale '%s' y no lo entiendo." % (nombre, actual), ROJO)
            return
        if not f[5] - 1e-9 <= float(nuevo) <= f[6] + 1e-9:
            self._aviso("%s va de %s a %s: ya está en la punta." % (nombre, _g(f[5]), _g(f[6])), NARANJA)
            return
        if self._partido_poner_valor(nombre, nuevo):
            self._aviso("%s = %s en el editor (todavía NO está guardado ni cargado)." % (
                nombre, self.pendientes_partido.get(nombre, nuevo)), GRIS)

    def _partido_poner(self, nombre):
        fila = self.filas_partido.get(nombre)
        if not fila or fila["entrada"] is None:
            return
        escrito = str(fila["entrada"].get()).strip()
        if not escrito:
            return
        if self._partido_poner_valor(nombre, escrito):
            fila["entrada"].delete(0, "end")
            self._aviso("%s = %s en el editor (todavía NO está guardado ni cargado)." % (
                nombre, self._valores_partido().get(nombre)), GRIS)

    def _partido_elegir(self, nombre, que):
        if self._partido_poner_valor(nombre, que):
            self._aviso("%s = %s en el editor (todavía NO está guardado ni cargado)." % (nombre, que), GRIS)

    # ---- guardar, descartar, volver ----

    def _anotar_en_hoja(self, cambios):
        """(B) Lo que vino de CANCHA y se guardó, a la hoja de la cancha."""
        try:
            de_cancha = [(n, a, b) for n, a, b in cambios if self.origen_partido.get(n) == "CANCHA"]
            if de_cancha and self.hoja is not None:
                for n, a, b in de_cancha:
                    hoja_poner(self.hoja, "COPIADO AL PANEL", n, "%s -> %s" % (a, b),
                               "PARTIDO " + time.strftime("%d/%m %H:%M"))
                self._escribir_hoja()
        except Exception:
            pass

    def _tomar_guardado(self, info):
        """Lo que devuelve guardar_partido: el archivo nuevo pasa a ser 'lo de hoy'."""
        if info.get("sin_cambios"):
            return
        self._anotar_en_hoja(info["cambios"])
        self.lectura = info["lectura_nueva"]
        self.stat_partido = self._stat_partido()
        self.hay_copias = bool(copias_del_partido(self.copias_partido))
        for nombre, _, _ in info["cambios"]:
            self.pendientes_partido.pop(nombre, None)
            self.origen_partido.pop(nombre, None)
        self.estado_archivo = "guardado"
        self.aviso_afuera = None
        self.pendientes |= {"partido", "cancha"}

    def _partido_guardar(self):
        if not self._partido_editable():
            return False
        errores = [r[1] for r in self._reglas_de_ahora() if r[0] == "error"]
        if errores:
            self._aviso("No guardo: " + errores[0], ROJO)
            return False
        if not self.pendientes_partido:
            self._aviso("No hay cambios para guardar.", GRIS)
            return False
        try:
            info = guardar_partido(self.partido_ruta, self.copias_partido, dict(self.pendientes_partido),
                                   self.lectura["huella"], origenes=dict(self.origen_partido))
        except CambioPorAfuera:
            self._releer_partido(por_afuera=True)
            self._aviso("Cambió mientras editabas (¿Arduino IDE?): revisá y apretá de nuevo. No guardé nada.", NARANJA)
            return False
        except NoSeEscribe as e:
            self._aviso("No guardé: %s" % e, ROJO)
            self.pendientes.add("partido")
            return False
        except OSError as e:
            self._aviso("No guardé: no puedo abrir el programa (%s)." % e.__class__.__name__, ROJO)
            return False
        self._tomar_guardado(info)
        self._aviso("Guardado, sin cargar: el robot todavía tiene lo de antes. (Copia: %s)" % (
            os.path.basename(info["copia"]) if info.get("copia") else "-"), AZUL)
        self._pintar_partido()
        self._actualizar_habilitados()
        return True

    def _partido_descartar(self):
        if self.cargando:
            return
        self.pendientes_partido = {}
        self.origen_partido = {}
        self._aviso("Descarté los cambios: quedó lo del archivo.", GRIS)
        self._pintar_partido()
        self._actualizar_habilitados()

    def _poner_varios(self, difs, origen, titulo, explicacion):
        """Pone en el EDITOR varios valores [(nombre, archivo, nuevo)], después
        de mostrar qué va a volver. No escribe nada. (2026-11-08, revisión)
        Primero VACÍA el editor: 'Volver' deja EXACTO la copia o los de
        fábrica (un cambio de prueba que había en el editor se descarta, y la
        ventanita lo dice)."""
        difs = [d for d in difs if d[0] in FILA_PARTIDO and d[0] not in self._no_usables()]
        nombres = {d[0] for d in difs}
        archivo = self.lectura["valores"] if self.lectura else {}
        se_van = [(n, archivo.get(n, "?"), v) for n, v in sorted(self.pendientes_partido.items()) if n not in nombres]
        if not difs and not se_van:
            self._aviso("Ya está todo así: no hay nada para cambiar.", GRIS)
            return False
        texto = explicacion + "\n\n"
        if difs:
            texto += " · ".join("%s %s → %s" % d for d in difs[:20]) + (" ..." if len(difs) > 20 else "")
        else:
            texto += "(el archivo ya los tiene)"
        if se_van:
            texto += ("\n\nSe DESCARTAN estos cambios del editor (vuelven a lo del archivo): " +
                      " · ".join("%s %s → %s" % d for d in se_van[:20]) + (" ..." if len(se_van) > 20 else ""))
        texto += ("\n\nQuedan en el EDITOR (amarillo). GUARDAR los escribe en el archivo; recién CARGAR EL "
                  "PARTIDO los lleva al robot.")
        if not messagebox.askyesno(titulo, texto, parent=self.raiz):
            return False
        self.pendientes_partido = {}
        self.origen_partido = {}
        for nombre, _, nuevo in difs:
            self.pendientes_partido[nombre] = nuevo
            self.origen_partido[nombre] = origen
        self._releer_partido()          # (saca los que quedaron iguales al archivo)
        if difs:
            self._aviso("Puse %d valores en el editor (%s). Revisalos y apretá CARGAR EL PARTIDO." % (
                len(difs), origen), AZUL)
        else:
            self._aviso("Descarté los cambios del editor: el archivo ya %s." % (
                "tiene los de fábrica" if origen == "fábrica" else "está como la copia"), AZUL)
        self._pintar_partido()
        self._actualizar_habilitados()
        return True

    def _partido_volver_copia(self):
        if not self._partido_editable():
            return False
        elegida = elegir_copia_para_volver(self.copias_partido, self.lectura["valores"])
        if elegida is None:
            if self.pendientes_partido and copias_del_partido(self.copias_partido):
                # El archivo ya está como la copia más nueva: lo que hay que volver es el EDITOR.
                return self._poner_varios([], "copia", "Volver a como estaba",
                                          "Las copias tienen lo mismo que el archivo:")
            self._aviso("Todavía no hay copias con otros valores (se hace una cada vez que el tablero guarda).", GRIS)
            return False
        ruta, difs = elegida
        return self._poner_varios(difs, "copia", "Volver a como estaba",
                                  "Vuelven los valores de la copia %s:" % os.path.basename(ruta))

    def _partido_volver_fabrica(self):
        if not self._partido_editable():
            return False
        difs = [(n, self.lectura["valores"][n], VALORES_DE_FABRICA[n]) for n in ORDEN_PARTIDO
                if n in self.lectura["valores"] and TIPO_PARTIDO[n] != "v"
                and not mismo_valor(n, self.lectura["valores"][n], VALORES_DE_FABRICA[n])]
        return self._poner_varios(difs, "fábrica", "Volver a los de fábrica",
                                  "Vuelven los valores de fábrica (la 0921f del 03/11):")

    # ---- CANCHA -> PARTIDO ----

    def _medido_para_partido(self, hoja=None, fuente=None):
        """Lo medido para pasar a PARTIDO: primero lo de esta sesión
        (_sugeridos) y, si no hay, la hoja abierta. -> [fila], con fila =
        {nombre, hoy, medido, fuente, aviso, tildado, puede, rojo}.
        hoja (2026-11-08, revisión: 'Volver a esta cancha' de COMPARAR): SOLO
        lo de ESA hoja, sin lo medido ahora."""
        if self.lectura is None:
            return []
        solo_hoja = hoja is not None
        s = {} if solo_hoja else self._sugeridos()
        if not solo_hoja:
            hoja = self.hoja
            fuente = "hoja %s" % (os.path.basename(self.ruta_hoja) if self.ruta_hoja else "abierta")
        fuente_hoja = fuente or "hoja"
        hoja = hoja_valores_panel(hoja) if hoja else {}
        filas = []

        def agregar(nombre, medido, fuente, aviso="", tildado=True, rojo=False, solo_si_distinto=False):
            hoy = self.lectura["valores"].get(nombre)
            if medido is None or hoy is None or nombre in self._no_usables():
                return
            fila = {"nombre": nombre, "hoy": hoy, "medido": str(medido), "fuente": fuente, "aviso": aviso,
                    "tildado": tildado, "puede": True, "rojo": rojo}
            _, motivo = validar_valor_partido(nombre, medido)
            if motivo:
                fila.update(puede=False, tildado=False, rojo=True, aviso=motivo)
            elif mismo_valor(nombre, str(medido), hoy):
                if solo_si_distinto:
                    return
                fila.update(puede=False, tildado=False, aviso="igual")
            else:
                fila["medido"] = texto_para_ino(nombre, str(medido), hoy)
            filas.append(fila)

        def de(nombre):
            v = s.get(nombre, (None,))[0]
            if v is not None:
                return v, "medido ahora"
            if nombre in hoja:
                return hoja[nombre], fuente_hoja
            return None, ""
        v, f = de("umbralBlanco")
        agregar("umbralBlanco", v, f)
        v, f = de("MS_MEDIO_AL_MEDIO")
        agregar("MS_MEDIO_AL_MEDIO", v, f, aviso="; ".join(a[0] for a in s.get("MS_MEDIO_AL_MEDIO", (None, []))[1])[:200])
        v, f = de("MS_MEDIO_FONDO_POR_TIEMPO")
        agregar("MS_MEDIO_FONDO_POR_TIEMPO", v, f)
        cruces = [] if solo_hoja else [a["datos"].get("izqder") for a in self.cancha["anchos"] if a["datos"].get("izqder")]
        if cruces and "MS_MEDIO_TOPE_LATERAL" in self.lectura["valores"]:
            necesita = 1.3 * max(cruces)
            if necesita > float(self.lectura["valores"]["MS_MEDIO_TOPE_LATERAL"]):
                agregar("MS_MEDIO_TOPE_LATERAL", int(math.ceil(necesita / 500.0) * 500), "medido ahora",
                        aviso="1,3 × el cruce más largo (%d ms)" % max(cruces))
        v, f = de("CAMARA_POR_CM")
        agregar("CAMARA_POR_CM", v, f, aviso="cambia todos los cm", tildado=False)
        idas = [] if solo_hoja else [(ms, cm) for ms, cm in self.cancha["idas"] if cm is not None]
        ab = recta([i[0] for i in idas], [i[1] for i in idas]) if len(idas) >= 2 else None
        if ab and "msAdelante" in self.lectura["valores"]:
            quiero = float(self.lectura["valores"]["msAdelante"]) / 10.0 - 3.3
            ms = ms_para_cm(ab[0], ab[1], quiero)
            nota = ("lo que hace falta para seguir llegando a %.0f cm (lo de hoy). Las idas del paso 5 se miden "
                    "SOLTANDO al final, como la 0921" % quiero)
            if s["msAdelante"][0]:
                nota += "; el sugerido del paso 5 da %s ms" % s["msAdelante"][0]
            agregar("msAdelante", ms, "medido ahora", aviso=nota, tildado=False)
        elif "msAdelante" in hoja:
            agregar("msAdelante", hoja["msAdelante"], fuente_hoja, tildado=False,
                    aviso="de la hoja: si se midió con la 0.1103 (frenando, otra potencia), NO vale para la 0921")
        for nombre in ("lateralInvertido", "camaraYInvertida"):
            v = s.get(nombre, (None,))[0]
            if v is not None:
                agregar(nombre, "true" if str(v) == "1" else "false", "medido ahora", aviso="¡CAMBIA UN SIGNO!",
                        tildado=False, rojo=True, solo_si_distinto=True)
        return filas

    def _pasar_a_partido(self, pares, origen="CANCHA"):
        """Pone en el EDITOR lo tildado [(nombre, valor)] y va a PARTIDO. No escribe nada.
        origen "CANCHA" se anota en la hoja abierta al guardar; el de una hoja vieja, no."""
        puestos = 0
        for nombre, valor in pares:
            if self._partido_poner_valor(nombre, valor, origen=origen, confirmar=False):
                puestos += 1
        self._ir_a_pestana("partido")
        self._aviso("Pasé %d valor%s. Revisalos y apretá CARGAR EL PARTIDO." % (puestos, "es" if puestos != 1 else ""),
                    AZUL)
        return puestos

    def _partido_pasar(self):
        return self._dialogo_pasar(self._medido_para_partido())

    def _dialogo_pasar(self, filas, titulo="Pasar lo medido en CANCHA a PARTIDO", origen="CANCHA"):
        """La ventanita para pasar lo medido en CANCHA. -> la función que
        hace 'Pasar los tildados' (la autoprueba la llama directo)."""
        if not filas:
            self._aviso("Todavía no hay nada medido en CANCHA (ni una hoja abierta) para pasar.", NARANJA)
            return None
        tildados = {f["nombre"]: f["tildado"] for f in filas if f["puede"]}
        w = tk.Toplevel(self.raiz)
        w.title(titulo)
        w.configure(bg=FONDO)
        w.transient(self.raiz)
        self._etiqueta(w, text="Se ponen en el EDITOR de PARTIDO (no se escribe nada hasta GUARDAR o CARGAR).",
                       font=F_NORMAL_B).grid(row=0, column=0, columnspan=6, sticky="w", padx=8, pady=6)
        for c, titulo in enumerate(("", "número", "HOY (archivo)", "MEDIDO", "de dónde", "aviso")):
            self._etiqueta(w, text=titulo, font=F_CHICO, fg="#616161").grid(row=1, column=c, sticky="w", padx=4)
        for i, f in enumerate(filas):
            fila = i + 2
            if f["puede"]:
                var = tk.BooleanVar(value=f["tildado"])
                ttk.Checkbutton(w, variable=var, command=lambda n=f["nombre"], x=var: tildados.__setitem__(
                    n, bool(x.get()))).grid(row=fila, column=0, padx=4)
            color = ROJO if f["rojo"] else "#212121"
            for c, texto in enumerate((f["nombre"], f["hoy"], f["medido"], f["fuente"], f["aviso"]), start=1):
                self._etiqueta(w, text=texto, font=F_NORMAL_B if c in (1, 3) else F_CHICO,
                               fg=color if c in (3, 5) else "#212121", wraplength=360 if c == 5 else 0).grid(
                    row=fila, column=c, sticky="w", padx=4, pady=1)

        def pasar():
            pares = [(f["nombre"], f["medido"]) for f in filas if f["puede"] and tildados.get(f["nombre"])]
            try:
                w.destroy()
            except Exception:
                pass
            return self._pasar_a_partido(pares, origen)
        abajo = tk.Frame(w, bg=FONDO)
        abajo.grid(row=len(filas) + 2, column=0, columnspan=6, sticky="e", padx=8, pady=8)
        ttk.Button(abajo, text="Pasar los tildados", style="Usar.TButton", command=pasar).pack(side="left", padx=4)
        ttk.Button(abajo, text="Cancelar", command=w.destroy).pack(side="left", padx=4)
        return pasar

    # ---- CARGAR ----

    def _puede_cargar(self, que="partido"):
        """-> (se puede, motivo). No usa _puede(): ese pide que el robot
        tenga el programa del tablero, y acá se carga encima del partido."""
        e = self.estado
        if self.cargando:
            return False, "Se está cargando un programa: esperá que termine."
        if self.demo:
            return False, "En la demo no se carga nada (es una COPIA de práctica)."
        if que == "partido":
            if self.lectura is None:
                return False, "No pude leer el programa de partido: %s" % (self.error_lectura or "?")
            if self.lectura["problemas"]:
                return False, self.lectura["problemas"][0]
            errores = [r[1] for r in self._reglas_de_ahora() if r[0] == "error"]
            if errores:
                return False, "Hay un ERROR (en rojo): " + errores[0]
        if not (self.conectado and self.es_arquero):
            if self.conectado:
                return False, "Esa Teensy NO es el arquero."
            if self.situacion == "ocupado":
                return False, "Cerrá el Monitor Serie u otra ventana del tablero (el puerto está ocupado)."
            if self.situacion == "otro":
                if self.serie == SERIE_DELANTERO:
                    return False, "Ese es el DELANTERO: enchufá SOLO el arquero."
                return False, "Esa Teensy no es el arquero (otra Teensy): enchufá SOLO el arquero."
            if e.esperando_sin_cable:
                return False, "SIN CABLE: el robot está haciendo una prueba solo. Enchufalo cuando termine."
            return False, "Enchufá el arquero por USB."
        if self._habla_tablero() and e.modo != "ESPERA":
            return False, "Esperá que termine el movimiento (o PARAR)."
        return True, ""

    def _cargar_partido(self):
        """▶ CARGAR EL PARTIDO CON ESTOS VALORES: guarda lo pendiente (con
        copia), compila, suelta el puerto, carga con cargar.py, se vuelve a
        conectar y confirma con la línea #PARTIDO."""
        puede, motivo = self._puede_cargar("partido")
        if not puede:
            self._aviso(motivo, NARANJA)
            return False
        cambios = self._cambios_reales()
        ojos = [r[1] for r in self._reglas_de_ahora() if r[0] == "ojo"]
        texto = TEXTO_CARGAR_PARTIDO + "\n\nLo que cambia: " + (
            " · ".join("%s %s → %s" % c for c in cambios) if cambios else "nada (se carga tal como está el archivo)")
        if ojos:
            texto += "\n\n⚠ OJO: " + "\n⚠ OJO: ".join(ojos)
        texto += "\n\n¿Cargar?"
        if not messagebox.askyesno("CARGAR EL PARTIDO", texto, parent=self.raiz):
            return False
        guardar = None
        if cambios:
            guardar = {"ruta": self.partido_ruta, "carpeta_copias": self.copias_partido,
                       "cambios": dict(self.pendientes_partido), "huella_esperada": self.lectura["huella"],
                       "origenes": dict(self.origen_partido)}
        self.cambios_en_carga = dict(self.pendientes_partido)
        self.origenes_en_carga = dict(self.origen_partido)
        self._lanzar_carga("partido", self.partido_carpeta, guardar)
        return True

    def _cargar_tablero(self):
        """Cargar el TABLERO (para MOVIMIENTOS y CANCHA): igual, sin guardar nada."""
        puede, motivo = self._puede_cargar("tablero")
        if not puede:
            self._aviso(motivo, NARANJA)
            return False
        texto = TEXTO_CARGAR_TABLERO
        n = len(self._cambios_reales())
        if n:
            texto += ("\n\nOJO: tenés %d cambio%s sin guardar en PARTIDO: siguen en la ventana, pero NO se cargan."
                      % (n, "s" if n != 1 else ""))
        if not messagebox.askyesno("Cargar el TABLERO", texto + "\n\n¿Cargar?", parent=self.raiz):
            return False
        self.cambios_en_carga = {}
        self._lanzar_carga("tablero", CARPETA, None)
        return True

    def _lanzar_carga(self, que, carpeta, guardar):
        self.estado_antes_de_carga = self.estado_archivo
        self.cargando = True
        self.que_carga = que
        self.puerto_cerrado_por_carga = False
        self.resultado_carga = None
        self.esperando_version_hasta = None
        self.log_carga = []
        try:
            self.txt_log_carga.configure(state="normal")
            self.txt_log_carga.delete("1.0", "end")
            self.txt_log_carga.configure(state="disabled")
        except Exception:
            pass
        conexion_vieja = self.conexion
        if self._habla_tablero():
            try:
                conexion_vieja.enviar_ya("x")     # como al cerrar: que no quede andando
            except Exception:
                pass

        def cerrar():
            try:
                conexion_vieja.cerrar(esperar=True)
            except TypeError:
                conexion_vieja.cerrar()
        cargador = Cargador(self.cola, self.ejecutar, cerrar, self.mirar_carga, self.listar_puertos, self.dormir_carga)
        self._aviso("Cargando %s: no desenchufes el USB..." % ("el PARTIDO" if que == "partido" else "el TABLERO"),
                    VIOLETA)
        self.pendientes |= {"partido", "conexion"}
        self._pintar_partido()
        self._actualizar_habilitados()
        self.correr_en_hilo(lambda: cargador.trabajo(que, carpeta, guardar))

    def _correr_en_hilo(self, f):
        threading.Thread(target=f, name="cargador", daemon=True).start()

    def _fabrica_conexion(self):
        if SIN_PROCESOS:
            raise RuntimeError("autoprueba: no se abre ningún USB de verdad")
        return Conexion(self.cola, self.puerto_forzado)

    def _llego_carga(self, msg):
        sub = msg[1]
        if sub == "etapa":
            _, _, n, total, texto = msg
            self.etapa_carga = "%d/%d  %s" % (n, total, texto)
            self._log_carga("== " + self.etapa_carga)
        elif sub == "linea":
            self._log_carga(msg[2])
        elif sub == "guardado":
            self._tomar_guardado(msg[2])
        elif sub == "revertido":
            self._log_carga(msg[2])
            self._releer_partido()
            self.pendientes_partido.update(self.cambios_en_carga)
            self.origen_partido.update(getattr(self, "origenes_en_carga", {}))
            self._releer_partido()
            # El archivo volvió a como estaba antes de esta carga y el robot no se tocó.
            self.estado_archivo = self.estado_antes_de_carga
        elif sub == "puerto_cerrado":
            self.puerto_cerrado_por_carga = True
            self.conectado = False
            self.situacion = "cargando"
            self.es_tablero = self.pidio_numeros = False
            self.programa_detectado = None
            self.estado.hola = None
            self.ultimo_partido = None
            self.pendientes.add("conexion")
        elif sub == "fin":
            try:
                self._fin_de_carga(msg[2])
            finally:
                self.cargando = False
                self.etapa_carga = ""
                if self.puerto_cerrado_por_carga:
                    # Se vuelve a conectar SIEMPRE (también si algo falló).
                    self.puerto_cerrado_por_carga = False
                    self.situacion = "buscando"
                    self.conexion = self.fabrica_conexion()
                    self.conexion.arrancar()
        self.pendientes |= {"partido", "conexion"}

    def _fin_de_carga(self, r):
        self.t_fin_carga = time.monotonic()
        self._log_carga("== " + (r.get("motivo") or ""))
        if r.get("afuera"):
            self._releer_partido(por_afuera=True)
        if not r.get("ok"):
            color = ROJO if r.get("nivel") == "malo" else NARANJA
            self.resultado_carga = (r.get("motivo") or "Falló.", color)
            self._aviso(self.resultado_carga[0], color)
            if r.get("a_medias"):
                # Ya había empezado a grabar: NO "todavía tiene lo de antes".
                self.estado_archivo = "a_medias"
            return
        if r.get("que") == "tablero":
            self.estado_archivo = "tablero"
            self.resultado_carga = ("Cargó el TABLERO: ya se puede usar MOVIMIENTOS y CANCHA. Para jugar, volvé acá "
                                    "y CARGAR EL PARTIDO.", AZUL)
            self._aviso(self.resultado_carga[0], AZUL)
            return
        # Cargó (cargar.py dijo que sí): falta que el robot lo confirme con su #PARTIDO.
        self.estado_archivo = "cargado_sin_confirmar"
        if r.get("partido"):
            self.esperando_version_hasta = time.monotonic()
            self._confirmar_con(r["partido"][-1])
        else:
            # Lo repite cada 1/2 s mientras parpadea: puede llegar por la conexión nueva.
            self.esperando_version_hasta = time.monotonic() + ESPERA_VERSION
            self.resultado_carga = ("Cargó. Esperando que el robot diga su versión (hasta %d s: con la batería "
                                    "apagada tarda, porque busca el giroscopio)..." % ESPERA_VERSION, GRIS)
            self._aviso(self.resultado_carga[0], GRIS)

    def _confirmar_con(self, m):
        """Llegó la #PARTIDO después de cargar: ¿tiene lo que se quería?"""
        self.ultimo_partido = m
        self.esperando_version_hasta = None
        if self.lectura is None:
            return
        estado, difs = comparar_con_robot(m, self.lectura)
        if estado == "IGUAL":
            resumen = ", ".join("%s %s" % (n, _g(m["valores"][n])) for n in
                                ("umbralBlanco", "MS_MEDIO_AL_MEDIO", "msAdelante", "umbralCm"))
            texto, color = ("LISTO: el robot tiene el PARTIDO %s con tus valores (lo dijo él: %s, sello %s)." % (
                m["version"], resumen, m["sello"]), VERDE)
            self.estado_archivo = "cargado"
        elif estado == "DISTINTO":
            texto, color = ("Cargó, pero el robot dice otros valores: %s. Cargalo de nuevo." %
                            self._texto_difs(difs), ROJO)
            self.estado_archivo = "no_coincide"
        else:
            texto, color = ("Cargó, pero el robot dice que tiene la %s y el archivo es la %s. Cargalo de nuevo." % (
                m["version"], self.lectura["version"] or "0921f"), ROJO)
            self.estado_archivo = "no_coincide"
        if not m["giro"]:
            texto += (" No encontró el giroscopio (es normal si la batería está apagada; en la cancha, prendé la "
                      "batería ANTES que el USB).")
        self.resultado_carga = (texto, color)
        self._aviso(texto, color)
        self.pendientes.add("partido")

    # ------------------------------------------------------------ latido y cierre

    def _latido(self):
        """Cada 250 ms: lo que depende del reloj."""
        try:
            ahora = time.monotonic()
            cambios = set()
            e = self.estado
            if (self.conectado and self.es_arquero and e.hola is None and self.programa_detectado is None
                    and not self.aviso_programa and self.t_conexion is not None
                    and ahora - self.t_conexion > 3.0):
                self.aviso_programa = True
                cambios |= {"conexion", "sensores"}
            if self.t_pedido is not None and ahora - self.t_pedido >= 3.0:
                self.t_pedido = None
                cambios.add("modo")
                if e.modo == "ESPERA":
                    self._olvidar_ida()          # el !10 no llegó: que no se anote después con ms viejos
                    self.pedido_cancha = None
                    self._aviso("El robot no arrancó la cuenta. ¿Tiene el programa del tablero?", NARANJA)
            ida = self.cancha.get("ida_espera_ok")
            if ida is not None and ahora - ida[1] > ESPERA_OK_IDA:
                self.cancha["ida_espera_ok"] = None
                self._aviso("El robot no confirmó msPruebaIda: la ida NO salió. Apretá IDA de nuevo.", NARANJA)
            if self.t_cortado_andando is not None and ahora >= self.t_cortado_andando:
                self.t_cortado_andando = None
                cambios.add("modo")
            if self.t_pedido_repetir is not None and ahora - self.t_pedido_repetir >= 5.0:
                self.t_pedido_repetir = None
                cambios.add("modo")
                self._aviso("El robot no mandó lo guardado. Probá 'Pedir el último resultado (r)'.", NARANJA)
            if self.t_pedido_panel is not None and ahora - self.t_pedido_panel > 4.0:
                self.t_pedido_panel = None
                self._aviso("No llegó el panel completo. Probá de nuevo.", NARANJA)
            if (e.modo in ("CUENTA", "MOVIENDO") and self.t_ultima_linea is not None
                    and not e.esperando_sin_cable):
                limite = 15.0 if e.modo == "CUENTA" else ((e.tope or 30000) + 5000) / 1000.0
                if ahora - self.t_ultima_linea > limite:
                    e.modo = "ESPERA"
                    e.cuenta = None
                    e.tanda_abierta = False
                    cambios.add("modo")
                    self._aviso("El robot no avisó que terminó. Si se sigue moviendo: PARAR o apagar la batería.", ROJO)
            if self.conectado and e.t_sensores is not None and ahora - e.t_sensores > 1.5:
                cambios.add("sensores")          # para que se vea "los datos son de hace..."
            # (2026-11-08) el programa de partido: ¿lo cambiaron por afuera? (cada 2 s)
            if not self.cargando and ahora - self.t_mirar_partido >= 2.0:
                self.t_mirar_partido = ahora
                self._releer_si_cambio()
            if self.esperando_version_hasta is not None and not self.cargando and ahora > self.esperando_version_hasta:
                self.esperando_version_hasta = None
                # estado_archivo sigue "cargado_sin_confirmar": si la #PARTIDO
                # llega más tarde (también después de desenchufar), confirma igual.
                texto = ("Cargó, pero el robot todavía no dijo su versión (¿es la 0921f?). Para que la repita: "
                         "batería APAGADA, desenchufá y enchufá el USB y esperá ~20 s: cuando la diga, acá sale "
                         "LISTO.")
                self.resultado_carga = (texto, NARANJA)
                self._aviso(texto, NARANJA)
                cambios.add("partido")
            self._pedir_repetir()
            if cambios:
                self._refrescar(cambios)
        except Exception:
            self._falla()
        finally:
            if not self.cerrada:
                self.raiz.after(250, self._latido)

    def _cerrar(self):
        # (2026-11-08) Cargando, NO se cierra: con la ventana cerrada, la carga
        # se queda sin a quién contarle y puede quedar a medias.
        if self.cargando:
            try:
                messagebox.showwarning("Se está cargando", TEXTO_NO_CERRAR, parent=self.raiz)
            except Exception:
                pass
            return
        n = len(self._cambios_reales())
        if n and not messagebox.askyesno(
                "Cambios sin guardar", "Tenés %d cambio%s sin guardar en la pestaña PARTIDO: si cerrás, se "
                "pierden.\n\n¿Cerrar igual?" % (n, "s" if n != 1 else ""), parent=self.raiz):
            return
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
        self.cerrada = True
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
        self.cerrado = False

    def enviar(self, texto):
        self.mandados.append(texto)
        return True

    def enviar_ya(self, texto):
        self.mandados.append(texto)
        return True

    def arrancar(self):
        pass

    def cerrar(self, esperar=False):
        self.cerrado = True


# Los renglones de ejemplo del contrato (DISENO.md 11.1): la ventana TIENE
# que entenderlos (y el robot imprimirlos así).
S_BUENA = ("#S t=12345 l=88,92,61 u=425 r=93.4 g=5 cal=3,3,0,0 p=86,-12,40 d=30.0 y=4.2 vy=0.0 va=0.0 ve=1 "
           "n=3 dg=1,0,193,33,396 am=150,-20,120 az=0,0,-1 q=24 e=ESPERA")
S_SIN_GIRO = ("#S t=500 l=90,85,60 u=425 r=-1 g=-1 cal=-1,-1,-1,-1 p=0,0,-1 d=-1.0 y=0.0 vy=0.0 va=0.0 ve=0 n=0 "
              "dg=1,0,200,0,383 am=0,0,-1 az=0,0,-1 q=0 e=ESPERA")
S_DELANTERO = "#S t=12345 l=310,322,705 u=424,434,440 r=93.4 g=5 p=55,24,40 am=150,-20,120 az=0,0,-1 q=18 e=ESPERA"
HOLA_BUENO = "#HOLA tablero-arquero 2026-11-03 serie=13348550 partido=0.1103"
R_9 = ("#R n=9 mov=ATAJAR nota=PROLIJO como=NORMAL pico=3.4 picoms=4120 fin=0.9 medio=0.4 cruces=0 fuera3=4 "
       "corr=-2 tope=0 mudas=0 racha=0 hueco=11 huecoms=3990 duro=7930 lineams=1010 idams=4342 atrasms=4742 "
       "acomodo=0 laterales=0 idas=2 arranques=3 conpelota=88 zonamuerta=40 velmedia=9.8 intentos=2 sale=1")
R_6 = ("#R n=6 mov=DESPEJE nota=CASI como=NORMAL pico=6.2 picoms=520 fin=1.1 medio=0.8 cruces=1 fuera3=12 corr=-4 "
       "tope=0 mudas=0 racha=0 hueco=12 huecoms=300 duro=3410 lineams=905 idams=383 atrasms=783 acomodo=1 "
       "laterales=0 idas=0 arranques=0 conpelota=0 zonamuerta=0 velmedia=0.0 intentos=0 sale=0")
MIRE_1 = "#MIRE k=1 t=2140 res=NO_VIENE miro=1 cuadros=4 d=38.2 y=-3.1 vy=0.0 va=2.5 av=0 lat=0 ms=0 ang=0.0 recto=0"
MIRE_2 = ("#MIRE k=2 t=3870 res=SALE_MIRANDO miro=1 cuadros=4 d=29.6 y=-7.4 vy=-10.0 va=30.0 av=162 lat=-94 ms=472 "
          "ang=-25.0 recto=0")
ANCHO_BUENO = ("#ANCHO ok=1 error=0 medioizq=2180 izqder=4344 derizq=4547 almedioizq=2213 almedioder=2314 "
               "desvio=2.9,3.4,3.6,2.1 endereza=0 golpe=2.4 vel=100 rampa=10 umbral=425 calgiro=3")
V_BUENA = "#V " + " ".join("%s=%s" % kv for kv in VALORES_DE_MENTIRA.items())
DEF_BUENA = "#DEF " + " ".join("%s=%s" % kv for kv in VALORES_DE_MENTIRA.items())
EJEMPLOS_CONTRATO = (
    (HOLA_BUENO, "HOLA"), ("#GIRO buscando", "GIRO"), ("#GIRO ok=1 estado=5", "GIRO"),
    ("#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210", "GUARDADO"),
    ("#GUARDADO que=cancha hay=0", "GUARDADO"), (S_BUENA, "S"), (S_SIN_GIRO, "S"),
    ("#OK umbralCm=32.0", "OK"), ("#ERROR umbralCm fuera de rango: va de 20.0 a 45.0", "ERROR"),
    ("#CUENTA 3.0 cable", "CUENTA"), ("#CUENTA 10.0 sincable", "CUENTA"),
    ("#INICIO 9 ATAJAR tope=30000 sincable=0", "INICIO"), ("#M 0 0.0 s", "M"), ("#M 3880 0.2 i", "M"),
    ("#M 4500 -1085.4 a", "M"), (MIRE_1, "MIRE"), (MIRE_2, "MIRE"), (R_9, "R"),
    ("#USADOS rendimientoCostado=0.75", "USADOS"),
    ("#CONSEJO Si NO LLEGO de costado (la pelota le quedo mas afuera): BAJAR rendimientoCostado de a 0.05 "
     "(ahora 0.75). Si se PASO de costado: SUBIRLO. Va al reves: mas chico = MAS costado.", "CONSEJO"),
    ("RESULTADO: 9 ATAJAR -> PROLIJO  pico +3.4 grados a los 4120 ms, final +0.9, cruces 0, duro 7930 ms, "
     "vuelta mas larga 11 ms", "TEXTO"),
    ("#FIN 9", "FIN"), ("#REPITE 6 DESPEJE sincable=1 apagado=1", "REPITE"), (R_6, "R"), ("#USADOS", "USADOS"),
    ("#FIN 6", "FIN"), ("#FRENO tecla", "FRENO"), ("#FRENO sobre la linea", "FRENO"),
    ("#PASO A calibrando giro=2", "PASO"), ("#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0", "PASO"),
    ("#PASO I paso=1 ms=1830 como=L", "PASO"),
    ("#LIN fase=v n=3920 izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1", "LIN"), (ANCHO_BUENO, "ANCHO"),
    ("#ANCHO ok=0 error=4 medioizq=2180 izqder=0 derizq=0 almedioizq=0 almedioder=0 desvio=2.9,0.0,0.0,0.0 "
     "endereza=0 golpe=0.0 vel=100 rampa=10 umbral=425 calgiro=3", "ANCHO"),
    ("#FONDO ms=1730 como=TOCO desvio=1.8 umbral=425 probado=0", "FONDO"),
    ("#CAM n=76 cero=0 val=76 xp=84,90,86.9 yp=-1.2 am=0 az=0", "CAM"),
    ("#MEDIO ok=1 pasos=1830,400,2000,943,610,2228,520,320 como=L,T,T,T,L,T,L,T desvio=3.1 acomodo=0", "MEDIO"),
    ("#PANEL float rendimientoCostado = 0.75;", "PANEL"), ("#PANEL const unsigned long MS_MIRAR = 140;", "PANEL"),
    ("#FINPANEL", "FINPANEL"),
    ("=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===", "TEXTO"),
    ("=== REGISTRO DE DESPEJES: los ultimos 12 intentos, del mas viejo al mas nuevo. Este encendido es el 7 ===",
     "TEXTO"),
    ("=== AJUSTAR-MOVIMIENTOS 2026-10-27 — el robot SE MUEVE solo al apretar 1..9 ===", "TEXTO"),
    ("#HOLA tablero-delantero 2026-10-27", "HOLA"), (S_DELANTERO, "ROTA"),
    # 2026-11-03 (revisión): renglones que tablero.ino arma y que no estaban
    # en el DISENO 11.1 (copiados de cómo los imprime el robot).
    ("#INICIO F FONDO_COMPROBAR tope=8000 sincable=1", "INICIO"),
    ("#INICIO L LINEA_GUIADA tope=50000 sincable=1", "INICIO"),
    ("#REPITE L LINEA_GUIADA sincable=1 apagado=1", "REPITE"),
    ("#GUARDADO que=cancha hay=1 n=L nombre=LINEA_GUIADA nota=ERROR sincable=1 nuevo=1 mismo=0 hace=-1",
     "GUARDADO"),
    ("#GUARDADO que=mov hay=1 n=10 nombre=IDA nota=NO_VALE sincable=0 nuevo=0 mismo=1 hace=1520", "GUARDADO"),
    ("#MEDIO ok=0 pasos=1830,0,0,0,0,0,0,0 como=L,-,-,-,-,-,-,- desvio=0.0 acomodo=0", "MEDIO"),
    ("#FRENO no es el arquero", "FRENO"), ("#FRENO tope de tiempo", "FRENO"), ("#FRENO recien parado", "FRENO"),
    ("#FRENO sin giroscopio", "FRENO"), ("#FRENO sin camara", "FRENO"),
    ("(la camara no mando nada, o la frenaron: no hay medicion)", "TEXTO"), ("#FIN C", "FIN"),
)


def _autoprueba_traductor(verificar):
    def tipo(renglon):
        return interpretar(renglon)["tipo"]

    for renglon, esperado in EJEMPLOS_CONTRATO:
        verificar(tipo(renglon) == esperado, "ejemplo del contrato: %r da %s" % (renglon[:50], tipo(renglon)))
    # --- #S ---
    m = interpretar(S_BUENA)
    verificar(m["t"] == 12345 and m["e"] == "ESPERA", "S: t y e")
    verificar(m["l"] == [88, 92, 61] and m["u"] == 425, "S: línea y UN umbral")
    verificar(m["r"] == 93.4 and m["g"] == 5 and m["q"] == 24 and m["cal"] == [3, 3, 0, 0], "S: rumbo, estado, cal, cámara")
    verificar(m["p"] == (86, -12, 40) and m["am"] == (150, -20, 120) and m["az"] == (0, 0, -1), "S: pelota y arcos")
    verificar(m["d"] == 30.0 and m["y"] == 4.2 and m["vy"] == 0.0 and m["va"] == 0.0, "S: d y v")
    verificar(m["ve"] is True and m["n"] == 3 and m["dg"] == (1, 0, 193, 33, 396), "S: ve, n, dg")
    m = interpretar(S_SIN_GIRO)
    verificar(m["r"] is None and m["g"] is None and m["cal"] is None, "S sin giroscopio")
    verificar(m["d"] is None and not visible(m["p"]) and m["ve"] is False, "S: nunca la vio")
    verificar(tipo(S_BUENA + "\r") == "S" and tipo(S_BUENA + "\r\n") == "S", "S con \\r y \\r\\n")
    verificar(interpretar(S_BUENA.replace("r=93.4", "r=nan"))["r"] is None, "S: r=nan no rompe el renglón")
    verificar(interpretar(S_BUENA.replace("q=24", "q=17.5"))["q"] == 17.5, "S: q con decimales")
    verificar(tipo(S_BUENA.replace(" e=ESPERA", " zz=9 e=ESPERA")) == "S", "S con un campo de más (se ignora)")
    for k in range(len(S_BUENA)):
        verificar(tipo(S_BUENA[:k]) != "S", "S cortada en %d se tomó como buena" % k)
    for malo, que in ((S_BUENA.replace("dg=1,0,193,33,396", "dg=1,0,193,33"), "dg con 4"),
                      (S_BUENA.replace("u=425", "u=424,434,440"), "u con 3"),
                      (S_BUENA.replace("r=93.4", "r=93,4"), "coma decimal"),
                      (S_BUENA[:40] + S_BUENA, "dos pegados"), (S_BUENA.replace(" e=ESPERA", ""), "sin e"),
                      (S_BUENA.replace("e=ESPERA", "e=ESP"), "e cortado"), ("#S", "vacío"),
                      (S_BUENA.replace("t=12345 ", "") + " t=1", "t no va primero"),
                      (S_BUENA.replace("l=88,92,61", "l=88,92"), "2 sensores"),
                      (S_BUENA.replace("cal=3,3,0,0", "cal=3,3,0"), "cal con 3"),
                      (S_BUENA.replace("p=86,-12,40", "p=86,-12"), "pelota cortada"),
                      (S_BUENA.replace("ve=1", "ve=2"), "ve raro"), ("#S#INICIO 1 X tope=1 sincable=0", "pegados")):
        verificar(tipo(malo) == "ROTA", "S mala (%s)" % que)
    # --- #HOLA ---
    m = interpretar(HOLA_BUENO)
    verificar(m["nombre"] == PROGRAMA and m["fecha"] == FECHA_CONTRATO and m["serie"] == SERIE_ARQUERO
              and m["partido"] == "0.1103", "HOLA")
    m = interpretar("#HOLA tablero-delantero 2026-10-27")
    verificar(m["nombre"] == PROGRAMA_DELANTERO and m["serie"] is None, "HOLA del delantero")
    verificar(tipo("#HOLA") == "ROTA", "HOLA vacío")
    # --- #V #DEF #OK con camelCase ---
    m = interpretar(V_BUENA)
    verificar(list(m["valores"]) == list(ORDEN_NUMEROS) and m["valores"]["umbralCm"] == "30.0"
              and m["valores"]["MS_MIRAR"] == "120", "V con los 46 camelCase")
    verificar(interpretar(DEF_BUENA)["valores"] == VALORES_DE_MENTIRA, "DEF")
    for malo in ("#V", "#V umbralCm=", "#V umbralCm=1,5", "#V 1umbral=3", "#V umbralCm=nan", "#V umbralCm",
                 "#DEF", "#DEF _x=3", "#OK 9MS=3"):
        verificar(tipo(malo) == "ROTA", "V/DEF/OK malo: %r" % malo)
    m = interpretar("#OK umbralCm=32.0")
    verificar(m["nombre"] == "umbralCm" and m["valor"] == "32.0", "OK camelCase")
    verificar(interpretar("#OK MS_MIRAR=140")["nombre"] == "MS_MIRAR", "OK MAYÚSCULAS")
    for malo in ("#OK umbralCm", "#OK a=1 b=2", "#OK", "#OK umbralCm=treinta"):
        verificar(tipo(malo) == "ROTA", "OK malo: %r" % malo)
    verificar(interpretar("#ERROR no hay nada guardado")["mensaje"] == "no hay nada guardado", "ERROR")
    # --- #GIRO #CUENTA #INICIO #FIN #REPITE #GUARDADO ---
    verificar(interpretar("#GIRO buscando")["buscando"] and interpretar("#GIRO ok=0 estado=-1")["estado"] is None,
              "GIRO")
    verificar(interpretar("#GIRO ok=1 estado=5")["ok"] is True, "GIRO ok")
    verificar(tipo("#GIRO") == "ROTA" and tipo("#GIRO ok=3 estado=5") == "ROTA", "GIRO malo")
    m = interpretar("#CUENTA 10.0 sincable")
    verificar(m["segundos"] == 10.0 and m["sincable"] and m["modo"] == "sincable", "CUENTA sincable")
    for malo in ("#CUENTA 3.0 rapido", "#CUENTA 3.0", "#CUENTA", "#CUENTA uno cable"):
        verificar(tipo(malo) == "ROTA", "CUENTA mala: %r" % malo)
    for codigo in CODIGOS:
        m = interpretar("#INICIO %s ALGO tope=1000 sincable=1" % codigo)
        verificar(m["tipo"] == "INICIO" and m["codigo"] == codigo and m["tope"] == 1000 and m["sincable"],
                  "INICIO %s" % codigo)
        verificar(interpretar("#FIN %s" % codigo)["codigo"] == codigo, "FIN %s" % codigo)
    for malo in ("#INICIO 11 X tope=1 sincable=0", "#INICIO Z X tope=1 sincable=0", "#INICIO 6 DESPEJE",
                 "#INICIO 6 despeje tope=1 sincable=0", "#INICIO 6 DESPEJE tope=1 sincable=2", "#FIN", "#FIN 11",
                 "#FIN 6 7"):
        verificar(tipo(malo) == "ROTA", "INICIO/FIN malo: %r" % malo)
    m = interpretar("#REPITE 9 ATAJAR sincable=1 apagado=0")
    verificar(m["codigo"] == "9" and m["sincable"] and not m["apagado"], "REPITE")
    m = interpretar("#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210")
    verificar(m["hay"] and m["nuevo"] and m["mismo"] and m["n"] == "9" and m["hace"] == 48210, "GUARDADO")
    m = interpretar("#GUARDADO que=cancha hay=1 n=A nombre=ANCHO nota=OK sincable=1 nuevo=0 mismo=0 hace=-1")
    verificar(m["n"] == "A" and m["hace"] is None and not m["nuevo"], "GUARDADO de cancha")
    verificar(tipo("#GUARDADO que=otra hay=0") == "ROTA" and tipo("#GUARDADO que=mov hay=1 n=99") == "ROTA",
              "GUARDADO malo")
    # --- #M #MIRE #R #USADOS ---
    verificar(interpretar("#M 4500 -1085.4 a")["grados"] == -1085.4, "M grande")
    for f in FASES:
        verificar(interpretar("#M 10 1.0 %s" % f)["fase"] == f, "M fase %s" % f)
    for malo in ("#M 10 1.0", "#M 10 1.0 z", "#M 10 nan i", "#M -5 1.0 i", "#M 10 1.0 i x"):
        verificar(tipo(malo) == "ROTA", "M mala: %r" % malo)
    for res in RESULTADOS_MIRE:
        verificar(interpretar(MIRE_1.replace("NO_VIENE", res))["res"] == res, "MIRE %s" % res)
    verificar(tipo(MIRE_1.replace("NO_VIENE", "GOL")) == "ROTA", "MIRE con res desconocido")
    m = interpretar(MIRE_2)
    verificar(m["lat"] == -94 and m["ang"] == -25.0 and m["ms"] == 472 and m["va"] == 30.0, "MIRE: datos")
    m = interpretar(R_9)
    verificar(m["n"] == 9 and m["nota"] == "PROLIJO" and m["como"] == "NORMAL" and m["sale"] == 1
              and m["velmedia"] == 9.8 and m["idams"] == 4342 and m["intentos"] == 2, "R de la 9")
    verificar(all(c in m for c in CAMPOS_R), "R con los 29 campos")
    for nota in NOTAS:
        verificar(interpretar(R_6.replace("nota=CASI", "nota=" + nota))["nota"] == nota, "R con nota %s" % nota)
    for malo in (R_6.replace("nota=CASI", "nota=GENIAL"), R_6.replace("n=6", "n=11"), R_6[:30],
                 R_6.replace(" nota=CASI", ""), R_6.replace("pico=6.2", "pico=abc"), R_6.replace("como=NORMAL", "como=x")):
        verificar(tipo(malo) == "ROTA", "R mala")
    verificar(interpretar("#USADOS")["valores"] == {}, "USADOS vacío")
    verificar(interpretar("#USADOS rendimientoCostado=0.75 umbralCm=32.0")["valores"] ==
              {"rendimientoCostado": "0.75", "umbralCm": "32.0"}, "USADOS")
    # --- cancha ---
    m = interpretar(ANCHO_BUENO)
    verificar(m["ok"] and m["almedioizq"] == 2213 and m["desvio"] == [2.9, 3.4, 3.6, 2.1] and m["golpe"] == 2.4, "ANCHO")
    m = interpretar("#LIN fase=v n=3920 izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1")
    verificar(m["fase"] == "v" and m["izq"] == [71, 97, 84.0] and m["n"] == 3920, "LIN")
    m = interpretar("#MEDIO ok=1 pasos=1830,400,2000,943,610,2228,520,320 como=L,T,T,T,L,T,L,T desvio=3.1 acomodo=0")
    verificar(m["pasos"][5] == 2228 and m["como"][0] == "L", "MEDIO")
    m = interpretar("#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0")
    verificar(m["codigo"] == "A" and m["datos"]["ms"] == 2180 and m["datos"]["desvio"] == 2.9, "PASO")
    verificar(interpretar("#PASO A calibrando giro=2")["palabras"] == ["calibrando"], "PASO calibrando")
    verificar(interpretar("#PASO I paso=1 ms=1830 como=L")["datos"]["como"] == "L", "PASO de I")
    verificar(interpretar("#CAM n=76 cero=0 val=76 xp=84,90,86.9 yp=-1.2 am=0 az=0")["xp"][2] == 86.9, "CAM")
    for malo in ("#LIN fase=x n=1 izq=1,2,3 der=1,2,3 ade=1,2,3", "#LIN fase=v n=1 izq=1,2 der=1,2,3 ade=1,2,3",
                 "#FONDO ms=1 como=VOLO", "#MEDIO ok=1 pasos=1,2 como=L,T", "#CAM n=1", "#ANCHO ok=1",
                 "#MEDIO ok=1 pasos=1,2,3,4,5,6,7,8 como=L,T,T,T,L,T,L,X", "#PASO", "#PASO Z ms=1"):
        verificar(tipo(malo) == "ROTA", "cancha mala: %r" % malo)
    # --- números de 400 cifras en cada campo ---
    largo = "9" * 400
    for renglon in (S_BUENA, R_9, MIRE_2, ANCHO_BUENO, "#FONDO ms=1730 como=TOCO desvio=1.8 umbral=425 probado=0",
                    "#CAM n=76 cero=0 val=76 xp=84,90,86.9 yp=-1.2 am=0 az=0", "#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0",
                    "#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210"):
        cabeza, resto = renglon.split(" ", 1)
        piezas = resto.split()
        for k, pieza in enumerate(piezas):
            clave, igual, valor = pieza.partition("=")
            if not igual or not re.match(r"^[-\d.,]+$", valor):
                continue
            otro = list(piezas)
            otro[k] = clave + "=" + ",".join(largo for _ in valor.split(","))
            verificar(tipo(cabeza + " " + " ".join(otro)) == "ROTA", "400 cifras en %s %s" % (cabeza, clave))
    for malo in ("#M 10 %s i" % largo, "#M %s 1.0 i" % largo, "#CUENTA %s cable" % largo, "#OK umbralCm=" + largo,
                 "#V umbralCm=" + largo, "#INICIO 6 DESPEJE tope=%s sincable=0" % largo):
        verificar(tipo(malo) == "ROTA", "400 cifras: %r" % malo[:30])
    try:
        leer_numero(largo)
        verificar(False, "leer_numero aceptó 400 cifras")
    except ValueError:
        pass
    # --- #PANEL, TEXTO y DESCONOCIDA ---
    codigo = "const unsigned long MS_MIRAR                     = 140;   // comentario"
    verificar(interpretar("#PANEL " + codigo)["codigo"] == codigo, "PANEL respeta los espacios")
    for humano in ("Hola", "", "  #S t=1 e=ESPERA", "PARADO (frena 0,4 s y suelta las ruedas)", ">>> 6 DESPEJE en 3 s"):
        verificar(tipo(humano) == "TEXTO", "renglón para personas: %r" % humano)
    verificar(tipo(None) == "TEXTO" and tipo("#ZZZ 1") == "DESCONOCIDA" and tipo("#") == "DESCONOCIDA", "otros")
    verificar(tipo("#CONSEJO subir KP_RUMBO#S t=1 e=ESPERA") == "ROTA", "CONSEJO pegado con un #S")
    # --- a lo bruto ---
    rng = random.Random(7)
    buenos = [r for r, _ in EJEMPLOS_CONTRATO] + ["�� basura", "#S t=1 e=ESPERA"]
    validos = set(ETIQUETAS) | {"TEXTO", "ROTA", "DESCONOCIDA"}
    for _ in range(3000):
        a, b = rng.choice(buenos), rng.choice(buenos)
        renglon = a[rng.randint(0, len(a)):] + b[:rng.randint(0, len(b))]
        verificar(interpretar(renglon)["tipo"] in validos, "a lo bruto: %r" % renglon[:60])
    # --- reconocer_programa ---
    verificar(reconocer_programa("=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===")
              == "PARTIDO 0.1103 EN PRUEBA (0.1027d + registro de despejes)", "reconoce el partido")
    verificar(reconocer_programa("=== REGISTRO DE DESPEJES: los ultimos 12 ===") == "PARTIDO", "reconoce el registro")
    verificar(reconocer_programa("=== AJUSTAR-MOVIMIENTOS 2026-10-27 — el robot SE MUEVE ===") == "AJUSTAR",
              "reconoce ajustar")
    verificar(reconocer_programa("=============== AJUSTAR-MOVIMIENTOS 2026-10-27 ===============") == "AJUSTAR",
              "reconoce ajustar (la ayuda)")
    verificar(reconocer_programa("=== TABLERO DEL ARQUERO 2026-11-03 (copias del partido 0.1103) - NO ES EL DE "
                                 "PARTIDO ===") == "TABLERO_ARQUERO", "reconoce este tablero")
    verificar(reconocer_programa("hola") is None and reconocer_programa("=== OTRA COSA ===") is None, "no reconoce otros")


def _autoprueba_memoria(verificar):
    e = Estado()
    verificar(e.modo == "ESPERA" and e.hola is None, "memoria nueva")
    c = e.aplicar(interpretar(HOLA_BUENO))
    verificar("hola" in c and e.hola == (PROGRAMA, FECHA_CONTRATO, SERIE_ARQUERO, "0.1103"), "memoria: HOLA")
    e.aplicar(interpretar(V_BUENA))
    e.aplicar(interpretar(DEF_BUENA))
    verificar(e.valores["umbralCm"] == VALORES_DE_MENTIRA["umbralCm"] and e.defectos == VALORES_DE_MENTIRA, "memoria: V y DEF")
    c = e.aplicar(interpretar(S_BUENA), 10.0)
    verificar("sensores" in c and e.t_sensores == 10.0 and e.sensores["q"] == 24, "memoria: S")
    verificar(e.aplicar(interpretar(S_DELANTERO)) == {"rota"} and e.sensores["q"] == 24, "memoria: un #S roto no pisa")
    # la secuencia normal
    e.aplicar(interpretar("#CUENTA 3.0 cable"))
    verificar(e.modo == "CUENTA" and e.cuenta == 3.0 and not e.esperando_sin_cable, "memoria: CUENTA")
    e.aplicar(interpretar("#INICIO 9 ATAJAR tope=30000 sincable=0"))
    verificar(e.modo == "MOVIENDO" and e.codigo == "9" and e.tope == 30000 and e.tanda_abierta, "memoria: INICIO")
    for r in ("#M 0 0.0 s", "#M 3880 0.2 i", MIRE_1, MIRE_2):
        e.aplicar(interpretar(r))
    e.aplicar(interpretar(R_9))
    verificar(e.modo == "MOVIENDO" and e.informe["nota"] == "PROLIJO", "memoria: el R ya NO vuelve a ESPERA")
    e.aplicar(interpretar("#USADOS rendimientoCostado=0.75"))
    e.aplicar(interpretar("#CONSEJO uno"))
    c = e.aplicar(interpretar("#FIN 9"))
    verificar(e.modo == "ESPERA" and e.completo and not e.tanda_abierta and "fin" in c, "memoria: FIN vuelve a ESPERA")
    verificar(len(e.muestras) == 2 and len(e.intentos) == 2 and e.usados == {"rendimientoCostado": "0.75"}
              and e.consejos == ["uno"], "memoria: la tanda entera")
    # R sin FIN no vuelve a ESPERA, pero un #S sí
    e.aplicar(interpretar("#CUENTA 3.0 cable"))
    e.aplicar(interpretar("#INICIO 6 DESPEJE tope=20000 sincable=0"))
    e.aplicar(interpretar(R_6))
    verificar(e.modo == "MOVIENDO", "memoria: R sin FIN")
    e.aplicar(interpretar(S_BUENA))
    verificar(e.modo == "ESPERA" and not e.tanda_abierta, "memoria: un #S en espera sí")
    # tecla en la cuenta
    e.aplicar(interpretar("#CUENTA 3.0 cable"))
    verificar(e.informe is None, "memoria: cuenta nueva borra")
    c = e.aplicar(interpretar("#FRENO tecla"))
    verificar(e.modo == "ESPERA" and e.freno == "tecla" and "modo" in c, "memoria: tecla en la cuenta")
    # tecla moviéndose
    e.aplicar(interpretar("#CUENTA 3.0 cable"))
    e.aplicar(interpretar("#INICIO 1 COSTADO_IZQ tope=4000 sincable=0"))
    e.aplicar(interpretar("#FRENO tecla"))
    verificar(e.modo == "MOVIENDO" and e.freno == "tecla", "memoria: tecla moviéndose")
    e.aplicar(interpretar("#M 10 0.5 l"))
    e.aplicar(interpretar(R_6.replace("n=6 mov=DESPEJE nota=CASI como=NORMAL", "n=1 mov=COSTADO_IZQ nota=NO_VALE como=TECLA")))
    e.aplicar(interpretar("#FIN 1"))
    verificar(e.modo == "ESPERA" and e.informe["nota"] == "NO_VALE" and e.freno == "tecla", "memoria: NO_VALE")
    # FRENO sin INICIO borra la nota vieja
    c = e.aplicar(interpretar("#FRENO sobre la linea"))
    verificar(e.informe is None and e.muestras == [] and e.freno == "sobre la linea" and "resultado" in c,
              "memoria: FRENO sin INICIO borra la nota vieja")
    # #M sin INICIO (reconectado) empieza otro resultado
    e = Estado()
    for r in ("#CUENTA 3.0 cable", "#INICIO 6 DESPEJE tope=20000 sincable=0", "#M 0 1.0 i", R_6, "#FIN 6"):
        e.aplicar(interpretar(r))
    e.reiniciar_conexion()
    verificar(e.informe is not None, "memoria: reconectar guarda el resultado")
    e.aplicar(interpretar("#M 10 5.0 l"))
    verificar(e.informe is None and e.muestras == [(10, 5.0, "l")] and e.codigo is None, "memoria: #M sin INICIO")
    # REPITE no cambia el modo; PASO tampoco
    e = Estado()
    c = e.aplicar(interpretar("#REPITE 6 DESPEJE sincable=1 apagado=1"))
    verificar(e.modo == "ESPERA" and e.de_guardado and e.apagado and e.sincable and e.codigo == "6", "memoria: REPITE")
    e.aplicar(interpretar(R_6))
    e.aplicar(interpretar("#FIN 6"))
    verificar(e.informe["n"] == 6 and e.de_guardado and e.modo == "ESPERA", "memoria: la tanda repetida")
    e.aplicar(interpretar("#CUENTA 3.0 cable"))
    e.aplicar(interpretar("#INICIO A ANCHO tope=45000 sincable=0"))
    e.aplicar(interpretar("#PASO A calibrando giro=2"))
    e.aplicar(interpretar("#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0"))
    verificar(e.modo == "MOVIENDO" and len(e.pasos) == 2, "memoria: PASO no cambia el modo")
    e.aplicar(interpretar(ANCHO_BUENO))
    c = e.aplicar(interpretar("#FIN A"))
    verificar("cancha_lista" in c and e.cancha_lista[0] == "A" and e.cancha["A"]["almedioizq"] == 2213,
              "memoria: la tanda de cancha queda por código")
    for f in ("v", "i"):
        e.aplicar(interpretar("#LIN fase=%s n=1 izq=1,2,3 der=1,2,3 ade=1,2,3" % f))
    e.aplicar(interpretar("#FIN L"))
    verificar(set(e.cancha["L"]) == {"v", "i"} and len(e.cancha_lista[1]) == 2, "memoria: la línea queda por fase")
    # GUARDADO nuevo=1 deja el pedido de r
    e = Estado()
    e.aplicar(interpretar(HOLA_BUENO))
    e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210"))
    e.aplicar(interpretar("#GUARDADO que=cancha hay=0"))
    verificar(e.repetir == ["r"], "memoria: GUARDADO nuevo=1 pide r")
    e.aplicar(interpretar("#GUARDADO que=cancha hay=1 n=A nombre=ANCHO nota=OK sincable=1 nuevo=1 mismo=1 hace=10"))
    verificar(e.repetir == ["r", "rc"], "memoria: y rc")
    e.aplicar(interpretar(HOLA_BUENO))
    verificar(e.repetir == [], "memoria: otro HOLA empieza de cero")
    # SIN CABLE: reiniciar_conexion conserva el resultado y esperando_sin_cable
    e = Estado()
    e.aplicar(interpretar("#CUENTA 10.0 sincable"))
    verificar(e.esperando_sin_cable and e.cuenta_modo == "sincable", "memoria: CUENTA sincable")
    e.reiniciar_conexion()
    verificar(e.esperando_sin_cable and e.modo == "ESPERA", "memoria: reconectar conserva esperando_sin_cable")
    e.que_sin_cable = "mov"
    e.aplicar(interpretar(HOLA_BUENO))
    e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=1 mismo=1 hace=10"))
    verificar(e.esperando_sin_cable and e.repetir == ["r"], "memoria: vuelve y hay algo nuevo")
    e.aplicar(interpretar("#REPITE 6 DESPEJE sincable=1 apagado=0"))
    e.aplicar(interpretar(R_6))
    e.aplicar(interpretar("#FIN 6"))
    verificar(not e.esperando_sin_cable and e.de_guardado and e.informe["n"] == 6, "memoria: FIN apaga esperando_sin_cable")
    e.aplicar(interpretar("#CUENTA 10.0 sincable"))
    c = e.aplicar(interpretar("#FRENO tecla"))
    verificar(not e.esperando_sin_cable, "memoria: frenado en la cuenta sin cable: no hay nada que esperar")
    # (con el reloj de la PC: la cuenta empieza a los 100 s y vuelve a los 130 s)
    e.aplicar(interpretar("#CUENTA 10.0 sincable"), 100.0)
    e.reiniciar_conexion()
    e.aplicar(interpretar(HOLA_BUENO), 130.0)
    c = e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=0 mismo=1 "
                              "hace=60000"), 130.0)
    verificar(not e.esperando_sin_cable and "sin_cable_perdido" in c and e.repetir == [],
              "memoria: volvió sin nada nuevo (lo apagaron)")
    # nuevo=0 pero terminó DESPUÉS de la cuenta (un tablero.ino de antes lo
    # marcaba visto al mandarlo AL VACÍO): se pide igual
    e.aplicar(interpretar("#CUENTA 10.0 sincable"), 200.0)
    e.reiniciar_conexion()
    e.aplicar(interpretar(HOLA_BUENO), 240.0)
    c = e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=0 mismo=1 "
                              "hace=15000"), 240.0)
    verificar(e.esperando_sin_cable and e.repetir == ["r"] and "sin_cable_perdido" not in c,
              "memoria: nuevo=0 pero de después de la cuenta: se pide igual")
    e.aplicar(interpretar(HOLA_BUENO), 241.0)
    c = e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=0 mismo=0 "
                              "hace=-1"), 241.0)
    verificar("sin_cable_perdido" in c and e.repetir == [], "memoria: nuevo=0 de antes de apagar: perdido")
    # lo que llegó EN VIVO no se pide de nuevo, aunque el robot diga nuevo=1
    # (una prueba SIN CABLE queda nuevo=1: el Teensy no sabe si lo desenchufaron)
    e = Estado()
    for r, t in (("#CUENTA 10.0 sincable", 300.0), ("#INICIO 6 DESPEJE tope=20000 sincable=1", 310.0),
                 (R_6, 314.0), ("#FIN 6", 314.2)):
        e.aplicar(interpretar(r), t)
    verificar(e.vistos == [("mov", "6", 314.2)], "memoria: el FIN en vivo queda visto %r" % e.vistos)
    e.reiniciar_conexion()
    e.aplicar(interpretar(HOLA_BUENO), 330.0)
    e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=1 mismo=1 "
                          "hace=16000"), 330.0)
    verificar(e.repetir == [], "memoria: lo que llegó en vivo no se pide de nuevo")
    e.aplicar(interpretar(HOLA_BUENO), 331.0)
    e.aplicar(interpretar("#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=1 mismo=1 "
                          "hace=1000"), 331.0)
    verificar(e.repetir == ["r"], "memoria: otro resultado (más nuevo) sí se pide")
    e.aplicar(interpretar("#REPITE 6 DESPEJE sincable=1 apagado=0"), 332.0)
    e.aplicar(interpretar(R_6), 332.0)
    e.aplicar(interpretar("#FIN 6"), 332.1)
    verificar(len(e.vistos) == 1, "memoria: lo repetido con r no se anota como visto en vivo")
    # las tandas de PANEL
    e.aplicar(interpretar("#PANEL float rendimientoCostado = 0.75;"))
    e.aplicar(interpretar("#PANEL const unsigned long MS_MIRAR = 140;"))
    c = e.aplicar(interpretar("#FINPANEL"))
    verificar("panel_listo" in c and e.panel == ["float rendimientoCostado = 0.75;",
                                                 "const unsigned long MS_MIRAR = 140;"], "memoria: PANEL")
    c = e.aplicar(interpretar("#FINPANEL"))
    verificar(e.panel == [], "memoria: FINPANEL solo = nada para copiar")
    c = e.aplicar(interpretar("#GIRO buscando"))
    verificar("giro" in c and e.giro["buscando"], "memoria: GIRO")


def _autoprueba_cuentas(verificar):
    # el umbral (15/09)
    lin = [interpretar(LINEAS_DE_MENTIRA[f]) for f in ("v", "i", "d")]
    r = calcular_umbral(lin)
    verificar(r["umbral"] == 425 and r["margen_verde"] == 328 and r["margen_blanco"] == 328 and not r["avisos"],
              "umbral 97/753 -> 425 (%r)" % r)
    verificar(calcular_umbral(lin[:1])["falta"] == ["BLANCO IZQ", "BLANCO DER"], "umbral: falta el blanco")
    mal = [lin[0], interpretar("#LIN fase=i n=1 izq=50,90,80.0 der=1,2,3 ade=1,2,3"),
           interpretar("#LIN fase=d n=1 izq=1,2,3 der=50,90,80.0 ade=1,2,3")]
    verificar(calcular_umbral(mal)["umbral"] is None and "SUPERPONEN" in calcular_umbral(mal)["avisos"][0][0],
              "umbral: superpuestos")
    fragil = [lin[0], interpretar("#LIN fase=i n=1 izq=200,250,240.0 der=1,2,3 ade=1,2,3"),
              interpretar("#LIN fase=d n=1 izq=1,2,3 der=200,260,250.0 ade=1,2,3")]
    verificar(any("Frágil" in a for a, _ in calcular_umbral(fragil)["avisos"]), "umbral: frágil")
    sucia = [lin[0], interpretar("#LIN fase=i n=1 izq=90,763,200.0 der=1,2,3 ade=1,2,3"), lin[2]]
    verificar(any("sucia" in a for a, _ in calcular_umbral(sucia)["avisos"]), "umbral: muestra sucia")
    tres = [interpretar("#LIN fase=v n=1 izq=1,60,3 der=1,50,3 ade=1,2,3"), lin[0],
            interpretar("#LIN fase=v n=1 izq=1,80,3 der=1,90,3 ade=1,2,3"), lin[1], lin[2]]
    verificar(calcular_umbral(tres)["peor_verde"] == 97, "umbral: el verde en 3 lugares toma el máximo")
    # el ancho (06/10)
    for cruce, medio in ((4344, 2213), (4405, 2243), (4547, 2314), (4615, 2348)):
        verificar(ms_hasta_el_medio(cruce) == medio, "ms_hasta_el_medio(%d) = %d" % (cruce, ms_hasta_el_medio(cruce)))
    verificar(numeros_creibles(1000, 1400, 980) == (True, True) and numeros_creibles(1000, 3000, 4290) == (True, False)
              and numeros_creibles(1000, 1399)[0] is False and numeros_creibles(1000, 3001)[0] is False
              and numeros_creibles(1000, 3000, 2100)[1] is True and numeros_creibles(1000, 3000, 2099)[1] is False
              and numeros_creibles(1000, 3000, 4285)[1] is True, "numeros_creibles en los bordes")
    a1, a2 = interpretar(ANCHO_BUENO), interpretar(ANCHO_BUENO.replace("2213", "2243"))
    verificar(sugerido_ancho([a1, a2])[0] == 2228 and sugerido_ancho([a1, a2])[2] == "", "sugerido_ancho 2213/2243 -> 2228")
    verificar("tercera" in sugerido_ancho([a1, interpretar(ANCHO_BUENO.replace("2213", "2400"))])[2], "ancho: más de 3 %")
    verificar(sugerido_ancho([interpretar(ANCHO_BUENO.replace("ok=1", "ok=0"))])[0] is None, "ancho: la mala no cuenta")
    a8 = interpretar(ANCHO_BUENO.replace("rampa=10", "rampa=8"))
    verificar(sugerido_ancho([a1, a2], 10)[0] == 2228, "ancho: con la rampa del partido cuentan todas")
    sug, _, aviso = sugerido_ancho([a8], 10)
    verificar(sug is None and "RAMPA_MOV_PASO 8" in aviso, "ancho: medida con otra rampa NO da sugerido %r" % aviso)
    sug, _, aviso = sugerido_ancho([a1, a8], 10)
    verificar(sug == 2213 and "otra RAMPA_MOV_PASO" in aviso, "ancho: la de otra rampa no entra en el promedio")
    verificar(numeros_fuera_del_partido({"RAMPA_MOV_PASO": "8", "frenarAntesDeVolver": "1", "KP_RUMBO": "5.0"},
                                        {"RAMPA_MOV_PASO": "10", "frenarAntesDeVolver": "true", "KP_RUMBO": "5.0",
                                         "LADO_TRASERA": "89"},
                                        ("RAMPA_MOV_PASO", "frenarAntesDeVolver", "KP_RUMBO", "LADO_TRASERA"))
              == [("RAMPA_MOV_PASO", "8", "10")], "numeros_fuera_del_partido")
    verificar(sugerido_fondo([{"ms": 1730, "como": "TOCO"}, {"ms": 1690, "como": "TOCO"}]) == 2250, "fondo -> 2250")
    verificar(sugerido_fondo([{"ms": 1730, "como": "LINEA"}]) is None, "fondo sin TOCÓ")
    a, b = recta([10, 20, 30, 40, 50], [28, 58, 88.9, 117, 142])
    verificar(round(a, 2) == 2.87 and round(b, 2) == 0.68, "recta de la cámara 25/08: %r" % ((a, b),))
    a, b = recta([150, 250, 350], [13, 19, 33])
    verificar(round(a, 2) == 0.1 and round(b, 2) == -3.33 and ms_para_cm(a, b, 35) == 383, "recta de las idas 04/08")
    verificar(recta([1], [2]) is None and recta([3, 3], [1, 2]) is None, "recta imposible")
    # la diagonal (DISENO 7.4)
    for d, y, vy, va, miro, esperado in ((29.97, 4.18, 0, 0, False, (1, 0, 193, 33, 396)),
                                         (30, 0, 0, 0, False, (1, 0, 200, 0, 383)),
                                         (30, 10, 0, 0, False, (1, 0, 173, 72, 442)),
                                         (20, 15, 0, 0, False, (0, 2, 142, 0, 383)),
                                         (28, -6, -10, 30, True, (1, 0, 167, -85, 458)),
                                         (29.6, -7.4, -10, 30, True, (1, 0, 162, -94, 472)),
                                         (25, 14, 30, 40, True, (0, 1, 107, 0, 383))):
        dg = calcular_diagonal(d, y, vy, va, miro, True, VALORES_DE_MENTIRA)
        verificar(dg[:5] == esperado, "calcular_diagonal(%r, %r): %r" % (d, y, dg))
    verificar(round(calcular_diagonal(29.6, -7.4, -10, 30, True, True, VALORES_DE_MENTIRA)[5], 1) == -25.0, "ángulo -25")
    verificar(round(calcular_diagonal(30, 10, 0, 0, False, True, VALORES_DE_MENTIRA)[5], 1) == 18.4, "ángulo 18.4")
    verificar(calcular_diagonal(30, 10, 0, 0, False, True, dict(VALORES_DE_MENTIRA, despejeEnDiagonal="0"))[:5]
              == (1, 0, 200, 0, 383), "sin diagonal sale derecho")
    verificar(round(angulo_salida(193, 33, 0.8)) == 8, "angulo_salida ~8°")
    # qué haría (los 7 casos)
    s = interpretar(S_BUENA)
    casos = ((s, "SALDRIA"), (interpretar(S_SIN_GIRO), "NO_LA_VE"),
             (dict(s, y=25.0), "MUY_AL_COSTADO"), (dict(s, d=55.0), "LEJOS"), (dict(s, d=35.0), "MIRARIA"),
             (dict(s, dg=(0, 2, 142, 0, 383)), "NO_SALDRIA"), (dict(s, n=1), "JUNTANDO"))
    for sensores, esperado in casos:
        verificar(que_haria(sensores, VALORES_DE_MENTIRA)[0] == esperado, "que_haria %s" % esperado)
    verificar("8° a la DERECHA" in que_haria(s, VALORES_DE_MENTIRA)[1], "que_haria: 8° a la DERECHA")
    verificar("derecho" in que_haria(dict(s, dg=(1, 0, 200, 0, 383)), VALORES_DE_MENTIRA)[1], "que_haria derecho")
    verificar("muy abierto" in que_haria(dict(s, dg=(0, 2, 142, 0, 383)), VALORES_DE_MENTIRA)[1], "que_haria abierto")
    sin_mirar = dict(VALORES_DE_MENTIRA, medirAntesDeDespejar="0")
    verificar(que_haria(dict(s, d=35.0), sin_mirar)[0] == "LEJOS", "que_haria sin mirar: a 35 cm está lejos")
    verificar(que_haria(s, sin_mirar)[0] == "SALDRIA", "que_haria sin mirar: a 30 sale")
    # la pantalla: ⚠ en el arquero Y + = DERECHA
    verificar(cm_a_pantalla(10, 5, 400, 500, 2) == (410, 480), "cm_a_pantalla")
    verificar(cm_a_pantalla(30, 4.2, 400, 500, 2)[0] > 400, "una pelota con y > 0 queda a la DERECHA del centro")
    verificar(lado_pelota(4.2) == "4.2 cm a la DERECHA" and lado_pelota(-3) == "3.0 cm a la IZQUIERDA", "lado_pelota")
    verificar(lado(-3.24) == "3.2° a la DERECHA" and lado(2) == "2.0° a la IZQUIERDA", "lado del giroscopio")
    for l, esperado in (([88, 92, 61], "-"), ([500, 92, 61], "IZQ"), ([88, 500, 61], "DER"), ([500, 500, 61], "AREA")):
        verificar(estado_linea(l, 425) == esperado, "estado_linea %s" % esperado)
    # la pregunta de la 9: LE FALTÓ -> BAJAR, SE PASÓ -> SUBIR
    verificar("BAJÁ rendimientoCostado" in consejo_pelota("FALTO_COSTADO", VALORES_DE_MENTIRA)
              and "0.80 → 0.75" in consejo_pelota("FALTO_COSTADO", VALORES_DE_MENTIRA), "le faltó: BAJAR")
    verificar("SUBÍ rendimientoCostado" in consejo_pelota("SE_PASO", VALORES_DE_MENTIRA)
              and "0.80 → 0.85" in consejo_pelota("SE_PASO", VALORES_DE_MENTIRA), "se pasó: SUBIR")
    verificar("mínimo" in consejo_pelota("FALTO_COSTADO", dict(VALORES_DE_MENTIRA, rendimientoCostado="0.4")), "en el mínimo")
    verificar("máximo" in consejo_pelota("SE_PASO", dict(VALORES_DE_MENTIRA, rendimientoCostado="1.2")), "en el máximo")
    verificar("umbralCm" in consejo_pelota("TARDE", VALORES_DE_MENTIRA) and "msAdelante" in consejo_pelota("CORTO", VALORES_DE_MENTIRA)
              and "Bien" in consejo_pelota("DESPEJO", VALORES_DE_MENTIRA), "las otras respuestas")
    # el panel: las 7 líneas byte por byte
    for _, linea in LINEAS_PANEL_0_1103:
        m = _RE_PANEL.match(linea)
        verificar(m is not None, "línea del panel leíble: %r" % linea[:40])
        verificar(linea_del_panel(m.group(1), m.group(2), m.group(3), m.group(4)) == linea,
                  "linea_del_panel reproduce %r" % linea[:40])
    otra = linea_del_panel("int", "umbralBlanco", "431", COMENTARIOS_PANEL["umbralBlanco"], "MEDIDO 11/11 Roboliga")
    verificar(otra.startswith("int umbralBlanco" + " " * 33 + "= 431;   // [MEDIDO 11/11 Roboliga] desde que"),
              "linea_del_panel con etiqueta: %r" % otra[:80])
    verificar("[MEDIDO 11/11 x] para X cm" in linea_del_panel("int", "msAdelante", "390", COMENTARIOS_PANEL["msAdelante"],
                                                              "MEDIDO 11/11 x"), "etiqueta: el primer [...]")
    verificar(linea_del_panel("int", "a", "1", "sin corchetes", "E").endswith("// [E] sin corchetes"), "etiqueta adelante")
    texto = "// PANEL DE CONTROL\n" + "\n".join(l for _, l in LINEAS_PANEL_0_1103) + "\n// fin del panel de control\nint x = 1; // afuera"
    panel = leer_panel(texto)
    verificar(set(panel) == set(COMENTARIOS_PANEL) and panel["CAMARA_POR_CM"][:2] == ("const float", "2.87")
              and "x" not in panel, "leer_panel sobre 11.3")
    if os.path.exists(PARTIDO_INO):
        with open(PARTIDO_INO, encoding="utf-8", errors="replace") as f:      # solo LEERLO
            real = f.read()
        panel_real = leer_panel(real)
        faltan = [n for n in ORDEN_NUMEROS if DECLARACION[n] and n not in panel_real]
        verificar(not faltan, "leer_panel del partido de verdad: faltan %r" % faltan)
        for n in ORDEN_NUMEROS:
            if DECLARACION[n]:
                verificar(panel_real[n][0] == DECLARACION[n], "la declaración de %s en el partido" % n)
        verificar(version_del_partido(real) is not None, "la versión del partido")
    verificar(copias_viejas(VALORES_DE_MENTIRA, panel) == [], "copias viejas: ninguna")
    viejas = copias_viejas(dict(VALORES_DE_MENTIRA, umbralBlanco="400", lateralInvertido="1"), panel)
    verificar(viejas == [("umbralBlanco", "400", "425"), ("lateralInvertido", "1", "false")], "copias viejas: %r" % viejas)
    # la hoja de cancha, ida y vuelta
    hoja = hoja_nueva("Roboliga 2026 - cancha 2", "2026-11-11 10:42")
    hoja_agregar(hoja, "LINEA", "verde", "izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1 n=3920", "10:43")
    hoja_agregar(hoja, "LINEA", "verde", "otra = con igual", "")
    hoja_poner(hoja, "LINEA", "umbral", "425", "peorVerde 97, peorBlanco 753, margen 328/328")
    hoja_poner(hoja, "COPIADO AL PANEL", "umbralBlanco", "425 -> 431", "11/11 10:55")
    hoja_poner(hoja, "NOTAS", "raro", "con   # adentro\ny un enter")
    texto = hoja_a_texto(hoja)
    verificar(texto_a_hoja(texto) == hoja, "hoja: ida y vuelta exacta")
    verificar(texto.startswith(CABECERA_HOJA) and "[COPIADO AL PANEL]\numbralBlanco = 425 -> 431   # 11/11 10:55" in texto,
              "hoja: el formato")
    verificar(hoja_valor(hoja, "LINEA", "verde_2") == "otra = con igual", "hoja: valor con =")
    verificar(hoja_valores_panel(hoja) == {"umbralBlanco": "431"}, "hoja: lo copiado manda")
    escuela = texto_a_hoja(HOJA_ESCUELA)
    verificar(hoja_valores_panel(escuela) == {"umbralBlanco": "425", "MS_MEDIO_AL_MEDIO": "2228",
                                              "MS_MEDIO_FONDO_POR_TIEMPO": "2000", "CAMARA_POR_CM": "2.87",
                                              "msAdelante": "383", "lateralInvertido": "0", "camaraYInvertida": "1"},
              "la hoja de la escuela: %r" % hoja_valores_panel(escuela))
    filas = comparar_hojas([("escuela", escuela), ("roboliga", hoja)])
    verificar(filas[0] == ("umbralBlanco", ["425", "431"]) and filas[1] == ("MS_MEDIO_AL_MEDIO", ["2228", "-"]), "comparar_hojas")
    # nuevo_valor, normalizar_entrada
    for actual, paso, signo, esperado in (("0.8", 0.05, -1, "0.75"), ("0.80", 0.05, 1, "0.85"), ("1.5", 0.25, 1, "1.75"),
                                          ("2.87", 0.01, 1, "2.88"), ("1.4", 0.2, 1, "1.6"), ("500.0", 50, 1, "550.0"),
                                          ("425", 5, -1, "420"), ("30.0", 1, 1, "31.0"), ("0.25", 0.25, -1, "0.00")):
        verificar(nuevo_valor(actual, paso, signo) == esperado, "nuevo_valor(%r, %r) = %r" % (actual, paso, nuevo_valor(actual, paso, signo)))
    for escrito, esperado in (("0,7", "0.7"), (" 12 ", "12"), ("abc", None), ("", None), ("9" * 400, None), ("1 2", None)):
        verificar(normalizar_entrada(escrito) == esperado, "normalizar_entrada(%r)" % escrito[:10])
    verificar(formatear_valor(0.8, "f") == "0.8" and formatear_valor(30, "f") == "30.0" and formatear_valor(0.75, "f") == "0.75"
              and formatear_valor(-0.001, "f") == "0.0" and formatear_valor(2.87, "f") == "2.87"
              and formatear_valor(424.6, "i") == "425", "formatear_valor como el robot")
    # puede_mover, cada motivo en orden
    ok = dict(conectado=True, es_arquero=True, programa_ok=True, modo="ESPERA")
    verificar(puede_mover(**ok) == (True, ""), "se puede mover")
    for cambio, palabra in ((dict(conectado=False), "No hay robot"), (dict(es_arquero=False), "NO es el arquero"),
                            (dict(programa_ok=False), "TABLERO.bat"), (dict(serie_robot="15708680"), "dice que NO"),
                            (dict(sin_cable=True), "SIN CABLE"), (dict(buscando_giro=True), "giroscopio"),
                            (dict(modo="MOVIENDO"), "Esperá"), (dict(modo="CUENTA"), "Esperá"),
                            (dict(pendiente=True), "conteste")):
        si, motivo = puede_mover(**dict(ok, **cambio))
        verificar(not si and palabra in motivo, "puede_mover: %r -> %r" % (cambio, motivo))
    verificar(puede_mover(**dict(ok, programa_ok=False, cartel="Tiene el programa de PARTIDO"))[1]
              == "Tiene el programa de PARTIDO", "puede_mover con el cartel de 2.6")
    verificar(puede_mover(**dict(ok, conectado=False, sin_cable=True))[1] == "No hay robot conectado.", "el orden")
    # los avisos sin bloquear
    verificar(avisos_de_numeros(VALORES_DE_MENTIRA) == [], "con los números del partido no hay avisos")
    avisos = avisos_de_numeros(dict(VALORES_DE_MENTIRA, distanciaParaMirar="30.0", MS_MAX_MIRANDO="150",
                                    msAdelante="300", pwmMaxLateral="170", despejeEnDiagonal="0"),
                               interpretar(S_BUENA.replace("l=88,92,61", "l=600,92,61")), 1)
    verificar(len(avisos) == 6, "los 6 avisos: %r" % avisos)
    # las notas
    verificar(contar_cruces([0, 3, -3, 3, 1, -1]) == 2 and contar_cruces([1, -1, 1.9]) == 0, "cruces")
    verificar(nota_derecho(3, 1, 2) == "PROLIJO" and nota_derecho(5, 1, 2) == "CASI" and nota_derecho(3, 4, 2) == "ZIGZAGUEA"
              and nota_derecho(12, 0, 1) == "SE_TUERCE" and nota_derecho(2, 0, 11) == "SE_TUERCE"
              and nota_derecho(10, 0, 1) == "CASI", "nota_derecho")
    verificar(calcular_nota("TECLA", 0, 0, 100, 100, 6, 0, 1, 0, 0) == "NO_VALE"
              and calcular_nota("NORMAL", 3, 3, 100, 100, 6, 0, 1, 0, 0) == "NO_VALE"
              and calcular_nota("NORMAL", 0, 5, 100, 100, 6, 0, 1, 0, 0) == "NO_VALE"
              and calcular_nota("NORMAL", 0, 0, 100, 4, 6, 0, 1, 0, 0) == "NO_VALE"
              and calcular_nota("NO_SALIO", 0, 0, 100, 100, 9, 0, 1, 0, 0) == "NO_SALIO"
              and calcular_nota("NORMAL", 0, 0, 100, 100, 9, 1, 1, 0, 0) == "PROLIJO", "calcular_nota")
    frases = explicar_informe(interpretar(R_6), [], VALORES_DE_MENTIRA, [(0, 0, "i"), (500, 6.2, "i"), (600, 1, "p")])
    todo = " ".join(f for f, _ in frases)
    verificar("6.2° a la IZQUIERDA a los 520 ms" in todo and "ida" in todo and "905 ms" in todo, "explicar el despeje: %r" % todo[:200])
    frases = explicar_informe(interpretar(R_9), [interpretar(MIRE_1), interpretar(MIRE_2)], VALORES_DE_MENTIRA)
    todo = " ".join(f for f, _ in frases)
    verificar("Intentos de despeje: 2" in todo and "SALIÓ (miró antes)" in todo, "explicar la 9")
    # frenado con una tecla en la ida: no dice "NO vio la línea", "se enderezó" ni "al final, quieto"
    cortado = R_6.replace("como=NORMAL", "como=TECLA").replace("lineams=905", "lineams=0").replace("acomodo=1", "acomodo=0")
    todo = " ".join(f for f, _ in explicar_informe(interpretar(cortado), [], VALORES_DE_MENTIRA))
    verificar("NO vio la línea" not in todo and "primer intento" not in todo and "Al final, quieto" not in todo
              and "Lo cortaron" in todo, "explicar: frenado con tecla %r" % todo[-160:])
    todo = " ".join(f for f, _ in explicar_informe(interpretar(R_9.replace("sale=1", "sale=0")), [], VALORES_DE_MENTIRA))
    verificar("desde que arrancó la ida" not in todo, "explicar la 9 sin salir: no dice 'desde que arrancó la ida'")
    # las tablas del contrato
    for tabla in (NUMEROS, VALORES_DE_MENTIRA, RANGOS_DE_MENTIRA):
        verificar(list(tabla) == list(ORDEN_NUMEROS) and len(tabla) == 46, "las 46 claves en orden")
    verificar(sum(1 for f in TABLA if f[6]) == 42, "42 con declaración en el panel")
    for m in MOVIMIENTOS:
        for nombre in m["numeros"]:
            verificar(nombre in NUMEROS, "falta %s en NUMEROS" % nombre)
    verificar([m["n"] for m in MOVIMIENTOS] == list(range(1, 11)), "los 10 movimientos")


def _autoprueba_robot_de_mentira(verificar):
    validos = {"M", "MIRE", "R", "USADOS", "CONSEJO", "FIN", "TEXTO"}
    for n in MOV_POR_NUMERO:
        for cortado in (None, 150):
            renglones = demo_resultado(n, VALORES_DE_MENTIRA, random.Random(n), cortado)
            tipos = [interpretar(r)["tipo"] for r in renglones]
            verificar("ROTA" not in tipos and set(tipos) <= validos, "mentira %d: fuera del contrato %r" % (n, renglones[-4:]))
            verificar(tipos.count("R") == 1 and tipos[-1] == "FIN" and tipos.count("USADOS") == 1, "mentira %d: tanda" % n)
            verificar(tipos.index("R") < tipos.index("USADOS") < tipos.index("FIN"), "mentira %d: el orden" % n)
            r = interpretar(next(x for x in renglones if x.startswith("#R ")))
            verificar(r["mov"] == MOV_POR_NUMERO[n]["nombre"] and r["n"] == n, "mentira %d: nombre" % n)
            verificar((r["nota"] == "NO_VALE") == (cortado is not None), "mentira %d: NO_VALE si la cortan (%s)" % (n, r["nota"]))
            if cortado is None:
                verificar(r["nota"] in ("PROLIJO", "CASI"), "mentira %d: con los del partido va bien (%s)" % (n, r["nota"]))
    # la 9: un intento que no sale y otro que sale
    renglones = demo_resultado(9, VALORES_DE_MENTIRA, random.Random(9))
    intentos = [interpretar(x) for x in renglones if x.startswith("#MIRE")]
    verificar([i["res"] for i in intentos] == ["NO_VIENE", "SALE_MIRANDO"] and intentos[1]["lat"] == -94, "mentira 9: intentos")
    r = interpretar(next(x for x in renglones if x.startswith("#R ")))
    verificar(r["sale"] == 1 and r["intentos"] == 2, "mentira 9: salió")
    verificar(any("BAJAR rendimientoCostado" in x for x in renglones), "mentira 9: el consejo de rendimientoCostado")
    abierto = demo_resultado(9, dict(VALORES_DE_MENTIRA, MAX_ANGULO_DIAGONAL="20.0"), random.Random(9))
    r = interpretar(next(x for x in abierto if x.startswith("#R ")))
    verificar(r["nota"] == "NO_SALIO" and r["como"] == "NO_SALIO" and any("MUY ABIERTO" in x for x in abierto),
              "mentira 9: muy abierto -> NO_SALIO")
    zig = demo_resultado(6, dict(VALORES_DE_MENTIRA, KP_RUMBO="9.5"), random.Random(6))
    verificar(interpretar(next(x for x in zig if x.startswith("#R ")))["nota"] == "ZIGZAGUEA"
              and any("bajar KP_RUMBO" in x for x in zig), "mentira: KP alto zigzaguea")
    verificar(any("6000" in x or "regla" in x for x in demo_resultado(10, VALORES_DE_MENTIRA, random.Random(1))), "mentira 10: regla")
    # las órdenes, sin hilos (la demo a mil veces más rápido)
    cola = queue.Queue()
    robot = RobotDeMentira(cola, escala_tiempo=0.001)
    robot.forzar_linea = False

    def dijo(orden):
        robot._t_freno = -1e9
        robot.comando(orden)
        salida = []
        while not cola.empty():
            msg = cola.get()
            salida.append(msg[1] if msg[0] == "linea" else msg)
        return salida

    salida = dijo("h")
    verificar(salida[0] == robot.linea_hola() and interpretar(salida[1])["tipo"] == "GUARDADO" and len(salida) == 3, "mentira: h")
    verificar(dijo("H")[0] == robot.linea_hola(), "mentira: H")
    salida = dijo("?")
    verificar(interpretar(salida[0])["valores"] == VALORES_DE_MENTIRA and interpretar(salida[1])["tipo"] == "DEF", "mentira: ?")
    salida = dijo("=umbralCm 32")
    verificar(salida[0] == "#OK umbralCm=32.0" and interpretar(salida[1])["valores"]["umbralCm"] == "32.0", "mentira: =")
    verificar(dijo("=rendimientocostado=0.75")[0] == "#OK rendimientoCostado=0.75", "mentira: = sin mayúsculas y con =")
    verificar(dijo("=MS_MIRAR\t140")[0] == "#OK MS_MIRAR=140", "mentira: = con tab")
    for orden, error in (("=", "#ERROR falta el nombre. Se escribe: =NOMBRE valor   (ej: =umbralCm 32)"),
                         ("=noExiste 3", "#ERROR no existe el numero noExiste. Con ? salen todos los nombres"),
                         ("=umbralCm", "#ERROR falta el valor de umbralCm. Se escribe: =NOMBRE valor"),
                         ("=umbralCm 3,5", "#ERROR umbralCm: usar punto decimal (4.5, no 4,5)"),
                         ("=umbralCm nan", "#ERROR umbralCm: no es un numero: nan"),
                         ("=msAdelante 3.5", "#ERROR msAdelante va sin decimales"),
                         ("=despejeEnDiagonal 2", "#ERROR despejeEnDiagonal es si o no: va 0 o 1"),
                         ("=umbralCm 50", "#ERROR umbralCm fuera de rango: va de 20.0 a 45.0"),
                         ("!", "#ERROR falta el numero del movimiento: !1 a !10, o !L !A !F !C !I"),
                         ("!11", "#ERROR no hay movimiento 11: son del !1 al !10, y los de cancha !L !A !F !C !I"),
                         ("!L", "#ERROR !L necesita la fase: !L v (verde), !L i (blanco izq), !L d (blanco der) o !L c (sin cable)"),
                         ("zz", "#ERROR no entiendo 'zz'. ? = ayuda"), ("x" * 80, "#ERROR orden demasiado larga"),
                         ("rc", "#ERROR no hay nada guardado"), ("r", "#ERROR no hay nada guardado")):
        salida = dijo(orden)
        verificar(salida[:1] == [error], "mentira: %r -> %r" % (orden[:20], salida[:1]))
    salida = dijo("v")
    verificar("#PANEL float umbralCm = 32.0;" in salida and "#PANEL float rendimientoCostado = 0.75;" in salida
              and "#PANEL const unsigned long MS_MIRAR = 140;" in salida and salida[-1] == "#FINPANEL", "mentira: v %r" % salida)
    dijo("=despejeEnDiagonal 0")
    verificar("#PANEL bool despejeEnDiagonal = false;" in dijo("v"), "mentira: v con un bool")
    salida = dijo("0")
    verificar(interpretar(salida[-1])["valores"] == VALORES_DE_MENTIRA, "mentira: 0")
    salida = dijo("v")
    verificar(salida[-1] == "#FINPANEL" and "(nada para copiar: estan todos como en el partido)" in salida, "mentira: v sin cambios")
    verificar(dijo("g") == ["#GIRO buscando", "#GIRO ok=1 estado=5"], "mentira: g")
    verificar(dijo("x") == ["PARADO (frena 0,4 s y suelta las ruedas)"], "mentira: x")
    verificar(interpretar(robot.linea_S())["tipo"] == "S", "mentira: #S")
    # un movimiento entero (secuencia a)
    salida = dijo("!6")
    tipos = [interpretar(x)["tipo"] for x in salida]
    verificar(tipos[:7] == ["TEXTO", "CUENTA", "CUENTA", "CUENTA", "CUENTA", "CUENTA", "CUENTA"] and tipos[7] == "INICIO"
              and tipos[-1] == "FIN" and "R" in tipos and "ROTA" not in tipos, "mentira: !6 %r" % tipos[:9])
    verificar(interpretar(salida[1])["segundos"] == 3.0 and interpretar(salida[6])["segundos"] == 0.5, "mentira: cuenta 3 s")
    # frenada en la cuenta (b)
    robot.enviar("x")
    salida = dijo("!1")
    verificar(salida[-2:] == ["#FRENO tecla", "cancelado: no se movio"] and "#INICIO" not in " ".join(salida), "mentira: (b)")
    # recién parado, sobre la línea (d)
    robot._t_freno = time.monotonic() + 10
    robot.comando("!2")
    salida = [cola.get()[1] for _ in range(cola.qsize())]
    verificar(salida[0] == "#FRENO recien parado", "mentira: recién parado %r" % salida)
    robot.forzar_linea = True
    verificar(dijo("!8")[0] == "#FRENO sobre la linea" and dijo("!A")[0] == "#FRENO sobre la linea", "mentira: sobre la línea")
    verificar(interpretar(dijo("!6")[1])["tipo"] == "CUENTA", "mentira: el 6 no mira la línea")
    robot.forzar_linea = False
    # cada medición de cancha con los números de las bitácoras
    salida = dijo("!L v")
    verificar(salida[0] == "#INICIO L LINEA_VERDE tope=5000 sincable=0" and salida[1] == LINEAS_DE_MENTIRA["v"]
              and salida[-1] == "#FIN L", "mentira: !L v %r" % salida)
    lineas = [interpretar(x) for x in dijo("!L i") + dijo("!L d") + dijo("!L v") if x.startswith("#LIN")]
    verificar(calcular_umbral(lineas)["umbral"] == 425, "mentira: la línea da 425")
    a1 = [interpretar(x) for x in dijo("!A") if x.startswith(("#ANCHO", "#PASO"))]
    a2 = [interpretar(x) for x in dijo("!A") if x.startswith("#ANCHO")]
    verificar(a1[-1]["almedioizq"] == 2213 and a1[-1]["almedioder"] == 2314 and a2[0]["almedioizq"] == 2243
              and a2[0]["almedioder"] == 2348 and sugerido_ancho([a1[-1], a2[0]])[0] == 2228, "mentira: el ancho 06/10")
    verificar(sum(1 for x in a1 if x["tipo"] == "PASO") == 6, "mentira: los #PASO del ancho")
    f1 = [interpretar(x) for x in dijo("!F") if x.startswith("#FONDO")][0]
    f2 = [interpretar(x) for x in dijo("!F") if x.startswith("#FONDO")][0]
    verificar(f1["ms"] == 1730 and f1["como"] == "TOCO" and sugerido_fondo([f1, f2]) == 2250, "mentira: el fondo")
    cam = [interpretar(x) for x in dijo("!C") if x.startswith("#CAM")][0]
    verificar(cam["xp"][2] == 86.9 and cam["val"] == 76, "mentira: la cámara a 30 cm")
    medio = [interpretar(x) for x in dijo("!I") if x.startswith(("#MEDIO", "#PASO"))]
    verificar(medio[-1]["ok"] and medio[-1]["pasos"][5] == 2228 and len(medio) == 9, "mentira: ir al medio")
    verificar(dijo("rc")[0] == "#REPITE I IR_AL_MEDIO sincable=0 apagado=0", "mentira: rc repite la última de cancha")
    # SIN CABLE: desconectado -> conectado -> GUARDADO nuevo=1 -> r -> REPITE -> FIN
    salida = dijo("!6 c")
    textos = [x for x in salida if isinstance(x, str)]
    eventos = [x[0] for x in salida if isinstance(x, tuple)]
    verificar(eventos == ["desconectado", "conectado"], "mentira sin cable: se desenchufa y vuelve %r" % eventos)
    verificar(interpretar(textos[1])["tipo"] == "CUENTA" and interpretar(textos[1])["modo"] == "sincable"
              and "#INICIO" not in " ".join(textos), "mentira sin cable: cuenta y NO dice nada moviéndose")
    hola = textos.index(robot.linea_hola())
    g = interpretar(textos[hola + 1])
    verificar(g["que"] == "mov" and g["nuevo"] and g["n"] == "6" and g["sincable"], "mentira sin cable: GUARDADO nuevo=1")
    salida = dijo("r")
    verificar(salida[0] == "#REPITE 6 DESPEJE sincable=1 apagado=0" and salida[-1] == "#FIN 6"
              and any(x.startswith("#R ") for x in salida), "mentira sin cable: r")
    verificar(interpretar(dijo("h")[1])["nuevo"] is False, "mentira sin cable: después de r, nuevo=0")
    salida = dijo("!L c")
    verificar(sum(1 for x in salida if isinstance(x, str) and x.startswith("#LIN")) == 0
              and interpretar(dijo("rc")[0])["nombre"] == "LINEA_GUIADA", "mentira: la línea guiada sin cable")
    # (c) tecla MOVIÉNDOSE, con el robot en un hilo
    cola2 = queue.Queue()
    robot2 = RobotDeMentira(cola2, escala_tiempo=0.05)
    robot2.forzar_linea = False
    hilo = threading.Thread(target=robot2.comando, args=("!6",), daemon=True)
    hilo.start()
    vistos, fin = [], time.monotonic() + 20
    while time.monotonic() < fin:
        try:
            msg = cola2.get(timeout=0.002)
        except queue.Empty:
            continue
        vistos.append(msg[1])
        if msg[1].startswith("#INICIO"):
            time.sleep(0.02)
            robot2.enviar("x")
        if msg[1].startswith("#FIN"):
            break
    hilo.join(5)
    tipos = [interpretar(x)["tipo"] for x in vistos]
    k = tipos.index("INICIO")
    verificar(tipos[k + 1] == "FRENO" and interpretar(vistos[k + 1])["motivo"] == "tecla", "mentira (c): FRENO tecla")
    r = interpretar(next(x for x in vistos if x.startswith("#R ")))
    verificar(r["nota"] == "NO_VALE" and r["como"] == "TECLA"
              and any(x == "#CONSEJO Lo frenaron antes de terminar: este resultado no vale." for x in vistos)
              and tipos[-1] == "FIN", "mentira (c): la tanda NO_VALE")


def _autoprueba_cable(verificar):
    """El hilo del USB con un puerto DE MENTIRA (no abre ningún USB): elige
    al arquero aunque esté el delantero, corta los renglones, se da cuenta
    cuando se desenchufa y avisa si el puerto está ocupado."""

    class Puerto:
        def __init__(self, device, serie, vid=VID_TEENSY):
            self.device, self.vid, self.pid, self.serial_number = device, vid, PID_SERIE, serie

    class SerieDeMentira:
        aperturas = []

        def __init__(self, puerto, baudios, timeout=None, write_timeout=None):
            SerieDeMentira.aperturas.append(puerto)
            if len(SerieDeMentira.aperturas) > 1:
                raise OSError("Acceso denegado")          # la segunda vez: "ocupado"
            self.por_leer = (b"basura del medio\n" + HOLA_BUENO.encode() + b"\r\n" + S_BUENA.encode()
                             + b"\n#M 10 -0.4 l\n#M 20 -0")   # el último quedó cortado
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

    def puertos(*lista):
        class ModuloPuertos:
            @staticmethod
            def comports():
                return list(lista)
        return ModuloPuertos

    ARQ, DEL, OTRA = Puerto("COM11", SERIE_ARQUERO), Puerto("COM9", SERIE_DELANTERO), Puerto("COM3", "16000000")
    g = globals()
    guardados = {k: g[k] for k in ("serial", "list_ports")}
    g["serial"], g["list_ports"] = ModuloSerie, puertos(DEL, ARQ, OTRA)
    cola = queue.Queue()
    cable = Conexion(cola)
    try:
        cable.enviar("!1")
        verificar(cable._salida.empty(), "cable: sin conexión no guarda órdenes")
        cable.arrancar()
        recibido = []
        fin = time.monotonic() + 6
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
    verificar(recibido[0] == ("conectado", "COM11", SERIE_ARQUERO), "cable: elige al arquero %r" % (recibido[:1],))
    verificar(set(SerieDeMentira.aperturas) == {"COM11"}, "cable: NUNCA abre el delantero ni la otra %r" % SerieDeMentira.aperturas)
    renglones = [m[1] for m in recibido if m[0] == "linea"]
    verificar(renglones == ["basura del medio", HOLA_BUENO, S_BUENA, "#M 10 -0.4 l"], "cable: renglones %r" % renglones)
    verificar(SerieDeMentira.ultima.escrito == b"h\n", "cable: escribió %r" % SerieDeMentira.ultima.escrito)
    tipos = [m[0] for m in recibido]
    verificar("desconectado" in tipos and tipos[-1] == "ocupado", "cable: desenchufe y ocupado %r" % tipos)
    cable._salida.put("!1")
    cable._vaciar_salida()
    verificar(cable._salida.empty(), "cable: lo pendiente se tira al desenchufar")
    # Solo el delantero, o solo una desconocida: avisa UNA vez y nunca la abre
    for solo in (DEL, OTRA):
        SerieDeMentira.aperturas = []
        g["serial"], g["list_ports"] = ModuloSerie, puertos(solo)
        cola = queue.Queue()
        cable = Conexion(cola)
        try:
            verificar(cable._elegir_puerto() == (None, (solo.device, solo.serial_number)), "cable: %s no se elige" % solo.device)
            cable.arrancar()
            time.sleep(1.3)                      # que busque un par de veces
        finally:
            cable.cerrar()
            g.update(guardados)
        recibido = []
        while not cola.empty():
            recibido.append(cola.get())
        verificar(recibido == [("otro", solo.device, solo.serial_number)], "cable: avisa una sola vez %r" % recibido)
        verificar(SerieDeMentira.aperturas == [] and not cable.enviar("!1"), "cable: al %s no le abre ni le escribe" % solo.device)
    # --puerto: solo si ESE es el arquero
    g["list_ports"] = puertos(DEL, ARQ)
    try:
        verificar(Conexion(queue.Queue(), "com9")._elegir_puerto() == (None, ("COM9", SERIE_DELANTERO)),
                  "cable: --puerto del delantero se niega")
        verificar(Conexion(queue.Queue(), "com11")._elegir_puerto() == (("COM11", SERIE_ARQUERO), None),
                  "cable: --puerto del arquero sí")
        verificar(Conexion(queue.Queue(), "COM99")._elegir_puerto() == (None, None), "cable: --puerto que no está")
    finally:
        g.update(guardados)
    # --quien: cada caso (salida y código) sin abrir nada
    for lista, ultima, codigo in (([ARQ], "QUIEN=ARQUERO", 0), ([DEL], "QUIEN=DELANTERO", 3),
                                  ([ARQ, DEL], "QUIEN=DELANTERO", 3), ([OTRA], "QUIEN=OTRO", 4),
                                  ([Puerto("COM5", None)], "QUIEN=OTRO", 4), ([], "QUIEN=NINGUNO", 5),
                                  ([Puerto("COM1", None, vid=0x1234)], "QUIEN=NINGUNO", 5),
                                  ([ARQ, OTRA], "QUIEN=VARIOS", 6), ([ARQ, Puerto("COM12", SERIE_ARQUERO)], "QUIEN=VARIOS", 6)):
        renglones, c = quien(lista)
        verificar(renglones[-1] == ultima and c == codigo and AVISO_QUIEN in renglones, "quien %r" % ultima)
        verificar(all(r.encode("ascii") for r in renglones), "quien: ASCII")
    verificar(quien([ARQ])[0][0] == "Teensy serie 13348550 en COM11  ->  ARQUERO", "quien: el renglón del arquero")
    import io
    SerieDeMentira.aperturas = []
    g["serial"], g["list_ports"] = ModuloSerie, puertos(ARQ)
    salida = io.StringIO()
    try:
        c = main_quien(salida, mirar_carga=lambda: [])
    finally:
        g.update(guardados)
    verificar(c == 0 and salida.getvalue().strip().endswith("QUIEN=ARQUERO") and SerieDeMentira.aperturas == [],
              "main_quien sin abrir nada")
    # un Teensy en MODO CARGA (sin puerto COM): teensy-cli le cargaría a ESE
    EN_CARGA = [r"USB\VID_16C0&PID_0478\5&1234&0&1"]
    g["serial"], g["list_ports"] = ModuloSerie, puertos(ARQ)
    salida = io.StringIO()
    try:
        c = main_quien(salida, mirar_carga=lambda: EN_CARGA)
    finally:
        g.update(guardados)
    verificar(c == 8 and salida.getvalue().strip().endswith("QUIEN=CARGA") and "MODO CARGA" in salida.getvalue()
              and SerieDeMentira.aperturas == [], "main_quien: un Teensy en modo carga NO es el arquero")
    for lista, carga, ultima, codigo in (([ARQ], EN_CARGA, "QUIEN=CARGA", 8), ([], EN_CARGA, "QUIEN=CARGA", 8),
                                         ([DEL], EN_CARGA, "QUIEN=DELANTERO", 3), ([ARQ], [], "QUIEN=ARQUERO", 0),
                                         ([ARQ], None, "QUIEN=ARQUERO", 0)):
        renglones, c = quien(lista, carga)
        verificar(renglones[-1] == ultima and c == codigo and all(r.encode("ascii") for r in renglones),
                  "quien con modo carga %r %r" % (carga, ultima))
    verificar(AVISO_SIN_PNPUTIL in quien([ARQ], None)[0] and AVISO_SIN_PNPUTIL not in quien([ARQ], [])[0],
              "quien: avisa si no pudo mirar el modo carga")
    # lo que lista pnputil: un Teensy en modo carga sale como USB\... y su HID\... (es UNO solo)
    listado = ("Id. de instancia:   USB\\VID_16C0&PID_0478\\5&ABC&0&2\r\nDescripción: USB Input Device\r\n"
               "Id. de instancia:   HID\\VID_16C0&PID_0478\\6&1&0&0000\r\n"
               "Id. de instancia:   USB\\VID_16C0&PID_0483\\13348550\r\n")
    verificar(ids_en_modo_carga(listado) == ["USB\\VID_16C0&PID_0478\\5&ABC&0&2"], "modo carga: USB y HID son UNO")
    verificar(ids_en_modo_carga("HID\\vid_16c0&pid_0478\\6&1&0&0000\r\n") == ["HID\\vid_16c0&pid_0478\\6&1&0&0000"],
              "modo carga: solo el HID también cuenta")
    verificar(ids_en_modo_carga(listado.split("Id. de instancia:   USB\\VID_16C0&PID_0483")[1]) == [],
              "modo carga: un Teensy con programa (PID 0483) NO es modo carga")
    g["list_ports"] = None
    try:
        salida = io.StringIO()
        verificar(main_quien(salida) == 7 and "QUIEN=NOSE" in salida.getvalue(), "main_quien sin pyserial")
    finally:
        g.update(guardados)


def _autoprueba_ventana(verificar, carpeta):
    """La ventana entera con un tkinter DE MENTIRA (unittest.mock): no se
    abre nada en la pantalla. Sirve para encontrar errores de tipeo en la
    parte de la ventana, que si no solo aparecerían con el robot enchufado."""
    from unittest import mock
    g = globals()
    guardados = {k: g[k] for k in ("tk", "ttk", "messagebox", "filedialog")}
    lanzados = []
    try:
        g["tk"] = mock.MagicMock(name="tk")
        g["ttk"] = mock.MagicMock(name="ttk")
        g["messagebox"] = mock.MagicMock(name="messagebox")
        g["filedialog"] = mock.MagicMock(name="filedialog")
        g["messagebox"].askyesno.return_value = True
        raiz = mock.MagicMock(name="raiz")
        cable = _CableDePrueba()
        canchas = os.path.join(carpeta, "canchas")
        en_carga = []
        v = Ventana(raiz, cable, queue.Queue(), estricto=True, carpeta_canchas=canchas,
                    lanzador=lambda *a, **k: lanzados.append((a, k, list(cable.mandados), cable.cerrado)),
                    mirar_carga=lambda: en_carga)

        def llegan(*renglones):
            v._procesar_lote([("linea", r) for r in renglones])

        etiquetas = g["tk"].Label.return_value      # (todos los Label de mentira son el mismo)

        def mostro(texto, ultimas=80):
            return any(texto in str(c) for c in etiquetas.configure.call_args_list[-ultimas:])

        v._procesar_lote([("buscando",)])
        v._procesar_lote([("ocupado", "COM11", "Acceso denegado")])
        v._pintar_cuenta()
        verificar(mostro("ocupado"), "ventana: dice por qué PROBAR está apagado (ocupado)")
        v._parar()
        verificar(cable.mandados == [], "ventana: sin conexión no manda nada")
        # el delantero (su puerto ni se abrió): cartel rojo y nada mandado
        v._procesar_lote([("otro", "COM9", SERIE_DELANTERO)])
        v._pintar_conexion()
        verificar(mostro("DELANTERO"), "ventana: ESE ES EL DELANTERO")
        v._probar()
        v._parar()
        verificar(cable.mandados == [] and v.ultimo_aviso[1] == ROJO, "ventana: al delantero nada")
        v._procesar_lote([("otro", "COM3", "16000000")])
        v._pintar_conexion()
        verificar(mostro("NO ES EL ARQUERO"), "ventana: otra Teensy")
        # AL CONECTAR NO MANDA NADA
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        verificar(cable.mandados == [], "ventana: al conectar no manda nada")
        llegan("algo para personas", "#ZZZ", S_DELANTERO)
        verificar(cable.mandados == [], "ventana: sin un #S bueno sigue sin mandar nada")
        v._probar()
        verificar(cable.mandados == [], "ventana: sin #HOLA no mueve")
        v.t_conexion -= 10
        v._latido()
        verificar(v.aviso_programa and "No sé qué programa" in v._texto_programa()[0], "ventana: no sé qué programa")
        v._parar()
        # sin saber qué programa es, PARAR manda un ESPACIO: si era ajustar, una x le pisaba los números
        verificar(cable.mandados == [" "] and NO_OBEDECE_PARAR in v.ultimo_aviso[0], "ventana: PARAR sin tablero avisa")
        for escrito in ("!1", "=umbralCm 32", "x", "0", "v", "r"):
            v.ent_consola.get.return_value = escrito
            v._consola_mandar()
        verificar(cable.mandados == [" "], "ventana: la consola sin tablero no manda órdenes %r" % cable.mandados)
        v.ent_consola.get.return_value = "h"
        v._consola_mandar()
        verificar(cable.mandados == [" ", "h"], "ventana: la consola sin tablero deja h")
        del cable.mandados[:]
        # el partido detectado por su texto: NO manda h
        llegan("=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===")
        llegan(S_BUENA)
        verificar(cable.mandados == [] and "PARTIDO" in v._texto_programa()[0] and not v._puede()[0],
                  "ventana: el partido por su texto, sin mandar h")
        v._procesar_lote([("desconectado", "COM11")])
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan("=== AJUSTAR-MOVIMIENTOS 2026-10-27 — el robot SE MUEVE solo al apretar 1..9 ===", S_BUENA)
        v._parar()
        verificar(cable.mandados == ["0"] and "AJUSTAR" in v._texto_programa()[0],
                  "ventana: en ajustar PARAR manda 0 (la x pisa sus números) %r" % cable.mandados)
        del cable.mandados[:]
        v._procesar_lote([("desconectado", "COM11")])
        # el tablero del DELANTERO cargado en el arquero: bloqueado, en rojo
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan("#HOLA tablero-delantero 2026-10-27", S_BUENA)
        verificar(cable.mandados == [] and "DELANTERO" in v._texto_programa()[0] and v._texto_programa()[1] == ROJO
                  and not v._puede()[0], "ventana: el tablero del delantero bloqueado")
        v._parar()
        verificar(cable.mandados == ["x"], "ventana: PARAR igual le manda x al tablero del delantero")
        del cable.mandados[:]
        v._procesar_lote([("desconectado", "COM11")])
        # el primer #S manda h; el #HOLA, ?
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan(S_BUENA)
        verificar(cable.mandados == ["h"], "ventana: con el primer #S manda h %r" % cable.mandados)
        llegan(S_BUENA, S_BUENA)
        verificar(cable.mandados == ["h"], "ventana: no pide h a cada rato")
        v.t_pide_hola -= 2.5
        llegan(S_BUENA)
        verificar(cable.mandados == ["h", "h"], "ventana: sin #HOLA vuelve a pedir h a los 2 s")
        llegan(HOLA_BUENO, "#GUARDADO que=mov hay=0", "#GUARDADO que=cancha hay=0")
        verificar(cable.mandados == ["h", "h", "?"] and v._programa_ok(), "ventana: con el #HOLA manda ? %r" % cable.mandados)
        llegan(V_BUENA, DEF_BUENA, S_BUENA, "#GIRO ok=1 estado=5")
        v.t_pide_hola -= 10
        llegan(S_BUENA)
        verificar(cable.mandados == ["h", "h", "?"], "ventana: con #HOLA no pregunta más")
        v._pintar_cuenta()
        verificar(mostro("Listo") and mostro("3 s"), "ventana: Listo")
        verificar("Programa del tablero cargado" in v._texto_programa()[0], "ventana: programa OK")
        # PROBAR, doble clic, la cuenta, PARAR
        v._probar()
        verificar(cable.mandados[-1] == "!1", "ventana: PROBAR manda !1")
        v._probar()
        verificar(cable.mandados.count("!1") == 1, "ventana: doble clic no lo manda dos veces")
        llegan(">>> 1 COSTADO_IZQ en 3 s. Saca la mano. Cualquier tecla lo frena.", "#CUENTA 3.0 cable", "#CUENTA 2.5 cable")
        v._probar()
        v._paso("velLenta", +1)
        v.ent_consola.get.return_value = "?"
        v._consola_mandar()
        verificar(cable.mandados.count("!1") == 1 and cable.mandados[-1] == "!1", "ventana: en la cuenta nada salvo PARAR")
        v._parar()
        verificar(cable.mandados[-1] == "x", "ventana: PARAR en la cuenta manda x")
        llegan("#FRENO tecla", "cancelado: no se movio")
        verificar(v.estado.modo == "ESPERA" and v.estado.freno == "tecla", "ventana: frenado en la cuenta")
        # al PROBAR sin giroscopio, el robot lo busca ANTES de contar: no es "no arrancó"
        v._probar()
        llegan("Sin giroscopio. Lo busco de nuevo (tarda hasta 6 s)...", "#GIRO buscando")
        verificar(v.t_pedido is not None and v.t_pedido > time.monotonic() and "ARRANCA" in v.ultimo_aviso[0],
                  "ventana: buscando el giroscopio al PROBAR avisa que puede arrancar")
        v.t_pedido -= ESPERA_GIRO                  # (pasaron ~7 s: todavía dentro de los 3 s de siempre)
        v._latido()
        verificar(v.t_pedido is not None and "no arrancó" not in v.ultimo_aviso[0],
                  "ventana: buscando el giroscopio no dice 'no arrancó'")
        llegan("#GIRO ok=0 estado=-1", "#FRENO sin giroscopio")
        verificar(v.t_pedido is None and v.estado.modo == "ESPERA", "ventana: no encontró el giroscopio")
        llegan("#GIRO ok=1 estado=5")
        # + y -, la cajita, los interruptores
        v._elegir(9)
        v._paso("rendimientoCostado", -1)
        verificar(cable.mandados[-1] == "=rendimientoCostado 0.75", "ventana: [-] %r" % cable.mandados[-1])
        v._paso("umbralCm", +1)
        verificar(cable.mandados[-1] == "=umbralCm 31.0", "ventana: [+] %r" % cable.mandados[-1])
        llegan("#OK umbralCm=31.0", "#ERROR umbralCm fuera de rango: va de 20.0 a 45.0")
        v.estado.valores["umbralCm"] = "45.0"
        antes = len(cable.mandados)
        v._paso("umbralCm", +1)
        verificar(len(cable.mandados) == antes, "ventana: [+] en la punta no manda")
        v.filas["umbralCm"]["entrada"].get.return_value = "32,5"
        v._poner("umbralCm")
        verificar(cable.mandados[-1] == "=umbralCm 32.5", "ventana: la cajita con coma")
        v.filas["umbralCm"]["entrada"].get.return_value = "treinta"
        antes = len(cable.mandados)
        v._poner("umbralCm")
        verificar(len(cable.mandados) == antes, "ventana: la cajita con letras no manda")
        v._mandar_valor("despejeEnDiagonal", "0")
        verificar(cable.mandados[-1] == "=despejeEnDiagonal 0", "ventana: interruptor")
        llegan(V_BUENA)
        # los 10 movimientos con gráfico
        for n in MOV_POR_NUMERO:
            v._elegir(n)
            v._probar()
            verificar(cable.mandados[-1] == "!%d" % n, "ventana: PROBAR %d" % n)
            llegan("#CUENTA 3.0 cable", "#INICIO %d %s tope=%d sincable=0" % (n, MOV_POR_NUMERO[n]["nombre"],
                                                                             tope_de(n, VALORES_DE_MENTIRA)))
            verificar(v.estado.modo == "MOVIENDO", "ventana: moviendo %d" % n)
            v._pintar_cuenta()
            verificar(mostro("MOVIENDO"), "ventana: dice que se mueve")
            llegan(*demo_resultado(n, VALORES_DE_MENTIRA, random.Random(n)))
            verificar(v.estado.modo == "ESPERA" and v.estado.completo and v.estado.informe["n"] == n, "ventana: terminó %d" % n)
            v._dibujar_grafico()
            v._pintar_resultado()
            verificar(v.ultimo_mov is not None and v.ultimo_mov.informe["n"] == n, "ventana: guarda el %d" % n)
        # la 9: la tabla de intentos y las 5 respuestas
        v._elegir(9)
        llegan("#CUENTA 3.0 cable", "#INICIO 9 ATAJAR tope=30000 sincable=0", "#M 0 0.0 s", "#M 3880 0.2 i",
               "#M 4500 -1.4 a", MIRE_1, MIRE_2, R_9, "#USADOS rendimientoCostado=0.75", "#FIN 9")
        v._pintar_resultado()
        verificar(len(v.estado.intentos) == 2, "ventana: la 9 con sus intentos")
        textos = " ".join(str(c) for c in v.txt_intentos.insert.call_args_list)
        verificar("SALIÓ (miró antes)" in textos and "-25.0" in textos and "-94" in textos, "ventana: la tabla de intentos")
        for clave, _ in RESPUESTAS_PELOTA:
            v._pregunta_pelota(clave)
            verificar(v.respuesta_pelota[1] == consejo_pelota(clave, v.estado.valores), "ventana: respuesta %s" % clave)
        verificar("BAJÁ" in consejo_pelota("FALTO_COSTADO", v.estado.valores), "ventana: le faltó -> bajar")
        # Copiar al panel, volver a los números, pedir el último
        v._copiar_panel()
        verificar(cable.mandados[-1] == "v", "ventana: Copiar al panel manda v")
        llegan("--- PARA COPIAR ---", "#PANEL float rendimientoCostado = 0.75;", "#FINPANEL")
        verificar(raiz.clipboard_append.called, "ventana: copió al portapapeles")
        v._volver_numeros()
        mand = cable.mandados
        despues = mand[len(mand) - mand[::-1].index("0"):]
        verificar(despues == ["=%s %s" % c for c in v._numeros_de_la_0921()] and "=frenarAntesDeVolver 0" in despues
                  and all(x.split()[0][1:] in v.lectura["valores"] or x.split()[0][1:] in COMO_LA_0921
                          for x in despues),
                  "ventana: volver a los números manda 0 y después los de la 0921 %r" % (despues,))
        # (revisión 08/11) el lazo: si en PARTIDO cambian LADO_TRASERA, el botón le pone ESE
        viejo = v.lectura["valores"]["LADO_TRASERA"]
        v.lectura["valores"]["LADO_TRASERA"] = "91"         # (solo en la memoria: el archivo no se toca)
        verificar(("LADO_TRASERA", "91") in v._numeros_de_la_0921(), "ventana: el botón pone el LADO_TRASERA de la 0921")
        llegan("#OK LADO_TRASERA=91")
        verificar(not any("LADO_TRASERA" in a[0] for a in v._sugeridos()["MS_MEDIO_AL_MEDIO"][1]),
                  "ventana: con el LADO_TRASERA de la 0921 en el robot, CANCHA no avisa")
        llegan("#OK LADO_TRASERA=%s" % viejo)
        v.lectura["valores"]["LADO_TRASERA"] = viejo
        llegan("#OK RAMPA_MOV_PASO=8")
        a = v._aviso_numeros(NUMEROS_DEL_MOVIMIENTO)
        verificar(a is not None and BOTON_NUMEROS_PARTIDO in a[0] and "al panel" not in a[0],
                  "ventana: el aviso de CANCHA manda al botón nuevo (%r)" % (a,))
        llegan("#OK RAMPA_MOV_PASO=%s" % v._valor_de_hoy("RAMPA_MOV_PASO")[0])
        v._pedir_ultimo()
        verificar(cable.mandados[-1] == "r", "ventana: pedir el último manda r")
        llegan("#ERROR no hay nada guardado")
        verificar(v.t_pedido_repetir is None, "ventana: el #ERROR suelta el pedido")
        # los dibujos con y sin datos
        for s in (interpretar(S_BUENA), interpretar(S_SIN_GIRO), None,
                  interpretar(S_BUENA.replace("p=86,-12,40", "p=86,-12,2000").replace("ve=1", "ve=0")),
                  interpretar(S_BUENA.replace("dg=1,0,193,33,396", "dg=0,2,142,0,383").replace("l=88,92,61", "l=600,700,800")),
                  interpretar(S_BUENA.replace("d=30.0", "d=36.0").replace("vy=0.0", "vy=-12.0").replace("g=5", "g=3"))):
            v.estado.sensores = s
            v._dibujar_pelota()
            v._textos_pelota()
            v._dibujar_lineas()
            v._dibujar_brujula()
            v._textos_sensores()
        llegan(S_BUENA)
        v.ultimo_mov, guardado = None, v.ultimo_mov
        v.estado.codigo = "L"
        v._pintar_resultado()
        v._dibujar_grafico()
        v.ultimo_mov = guardado
        v.estado.codigo = "9"
        # el latido: si el robot no contesta, vuelve sola
        llegan("#CUENTA 3.0 cable", "#INICIO 2 COSTADO_DER tope=4000 sincable=0")
        v.t_ultima_linea -= 30
        v._latido()
        verificar(v.estado.modo == "ESPERA" and v.ultimo_aviso[1] == ROJO, "ventana: el latido la vuelve a ESPERA")
        v._refrescar({"conexion", "hola", "valores", "sensores", "modo", "resultado", "freno", "ok", "error",
                      "panel_listo", "rota", "giro", "cancha", "cancha_lista"})
        # desconectar y reconectar en medio de un movimiento
        v._probar()
        llegan("#CUENTA 3.0 cable", "#INICIO 6 DESPEJE tope=20000 sincable=0")
        v._procesar_lote([("desconectado", "COM11")])
        verificar(v.ultimo_aviso[1] == ROJO and "SIGUE SOLO" in v.ultimo_aviso[0] and v._cortado_andando(),
                  "ventana: se cortó el cable andando: ROJO, sigue solo %r" % (v.ultimo_aviso[0],))
        v._pintar_cuenta()
        verificar(mostro("SIGUE SOLO"), "ventana: se cortó andando: la cuenta lo dice en rojo")
        antes = len(cable.mandados)
        v._probar()
        verificar(len(cable.mandados) == antes and v.ultimo_aviso[1] == NARANJA, "ventana: desenchufado no manda")
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        verificar(v._cortado_andando() and "Volvió el cable" in v._puede()[1], "ventana: volvió el cable, puede seguir andando")
        llegan(*demo_resultado(6, VALORES_DE_MENTIRA, random.Random(6)))
        verificar(len(cable.mandados) == antes and v.estado.informe["mov"] == "DESPEJE", "ventana: reconectado, no manda nada")
        verificar(not v._cortado_andando(), "ventana: con el #FIN ya no anda suelto")
        llegan(HOLA_BUENO, "#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=0 nuevo=0 mismo=1 hace=5",
               "#GUARDADO que=cancha hay=0", S_BUENA)
        verificar(cable.mandados[antes:] == ["?"] and v._puede()[0], "ventana: reconectado, vuelven los botones %r" % cable.mandados[antes:])
        # ---- SIN CABLE ----
        v._elegir(6)
        v._poner_sin_cable(True)
        v._probar()
        verificar(cable.mandados[-1] == "!6 c" and v.estado.que_sin_cable == "mov", "sin cable: PROBAR manda !6 c")
        llegan(">>> 6 DESPEJE SIN CABLE: arranca en 10 s. DESENCHUFEN EL USB. Al terminar, el LED dice la nota.",
               "#CUENTA 10.0 sincable", "#CUENTA 9.0 sincable")
        v._pintar_cuenta()
        verificar(mostro("DESENCHUFÁ"), "sin cable: DESENCHUFÁ EL USB AHORA")
        v._procesar_lote([("desconectado", "COM11")])
        verificar(v.ultimo_aviso[1] == VIOLETA and not v._puede()[0] and "SIN CABLE" in v._puede()[1],
                  "sin cable: desconectado sin aviso rojo, PROBAR apagado")
        v._pintar_conexion()
        verificar(mostro("SIN CABLE"), "sin cable: el cartel violeta arriba")
        v.t_ultima_linea -= 300
        v._latido()
        verificar(v.ultimo_aviso[1] != ROJO and v.estado.esperando_sin_cable, "sin cable: el latido de silencio no salta")
        v._parar()
        verificar(SIN_CABLE_NO_LLEGA in v.ultimo_aviso[0], "sin cable: PARAR no llega")
        # el reloj, como en la cancha: la cuenta empezó hace 30 s, y el DESPEJE
        # anterior (el que llegó en vivo) terminó hace 60 s
        v.estado.t_sin_cable -= 30
        v.estado.vistos = [(q, c, t - 60) for q, c, t in v.estado.vistos]
        antes = len(cable.mandados)
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        verificar(len(cable.mandados) == antes, "sin cable: al volver no manda nada solo")
        tanda = demo_resultado(6, VALORES_DE_MENTIRA, random.Random(66))
        llegan(HOLA_BUENO, "#GUARDADO que=mov hay=1 n=6 nombre=DESPEJE nota=CASI sincable=1 nuevo=1 mismo=1 hace=900",
               "#GUARDADO que=cancha hay=0")
        verificar(cable.mandados[antes:] == ["?", "r"], "sin cable: manda ? y r %r" % cable.mandados[antes:])
        llegan(S_BUENA, S_BUENA)
        verificar(cable.mandados[antes:].count("r") == 1, "sin cable: r UNA sola vez")
        llegan("#REPITE 6 DESPEJE sincable=1 apagado=0", *tanda)
        verificar(not v.estado.esperando_sin_cable and v.estado.de_guardado and v.estado.informe["n"] == 6
                  and v.t_pedido_repetir is None, "sin cable: llegó la tanda guardada")
        v._pintar_resultado()
        verificar(mostro("Resultado guardado (SIN CABLE)"), "sin cable: el cartel guardado")
        llegan("#REPITE 6 DESPEJE sincable=1 apagado=1", *tanda)
        v._pintar_resultado()
        verificar(mostro("se apagó"), "sin cable: apagado=1")
        v._poner_sin_cable(False)
        # ---- CANCHA ----
        verificar(v._bloqueado("2") and v._bloqueado("3") and v._bloqueado("5") and not v._bloqueado("4"),
                  "cancha: 2, 3 y 5 bloqueados sin el 1")
        v._elegir_paso("2")
        antes = len(cable.mandados)
        verificar(not v._medir_cancha("!A", ("A",), True) and len(cable.mandados) == antes, "cancha: el 2 bloqueado no manda")
        for clave, _, _, _ in PASOS_CANCHA:
            v._elegir_paso(clave)
        v._elegir_paso("1")
        verificar(v._medir_cancha("!L v", ("L", "v")) and cable.mandados[-1] == "!L v", "cancha: MEDIR VERDE")
        hoja_visible = []
        original = v._pintar_cancha

        def espia():
            hoja_visible.append(v.ruta_hoja is not None and os.path.exists(v.ruta_hoja)
                                and "verde_1" in open(v.ruta_hoja, encoding="utf-8").read())
            original()
        v._pintar_cancha = espia
        llegan("#INICIO L LINEA_VERDE tope=5000 sincable=0", LINEAS_DE_MENTIRA["v"], "#FIN L")
        v._pintar_cancha = original
        verificar(hoja_visible and all(hoja_visible), "cancha: la hoja se escribe ANTES de mostrar")
        verificar(os.path.dirname(v.ruta_hoja) == canchas and os.path.exists(os.path.join(canchas, NOMBRE_HOJA_ESCUELA)),
                  "cancha: la hoja va en la carpeta temporal (y la de la escuela al lado)")
        for fase in ("i", "d"):
            v._medir_cancha("!L %s" % fase, ("L", fase))
            llegan("#INICIO L LINEA tope=5000 sincable=0", LINEAS_DE_MENTIRA[fase], "#FIN L")
        verificar(v._sugeridos()["umbralBlanco"][0] == "425", "cancha: el paso 1 da 425")
        v._usar_sugerido()
        verificar(cable.mandados[-1] == "=umbralBlanco 425" and not v._bloqueado("2"), "cancha: USAR ESTE -> =umbralBlanco 425")
        llegan("#OK umbralBlanco=425", V_BUENA)
        v._copiar_sugerido()
        verificar("0.1103" in v.ultimo_aviso[0] and "PASAR A PARTIDO" in v.ultimo_aviso[0],
                  "cancha: COPIAR AL PANEL dice que es para la 0.1103 y que para jugar es PASAR A PARTIDO")
        verificar(v.ultima_linea_copiada.startswith("int umbralBlanco" + " " * 33 + "= 425;   // [MEDIDO ")
                  and "desde que lectura" in v.ultima_linea_copiada, "cancha: COPIAR AL PANEL con la línea exacta")
        texto = open(v.ruta_hoja, encoding="utf-8").read()
        verificar("umbral = 425" in texto and "umbralBlanco = 425 -> 425" in texto, "cancha: la hoja tiene el umbral")
        # el paso 2 con dos #ANCHO
        v._elegir_paso("2")
        for linea in (ANCHO_BUENO, ANCHO_BUENO.replace("2213", "2243").replace("2314", "2348")):
            verificar(v._medir_cancha("!A", ("A",), True) and cable.mandados[-1] == "!A", "cancha: MEDIR el ancho")
            llegan("#CUENTA 3.0 cable", "#INICIO A ANCHO tope=45000 sincable=0", "#PASO A calibrando giro=3",
                   "#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0", linea, "#FIN A")
            v.w["cm"].get.return_value = ""
            v._en_el_medio("anchos", "SI")
        verificar(v._sugeridos()["MS_MEDIO_AL_MEDIO"][0] == "2228", "cancha: el paso 2 da 2228")
        v._usar_sugerido()
        verificar(cable.mandados[-1] == "=MS_MEDIO_AL_MEDIO 2228", "cancha: USAR ESTE del ancho")
        # el paso 5: !10 recién después del #OK
        v._elegir_paso("5")
        v._ida(250)
        verificar(cable.mandados[-1] == "=msPruebaIda 250", "cancha: IDA 250 manda primero el número")
        # (revisión 08/11) la 0921 termina la ida SOLTANDO: el paso 5 mide igual (frenarAntesDeVolver 0)
        verificar(cable.mandados[-2] == "=frenarAntesDeVolver 0" and v._valor_de_hoy("frenarAntesDeVolver")[0] == "false"
                  and any(f[0] == "frenarAntesDeVolver" for f in numeros_fuera_del_partido(
                      v.estado.valores, {"frenarAntesDeVolver": "false"}, ("frenarAntesDeVolver",))),
                  "cancha: IDA con el robot frenando manda antes =frenarAntesDeVolver 0")
        llegan("#OK msPruebaIda=250", V_BUENA)
        verificar(cable.mandados[-1] == "!10", "cancha: el !10 recién después del #OK")
        llegan("#CUENTA 3.0 cable", "#INICIO 10 IDA tope=3250 sincable=0",
               *demo_resultado(10, dict(VALORES_DE_MENTIRA, msPruebaIda="250"), random.Random(1)))
        v.w["cm"].get.return_value = "19"
        v._anotar_ida()
        v._ida(350)
        llegan("#OK msPruebaIda=350", "#CUENTA 3.0 cable", "#INICIO 10 IDA tope=3350 sincable=0",
               *demo_resultado(10, dict(VALORES_DE_MENTIRA, msPruebaIda="350"), random.Random(2)))
        v.w["cm"].get.return_value = "33"          # (las cajitas de mentira son todas la misma)
        v._anotar_ida()
        v.w["quiero"].get.return_value = "35"
        verificar(v.cancha["idas"] == [[250, 19.0], [350, 33.0]] and v._sugeridos()["msAdelante"][0] == "364",
                  "cancha: la recta de las idas %r" % (v._sugeridos()["msAdelante"][0],))
        # un IDA cuyo #OK no llegó a tiempo NO manda !10 después (ni al tocar msPruebaIda con + y -)
        v._ida(500)
        verificar(cable.mandados[-1] == "=msPruebaIda 500", "cancha: IDA 500")
        v.cancha["ida_espera_ok"] = (500, time.monotonic() - 10)
        llegan("#OK msPruebaIda=500")
        verificar(cable.mandados[-1] == "=msPruebaIda 500", "cancha: un #OK viejo de msPruebaIda no manda !10")
        v._ida(500)
        v._procesar_lote([("desconectado", "COM11"), ("conectado", "COM11", SERIE_ARQUERO)])
        verificar(v.cancha["ida_espera_ok"] is None and v.cancha["ida_en_curso"] is None,
                  "cancha: al reconectar se olvida la ida")
        llegan(HOLA_BUENO, "#GUARDADO que=mov hay=0", "#GUARDADO que=cancha hay=0", V_BUENA, S_BUENA,
               "#OK msPruebaIda=500")
        verificar(cable.mandados[-1] != "!10", "cancha: después de reconectar, el #OK no manda !10")
        llegan("#OK msPruebaIda=383")
        # una ida vieja que quedó "en curso" (p. ej. SIN CABLE y se perdió) no
        # se anota con el #FIN 10 de una prueba de MOVIMIENTOS
        idas_antes = list(v.cancha["idas"])
        v.cancha["ida_en_curso"] = 500
        elegido_antes, sin_cable_antes = v.elegido, v.sin_cable
        v.elegido, v.sin_cable = 10, False
        v._probar()
        verificar(cable.mandados[-1] == "!10" and v.cancha["ida_en_curso"] is None,
                  "cancha: PROBAR en MOVIMIENTOS olvida la ida vieja")
        llegan("#CUENTA 3.0 cable", "#INICIO 10 IDA tope=3383 sincable=0",
               *demo_resultado(10, VALORES_DE_MENTIRA, random.Random(4)))
        verificar(v.cancha["idas"] == idas_antes, "cancha: el #FIN 10 de MOVIMIENTOS no se anota como ida")
        v.elegido, v.sin_cable = elegido_antes, sin_cable_antes
        # el resto de los pasos, con datos
        v._elegir_paso("3")
        v._medir_cancha("!F", ("F", "medir"))
        llegan("#CUENTA 3.0 cable", "#INICIO F FONDO tope=6000 sincable=0")
        v._actualizar_habilitados()
        v._toco()
        verificar(cable.mandados[-1] == "t", "cancha: TOCÓ manda t moviéndose")
        llegan("#FONDO ms=1730 como=TOCO desvio=1.8 umbral=425 probado=0", "#FIN F")
        llegan("#INICIO F FONDO tope=6000 sincable=0", "#FONDO ms=1690 como=TOCO desvio=1.8 umbral=425 probado=0", "#FIN F")
        verificar(v._sugeridos()["MS_MEDIO_FONDO_POR_TIEMPO"][0] == "2250", "cancha: el fondo da 2250")
        v._elegir_paso("4")
        for cm, prom in ((10, 28), (20, 58), (30, 88.9), (40, 117), (50, 142)):
            v._medir_cancha("!C", ("C", "4b", cm))
            llegan("#INICIO C CAMARA tope=4000 sincable=0", "#CAM n=76 cero=0 val=76 xp=1,2,%s yp=-1.2 am=0 az=0" % prom, "#FIN C")
        verificar(v._sugeridos()["CAMARA_POR_CM"][0] == "2.87", "cancha: la tabla de la cámara da 2.87")
        v._medir_cancha("!C", ("C", "4a", 0))
        llegan("#INICIO C CAMARA tope=4000 sincable=0", "#CAM n=76 cero=76 val=0 xp=0,0,0.0 yp=0.0 am=0 az=0", "#FIN C")
        # repetida sola con rc: si es de ANTES DE APAGAR, o esta ventana no la pidió, NO se suma
        antes_lineas = len(v.cancha["lineas"])
        llegan("#REPITE L LINEA_GUIADA sincable=1 apagado=1", LINEAS_DE_MENTIRA["v"], "#FIN L")
        verificar(len(v.cancha["lineas"]) == antes_lineas and "ANTES DE APAGAR" in v.ultimo_aviso[0],
                  "cancha: una medición de antes de apagar NO se suma")
        llegan("#REPITE L LINEA_GUIADA sincable=1 apagado=0", LINEAS_DE_MENTIRA["v"], "#FIN L")
        verificar(len(v.cancha["lineas"]) == antes_lineas, "cancha: una repetida que esta ventana no pidió NO se suma")
        v.ent_consola.get.return_value = "rc"
        v._consola_mandar()
        verificar(cable.mandados[-1] == "rc" and v.rc_a_mano, "cancha: rc escrito a mano")
        llegan("#REPITE L LINEA_GUIADA sincable=1 apagado=0", LINEAS_DE_MENTIRA["v"], "#FIN L")
        verificar(len(v.cancha["lineas"]) == antes_lineas + 1 and not v.rc_a_mano, "cancha: con rc a mano SÍ se suma")
        verificar(v._sugeridos()["umbralBlanco"][0] == "425", "cancha: el umbral sigue en 425")
        v._elegir_paso("2c")
        v._medir_cancha("!I", ("I",), True)
        llegan("#CUENTA 3.0 cable", "#INICIO I IR_AL_MEDIO tope=40000 sincable=0", "#PASO I paso=1 ms=1830 como=L",
               "#MEDIO ok=1 pasos=1830,400,2000,943,610,2228,520,320 como=L,T,T,T,L,T,L,T desvio=3.1 acomodo=0", "#FIN I")
        v._elegir_paso("6")
        v._signo_probar()
        verificar(cable.mandados[-1] == "!2", "cancha: 6a prueba el 2")
        llegan("#CUENTA 3.0 cable", "#INICIO 2 COSTADO_DER tope=4000 sincable=0",
               *demo_resultado(2, VALORES_DE_MENTIRA, random.Random(2)))
        v._signo_lateral(True)
        llegan(S_BUENA)
        v._signo_camara()
        verificar(v._sugeridos()["lateralInvertido"][0] == "0" and v._sugeridos()["camaraYInvertida"][0] == "1",
                  "cancha: los signos")
        # 6a contra el lateralInvertido CON EL QUE CORRIÓ: con el 1 puesto y fue a la DERECHA, queda el 1
        llegan("#OK lateralInvertido=1")
        v._signo_probar()
        verificar(cable.mandados[-1] == "!2", "cancha: 6a de nuevo, con el 1 puesto")
        llegan("#CUENTA 3.0 cable", "#INICIO 2 COSTADO_DER tope=4000 sincable=0",
               *demo_resultado(2, dict(VALORES_DE_MENTIRA, lateralInvertido="1"), random.Random(3)))
        v._signo_lateral(True)
        verificar(v._sugeridos()["lateralInvertido"][0] == "1", "cancha: 6a con el 1 puesto y a la DERECHA: sigue el 1")
        v._signo_lateral(False)
        verificar(v._sugeridos()["lateralInvertido"][0] == "0", "cancha: 6a con el 1 puesto y a la IZQUIERDA: el 0")
        llegan("#OK lateralInvertido=0")
        # con números de moverse que no son los del partido, CANCHA avisa
        llegan("#OK RAMPA_MOV_PASO=8")
        verificar(any("RAMPA_MOV_PASO 8 (partido 10)" in a[0] for a in v._sugeridos()["MS_MEDIO_AL_MEDIO"][1])
                  and any("RAMPA_MOV_PASO" in q and ok == "amarillo" for q, ok in v._chequeos()),
                  "cancha: avisa que mide con una rampa que no es la del partido")
        llegan("#OK RAMPA_MOV_PASO=10")
        verificar(not any("partido 10" in a[0] for a in v._sugeridos()["MS_MEDIO_AL_MEDIO"][1]),
                  "cancha: con la rampa del partido no avisa")
        v._elegir_paso("0")
        v._tilde_bateria()
        v._elegir_paso("R")
        v._tilde_revisado(0)
        v._pintar_cancha()
        v._comparar()
        verificar(any("umbralBlanco" in l and "425" in l for l in v.ultima_comparacion), "cancha: COMPARAR")
        v._terminar_cancha()
        # (revisión 08/11) "Volver a esta cancha" ya NO deja líneas para el panel de la 0.1103:
        # abre la ventanita de PASAR A PARTIDO con los números de ESA hoja (solo el editor).
        copiada_antes = v.ultima_linea_copiada
        filas_hoja = v._medido_para_partido(hoja=texto_a_hoja(HOJA_ESCUELA), fuente="cancha escuela")
        pasar = v._volver_a_cancha("escuela", texto_a_hoja(HOJA_ESCUELA))
        verificar(callable(pasar) and v.ultima_linea_copiada == copiada_antes
                  and {"umbralBlanco", "MS_MEDIO_AL_MEDIO"} <= {f["nombre"] for f in filas_hoja}
                  and all(f["fuente"] == "cancha escuela" and f["hoy"] == v.lectura["valores"][f["nombre"]]
                          for f in filas_hoja), "cancha: volver a esta cancha abre PASAR con la hoja %r" % (
                      [(f["nombre"], f["hoy"], f["medido"]) for f in filas_hoja],))
        verificar(not any(f["nombre"] == "msAdelante" and f["tildado"] for f in filas_hoja),
                  "cancha: volver a esta cancha: msAdelante sin tildar")
        pasar()
        verificar(v.pestana_elegida == "partido" and all(v.origen_partido.get(n) == "hoja" for n in v.pendientes_partido),
                  "cancha: volver a esta cancha va a PARTIDO y solo toca el editor")
        v._partido_descartar()
        v._ir_a_pestana("cancha")
        llegan(V_BUENA.replace("umbralBlanco=425", "umbralBlanco=300"))
        verificar(v._numeros_de_la_hoja_distintos() == [("umbralBlanco", "425")], "cancha: lo de la hoja que falta poner")
        v._reponer_de_la_hoja()
        verificar(cable.mandados[-1] == "=umbralBlanco 425", "cancha: volver a poner los de la hoja")
        g["filedialog"].askopenfilename.return_value = v.ruta_hoja
        v._hoja_abrir()
        verificar(v.hoja is not None and hoja_valor(v.hoja, "LINEA", "umbral") == "425", "cancha: abrir la hoja")
        # ---- VOLVER AL PARTIDO (2026-11-08): ya NO lanza CARGAR-PARTIDO.bat (cargaba la 0.1103) ----
        llegan(V_BUENA, S_BUENA)
        g["messagebox"].askyesno.return_value = False
        v._volver_al_partido()
        verificar(lanzados == [] and not cable.cerrado and v.pestana_elegida == "partido",
                  "volver al partido: va a la pestaña PARTIDO, no lanza nada, y con NO no carga")
        g["messagebox"].askyesno.return_value = True
        # todo lo mandado: ASCII, de 71 o menos, y solo al arquero
        verificar(all(m.encode("ascii") and len(m) <= LARGO_MAX_ORDEN for m in cable.mandados),
                  "ventana: todo lo mandado es ASCII de 71 o menos")
        # al cerrar manda x
        v2 = Ventana(mock.MagicMock(name="raiz2"), _CableDePrueba(), queue.Queue(), estricto=True,
                     carpeta_canchas=canchas)
        v2._procesar_lote([("conectado", "COM11", SERIE_ARQUERO), ("linea", HOLA_BUENO)])
        v2.conexion.mandados[:] = []
        v2._cerrar()
        verificar(v2.conexion.mandados == ["x"] and v2.cerrada, "ventana: al cerrar manda x")
        # a otra serie NUNCA le manda nada
        c3 = _CableDePrueba()
        v3 = Ventana(mock.MagicMock(name="raiz3"), c3, queue.Queue(), estricto=True, carpeta_canchas=canchas)
        v3._procesar_lote([("conectado", "COM3", "16000000"), ("linea", HOLA_BUENO), ("linea", S_BUENA)])
        v3._probar()
        v3._parar()
        v3._paso("umbralCm", 1)
        v3.ent_consola.get.return_value = "!1"
        v3._consola_mandar()
        v3._cerrar()
        verificar(c3.mandados == [], "ventana: a otra serie no le manda nada %r" % c3.mandados)
        # la demo: VOLVER AL PARTIDO nunca lanza nada; y una falla no la tira abajo
        lanzados4 = []
        v4 = Ventana(mock.MagicMock(name="raiz4"), _CableDePrueba(), queue.Queue(), demo=True, carpeta_canchas=canchas,
                     lanzador=lambda *a, **k: lanzados4.append(a))
        v4._procesar_lote([("conectado", "DEMO", SERIE_ARQUERO), ("linea", HOLA_BUENO)])
        v4._volver_al_partido()
        verificar(lanzados4 == [] and "demo" in v4.ultimo_aviso[0], "demo: no se carga nada")
        v4._procesar_lote([("tipo-que-no-existe",), ("linea",)])     # el segundo rompe: queda anotado y sigue
        # la ventana con el robot de mentira entero (sin hilos), y el flujo sin cable de punta a punta
        cola5 = queue.Queue()
        robot = RobotDeMentira(cola5, escala_tiempo=0.001)
        robot.forzar_linea = False
        v5 = Ventana(mock.MagicMock(name="raiz5"), robot, cola5, demo=True, estricto=True, carpeta_canchas=canchas)
        cola5.put(("conectado", "DEMO", SERIE_ARQUERO))
        robot.saludo_de_arranque()
        cola5.put(("linea", robot.linea_S()))

        def girar():
            for _ in range(20):
                lote = []
                while not cola5.empty():
                    lote.append(cola5.get())
                v5._procesar_lote(lote)
                while not robot._entrada.empty():
                    robot.comando(robot._entrada.get())
                if cola5.empty():
                    break
        girar()
        verificar(v5._programa_ok() and v5.estado.valores == VALORES_DE_MENTIRA, "demo: arranca y pide los números")
        v5._elegir(9)
        v5._poner_sin_cable(True)
        v5._probar()
        girar()
        verificar(v5.estado.de_guardado and v5.estado.informe["n"] == 9 and not v5.estado.esperando_sin_cable,
                  "demo: la 9 SIN CABLE de punta a punta")

        # Nada de lo que se mostró tiene caracteres de más de U+FFFF (un Tk
        # viejo no puede mostrarlos y tira un error).
        def textos(x):
            if isinstance(x, str):
                yield x
            elif isinstance(x, (list, tuple)):
                for y in x:
                    yield from textos(y)
            elif isinstance(x, dict):
                for y in x.values():
                    yield from textos(y)
        llamadas = [c for nombre in ("tk", "ttk", "messagebox") for c in g[nombre].mock_calls] + list(raiz.mock_calls)
        raros = {t for c in llamadas for t in textos(list(c[1]) + list(c[2].values()))
                 if any(ord(ch) > 0xFFFF for ch in t)}
        verificar(len(llamadas) > 1000 and not raros, "ventana: caracteres que Tk no muestra: %r" % list(raros)[:3])
    finally:
        g.update(guardados)


def _ino_sintetico(sin=(), extra_arriba=(), extra_abajo=()):
    """Un .ino de mentira armado desde TABLA_PARTIDO, con LF, emojis y TODAS
    las trampas del de verdad (2026-11-08, DISENO 16)."""
    l = ["/* =====================================================================",
         "   ARQUERO de mentira 🚨 (para la autoprueba)",
         "   umbralBlanco = 999 -> esto es un comentario, no se lee",
         "int umbralBlanco = 1;",
         "   volverAlMedioEnElLateral = false -> sin limite",
         "   ===================================================================== */",
         "#include <Arduino.h>",
         "// int umbralBlanco = 2;",
         "int umbralBlancoX = 3;",
         "int Xp = 0, Yp = 0;",
         "enum Fase { APAGADO, ARMANDOSE,",
         "            ESPERANDO };",
         'const char* VERSION_PARTIDO = "0921g";',
         'const char* SELLO = "fabrica";']
    l += list(extra_arriba)
    for f in TABLA_PARTIDO:
        if f[0] in sin:
            continue
        l.append("%s %s = %s;   // 🏷️ %s vale %s (el comentario repite el número) {" % (f[1], f[0], f[3], f[0], f[3]))
    l += ["void funcion() {",
          "  int msAdelante = 1;            // sangrado: es de la función",
          '  const char* x = "{ no cuenta";',
          "  char c = '}';",
          "  if (x) { msAdelante = 2; }",
          "}"]
    l += list(extra_abajo)
    l.append("")
    return "\n".join(l)


def _sin_comentarios(texto):
    """El texto sin // ni /* */ (respeta las "cadenas"): para buscar código."""
    salida, i, n, estado = [], 0, len(texto), "normal"
    while i < n:
        c = texto[i]
        if estado == "normal":
            if texto.startswith("/*", i):
                estado, i = "bloque", i + 2
                continue
            if texto.startswith("//", i):
                estado, i = "linea", i + 2
                continue
            if c in "\"'":
                estado = c
            salida.append(c)
        elif estado == "bloque":
            if texto.startswith("*/", i):
                estado, i = "normal", i + 2
                continue
        elif estado == "linea":
            if c == "\n":
                estado = "normal"
                salida.append(c)
        else:
            salida.append(c)
            if c == "\\":
                salida.append(texto[i + 1:i + 2])
                i += 2
                continue
            if c == estado:
                estado = "normal"
        i += 1
    return "".join(salida)


def _archivos_de(carpeta):
    try:
        return sorted(os.listdir(carpeta))
    except OSError:
        return []


def _autoprueba_partido(verificar, carpeta):
    """A a H del DISENO 16: la tabla, leer, la ida y vuelta, un cambio, los
    inválidos, el archivo raro, las copias, por afuera y las fallas. Todo
    sobre COPIAS en la carpeta temporal."""
    from unittest import mock
    # ---- A. La tabla ----
    nombres = [f[0] for f in TABLA_PARTIDO]
    verificar(len(nombres) == len(set(nombres)) == 73, "partido: 73 nombres sin repetir (%d)" % len(nombres))
    for f in TABLA_PARTIDO:
        nombre, decl, tipo, fabrica, unidad, minimo, maximo, paso = f[:8]
        verificar(tipo in ("i", "u", "f", "b", "l", "v") and len(f) == 12, "partido: el tipo de %s" % nombre)
        verificar(f[8] in [g[0] for g in GRUPOS_PARTIDO], "partido: el grupo de %s" % nombre)
        verificar(_RE_VALOR_SEGUN_TIPO[tipo].match(fabrica), "partido: la fábrica de %s se escribe bien" % nombre)
        if tipo in ("i", "u", "f"):
            verificar(paso is not None and paso > 0 and minimo <= valor_de_texto(nombre, fabrica) <= maximo,
                      "partido: %s de fábrica cae en [mín, máx] y paso > 0" % nombre)
        else:
            verificar(paso is None, "partido: %s no tiene paso" % nombre)
        if "fuerza" in unidad and tipo in ("i", "u"):
            verificar(maximo <= 255, "partido: %s nunca pasa de 255" % nombre)
    verificar(not set(ESTADO_DEL_PARTIDO) & set(nombres), "partido: ninguna variable de estado en la tabla")
    verificar(all(TIPO_PARTIDO.get(n) == "b" for n in ORDEN_INTERRUPTORES) and len(ORDEN_INTERRUPTORES) == 13,
              "partido: los 13 interruptores son bool de la tabla")
    verificar(all(n in FILA_PARTIDO for n in NUMEROS_LINEA_PARTIDO) and CAMPOS_PARTIDO[0] == "version"
              and CAMPOS_PARTIDO[-1] == "interruptores", "partido: los campos de la línea están en la tabla")
    verificar(VALORES_DE_FABRICA == {f[0]: f[3] for f in TABLA_PARTIDO}, "partido: VALORES_DE_FABRICA = la columna hoy")
    respaldo = os.path.normpath(os.path.join(CARPETA, "..", "..", "respaldos", "partido-0921f", "partido-0921f.ino"))
    if os.path.exists(respaldo):
        with open(respaldo, "rb") as fr:           # solo LEERLO
            r0 = leer_partido(fr.read().decode("utf-8"))
        verificar(r0["valores"] == VALORES_DE_FABRICA and not r0["faltan"] and r0["version"] is None,
                  "partido: la fábrica es la 0921f congelada en respaldos (%r)" % r0["faltan"][:3])
    # ---- B. Leer ----
    sint = _ino_sintetico()
    r = leer_partido(sint)
    verificar(not r["faltan"] and not r["repetidos"] and not r["raros"] and not r["problemas"],
              "partido: el sintético se lee entero (%r %r %r %r)" % (r["faltan"][:3], r["repetidos"], r["raros"][:2],
                                                                   r["problemas"]))
    verificar(r["valores"] == VALORES_DE_FABRICA and r["version"] == "0921g" and r["sello"] == "fabrica",
              "partido: el sintético da los de fábrica")
    verificar(r["tipos"]["MS_MEDIO_AL_MEDIO"] == "const unsigned long", "partido: el tipo tal cual")
    mapa, problema = mapa_de_lineas(sint)
    verificar(problema is None and len(mapa) == len(sint.split("\n")), "partido: el mapa de las llaves")
    verificar(mapa[3] == (0, True) and mapa[11] == (1, False), "partido: el mapa ve el /* */ y el enum")
    real = os.path.join(PARTIDO_CARPETA, NOMBRE_PARTIDO_INO)       # (la COPIA en la temporal)
    hay_real = os.path.exists(real)
    if hay_real:
        with open(real, "rb") as fr:
            datos_real = fr.read()
        rr = leer_partido(datos_real.decode("utf-8"))
        verificar(not rr["faltan"] and not rr["repetidos"] and not rr["raros"] and not rr["problemas"],
                  "el partido cambió: revisar TABLA_PARTIDO (faltan %r, repetidos %r, raros %r, problemas %r)" % (
                      rr["faltan"][:5], rr["repetidos"][:5], rr["raros"][:3], rr["problemas"]))
        verificar(b"\r" not in datos_real and datos_real.endswith(b"\n"), "partido: el de verdad es LF")
    # ---- C. Ida y vuelta ----
    verificar(escribir_partido(sint, r, {})[0] == sint, "partido: sin cambios da el mismo texto")
    mismos = dict(VALORES_DE_FABRICA, CAMARA_POR_CM="2.870", umbralCm="30", msAdelante="533.0")
    del mismos["VEL_MEDIO_MEDIR"]
    nuevo, reales = escribir_partido(sint, r, mismos, sello="20261108-154210")
    verificar(nuevo == sint and reales == [], "partido: con los mismos valores (2.870, 30) da el mismo texto")
    crlf = sint.replace("\n", "\r\n")
    rc = leer_partido(crlf)
    verificar(rc["valores"] == VALORES_DE_FABRICA and escribir_partido(crlf, rc, mismos)[0] == crlf,
              "partido: con CRLF también queda igual")
    nuevo, _ = escribir_partido(crlf, rc, {"umbralBlanco": "430"})
    dif = [(a, b) for a, b in zip(crlf.split("\n"), nuevo.split("\n")) if a != b]
    verificar(len(dif) == 1 and dif[0][1].endswith("\r") and "= 430;" in dif[0][1], "partido: un cambio con CRLF")
    copias = os.path.join(carpeta, "copias-partido-prueba")
    if hay_real:
        lista_antes = _archivos_de(PARTIDO_CARPETA)
        lec = leer_partido_archivo(real)
        info = guardar_partido(real, copias, {n: x for n, x in lec["valores"].items() if TIPO_PARTIDO[n] != "v"},
                               lec["huella"])
        with open(real, "rb") as fr:
            verificar(fr.read() == datos_real and info["sin_cambios"] and not _archivos_de(copias)
                      and _archivos_de(PARTIDO_CARPETA) == lista_antes,
                      "partido: guardar lo mismo no toca nada (ni copia, ni .tmp)")
    # ---- D. Un cambio ----
    nuevo, reales = escribir_partido(sint, r, {"umbralBlanco": "430"})
    dif = [(i, a, b) for i, (a, b) in enumerate(zip(sint.split("\n"), nuevo.split("\n"))) if a != b]
    n0 = r["donde"]["umbralBlanco"][0]
    verificar(len(dif) == 1 and dif[0][0] == n0 and dif[0][2] == dif[0][1].replace("= 425;", "= 430;", 1)
              and reales == [("umbralBlanco", "425", "430")], "partido: umbralBlanco 425 -> 430, una sola línea")
    verificar(dif[0][2].endswith("vale 425 (el comentario repite el número) {"), "partido: el comentario sigue igual")
    r2 = leer_partido(nuevo)
    verificar(r2["valores"] == dict(VALORES_DE_FABRICA, umbralBlanco="430"), "partido: leído de nuevo da 430")
    nuevo, _ = escribir_partido(sint, r, {"msAdelante": "540"})
    l_nueva = nuevo.split("\n")[r["donde"]["msAdelante"][0]]
    verificar(l_nueva.startswith("int msAdelante = 540;") and "msAdelante vale 533" in l_nueva,
              "partido: msAdelante 540 y el comentario con 533 intacto")
    for nombre, pedido, esperado in (("kpLateral", "12", "12.0"), ("CAMARA_POR_CM", "2.9", "2.90"),
                                     ("rendimientoCostado", "0.75", "0.75"), ("despejeEnDiagonal", "true", "true"),
                                     ("perseguirEnElDespeje", "false", "false"), ("LADO_MEDIO", "1", "1"),
                                     ("MS_QUIETO_PARA_PATRULLAR", "15000", "15000"), ("umbralCm", "31", "31.0"),
                                     ("kpPersecucion", "0", "0.0"), ("CAMARA_POR_CM", "2,95", "2.95"),
                                     ("MS_MEDIO_AL_MEDIO", "2228.0", "2228")):
        verificar(texto_para_ino(nombre, pedido, VALORES_DE_FABRICA[nombre]) == esperado,
                  "partido: %s %s -> %s (%r)" % (nombre, pedido, esperado,
                                                 texto_para_ino(nombre, pedido, VALORES_DE_FABRICA[nombre])))
    nuevo, reales = escribir_partido(sint, r, {"umbralBlanco": "430"}, sello="20261108-154210")
    dif = [i for i, (a, b) in enumerate(zip(sint.split("\n"), nuevo.split("\n"))) if a != b]
    r3 = leer_partido(nuevo)
    verificar(sorted(dif) == sorted([n0, r["donde_sello"][0]]) and re.match(r"^[0-9]{8}-[0-9]{6}$", r3["sello"]),
              "partido: con sello cambian solo esa línea y la del SELLO")
    # ---- E. Inválidos (dan error, el archivo queda igual y no hay copia) ----
    for malo in ("abc", "", "4.5", "1e3", "0x10", "nan", "inf", "425;", "425 // x", "4\n25", "٤٢٥", " ", "-",
                 "4294967296000000000"):
        verificar(validar_valor_partido("umbralBlanco", malo)[1] is not None, "partido: rechaza %r" % malo)
    for malo in ("-5", "2228.5"):
        verificar(validar_valor_partido("MS_MEDIO_AL_MEDIO", malo)[1] is not None, "partido: unsigned rechaza %r" % malo)
    verificar(validar_valor_partido("umbralBlanco", "2000")[1] == "umbralBlanco va de 100 a 1000",
              "partido: el texto del rango")
    for nombre, malo in (("pwmMaxLateral", "300"), ("LADO_MEDIO", "0"), ("VEL_MEDIO_MEDIR", "100"),
                         ("noExiste", "1"), ("despejeEnDiagonal", "2"), ("kpLateral", "40")):
        verificar(validar_valor_partido(nombre, malo)[1] is not None, "partido: rechaza %s = %s" % (nombre, malo))
    verificar(validar_valor_partido("umbralBlanco", "430") == (430, None) and
              validar_valor_partido("kpLateral", "12,5") == (12.5, None), "partido: acepta lo bueno")
    if hay_real:
        lec = leer_partido_archivo(real)
        for cambios in ({"umbralBlanco": "abc"}, {"umbralBlanco": "2000"}, {"VEL_MEDIO_MEDIR": "90"},
                        {"noExiste": "3"}, {"pwmMinLateral": "110", "pwmMaxLateral": "100"}):
            try:
                guardar_partido(real, copias, cambios, lec["huella"])
                salio = "escribió"
            except NoSeEscribe:
                salio = "no"
            with open(real, "rb") as fr:
                verificar(salio == "no" and fr.read() == datos_real and not _archivos_de(copias),
                          "partido: %r no se escribe ni hace copia" % cambios)
    # las reglas
    fab = dict(VALORES_DE_FABRICA)

    def codigos(cambios, medido=None):
        return {c for _, _, _, c in _reglas_con_codigo(dict(fab, **cambios), fab, medido)}

    verificar("E6" in codigos({"pwmMinLateral": "170"}), "partido: pwmMinLateral 170 da error")
    verificar("E1" in codigos({"pwmMinLateral": "110", "pwmMaxLateral": "100"}), "partido: E1")
    verificar("E2" in codigos({"PWM_MIN_ACOMODO": "100", "PWM_MAX_ACOMODO": "90"}), "partido: E2")
    verificar("E3" in codigos({"TOLERANCIA_ACOMODO": "6.0", "TOLERANCIA_ESPERA": "6.0"}), "partido: E3")
    verificar("E5" in codigos({"umbralCm": "45.0", "msAdelante": "400"}), "partido: E5")
    verificar("E4" in codigos({"MS_MAX_ESPERA_PLANO": "300", "MS_PAUSA_MEDIO": "400"}), "partido: E4")
    reglas = _reglas_con_codigo(fab, fab)
    verificar([(n, c) for n, _, _, c in reglas] == [("nota", "N1")], "partido: con fábrica solo la N1 (%r)" % reglas)
    verificar("O2" in codigos({"msAdelante": "700"}) and "O6" in codigos({"LADO_TRASERA": "90"}),
              "partido: O2 con msAdelante 700 y O6 al tocar LADO_TRASERA")
    verificar({"O1", "O3", "O4"} <= codigos({"msAdelante": "420", "pwmMaxLateral": "170",
                                              "volverAlMedioEnElLateral": "false"}), "partido: O1, O3, O4")
    verificar({"O7", "O14"} <= codigos({"potenciaDespeje": "210"}) and "E5" not in codigos(
        {"potenciaDespeje": "210", "umbralCm": "45.0", "msAdelante": "400"}), "partido: O7/O14 (y sin 200 no hay E5)")
    verificar({"O8", "O9", "O10", "O11", "O12", "O13", "O17"} <= codigos(
        {"CAMARA_POR_CM": "2.90", "LADO_MEDIO": "1", "despejeEnDiagonal": "true", "enderezarActivado": "false",
         "camaraYInvertida": "false", "patrullarSinPelota": "false", "MS_MAX_ADELANTE": "500", "msAdelante": "533"}),
        "partido: O8 a O13 y O17")
    verificar("O15" in codigos({"MS_ASENTAR_ACOMODO": "150"}) and "O16" in codigos({}, {"peor_verde": 380}) and
              "O5" in codigos({}, {"cruce": 5000}) and "O5" in codigos({"MS_MEDIO_TOPE_LATERAL": "4000"}),
              "partido: O5, O15 y O16")
    verificar("N3" in codigos({"MS_QUIETO_PARA_PATRULLAR": "6000"}), "partido: N3")
    verificar(all(len(x) == 3 for x in reglas_partido(fab, fab)), "partido: reglas_partido da (nivel, texto, nombres)")
    # ---- F. Archivo raro ----
    sin_umbral = _ino_sintetico(sin=("umbralBlanco",))
    rf = leer_partido(sin_umbral)
    verificar(rf["faltan"] == ["umbralBlanco"], "partido: sin umbralBlanco va a faltan")
    try:
        escribir_partido(sin_umbral, rf, {"umbralBlanco": "430"})
        verificar(False, "partido: escribir lo que falta tiene que dar error")
    except NoSeEscribe:
        pass
    nuevo, reales = escribir_partido(sin_umbral, rf, {"kpLateral": "12"})
    verificar(reales == [("kpLateral", "11.5", "12.0")], "partido: con uno que falta, los demás se escriben")
    doble = _ino_sintetico(extra_abajo=("int umbralBlanco = 430;",))
    rd = leer_partido(doble)
    verificar(rd["repetidos"] == ["umbralBlanco"] and "umbralBlanco" not in rd["valores"],
              "partido: una línea repetida va a repetidos")
    try:
        escribir_partido(doble, rd, {"umbralBlanco": "430"})
        verificar(False, "partido: un repetido no se escribe")
    except NoSeEscribe:
        pass
    raro = _ino_sintetico(sin=("umbralBlanco", "msAdelante"),
                          extra_arriba=("float umbralBlanco = 425;", "int msAdelante = 500 + 33;"))
    rr2 = leer_partido(raro)
    verificar(sorted(n for n, _ in rr2["raros"]) == ["msAdelante", "umbralBlanco"], "partido: float y 500 + 33 son raros")
    llaves = _ino_sintetico(extra_abajo=("void otra() {", "  int a = 1;"))
    rl = leer_partido(llaves)
    verificar(rl["problemas"], "partido: llaves desparejas")
    try:
        escribir_partido(llaves, rl, {"kpLateral": "12"})
        verificar(False, "partido: con llaves desparejas no se escribe nada")
    except NoSeEscribe:
        pass
    verificar(leer_partido(_ino_sintetico(extra_abajo=("}",)))["problemas"], "partido: una } de más")
    raros = os.path.join(carpeta, "raro")
    os.makedirs(raros, exist_ok=True)
    feo = os.path.join(raros, NOMBRE_PARTIDO_INO)
    with open(feo, "wb") as fw:
        fw.write(sint.encode("utf-8")[:40] + b"\xff\xfe" + sint.encode("utf-8")[40:])
    with open(feo, "rb") as fr:
        bytes_feos = fr.read()
    try:
        leer_partido_archivo(feo)
        verificar(False, "partido: un archivo que no es UTF-8 tiene que dar error")
    except NoSeEscribe as e:
        verificar("UTF-8" in str(e), "partido: dice que no es UTF-8")
    try:
        guardar_partido(feo, copias, {"kpLateral": "12"}, huella_de(bytes_feos))
        verificar(False, "partido: un archivo que no es UTF-8 no se escribe")
    except NoSeEscribe:
        pass
    with open(feo, "rb") as fr:
        verificar(fr.read() == bytes_feos and not _archivos_de(copias), "partido: el no-UTF-8 quedó igual")
    # ---- G. Copias ----
    if hay_real:
        lista_antes = _archivos_de(PARTIDO_CARPETA)
        lec = leer_partido_archivo(real)
        ahora1 = time.strptime("2026-11-08 15:42:10", "%Y-%m-%d %H:%M:%S")
        info = guardar_partido(real, copias, {"umbralBlanco": "430"}, lec["huella"], ahora=ahora1,
                               origenes={"umbralBlanco": "CANCHA"})
        hechas = [a for a in _archivos_de(copias) if a != "registro.txt"]
        with open(info["copia"], "rb") as fr:
            verificar(fr.read() == datos_real, "partido: la copia tiene los bytes de ANTES")
        verificar(hechas == ["partido-0921_2026-11-08_15-42-10.txt"] and RE_NOMBRE_COPIA.match(hechas[0])
                  and not _adentro_de(copias, PARTIDO_CARPETA), "partido: UNA copia .txt, afuera del programa (%r)" % hechas)
        with open(os.path.join(copias, "registro.txt"), encoding="utf-8") as fr:
            verificar("GUARDADO umbralBlanco 425 -> 430 (de CANCHA)" in fr.read(), "partido: el registro")
        verificar(_archivos_de(PARTIDO_CARPETA) == lista_antes, "partido: la carpeta del programa no tiene nada de más")
        verificar(not [a for a in _archivos_de(copias) if os.path.splitext(a)[1].lower() in EXTENSIONES_QUE_COMPILAN],
                  "partido: en copias-partido no hay nada que se compile")
        lec2 = leer_partido_archivo(real)
        verificar(lec2["valores"]["umbralBlanco"] == "430" and lec2["huella"] == info["huella_nueva"]
                  and (lec2["sello"] is None or re.match(r"^[0-9]{8}-[0-9]{6}$", lec2["sello"])),
                  "partido: quedó 430 (y el sello con la hora)")
        cambiadas = [i for i, (a, b) in enumerate(zip(datos_real.decode("utf-8").split("\n"),
                                                     lec2["texto"].split("\n"))) if a != b]
        esperadas = {lec["donde"]["umbralBlanco"][0]} | ({lec["donde_sello"][0]} if lec["donde_sello"] else set())
        verificar(set(cambiadas) == esperadas, "partido: en el de verdad cambiaron solo esas líneas")
        info2 = guardar_partido(real, copias, {"msAdelante": "540"}, lec2["huella"], ahora=ahora1)
        verificar(info2["copia"].endswith("partido-0921_2026-11-08_15-42-10_2.txt"), "partido: la segunda es _2")
        # una copia más nueva con los MISMOS valores del archivo: se saltea
        with open(real, "rb") as fr:
            actual = fr.read()
        with open(os.path.join(copias, "partido-0921_2026-11-08_15-50-00.txt"), "wb") as fw:
            fw.write(actual)
        verificar(copias_del_partido(copias)[0].endswith("15-50-00.txt") and
                  copias_del_partido(copias)[1].endswith("15-42-10_2.txt"), "partido: las copias, de la más nueva")
        elegida = elegir_copia_para_volver(copias, leer_partido_archivo(real)["valores"])
        verificar(elegida is not None and elegida[0].endswith("15-42-10_2.txt")
                  and elegida[1] == [("msAdelante", "540", "533")], "partido: volver elige la más nueva distinta (%r)" % (
                      elegida,))
        with open(real, "rb") as fr:
            verificar(fr.read() == actual, "partido: elegir la copia no escribe nada")
        # ---- H. Por afuera y fallas ----
        lec3 = leer_partido_archivo(real)
        de_afuera = actual.replace(b"#include <Arduino.h>", b"#include <Arduino.h>  ", 1)
        with open(real, "wb") as fw:
            fw.write(de_afuera)
        antes_copias = _archivos_de(copias)
        try:
            guardar_partido(real, copias, {"kpLateral": "12"}, lec3["huella"])
            verificar(False, "partido: con la huella vieja tiene que dar CambioPorAfuera")
        except CambioPorAfuera:
            pass
        with open(real, "rb") as fr:
            verificar(fr.read() == de_afuera and _archivos_de(copias) == antes_copias,
                      "partido: cambió por afuera: no escribe ni copia")
        lec4 = leer_partido_archivo(real)
        with mock.patch.object(os, "replace", side_effect=PermissionError("ocupado")):
            try:
                guardar_partido(real, copias, {"kpLateral": "12"}, lec4["huella"])
                verificar(False, "partido: con os.replace ocupado tiene que dar error")
            except NoSeEscribe as e:
                verificar("otro programa" in str(e), "partido: dice 'otro programa'")
        with open(real, "rb") as fr:
            verificar(fr.read() == de_afuera and _archivos_de(PARTIDO_CARPETA) == lista_antes,
                      "partido: con os.replace ocupado el original queda entero y sin .tmp")
    de_verdad = next(iter(RUTAS_QUE_NO_SE_ESCRIBEN), None)
    if de_verdad:
        try:
            guardar_partido(de_verdad, copias, {"kpLateral": "12"}, "huella-que-no-es")
            verificar(False, "partido: la ruta DE VERDAD no se escribe")
        except NoSeEscribe:
            pass                # (si hubiera abierto el archivo, sería CambioPorAfuera)
        try:
            escribir_bytes_atomico(de_verdad, b"x")
            verificar(False, "partido: la ruta DE VERDAD no se escribe (bytes)")
        except NoSeEscribe:
            pass
    if hay_real:
        with open(real, "wb") as fw:          # la COPIA de la temporal vuelve a como era (para las que siguen)
            fw.write(datos_real)


def _autoprueba_linea_version(verificar):
    """I del DISENO 16: la #PARTIDO (la manda el programa de PARTIDO, no tablero.ino)."""
    m = interpretar(LINEA_PARTIDO_FABRICA)
    verificar(m["tipo"] == "PARTIDO" and m["version"] == "0921g" and m["sello"] == "fabrica" and m["giro"] is True,
              "versión: la línea de fábrica se entiende (%r)" % m.get("motivo"))
    verificar(all(mismo_valor(n, m["valores"][n], VALORES_DE_FABRICA[n]) for n in NUMEROS_LINEA_PARTIDO),
              "versión: los números son los de fábrica")
    verificar(m["interruptores"] == [VALORES_DE_FABRICA[n] == "true" for n in ORDEN_INTERRUPTORES],
              "versión: los 13 interruptores son los de fábrica")
    verificar(linea_partido_de(VALORES_DE_FABRICA) == LINEA_PARTIDO_FABRICA,
              "versión: la línea de fábrica es la que imprime avisarVersion()")
    rotas = (LINEA_PARTIDO_FABRICA.replace("version=0921g ", ""),
             LINEA_PARTIDO_FABRICA.replace("version=0921g sello=fabrica", "sello=fabrica version=0921g"),
             LINEA_PARTIDO_FABRICA.replace(" interruptores=1111111110101", "") + " interruptores=1111111110101 x=1",
             LINEA_PARTIDO_FABRICA.replace("1111111110101", "111111111010"),
             LINEA_PARTIDO_FABRICA.replace("1111111110101", "1111111112101"),
             LINEA_PARTIDO_FABRICA.replace("umbralBlanco=425", "umbralBlanco=abc"),
             LINEA_PARTIDO_FABRICA.replace("umbralBlanco=425", "umbralBlanco=٤٢٥"),
             LINEA_PARTIDO_FABRICA.replace("giro=1", "giro=2"),
             "#PARTIDO", "#PARTIDO version=0921g",
             LINEA_PARTIDO_FABRICA[:60] + S_BUENA, LINEA_PARTIDO_FABRICA + S_BUENA)
    for r in rotas:
        verificar(interpretar(r)["tipo"] == "ROTA", "versión: ROTA %r" % r[:70])
    otra = interpretar(LINEA_PARTIDO_FABRICA.replace(" interruptores=", " campoNuevo=7 interruptores="))
    verificar(otra["tipo"] == "PARTIDO", "versión: una clave de más se acepta (para el futuro)")
    verificar(interpretar(HOLA_BUENO)["tipo"] == "HOLA", "versión: el #HOLA sigue igual")
    t = interpretar("=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===")
    verificar(t["tipo"] == "TEXTO" and reconocer_programa(t["texto"]).startswith("PARTIDO 0.1103"),
              "versión: la 0.1103 sigue siendo TEXTO")
    lec = leer_partido(_ino_sintetico())
    verificar(comparar_con_robot(m, lec) == ("IGUAL", []), "versión: fábrica contra fábrica (11.50 contra 11.5)")
    lec430 = leer_partido(escribir_partido(lec["texto"], lec, {"umbralBlanco": "430"})[0])
    verificar(comparar_con_robot(m, lec430) == ("DISTINTO", [("umbralBlanco", 425, 430)]), "versión: 425 contra 430")
    otro_sello = leer_partido(lec["texto"].replace('SELLO = "fabrica"', 'SELLO = "20261108-154210"'))
    verificar(comparar_con_robot(m, otro_sello)[0] == "DISTINTO", "versión: sello distinto")
    verificar(comparar_con_robot(interpretar(LINEA_PARTIDO_FABRICA.replace("0921g", "0921h")), lec)[0] == "OTRA_VERSION",
              "versión: otra versión")
    verificar(comparar_con_robot(interpretar(LINEA_PARTIDO_FABRICA.replace("interruptores=1", "interruptores=0")),
                                 lec)[1] == [("volverAlMedioEnElLateral", False, True)], "versión: un interruptor distinto")
    ino_tablero = os.path.join(CARPETA, "tablero.ino")
    if os.path.exists(ino_tablero):
        with open(ino_tablero, encoding="utf-8", errors="replace") as f:      # solo LEERLO
            verificar("#PARTIDO" not in f.read(), "versión: tablero.ino NO imprime #PARTIDO")
    real = os.path.join(PARTIDO_CARPETA, NOMBRE_PARTIDO_INO)
    if not os.path.exists(real):
        return
    with open(real, encoding="utf-8") as f:
        texto = f.read()
    if 'VERSION_PARTIDO = "' not in texto:
        print("    (el programa de partido todavía es la 0921f: salteo lo de avisarVersion)")
        return
    codigo = _sin_comentarios(texto)
    cuerpo = codigo.split("void avisarVersion() {", 1)[1].split("\n}\n", 1)[0]
    campos = re.findall(r'Serial\.print\("(?:#PARTIDO )? ?([A-Za-z_]+)="\)', cuerpo)
    verificar(campos == list(CAMPOS_PARTIDO), "versión: avisarVersion imprime en el orden de CAMPOS_PARTIDO %r" % campos)
    llaves = re.findall(r"[A-Za-z]+", cuerpo.split("llaves[] = {", 1)[1].split("}", 1)[0])
    verificar(llaves == list(ORDEN_INTERRUPTORES), "versión: llaves[] en el orden de ORDEN_INTERRUPTORES")
    verificar(cuerpo.strip().startswith("if (!Serial) return;") and "delay(" not in cuerpo,
              "versión: avisarVersion empieza con if (!Serial) return; y no espera")
    verificar(not re.search(r"\bSerial\.(begin|read|available)\b", codigo), "versión: no hay Serial.begin/read/available")
    verificar(codigo.count("avisarVersion();") == 2, "versión: avisarVersion() se llama 2 veces")
    lec_real = leer_partido(texto)
    for nombre in NUMEROS_LINEA_PARTIDO:
        dec = DECIMALES_LINEA_PARTIDO.get(nombre)
        if dec:
            verificar(re.search(r"Serial\.print\(%s, %d\);" % (nombre, dec), cuerpo), "versión: %s con %d decimales" % (
                nombre, dec))
    verificar(lec_real["version"] is not None and lec_real["sello"] is not None, "versión: VERSION_PARTIDO y SELLO")


def _autoprueba_carga(verificar):
    """J del DISENO 16: la carga, SIN ejecutarla."""
    c = os.path.join("x", "partido-0921")
    o = orden_compilar(c)
    verificar(o == [PIO, "run", "-e", "teensy41", "-d", c] and not {"-t", "upload", "monitor"} & set(o),
              "carga: compilar sin -t")
    o = orden_cargar(c)
    verificar(o == [PY, CARGAR_PY, os.path.abspath(c)] and "--mirar" not in o, "carga: cargar.py sin --mirar")
    verificar(os.path.exists(CARGAR_PY), "carga: cargar.py existe")
    with open(CARGAR_PY, encoding="utf-8", errors="replace") as f:       # solo LEERLO
        texto = f.read()
    verificar(SERIE_ARQUERO in texto and SERIE_DELANTERO in texto and "upload" in texto,
              "carga: cargar.py tiene sus controles (arquero, delantero, upload)")
    env = entorno_carga()
    verificar(env["PYTHONIOENCODING"] == "utf-8" and env["PYTHONUTF8"] == "1" and env["PYTHONUNBUFFERED"] == "1",
              "carga: el entorno")
    op = opciones_de_proceso("aca")
    verificar(op["stdin"] == subprocess.DEVNULL and op["stdout"] == subprocess.PIPE and op["stderr"] == subprocess.STDOUT
              and op["creationflags"] == 0x08000000 and op["cwd"] == "aca", "carga: las opciones del proceso")
    # (revisión 08/11) "sigue con lo que tenía" SOLO si se ve que no empezó a grabar;
    # si no (o no se sabe), PUEDE_A_MEDIAS y cómo salir (el Arduino IDE).
    fallo = ["FALLO. Si es un error de compilacion, no se cargo nada.",
             "Si dice 'Waiting for Teensy device': desenchufa y enchufa el USB y"]
    filas = ((["NO HAY NINGUN TEENSY ENCHUFADO. Fijate el cable USB."], 2, "No veo ningún robot", False),
             (["HAY 2 TEENSY ENCHUFADOS. Deja uno solo:"], 2, "más de un Teensy", False),
             (["ESE ES EL TEENSY DEL DELANTERO (serie 15708680). No cargo nada."], 2, "DELANTERO", False),
             (["El Teensy enchufado tiene serie 99, y el del arquero es 13348550.", "EOFError"], 1, "no es el arquero",
              False),
             (["OTRA VENTANA ESTA USANDO EL ROBOT (AJUSTAR, MIRAR, MONITOR o el"], 2, "Otro programa tiene el puerto",
              False),
             (["Waiting for Teensy device..."] + fallo, 1, "no entró a cargar", True),
             (["NO ENCUENTRO EL PROGRAMA en:"], 2, "No encuentro la carpeta", False),
             (fallo, 1, "Falló (código 1)", True),
             (["cualquier cosa"], 7, "Falló (código 7)", True),
             (["Configuring upload protocol...", "Rebooting...", "Uploading .pio/build/teensy41/firmware.hex",
               "Found HalfKay Bootloader", "Programming.......", "error writing to Teensy",
               "*** [upload] Error 1"] + fallo, 1, "Falló (código 1)", True),
             (["Compiling .pio/build/teensy41/src/partido-0921.ino.cpp.o",
               "partido-0921.ino:12:3: error: 'algo' was not declared in this scope",
               "*** [.pio/build/teensy41/src/partido-0921.ino.cpp.o] Error 1"] + fallo, 1, "No compiló", False))
    for lineas, codigo, espera, a_medias in filas:
        ok, motivo, nivel = motivo_de_carga(lineas, codigo)
        verificar(not ok and espera in motivo and nivel in ("malo", "ojo"), "carga: motivo %r -> %r" % (espera, motivo))
        verificar((PUEDE_A_MEDIAS in motivo) == a_medias and (SIGUE_CON_LO_QUE_TENIA in motivo) == (not a_medias)
                  and (("Arduino IDE" in motivo) == a_medias), "carga: a medias o sigue con lo que tenía (%s)" % espera)
    verificar(motivo_de_carga(["SUCCESS"], 0) == (True, "SALIÓ BIEN", "bien"), "carga: código 0")
    verificar("4 min" in motivo_de_carga([], 0, tope=True)[1] and PUEDE_A_MEDIAS in motivo_de_carga([], 0, tope=True)[1]
              and SIGUE_CON_LO_QUE_TENIA not in motivo_de_carga([], 0, tope=True)[1], "carga: el tope (a medias)")
    salida = ["=" * 60, " Cargando partido-0921 al arquero (COM11)", "--- lo que dijo el robot al arrancar ---",
              "o=10000 interruptores=1111111110101", LINEA_PARTIDO_FABRICA, LINEA_PARTIDO_FABRICA,
              "-" * 40]
    p = lineas_partido_de_la_salida(salida)
    verificar(len(p) == 2 and p[0]["version"] == "0921g", "carga: las #PARTIDO de la salida de cargar.py")
    verificar(lineas_partido_de_la_salida(["--- lo que dijo el robot al arrancar ---",
                                           "(nada: el programa de partido solo lo dice una vez y a veces no llego)"]) == [],
              "carga: sin #PARTIDO")


class _ProcesoDeMentira:
    def __init__(self, renglones, codigo):
        import io
        self.stdout = io.BytesIO("".join(r + "\r\n" for r in renglones).encode("utf-8"))
        self.codigo = codigo
        self.pid = 4242

    def wait(self):
        return self.codigo

    def kill(self):
        pass


def _autoprueba_pestana_partido(verificar, carpeta):
    """K del DISENO 16: la pestaña PARTIDO con un tkinter DE MENTIRA, un
    Popen de mentira (anota cómo estaba el cable en cada llamada) y el hilo
    corriendo en el momento (correr_en_hilo = lambda f: f())."""
    from unittest import mock
    import contextlib
    import io
    import types
    g = globals()
    guardados = {k: g[k] for k in ("tk", "ttk", "messagebox", "filedialog")}
    real = os.path.join(PARTIDO_CARPETA, NOMBRE_PARTIDO_INO)
    if not os.path.exists(real):
        print("    (no está el programa de partido: salteo la pestaña)")
        return
    prog = os.path.join(carpeta, "k-partido")
    os.makedirs(prog, exist_ok=True)
    ruta = os.path.join(prog, NOMBRE_PARTIDO_INO)
    with open(real, "rb") as f, open(ruta, "wb") as fw:
        fw.write(f.read())
    with open(os.path.join(prog, "platformio.ini"), "w") as fw:
        fw.write("; de mentira\n")
    g_copias = g["COPIAS_PARTIDO"]
    g["COPIAS_PARTIDO"] = os.path.join(carpeta, "k-copias")

    def bytes_de(r=ruta):
        with open(r, "rb") as f:
            return f.read()
    try:
        g["tk"] = mock.MagicMock(name="tk")
        g["ttk"] = mock.MagicMock(name="ttk")
        g["messagebox"] = mock.MagicMock(name="messagebox")
        g["filedialog"] = mock.MagicMock(name="filedialog")
        caja = g["messagebox"]
        caja.askyesno.return_value = True
        raiz = mock.MagicMock(name="raiz")
        cola = queue.Queue()
        cables = [_CableDePrueba()]
        llamadas = []
        respuestas = {"compilar": [], "cargar": []}

        def ejecutar(orden, **kw):
            que = "cargar" if CARGAR_PY in orden else "compilar"
            c = cables[-1]
            llamadas.append((que, c.cerrado, list(c.mandados), kw))
            r = respuestas[que].pop(0) if respuestas[que] else None
            if callable(r):
                r = r()
            codigo, renglones = r if r is not None else (0, ["[SUCCESS]"])
            return _ProcesoDeMentira(renglones, codigo)

        def fabrica():
            c = _CableDePrueba()
            cables.append(c)
            return c

        def salida_buena():
            lec = leer_partido_archivo(ruta)
            return 0, ["=" * 60, " Cargando partido-0921 al arquero (COM11)",
                       " 🚨 Si es el programa de partido, el robot va a arrancar solo.",
                       "--- lo que dijo el robot al arrancar ---",
                       linea_partido_de(lec["valores"], lec["version"] or "0921g", lec["sello"] or "fabrica"),
                       "-" * 40]
        puertos = [types.SimpleNamespace(device="COM11", vid=VID_TEENSY, pid=PID_SERIE, serial_number=SERIE_ARQUERO)]
        en_carga = []
        v = Ventana(raiz, cables[0], cola, estricto=True, carpeta_canchas=os.path.join(carpeta, "k-canchas"),
                    partido_carpeta=prog, ejecutar=ejecutar, correr_en_hilo=lambda f: f(), fabrica_conexion=fabrica,
                    listar_puertos=lambda: list(puertos), mirar_carga=lambda: list(en_carga), dormir=lambda s: None)

        def girar():
            lote = []
            while not cola.empty():
                lote.append(cola.get())
            v._procesar_lote(lote)

        def llegan(*renglones):
            v._procesar_lote([("linea", r) for r in renglones])

        # ---- 37: las filas, plegar, [+], guardar, descartar ----
        verificar(set(v.filas_partido) == set(ORDEN_PARTIDO) and v.lectura is not None, "pestaña: están todas las filas")
        verificar(v.grupo_plegado["avanzado"] and not any(v.grupo_plegado[k] for k, _, _ in GRUPOS_PARTIDO[:-1]),
                  "pestaña: AVANZADO arranca plegado")
        verificar(v.pestana_elegida == "movimientos", "pestaña: sin --pestana arranca en MOVIMIENTOS")
        v._plegar_grupo("avanzado")
        verificar(not v.grupo_plegado["avanzado"], "pestaña: se despliega")
        v._plegar_grupo("avanzado")
        antes = bytes_de()
        v._partido_paso("umbralBlanco", +1)
        verificar(v.pendientes_partido == {"umbralBlanco": "430"} and bytes_de() == antes,
                  "pestaña: [+] en umbralBlanco: 430 en el editor y el archivo igual (%r %r)" % (
                      v.pendientes_partido, v.ultimo_aviso))
        v._pintar_partido()
        verificar(v._cambios_reales() == [("umbralBlanco", "425", "430")], "pestaña: 1 cambio sin guardar")
        v._partido_paso("umbralBlanco", -1)
        verificar(v.pendientes_partido == {}, "pestaña: volver al valor del archivo lo saca del editor")
        v.filas_partido["kpLateral"]["entrada"].get.return_value = "12,5"
        v._partido_poner("kpLateral")
        verificar(v.pendientes_partido == {"kpLateral": "12.5"}, "pestaña: la cajita con coma")
        v.filas_partido["kpLateral"]["entrada"].get.return_value = "٤٢٥"
        v._partido_poner("kpLateral")
        verificar(v.pendientes_partido == {"kpLateral": "12.5"} and v.ultimo_aviso[1] == ROJO,
                  "pestaña: la cajita con dígitos raros no entra")
        v._partido_descartar()
        verificar(v.pendientes_partido == {} and bytes_de() == antes, "pestaña: descartar vacía el editor")
        v._partido_paso("umbralBlanco", +1)
        verificar(v._partido_guardar() and leer_partido_archivo(ruta)["valores"]["umbralBlanco"] == "430"
                  and len(copias_del_partido(COPIAS_PARTIDO)) == 1 and v.pendientes_partido == {}
                  and v.estado_archivo == "guardado", "pestaña: GUARDAR cambia el archivo y hace la copia")
        verificar(v._partido_guardar() is False and len(copias_del_partido(COPIAS_PARTIDO)) == 1,
                  "pestaña: guardar sin cambios no hace nada")
        # los interruptores y las ventanitas
        caja.askyesno.return_value = False
        v._partido_elegir("despejeEnDiagonal", "true")
        verificar("despejeEnDiagonal" not in v.pendientes_partido, "pestaña: la diagonal pide confirmar (NO)")
        caja.askyesno.return_value = True
        v._partido_elegir("despejeEnDiagonal", "true")
        v._partido_elegir("perseguirEnElDespeje", "false")
        verificar(v.pendientes_partido == {"despejeEnDiagonal": "true", "perseguirEnElDespeje": "false"}
                  and v._fila_no_aplica("kpPersecucion", v._valores_partido()), "pestaña: SÍ/NO y la fila gris")
        v._partido_elegir("LADO_MEDIO", "1")
        verificar(v.pendientes_partido.get("LADO_MEDIO") == "1" and
                  any(c == "O9" for _, _, _, c in v._reglas_de_ahora()), "pestaña: LADO_MEDIO y su aviso")
        v._partido_paso("VEL_MEDIO_MEDIR", +1)
        verificar("VEL_MEDIO_MEDIR" not in v.pendientes_partido, "pestaña: VEL_MEDIO_MEDIR es solo para ver")
        v._partido_descartar()
        # un ERROR no deja guardar ni cargar
        v._partido_poner_valor("pwmMaxLateral", "80")
        v._partido_poner_valor("pwmMinLateral", "110")
        verificar(any(c == "E1" for _, _, _, c in v._reglas_de_ahora()) and not v._partido_guardar()
                  and "ERROR" in v._puede_cargar("partido")[1], "pestaña: un ERROR no deja guardar ni cargar")
        verificar(bytes_de() == leer_partido_archivo(ruta)["texto"].encode("utf-8") and
                  leer_partido_archivo(ruta)["valores"]["pwmMinLateral"] == "70", "pestaña: con ERROR no escribió")
        v._partido_descartar()
        # ---- 22 y 26: volver a como estaba / a los de fábrica (solo el editor) ----
        antes = bytes_de()
        verificar(v._partido_volver_copia() and v.pendientes_partido == {"umbralBlanco": "425"} and bytes_de() == antes,
                  "pestaña: volver a como estaba pone 425 en el editor sin escribir")
        v._partido_descartar()
        verificar(v._partido_volver_fabrica() and all(mismo_valor(n, v._valores_partido()[n], VALORES_DE_FABRICA[n])
                                                       for n in ORDEN_PARTIDO) and bytes_de() == antes,
                  "pestaña: volver a fábrica deja el editor igual a VALORES_DE_FABRICA")
        v._partido_descartar()
        # (revisión 08/11) con un cambio de prueba en el editor, 'Volver' lo DESCARTA (y la ventanita lo dice)
        v._partido_poner_valor("kpLateral", "12.5")
        caja.askyesno.reset_mock()
        verificar(v._partido_volver_fabrica() and all(mismo_valor(n, v._valores_partido()[n], VALORES_DE_FABRICA[n])
                                                       for n in ORDEN_PARTIDO)
                  and "kpLateral" not in v.pendientes_partido and "DESCARTAN" in caja.askyesno.call_args[0][1]
                  and "recién CARGAR EL PARTIDO los lleva al robot" in caja.askyesno.call_args[0][1]
                  and bytes_de() == antes, "pestaña: volver a fábrica con un cambio en el editor: queda EXACTO fábrica")
        v._partido_descartar()
        v._partido_poner_valor("pwmMaxLateral", "120")
        verificar(v._partido_volver_copia() and v.pendientes_partido == {"umbralBlanco": "425"} and bytes_de() == antes,
                  "pestaña: volver a como estaba también descarta el editor (%r)" % (v.pendientes_partido,))
        v._partido_descartar()
        v._partido_poner_valor("kpLateral", "12.5")
        verificar(v._poner_varios([], "fábrica", "Volver a los de fábrica", "x") and v.pendientes_partido == {}
                  and v.origen_partido == {}, "pestaña: con el archivo ya así, 'Volver' descarta el editor")
        verificar(not v._poner_varios([], "fábrica", "Volver a los de fábrica", "x")
                  and "Ya está todo así" in v.ultimo_aviso[0], "pestaña: sin nada que volver: 'Ya está todo así'")
        # ---- por afuera ----
        lec = leer_partido_archivo(ruta)
        n, ini, fin = lec["donde"]["kpLateral"]
        lineas = lec["texto"].split("\n")
        lineas[n] = lineas[n][:ini] + "11.0" + lineas[n][fin:]
        with open(ruta, "wb") as fw:
            fw.write("\n".join(lineas).encode("utf-8"))
        st = os.stat(ruta)
        os.utime(ruta, ns=(st.st_atime_ns, st.st_mtime_ns + 2 * 10 ** 9))
        v.t_mirar_partido = 0.0
        v._latido()
        verificar(v.lectura["valores"]["kpLateral"] == "11.0" and "por afuera" in (v.aviso_afuera or ""),
                  "pestaña: cambió por afuera: lo vuelve a leer")
        v._partido_paso("umbralBlanco", -1)
        lineas[n] = lineas[n][:ini] + "11.5" + lineas[n][fin:]
        with open(ruta, "wb") as fw:
            fw.write("\n".join(lineas).encode("utf-8"))
        os.utime(ruta, ns=(st.st_atime_ns, st.st_mtime_ns + 4 * 10 ** 9))
        verificar(not v._partido_guardar() and "mientras editabas" in v.ultimo_aviso[0]
                  and v.pendientes_partido == {"umbralBlanco": "425"} and leer_partido_archivo(ruta)["valores"][
                      "umbralBlanco"] == "430", "pestaña: guardar con la huella vieja no escribe y avisa")
        v._partido_descartar()
        # ---- 38: CARGAR ----
        verificar(not v._cargar_partido() and llamadas == [] and "Enchufá" in v.ultimo_aviso[0],
                  "pestaña: CARGAR sin conexión no lanza nada")
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan(S_BUENA, HOLA_BUENO)
        verificar(v._habla_tablero() and v._puede_cargar("partido")[0], "pestaña: con el tablero conectado se puede")
        caja.askyesno.return_value = False
        v._partido_paso("umbralBlanco", +1)
        verificar(not v._cargar_partido() and llamadas == [], "pestaña: CARGAR con NO no hace nada")
        caja.askyesno.return_value = True
        del cables[0].mandados[:]
        respuestas["cargar"].append(salida_buena)
        verificar(v._cargar_partido() and v.cargando, "pestaña: CARGAR arranca")
        girar()
        verificar([x[0] for x in llamadas] == ["compilar", "cargar"], "pestaña: compila y después carga %r" % (
            [x[0] for x in llamadas],))
        verificar(llamadas[0][1] is False and llamadas[0][2] == ["x"] and llamadas[1][1] is True,
                  "pestaña: compila con el cable SIN cerrar (y la x antes), carga con el cable cerrado")
        verificar(llamadas[0][3]["stdin"] == subprocess.DEVNULL and llamadas[0][3]["cwd"] == prog,
                  "pestaña: las opciones de la orden")
        verificar(len(cables) == 2 and v.conexion is cables[1] and not v.cargando, "pestaña: se vuelve a conectar")
        verificar(leer_partido_archivo(ruta)["valores"]["umbralBlanco"] == "435" and v.pendientes_partido == {},
                  "pestaña: guardó lo pendiente antes de cargar")
        verificar("LISTO" in v.ultimo_aviso[0] and v.ultimo_aviso[1] == VERDE and v.estado_archivo == "cargado",
                  "pestaña: CONFIRMADO con la #PARTIDO (%r)" % (v.ultimo_aviso,))
        verificar(len(copias_del_partido(COPIAS_PARTIDO)) == 2, "pestaña: la carga hizo su copia")
        verificar(v._texto_estado_partido()[0] == "Cargado y confirmado por el robot", "pestaña: el cartel de arriba, LISTO")
        # sin #PARTIDO en la salida: la espera por la conexión nueva
        del llamadas[:]
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        respuestas["cargar"].append((0, ["--- lo que dijo el robot al arrancar ---", "(nada)"]))
        verificar(v._cargar_partido(), "pestaña: CARGAR sin cambios igual carga")
        girar()
        verificar(v.esperando_version_hasta is not None and len(cables) == 3, "pestaña: espera la versión")
        verificar(v.estado_archivo == "cargado_sin_confirmar" and "todavía no dijo" in v._texto_estado_partido()[0]
                  and "lo de antes" not in v._texto_estado_partido()[0],
                  "pestaña: mientras espera, arriba NO dice 'tiene lo de antes' (%r)" % (v._texto_estado_partido(),))
        lec = leer_partido_archivo(ruta)
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan(linea_partido_de(lec["valores"], lec["version"] or "0921g", lec["sello"] or "fabrica"))
        verificar("LISTO" in v.ultimo_aviso[0] and v.esperando_version_hasta is None and cables[-1].mandados == [],
                  "pestaña: la #PARTIDO llegó por la conexión nueva")
        # otra vez (CON un cambio), y no llega a tiempo
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        v._partido_paso("kpLateral", +1)
        respuestas["cargar"].append((0, []))
        v._cargar_partido()
        girar()
        v.esperando_version_hasta -= ESPERA_VERSION + 5
        v._latido()
        verificar("no dijo su versión" in v.ultimo_aviso[0] and v.esperando_version_hasta is None
                  and v.estado_archivo == "cargado_sin_confirmar" and "lo de antes" not in v._texto_estado_partido()[0],
                  "pestaña: pasado el tiempo sin la línea avisa (y arriba no dice 'tiene lo de antes')")
        # (revisión 08/11) llega TARDE (desenchufar y enchufar, como dice el aviso): confirma igual
        lec = leer_partido_archivo(ruta)
        v._procesar_lote([("desconectado", "COM11"), ("conectado", "COM11", SERIE_ARQUERO)])
        llegan(linea_partido_de(lec["valores"], lec["version"] or "0921g", lec["sello"] or "fabrica"))
        verificar("LISTO" in v.ultimo_aviso[0] and v.resultado_carga[1] == VERDE and v.estado_archivo == "cargado"
                  and v._texto_estado_partido()[0] == "Cargado y confirmado por el robot",
                  "pestaña: la #PARTIDO que llega tarde confirma (%r)" % (v.resultado_carga,))
        # una #PARTIDO repetida (parpadeando) no cambia nada; una de OTROS valores, sí
        llegan(linea_partido_de(lec["valores"], lec["version"] or "0921g", lec["sello"] or "fabrica"))
        verificar(v.estado_archivo == "cargado", "pestaña: la repetida sigue en cargado")
        llegan(linea_partido_de(lec["valores"], lec["version"] or "0921g", "20200101-000000"))
        verificar(v.estado_archivo == "no_coincide" and v._texto_estado_partido()[2] == ROJO,
                  "pestaña: una #PARTIDO con otro sello: arriba en rojo")
        llegan(linea_partido_de(lec["valores"], lec["version"] or "0921g", lec["sello"] or "fabrica"))
        # distinto
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        respuestas["cargar"].append((0, ["--- lo que dijo el robot al arrancar ---",
                                         linea_partido_de(dict(lec["valores"], umbralBlanco="425"), lec["version"] or "0921g",
                                                          lec["sello"] or "fabrica")]))
        v._cargar_partido()
        girar()
        verificar("otros valores" in v.ultimo_aviso[0] and v.ultimo_aviso[1] == ROJO and v.estado_archivo == "no_coincide",
                  "pestaña: el robot dice otros valores")
        # ---- 39: no compila / OTRA VENTANA ----
        del llamadas[:]
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        cable = cables[-1]
        v._partido_paso("umbralBlanco", +1)
        antes = bytes_de()
        respuestas["compilar"].append((1, ["Compiling...", "partido-0921.ino:12: error: algo no compila"]))
        v._cargar_partido()
        girar()
        verificar(bytes_de() == antes and [x[0] for x in llamadas] == ["compilar"] and not cable.cerrado
                  and v.conexion is cable and v.pendientes_partido == {"umbralBlanco": "440"}
                  and "NO COMPILÓ" in v.ultimo_aviso[0], "pestaña: no compiló: el archivo vuelve, no carga, el cable sigue")
        verificar(v.estado_archivo == "no_coincide", "pestaña: no compiló: arriba queda lo que se sabía antes")
        del llamadas[:]
        cuantos = len(cables)
        respuestas["cargar"] += [(2, ["OTRA VENTANA ESTA USANDO EL ROBOT (AJUSTAR, MIRAR, MONITOR o el"]), salida_buena]
        v._cargar_partido()
        girar()
        verificar([x[0] for x in llamadas] == ["compilar", "cargar", "cargar"] and len(cables) == cuantos + 1
                  and "LISTO" in v.ultimo_aviso[0], "pestaña: OTRA VENTANA: un solo reintento")
        del llamadas[:]
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        respuestas["cargar"] += [(2, ["OTRA VENTANA ESTA USANDO EL ROBOT"]), (2, ["OTRA VENTANA ESTA USANDO EL ROBOT"])]
        v._cargar_partido()
        girar()
        verificar([x[0] for x in llamadas] == ["compilar", "cargar", "cargar"] and len(cables) == cuantos + 2
                  and "Otro programa tiene el puerto" in v.ultimo_aviso[0], "pestaña: OTRA VENTANA dos veces: avisa y reconecta")
        # el DELANTERO o un modo carga: no se escribe ni se compila nada
        del llamadas[:]
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        v._partido_paso("umbralBlanco", +1)
        antes = bytes_de()
        en_carga.append(r"USB\VID_16C0&PID_0478\5&1234&0&1")
        v._cargar_partido()
        girar()
        verificar(llamadas == [] and bytes_de() == antes and "MODO CARGA" in v.ultimo_aviso[0]
                  and v.pendientes_partido, "pestaña: con un Teensy en modo carga no hace nada")
        del en_carga[:]
        puertos.append(types.SimpleNamespace(device="COM9", vid=VID_TEENSY, pid=PID_SERIE, serial_number=SERIE_DELANTERO))
        v._cargar_partido()
        girar()
        verificar(llamadas == [] and bytes_de() == antes and "DELANTERO" in v.ultimo_aviso[0],
                  "pestaña: con el delantero enchufado no hace nada")
        puertos.pop()
        # ---- el TABLERO ----
        respuestas["cargar"].append((0, ["--- lo que dijo el robot al arrancar ---", "=== TABLERO DEL ARQUERO ==="]))
        caja.askyesno.reset_mock()
        verificar(v._cargar_tablero(), "pestaña: cargar el TABLERO")
        girar()
        verificar("NO se cargan" in caja.askyesno.call_args[0][1] and llamadas[0][3]["cwd"] == CARPETA
                  and bytes_de() == antes and v.pendientes_partido and "TABLERO" in v.ultimo_aviso[0],
                  "pestaña: el TABLERO no guarda nada y avisa lo pendiente")
        v._partido_descartar()
        verificar(v.estado_archivo == "tablero" and "TABLERO" in v._texto_estado_partido()[0]
                  and "confirmado" not in v._texto_estado_partido()[0],
                  "pestaña: después de cargar el TABLERO, arriba NO dice 'cargado y confirmado'")
        # (revisión 08/11) falla grabando: arriba y abajo dicen que puede haber quedado a medias
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        v._partido_paso("umbralBlanco", +1)
        respuestas["cargar"].append((1, ["Rebooting...", "Uploading .pio/build/teensy41/firmware.hex",
                                         "Found HalfKay Bootloader", "Programming.....", "error writing to Teensy",
                                         "*** [upload] Error 1", "FALLO. Si es un error de compilacion, no se cargo nada."]))
        v._cargar_partido()
        girar()
        verificar(PUEDE_A_MEDIAS in v.resultado_carga[0] and "Arduino IDE" in v.resultado_carga[0]
                  and SIGUE_CON_LO_QUE_TENIA not in v.resultado_carga[0] and v.estado_archivo == "a_medias"
                  and "lo de antes" not in v._texto_estado_partido()[0] and v._texto_estado_partido()[2] == ROJO,
                  "pestaña: falla grabando: a medias (%r / %r)" % (v.resultado_carga, v._texto_estado_partido()))
        # vuelve a aparecer con el tablero: arriba lo dice
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan(S_BUENA, HOLA_BUENO)
        verificar(v.estado_archivo == "tablero", "pestaña: el #HOLA del tablero corrige el cartel de arriba")
        # ---- 40: mientras carga ----
        v._procesar_lote([("conectado", "COM11", SERIE_ARQUERO)])
        llegan(S_BUENA, HOLA_BUENO)
        c = cables[-1]
        del c.mandados[:]
        v.cargando = True
        verificar(not v._mandar("h") and c.mandados == [] and not v._puede_cargar("partido")[0]
                  and not v._partido_editable(), "pestaña: mientras carga no manda nada y todo queda gris")
        v._partido_paso("umbralBlanco", +1)
        v._actualizar_habilitados()
        verificar(v.pendientes_partido == {}, "pestaña: mientras carga no se edita")
        caja.showwarning.reset_mock()
        v._cerrar()
        verificar(not v.cerrada and caja.showwarning.called, "pestaña: cerrar durante una carga no deja")
        v.cargando = False
        # ---- 42: una #PARTIDO por el cable ----
        v._procesar_lote([("desconectado", "COM11"), ("conectado", "COM11", SERIE_ARQUERO)])
        v.t_fin_carga = None
        v._procesar_lote([("desconectado", "COM11"), ("conectado", "COM11", SERIE_ARQUERO)])
        c = cables[-1]
        del c.mandados[:]
        llegan(LINEA_PARTIDO_FABRICA, LINEA_PARTIDO_FABRICA)
        verificar(v.programa_detectado == "PARTIDO 0921g" and c.mandados == [] and "PARTIDO" in v._texto_programa()[0]
                  and "Cargar el TABLERO" in v._texto_programa()[0], "pestaña: la #PARTIDO: no le manda nada")
        verificar("NO coincide" in v._texto_robot_partido()[0], "pestaña: EL ROBOT TIENE (fábrica contra 430)")
        llegan("=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===")
        v.ultimo_partido = None
        verificar("NO la de jugar" in v._texto_robot_partido()[0], "pestaña: la 0.1103 en rojo")
        # ---- 43: pasar de CANCHA ----
        v.cancha["lineas"] = [interpretar(LINEAS_DE_MENTIRA[f]) for f in ("v", "i", "d")]
        v.cancha["idas"] = [[400, 30.0], [600, 48.0]]
        filas = {f["nombre"]: f for f in v._medido_para_partido()}
        verificar(filas["umbralBlanco"]["tildado"] and filas["umbralBlanco"]["medido"] == "425"
                  and filas["umbralBlanco"]["hoy"] == leer_partido_archivo(ruta)["valores"]["umbralBlanco"] != "425",
                  "pestaña: pasar: umbralBlanco tildado")
        verificar("msAdelante" in filas and not filas["msAdelante"]["tildado"], "pestaña: pasar: msAdelante SIN tildar")
        verificar("SOLTANDO" in filas["msAdelante"]["aviso"], "pestaña: pasar: msAdelante dice con qué se midió")
        verificar("Se pasa del medio → BAJAR" in FILA_PARTIDO["MS_MEDIO_AL_MEDIO"][10]
                  and "izquierda del medio" not in FILA_PARTIDO["MS_MEDIO_AL_MEDIO"][10],
                  "pestaña: MS_MEDIO_AL_MEDIO sin 'izquierda/derecha' (vale desde los dos laterales)")
        antes = bytes_de()
        pasar = v._dialogo_pasar(list(filas.values()))
        v._ir_a_pestana("cancha")
        verificar(pasar() == 1 and v.pendientes_partido == {"umbralBlanco": "425"} and bytes_de() == antes
                  and v.pestana_elegida == "partido" and v.origen_partido == {"umbralBlanco": "CANCHA"},
                  "pestaña: pasar toca solo el editor y va a PARTIDO")
        v._partido_descartar()
        # ---- 44: --pestana y VOLVER AL PARTIDO ----
        caja.askyesno.return_value = False
        del llamadas[:]
        v._ir_a_pestana("movimientos")
        v._volver_al_partido()
        verificar(v.pestana_elegida == "partido" and llamadas == [] and not cables[-1].cerrado,
                  "pestaña: VOLVER AL PARTIDO va a PARTIDO (y con NO no carga nada)")
        caja.askyesno.return_value = True
        v2 = Ventana(mock.MagicMock(name="raiz-k2"), _CableDePrueba(), queue.Queue(), estricto=True,
                     carpeta_canchas=os.path.join(carpeta, "k-canchas"), partido_carpeta=prog, pestana="partido")
        verificar(v2.pestana_elegida == "partido", "pestaña: con pestana='partido' arranca ahí")
        # al cerrar con cambios sin guardar, pregunta
        v2._partido_paso("umbralBlanco", +1)
        caja.askyesno.return_value = False
        v2._cerrar()
        verificar(not v2.cerrada, "pestaña: cerrar con cambios sin guardar pregunta (NO)")
        caja.askyesno.return_value = True
        v2._cerrar()
        verificar(v2.cerrada, "pestaña: y con SÍ cierra")
        # ---- 41: la demo ----
        v3 = Ventana(mock.MagicMock(name="raiz-k3"), _CableDePrueba(), queue.Queue(), demo=True, estricto=True,
                     carpeta_canchas=os.path.join(carpeta, "k-canchas"), partido_carpeta=prog, ejecutar=ejecutar)
        v3._procesar_lote([("conectado", "DEMO", SERIE_ARQUERO)])
        antes = bytes_de()
        del llamadas[:]
        verificar(not v3._cargar_partido() and llamadas == [] and "demo" in v3.ultimo_aviso[0]
                  and os.path.normcase(v3.partido_ruta) != os.path.normcase(ruta), "demo: CARGAR no lanza nada")
        quiero = str(int(leer_partido_archivo(ruta)["valores"]["umbralBlanco"]) + 5)
        v3._partido_paso("umbralBlanco", +1)
        verificar(v3._partido_guardar() and bytes_de() == antes
                  and leer_partido_archivo(v3.partido_ruta)["valores"]["umbralBlanco"] == quiero
                  and _adentro_de(v3.copias_partido, v3.carpeta_demo), "demo: GUARDAR escribe solo la COPIA de práctica")
        # ---- 45 y 46: los argumentos y PARTIDO.bat ----
        verificar(armar_argumentos().parse_args(["--pestana", "partido"]).pestana == "partido" and
                  armar_argumentos().parse_args([]).pestana == "movimientos", "pestaña: --pestana partido")
        with contextlib.redirect_stderr(io.StringIO()):
            try:
                armar_argumentos().parse_args(["--pestana", "cualquiera"])
                verificar(False, "pestaña: --pestana inválida tiene que dar error")
            except SystemExit:
                pass
        bat = os.path.normpath(os.path.join(CARPETA, "..", "..", "PARTIDO.bat"))
        verificar(os.path.exists(bat), "pestaña: está PARTIDO.bat")
        with open(bat, "rb") as f:                     # solo LEERLO
            datos = f.read()
        verificar(all(b < 128 for b in datos) and b"--pestana partido" in datos and b"cargar.py" not in datos
                  and b"upload" not in datos, "pestaña: PARTIDO.bat es ASCII, abre PARTIDO y no carga nada")
        # ---- 47: nada de lo que se mostró pasa de U+FFFF ----
        def textos(x):
            if isinstance(x, str):
                yield x
            elif isinstance(x, (list, tuple)):
                for y in x:
                    yield from textos(y)
            elif isinstance(x, dict):
                for y in x.values():
                    yield from textos(y)
        llamados = [c for nombre in ("tk", "ttk", "messagebox") for c in g[nombre].mock_calls] + list(raiz.mock_calls)
        raros = {t for c in llamados for t in textos(list(c[1]) + list(c[2].values())) if any(ord(ch) > 0xFFFF for ch in t)}
        verificar(len(llamados) > 1000 and not raros, "pestaña: caracteres que Tk no muestra: %r" % list(raros)[:3])
    finally:
        g.update(guardados)
        g["COPIAS_PARTIDO"] = g_copias


def autoprueba():
    try:
        sys.stdout.reconfigure(errors="replace")
    except Exception:
        pass
    print("AUTOPRUEBA del tablero del ARQUERO: no abre la ventana ni el USB.")
    cuantas = [0]

    def verificar(condicion, que):
        cuantas[0] += 1
        if not condicion:
            raise AssertionError(que)

    # Nada se escribe fuera de una carpeta temporal: la hoja de cancha y el
    # tablero-error.txt de las pruebas van ahí.
    g = globals()
    carpeta = tempfile.mkdtemp(prefix="tablero-arquero-")
    guardados = {k: g[k] for k in ("ARCHIVO_FALLAS", "CANCHAS", "PARTIDO_CARPETA", "COPIAS_PARTIDO",
                                   "RUTAS_QUE_NO_SE_ESCRIBEN", "CARPETA_DEMO", "SIN_PROCESOS", "ESPERA_REINTENTO")}
    # (2026-11-08) Cinturón y tiradores: el programa de partido DE VERDAD solo
    # se LEE. Se anota su huella y lo que hay en su carpeta, y al final tienen
    # que dar igual. Las pruebas trabajan sobre una COPIA en la temporal.
    real_carpeta = PARTIDO_CARPETA
    real_ino = os.path.join(real_carpeta, NOMBRE_PARTIDO_INO)

    def foto():
        try:
            with open(real_ino, "rb") as f:
                huella = huella_de(f.read())
        except OSError:
            huella = None
        return huella, _archivos_de(real_carpeta)
    foto_antes = foto()
    copia = os.path.join(carpeta, "partido-0921")
    os.makedirs(copia, exist_ok=True)
    for nombre in (NOMBRE_PARTIDO_INO, "platformio.ini"):
        if os.path.exists(os.path.join(real_carpeta, nombre)):
            with open(os.path.join(real_carpeta, nombre), "rb") as f, open(os.path.join(copia, nombre), "wb") as fw:
                fw.write(f.read())
    g["ARCHIVO_FALLAS"] = os.path.join(carpeta, "tablero-error.txt")
    g["CANCHAS"] = os.path.join(carpeta, "canchas")
    g["PARTIDO_CARPETA"] = copia
    g["COPIAS_PARTIDO"] = os.path.join(carpeta, "copias-partido")
    g["RUTAS_QUE_NO_SE_ESCRIBEN"] = {os.path.normcase(os.path.abspath(real_ino))}
    g["CARPETA_DEMO"] = carpeta
    g["SIN_PROCESOS"] = True
    g["ESPERA_REINTENTO"] = 0.0
    partes = (("el traductor", _autoprueba_traductor), ("la memoria", _autoprueba_memoria),
              ("las cuentas", _autoprueba_cuentas), ("el robot de mentira", _autoprueba_robot_de_mentira),
              ("el cable (con un USB de mentira)", _autoprueba_cable),
              ("la ventana (con un tkinter de mentira)", lambda ver: _autoprueba_ventana(ver, carpeta)),
              ("el programa de partido", lambda ver: _autoprueba_partido(ver, carpeta)),
              ("la línea de versión", _autoprueba_linea_version),
              ("la carga (sin ejecutarla)", _autoprueba_carga),
              ("la pestaña PARTIDO (tkinter de mentira)", lambda ver: _autoprueba_pestana_partido(ver, carpeta)))
    try:
        for nombre, prueba in partes:
            antes = cuantas[0]
            try:
                prueba(verificar)
            except Exception:
                print("  FALLA en %s:" % nombre)
                traceback.print_exc(file=sys.stdout)
                return 1
            print("  %-42s %5d verificaciones bien" % (nombre, cuantas[0] - antes))
    finally:
        g.update(guardados)
        try:
            import shutil
            shutil.rmtree(carpeta, ignore_errors=True)
        except Exception:
            pass
    despues = foto()
    cuantas[0] += 2
    if despues != foto_antes:
        print("  FALLA: el programa de partido DE VERDAD cambió durante la autoprueba: %r -> %r" % (foto_antes, despues))
        return 1
    print("  %-42s %5d verificaciones bien" % ("el partido de verdad, intacto", 2))
    print("OK (%d verificaciones)" % cuantas[0])
    return 0


# =====================================================================

PESTANAS = ("pelota", "sensores", "movimientos", "cancha", "partido", "consola")


def armar_argumentos():
    """Los argumentos (aparte, para probarlos sin abrir la ventana)."""
    ap = argparse.ArgumentParser(description="Tablero del ARQUERO: una ventana para mirar y probar el robot.")
    ap.add_argument("--demo", action="store_true", help="sin robot: un robot de mentira")
    ap.add_argument("--autoprueba", action="store_true", help="se prueba solo, sin abrir la ventana ni el USB")
    ap.add_argument("--quien", action="store_true", help="qué Teensy hay enchufada, sin abrir ningún puerto")
    ap.add_argument("--puerto", default=None, help="elegir el COM (ej: COM7). Solo se abre si ES el arquero")
    ap.add_argument("--pestana", choices=PESTANAS, default="movimientos",
                    help="en qué pestaña arranca (PARTIDO.bat: partido)")
    return ap


def main():
    a = armar_argumentos().parse_args()
    if a.autoprueba:
        return autoprueba()
    if a.quien:
        return main_quien()
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
        ventana = Ventana(raiz, cable, cola, demo=a.demo, pestana=a.pestana, puerto_forzado=a.puerto)
    except Exception:
        texto = traceback.format_exc()
        guardar_falla(texto)
        print(texto)
        try:
            messagebox.showerror("Tablero del arquero", "No pude abrir la ventana:\n\n" + texto[-1500:])
        except Exception:
            pass
        return 1
    cable.arrancar()
    raiz.mainloop()
    cable.cerrar()
    if ventana.conexion is not cable:       # (se volvió a conectar después de una carga)
        try:
            ventana.conexion.cerrar()
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
