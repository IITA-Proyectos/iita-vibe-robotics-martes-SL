---
name: robotics-best-practices
description: Best practices transversales para robótica de competición — workflow profesional, testing sistemático, calibración, debugging, control de versiones, documentación, mecánica, y disciplinas operativas que diferencian a equipos ganadores. Usar SIEMPRE que se trabaje en metodología de desarrollo de robots de competición, organización del workflow del taller, debugging sistemático de robots, control de versiones de programas Pybricks, documentación de calibraciones, gestión de baterías, troubleshooting, o se mencione 'best practices', 'workflow', 'testing', 'mechanical design', 'team strategy', 'qué hacer cuando no funciona'. Aplica a cualquier categoría de robótica (WRO, RCJ, FLL, sumo, line follower).
---

# Robotics Best Practices — disciplinas profesionales

Las skills técnicas son el 50% de un equipo ganador. El otro 50% son las **disciplinas operativas**: cómo el equipo trabaja, prueba, documenta, y se prepara para la competición. Esta skill cubre las prácticas que diferencian a equipos top de los amateurs.

## Workflow de desarrollo profesional

### El ciclo iterativo

1. **Definir el objetivo** del programa/mission con un test específico. Ej: "el robot recorre 1000 mm y queda dentro de un cuadrado de 30×30 mm".
2. **Programar la versión 0** funcional pero rudimentaria.
3. **Probar 5 veces.** Anotar los resultados (no solo "funciona" o "no funciona", sino dónde y cómo falló).
4. **Mejorar** la parte que más falla.
5. **Probar 5 veces más.**
6. Repetir hasta tener **8 de 10 corridas exitosas**. Entonces declarar la versión "estable".
7. **Solo entonces** pasar a la siguiente mission/programa.

**Trampa común**: pasar a la siguiente mission antes de tener la actual estable. Resultado: 5 missions a medias, ninguna confiable.

### "Funciona" no es binario

Cuando alguien dice "funciona", preguntar:
- ¿Cuántas veces lo probaste?
- ¿En qué condiciones (batería, piso, luz)?
- ¿Cuántas veces falló?
- ¿Por qué falló cuando falló?

**Una mission solo se declara "lista" cuando funciona 8/10 veces o más en condiciones realistas de competición.**

## Testing sistemático

### Test logbook

Cada equipo debe llevar un **logbook** (digital o papel) donde se registra cada test:

| Fecha | Programa | Versión | Bater % | Resultado | Notas |
|---|---|---|---|---|---|
| 2026-04-07 14:30 | mission_north | v3 | 95% | OK | |
| 2026-04-07 14:32 | mission_north | v3 | 95% | OK | |
| 2026-04-07 14:33 | mission_north | v3 | 92% | FAIL | Se desvió 5 cm en el último giro |
| 2026-04-07 14:36 | mission_north | v3 | 90% | OK | |

Al revisar el logbook se ven patrones que en el momento no se notan: "fallaba siempre cuando la batería bajaba de 92%" o "fallaba siempre la última corrida del día = los motores recalientan".

### Test conditions matrix

Antes de declarar un programa estable, probarlo bajo distintas condiciones:

| Condición | Test |
|---|---|
| Batería 100% | 5 corridas |
| Batería 70% | 5 corridas |
| Mat con polvo | 5 corridas |
| Diferentes ángulos de luz | 5 corridas |
| Después de 1 hora corriendo | 5 corridas |
| Mat de práctica vs mat oficial (si lo hay) | 5 corridas cada uno |

Si pasa las 6 condiciones con 8/10 = el programa está listo para competición.

## Calibración como disciplina

### Calibración inicial del robot (una vez)

1. **`wheel_diameter`** empíricamente con `straight(1000)` y cinta métrica.
2. **`axle_track`** empíricamente con `turn(360)` y comparación con gyro.
3. **Voltaje de referencia**: anotar a qué voltaje se hicieron las calibraciones (típicamente 7.8-8.0 V).

### Calibración por sesión (cada vez que se enciende el robot)

1. **Verificar bater** > 7800 mV.
2. **Calibrar `WHITE` y `BLACK`** del ColorSensor sobre el mat actual.
3. **Reset del IMU** (heading = 0).
4. **Test de straight 500 mm** y verificar visualmente que va recto.
5. **Test de turn(360)** y verificar que vuelve al heading inicial con error <2°.

Si falla cualquiera de los pasos 4-5, **NO empezar a programar/competir** hasta resolverlo.

### Documentar las calibraciones en el código

```python
# CALIBRACIONES — Robot v3.2 — 2026-04-07
# wheel_diameter calibrado con: straight(1000) → 998 mm reales (3 corridas, batería 8.0V)
# axle_track calibrado con: turn(360) → gyro reportó -0.5° (2 corridas, batería 8.0V)
# Sensores de color calibrados sobre mat WRO 2026 oficial
WHEEL_DIAMETER = 56.1
AXLE_TRACK = 113
WHITE_LEFT = 92
BLACK_LEFT = 8
WHITE_RIGHT = 90
BLACK_RIGHT = 10
```

## Debugging sistemático

### El método de los 5 porqués

Cuando algo falla, no parar al primer "ah, era esto". Preguntar 5 veces "¿por qué?":

