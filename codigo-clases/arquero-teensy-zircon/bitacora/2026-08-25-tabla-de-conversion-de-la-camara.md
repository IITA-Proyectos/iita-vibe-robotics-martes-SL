# 2026-08-25 — La cámara no estaba inclinada: estaba 11 cm más abajo

**Quiénes:** Gustavo Viollaz + los chicos (cancha) + Claude (código, carga y lectura del serie)
**Robot:** el **ARQUERO** — ver [identificación](2026-07-28-identificacion-arquero.md)
**Programa nuevo:** `pruebas/tabla-camara`
**Cuenta nueva de Claude.** Esta carpeta pasa a ser el espacio de trabajo del arquero; el
delantero lo sigue llevando la otra mesa y no se toca.

**Resumen del día:** se cerró el pendiente que venía trabado hace dos clases. La cámara exagera
las distancias por un **factor parejo de 2,87**, y la causa quedó identificada y confirmada con
regla: **la cámara está montada a ~8 cm del piso, no a los 18,7 cm para los que fue calibrada.**
Con eso, todo el programa pasó a hablar en centímetros reales, el despeje se subió a 30 cm, y se
empezó la **ubicación inicial**: que el robot se acomode solo en el centro de su arco tomando de
referencia el arco del rival.

| | |
|---|---|
| §1 a §5 | La cámara: cuánto exagera, por qué, y cómo se confirmó |
| §6 | La conversión aplicada en el Teensy, y el despeje a 30 cm |
| §7 | Ubicación inicial usando el arco del rival |
| §8 | 🚨 El tanteo automático del signo: falló, y por qué |

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

## 6. La conversión, aplicada en el Teensy

**Decisión del profe: el código de la cámara no se toca.** Dos razones, y la segunda pesa tanto
como la primera:

- La cámara anda bien en todo lo demás: 26 paquetes por segundo, sin errores, arranca sola.
- **Ese script es el mismo que usa el delantero.** Tocar la matriz para que ande en nuestro robot
  le rompería las lecturas a la otra mesa.

Así que la traducción va en el Teensy, apenas llega el número.

### El cambio de unidades, que NO cambia el comportamiento

Esta parte es importante entenderla: traducir de unidades de cámara a centímetros reales **no
cambia nada de lo que hace el robot**. Es como pasar de pulgadas a centímetros — la mesa mide lo
mismo, cambia el número.

| Parámetro | Antes | Ahora | ¿Cambia algo? |
|---|---|---|---|
| `umbralDesvio` | 15 cámara | 5,2 cm reales | no |
| `ZONA_MUERTA_PELOTA` | 4,0 cámara | 1,4 cm reales | no |
| `kpLateral` | 4,0 | **11,5** | no |

⚠️ **El que se puede pasar por alto es `kpLateral`.** Si los desvíos ahora son números 2,87 veces
más chicos, la fuerza que los multiplica tiene que ser 2,87 veces más grande para que el robot
empuje igual: `4,0 × 2,87 = 11,5`. Quedó avisado en el código, porque a alguien le va a parecer
un error y va a querer "corregirlo" de vuelta a 4 — y ahí el robot va a seguir la pelota casi
tres veces más flojo.

### Y un cambio que SÍ es de comportamiento, pedido por el equipo

| | Antes | Ahora |
|---|---|---|
| `umbralCm` | 48 cámara = **16,7 cm reales** | **30 cm reales** |

El robot esperaba a tener la pelota casi encima. Ahora sale a buscarla al doble de distancia.

⚠️ Efecto secundario a mirar en cancha: **disparar de más lejos lo vuelve más exigente con la
alineación.** Los mismos 5,2 cm de desvío tolerado, vistos desde 30 cm en vez de 16,7, son un
ángulo bastante más angosto. Si se lo ve dudar y no despejar, `umbralDesvio` es el primer número
a tocar.

---

## 7. Ubicación inicial: acomodarse solo en el centro del arco

