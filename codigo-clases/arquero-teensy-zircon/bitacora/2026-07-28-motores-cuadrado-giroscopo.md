# 2026-07-28 — Motores en banco, cuadrado en el piso y lazo cerrado con giroscopo

**Quiénes:** Gustavo Viollaz (banco y piso) + Claude (código, carga y lectura del serie)
**Robot:** **arquero** (el que el equipo llama "robot 2") — ver
[2026-07-28-identificacion-arquero.md](2026-07-28-identificacion-arquero.md)
**Programas cargados, en orden:** `pruebas/motores-a-mano` → `pruebas/cuadrado-lento` →
`pruebas/cuadrado-giroscopo`. Todos compilados para ROBOT1 (mapeo del arquero).

> ⚠️ **El delantero no se tocó.** Todo lo de abajo es del arquero. El objetivo original de la
> clase era programar el delantero y ese robot ni siquiera vino al banco.

---

## Qué queríamos probar

Tres cosas, en este orden: (1) si los tres motores responden, para cerrar el bug anotado de la
rueda trasera; (2) si el robot anda derecho **con el peso encima**, que es lo que la prueba de
banco no puede decir; (3) si cerrando el lazo con el giroscopio los giros salen de 90° de verdad.

---

## Qué pasó de verdad

### 1. Los tres motores andan (ruedas al aire)

Probados uno por uno, en los dos sentidos, con `motores-a-mano`:

| Driver | Pines | Rueda (medida) | Sentido A | Sentido B |
|---|---|---|---|---|
| U5 | 2/5/3 | izquierda | ✅ gira | ✅ gira, al revés |
| U17 | 8/7/6 | derecha | ✅ gira | ✅ gira, al revés |
| U7 | 11/12/4 | trasera | ✅ gira | ✅ gira, al revés |

**No se reprodujo el bug de la trasera.** A PWM 100 los tres drivers responden bien.

### 2. El "no frena" NO es un bug — es de diseño

Observado por Gustavo: al mandar parar, las ruedas *"no frenan de inmediato, parecen únicamente
apagarse"*. Es correcto: `parar()` deja las dos patas de dirección en 0, que deja el motor
suelto (coast). **No hay freno activo en ninguna parte de este firmware**, ni en las pruebas ni
en el código de competencia. Las tres ruedas hacen lo mismo porque tienen que hacer lo mismo.

**Conclusión del equipo:** si hay un problema de movimiento, es mecánico (construcción), no
eléctrico ni de firmware.

### 3. Cuadrado por cronómetro: los giros salían de 160°

Con `cuadrado-lento` en el piso, `velAvance=70`, `msLado=1500`, `msGiro=700`:

- **avanza bien** con carga, las tres ruedas arrancan;
- **los giros salieron de ~160° en vez de 90°.**

Regla de tres → 700·90/160 ≈ 394, se dejó en **400 ms**. **Ese valor quedó SIN PROBAR**: antes
de correrlo se pasó al sketch con giroscopio, que ataca el problema de raíz.

### 4. El giroscopio cuenta positivo en sentido HORARIO

Medido a mano, robot quieto sobre la mesa, girándolo un cuarto de vuelta a la derecha:

| | Rumbo |
|---|---|
| antes | 359,9° |
| después de girar a la derecha | 99,7° |

**El número sube girando a la derecha.** No se dedujo: se midió.

### 5. El lazo cerrado FUNCIONA — y con el signo correcto de entrada

Primera corrida de `cuadrado-giroscopo`, lados de 300 ms, rumbo de referencia 323,6°:

| Giro | Objetivo | Qué hizo | Error final |
|---|---|---|---|
| 1º | 53,6° | se pasó 25,1° → corrigió 2 veces | **0,1°** |
| 2º | 143,6° | llegó sin corregir | **−1,5°** |
| 3º | 233,6° | no llegó (ver abajo) | — |

**El primer giro es el que vale la pena mirar: se pasó 25 grados y volvió.** Eso es exactamente
lo que el cronómetro no puede hacer. De 160° de error a 1,5°.

**No hizo falta invertir ningún signo** (`giroInvertido=false`, `correccionInvertida=false`).
Esos son los valores buenos, ya están puestos por defecto en el sketch.

### 6. Lo que quedó abierto: el BNO055 leyendo 0.0

En el 3er giro el rumbo pasó a leer **exactamente 0.0** y ahí se quedó — incluso con el robot
parado, tres lecturas seguidas separadas por segundos. La librería Adafruit devuelve ceros
cuando no puede leer el chip.

🚨 **DATO CONTAMINADO, NO CONCLUIR NADA TODAVÍA.** A mitad de la corrida el robot se enredó con
el cable USB y Gustavo **lo levantó**. Con las ruedas al aire el robot no gira aunque los
motores anden → el rumbo no cambia → el giro se rinde a los 5 s. **El "no llegó" está
explicado por el levantón.**

