# 🤖 IITA Vibe Robotics — Martes San Lorenzo

**Taller de Vibe Coding + Robótica con LEGO Spike Prime y Pybricks**

Instituto de Innovación y Tecnología Aplicada (IITA) — Sede San Lorenzo Chico

## ¿Qué es esto?

Este repositorio es el espacio de trabajo colaborativo del taller de los martes.
Acá cada alumno guarda sus programas, y entre todos vamos construyendo una
biblioteca de código probado, directivas para IA, skills especializadas, y
rutinas que funcionan.

**Vibe Coding** = programar robots pidiéndole a una IA (Claude, ChatGPT, etc)
que escriba el código, y después probarlo, ajustarlo e iterarlo en el robot real.
La IA propone, el robot dispone. 🎯

## 📁 Estructura del repo

```
├── alumnos/              ← Carpeta personal de cada uno
│   ├── octavio/
│   ├── maximio/
│   ├── juanse/
│   ├── laureano/
│   ├── profegustavo/
│   └── profefranco/
│
├── robots/               ← Definiciones de robots compartidos
│
├── codigo-ejemplo/       ← Programas probados y funcionando
│   ├── movimiento/       ← Rectas, giros, cuadrados, curvas
│   ├── sensores/         ← Line follower, detección de color
│   └── calibracion/      ← Tests de calibración
│
├── directivas-ia/        ← Prompts y directivas para IAs
│   └── system-prompts/   ← Prompts de sistema para Claude/GPT
│
├── skills/               ← 🧠 Skills CASERAS de Claude (11 skills, en español)
│   ├── README.md         ← Índice + cómo subirlas al skill manager
│   ├── pybricks-spike-fundamentals/
│   ├── pybricks-precision-driving/
│   ├── pybricks-line-following/
│   ├── pybricks-odometry-localization/
│   ├── wro-robomission-strategy/
│   ├── wro-future-engineers/
│   ├── wro-football/
│   ├── rcj-rescue-line/
│   ├── robot-sumo/
│   ├── robotics-best-practices/
│   └── robotics-control-theory/
│
├── skills-pack/          ← 🎒 Skills EXTERNAS importadas (10 skills, en inglés, formato rico)
│   ├── README.md         ← Origen, contenido y cómo activarlas
│   ├── pybricks-api-coach/
│   ├── drivebase-tuner/
│   ├── sensor-calibration-logger/
│   ├── line-follower-tuner/
│   ├── mission-state-machine-builder/
│   ├── attachment-cycle-optimizer/
│   ├── competition-debugger/
│   ├── wro-robomission-strategist/
│   ├── fll-robot-game-planner/
│   └── rescue-line-course-planner/
│
├── rutinas-exitosas/     ← Código que FUNCIONÓ bien en pruebas
│
└── docs/                 ← Documentación y guías
```

## 🚀 ¿Cómo empezar?

