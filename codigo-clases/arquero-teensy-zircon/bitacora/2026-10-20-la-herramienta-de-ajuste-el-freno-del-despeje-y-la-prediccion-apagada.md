# 2026-10-20 (martes) — La herramienta de ajuste, el freno del despeje y la predicción apagada otra vez

**Quiénes:** Diego y Laureano (cancha, pruebas y pedidos)
+ Claude (herramienta, código y carga)
**Robot:** el **ARQUERO** — Teensy con número de serie `13348550`
**Programas nuevos:** `pruebas/ajustar-movimientos` (con `AJUSTAR.bat` y `MIRAR.bat`),
`pruebas/herramientas/cargar.py`, `funciona/seguir-y-despejar/CARGAR-PARTIDO.bat`
**Modificados:** `funciona/seguir-y-despejar` (0.1020), `pruebas/herramientas/mirar.py`,
README del arquero y de herramientas, y el mapa de la EEPROM en `medir-ancho` y `derecho-y-vuelta`
**PC:** la notebook que viaja (usuario `sarap`). Se compiló igual que el 13/10: plataforma
`teensy` 6.0.0, Teensyduino 1.62.

---

## 🎯 EL RESUMEN DEL DÍA

**1. Hay una herramienta para ajustar la prolijidad sin escribirle a nadie** (§1). El robot hace
un movimiento con el código del partido, el giroscopio mide, y dice qué número probar tocar.
Los números se cambian con teclas, sin compilar.

**2. Por primera vez hay grados de cómo se mueve el robot con los motores nuevos** (§2). Con los
números del partido: de costado lento y el retroceso solo, prolijos. De costado rápido y el
despeje, no.

**3. El despeje ahora frena y espera 400 ms antes de volver** (§3). Pedido del equipo: al llegar
al tope de la ida el robot se inclinaba y recién después volvía al piso.

**4. La predicción se apagó otra vez** (§4). Con la pelota quieta enfrente, prendida hacía ir y
venir al robot. Es la pregunta abierta del 12/10, y la respuesta es que sí era la predicción.

**5. En el robot quedó la `0.1020 EN PRUEBA`** (§5), cargada al final de la clase y **sin probar
en cancha**.

---

## 1. La herramienta `pruebas/ajustar-movimientos`

**Para qué:** el equipo preguntó cómo corregir sin escribirle a Claude cada vez, "porque seguro
serán muchas". Es también uno de los pendientes del profe del 13/10 (la herramienta para la
corrección de rumbo, con el autodiagnóstico por giroscopio).

**Cómo se usa:** doble clic en `AJUSTAR.bat` (carga la herramienta y abre la consola), el robot en
el piso de la cancha, y una tecla:

| Tecla | Movimiento |
|---|---|
| `1` / `2` | de costado izquierda / derecha, lento (70) |
| `3` / `4` | de costado izquierda / derecha, rápido (150, el tope del seguimiento) |
| `5` | ida y vuelta rápido |
| `6` | despeje: ida, pausa, retroceso hasta la línea |
| `7` | retroceso solo hasta la línea |
| `8` | seguir la pelota 10 s (no despeja) |

Al terminar dice la torcida máxima y cuándo, los cruces de un lado al otro, cómo terminó, la
corrección promedio, un dibujo del rumbo, una nota (PROLIJO / CASI / ZIGZAGUEA / SE TUERCE) y qué
número probar. `n`/`b` eligen un número, `+`/`-` lo cambian (queda guardado en la EEPROM, en
1024..1535), `v` imprime qué copiar al partido, `x` vuelve a los del partido.

**Lo que salió de leer las bitácoras para armarla:** `KP_RUMBO` 5 y `KI_RUMBO` 1,5 se ajustaron el
04/08 en el cuadrado, cuando la corrección iba **solo a la rueda de atrás**. En el partido va a las
tres ruedas y nunca se volvieron a ajustar.

**La revisión.** Cuatro miradas independientes la revisaron mientras el equipo ya la usaba. Lo
confirmado y arreglado en la misma clase:

- 🚨 **Apretar dos veces el número para frenarlo armaba otra cuenta y el robot volvía a arrancar**.
  Ahora tira lo tecleado y espera 1,5 s.
- 🚨 **Después del despeje o del retroceso el robot queda sobre la línea del área, y de ahí, yendo
  de costado, no podía ver el lateral.** Ahora no arranca de costado si un sensor de atrás ve
  blanco.
- Ctrl+C manda un `0` antes de cerrar la consola. Una sola lectura muda del giroscopio ya no anula
  la prueba. El informe muestra los números **con los que se corrió**. Avisa que `rampaPaso`,
  `potenciaDespeje` y `msAdelante` cambian cuánto sale el despeje. El cargador avisa si otra
  ventana tiene tomado el robot.
- **El dibujo del rumbo estaba espejado** en la versión que se usó para §2 (el giroscopio cuenta
  positivo en sentido horario, bitácora del 28/07). Ahora el `*` va para donde apunta el robot. En
  los números: **`+` = se torció a la izquierda**.
