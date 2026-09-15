# 2026-09-15 — Cancha de tela, y el mapa de sensores estaba mal desde siempre

**Quiénes:** Máximo (cancha y mesa) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680` · placa Mark1
**Programa cargado al cerrar:** `funciona/delantero/delantero.ino`, con la patada en **modo
prueba lenta** (`VEL_PATADA = 110`)

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🆕 **Cancha nueva: de TELA** | el verde se desplomó de ~600 a ~80. Todo lo medido antes quedó sin valor |
| 🔴 **El robot estaba CIEGO a la línea** | con los umbrales viejos, el sensor delantero no la vio **ni una vez** |
| ✅ **Umbrales re-medidos** | `663/661/757` → **`390/427/413`**, con ~300 cuentas de margen |
| 🎯 **El mapa de sensores estaba mal** | y había **tres versiones distintas** dando vueltas. Ninguna correcta |
| ✅ **`escaparDeLinea()` estaba bien** | el error era de rótulos, no de comportamiento |
| ✅ **Freno eléctrico** | portado del arquero. `parar()` soltaba las ruedas |
| ✅ **Escape rediseñado** | 4 pasadas hasta: retroceso inmediato 300 ms, ciego 1 s |
| 🐛 **Dos bugs del giroscopio** | la telemetría mentía, y una vez caído no se recuperaba nunca |
| ⚠️ **Nada se validó en jugada completa** | se probó por partes, no de punta a punta |

---

## 0. Lo que cambió antes de empezar

La cancha es **otra**: material tipo tela, el verde mucho más oscuro, la línea blanca distinta,
y **rampitas en los bordes** para que la pelota vuelva si se sale.

Eso invalidó de un saque toda la calibración del 08/09. No es que estuviera mal medida: estaba
medida sobre otra superficie.

---

## 1. 🔴 El robot estaba ciego a la línea, y se puede probar

Se corrió `pruebas/grabar-linea/` en la cancha (91,4 s, 12 mesetas). El verde nuevo:

| sensor | verde | blanco | separación |
|---|---|---|---|
| 1 | hasta **132** | desde **649** | 517 |
| 2 | hasta **99** | desde **755** | 656 |
| 3 | hasta **140** | desde **687** | 547 |

Contra los umbrales que tenía cargados (`663 / 661 / 757`):

> **El sensor 3 (delantero) midió 745 y 755 con la línea blanca debajo, en dos mesetas
> distintas. Su umbral era 757. No la vio ninguna de las dos veces.**

El sensor 1 estaba menos grave: su 663 quedaba justo por encima del blanco más flojo (649), así
que una línea apenas pisada se le escapaba, pero una bien pisada la veía.

**O sea que el robot NO estaba escapando de más: estaba escapando de MENOS, y de la línea de
verdad.** Veníamos convencidos de lo contrario desde la clase pasada.

### Los umbrales nuevos

```cpp
int UMBRAL_LINEA[3] = { 390, 427, 413 };   // cancha de tela, 2026-09-15
```

Punto medio verde↔blanco con el peor caso de cada lado. Quedan **~260-330 cuentas de margen para
cada lado, en los tres**. Nunca tuvimos tanto aire: veníamos peleando por 11 y por 16.

### 🎁 El problema del sensor 3 se lo llevó la cancha

En la cancha vieja el sensor 3 tenía **11 cuentas** entre verde (751) y blanco (762). Era
imposible: ningún umbral entraba ahí, y la conclusión del 08/09 era que había que **levantarlo
físicamente**.

Acá tiene **547**.

**No lo arreglamos: se lo llevó el cambio de superficie.** Quedó escrito en el código con un 🛑,
porque ese comentario que decía "hay que subir el sensor 3" seguía sonando vigente y hoy
**empeoraría** las cosas. Si algún día vuelven a jugar sobre la superficie vieja, el problema
vuelve entero.

---

## 2. 🎯 El mapa de sensores estaba mal, y de tres formas distintas

Al preguntar "¿y cuál es el derecho?" aparecieron **tres versiones que no coincidían**:

| | sensor 1 | sensor 2 | sensor 3 |
|---|---|---|---|
| El dibujo del equipo | delantero | izquierdo | derecho |
| La bitácora del 18/08 | izquierdo | derecho | adelante |
| Los comentarios del código 2025 | izquierdo | centro | derecho |

**Ninguna era la correcta.**

Se hizo `pruebas/identificar-sensores/` (nuevo): un sensor por vez sobre el blanco, los otros dos
en negro, y el programa dice **qué número saltó**.

```
NEGRO (los tres)      S1=68    S2=54    S3=239
ADELANTE en blanco    S1=70    S2=78    S3=761      subida: +2  +24  +522
IZQUIERDO en blanco   S1=57    S2=759   S3=66
DERECHO en blanco     S1=762   S2=56    S3=66
```

```
sensor 1  =  DERECHO     A11, pin físico 25
sensor 2  =  IZQUIERDO   A13, pin físico 27
sensor 3  =  DELANTERO   A12, pin físico 26
```

Los tres a **120°**, en los lados del triángulo — no en fila. Por eso no existe ningún "centro".

### ✅ Y el código estaba bien

`escaparDeLinea()` hace lo correcto, y se puede ver con la geometría:

| sensor | está entre | apaga |
|---|---|---|
| 3 delantero | izq + der | **trasera** |
| 1 derecho | trasera + der | **izquierda** |
| 2 izquierdo | trasera + izq | **derecha** |

Cada sensor está **enfrentado** a una rueda, y el escape apaga justo esa — que es cómo se traslada
en diagonal un robot de tres ruedas omni.

> 🚨 **De dónde salió el error, y es la lección del día:** los nombres `IZQ / DER / TRA` del
> comentario de `escaparDeLinea()` son de **la rueda que se apaga**, no de dónde está el sensor.
> Leerlos como posición del sensor tuvo el mapa mal escrito durante un mes — y me hizo equivocar
> a mí también, dos veces, en sentidos opuestos.

Rótulos corregidos en **6 archivos** y en el banner del robot, que ahora imprime
`S1=DERECHO S2=IZQUIERDO S3=DELANTERO (medido 15/09)`.

---

## 3. El escape de línea, cuatro pasadas hasta que funcionó

Se rediseñó probando en cancha entre pasada y pasada:

| | qué hacía | qué pasó |
|---|---|---|
| v1 | freno eléctrico 1 s + retroceso ciego **1500 ms** a 200 | "retrocede mucho y muy rápido" |
| v2 | igual, **500 ms** | seguía largo |
| v3 | igual, **200 ms** | **se quedaba clavado** |
| **v4** | **sin freno: retrocede YA 300 ms a 200**, ciego 1 s | ← el que quedó |

### Por qué se quedaba clavado dentro del área chica

Frenar primero dejaba el robot **quieto**, y arrancar desde quieto se come casi esos 200 ms nada
más que en superar el piso de PWM (**~70 desde parado, contra ~40 rodando**).

**Frenaba, y después casi no llegaba a moverse.** Sacar el freno del medio no es sólo ahorrar
tiempo: el robot llega en movimiento, y el retroceso aprovecha que las ruedas ya giran.

### La dirección: sólo el que la vio PRIMERO

Antes se escapaba con la máscara completa, sumando direcciones. El síntoma que apareció: al llegar
en diagonal, un segundo sensor pisa la línea un instante después, la dirección resultante se corre,
y el robot **sale para cualquier lado en vez de por donde entró**.

Ahora hay **un cronómetro por sensor** y gana el que lleva más tiempo viendo blanco. El monitor lo
dice:

```
!!! LINEA BLANCA: la vio PRIMERO el sensor 1 (DERECHO)   [tambien veian: 3  -> los ignoro]
```

> **Es el mismo patrón que ya mordió dos veces el 01/09:** una decisión que hay que tomar UNA VEZ
> y sostener, replanteada a cada instante.

De paso desapareció el "compromiso de 400 ms": la máscara **queda congelada sola** porque durante
la ventana ciega no se lee nada. Un mecanismo menos.

---

## 4. Freno eléctrico — `parar()` soltaba las ruedas

Portado de la mesa del **arquero**, que lo tiene medido en el piso el 18/08:

```cpp
void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  ...
}
```

Cortocircuita los bornes del motor: la corriente que el propio motor genera al girar lo frena a él
mismo.

> ⚠️ **Se parece peligrosamente a `parar()`:** las patas de dirección quedan igual, en 0. **La
> única diferencia es el PWM.** En 0 el driver apaga la salida y la rueda queda suelta; en 255
> queda trabada. Mismo estado de las patas, efecto opuesto.

Se puso además en la rama de **los tres sensores a la vez**, donde antes hacía `parar()`. Con
rampitas en el borde, un robot quieto **pero suelto** sobre un plano inclinado se desliza solo — y
esa rama salta justo cuando algo raro está pasando.

---

## 5. 🐛 Dos bugs del giroscopio, encontrados al verificarlo

El chip está sano (`SYS_STATUS=5` = fusión corriendo, lo dice el propio sensor). Pero:

**1. La telemetría mentía.** Imprimía `ultimoRumbo`, que es una **caché**. `ultimoRumbo` sólo se
escribe adentro de `rumboActual()`, y `rumboActual()` **no se llama en `BUSCANDO`**. O sea que el
campo `rumbo=` repetía el valor congelado del arranque. Ese `rumbo=360` fijo era el
`Rumbo cero = 359,9` del banner.

**2. Peor: el giroscopio no podía recuperarse nunca.** `giroscopoSano()` mira `giroCaido`, y
`giroCaido` **sólo se actualiza adentro de `rumboActual()`**. Si el sensor se caía una vez, nada
volvía a preguntarle al chip → quedaba dado por muerto **para el resto del partido**, aunque se
recuperara al instante.

**Arreglo:** la telemetría llama a `rumboActual()` **siempre que haya giroscopio**, no sólo si está
sano. El chequeo sigue corriendo cada 2 s pase lo que pase, y si se cae imprime `rumbo=CAIDO` en
vez de un número inventado. Cuesta una lectura I2C cada 2 segundos.

---

## 6. Potencias subidas por la tela

| | antes | ahora | por qué |
|---|---|---|---|
| `VEL_AVANCE` | 55 | **95** | estaba **debajo del piso de arranque** (~70) |
| `VEL_ORB_IMPULSO` | 99 | **120** | despegar cuesta más en tela |
| `VEL_ORB_TRASERA` | 48 | **67** | 48 dejaba 8 cuentas sobre el piso de rodadura |
| `XP_ORBITA` | 22 | **34** | chocaba la pelota al entrar a orbitar |

**Lo de `VEL_AVANCE` no era "poca potencia": estaba mal.** A `AVANZANDO` se entra desde
`CENTRANDO`, que pulsa y **deja el robot parado** entre pulso y pulso. Arrancaba a avanzar desde
quieto con 55, cuando el piso desde quieto es ~70. Estaba anotado como pendiente desde el
**04/08** y nunca se había tocado.

Recorrido de las perillas que se tantearon en cancha:

```
XP_ORBITA         22 -> 34 -> 38 -> 34
VEL_ORB_IMPULSO   99 -> 130 -> 120
VEL_ORB_TRASERA   48 ->  75 ->  67
MS_ESCAPE_CIEGO   1500 -> 500 -> 200 -> (reemplazado por MS_RETROCESO_LINEA = 300)
```

---

## 7. Números de hoy, todos juntos

```
VERDE de la tela      S1  66..132    S2  41..99     S3  52..140
BLANCO de la línea    S1 649..767    S2 755..766    S3 687..765
NEGRO                 S1  68         S2  54         S3 239   (S3 alto: ver pendiente 10)

