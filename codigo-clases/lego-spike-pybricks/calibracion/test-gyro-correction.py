# TEST DE HEADING CORRECTION — Calibrar escala del giroscopio
# Cada hub Spike tiene ~±1% de error en el gyro.
# Este test lo mide y lo corrige (se guarda en el hub).
#
# PROCEDIMIENTO:
#   1. Poné el robot contra una pared (referencia)
#   2. Corré este programa
#   3. Girá el robot A MANO exactamente 5 vueltas completas
#      (volvé a la misma posición contra la pared)
#   4. Apretá el botón central del hub
#   5. El programa calcula y guarda la corrección
#
# La corrección se guarda en el flash del hub.
# Persiste hasta que actualices el firmware.

from pybricks.hubs import PrimeHub
from pybricks.parameters import Axis, Button, Color
from pybricks.tools import wait

hub = PrimeHub()

# Esperar calibración inicial
hub.imu.reset_heading(0)
wait(2000)

hub.display.text("5x")
hub.light.on(Color.YELLOW)
hub.speaker.beep(800, 300)

# Esperar a que el usuario gire 5 vueltas y apriete el botón
while Button.CENTER not in hub.buttons.pressed():
    wait(50)

wait(500)  # Debounce

# Leer el ángulo raw (sin calibración)
raw = hub.imu.rotation(-Axis.Z, calibrated=False)
correction = raw / 5.0

# Guardar en el hub
hub.imu.settings(heading_correction=correction)

hub.display.number(int(correction))
hub.light.on(Color.GREEN)
hub.speaker.beep(1200, 200)
wait(100)
hub.speaker.beep(1500, 300)

# El valor queda guardado en el hub
# Debería ser cercano a 360 (ej: 357.2 o 362.8)
