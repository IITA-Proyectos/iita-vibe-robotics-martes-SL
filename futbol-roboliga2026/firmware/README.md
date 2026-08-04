# Firmware de los robots

Acá vive el **firmware vivo** de cada robot: lo que se lleva a la cancha.

| Carpeta | Robot | Rama donde se trabaja |
|---|---|---|
| [`delantero/`](delantero/) | el que el equipo llama *"robot 2"* → `#define ROBOT2` | `robot/delantero` |
| `arquero/` | el otro → `#define ROBOT1` | `robot/arquero` |

## Esto NO es `pruebas/`

| | [`../pruebas/`](../pruebas/) | `firmware/` |
|---|---|---|
| Para qué | medir, diagnosticar, responder **una** pregunta | jugar |
| Vida útil | descartable | permanente, evoluciona |
| Se rompe sin culpa | sí | no: se rompe en una rama y se prueba antes de mergear |

Cuando una prueba deja una técnica que sirve, **la técnica** se muda acá. La prueba queda donde
está, como registro de cómo se midió.

## Dos sesiones en paralelo — las reglas

Hay una sesión trabajando el **arquero** y otra el **delantero**, cada una en su rama y su
worktree. Para que no se pisen:

**1. Cada sesión toca SOLO la carpeta de su robot.** `firmware/delantero/` es del delantero,
`firmware/arquero/` del arquero. Nadie edita la del otro.

**2. Lo compartido se toca con aviso.** `robots-2025/`, `pruebas/`, `bugs-conocidos.md`,
`entorno.md`, las skills. Si hay que cambiar algo ahí, avisar a la otra sesión: son los archivos
donde de verdad se puede generar un conflicto.

**3. `git fetch` + `git merge origin/main` antes de pushear.** El repo es compartido y los
alumnos también pushean directo.

**4. 🚨 UN SOLO ROBOT CONECTADO POR VEZ.** Ésta es la importante y no es de git.
El cargador del Teensy **no elige a qué placa le manda el programa**: agarra la que encuentra.
Con los dos robots enchufados al mismo tiempo, la sesión del arquero puede flashear el delantero
sin enterarse — y viceversa. Antes de cargar, **verificar que sólo hay un Teensy conectado**:

```bash
pio device list
```

Si aparecen dos, desenchufar uno. No hay forma de elegir desde el `platformio.ini`.

## Cómo cargar

Desde la carpeta del robot:

```bash
pio run -e teensy41 -t upload
```

Cada carpeta trae su `platformio.ini` con `upload_protocol = teensy-gui`, así que **no hay que
apretar ningún botón**: reinicia la placa solo.

También se abre tal cual con el Arduino IDE (Placa → Teensy 4.1). La carpeta y el `.ino` se
llaman igual justamente para eso.

> Para probar en el piso, **desenchufá el USB después de cargar**. El programa queda en el Teensy
> y corre con la batería. Con el cable puesto, el robot lo arranca al moverse.
