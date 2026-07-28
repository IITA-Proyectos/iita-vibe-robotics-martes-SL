---
title: "Análisis FIEL — Arquero 2025 (definitivo-arquero)"
date: 2026-06-18
status: referencia-fiel
tipo: analisis-codigo-historico
fuente: "futbol-roboliga2026/robots-2025/arquero/arquero-2025.ino (1207 líneas)"
fuente-original: "repo open-soccer-robocup-team2026 → software/_deprecated-2025/robot-arquero/definitivo-arquero_6-9-2026"
copiado-a-este-repo: 2026-07-28
generado-por: "Claude (workflow 14 agentes, análisis línea por línea + crítico de completitud)"
---

> **TRANSCRIPCIÓN FIEL del código del Nacional 2025 (Buenos Aires, campeones).** Cada valor citado con `archivo:línea`.
> Objetivo: reconstruir el comportamiento EXACTO en el robot 2026 (más sensores), sin perder nada.
> ⚠️ Los DOS programas (`definitivo-arquero` y `definitivo-delantero`) son el MISMO firmware unificado arquero+delantero; cambian el `#define` de robot (arquero=ROBOT1, delantero=ROBOT2) y el estado inicial que selecciona el modo.
> ⚠️ Las mejoras con sensores nuevos van en una sección APARTE y marcada — NO contaminan la transcripción.
> Completitud (crítico adversarial que re-leyó el programa): **cobertura_ok = TRUE (todo lo material; faltantes solo debug/comentado)**.

# DOCUMENTO FIEL — ARQUERO 2025 (`definitivo-arquero_6-9-2026`)

**Archivo:** [`arquero-2025.ino`](arquero-2025.ino) (1207 líneas, Arduino/C++). Los números de
línea de este documento coinciden **exactamente** con ese archivo — el contenido es idéntico al
original, solo cambió el nombre (antes no tenía extensión).
**Contexto:** Nacional 2025 (Buenos Aires, campeones). Robot mono-placa, librería `zirconLib`, BNO055 (giroscopio), cámara OpenMV v1 (paquete de 9 bytes), kicker por motores (no hay solenoide: la "patada" es un avance fuerte de las ruedas).
**Build activo:** `#define ROBOT1` (L10); `//#define ROBOT2` comentado (L11). Se documentan AMBOS robots.
**Particularidad estructural:** el archivo es un firmware UNIFICADO que contiene la FSM del ARQUERO y la del DELANTERO en el mismo `switch`. El estado inicial es `impulso_inicial` (L131), que es ARQUERO, de modo que **este robot arranca y vive como ARQUERO**. Toda esta documentación es 100% fiel al código verificado línea por línea; cada afirmación cita su línea.

---

## 1. Comportamiento en cancha (qué hace el robot realmente)

El arquero **patrulla lateralmente** (strafe izquierda/derecha) sobre la línea de su arco, siguiendo a la pelota en el eje Y, y cuando la pelota le llega cerca y centrada, ejecuta una **secuencia de despeje** (pausa → patada hacia adelante → pausa larga → retroceso hasta la línea → reposicionamiento) y vuelve a patrullar. Concretamente, lo que el código ejecuta:

1. **Arranque (`impulso_inicial`, L1016):** al encender, da un impulso lateral fuerte de **40 ms** (PWM 90/90/153) para despegarse y entra a patrullar por la derecha (`moverce_derecha`, L1024).

2. **Patrulla (`moverce_derecha` / `moverce_izquierda`, L1030 / L1078):** se desplaza lateralmente usando `adproporcional()` / `aiproporcional()`. Estas funciones **sí usan el giroscopio** (variable `error`, L188-232) para repartir el PWM de las tres ruedas y mantener el robot derecho mientras hace strafe (corrección de rumbo). Mientras patrulla decide a qué lado ir según el **signo de Yp** de la pelota (ver paso 4).

3. **Rebote en el borde:** si un sensor de línea ve blanco (`s1>=blanco1` o `s2>=blanco2`, L1070 / L1117), para y entra a un **impulso temporizado de 350 ms** hacia el lado opuesto (`impulso_izquierda` / `impulso_derecha`, L1073 / L1120) para no quedarse trabado oscilando en el borde (comentario explícito L1127, L1139).

4. **Decisión por la pelota (dentro de cada `moverce_*`, L1036-1062 / L1084-1110):**
   - Si **no hay pelota** → `pd = 1` (patrulla a velocidad base, L1065 / L1113).
   - Si **hay pelota** y está **cerca y centrada** (`Xp <= 140` Y `abs(Yp) <= 3`) → para e inicia la patada (`PATEANDO_pausa_inicial_arquero`, L1041 / L1089).
   - Si **hay pelota desviada** (`abs(Yp) >= 5`) → `pd = 1.5` (corrige más fuerte) y elige lado: `Yp < 0` → derecha; `Yp >= 0` → izquierda (L1049-1058 / L1097-1106).
   - Si la pelota está en la **banda muerta** `3 < abs(Yp) < 5` (y no cumple cercanía) → `parar()` (L1060 / L1108). El robot se queda quieto en ese rango.

5. **Despeje (secuencia de patada, L1151-1205):**
   - `PATEANDO_pausa_inicial_arquero` (L1151): para **200 ms** (deja pasar la inercia lateral antes de patear hacia adelante; comentario L1152).
   - `PATEANDO_adelante_arquero` (L1162): `avanzar_patear()` durante **450 ms** (golpe de avance fuerte, M1=250 / M2=150[R1], M3=0).
   - `PATEANDO_pausa_arquero` (L1174): para **1000 ms**.
   - `PATEANDO_atras_arquero` (L1184): retrocede recto (M1=150, M2=150, M3=0) **sin límite de tiempo**, hasta que cualquier sensor vea blanco (vuelve a su línea de fondo).
   - `avanzar_despues_de_patear` (L1197): avanza **1000 ms** para reposicionarse y vuelve a `moverce_derecha`.

6. **Falla de hardware:** si el BNO055 no inicializa en el arranque, el firmware queda colgado en `while(1)` (L246-249) — el robot no hace nada.

**Lo que el arquero NO hace (verificado):** no usa el color del arco para patrullar (decide por signo de Yp y por línea blanca); no aplica un PID cerrado al heading (la `correccion=error*0.3` de L340 se calcula pero nunca se escribe a motores — el único uso del giroscopio es vía `error` dentro de las funciones proporcionales del strafe); las patadas del arquero (`PATEANDO_*_arquero`) **no chequean línea** durante el golpe (a diferencia de las del delantero). El gran bloque DELANTERO del `switch` (GIRANDO, APUNTAR_PELOTA, AVANZANDO, CENTRANDO_*, PATEANDO largo/corto) **no se alcanza** desde el ciclo del arquero: es código co-residente pero muerto para este robot.

---

## 2. Máquina de estados del ARQUERO (estados, entradas/salidas, tiempos exactos)

> Estados declarados en el enum, sección ARQUERO: L116-119. Estado inicial: `estado = impulso_inicial` (L131).
> Todos los tiempos abajo son los literales del código. `millis_inicio_estado` se resetea con `millis()` en cada transición.

### `impulso_inicial` (L1016-1028) — estado INICIAL
- **Acción de motores (L1018-1020):** M2 dir(INA2=1,INB2=0) PWM=`1.8*50`=**90**; M1 dir(INA1=1,INB1=0) PWM=`1.8*50`=**90**; M3 dir(INA3=0,INB3=1) PWM=`1.8*85`=**153**. → impulso lateral fuerte (strafe derecha).
- **Salida (L1022):** tras **40 ms** → `moverce_derecha`.
- Números mágicos inline: factor **1.8**, bases **50** y **85** (no son constantes nombradas).

### `moverce_derecha` (L1030-1076)
- **Acción (L1031):** `adproporcional()` (strafe a la derecha, con corrección de rumbo por `error`).
- **Salidas, en orden de evaluación:**
  - `if (haypelota)` (L1036):
    - **L1038** `if (Xp <= tolerancia_cercania[140] && abs(Yp) <= 3)` → `parar()` + `PATEANDO_pausa_inicial_arquero` (L1040-1042). **PATEA.**
    - **L1045** `if (abs(Yp) >= 5)` → `pd = 1.5` (L1047); si `Yp<0` → `moverce_derecha` (L1051), si `Yp>=0` → `moverce_izquierda` (L1056).
    - **L1060** `else { parar(); }` (banda muerta `3 < abs(Yp) < 5`).
  - `else { pd = 1; }` (L1063-1066): sin pelota.
  - **L1070** `if (s1 >= blanco1 or s2 >= blanco2)` → `parar()` + `impulso_izquierda` (L1072-1074). (Nota: NO chequea `s3` aquí.)