**Pedido del profe:** apoyar el robot en cualquier lado cerca de su arco y que se acomode solo en
el centro de la línea, usando el **arco azul** de referencia. Y que **todos los movimientos usen
el giroscopio**.

### El arquero no puede ver su propio arco

Se para con su arco a la espalda y la cámara mira para adelante. **Lo único que ve es el arco del
RIVAL** (el azul, confirmado por el equipo).

No importa: **los dos arcos están sobre la misma línea central de la cancha**, así que centrarse
con el de enfrente centra al robot en el suyo. Es pararse en el medio de un pasillo mirando la
puerta del fondo.

### Dos movimientos, dos referencias distintas

| Movimiento | Referencia | ¿Existía? |
|---|---|---|
| Para **atrás**, hasta la línea | los dos sensores de atrás | ✅ es el regreso del despeje |
| De **costado**, hasta centrarse | el arco azul, por la cámara | 🆕 |

**Va primero el retroceso.** Moverse de costado no cambia la distancia a la línea, así que
centrarse *después* deja la posición final buena en las dos cosas a la vez.

Lo del giroscopio ya estaba resuelto por la arquitectura: `moverDeCostado()`,
`adelanteControlado()` y `atrasControlado()` **suman la corrección de rumbo** a las tres ruedas.
Cualquier movimiento escrito con esas funciones lo hereda gratis.

### Por qué el desvío del arco NO se convierte a centímetros

El arco está lejísimos, muy fuera de los 10-50 cm donde medimos la conversión, y además la cámara
recorta `X` en 200. Convertirlo sería inventar precisión.

Y no hace falta: **para centrarse no importa cuántos centímetros estás corrido, importa para qué
lado y dónde está el cero.** Un número deformado cruza el cero exactamente en el mismo lugar. Es
usar una balanza descalibrada para saber cuál de dos bolsas pesa más: no te dice el peso, pero te
dice cuál.

Por eso el centrado **no depende de la calibración rota**, al revés que el despeje.

---

## 8. 🚨 El tanteo automático del signo: falló, y por un motivo conocido

Como en la cancha no hay cable para apretar teclas, la primera versión hacía que el robot
**descubriera solo** de qué lado es cada signo: se movía 600 ms hacia donde creía y comparaba si
el desvío había bajado o subido. Si subía, daba vuelta el signo.

### Qué pasó en cancha

> *"Encontró la línea al retroceder, después estaba yendo para el lado correcto, y después de
> medio segundo comenzó a alejarse cada vez más al lateral."*

El robot **arrancó bien** y el tanteo le dio vuelta el signo igual. Después se fue de lado sin
parar.

### Por qué falló

**El arco está lejísimos.** Cuando el robot se corre 20 cm, la posición del arco en la imagen
cambia muy poco. Comparar una lectura contra otra con un cambio tan chico **es comparar ruido**.

> Es el mismo error que la prueba del freno sobre la mesa el 18/08: **un experimento que no puede
> distinguir las dos respuestas no es una medición**, por más prolijo que sea el procedimiento.
> Ya nos pasó dos veces con el mismo formato. Vale la pena tenerlo a mano.

### Y la medición buena estaba en la observación del equipo

*"Iba para el lado correcto"* contesta la pregunta mejor que cualquier tanteo automático. **El
signo del arco es el mismo que el de la pelota**, tal como se deducía leyendo el código de la
cámara: los dos números salen de la misma función y la misma matriz, y solo cambia el color que
buscan.

### Los tres arreglos

1. **Signo fijo**, `arcoInvertido = false`. Se sacó el tanteo. Queda la tecla `A` para darlo
   vuelta a mano si alguna vez hiciera falta.
2. **Lectura suavizada** (filtro exponencial, peso 0,3 a lo nuevo), porque el arco lejos tiembla.
3. **Protección contra fuga:** si el desvío empeora más de 15 unidades respecto de cómo arrancó,
   el robot **para y avisa**. Ya no se puede ir caminando. En el peor caso queda quieto en el
   lugar equivocado, que es un problema mucho menor que un robot que se va de la cancha.

