---
title: "Análisis FIEL — Delantero 2025 (definitivo-delantero)"
date: 2026-06-18
status: referencia-fiel
tipo: analisis-codigo-historico
fuente: "futbol-roboliga2026/robots-2025/delantero/delantero-2025.ino (1214 líneas)"
fuente-original: "repo open-soccer-robocup-team2026 → software/_deprecated-2025/robot-delantero/definitivo-delantero.cpp"
copiado-a-este-repo: 2026-07-28
generado-por: "Claude (workflow 14 agentes, análisis línea por línea + crítico de completitud)"
---

> **TRANSCRIPCIÓN FIEL del código del Nacional 2025 (Buenos Aires, campeones).** Cada valor citado con `archivo:línea`.
> Objetivo: reconstruir el comportamiento EXACTO en el robot 2026 (más sensores), sin perder nada.
> ⚠️ Los DOS programas (`definitivo-arquero` y `definitivo-delantero`) son el MISMO firmware unificado arquero+delantero; cambian el `#define` de robot (arquero=ROBOT1, delantero=ROBOT2) y el estado inicial que selecciona el modo.
> ⚠️ Las mejoras con sensores nuevos van en una sección APARTE y marcada — NO contaminan la transcripción.
> Completitud (crítico adversarial que re-leyó el programa): **EXTREMADAMENTE fiel; cobertura_ok=false SOLO por 4 ítems no materiales (ver apéndice)**.

# DOCUMENTO FIEL — ROBOT DELANTERO 2025 (`definitivo-delantero.cpp`)

**Archivo fuente:** [`delantero-2025.ino`](delantero-2025.ino) (1214 líneas). Los números de línea
de este documento coinciden **exactamente** con ese archivo — el contenido es idéntico al
original, solo cambió el nombre.
**Contexto:** Nacional 2025 (Buenos Aires, campeones). Robot mono-placa con librería `zirconLib`, kicker físico (motor de pateo con rampa de PWM), cámara OpenMV v1 (paquete de 9 bytes por `Serial1`) e IMU BNO055 (I2C `0x28`) para heading.
**Robot activo en el build:** `ROBOT2` (línea 11 `#define ROBOT2`; `ROBOT1` está comentado en línea 10). Documento AMBOS robots.
**Método:** todo valor citado con número de línea verificado contra el `.cpp` real. Las primitivas de motor también se cruzan con `zirconLib` (apéndice del documento). Lo especulativo está SOLO en la sección 7.

---

## 1. Comportamiento en cancha (qué hace realmente el robot)

El programa contiene **DOS modos de juego embebidos en la misma máquina de estados** (mismo `switch`, línea 381): un modo **DELANTERO** y un modo **ARQUERO**. El estado inicial es `AVANCE_INICIO` (línea 138), que pertenece al modo DELANTERO. El modo arquero **no se alcanza nunca en este build** salvo que se cambie el estado inicial: ningún estado del flujo delantero transiciona a `impulso_inicial` ni a los `moverce_*`. Es decir, **tal como está compilado, el robot juega de DELANTERO**; el bloque arquero (líneas 1030-1211) es código presente pero inalcanzable desde el flujo normal.

### Modo DELANTERO (el que corre)
1. **Arranca pateando hacia adelante 700 ms** (`AVANCE_INICIO`): da un empujón inicial a la pelota si la tiene encima, acelerando el kicker con rampa.
2. **Gira sobre su eje buscando la pelota** (`IMPULSO_INICIAL_GIRANDO` → `GIRANDO`): primero un impulso fuerte de 70 ms a PWM 150 para vencer la inercia, luego giro lento sostenido a PWM 30.
3. **Cuando ve la pelota, frena y espera por inercia** (~1 s o 700 ms según el estado), luego **apunta** la pelota rotando hasta tenerla al frente (`APUNTAR_PELOTA`, tolerancia ±15°).
4. **Avanza recto hacia la pelota** (`AVANZANDO`) hasta tenerla cerca (`Xp ≤ 50`).
5. **Orbita la pelota** (centrado horario/antihorario, `CENTRANDO_*`) para alinear el conjunto robot+pelota con el arco rival (el AMARILLO, hardcodeado).
6. **Patea**: si quedó alineado con el arco hace una **patada larga** (`PATEANDO_*`: pausa 1 s → avanza pateando 500 ms → pausa 500 ms → retrocede 200 ms). Si llega a una línea blanca razonablemente orientado, hace una **patada corta**.
7. **Reacciona a las líneas blancas**: si cualquier sensor de línea detecta el borde del campo, **retrocede 400 ms** alejándose de esa línea (`DETECTA_LINEA_1/2/3`) y vuelve a buscar.
8. Hay **timeouts de seguridad** en todos los estados: si pierde la pelota >500 ms (estados de búsqueda) o >3 s (centrando), o si pasa demasiado tiempo en un estado (9 s, 10 s, 20 s, 25 s según el caso), vuelve a girar buscando.

### Modo ARQUERO (presente pero NO alcanzado en este build)
Patrullaría lateralmente sobre la línea de su arco (`moverce_derecha` ↔ `moverce_izquierda`), rebotando contra los blancos laterales con impulsos de 350 ms; al ver la pelota cerca y centrada (`Xp ≤ 50` y `|Yp| ≤ 5`) ejecutaría una secuencia de despeje (pausa 500 ms → patada 300 ms → pausa 1 s → retroceso hasta tocar línea). Ver §2.B.

---

## 2. Máquina de estados / modos

Variable de estado: `Estado estado = AVANCE_INICIO;` (línea 138). Enum completo: líneas 123-137. Cronómetros: `millis_inicio_estado` (línea 140), `millis_inicio_centrando` (141), `millis_pelota` (142, marca la última vez que se vio la pelota, actualizada en línea 329).

> **Regla estructural fiel:** en casi todos los estados, las comprobaciones de línea y timeout están al final del `case` SIN `else`/`return`, por lo que se evalúan SIEMPRE y la **última asignación de `estado` en el ciclo gana**.

### 2.A — Estados del modo DELANTERO

#### `AVANCE_INICIO` (383-391) — estado inicial
- Acción: `avanzar_patear()` (rampa de kicker + avance, línea 384).
- Salida: tras **700 ms** (línea 385) → `parar()`, resetea cronómetro, `estado = IMPULSO_INICIAL_GIRANDO` (387-389).

#### `IMPULSO_INICIAL_GIRANDO` (393-430) — impulso fuerte de giro
- Acción: los 3 motores a **PWM 150**, dir (INA=0, INB=1) (395-397) → giro a alta potencia.
- Salida por tiempo: tras **70 ms** (399) → `estado = GIRANDO` + reset (401-402).
- Salida por pelota: `if (haypelota)` (405) → `parar()`; y si además pasaron **1000 ms** (408, comentario "esperar un sgundo por la inercia") → `estado = APUNTAR_PELOTA` + reset (410-411).
- Salidas por línea (independientes): `s1≥blanco1`→`DETECTA_LINEA_1` (415-419); `s2≥blanco2`→`DETECTA_LINEA_2` (420-424); `s3≥blanco3`→`DETECTA_LINEA_3` (425-429).
- `break` (430).
> **Nota fiel:** los `if` de las líneas 399, 405, 415, 420, 425 son hermanos (todos dentro del case), se evalúan en el mismo ciclo; el de línea (último) prevalece si dispara.

#### `GIRANDO` (432-471) — giro lento de búsqueda
- Salida por pelota: `if (haypelota)` (433) → `parar()`; si pasaron **700 ms** (436) → `APUNTAR_PELOTA` + reset (438-439).
- Acción si NO hay pelota: `else { girar(); }` (442-445) → giro a PWM 30.
- Salida por tiempo: `if ((millis()-millis_inicio_estado >= 9000) && (abs(error) <= 50))` (447) → `AVANZANDO_POR_TIEMPO` + reset (450-451).
  - **AMBIGUO/INCONSISTENCIA (447):** el comentario dice "rango de ±70" pero el código compara `abs(error) <= 50`. **Valor real = 50.**
- Salidas por línea: tres `if` separados (455-469) → `DETECTA_LINEA_1/2/3` + reset.
- `break` (471).

#### `AVANZANDO_POR_TIEMPO` (473-505) — avance ciego temporizado
- Acción: `avanzar()` (474).
- Salida por tiempo: tras **500 ms** (476) → `IMPULSO_INICIAL_GIRANDO` + reset (478-479).
- Salida por pelota: `if (haypelota == true)` (483) → `APUNTAR_PELOTA` + reset (485-486).
- Salidas por línea: tres `if` (490-504) → `DETECTA_LINEA_*` + reset.
- `break` (505).

#### `APUNTAR_PELOTA` (507-559) — rota para encarar la pelota
- Acción: `if (abs(anguloPelota) >= tolerancia_apuntado)` [≥15°] (508):
  - `if (anguloPelota > 0)` (510): los 3 motores a **PWM 100·a = 40**, dir (0,1) "antihorario" (513-515).
  - `else` (517): los 3 a **100·a = 40**, dir (1,0) "horario" (520-522).
- Salida apuntado: `else` (525) [abs<15°] → `AVANZANDO` + reset (527-528).
- Salida sin pelota: `if (millis()-millis_pelota >= 500)` (531) → `IMPULSO_INICIAL_GIRANDO` + reset (533-534).
- Timeout: `if (millis()-millis_inicio_estado >= 10000)` (537) → `IMPULSO_INICIAL_GIRANDO` + reset (539-540).
- Salidas por línea: tres `if` (544-558) → `DETECTA_LINEA_*` + reset.
- `break` (559).

#### `AVANZANDO` (562-611) — avanza hacia la pelota apuntada
- Acción: `avanzar()` (563).
- Salida a centrar: `if (haypelota && (Xp <= tolerancia_cercania))` [Xp≤50] (567) → setea `millis_inicio_centrando = millis()`, reset, `CENTRANDO_horario` (569-571).
- Salida re-apuntar: `if (abs(anguloPelota) >= tolerancia_apuntado)` [≥15°] (575) → `APUNTAR_PELOTA` + reset (577-578).
- Salida sin pelota: `if (millis()-millis_pelota >= 500)` (582) → `IMPULSO_INICIAL_GIRANDO` + reset (584-585).
- Timeout: `if (millis()-millis_inicio_estado >= 20000)` [20 s, el más largo] (589) → `IMPULSO_INICIAL_GIRANDO` + reset (591-592).
- Salidas por línea: tres `if` (596-610) → `DETECTA_LINEA_*` + reset.
- `break` (611).