### `moverce_izquierda` (L1078-1124) — espejo del anterior
- **Acción (L1079):** `aiproporcional()` (strafe a la izquierda con corrección por `error`).
- **Salidas:** idénticas a `moverce_derecha` salvo:
  - Cerca+centrada (L1086) → `PATEANDO_pausa_inicial_arquero`.
  - Desviada `abs(Yp)>=5` (L1093): `pd=1.5`; `Yp<0`→`moverce_derecha` (L1099), `Yp>=0`→`moverce_izquierda` (L1104).
  - Banda muerta → `parar()` (L1108).
  - Sin pelota → `pd=1` (L1113).
  - **L1117** `if (s1>=blanco1 or s2>=blanco2)` → `parar()` + `impulso_derecha` (L1120). (Opuesto al de `moverce_derecha`, que iba a `impulso_izquierda`.)

### `impulso_derecha` (L1126-1136) — anti-traba en el borde
- **Acción (L1128):** `adproporcional()` (empuja a la derecha).
- **Salida (L1130):** tras **350 ms** → `moverce_derecha`.
- Comentario L1127: existe porque en los costados se traba con el blanco cambiando erráticamente de izquierda a derecha.

### `impulso_izquierda` (L1138-1148) — espejo
- **Acción (L1140):** `aiproporcional()` (empuja a la izquierda).
- **Salida (L1142):** tras **350 ms** → `moverce_izquierda`.

### `PATEANDO_pausa_inicial_arquero` (L1151-1160)
- **Acción (L1153):** `parar()`.
- **Salida (L1154):** tras **200 ms** → `PATEANDO_adelante_arquero`. (Comentario L1152: esperar por la inercia antes de pasar de movimiento horizontal a vertical.)

### `PATEANDO_adelante_arquero` (L1162-1172)
- **Acción (L1164):** `avanzar_patear()` (M1=patadM1=250, M2=patadM2[R1=150/R2=200], M3=0).
- **Salida (L1165):** tras **450 ms** → `parar()` + `PATEANDO_pausa_arquero`.
- **NO chequea sensores de línea durante el golpe.**

### `PATEANDO_pausa_arquero` (L1174-1182)
- **Acción (L1176):** `parar()`.
- **Salida (L1177):** tras **1000 ms** → `PATEANDO_atras_arquero`.

### `PATEANDO_atras_arquero` (L1184-1195)
- **Acción de motores INLINE (L1186-1188):** M1 PWM=**150** dir(0,1); M2 PWM=**150** dir(1,0); M3 PWM=**0** dir(1,0). → retroceso recto. (No usa `retroceder_patear()` ni `retroceder*()`.)
- **Salida (L1190):** cuando `s1>=blanco1` o `s2>=blanco2` o `s3>=blanco3` → `avanzar_despues_de_patear`. **SIN timeout** — único estado del arquero sin salida por tiempo; si nunca ve blanco, se queda trabado aquí.

### `avanzar_despues_de_patear` (L1197-1205)
- **Acción (L1199):** `avanzar()` (M1=100, M2=100, M3=0).
- **Salida (L1200):** tras **1000 ms** → `moverce_derecha` (cierra el ciclo).

### Diagrama de flujo (solo transiciones reales del arquero)

```
impulso_inicial (40 ms, strafe der.)
        │
        ▼
moverce_derecha ◄──────────────► moverce_izquierda
   │   │   │                         │   │   │
   │   │   └ blanco(s1|s2) → impulso_izquierda (350 ms) ─┐
   │   │                                                  │
   │   │      blanco(s1|s2) → impulso_derecha (350 ms) ───┘
   │   │                       (cada impulso vuelve a su moverce)
   │   │
   │   └ pelota desviada (abs(Yp)>=5, pd=1.5): Yp<0→der, Yp>=0→izq
   │
   └ pelota cerca+centrada (Xp<=140 && abs(Yp)<=3)
              │
              ▼
   PATEANDO_pausa_inicial_arquero (200 ms)
              ▼
   PATEANDO_adelante_arquero (450 ms, avanzar_patear)
              ▼
   PATEANDO_pausa_arquero (1000 ms)
              ▼
   PATEANDO_atras_arquero (150/150/0, hasta ver blanco — SIN timeout)
              ▼
   avanzar_despues_de_patear (1000 ms, avanzar)
              ▼
        moverce_derecha   (retoma patrulla)
```

> **Estados DELANTERO co-residentes pero inalcanzables desde el ciclo arquero** (declarados L122-129, sus `case` existen pero el flujo del arquero nunca entra): `AVANCE_INICIO`, `IMPULSO_INICIAL_GIRANDO`, `GIRANDO`, `APUNTAR_PELOTA(_antihorario)`, `AVANZANDO`, `AVANZANDO_POR_TIEMPO`, `CENTRANDO_horario/antihorario`, `IMPULSO_CENTRANDO_antihorario/horario`, `PATEANDO_corto_*`, `PATEANDO_pausa_inicial/adelante/pausa/atras`, `DETECTA_LINEA_1/2/3`. (Los `DETECTA_LINEA_*` SÍ se pueden alcanzar, pero solo desde estados delantero, no desde el arquero.) Estados huérfanos sin `case` en el switch: `PRIMER_IMPULSO_INICIAL_GIRANDO` (L122), `CENTRANDO_giroscopo` (L125).

---

## 3. Tabla de CONSTANTES EXACTAS por categoría (ROBOT1 vs ROBOT2)

### 3.1 Pines (categoría PIN)

| Símbolo | ROBOT1 (activo) | línea R1 | ROBOT2 | línea R2 |
|---|---|---|---|---|
| INA1 / INB1 / PWM1 | 2 / 5 / 3 | L38-40 | 8 / 7 / 6 | L14-16 |
| INA2 / INB2 / PWM2 | 8 / 7 / 6 | L42-44 | 11 / 12 / 4 | L18-20 |
| INA3 / INB3 / PWM3 | 11 / 12 / 4 | L46-48 | 2 / 5 / 3 | L22-24 |
| BNO055 (id, addr) | 55, 0x28 | L68 (ambos) | 55, 0x28 | L68 |
| LED | LED_BUILTIN | L244 (ambos) | — | — |
| Sensores línea | readLine(1)=s1, readLine(2)=s2, readLine(3)=s3 | L344-346 (ambos) | — | — |
| UART cámara | Serial1 | L239 (ambos) | — | — |

> Observación: los pines de motor 1 y motor 3 están **intercambiados** entre R1 y R2; motor 2 también difiere. Los sensores de línea y el resto del HW se leen vía `zirconLib` (pines internos de la librería).

### 3.2 Umbrales de línea (categoría UMBRAL)

| Símbolo | ROBOT1 | línea R1 | ROBOT2 | línea R2 |
|---|---|---|---|---|
| blanco1 (sensor izq) | **500** | L50 | 650 | L26 |
| blanco2 (sensor centro) | **650** | L51 | 650 | L27 |
| blanco3 (sensor der) | **600** | L52 | 750 | L28 |

### 3.3 Tolerancias / umbrales de visión (categoría UMBRAL — iguales en ambos)

| Símbolo | Valor | Línea | Uso |
|---|---|---|---|
| tolerancia_centrado | 30.0 | L109 | `abs(Yp-Yam)<=30` (solo estados delantero; el arquero NO lo usa) |
| tolerancia_cercania | 140.0 | L110 | `Xp<=140` para patear (arquero L1038/L1086) |
| tolerancia_apuntado | 15.0 | L111 | `abs(anguloPelota)>=15` (solo delantero) |
| umbral Y centrado patada (inline) | 3 | L1038, L1086 | `abs(Yp)<=3` |
| umbral Y desvío (inline) | 5 | L1045, L1093 | `abs(Yp)>=5` |
| banda muerta error (proporcionales) | -1 < error < 1 | L188, L212 | selección de rama centrada |

### 3.4 PWM / potencia (categoría PWM — valores efectivos)