1. Abrí [code.pybricks.com](https://code.pybricks.com) en tu navegador
2. Conectá tu Spike Prime por Bluetooth
3. Copiá un ejemplo de `codigo-ejemplo/` y pegalo en Pybricks
4. ¡Probalo en el robot!
5. Cuando algo funcione bien, guardalo en tu carpeta `alumnos/tunombre/`

## 🤖 ¿Cómo hacer Vibe Coding?

1. Abrí Claude (claude.ai) o ChatGPT
2. Copiá el prompt de `directivas-ia/system-prompts/pybricks-expert.md`
3. Pegalo como primer mensaje
4. Pedile que programe lo que necesitás para tu robot
5. Copiá el código a Pybricks, probalo, y pedile ajustes
6. Cuando funcione, guardalo en tu carpeta y en `rutinas-exitosas/`

## 🧠 Skills de IA — el "cerebro" del taller (21 skills en total)

El repo tiene **dos colecciones de skills** que enseñan a Claude (y a ChatGPT)
todo lo necesario para programar robots LEGO Spike Prime con Pybricks a nivel
de competición. Las dos se complementan:

### `skills/` — 11 skills caseras del taller (en español)

Skills creadas específicamente para el taller IITA. Densas, pedagógicas, con
código completo en español rioplatense. Cubren desde fundamentos hasta
categorías de competición específicas que el bundle externo no tiene.

| Skill | Cubre |
|---|---|
| `pybricks-spike-fundamentals` | API base de Pybricks: hub, motores, sensores, estructura típica |
| `pybricks-precision-driving` | DriveBase con gyro, calibración, squaring contra paredes |
| `pybricks-line-following` | P/PI/PID, dos sensores normalizados, intersecciones, gaps |
| `pybricks-odometry-localization` | Tracking de pose (x, y, θ), navegación por waypoints, fusión gyro+encoders |
| `wro-robomission-strategy` | Single-run vs multi-run, attachments, recovery, time management |
| `wro-future-engineers` | Auto autónomo Ackermann con OpenCV, parking paralelo |
| `wro-football` | IR ball tracking, striker vs goalkeeper, "approach from behind" |
| `rcj-rescue-line` | Gaps, marcadores verdes, rampas, evacuación de víctimas |
| `robot-sumo` | Edge detection, bull rush, dodge, push patterns |
| `robotics-best-practices` | Workflow, testing, calibración, debugging, mecánica, pre-competition |
| `robotics-control-theory` | PID Ziegler-Nichols, feedforward, filtros, state machines |

Índice completo en [`skills/README.md`](skills/README.md).

### `skills-pack/` — 10 skills externas importadas (en inglés)

Bundle externo "LEGO SPIKE Competition Skills Pack" con formato más rico
(SKILL.md + `references/` + `assets/` + `scripts/` + `agents/openai.yaml`).
Incluye templates Python copiables, scripts ejecutables y compatibilidad
nativa con ChatGPT Custom GPTs.

| Skill | Cubre |
|---|---|
| `pybricks-api-coach` | Generar/refactorizar/revisar código Pybricks correcto |
| `drivebase-tuner` | Calibrar `wheel_diameter` y `axle_track` (incluye script) |
| `sensor-calibration-logger` | Calibrar sensores, analizar logs CSV (incluye script) |
| `line-follower-tuner` | Tuning de seguidores P/PD con template Python |
| `mission-state-machine-builder` | State machines con timeouts, retries, fallbacks |
| `attachment-cycle-optimizer` | Cycles de attachments con homing y reset |
| `competition-debugger` | Fault tree de "mi robot funciona a veces" |
| `wro-robomission-strategist` | Ranking de missions, planes safe/aggressive |
| `fll-robot-game-planner` | Launch-return cycles, asignación de attachments |
| `rescue-line-course-planner` | Módulos: line tracking, gap recovery, intersections |

Índice completo en [`skills-pack/README.md`](skills-pack/README.md).

### ¿Cómo se complementan las dos colecciones?

**No compiten — se sintetizan.** Cuando tenés las dos cargadas en el skill manager:

- Para una pregunta sobre **calibrar el robot**, Claude puede usar `pybricks-precision-driving` (caseras, explica el porqué) + `drivebase-tuner` (pack, da el script ejecutable).
- Para **line following**, puede usar `pybricks-line-following` (caseras, código completo) + `line-follower-tuner` (pack, template + checklist).
- Para **WRO**, puede usar `wro-robomission-strategy` (caseras, foco en código) + `wro-robomission-strategist` (pack, foco en estrategia).

La síntesis de las dos colecciones cubre **mejor que cualquiera de las dos solas**.

### ¿Cómo usar las skills?

**Opción A — Skill manager de Claude:**

1. Comprimir cada carpeta `skills/<nombre>/` y `skills-pack/<nombre>/` en `.zip`.
2. claude.ai → Settings → Capabilities → Skills → Upload skill.
3. Subir las 21 skills (o las que quieras usar).
4. En cualquier conversación donde menciones "Pybricks", "WRO", "rescue line", "sumo", etc., Claude activa la(s) skill(s) correspondiente(s) automáticamente.

**Opción B — ChatGPT Custom GPT** (solo `skills-pack/` por su formato compatible):

1. Crear un Custom GPT nuevo.
2. Subir los archivos de la skill como Knowledge.
3. Pegar el SKILL.md como Instructions.

**Opción C — Pegar como prompt** (workaround universal): copiar el SKILL.md
correspondiente al inicio de cualquier conversación con Claude/ChatGPT/Gemini.

### ¿Querés mejorar una skill?

Las skills son **documentos vivos**:

- Las **caseras** (`skills/`) las podés editar libremente cuando descubras
  algo en el taller.
- Las del **pack externo** (`skills-pack/`) idealmente NO se modifican
  para preservar el origen externo. Si querés agregarles algo, mejor crear una
  skill casera nueva en `skills/` que las complemente.

## 👥 Equipo

| Nombre | Carpeta |
|--------|--------|
| Octavio | `alumnos/octavio/` |
| Maximio | `alumnos/maximio/` |
| Juanse | `alumnos/juanse/` |
| Laureano | `alumnos/laureano/` |
| Profe Gustavo | `alumnos/profegustavo/` |
| Profe Franco | `alumnos/profefranco/` |

## 📝 Reglas del repo

- Cada uno trabaja en SU carpeta
- Si algo funciona bien → copialo también a `rutinas-exitosas/`
- Poné comentarios en el código explicando qué hace
- Si descubrís un buen prompt para la IA → compartilo en `directivas-ia/`
- Si descubrís una técnica nueva → mejorá la skill correspondiente en `skills/`
- No modifiques las skills del `skills-pack/` (son externas) — si querés agregarles algo, creá una skill nueva en `skills/`
- No borres código de otros

---

*IITA — Innovación y Tecnología Aplicada — Salta, Argentina*