- Un cruce ahora pide pasar de +2 a −2 grados (era 1): siguiendo la pelota contaba como zigzag
  bamboleos de 1 o 2 grados.

**El monitor (`mirar.py`) ya no se cierra al desenchufar el cable:** espera y se reconecta solo.
Lo pidió el equipo, para las pruebas sin cable.

---

## 2. La línea de base: los números del partido, sin tocar nada

Corridas del equipo con la versión de la herramienta anterior a los arreglos. Valores del
partido: `kpRumbo` 5, `kiRumbo` 1,5, rampa 10, despeje 200 × 383 ms, retroceso 110, y la
**predicción prendida**. Grados con signo: `+` = izquierda.

| Movimiento | Corridas | Nota | Torcida máxima | Cómo terminó | Cruces (banda 1°) |
|---|---|---|---|---|---|
| 1 costado izq. lento | 1 | PROLIJO | −3,3° | −0,7° | 0 |
| 2 costado der. lento | 1 | PROLIJO | +3,2° | +2,2° | 0 |
| 3 costado izq. rápido | 1 | ZIGZAGUEA | +10,8° | +10,8° | 5 |
| 4 costado der. rápido | 1 | ZIGZAGUEA | −19,4° | −18,9° | 5 |
| 6 despeje | 6 | 4 ZIGZAGUEA, 2 SE TUERCE | +10,5 / −24,8 / +10,1 / +7,5 / −16,9 / +10,9° | +9,0 / +6,5 / +7,9 / +1,9 / −0,4 / −2,3° | 2 / 4 / 4 / 4 / 6 / 2 |
| 7 retroceso solo | 2 | PROLIJO | +1,6 / +2,3° | +0,6 / +0,8° | 0 / 0 |

La vuelta del loop más lenta fue de 2 a 3 ms en todas: **no hay ningún retardo escondido** como el
del 06/10. El giroscopio contestó en todas.

**Lo que dicen, leído por Claude (son interpretaciones, no mediciones aparte):**

1. **De costado lento: bien.** Hasta 3,3° y sin zigzag. La torcida promedio cambia de signo con el
   lado (−1,3° yendo a la izquierda, +1,0° a la derecha), igual que la corrección promedio (−6 y
   +4): es la mezcla 50/50/89, pero chica.
2. **De costado rápido: dos problemas distintos.** Mientras va, ±4° unas dos veces por segundo
   (eso sí es la corrección de rumbo). Y al **frenar** gira de golpe: +10,8° yendo a la izquierda y
   −19,4° yendo a la derecha, cuando ya terminó de moverse. Eso es frenar en seco a 150, no la
   corrección. En el partido frena así al pisar un lateral y en la patrulla.
3. **El despeje se tuerce en el cambio de ida a vuelta.** En las 6 corridas la peor torcida fue
   entre 0,3 y 0,6 s: la ida termina a los 383 ms, ahí soltaba los motores yendo a 200 y arrancaba
   para atrás de golpe a 110. En 2 corridas quedó yendo y viniendo 15 a 20° durante todo el
   retroceso. El retroceso solo (7), que arranca quieto y con rampa, dio 2° las dos veces.
4. **Siguiendo la pelota** (5 corridas): con la pelota moviéndose, de 11 a 26 cambios de lado en
   10 s y bamboleos de 2 a 4°. En una corrida con la pelota quieta (la cámara midió 0,3 cm/s)
   cambió una sola vez de lado. Esa corrida es rara: estuvo los 10 s corrigiendo sin entrar nunca
   en la zona muerta, torcido fijo 1,1°. **Falta que el equipo diga si el robot se movía o estaba
   quieto zumbando** (si zumbaba, es `pwmMinLateral = 45`, que de costado no arranca desde parado).

---

## 3. El despeje frena antes de volver

**Lo que vio el equipo:** "cuando llega a su tope hace como una inclinación y recién vuelve al
piso".

**El cambio** (`frenarAntesDeVolver`, `MS_ESPERA_ANTES_DE_VOLVER`): al terminar la ida **frena**,
se queda frenado y recién ahí retrocede. Antes soltaba los motores 150 ms.

**Cómo se eligió el número:** se puso 100 ms, el equipo lo probó en la herramienta (movimiento 6)
y subió a **400 ms**: *"funciona decentemente mejor"*. **No quedaron anotados los grados** de esas
corridas. Cuesta 0,25 s más de despeje que antes.

Para volver: `frenarAntesDeVolver = false`.

---

## 4. La predicción, apagada otra vez

**Lo que vio el equipo:** con la pelota **quieta enfrente**, el robot iba y venía sin centrarse, y
de tanto corregir se alejaba del arco; en la herramienta y en el programa de partido nuevo, pero
no en el viejo. Con la predicción apagada en la herramienta: *"funciona mucho mejor"*.

