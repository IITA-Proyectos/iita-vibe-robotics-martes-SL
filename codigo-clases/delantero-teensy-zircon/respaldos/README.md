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
| `delantero-2026-09-29-giro-continuo-centrado-por-angulo-destrabe.ino` | **compila y carga** | Búsqueda con giro continuo a 50 con rampa, freno de 200 ms al ver la pelota, centrado **por ángulo** (12°/6°) y destrabe (5 s sin recortar 10 cm → avanza 1 s a 150). |

### Sobre el del 29/09

Es el más avanzado y el que arregla el error que trababa el centrado cuando la pelota está
lejos (ver `bitacora/2026-09-29-...md`, punto 5). Pero **todavía no jugó**, y además arrastra
las 4 mejoras de la patada del 22/09 que también siguen sin validar en cancha.

Si hay que jugar un partido y algo se pone raro, lo seguro es `partido1-2026-09-21/`.

Perilla útil: en el del 29/09, `BUSCA_CONTINUO = false` devuelve la búsqueda al giro a pulsos
de antes, sin borrar nada.