#### `CENTRANDO_horario` (613-671) — orbita la pelota (horario)
- Acción (siempre, 615-617): M1 `60·c=24` dir(0,1); M2 `60·c=24` dir(0,1); M3 `180·c=72` dir(1,0) → órbita.
- Salida a patear (alineado): `if (ARCO_CONTRINCANTE && haypelota && (abs(Yp - Ycontrincante) <= tolerancia_centrado))` [≤30] (621) → `PATEANDO_pausa_inicial` + reset de ambos cronómetros (623-625).
- Salida patear por tiempo: `else if ((millis()-millis_inicio_centrando >= 4000) && (abs(error)<=1))` (628) → `PATEANDO_pausa_inicial` (630-632).
- Salida re-apuntar: `if (abs(anguloPelota) >= tolerancia_apuntado)` (636) → `APUNTAR_PELOTA_horario` + reset (638-639).
- Salida sin pelota: `if (millis()-millis_pelota >= 3000)` [3 s] (643) → `IMPULSO_INICIAL_GIRANDO` (645-646).
- Timeout: `if (millis()-millis_inicio_centrando >= 25000)` [25 s] (650) → `IMPULSO_INICIAL_GIRANDO` (652-653).
- Salida por línea (PATRÓN DISTINTO): `if ((s1≥blanco1) or (s2≥blanco2) or (s3≥blanco3))` (657):
  - `if (abs(error)<=80)` (659) → `PATEANDO_corto_pausa_inicial` (661-662).
  - `else` (664) → `IMPULSO_CENTRANDO_antihorario` (666-667).
- `break` (671).

#### `IMPULSO_CENTRANDO_antihorario` (673-683) — arranque de órbita antihoraria
- Acción (674-676): M1 `60·ic` dir(1,0); M2 `60·ic` dir(1,0); M3 `180·ic` dir(0,1).
  - ROBOT2 (ic=0.55): 33/33/99. ROBOT1 (ic=0.5): 30/30/90.
- Salida: tras **500 ms** (678) → `CENTRANDO_antihorario` + reset (680-681).
- `break` (683).

#### `CENTRANDO_antihorario` (685-742) — orbita la pelota (antihorario, espejo)
- Acción (686-688): M1 `60·c=24` dir(1,0); M2 `60·c=24` dir(1,0); M3 `180·c=72` dir(0,1).
- Mismas salidas que `CENTRANDO_horario`: alineado→`PATEANDO_pausa_inicial` (692-696); por tiempo `4000 ms && abs(error)<=1`→`PATEANDO_pausa_inicial` (699-703); re-apuntar→`APUNTAR_PELOTA_antihorario` (707-710); sin pelota 3 s→`IMPULSO_INICIAL_GIRANDO` (714-717); timeout 25 s→`IMPULSO_INICIAL_GIRANDO` (721-724); línea con `abs(error)<=80`→`PATEANDO_corto_pausa_inicial`, else→`IMPULSO_CENTRANDO_horario` (728-739).
- `break` (742).

#### `IMPULSO_CENTRANDO_horario` (744-754) — arranque de órbita horaria
- Acción (745-747): M1 `60·ic` dir(0,1); M2 `60·ic` dir(0,1); M3 `180·ic` dir(1,0).
- Salida: tras **300 ms** (749) → `CENTRANDO_horario` + reset (751-752).
- **Asimetría fiel:** este impulso dura 300 ms vs 500 ms el antihorario (678).

#### `APUNTAR_PELOTA_antihorario` (756-798) — re-apunta durante órbita antihoraria
- Acción: `if (abs(anguloPelota)>=15)` (757): si `anguloPelota>0` los 3 a `100·a=40` dir(0,1) (762-764); `else` dir(1,0) (769-771).
- Salida apuntado: `else` (774) → `CENTRANDO_antihorario` + reset (776-777).
- Salida pelota lejos: `if (haypelota && (Xp >= tolerancia_cercania))` [Xp≥50, condición INVERTIDA] (780) → `APUNTAR_PELOTA` + reset (782-783).
- Salida sin pelota: `if (millis()-millis_pelota >= 500)` (786) → `IMPULSO_INICIAL_GIRANDO` (788-789).
- Timeout: `if (millis()-millis_inicio_centrando >= 10000)` (792) — **usa `millis_inicio_centrando`** → `IMPULSO_INICIAL_GIRANDO` (794-795).
- `break` (798).

#### `APUNTAR_PELOTA_horario` (800-843) — re-apunta durante órbita horaria (espejo)
- Acción: `if (abs(anguloPelota)>=15)` (802): `anguloPelota>0`→ 3 motores `100·a=40` dir(0,1) (807-809); `else`→ dir(1,0) (814-816).
- Salida apuntado: `else` (819) → `CENTRANDO_horario` + reset (821-822).
- Salida pelota lejos: `if (haypelota && (Xp >= tolerancia_cercania))` (825) → `APUNTAR_PELOTA` + reset (827-828).
- Salida sin pelota: `if (millis()-millis_pelota >= 500)` (831) → `IMPULSO_INICIAL_GIRANDO` (833-834).
- Timeout: `if (millis()-millis_inicio_estado >= 10000)` (837) — **usa `millis_inicio_estado`** (asimetría vs el antihorario) → `IMPULSO_INICIAL_GIRANDO` (839-840).
- `break` (843).

#### Secuencia PATADA CORTA (845-887)
- `PATEANDO_corto_pausa_inicial` (847-855): `parar()`; tras **500 ms** → `PATEANDO_corto_adelante`.
- `PATEANDO_corto_adelante` (857-866): `avanzar_patear()`; tras **200 ms** → `parar()` → `PATEANDO_corto_pausa`.
- `PATEANDO_corto_pausa` (868-876): `parar()`; tras **500 ms** → `PATEANDO_corto_atras`.
- `PATEANDO_corto_atras` (878-887): `retroceder_patear()`; tras **300 ms** → `parar()` → `IMPULSO_INICIAL_GIRANDO`.
- **Secuencia:** pausa 500 → adelante 200 → pausa 500 → atrás 300 → vuelve a girar. (NO vigila líneas.)

#### Secuencia PATADA LARGA (889-992)
- `PATEANDO_pausa_inicial` (891-914): `parar()`; tras **1000 ms** → `PATEANDO_adelante`. Además vigila líneas (899-913) → `DETECTA_LINEA_*`.
- `PATEANDO_adelante` (916-940): `avanzar_patear()`; tras **500 ms** → `parar()` → `PATEANDO_pausa`. Vigila líneas (925-939).
- `PATEANDO_pausa` (942-965): `parar()`; tras **500 ms** → `PATEANDO_atras`. Vigila líneas (950-964).
- `PATEANDO_atras` (967-992): `retroceder_patear()`; tras **200 ms** → `parar()` → `IMPULSO_INICIAL_GIRANDO`. Vigila líneas (976-990).
- **Secuencia:** pausa 1000 → adelante 500 → pausa 500 → atrás 200 → vuelve a girar. En CUALQUIER fase, ver línea aborta hacia `DETECTA_LINEA_*`.

#### Retrocesos por línea (994-1028)
- `DETECTA_LINEA_1` (994-1004): `retroceder1()`; tras **400 ms** → `parar()` → `IMPULSO_INICIAL_GIRANDO`.
- `DETECTA_LINEA_2` (1006-1016): `retroceder2()`; tras **400 ms** → idem.
- `DETECTA_LINEA_3` (1018-1028): `retroceder3()`; tras **400 ms** → idem.

### 2.B — Estados del modo ARQUERO (1030-1211, NO alcanzados en este build)

#### `impulso_inicial` (1032-1044)
- Acción asimétrica (traslación lateral): M2 `1.8·50=90` dir(1,0) (1034); M1 `1.8·50=90` dir(1,0) (1035); M3 `1.8·85=153` dir(0,1) (1036). Factor 1.8 hardcodeado inline.
- Salida: tras **40 ms** (1038) → `moverce_derecha`.

#### `moverce_derecha` (1046-1092)
- Acción: `adproporcional()` (1047).
- Si ve pelota (1052):
  - `if ((Xp <= tolerancia_cercania) && (abs(Yp) <= 5))` [Xp≤50 y |Yp|≤5] (1054) → `parar()`, `PATEANDO_pausa_inicial_arquero` (1056-1058).
  - `if (abs(Yp) >= 5)` (1061): `pd = 1.5` (1063); si `Yp<0`→`moverce_derecha` (1065-1069); `else`→`moverce_izquierda` (1070-1074).
  - `else { parar(); }` (1076-1077) — rama del `if(abs(Yp)>=5)`: cuando `abs(Yp)<5` (y no entró al pateo de 1054). **Zona muerta de centrado del arquero = 5.**
- `else { pd = 1; }` (1079-1082): sin pelota.
- Si ve blanco: `if (s1 >= blanco1 or s2 >= blanco2)` (1086, solo s1/s2) → `parar()`, `impulso_izquierda` (1088-1090).

#### `moverce_izquierda` (1094-1141) — espejo
- Acción: `aiproporcional()` (1095). Misma lógica de pelota (1100-1130). Si ve blanco (1134, solo s1/s2) → `impulso_derecha` (1137).

#### `impulso_derecha` (1143-1153): `adproporcional()`; tras **350 ms** → `moverce_derecha`.
#### `impulso_izquierda` (1155-1165): `aiproporcional()`; tras **350 ms** → `moverce_izquierda`.

