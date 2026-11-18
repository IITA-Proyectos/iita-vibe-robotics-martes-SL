# Respaldos del delantero

Copias del programa principal para poder volver atrás **sin depender de Git**: se abren con el
Arduino IDE y se cargan tal cual.

Nombre: `delantero-YYYY-MM-DD-que-tiene.ino`.

## ⚠ "Estable" no es lo mismo que "probado"

Es la distinción que importa cuando hay que elegir cuál cargar antes de un partido:

- **PROBADO EN PARTIDO** — jugó y ganó. Es lo que hay que cargar si algo sale mal y no hay
  tiempo de averiguar qué.
- **PROBADO EN CANCHA** — se lo vio andar y hacer lo que tenía que hacer, pero no jugó un
  partido.
- **COMPILA Y CARGA** — el banner de arranque dice los valores correctos, pero el robot todavía
  no lo usó de verdad. **No es estable todavía**, aunque ande bien en la mesa.

## Qué hay

| archivo | estado | qué tiene |
|---|---|---|
| `delantero-2026-09-15-freno1s-antes-de-retroceso300.ino` | probado en cancha | Cancha de tela recalibrada (umbrales 390/427/413), freno activo, espera de 1 s antes de retroceder de la línea. |
| `partido1-2026-09-21/` | **probado en partido** | Lo que se usó en el partido 1: arco fijo por programa y escape de línea. |
| `delantero-2026-09-29-giro-continuo-centrado-por-angulo-destrabe.ino` | compila y carga (lo reemplaza el del 06/10) | Búsqueda con giro continuo a 50 con rampa, freno de 200 ms al ver la pelota, centrado **por ángulo** (12°/6°) y destrabe (5 s sin recortar 10 cm → avanza 1 s a 150). |
| `delantero-2026-10-06-destrabe-y-orbita-que-no-pierde-la-pelota.ino` | **probado en cancha** | Todo lo del 29/09 **más**: destrabe funcionando de verdad (3 s sin recortar 10 cm → avanza 0,5 s a 150 y busca de vuelta), órbita que aguanta 1 s sin ver la pelota frenada y la recentra si se corre de 18°, puntería apretada (arco 8° en vez de 15°), `XP_ORBITA` 32 cm y patada de 500 ms. |
| `delantero-2026-11-11-ROBOLIGA-2026-tercer-puesto.ino` | **PROBADO EN PARTIDO** | El que jugo la **Roboliga 2026** (11/11): 3 puntos y **tercer puesto** — 4-5, 3-7 y **10-0**. Patada 190 x 450, umbrales 300/300/440, freno antes de patear, saque, camara con horizonte, y el plan B del rumbo cero **a 1 vuelta**. |

### Sobre el de la Roboliga 2026 — el que hay que llevar a una competencia

Es el unico de la lista que **jugo una competencia entera**, y el mejor resultado que tuvo el
equipo. Si en una cancha algo se pone raro y no hay tiempo de averiguar, es a este que hay que
volver.

Lo que vino despues (17/11) le saco la espera de la vuelta para patear mas rapido: eso es mas
rapido pero **no jugo todavia**. Para volver a la logica de la Roboliga desde el programa
nuevo, alcanza con `VUELTAS_PARA_RUMBO0 = 1.0`.

### Sobre el del 06/10 — el más nuevo y el más probado

Es el primero que pasó las cinco pruebas en cancha: encuentra la pelota rápido, destraba cuando
se queda pegado, mantiene la pelota orbitando, patea con las tolerancias apretadas y la pelota
llega al arco. El equipo lo resumió como *"es bastante confiable"*.

**Pero no jugó un partido todavía.** Para competir, eso sigue siendo la diferencia que importa.

### Sobre el del 29/09

Arregló el error que trababa el centrado cuando la pelota está lejos (ver
`bitacora/2026-09-29-...md`, punto 5), pero el destrabe que traía **no funcionaba**: el reloj se
borraba con cada parpadeo de la cámara. Eso se arregló el 06/10. **Usá el del 06/10 en vez de
éste**; queda sólo como punto de vuelta atrás.

Si hay que jugar un partido y algo se pone raro, lo seguro es `partido1-2026-09-21/`.

Perilla útil: en el del 29/09, `BUSCA_CONTINUO = false` devuelve la búsqueda al giro a pulsos
de antes, sin borrar nada.