| Concepto | Valor (R1) | Valor (R2) | Línea | Notas |
|---|---|---|---|---|
| patadM1 | 250 | 250 | L55 / L31 | PWM M1 en patada |
| patadM2 | **150** | **200** | L54 / L30 | PWM M2 en patada (ÚNICA potencia que difiere por robot) |
| c (factor centrado) | 0.4 | 0.4 | L56 / L32 | (solo delantero) |
| ic (factor impulso centrado) | **0.5** | **0.55** | L57 / L33 | (solo delantero) |
| g (factor girar) | 0.3 | 0.3 | L80 | `girar()`=100*g=30 (solo delantero) |
| a (factor apuntar) | 0.4 | 0.4 | L81 | 100*a=40 (solo delantero) |
| pd (factor proporcional) | 1 (sin pelota) / 1.5 (pelota desviada) | igual | L87, L1047/1065/1095/1113 | multiplica los PWM de `ai/adproporcional` |
| `avanzar()` | M1=100, M2=100, M3=0 | igual | L152-154 | |
| `impulso_inicial` | M1=90, M2=90, M3=153 | igual | L1018-1020 | `1.8*50`, `1.8*85` |
| `PATEANDO_atras_arquero` | M1=150, M2=150, M3=0 | igual | L1186-1188 | |
| `aiproporcional` PWM (pd=1) | banda muerta: 50/50/89; error>0: 50/50/40; error<0: M2=65,M1=40,M3=100 | igual | L188-208 | comentarios `//60 //99 //50 //120 //75` = valores ANTIGUOS, NO activos |
| `adproporcional` PWM (pd=1) | banda muerta: 50/50/89; error>0: 50/50/100; error<0: M2=65,M1=40,M3=40 | igual | L212-232 | idem |
| `avanzar_patear()` | M1=250, M2=150, M3=0 | M1=250, M2=200, M3=0 | L175-177 | rampa NO (PWM fijo inmediato) |
| `retroceder_patear()` | M1=250, M2=150, M3=0 (dir inv.) | M1=250, M2=200, M3=0 | L181-183 | (solo delantero) |

### 3.5 Tiempos / timeouts del ARQUERO (categoría TIEMPO — iguales en ambos robots)

| Estado | Línea | Timeout | Acción al cumplirse |
|---|---|---|---|
| impulso_inicial | L1022 | **40 ms** | → moverce_derecha |
| impulso_derecha | L1130 | **350 ms** | → moverce_derecha |
| impulso_izquierda | L1142 | **350 ms** | → moverce_izquierda |
| PATEANDO_pausa_inicial_arquero | L1154 | **200 ms** | → PATEANDO_adelante_arquero |
| PATEANDO_adelante_arquero | L1165 | **450 ms** | parar → PATEANDO_pausa_arquero |
| PATEANDO_pausa_arquero | L1177 | **1000 ms** | → PATEANDO_atras_arquero |
| PATEANDO_atras_arquero | L1190 | **(sin timeout)** | espera blanco (s1|s2|s3) → avanzar_despues_de_patear |
| avanzar_despues_de_patear | L1200 | **1000 ms** | → moverce_derecha |

### 3.6 PID / heading (categoría PID — iguales en ambos)

| Símbolo | Valor | Línea | Uso real |
|---|---|---|---|
| kp | 0.3 | L73 | `correccion = error*kp` (L340) — **calculada, NUNCA aplicada a motores** |
| error | currentYaw - initialYaw, wrap a (-180,180] | L337-339 | **SÍ usado**: selector de rama en `ai/adproporcional` (L188-232) |
| initialYaw | event.orientation.x al arrancar | L254 | referencia de rumbo capturada en setup |
| correccion | error*0.3 | L340 | sin consumidor (código muerto de heading-hold) |

### 3.7 Otros

| Símbolo | Valor | Línea |
|---|---|---|
| BAUD_RATE (Serial y Serial1) | 19200 | L77, L238-239 |
| START_BYTE | 0xAA (con `;` extra en el `#define`) | L76 — definido pero NO usado (el parser usa headers 201/202/203) |
| offset Y cámara | -100 | L280-284 |
| bytes por paquete cámara | 9 | L263 |
| headers cámara | 201 / 202 / 203 | L266, L277 |

---

## 4. Lógica de juego paso a paso (cámara → decisión → movimiento)

**Cada iteración de `loop()` (L259-1207):**

1. **LED de pelota (L261):** `digitalWrite(LED_BUILTIN, haypelota)`.

2. **Lectura de cámara (L263-329):** si `Serial1.available() >= 9` (L263), lee `header1` (L265). Si `header1==201` (L266) lee 8 bytes más: `codedXp, codedYp, header2, codedXam, codedYam, header3, codedXaz, codedYaz` (L268-275). Si la trama valida `header1==201 && header2==202 && header3==203` (L277), decodifica:
   - `Xp=codedXp`, `Yp=codedYp-100` (L279-280); igual para arco amarillo (`Xam`,`Yam`) y azul (`Xaz`,`Yaz`) (L281-284). **A la X no se le resta 100; solo a la Y.**
   - Calcula ángulos `atan2(Y,X)*180/PI` (L287-289) — solo el delantero los consume.
   - Presencia: `Xp==0`→`haypelota=false`; si no, `haypelota=true` y sella `millis_pelota=millis()` (L300-306). Igual para arcos por `Xam==0`/`Xaz==0` (L308-316).
   - Si `header1 != 201` (L323): fuerza `hayarco_azul=hayarco_amarillo=haypelota=false` (L325-327).

3. **Arco objetivo (L331):** `ARCO_CONTRINCANTE = hayarco_amarillo` (el arquero NO lo usa para patrullar; es para el bloque delantero).

4. **Giroscopio (L334-340):** `currentYaw=event.orientation.x` (L336); `error=currentYaw-initialYaw` (L337) con wrap a (-180,180] (L338-339); `correccion=error*0.3` (L340, sin uso).

5. **Sensores de línea (L344-346):** `s1=readLine(1)` (izq), `s2=readLine(2)` (centro), `s3=readLine(3)` (der).

6. **FSM (`switch(estado)`, L356):** ejecuta el `case` del estado actual. Para el arquero, el flujo de la sección 2.

**Decisión de patrulla (núcleo del arquero, dentro de `moverce_*`):**
- Sin pelota → `pd=1`, sigue strafeando hacia donde apunta el estado (`adproporcional`/`aiproporcional`), corrigiendo rumbo con `error`.
- Pelota cerca y centrada (`Xp<=140 && abs(Yp)<=3`) → para y dispara la secuencia de despeje.
- Pelota desviada (`abs(Yp)>=5`) → `pd=1.5`; va hacia el lado del signo de Yp (`Yp<0`→derecha; `Yp>=0`→izquierda).
- Pelota en `3<abs(Yp)<5` → para (banda muerta).
- Línea blanca (`s1` o `s2`) → impulso temporizado de 350 ms al lado opuesto.

---

## 5. Movimiento de bajo nivel (traducción a PWM, pisos/deadzone, kicker)

**Convención atómica por rueda (las 3 funciones de base, L140-184):** cada rueda se comanda con `analogWrite(PWMx, magnitud)` (0-255) + dos pines de dirección complementarios `digitalWrite(INAx, ...); digitalWrite(INBx, ...)`. Sentido `(INA=1,INB=0)` y `(INA=0,INB=1)` son opuestos; `(0,0)` = libre/coast (es lo que hace `parar()`, L146-150). El signo del movimiento lo da SIEMPRE la dirección, nunca el signo del PWM.

**Vía zirconLib vs inline:** este arquero **no llama** a las primitivas `motorN()` de zirconLib; usa `analogWrite`/`digitalWrite` directos sobre los pines del robot activo. Esto corresponde al esquema **Mark1** de zirconLib (PWM dedicado + 2 pines DIR digitales). Por tanto el tope `motorLimit=100` de zirconLib **no aplica aquí**: el arquero escribe PWM hasta 250 directo (patada). De `zirconLib` solo se usan `InitializeZircon()` (L237) y `readLine(1..3)` (L344-346).

**Geometría omni de 3 ruedas:** M1 y M2 son las delanteras (en `avanzar()` van a PWM≠0 con dirección opuesta entre sí; M3=0), M3 es la trasera (queda inerte al avanzar recto y se usa al máximo en los strafes proporcionales para corregir rumbo). En `impulso_inicial` y `PATEANDO_atras_arquero` se ve el patrón de strafe lateral (M1/M2 a un sentido, M3 al opuesto y/o a 0).

**Pisos / deadzone:** **NO hay ningún piso de PWM ni compensación de zona muerta en ninguna capa.** Los valores son fijos por tabla, afinados a mano de banco, modulados solo por `pd`/`g`/`a`/`c`/`ic`/`1.8`. No hay rampa de aceleración ni de freno; `avanzar_patear()` aplica el PWM de patada inmediato.

**Kicker:** NO hay solenoide. La "patada" es `avanzar_patear()` (L174-178): M1=250 + M2=150[R1]/200[R2] hacia adelante con M3 apagado, durante 450 ms (arquero). El retroceso de la secuencia es inline (L1186-1188), no usa la función de patada.

**Banda muerta del control de rumbo (`ai/adproporcional`, L188/L212):** la única "deadband" del sistema es la del `error` de heading: si `-1 < error < 1` usa la rama centrada (M3 al valor alto 89); si `error>0` o `error<0` redistribuye el PWM de M3/M1/M2 para corregir rumbo. Es bang-bang en 3 bandas, no un PID continuo.