#### Patada del arquero (1167-1211)
- `PATEANDO_pausa_inicial_arquero` (1168-1177): `parar()`; tras **500 ms** → `PATEANDO_adelante_arquero`.
- `PATEANDO_adelante_arquero` (1179-1189): `avanzar_patear()`; tras **300 ms** → `parar()` → `PATEANDO_pausa_arquero`.
- `PATEANDO_pausa_arquero` (1191-1199): `parar()`; tras **1000 ms** → `PATEANDO_atras_arquero`.
- `PATEANDO_atras_arquero` (1201-1211): `retroceder_patear()`; sale SOLO cuando `(s1≥blanco1) or (s2≥blanco2) or (s3≥blanco3)` (1204) → `parar()` → `moverce_derecha`.
  - **Tema a analizar (fiel):** este estado **NO tiene timeout temporal**, solo sale por línea. Si nunca detecta blanco, queda retrocediendo indefinidamente con `retroceder_patear()` (PWM 250/170).

Cierres: `}` switch (1213), `}` loop (1214).

---

## 3. Tabla de CONSTANTES EXACTAS por categoría

### 3.1 Pines de motor (difieren entre robots)
| Señal | ROBOT2 (activo) | línea R2 | ROBOT1 | línea R1 |
|---|---|---|---|---|
| INA1 / INB1 / PWM1 | 8 / 7 / 6 | 14-16 | 2 / 5 / 3 | 38-40 |
| INA2 / INB2 / PWM2 | 11 / 12 / 4 | 18-20 | 8 / 7 / 6 | 42-44 |
| INA3 / INB3 / PWM3 | 2 / 5 / 3 | 22-24 | 11 / 12 / 4 | 46-48 |

> Los 3 motores están **rotados** entre placas (los pines de M1 de R2 son los de M3 de R1, etc.). Los de ROBOT1 coinciden con `zirconLib "Mark1"`.

### 3.2 Umbrales de línea blanca
| Const | ROBOT2 | línea | ROBOT1 | línea |
|---|---|---|---|---|
| blanco1 | 650 | 26 | 600 | 50 |
| blanco2 | 650 | 27 | 600 | 51 |
| blanco3 | 750 | 28 | 600 | 52 |

### 3.3 PWM / potencia
| Concepto | Valor (R2) | Valor (R1) | línea |
|---|---|---|---|
| `c` (factor centrado) | 0.4 | 0.4 | 32 / 56 |
| `ic` (factor impulso centrado) | 0.55 | 0.5 | 33 / 57 |
| `g` (factor giro) | 0.3 | 0.3 | 88 |
| `a` (factor apuntado) | 0.4 | 0.4 | 89 |
| `pd` (factor avance prop., inicial) | 1 | 1 | 90 |
| `patadM1` (retroceso kicker M1) | 250 | 250 | 31 / 55 |
| `patadM2` (retroceso kicker M2) | 170 | 170 | 30 / 54 |
| `velocidadFinalPateo` (tope rampa) | 240 | 240 | 70 |
| `pasoPateo` (incremento rampa) | 5 | 5 | 71 |
| `girar()`: 3 motores | 100·g = 30 | 30 | 148-150 |
| `avanzar()`: M1,M2 / M3 | 100 / 0 | igual | 159-161 |
| `APUNTAR_PELOTA`: 3 motores | 100·a = 40 | 40 | 513-522 |
| `IMPULSO_INICIAL_GIRANDO`: 3 mot. | 150 | 150 | 395-397 |
| `CENTRANDO_*`: M1,M2 / M3 | 60·c=24 / 180·c=72 | 24 / 72 | 615-617, 686-688 |
| `IMPULSO_CENTRANDO_*`: M1,M2 / M3 | 60·ic=33 / 180·ic=99 | 30 / 90 | 674-676, 745-747 |
| `aiproporcional`/`adproporcional` (pd=1): M1,M2 / M3 banda ±2 | 50 / 89 | igual | 213-215, 237-239 |
| ai/ad rama error>0 (ai): M2,M1 / M3 | 50,50 / 40 | igual | 219-223 |
| ai/ad rama error<0 (ai): M2,M1 / M3 | 65,40 / 100 | igual | 227-231 |
| ad rama error>0: M2,M1 / M3 | 50,50 / 100 | igual | 243-247 |
| ad rama error<0: M2,M1 / M3 | 65,40 / 40 | igual | 251-255 |
| ARQUERO `impulso_inicial`: M1,M2 / M3 | 1.8·50=90 / 1.8·85=153 | igual | 1034-1036 |
| ARQUERO `pd` con pelota desviada | 1.5 | 1.5 | 1063, 1111 |
| ARQUERO `pd` sin pelota | 1 | 1 | 1081, 1129 |

> Comentarios `//60 //89 //99 //75 //120 //50` en `ai/adproporcional` (213-255): son valores objetivo/anteriores; **NO se aplican**, el valor en uso es el `pd*N`.

### 3.4 Tiempos (ms)
| Estado / acción | Valor | línea |
|---|---|---|
| `intervaloPateo` (paso rampa kicker) | 20 | 73 |
| `AVANCE_INICIO` | 700 | 385 |
| `IMPULSO_INICIAL_GIRANDO` → GIRANDO | 70 | 399 |
| `IMPULSO_INICIAL_GIRANDO` espera inercia (pelota) | 1000 | 408 |
| `GIRANDO` espera inercia (pelota) | 700 | 436 |
| `AVANZANDO_POR_TIEMPO` duración | 500 | 476 |
| `IMPULSO_CENTRANDO_antihorario` | 500 | 678 |
| `IMPULSO_CENTRANDO_horario` | 300 | 749 |
| centrado patear por tiempo | 4000 | 628, 699 |
| PATADA CORTA: pausa/adelante/pausa/atrás | 500 / 200 / 500 / 300 | 849/859/870/880 |
| PATADA LARGA: pausa/adelante/pausa/atrás | 1000 / 500 / 500 / 200 | 893/918/944/969 |
| `DETECTA_LINEA_1/2/3` retroceso | 400 | 997/1009/1021 |
| ARQUERO `impulso_inicial` | 40 | 1038 |
| ARQUERO `impulso_derecha`/`izquierda` | 350 | 1147/1159 |
| ARQUERO patada: pausa/adelante/pausa | 500 / 300 / 1000 | 1171/1182/1193 |

### 3.5 Timeouts y "sin ver pelota" (ms)
| Estado | Umbral | línea |
|---|---|---|
| `GIRANDO` → avanzar por tiempo (+ abs(error)≤50) | 9000 | 447 |
| `APUNTAR_PELOTA` timeout | 10000 | 537 |
| `AVANZANDO` timeout | 20000 | 589 |
| `CENTRANDO_*` timeout (sobre `millis_inicio_centrando`) | 25000 | 650, 721 |
| `APUNTAR_PELOTA_antihorario` timeout (sobre `millis_inicio_centrando`) | 10000 | 792 |
| `APUNTAR_PELOTA_horario` timeout (sobre `millis_inicio_estado`) | 10000 | 837 |
| Sin pelota (búsqueda/apuntado) | 500 | 531, 582, 786, 831 |
| Sin pelota (centrando) | 3000 | 643, 714 |

### 3.6 Umbrales de decisión
| Umbral | Valor | línea |
|---|---|---|
| `tolerancia_centrado` (alineado arco, |Yp-Ycontrincante|) | 30.0 | 118 |
| `tolerancia_cercania` (Xp) | 50.0 | 119 |
| `tolerancia_apuntado` (ángulo, grados) | 15.0 | 120 |
| Centrado: error para patear por tiempo | abs(error) ≤ 1 | 628, 699 |
| Centrado: error al pisar línea | abs(error) ≤ 80 | 659, 730 |
| GIRANDO: error para avanzar por tiempo | abs(error) ≤ 50 | 447 |
| ARQUERO: zona muerta de centrado en Y | abs(Yp) < 5 / ≥ 5 | 1054, 1061, 1102, 1109 |

### 3.7 PID / heading
| Const | Valor | línea | Uso |
|---|---|---|---|
| `kp` | 0.3 | 81 | `correccion = error*kp` (365) |
| wrap de error | ±180 | 363-364 | normaliza error a (-180,180] |

> **Fiel:** `correccion` (365) se CALCULA cada loop pero **no se aplica a ningún motor** en el `switch`. La corrección real de heading se hace por las ramas `error>0`/`error<0`/banda(±2) dentro de `ai/adproporcional`. `correccion` es código muerto. **El giroscopio se usa como GATE** (umbrales abs(error)≤50/≤80/≤1), no como feedback continuo en el delantero.

### 3.8 Otros
| Const | Valor | línea |
|---|---|---|
| `BAUD_RATE` (Serial y Serial1) | 19200 | 85 |
| `START_BYTE` | 0xAA (con `;` extra; **no usado**) | 84 |
| BNO055 | id=55, addr=0x28 | 76 |
| `LED_BUILTIN` | = haypelota | 285 |

---

## 4. Lógica de juego paso a paso (cámara → decisión → movimiento)

1. **Lectura de cámara** (287-353): si `Serial1.available() >= 9`, lee `header1`; si `==201` lee los 8 bytes restantes; valida `header1==201 && header2==202 && header3==203` (301). Decodifica `Xp/Yp`, `Xam/Yam`, `Xaz/Yaz` (Y con `-100`, 303-308).
2. **Ángulos** (311-313): `anguloPelota = atan2(Yp,Xp)·180/PI`, idem arcos.
3. **Presencia** (324-340): `Xp==0`→`haypelota=false`; si no `haypelota=true` y `millis_pelota=millis()` (timestamp). Idem arcos por `Xam==0`/`Xaz==0`.
4. **Arco rival** (355-356): `ARCO_CONTRINCANTE = hayarco_amarillo`; `Ycontrincante = Yam`. **El arco a atacar es el AMARILLO, hardcodeado.**
5. **Heading** (359-365): `error = currentYaw - initialYaw`, wrap ±180, `correccion = error·0.3` (no usada).
6. **Sensores de línea** (369-371): `s1` izq, `s2` centro, `s3` der.
7. **Decisión (switch)**: buscar (girar) → ver pelota (frenar+inercia) → apuntar (rotar a 40 PWM hasta |ang|<15°) → avanzar (100 PWM hasta Xp≤50) → orbitar (24/24/72 PWM hasta |Yp-Ycontrincante|≤30 o 4 s) → patear largo; o, al pisar línea con abs(error)≤80, patear corto; o, al pisar línea con error>80, invertir el sentido de centrado.

