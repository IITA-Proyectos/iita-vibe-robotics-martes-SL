# 2026-11-03 — El brillo de la pelota, el amarillo nuevo y la pelota de lejos

**Quiénes:** Máximo y Juanse (mesa del delantero) + Claude (código y revisión)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**PC:** la computadora móvil del equipo
**Programa en el robot al cerrar:** el principal **con el filtro del brillo**: en el monitor
aparece `brillos descartados`. **El `XP_MAX` 200 no sé si llegó a cargarse**, porque se cambió
al final. 🔴 **Antes de jugar: `CARGAR-ROBOT.bat`.**

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 👏 **Cómo anduvo** | **los profes reaccionaron bien a cómo llegó a andar el robot hoy** |
| 🟡 **El problema** | la cámara veía el brillo de la pelota como **arco amarillo**, y el robot pateaba |
| 🤖 **Arreglo en el robot** | si llega un arco amarillo pegado a la pelota, el robot no le cree: es el brillo |
| 📷 **Arreglo en la cámara** | amarillo nuevo `(50, 97, -25, -3, 50, 111)`: la **opción 5** de cinco, la que funcionó |
| 🔭 **La pelota de lejos** | `XP_MAX` 150 → **200**: girando, la cámara veía la pelota lejos y el robot no frenaba |
| ❓ **Sin anotar** | cómo salió la patada 180 × 450 y el giro a 35 |

---

## 1. 👏 Cómo anduvo

La clase empezó probando el principal tal como quedó el 27/10: patada `180 x 450 ms`, `KP 6.0`,
giro de búsqueda a `35`. Al final, **los profes tuvieron buenas reacciones de cómo llegó a andar
el robot hoy.**

**No quedó anotado:** si la patada salió derecha, si el robot se levantó al frenar y si el giro
a 35 arrancó siempre.

---

## 2. 🟡 El problema: el brillo de la pelota

Ya se había visto el 27/10. La luz del aula hace un brillo arriba de la pelota, la cámara lo ve
amarillo y lo manda como **ARCO AMARILLO**, pegado a la pelota. Uno de los paquetes que llegaron:

```
[201,19,101, 202,23,102, 203,0,100]   ->  pelota a 19 cm, "arco" a 23 cm, en la misma direccion
```

El robot cree que tiene el arco justo detrás de la pelota, y patea.

**Por qué sale amarillo (explicación, no medida):** la luz deja el rojo y el verde de la cámara
al máximo, y rojo más verde a tope da amarillo claro.

### Lo que se midió, con el histograma LAB del OpenMV IDE

**Primera medida:**

| | L | A | B |
|---|---|---|---|
| brillo | 88 a 100 | −19 a 5 | −1 a 55 |
| arco | 86 a 99 | −20 a 0 | 14 a 62 |
| umbral que había | 85 a 99 | −16 a 14 | 17 a 111 |

**Segunda medida, con el cuadradito encima de cada cosa sola.** "La mitad del medio" es del LQ
al UQ: ahí cae la mitad de los puntos.

| | L | A | B |
|---|---|---|---|
| arco: todo | 92 a 98 | −19 a −2 | 38 a 66 |
| arco: la mitad del medio | 95 a 98 | −15 a −9 | **47 a 58** |
| brillo: todo | 52 a 100 | −19 a 79 | 0 a 65 |
| brillo: la mitad del medio | **98 a 98** | −15 a −12 | 43 a 51 |

**Qué dice:** la A es la misma y la B casi se pisa. Hay dos diferencias: **el brillo está
quemado** (L en 98 o más) y **el arco es un poco más amarillo** (B más alta).

---

## 3. 🚫 Lo que se pensó y se dejó

| idea | por qué no |
|---|---|
| Que se fije en la forma y el tamaño (el arco es grande y rectangular) | Al robot la cámara le manda solo dónde está cada cosa, no el tamaño. En la cámara, el equipo lo descartó: **muchas veces el arco está lejos o tapado por un robot** |
| Cambiar el programa de la cámara | Decisión del equipo: **en la cámara solo se tocan los valores LAB** |
| Fijar la exposición y la ganancia de la cámara | Sería el arreglo de fondo (el brillo no se quemaría), pero cambian todos los colores y hay que volver a medir el naranja, el amarillo y el azul. **No antes de la Roboliga** |

---

## 4. 🤖 Arreglo 1, en el robot: el filtro del brillo

**El pedido:** que el programa principal decida, con el amarillo que le llega, si es el arco o
no, sin tocar el programa de la cámara.

**Cómo decide:** si llega un arco amarillo **en la misma dirección que la pelota** (a menos de
`BRILLO_ANG` = 10°) y **a menos de 1,5 veces la distancia de la pelota** (`BRILLO_FACTOR`), no
le cree: es el brillo.

- **Llave:** `FILTRO_BRILLO_PELOTA` en OTROS AJUSTES, sección CAMARA Y GIROSCOPIO. `false` = como
  antes.
- **El cartel** dice `camara: un arco amarillo pegado a la pelota es su brillo -> NO le creo`.
- **La telemetría** cuenta `brillos descartados: N`.
- **Comprobado:** se probó en la PC con el caso visto (19 cm y 23 cm da *brillo*) y con arcos
  inventados lejos o al costado (dan *arco*). Compila en los tres programas de partido. **Está
  cargado:** el monitor lo mostró, con el robot en la mesa y `brillos descartados: 0`.
- **No se vio todavía en la cancha** descartando un brillo de verdad.