**Por qué, según los números de §2:** en una de esas corridas la cámara midió que la pelota "se
movía" a 6,2 cm/s, más que el piso de ruido de 4, y el robot cambió 11 veces de lado. Es la
hipótesis del 12/10: la cámara va arriba del robot, así que cuando el robot se mueve la pelota
parece moverse en la imagen; la predicción toma eso como velocidad de la pelota y lo manda más
lejos.

**Lo que no se arregla con esto:** que se aleje del arco. Yendo de costado el robot se va corriendo
para adelante y nada lo vuelve a la línea del área hasta la patrulla (ver pendientes).

Para volver: `predecirTrayectoria = true`.

---

## 5. Lo que quedó en el robot

| Cartel | Estado |
|---|---|
| `0.1020 EN PRUEBA (ESTABLE 0.1013 + frena 400 ms antes de volver; prediccion apagada)` | 🚨 **LA QUE ESTÁ EN EL ROBOT.** Cargada al final de la clase, sin probar en cancha |

Es la ESTABLE 0.1013 (`respaldos/estable1013conlimiteslaterales/`) con **una sola diferencia**: el
freno de 400 ms antes de volver del despeje. La predicción quedó apagada, igual que en la estable.

---

## 6. Otras cosas que se vieron

- **La cámara mide distinto a cada lado** (con `ver-prediccion`, al empezar la clase): con la
  pelota al medio dijo 50 cm, movida a la izquierda 37 cm y a la derecha 54 cm. El equipo cree que
  la dejó a la misma distancia, pero no se midió con regla. La conversión a cm (2,87) se midió solo
  con la pelota enfrente; el costado nunca se midió.
- **La cámara mandó 16 paquetes por segundo**; en las bitácoras viejas eran 25 o 26.

---

## 7. Decisiones del equipo

- Con la pelota a 55 cm o más, que **deje de seguirla de costado** y vuelva al medio **como la
  patrulla** (yendo al lateral y volviendo por tiempo); a 50 cm o menos, que la siga otra vez.
  **No está hecho:** el equipo eligió hacer primero la herramienta.
- Andar con los sensores de atrás sobre la línea del área: **para más adelante**, cuando los
  movimientos estén prolijos.
- "Primero nos centremos en la estabilidad y prolijidad."

---

## 8. Errores del día (míos)

1. **El equipo usó la herramienta antes de que terminara la revisión**, por el apuro. La revisión
   encontró dos problemas de seguridad que estaban en esa versión (el doble arranque y el lateral
   ciego sobre la línea). Avisé apenas los vi y se arreglaron en la misma clase, pero las corridas
   de §2 se hicieron con ellos.
2. **El dibujo del rumbo estaba espejado** en esa misma versión. Los números están bien; el `*` de
   las corridas de §2 va para el lado contrario.
3. **Dejé el monitor (`mirar.py`) roto unos minutos**, a mitad de un cambio: si alguien lo abría
   en ese rato, no arrancaba.
4. **La primera nota del seguimiento exageraba el zigzag**: contaba cruces de 1°.

---

## 9. Qué queda pendiente

### 🔴 Lo primero, en la cancha, con la 0.1020

- 🔴 **El despeje con el freno de 400 ms:** ¿sale derecho y llega a tocar la pelota? Anotar los
  grados con la herramienta (movimiento 6, tres veces).
- 🔴 **La pelota quieta enfrente con la predicción apagada:** ¿se queda quieto? (movimiento 8:
  "idas y vueltas" tiene que dar casi 0).
- 🔴 **¿El robot zumbaba quieto** en la corrida de §2 con la pelota quieta?

### ⬜ Con la herramienta

- ⬜ **De costado rápido, mientras va:** `kpRumbo` 4,5 y los movimientos 3 y 4, tres veces cada uno.
- ⬜ **El giro al frenar de costado a 150** (11 a 19°): no se arregla con números de la
  herramienta. Idea: bajar la velocidad antes de frenar. Es un cambio de código.

### ⬜ Del equipo y del profe

- ⬜ **La regla de 55/50 cm** (§7).
- ⬜ **Que no se aleje del arco:** andar con los sensores de atrás sobre la línea del área.
- ⬜ **Todo lo ajustable arriba del programa de partido.** El delantero ya lo hizo el 20/10
  ("panel de control"): se puede copiar la forma.
- ⬜ **El `.bat` de preparación de cancha sin internet.** La herramienta de ajuste es una primera
  parte.
- ⬜ **La cámara a cada lado:** cinta en el piso a 50 cm, pelota a la izquierda, al medio y a la
  derecha, y anotar lo que dice `ver-prediccion`.

### ⬜ Arrastres

- ⬜ **Revisar el código del límite lateral** (del 13/10).
- ⬜ **Medir con los motores nuevos** (`medir-ancho`, el tablero, `probar-ruedas`, la regla).
- ⬜ **El script de la cámara** sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
- ⬜ `cargar.py` no reconoce un Teensy en modo carga (lo encontró la revisión).