---

## 5. Movimiento de bajo nivel (traducción a PWM)

**Convención atómica:** cada rueda = `analogWrite(PWMn, magnitud)` + dos pines de dirección `digitalWrite(INAn, x); digitalWrite(INBn, !x)`. El sentido lo da SIEMPRE la pareja (INA,INB), nunca el signo del PWM. Esto es el esquema **"Mark1"** de `zirconLib` (PWM dedicado + 2 DIR digitales) expandido inline; este archivo NO llama a `motorN()`.

**Sin deadzone ni piso de PWM:** todos los valores son fijos por tabla (afinados a banco), escalados solo por `g/a/c/ic/pd/1.8`. La saturación `min(power,100)` de `zirconLib` NO aplica aquí (no se pasa por `motorN`), por eso se escriben PWM hasta 240/250 directo. No hay rampa de desaceleración ni short-brake; `parar()` deja ambas DIR en 0 (coast).

**Roles de rueda (deducidos):** M1, M2 = delanteras (a 100 con dir opuestas en `avanzar`), M3 = trasera (queda en 0 al avanzar recto; se modula 72/99/100 en órbitas y avances proporcionales para corregir heading). Omni de 3 ruedas.

**Kicker (rampa, `avanzar_patear`, 181-201):** cada `intervaloPateo` (20 ms), si `velocidadActualPateo < 240` suma `pasoPateo`(5) con clamp a 240; aplica ese PWM a M1 dir(1,0) y M2 dir(0,1), M3=0. Tarda ~960 ms en llegar al tope. **`velocidadActualPateo` NUNCA se resetea a 0 entre patadas** → solo la primera patada tras el boot es rampa suave; las siguientes ya arrancan en 240. **Retorno del kicker (`retroceder_patear`, 204-208):** M1=`patadM1`=250 dir(0,1), M2=`patadM2`=170 dir(1,0), M3=0 (retroceso asimétrico).

---

## 6. Visión: formato del dato y parseo

**Enlace:** `Serial1` a 19200 baud (263). **Paquete OpenMV v1 = 9 bytes** (287). Orden de lectura (289-299):

`[header1=201][Xp][Yp][header2=202][Xam][Yam][header3=203][Xaz][Yaz]`

- Lectura disparada por `Serial1.available() >= 9` (287).
- `header1` debe ser 201 (290); validación conjunta `201/202/203` (301). Si falla `header1`, se ponen todas las presencias en false (347-352).
- **Decodificación** (303-308): las X se usan crudas; las Y restan 100 (`Yp = codedYp - 100`, etc.) → permite Y negativos en un byte (rango -100..+155).
- Ángulos por `atan2(Y,X)·180/PI` (311-313).
- Presencia por `X==0` (324, 332, 337).
- **Sin checksum ni byte de sincronización real** (el `START_BYTE 0xAA` definido en 84 no se usa; lleva `;` de más).

---

## 7. MEJORAS CON MÁS SENSORES (NO es transcripción — es PROPUESTA)

> Esta sección es especulativa, sobre el sistema 2026 (3 placas CENTRAL/TOP/DOWN, 4 ToF, OTOS, 2 BNO, 2 cámaras). Las secciones 1-6 son la transcripción fiel; nada de abajo describe el código 2025.

### 7.1 Replicar EXACTO el comportamiento 2025 (paridad primero)
El comportamiento 2025 se puede reproducir 1:1: la FSM (buscar→apuntar→avanzar→orbitar→patear, retroceso por línea) es portable tal cual. La única dependencia "dura" es el dato de cámara y los 3 sensores de línea, ambos disponibles en 2026 (TOP da pelota/arcos; DOWN da línea). Recomendación: portar la FSM con los MISMOS umbrales (15°/Xp50/Yp30/error 50-80-1) y tiempos como **línea base de regresión** antes de tocar nada — así cualquier mejora se mide contra un baseline conocido.

### 7.2 Cerrar el lazo de heading (la mejora más jugosa)
Hoy el giroscopio es solo gate y `correccion=error·0.3` está muerto. Con 2 BNO + el patrón PI con feedforward (skill `control-pid-zona-muerta`) se puede:
- Reemplazar las 3 ramas hardcodeadas de `ai/adproporcional` (50/50/89, 50/50/40, 65/40/100…) por una corrección continua sobre la trasera M3, con deadzone-compensation y anti-windup. Risk-fix: re-tuneo en banco; risk-no-fix: el robot serpentea/orbita torcido.

### 7.3 Sustituir umbrales de píxel por pose real
- `Xp ≤ 50` (cercanía) y `|Yp| ≤ 5` (centrado del arquero) son coordenadas de cámara, sensibles a iluminación/FOV. Con OTOS+ToF fusionados (skill `fusion-pose-odometria-landmarks`) se puede expresar cercanía/alineación en cm/grados absolutos, más robusto a Incheon.
- El arco rival hardcodeado (`ARCO_CONTRINCANTE = hayarco_amarillo`, 355) puede pasar a config por lado de cancha (botón/strap), eliminando la edición de código entre partidos.

### 7.4 Robustez del enlace de cámara
- Añadir el `START_BYTE 0xAA` real (hoy sin uso, con bug del `;`) o un checksum: hoy un byte que coincida con 201/202/203 puede desincronizar el frame. Con 2 cámaras conviene un framing con sync + longitud.

### 7.5 Tolerancia a fallas
- `PATEANDO_atras_arquero` (1204) sin timeout: agregar salida por tiempo (estado seguro) para no quedar retrocediendo. Patrón watchdog (skill `sistemas-criticos-tolerancia-fallas`).
- `velocidadActualPateo` sin reset entre patadas: resetear a 0 al entrar a los `*_pausa_inicial*` para que la rampa del kicker sea consistente.
- BNO `while(1)` en setup (272): degradar con gracia (modo sin heading) en vez de bloquear el robot entero si falla la IMU.

> Todo lo anterior requiere **plan de prueba en hardware real** (skill `hardware-test-protocol`) y validación de banco antes de marcar nada como hecho.

---

## APÉNDICE — Cruce con `zirconLib`

Las funciones del `.cpp` usadas vía `zirconLib` (no definidas en este archivo): `InitializeZircon()` (261), `readLine(1/2/3)` (369-371). `zirconLib` también provee `motorN(power,direction)` con esquema Mark1 (`digitalWrite(dirA,dir); digitalWrite(dirB,!dir); analogWrite(pwm,min(power,100))`) — pero **este delantero NO la usa para mover**: expande la escritura de motores inline (de ahí que pueda superar el `motorLimit=100` de la librería, escribiendo 150/240/250). Los pines de motor de ROBOT1 coinciden con los de `zirconLib "Mark1"`; los de ROBOT2 están permutados.

---

**Inconsistencias/ambigüedades fieles (resumen):**
1. L447: comentario "±70" vs código `abs(error)<=50` → real = 50.
2. Asimetría impulso centrado: antihorario 500 ms (678) vs horario 300 ms (749).
3. Asimetría timeout APUNTAR_*: antihorario usa `millis_inicio_centrando` (792), horario usa `millis_inicio_estado` (837).
4. `correccion=error·kp` (365) calculada pero no aplicada (código muerto).
5. `START_BYTE 0xAA;` (84) con `;` extra, nunca usado.
6. `velocidadActualPateo` no se resetea entre patadas (181-201).
7. `PATEANDO_atras_arquero` (1201-1211) sin timeout — solo sale por línea.
8. Modo ARQUERO (1030-1211) presente pero **inalcanzable** con el estado inicial `AVANCE_INICIO`.
9. Patrón de salida por línea NO uniforme: tres `if` separados→`DETECTA_LINEA_*` en búsqueda/avance; un `if ... or ...` en centrado→patada corta o invertir sentido.

---

## Apéndice — Completitud (código muerto / detalles no materiales)

> `cobertura_ok=false` SOLO por estos 4 ítems no materiales (no alteran el comportamiento). El resto, verificado fiel.

- Enum miembro PRIMER_IMPULSO_INICIAL_GIRANDO (linea 129): declarado en el enum Estado pero SIN case en el switch y NUNCA asignado. El documento lista el enum (123-137) pero no menciona que este miembro existe ni que es codigo muerto (no se alcanza). Omision material de un estado declarado de la FSM.
- Variable global 'int i = 0;' (linea 143): declarada pero nunca usada en todo el programa. No figura en el documento ni como constante ni en las notas de codigo muerto (donde si estan correccion, START_BYTE).
- Declaracion 'float correccion;' (linea 77): el documento cita correccion solo en linea 365 (la asignacion). La DECLARACION sin inicializar esta en linea 77 — menor, pero la variable arranca indeterminada antes del primer loop (no se inicializa a 0). No documentado.
- avanzar() motor3 pines de direccion (linea 161): el documento dice 'M3 queda en 0' pero no transcribe que igual setea INA3=1, INB3=0 con PWM 0 (coast con direccion preseteada). Detalle de fidelidad menor ya capturado parcialmente en el JSON.

**Ramas:** Estado declarado PRIMER_IMPULSO_INICIAL_GIRANDO (linea 129) sin case en el switch (lineas 381-1213): no documentado como rama inexistente/muerta. Es un estado de la FSM presente en el enum que el documento no advierte que carece de case (caer en el seria no-op = solo el codigo comun del loop, sin movimiento).


---

## Apéndice — Tabla EXHAUSTIVA de constantes (152 ítems)

