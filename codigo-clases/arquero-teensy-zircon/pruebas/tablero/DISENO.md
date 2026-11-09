# TABLERO DEL ARQUERO — el diseño para construirlo en paralelo

IITA Salta · taller de los martes · Roboliga 2026 (11/11) · escrito el 2026-11-03

Es la ventana con botones del arquero, igual a la que hizo el delantero el 27/10, pero con
**los movimientos del arquero**, **MIRAR ANTES DE SALIR**, un **modo SIN CABLE** (en la cancha no
hay USB) y una pestaña **CANCHA** para preparar una cancha nueva sin internet.

---

## 0. Cómo se usa este documento

**Dos constructores, sin hablarse:**

| Quién | Hace | No toca |
|---|---|---|
| **ROBOT** | `pruebas/tablero/tablero.ino` y `pruebas/tablero/platformio.ini` | todo lo demás |
| **VENTANA** | `pruebas/tablero/tablero.py`, `TABLERO.bat`, `TABLERO-DEMO.bat` (los dos en `arquero-teensy-zircon/`) y `pruebas/tablero/canchas/escuela-iita-bitacoras.txt` | todo lo demás |

(Todas las rutas relativas de este documento son desde `codigo-clases/arquero-teensy-zircon/`.)

- Lo que comparten es la **sección 2 (EL CONTRATO)** y las tablas de las secciones 3 a 6. Las dos
  partes las copian **tal cual**. Si alguno encuentra un error en el contrato, **no lo cambia por su
  cuenta**, porque el otro no se va a enterar: lo resuelve de su lado sin romper el contrato y lo
  anota en su informe final. **Nadie modifica este DISENO.md.** (Lo cambiado después, en la
  revisión del 03/11, está en la sección 15, y en los dos lados.)
- **Prioridades.** **A** = imprescindible para la Roboliga. **B** = si sobra tiempo. Si algo de B
  falta de un lado, el otro igual anda: una orden que el robot no conoce contesta `#ERROR no entiendo
  ...` y la ventana muestra "el programa del robot todavía no tiene esto".
- Cada uno copia el contrato completo en el comentario de arriba de su archivo, con la frase "no
  cambiarlo sin cambiar el otro lado también" (como hizo el delantero).

### Reglas duras para los dos constructores

- **NUNCA** cargar nada a un Teensy, abrir puertos COM, ni correr `cargar.py`, `mirar.py`, ningún
  `.bat`, `pio ... -t upload` o `pio device monitor`.
- ROBOT: compilar **sí** se puede: `"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run -d pruebas/tablero`.
- VENTANA: `tablero.py` se corre **solo** con `--autoprueba` (y `--quien` contra puertos falsos
  dentro de la autoprueba). **Nunca** sin argumentos ni con `--demo`, porque abriría una ventana
  en la pantalla de los chicos.
- No modificar nada fuera de los archivos de la tabla de arriba. No tocar el programa de partido, ni
  `ajustar-movimientos`, ni nada del delantero, ni la cámara. No hacer `git commit` ni `push`.
- Comentarios y textos en castellano rioplatense, para adolescentes, explicando **el porqué**, con
  fechas (2026-11-03). En el `.ino`, **lo que se imprime va sin tildes ni eñes** ("numero",
  "linea"). Los tildes y emojis quedan solo en comentarios (así lo resolvió el delantero). En el
  `.py`, los textos de la ventana van **con** tildes. Los `.bat` van en ASCII puro.

---

## 1. De dónde se copia, y avisos

### 1.1 El programa de partido HOY (copiar de acá, no de otro lado)

- `funciona/seguir-y-despejar/seguir-y-despejar.ino`. Su encabezado dice **VERSION 0.1103 - EN
  PRUEBA** (`const char* VERSION = "0.1103 EN PRUEBA (0.1027d + registro de despejes)"`). Es la
  **0.1027d** (MIRAR más corto) **más un REGISTRO DE DESPEJES** en EEPROM. Se modificó hoy, 18:47,
  y no está commiteado. Alguien lo está editando en paralelo.
- **El ROBOT copia del archivo tal como esté cuando construye**, y pone en `PARTIDO_COPIADO` lo que
  diga su `VERSION` (lo primero, hasta el espacio: hoy `0.1103`). Si no es `0.1103`, revisa los
  números del panel contra la tabla de la sección 3 y lo avisa en su informe.
- Los números de MIRAR en la 0.1103 son distintos de los que circulaban: `distanciaParaMirar` 40.0
  (no 45), `MS_ASENTAR_MIRADA` 60 (no 100), `MS_MIRAR` 120 (no 200, y además pide 3 cuadros como
  mínimo), `MS_MAX_MIRANDO` 500 y `MS_DESCANSO_MIRADA` 500. Si el robot ya estaba quieto y la pelota
  ya está a `umbralCm`, sale sin mirar.
- `ajustar-movimientos.ino` sirve de **modelo** (los movimientos de costado 1 a 5, la medición, la
  nota, las sugerencias y el modo sin cable). Pero **todo lo que mueve el robot se copia del
  partido**, no de ajustar-movimientos. Su movimiento 9 es de **antes** de MIRANDO, así que no sirve.
  Además su `umbralDesvio` vale 7 con máximo 15, y en el partido vale 20.

### 1.2 Mapa de la EEPROM del arquero (es UNA sola y la comparten todos los programas)

| Bytes | De quién | El tablero |
|---|---|---|
| 0..255 | medir-ancho / medir-ancho-sin-giro | **NO** escribe (leer, prioridad B) |
| 256..767 | derecho-y-vuelta | no toca |
| 768..1023 | libre | no toca |
| 1024..2047 | ajustar-movimientos | no toca |
| **2048..2943** | **TABLERO: el último MOVIMIENTO** | escribe |
| **2944..3071** | **TABLERO: la última MEDICIÓN DE CANCHA** | escribe |
| 3072..3583 | **registro de despejes de la 0.1103** (magia `REG1`) | **NUNCA** escribe |
| 3584..3595 | (2026-11-09) partido **0921i**: el largo de la línea del área que midió solo (magia `LIN1`) | no toca |
| 3600..3611 | (2026-11-09) `pruebas/medir-ancho-area` (magia `MAA1`) | no toca (la P guarda en 2944..3071) |
| 3612..4283 | libre | no toca |

### 1.3 Los dos robots (por qué nunca cargar el de uno en el otro)

| | Serie USB | En modo carga | Motores (INA/INB/PWM) |
|---|---|---|---|
| ARQUERO | **13348550** | 00145E47 | M1 izq 2/5/3, M2 der 8/7/6, M3 trasera 11/12/4 |
| DELANTERO | 15708680 | 0017F834 | IZQ 8/7/6, DER 11/12/4, TRA 2/5/3 |

VID de PJRC `0x16C0`; PID `0x0483` con un programa corriendo. El programa del arquero en el
delantero mueve las ruedas equivocadas, y al revés.

### 1.4 Cosas de esta PC

- Python de PlatformIO: `%USERPROFILE%\.platformio\penv\Scripts\python.exe` (y `pythonw.exe`), con
  pyserial 3.5 y Tk 9.0. **No hay `wmic`**: no se ve un Teensy en modo carga (sin COM).
- PowerShell está Restricted: nada de `.ps1`. Todo va en Python + `.bat`.
- El delantero carga con `teensy-gui` y deja abierto el Teensy Loader. El arquero carga con
  `teensy-cli`, y `cargar.py` mata `teensy.exe`. **El tablero del arquero sigue el camino del
  arquero (`cargar.py`).**

---

## 2. EL CONTRATO (robot ↔ ventana)

### 2.1 Transporte

- USB serie (`Serial`) a **19200**. Cada renglón termina en `'\n'` solo (el robot usa
  `Serial.print('\n')`, nunca `println`). La ventana igual tolera `"\r\n"`.
- La PC manda ASCII: `(texto + "\n").encode("ascii", "replace")`. Una orden por renglón, de como
  mucho **71 caracteres** (el búfer del robot es de 72).
- Un renglón es para la ventana solo si su **primer carácter es `#`**. El resto es texto para
  personas (la ventana lo muestra en la CONSOLA).
- Campos `NOMBRE=valor` separados por **un** espacio. Las listas van con comas y los decimales con
  punto. El robot no imprime tildes. No hay "-0.0" (sale "0.0").
- Los nombres de los números son **los del panel del partido**, con sus mayúsculas y minúsculas
  (`umbralCm`, `KP_RUMBO`, `MS_MIRAR`). La regex de nombre de la ventana es
  `^[A-Za-z][A-Za-z0-9_]*$` (la del delantero solo aceptaba mayúsculas: **cambiarla**). El robot
  compara los nombres **sin distinguir mayúsculas**. En la tabla no hay dos nombres que choquen así.
- **Las 26 etiquetas:** `HOLA S V DEF OK ERROR GIRO CUENTA INICIO PASO FRENO M MIRE R USADOS CONSEJO
  FIN REPITE GUARDADO LIN ANCHO FONDO CAM MEDIO PANEL FINPANEL`. Una `#OTRA` que no esté en la lista
  es DESCONOCIDA y no se usa. Si dentro de un renglón aparece otra `#ETIQUETA` seguida de espacio o
  de fin de renglón, el renglón es ROTA ("dos renglones pegados"). Por eso ningún texto del robot
  (los consejos tampoco) puede tener `#ETIQUETA ` adentro.
- **Cuándo habla el robot.** Solo si hay alguien escuchando, o sea `if (Serial)` (en el Teensy es
  verdadero cuando un programa de la PC tiene el puerto abierto, con DTR; pyserial lo prende al
  abrir). Así el robot sin cable no se traba escribiendo al vacío. **Mientras se mueve no manda
  NADA**, salvo los `#PASO` de las mediciones A e I, que salen solo con el robot frenado entre un
  tramo y otro (sección 5).

### 2.2 Los códigos de cada prueba

| Código | NOMBRE | Qué es |
|---|---|---|
| 1 | `COSTADO_IZQ` | de costado a la izquierda, lento |
| 2 | `COSTADO_DER` | de costado a la derecha, lento (es la prueba de `lateralInvertido`) |
| 3 | `RAPIDO_IZQ` | de costado a la izquierda, rápido |
| 4 | `RAPIDO_DER` | de costado a la derecha, rápido |
| 5 | `IDA_Y_VUELTA` | rápido a la izquierda y vuelta sin frenar |
| 6 | `DESPEJE` | el despeje derecho completo del partido, sin pelota |
| 7 | `UBICARSE` | el arranque del partido: atrás hasta la línea, enderezarse y el poquito |
| 8 | `SEGUIR` | seguir la pelota de costado como el partido (sin despejar) |
| 9 | `ATAJAR` | seguir + MIRAR + despejar en diagonal, como el partido |
| 10 | `IDA` | solo la ida del despeje, frena y se queda (para medir con regla) |
| L | `LINEA_VERDE` / `LINEA_BLANCO_IZQ` / `LINEA_BLANCO_DER` / `LINEA_GUIADA` | medir los sensores de línea (no mueve) |
| A | `ANCHO` | medir el ancho / MS_MEDIO_AL_MEDIO (mueve) |
| F | `FONDO` / `FONDO_COMPROBAR` | medir el fondo (mueve) |
| C | `CAMARA` | 3 s de cámara (no mueve) |
| I | `IR_AL_MEDIO` | la rutina de ir al medio del partido, para comprobar (mueve) |

### 2.3 PC → ROBOT (las órdenes)

| Orden | Qué hace | Contesta |
|---|---|---|
| `!1` .. `!10` | hacer esa prueba con cable (cuenta 3 s) | `>>>` + `#CUENTA`... o `#FRENO` |
| `!1 c` .. `!10 c` | lo mismo **SIN CABLE** (cuenta 10 s para desenchufar) | ídem |
| `!L v` / `!L i` / `!L d` | medir 4 s: verde / blanco IZQ / blanco DER (no mueve, sin cuenta) | `#INICIO` ... `#LIN` `#FIN` |
| `!L c` | la línea GUIADA, sin cable: verde, blanco IZQ y blanco DER, guiado por el LED | ídem, con 3 `#LIN` |
| `!A` / `!A c` | medir el ancho | `#CUENTA`... `#INICIO`, `#PASO`..., `#ANCHO`, `#FIN` |
| `!F` / `!F c` | el fondo: medir (con TOCÓ) / comprobar por tiempo | ... `#FONDO` `#FIN` |
| `!C` / `!C c` | 3 s de cámara (con cable, sin cuenta) | `#INICIO` ... `#CAM` `#FIN` |
| `!I` / `!I c` | ir al medio (la rutina del partido) | ... `#PASO`..., `#MEDIO`, `#FIN` |
| `x` | en ESPERA: frena 400 ms y suelta | texto `PARADO (frena 0,4 s y suelta las ruedas)` |
| *cualquier carácter* | en CUENTA o MOVIENDO lo FRENA (`\r` y `\n` no). En `!F`, la `t` es TOCÓ | `#FRENO tecla` (o `#FONDO como=TOCO`) |
| `=NOMBRE valor` | cambiar un número en la RAM (también `=NOMBRE=valor` y con tab) | `#OK` + `#V`, o `#ERROR` |
| `?` | ayuda | `#V`, `#DEF` y texto |
| `v` | líneas para copiar al panel | texto + `#PANEL`... + `#FINPANEL` |
| `0` | volver todos los números a los del partido (las copias) | texto + `#V` |
| `h` o `H` | saludo | `#HOLA` + 2 `#GUARDADO` |
| `g` | buscar el giroscopio de nuevo (`bno.begin()` otra vez, hasta ~6 s) | `#GIRO buscando` ... `#GIRO ok=` |
| `r` | repetir el último resultado de MOVIMIENTO (de la RAM o de la EEPROM) | `#REPITE` + la tanda + `#FIN`, o `#ERROR no hay nada guardado` |
| `rc` | repetir la última MEDICIÓN DE CANCHA | ídem |

- Se ignoran los espacios de los costados y los renglones vacíos. Las letras de `!L`, `!A`, `!F`,
  `!C`, `!I` y sus `v i d c` se aceptan en mayúscula o minúscula. `!6c` es lo mismo que `!6 c`.
- **Las órdenes solo se leen en ESPERA.** En CUENTA o MOVIENDO, el primer byte que no sea `\r` ni
  `\n` frena y **no se interpreta** (después el robot tira lo que llegue durante 50 ms).
- `!` sin número: `#ERROR falta el numero del movimiento: !1 a !10, o !L !A !F !C !I`.

### 2.4 ROBOT → PC, renglón por renglón (con ejemplos reales)

**`#HOLA`** — en `setup()` (después de esperar `Serial` hasta 3 s), con `h`/`H`, y **cada vez que se
abre el puerto estando en ESPERA** (el robot mira cuándo `(bool)Serial` pasa de falso a verdadero,
como hace la 0.1103 con su registro; esa mirada se hace solo en ESPERA).
```
#HOLA tablero-arquero 2026-11-03 serie=13348550 partido=0.1103
```
- El 1er campo **tiene que ser exactamente `tablero-arquero`** (el del delantero dice
  `tablero-delantero`). El 2do es la fecha del contrato (solo se muestra). `serie=` es el número que
  el programa lee de SU propio Teensy (sección 7.6). `partido=` es la versión de la que son las copias.
- Después del `#HOLA` salen siempre los dos `#GUARDADO` (mov y cancha). En `setup()` además salen
  `#GIRO`, `#V` y `#DEF`.

**`#GUARDADO`** — lo que quedó guardado (RAM o EEPROM). Sale después de cada `#HOLA`.
```
#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210
#GUARDADO que=cancha hay=1 n=A nombre=ANCHO nota=OK sincable=1 nuevo=0 mismo=0 hace=-1
#GUARDADO que=mov hay=0
```
- `que`: `mov` o `cancha`. Con `hay=0` no vienen más campos.
- `nota`: la del `#R` en un movimiento. En cancha es `OK` o `ERROR`.
- `nuevo=1`: **nadie lo vio todavía** (cuando terminó no había nadie escuchando). La ventana pide `r`
  o `rc`. `nuevo` pasa a 0 cuando el robot lo manda con alguien escuchando. **Salvo en una prueba
  SIN CABLE** (revisión del 03/11, ver sección 15): esa queda `nuevo=1` aunque la tanda haya salido,
  porque el Teensy no se entera de que lo desenchufaron (`Serial` sigue en true) y la manda AL VACÍO.
  Solo `r`/`rc` la pasan a 0. La ventana no vuelve a pedir lo que ya le llegó en vivo: compara el
  código y `hace=` con la hora de los `#FIN` que vio.
- `mismo=1`: se hizo en este mismo encendido; `hace` = los ms desde que terminó. `mismo=0`: viene de
  la EEPROM, de antes de apagar, y entonces `hace=-1`.

**`#GIRO`** — el giroscopio.
```
#GIRO buscando
#GIRO ok=1 estado=5
#GIRO ok=0 estado=-1
```
`estado` = registro SYS_STATUS 0x39 leído a mano por I2C (5 = fusión OK), o -1 si no contesta.
Sale en `setup()` (antes y después de buscarlo) y con `g`. Mientras está `buscando`, la ventana no
deja probar nada.

**`#S`** — los sensores. Cada **100 ms**, **solo en ESPERA** y solo si hay alguien escuchando. El
orden es fijo: `t` primero y `e` último (así la ventana detecta un renglón cortado). La ventana
ignora los campos de más.
```
#S t=12345 l=88,92,61 u=425 r=93.4 g=5 cal=3,3,0,0 p=86,-12,40 d=30.0 y=4.2 vy=0.0 va=0.0 ve=1 n=3 dg=1,0,193,33,396 am=150,-20,120 az=0,0,-1 q=24 e=ESPERA
#S t=500 l=90,85,60 u=425 r=-1 g=-1 cal=-1,-1,-1,-1 p=0,0,-1 d=-1.0 y=0.0 vy=0.0 va=0.0 ve=0 n=0 dg=1,0,200,0,383 am=0,0,-1 az=0,0,-1 q=0 e=ESPERA
```
| Campo | Qué es |
|---|---|
| `t` | `millis()` |
| `l` | `analogRead` 0..1023 en este orden: **A13 atrás IZQUIERDA, A11 atrás DERECHA, A12 adelante** (el de adelante el partido no lo usa) |
| `u` | `umbralBlanco` (UN número, no tres como el delantero) |
| `r` | rumbo 0..360 con 1 decimal (`rumboActual()` del partido), o -1 si está mudo o no hay giroscopio |
| `g` | SYS_STATUS (0x39 a mano), -1 si no contesta |
| `cal` | `bno.getCalibration` sistema,giro,acel,mag (0..3 cada uno); `-1,-1,-1,-1` sin giroscopio |
| `p` | `Xp,Yp,edad` CRUDOS de la última vez que vio la pelota (`Xp>0`): Xp 1..200, Yp -100..100 tal como llega (ya con el -100, **sin** invertir). `edad` = ms desde ese paquete; `0,0,-1` si nunca la vio |
| `d` | cm hacia adelante de ese paquete = `Xp / CAMARA_POR_CM`, 1 decimal. `-1.0` si nunca (la ventana toma cualquier `d < 0` como "nunca la vio") |
| `y` | cm al costado de ese paquete, **+ = a la DERECHA** del robot (como `desvioPelota()` del partido: `(camaraYInvertida ? -Yp : Yp) / CAMARA_POR_CM`), 1 decimal |
| `vy` | `velocidadLateral` del partido (cm/s, + = va a la derecha), 1 decimal |
| `va` | `velocidadAcercamiento` (cm/s, + = se viene encima), 1 decimal |
| `ve` | 1/0 = `veLaPelota()` (Xp>0 y paquete de hace menos de 500 ms) |
| `n` | `vecesSeguidas` 0..3 (los cuadros seguidos en zona de disparo, contados por el `leerCamara()` del partido) |
| `dg` | "si saliera AHORA": `sale,motivo,avance,lateral,msIda` de `calcularDiagonal()` (sección 7.4). `sale` 1/0; `motivo` 0 sale, 1 el avance no alcanza, 2 muy abierto; `lateral` con signo (+ = derecha). Si no sale, `lateral=0` y `msIda=msAdelante` |
| `am`, `az` | arco amarillo y azul, CRUDOS como los manda la cámara (`X,Y,edad`, Y ya con el -100). El partido no los usa: son solo para mirar |
| `q` | paquetes de cámara por segundo (se recalcula cada 1 s) |
| `e` | siempre `ESPERA` |

- Validación en la ventana: `l` trae 3 números, `cal` 4, `p`/`am`/`az` 3, `dg` 5, y `u` 1.
- Ejemplo de la cuenta: `Xp=86` da `d=86/2.87=29.97 → 30.0`. `Yp=-12` con `camaraYInvertida`
  da `y=+12/2.87=4.18 → 4.2` (DERECHA). Con eso `calcularDiagonal` da `1,0,193,33,396`:
  sale a ~8° a la derecha.

**`#V` y `#DEF`** — todos los números en el orden de la tabla de la sección 3, en un solo renglón.
`#V` = los de ahora; `#DEF` = las copias del partido (lo que vuelve con `0`). Los enteros y los
`u` van sin punto. Los bool van `0`/`1`. Los float van con 1 o 2 decimales (2 solo si hacen falta:
`30.0`, `0.8`, `0.75`, `2.87`).
```
#V umbralBlanco=425 CAMARA_POR_CM=2.87 MS_MEDIO_AL_MEDIO=2228 MS_MEDIO_FONDO_POR_TIEMPO=2000 kpLateral=11.5 pwmMinLateral=45 pwmMaxLateral=150 ZONA_MUERTA_PELOTA=1.4 predecirTrayectoria=0 msAnticipacionSeguir=500.0 LADO_TRASERA=89 volverAlMedioEnElLateral=1 umbralCm=30.0 umbralDesvio=20.0 potenciaDespeje=200 msAdelante=383 frenarAntesDeVolver=1 MS_ESPERA_ANTES_DE_VOLVER=400 potenciaRetroceso=110 msFreno=200 empujonFinal=1 potenciaEmpujon=100 msEmpujonFinal=320 avanzarAlUbicarse=1 despejeEnDiagonal=1 predecirEnDiagonal=1 rendimientoCostado=0.8 MAX_ANGULO_DIAGONAL=30.0 medirAntesDeDespejar=1 distanciaParaMirar=40.0 MS_ASENTAR_MIRADA=60 MS_MIRAR=120 MS_MAX_MIRANDO=500 MS_DESCANSO_MIRADA=500 KP_RUMBO=5.0 KI_RUMBO=1.5 LIMITE_INTEGRAL=30.0 MAX_CORRECCION_RUMBO=100 ZONA_MUERTA_RUMBO=1.0 RAMPA_MOV_PASO=10 lateralInvertido=0 camaraYInvertida=1 velLenta=70 msCostado=1000 msSeguir=10000 msPruebaIda=383
```
Salen: `#V` en `setup()`, con `?`, después de cada `#OK` y con `0`. `#DEF` en `setup()` y con `?`.

**`#OK`** — confirma un cambio, con el valor ya redondeado. Lo sigue un `#V` completo.
```
#OK umbralCm=32.0
#OK despejeEnDiagonal=0
```

**`#ERROR texto`** — los textos son exactamente estos (X = nombre, V = lo que mandaron):
```
#ERROR falta el nombre. Se escribe: =NOMBRE valor   (ej: =umbralCm 32)
#ERROR no existe el numero X. Con ? salen todos los nombres
#ERROR falta el valor de X. Se escribe: =NOMBRE valor
#ERROR X: usar punto decimal (4.5, no 4,5)
#ERROR X: no es un numero: V
#ERROR X va sin decimales
#ERROR X es si o no: va 0 o 1
#ERROR X fuera de rango: va de MIN a MAX
#ERROR falta el numero del movimiento: !1 a !10, o !L !A !F !C !I
#ERROR no hay movimiento X: son del !1 al !10, y los de cancha !L !A !F !C !I
#ERROR !L necesita la fase: !L v (verde), !L i (blanco izq), !L d (blanco der) o !L c (sin cable)
#ERROR no hay nada guardado
#ERROR no entiendo 'V'. ? = ayuda
#ERROR orden demasiado larga
```
`nan` e `inf` dan "no es un numero". MIN y MAX se imprimen con el mismo formato que el valor.

**`#CUENTA segundos modo`** — la cuenta antes de moverse. `modo` es `cable` o `sincable`.
- Con cable: dura **3000 ms** y sale a los 0, 500, ..., 2500 ms: `3.0 2.5 2.0 1.5 1.0 0.5`.
- Sin cable: dura **10000 ms** y sale cada 1000 ms: `10.0 9.0 ... 1.0` (solo si hay alguien escuchando).
- Antes del primero sale un renglón humano:
  - `>>> 6 DESPEJE en 3 s. Saca la mano. Cualquier tecla lo frena.`
  - `>>> 6 DESPEJE SIN CABLE: arranca en 10 s. DESENCHUFEN EL USB. Al terminar, el LED dice la nota.`
```
#CUENTA 3.0 cable
#CUENTA 10.0 sincable
```
- La CUENTA la marca el robot (con estos renglones), **no un reloj de la PC**.
- `!L` y `!C` con cable no mueven: **no tienen cuenta** y arrancan directo con `#INICIO`.

**`#INICIO codigo NOMBRE tope=ms sincable=0|1`** — arranca. Desde acá no sale **nada** hasta el
final (salvo los `#PASO` de A e I). `tope` es el tope duro de esa prueba en ms (sección 4.3): la
ventana espera `tope + 5000` ms sin renglones antes de dar el robot por perdido.
```
#INICIO 6 DESPEJE tope=20000 sincable=0
#INICIO A ANCHO tope=45000 sincable=1
#INICIO L LINEA_VERDE tope=5000 sincable=0
```

**`#PASO codigo ...`** — avance de A e I. Sale con el robot frenado entre tramo y tramo, y solo si
hay alguien escuchando.
```
#PASO A calibrando giro=2
#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0
#PASO I paso=1 ms=1830 como=L
```