Lo que ese levantón **no** explica es por qué el sensor quedó clavado en 0.0 estando quieto.
Puede ser que el tirón haya movido un cable del I2C. **Quedó sin verificar.**

---

## Números para no volver a medirlos

| Qué | Valor |
|---|---|
| Giro por cronómetro | 700 ms → **160°** (arquero, ese piso, esa batería) |
| `msGiro` estimado para 90° | 400 ms — **sin probar** |
| Giroscopio | positivo = **horario** |
| `giroInvertido` / `correccionInvertida` | **false / false** (correctos) |
| Error de giro con lazo cerrado | **0,1° y −1,5°** |
| Velocidad de avance usada | PWM 70 (competencia usa 100) |
| Motores, ruedas al aire | los 3 drivers OK en ambos sentidos, PWM 100 |
| Puerto del Teensy | COM5 · `teensy:avr:teensy41` |

---

## Qué queda pendiente

### Lo primero el martes que viene, en este orden

1. **¿El BNO055 revivió?** Prender el robot y leer el rumbo (tecla `h` de
   `cuadrado-giroscopo`). Girarlo a mano y ver si el número cambia.
   - Si **cambia** → fue el tirón del cable. Revisar igual que el conector de los pines 18/19
     esté firme.
   - Si sigue en **0.0** → el sensor no arranca. Ahí sí es un problema serio, porque el
     firmware de competencia depende del mismo chip.
2. **Separar vibración de consumo eléctrico.** Si el sensor se vuelve a caer: hacer andar los
   motores con las **ruedas al aire** y mirar el rumbo. Si al aire aguanta y solo se cae en el
   piso → vibración/cable flojo. Si también se cae al aire → es el consumo de los motores
   metiendo ruido o bajando la tensión. Son arreglos distintos.
3. **Correr el cuadrado con giroscopio entero, sin cable.** Ya está listo para eso: apoyar,
   prender la batería, 10 s de LED parpadeando, y arranca. Falta subir `msLado` de 300 (lo
   dejamos corto para calibrar cerca de la compu) a 1500.

### Deudas del sketch

- ⬜ **`cuadrado-giroscopo` no detecta que el sensor se cayó a mitad de camino.** Chequea el
  BNO055 solo al encender. Si se cae después, sigue girando con datos basura. Hay que agregarle
  detección + frenar (y ojo: **el código de competencia tiene el mismo agujero, peor** — si el
  BNO falla al arrancar se cuelga en un `while(1)` mudo y el robot no hace nada, sin avisar).
- ⬜ Los valores ajustados con las teclas **se pierden al cortar la energía**. Cuando encontremos
  los buenos hay que escribirlos en el sketch y recompilar.

### Del bug original

- ⬜ **El caso PWM=0 nunca se probó.** El bug anotado decía *"la trasera gira teniendo PWM 0"*.
  Lo que probamos fue PWM 100 y PWM 40. Para cerrarlo de verdad hay que bajar la potencia a 0 y
  pedir la trasera: si gira con PWM 0, el driver ignora el PWM. Es una tecla.
- ⬜ **Todo se midió sin carga o en un solo piso.** Un motor que anda perfecto en el aire puede
  patinar o rozar con el peso encima.

### Lo grande

- ⬜ **El delantero sigue sin tocarse.** Cuando venga al banco: según la bitácora anterior, hay
  que **observarlo andar antes de flashearlo** si se quiere conservar la evidencia de qué corrió
  en el Nacional 2025 (la memoria del Teensy no se puede leer de vuelta). El arquero barre de
  lado a lado al encender; el delantero avanza y después gira sobre su eje.
- ⬜ **Sin OpenMV IDE no hay visión.** No está instalado. Sin la cámara mandando por `Serial1`,
  el delantero gira buscando la pelota para siempre.

---

## Herramientas que quedaron armadas

Toda la clase se trabajó **sin abrir el Arduino IDE**: compilar, cargar, mandar teclas y leer
respuestas, todo por consola. Los scripts quedaron en
[`../pruebas/herramientas/`](../pruebas/herramientas/) con las rutas de la compu del taller
anotadas.

También quedó instalada `zirconLib` **con los dos parches aplicados** en
`Documents\Arduino\libraries\zirconLib\` (marcados con el comentario `PARCHE IITA 2026-07-28`).
Con eso `delantero.ino` y `arquero.ino` **compilan los dos**, cosa que antes no pasaba.

> 🚨 **Cargar un sketch reinicia el Teensy.** Los sketches que arrancan solos empiezan la cuenta
> regresiva apenas termina la carga. Nos pasó: hubo que frenarlo a mano. Cargar con la batería
> apagada o con el robot en el piso.
