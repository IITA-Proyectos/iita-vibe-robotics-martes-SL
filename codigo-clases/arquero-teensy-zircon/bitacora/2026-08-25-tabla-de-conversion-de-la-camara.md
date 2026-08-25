# 2026-08-25 — La cámara no estaba inclinada: estaba 11 cm más abajo

**Quiénes:** Gustavo Viollaz + los chicos (cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa nuevo:** `pruebas/tabla-camara`
**Cuenta nueva de Claude.** Esta carpeta pasa a ser el espacio de trabajo del arquero; el
delantero lo sigue llevando la otra mesa y no se toca.

**Resumen del día:** se cerró el pendiente que venía trabado hace dos clases. La cámara exagera
las distancias por un **factor parejo de 2,87**, y la causa quedó identificada y confirmada con
regla: **la cámara está montada a ~8 cm del piso, no a los 18,7 cm para los que fue calibrada.**

---

## 1. La pregunta del día, y por qué hacían falta cinco puntos

Veníamos sabiendo que la cámara exagera (el 11/08: dice 48, la pelota está a 12). Lo que **no**
sabíamos es lo único que importaba para arreglarlo:

> ¿Exagera **parejo**, o exagera **cada vez más** a medida que la pelota se aleja?

Las dos cosas se ven igual si medís un solo punto, y llevan a arreglos completamente distintos:

| Si el factor sale… | La causa fue… | El arreglo es… |
|---|---|---|
| parejo | cambió la **altura** de la cámara | una división |
| creciendo con la distancia | cambió la **inclinación** | una tabla entera con interpolación |

La hipótesis que traía el profe era la inclinación. **Los datos dijeron que no**, y eso simplificó
mucho el arreglo.

Es el mismo motivo por el que el 04/08 hicieron falta **tres** saltos para calibrar la distancia
del robot y no uno: cuando hay dos explicaciones posibles, una sola medición no las separa.

---

## 2. El programa: medir sin computadora

`pruebas/tabla-camara`. El cable USB no llega a la cancha, así que el robot mide solo, se acuerda,
y después se le pregunta en la mesa con la tecla `m`. Mismo patrón que `calibrar-linea`.

**No mueve los motores** en ningún momento: se puede prender arriba de la mesa.

Guía por LED: N destellos anuncian la posición N (1 = 10 cm … 5 = 50 cm, 6 = sin pelota),
después 8 s apagado para acomodar la pelota, 2 s de parpadeo rápido para sacar la mano, y 3 s de
LED fijo midiendo. Seis posiciones, ~1 minuto y medio en total.

**La sexta posición es un control:** sin pelota, la cámara *tiene* que decir que no ve nada. Si
igual manda un número, estaba mirando otra cosa naranja y ninguna medición sirve.

> Se puso porque la otra mesa ya se comió ese problema: la cámara se queda con la mancha naranja
> **más grande** que ve, no con la más parecida a una pelota. Una silla naranja al fondo le gana a
> la pelota aunque esté mucho más lejos. En cancha limpia no pasó.

---

## 3. 🚨 El programa salió con un error, y los datos lo delataron

**El primer anuncio no se ve.** El destello único de la posición 1 sale pegado al parpadeo rápido
de la cuenta de arranque, y se confunde con él. El equipo no lo vio, y el robot midió su primer
casillero mientras ellos todavía estaban acomodando la pelota.

**Resultado: toda la tabla quedó corrida un lugar.**

Lo que llegó a la compu, tal cual:

| Casillero | paq | Xp=0 | min | max | promedio |
|---|---|---|---|---|---|
| "10 cm" | 77 | 0 | 28 | 28 | 28,0 |
| "20 cm" | 77 | 0 | 28 | 28 | 28,0 |
| "30 cm" | 77 | 0 | 58 | 58 | 58,0 |
| "40 cm" | 77 | 0 | 87 | 89 | 88,9 |
| "50 cm" | 77 | 0 | 117 | 117 | 117,0 |
| control | 77 | 0 | 142 | 142 | 142,0 |

Y el programa gritó **"VIO ALGO SIN PELOTA, las mediciones quedan en duda"**.

### Era falsa alarma, y se demostró con los propios números

Dos pistas, ninguna de las cuales hace falta ver el robot para encontrar:

1. **Los casilleros "10" y "20" dieron exactamente lo mismo: 28 y 28.** Eso solo pasa si la pelota
   estuvo en el mismo lugar las dos veces — que es exactamente lo que el equipo hizo al no ver el
   anuncio.
2. **El control leyó 142 clavado, sin moverse un punto — y 142 cae justo sobre la recta que forman
   los otros cuatro.** Una silla naranja daría un número cualquiera, sin relación con dónde está la
   pelota. Ese 142 no era una silla: **era la pelota a 50 cm.**

El equipo confirmó después el orden: **10, 10, 20, 30, 40, 50**. La medición estaba entera, solo
mal etiquetada.

> **La lección:** un dato "contaminado" no siempre está perdido. Antes de tirar una medición,
> mirá si los números tienen una estructura que explique qué pasó. Acá la estructura estaba a la
> vista: dos valores repetidos y un "control" demasiado prolijo.

---

## 4. 🎯 EL RESULTADO

Corriendo la etiqueta un lugar, la tabla real:

| Pelota (regla) | Cámara | Factor |
|---|---|---|
| 10 cm | 28 | **2,80** |
| 10 cm (repetida) | 28 | **2,80** |
| 20 cm | 58 | **2,90** |
| 30 cm | 89 | **2,97** |
| 40 cm | 117 | **2,93** |
| 50 cm | 142 | **2,84** |

**El factor es parejo.** Sube y baja tres centésimas, sin tendencia. La recta que ajusta los cinco
puntos es:

```
lo que dice la cámara  =  2,87 × centímetros reales  +  0,7
```

El error más grande contra esa recta es de 2 unidades de cámara, o sea **menos de 1 cm real**.

**Entonces: se arregla con una división por 2,87.** No hace falta tabla ni interpolación.

---

## 5. La predicción que cerró el caso

Si lo único que cambió es la altura, entonces la altura real se puede **calcular** a partir del
factor, y después ir a medirla con la regla. Si coincide, no queda nada por suponer.

La cuenta. El programa de la cámara calcula como si estuviera a **18,7 cm**, y además corrige
porque el centro de la pelota está a **~2,15 cm** del piso y no apoyado (`X = x·(h−r)/h`). Las dos
cosas juntas dan una altura efectiva de `18,7 − 2,15 = 16,55`. En la realidad manda la altura de
*esta* cámara menos ese mismo radio:

```
lo que dice / lo real = 16,55 / (altura_real − 2,15) = 2,87
altura_real = 2,15 + 16,55/2,87 = 7,9 cm
```

**Predicho 7,9 cm. Medido con regla: 7 a 8 cm.** ✅

### Segundo chequeo, de yapa

La recta pasa por **0,7** cuando debería pasar por cero. Eso confirma que la cámara está
prácticamente **al ras del frente del chasis**, que es desde donde el equipo midió con la regla.
Si estuviera 5 cm más atrás, ese número habría dado 14 en vez de 0,7.

**Caso cerrado:** la cámara se remontó ~11 cm más abajo que en 2025. Eso estira todas las
distancias por un factor parejo. No hay inclinación que corregir.

---

## Números de hoy, para no volver a medirlos

| Qué | Valor |
|---|---|
| **Factor de la cámara** | **2,87 unidades de cámara por cm real** |
| Para convertir | `cm_reales = Xp / 2,87` |
| Recta ajustada | `cámara = 2,87 · real + 0,7` |
| Error máximo contra la recta | 2 unidades de cámara (< 1 cm real) |
| Rango medido | 10 a 50 cm reales. **Fuera de ahí no está medido** |
| **Altura real de la cámara** | **~8 cm** (calibrada para 18,7) |
| Posición de la cámara | prácticamente al ras del frente del chasis |
| Paquetes por medición de 3 s | 77 → **~26 por segundo**, igual que el 04/08 |
| Estabilidad | en 4 de 6 posiciones, **mínimo = máximo**. Cero ruido en cancha limpia |
| Yp con la pelota centrada | 10 a 11 — **no da cero, ver pendientes** |

### Lo que esos números significan para los parámetros de hoy

| Parámetro | Dice | Es en cm reales |
|---|---|---|
| `umbralCm = 48` | "despeja a 48 cm" | **16,7 cm** |
| `umbralDesvio = 15` | "desvío 15 cm" | **5,2 cm** |
| `ZONA_MUERTA_PELOTA = 4` | "4 cm" | **1,4 cm** |

Y el desajuste que salta a la vista: **el robot dispara con la pelota a 16,7 cm y sale 50 cm a
despejarla.** Se aleja tres veces más de lo necesario y deja el arco solo.

---

## Qué queda pendiente

### 🔴 Lo primero la próxima vez

- ⬜ **Aplicar la conversión en el Teensy** (decisión del profe: no se toca el código de la
  cámara, que anda bien). Es un cambio de unidades **sin cambio de comportamiento**: hay que
  convertir los tres umbrales **y multiplicar `kpLateral` por 2,87**, porque si los desvíos pasan
  a ser números 2,87 veces más chicos, la fuerza del seguimiento tiene que subir lo mismo para
  que el robot se mueva igual. Si se convierte sin tocar `kpLateral`, el robot va a seguir la
  pelota mucho más flojo y va a parecer que se rompió.
- ⬜ **Después, y como decisión aparte: achicar el avance del despeje.** Hoy son 50 cm para una
  pelota a 16,7. Es una decisión de estrategia de arquero, no una cuenta.

### De la cámara

- ⬜ **El eje Y (desvío lateral) no está medido.** La teoría dice que un cambio de altura escala
  los dos ejes por igual, así que *debería* ser el mismo 2,87 — pero **eso es predicción, no
  medición**. Se mide igual que hoy: pelota corrida 10, 20, 30 cm al costado, a distancia fija.
- ⬜ **Con la pelota centrada, `Yp` dio 10-11 en vez de 0.** Puede ser que la fila de marcas no
  estuviera centrada con el robot, o que haya un corrimiento fijo de la cámara. Sin diagnosticar.
- ⬜ 🚨 **Zona muerta justo adelante.** Sigue: `Xp = 0` significa a la vez "no la veo" y "la tengo
  pegada encima". Esto la conversión **no lo arregla**.
- ⬜ Los umbrales de color son de la luz del laboratorio 2025.

### Del programa `tabla-camara`

- ⬜ **Arreglar el anuncio de la primera posición**, que es el error que corrió la tabla: falta una
  pausa oscura entre la cuenta de arranque y el primer destello, y destellos más largos para poder
  contarlos.

### De antes

- ⬜ **El robot no vuelve al centro del arco** cuando pierde de vista la pelota.
- ⬜ **Probar el giroscopio con la batería cargada.** Viene de tres clases atrás.
- ⬜ Volver a probar el límite lateral una vez que el enderezado esté fino.

---

## Lo que se aprendió del método

**Una predicción que se puede ir a medir vale más que una explicación que suena bien.**

Teníamos dos historias que explicaban igual de bien lo que veíamos: "se inclinó la cámara" y "se
bajó la cámara". Ninguna de las dos se podía descartar discutiendo. Pero cada una hacía una
**predicción distinta y comprobable**:

- la inclinación predice un factor que **crece** con la distancia;
- la altura predice un factor **parejo**, y además dice **exactamente a qué altura** tiene que
  estar la cámara.

Cinco puntos con la regla contestaron la primera pregunta, y una regla contra el chasis contestó
la segunda. Ninguna de las dos mediciones llevó más de dos minutos.

> Cuando tengan dos explicaciones posibles, no busquen cuál suena mejor: busquen **en qué se
> contradicen**, y midan eso.