**`#FRENO motivo`** — se frenó, o no arrancó. Los motivos son exactamente: `tecla`, `tope de
tiempo`, `sin giroscopio`, `sin camara`, `sobre la linea`, `recien parado`, `no es el arquero`.
- **Sin `#INICIO` antes** (frenado en la cuenta, o no arrancó): no hay informe. Después sale el
  texto humano (`cancelado: no se movio`, o la explicación).
- **Después de `#INICIO`** solo puede ser `tecla` o `tope de tiempo`, y sigue la tanda completa con
  `nota=NO_VALE` (o `#ANCHO ok=0 error=6/7`, etc.).
```
#FRENO tecla
#FRENO sobre la linea
```

**`#M t_ms grados fase`** — las muestras del giroscopio, todas juntas **después** de moverse. `t` =
ms desde el `#INICIO`. `grados` = `diferencia(rumboBase, rumbo)` con 1 decimal: **NEGATIVO = giró a
la DERECHA, POSITIVO = a la IZQUIERDA**. Las lecturas mudas no se mandan. `fase` es una letra:

| fase | qué es | color en el gráfico |
|---|---|---|
| `l` | de costado (1 a 5) | `#6a1b9a` |
| `s` | siguiendo o esperando la pelota (8, 9) | `#00838f` |
| `m` | MIRANDO, quieto (9) | `#f9a825` |
| `i` | ida del despeje | `#e65100` |
| `p` | pausa frenado antes de volver | `#8d6e63` |
| `a` | atrás hasta la línea | `#1565c0` |
| `f` | frenando | `#c62828` |
| `e` | enderezándose (ACOMODANDO) | `#2e7d32` |
| `c` | empujoncito final | `#5d4037` |
| `v` | volviendo al medio (LATERAL_AL_MEDIO y la rutina) | `#3949ab` |
| `q` | quieto al final (asentar) | `#9e9e9e` |
```
#M 0 0.0 i
#M 120 -2.3 i
#M 3880 0.2 i
```

**`#MIRE ...`** — solo en la 9: un renglón por intento de despeje (como el REGISTRO de la 0.1103),
todos juntos después de moverse, en orden. Hasta 10.
```
#MIRE k=1 t=2140 res=NO_VIENE miro=1 cuadros=4 d=38.2 y=-3.1 vy=0.0 va=2.5 av=0 lat=0 ms=0 ang=0.0 recto=0
#MIRE k=2 t=3870 res=SALE_MIRANDO miro=1 cuadros=4 d=29.6 y=-7.4 vy=-10.0 va=30.0 av=162 lat=-94 ms=472 ang=-25.0 recto=0
```
| Campo | Qué es |
|---|---|
| `k` | número de intento (1..) |
| `t` | ms desde el `#INICIO` |
| `res` | `SALE_MIRANDO`, `SALE_SIN_MIRAR`, `MUY_ABIERTO`, `NO_ALCANZA`, `NO_VIENE`, `POCOS_CUADROS` (el mismo `enum Resultado` de la 0.1103) |
| `miro` | 1 si pasó por MIRANDO |
| `cuadros` | cuadros de la mirada (`nMirada`), 0 si no miró |
| `d`, `y` | la pelota cuando decidió (cm; y + = derecha) |
| `vy`, `va` | las velocidades que usó: las de la mirada (`vyMirada`, `vaMirada`) si miró, o `velocidadLateral` y 0 si no |
| `av`, `lat`, `ms` | avance, costado (con signo) y msIda con que salió; 0 si no salió |
| `ang` | el ángulo que **necesitaba** para encontrarse con la pelota, en grados con signo (+ = derecha): `atan(costado * rendimientoCostado / avance)`, calculado ANTES de que `COSTADO_MINIMO_UTIL` lo pase a 0. Con `NO_ALCANZA` no se llega a calcular: `0.0` (la ventana muestra "-") |
| `recto` | si salió DERECHO, por qué: 0 salió en diagonal (o no salió), 1 costado chico (< `COSTADO_MINIMO_UTIL` 20), 2 `despejeEnDiagonal` apagado, 3 sin giroscopio, 4 no la veía |

Igual que el partido, un "no sale" con el mismo `res` que el anterior y a menos de 1 s **no se anota**.

**`#R ...`** — el informe. **Siempre trae todos los campos y en este orden.** Los que no aplican
van en 0. Todos los `...ms` son **ms desde el `#INICIO`**, salvo `lineams`.
```
#R n=6 mov=DESPEJE nota=CASI como=NORMAL pico=6.2 picoms=520 fin=1.1 medio=0.8 cruces=1 fuera3=12 corr=-4 tope=0 mudas=0 racha=0 hueco=12 huecoms=300 duro=3410 lineams=905 idams=383 atrasms=783 acomodo=1 laterales=0 idas=0 arranques=0 conpelota=0 zonamuerta=0 velmedia=0.0 intentos=0 sale=0
```
| Campo | Qué es |
|---|---|
| `n`, `mov` | código y NOMBRE |
| `nota` | `PROLIJO`, `CASI`, `ZIGZAGUEA`, `SE_TUERCE`, `NO_SALIO` o `NO_VALE` (sección 4.4) |
| `como` | cómo terminó: `NORMAL`, `LATERAL` (1 a 5: pisó el lateral y frenó), `SIN_LINEA` (6, 7: volvió hasta el tope sin ver la línea), `NO_SALIO` (9), `TECLA`, `TOPE` |
| `pico`, `picoms` | lo más torcido, con signo (grados, 1 decimal), y cuándo |
| `fin` | el promedio de los últimos 150 ms de la fase `q` (quieto al final) |
| `medio` | el error promedio CON signo mientras se movía |
| `cruces` | veces que pasó de más de +2° a menos de -2°, o al revés |
| `fuera3` | % de las muestras en movimiento con más de 3° |
| `corr` | corrección de rumbo promedio (PWM, con signo) |
| `tope` | % del tiempo con la corrección al tope (`MAX_CORRECCION_RUMBO`) |
| `mudas`, `racha` | lecturas mudas del giroscopio, y la peor racha seguida |
| `hueco`, `huecoms` | la vuelta del programa más larga en movimiento (ms), y cuándo |
| `duro` | ms desde el `#INICIO` hasta el final |
| `lineams` | 6, 7, 9: ms desde que arrancó el retroceso hasta ver la línea (0 = no la vio) |
| `idams`, `atrasms` | 6, 9, 10: cuándo terminó la ida y cuándo arrancó el retroceso |
| `acomodo` | reintentos de ACOMODANDO que usó (0..3) |
| `laterales` | 8, 9: veces que pisó un lateral y volvió al medio |
| `idas`, `arranques` | 8, 9: veces que cambió de lado, y veces que salió de quieto |
| `conpelota`, `zonamuerta` | 8, 9: % de las vueltas viendo la pelota, y % quieto por la zona muerta |
| `velmedia` | 8, 9: \|velocidadLateral\| promedio de la pelota (cm/s, 1 decimal) |
| `intentos`, `sale` | 9: cantidad de `#MIRE` (los que se contaron, aunque se manden 10), y 1 si despejó |

En la **9**, los campos de la nota (`pico`, `picoms`, `fin`, `medio`, `cruces`, `fuera3`, `corr`,
`tope`, `mudas`, `racha`, `hueco`) **se miden desde que arranca la ida** (lo de antes es seguir la
pelota). Las `#M` sí van desde el principio.

**`#USADOS NOMBRE=valor ...`** — los números que eran **distintos de los del partido** cuando se
corrió. Sale en cada tanda de movimiento, aunque sea vacío (`#USADOS` solo).
```
#USADOS rendimientoCostado=0.75 umbralCm=32.0
#USADOS
```

**`#CONSEJO texto`** — 0 o más, después de `#USADOS`. Textos en la sección 4.5.

**`#FIN codigo`** — el **último** renglón de máquina de **toda** tanda (movimiento o cancha, en vivo
o repetida). Con esto la ventana sabe que el resultado está completo y vuelve a ESPERA.
```
#FIN 6
#FIN A
```

**`#REPITE codigo NOMBRE sincable=0|1 apagado=0|1`** — va antes de una tanda repetida con `r`/`rc`.
`apagado=1` si sale de la EEPROM (se apagó en el medio): ahí las `#M` vienen raleadas (hasta 100) y
los consejos usan los números de AHORA. El modo no cambia (sigue ESPERA). Después vienen los
mismos renglones de la tanda original, **sin** el `#FRENO`, y al final el `#FIN`.
```
#REPITE 9 ATAJAR sincable=1 apagado=0
```

**Las mediciones de CANCHA** (formato exacto en la sección 5):
```
#LIN fase=v n=3920 izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1
#ANCHO ok=1 error=0 medioizq=2180 izqder=4344 derizq=4547 almedioizq=2213 almedioder=2314 desvio=2.9,3.4,3.6,2.1 endereza=0 golpe=2.4 vel=100 rampa=10 umbral=425 calgiro=3
#FONDO ms=1730 como=TOCO desvio=1.8 umbral=425 probado=0
#CAM n=76 cero=0 val=76 xp=84,90,86.9 yp=-1.2 am=0 az=0
#MEDIO ok=1 pasos=1830,400,2000,943,610,2228,520,320 como=L,T,T,T,L,T,L,T desvio=3.1 acomodo=0
```

**`#PANEL linea` ... `#FINPANEL`** — con `v`. Van SOLO los números que cambiaron y que existen en el
partido, con este formato: declaración del panel + " " + nombre + " = " + valor + ";" (los bool
como `true`/`false`). Siempre cierra con `#FINPANEL`; si no cambió nada, sale `#FINPANEL` solo.
```
#PANEL float rendimientoCostado = 0.75;
#PANEL const unsigned long MS_MIRAR = 140;
#PANEL bool despejeEnDiagonal = false;
#FINPANEL
```
Antes salen los textos humanos: `--- PARA COPIAR AL PANEL DE CONTROL (funciona/seguir-y-despejar) ---`,
`Buscar cada nombre (Ctrl+F) y cambiar SOLO el numero.`, y si corresponde `(nada para copiar: estan
todos como en el partido)` y `(no se copian porque son solo de esta prueba: velLenta msCostado ...)`.

### 2.5 Las secuencias

Después de `!n` (con cable):
- **(a) Normal:** `>>> ...` → `#CUENTA 3.0 cable` ... `0.5` → `#INICIO n NOMBRE tope=.. sincable=0`
  → (silencio) → `#M`... → (`#MIRE`... en la 9) → `#R` → `#USADOS` → `#CONSEJO`... → texto
  `RESULTADO: ...` → `#FIN n` → vuelven los `#S`.
- **(b) Tecla en la cuenta:** `#CUENTA`... → `#FRENO tecla` → `cancelado: no se movio`.
- **(c) Tecla moviéndose:** `#INICIO` → (silencio) → `#FRENO tecla` → `#M`... → `#R ... nota=NO_VALE
  como=TECLA` → `#USADOS` → `#CONSEJO Lo frenaron antes de terminar: este resultado no vale.` → `#FIN n`.
- **(d) No arranca** (sin giroscopio después de intentar arrancarlo, sobre la línea, sin cámara,
  recién parado, no es el arquero): `#FRENO motivo` + texto, **sin** `#CUENTA`.
- **(e) Sin giroscopio al terminar la cuenta:** `#CUENTA`... → `#FRENO sin giroscopio`, sin `#INICIO`.
- **(f) Tope de tiempo:** `#INICIO` → `#FRENO tope de tiempo` → la tanda con `nota=NO_VALE como=TOPE`.

**SIN CABLE** (`!6 c`), lo que ve la ventana:
1. `>>> 6 DESPEJE SIN CABLE ...` → `#CUENTA 10.0 sincable`, `9.0`, ... mientras el cable esté puesto.
2. Desenchufan: la ventana pierde el puerto (`desconectado`). **Es lo esperado, no una falla.**
3. El robot cuenta, se mueve solo y frena solo (con su tope). Al terminar arma el resultado, lo
   guarda en la EEPROM con `nuevo=1` y, como `Serial` sigue en true aunque no haya cable, **lo manda
   al vacío** (no pasa nada). El LED repite la nota (sección 7.8).
4. Enchufan **sin apagar la batería**: la ventana se reconecta sola. El robot (en ESPERA) ve que se
   abrió el puerto → `#HOLA` → `#GUARDADO que=mov ... nuevo=1 mismo=1` → la ventana manda `r` →
   `#REPITE 6 DESPEJE sincable=1 apagado=0` → la tanda → `#FIN 6`.
5. Si lo enchufan **mientras todavía se mueve**: el robot no dice nada hasta terminar. Al terminar
   ya hay alguien escuchando y manda la tanda en vivo. `nuevo` queda en 1 (es sin cable), pero la
   ventana sabe que ya la vio y no la pide de nuevo.
6. Si **apagaron la batería** antes de enchufar: `#HOLA` → `#GUARDADO ... nuevo=1 mismo=0 hace=-1` →
   `r` → `#REPITE ... apagado=1` con las `#M` raleadas.

**Las mediciones de CANCHA:** igual que un movimiento, pero la tanda es `#LIN`/`#ANCHO`/`#FONDO`/
`#CAM`/`#MEDIO` (sin `#M`, `#R`, `#USADOS` ni `#CONSEJO`) y `#FIN codigo`. A e I mandan `#PASO`
mientras tanto.

### 2.6 Cómo se reconoce el robot y el programa

- **Que es el ARQUERO:** por el número de serie que da Windows en `list_ports.comports()`:
  `vid == 0x16C0` y `serial_number == "13348550"`. Se mira **sin abrir el puerto**. La ventana
  **nunca abre el puerto de otra Teensy** (ni la del delantero `15708680` ni ninguna desconocida), ni
  siquiera con `--puerto`. `--puerto COMx` solo sirve para elegir el COM cuando Windows lo cambió:
  se abre únicamente si `list_ports` dice que ese COM es una Teensy con la serie del arquero. Si no
  aparece, o tiene otra serie, se niega ("ese puerto no es el arquero").
- **Que el programa es el TABLERO DEL ARQUERO:** solo con un `#HOLA` cuyo 1er campo sea
  `tablero-arquero`. Un `#S` solo **no alcanza** (el tablero del delantero también manda `#S`). Si
  el `serie=` del `#HOLA` no es 13348550, cartel rojo (el robot igual se niega a moverse).
- **Al conectar, la ventana no manda nada.** Con el primer `#S`, si todavía no llegó un `#HOLA`,
  manda `h` (y lo repite cada 2 s mientras no llegue). Después del `#HOLA` manda `?` una vez por
  conexión. Si llega `#GUARDADO ... nuevo=1`, manda `r` (o `rc`) una vez. **Nunca manda nada si el
  modo no es ESPERA o si hay un pedido pendiente.**
- **Otros programas, por su texto** (la ventana lo detecta en los renglones humanos y NO manda `h`):

| Texto que llega | Qué tiene el robot | Cartel |
|---|---|---|
| empieza con `=== ARQUERO  version` (dos espacios) o `=== REGISTRO DE DESPEJES` | el PARTIDO | "Tiene el programa de PARTIDO (versión ...). Con la batería prendida se mueve solo y NO obedece PARAR. Para usar el tablero: cerrá esta ventana y TABLERO.bat." |
| empieza con `=== AJUSTAR-MOVIMIENTOS` | ajustar-movimientos | "Tiene AJUSTAR-MOVIMIENTOS: no habla con esta ventana. Usá TABLERO.bat." |
| `#HOLA tablero-delantero ...` | **el tablero del DELANTERO** | rojo: "Este robot tiene el TABLERO DEL DELANTERO: los pines están cruzados. NO probar nada. Cargá el del arquero con TABLERO.bat." (PARAR igual manda `x`: ese programa lo obedece) |
| empieza con `=== TABLERO DEL ARQUERO` | este tablero (el cartel de `setup()`) | "Preguntándole..." (espera el `#HOLA`) |
| nada en 3 s | no se sabe | rojo: "No sé qué programa tiene: puede NO obedecer PARAR. Levantalo o apagá la batería." (PARAR manda un **espacio**, no `x`: si fuera ajustar-movimientos, la `x` le pisaría los números y su EEPROM; el espacio lo frena en ajustar y en los dos tableros) |

PROBAR se habilita **recién** con el `#HOLA` correcto (ver 8.7).

---

## 3. LOS NÚMEROS (la tabla compartida)

46 números, **en este orden** (es el orden de `#V`, `#DEF` y `ajustes[]`). El valor por defecto es
el del **PANEL DE CONTROL de la 0.1103**. Tipo: `i` int, `u` unsigned long, `f` float, `b` bool
(0/1). El paso es el de los botones `−`/`+` de la ventana (el robot no lo usa). La declaración es la
del panel (la usa `#PANEL`; vacía = solo de esta prueba).

| # | Nombre | t | Defecto | Mín..Máx | Paso | Declaración en el panel | Sección del panel |
|---|---|---|---|---|---|---|---|
| 1 | `umbralBlanco` | i | 425 | 100..1000 | 5 | `int` | 2 cancha |
| 2 | `CAMARA_POR_CM` | f | 2.87 | 2.00..4.00 | 0.01 | `const float` | 2 |
| 3 | `MS_MEDIO_AL_MEDIO` | u | 2228 | 500..3200 | 10 | `const unsigned long` | 2 |
| 4 | `MS_MEDIO_FONDO_POR_TIEMPO` | u | 2000 | 500..6000 | 100 | `const unsigned long` | 2 |
| 5 | `kpLateral` | f | 11.5 | 5..20 | 0.5 | `float` | 3 seguir |
| 6 | `pwmMinLateral` | i | 45 | 30..80 | 5 | `int` | 3 |
| 7 | `pwmMaxLateral` | i | 150 | 80..170 | 10 | `int` | 3 |
| 8 | `ZONA_MUERTA_PELOTA` | f | 1.4 | 1..4 | 0.2 | `const float` | 3 |
| 9 | `predecirTrayectoria` | b | 0 | 0..1 | - | `bool` | 1 |
| 10 | `msAnticipacionSeguir` | f | 500 | 0..800 | 50 | `float` | 3 |
| 11 | `LADO_TRASERA` | i | 89 | 80..100 | 1 | `const int` | 3 |
| 12 | `volverAlMedioEnElLateral` | b | 1 | 0..1 | - | `bool` | 1 |
| 13 | `umbralCm` | f | 30.0 | 20..45 | 1 | `float` | 4 despeje |
| 14 | `umbralDesvio` | f | 20.0 | 3..30 | 1 | `float` | 4 |
| 15 | `potenciaDespeje` | i | 200 | 140..230 | 10 | `int` | 4 |
| 16 | `msAdelante` | i | 383 | 100..800 | 10 | `int` | 4 |
| 17 | `frenarAntesDeVolver` | b | 1 | 0..1 | - | `bool` | 1 |
| 18 | `MS_ESPERA_ANTES_DE_VOLVER` | u | 400 | 0..800 | 25 | `const unsigned long` | 4 |
| 19 | `potenciaRetroceso` | i | 110 | 80..140 | 10 | `int` | 4 |
| 20 | `msFreno` | u | 200 | 100..400 | 25 | `unsigned long` | 4 |
| 21 | `empujonFinal` | b | 1 | 0..1 | - | `bool` | 1 |
| 22 | `potenciaEmpujon` | i | 100 | 60..150 | 10 | `int` | 4 |
| 23 | `msEmpujonFinal` | i | 320 | 0..600 | 20 | `int` | 4 |
| 24 | `avanzarAlUbicarse` | b | 1 | 0..1 | - | `bool` | 1 |
| 25 | `despejeEnDiagonal` | b | 1 | 0..1 | - | `bool` | 1 |
| 26 | `predecirEnDiagonal` | b | 1 | 0..1 | - | `bool` | 1 |
| 27 | `rendimientoCostado` | f | 0.80 | 0.40..1.20 | 0.05 | `float` | 4b |
| 28 | `MAX_ANGULO_DIAGONAL` | f | 30.0 | 10..45 | 5 | `const float` | 4b |
| 29 | `medirAntesDeDespejar` | b | 1 | 0..1 | - | `bool` | 4c |
| 30 | `distanciaParaMirar` | f | 40.0 | 25..60 | 1 | `float` | 4c |
| 31 | `MS_ASENTAR_MIRADA` | u | 60 | 0..300 | 10 | `const unsigned long` | 4c |
| 32 | `MS_MIRAR` | u | 120 | 40..600 | 20 | `const unsigned long` | 4c |
| 33 | `MS_MAX_MIRANDO` | u | 500 | 200..1500 | 50 | `const unsigned long` | 4c |
| 34 | `MS_DESCANSO_MIRADA` | u | 500 | 0..2000 | 50 | `const unsigned long` | 4c |
| 35 | `KP_RUMBO` | f | 5.0 | 1..10 | 0.5 | `const float` | 5 rumbo |
| 36 | `KI_RUMBO` | f | 1.5 | 0..3 | 0.25 | `const float` | 5 |
| 37 | `LIMITE_INTEGRAL` | f | 30.0 | 10..40 | 5 | `const float` | 5 |
| 38 | `MAX_CORRECCION_RUMBO` | i | 100 | 40..120 | 10 | `const int` | 5 |
| 39 | `ZONA_MUERTA_RUMBO` | f | 1.0 | 0.25..3 | 0.25 | `const float` | 5 |
| 40 | `RAMPA_MOV_PASO` | i | 10 | 2..20 | 1 | `const int` | 5 |
| 41 | `lateralInvertido` | b | 0 | 0..1 | - | `bool` | 9 signos |
| 42 | `camaraYInvertida` | b | 1 | 0..1 | - | `bool` | 9 |
| 43 | `velLenta` | i | 70 | 40..150 | 5 | (solo de esta prueba) | - |
| 44 | `msCostado` | i | 1000 | 300..1500 | 100 | (solo de esta prueba) | - |
| 45 | `msSeguir` | i | 10000 | 3000..20000 | 1000 | (solo de esta prueba) | - |
| 46 | `msPruebaIda` | i | 383 | 100..800 | 10 | (solo de esta prueba) | - |

Reglas:
- En el robot, los que en el panel son `const` pasan a **variables con el MISMO nombre**, para que el
  código copiado compile sin tocarlo. Pero `#PANEL` igual escribe la declaración del panel (con `const`).
- `ponerValor`: los float se redondean a 2 decimales y los enteros con `lround`. Los bool aceptan
  solo 0 o 1 (`#ERROR X es si o no: va 0 o 1`). El rango se chequea con tolerancia 1e-6.
- `cambiado(i)`: `|valor - defecto| > 0.001`.
- **No hay controles cruzados en el robot** (por ejemplo, `distanciaParaMirar` menor que `umbralCm`).
  Los avisa la ventana (8.4).
- **Lo que NO está en la tabla queda fijo** (copias `const` del partido): `VECES_PARA_CREERLE` 3,
  `MS_PAUSA_MEDIO` 150, `MS_MAX_RETROCESO` 1200, `MS_ENFRIAMIENTO` 1500, `MS_QUIETO_PARA_PREDECIR` 400,
  `PWM_POR_CM_S` 2.0, `PISO_VELOCIDAD` 4.0, `SUAVIZADO_VELOCIDAD` 0.3, `PRESUPUESTO_RUEDA` 210,
  `PWM_MIN_AVANCE_DESPEJE` 140, `MS_MAX_ADELANTE` 800, `COSTADO_MINIMO_UTIL` 20, `LADO_FRENTE` 50,
  `RAMPA_MOV_MS` 10, toda la sección 6 (acomodo), los de la sección 7 que use ir al medio
  (`VEL_MEDIO_BUSCAR` 110, `VEL_MEDIO_MEDIR` 100, `VEL_MEDIO_RECTO` 110, `LADO_MEDIO` -1,
  `MS_MEDIO_ALEJARSE` 400, `MS_MEDIO_ADELANTE` 943, `MS_MEDIO_TOPE_LATERAL` 6000,
  `MS_MEDIO_QUIETO` 300, `LECTURAS_PARA_CREER_LATERAL` 3, `CUADROS_PARA_CORTAR_PATRULLA` 5,
  `PWM_CUENTA_COMO_MOVERSE` 60), `enderezarActivado` true, `perseguirEnElDespeje` false,
  `kpPersecucion` 6.0, `PWM_MAX_PERSECUCION` 60, `msAnticipacion` 0, `patrullaPorElLateral` true.
- `rampaEnRetroceso` de ajustar-movimientos **no existe** acá. No es del partido, y con la diagonal
  abre la vuelta de más (bitácora del 27/10).

**Las explicaciones de la ventana** (`NUMEROS[nombre] = (paso, explicación, opciones)`; los bool
llevan `("NO", "SÍ")`):