UMBRALES              390 / 427 / 413      confirmación 5 ms
AL VER LÍNEA          retrocede ya 300 ms a 200, ciego 1000 ms
ÓRBITA                Xp<34 · impulso 120 x 300 ms -> crucero 67 · máx 20 s
PATADA                110 x 420 ms (MODO PRUEBA) · rampa +15 cada 5 ms
AVANCE                95
GIROSCOPIO            SYS_STATUS=5 · rumbo cero 359,9 / 0,0
```

---

## 8. Programa nuevo

| Programa | Responde |
|---|---|
| `pruebas/identificar-sensores/` | ¿Qué número de sensor es cada posición física? |

Y `pruebas/grabar-linea/` ganó **clasificación automática**: con 600 cuentas de hueco entre verde
y blanco, el robot ya puede decidir solo qué es cada meseta (`< 300` verde, `> 600` blanco, en el
medio `?` y no la usa) y proponer el umbral. En la cancha vieja esto era imposible: verde 751 y
blanco 762 estaban pegados.

---

## ⚠️ Qué queda por VER (nada de esto se validó)

1. 🔴 **Ninguna jugada completa.** Se probó por partes. No se vio el ciclo
   buscar → centrar → avanzar → orbitar → patear de punta a punta.
2. 🔴 **El retroceso de 300 ms nunca se probó.** Se cargó al cerrar la clase.
3. **El giroscopio no se verificó girando el robot.** Sabemos que la fusión corre; falta el
   minuto de girarlo a mano y ver que `rumbo=` acompañe y vuelva.
4. **La rampa de la patada sigue sin probarse** (se agregó el 08/09 y nunca se vio andar).
5. **El filtro de 5 ms tampoco.**

## 🔧 Qué queda por HACER

6. 🔴 **Volver la patada a `VEL_PATADA = 215`.** Hoy está en **110** para poder mirarla. Así no se
   puede jugar.
7. 🔴 **Correr `pruebas/piso-de-pwm/`** sobre la tela. Existe desde julio y **nunca se corrió**.
   Ese "~70 desde quieto / ~40 rodando" que usamos como base para *todo* nunca se midió, y encima
   es de otra superficie. Es la medición que desbloquea las demás.
8. 🔴 **Correr `pruebas/tabla-camara/`.** `XP_ORBITA = 34` está en **unidades desconocidas**: el
   factor de escala de la cámara nunca se midió. Hoy lo tanteamos, no lo calculamos. Pendiente
   desde el **25/08**.
9. **Las rampitas del borde: el firmware no sabe que existen.** Y hay una oportunidad barata:
   `bno.getEvent()` ya trae **cabeceo y alabeo** en la misma transacción I2C que el rumbo. Detectar
   "me subí a una rampa" no cuesta ni un milisegundo más.
10. **El sensor 3 lee 239 sobre el negro**, contra 68 y 54 de los otros dos. Casi cuatro veces más.
    Puede ser altura o que no estaba bien apoyado. Anotado, sin explicar.
11. **Los tres pedidos del 08/09 siguen abiertos:**
    - comprobar el giroscopio para la patada después de una vuelta (`PATEAR_AL_RUMBO0`)
    - si la pelota está cerca de la línea y bien alineada, que patee en vez de escapar
    - revisar la rampa de aceleración
12. 🐛 **`sentidoParaOrbitar()`: las dos ramas eligen lados OPUESTOS** con el mismo signo. La rama
    del arco se calibró en cancha; la del giroscopio quedó al revés. No hay valor de la perilla
    que las ponga de acuerdo.
13. 📓 **La bitácora del 08/09 nunca se escribió.** Los números de esa clase están sólo en el chat.

---

## Nota de método

**Tres cosas que salieron bien hoy, y las tres son del mismo tipo: preguntar en vez de suponer.**

La pregunta *"¿y cuál es el derecho?"* destapó un mapa de sensores que estaba mal **desde siempre**
y que nadie había cuestionado porque estaba escrito en tres lugares distintos — con la mala suerte
de que los tres decían cosas diferentes, así que cualquiera que abriera uno se quedaba tranquilo.

**Y el error que yo repetí dos veces:** leer la bitácora del 18/08 en vez de medir. La primera vez
dije que el sensor 2 era el derecho; después "corregí" el banner poniendo justo eso, empeorando lo
que quería arreglar. **La bitácora tenía izquierda y derecha cruzadas**, que es exactamente el
error que ese mismo documento advierte sobre los rótulos del código 2025.

Lo único que lo resolvió fue apoyar un sensor por vez sobre el blanco y mirar qué número saltaba.
**+522 contra +24.** No hay forma de discutir eso.

> **Y la que se repite clase a clase:** *la pregunta fácil es "cuál salta", no "cuál es".*