⚠️ **El lado malo:** si la pelota está **pegada al arco** (por ejemplo, pelota a 30 cm y arco a
40 cm en la misma línea), el robot **tampoco le cree a ese arco**. Ahí no patea apuntando al arco:
sigue rodeando, y a la vuelta patea mirando al rumbo cero (el plan B). Ahora que la cámara ya no
ve el brillo (punto 5), **conviene probar con `FILTRO_BRILLO_PELOTA = false`**.

---

## 5. 📷 Arreglo 2, en la cámara: el amarillo nuevo

Se probaron **cinco opciones**. El "cuánto queda" es una cuenta aproximada hecha con los
histogramas, no medida.

| | umbral amarillo | idea | del arco queda | del brillo queda |
|---|---|---|---|---|
| 1 | `(50, 98, -27, 1, 38, 111)` | la de antes (para comparar) | todo | todo |
| 2 | `(50, 98, -25, -3, 53, 111)` | solo más amarillo | ~4 de 10 | ~2 de 10 |
| 3 | `(50, 97, -25, -3, 38, 111)` | sin lo quemado | ~4 de 10 | ~1 de 10 |
| 4 | `(50, 97, -25, -3, 45, 111)` | mezcla suave | ~3 de 10 | casi nada |
| **5** | **`(50, 97, -25, -3, 50, 111)`** | **mezcla fuerte** | ~2 o 3 de 10 | nada |

✅ **El equipo la probó y funcionó la 5.**

- **Copia en el repo:** `vision/camara-delantero-2026-11-03.py`. Es el programa de la cámara,
  igual a `camara-delantero-2026-10-13.py`, cambiando solo la línea del amarillo.
- **Falta ver** que la 5 vea el arco **de lejos** y **con un robot adelante**.
- **Depende de la luz:** el 97 funciona porque en el aula el brillo se quema. En la Roboliga hay
  que volver a mirarla.
- ⚠️ En un momento la cámara tuvo `(17, 70, -27, 14, 38, 111)`. **Esa no ve el arco:** el arco
  tiene L de 92 a 98 y ese umbral corta en 70.
- **Para comparar umbrales sin tocar el programa:** OpenMV IDE, *Herramientas → Visión
  artificial → Editor de umbrales*. Se tiene que ver blanco el arco y negro el brillo.

---

## 6. 🔭 La pelota de lejos: `XP_MAX` 150 → 200

**Lo que pasaba:** girando para buscar, con la pelota lejos, el LED rojo de la cámara se prendía
pero el robot seguía girando.

**Por qué (deducido del programa, no confirmado con el monitor):** el robot no le creía a una
pelota a más de `XP_MAX` = 150 cm. Se había puesto así para que no fuera a manchas naranjas de
afuera de la cancha. La cámara la veía, pero para el robot "no había pelota".

**El cambio:** `XP_MAX = 200`. La cámara manda como máximo 200, así que ahora el robot le cree a
cualquier pelota que vea. Cuando la ve, ya frenaba 200 ms (`MS_FRENO_AL_VER`) antes de ir hacia
ella.

- **Riesgo:** que vaya a algo naranja de afuera de la cancha. Si pasa, poner 180 o volver a 150.
- **Estado:** compila en los tres programas. **No confirmado en el robot.**
- **Cómo probarlo:** con la pelota en la otra punta, el monitor tiene que decir
  `*** la vi a XX cm -> freno el envion` y el robot tiene que ir hacia ella.

---

## 7. 🖥️ La cámara en la PC

- **Al principio el IDE no mostraba la imagen y el Viewer sí.** La cámara la puede usar un solo
  programa a la vez (COM5), y el IDE frena el programa de la cámara cuando se conecta (hay que
  darle *Play*). Más tarde midieron con el histograma LAB del IDE.
- **No actualizar el firmware de la cámara antes de la Roboliga.**
- **Monitor del robot:** `pio device monitor -p COM6 -b 19200`. Hay que cerrarlo (Ctrl+C) antes
  de `CARGAR-ROBOT.bat`.
- **En la mesa** el sensor de línea de adelante marca ~766, porque la mesa es clara: el robot cree
  que está en la línea blanca. **No es una falla.**

---

## ⚠️ Qué queda por VER

1. 🔴 **La pelota de lejos con `XP_MAX` 200:** que frene y vaya, y que no se vaya detrás de cosas
   naranjas de afuera.
2. 🔴 **El amarillo de la opción 5:** que vea el arco de lejos y con un robot adelante.
3. **La pelota pegada al arco:** ¿patea? Si no, probar `FILTRO_BRILLO_PELOTA = false`.
4. **La patada 180 × 450 y el giro a 35:** anotar qué se ve.

## 🔧 Qué queda por HACER — en orden

1. 🔴 **Antes de jugar: `CARGAR-ROBOT.bat`** con el arco que toque, para que entre el `XP_MAX` 200.
2. 🔴 **En la Roboliga (11/11):** medir la línea con `grabar-linea` y los LAB de la cámara con la
   luz de allá, sobre todo el amarillo.
3. **Siguen de antes:** medir `L`, revisar el cable USB, el nombre y correr `CALIBRAR-ROBOT.bat`
   entero una vez.

---

## Nota de método

**Lo que salió bien:** se midió antes de elegir. Los histogramas mostraron que por A y B no se
separaban, y que la diferencia estaba en lo quemado. Y el equipo probó cinco opciones en vez de
una: la que funcionó salió de probar, no de adivinar.

**Lo que falta, otra vez:** anotar qué se vio. Hoy el robot anduvo bien delante de los profes,
pero de la patada a 180 no quedó escrito nada. Dos renglones por prueba alcanzan.