| Nombre | Explicación (texto de la ventana) |
|---|---|
| umbralBlanco | Desde qué lectura un sensor de atrás dice BLANCO. Se MIDE en cada cancha (pestaña CANCHA, paso 1). |
| CAMARA_POR_CM | Unidades de cámara por cm real. Es de la CÁMARA: cambiarlo cambia TODOS los cm (y kpLateral está pensado para 2.87). |
| MS_MEDIO_AL_MEDIO | Del lateral izquierdo al medio a velocidad 100 (ms). Se MIDE (CANCHA, paso 2). |
| MS_MEDIO_FONDO_POR_TIEMPO | Ir al medio: cuánto retrocede buscando el fondo si no ve la línea (ms). CANCHA, paso 3. |
| kpLateral | Siguiendo: fuerza por cada cm que la pelota está al costado. |
| pwmMinLateral | Siguiendo: lo mínimo que empuja. Si zumba sin moverse: subir de a 5. |
| pwmMaxLateral | Siguiendo: lo máximo = la velocidad de costado. Más de 150 nunca se probó. |
| ZONA_MUERTA_PELOTA | cm al costado que deja pasar sin moverse. Si tiembla con la pelota quieta: subir. |
| predecirTrayectoria | SÍ = sigue adonde VA a estar la pelota. Apagada desde el 20/10 (iba y venía). |
| msAnticipacionSeguir | Con la predicción prendida: cuánto adelanta (ms). |
| LADO_TRASERA | De costado, fuerza de la rueda de atrás (%). Si de costado gira solo: 2 más o 2 menos. Cambia MS_MEDIO_AL_MEDIO. |
| volverAlMedioEnElLateral | SÍ = al pisar el lateral siguiendo la pelota, vuelve al medio (como juega). |
| umbralCm | Sale a despejar con la pelota a esto o menos (cm). ATADO a msAdelante: la ida tiene que llegar más lejos. |
| umbralDesvio | ...y a esto o menos al costado (cm). Saliendo DERECHO, más de 7 no la alcanza. |
| potenciaDespeje | Fuerza de la ida. Cambia cuánto sale (msAdelante se calculó a 200). |
| msAdelante | Cuánto dura la ida (ms). 383 ≈ 35 cm (es una CUENTA, sin probar). Se mide en CANCHA, paso 5. |
| frenarAntesDeVolver | SÍ = al terminar la ida frena y espera (como juega desde el 20/10). NO = suelta 150 ms. |
| MS_ESPERA_ANTES_DE_VOLVER | Cuánto se queda frenado antes de volver (ms). |
| potenciaRetroceso | Fuerza de la vuelta hasta la línea (también al ubicarse). |
| msFreno | Cuánto sostiene el freno eléctrico cada vez que frena (ms). Lo usan TODOS los frenos. |
| empujonFinal | SÍ = después de volver del despeje avanza el poquito (~10 cm). |
| potenciaEmpujon | Fuerza del empujoncito. Atada a msEmpujonFinal. |
| msEmpujonFinal | Cuánto dura el empujoncito (ms). 320 ≈ 10 cm a 100. |
| avanzarAlUbicarse | SÍ = al encontrar la línea del área avanza el poquito. |
| despejeEnDiagonal | SÍ = despeja en diagonal. NO = sale derecho, como hasta el 20/10. |
| predecirEnDiagonal | SÍ = apunta adonde VA a estar la pelota (solo si estaba quieto o miró). |
| rendimientoCostado | ⚠ VA AL REVÉS: más chico = MÁS costado. Si no llega de costado: BAJAR de a 0.05. Si se pasa: SUBIR. |
| MAX_ANGULO_DIAGONAL | Si tiene que salir más abierto que esto (grados), NO SALE. |
| medirAntesDeDespejar | SÍ = MIRA antes de salir (frena, mide quieto, sale a umbralCm). NO = sale apenas la ve a umbralCm. |
| distanciaParaMirar | A esta distancia (cm) empieza a mirar. Más = ve venir pelotas más rápidas, pero frena más seguido. |
| MS_ASENTAR_MIRADA | Primero frena esto (ms) sin medir: todavía se está frenando. |
| MS_MIRAR | Después mide quieto esto (ms) como mínimo, y 3 cuadros como mínimo. |
| MS_MAX_MIRANDO | Tope de la mirada (ms). Si la pelota no viene, vuelve a seguirla. |
| MS_DESCANSO_MIRADA | Después de una mirada que no salió, no vuelve a mirar hasta que pase esto (ms). |
| KP_RUMBO | Corrección por cada grado torcido. Si zigzaguea: bajar de a 0.5. Si se le escapa: subir. |
| KI_RUMBO | Corrige la torcida que queda fija para un lado. |
| LIMITE_INTEGRAL | Tope de lo que acumula KI_RUMBO. |
| MAX_CORRECCION_RUMBO | Tope de la corrección, en fuerza. |
| ZONA_MUERTA_RUMBO | Grados que deja pasar sin corregir. |
| RAMPA_MOV_PASO | Arranque suave: sube esto cada 10 ms. Menos = más suave, pero cambia msAdelante y MS_MEDIO_AL_MEDIO. |
| lateralInvertido | Tocar SOLO si el movimiento 2 (costado DERECHA) va a la IZQUIERDA. |
| camaraYInvertida | Tocar SOLO si el 2 va bien y aun así, siguiendo la pelota, se aleja de ella. |
| velLenta | De costado LENTO (1 y 2). Solo de esta prueba. |
| msCostado | Cuánto dura cada tramo de costado (ms). Solo de esta prueba. |
| msSeguir | Cuánto dura SEGUIR y cuánto espera ATAJAR (ms). Solo de esta prueba. |
| msPruebaIda | Cuánto dura la IDA del movimiento 10 (ms). Solo de esta prueba. |

---

## 4. LOS MOVIMIENTOS (1 a 10)

### 4.1 La regla de oro

**Cada movimiento hace EXACTAMENTE lo que hace el partido 0.1103.** El robot copia las
**funciones y los `case` del `loop()` del partido** (con los mismos nombres de fase) y solo cambia
**por dónde entra y dónde termina** cada prueba (tabla 4.3), con las modificaciones numeradas de la
sección 7.3. Los movimientos 1 a 5 no existen en el partido como tales: son los de
ajustar-movimientos (fuerza fija de costado), hechos con el `moverDeCostado()` del partido.

Diferencias con el partido, que la ventana dice en el texto de cada movimiento:
- El rumbo que sostiene (`rumboBase`) es el que tenía **al terminar la cuenta**, no el del armado.
- No hay patrulla. La rutina de ir al medio solo está en `!I` (y en la vuelta del lateral de 8 y 9).
- No escribe el REGISTRO DE DESPEJES (el `#MIRE` lo reemplaza).

### 4.2 Lo que mide (todas las pruebas 1 a 10)

Como `anotar()` de ajustar-movimientos, en cada vuelta del `loop()` mientras dura la prueba:
- Si en esa vuelta nadie llamó a `correccionDeRumbo()`, se llama a `medirSinCorregir()` (lee el rumbo
  sin mover nada). Así hay un error en cada vuelta, también frenando, en la pausa o enderezándose.
- Lectura muda: cuenta en `mudas`, alarga la `racha` y no hace muestra.
- `enMovimiento` = las fases que corrigen el rumbo con motores (`l`, `s` cuando se mueve, `i`, `a`,
  `c`, `v`). Con eso se cuentan `muestras`, `fuera3`, `corr`, `tope`, `medio` y el **hueco** (la
  vuelta de `loop()` más larga). La vuelta en que `ADELANTE_CHICO` hace su `delay(msFreno)` (es del
  partido, se sabe) **no cuenta** como hueco.
- `pico` / `picoms`, los `cruces` (banda ±2°) y `fin` se calculan en TODAS las fases (`fin` = el
  promedio de los últimos 150 ms de `q`).
- **Muestras para el gráfico:** una cada `MS_MUESTRA`: 10 ms en 1 a 7 y en 10, y 20 ms en 8 y 9.
  En la RAM entran hasta **1200**: si se llena, se dejan de guardar muestras, pero las cuentas siguen.
- En la 9, las cuentas de la nota se ponen en cero al entrar a `ADELANTE` (ver `#R`).

### 4.3 Cada movimiento: por dónde entra, cuándo termina, qué números muestra

**Antes de arrancar (al recibir `!n`), en este orden:**
1. si el Teensy no es el arquero → `#FRENO no es el arquero`;
2. si hace menos de 1500 ms que lo frenaron con una tecla → `#FRENO recien parado`;
3. si no hay giroscopio o está mudo: intenta arrancarlo (como `g`, con texto). Si sigue sin
   contestar → `#FRENO sin giroscopio`;
4. 1 a 5, 8, 9, A, I, F: si algún sensor de atrás ve blanco → `#FRENO sobre la linea` + `un sensor
   de atras esta sobre una linea blanca: corranlo ~10 cm para adelante`;
5. 9: si la cámara no mandó un paquete en los últimos 500 ms → `#FRENO sin camara`;
6. si pasa todo: `>>> ...` y `#CUENTA`.

**Al terminar la cuenta** (`arrancarMovimiento`): vuelve a mirar el giroscopio (si no contesta →
`#FRENO sin giroscopio`). Después pone `rumboBase = rumboActual()`, `reiniciarCorreccion()`,
`reiniciarRampaMovimiento()`, `lecturasDeLateral = 0`, `vecesSeguidas = 0`, `hayMirada = false`,
`t_finMirada = 0` (así no arranca "descansando"), `enderezandoEnEspera = ubicandose = patrullando =
empujonDeArranque = false`, empieza la medición, manda `#INICIO` y entra en la fase de la tabla.

| n | Entra por | Termina (normal) | Tope (ms) | Números en la ventana |
|---|---|---|---|---|
| 1 | fase de prueba `COSTADO`, lado -1, fuerza `velLenta` | `msCostado` cumplido → `como=NORMAL`; o 3 lecturas seguidas de `pisaElLateral(-1)` → `como=LATERAL`. Después **f** + **q** | msCostado+3000 | velLenta, msCostado, LADO_TRASERA, KP_RUMBO, KI_RUMBO, LIMITE_INTEGRAL, MAX_CORRECCION_RUMBO, ZONA_MUERTA_RUMBO, RAMPA_MOV_PASO |
| 2 | ídem, lado +1 | ídem con `pisaElLateral(+1)` | msCostado+3000 | los de 1 + lateralInvertido |
| 3 | `COSTADO`, lado -1, fuerza `pwmMaxLateral` | ídem 1 | msCostado+3000 | pwmMaxLateral, msCostado, LADO_TRASERA, KP_RUMBO, KI_RUMBO, MAX_CORRECCION_RUMBO, RAMPA_MOV_PASO |
| 4 | ídem, lado +1 | ídem 2 | msCostado+3000 | los de 3 |
| 5 | `COSTADO_VUELTA`: -1 a `pwmMaxLateral` durante `msCostado`, se da vuelta **sin frenar** (reinicia la rampa y `lecturasDeLateral`) y va `msCostado` a +1 | al terminar la vuelta, o lateral | 2·msCostado+3000 | los de 3 |
| 6 | `ADELANTE` con `lateralDelDespeje=0`, `pwmAvanceDespeje=potenciaDespeje`, `msIdaDespeje=msAdelante` (lo mismo que hace `intentarDespejar` al salir derecho) | sigue la cadena del partido: `PAUSA_MEDIO` → `ATRAS_HASTA_LINEA` → `FRENANDO` → `ACOMODANDO`/`ACOMODO_ASENTAR` → `terminarDespeje()` → (`ADELANTE_CHICO`) → al llegar a **`ENFRIANDO`** termina: **q** 500 ms. `como=SIN_LINEA` si el retroceso cortó por `MS_MAX_RETROCESO` | 20000 | potenciaDespeje, msAdelante, frenarAntesDeVolver, MS_ESPERA_ANTES_DE_VOLVER, potenciaRetroceso, msFreno, empujonFinal, potenciaEmpujon, msEmpujonFinal, KP_RUMBO, KI_RUMBO, MAX_CORRECCION_RUMBO |
| 7 | `arrancarUbicacion()` → `UBIC_ATRAS` | `UBIC_FRENANDO` → enderezarse → `terminarDespeje()` con `ubicandose` → (`ADELANTE_CHICO` si `avanzarAlUbicarse` y encontró la línea) → al llamar a **`pasarAEsperar()`** termina: **q**. **Sin** la rutina de ir al medio: si en `MS_MAX_RETROCESO` no ve la línea va a `UBIC_FRENANDO` (`como=SIN_LINEA`) | 20000 | potenciaRetroceso, avanzarAlUbicarse, potenciaEmpujon, msEmpujonFinal, msFreno, KP_RUMBO, KI_RUMBO |
| 8 | `ESPERANDO` | cuando pasaron `msSeguir` **y** está en `ESPERANDO` o `SIGUIENDO` (si está volviendo del lateral, lo deja terminar) → **f** + **q**. Sin despejar. Al pisar el lateral hace la vuelta del partido (`LATERAL_AL_MEDIO` → `MEDIO_AL_AREA` → `UBIC_FRENANDO` → enderezarse → empujón → `pasarAEsperar()`), y ahí **sigue** la prueba | msSeguir+20000 | kpLateral, pwmMinLateral, pwmMaxLateral, ZONA_MUERTA_PELOTA, predecirTrayectoria, msAnticipacionSeguir, volverAlMedioEnElLateral, LADO_TRASERA, msSeguir, KP_RUMBO, KI_RUMBO |
| 9 | `ESPERANDO` | como el partido: `SIGUIENDO` → `hayQueDespejar()` → `intentarDespejar()` → `MIRANDO` → sale o no; si no sale vuelve a `SIGUIENDO` y reintenta. Termina al llegar a **`ENFRIANDO`** después del primer despeje que SALIÓ (`sale=1`, **q** 500 ms). Si pasan `msSeguir` estando en `ESPERANDO`/`SIGUIENDO` sin haber salido → **f** + **q**, `nota=NO_SALIO`, `como=NO_SALIO` | msSeguir+20000 | umbralCm, umbralDesvio, distanciaParaMirar, medirAntesDeDespejar, MS_ASENTAR_MIRADA, MS_MIRAR, MS_MAX_MIRANDO, MS_DESCANSO_MIRADA, despejeEnDiagonal, predecirEnDiagonal, rendimientoCostado, MAX_ANGULO_DIAGONAL, potenciaDespeje, msAdelante, msSeguir |
| 10 | `ADELANTE` como el 6 pero `msIdaDespeje = msPruebaIda` | al terminar `PAUSA_MEDIO` **no vuelve**: **q** 500 ms y termina (se queda donde llegó, para medir con regla) | msPruebaIda+3000 | msPruebaIda, potenciaDespeje, frenarAntesDeVolver |

- **f** = `frenar()` durante `msFreno` (fase `f`). **q** = `parar()` 500 ms midiendo sin corregir
  (fase `q`). Después se cierra el resultado.
- En 8 y 9, `ESPERANDO` es la del partido **sin la patrulla**. Enderezarse esperando (más de
  `TOLERANCIA_ESPERA` 8°) **sí** está, y su `pasarAEsperar()` **no** termina la prueba.
- La 9 arranca esperando la pelota: la persona la tira hacia el arco **después** de la cuenta. Una
  pelota quieta puesta a 25 cm sale sin mirar (el robot ya estaba quieto), igual que en el partido.
- **Tecla o tope** en cualquier momento: `frenar()` en el acto, `#FRENO tecla` / `#FRENO tope de
  tiempo`, y se cierra el resultado **ya** (sin `q`), con `nota=NO_VALE`. Después, en ESPERA, sostiene
  el freno 400 ms y suelta (como el delantero).
- **El giroscopio se cae en el medio:** NO se frena (el partido sigue sin corregir). Se cuentan las
  mudas y la nota sale `NO_VALE` si la racha es ≥ 3 o si las mudas pasan del 2 %.

### 4.4 La nota

Es la `calcularNota()` de ajustar-movimientos, con nombres:
1. `NO_VALE` si terminó por `TECLA` o `TOPE`, o si `racha >= 3`, o si `mudas * 50 > muestras + mudas`,
   o si hubo menos de 5 muestras en movimiento.
2. (solo 9) `NO_SALIO` si no despejó.
3. `ZIGZAGUEA` si `cruces >= 4`.
4. `SE_TUERCE` si `|pico| > 10` o `|fin| > 10`.
5. `PROLIJO` si `|pico| <= 4`, `cruces <= 2` y `|fin| <= 3`.
6. Si no, `CASI`.

| Nota | Fondo | Letra | Texto en la ventana | Explicación |
|---|---|---|---|---|
| PROLIJO | `#2e7d32` | white | PROLIJO | Fue derecho: 4° o menos, casi sin zigzag, y terminó derecho. |
| CASI | `#f9a825` | `#212121` | CASI | Anda, pero se torció un poco. Mirá el consejo. |
| ZIGZAGUEA | `#ef6c00` | white | ZIGZAGUEA | Va y viene: cruzó 4 veces o más. |
| SE_TUERCE | `#c62828` | white | SE TUERCE | Se torció más de 10°. |
| NO_SALIO | `#6d4c41` | white | NO SALIÓ | En msSeguir no salió a despejar. Mirá los intentos: dicen por qué. |
| NO_VALE | `#757575` | white | NO VALE | La frenaron, se pasó del tiempo, o el giroscopio no contestó. |

### 4.5 Los consejos (los arma el robot)

Se mandan en este orden, con el valor de AHORA entre paréntesis (los textos van sin tildes):
1. `TECLA`: `Lo frenaron antes de terminar: este resultado no vale.` / `TOPE`: `Se corto por el tope
   de tiempo (N ms): no deberia pasar. Avisar.`
2. `NO_VALE` por el giroscopio: `El giroscopio no contesto (N lecturas mudas, la peor racha R
   seguidas): bateria prendida ANTES del USB, y el cable del giroscopio (29/09).`
3. `hueco > 40`: `Una vuelta del programa tardo N ms (a los M ms): eso es un RETARDO y con eso
   zigzaguea igual, toquen lo que toquen. Avisar antes de cambiar numeros.`
4. Según la nota (si no es NO_VALE). `picoRel` es `picoms` contado desde que arrancó el movimiento
   (en la 9, desde que arrancó la ida):
   - PROLIJO: `Nada que tocar: anotar los numeros en la bitacora. Probar tambien para el otro lado.`
   - CASI: si `picoRel < 300` y es 1 a 5 u 8 → `La peor torcida fue al ARRANCAR: bajar
     RAMPA_MOV_PASO de a 1 (ahora 10). OJO: cambia msAdelante y MS_MEDIO_AL_MEDIO (hay que medirlos
     de nuevo).` (**nunca** en 6, 9 y 10: en el despeje, bajar la rampa cambia cuánto sale y no
     endereza nada) · si no, si `|medio| > 2` → `Queda torcido siempre para el mismo lado: subir
     KI_RUMBO de a 0.25 (ahora 1.5).` · si no → `Esta cerca. Repetir 2 o 3 veces sin tocar nada: si
     da parecido, es asi.`
   - ZIGZAGUEA: `Corrige de mas y se pasa: bajar KP_RUMBO de a 0.5 (ahora 5.0). Si sigue, bajar
     KI_RUMBO de a 0.25 (ahora 1.5).`
   - SE_TUERCE: si `tope > 20` → `La correccion estuvo al tope el N% del tiempo y no alcanzo: subir
     MAX_CORRECCION_RUMBO de a 10 (ahora 100), o ir mas despacio.` · si no, en 6/9 con el pico en la
     pausa (`idams <= picoms < atrasms`) → `Se torcio en la PAUSA, frenado al terminar la ida (ahi no
     corrige): probar MS_ESPERA_ANTES_DE_VOLVER de a 25 (ahora 400).` (con `frenarAntesDeVolver = 0`:
     `probar frenarAntesDeVolver = 1`) · si no, en 6/9 con el pico en los primeros 300 ms del
     retroceso → `Se torcio al ARRANCAR PARA ATRAS: el partido no reinicia la rampa ahi. No hay
     numero para eso: anotarlo en la bitacora.` · si no, con el pico en una fase `f` → `Se tuerce al
     FRENAR: el freno es igual al del partido, no hay numero para tocar.` · si no → `Corrige flojo y
     se le escapa: subir KP_RUMBO de a 0.5 (ahora 5.0).`
5. 1 a 5 con `|corr| >= 15`: `Corrigio en promedio N todo el tiempo: de costado tiende a girar solo.
   Correr el MISMO movimiento para el otro lado: si la correccion cambia de signo es la mezcla
   (LADO_TRASERA 2 mas o 2 menos, ahora 89; OJO: cambia MS_MEDIO_AL_MEDIO); si tiene el mismo signo,
   es un motor que tira mas (KI_RUMBO).`
6. 6, 7, 9 con `como=SIN_LINEA` (o `lineams=0` habiendo retrocedido): `No vio la linea al volver:
   freno por tiempo (1200 ms). Mirar umbralBlanco (CANCHA, paso 1) y que haya linea atras.`
7. `acomodo == 3`: `No se pudo enderezar en 3 intentos: mirar la bateria y el piso.`
8. 8, 9 con `idas >= 3` y `velmedia < 4`: `VA Y VIENE con la pelota casi quieta. De a uno: subir
   ZONA_MUERTA_PELOTA de a 0.2 (ahora 1.4), o probar predecirTrayectoria = 0, o bajar pwmMinLateral
   de a 5 (ahora 45).`
9. 8, 9 con `conpelota < 50`: `Vio la pelota solo el N% del tiempo: ponerla de frente, a menos de 60 cm.`
10. 9, por los intentos (uno por cada `res` que aparezca):
    - MUY_ABIERTO: `N veces no salio por MUY ABIERTO (tenia que salir a mas de 30 grados). La perilla
      es MAX_ANGULO_DIAGONAL (ahora 30.0), de a 5. OJO: salir abierto y no llegar es peor que no salir.`
    - NO_ALCANZA: `N veces no salio porque el avance no alcanzaba (muy al costado o muy rapida). No se
      arregla con rendimientoCostado: ese numero tiene que ser el de verdad. Anotarlo.`
    - NO_VIENE: `N veces miro y la pelota no venia (o no llegaba a umbralCm a tiempo). Con la pelota
      quieta lejos es normal. Si pasa con pelotas que SI vienen: subir MS_MAX_MIRANDO de a 50 (ahora 500).`
    - POCOS_CUADROS: `N veces no junto 3 cuadros a tiempo: la camara manda pocos (mirar q en
      SENSORES). Revisar la camara antes de tocar MS_MIRAR.`
    - salió con `recto=3`: `Salio DERECHO porque el giroscopio no contestaba.` · con `recto=2`:
      `Salio DERECHO porque despejeEnDiagonal esta apagado.`
    - si `sale=1`: `Si NO LLEGO de costado (la pelota le quedo mas afuera): BAJAR rendimientoCostado
      de a 0.05 (ahora 0.80). Si se PASO de costado: SUBIRLO. Va al reves: mas chico = MAS costado.`
11. 9 con `NO_SALIO`: `En msSeguir (10000 ms) no salio a despejar. Mirar los intentos: dicen por que.`
12. 10: `Medir con regla desde la cinta hasta el frente del chasis y anotarlo en CANCHA (paso 5).`

Texto humano final (después de los consejos, antes del `#FIN`):
`RESULTADO: 6 DESPEJE -> CASI  pico +6.2 grados a los 520 ms, final +1.1, cruces 1, duro 3410 ms,
vuelta mas larga 12 ms` y `Grados: NEGATIVO = a la DERECHA, POSITIVO = a la IZQUIERDA.   ? = ayuda`.

### 4.6 Los textos de cada movimiento en la ventana

`MOVIMIENTOS = [dict(n, nombre, tipo, boton, titulo, que_hace, lugar, numeros)]`, con `tipo` uno de
`costado`, `despeje`, `seguir`, `atajar` o `ida`:

| n | boton | que_hace | lugar |
|---|---|---|---|
| 1 | `1   COSTADO  izq  (lento)` | De costado a la IZQUIERDA a fuerza fija (velLenta), con la corrección de rumbo del partido. Frena al pisar el lateral. Mide cuánto se tuerce. | Lugar a la IZQUIERDA (~50 cm). Sensores de atrás FUERA de la línea. |
| 2 | `2   COSTADO  der  (lento)` | Igual a la DERECHA. Es la prueba de lateralInvertido: tiene que ir a la DERECHA. | Lugar a la DERECHA. |
| 3 | `3   COSTADO  izq  (rápido)` | Como el 1 a pwmMaxLateral (la velocidad tope del seguimiento). | Lugar a la IZQUIERDA. |
| 4 | `4   COSTADO  der  (rápido)` | Como el 2 a pwmMaxLateral. | Lugar a la DERECHA. |
| 5 | `5   IDA Y VUELTA  (rápido)` | Rápido a la izquierda y se da vuelta SIN frenar, como cuando la pelota cruza siguiéndola. | Lugar a los DOS costados. |
| 6 | `6   DESPEJE  (derecho)` | El despeje del partido entero, sin pelota: ida, freno, vuelta hasta la línea, enderezarse y el empujoncito. | Delante de la línea del área, como juega. Lugar adelante (~40 cm). |
| 7 | `7   UBICARSE  (arranque)` | Lo que hace al prender la batería en el partido: atrás hasta la línea, enderezarse y avanzar el poquito. Acá NO hace la rutina del medio si no ve la línea. | A ~20 cm delante de la línea del área. |
| 8 | `8   SEGUIR  la pelota` | Sigue la pelota de costado como el partido, sin despejar, durante msSeguir. Si pisa el lateral vuelve al medio como el partido. | En su lugar de juego. Mové la pelota de un lado al otro, a ~40-60 cm. |
| 9 | `9   ATAJAR  (mirar y despejar)` | Como el partido: la sigue, a distanciaParaMirar frena y MIRA, y sale en diagonal a umbralCm. Si no sale, reintenta. Termina al primer despeje (o a los msSeguir). | En su lugar de juego. Después de la cuenta, tirá la pelota hacia el arco. |
| 10 | `10  IDA  (para medir)` | Solo la ida del despeje (msPruebaIda), frena y se queda. Para medir con regla cuánto sale. | Cinta en el frente del chasis. Lugar adelante. |

Además, en el texto de cada uno: `⚠ ` + "El rumbo que sostiene es el de cuando termina la cuenta." +
" Y el cable flojo, o SIN CABLE."

---

## 5. LAS MEDICIONES DE CANCHA (L A F C I)

### 5.1 `!L` — los sensores de línea (lógica de `calibrar-linea`, sin motores)

- **Con cable:** `!L v`, `!L i` o `!L d`. Sin cuenta: `#INICIO L LINEA_VERDE tope=5000 sincable=0`
  (o `LINEA_BLANCO_IZQ` / `LINEA_BLANCO_DER`). Durante **4000 ms** lee los 3 sensores sin parar y
  guarda, por sensor, el mín, el máx, la suma y la cantidad. Después manda `#LIN` y `#FIN L`.