---

## 6. Visión: formato del dato de cámara y parseo

**Paquete OpenMV v1: 9 bytes** (L263). Estructura (L265-275):

| Byte | Variable | Significado |
|---|---|---|
| 0 | header1 = 201 | sincronismo (L266) |
| 1 | codedXp | X pelota |
| 2 | codedYp | Y pelota (codificada con +100) |
| 3 | header2 = 202 | validación (L270, L277) |
| 4 | codedXam | X arco amarillo |
| 5 | codedYam | Y arco amarillo |
| 6 | header3 = 203 | validación (L273, L277) |
| 7 | codedXaz | X arco azul |
| 8 | codedYaz | Y arco azul |

**Decodificación (L279-284):** las X se copian tal cual (`Xp=codedXp`); las Y se recentran restando 100 (`Yp=codedYp-100`), permitiendo rango aproximado -100..+155. **Presencia por X==0** (L300, L308, L313): si la coordenada X de un objeto es 0, se considera no detectado. La validación de trama es triple-header (201/202/203); si falla el header1, se borran los tres flags (L325-327). El `START_BYTE 0xAA` (L76) NO interviene en el parseo real.

**Fragilidad fiel del parser:** si entran ≥9 bytes con `header1==201` pero `header2/header3` no son 202/203, no se resetean flags ni se actualizan posiciones → quedan los valores del ciclo anterior (datos "pegados"). No hay checksum ni resincronización por byte.

---

## 7. MEJORAS CON MÁS SENSORES (NO es transcripción — es PROPUESTA)

> **Sección especulativa.** Nada de lo siguiente está en el código 2025. Describe cómo el sistema 2026 (3 placas CENTRAL/TOP/DOWN, 4 ToF, OTOS de odometría, 2 BNO, 2 cámaras, comunicación inter-placa) podría **replicar EXACTO** este comportamiento y luego superarlo. Todo lo de aquí es para validar en hardware real (regla no negociable del repo).

### 7.0 Primero: replicar idéntico (criterio de no-regresión)
Antes de mejorar, el sistema 2026 debe poder reproducir la conducta exacta del arquero 2025 como modo de referencia: patrulla lateral por signo de Yp, patada por `Xp<=140 && abs(Yp)<=3`, secuencia 200→450→1000 ms→retroceso hasta blanco→1000 ms. Es el "modo legacy" contra el cual medir si lo nuevo realmente mejora. Plan de prueba: misma cancha, misma pelota, contar despejes exitosos por minuto.

### 7.1 Localización absoluta del arco con ToF + OTOS (sustituir "retroceder hasta ver blanco")
- **Problema 2025:** `PATEANDO_atras_arquero` (L1184) retrocede **sin timeout** hasta tocar blanco. Si no llega a la línea, se cuelga.
- **Propuesta:** con 4 ToF (trilateración a paredes) + OTOS (odometría) fusionados en una pose (módulos `pose_fusion`/`pose_filter` ya escritos, no cableados), el arquero conoce su posición en cm. El regreso al arco pasa a ser "ir a la pose objetivo (centro del arco)" con corte por distancia, no por sensor de línea. Elimina el cuelgue y permite volver al punto exacto, no solo "a la línea".
- **Replica + mejora:** la patrulla lateral deja de depender del rebote en blanco (L1070/L1117) y pasa a límites por coordenada X de pose (no salir del área), eliminando el bug de oscilación errática que los `impulso_*` de 350 ms intentan parchear (comentario L1127).
- `risk-no-fix`: el arquero sigue pudiendo colgarse y desposicionarse tras cada despeje. `risk-fix`: si la pose deriva (OTOS) o el ToF da outliers, el arquero podría creer que está en el arco sin estarlo → exige medir ruido ANTES de tunear y gating de frescura. `tiempo`: 2-3 días (cablear pose_fusion + test de ruido + titración). `prioridad`: P1.

### 7.2 Heading-hold real con BNO (activar la `correccion` muerta)
- **Problema 2025:** `correccion=error*0.3` (L340) se calcula y se tira; el rumbo solo se corrige por las 3 bandas bang-bang de `ai/adproporcional`.
- **Propuesta:** cerrar un PID de heading de verdad sobre el BNO (o fusión de los 2 BNO para robustez), aplicado como trim a las ruedas durante el strafe. El arquero patrullaría perfectamente paralelo a su línea sin serpenteo. **Obligatorio** pasar por `control-pid-zona-muerta` (actuador cuantizado, sin deadzone hoy) + `dinamica-omni-3-ruedas` (planta medida).
- `risk-no-fix`: el arquero se ladea al strafear y patea torcido. `risk-fix`: un PID mal tuneado oscila peor que el bang-bang actual; hay que titrar de banco. `tiempo`: 1-2 días. `prioridad`: P1.

### 7.3 Predicción de pelota con ball_predict (anticipar el tiro)
- **Problema 2025:** el arquero reacciona a la Yp actual; con la cámara a baja tasa, llega tarde a pelotas rápidas. La banda muerta `3<abs(Yp)<5` (L1060/L1108) lo deja quieto justo cuando debería moverse.
- **Propuesta:** usar `ball_predict` (ya corre) para extrapolar `Yp + vYp·Δt` (predict step, ver memoria del repo) y posicionarse donde la pelota VA a estar. Cap `MAX_EXTRAP_MS ~50-80ms` (P0 según memoria). Reduce goles por pelota cruzada.
- `risk-no-fix`: el arquero llega tarde a tiros laterales. `risk-fix`: extrapolar de más con pelota errática genera movimiento espurio → cap de extrapolación. `tiempo`: 1 día (la predicción ya existe; es cablearla al lazo del arquero). `prioridad`: P1.

### 7.4 Dos cámaras para FOV completo del arco
- **Problema 2025:** una sola cámara OpenMV; pelota fuera del FOV = `haypelota=false` y el arquero deja de seguir.
- **Propuesta:** TOP + segunda cámara amplían el campo; fusionar detecciones en CENTRAL. El arquero no pierde la pelota en los extremos del área. Requiere calibración LAB consistente entre cámaras (`openmv-vision-tuning`) — crítico para Incheon por iluminación distinta a Salta.
- `risk-no-fix`: pierde pelotas en ángulos amplios. `risk-fix`: dos cámaras desincronizadas dan detecciones contradictorias → necesita arbitraje. `tiempo`: 2 días + dependencia de TASK-214 (matrices de cámara de Elías). `prioridad`: P2.

### 7.5 Estado seguro y watchdog (robustez del bare-metal)
- **Problema 2025:** `while(1)` si falla el BNO (L248) deja el robot inerte; el `PATEANDO_atras_arquero` sin timeout puede colgar.
- **Propuesta:** watchdog + estado seguro (motores a coast, reintento de init de sensores, timeout de respaldo en todos los estados, incluido el retroceso). Lente: `sistemas-criticos-tolerancia-fallas` + `tiempo-real-determinismo`. Multi-placa permite que CENTRAL degrade con gracia si TOP/DOWN no responden.
- `risk-no-fix`: un fallo de sensor saca al arquero del partido. `risk-fix`: un watchdog mal puesto reinicia en loop. `tiempo`: 1-2 días. `prioridad`: P0 (no competir bien si el robot se cuelga).

---

### Ambigüedades / inconsistencias marcadas FIELMENTE (transcritas, no corregidas)
1. **L340 `correccion`** se calcula (`error*0.3`) pero nunca se escribe a motores: heading-hold muerto. El giroscopio SÍ se usa, pero solo vía `error` como selector de banda en `ai/adproporcional`.
2. **L76 `START_BYTE 0xAA;`** con `;` dentro del `#define`; nunca usado (parser real usa 201/202/203).
3. **L1184 `PATEANDO_atras_arquero` sin timeout** — único estado del arquero que solo sale por sensor; riesgo de cuelgue.
4. **L1060/L1108 banda muerta `3<abs(Yp)<5`** — el robot se queda quieto en ese rango de desvío.
5. **Decodificación asimétrica X/Y** (L279-284): a la Y se le resta 100, a la X no.
6. **Comentarios `//60 //99 //120 //50 //75`** en `ai/adproporcional` (L189-231) son valores de tuneo ANTIGUOS; los activos son los `pd*N`.
7. **Diferencias R1 vs R2 (las únicas):** pines de motor (M1↔M3 intercambiados), `blanco1` (500 vs 650), `blanco3` (600 vs 750), `patadM2` (150 vs 200), `ic` (0.5 vs 0.55). `blanco2=650`, `patadM1=250`, `c=0.4`, y TODOS los timeouts/tolerancias/kp/g/a/pd y números mágicos del loop son comunes.
8. **Estados huérfanos:** `PRIMER_IMPULSO_INICIAL_GIRANDO` (L122) y `CENTRANDO_giroscopo` (L125) están en el enum pero no tienen `case`.
9. **Co-residencia arquero/delantero:** todo el bloque DELANTERO del `switch` es código no alcanzado por el ciclo del arquero (que arranca en `impulso_inicial`).

