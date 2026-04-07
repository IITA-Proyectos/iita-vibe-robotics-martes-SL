# 🧠 Skills de Claude para Robótica de Competición

Set de **11 skills** para que Claude (y cualquier IA compatible con el formato Anthropic Skills) pueda asistir profesionalmente en programación de robots LEGO Spike Prime con Pybricks, cubriendo todos los formatos de competición de robótica educativa.

## ¿Qué es una skill?

Una **skill** es una unidad de conocimiento especializada que se activa automáticamente cuando Claude detecta que la conversación trata sobre el tema. Cada skill tiene un `SKILL.md` con un frontmatter YAML (`name` + `description`) que el matcher usa para decidir si activarla, seguido de contenido markdown denso con APIs, patrones, ejemplos de código, y best practices.

## Las 11 skills

### Núcleo Pybricks (4)

| # | Skill | Cubre |
|---|---|---|
| 1 | **pybricks-spike-fundamentals** | API base de Pybricks, hub, motores, sensores, estructura típica de programa, debugging |
| 2 | **pybricks-precision-driving** | DriveBase, gyro-assisted driving, calibración empírica, squaring contra paredes, drift management |
| 3 | **pybricks-line-following** | Seguidores P/PI/PID, calibración de ColorSensor, dos sensores, intersecciones, gap recovery |
| 4 | **pybricks-odometry-localization** | Odometría diferencial, dead reckoning, fusión gyro+encoders, kinematics, pose tracking (x, y, θ) |

### Competiciones WRO (3)

| # | Skill | Cubre |
|---|---|---|
| 5 | **wro-robomission-strategy** | Single-run vs multi-run, attachments, recovery patterns, time management, pre-round checklist, scoring strategy |
| 6 | **wro-future-engineers** | Auto autónomo, OpenCV en Raspberry Pi, detección de obstáculos por color, parking paralelo, SLAM básico |
| 7 | **wro-football** | RoboSport / WRO Football, IR ball tracking, goalkeeper vs striker roles, posicionamiento en cancha |

### Competiciones RoboCup Junior (1)

| # | Skill | Cubre |
|---|---|---|
| 8 | **rcj-rescue-line** | RoboCup Rescue Line: gaps, intersecciones, obstáculos, rampas, victims, evacuación, scoring oficial |

### Otros formatos (1)

| # | Skill | Cubre |
|---|---|---|
| 9 | **robot-sumo** | Mini-sumo y full-sumo, edge detection, opponent tracking, push strategies, bullying patterns |

### Transversales (2)

| # | Skill | Cubre |
|---|---|---|
| 10 | **robotics-best-practices** | Workflow profesional: testing, calibración, debugging, control de versiones, documentación, mecánica |
| 11 | **robotics-control-theory** | Teoría de control aplicada: PID tuning real (Ziegler-Nichols), feedforward, filtros (low-pass, complementary, Kalman simple), state machines |

## Cómo activar las skills en Claude

Las skills se gestionan desde la interfaz de Claude. Para que estas 11 queden activas en futuras sesiones:

### Opción A — Subir individualmente (web UI)

Para cada carpeta `skills/<nombre>/`:

1. Comprimir la carpeta en `.zip`.
2. Ir a claude.ai → Settings → Capabilities → Skills → Upload skill.
3. Subir el zip.
4. Confirmar que aparece en la lista de skills activas.
5. Repetir con las otras 10.

### Opción B — Filesystem local (Claude Desktop)

Si usás Claude Desktop con MCP filesystem habilitado, podés clonar el repo y apuntar a `<clone>/skills/<nombre>/SKILL.md`. Claude las descubre automáticamente al inicio de cada sesión si están en un path que el filesystem MCP pueda leer.

### Opción C — Vibe Coding sin upload (workaround)

Si no querés subirlas al skill manager, podés simplemente **pegar el contenido del SKILL.md correspondiente** al inicio de tu conversación con Claude/ChatGPT cuando vayas a programar robots. Funciona como un system prompt one-shot. Menos elegante pero efectivo.

## Estructura de cada skill

```
nombre-skill/
└── SKILL.md          ← Frontmatter YAML + contenido markdown denso
```

El frontmatter sigue el formato de Anthropic:

```yaml
---
name: nombre-skill
description: Descripción precisa de cuándo activar esta skill, con keywords que el matcher reconoce.
---
```

El contenido markdown debe ser **denso, accionable y con ejemplos de código reales que compilen y corran sin modificaciones**.

## Cómo escribir una skill nueva

Si descubrís una técnica nueva, un patrón que funciona, o una categoría de competición no cubierta:

1. Crear `skills/nombre-skill/SKILL.md`.
2. Frontmatter con `name` (kebab-case) y `description` (1-3 oraciones, con keywords obvios).
3. Contenido en markdown:
   - **Cuándo usar / cuándo NO usar** la skill (delimitar el scope).
   - **APIs / objetos clave** con código mínimo de inicialización.
   - **Patrones que funcionan** con código completo copiable.
   - **Errores típicos** en tabla con síntoma → causa → solución.
   - **Recursos externos** (links a docs oficiales, papers, repos de referencia).
4. Mantener el SKILL.md por debajo de 15 KB. Si necesita más, dividirlo en sub-skills temáticas.

## Mantenimiento

Estas skills son **documentos vivos**. Cuando un alumno descubre algo en el taller, cuando ganamos o perdemos en una competición y aprendemos por qué, cuando un patrón resulta más confiable que otro: **se actualiza la skill correspondiente**. El repo es la memoria a largo plazo del taller.

## Versión

| Versión | Fecha | Descripción |
|---|---|---|
| 1.0 | 2026-04-07 | Set inicial de 11 skills cubriendo Pybricks, precision driving, line following, odometría, WRO RoboMission/Future Engineers/Football, RCJ Rescue Line, Sumo, best practices y control theory |