- **Sin cable (guiada):** `!L c` → `#CUENTA 10.0 sincable`... → `#INICIO L LINEA_GUIADA tope=50000
  sincable=1` → con el LED, como calibrar-linea:
  - **1 destello repetido** durante 8 s: apoyarlo en el VERDE. **LED fijo** 4 s: mide la fase `v`.
  - **2 destellos** 8 s: el sensor de atrás IZQUIERDO bien sobre la línea. **Fijo** 4 s: fase `i`.
  - **3 destellos** 8 s: el DERECHO sobre la línea. **Fijo** 4 s: fase `d`.
  - **Latido lento**: terminó. Guarda las 3 fases y manda 3 `#LIN` (si hay alguien) y `#FIN L`.
- Formato: `#LIN fase=v|i|d n=<muestras> izq=min,max,prom der=min,max,prom ade=min,max,prom` (prom
  con 1 decimal). `izq` = A13, `der` = A11, `ade` = A12.
- **La cuenta la hace la VENTANA** (`calcular_umbral`, prueba con los números del 15/09):
  - `peorVerde` = el máximo de `max` de izq y der, juntando **todas** las fases `v` medidas (se puede
    medir el verde en 2 o 3 lugares).
  - `peorBlanco` = `min(izq.max de la fase i, der.max de la fase d)`. Se usa el MÁXIMO del blanco
    porque el sensor se sale un rato de la línea. Y van **separados** porque la línea de la escuela
    es más angosta que la separación de los dos sensores (15/09).
  - `umbral = (peorVerde + peorBlanco) // 2`, y los márgenes `umbral - peorVerde` y `peorBlanco - umbral`.
  - Avisos: **"SE SUPERPONEN, no hay umbral que sirva"** si `peorBlanco <= peorVerde` (revisar
    altura, suciedad, sol, que la línea sea blanca). **"Frágil"** si algún margen es menor que 100.
    **"Muestra sucia"** si en la fase de blanco `prom < (max + peorVerde) / 2` ("el sensor estuvo
    afuera de la línea buena parte del rato: el máximo igual sirve, pero repetilo bien centrado").
    El A12 se muestra, pero no entra en la cuenta.
  - Con 15/09 (verde 97, blanco 753) da **425**, a 328 de cada lado.

### 5.2 `!A` — el ancho y MS_MEDIO_AL_MEDIO (lógica de `medir-ancho`, MUEVE)

- Necesita giroscopio y que ningún sensor de atrás vea blanco (si no, `#FRENO ...` y no arranca).
- `#CUENTA` (3 s o 10 s) → `#INICIO A ANCHO tope=45000 sincable=0|1`.
- Quieto, espera que la calibración del giro (`getCalibration`) llegue a **3**, como mucho 10 s.
  Manda `#PASO A calibrando giro=N` cada 1 s.
- Si ahora un sensor ve blanco → error 2. Fija `rumboBase`.
- Los 4 tramos, a `VEL_MEDIO_MEDIR` (100) con el `moverDeCostado()` del partido (rampa, mezcla y
  corrección de rumbo con los números de AHORA):

  | tramo | hacia | termina | se anota |
  |---|---|---|---|
  | 0 | IZQUIERDA | `llegoAlCostado(-1)` (sensor de ese lado en blanco Y el otro NO) | `medioizq` |
  | 1 | DERECHA | `llegoAlCostado(+1)` | `izqder` |
  | 2 | IZQUIERDA | `llegoAlCostado(-1)` | `derizq` |
  | 3 | DERECHA | por tiempo: `msHastaElMedio(izqder)` | (comprueba: tiene que quedar en el medio) |

- Topes por tramo (`topeDelTramo` de medir-ancho): tramo 0, 4000; tramo 1, `medioizq*3+500`; tramo 2,
  `izqder*13/10+300`; nunca más de 6000. Si no llega → error 4.
- `numerosCreibles` al terminar el tramo 1 (`izqder/medioizq` entre 1,4 y 3) y el 2 (`derizq/izqder`
  entre 0,7 y 1,43). Si no da → error 5.
- Entre tramos: `frenar()` 200 ms, quieto 300 ms. Si quedó torcido más de 3°, se endereza como
  medir-ancho (giro en el lugar, PWM 75..85, hasta 2500 ms) y suma `endereza`. Ahí manda
  `#PASO A tramo=N ms=<lo que tardó> desvio=<peor del tramo> endereza=<total>`.
- Giroscopio caído (10 lecturas mudas seguidas) → error 3.
- `msHastaElMedio(cruce) = cruce/2 + perdida/2`, con `perdida = (VEL_MEDIO_MEDIR / RAMPA_MOV_PASO) *
  RAMPA_MOV_MS / 2 + 33` (a 100 con la rampa 10/10 da 83, o sea `cruce/2 + 41`).
  `almedioizq = msHastaElMedio(izqder)` y `almedioder = msHastaElMedio(derizq)`.
  **Comprobación: 4344 → 2213, 4405 → 2243, 4547 → 2314, 4615 → 2348.**
- Errores: 0 ok, 2 arrancó sobre una línea, 3 sin giroscopio, 4 no encontró la línea, 5 números
  raros, 6 lo frenaron, 7 tope total. Con error, `#ANCHO ok=0 error=N` con lo que haya medido hasta ahí.
- Formato: `#ANCHO ok= error= medioizq= izqder= derizq= almedioizq= almedioder= desvio=a,b,c,d
  endereza= golpe= vel= rampa= umbral= calgiro=` (`desvio` = el peor |desvío| de cada tramo con 1
  decimal; `golpe` = la peor torcida al llegar a un lateral; `vel`, `rampa` y `umbral` = con qué se
  midió; `calgiro` = la calibración del giro al arrancar).
- **No escribe la EEPROM 0..255 de medir-ancho.** Guarda en el bloque de cancha del tablero (6.2).

### 5.3 `!F` — el fondo, MS_MEDIO_FONDO_POR_TIEMPO (nuevo, MUEVE para atrás)

- **Medir (con cable):** `!F` → cuenta 3 s → `#INICIO F FONDO tope=6000 sincable=0` → `atrasControlado(
  VEL_MEDIO_RECTO)` (110) sosteniendo el rumbo, con el cronómetro andando. Termina:
  - con la letra **`t`** (el botón TOCÓ) → `como=TOCO`;
  - si algún sensor de atrás ve blanco → `como=LINEA`;
  - a los 4000 ms → `como=TOPE`;
  - con cualquier otra tecla → `#FRENO tecla` y `como=PARADO`.
  Después: `frenar()` `msFreno`, `parar()`, `#FONDO`, `#FIN F`.
- **Comprobar (sin cable):** `!F c` → 10 s → `#INICIO F FONDO_COMPROBAR tope=8000 sincable=1` →
  retrocede `MS_MEDIO_FONDO_POR_TIEMPO` (el de ahora) o hasta la línea → `como=TIEMPO` o `LINEA`. La
  ventana pregunta después si llegó a tocar la rampa.
- Formato: `#FONDO ms=<lo que anduvo> como=TOCO|LINEA|TOPE|TIEMPO|PARADO desvio=<peor> umbral=<umbralBlanco>
  probado=<ms que se probó en comprobar, 0 al medir>`.
- La ventana: `sugerido = max(ms de las corridas TOCO) + 500`, redondeado para arriba a 50 (la regla
  escrita en el panel). Si alguna dio `LINEA`: "esta cancha tiene línea de fondo alcanzable"; y si
  frenó por una línea ANTES del fondo: "hay otra línea en el camino y la rutina del partido cortaría ahí".

### 5.4 `!C` — la cámara con esta luz (lógica de `tabla-camara`, no mueve)

- `!C` → sin cuenta → `#INICIO C CAMARA tope=4000 sincable=0` → 3000 ms juntando paquetes (los del
  `leerCamara()` del partido) → `#CAM` → `#FIN C`. `!C c` → 10 s de cuenta y lo mismo.
- Formato: `#CAM n=<paquetes> cero=<con Xp=0> val=<con Xp>0> xp=min,max,prom yp=<Yp promedio CRUDO de
  los válidos> am=<paquetes con el amarillo> az=<con el azul>` (`xp=0,0,0.0` si `val=0`).
- La ventana:
  - **4a control rápido:** pelota en una cinta a 30 cm del frente del chasis → `cm = prom /
    CAMARA_POR_CM` (tiene que dar 27 a 33) y `factor = prom / 30` (fuera de 2,6..3,2 → pedir la 4b).
    **Sin pelota** → `val` tiene que ser 0 (si no: "hay otra cosa naranja: sacarla o taparla").
  - **4b tabla:** "MEDIR 10 / 20 / 30 / 40 / 50 cm" y "SIN PELOTA" (la persona aprieta cuando la
    pelota ya está: así no se corre la tabla, como pasó el 25/08) → recta por cuadrados mínimos
    `cámara = a·cm + b` → `CAMARA_POR_CM` sugerido = `a` con 2 decimales. **Con los datos del 25/08
    (10→28, 20→58, 30→88,9, 40→117, 50→142) da a = 2,87 y b = 0,68.** Si el factor crece con la
    distancia: "inclinación: no alcanza con dividir".
  - El umbral naranja del script de la cámara **no se toca** desde acá (es del delantero también).

### 5.5 `!I` — ir al medio (la rutina del partido, para comprobar MS_MEDIO_AL_MEDIO)

- Necesita giroscopio y no estar sobre una línea. Cuenta → `#INICIO I IR_AL_MEDIO tope=40000
  sincable=0|1` → entra como el partido cuando no encuentra la línea: `ubicandose = true`,
  `encontroLaLinea = false`, `pausaMedio(MEDIO_AL_LATERAL)`, y la cadena del partido **de la 0921g**
  (pasos 1 a 7 → `UBIC_FRENANDO` → enderezarse → empujón) hasta `pasarAEsperar()`.
- (2026-11-09, revisión) **NO tiene la vuelta con la línea del área de la 0921i**: el paso 6 (al medio) es SOLO por
  tiempo (`MS_MEDIO_AL_MEDIO`). La 0921i, con `buscarLineaDelAreaAlVolver` true, corta al ver la línea del área
  (`MEDIO_LINEA_AVANZAR` con `MS_AVANCE_10CM`, `MEDIO_LINEA_AL_MEDIO` con `MS_PUNTA_A_PUNTA_FIJO / 2` a `VEL_LINEA`).
  El paso 2c lo dice ("vuelve al medio SOLO POR TIEMPO") y manda a verlo con el partido cargado. Copiarla a
  `tablero.ino` (con esos 4 en la RAM y en el `!I`) queda para después: no se pudo probar con el robot.
- En cada `MEDIO_PAUSA` (robot frenado) manda `#PASO I paso=N ms=<lo que duró> como=L|T`.
- Formato: `#MEDIO ok=<1 si terminó encontrando la línea del área sin abandonar> pasos=p1,...,p8
  como=c1,...,c8 desvio=<peor> acomodo=<reintentos>`. Los pasos son: 1 al lateral, 2 alejarse, 3 al
  fondo, 4 adelante, 5 al lateral de nuevo, 6 al medio, 7 atrás hasta el área, 8 empujón. `como` es
  `L` (vio la línea), `T` (por tiempo) o `-` (no lo hizo).
- La ventana pregunta "¿quedó en el medio? SÍ / corrido a la IZQ __ cm / a la DER __ cm" y avisa si
  el paso 5 no tardó mucho menos que el 1 (tendría que estar ya cerca del lateral).

---

## 6. SIN CABLE Y LA EEPROM

### 6.1 Las reglas del modo sin cable

- Lo pide la orden (`c` al final). No es un modo que quede prendido en el robot.
- La cuenta es de **10 s** (como `MS_CUENTA_SIN_CABLE` de ajustar-movimientos).
- **El robot NO detecta que se desenchufó, a propósito**: no se frena porque se cortó el cable. Se
  frena con PARAR (si el cable está), con su tope o al terminar.
- Mientras se mueve no imprime nada. Al terminar, frena, arma el resultado, lo **guarda en la
  EEPROM** (siempre, con o sin cable), y lo imprime solo si hay alguien escuchando (`if (Serial)`).
  Si no había nadie, queda `nuevo=1`. **Sin cable queda `nuevo=1` siempre** (03/11): en el core del
  Teensy 4.1 `Serial` es "USB configurado y DTR arriba", y eso no cambia al desenchufar (no mira el
  VBUS ni la suspensión), así que `if (Serial)` no sirve para saber si alguien lo vio.
- Al volver a enchufar (en ESPERA): `#HOLA` + `#GUARDADO` (sección 2.5).
- **La EEPROM se escribe solo con el robot quieto** (después de `frenar()`), nunca en MOVIENDO ni en
  la CUENTA. Puede tardar unos ms: no importa, ya está frenado.

### 6.2 Los dos bloques del tablero

```
DIR_MOV    = 2048   tamaño 896   (2048..2943)   el último MOVIMIENTO (1 a 10)
DIR_CANCHA = 2944   tamaño 128   (2944..3071)   la última MEDICIÓN DE CANCHA (L A F C I)
```
Cada bloque empieza con una cabecera de 12 bytes:

| Campo | Tipo | Qué es |
|---|---|---|
| `magia` | uint32 | `0x41544D31` ("ATM1") en el de movimiento, `0x41544331` ("ATC1") en el de cancha |
| `version` | uint8 | 1 |
| `codigo` | uint8 | 1..10, o el carácter `'L' 'A' 'F' 'C' 'I'` |
| `nuevo` | uint8 | 1 = nadie lo vio |
| `sinCable` | uint8 | 1 = se pidió sin cable |
| `largo` | uint16 | `sizeof` de lo guardado (para validar) |
| `libre` | uint16 | 0 |

- Al leer: si la magia, la versión, el largo o el código no dan, es **"no hay nada guardado"** (una
  EEPROM virgen viene con 0xFF). Nunca se imprime basura.
- **Bloque de movimiento** (con `static_assert(sizeof(...) <= 896)`): los campos del `#R` (en
  décimas los grados, enteros el resto), hasta 10 `#MIRE` (décimas, int16), los `#USADOS` (hasta 12
  pares índice uint8 + float; si hubo más, se guarda cuántos) y **hasta 100 muestras** (t uint16,
  décimas int16 y fase char, en tres arreglos). Si en la RAM hubo más de 100, se guarda **una de cada
  k** (`k = ceil(n/100)`). Si con todo eso no entra en 896, se guardan hasta 8 `#MIRE` en la EEPROM
  (en la RAM siguen 10) y se avisa en el informe del constructor.
- **Bloque de cancha** (`static_assert(<= 128)`): según el código, los `#LIN` (hasta 3 fases: fase,
  n, y mín/máx/promedio×10 de los 3 sensores), o el `#ANCHO`, el `#FONDO`, el `#CAM` o el `#MEDIO`.
- Con `r`/`rc`: si el resultado sigue en la RAM (`mismo=1`), se manda de la RAM (todas las muestras,
  `apagado=0`). Si no, se manda de la EEPROM (`apagado=1`). Los consejos se vuelven a calcular con
  los números de AHORA. Si `apagado=1` y había `#USADOS`, se agrega
  `#CONSEJO Se corrio con numeros cambiados que se perdieron al apagar: los consejos usan los de ahora.`
- Después de mandarlo con alguien escuchando, `nuevo = 0` (en la RAM y en el byte de la cabecera),
  salvo al terminar una prueba SIN CABLE (ver 6.1): esa solo pasa a 0 con `r`/`rc`.

---

## 7. EL PROGRAMA DEL ROBOT (`pruebas/tablero/tablero.ino`) — para el constructor ROBOT

### 7.1 `platformio.ini` (el patrón del arquero, otro build_dir)

```ini
; PlatformIO — TABLERO del ARQUERO (el programa del robot que va con la ventana).
; Tambien se abre tal cual con el Arduino IDE (Teensy 4.1).

[platformio]
src_dir = .
build_dir = ${PROJECT_DIR}/../../../../.pio-build/tablero-arquero

[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
upload_protocol = teensy-cli
monitor_speed = 19200
lib_deps =
    adafruit/Adafruit BNO055
    adafruit/Adafruit Unified Sensor
```
- `tablero-arquero`, **no** `tablero`: ese es el `build_dir` del delantero. `teensy-cli` (el del
  arquero, `cargar.py` lo usa con `-e teensy41`), no `teensy-gui`.
- Si la compilación falla con `SPI.h: No such file`, agregar `SPI` a `lib_deps`, como hizo el delantero.
- La primera compilación baja las librerías a `pruebas/tablero/.pio/libdeps` (necesita internet). Si
  no hay internet, **copiar** (no mover) `pruebas/ajustar-movimientos/.pio/libdeps/teensy41` a
  `pruebas/tablero/.pio/libdeps/teensy41`. **Compilarlo acá antes del viaje.**

### 7.2 El orden del archivo

1. **Comentario grande:** qué es; los 10 movimientos y las 5 mediciones; qué mide; cómo se usa (con
   cable y SIN CABLE); las órdenes; qué NO hace a propósito (no arranca solo nunca, no patrulla, no
   escribe el registro del partido); 🚨 que MUEVE el robot; **COPIAS DEL PRINCIPAL** ("copia de
   funciona/seguir-y-despejar version 0.1103: si el partido cambia, cambiarlo aca tambien"); el
   mapa de la EEPROM; el contrato completo.
2. `#include <Arduino.h>`, `<Wire.h>`, `<EEPROM.h>`, `<Adafruit_Sensor.h>`, `<Adafruit_BNO055.h>`.
3. `SALUDO_PROGRAMA = "tablero-arquero"`, `FECHA_CONTRATO = "2026-11-03"`, `PARTIDO_COPIADO = "0.1103"`,
   `SERIE_ARQUERO = 13348550`.
4. Pines, **COPIA del partido**: `INA1 2, INB1 5, PWM1 3` (izq), `INA2 8, INB2 7, PWM2 6` (der),
   `INA3 11, INB3 12, PWM3 4` (trasera), `LED 13`, `LINEA_ATRAS_DER A11`, `LINEA_ATRAS_IZQ A13`,
   `LINEA_ADELANTE A12`. `BAUDIOS 19200`.
5. **Todos los tipos propios** (`enum Modo { ESPERA, CUENTA, MOVIENDO }`, el `enum Fase` del partido
   con las fases de prueba agregadas al final, `enum Resultado`, `struct Ajuste`, `struct Intento`,
   los `struct` de la EEPROM) y **enseguida, A PROPÓSITO, `void paraQueCompile() {}` como PRIMERA
   función**. PlatformIO mete sus prototipos antes de la primera función. Con la tabla larga
   arriba los metía adentro de la tabla, y no compilaba (pasó en ajustar-movimientos y en el delantero).
6. Los 46 números (variables, con comentario `del PANEL DE CONTROL de la 0.1103` o `solo de esta
   prueba`) y los fijos (`const`, copia del partido).
7. `ajustes[]`: `{"nombre", tipo, &var, min, max, "declaracion en el panel", 0}` (el defecto se llena
   en `setup()` con `valorDe`). Para los bool, `p` apunta a un `bool`.
8. El estado global copiado del partido (giroscopio, cámara, mirada, despeje, lateral, ubicación,
   medio...) + el del tablero (modo, prueba, medición, muestras, intentos).
9. **LAS COPIAS DEL PRINCIPAL**, en el orden del partido: `parar`, `frenar`, `rueda`, `aplicar`,
   `reiniciarRampaMovimiento`, `rampa`, `rumboActual`, `diferencia`, `reiniciarCorreccion`,
   `correccionDeRumbo`, `veBlancoIzq/Der`, `algunoDeAtrasVeBlanco`, `pisaElLateral`, `moverDeCostado`,
   `adelanteControlado`, `avanzarPersiguiendo`, `atrasControlado`, `leerCamara`, `veLaPelota`,
   `hayQueDespejar`, `distanciaPelota`, `desvioPelota`, `desvioPredicho`, `calcularDiagonal`,
   `atrasEnDiagonal`, `empezarMirada`, `agregarMuestraMirada`, `calcularMirada`, `pasarAEsperar`,
   `arrancarUbicacion`, `pausaMedio`, `abandonarMedio`, `enFaseDeMedio`, `cortarPatrulla`,
   `arrancarVueltaDelLateral`, `pasarAEnderezarse`, `intentarDespejar`, `terminarDespeje`, y el
   `switch` del `loop()` del partido adentro de `void pasoDelPartido(unsigned long ahora)`, con el
   chequeo de arriba del `loop()` que corta la patrulla (`if (patrullando && enFaseDeMedio() &&
   veLaPelota() && cuadrosConPelota >= CUADROS_PARA_CORTAR_PATRULLA) cortarPatrulla();`), porque
   la vuelta del lateral de 8 y 9 lo usa.
10. Las fases de prueba (`COSTADO`, `COSTADO_VUELTA`, el freno y el asentar del final), copiadas de
    ajustar-movimientos.
11. La medición (`medirSinCorregir`, `anotar`, `calcularNota`) y el informe (`#M`, `#MIRE`, `#R`,
    `#USADOS`, los consejos, `#FIN`).
12. La EEPROM (guardar, leer, validar, repetir).
13. Las mediciones de cancha (L A F C I).
14. La máquina ESPERA / CUENTA / MOVIENDO (`empezarCuenta`, `pasoDeLaCuenta`, `arrancarMovimiento`,
    `terminarPrueba`, `noArranca`, `hayTeclaDeVerdad`, `tirarLoQueLlego`).
15. Las ayuditas sin `String` y sin tildes (`finDeLinea` solo `'\n'`, `decir`, `imprimirDecimal` con 1 o
    2 decimales, `imprimirGrados` sin "-0.0", `imprimirNumero(v, tipo)`, `mismoNombre` sin mayúsculas),
    la tabla (`valorDe`, `ponerValor`, `cambiado`, `buscarAjuste`, `mandarValores`, `mandarDefectos`,
    `volverAlPrincipal`, `mandarPanel`, `ayuda`), `mandarEstado()` (el `#S`) y las órdenes
    (`cambiarNumero`, `procesarLinea`, `leerOrdenes`, `mandarError`). Copiar del tablero del delantero
    lo que sea igual (validaciones de `cambiarNumero`, `leerOrdenes` con búfer de 72).
16. `setup()` y `loop()`.

### 7.3 Lo ÚNICO que se cambia de la copia del partido

Cada cambio va marcado en el código con `// TABLERO (2026-11-03): ...` explicando por qué.
- **M1.** Los `const` del panel que están en la tabla pasan a variables con el mismo nombre.
- **M2.** `correccionDeRumbo()`: además anota `ultimaMuda`, `ultimoError`, `ultimaCorreccion` y
  `leyoGiroEnEstaVuelta = true` (como ajustar-movimientos). No cambia la cuenta.
- **M3.** `calcularDiagonal()`: al principio pone en cero lo que anota (`diagD`, `diagY`, `diagVy`,
  `diagVa`, `diagAvance`, `diagCostado`, `diagRecto`), y antes de cada `return` anota lo que vio
  (`diagCostado` antes de que `COSTADO_MINIMO_UTIL` lo pase a 0; `diagRecto` según por qué salió
  derecho). **No cambia ninguna cuenta.** Así se arregla de paso el problema 3 de ajustar-movimientos
  (el ángulo de la pasada anterior).
- **M4.** `leerCamara()`: además guarda los arcos (`paquete[4]`, `paquete[5]-100`, `paquete[7]`,
  `paquete[8]-100` con su hora si X>0), la última pelota buena (`XpBueno`, `YpBueno`, `t_pelotaBuena`),
  cuenta paquetes (para `q` y para `!C`) y suma a la medición de `!C` si está andando. No cambia
  nada de lo que ya hacía.
- **M5.** `intentarDespejar()` y `MIRANDO`: `registrarIntento(...)` → `anotarIntento(...)` (misma
  firma, misma regla de "un mismo no-sale una vez por segundo", pero guarda en la RAM para `#MIRE`,
  con `t`, `ang` y `recto` sacados de M3). **Se sacan los `Serial.print` de MIRANDO.**
- **M6.** `ESPERANDO`: sin la rama de la patrulla. `hayQueDespejar()` solo en la prueba 9.
- **M7.** `SIGUIENDO`: el bloque de `hayQueDespejar()` solo en la prueba 9.
- **M8.** `UBIC_ATRAS`: en la prueba 7, en el tope va a `frenar(); fase = UBIC_FRENANDO;` (sin la
  rutina del medio).
- **M9.** Las salidas de la tabla 4.3: `pasarAEsperar()` termina la prueba en 7 e I; llegar a
  `ENFRIANDO` la termina en 6 y 9; el final de `PAUSA_MEDIO` la termina en 10; `msSeguir` en 8 y 9.
- **M10.** `ADELANTE_CHICO`: el `delay(msFreno)` queda, pero esa vuelta no cuenta como hueco.
- **M11.** **NO se copia nada del REGISTRO DE DESPEJES** (`cargarRegistro`, `guardarRegistro`,
  `imprimirRegistro`, la impresión al conectar, el guardado en ENFRIANDO). El tablero no escribe
  3072..3583.
- **M12.** `APAGADO`, `ARMANDOSE` y `arrancarPatrulla` no se usan: el tablero **nunca arranca solo**.
- **M13.** El aviso del LED "sin giroscopio" del final del `loop()` del partido corre solo en
  MOVIENDO; en ESPERA el LED lo maneja el tablero (7.8).

### 7.4 La vista previa de la diagonal (el `dg=` del `#S`)

En ESPERA, cada 100 ms, antes de armar el `#S`: si `veLaPelota()` y el giroscopio contesta, guarda
`rumboBase`, pone `rumboBase = rumboActual()` y `hayMirada = false`, llama a `calcularDiagonal(av,
lat, ms)`, toma `motivoNoSale` y **restaura `rumboBase`** (en ESPERA vale -1). Sin pelota o sin
giroscopio manda lo que devuelva igual (`1,0,200,0,383` = "sin datos sale derecho"); la ventana
mira `ve` antes de creerle. **Ejemplos para comprobar a mano** (con los números de hoy y el robot quieto):

| d | y | vy | va | ¿miró? | dg |
|---|---|---|---|---|---|
| 29.97 | 4.18 | 0 | 0 | no | `1,0,193,33,396` |
| 30 | 0 | 0 | 0 | no | `1,0,200,0,383` |
| 30 | 10 | 0 | 0 | no | `1,0,173,72,442` (18,4°) |
| 20 | 15 | 0 | 0 | no | `0,2,142,0,383` (necesitaba 36,9°) |
| 28 | -6 | -10 | 30 | sí | `1,0,167,-85,458` |
| 29.6 | -7.4 | -10 | 30 | sí | `1,0,162,-94,472` (25,0° a la izquierda) |
| 25 | 14 | 30 | 40 | sí | `0,1,107,0,383` |

### 7.5 `setup()` y `loop()`

`setup()`:
1. Guarda los defectos (`valorDe` de cada ajuste). `pinMode` de todo y `parar()`.
2. `Serial.begin(19200)`, `Serial1.begin(19200)` (la cámara), `while (!Serial && millis() < 3000) {}`.
3. Lee su propia serie (7.6). `#HOLA`, el cartel humano `=== TABLERO DEL ARQUERO 2026-11-03 (copias
   del partido 0.1103) - NO ES EL DE PARTIDO ===`, `Se mueve SOLO al pedirlo, despues de la cuenta.
   Cualquier tecla lo frena.` y, si la serie no es la del arquero, `ESTE TEENSY NO ES EL ARQUERO (serie
   N): no muevo nada.`
4. `#GIRO buscando`. Arranca el giroscopio **como el partido** (`bno.begin()` sin argumentos = modo
   NDOF; `delay(1000)`; `setExtCrystalUse(true)`; espera hasta 5 s a que dé un dato), pero con 3
   intentos (como ajustar: "aca no hay apuro"). `#GIRO ok=.. estado=..`. **Nunca `getSystemStatus()`**:
   tiene un `delay(200)` adentro.
5. Lee los dos bloques de la EEPROM. `#V`, `#DEF`, los dos `#GUARDADO` y la ayuda.
6. `estabaConectado = (bool)Serial`, así el `loop()` no manda un segundo `#HOLA` enseguida.

`loop()`:
- **MOVIENDO:** mide el hueco; si `hayTeclaDeVerdad()` → frena (en `!F`, la `t` es TOCÓ); `leerCamara()`;
  si `ahora - t_inicio > tope` → `TOPE`; el paso de la prueba (`pasoDelPartido`, las fases de prueba
  o la medición de cancha); la medición; `return`. **Nada por Serial** (salvo `#PASO` de A e I).
- **QUIETO:** `leerCamara()`; los 400 ms después de un freno, `frenar()`; si no, `parar()` en cada
  vuelta (ruedas sueltas, para acomodarlo a mano); en CUENTA → `pasoDeLaCuenta()` y `return`. En
  ESPERA: el LED (7.8), mira si se abrió el puerto (→ `#HOLA` + `#GUARDADO`), `leerOrdenes()`, cada
  1 s los paquetes por segundo, y cada 100 ms el `#S` (solo si `Serial`).

### 7.6 La protección propia: "¿soy el arquero?"

El programa lee el número de serie de SU Teensy igual que lo calcula Teensyduino:
```cpp
uint32_t serieDeEsteTeensy() {
  uint32_t num = HW_OCOTP_MAC0 & 0xFFFFFF;
  if (num < 10000000) num = num * 10;   // igual que usb_init_serialnumber() del Teensy 4
  return num;
}
```
Si no es 13348550, el robot **no mueve nada nunca**: toda orden `!` contesta `#FRENO no es el
arquero`. Así, aunque alguien le cargue este programa al delantero, no mueve las ruedas cruzadas.
Si esto no compila, sacarlo, avisarlo en el informe y dejar el resto (las otras protecciones siguen).

### 7.7 La cuenta y el arranque

- `empezarCuenta(codigo, sinCable)`: los chequeos de 4.3. Si pasan, `modo = CUENTA`, `>>> ...` y el
  primer `#CUENTA`. El LED titila cada 100 ms.
- `pasoDeLaCuenta()`: una tecla → `#FRENO tecla`, `cancelado: no se movio`, `tirarLoQueLlego()` y
  ESPERA. Manda los `#CUENTA` a sus tiempos (solo si hay alguien). Al cumplirse, `arrancarMovimiento()`.
- `tirarLoQueLlego()`: durante 50 ms vacía lo que llegue (por ejemplo el `\n` de `"x\n"`).
- `terminarPrueba(como)`: en este orden, `frenar()` (lo primero); `enMovimiento = false`; (si
  corresponde, la fase `f` y la `q` de la tabla 4.3); cerrar las cuentas; la nota; los consejos;
  **guardar en la EEPROM**; mandar la tanda si hay alguien (si no, `nuevo = 1`); `modo = ESPERA` con
  `t_freno` (sostiene el freno 400 ms y suelta).

### 7.8 El LED

| Cuándo | LED |
|---|---|
| ESPERA, sin resultado todavía | cambia cada 1 s |
| ESPERA con resultado de movimiento | destellos con la nota: 1 PROLIJO, 2 CASI, 3 ZIGZAGUEA, 4 SE_TUERCE, 5 NO_SALIO; **temblando** (50 ms) = NO_VALE |
| ESPERA con resultado de cancha | latido lento (un destello corto cada 2 s) = bien; destellos = el código de error del ancho (2, 3, 4, 5) |
| ESPERA sin giroscopio | temblando (50 ms), como el partido |
| CUENTA | titila cada 100 ms |
| MOVIENDO | lo que haga el partido en esa fase; la línea guiada usa sus destellos (5.1) |

Destellos = `veces` destellos de 200 ms cada 400 ms y una pausa de 1200 ms (la `destellos()` de medir-ancho).

---

## 8. LA VENTANA (`pruebas/tablero/tablero.py`) — para el constructor VENTANA

### 8.1 El armazón: copiar el del delantero

Las mismas 7 partes, en el mismo orden y con los mismos nombres:
1. EL CONTRATO (constantes),
2. EL TRADUCTOR (`interpretar` y los `_i_*`),
3. LA MEMORIA (`class Estado` + cuentas puras),
4. EL CABLE (`class Conexion`),
5. EL ROBOT DE MENTIRA,
6. LA VENTANA (`class Ventana`),
7. LA AUTOPRUEBA + `main()`.

Va con `# -*- coding: utf-8 -*-`, el docstring para los chicos, y tkinter/pyserial en try/except.
Se copia casi tal cual: el traductor, `Estado.aplicar`, `Conexion`, la cola de 50 ms, el latido de
250 ms, `_mandar` como ÚNICA salida, `_reconocer_tablero`, `_puede`/`_actualizar_habilitados`, PARAR
(tk.Button rojo que nunca se deshabilita, y Esc), la CONSOLA, `_mostrar_panel` con portapapeles, las
fallas (`tablero-error.txt`), `nuevo_valor`, `normalizar_entrada`, `desenrollar`, las escalas, el
gráfico y el estilo (`clam`, "Segoe UI", los colores, `state("zoomed")`, minsize 1000x640).

**Constantes del arquero:**
```python
SERIE_ARQUERO = "13348550"
SERIE_DELANTERO = "15708680"          # a este NUNCA se le abre el puerto
VID_TEENSY = 0x16C0
PID_SERIE = 0x0483
BAUDIOS = 19200
PROGRAMA = "tablero-arquero"
FECHA_CONTRATO = "2026-11-03"
ETIQUETAS = ("HOLA", "S", "V", "DEF", "OK", "ERROR", "GIRO", "CUENTA", "INICIO", "PASO", "FRENO",
             "M", "MIRE", "R", "USADOS", "CONSEJO", "FIN", "REPITE", "GUARDADO",
             "LIN", "ANCHO", "FONDO", "CAM", "MEDIO", "PANEL", "FINPANEL")
MODOS = ("ESPERA", "CUENTA", "MOVIENDO")
CODIGOS = ("1","2","3","4","5","6","7","8","9","10","L","A","F","C","I")
FASES = {...}            # las 11 de la sección 2.4, con nombre y color
CAMPOS_R = (...)         # los 29 del #R, en orden
NOTAS = {...}            # las 6 de la sección 4.4
RESULTADOS_MIRE = ("SALE_MIRANDO", "SALE_SIN_MIRAR", "MUY_ABIERTO", "NO_ALCANZA", "NO_VIENE", "POCOS_CUADROS")
MAX_MUESTRAS = 2000
EDAD_VIEJA_MS = 300
```
Además `NUMEROS`, `MOVIMIENTOS` (sección 4.6), `VALORES_DE_MENTIRA` y `RANGOS_DE_MENTIRA` (los dos
salen de la tabla de la sección 3: **los valores de la demo son los del partido**, no como en el
delantero, que quedaron viejos; y **las pruebas los calculan del diccionario**, no con números fijos).

**Rutas** (desde `CARPETA` = la del .py): `PARTIDO_INO = ../../funciona/seguir-y-despejar/seguir-y-despejar.ino`,
`CARGAR_PARTIDO_BAT = ../../funciona/seguir-y-despejar/CARGAR-PARTIDO.bat`, `CANCHAS = CARPETA/canchas`.

### 8.2 El traductor: lo que cambia respecto del delantero

- Regex de nombre: `^[A-Za-z][A-Za-z0-9_]*$`.
- `_i_hola`: el programa (1er campo), la fecha (2do, si no tiene `=`) y los pares (`serie`, `partido`).
- `_i_s`: exige `t` primero y `e` último; `l` 3 números, `u` 1, `cal` 4, `p`/`am`/`az` 3 (`(x, y,
  edad)`), `dg` 5 (`sale, motivo, avance, lateral, msIda`); `r < 0` y `g < 0` dan None; `d < 0` es
  "nunca la vio". Los campos de más se ignoran.
- `_i_cuenta`: `(segundos, modo)` con `modo` en `cable`/`sincable`.
- `_i_inicio`: `(codigo en CODIGOS, NOMBRE, tope, sincable)`.
- `_i_m`: `fase` en FASES y `t >= 0`.
- `_i_r`: exige `n`, `mov` y `nota` (en NOTAS). Los que faltan quedan None.
- Nuevos: `_i_def` (como `_i_v`), `_i_giro`, `_i_paso` (código + pares), `_i_mire`, `_i_usados`
  (pares, puede venir vacío), `_i_fin` (código), `_i_repite`, `_i_guardado`, `_i_lin`, `_i_ancho`,
  `_i_fondo`, `_i_cam`, `_i_medio`. Todos atrapan `ValueError`, `KeyError`, `IndexError`, `TypeError`
  y `OverflowError` y devuelven ROTA.
- `reconocer_programa(texto)` (pura): para los renglones humanos, devuelve `PARTIDO <version>`,
  `AJUSTAR`, `TABLERO_ARQUERO` o None (sección 2.6).

### 8.3 La memoria: lo que cambia

- Guarda además: `defectos` (de `#DEF`), `giro` (`buscando`/ok/estado), `guardado` (`mov` y
  `cancha`), `intentos` (`#MIRE`), `usados`, `pasos` (`#PASO`), `cancha` (el último resultado de cada
  código y fase), `de_guardado` (el resultado vino de `#REPITE`), `apagado`.
- Reglas nuevas de `aplicar`: `CUENTA` con `sincable` prende `esperando_sin_cable`. `INICIO` borra el
  resultado y pasa a MOVIENDO. `REPITE` empieza un resultado nuevo **sin** cambiar el modo. `PASO` no
  cambia el modo. `FIN` → ESPERA y "resultado completo" (y apaga `esperando_sin_cable`). `R` **ya no**
  cambia el modo (lo hace `FIN`). `GUARDADO` con `nuevo=1` deja un pedido pendiente de `r`/`rc`.
  Las mediciones de cancha se guardan por código. El resto, como el delantero (`S` en espera saca de
  MOVIENDO; `M` con informe viejo empieza otro; `FRENO` sin MOVIENDO borra la nota vieja).
- `reiniciar_conexion()` olvida `hola`, sensores, modo y cuenta, pero **conserva** el resultado y
  `esperando_sin_cable`.

**Cuentas puras nuevas (todas con prueba):**
- `cm_a_pantalla(x_cm, y_cm, cx, cy, escala) = (cx + y*escala, cy - x*escala)`. ⚠ En el arquero
  **Y positivo es la DERECHA** (al revés que la del delantero): no copiar el signo de allá.
- `que_haria(sensores, valores) -> (codigo, frase)`. Con el robot quieto:
  - `NO_LA_VE` si `ve=0`.
  - `MUY_AL_COSTADO` si `|y| > umbralDesvio` ("la sigue de costado").
  - `LEJOS` si `d > distDisparo` ("la sigue de costado"); `distDisparo = distanciaParaMirar` con
    `medirAntesDeDespejar`, si no `umbralCm`.
  - `MIRARIA` si `umbralCm < d <= distDisparo` y `n == 3` ("se frenaría a MIRAR").
  - `SALDRIA` si `d <= umbralCm`, `n == 3` y `dg.sale == 1`, con el ángulo `atan(lat*rendimientoCostado/av)`
    ("SALDRÍA YA, derecho" o "en diagonal 8° a la DERECHA").
  - `NO_SALDRIA` si está en zona de salir y `dg.sale == 0` ("NO saldría: muy abierto / el avance no alcanza").
  - `JUNTANDO` si está en zona pero `n < 3`.
- `calcular_diagonal(d, y, vy, va, hay_mirada, quieto, valores)`: copia en Python de la del partido,
  **solo para el robot de mentira**. Tiene que dar los ejemplos de 7.4.
- `angulo_salida(av, lat, rendimiento)`.
- `lado_pelota(y)` → "4.2 cm a la DERECHA". `lado(grados)` del delantero (+ = IZQUIERDA) para el giroscopio.
- `estado_linea(l, u) -> "-" | "IZQ" | "DER" | "AREA"` (el lateral izquierdo es el de atrás IZQ en
  blanco y el otro no; los dos = la línea del área).
- `calcular_umbral(fases)`, `ms_hasta_el_medio(cruce, vel=100, rampa=10)`, `numeros_creibles(...)`,
  `recta(xs, ys) -> (a, b)`, `sugerido_ancho(corridas)`, `sugerido_fondo(corridas)`,
  `ms_para_cm(a, b, cm)`.
- `consejo_pelota(respuesta, valores)` (la pregunta de la 9, 8.4).
- `leer_panel(texto) -> {nombre: (declaracion, valor_texto, comentario, numero_de_linea)}`: lee las
  líneas del PANEL DE CONTROL del partido con una regex (`^(declaración) (nombre)\s*= (valor);\s*//
  (comentario)$`), solo entre `PANEL DE CONTROL` y `fin del panel de control`.
- `linea_del_panel(decl, nombre, valor, comentario, etiqueta=None)`: `(decl + " " + nombre).ljust(49)
  + "= " + (valor + ";").ljust(7) + "// " + comentario`. Con `etiqueta` (por ejemplo `"MEDIDO 11/11
  Roboliga"`) reemplaza el primer `[...]` del comentario por `[etiqueta]`, o lo agrega adelante si no
  hay. **Con `etiqueta=None` tiene que reproducir byte por byte las líneas 79, 80, 81, 82, 101, 189 y
  190 de la 0.1103** (en COMENTARIOS_PANEL va una copia de esos 7 comentarios, para cuando no se
  encuentra el archivo).
- `copias_viejas(defectos, panel) -> [(nombre, el_del_tablero, el_del_partido)]`: los números en que
  las COPIAS del robot no coinciden con el archivo del partido.
- `hoja_a_texto(hoja)` / `texto_a_hoja(texto)` (ida y vuelta exacta) y `comparar_hojas(hojas)`.
- `puede_mover(...)` con los motivos de 8.7.
- `explicar_informe(inf, intentos, valores) -> [(frase, ''|'malo'|'bueno')]`: el pico ("6.2° a la
  IZQUIERDA a los 520 ms, en la ida"), el final (bueno ≤ 3), los cruces (malo ≥ 4), el promedio (malo
  si > 2), la corrección y el tope (malo si > 20 %), las mudas (malo si racha ≥ 3), el hueco (malo si
  > 40). En 6, 7 y 9: la línea ("vio la línea a los 905 ms de volver", o "NO vio la línea", malo) y el
  acomodo. En 8 y 9: idas, arranques, con pelota, zona muerta y velocidad ("va y viene", malo). En 9:
  la tabla de intentos.
- `nota_derecho(pico, cruces, fin)` y `contar_cruces` (los de la demo, con los criterios de 4.4).

### 8.4 Las pestañas

Cinco: `  PELOTA  `, `  SENSORES  `, `  MOVIMIENTOS  ` (la que se elige al abrir), `  CANCHA  `,
`  CONSOLA  `. Solo se dibuja la visible. **Arriba**: a la izquierda `lbl_conexion` y `lbl_programa`;
a la derecha **■ PARAR (Esc)**. **Abajo**: `lbl_aviso`. Título: `Tablero del ARQUERO` (+
`   (ROBOT DE MENTIRA)`).

`lbl_conexion`: `●  CONECTADO · COMx · ARQUERO` (verde) · `ESE ES EL DELANTERO: no le hablo` (rojo) ·
`ESA TEENSY NO ES EL ARQUERO (serie N)` (rojo) · `puerto OCUPADO` (naranja) · `○ Buscando el
robot...` (gris) · `ROBOT DE MENTIRA` (violeta) · `SIN CABLE: el robot hace la prueba solo. Enchufalo
cuando termine (sin apagar la batería)` (violeta).

`lbl_programa`: `Programa del tablero cargado (2026-11-03, copias del partido 0.1103)` (verde), o los
carteles de 2.6. Con `copias_viejas` no vacío: `⚠ COPIAS VIEJAS: el partido cambió (umbralCm: tablero
30.0, partido 32.0...). Avisar: hay que actualizar tablero.ino.` (naranja).

**PELOTA** (vista desde arriba). Canvas verde `#2e7d32` a la izquierda y un panel de 410 px a la
derecha. El robot abajo al centro, en `(w/2, h-90)`, con el triángulo rojo de frente y debajo una
franja gris **"ARCO PROPIO (atrás)"**. `escala = max(0.5, min((h-130)/85, (w/2-30)/50))` px por cm
(la cámara ve hasta ~70 cm y ±35 cm). Encima va:
- rótulos `◄ IZQUIERDA del robot` / `DERECHA del robot ►`;
- **la zona de MIRAR**: rectángulo amarillo claro `X ≤ distanciaParaMirar`, `|Y| ≤ umbralDesvio`
  (solo con `medirAntesDeDespejar`), con el rótulo `acá se frena a MIRAR (40 cm)`;
- **la zona de SALIR**: rectángulo naranja `X ≤ umbralCm`, `|Y| ≤ umbralDesvio`, `acá SALE (30 cm)`.
  (Son rectángulos porque el partido compara X e Y por separado: no es un círculo.);
- **el cono de ±MAX_ANGULO_DIAGONAL** (líneas punteadas desde el robot): `tope de la diagonal (30°):
  con la pelota QUIETA, afuera de esto NO SALE` (con la pelota quieta, el ángulo de salida es
  justo el de la pelota);
- la **zona muerta** ±`ZONA_MUERTA_PELOTA` (dos rayitas finas delante del robot);
- **hasta dónde llega la ida**: una marca a `msAdelante/10 - 3.3` cm (`llega la ida ≈ 35 cm (una
  cuenta del 04/08, a 200)`); en rojo si queda más cerca que `umbralCm`;
- **la pelota** en `(d, y)`: círculo naranja `#ff8f00` (gris si es vieja), con raya punteada desde el
  robot y `N cm`; y **la flecha de su velocidad** hasta `(d - va*0.5, y + vy*0.5)` (dónde estaría en
  0,5 s), solo si `|vy|` o `va` pasan de 4 cm/s;
- **la diagonal "si saliera ahora"**: con `que_haria` en SALDRIA, una flecha verde desde el robot en
  la dirección `angulo_salida`; en NO_SALDRIA, el motivo en rojo;
- **los arcos**, solo si la cámara los vio hace menos de 300 ms: un rectángulo amarillo/azul en el
  BORDE del dibujo, en la dirección `atan2(Y, X)` de sus números crudos, con `arco X (dirección; la
  distancia no está medida, y el partido no los usa)`. **No se inventa una distancia.**
- En el panel derecho: la frase de `que_haria` **en grande** (20 bold), la pelota ("Pelota a 30 cm,
  4.2 cm a la DERECHA"), las velocidades, `n` ("3 de 3 cuadros"), la cámara (q: verde ≥ 16, naranja
  1..15, rojo 0) y "cómo se lee".

**SENSORES** (3 columnas, pesos 3, 2, 2):
- `LÍNEA BLANCA`: un dibujo del robot desde arriba (círculo) con los 3 sensores **en su lugar**: atrás
  izquierda (A13), atrás derecha (A11) y adelante (A12). Cada uno con su barra 0..1023 (relleno blanco
  si `v >= u`, verde `#66bb6a` si no), la raya roja de `umbral N`, el valor y BLANCO/VERDE. El de
  adelante con `el partido NO lo usa`. Abajo, `estado_linea`: "pisa el lateral IZQUIERDO" / "...
  DERECHO" / "los dos: la línea del ÁREA" / "-". Si los 3 leen blanco: `¿Está sobre la MESA? Así no
  se mueve bien (ir-al-medio leyó 505/651/601 en la mesa).`
- `GIROSCOPIO`: la brújula del delantero (0 arriba, sentido horario), el rumbo grande, la fusión
  (`fusión OK (estado 5)` en verde) y la calibración `giro 3/3` (medir-ancho espera 3). Si `r = -1`:
  **`MUDO / SIN GIROSCOPIO`** en rojo, con `prendé la batería ANTES que el USB, y apretá "Buscar el
  giroscopio de nuevo"`. Botón **Buscar el giroscopio de nuevo** (manda `g`).
- `CÁMARA`: q grande, la última vez que vio la pelota ("hace 0.2 s"), Xp/Yp crudos, d/y en cm, los
  arcos crudos.

**MOVIMIENTOS** (como el delantero, 3 columnas):
- **Izquierda**: `Elegí un movimiento:` y los 10 botones (bajitos, para que entren en 1366x768);
  `Copiar al panel`; `Volver a los números del partido`; `Pedir el último resultado (r)`; y abajo,
  separado, **`VOLVER AL PARTIDO (cargar el programa de partido)`** (8.6).
- **Medio**: título, qué hace, lugar (en `#bf360c`, con `⚠`), el botón **`▶  PROBAR  NOMBRE`**, al
  lado el tilde **`SIN CABLE (10 s para desenchufar)`** (queda prendido hasta que lo saquen; cuando
  está prendido, el botón dice `▶  PROBAR SIN CABLE`) y `lbl_cuenta`; `Sus números (se cambian SIN
  recargar el robot):` y la tabla con `−`, valor, `+`, cajita, `Poner` e interruptores.
- **Derecha**: `RESULTADO · NOMBRE`, la nota enorme, la explicación, el **gráfico** (las fases con sus
  colores de 2.4; una línea vertical punteada donde cambia de fase; la franja verde ±4°), el detalle
  (`Lo que midió:` y `Qué tocar:` con los `#CONSEJO`). Si vino de `#REPITE`: arriba `Resultado
  guardado (SIN CABLE)`, y con `apagado=1`, `⚠ el robot se apagó: muestras raleadas y consejos con
  los números de ahora`. `#USADOS` va como `Se corrió con: umbralCm=32.0 (el partido: 30.0)`.
- **En la 9** se agrega: la **tabla de intentos** (k, t, resultado en palabras, d, y, vy, va, ángulo,
  costado) y la pregunta **"¿Qué pasó con la pelota?"** con 5 botones. `consejo_pelota` contesta
  (sin cambiar nada solo):
  - `LA DESPEJÓ` → "¡Bien! Anotalo. Repetí 3 veces antes de cambiar algo."
  - `LE FALTÓ COSTADO (la pelota quedó más afuera)` → "BAJÁ rendimientoCostado de a 0.05 (ahora 0.80
    → 0.75). Va al revés: más chico = MÁS costado." (en el mínimo 0.40: "ya está en el mínimo").
  - `SE PASÓ DE COSTADO (la pelota quedó más adentro)` → "SUBÍ rendimientoCostado de a 0.05 (ahora
    0.80 → 0.85)."
  - `LLEGÓ TARDE (ya había pasado)` → "Subí umbralCm de a 2 (ahora 30): sale antes. OJO: atado a
    msAdelante, la ida tiene que llegar más lejos que umbralCm."
  - `SE QUEDÓ CORTO (no llegó adelante)` → "Subí msAdelante de a 20 (ahora 383) y medilo con regla
    (CANCHA, paso 5)."
  La respuesta se anota en la CONSOLA y en la hoja de cancha abierta.
