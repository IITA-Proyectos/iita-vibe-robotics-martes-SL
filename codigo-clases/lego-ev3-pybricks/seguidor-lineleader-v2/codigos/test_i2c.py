#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.iodevices import I2CDevice
from pybricks.parameters import Port
from pybricks.tools import wait

ev3 = EV3Brick()
ev3.screen.clear()
ev3.screen.draw_text(10, 50, "Buscando...")
ev3.speaker.beep()

PUERTO = Port.S1

addr_exito = None
regs_exito = []

# Buscar en todas las direcciones
for addr in range(0x01, 0x20):
    dev = I2CDevice(PUERTO, addr)
    try:
        # Si logramos leer el fabricante, existe la placa
        dev.read(reg=0x08, length=8)
        addr_exito = addr
        break
    except OSError:
        pass

# Si encontró el sensor, probar qué registros de datos andan
if addr_exito is not None:
    dev = I2CDevice(PUERTO, addr_exito)
    for reg in [0x41, 0x42, 0x43, 0x44, 0x49, 0x54, 0x59]:
        try:
            # Intentar leer 8 bytes (tamaño típico de un array)
            dev.read(reg=reg, length=8)
            regs_exito.append(hex(reg))
        except OSError:
            pass

# Mostrar resultados directamente en la PANTALLA DEL EV3
ev3.screen.clear()
if addr_exito is not None:
    ev3.screen.draw_text(0, 0, "Sensor OK!")
    ev3.screen.draw_text(0, 25, "Dir: " + hex(addr_exito))
    
    # Armamos un string con todos los registros que funcionaron
    texto_regs = ", ".join(regs_exito)
    ev3.screen.draw_text(0, 50, "Regs de 8 bytes:")
    
    # Imprimimos los registros separando en renglones si son muchos
    if len(texto_regs) > 15:
        ev3.screen.draw_text(0, 75, texto_regs[:15])
        ev3.screen.draw_text(0, 95, texto_regs[15:])
    else:
        ev3.screen.draw_text(0, 75, texto_regs)
        
    ev3.speaker.beep(1000, 500)
else:
    ev3.screen.draw_text(0, 40, "FALLO TOTAL")
    ev3.screen.draw_text(0, 70, "Revise puerto 1")
    ev3.speaker.beep(200, 1000)

wait(15000)
