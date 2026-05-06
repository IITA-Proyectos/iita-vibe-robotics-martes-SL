from pybricks.hubs import PrimeHub
from pybricks.parameters import Icon
from pybricks.tools import wait

# Inicializar el Hub
hub = PrimeHub()

# Mostrar una cara feliz en la matriz de luces
hub.display.icon(Icon.HAPPY)

# Hacer un sonido (beep)
hub.speaker.beep()

# Esperar 2 segundos
wait(2000)

# Borrar la pantalla
hub.display.off()