| Nombre | Valor | Robot | Categoría | Línea | Significado |
|---|---|---|---|---|---|
| `kp` | `0.3` | ambos | PID | 81 | Ganancia proporcional del lazo de heading (giroscopo). correccion = error * kp (linea 365). NOTA: 'correccion' se calcula pero AMBIGUO si se usa: ver notas, no se aplica directamente a motores en el switch. |
| `correccion = error * kp` | `kp=0.3` | ambos | PID | 365 | Salida del lazo P de heading. AMBIGUO: la variable 'correccion' se calcula pero no se aplica explicitamente a ningun motor en el switch (la correccion de heading real se hace via las ramas error>0/error<0 en ai/adproporcional). Ver notas. |
| `patadM2 (ROBOT2)` | `170` | R2 | PWM-potencia | 30 | PWM aplicado al motor 2 en retroceder_patear() (retroceso del kicker tras la patada). Ver linea 206. |
| `patadM1 (ROBOT2)` | `250` | R2 | PWM-potencia | 31 | PWM aplicado al motor 1 en retroceder_patear() (retroceso del kicker tras la patada). Ver linea 205. |
| `c (ROBOT2)` | `0.4` | R2 | PWM-potencia | 32 | Factor de velocidad de centrado (multiplica los PWM en CENTRANDO_horario/antihorario). Comentario linea 61: 'velocidad centrando'. |
| `ic (ROBOT2)` | `0.55` | R2 | PWM-potencia | 33 | Factor de velocidad del impulso de centrado (multiplica PWM en IMPULSO_CENTRANDO_*). Comentario linea 62: 'velocidad impulso centrando'. |
| `patadM2 (ROBOT1)` | `170` | R1 | PWM-potencia | 54 | PWM motor 2 en retroceder_patear() para ROBOT1. Igual a ROBOT2 (170). |
| `patadM1 (ROBOT1)` | `250` | R1 | PWM-potencia | 55 | PWM motor 1 en retroceder_patear() para ROBOT1. Igual a ROBOT2 (250). |
| `c (ROBOT1)` | `0.4` | R1 | PWM-potencia | 56 | Factor velocidad centrado ROBOT1. Igual a ROBOT2 (0.4). |
| `ic (ROBOT1)` | `0.5` | R1 | PWM-potencia | 57 | Factor velocidad impulso centrado ROBOT1. UNICA DIFERENCIA de tuneo escalar entre robots: R1=0.5 vs R2=0.55. |
| `velocidadActualPateo` | `0 (inicial)` | ambos | PWM-potencia | 69 | Variable de rampa del kicker. Arranca en 0 y sube de a pasoPateo (5) cada intervaloPateo (20ms) en avanzar_patear() hasta velocidadFinalPateo (240). |
| `velocidadFinalPateo` | `240` | ambos | PWM-potencia | 70 | PWM maximo del motor de pateo (rampa de aceleracion del kicker). Tope de velocidadActualPateo en avanzar_patear(). |
| `pasoPateo` | `5` | ambos | PWM-potencia | 71 | Incremento de PWM por cada paso de rampa del kicker (avanzar_patear, linea 191). |
| `g` | `0.3` | ambos | PWM-potencia | 88 | Factor de velocidad al GIRAR (buscar pelota). En girar() los 3 motores van a 100*g = 30 PWM. Comentario: 'girando'. |
| `a` | `0.4` | ambos | PWM-potencia | 89 | Factor de velocidad al APUNTAR la pelota. En APUNTAR_PELOTA los motores van a 100*a = 40 PWM. Comentario: 'apuntando pelota'. |
| `pd` | `1 (inicial)` | ambos | PWM-potencia | 90 | Factor proporcional de avance (multiplica los PWM en aiproporcional/adproporcional). Inicial 1. REASIGNADO en runtime: pd=1.5 cuando arquero ve pelota desalineada (lineas 1063,1111) y pd=1 cuando no ve pelota (lineas 1081,1129). Comentario: 'velocidades avances proporcionales'. |
| `girar(): PWM 3 motores` | `100 * g = 30` | ambos | PWM-potencia | 148 | Funcion girar(): los 3 motores a analogWrite(100*g)=30 PWM, todos sentido (INA=0,INB=1) -> giro sobre el eje. g=0.3 (linea 88). |
| `avanzar(): PWM motor1 y motor2` | `100` | ambos | PWM-potencia | 159 | Funcion avanzar(): motor1 a PWM 100 (INA=1,INB=0), motor2 a PWM 100 (INA=0,INB=1), motor3 a PWM 0 (apagado). Avance recto. |
| `avanzar(): PWM motor3` | `0` | ambos | PWM-potencia | 161 | En avanzar() el motor 3 (trasero) queda en 0 PWM (no contribuye al avance recto). |
| `retroceder1(): PWM motores` | `m1=0, m2=100, m3=100` | ambos | PWM-potencia | 165 | retroceder1() usado en DETECTA_LINEA_1: motor1 PWM 0 (INA=0,INB=1), motor2 PWM 100 (INA=0,INB=1), motor3 PWM 100 (INA=1,INB=0). Retrocede alejandose de linea detectada por sensor izquierdo. |
| `retroceder2(): PWM motores` | `m1=100, m2=0, m3=100` | ambos | PWM-potencia | 170 | retroceder2() usado en DETECTA_LINEA_2: motor1 PWM 100 (INA=1,INB=0), motor2 PWM 0 (INA=1,INB=0), motor3 PWM 100 (INA=0,INB=1). Retrocede de linea central. |
| `retroceder3(): PWM motores` | `m1=100, m2=100, m3=0` | ambos | PWM-potencia | 174 | retroceder3() usado en DETECTA_LINEA_3: motor1 PWM 100 (INA=0,INB=1), motor2 PWM 100 (INA=1,INB=0), motor3 PWM 0. Retrocede de linea detectada por sensor derecho. |
| `retroceder_patear(): PWM motor1` | `patadM1 = 250` | ambos | PWM-potencia | 205 | En retroceder_patear() (retorno del kicker) motor1 a PWM patadM1=250 (INA=0,INB=1). Mismo valor R1 y R2. |
| `retroceder_patear(): PWM motor2` | `patadM2 = 170` | ambos | PWM-potencia | 206 | En retroceder_patear() motor2 a PWM patadM2=170 (INA=1,INB=0). Mismo valor R1 y R2. |
| `retroceder_patear(): PWM motor3` | `0` | ambos | PWM-potencia | 207 | En retroceder_patear() motor3 a PWM 0 (apagado, INA=1,INB=0). |
| `aiproporcional() rama error>0` | `m1=pd*50, m2=pd*50, m3=pd*40` | ambos | PWM-potencia | 217 | Avance izquierdo, error>0 (desviado un lado): motor izq pd*50, motor der pd*50, motor atras pd*40. Comentarios //60,//60,//50. |
| `aiproporcional() rama error<0` | `m1=pd*40, m2=pd*65, m3=pd*100` | ambos | PWM-potencia | 225 | Avance izquierdo, error<0: motor izq(PWM2) pd*65, motor der(PWM1) pd*40, motor atras(PWM3) pd*100. Comentarios //75,//50,//120. Corrige asimetricamente para mantener heading. |
| `adproporcional() rama error>0` | `m1=pd*50, m2=pd*50, m3=pd*100` | ambos | PWM-potencia | 241 | Avance derecho, error>0: motor izq pd*50, motor der pd*50, motor atras pd*100. Comentarios //60,//60,//120. |
| `adproporcional() rama error<0` | `m1=pd*40, m2=pd*65, m3=pd*40` | ambos | PWM-potencia | 249 | Avance derecho, error<0: motor der(PWM1) pd*40, motor izq(PWM2) pd*65, motor atras(PWM3) pd*40. Comentarios //50,//75,//50. |
| `IMPULSO_INICIAL_GIRANDO PWM` | `150 (3 motores)` | ambos | PWM-potencia | 395 | Giro con mas potencia: los 3 motores a PWM 150 (INA=0,INB=1). Es un 'kick' de arranque del giro para vencer la inercia. Mayor que girar() (30 PWM). |
| `APUNTAR_PELOTA PWM` | `100 * a = 40` | ambos | PWM-potencia | 513 | Al apuntar la pelota los 3 motores giran a PWM 100*a=40. Sentido segun signo del anguloPelota: >0 antihorario (INA=0,INB=1), <0 horario (INA=1,INB=0). a=0.4 (linea 89). |
| `CENTRANDO_horario PWM m1,m2` | `60 * c = 24` | ambos | PWM-potencia | 615 | Orbitar pelota en horario: motores 1 y 2 a 60*c (c=0.4 -> 24 PWM), sentido (INA=0,INB=1). Linea 615-616. |
| `CENTRANDO_horario PWM m3` | `180 * c = 72` | ambos | PWM-potencia | 617 | En CENTRANDO_horario el motor 3 (trasero) a 180*c (c=0.4 -> 72 PWM), sentido (INA=1,INB=0). Mas potencia atras para que orbite. |
| `IMPULSO_CENTRANDO_antihorario PWM m1,m2` | `60 * ic` | ambos | PWM-potencia | 674 | Impulso de centrado antihorario: m1,m2 a 60*ic (R2: ic=0.55 -> 33 PWM; R1: ic=0.5 -> 30 PWM), sentido (INA=1,INB=0). |
| `IMPULSO_CENTRANDO_antihorario PWM m3` | `180 * ic` | ambos | PWM-potencia | 676 | m3 a 180*ic (R2: 99 PWM; R1: 90 PWM), sentido (INA=0,INB=1). |
| `CENTRANDO_antihorario PWM m1,m2` | `60 * c = 24` | ambos | PWM-potencia | 686 | Orbitar antihorario: m1,m2 a 60*c=24 PWM, sentido (INA=1,INB=0). |
| `CENTRANDO_antihorario PWM m3` | `180 * c = 72` | ambos | PWM-potencia | 688 | m3 a 180*c=72 PWM, sentido (INA=0,INB=1). |
| `IMPULSO_CENTRANDO_horario PWM m1,m2` | `60 * ic` | ambos | PWM-potencia | 745 | Impulso horario: m1,m2 a 60*ic (R2:33, R1:30), sentido (INA=0,INB=1). |
| `IMPULSO_CENTRANDO_horario PWM m3` | `180 * ic` | ambos | PWM-potencia | 747 | m3 a 180*ic (R2:99, R1:90), sentido (INA=1,INB=0). |
| `APUNTAR_PELOTA_antihorario PWM` | `100 * a = 40` | ambos | PWM-potencia | 762 | Re-apuntar pelota durante orbita antihoraria: 3 motores a 100*a=40, sentido segun signo anguloPelota. |
| `APUNTAR_PELOTA_horario PWM` | `100 * a = 40` | ambos | PWM-potencia | 807 | Re-apuntar durante orbita horaria: 3 motores a 100*a=40, sentido segun signo anguloPelota. |
| `ARQUERO impulso_inicial PWM m1,m2` | `1.8 * 50 = 90` | ambos | PWM-potencia | 1034 | Modo ARQUERO. impulso_inicial: motor2 y motor1 a 1.8*50=90 PWM (INA=1,INB=0). Impulso lateral inicial. NOTA: el factor 1.8 esta hardcodeado inline (no usa pd). |
| `ARQUERO impulso_inicial PWM m3` | `1.8 * 85 = 153` | ambos | PWM-potencia | 1036 | motor3 (trasero) a 1.8*85=153 PWM (INA=0,INB=1). |
| `ARQUERO pd cuando ve pelota` | `1.5` | ambos | PWM-potencia | 1063 | Cuando el arquero ve pelota desviada, sube pd a 1.5 (mas rapido para interceptar). Lineas 1063 y 1111. |
| `ARQUERO pd cuando no ve pelota` | `1` | ambos | PWM-potencia | 1081 | Cuando no ve pelota, pd vuelve a 1. Lineas 1081 y 1129. |
| `ROBOT2` | `#define activo (linea 11)` | na | otro | 11 | Selector de robot: ROBOT2 esta DEFINIDO (activo). ROBOT1 esta comenteado en linea 10. Por lo tanto el bloque #if defined(ROBOT2) (lineas 13-35) es el que compila. ROBOT1 (lineas 37-59) NO compila en este archivo tal como esta. |
| `ARCO_CONTRINCANTE` | `false (inicial)` | ambos | otro | 65 | Bool: arco al que hay que hacer gol. Inicializado en false pero REASIGNADO cada loop en linea 355 a hayarco_amarillo. Es decir: el arco rival es el AMARILLO (hardcodeado). |
| `Ycontrincante` | `0 (inicial)` | ambos | otro | 66 | int: coordenada Y del arco rival. Inicial 0 pero REASIGNADO cada loop en linea 356 a Yam (Y del arco amarillo). Usado en CENTRANDO para alinear el tiro. |
| `bno (Adafruit_BNO055)` | `id=55, addr=0x28` | ambos | otro | 76 | Instancia del giroscopo/IMU BNO055. ID sensor 55, direccion I2C 0x28. |
| `START_BYTE` | `0xAA` | ambos | otro | 84 | #define del byte de inicio de protocolo serial. NOTA: definido pero AMBIGUO/no usado en el codigo (el parseo usa headers 201/202/203, no 0xAA). Ademas el define lleva ';' al final (0xAA;) lo que es sospechoso. |
| `BAUD_RATE` | `19200` | ambos | otro | 85 | Velocidad serial (long const). Usado en Serial.begin y Serial1.begin (lineas 262-263). Serial1 es el enlace con la camara OpenMV. |
| `estado (inicial)` | `AVANCE_INICIO` | ambos | otro | 138 | Estado inicial de la maquina de estados al arrancar (linea 138). El flujo arranca por la rama DELANTERO (no arquero). |
| `ARCO_CONTRINCANTE = hayarco_amarillo` | `runtime` | ambos | otro | 355 | HARDCODEADO: el arco rival es el AMARILLO. Para cambiar de lado de cancha habria que editar esta linea (y la 356). Comentario linea 354: 'COLOCAR CUAL ES EL ARCO AL QUE AHI QUE HACER GOL'. |
| `INA1 (ROBOT2)` | `8` | R2 | pin | 14 | Pin direccion A motor 1 (motor derecho segun comentarios). |
| `INB1 (ROBOT2)` | `7` | R2 | pin | 15 | Pin direccion B motor 1. |
| `PWM1 (ROBOT2)` | `6` | R2 | pin | 16 | Pin PWM motor 1. |
| `INA2 (ROBOT2)` | `11` | R2 | pin | 18 | Pin direccion A motor 2 (motor izquierdo segun comentarios). |
| `INB2 (ROBOT2)` | `12` | R2 | pin | 19 | Pin direccion B motor 2. |
| `PWM2 (ROBOT2)` | `4` | R2 | pin | 20 | Pin PWM motor 2. |
| `INA3 (ROBOT2)` | `2` | R2 | pin | 22 | Pin direccion A motor 3 (motor atras/trasero segun comentarios). |
| `INB3 (ROBOT2)` | `5` | R2 | pin | 23 | Pin direccion B motor 3. |
| `PWM3 (ROBOT2)` | `3` | R2 | pin | 24 | Pin PWM motor 3. |
| `INA1 (ROBOT1)` | `2` | R1 | pin | 38 | Pin direccion A motor 1. NOTA: el mapeo de pines de ROBOT1 difiere de ROBOT2 (R1 usa 2/5/3 para motor1, R2 usa 8/7/6). Bloque NO compila (ROBOT1 comentado). |
| `INB1 (ROBOT1)` | `5` | R1 | pin | 39 | Pin direccion B motor 1. |
| `PWM1 (ROBOT1)` | `3` | R1 | pin | 40 | Pin PWM motor 1. |
| `INA2 (ROBOT1)` | `8` | R1 | pin | 42 | Pin direccion A motor 2. |
| `INB2 (ROBOT1)` | `7` | R1 | pin | 43 | Pin direccion B motor 2. |
| `PWM2 (ROBOT1)` | `6` | R1 | pin | 44 | Pin PWM motor 2. |
| `INA3 (ROBOT1)` | `11` | R1 | pin | 46 | Pin direccion A motor 3. |
| `INB3 (ROBOT1)` | `12` | R1 | pin | 47 | Pin direccion B motor 3. |
| `PWM3 (ROBOT1)` | `4` | R1 | pin | 48 | Pin PWM motor 3. |
| `LED_BUILTIN` | `= haypelota` | ambos | pin | 285 | El LED interno se enciende cuando haypelota es true (indicador visual de deteccion). pinMode OUTPUT en setup linea 268. |
| `tiempoAnteriorPateo` | `0 (inicial)` | ambos | tiempo | 72 | Marca de tiempo (millis) del ultimo incremento de la rampa del kicker. Usado para temporizar la rampa en avanzar_patear(). |
| `intervaloPateo` | `20` | ambos | tiempo | 73 | Milisegundos entre incrementos de la rampa del kicker (avanzar_patear, linea 185). Comentario: 'milisegundos entre incrementos'. |
| `AVANCE_INICIO duracion` | `700 ms` | ambos | tiempo | 385 | Estado inicial DELANTERO: ejecuta avanzar_patear() (rampa kicker + avance motores 1y2) durante 700ms, luego parar() y pasa a IMPULSO_INICIAL_GIRANDO. |
| `IMPULSO_INICIAL_GIRANDO duracion` | `70 ms` | ambos | tiempo | 399 | Tras 70ms de impulso de giro pasa a GIRANDO. |
| `IMPULSO_INICIAL_GIRANDO espera inercia (haypelota)` | `1000 ms` | ambos | tiempo | 408 | Si ve pelota: parar() y esperar 1000ms (1s) 'por la inercia' antes de pasar a APUNTAR_PELOTA. NOTA: condicion usa millis_inicio_estado que pudo NO haberse reseteado al entrar a este estado, comportamiento dependiente del timing previo. |
| `GIRANDO espera inercia (haypelota)` | `700 ms` | ambos | tiempo | 436 | En GIRANDO si ve pelota: parar() y esperar 700ms por inercia antes de APUNTAR_PELOTA; si no ve pelota sigue girar(). |
| `AVANZANDO_POR_TIEMPO duracion` | `500 ms` | ambos | tiempo | 476 | Avanza recto (avanzar()) 500ms y vuelve a IMPULSO_INICIAL_GIRANDO. Salida temprana a APUNTAR_PELOTA si ve pelota (linea 483). |
| `IMPULSO_CENTRANDO_antihorario duracion` | `500 ms` | ambos | tiempo | 678 | Tras 500ms de impulso antihorario -> CENTRANDO_antihorario. |
| `IMPULSO_CENTRANDO_horario duracion` | `300 ms` | ambos | tiempo | 749 | Tras 300ms -> CENTRANDO_horario. NOTA: difiere del impulso antihorario que dura 500ms (asimetria de tuneo). |
| `PATEANDO_corto_pausa_inicial duracion` | `500 ms` | ambos | tiempo | 849 | PATADA CORTA: parar() 500ms -> PATEANDO_corto_adelante. |
| `PATEANDO_corto_adelante duracion` | `200 ms` | ambos | tiempo | 859 | avanzar_patear() (rampa kicker) durante 200ms -> parar() -> PATEANDO_corto_pausa. La corta avanza 200ms (vs 500ms la larga). |
| `PATEANDO_corto_pausa duracion` | `500 ms` | ambos | tiempo | 870 | parar() 500ms -> PATEANDO_corto_atras. |
| `PATEANDO_corto_atras duracion` | `300 ms` | ambos | tiempo | 880 | retroceder_patear() (retorno kicker, m1=250 m2=170) 300ms -> parar() -> IMPULSO_INICIAL_GIRANDO. |
| `PATEANDO_pausa_inicial duracion` | `1000 ms` | ambos | tiempo | 893 | PATADA LARGA: parar() 1000ms (1s) -> PATEANDO_adelante. Tiene salidas por linea (DETECTA_LINEA_1/2/3) en lineas 899-913. |
| `PATEANDO_adelante duracion` | `500 ms` | ambos | tiempo | 918 | avanzar_patear() (rampa kicker hasta 240) durante 500ms -> parar() -> PATEANDO_pausa. La larga avanza 500ms. Tiene salidas por linea (lineas 925-939). |
| `PATEANDO_pausa duracion` | `500 ms` | ambos | tiempo | 944 | parar() 500ms -> PATEANDO_atras. Salidas por linea (lineas 950-964). |
| `PATEANDO_atras duracion` | `200 ms` | ambos | tiempo | 969 | retroceder_patear() (retorno kicker) 200ms -> parar() -> IMPULSO_INICIAL_GIRANDO. La larga retrocede 200ms (vs 300ms la corta). Salidas por linea (lineas 976-990). |
| `DETECTA_LINEA_1 duracion retroceso` | `400 ms` | ambos | tiempo | 997 | retroceder1() durante 400ms -> parar() -> IMPULSO_INICIAL_GIRANDO. Reaccion a linea izquierda. |
| `DETECTA_LINEA_2 duracion retroceso` | `400 ms` | ambos | tiempo | 1009 | retroceder2() durante 400ms -> parar() -> IMPULSO_INICIAL_GIRANDO. Reaccion a linea central. |
| `DETECTA_LINEA_3 duracion retroceso` | `400 ms` | ambos | tiempo | 1021 | retroceder3() durante 400ms -> parar() -> IMPULSO_INICIAL_GIRANDO. Reaccion a linea derecha. |
| `ARQUERO impulso_inicial duracion` | `40 ms` | ambos | tiempo | 1038 | Tras 40ms -> moverce_derecha. |
| `ARQUERO impulso_derecha duracion` | `350 ms` | ambos | tiempo | 1147 | adproporcional() durante 350ms -> moverce_derecha. Impulso de rebote hacia la derecha. |
| `ARQUERO impulso_izquierda duracion` | `350 ms` | ambos | tiempo | 1159 | aiproporcional() durante 350ms -> moverce_izquierda. Impulso de rebote hacia la izquierda. |
| `ARQUERO PATEANDO_pausa_inicial_arquero duracion` | `500 ms` | ambos | tiempo | 1171 | Patada arquero: parar() 500ms -> PATEANDO_adelante_arquero. |
| `ARQUERO PATEANDO_adelante_arquero duracion` | `300 ms` | ambos | tiempo | 1182 | avanzar_patear() (rampa kicker) 300ms -> parar() -> PATEANDO_pausa_arquero. |
| `ARQUERO PATEANDO_pausa_arquero duracion` | `1000 ms` | ambos | tiempo | 1193 | parar() 1000ms (1s) -> PATEANDO_atras_arquero. |
| `GIRANDO timeout -> avanzar` | `9000 ms y abs(error)<=50` | ambos | timeout | 448 | Si pasaron >=9s girando Y el heading esta dentro de +-50 grados -> pasa a AVANZANDO_POR_TIEMPO. Comentario menciona '+-70' pero el codigo usa 50 (DISCREPANCIA comentario vs codigo). |
| `APUNTAR_PELOTA perdida pelota` | `500 ms` | ambos | timeout | 531 | Si pasaron >=500ms desde la ultima vez que vio la pelota (millis_pelota) -> vuelve a IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA timeout` | `10000 ms` | ambos | timeout | 537 | Timeout de 10s en APUNTAR_PELOTA -> IMPULSO_INICIAL_GIRANDO. |
| `AVANZANDO perdida pelota` | `500 ms` | ambos | timeout | 582 | Si pasaron >=500ms sin ver pelota -> IMPULSO_INICIAL_GIRANDO. |
| `AVANZANDO timeout` | `20000 ms` | ambos | timeout | 589 | Timeout de 20s en AVANZANDO -> IMPULSO_INICIAL_GIRANDO. |
| `CENTRANDO patear por tiempo` | `4000 ms y abs(error)<=1` | ambos | timeout | 628 | Si lleva >=4s centrando Y heading dentro de +-1 grado -> patea igual (PATEANDO_pausa_inicial). Resetea millis_inicio_centrando. |
| `CENTRANDO_horario perdida pelota` | `3000 ms` | ambos | timeout | 643 | Si pasaron >=3s sin ver pelota -> IMPULSO_INICIAL_GIRANDO. |
| `CENTRANDO_horario timeout` | `25000 ms` | ambos | timeout | 650 | Timeout de 25s centrando -> IMPULSO_INICIAL_GIRANDO. Marcado con comentario STOP. |
| `CENTRANDO_antihorario patear por tiempo` | `4000 ms y abs(error)<=1` | ambos | timeout | 699 | Si >=4s centrando y heading +-1 grado -> PATEANDO_pausa_inicial. Comentario: 'patee al lado opuesto del contrincante'. |
| `CENTRANDO_antihorario perdida pelota` | `3000 ms` | ambos | timeout | 714 | 3s sin ver pelota -> IMPULSO_INICIAL_GIRANDO. |
| `CENTRANDO_antihorario timeout` | `25000 ms` | ambos | timeout | 721 | Timeout 25s -> IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA_antihorario perdida pelota` | `500 ms` | ambos | timeout | 786 | 500ms sin pelota -> IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA_antihorario timeout` | `10000 ms (millis_inicio_centrando)` | ambos | timeout | 792 | Timeout 10s pero medido contra millis_inicio_centrando (NO millis_inicio_estado). Marcado STOP. -> IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA_horario perdida pelota` | `500 ms` | ambos | timeout | 831 | 500ms sin pelota -> IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA_horario timeout` | `10000 ms (millis_inicio_estado)` | ambos | timeout | 837 | Timeout 10s medido contra millis_inicio_estado (a diferencia del antihorario que usa millis_inicio_centrando). Asimetria. -> IMPULSO_INICIAL_GIRANDO. |
| `blanco1 (ROBOT2)` | `650` | R2 | umbral | 26 | Umbral del sensor de linea 1 (izquierdo). Si readLine(1) >= 650 se considera que detecto linea blanca. |
| `blanco2 (ROBOT2)` | `650` | R2 | umbral | 27 | Umbral del sensor de linea 2 (centro). readLine(2) >= 650 -> linea blanca. |
| `blanco3 (ROBOT2)` | `750` | R2 | umbral | 28 | Umbral del sensor de linea 3 (derecho). readLine(3) >= 750 -> linea blanca. Notar que es mas alto que los otros dos (650). |
| `blanco1 (ROBOT1)` | `600` | R1 | umbral | 50 | Umbral sensor de linea 1 (izquierdo) para ROBOT1. Mas bajo que ROBOT2 (650). |
| `blanco2 (ROBOT1)` | `600` | R1 | umbral | 51 | Umbral sensor de linea 2 (centro) para ROBOT1. |
| `blanco3 (ROBOT1)` | `600` | R1 | umbral | 52 | Umbral sensor de linea 3 (derecho) para ROBOT1. Los 3 umbrales iguales (600), a diferencia de ROBOT2 donde blanco3=750. |
| `tolerancia_centrado` | `30.0` | ambos | umbral | 118 | Tolerancia (en unidades de Y) para considerar que la pelota esta alineada con el arco rival al centrar: abs(Yp - Ycontrincante) <= 30 -> pasa a patear (lineas 621,692). |
| `tolerancia_cercania` | `50.0` | ambos | umbral | 119 | Tolerancia (en unidades de Xp) para considerar la pelota 'cerca'. AVANZANDO->CENTRANDO si Xp <= 50 (linea 567). Tambien usada en arquero (Xp <= 50, lineas 1054,1102) y en APUNTAR_PELOTA_* (Xp >= 50, lineas 780,825). |
| `tolerancia_apuntado` | `15.0` | ambos | umbral | 120 | Tolerancia ANGULAR (grados) para considerar la pelota apuntada: abs(anguloPelota) >= 15 -> hay que seguir apuntando; < 15 -> apuntada. Usada en APUNTAR_PELOTA, AVANZANDO, CENTRANDO_*, APUNTAR_PELOTA_*. |
| `aiproporcional() banda muerta error` | `-2 < error < 2` | ambos | umbral | 212 | Avance izquierdo proporcional. Si el error de heading esta entre -2 y +2 (zona casi recta): m1=pd*50, m2=pd*50, m3=pd*89. Comentarios indican valores objetivo //60,//60,//99. |
| `adproporcional() banda muerta error` | `-2 < error < 2` | ambos | umbral | 236 | Avance derecho proporcional. error entre -2 y 2: m1=pd*50, m2=pd*50, m3=pd*89. Comentarios //60,//60,//99. |
| `correccion heading clamp` | `+-180 wrap` | ambos | umbral | 363 | Normalizacion ANGULAR del error de heading: si error>180 -> error-=360; si error<-180 -> error+=360 (lineas 363-364). Mantiene error en rango (-180,180]. |
| `APUNTAR_PELOTA salida apuntada` | `abs(anguloPelota) < 15` | ambos | umbral | 508 | Si abs(anguloPelota) < tolerancia_apuntado(15 grados) -> pelota apuntada -> pasa a AVANZANDO. |
| `AVANZANDO->CENTRANDO cercania` | `Xp <= 50` | ambos | umbral | 567 | En AVANZANDO, si ve pelota y Xp <= tolerancia_cercania(50) -> resetea millis_inicio_centrando y pasa a CENTRANDO_horario. |
| `AVANZANDO re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 575 | En AVANZANDO si abs(anguloPelota) >= tolerancia_apuntado(15 grados) vuelve a APUNTAR_PELOTA. |
| `CENTRANDO alineado arco` | `abs(Yp - Ycontrincante) <= 30` | ambos | umbral | 621 | Si ve arco rival, ve pelota y abs(Yp - Ycontrincante) <= tolerancia_centrado(30) -> pasa a PATEANDO_pausa_inicial (alineado para gol). |
| `CENTRANDO re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 636 | En CENTRANDO_horario si pierde el apuntado (abs(anguloPelota)>=15 grados) -> APUNTAR_PELOTA_horario. |
| `CENTRANDO_horario salida linea: umbral error` | `abs(error) <= 80` | ambos | umbral | 659 | Si detecta linea blanca (s1/s2/s3) Y heading dentro de +-80 grados -> patada corta (PATEANDO_corto_pausa_inicial). Si abs(error)>80 -> IMPULSO_CENTRANDO_antihorario (orbita al otro lado). Comentario: 'tal vez bajar o subir despues'. |
| `CENTRANDO_antihorario alineado arco` | `abs(Yp - Ycontrincante) <= 30` | ambos | umbral | 692 | Mismo criterio que horario: alineado -> PATEANDO_pausa_inicial. |
| `CENTRANDO_antihorario re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 707 | Pierde apuntado (>=15 grados) -> APUNTAR_PELOTA_antihorario. |
| `CENTRANDO_antihorario salida linea: umbral error` | `abs(error) <= 80` | ambos | umbral | 730 | Linea detectada y heading +-80 grados -> PATEANDO_corto_pausa_inicial; si abs(error)>80 -> IMPULSO_CENTRANDO_horario (orbita al otro lado). |
| `APUNTAR_PELOTA_antihorario salida apuntada` | `abs(anguloPelota) < 15` | ambos | umbral | 757 | Si apuntada (abs<15 grados) vuelve a CENTRANDO_antihorario. |
| `APUNTAR_PELOTA_antihorario cercania` | `Xp >= 50` | ambos | umbral | 780 | Si ve pelota y Xp >= tolerancia_cercania(50) (se alejo) -> APUNTAR_PELOTA (re-aproximar). Marcado STOP. |
| `APUNTAR_PELOTA_horario salida apuntada` | `abs(anguloPelota) < 15` | ambos | umbral | 802 | Si apuntada (<15 grados) vuelve a CENTRANDO_horario. |
| `APUNTAR_PELOTA_horario cercania` | `Xp >= 50` | ambos | umbral | 825 | Si Xp >= 50 -> APUNTAR_PELOTA. Marcado STOP. |
| `ARQUERO moverce_derecha cercania+alineado` | `Xp <= 50 && abs(Yp) <= 5` | ambos | umbral | 1054 | En moverce_derecha (usa adproporcional): si ve pelota cerca (Xp<=tolerancia_cercania 50) y centrada (abs(Yp)<=5) -> parar() y PATEANDO_pausa_inicial_arquero (patea). |
| `ARQUERO banda muerta Yp` | `abs(Yp) >= 5` | ambos | umbral | 1061 | Si la pelota esta lateralmente desviada (abs(Yp)>=5): pd=1.5 y elige direccion: Yp<0 -> moverce_derecha, Yp>=0 -> moverce_izquierda. Si abs(Yp)<5 -> parar(). Threshold de centrado del arquero = 5. |
| `ARQUERO moverce_derecha salida linea` | `s1>=blanco1 or s2>=blanco2` | ambos | umbral | 1086 | Si sensor izq o centro ve linea -> parar() y impulso_izquierda (rebota hacia el otro lado del arco). Solo chequea s1 y s2 (no s3). |
| `ARQUERO moverce_izquierda cercania+alineado` | `Xp <= 50 && abs(Yp) <= 5` | ambos | umbral | 1102 | En moverce_izquierda (usa aiproporcional): si pelota cerca y centrada -> PATEANDO_pausa_inicial_arquero. |
| `ARQUERO moverce_izquierda salida linea` | `s1>=blanco1 or s2>=blanco2` | ambos | umbral | 1134 | Si linea izq o centro -> parar() y impulso_derecha. Solo s1 y s2. |
| `ARQUERO PATEANDO_atras_arquero salida` | `s1/s2/s3 >= blanco` | ambos | umbral | 1204 | retroceder_patear() hasta que CUALQUIER sensor (s1,s2 o s3) detecte linea -> parar() y moverce_derecha. NOTA: este estado NO tiene timeout por tiempo, solo sale por deteccion de linea. Si nunca ve linea queda retrocediendo indefinidamente. |
| `Serial1 read threshold` | `>= 9 bytes` | ambos | vision | 287 | El loop lee el paquete de la camara OpenMV cuando hay al menos 9 bytes disponibles en Serial1 (paquete de 9 bytes: header1, Xp, Yp, header2, Xam, Yam, header3, Xaz, Yaz). |
| `header1 esperado` | `201` | ambos | vision | 290 | Primer byte de cabecera del paquete de vision. Si header1 != 201 -> se descarta el frame y se ponen todas las presencias en false (lineas 347-352). |
| `header2 esperado` | `202` | ambos | vision | 301 | Segundo byte de cabecera (entre datos de pelota y arco amarillo). Se valida header1==201 && header2==202 && header3==203 para aceptar el frame. |
| `header3 esperado` | `203` | ambos | vision | 301 | Tercer byte de cabecera (entre arco amarillo y arco azul). Validacion conjunta en linea 301. |
| `offset decodificacion Y` | `-100` | ambos | vision | 304 | Las coordenadas Y se decodifican restando 100 al byte recibido: Yp=codedYp-100, Yam=codedYam-100, Yaz=codedYaz-100 (lineas 304,306,308). Permite Y negativos en un byte 0..255 (rango -100..+155). Las X NO llevan offset (Xp=codedXp, etc.). |
| `calculo anguloPelota` | `atan2(Yp,Xp)*180/PI` | ambos | vision | 311 | Angulo de la pelota en grados respecto al frente del robot. Igual para arco amarillo (linea 312) y arco azul (linea 313). |
| `deteccion presencia (Xp==0)` | `Xp==0 -> no hay` | ambos | vision | 324 | Si Xp==0 haypelota=false; si no, haypelota=true y se actualiza millis_pelota (timestamp de ultima vez que vio la pelota). Mismo criterio para arco amarillo (Xam==0) y azul (Xaz==0), lineas 332-340. |
| `readLine(1/2/3)` | `sensores 1,2,3` | ambos | vision | 369 | Lectura de los 3 sensores de linea: s1=readLine(1) izquierdo, s2=readLine(2) centro, s3=readLine(3) derecho (lineas 369-371). Se comparan contra blanco1/2/3. |

