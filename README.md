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
├── skills/               ← 🧠 Skills de Claude para robótica (11 skills)
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

## 🧠 Skills de Claude para robótica — el "cerebro" del taller

La carpeta `skills/` contiene **11 skills profesionales para Claude** que cubren
todo el conocimiento de Pybricks + robótica de competición que vamos acumulando
en el taller. Cada skill es una unidad de conocimiento que Claude activa
automáticamente cuando el tema aparece en la conversación.

**Cuando Claude tiene estas skills cargadas, programa robots como un experto
de WRO, RoboCup y FLL** sin que vos tengas que explicarle desde cero.

### Las 11 skills que tenemos

| # | Skill | Cubre |
|---|---|---|
| 1 | `pybricks-spike-fundamentals` | API base de Pybricks: hub, motores, sensores, estructura típica de programa |
| 2 | `pybricks-precision-driving` | DriveBase con gyro, calibración de geometría, squaring contra paredes |
| 3 | `pybricks-line-following` | Seguidores P/PI/PID profesionales con uno o dos sensores, intersecciones, gaps |
| 4 | `pybricks-odometry-localization` | Tracking de pose (x, y, θ), navegación por waypoints, fusión gyro+encoders |
| 5 | `wro-robomission-strategy` | Single-run vs multi-run, attachments, recovery, time management para WRO |
| 6 | `wro-future-engineers` | Auto autónomo Ackermann con OpenCV, parking paralelo, Future Engineers |
| 7 | `wro-football` | IR ball tracking, striker vs goalkeeper, WRO Football y RCJ Soccer |
| 8 | `rcj-rescue-line` | RoboCup Rescue Line: gaps, marcadores verdes, rampas, evacuación de víctimas |
| 9 | `robot-sumo` | Mini-sumo y full-sumo: edge detection, bull rush, dodge, push patterns |
| 10 | `robotics-best-practices` | Workflow profesional: testing, calibración, debugging, mecánica, pre-competition |
| 11 | `robotics-control-theory` | PID Ziegler-Nichols, feedforward, filtros (low-pass, complementary, Kalman), state machines |

**El índice completo y cómo activarlas en el skill manager está en
[`skills/README.md`](skills/README.md).**

### ¿Cómo usar las skills?

**Opción A — Subirlas al skill manager de Claude (la oficial):**

1. Comprimir cada carpeta `skills/<nombre>/` en `.zip`.
2. Ir a claude.ai → Settings → Capabilities → Skills → Upload skill.
3. Subir los 11 zips uno por uno.
4. Listo. En cualquier conversación donde menciones "Pybricks", "WRO", "rescue line", "sumo", etc., Claude activa la skill correspondiente automáticamente.

**Opción B — Pegar el contenido como prompt (workaround rápido):**

Si no tenés el skill manager, copiá el contenido del `SKILL.md` correspondiente
al inicio de tu conversación con Claude/ChatGPT. Funciona igual de bien para
esa sesión.

### ¿Querés mejorar una skill?

Las skills son **documentos vivos**. Cuando descubras un patrón nuevo, una
técnica de calibración que funciona, o un bug recurrente:

1. Editá el `SKILL.md` correspondiente.
2. Commiteá con un mensaje claro de qué agregaste.
3. Los próximos alumnos (y vos en 6 meses) lo van a agradecer.

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
- No borres código de otros

---

*IITA — Innovación y Tecnología Aplicada — Salta, Argentina*
