# 2026-11-03 (martes) — El programa del 21/09 vuelve a la cancha

**Quiénes:** Diego y Laureano (cancha, soldadura, pruebas y pedidos)
+ Claude (código y carga)
**Robot:** el **ARQUERO** — Teensy con número de serie `13348550`
**Programas nuevos:** `funciona/partido-0921/` (**el que se usa para jugar**, versión 0921f),
`funciona/partido-0921c/` y `partido-0921d/` (versiones intermedias),
`respaldos/partido-2026-09-21/` (copia exacta del 21/09)
**Modificados:** `funciona/seguir-y-despejar` (0.1103: registro de despejes), el README del
arquero y el mapa de la EEPROM en `medir-ancho`, `derecho-y-vuelta` y `ajustar-movimientos`
**En construcción, no subido:** `pruebas/tablero` (la ventana con botones)
**PC:** la notebook que viaja. Plataforma `teensy` 6.0.0, Teensyduino 1.62.

---

## 🎯 EL RESUMEN DEL DÍA

**1. El giroscopio quedó soldado** (§1): ya no tiene la cinta. Se probó con `probar-giroscopo`, y
la cámara con `ver-prediccion`. Las dos andan.

**2. El equipo volvió al programa de los amistosos del 21/09** (§3): lo probaron contra la 0.1103 y
*"creemos que esta funciona mejor"*. Desde hoy se juega con `funciona/partido-0921/`, y se le fueron
haciendo cambios a pedido, de la b a la f (§4).

**3. En el robot quedó la 0921f** (§8). La última vuelta en cancha fue con la 0921d; la e y la f se
cargaron al final y **no se llegaron a probar**.

**4. El tablero se está terminando** (§7), y el equipo pidió adaptarlo a la 0921 y poder cambiar
valores y cargar el programa desde ahí.

**5. Tienen partido el domingo o el lunes**, y no hay más clase antes.

---

## 1. El giroscopio soldado y la cámara

- **Giroscopio:** lo soldaron (antes iba con cinta; el 29/09 el problema había sido el cable). Con
  `probar-giroscopo` (el que no mueve el robot) contestó.
- **Cámara:** con `ver-prediccion`, primero dijo *"LA CAMARA NO ESTA HABLANDO"*: la batería estaba
  apagada (la cámara se alimenta de la batería, no del USB). Con la batería prendida, anduvo.

---

## 2. La 0.1103: el registro de despejes

**Por qué:** el 27/10 se pidió pasar las líneas `MIRE ...`, y el equipo contestó *"en cancha no
tenemos USB"*. La **0.1103** es la 0.1027d con un registro: anota los últimos 40 intentos de despeje
(si salió o por qué no, distancia, costado y las velocidades medidas) y los guarda en la EEPROM
(**3072 .. 3583**) solo con el robot quieto a propósito. Al enchufar y abrir MIRAR.bat los imprime
con un resumen.

Se cargó y arrancó bien (registro vacío, encendido 1). **No se llegó a usar**: el equipo cambió de
programa (§3). El registro está solo en la 0.1103, no en la 0921.

---

## 3. El programa del 21/09

El equipo pidió *"el programa del primer partido"*: el de los amistosos del 21/09 (commit
`d93aaaf`). Se sacó de GitHub a `respaldos/partido-2026-09-21/` (copia exacta, no se toca) y se
cargó. Diferencias con la 0.1103 (comparadas con un script, no a ojo):

| | 21/09 | 0.1103 |
|---|---|---|
| Ida del despeje | 533 ms (~50 cm) | 383 ms (~35 cm) |
| Costado máx / mín | 120 / 60 | 150 / 45 |
| Margen al costado para salir | 7 cm | 20 cm |
| Predicción | prendida (250 ms) | apagada |
| Diagonal | apagada | prendida |
| Freno antes de volver | no (suelta 150 ms) | sí (400 ms) |
| Patrulla, límite lateral, ir al medio | no | sí |

Iguales: fuerza de la ida 200, retroceso 110, despeja a 30 cm, `kpLateral` 11,5, zona muerta
1,4 cm, rampa 10, rumbo KP 5 / KI 1,5, umbral de línea 425.