- Avisos que la ventana pone sola, en naranja, sin bloquear: `distanciaParaMirar` menor o igual que
  `umbralCm` ("mira recién cuando ya podría salir"); `MS_MAX_MIRANDO < MS_ASENTAR_MIRADA + MS_MIRAR`
  ("nunca llega a medir"); `msAdelante/10 - 3.3 <= umbralCm` ("la ida no llega a umbralCm");
  `pwmMaxLateral > 150` ("nunca se probó"); `umbralDesvio > 7` con `despejeEnDiagonal = 0` ("derecho,
  más de 7 no la alcanza"); un sensor de atrás sobre blanco en 1 a 5, 8, 9 ("no va a arrancar").

**CANCHA** (la preparación de una cancha nueva, sin internet; ver 8.5).

**CONSOLA**: la del delantero (tilde `Mostrar también los #S`, `Borrar`, `Ayuda del robot (?)`, Entry
+ `Mandar`). Sin el programa del tablero solo deja mandar `h` y `?`.

### 8.5 La pestaña CANCHA

**A la izquierda**, la lista de pasos como semáforo (gris sin hacer, verde medido y cierra, amarillo
con aviso, rojo falló). Al lado de cada uno: el **VALOR DE HOY** (del archivo del partido con
`leer_panel`; si no está, el de `#DEF`; si no, el de la tabla 3) con su etiqueta (`[MEDIDO 15/09]`,
`[a ojo]`, `[CUENTA]`) y el **MEDIDO AHORA**. **A la derecha**, el paso elegido: 3 o 4 renglones
grandes de "qué hacer", los botones de medir (+ el tilde SIN CABLE), el resultado con su dibujo, el
**valor sugerido**, **`USAR ESTE`** (manda `=NOMBRE valor` y lo anota en la hoja) y **`COPIAR AL
PANEL`** (la línea entera de `linea_del_panel` con la etiqueta `[MEDIDO dd/mm <cancha>]`, al
portapapeles, con la instrucción grande: "Ctrl+F con el nombre y cambiá SOLO el número y la etiqueta
de ESA línea. No pegues todo junto: queda repetido y no compila."). **Abajo**, la HOJA DE CANCHA
abierta (nombre y archivo) con `Nueva` / `Abrir` / `Comparar`, y **`TERMINAR`**.

| Paso | Prior. | Qué | Orden | Mueve | Número |
|---|---|---|---|---|---|
| 0 | A | ¿ANDA TODO? | (los `#S`, `g`) | no | - |
| 1 | A | La línea | `!L v` (repetible), `!L i`, `!L d`, o `!L c` | no | `umbralBlanco` (línea 79) |
| 2 | A | El ancho | `!A` / `!A c`, 2 corridas | sí | `MS_MEDIO_AL_MEDIO` (línea 81) |
| 2c | B | Comprobar ir al medio (por tiempo: sin la vuelta con la línea de la 0921i, 5.5) | `!I` / `!I c` | sí | - |
| 3 | B | El fondo | `!F` con TOCÓ (x2), o `!F c` comprobar | sí | `MS_MEDIO_FONDO_POR_TIEMPO` (línea 82) |
| 4 | B | La cámara con esta luz | `!C` (4a y 4b) | no | `CAMARA_POR_CM` (línea 80) |
| 5 | B | Cuánto sale el despeje | `=msPruebaIda 250` + `!10`, ídem 383 y 500 | sí | `msAdelante` (línea 101) |
| 6 | B | Signos | `!2` y los `#S` | 6a sí | `lateralInvertido`, `camaraYInvertida` (189, 190) |

- **Los pasos 2, 3 y 5 quedan bloqueados** hasta que el 1 esté hecho, o hasta confirmar "usar el
  umbral de hoy (425)".
- **Paso 0**: SÍ/NO grande por cada cosa. Programa `tablero-arquero` ✔; serie 13348550 ✔; giroscopio
  contesta (`r ≠ -1`) ✔; fusión `g = 5` ✔; calibración del giro N/3; cámara `q ≥ 16` (naranja 1..15,
  rojo 0); ve la pelota; sobre el verde los 2 de atrás en VERDE (si alguno dice BLANCO con el umbral
  de hoy: "el paso 1 es obligatorio"); los 3 en blanco = sobre la mesa; y un tilde a mano: "prendí la
  batería ANTES que el USB".
- **Paso 1**: botones `MEDIR VERDE` (se puede repetir en 2 o 3 lugares), `MEDIR BLANCO IZQ`, `MEDIR
  BLANCO DER`, o `TODO SIN CABLE (guiado por el LED)`. Cuenta y avisos de 5.1, con un dibujo de barras
  (verde y blanco por sensor y la raya del umbral). Al `USAR ESTE`: `=umbralBlanco N`, así los pasos 2
  y 3 buscan las líneas con el umbral NUEVO (era la trampa del panel).
- **Paso 2**: "Apoyalo en la MITAD de la cancha, mirando la cancha, a la altura en que juega (justo
  delante de la línea del área), con los sensores fuera de las líneas, y apretá MEDIR." 2 corridas.
  Muestra: los 3 tiempos, al medio desde la izq y desde la der, la asimetría %, el peor desvío de los
  4 tramos (verde < 5, amarillo 5..10, rojo > 10 = no sirve), las enderezadas y la repetibilidad
  entre corridas. Después de cada una pregunta "¿quedó en el medio? SÍ / corrido a la IZQ __ cm / DER
  __ cm". **Sugerido = el promedio de `almedioizq` de las corridas buenas** (así se eligió 2228 el
  06/10). Si difieren más de 3 %: "hacé una tercera". "Desde la derecha" se muestra solo como dato.
  Opcional: el mismo paso mirando al OTRO arco (la asimetría parece de la cancha, 06/10).
  Avisos: si `MS_MEDIO_TOPE_LATERAL` (6000) < 1,3 × el cruce medido, avisarlo.