> "El robot no completó la mission."
>
> ¿Por qué? "Porque se desvió 5 cm a la izquierda."
>
> ¿Por qué? "Porque el motor izquierdo giró menos."
>
> ¿Por qué? "Porque las ruedas patinaron en el arranque."
>
> ¿Por qué? "Porque la aceleración era muy alta."
>
> ¿Por qué? "Porque copiamos el código de otra mission sin ajustar settings."

**Causa raíz encontrada**: settings hardcoded en cada mission en lugar de un set único calibrado.

### Visualización en el hub

`print()` no se ve en standalone. Usar:

```python
hub.display.text(str(int(value)))         # mostrar números
hub.display.icon(STAR)                     # marcadores visuales de estado
hub.light.on(Color.GREEN / RED / YELLOW)  # códigos de color rápidos
hub.speaker.beep(frequency, duration)     # alertas sonoras
```

### Bisectar el problema

Si un programa falla, **cortarlo a la mitad**, probar la primera mitad, después la segunda mitad. Identificar en qué mitad está el bug. Repetir bisectando hasta aislar la línea problemática.

## Control de versiones y backup

### Naming convention para iteraciones

- `mission_north_v1.py` — primera versión funcional
- `mission_north_v2.py` — agregado check de pelota
- `mission_north_v3.py` — calibrado para mat oficial

**NUNCA borrar versiones viejas que funcionaron**. La v2 puede ser más estable que la v3 cuando estás bajo presión en competición.

### Repositorio compartido

Cada cambio importante se commitea con un mensaje descriptivo. Para el taller IITA, el repo es `IITA-Proyectos/iita-vibe-robotics-martes-SL` con la convención de carpetas establecida en el README.

### Backups antes de competición

**El día anterior a la competición**:
1. Hacer backup completo del código a un USB.
2. Imprimir el código de cada programa en papel (sí, papel — si todo falla en la cancha, podés tipearlo de nuevo).
3. Tener 2 hubs cargados con los programas listos.
4. Tener las baterías de repuesto cargadas.
5. Tener cables de repuesto.

## Diseño mecánico — principios básicos

### Bajo y centrado

El centro de gravedad bajo da **estabilidad**. Robots altos se vuelcan en aceleración, frenado, o giros rápidos.

### Modular

Los attachments deben **encajar y quitarse en <5 segundos** sin destornillador. Pins LEGO con tope mecánico funcionan perfectamente.

### Robusto

El robot va a chocar con cosas. La base estructural debe aguantar 100 caídas sin deformarse. Reforzar los puntos de fijación de los motores y del hub con beams cruzados.

### Predecible

Si el robot tiene partes que se mueven libremente (un brazo que cuelga), va a comportarse distinto cada vez. **Todo movimiento debe estar controlado o fijado**.

### Probado

Antes de competición, **golpear el robot deliberadamente** desde varios ángulos. Si algo se rompe ahora, mejor que en la cancha.

## Gestión de baterías

- **Cargar baterías la noche anterior** a la competición, no la mañana de.
- **NO sobrecargar** (dejarlas conectadas días seguidos las daña).
- **Etiquetar las baterías** con número (Bater 1, Bater 2) para rotarlas y trackear el desgaste.
- **Reemplazar baterías** que ya no llegan a 8.0V después de carga completa.
- **Llevar al menos 2 baterías cargadas** a la competición.

## Pre-competition checklist

24 horas antes:

- [ ] Todas las baterías cargadas y verificadas a >8.0V
- [ ] Todos los programas commiteados al repo
- [ ] Backup en USB
- [ ] Código impreso en papel
- [ ] 2 hubs flasheados con Pybricks último firmware
- [ ] Todos los cables verificados sin daño visible
- [ ] Sensores limpios (lentes con paño seco)
- [ ] Rueda y axles libres de polvo y pelos
- [ ] Mat de práctica plegado para llevar
- [ ] Logbook de tests con últimas corridas anotadas
- [ ] Equipo completo informado del horario
- [ ] Comida y agua para el equipo durante la competición

En la cancha:

- [ ] Verificar que el mat oficial se siente igual al de práctica (calibrar de nuevo)
- [ ] Verificar iluminación de la cancha vs práctica (recalibrar sensores de color)
- [ ] Pre-round check programático corriendo
- [ ] Programa #1 seleccionado en el hub
- [ ] Equipo posicionado en el lado correcto del mat

## Filosofía general del equipo

1. **La preparación gana competencias, no la inspiración del momento.**
2. **El robot es del equipo, no del que lo programó.** Todos deben saber cómo arrancarlo, posicionarlo, y resetearlo.
3. **Ningún cambio el día de la competencia** salvo emergencia. Lo que fue probado y funciona se mantiene.
4. **Aceptar los empates con dignidad.** A veces el otro equipo simplemente lo hace mejor.
5. **Después de cada competición, retrospectiva del equipo:** qué funcionó, qué no, qué cambiar para la próxima. Documentar las lecciones en el repo.

## Recursos

- The Pragmatic Programmer (Hunt & Thomas) — best practices generales de software adaptables a robótica.
- LEGO Mindstorms Building Engineering (Yoshihito Isogawa) — referencia mecánica.
- FLL Coach Handbook (FIRST) — best practices de coaching de equipos juveniles.
- Repos de equipos top de WRO/FLL en GitHub — leer cómo organizan ellos.