### Notas de constantes

CONSTANTES AMBIGUAS / CALCULADAS EN RUNTIME / DISCREPANCIAS:

1. SELECTOR DE ROBOT: en este archivo ROBOT2 esta DEFINIDO (linea 11) y ROBOT1 COMENTADO (linea 10). Por lo tanto SOLO el bloque ROBOT2 (lineas 13-35) compila. Documentamos igual los valores de ROBOT1 (lineas 37-59) por pedido. UNICA diferencia de tuneo escalar entre robots: ic (R1=0.5 / R2=0.55) y blanco3 (R1=600 / R2=750). Ademas el MAPEO DE PINES de motores es DISTINTO entre robots (R1 motor1=2/5/3 vs R2 motor1=8/7/6; R1 motor2=8/7/6 vs R2 motor2=11/12/4; R1 motor3=11/12/4 vs R2 motor3=2/5/3) — los 3 motores estan rotados entre placas. c, patadM1, patadM2 son iguales en ambos.

2. 'correccion' (linea 365 = error*kp, kp=0.3): se CALCULA cada loop pero AMBIGUO/no se aplica directamente a ningun analogWrite en el switch. La correccion real de heading se hace via las ramas error>0 / error<0 / banda(-2,2) dentro de ai/adproporcional. La variable 'correccion' parece codigo muerto / residual.