---

## 4. La 0921, de la b a la f

Todo en `funciona/partido-0921/`. Cada cambio fue un pedido del equipo:

| Versión | Qué se le hizo | Lo que dijo el equipo |
|---|---|---|
| **b** | Costado máx 120 → **140**. La **predicción borrada entera** del código (no apagada). Margen 7 → **20 cm**. Antes de volver, **espera a que el giroscopio lo vea plano** (§5) y la vuelta **arranca con rampa** | — |
| b | Costado máx → **160** | — |
| b | Costado mín 60 → **80** | — |
| b | Ida 533 → **600 ms** (y el tope de la vuelta `MS_MAX_RETROCESO` 1200 → **1500**) | — |
| b | Costado mín → **70**, ida de vuelta a **533** (el tope quedó en 1500) | — |
| **c** | + límite lateral, patrulla e ir al medio al arrancar (copiados de la estable 0.1013) + la ida desde 70 (§6) | *"creo que el anterior programa estaba mejor"* → se volvió a la b |
| **d** | La b + **corregir con la cámara en la ida** (`perseguirEnElDespeje`, §6) + **mínimo 300 ms** quieto antes de volver | — |
| **e** | La d + **límite lateral** y **patrulla** (de la c) | — |
| **f** | La e + **ir al medio al arrancar** otra vez | sin probar |

Al volver de la c a la b, la b se rearmó desde el 21/09 con un script, y se comprobó que todos los
números coincidieran.

---

## 5. El retroceso que oscilaba

**Lo que vio el equipo:** *"El despeje va derecho pero en el retroceso oscila demasiado. Creemos que
es porque al frenar de golpe el robot se inclina y más se intenta retroceder de inmediato. ¿Se puede
usar el giroscopio para detectar que dejó de estar inclinado?"*

**Sí:** el BNO055 también mide la inclinación. En la 0921b:
- **Espera de plano** (`esperarPlanoAntesDeVolver`): mientras espera la pelota, el robot aprende su
  inclinación "de siempre". Después de la ida no vuelve hasta estar a 1,5° de esa inclinación
  durante 100 ms seguidos. Mínimo 150 ms (300 desde la d), máximo 800 ms. Sin giroscopio, vuelve
  al mínimo.
- **La vuelta con rampa** (`rampaAlVolver`): en el 21/09 la vuelta arrancaba de golpe a 110 para
  atrás. El 20/10 la herramienta midió que el retroceso solo, arrancando quieto y con rampa, se
  tuerce 2°; pegado a la ida, de 15 a 20°.

**La revisión** (7 agentes, solo leyendo): **ningún error confirmado**. Quedaron dos avisos: con
20 cm sale a pelotas que derecho no toca, y "plano" no es lo mismo que "frenado" (puede estar
plano todavía yendo para adelante, y entonces lo que ayuda es la rampa).

**No quedó medido** si la oscilación mejoró.

---

## 6. El saltito a la izquierda, y corregir con la cámara

**Lo que vio el equipo:** *"sale a despejar, hace un tirón recto, se mueve un poco a la izquierda y
vuelve a seguir derecho"*, en todos los despejes, **también con la pelota quieta**.

**La teoría de Claude** (no medida): no es el envión de venir de costado (pasa con la pelota quieta).
Las dos ruedas de adelante no arrancan exactamente juntas (una rueda parada necesita ~70). El robot
gira un poquito, y el giroscopio lo endereza sumándole también a la rueda de atrás, que es la que
empuja de costado: el saltito. **El arreglo probado** fue la ida con la rampa desde 70
(`pisoRampaIda`) en la c; el equipo volvió a la b sin decir qué parte de la c no le gustó. En la e y
la f está en 0.

