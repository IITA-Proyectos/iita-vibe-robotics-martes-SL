# ARQUERO — Teensy 4.1 sobre placa Zircon

Carpeta del **robot arquero** del equipo de fútbol, Roboliga 2026. Todo lo de este robot vive
acá y no se mezcla con lo de las otras mesas.

> 🚨 **El equipo trabaja en DOS mesas a la vez, con DOS robots distintos.** Esta carpeta es la del
> **arquero**. Lo del **delantero** está en
> [`../delantero-teensy-zircon/`](../delantero-teensy-zircon/).
>
> **Los dos robots están cableados distinto** — por eso existe el `#define ROBOT1` / `ROBOT2`.
> Un programa del delantero cargado en el arquero mueve las ruedas equivocadas, compila igual y
> parece que el robot está roto. No está roto: es el `#define`.
>
> Ya pasó una vez que una mesa dio de baja la bitácora de la otra creyendo que se contradecían.
> **Antes de corregir la medición de otro, fijate si está hablando de tu robot.**

## Este robot

| | |
|---|---|
| Puesto | **ARQUERO** |
| Se compila con | **`#define ROBOT1`** |
| Placa | Teensy 4.1 sobre Zircon Rev v15 |
| Cámara | OpenMV H7, por `Serial1` (pines 0 y 1) a 19200 |
| Giroscopio | BNO055 en I2C `0x28` (pines 18 y 19) |

### Mapa de ruedas — MEDIDO en banco, no deducido

| Pines | Driver | Rueda |
|---|---|---|
| 2 / 5 / 3 | U5 | **izquierda** |
| 8 / 7 / 6 | U17 | **derecha** |
| 11 / 12 / 4 | U7 | **trasera** |

### Mapa de sensores de línea — MEDIDO en banco

| Pin | Posición |
|---|---|
| A12 | **adelante** |
| A13 | **atrás izquierda** |
| A11 | **atrás derecha** |

Más claro = número más alto. En cancha: verde 356 y 465, línea blanca ~760,
**umbral 620**. Los dos de atrás no son iguales entre sí — 110 puntos de
diferencia sobre el mismo verde. Ver
[la bitácora del 11/08](bitacora/2026-08-11-sensores-de-linea-y-arquero-completo.md).

⚠️ Los comentarios del código 2025 dicen izquierda/derecha **al revés** — están escritos mirando
al robot de frente en vez de desde el robot. La tabla de arriba es lo medido. Ver
[la bitácora de identificación](bitacora/2026-07-28-identificacion-arquero.md).

## Cómo está organizado

| Carpeta | Qué hay |
|---|---|
| [`bitacora/`](bitacora/) | Una entrada por clase: qué se probó, qué pasó, y los números |
| [`pruebas/`](pruebas/) | Programas para **medir y diagnosticar**. Responden una pregunta y paran |
| [`funciona/`](funciona/) | Lo que ya **anda de punta a punta** y se puede usar |

### `pruebas/`

| Prueba | Responde |
|---|---|
| [`ver-camara/`](pruebas/ver-camara/) | ¿La cámara está hablando? ¿Ve la pelota, y a qué distancia? |
| [`ver-linea/`](pruebas/ver-linea/) | Los tres sensores de línea en vivo. Cuál es cuál |
| [`buscar-sensores/`](pruebas/buscar-sensores/) | ¿En qué pines están los sensores? Lee las 18 entradas a la vez |
| [`calibrar-linea/`](pruebas/calibrar-linea/) | Verde vs. blanco en la cancha, sin cable. Calcula el umbral |
| [`probar-freno/`](pruebas/probar-freno/) | ¿Esta placa puede frenar los motores, o solo soltarlos? |
| [`cuadrado-lento/`](pruebas/cuadrado-lento/) | Con el peso encima, ¿anda derecho? (giros por cronómetro) |
| [`cuadrado-giroscopo/`](pruebas/cuadrado-giroscopo/) | Lo mismo con lazo cerrado de rumbo |
| [`calibrar-15cm/`](pruebas/calibrar-15cm/) | ¿Cuánto tiempo de motor es una distancia dada? |
| [`herramientas/`](pruebas/herramientas/) | Compilar, cargar y hablarle al robot **sin el Arduino IDE** |

### `funciona/`

| Programa | Qué hace |
|---|---|
| [`seguir-y-despejar/`](funciona/seguir-y-despejar/) | ⭐ **El arquero completo.** Sigue la pelota de costado sin dejar de mirar al frente, y la despeja cuando se acerca |
| [`despeje-pelota/`](funciona/despeje-pelota/) | Solo el despeje, sin seguimiento. Más simple para probar de a una cosa |

## Cargar un programa

Cada carpeta trae su `platformio.ini`, y el `.ino` se llama igual que la carpeta para que también
se abra tal cual con el **Arduino IDE** (Placa → Teensy 4.1).

Desde la carpeta del programa:

```bash
pio run -e teensy41 -t upload
```

O sin abrir ningún IDE, con los scripts de [`pruebas/herramientas/`](pruebas/herramientas/).

## Tres cosas que muerden

**Cargar un sketch reinicia el Teensy.** Los programas que arrancan solos empiezan su cuenta
regresiva apenas termina la carga. Cargá con la batería apagada, o frená en el acto.

**El giroscopio se alimenta de la batería, no del USB.** Con la batería apagada contesta que
existe pero devuelve puros ceros. Si ves rumbo `0.0`, lo primero que se revisa es la batería.

**No hay botón de arranque.** Los motores salen a andar apenas hay energía. Apoyalo donde querés
que arranque **antes** de conectar la batería.

**Sí hay freno, pero hay que pedirlo.** `parar()` sólo suelta los motores y el robot sigue de
largo; `frenar()` los cortocircuita y lo detiene en el acto. Medido el 18/08: las dos formas de
pedir el freno funcionan en esta placa. Ojo que **la que usamos se parece a soltar** — mismo
estado de las patas de dirección, y la diferencia está en el PWM.

## Regla de oro

**Nada está validado hasta que alguien lo ve andar.** Anoten los resultados en
[`bitacora/`](bitacora/) — con los números, no con "anduvo".