3. START_BYTE 0xAA (linea 84): definido pero NO usado (el parseo serial usa headers 201/202/203). Ademas el #define termina en ';' (#define START_BYTE 0xAA;) lo cual es un bug latente si se llegara a usar.

4. pd (factor avance): valor inicial 1 (linea 90) pero MUTABLE en runtime SOLO en modo arquero: pd=1.5 (ve pelota desviada, lineas 1063/1111) y pd=1 (no ve pelota, lineas 1081/1129). En modo delantero pd nunca cambia, queda en 1. Por eso los PWM de ai/adproporcional (pd*50, pd*89, etc.) en delantero dan los valores nominales; en arquero pueden ir x1.5.

5. ARCO_CONTRINCANTE y Ycontrincante: inicializados (false / 0) pero REASIGNADOS cada loop (lineas 355-356) a hayarco_amarillo y Yam. El arco rival esta HARDCODEADO al AMARILLO. Para cambiar de lado de cancha hay que editar esas 2 lineas.

6. DISCREPANCIA comentario vs codigo en GIRANDO timeout (linea 447-448): el comentario dice 'rango de +-70' pero el codigo usa abs(error)<=50.

7. Comentarios //60 //99 //75 //120 etc. en ai/adproporcional (lineas 213-255): son valores objetivo/anteriores documentados al lado del valor actual (que es menor). El valor EN USO es el numerico (pd*50, pd*89...), el del comentario NO se aplica.

