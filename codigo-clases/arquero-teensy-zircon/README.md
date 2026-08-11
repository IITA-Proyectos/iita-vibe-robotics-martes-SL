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
| [`cuadrado-lento/`](pruebas/cuadrado-lento/) | Con el peso encima, ¿anda derecho? (giros por cronómetro) |
| [`cuadrado-giroscopo/`](pruebas/cuadrado-giroscopo/) | Lo mismo con lazo cerrado de rumbo |
| [`calibrar-15cm/`](pruebas/calibrar-15cm/) | ¿Cuánto tiempo de motor es una distancia dada? |
| [`herramientas/`](pruebas/herramientas/) | Compilar, cargar y hablarle al robot **sin el Arduino IDE** |

### `funciona/`

| Programa | Qué hace |
|---|---|
| [`despeje-pelota/`](funciona/despeje-pelota/) | Ve la pelota naranja cerca → salta 30 cm, la saca, vuelve y se endereza |

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

**No hay botón de arranque ni freno.** Los motores salen a andar apenas hay energía, y al cortar
quedan sueltos: el robot sigue de largo. Apoyalo donde querés que arranque **antes** de conectar
la batería.

## Regla de oro

**Nada está validado hasta que alguien lo ve andar.** Anoten los resultados en
[`bitacora/`](bitacora/) — con los números, no con "anduvo".