**Corregir con la cámara** (pedido: *"que con la camarita vaya viendo la pelota y con eso se vaya
centrando para pegarle mejor"*): ya estaba escrito desde el 08/09 y nunca se había prendido
(`perseguirEnElDespeje`). En la ida, mientras ve la pelota, se corre hacia ella (6 por cm, tope 60).
Prendido desde la d.

---

## 7. El tablero

Copiado del que subió el delantero el 27/10. La sesión del 27/10 se cerró con el armado a medio
hacer, y hoy se rearmó: entender → diseñar → construir → revisar → arreglar. Hoy se abrió el
**demo** (`TABLERO-DEMO.bat`, robot de mentira): la autoprueba dio **OK, 4106 verificaciones**.
Tiene PELOTA, SENSORES, MOVIMIENTOS, **CANCHA** (preparar una cancha nueva sin internet) y CONSOLA,
y **modo sin cable**.

**Problema:** se diseñó para la 0.1103, antes de que el equipo cambiara a la 0921. **Pedido del
equipo para antes del partido:** adaptarlo a la 0921f, poder **cargar el programa desde el
tablero**, y una pestaña nueva para **ver y cambiar los valores del programa de partido** (por
ejemplo el umbral del sensor) y cargarlo con esos valores. **No está subido** en este commit.

---

## 8. Lo que quedó en el robot

| Versión | Estado |
|---|---|
| **0921f** (`funciona/partido-0921/`) | 🚨 **LA QUE ESTÁ EN EL ROBOT.** Cargada al final, **sin probar**. No escribe nada por el cable |

**Probada en cancha hoy:** la 0921d (sin límite lateral, patrulla ni ir al medio).

---

## 9. Errores del día (míos)

1. **El tablero se reinició tres veces**: para agregar CANCHA, porque yo había pedido que el robot se
   frenara al desenchufar el cable (en la cancha no tienen USB, así que eso estaba mal), y porque la
   sesión del 27/10 se cerró a la mitad. Por eso tardó mucho más que el del delantero.
2. **Se diseñó para un programa que hoy dejó de usarse.** Hay que adaptarlo.
3. **Corté a la mitad la verificación de la c** para cargarla rápido. Quedaron casos de la patrulla
   sin confirmar (§10).
4. Un error de compilación por una llave de más al borrar la predicción. Se vio al compilar y no
   llegó al robot.

---

## 10. Qué queda pendiente

### 🔴 Antes del partido (domingo o lunes)

- 🔴 **Probar la 0921f en cancha.** Que el límite lateral frene antes de salirse **a 160** (en la
  0.1013 se usó a 150), que la patrulla vuelva al medio, y que ir al medio al arrancar termine
  centrado.
- 🔴 **`MS_MEDIO_AL_MEDIO` (2228 ms) se midió el 06/10, antes de mover los motores.** Si ir al medio,
  la patrulla o el límite lateral no lo dejan centrado, hay que volver a medirlo.
- 🔴 **El tablero adaptado a la 0921f**, con la pestaña para cambiar valores y cargar.
- 🔴 **Con margen de 20 cm sale derecho a pelotas que no toca.** Si deja el arco solo de gusto,
  bajar `umbralDesvio` a 10 o 12.

### ⬜ Para mirar en cancha

- ⬜ **El saltito a la izquierda.** Si sigue, probar `pisoRampaIda` en 70 de nuevo, solo eso.
- ⬜ **La corrección por cámara**: ¿le pega más centrado? ¿Hace eses? (bajar `kpPersecucion` 6 → 4).
- ⬜ **¿El retroceso dejó de oscilar?** Comparar con `esperarPlanoAntesDeVolver = false` y con
  `rampaAlVolver = false`, de a uno.
- ⬜ **Al arrancar, si encuentra la línea, avanza ~10 cm** (`avanzarAlUbicarse`), igual que la
  0.1013. La 0921b no lo hacía.

### ⬜ Sin verificar (vienen de la 0.1013)

- ⬜ La patrulla arrancando sobre un lateral; un tramo de la patrulla que va 2,2 s de costado sin
  mirar líneas; sin giroscopio no hay límite lateral (sigue la pelota a 160 hasta la madera).

### ⬜ Arrastres

- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
- ⬜ `cargar.py` no reconoce un Teensy en modo carga.