⚠️ **Al cierre de la clase esto quedó cargado y en prueba. Falta anotar el resultado.**

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

- ⬜ **Anotar cómo salió la ubicación inicial.** Quedó cargada y en prueba al cierre. Las
  preguntas: ¿llega al centro y se queda? ¿se pasa y vuelve (`kpArco` alto)? ¿frena corrido
  (`ZONA_MUERTA_ARCO` alta)? ¿saltó el aviso de "me estoy alejando"?
- ⬜ **Probar el despeje a 30 cm.** Se cargó pero no se llegó a probar en el día.
- ⬜ **Achicar el avance del despeje.** Hoy sale ~50 cm para una pelota que está a 30. Es una
  decisión de estrategia de arquero, no una cuenta.
- ⬜ **Volver al centro cuando pierde la pelota**, usando el mismo arco azul. Es la segunda mitad
  del pedido del profe; hoy se hizo solo la ubicación inicial.

### De la ubicación inicial

- ⬜ **`kpArco = 2,0` y `ZONA_MUERTA_ARCO = 6` están puestos a ojo.** No hubo tiempo de medir.
- ⬜ **`ver-arcos` quedó escrito y sin correr.** Mide qué da `Yaz` con el robot a la izquierda,
  al centro y a la derecha del arco, con la estabilidad de la lectura. Sigue siendo la forma
  prolija de ajustar los dos números de arriba.
- ⬜ **El rumbo base se fija donde el robot esté apuntando al arrancar.** Si lo apoyan torcido,
  "derecho" queda torcido para toda la corrida. Con el arco azul se podría fijar una referencia
  de verdad, pero eso pide que el robot gire, y hoy nunca gira.

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

## 9. 🚨 En la cancha el robot es MUDO — y medio programa estaba escrito olvidándolo

Lo levantó el equipo al final de la clase:

> *"Siempre decís lo de avisos o teclas por terminal, pero no podemos ver la terminal ni tampoco
> usarla si está desconectado del USB."*

Es exacto, y es peor de lo que parece: **el Teensy no guarda lo que imprime cuando no hay nadie
escuchando.** Con el USB desenchufado, cada `Serial.println` se descarta en el momento. No quedan
esperando para cuando uno se conecte: se perdieron.

O sea que todos los avisos del firmware — *"no encontré la línea"*, *"ME ESTOY ALEJANDO"*,
*"centrado, desvío final ..."* — **no los va a leer nadie nunca**, porque solo ocurren en la
cancha. Y las teclas que el programa ofrece para arreglar algo en el momento (`A`, `f`/`F`,
`x`/`z`) tampoco sirven ahí.

### Lo raro es que la solución ya la teníamos

Los programas de medición (`calibrar-linea`, `tabla-camara`, `ver-arcos`) están escritos con el
patrón correcto: **el robot mide solo, se acuerda en memoria, y recién se le pregunta con una
tecla cuando vuelve a la compu.** El firmware de juego no lo usa.

### Los dos únicos canales que existen en la cancha

| Canal | Cuándo | Para qué |
|---|---|---|
| **El LED del robot** | en vivo | en qué fase está, y si algo salió mal |
| **La memoria + una tecla** | después, en la mesa | qué pasó durante toda la corrida |

Y hay un tercero que descubrimos hoy y no estábamos usando: **el LED de la cámara**, que es un
LED RGB de tres canales y dice qué está viendo (rojo = pelota, verde = arco amarillo, azul = arco
azul, y se mezclan). Sirve, por ejemplo, para saber si hay algo naranja de fondo antes de medir.

⬜ **Pendiente que sale de esto:** que `seguir-y-despejar` lleve un resumen en memoria de lo que
pasó en la corrida (¿encontró la línea?, desvío del arco al empezar y al terminar, ¿saltó la
protección de fuga?, cuántos despejes) y lo cuente con una tecla al volver a la mesa. Y que los
avisos importantes tengan **código de LED**, no solo texto.

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
