from pybricks.hubs import PrimeHub
from pybricks.pupdevices import ColorSensor
from pybricks.parameters import Port
from pybricks.tools import wait

hub = PrimeHub()
sensor = ColorSensor(Port.C)

hub.screen.clear()

while True:
    reflejo = sensor.reflection()
    hub.screen.print("Reflejo:", reflejo)
    wait(100)