- **Paso 3**: "Apoyalo junto al lateral IZQUIERDO, en el lugar MÁS ADELANTADO donde lo podrían apoyar,
  apretá MEDIR y apretá **TOCÓ** (botón grande, manda `t`) cuando toque la rampa o la pared." 2 veces.
  Sugerido de 5.3.
- **Paso 4**: 4a y 4b de 5.4. "Ojo: cambiarlo cambia TODOS los cm del programa, y kpLateral está
  pensado para 2.87."
- **Paso 5**: "Cinta de largada en el frente del chasis. Después de cada ida, medí con regla hasta el
  frente del chasis, escribí los cm y volvé a ponerlo en la cinta." Tres botones `IDA 250 ms`, `IDA
  383 ms`, `IDA 500 ms`: cada uno manda `=msPruebaIda X` y, **cuando llega el `#OK` de ese número**,
  manda `!10` (o `!10 c`). Recta `cm = a·ms + b` (con 150/250/350 → 13/19/33 da a = 0,1 y b = -3,33),
  la distancia que quieren (por defecto `umbralCm + 5`) y `ms = (cm - b) / a` (para 35 da 383). Avisa
  si la ida no pasa de `umbralCm`. Atajo si no hay tiempo: pelota quieta a 30 cm, ¿la toca y la saca?
  Si no, subir de a 30.
- **Paso 6**: 6a = el movimiento 2 y la pregunta "¿fue a la DERECHA?" (si fue a la izquierda:
  `lateralInvertido = 1`). 6b = "poné la pelota a la DERECHA del robot": muestra el Yp crudo
  (negativo = `camaraYInvertida = 1` está bien, como el 11/08 con Yp = -24).
- **Lista de "revisar a ojo"** (tildes que se guardan en la hoja): MS_MAX_RETROCESO 1200 (¿lo apoyan
  más lejos de la línea del área?), MS_MEDIO_TOPE_LATERAL 6000 (≥ 1,3 × cruce), MS_MEDIO_ADELANTE 943
  (¿el área es más honda?), msEmpujonFinal 320 y MS_MEDIO_ALEJARSE 400 (el piso), umbralCm y
  msAdelante (el tamaño de la cancha), LADO_MEDIO -1 (¿el lateral izquierdo tiene línea?), el ancho
  de la línea contra la separación de los sensores de atrás, que no haya otra línea entre el lateral
  izquierdo y el fondo, rampa o pared en los bordes, la luz y las cosas naranjas,
  MS_QUIETO_PARA_PATRULLAR 10000 (preguntarle al juez), y la batería (la de jugar, cargada).
- **COPIAS A MANO** (se listan al copiar, la ventana **no** toca esos archivos): umbralBlanco →
  ajustar-movimientos (`umbralBlanco`), monitor-robot, ir-al-medio, medir-ancho y medir-ancho-sin-giro
  (`UMBRAL_BLANCO`); CAMARA_POR_CM → ajustar-movimientos, monitor-robot, ver-prediccion;
  MS_MEDIO_AL_MEDIO → ir-al-medio (`MS_AL_MEDIO`); MS_MEDIO_FONDO_POR_TIEMPO → ir-al-medio
  (`MS_FONDO_POR_TIEMPO`).
- **TERMINAR**: muestra todas las líneas para el panel, las copias a mano y el último paso: "cerrá el
  tablero, CARGAR-PARTIDO.bat (o el botón VOLVER AL PARTIDO) y fijate que diga `=== ARQUERO  version ...`".

**HOJA DE CANCHA** (`canchas/AAAA-MM-DD_<nombre>.txt`, utf-8, se abre con el Bloc de notas):
- **Se escribe después de cada medición y ANTES de mostrarla** (regla del delantero del 08/09). Se
  escribe atómico: archivo temporal + `os.replace`.
- Formato: secciones `[DATOS] [LINEA] [ANCHO] [FONDO] [CAMARA] [DESPEJE] [SIGNOS] [REVISADO]
  [COPIADO AL PANEL] [NOTAS]` y renglones `clave = valor   # nota`. Los `#` al principio son
  comentarios. Ejemplo:
  ```
  # HOJA DE CANCHA del ARQUERO - la escribe el tablero. Se puede leer con el Bloc de notas.
  [DATOS]
  cancha = Roboliga 2026 - cancha 2
  fecha = 2026-11-11 10:42
  midieron = Diego y Laureano
  partido_cargado = 0.1103
  bateria = la azul, cargada
  [LINEA]
  verde_1 = izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1 n=3920   # 10:43
  blanco_izq = izq=310,763,702.5 der=80,95,88.0 ade=50,60,55.0 n=3900
  blanco_der = izq=82,96,89.1 der=300,753,690.3 ade=50,61,55.2 n=3910
  umbral = 425   # peorVerde 97, peorBlanco 753, margen 328/328
  [ANCHO]
  corrida_1 = medioizq=2180 izqder=4344 derizq=4547 almedioizq=2213 almedioder=2314 desvio=2.9,3.4,3.6,2.1 endereza=0 en_el_medio=SI
  sugerido = 2228
  [COPIADO AL PANEL]
  umbralBlanco = 425 -> 431   # 11/11 10:55
  ```
- [DATOS] lleva: cancha y lugar, fecha y hora, quiénes midieron, versión del partido cargada,
  batería, piso, luz, rampa sí/no, ancho de la línea en mm, ancho de la cancha en cm (opcional) y a
  qué arco miraba.
- La ventana trae (el constructor VENTANA lo escribe) `canchas/escuela-iita-bitacoras.txt` con los
  valores de hoy, con `origen = de las bitacoras, no medido con el tablero`: umbral 425 (15/09:
  verde 97, blanco 753; A13 95/763, A11 81/753, A12 56/759), ancho 06/10 (corridas 4344/4547 →
  2213/2314 y 4405/4615 → 2243/2348, promedio 2228, repetibilidad 1,5 %, asimetría 4 %), fondo 2000
  [a ojo], cámara 25/08 (10→28, 20→58, 30→88,9, 40→117, 50→142 → 2,87), despeje 04/08 (150/250/350 ms →
  13/19/33 cm → 383 para 35 cm), signos 11/08 (Yp = -24 a la derecha).
- `COMPARAR`: una tabla con un renglón por número y una columna por cancha, más la columna HOY (el
  panel). `Volver a esta cancha` arma las líneas del panel con los valores de esa hoja.
- La hoja guarda lo que mandó con `USAR ESTE` en [COPIADO AL PANEL] (de cuánto a cuánto).
- **Al reconectar, la ventana NO le manda sola los números al robot.** Si ve (por el `#V`) que el robot
  volvió a los del partido y en la hoja había cambiados, ofrece el botón "Volver a poner los N números
  de la hoja".

### 8.6 VOLVER AL PARTIDO (el botón)

- Habilitado solo conectado al ARQUERO, con el tablero, en ESPERA y sin pedido pendiente.
- `messagebox.askyesno`: "Esto CIERRA el tablero y carga el programa de PARTIDO. 🚨 Apenas termina la
  carga, el robot ARRANCA SOLO (a los 2 s retrocede hasta la línea). Batería APAGADA o el robot en el
  piso. Dejá enchufado SOLO el arquero. ¿Cargar?"
- Si dice que sí, en este orden: manda `x` (`enviar_ya`), cierra la conexión (para los hilos y suelta
  el puerto), espera 0,5 s, lanza `CARGAR-PARTIDO.bat` en una consola nueva
  (`subprocess.Popen(["cmd.exe", "/c", CARGAR_PARTIDO_BAT], cwd=<su carpeta>,
  creationflags=subprocess.CREATE_NEW_CONSOLE)`) y **cierra la ventana** (si quedara abierta se
  volvería a conectar y le sacaría el puerto al cargador). Ese `.bat` usa `cargar.py`, que vuelve a
  controlar que haya un solo Teensy y que sea el arquero.
- En `--demo`: no lanza nada ("en la demo no se carga nada").

### 8.7 Habilitar y deshabilitar

`puede_mover(...)` devuelve `(bool, motivo)`, con estos motivos en orden:
1. "No hay robot conectado."
2. "Este robot NO es el arquero (serie N): el tablero no le manda nada."
3. "El robot no tiene cargado el programa del tablero: usá TABLERO.bat." (o el cartel de 2.6)
4. "El robot dice que NO es el arquero (serie N): no se mueve." (el `serie=` del `#HOLA`)
5. "SIN CABLE: el robot está haciendo la prueba solo. Enchufalo cuando termine."
6. "El robot está buscando el giroscopio..."
7. "Esperá a que termine." (CUENTA / MOVIENDO)
8. "Esperando que el robot conteste..."

- PROBAR, los `−`/`+`/Poner, Copiar, Volver a los números, Pedir el último, los botones de CANCHA y
  VOLVER AL PARTIDO: solo con `puede_mover` verdadero. **PARAR nunca se deshabilita.**
- `lbl_cuenta`: en CUENTA con cable, "Arranca en X s... (PARAR o Esc para cancelar)" (naranja); en
  CUENTA sin cable, **"DESENCHUFÁ EL USB AHORA: arranca en X s"** (violeta, grande); en MOVIENDO,
  "¡SE ESTÁ MOVIENDO! (PARAR o Esc para frenarlo)" (rojo); con pedido pendiente, "Pedido...
  esperando que el robot cuente."; con un freno sin informe, "Se frenó: motivo"; si está todo bien,
  "Listo. Al apretar PROBAR cuenta 3 s y arranca." (o "10 s para desenchufar").
- `_probar`: manda `!n` o `!n c` y anota `t_pedido`. 3 s anti doble clic.
- `_parar`: si es otra Teensy no manda nada ("apagá la batería"). Si no, manda `x` (un tablero),
  `0` (ajustar-movimientos) o un **espacio** si no sabe qué programa es (revisión del 03/11, ver
  2.6 y la sección 15). Sin conexión y sin cable: "Sin cable, PARAR no llega: levantalo o apagá la
  batería."
- Durante CUENTA o MOVIENDO la ventana no manda nada más que PARAR.

### 8.8 El latido (cada 250 ms)

- 3 s conectado al arquero sin `#HOLA` ni texto reconocido → cartel de "no sé qué programa" (salvo
  con `esperando_sin_cable`: "el robot todavía se está moviendo solo: esperá").
- Sin `#HOLA` y con `#S`: `h` cada 2 s. `t_pedido` de más de 3 s sin `#CUENTA`/`#INICIO`/`#FRENO`/
  `#ERROR` → "El robot no arrancó la cuenta. ¿Tiene el programa del tablero?".
- Panel de más de 4 s → "No llegó el panel completo".
- En CUENTA o MOVIENDO, sin renglones durante `tope + 5000` ms (del `#INICIO`; en CUENTA, 15 s) →
  vuelve a ESPERA con el aviso rojo "Si se sigue moviendo: PARAR o apagar la batería". **Con
  `esperando_sin_cable` no**: ahí el silencio y el desenchufe son lo esperado.
- Sensores de hace más de 1,5 s → "Los datos son de hace N s".

### 8.9 `--quien` (lo usa TABLERO.bat)

`tablero.py --quien` mira `list_ports.comports()` **sin abrir ningún puerto** (no necesita tkinter) e
imprime una línea por cada Teensy y al final `QUIEN=...`:
```
Teensy serie 13348550 en COM11  ->  ARQUERO
QUIEN=ARQUERO
```
| Caso | Última línea | Código de salida |
|---|---|---|
| exactamente una Teensy y es 13348550 | `QUIEN=ARQUERO` | 0 |
| alguna es 15708680 | `QUIEN=DELANTERO` | 3 |
| una sola, con otra serie (o sin serie) | `QUIEN=OTRO` | 4 |
| ninguna | `QUIEN=NINGUNO` | 5 |
| más de una (y ninguna es el delantero) | `QUIEN=VARIOS` | 6 |
| alguna en MODO CARGA (y ninguna es el delantero) | `QUIEN=CARGA` | 8 |

**Modo carga** (revisión del 03/11): un Teensy en modo carga (HalfKay, `VID_16C0&PID_0478`) no tiene
puerto COM, y `teensy-cli` le carga PRIMERO a ese, aunque sea el delantero. `--quien` lo busca con
`pnputil /enum-devices /connected` (solo lista, no abre nada). Si `pnputil` no anda, sigue como antes
y agrega "(No pude mirar si hay un Teensy en MODO CARGA: fijate que el OTRO robot NO este enchufado a
esta PC.)". VOLVER AL PARTIDO hace el mismo control antes de lanzar `CARGAR-PARTIDO.bat`.

### 8.10 El robot de mentira (`--demo`)

Imita el contrato completo con datos inventados pero creíbles, **con los valores del partido**:
- `#HOLA tablero-arquero ... serie=13348550 partido=0.1103`, `#GIRO ok=1 estado=5`, `#V`, `#DEF`,
  `#GUARDADO`, y `#S` cada 100 ms (pelota que va y viene y cada 12 s se esconde 2,5 s; los sensores en
  verde y cada 7 s el de atrás izquierdo pisa blanco; arcos a veces).
- Los 10 movimientos con `#M`, `#MIRE` (en la 9, con `calcular_diagonal`), `#R`, `#USADOS`,
  `#CONSEJO` y `#FIN`. **Las mediciones de cancha dan los números de las bitácoras**: L (verde izq
  97 / der 81; blanco izq 763 / der 753 → 425), A (corrida 1: 2180/4344/4547 → 2213/2314; corrida 2:
  2210/4405/4615 → 2243/2348), F (1730 TOCO), C (a 30 cm: xp prom 86,9), I (los pasos del ejemplo).
- **Sin cable**: con `!n c` cuenta, a los 2 s "se desenchufa" (manda `("desconectado", ...)` a la
  cola), hace el movimiento sin mandar nada y "se vuelve a enchufar" (`("conectado", "DEMO",
  SERIE_ARQUERO)` + `#HOLA` + `#GUARDADO ... nuevo=1`), y contesta `r` con `#REPITE`.
- Acepta un factor de tiempo (`escala_tiempo`) para que la autoprueba corra rápido.

---

## 9. LOS `.bat` (para el constructor VENTANA)

### 9.1 `TABLERO.bat` (en `arquero-teensy-zircon/`) — la lógica del delantero, al revés

ASCII puro en `echo` y `rem`, `chcp 65001 >nul` (como CARGAR-PARTIDO.bat, porque `cargar.py` imprime
emojis). Variables: `PY`, `PYW` (los de PlatformIO) y `TABLERO=%~dp0pruebas\tablero`.
1. Si no existe `PY`: `XXX  No encuentro el Python de PlatformIO en: ...`, pause, `exit /b 1`.
2. Cartel: `ARQUERO - TABLERO (la ventana con botones)` / `NO es el programa de partido. Al terminar:
   CARGAR-PARTIDO.bat (o el boton VOLVER AL PARTIDO de la ventana).`
3. `:verificar` → `"%PY%" "%TABLERO%\tablero.py" --quien` y guarda `%ERRORLEVEL%`. Si no es 0 → `:noconfirmado`.
4. `:preguntar` con `set /p "RESP=   Cargar el programa del tablero al robot? (S/N) [S]: "`: Enter
   solo = S, borrar las comillas, S/SI → cargar, N/NO → abrir, otra cosa → preguntar de nuevo. Antes
   avisa: `Si el robot YA tiene el tablero cargado, contesta N.` y `Si hay una ventana del tablero
   abierta, cerrala ANTES de cargar.`
5. `:cargar`: **mira OTRA VEZ** con `--quien` (pudieron cambiar el robot mientras esperaba); si no es 0
   → `:noconfirmado`. Después `"%PY%" "%~dp0pruebas\herramientas\cargar.py" "%TABLERO%"` (controla un
   solo Teensy, la serie, que no sea el delantero, que el puerto esté libre, mata `teensy.exe` y carga
   con `teensy-cli`). Si `errorlevel 1`: `XXX  LA CARGA FALLO. El robot sigue con el programa que tenia.
   No abro la ventana.`, pause, `exit /b 1`. **No abrir el Teensy Loader.**
6. `:abrir`: el cartel (`Arriba tiene que decir ARQUERO y "Programa del tablero cargado". Si no, la
   carga no entro.` / `En el piso, con lugar alrededor, y el cable flojo (o SIN CABLE).` / `AL
   TERMINAR: cerrar la ventana y CARGAR-PARTIDO.bat (en funciona\seguir-y-despejar). El robot arranca
   solo apenas termina la carga.`), `start "" "%PYW%" "%TABLERO%\tablero.py"` (sin pythonw: `start
   "Tablero del arquero" "%PY%" ...`), `(Esta ventana negra se puede cerrar: la del tablero sigue.)`, pause.
7. `:noconfirmado` según el código: 3 → `XXX  Ese es el DELANTERO. No le cargo nada: el tablero del
   arquero tiene los pines cruzados para el delantero.`; 4 → `XXX  Hay un robot que NO es el arquero,
   o no pude leer su numero.`; 5 → `XXX  No veo ningun robot enchufado por USB...`; 6 → `XXX  Hay
   mas de un Teensy: el cargador no elige a cual le carga.`; 8 → `XXX  Hay un Teensy en MODO CARGA
   ...` (revisión del 03/11); otro → `XXX  No pude confirmar que sea el arquero.` Después `Deja enchufado SOLO el ARQUERO, espera unos segundos y apreta una tecla para
   mirar de nuevo.`, `Para salir: cerra esta ventana negra.`, `Sin robot: TABLERO-DEMO.bat.`, pause y
   vuelve a `:verificar`. **Es un lazo: nunca carga sin confirmar.**

### 9.2 `TABLERO-DEMO.bat`

Igual que el del delantero, con los textos del arquero: `start "" "%PYW%" "%TABLERO%\tablero.py"
--demo` (o con `python` y el título `Tablero de mentira (arquero)`, o el error de que no hay Python).
No carga nada ni abre el USB.

---

## 10. LA SEGURIDAD, en capas (resumen)

| Capa | Qué protege |
|---|---|
| `TABLERO.bat` | carga solo si `--quien` dice ARQUERO, dos veces (antes de preguntar y justo antes de cargar); el resto lo vuelve a controlar `cargar.py` |
| `tablero.ino` | nunca arranca solo; se mueve solo con `!`, después de la cuenta (3 s / 10 s), con tope por prueba; cualquier byte lo frena en la cuenta o moviéndose; sin giroscopio no arranca; 1500 ms después de un freno no acepta otro; si su serie no es la del arquero no mueve nada; no escribe fuera de 2048..3071 y nunca moviéndose |
| `tablero.py` | nunca abre el puerto de otra Teensy; al que no es el arquero no le manda nada; al conectar no manda nada hasta ver un `#S`; mueve solo con el `#HOLA` correcto, en ESPERA y sin pedido pendiente; en CUENTA/MOVIENDO solo manda PARAR; tira lo pendiente al conectar y al desconectar; al cerrar manda `x`; VOLVER AL PARTIDO pide confirmación y cierra antes de cargar; una falla no la tira abajo |
| la persona | **SIN CABLE, PARAR no llega**: se frena levantándolo o con la llave de la batería. Batería ANTES que el USB. En el piso, nunca en la mesa |

---

## 11. LA AUTOPRUEBA (`tablero.py --autoprueba`)

El mismo esqueleto que el delantero: `sys.stdout.reconfigure(errors="replace")`, el cartel
`AUTOPRUEBA del tablero del ARQUERO: no abre la ventana ni el USB.`, `verificar(cond, que)`, una
línea `  %-42s %5d verificaciones bien` por parte, `  FALLA en <parte>:` + traceback y código 1, o
`OK (N verificaciones)` y código 0. Se corre con
`"%USERPROFILE%\.platformio\penv\Scripts\python.exe" -B pruebas\tablero\tablero.py --autoprueba`.
Tiene que andar también sin tkinter y sin pyserial, y **no puede escribir nada fuera de una carpeta
temporal** (`tempfile.mkdtemp()`: la hoja de cancha y el `tablero-error.txt` de las pruebas van ahí).

### 11.1 Renglones de ejemplo que la ventana TIENE que entender (y el robot imprimir así)

