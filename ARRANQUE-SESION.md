# Prompt de arranque — sesión delantero

> Copiá todo lo que está dentro del bloque y pegalo como primer mensaje en la sesión nueva.
> Abrila en esta misma carpeta: `C:\Users\violl\iita-martes-delantero`

---

```
Trabajamos el robot DELANTERO de fútbol del IITA para la Roboliga 2026, en este worktree
(rama robot/delantero). El firmware vivo está en
futbol-roboliga2026/firmware/delantero/delantero.ino y ya está cargado en el robot.

Leé primero estos dos, que son el estado del proyecto:
  futbol-roboliga2026/firmware/delantero/README.md
  futbol-roboliga2026/bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md

CÓMO SE CARGA AL ROBOT (esto no está documentado y cuesta descubrirlo):
- Desde futbol-roboliga2026/firmware/delantero/ corré:  pio run -e teensy41 -t upload
- La app Teensy Loader TIENE que estar abierta, si no el cargador pide apretar un botón:
  ~/.platformio/packages/tool-teensy/teensy.exe
- Verificá que haya UN SOLO Teensy conectado antes de cargar. El cargador no elige placa:
  con los dos robots enchufados podés flashear el arquero por error. Hay otra sesión
  trabajando ese robot en paralelo.
- Para probar en el piso, desenchufá el USB después de cargar: el programa queda en el
  Teensy y corre con la batería. Si no, el robot arranca el cable al moverse.
- Monitor serie a 19200.

LO QUE ESTÁ MEDIDO EN BANCO (no lo vuelvas a discutir ni a deducir del código):
- Mapeo de ruedas: pines 8/7/6 = IZQUIERDA, 11/12/4 = DERECHA, 2/5/3 = TRASERA.
- L (centro del robot al centro de cada rueda) = 8,75 cm. Las tres ruedas a 120°.
- Piso de arranque desde quieto: ~70 de PWM. Pero con el robot YA rodando alcanza ~40.
  Son dos números distintos y eso importa.
- Para apagar un motor de verdad hay que poner las DOS patas de dirección en 0.
- Las dos ruedas de adelante están montadas espejadas: para avanzar derecho necesitan
  polaridad OPUESTA entre sí. Las tres iguales = el robot rota sobre su eje.
- Los rótulos izquierda/derecha de los comentarios del código 2025 están ESPEJADOS.
  No razones sobre lados usando esos comentarios.
- El punto de giro nulo (strafe puro) está en ~89, no en 100: el equipo 2025 lo midió
  con giroscopio y quedó escrito en arquero.ino:188-191 (50/50/89).

LO QUE NO ESTÁ MEDIDO: la curva PWM→velocidad de cada rueda (cero puntos); si el motor
trasero está montado espejado respecto de los de adelante; los ángulos exactos del trío.

QUÉ HACE HOY EL FIRMWARE: máquina de estados BUSCANDO → CENTRANDO → AVANZANDO →
ORBITANDO → PATEA_ADEL → PATEA_ATRAS. La órbita es "pegada": las dos de adelante van con
PWM 30 (debajo del piso a propósito, para que NO giren) y solo empuja la trasera. Con eso
el radio queda clavado en R = 2·L ≈ 17,5 cm por pura geometría, sin depender de calibrar
nada. VEL_ORB_TRASERA es la única perilla de velocidad y no cambia el radio.
Falta validarlo en banco: hay que medir el círculo con tiza y confirmar ~35 cm de diámetro.

LO PRIMERO QUE HAY QUE HACER, y viene postergado hace tres sesiones:
RECALIBRAR LOS UMBRALES DE COLOR DE LA CÁMARA con el OpenMV IDE. Los que tiene son de la
luz del laboratorio de 2025. Con la pelota quieta, Xp pega saltos no físicos (se vio ir de
60 a 146 cm) y aparecen los valores de recorte (Xp=200, Yp=±100) que son manchas naranjas
del entorno, no la pelota. Mientras siga así, el robot obedece mediciones falsas y
cualquier ajuste de control se hace a ciegas. El script de la cámara está en
futbol-roboliga2026/robots-2025/vision-openmv/ y su README explica el protocolo de 9 bytes.

CÓMO QUIERO QUE TRABAJES:
- Verificá contra el código y el hardware antes de afirmar. No presentes hipótesis como
  hechos: si no lo mediste, decí "según X, falta confirmar".
- Un cambio por vez. Si tocás tres cosas y empeora, no se sabe cuál fue.
- Nada "funciona" porque compila. Lo valida el humano con el robot en la mano.
- Al diseñar un test de banco, elegí la pregunta más fácil de contestar: "¿cuál falta?"
  es más confiable que "¿cuál es?".
- Los lectores son 4 adolescentes y yo. Explicá el mecanismo, no solo el parche.
- Toda sesión deja entrada en futbol-roboliga2026/bitacora/, con los números.
```

---

## Errores de la sesión anterior, para no repetirlos

Van acá y no en el prompt para no hacerlo eterno, pero vale la pena que la sesión nueva
los lea si algo no cierra:

1. **Se afirmó dos veces que este robot era el arquero.** Es el **delantero**. El error vino
   de pruebas cronometradas de "una rueda por vez", que dieron resultados contradictorios.
   Las pruebas de "apagá una y mirá cuál queda quieta" lo resolvieron a la primera.
2. **Se dijo que la rueda derecha andaba en un solo sentido.** Falso: era otra rueda, mal
   identificada por el mapeo equivocado.
3. **Se dijo que `PWM = 0` no apaga el motor.** Sin confirmar — salió de esa misma rueda mal
   identificada. Hay que re-medirlo limpio.
4. **Se dijo que 50/100 daba giro exactamente cero.** El nulo real está en ~89. Con 100 el
   radio es ~140 cm, que en la cancha se ve como una recta: el síntoma no distinguía.
5. **La prueba con la trasera en 210 se descartó como fallida y no lo era** — el reporte fue
   "rodeaba pero patinaba", o sea radio correcto y velocidad de más. Correspondía bajar la
   escala manteniendo la relación.
6. **Se citó la órbita 24/24/72 del campeón 2025 como una relación 1:3 a replicar.** No lo
   es: 24 está debajo de cualquier piso, sus ruedas de adelante no giraban.
7. **Se mandó a cambiar cable y puerto USB** cuando el problema era que faltaba abrir la app
   Teensy Loader.

## Estado del repo

| | |
|---|---|
| Worktree delantero | `C:\Users\violl\iita-martes-delantero` — rama `robot/delantero` |
| Worktree arquero | *sin crear*: `git worktree add C:\Users\violl\iita-martes-arquero -b robot/arquero main` |
| Integración | `C:\Users\violl\iita-viberobotev3` — rama `main` |
| Experimento descartado | rama `experimento/orbita-lazo-cerrado` — lazo cerrado inestable, **no cargar**, tiene 3 piezas reusables |
