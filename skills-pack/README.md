# 🎒 Skills Pack — LEGO Spike Competition Skills (externo)

Este folder contiene un **set externo de 10 skills** importadas desde el bundle
[LEGO SPIKE Competition Skills Pack](https://github.com/anthropics/skills) (formato
oficial Anthropic Skills) traído por Gustavo el 2026-04-07.

A diferencia de las skills "caseras" del taller que están en
[`../skills/`](../skills/), estas tienen un **formato más rico** con sub-recursos:

```
skills-pack/<nombre>/
├── SKILL.md                      ← Frontmatter YAML + workflow + reglas
├── agents/openai.yaml            ← Metadata para ChatGPT/OpenAI Agents
├── references/<nombre>.md        ← Documentación de referencia ampliada
├── assets/<archivo>              ← Templates de código copiables (.py, .csv, .json, .md)
└── scripts/<archivo>.py          ← Scripts auxiliares ejecutables (cuando aplica)
```

## Las 10 skills del pack

| # | Skill | Cubre |
|---|---|---|
| 1 | **pybricks-api-coach** | Generar, refactorizar y revisar código Pybricks correcto. Estructura de programa, port mapping, DriveBase setup, helpers reusables. Patrones safe para FLL, WRO, RCJ. |
| 2 | **drivebase-tuner** | Calibrar `wheel_diameter` y `axle_track` desde mediciones reales. Incluye script Python para calcular constantes corregidas a partir de tests repetidos. |
| 3 | **sensor-calibration-logger** | Calibrar sensores Spike y analizar logs. Convierte muestras crudas en thresholds con margen de ruido. Incluye script de análisis de CSVs labeled. |
| 4 | **line-follower-tuner** | Construir y sintonizar seguidores de línea P/PD estables. Incluye template Python copiable + patrones de tuning + recuperación de gaps. |
| 5 | **mission-state-machine-builder** | Convertir runs autónomos en state machines robustas con timeouts, retries y fallbacks. Incluye script de scaffolding desde JSON. |
| 6 | **attachment-cycle-optimizer** | Diseñar cycles de attachments motorizados (brazos, lifts, claws) con homing y reset confiables entre runs. Template Python copiable. |
| 7 | **competition-debugger** | Diagnosticar comportamiento inconsistente con un fault tree estructurado. "Mi robot funciona a veces" → causas ranqueadas + tests discriminantes. |
| 8 | **wro-robomission-strategist** | Convertir el rulebook anual de WRO RoboMission en estrategia de scoring, ranking de missions, planes safe/aggressive/fallback. |
| 9 | **fll-robot-game-planner** | Descomponer el FLL Robot Game en launch-return cycles, asignación de attachments, ranking de runs, plan de práctica. |
| 10 | **rescue-line-course-planner** | RoboCup Junior Rescue Line: descomponer en módulos (line tracking, gap recovery, intersections, ramps, victims) con test ladder progresivo. |

## Orden sugerido de instalación

Del README original del bundle:

1. pybricks-api-coach
2. drivebase-tuner
3. sensor-calibration-logger
4. line-follower-tuner
5. mission-state-machine-builder
6. attachment-cycle-optimizer
7. competition-debugger
8. wro-robomission-strategist
9. fll-robot-game-planner
10. rescue-line-course-planner

## Cómo se complementan con las skills caseras de `../skills/`

Las dos colecciones **NO compiten — se complementan**:

| `../skills/` (caseras IITA) | `skills-pack/` (externas) |
|---|---|
| **11 skills**, en español rioplatense | **10 skills**, en inglés |
| Estilo: explicación pedagógica densa con código completo | Estilo: workflow + checklist + assets + scripts |
| Categorías más amplias (sumo, football, future engineers, control theory) | Foco en Pybricks + WRO/FLL/Rescue |
| Formato simple: solo SKILL.md | Formato rico: SKILL.md + references/ + assets/ + scripts/ + agents/ |
| Solo Claude (frontmatter Anthropic) | Claude + ChatGPT (frontmatter Anthropic + agents/openai.yaml) |

**Recomendación práctica**: tener las dos cargadas en el skill manager. Cuando
Claude detecta el contexto, va a activar la(s) skill(s) más relevante(s) de
cualquiera de las dos colecciones. Si se sintetizan, mejor.

## Cómo activarlas

### En Claude (claude.ai)

1. Comprimir cada carpeta `skills-pack/<nombre>/` en `.zip`.
2. claude.ai → Settings → Capabilities → Skills → Upload skill.
3. Subir los 10 zips uno por uno.
4. Confirmar que aparecen activos.

### En ChatGPT (Custom GPTs / Agents)

Estas skills tienen `agents/openai.yaml` que ChatGPT entiende nativamente:

1. Crear un Custom GPT nuevo en chat.openai.com.
2. En "Knowledge", subir los archivos del SKILL.md + references + assets de la skill que querés usar.
3. En "Instructions", pegar el contenido del SKILL.md como base del system prompt.
4. Opcionalmente, usar el `display_name` y `short_description` del `agents/openai.yaml` como nombre y descripción del Custom GPT.

### Workaround rápido (cualquier IA)

Si no querés crear Custom GPTs ni subir nada:

1. Abrir el SKILL.md de la skill que necesitás.
2. Copiar todo el contenido (incluyendo el frontmatter).
3. Pegarlo como primer mensaje en una conversación con Claude/ChatGPT/Gemini.
4. Después pedir lo que necesités con normalidad.

Funciona igual de bien para esa sesión.

## ¿Por qué dos colecciones?

Porque cada una aporta cosas distintas:

- **Las caseras** (`../skills/`) son **más densas y explicativas**, en español, con código completo que un alumno principiante puede copiar/pegar y correr inmediatamente. Cubren más territorio (sumo, football, future engineers, control theory, odometría avanzada).
- **Las del pack** (`skills-pack/`) son **más estructuradas y reusables**, en inglés, con assets/scripts ejecutables y workflow checklist al estilo Anthropic Skills oficial. Tienen mejor compatibilidad multiplataforma (Claude + ChatGPT).

Mantenerlas separadas en dos carpetas:
- Preserva el origen externo del pack (no lo modificamos).
- Permite actualizar el pack si sale una versión nueva sin tocar nuestras caseras.
- Hace claro qué es nuestro y qué viene de afuera.

## Versión

| Versión | Fecha | Descripción |
|---|---|---|
| 1.0 | 2026-04-07 | Importación inicial del bundle "LEGO SPIKE Competition Skills Pack" — 10 skills + assets + scripts + references + agents/openai.yaml |