```
#HOLA tablero-arquero 2026-11-03 serie=13348550 partido=0.1103
#GIRO buscando
#GIRO ok=1 estado=5
#GUARDADO que=mov hay=1 n=9 nombre=ATAJAR nota=CASI sincable=1 nuevo=1 mismo=1 hace=48210
#GUARDADO que=cancha hay=0
#S t=12345 l=88,92,61 u=425 r=93.4 g=5 cal=3,3,0,0 p=86,-12,40 d=30.0 y=4.2 vy=0.0 va=0.0 ve=1 n=3 dg=1,0,193,33,396 am=150,-20,120 az=0,0,-1 q=24 e=ESPERA
#S t=500 l=90,85,60 u=425 r=-1 g=-1 cal=-1,-1,-1,-1 p=0,0,-1 d=-1.0 y=0.0 vy=0.0 va=0.0 ve=0 n=0 dg=1,0,200,0,383 am=0,0,-1 az=0,0,-1 q=0 e=ESPERA
#OK umbralCm=32.0
#ERROR umbralCm fuera de rango: va de 20.0 a 45.0
#CUENTA 3.0 cable
#CUENTA 10.0 sincable
#INICIO 9 ATAJAR tope=30000 sincable=0
#M 0 0.0 s
#M 3880 0.2 i
#M 4500 -1085.4 a
#MIRE k=1 t=2140 res=NO_VIENE miro=1 cuadros=4 d=38.2 y=-3.1 vy=0.0 va=2.5 av=0 lat=0 ms=0 ang=0.0 recto=0
#MIRE k=2 t=3870 res=SALE_MIRANDO miro=1 cuadros=4 d=29.6 y=-7.4 vy=-10.0 va=30.0 av=162 lat=-94 ms=472 ang=-25.0 recto=0
#R n=9 mov=ATAJAR nota=PROLIJO como=NORMAL pico=3.4 picoms=4120 fin=0.9 medio=0.4 cruces=0 fuera3=4 corr=-2 tope=0 mudas=0 racha=0 hueco=11 huecoms=3990 duro=7930 lineams=1010 idams=4342 atrasms=4742 acomodo=0 laterales=0 idas=2 arranques=3 conpelota=88 zonamuerta=40 velmedia=9.8 intentos=2 sale=1
#USADOS rendimientoCostado=0.75
#CONSEJO Si NO LLEGO de costado (la pelota le quedo mas afuera): BAJAR rendimientoCostado de a 0.05 (ahora 0.75). Si se PASO de costado: SUBIRLO. Va al reves: mas chico = MAS costado.
RESULTADO: 9 ATAJAR -> PROLIJO  pico +3.4 grados a los 4120 ms, final +0.9, cruces 0, duro 7930 ms, vuelta mas larga 11 ms
#FIN 9
#REPITE 6 DESPEJE sincable=1 apagado=1
#R n=6 mov=DESPEJE nota=CASI como=NORMAL pico=6.2 picoms=520 fin=1.1 medio=0.8 cruces=1 fuera3=12 corr=-4 tope=0 mudas=0 racha=0 hueco=12 huecoms=300 duro=3410 lineams=905 idams=383 atrasms=783 acomodo=1 laterales=0 idas=0 arranques=0 conpelota=0 zonamuerta=0 velmedia=0.0 intentos=0 sale=0
#USADOS
#FIN 6
#FRENO tecla
#FRENO sobre la linea
#PASO A calibrando giro=2
#PASO A tramo=0 ms=2180 desvio=2.9 endereza=0
#PASO I paso=1 ms=1830 como=L
#LIN fase=v n=3920 izq=71,97,84.0 der=60,81,70.2 ade=44,56,50.1
#ANCHO ok=1 error=0 medioizq=2180 izqder=4344 derizq=4547 almedioizq=2213 almedioder=2314 desvio=2.9,3.4,3.6,2.1 endereza=0 golpe=2.4 vel=100 rampa=10 umbral=425 calgiro=3
#ANCHO ok=0 error=4 medioizq=2180 izqder=0 derizq=0 almedioizq=0 almedioder=0 desvio=2.9,0.0,0.0,0.0 endereza=0 golpe=0.0 vel=100 rampa=10 umbral=425 calgiro=3
#FONDO ms=1730 como=TOCO desvio=1.8 umbral=425 probado=0
#CAM n=76 cero=0 val=76 xp=84,90,86.9 yp=-1.2 am=0 az=0
#MEDIO ok=1 pasos=1830,400,2000,943,610,2228,520,320 como=L,T,T,T,L,T,L,T desvio=3.1 acomodo=0
#PANEL float rendimientoCostado = 0.75;
#PANEL const unsigned long MS_MIRAR = 140;
#FINPANEL
=== ARQUERO  version 0.1103 EN PRUEBA (0.1027d + registro de despejes) ===
=== REGISTRO DE DESPEJES: los ultimos 12 intentos, del mas viejo al mas nuevo. Este encendido es el 7 ===
=== AJUSTAR-MOVIMIENTOS 2026-10-27 — el robot SE MUEVE solo al apretar 1..9 ===
#HOLA tablero-delantero 2026-10-27
#S t=12345 l=310,322,705 u=424,434,440 r=93.4 g=5 p=55,24,40 am=150,-20,120 az=0,0,-1 q=18 e=ESPERA
```
(Los dos últimos son del tablero del DELANTERO: el `#HOLA` tiene que dar "tablero del delantero" y
ese `#S` tiene que dar ROTA, porque su `u` trae 3 números.)

### 11.2 Lo que tiene que comprobar cada parte

1. **el traductor**: cada renglón de 11.1 da su tipo y sus datos. El `#S` bueno cortado en CADA
   posición nunca da S. Variantes de `#S` con `\r` y `\r\n`, sin giroscopio, `nan`, `q` con
   decimales, campos de más, `dg` con 4 números (ROTA), `u=424,434,440` (ROTA), coma decimal, dos
   pegados. `#V`/`#DEF`/`#OK` con nombres camelCase (`umbralCm`, `MS_MIRAR`) y con un nombre que
   empieza con número (ROTA). `#INICIO` con cada código de CODIGOS y con uno que no existe (ROTA).
   `#CUENTA 3.0 cable` y `#CUENTA 3.0 rapido` (ROTA). `#R` con cada nota y con una que no existe.
   `#MIRE` con cada `res`. `#USADOS` vacío. Números de 400 cifras en cada campo. `#PANEL` con sus
   espacios. TEXTO y DESCONOCIDA. 3000 renglones al azar hechos de pedazos: siempre un tipo válido.
   `reconocer_programa` con los textos del partido, de ajustar y del tablero.
2. **la memoria**: la secuencia normal (CUENTA → INICIO → M → MIRE → R → USADOS → CONSEJO → FIN →
   ESPERA); `R` sin `FIN` no vuelve a ESPERA, pero un `#S` sí; tecla en la cuenta; tecla moviéndose;
   `FRENO` sin `INICIO` borra la nota vieja; `#M` sin `INICIO` (reconexión) empieza otro resultado;
   `REPITE` no cambia el modo; `PASO` no cambia el modo; las tandas de cancha quedan por código y
   fase; `GUARDADO nuevo=1` deja el pedido de `r`; `reiniciar_conexion` conserva el resultado y
   `esperando_sin_cable`; las tandas de PANEL.
3. **las cuentas**:
   - `calcular_umbral` (97/753 → 425, márgenes 328/328; superpuestos; frágil; muestra sucia; el
     verde en 3 lugares toma el máximo);
   - `ms_hasta_el_medio` (4344 → 2213, 4405 → 2243, 4547 → 2314, 4615 → 2348);
   - `numeros_creibles` (1,4 y 3; 0,7 y 1,43, en los bordes);
   - `sugerido_ancho` (2213 y 2243 → 2228; aviso con más de 3 %);
   - `sugerido_fondo` (1730 y 1690 → 2250);
   - `recta` (cámara del 25/08 → 2,87 / 0,68; idas del 04/08 → 0,1 / -3,33) y `ms_para_cm` (35 → 383);
   - `calcular_diagonal` con la tabla de 7.4;
   - `que_haria` en cada uno de sus 7 casos, y con `medirAntesDeDespejar = 0`;
   - `cm_a_pantalla` (una pelota con y > 0 queda a la DERECHA del centro);
   - `estado_linea` en sus 4 casos;
   - `consejo_pelota` (LE_FALTÓ → **bajar**, SE_PASÓ → **subir**, en el mínimo y en el máximo);
   - `linea_del_panel` (reproduce las líneas 79, 80, 81, 82, 101, 189 y 190 de 11.3; con etiqueta,
     reemplaza el primer `[...]`);
   - `leer_panel` sobre 11.3 (y, si existe, sobre el archivo real del partido: **solo leerlo**, y que
     entre lo que encuentra estén los 42 números de la tabla 3 que tienen declaración);
   - `copias_viejas`;
   - la hoja de cancha de ida y vuelta;
   - `nuevo_valor`, `normalizar_entrada` y `puede_mover` (cada motivo);
   - que NUMEROS, VALORES_DE_MENTIRA y RANGOS_DE_MENTIRA tengan las 46 claves de la tabla 3, en el
     mismo orden, y que estén todos los números de MOVIMIENTOS.
4. **el robot de mentira**: cada movimiento entero y cortado (NO_VALE si lo cortan); la 9 con un
   intento que no sale y otro que sale; cada medición de cancha con los números de las bitácoras; sin
   cable con `escala_tiempo` (desconectado → conectado → `#GUARDADO nuevo=1` → `r` → `#REPITE` → `#FIN`);
   las órdenes `h ? = v 0 g r rc x` y los errores.
5. **el cable**: con clases falsas de `serial` y `list_ports`: con arquero y delantero enchufados,
   abre solo el arquero; con solo el delantero o una desconocida, avisa UNA vez y nunca la abre ni le
   escribe; `--puerto` del delantero se niega; renglones partidos de a 7 bytes; desconectado y
   ocupado; tira lo pendiente; **`--quien` con cada caso de 8.9 (salida y código)** sin abrir nada.
6. **la ventana** (con `unittest.mock.MagicMock` para tk/ttk/messagebox, `_CableDePrueba` y
   `estricto=True`): todo el flujo del delantero, adaptado (ocupado, otro, conectado sin mandar nada,
   el partido detectado por su texto sin mandar `h`, el tablero del delantero bloqueado, el primer
   `#S` que manda `h` y `?`, PROBAR, el doble clic, nada en la cuenta salvo PARAR, PARAR, + y −, la
   cajita con coma y con letras, los interruptores, los 10 movimientos con gráfico, Copiar al panel,
   Volver a los números, todos los dibujos con y sin datos, el latido, desconectar y reconectar en
   medio de un movimiento, cerrar mandando `x`, y una falla que no rompe). Y además:
   - **SIN CABLE**: `!6 c` → `#CUENTA 10.0 sincable` → desconectado: sin aviso rojo, PROBAR apagado,
     el latido de silencio no salta → conectado → `#HOLA` → `#GUARDADO ... nuevo=1` → la ventana manda
     `r` UNA vez → `#REPITE` → la tanda → `#FIN` → resultado con el cartel "guardado (SIN CABLE)";
   - **VOLVER AL PARTIDO** con `subprocess.Popen` falso: con askyesno en NO no pasa nada; en SÍ, antes
     de lanzar se mandó `x` y se cerró el cable, se lanzó la ruta de CARGAR-PARTIDO.bat y la ventana
     se cerró; en demo nunca se lanza;
   - la 9 con la tabla de intentos y los 5 botones de la pregunta;
   - CANCHA: los pasos bloqueados sin el 1, el paso 1 completo con USAR ESTE (manda
     `=umbralBlanco 425`), el paso 2 con dos `#ANCHO`, el 5 manda `!10` recién después del `#OK`, la
     hoja escrita en la carpeta temporal **antes** de mostrarse, COPIAR AL PANEL con la línea exacta;
   - **todo lo que se manda es ASCII y de 71 caracteres o menos**, y nunca se manda nada a una serie
     que no sea la del arquero.

### 11.3 Las líneas reales del panel de la 0.1103 (para `linea_del_panel` y `leer_panel`)

```
int umbralBlanco                                 = 425;   // [MEDIDO 15/09] desde que lectura un sensor de linea dice blanco. En cada cancha: medirlo con calibrar-linea y copiarlo TAMBIEN en las pruebas (ver arriba)
const float CAMARA_POR_CM                        = 2.87;  // [MEDIDO 25/08] unidades de camara por cm real, pelota enfrente. Es de la CAMARA, no de la cancha: se mide de nuevo solo si se movio la camara (tabla-camara). Cambiarlo cambia TODOS los cm
const unsigned long MS_MEDIO_AL_MEDIO            = 2228;  // [MEDIDO 06/10, antes de mover los motores] del lateral izquierdo al medio a velocidad 100 (medir-ancho). Lo usan ir al medio, la patrulla y el limite lateral. Vale con los numeros de mezcla, rampa y rumbo de hoy: si se tocan, medirlo de nuevo
const unsigned long MS_MEDIO_FONDO_POR_TIEMPO    = 2000;  // [a ojo, no medido] ir al medio: cuanto retrocede buscando el fondo antes de darlo por encontrado (la rampita no lo deja pisar la linea)
int msAdelante                                   = 383;   // cuanto dura la ida (ms). [CUENTA, SIN PROBAR] para X cm: ms = (X + 3.3) x 10, a potencia 200 (04/08, antes de la rampa y del freno: puede salir menos). Si no toca la pelota: subir y medir con regla
bool lateralInvertido                            = false; // [MEDIDO 11/08] se prueba SIN pelota: ajustar-movimientos, movimiento 2 (costado DERECHA). Si va a la IZQUIERDA, cambiar esto
bool camaraYInvertida                            = true;  // [MEDIDO 11/08] cambiar SOLO si el movimiento 2 de ajustar-movimientos va bien a la derecha y aun asi, siguiendo la pelota, se aleja de ella
```
(Son las líneas 79, 80, 81, 82, 101, 189 y 190. La declaración y el nombre ocupan 49 columnas, después
`= `, el valor con `;` en 7 columnas y `// `.)

---

## 12. LA PRUEBA CON EL ROBOT (la hacen Diego y Laureano; los constructores no pueden)

Con el robot en el piso, batería prendida y DESPUÉS el USB, `TABLERO.bat`:
1. Arriba dice `ARQUERO` y `Programa del tablero cargado (... 0.1103)`. En SENSORES: el rumbo se
   mueve al girarlo a mano, `fusión OK`, las barras pasan a BLANCO con una hoja, `q` entre 16 y 26.
2. PELOTA: la pelota a la derecha aparece a la DERECHA. A 30 cm de frente dice "SALDRÍA YA".
3. PARAR en la cuenta: no se mueve. PARAR moviéndose: frena. Cerrar la ventana en espera: no pasa nada.
4. El 2 va a la DERECHA. El 6 vuelve a la línea y se endereza. El 9 con una pelota tirada despacio.
5. SIN CABLE con el 1: desenchufar en la cuenta, esperar los destellos, enchufar: aparece solo.
6. CANCHA pasos 0 y 1 en la cancha de la escuela: tiene que dar cerca de 425.
7. VOLVER AL PARTIDO: carga y dice `=== ARQUERO  version 0.1103`.
Anotar todo en la bitácora.

---

## 13. Lo que entendí y lo que hay que preguntarles a Diego y Laureano

(Regla del profe Franco: decirles qué se entendió y que completen lo que falte.)

**Lo que entendí:** quieren el tablero del delantero, pero con sus movimientos, los del partido de
hoy (la 0.1103, con MIRAR); que ande sin cable porque en la cancha no hay USB; y una pestaña para
medir una cancha nueva (primero la línea, después el medio) que les dé la línea para copiar al panel
y guarde las mediciones de cada cancha.

**Preguntas:**
1. ¿La cuenta de 3 s con cable les parece bien, o prefieren 1,5 s como el delantero?
2. ¿Medir el ancho mirando a cada arco en la Roboliga (la asimetría del 06/10)?
3. ¿`MS_MEDIO_AL_MEDIO` = el promedio de 2 corridas, como el 06/10?
4. El fondo: ¿la regla del panel (+500 ms) o descontar los 1200 ms que ya retrocedió?
5. ¿El paso 5 (cuánto sale el despeje) lo quieren en CANCHA (así está) o en MOVIMIENTOS? ¿A cuántos cm
   quieren que llegue la ida?
6. En la 9, ¿que termine en el primer despeje (así está) o que siga despejando hasta msSeguir?
7. ¿Quieren un `PREPARAR-CANCHA.bat` que abra el tablero directo en CANCHA? (Hoy queda fuera de los
   archivos que se pueden tocar.)

---

## 14. Archivos

| Archivo | Quién | Prioridad |
|---|---|---|
| `pruebas/tablero/DISENO.md` | este documento (nadie lo cambia) | - |
| `pruebas/tablero/tablero.ino` | ROBOT | A (salvo F, C, I y la línea guiada: B) |
| `pruebas/tablero/platformio.ini` | ROBOT | A |
| `pruebas/tablero/tablero.py` | VENTANA | A (CANCHA pasos 2c..6, COMPARAR, `copias_viejas`: B) |
| `pruebas/tablero/canchas/escuela-iita-bitacoras.txt` | VENTANA | B |
| `TABLERO.bat` | VENTANA | A |
| `TABLERO-DEMO.bat` | VENTANA | A |

**Para copiar** (solo leer):
- `funciona/seguir-y-despejar/seguir-y-despejar.ino` (0.1103: panel líneas 19-200; motores ~1663;
  rumbo ~1774; línea ~1831; `moverDeCostado` ~1856; `leerCamara` ~1927; `calcularDiagonal` ~2072;
  `intentarDespejar` ~2292; `loop` ~2474; `MIRANDO` ~3008).
- `pruebas/ajustar-movimientos/ajustar-movimientos.ino` (movimientos 1-5, `anotar`, `calcularNota`,
  `imprimirSugerencia`, `ledNota`, el modo sin cable).
- `pruebas/medir-ancho/medir-ancho.ino` (tramos, `topeDelTramo`, `numerosCreibles`, `msHastaElMedio`,
  enderezar, `preguntarSiFusiona`).
- `pruebas/calibrar-linea/calibrar-linea.ino` y `pruebas/tabla-camara/tabla-camara.ino` (la lógica de
  la línea y de la cámara).
- `pruebas/herramientas/cargar.py` (lo usa TABLERO.bat; no se modifica).
- El tablero del delantero: `../delantero-teensy-zircon/pruebas/tablero/tablero.py`, `tablero.ino`,
  `platformio.ini`, `../delantero-teensy-zircon/TABLERO.bat` y `TABLERO-DEMO.bat`.

**Idea para después** (prioridad C, no hacer ahora): una orden que **lea** el REGISTRO DE DESPEJES de
la 0.1103 (EEPROM 3072..3559, solo lectura) para verlo en la ventana sin cargar el partido.

---

## 15. CAMBIOS DE LA REVISIÓN (2026-11-03, a la noche)

Después de construirlo, una revisión encontró cosas para arreglar. Lo que se cambió (los dos lados):

| Qué | Dónde | Por qué |
|---|---|---|
| Una prueba SIN CABLE queda `nuevo=1` aunque la tanda haya salido | `terminarPrueba()` en el `.ino` | el Teensy no se entera del desenchufe: la mandaba al vacío y la marcaba vista (2.4, 6.1) |
| La ventana no pide de nuevo lo que ya vio en vivo; y si el `#GUARDADO` dice `nuevo=0` pero terminó después de la cuenta sin cable, lo pide igual | `Estado` (`vistos`, `t_sin_cable`) | con un `.ino` viejo; antes decía en falso "¿lo apagaron?" |
| `MS_MEDIO_AL_MEDIO` va de 500 a **3200** (antes 6000) | tabla 3, `.ino` y `.py` | en `!I` el paso 6 va de costado ese tiempo sin mirar la línea; un cruce son ~4400 ms. `!A` nunca mide más de ~3150 |
| Se corta el cable CON CABLE contando o andando: cartel ROJO "SIGUE SOLO", PROBAR apagado hasta que el robot hable o pase su tope + 5 s | ventana | el robot no se entera y sigue; antes decía "ESPERA" |
| `--quien` y VOLVER AL PARTIDO buscan un Teensy en MODO CARGA (código 8) | ventana, `TABLERO.bat` | `teensy-cli` le carga primero a ese (8.9) |
| PARAR con un programa desconocido manda un espacio | ventana | la `x` en ajustar-movimientos pisa sus números (2.6) |
| El IDA del paso 5 guarda la hora: solo un `#OK` de los 3 s siguientes, con el paso 5 a la vista, manda `!10`; se olvida con un `#FRENO`, un `#ERROR`, al (des)conectar o si nadie contesta | ventana | un pedido viejo mandaba `!10` solo al tocar `msPruebaIda` con + y − |
| 6a: `lateralInvertido` se calcula contra el valor CON EL QUE CORRIÓ el 2 (de `#USADOS`) | ventana | antes daba siempre 0 = derecha: al repetirlo con el 1 puesto, sugería volver al malo |
| Una medición de cancha repetida sola con `rc` NO se suma si es de antes de apagar (`apagado=1`) o si esta ventana no la pidió; escribiendo `rc` en la CONSOLA sí | ventana | una medición de la escuela se mezclaba con la cancha de hoy |
| El informe de un movimiento frenado con tecla o por el tope no dice "NO vio la línea", "se enderezó" ni "al final, quieto" | ventana | eran falsos (el robot tampoco da ese consejo) |
| CANCHA avisa si el robot mide con rampa, mezcla, rumbo o ida que no son los del partido; el sugerido del ancho usa solo corridas con la rampa del partido; la hoja guarda `vel`, `rampa` y `umbral` de cada corrida | ventana | lo medido vale para los números de ESE momento |
| Buscando el giroscopio al PROBAR: avisa que puede arrancar y no dice "no arrancó" | ventana | el robot lo busca hasta ~6 s antes de contar |
| La columna "ángulo" de la 9 dice `-` salvo en SALE_* y MUY_ABIERTO | ventana | el robot manda `ang=0.0` en lo demás |
| Más renglones de ejemplo en la autoprueba (`#INICIO F FONDO_COMPROBAR`, `#REPITE L LINEA_GUIADA`, `#MEDIO ok=0`, los `#FRENO`, `#FIN C`) | autoprueba | copiados de cómo los arma el `.ino` |

Terminado el 2026-11-08 (la sesión del 03/11 se cortó al final del arreglo):

| Qué | Dónde | Por qué |
|---|---|---|
| Un Teensy en modo carga cuenta UNA vez (pnputil lo lista como `USB\...` y su hijo `HID\...`) | ventana (`ids_en_modo_carga`) | `--quien` decía dos renglones de "MODO CARGA" por un solo Teensy |
| PROBAR en MOVIMIENTOS y un "volvió sin nada nuevo" (`sin_cable_perdido`) olvidan el IDA del paso 5 que quedó en curso | ventana | un `#FIN 10` posterior se anotaba en la hoja con los ms de la ida vieja |
| El cartel de VOLVER AL PARTIDO y el final de `TABLERO.bat` dicen que cargan la **0.1103** (`seguir-y-despejar`), NO la 0921 | ventana, `TABLERO.bat` | desde el 03/11 se juega con `funciona/partido-0921`; el tablero todavía es de la 0.1103 (adaptarlo es otro trabajo) |

**Lo que NO se cambió, a propósito:** el robot sigue sin frenarse si se corta el cable (6.1). Se
podría mirar el contador de cuadros del USB (`USB1_FRINDEX`: sin PC no cambia) para frenar una prueba
CON cable cuando se suelta, pero hay que probarlo con el robot antes de confiar en eso.

## 16. LA PESTAÑA PARTIDO Y LA LÍNEA `#PARTIDO` (2026-11-08)

Pedido del equipo (03/11): *"el tablero necesitamos que este adaptado a este programa que parece mucho mas
confiable. tambien estaria bueno que desde ahi se pueda cargar el programa"*, *"poder cargar todo facil y
cambiar todo facil es la prioridad mas grande"* y *"agrega tambien lo de la linea con la version"*.

### 16.1 Qué es
Una pestaña nueva, **★ PARTIDO** (entre CANCHA y CONSOLA), que muestra y cambia los valores del programa
**con el que se juega**: `funciona/partido-0921/partido-0921.ino` (la 0921, NO la 0.1103 de
`seguir-y-despejar`). Arriba lo de cada cancha (los sensores, el tamaño), abajo lo demás, en 6 grupos
(AVANZADO arranca plegado). Desde ahí se **guarda** (con copia) y se **carga** con un botón. El robot
dice su versión y sus valores por el cable al arrancar, y la ventana confirma que quedó cargado.

`PARTIDO.bat` (en `arquero-teensy-zircon`) abre la ventana directo en esa pestaña
(`tablero.py --pestana partido`). No carga nada solo.

### 16.2 La lista (`TABLA_PARTIDO` en `tablero.py`, sección 1b)
73 filas: `(nombre, declaración, tipo, FÁBRICA, unidad, mín, máx, paso, grupo, qué es, para qué lado,
atado)`. Tipos: `i` int, `u` unsigned long, `f` float, `b` bool, `l` lado (-1/+1), `v` solo para ver
(`VEL_MEDIO_MEDIR`). FÁBRICA = el texto tal cual en la 0921f (congelada en `respaldos/partido-0921f/`).
Los mínimos y máximos son SEGUROS (ninguna fuerza pasa de 255). Las variables de estado
(`msIdaDespeje`, `rumboBase`, los `t_*`...) NO van: las pisa el programa.

### 16.3 Leer y escribir el `.ino` (todo puro, salvo `guardar_partido`)
- Se lee en BYTES, UTF-8 estricto. `mapa_de_lineas` sigue las llaves saltando comentarios, "cadenas" y
  'caracteres': solo vale una declaración que empieza en la columna 0 con profundidad 0 y fuera de un
  `/* */`. Una por nombre (`_re_declaracion`, con `[ \t]` y `[0-9]`, no `\s` ni `\d`).
- Lo que falta, está repetido o es raro (otro tipo, `500 + 33`) queda GRIS y no se escribe. Si las llaves
  no cierran, no se escribe nada.
- Se cambia SOLO el texto del valor; el comentario queda. Mismo valor (2.870 y 2.87) = no se reescribe:
  sin cambios, el archivo queda idéntico byte a byte (también con CRLF). NUNCA `splitlines()` ni
  `escribir_atomico()` (pasa todo a CRLF): `split("\n")` y bytes.
- Validar en 3 capas: cada valor (tipo y rango), las reglas (16.4), y `verificar_escritura` (mismos
  renglones, solo cambian esos valores, leyéndolo de nuevo da lo pedido).