8. ASIMETRIAS de tuneo notables: IMPULSO_CENTRANDO_antihorario dura 500ms (linea 678) vs IMPULSO_CENTRANDO_horario 300ms (linea 749). APUNTAR_PELOTA_antihorario timeout mide contra millis_inicio_centrando (linea 792) vs APUNTAR_PELOTA_horario contra millis_inicio_estado (linea 837). Estas asimetrias parecen producto de tuneo manual, conviene verificarlas en banco.

9. velocidadActualPateo (rampa kicker): NO se resetea a 0 entre patadas dentro del codigo mostrado (arranca en 0 al boot y sube a 240; tras la primera patada queda en 240 y las siguientes ya arrancan al maximo). Posible bug latente del kicker (la rampa solo es 'suave' la primera vez). VERIFICAR en hardware.

10. PATEANDO_atras_arquero (linea 1201): sin timeout temporal, solo sale por deteccion de linea (s1/s2/s3). Riesgo de quedar retrocediendo si no encuentra linea.

11. El factor 1.8 del impulso_inicial del arquero (lineas 1034-1036) esta hardcodeado inline, no es una constante nombrada.

12. Magnitudes PWM reales calculadas (asumiendo factores compilados de ROBOT2): girar=30; APUNTAR=40; CENTRANDO m1/m2=24, m3=72; IMPULSO_CENTRANDO m1/m2=33(R2)/30(R1), m3=99(R2)/90(R1); avanzar m1/m2=100; impulso giro inicial=150; kicker rampa hasta 240; retorno kicker m1=250/m2=170; arquero impulso m1/m2=90, m3=153.

13. NOTA de categorizacion: varios umbrales son ANGULARES en grados (tolerancia_apuntado=15, error<=50/<=80/<=1, wrap +-180) — se etiquetaron como 'umbral' porque el schema no tiene categoria 'angulo'; el significado de cada uno aclara que es un angulo en grados.