**Rutas absolutas relevantes:**
- Programa: `C:\Users\violl\dev\open-soccer-robocup-team2026\software\_deprecated-2025\robot-arquero\definitivo-arquero_6-9-2026`
- zirconLib: `C:\Users\violl\dev\open-soccer-robocup-team2026\software\_deprecated-2025\zirconLib\zirconLib.cpp` / `.h`
- Doc canónico de valores históricos: `C:\Users\violl\dev\open-soccer-robocup-team2026\docs\firmware\MOTION-CONTROL-HISTORICO.md`

---

## Apéndice — Tabla EXHAUSTIVA de constantes (153 ítems)

| Nombre | Valor | Robot | Categoría | Línea | Significado |
|---|---|---|---|---|---|
| `error (init)` | `0` | ambos | PID | 70 | Error de heading = currentYaw - initialYaw (recalc cada loop, L337). |
| `kp` | `0.3` | ambos | PID | 73 | Ganancia proporcional del heading. correccion = error*kp (L340). NOTA: 'correccion' se calcula pero no se observo su uso aplicado a motores en este archivo. |
| `banda muerta error aiproporcional/adproporcional` | `-1 < error < 1` | ambos | PID | 188 | Umbral de zona muerta del error de heading que selecciona la rama centrada vs error>0 / error<0 (L188,L193,L201,L212,L217,L225). |
| `wrap error heading +180` | `error>180 -> error-360` | ambos | PID | 338 | Normalizacion del error de yaw al rango (-180,180]. |
| `wrap error heading -180` | `error<-180 -> error+360` | ambos | PID | 339 | Normalizacion del error de yaw (rama negativa). |
| `patadM2` | `200` | R2 | PWM-potencia | 30 | PWM motor 2 durante la patada (avanzar_patear/retroceder_patear) (R2). |
| `patadM1` | `250` | R2 | PWM-potencia | 31 | PWM motor 1 durante la patada (R2). |
| `c` | `0.4` | R2 | PWM-potencia | 32 | Factor de velocidad mientras centra (multiplica PWM en CENTRANDO_horario/antihorario) (R2). |
| `ic` | `0.55` | R2 | PWM-potencia | 33 | Factor de velocidad del impulso de centrado (multiplica PWM en IMPULSO_CENTRANDO_*) (R2). |
| `patadM2` | `150` | R1 | PWM-potencia | 54 | PWM motor 2 durante la patada (R1, ACTIVO). |
| `patadM1` | `250` | R1 | PWM-potencia | 55 | PWM motor 1 durante la patada (R1). |
| `c` | `0.4` | R1 | PWM-potencia | 56 | Factor velocidad centrando (R1). Comentario L61: 'c: velocidad centrando'. |
| `ic` | `0.5` | R1 | PWM-potencia | 57 | Factor velocidad impulso centrando (R1). Comentario L62: 'ic: velocidad impulso centrando'. |
| `g (velocidad girar)` | `0.3` | ambos | PWM-potencia | 80 | Factor de velocidad de girar(): PWM = 100*g = 30 en los 3 motores (L141-143). |
| `a (velocidad apuntar)` | `0.4` | ambos | PWM-potencia | 81 | Factor de velocidad al apuntar pelota: PWM = 100*a = 40 en APUNTAR_PELOTA y APUNTAR_PELOTA_antihorario (L486-495, L809-818). |
| `pd (init)` | `1` | ambos | PWM-potencia | 87 | Factor proporcional de velocidad en aiproporcional/adproporcional. Vale 1 sin pelota, 1.5 con pelota desalineada (L1047,L1065,L1095,L1113). |
| `girar() PWM` | `100*g = 30` | ambos | PWM-potencia | 141 | girar(): los 3 motores a PWM 100*g=30, todos INA=0/INB=1 (giro antihorario). |
| `parar() PWM` | `0` | ambos | PWM-potencia | 147 | parar(): los 3 PWM en 0, INA=INB=0 (freno/coast). |
| `avanzar() PWM` | `M1=100, M2=100, M3=0` | ambos | PWM-potencia | 152 | avanzar(): M1 PWM100 INA1=1/INB1=0; M2 PWM100 INA2=0/INB2=1; M3 PWM0 INA3=1/INB3=0. |
| `retroceder1() PWM` | `M1=0, M2=100, M3=100` | ambos | PWM-potencia | 157 | retroceder1(): retroceso usado al detectar linea sensor 1. M1 PWM0/INB1=1; M2 PWM100/INB2=1; M3 PWM100/INA3=1. |
| `retroceder2() PWM` | `M1=100, M2=0, M3=100` | ambos | PWM-potencia | 162 | retroceder2(): retroceso al detectar linea sensor 2. M1 PWM100/INA1=1; M2 PWM0/INA2=1; M3 PWM100/INB3=1. |
| `retroceder3() PWM` | `M1=100, M2=100, M3=0` | ambos | PWM-potencia | 167 | retroceder3(): retroceso al detectar linea sensor 3. M1 PWM100/INB1=1; M2 PWM100/INA2=1; M3 PWM0/INB3=1. |
| `avanzar_patear() PWM` | `M1=patadM1, M2=patadM2, M3=0` | ambos | PWM-potencia | 174 | avanzar_patear(): M1=patadM1, M2=patadM2 (R1:250/150, R2:250/200), M3 PWM0/INA3=0/INB3=0. Es el avance fuerte de patada. |
| `retroceder_patear() PWM` | `M1=patadM1, M2=patadM2, M3=0` | ambos | PWM-potencia | 180 | retroceder_patear(): mismas potencias patadM1/patadM2, sentido inverso; M3 PWM0/INA3=1/INB3=0. |
| `aiproporcional() PWM (error en [-1,1])` | `M2=pd*50, M1=pd*50, M3=pd*89` | ambos | PWM-potencia | 189 | Avance izquierdo proporcional, banda muerta error en (-1,1): M2/M1 pd*50, M3 pd*89. Comentarios al lado: 60/60/99 (valores antiguos). |
| `aiproporcional() PWM (error>0)` | `M2=pd*50, M1=pd*50, M3=pd*40` | ambos | PWM-potencia | 195 | Avance izquierdo, error>0: M2 pd*50, M1 pd*50, M3 pd*40. Comentarios: 60/60/50. |
| `aiproporcional() PWM (error<0)` | `M2=pd*65, M1=pd*40, M3=pd*100` | ambos | PWM-potencia | 203 | Avance izquierdo, error<0: M2 pd*65, M1 pd*40, M3 pd*100. Comentarios: 75/50/120. Compensa deriva de heading. |
| `adproporcional() PWM (error en [-1,1])` | `M2=pd*50, M1=pd*50, M3=pd*89` | ambos | PWM-potencia | 213 | Avance derecho proporcional, banda muerta: M2/M1 pd*50, M3 pd*89. Comentarios: 60/60/99. |
| `adproporcional() PWM (error>0)` | `M2=pd*50, M1=pd*50, M3=pd*100` | ambos | PWM-potencia | 219 | Avance derecho, error>0: M2 pd*50, M1 pd*50, M3 pd*100. Comentarios: 60/60/120. |
| `adproporcional() PWM (error<0)` | `M2=pd*65, M1=pd*40, M3=pd*40` | ambos | PWM-potencia | 227 | Avance derecho, error<0: M2 pd*65, M1 pd*40, M3 pd*40. Comentarios: 75/50/50. |
| `IMPULSO_INICIAL_GIRANDO PWM` | `150 (3 motores)` | ambos | PWM-potencia | 370 | Giro con mas potencia: los 3 motores PWM150, INA=0/INB=1 (antihorario). |
| `APUNTAR_PELOTA PWM giro` | `100*a = 40` | ambos | PWM-potencia | 486 | Gira los 3 motores a 40 (100*a). Sentido antihorario si anguloPelota>0, horario si <0 (L486-495). |
| `CENTRANDO_horario PWM` | `M1=60*c=24, M2=60*c=24, M3=180*c=72` | ambos | PWM-potencia | 594 | Orbita horaria: M1/M2 60*c, M3 180*c. c=0.4 ambos robots. M1/M2 INB=1, M3 INA=1. |
| `IMPULSO_CENTRANDO_antihorario PWM` | `M1=60*ic, M2=60*ic, M3=180*ic` | ambos | PWM-potencia | 688 | Impulso antihorario: M1/M2 60*ic, M3 180*ic. ic R1=0.5 (->30/30/90), R2=0.55 (->33/33/99). M1/M2 INA=1, M3 INB=1. |
| `CENTRANDO_antihorario PWM` | `M1=60*c, M2=60*c, M3=180*c` | ambos | PWM-potencia | 700 | Orbita antihoraria: M1/M2 60*c=24, M3 180*c=72. M1/M2 INA=1, M3 INB=1. |
| `IMPULSO_CENTRANDO_horario PWM` | `M1=60*ic, M2=60*ic, M3=180*ic` | ambos | PWM-potencia | 792 | Impulso horario: M1/M2 60*ic, M3 180*ic. M1/M2 INB=1, M3 INA=1. (NOTA: tras 500ms pasa a CENTRANDO_antihorario, L799 — posible inconsistencia, no a horario). |
| `APUNTAR_PELOTA_antihorario PWM` | `100*a = 40` | ambos | PWM-potencia | 809 | Gira los 3 motores a 40 (100*a); antihorario si anguloPelota>0, horario si <0. Si dentro de tolerancia -> CENTRANDO_antihorario (L824). |
| `impulso_inicial PWM (arquero)` | `M2=1.8*50=90, M1=1.8*50=90, M3=1.8*85=153` | ambos | PWM-potencia | 1018 | Estado INICIAL real del arquero: impulso lateral. M1/M2 INA=1 PWM 1.8*50=90; M3 INB=1 PWM 1.8*85=153. |
| `moverce_derecha pd con pelota` | `pd = 1.5` | ambos | PWM-potencia | 1047 | Sube el factor proporcional a 1.5 cuando hay que corregir lateralmente con pelota. |
| `moverce_derecha pd sin pelota` | `pd = 1` | ambos | PWM-potencia | 1065 | Sin pelota, pd vuelve a 1 (patrulla a velocidad base). |
| `moverce_izquierda pd con pelota` | `pd = 1.5` | ambos | PWM-potencia | 1095 | Factor proporcional 1.5 al corregir con pelota. |
| `moverce_izquierda pd sin pelota` | `pd = 1` | ambos | PWM-potencia | 1113 | Sin pelota pd=1. |
| `PATEANDO_atras_arquero PWM` | `M1=150, M2=150, M3=0` | ambos | PWM-potencia | 1186 | Retrocede a 150 (M1 INB=1, M2 INA=1, M3 PWM0/INA=1) hasta detectar blanco (s1/s2/s3>=umbral) -> avanzar_despues_de_patear. |
| `#define ROBOT1 (activo)` | `definido` | R1 | otro | 10 | Selecciona el bloque de pines/constantes de ROBOT1. Este es el robot ACTIVO en este build. |
| `#define ROBOT2 (comentado)` | `//#define ROBOT2 (deshabilitado)` | R2 | otro | 11 | Bloque ROBOT2 NO compila en este build; sus valores quedan documentados pero inactivos. |
| `ARCO_CONTRINCANTE (init)` | `false` | ambos | otro | 65 | Flag arco al que hay que hacer gol. Se reasigna en runtime a hayarco_amarillo (L331): el arco objetivo es el AMARILLO. |
| `initialYaw (init)` | `0` | ambos | otro | 71 | Yaw de referencia capturado en setup (L254) desde event.orientation.x al arrancar. |
| `START_BYTE` | `0xAA` | ambos | otro | 76 | Byte de inicio serial definido (#define START_BYTE 0xAA). No se observo su uso en el parser (el parser usa header 201/202/203). |
| `BAUD_RATE` | `19200` | ambos | otro | 77 | Baudios de Serial y Serial1 (setup L238-239). |
| `estado (inicial)` | `impulso_inicial` | ambos | otro | 131 | Estado inicial de la FSM = impulso_inicial (rama arquero). |
| `i (init)` | `0` | ambos | otro | 136 | Variable global int i declarada en 0; no se observo uso posterior. |
| `Serial.begin / Serial1.begin` | `BAUD_RATE=19200` | ambos | otro | 238 | Inicializa puertos serie a 19200 (USB debug y Serial1 hacia camara). |
| `while(1) si no hay BNO` | `bloqueo infinito` | ambos | otro | 248 | Si bno.begin() falla, el firmware queda en while(1) (cuelga). Comportamiento bloqueante. |
| `bno.setExtCrystalUse` | `true` | ambos | otro | 250 | Usa cristal externo del BNO055 para mejor estabilidad de yaw. |
| `INA1` | `8` | R2 | pin | 14 | Pin direccion A motor 1 (R2). |
| `INB1` | `7` | R2 | pin | 15 | Pin direccion B motor 1 (R2). |
| `PWM1` | `6` | R2 | pin | 16 | Pin PWM motor 1 (R2). |
| `INA2` | `11` | R2 | pin | 18 | Pin direccion A motor 2 (R2). |
| `INB2` | `12` | R2 | pin | 19 | Pin direccion B motor 2 (R2). |
| `PWM2` | `4` | R2 | pin | 20 | Pin PWM motor 2 (R2). |
| `INA3` | `2` | R2 | pin | 22 | Pin direccion A motor 3 (R2). |
| `INB3` | `5` | R2 | pin | 23 | Pin direccion B motor 3 (R2). |
| `PWM3` | `3` | R2 | pin | 24 | Pin PWM motor 3 (R2). |
| `INA1` | `2` | R1 | pin | 38 | Pin direccion A motor 1 (R1, ACTIVO). |
| `INB1` | `5` | R1 | pin | 39 | Pin direccion B motor 1 (R1). |
| `PWM1` | `3` | R1 | pin | 40 | Pin PWM motor 1 (R1). |
| `INA2` | `8` | R1 | pin | 42 | Pin direccion A motor 2 (R1). |
| `INB2` | `7` | R1 | pin | 43 | Pin direccion B motor 2 (R1). |
| `PWM2` | `6` | R1 | pin | 44 | Pin PWM motor 2 (R1). |
| `INA3` | `11` | R1 | pin | 46 | Pin direccion A motor 3 (R1). |
| `INB3` | `12` | R1 | pin | 47 | Pin direccion B motor 3 (R1). |
| `PWM3` | `4` | R1 | pin | 48 | Pin PWM motor 3 (R1). |
| `bno (Adafruit_BNO055 id, addr)` | `id=55, addr=0x28` | ambos | pin | 68 | Instancia giroscopo BNO055: sensor ID 55, direccion I2C 0x28. |
| `readLine sensor 1 (s1)` | `readLine(1)` | ambos | pin | 344 | Lectura sensor de linea izquierdo via zirconLib readLine(1). |
| `readLine sensor 2 (s2)` | `readLine(2)` | ambos | pin | 345 | Lectura sensor de linea centro readLine(2). |
| `readLine sensor 3 (s3)` | `readLine(3)` | ambos | pin | 346 | Lectura sensor de linea derecho readLine(3). |
| `AVANCE_INICIO timeout (delantero)` | `>= 700 ms` | ambos | timeout | 360 | avanzar_patear() durante 700ms; luego parar y pasar a IMPULSO_INICIAL_GIRANDO. (Estado del bloque delantero, no se alcanza desde impulso_inicial). |
| `IMPULSO_INICIAL_GIRANDO timeout` | `>= 50 ms` | ambos | timeout | 374 | Tras 50ms pasa a GIRANDO. |
| `IMPULSO_INICIAL_GIRANDO espera inercia con pelota` | `>= 1000 ms` | ambos | timeout | 383 | Si ve pelota: parar y tras 1000ms (inercia) -> APUNTAR_PELOTA. |
| `GIRANDO espera inercia con pelota` | `>= 700 ms` | ambos | timeout | 411 | En GIRANDO, si ve pelota: parar y tras 700ms -> APUNTAR_PELOTA. (comentario dice 'un segundo' pero es 700). |
| `GIRANDO salida por tiempo+heading` | `>= 8000 ms && (error<=0 o error>=350)` | ambos | timeout | 422 | Tras 8s girando sin pelota y con heading en cierta franja -> AVANZANDO_POR_TIEMPO. |
| `AVANZANDO_POR_TIEMPO timeout` | `>= 500 ms` | ambos | timeout | 449 | avanzar() 500ms y vuelve a IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA perdida de pelota` | `millis()-millis_pelota >= 500 ms` | ambos | timeout | 511 | Si no ve pelota hace 500ms -> IMPULSO_INICIAL_GIRANDO. |
| `APUNTAR_PELOTA timeout` | `>= 20000 ms` | ambos | timeout | 517 | Timeout 20s -> IMPULSO_INICIAL_GIRANDO. |
| `AVANZANDO perdida de pelota` | `millis()-millis_pelota >= 500 ms` | ambos | timeout | 562 | Sin pelota 500ms -> IMPULSO_INICIAL_GIRANDO. |
| `AVANZANDO timeout` | `>= 20000 ms` | ambos | timeout | 569 | Timeout 20s -> IMPULSO_INICIAL_GIRANDO. |
| `CENTRANDO_horario salida tiempo+yaw` | `>= 5000 ms && (currentYaw<=10 o currentYaw>=350)` | ambos | timeout | 606 | Tras 5s y yaw cerca de 0/360 -> PATEANDO_pausa_inicial. |
| `CENTRANDO_horario salida tiempo+amarillo` | `>= 4500 ms && ARCO_CONTRINCANTE` | ambos | timeout | 613 | Tras 4.5s viendo amarillo (aunque no centrado) -> PATEANDO_pausa_inicial. |
| `CENTRANDO_horario perdida pelota` | `millis()-millis_pelota >= 4000 ms` | ambos | timeout | 627 | Sin pelota 4s -> IMPULSO_INICIAL_GIRANDO. |
| `CENTRANDO_horario timeout` | `>= 20000 ms` | ambos | timeout | 633 | Timeout 20s -> PATEANDO_pausa_inicial. |
| `IMPULSO_CENTRANDO_antihorario duracion` | `>= 500 ms` | ambos | timeout | 692 | Tras 500ms -> CENTRANDO_antihorario. |
| `CENTRANDO_antihorario salida tiempo+yaw` | `>= 5000 ms && (currentYaw<=10 o >=350)` | ambos | timeout | 712 | Tras 5s y yaw cerca de 0 -> PATEANDO_pausa_inicial. |
| `CENTRANDO_antihorario salida tiempo+amarillo` | `>= 4500 ms && ARCO_CONTRINCANTE` | ambos | timeout | 719 | Tras 4.5s viendo amarillo -> PATEANDO_pausa_inicial. |
| `CENTRANDO_antihorario perdida pelota` | `millis()-millis_pelota >= 3000 ms` | ambos | timeout | 733 | Sin pelota 3s -> IMPULSO_INICIAL_GIRANDO. (NOTA: difiere de horario que usa 4000ms). |
| `CENTRANDO_antihorario timeout` | `>= 20000 ms` | ambos | timeout | 740 | Timeout 20s -> PATEANDO_pausa_inicial. |
| `IMPULSO_CENTRANDO_horario duracion` | `>= 500 ms` | ambos | timeout | 796 | Tras 500ms -> CENTRANDO_antihorario. |
| `PATEANDO_corto_pausa_inicial` | `>= 500 ms` | ambos | timeout | 833 | parar() 500ms -> PATEANDO_corto_adelante. |
| `PATEANDO_corto_adelante duracion` | `>= 200 ms` | ambos | timeout | 843 | avanzar_patear() 200ms (patada corta) -> parar -> PATEANDO_corto_pausa. |
| `PATEANDO_corto_pausa` | `>= 500 ms` | ambos | timeout | 854 | parar() 500ms -> PATEANDO_corto_atras. |
| `PATEANDO_corto_atras duracion` | `>= 400 ms` | ambos | timeout | 864 | retroceder_patear() 400ms -> IMPULSO_INICIAL_GIRANDO. |
| `PATEANDO_pausa_inicial (largo)` | `>= 1000 ms` | ambos | timeout | 877 | parar() 1000ms -> PATEANDO_adelante. Con salidas de linea (s1/s2/s3) a DETECTA_LINEA_*. |
| `PATEANDO_adelante duracion (largo)` | `>= 500 ms` | ambos | timeout | 902 | avanzar_patear() 500ms (patada larga) -> parar -> PATEANDO_pausa. Con salidas de linea. |
| `PATEANDO_pausa (largo)` | `>= 500 ms` | ambos | timeout | 928 | parar() 500ms -> PATEANDO_atras. Con salidas de linea. |
| `PATEANDO_atras duracion (largo)` | `>= 200 ms` | ambos | timeout | 953 | retroceder_patear() 200ms -> IMPULSO_INICIAL_GIRANDO. Con salidas de linea. |
| `DETECTA_LINEA_1 duracion` | `>= 400 ms` | ambos | timeout | 981 | retroceder1() 400ms -> IMPULSO_INICIAL_GIRANDO. |
| `DETECTA_LINEA_2 duracion` | `>= 400 ms` | ambos | timeout | 993 | retroceder2() 400ms -> IMPULSO_INICIAL_GIRANDO. |
| `DETECTA_LINEA_3 duracion` | `>= 400 ms` | ambos | timeout | 1005 | retroceder3() 400ms -> IMPULSO_INICIAL_GIRANDO. |
| `impulso_inicial duracion` | `>= 40 ms` | ambos | timeout | 1022 | Tras 40ms -> moverce_derecha. |
| `impulso_derecha duracion` | `>= 350 ms` | ambos | timeout | 1130 | adproporcional() 350ms y luego moverce_derecha. Evita oscilacion erratica en el borde (comentario L1127). |
| `impulso_izquierda duracion` | `>= 350 ms` | ambos | timeout | 1142 | aiproporcional() 350ms y luego moverce_izquierda. (comentario L1139). |
| `PATEANDO_pausa_inicial_arquero` | `>= 200 ms` | ambos | timeout | 1154 | parar() 200ms (deja pasar la inercia horizontal) -> PATEANDO_adelante_arquero. |
| `PATEANDO_adelante_arquero duracion` | `>= 450 ms` | ambos | timeout | 1165 | avanzar_patear() 450ms (patada del arquero) -> parar -> PATEANDO_pausa_arquero. |
| `PATEANDO_pausa_arquero` | `>= 1000 ms` | ambos | timeout | 1177 | parar() 1000ms -> PATEANDO_atras_arquero. |
| `avanzar_despues_de_patear duracion` | `>= 1000 ms` | ambos | timeout | 1200 | avanzar() 1000ms (vuelve a su lugar) -> moverce_derecha. Cierra el ciclo del arquero. |
| `blanco1` | `650` | R2 | umbral | 26 | Umbral sensor de linea 1 (izquierdo) para detectar blanco (R2). readLine(1)>=blanco1 -> linea. |
| `blanco2` | `650` | R2 | umbral | 27 | Umbral sensor de linea 2 (centro) para detectar blanco (R2). |
| `blanco3` | `750` | R2 | umbral | 28 | Umbral sensor de linea 3 (derecho) para detectar blanco (R2). |
| `blanco1` | `500` | R1 | umbral | 50 | Umbral sensor de linea 1 (izquierdo) para detectar blanco (R1, ACTIVO). |
| `blanco2` | `650` | R1 | umbral | 51 | Umbral sensor de linea 2 (centro) para detectar blanco (R1). |
| `blanco3` | `600` | R1 | umbral | 52 | Umbral sensor de linea 3 (derecho) para detectar blanco (R1). |
| `tolerancia_centrado` | `30.0` | ambos | umbral | 109 | Tolerancia en Y para considerar pelota y arco amarillo centrados: abs(Yp-Yam)<=30 -> patea (L505,L600,L706). |
| `tolerancia_cercania` | `140.0` | ambos | umbral | 110 | Umbral de distancia (Xp) para considerar la pelota cercana: Xp<=140 (L547 delantero; L1038,L1086 arquero). |
| `tolerancia_apuntado` | `15.0` | ambos | umbral | 111 | Tolerancia angular para considerar apuntada la pelota: abs(anguloPelota)>=15 -> sigue corrigiendo (L481,L555,L620,L726,L804). |
| `APUNTAR_PELOTA umbral apuntado` | `abs(anguloPelota) >= 15 (tolerancia_apuntado)` | ambos | umbral | 481 | Si supera 15 grados corrige; si no, pasa a AVANZANDO (L501). |
| `APUNTAR_PELOTA salida a patada` | `ARCO_CONTRINCANTE && haypelota && abs(Yp-Yam)<=30` | ambos | umbral | 505 | Si pelota y arco amarillo centrados (<=tolerancia_centrado) -> PATEANDO_pausa_inicial. |
| `AVANZANDO salida cercania` | `haypelota && Xp <= 140 (tolerancia_cercania)` | ambos | umbral | 547 | Si pelota suficientemente cerca -> CENTRANDO_horario. |
| `AVANZANDO re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 555 | Si se desalinea -> APUNTAR_PELOTA. |
| `CENTRANDO_horario salida centrado` | `ARCO_CONTRINCANTE && haypelota && abs(Yp-Yam)<=30` | ambos | umbral | 600 | Pelota+arco centrados -> PATEANDO_pausa_inicial. |
| `CENTRANDO_horario re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 620 | Si pelota se desalinea -> APUNTAR_PELOTA. |
| `CENTRANDO_horario decision linea por yaw` | `currentYaw<=90 o currentYaw>=270` | ambos | umbral | 642 | Al ver blanco (s1/s2/s3): si mira al frente (yaw<=90 o >=270) -> PATEANDO_corto_pausa_inicial; si no -> IMPULSO_CENTRANDO_antihorario (L640-683). |
| `CENTRANDO_antihorario salida centrado` | `ARCO_CONTRINCANTE && haypelota && abs(Yp-Yam)<=30` | ambos | umbral | 706 | Pelota+arco centrados -> PATEANDO_pausa_inicial. |
| `CENTRANDO_antihorario re-apuntar` | `abs(anguloPelota) >= 15` | ambos | umbral | 726 | Si pelota se desalinea -> APUNTAR_PELOTA_antihorario. |
| `CENTRANDO_antihorario decision linea por yaw` | `currentYaw<=90 o >=270` | ambos | umbral | 748 | Al ver blanco: si frente -> PATEANDO_corto_pausa_inicial; si no -> IMPULSO_CENTRANDO_antihorario (L746-787). |
| `APUNTAR_PELOTA_antihorario umbral` | `abs(anguloPelota) >= 15` | ambos | umbral | 804 | Mismo umbral tolerancia_apuntado=15. |
| `moverce_derecha cercania+centrado pelota` | `Xp<=140 (tolerancia_cercania) && abs(Yp)<=3` | ambos | umbral | 1038 | Pelota cerca y casi centrada en Y -> parar y PATEANDO_pausa_inicial_arquero (patea). |
| `moverce_derecha desalineacion Y` | `abs(Yp) >= 5` | ambos | umbral | 1045 | Si pelota desalineada en Y: pd=1.5 y elige direccion segun signo de Yp (Yp<0 derecha, else izquierda). |
| `moverce_derecha salida por blanco` | `s1>=blanco1 o s2>=blanco2` | ambos | umbral | 1070 | Si detecta linea (s1 o s2) -> parar y impulso_izquierda (rebote del borde). |
| `moverce_izquierda cercania+centrado pelota` | `Xp<=140 && abs(Yp)<=3` | ambos | umbral | 1086 | Pelota cerca y centrada -> PATEANDO_pausa_inicial_arquero. |
| `moverce_izquierda desalineacion Y` | `abs(Yp) >= 5` | ambos | umbral | 1093 | Pelota desalineada: pd=1.5 y elige direccion segun Yp. |
| `moverce_izquierda salida por blanco` | `s1>=blanco1 o s2>=blanco2` | ambos | umbral | 1117 | Si detecta linea -> parar e impulso_derecha. |
| `PATEANDO_atras_arquero salida por blanco` | `s1>=blanco1 o s2>=blanco2 o s3>=blanco3` | ambos | umbral | 1190 | Vuelve al arco retrocediendo hasta tocar la linea de fondo. |
| `anguloPelota (init)` | `0.0` | ambos | vision | 84 | Angulo a la pelota = atan2(Yp,Xp)*180/PI (L287). |
| `anguloArco_Amarillo (init)` | `0.0` | ambos | vision | 85 | Angulo arco amarillo = atan2(Yam,Xam)*180/PI (L288). |
| `anguloArco_Azul (init)` | `0.0` | ambos | vision | 86 | Angulo arco azul = atan2(Yaz,Xaz)*180/PI (L289). |
| `Serial1.available umbral parser` | `>= 9` | ambos | vision | 263 | Espera 9 bytes (paquete OpenMV v1) antes de parsear. |
| `header1 esperado` | `201` | ambos | vision | 266 | Primer byte de sincronismo del paquete de camara. Si !=201 -> haypelota/arcos=false (L323-328). |
| `header2 esperado` | `202` | ambos | vision | 277 | Segundo header de validacion del paquete. |
| `header3 esperado` | `203` | ambos | vision | 277 | Tercer header de validacion del paquete. |
| `offset Y decodificado` | `-100` | ambos | vision | 280 | Yp=codedYp-100, Yam=codedYam-100, Yaz=codedYaz-100. La camara codifica Y con bias +100 (rango -100..+155). |
| `conversion angulo a grados` | `*180.0/PI` | ambos | vision | 287 | Factor radianes->grados de atan2 para los 3 angulos (L287-289). |
| `deteccion pelota (Xp==0)` | `Xp==0 -> haypelota=false` | ambos | vision | 300 | Si Xp==0 no hay pelota; si !=0 haypelota=true y se sella millis_pelota (L305). |
| `deteccion arco amarillo (Xam==0)` | `Xam==0 -> hayarco_amarillo=false` | ambos | vision | 308 | Presencia arco amarillo segun Xam!=0. |
| `deteccion arco azul (Xaz==0)` | `Xaz==0 -> hayarco_azul=false` | ambos | vision | 313 | Presencia arco azul segun Xaz!=0. |

### Notas de constantes

VALORES CALCULADOS EN RUNTIME (no literales puros, derivados de constantes): girar()=100*g=30; APUNTAR_PELOTA/_antihorario=100*a=40; CENTRANDO_horario/antihorario M1/M2=60*c=24, M3=180*c=72 (con c=0.4 en ambos robots); IMPULSO_CENTRANDO_* M1/M2=60*ic, M3=180*ic -> R1(ic=0.5):30/30/90, R2(ic=0.55):33/33/99; impulso_inicial M1/M2=1.8*50=90, M3=1.8*85=153 (numeros magicos 1.8, 50, 85 inline en L1018-1020); aiproporcional/adproporcional usan pd*N con pd=1 o 1.5 (PWM efectivo varia: ej M3 puede ser 100*1.5=150). Los comentarios //60 //99 //120 //50 //75 al lado de las lineas L189-231 son VALORES ANTIGUOS de tuneo, NO los activos.

AMBIGUEDADES / posibles bugs detectados (no asumidos, transcritos del codigo):
- 'correccion' (L340 = error*kp con kp=0.3) se CALCULA cada loop pero NO se observo que se aplique a ningun motor en este archivo. AMBIGUO si es heading-hold muerto.
- START_BYTE (0xAA, L76) definido pero el parser real usa headers 201/202/203, no 0xAA. Constante sin uso observado.
- IMPULSO_CENTRANDO_horario (L791-801): mueve en sentido horario pero al terminar pasa a CENTRANDO_antihorario (L799), no a un centrado horario. Posible inconsistencia de transicion. Ademas IMPULSO_CENTRANDO_horario y CENTRANDO_giroscopo estan en el enum pero NUNCA se transiciona hacia IMPULSO_CENTRANDO_horario desde ningun estado observado (estado huerfano); CENTRANDO_giroscopo no tiene case en el switch.
- 'i' (L136) variable global int=0 sin uso posterior observado.
- PRIMER_IMPULSO_INICIAL_GIRANDO esta en el enum (L122) pero no tiene case en el switch.
- Diferencia intencional o no: CENTRANDO_horario pierde pelota a 4000ms (L627) vs CENTRANDO_antihorario a 3000ms (L733).
- ROBOT por robot: las UNICAS diferencias R1 vs R2 son: pines de motores (INA/INB/PWM 1 y 2 intercambiados; motor3 igual salvo nombres), blanco1 (R1=500/R2=650), blanco3 (R1=600/R2=750), patadM2 (R1=150/R2=200), ic (R1=0.5/R2=0.55). blanco2=650, patadM1=250, c=0.4 son iguales en ambos. TODOS los timeouts, tolerancias, kp, g, a, pd y numeros magicos del loop son COMUNES (no dependen del robot).
- El archivo es un firmware UNIFICADO arquero+delantero: el bloque 'DELANTERO' (AVANCE_INICIO, GIRANDO, APUNTAR_PELOTA, AVANZANDO, CENTRANDO_*, PATEANDO_corto/largo, DETECTA_LINEA_*) y el bloque 'ARQUERO' (impulso_inicial, moverce_*, impulso_*, PATEANDO_*_arquero) conviven. El estado inicial impulso_inicial (L131) hace que arranque como ARQUERO; el ciclo arquero NO entra a la rama delantero salvo via PATEANDO_pausa_inicial_arquero -> patada -> avanzar_despues_de_patear -> moverce_derecha (loop arquero cerrado). NOTA: las patadas del arquero (PATEANDO_*_arquero) NO tienen salidas de deteccion de linea, a diferencia de las patadas del delantero (PATEANDO_pausa/adelante/pausa/atras que si chequean s1/s2/s3).