- `guardar_partido`: ruta prohibida (la autoprueba) → huella sha256 (si cambió por afuera:
  `CambioPorAfuera`, no escribe) → validar → **copia** en `pruebas/tablero/copias-partido/partido-0921_AAAA-MM-DD_HH-MM-SS.txt`
  (**.txt**: PlatformIO compila todos los `.ino` de la carpeta, y `pruebas/tablero` también las
  subcarpetas) → `.tmp` + `os.replace` (3 intentos) → volver a leer. Cambia también
  `SELLO = "AAAAMMDD-HHMMSS"`. Anota en `copias-partido/registro.txt`.
- Cada 2 s (y al elegir la pestaña) mira si el archivo cambió por afuera: lo vuelve a leer y avisa.
- "Volver a como estaba" (la copia más nueva con VALORES distintos) y "Volver a los de fábrica" ponen los
  valores en el EDITOR (amarillo): no escriben nada. Primero VACÍAN el editor (16.10): queda EXACTO la copia o
  fábrica, y la ventanita lista los cambios del editor que se descartan.

### 16.4 Las reglas (`reglas_partido`)
ERROR (no deja guardar ni cargar): E1 pwmMinLateral > pwmMaxLateral · E2 PWM_MIN_ACOMODO > PWM_MAX_ACOMODO ·
E3 TOLERANCIA_ACOMODO ≥ TOLERANCIA_ESPERA · E4 MS_PAUSA_MEDIO > MS_MAX_ESPERA_PLANO · E5 (a fuerza 200) la
ida llega a `msAdelante/10 − 3,3` cm ≤ umbralCm · E6 fuera de rango o de otro tipo.
OJO (deja, y lo repite al cargar): O1 a O17 (DISENO del 08/11: alcance justo, MS_MAX_RETROCESO < 2,4 ×
msAdelante, pwmMaxLateral > 160, sin límite lateral y rápido, el tope lateral, cambiar lo del movimiento
o la cámara o un signo o LADO_MEDIO, diagonal SÍ, enderezar NO, sin patrulla, potenciaDespeje ≠ 200,
asentar < freno, umbral cerca de lo medido, MS_MAX_ADELANTE < msAdelante). NOTA: N1 (con fábrica sale solo
esta), N2 (no puede pasar con los rangos de hoy), N3.

### 16.5 CARGAR (un hilo: `Cargador`, nunca toca tkinter)
Botón verde **▶ CARGAR EL PARTIDO CON ESTOS VALORES** (y **Cargar el TABLERO**, para MOVIMIENTOS y
CANCHA). Gris si: se está cargando, es la demo, no se pudo leer, hay un ERROR, no está el arquero (o el
puerto está ocupado, o es el delantero), o el tablero se está moviendo. El motivo se ve abajo del botón.
Antes, una ventanita: *"APENAS TERMINA LA CARGA, EL ROBOT ARRANCA SOLO..."*, con lo que cambia y los OJO.
1. `quien()` (el de `--quien`, con pnputil): tiene que ser SOLO el arquero (13348550), sin delantero ni
   Teensy en MODO CARGA. Si no, no se escribió nada.
2. Guarda (con copia). 3. Compila: `pio run -e teensy41 -d <carpeta>` (sin `-t`; el robot sigue
   conectado). Si no compila, vuelve el archivo a la copia y los valores quedan en el editor.
4. Suelta el puerto (`Conexion.cerrar(esperar=True)`). 5. `cargar.py <carpeta>` tal cual (nunca
   `--mirar`; si dice OTRA VENTANA, UN reintento a los 2 s). Siempre con `stdin=DEVNULL`,
   `CREATE_NO_WINDOW`, `PYTHONIOENCODING=utf-8`, `PYTHONUNBUFFERED=1`, y un tope (300 s compilar, 240 s
   cargar; si se pasa, `taskkill /T`).
6. Se vuelve a conectar SIEMPRE y confirma con la `#PARTIDO` (de la salida de `cargar.py` o de la conexión
   nueva, hasta `ESPERA_VERSION` = 25 s; si llega más tarde, confirma igual): **LISTO** (verde) / otros
   valores (rojo) / todavía no dijo su versión (naranja).
Mientras carga, la ventana no le manda nada al robot y no se cierra (la carga quedaría a medias).
**VOLVER AL PARTIDO** (MOVIMIENTOS) ya NO lanza `CARGAR-PARTIDO.bat` (cargaba la 0.1103): va a la pestaña
PARTIDO y hace este mismo CARGAR.

### 16.6 La línea `#PARTIDO` (CONTRATO; la manda el programa de PARTIDO desde la 0921g, NO `tablero.ino`)
```
#PARTIDO version=0921g sello=fabrica giro=1 umbralBlanco=425 CAMARA_POR_CM=2.87 MS_MEDIO_AL_MEDIO=2228 MS_MEDIO_FONDO_POR_TIEMPO=2000 MS_MAX_RETROCESO=1500 msAdelante=533 umbralCm=30.0 umbralDesvio=20.0 potenciaDespeje=200 pwmMinLateral=70 pwmMaxLateral=160 kpLateral=11.50 MS_QUIETO_PARA_PATRULLAR=10000 interruptores=1111111110101
```
- Campos en ESTE orden (`CAMPOS_PARTIDO`): **version primero, interruptores último** (si no, vino cortado
  = ROTA). Decimales: CAMARA_POR_CM 2, umbralCm 1, umbralDesvio 1, kpLateral 2; el resto enteros.
- `giro` = `(hayGiroscopo && giroscopoRespondiendo) ? 1 : 0`. `interruptores` = 13 `0`/`1` en el orden de
  `ORDEN_INTERRUPTORES` (volverAlMedioEnElLateral, patrullarSinPelota, patrullaPorElLateral,
  irAlMedioSiNoHayLinea, avanzarAlUbicarse, perseguirEnElDespeje, esperarPlanoAntesDeVolver,
  rampaAlVolver, empujonFinal, despejeEnDiagonal, enderezarActivado, lateralInvertido, camaraYInvertida).
- `sello` = `const char* SELLO` ("fabrica" en el del taller; el tablero lo cambia al guardar): confirma
  TODOS los valores, también los que no van en la línea.
- La manda `avisarVersion()`: una vez al final de `setup()` y cada 1/2 s en `ARMANDOSE` (quieto, mientras
  parpadea). Empieza con `if (!Serial) return;`: sin cable no hace nada. No hay `Serial.begin`, `read`,
  `available` ni `delay` nuevos: **juega exactamente igual que la 0921f**.
- La ventana: `"PARTIDO"` en `ETIQUETAS` (27), `_i_partido`; al llegar, `programa_detectado = "PARTIDO
  0921g"` y NO le manda nada (ni `h` ni `?`). `comparar_con_robot` → IGUAL / DISTINTO / OTRA_VERSION.

### 16.7 CANCHA → PARTIDO
"PASAR A PARTIDO →" (en CANCHA, al lado de TERMINAR) y "Pasar lo medido en CANCHA →" (en PARTIDO): una
ventanita con HOY / MEDIDO / de dónde. Tildados: umbralBlanco, MS_MEDIO_AL_MEDIO, MS_MEDIO_FONDO_POR_TIEMPO
(y el tope lateral si hace falta). SIN tildar: CAMARA_POR_CM, msAdelante (lo que hace falta para seguir
llegando a lo de hoy) y los signos (solo si cambian, en rojo). Pasar solo toca el EDITOR. La columna HOY
de CANCHA ahora es la de la 0921.

### 16.8 La autoprueba
Cuatro partes nuevas (A a K del diseño del 08/11): "el programa de partido", "la línea de versión", "la
carga (sin ejecutarla)" y "la pestaña PARTIDO (tkinter de mentira)". Todo sobre COPIAS en la carpeta
temporal; el `.ino` de verdad queda en `RUTAS_QUE_NO_SE_ESCRIBEN`, y al final se controla que su huella y
su carpeta quedaron iguales. `SIN_PROCESOS = True`: nada se ejecuta de verdad (ni pio ni cargar.py) ni se
abre ningún USB.

### 16.9 Lo que NO se hizo (y por qué)
- La simulación de la carga en la demo (B): en la demo CARGAR queda gris; GUARDAR escribe la copia de
  práctica.
- "Pasar a PARTIDO" desde MOVIMIENTOS (B): MOVIMIENTOS prueba copias de la 0.1103.
- `README.md` no se tocó en este paso (no estaba en los archivos permitidos): falta agregar PARTIDO.bat y
  "0921g" en la fila de `partido-0921`.
- Cerrar la ventana durante una carga NO se deja (en vez de "la carga sigue sola"): sin la ventana, la
  salida de `cargar.py` se queda sin tubo y la carga puede quedar a medias.
- Nada de esto se probó con el robot de verdad (ni la carga ni la línea): primera prueba, con la batería
  APAGADA.

### 16.10 Arreglos de la revisión (2026-11-08, a la noche)
- **El cartel de arriba** (`_texto_estado_partido`) sale de `estado_archivo`, que ahora es *lo último que se
  sabe* y se pone en CADA cosa que pasa: `guardado` (GUARDAR) · `cargado_sin_confirmar` (cargar.py dijo que sí
  y falta la `#PARTIDO`) · `cargado` (`#PARTIDO` IGUAL) · `no_coincide` (DISTINTO u otra versión) ·
  `a_medias` (falló grabando) · `tablero` (cargó el TABLERO, o llegó su `#HOLA` o su cartel) · `otro`
  (`#HOLA` u otro cartel: ajustar, la 0.1103, el del delantero). Editar NO lo cambia (mientras haya cambios
  dice "N CAMBIOS SIN GUARDAR"). Si no compila, vuelve al de antes de la carga. Una `#PARTIDO` que llega
  sin esperar una carga también lo pone (el SELLO confirma todos los valores).
- **La `#PARTIDO` que llega tarde confirma**: con `cargado_sin_confirmar`, la `#PARTIDO` confirma aunque
  hayan pasado los 25 s o hayan desenchufado y enchufado (lo que aconseja el aviso naranja). Con la batería
  apagada el `setup()` tarda ~11,5 s en decir la primera (busca el giroscopio 10 veces).
- **"Sigue con lo que tenía" solo si no empezó a grabar** (`motivo_de_carga`): los rechazos de cargar.py
  (ninguno, varios, delantero, otra serie, puerto ocupado, no encuentro) y un error de compilación sin
  ningún renglón de grabar (`MARCAS_DE_GRABAR`). El tope, "Waiting for Teensy" y todo lo que no se reconoce
  dicen `PUEDE_A_MEDIAS` y cómo salir (desenchufar; si no lo ve más: Arduino IDE, Teensy 4.1, solo el
  arquero, botón blanco). Le pasó a este robot el 12/10 ("error writing to Teensy").
- **Volver** vacía el editor antes (ver 16.3). El texto de la ventanita: "GUARDAR los escribe en el
  archivo; recién CARGAR EL PARTIDO los lleva al robot".
- **CANCHA paso 5 mide SOLTANDO, como la 0921**: la 0921 termina la ida con `parar()` (sigue de largo) y el
  tablero, con `frenarAntesDeVolver = 1`, frena. `_ida` le manda `=frenarAntesDeVolver 0` antes de
  `=msPruebaIda`, y `_valor_de_hoy` da `false` para la 0921 (`COMO_LA_0921`), así CANCHA avisa si el robot
  frena. En PASAR, msAdelante dice con qué se midió.
- **"Volver a los números del PARTIDO (0921)"** (MOVIMIENTOS, `BOTON_NUMEROS_PARTIDO`): manda `0` (lo
  compilado en tablero.ino, copias de la 0.1103) y después `=NOMBRE valor` de los que la 0921 (el ARCHIVO)
  tiene distinto y entran en el rango del tablero (`_numeros_de_la_0921`: hoy pwmMinLateral 70,
  pwmMaxLateral 160, msAdelante 533, despejeEnDiagonal 0, frenarAntesDeVolver 0). Así el aviso amarillo de
  CANCHA ("números que NO son los del partido") se va: antes mandaba al mismo botón y no se iba nunca (p. ej.
  LADO_TRASERA 91 en PARTIDO). El aviso y el paso 0 nombran ese botón.
- **MS_MEDIO_AL_MEDIO** "para qué lado" ya no dice izquierda/derecha: el límite lateral usa el mismo número
  desde los DOS laterales. Dice "no llega al medio → SUBIR; se pasa → BAJAR".
- **"Volver a esta cancha: X"** (COMPARAR) abre la misma ventanita de PASAR A PARTIDO con los números de ESA
  hoja (`_medido_para_partido(hoja=...)`, origen "hoja": no se anota en la hoja abierta). COPIAR AL PANEL
  avisa que es para el panel de la 0.1103 y que para jugar es PASAR A PARTIDO.
- QUEDA para quien pueda tocar esos archivos: `README.md` (PARTIDO.bat, TABLERO.bat, 0921g "dice su versión
  por el cable"), `funciona/seguir-y-despejar/CARGAR-PARTIDO.bat` (carga la 0.1103 con el título "CARGAR EL
  PROGRAMA DE PARTIDO"), `pruebas/ajustar-movimientos/AJUSTAR.bat:13`, `tablero.ino:71`,
  `ajustar-movimientos.ino:43`, `pruebas/herramientas/README.md:47` y `cargar.py:9`, que siguen mandando a
  CARGAR-PARTIDO.bat.

### 16.11 LA 0921i (2026-11-09, a la tarde)

Pedido del equipo: *"no, adapta el programa solo para el 0921i despues podemos ver si lo adaptamos para la
estrategia 2"*. O sea: el tablero maneja la **0921i** que quedó en `funciona/partido-0921` (la de `ebb5518`);
`funciona/estrategia-2` NO se tocó (es otro trabajo).

**Ojo, hubo DOS "0921i".** A la mañana del 09/11 se armó una con la patrulla por la línea PRENDIDA y
`MS_QUIETO_PARA_PATRULLAR` 15000; a la tarde se cambió sin cambiarle el nombre. Un armado del tablero para la de la
mañana se cortó a la mitad (sin commitear) y se siguió desde ahí. Lo que vale es la de `ebb5518`: esos son los de
FÁBRICA de `TABLA_PARTIDO` (la autoprueba lo controla si el archivo dice `sello=fabrica`).

**Qué cambió contra la 0921g** (`git diff a5aba53 ebb5518 -- funciona/partido-0921/partido-0921.ino`):

| Qué | 0921g | 0921i | En el tablero |
|---|---|---|---|
| `MS_AVISO_ARMADO` | 2000, después de prender el giroscopio (~4,5 s en total) | **3000, desde que se PRENDE**. El giroscopio NO arranca en paralelo: `setup()` lo espera (`bno.begin`, `delay(1000)`, los datos) con el LED APAGADO y `loop()` empieza a los ~2,2-2,6 s; el parpadeo es solo lo que sobra (con 3000, ~0,5 s) | AVANZADO › Arranque, **de 3000 a 10000** (revisión: con 2000 la cuenta ya estaba cumplida al empezar `loop()` y se movía sin parpadear) |
| `msEmpujonFinal` | 320 (~10 cm) | **450** (~15 cm, cuenta del 18/08) | grupo 3 |
| `MS_QUIETO_PARA_PATRULLAR` | 10000 | **20000** | grupo 5 (va en la `#PARTIDO`) |
| la patrulla al lateral a los DOS lados (de la 0921h) | no | `alternarLadoPatrulla` true | grupo 5, nueva |
| la VUELTA AL MEDIO CON LA LÍNEA DEL ÁREA | no | `buscarLineaDelAreaAlVolver` true: volviendo de un lateral (patrulla, ir al medio, límite lateral), si un sensor de atrás ve la línea del área, avanza `MS_AVANCE_10CM` (450, a `potenciaEmpujon`) y va al medio `MS_PUNTA_A_PUNTA_FIJO / 2` a `VEL_LINEA` | grupo 5, nuevas |
| la PATRULLA POR LA LÍNEA DEL ÁREA | no | escrita pero **APAGADA** (`patrullaPorLaLinea` false) | grupo 5, nuevas |
| EEPROM | - | 3584..3595 `LIN1` (lo que la patrulla por la línea midió sola) | mapa 1.2 |
| `SELLO` | `fabrica` | decía `20261109-092611` (el de un guardado de la 0921g a las 09:26, que quedó pegado) → **vuelve a `fabrica`** (solo eso y comentarios en el `.ino`) | |

**La línea `#PARTIDO`** (16.6) NO cambia de campos: la 0921i manda los mismos 16 campos y los 13 interruptores que
la 0921g. Lo nuevo lo confirma el `SELLO`. `LINEA_PARTIDO_FABRICA` = la de la 0921i con `MS_QUIETO_PARA_PATRULLAR=20000`.
Una `#PARTIDO version=0921g` se sigue entendiendo y se dice **"VERSIÓN ANTERIOR"** (`VERSIONES_PARTIDO_ANTERIORES`).
`FABRICA_DE_ANTES` dice lo que valía la 0921g (y la 0921f, `None`): 10000 / 320 / 2000, sin las 10 filas de
`NUEVOS_EN_LA_0921I`. Un archivo 0921g o 0921f se lee: lo que no tiene es un OJO (no un ERROR) y esas filas quedan
grises. "Volver a como estaba" desde una copia de otra versión NO vuelve lo que cambió la versión
(`cambiados_por_la_version`), solo lo que cambió alguien con el tablero.

**El grupo 5 · LA PATRULLA Y EL LÍMITE LATERAL (0921i)**, en el orden en que lo pidieron, con subtítulos:

| Subtítulo | Filas (fábrica) | Gris si... |
|---|---|---|
| Cuándo sale a patrullar | `patrullarSinPelota` (SÍ), `MS_QUIETO_PARA_PATRULLAR` (20000, 5000..60000), `MS_SIN_PELOTA_PARA_PATRULLAR`, `CUADROS_PARA_CORTAR_PATRULLA` | sin patrulla |
| Cuál patrulla | `patrullaPorLaLinea` (NO; poner SÍ pide confirmar: está a medio probar), `patrullaPorElLateral` (SÍ), `alternarLadoPatrulla` (SÍ) | la del lateral: sin patrulla o con la de la línea |
| Volviendo al medio desde un lateral | `buscarLineaDelAreaAlVolver` (SÍ), `MS_AVANCE_10CM` (450, 100..800, de a 10) | si nunca vuelve de un lateral / sin la vuelta con la línea |
| El ANCHO DEL ÁREA | `MS_PUNTA_A_PUNTA_FIJO` (2548, 0..4000, de a 10), `VEL_LINEA` (90, 75..150, de a 5) | si no lo usa nadie (ni la vuelta con la línea ni la patrulla por la línea) |
| La patrulla POR LA LÍNEA DEL ÁREA | `MS_QUIETO_PARA_MIRAR` (3000, 0..5000: "ES DE PRUEBA", para jugar bajarlo), `VEL_ATRAS_A_LA_LINEA` (80, 75..150), `MS_PUNTA_SOSTENIDA` (80), `MS_TOPE_PUNTA` (5000) | sin la patrulla por la línea (así viene) |
| Límite lateral | `LECTURAS_PARA_CREER_LATERAL` | sin límite lateral |

Las grises salen de `SE_USA_SI_PARTIDO` (manda sobre `GRIS_SI_PARTIDO`), con las cuentas de lo que hace el `.ino`
(`arrancarPatrulla()`, `vueltaVioLaLineaDelArea()`, `MEDIO_LINEA_AL_MEDIO`): `patrulla_por_la_linea`,
`patrulla_al_lateral`, `vuelve_de_un_lateral` (la patrulla al lateral, `irAlMedioSiNoHayLinea` o
`volverAlMedioEnElLateral`), `vuelta_con_la_linea` y `usa_el_ancho_del_area`. Lo que falta en el archivo cuenta como
NO si es de la 0921i y como SÍ si es de siempre. `potenciaEmpujon` ahora también la usa el avance de la vuelta.
Quedan fijos en el `.ino` (no van en la tabla): `MS_MIN_ANTES_DE_PUNTA` 300, `LADO_PATRULLA_LINEA` -1,
`remedirLineaSiempre` false, `MS_VERDE_PARA_ARMAR` 150 y `MS_BLANCO_LINEA_VUELTA` 20.

**Las reglas nuevas** (16.4): **E7** `MS_PUNTA_A_PUNTA_FIJO` entre 1 y 499 · **E8** `MS_PUNTA_A_PUNTA_FIJO` 0 con la
vuelta con la línea (no iría al medio) · **O18** `MS_QUIETO_PARA_MIRAR` ≥ 1000 con la patrulla por la línea · **O19**
cambiar `VEL_LINEA` sin cambiar `MS_PUNTA_A_PUNTA_FIJO` (aunque hoy no lo use nadie) · **O20** 0 con la patrulla por la
línea (usa lo que midió solo) · **O21** `MS_TOPE_PUNTA` < `MS_PUNTA_A_PUNTA_FIJO` con la patrulla por la línea · **O22**
lo medido en CANCHA paso 7 fue a otra fuerza que `VEL_LINEA` (si no salió la O19). La O6 (tocar lo del movimiento)
nombra también `MS_PUNTA_A_PUNTA_FIJO`. Con la fábrica de la 0921i sale solo la N1, como antes.

**CANCHA paso 7 · Medir el ancho del ÁREA** y **la P** (CONTRATO, los dos lados):
- `!P [fuerza] [c]`: fuerza de 75 a 150 (sin número, `VEL_LINEA_AREA` = 90). La ventana manda la `VEL_LINEA` de la
  pestaña PARTIDO (el archivo con el editor encima): `!P 90` / `!P 90 c`.
- `#INICIO P ANCHO_AREA tope=12000 sincable=0|1` → `#AREA ok= error= ms= izq= vel= umbral= desvio= rampa=` (`ok`
  primero y `rampa` último; `ms` = de costado a costado, lo que va en `MS_PUNTA_A_PUNTA_FIJO`; `izq` = hasta la línea
  izquierda; `vel` = la fuerza con que midió) → `RESULTADO: P ANCHO_AREA -> ...` (texto) → `#FIN P`. Errores: 0 midió
  bien, 2 el sensor de ADELANTE sobre una línea al terminar la cuenta, 3 no encontró la línea en 4 s, 4 se cayó el
  giroscopio (10 mudas seguidas), 5 menos de 500 ms (no creíble), 6 lo frenaron, 7 el tope. `rc`, `#REPITE P` y
  `#GUARDADO que=cancha n=P` como las otras de cancha; guarda en el bloque de cancha del tablero (2944..3071): no
  toca 3584 ni 3600.
- La lógica es COPIA de `pruebas/medir-ancho-area` (los mismos números: la autoprueba los compara contra ese `.ino`):
  adentro del área, el sensor de ADELANTE en el verde; de costado a la IZQUIERDA hasta que ese sensor ve la línea del
  costado del área, frena, 500 ms quieto, y a la DERECHA (primero deja atrás la primera línea) hasta la del otro costado
  CONTANDO EL TIEMPO. Cambia: la fuerza la manda la ventana, la cuenta es la del tablero (3 s, o 10 s sin cable), PARAR
  y el tope andan, y el resultado va a la EEPROM del tablero.
- `FECHA_CONTRATO` = `2026-11-09`. Con un `#HOLA` de un tablero más viejo la ventana NO manda `!P` y dice "Cargar el
  TABLERO".
- La ventana: el paso dice cómo apoyarlo (y lo repite al mandar `!P`); el sugerido es el promedio de las corridas buenas
  A LA `VEL_LINEA` DE PARTIDO (las de otra fuerza no cuentan, y lo dice), con aviso si hay una sola o difieren más de
  5 %; la hoja guarda `[AREA] sugerido` y `vel`; en vez de USAR ESTE / COPIAR AL PANEL tiene **PASAR A PARTIDO →**, que
  lo pone (tildado) en `MS_PUNTA_A_PUNTA_FIJO` del EDITOR. De una hoja medida a otra fuerza sale en rojo y no se pasa.

**La autoprueba** suma: la tabla de la 0921i (fábrica, rangos, orden del grupo, textos, confirmación), las grises en
cada combinación, E7/E8/O18..O22, la 0921g como ANTERIOR (línea, archivo, copias), la P en el traductor, la memoria, el
robot de mentira y la ventana, el `#AREA` de `tablero.ino` campo por campo, los números de la P contra
`medir-ancho-area.ino`, y sobre el partido de verdad (solo leído): que sea la 0921i, que con `sello=fabrica` tenga los
de fábrica, y que siga haciendo lo que dicen los textos del grupo 5 (la vuelta a `VEL_LINEA` la mitad, el avance a
`potenciaEmpujon`, `patrullaPorLaLinea` manda, la alternancia, los 300 ms y la cuenta desde que se prende).

**La revisión (09/11 a la tarde) arregló tres textos que no decían lo que hace la 0921i:**
- `MS_AVISO_ARMADO`: el mínimo pasó de 2000 a **3000** (ver la tabla de arriba). Un archivo 0921g con 2000 (que ahí
  sí servía: se contaba después del giroscopio) sale en rojo (E6): para jugar, cargar la 0921i.
- CANCHA 2c: dice que vuelve al medio SOLO POR TIEMPO (5.5) y se llama "Comprobar ir al medio (por tiempo)".
  `MS_MEDIO_AL_MEDIO` dice que si en el partido corta en la línea y ahí queda corrido, es `MS_PUNTA_A_PUNTA_FIJO`.
- `MS_AVANCE_10CM` ya no dice "queda pegado a la línea → SUBIR" (eso es `msEmpujonFinal`): después del avance y de
  ir al medio viene `MEDIO_AL_AREA` (atrás HASTA la línea) y el empujoncito, así que dónde queda al final lo decide
  `msEmpujonFinal`; `MS_AVANCE_10CM` solo cambia por dónde va de costado al medio. `msEmpujonFinal` lo dice.

**Lo que NO se hizo:** la estrategia 2 (después, si el equipo quiere); nada se probó con el robot (el robot tiene
cargada la estrategia 2: cargar el TABLERO o el PARTIDO desde el tablero la reemplaza); `README.md` y los `.bat` no se
tocaron.
