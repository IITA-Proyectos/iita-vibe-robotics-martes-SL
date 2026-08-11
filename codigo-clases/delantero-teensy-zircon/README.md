# DELANTERO — Teensy 4.1 sobre placa Zircon

Carpeta del **robot delantero** del equipo de fútbol, Roboliga 2026. Todo lo de este robot vive
acá y no se mezcla con lo de la otra mesa.

> 🚨 **El equipo trabaja en DOS mesas a la vez, con DOS robots distintos.** Esta carpeta es la del
> **delantero**. Lo del **arquero** está en
> [`../arquero-teensy-zircon/`](../arquero-teensy-zircon/).
>
> **Los dos robots están cableados distinto** — por eso existe el `#define ROBOT1` / `ROBOT2`.
> Un programa del arquero cargado en el delantero mueve las ruedas equivocadas, compila igual y
> parece que el robot está roto. No está roto: es el `#define`.
>
> Ya pasó una vez que una mesa dio de baja la bitácora de la otra creyendo que se contradecían.
> **Antes de corregir la medición de otro, fijate si está hablando de tu robot.**

## Este robot

| | |
|---|---|
| Puesto | **DELANTERO** — el que el equipo llama *"robot 2"* |
| Se compila con | **`#define ROBOT2`** |
| Placa | Teensy 4.1 sobre Zircon · número de serie **`15708680`** |
| Cámara | OpenMV H7, por `Serial1` a 19200, protocolo de 9 bytes |
| Giroscopio | BNO055 en I2C `0x28` — **el firmware de hoy NO lo lee** |

> El número de serie del Teensy no cambia nunca y no depende del puerto. Con los dos robots
> enchufados, `Get-CimInstance Win32_PnPEntity | Where-Object PNPDeviceID -like "*VID_16C0*"`
> lista los dos y sabés cuál es cuál sin desenchufar nada.

### Mapa de ruedas — MEDIDO en banco, no deducido

| Pines | Driver | Rueda |
|---|---|---|
| 8 / 7 / 6 | U17 | **izquierda** |
| 11 / 12 / 4 | U7 | **derecha** |
| 2 / 5 / 3 | U5 | **trasera** |

⚠️ Los comentarios del código 2025 dicen izquierda/derecha **al revés** — están escritos mirando
al robot de frente en vez de desde el robot. La tabla de arriba es lo medido. Ver
[la bitácora de identificación](bitacora/2026-07-28-identificacion-robot-y-mapeo-ruedas.md).

> Fijate que **no coincide con el arquero**: allá los pines 2/5/3 son la rueda *izquierda* y acá
> son la *trasera*. Eso es exactamente lo que hace el `#define`.

### Cinco cosas del hardware que no están en el código

1. **Para apagar un motor, las dos patas de dirección en 0.** Sólo `PWM = 0` no es confiable.
2. **Las ruedas de adelante están montadas espejadas.** Para avanzar derecho necesitan polaridad
   **opuesta** entre sí. Las tres iguales = el robot rota sobre su eje.
3. **Hay dos pisos de PWM distintos, y la diferencia importa:** desde quieto hace falta ~**70**,
   pero con el robot **ya rodando alcanza ~40**. De ahí sale el impulso de arranque.
4. **`L` = 8,75 cm** del centro del robot al centro de cada rueda. Las tres a 120°.
5. **La cámara tiene zona muerta justo adelante.** Cuando la pelota se le mete encima manda
   `Xp = 0`, no un número chico. *Perder de vista una pelota que tenías pegada no es perderla:
   es tenerla.*

## Cómo está organizado

| Carpeta | Qué hay |
|---|---|
| [`bitacora/`](bitacora/) | Una entrada por clase: qué se probó, qué pasó, y los números |
| [`pruebas/`](pruebas/) | Programas para **medir y diagnosticar**. Responden una pregunta y paran |
| [`funciona/`](funciona/) | Lo que ya **anda de punta a punta** y se puede usar |
| [`vision/`](vision/) | Lo que corre **en la cámara** OpenMV, y cómo calibrarla |
| [`MEJORAS-PENDIENTES.md`](MEJORAS-PENDIENTES.md) | Lo que sigue, ordenado y con lo que hay que confirmar antes |

### `funciona/`

| Programa | Qué hace |
|---|---|
| [`delantero/`](funciona/delantero/) | Busca la pelota → la centra → avanza → **orbita** → patea al arco azul |

### `pruebas/`

| Prueba | Responde |
|---|---|
| [`quien-es-quien/`](pruebas/quien-es-quien/) | ¿Qué pines son qué rueda? Mueve dos y apaga una |
| [`identificar-robot/`](pruebas/identificar-robot/) | ¿Este robot es el arquero o el delantero? |
| [`adelante-atras/`](pruebas/adelante-atras/) | ¿Avanza derecho? (la polaridad espejada de las de adelante) |
| [`tres-ruedas/`](pruebas/tres-ruedas/) | ¿Responden las tres? |
| [`motores-a-mano/`](pruebas/motores-a-mano/) | Manejar cada motor a mano, sin secuencias cronometradas |
| [`diagnostico-motores/`](pruebas/diagnostico-motores/) | Diagnóstico general de los drivers |
| [`piso-de-pwm/`](pruebas/piso-de-pwm/) | ¿Con cuánto PWM arranca cada rueda en cada sentido? **Nunca se corrió** |
| [`buscar-pelota/`](pruebas/buscar-pelota/) | La base de la que salió el firmware vivo |

### `vision/`

| Archivo | Qué es |
|---|---|
| [`calibrar-umbrales.py`](vision/calibrar-umbrales.py) | Herramienta para el **OpenMV IDE**: congela la exposición, mide el color real de la pelota y cuenta cuántas manchas naranjas ve |
| [`README.md`](vision/README.md) | El protocolo de calibración de 6 pasos, y por qué hay que congelar antes de tocar un umbral |

El script que corre hoy **en** la cámara vive en
[`../../futbol-roboliga2026/robots-2025/vision-openmv/`](../../futbol-roboliga2026/robots-2025/vision-openmv/),
que es material 2025 compartido con el arquero. **Antes de tocarlo, avisar a la otra mesa.**

## Cargar un programa

Cada carpeta trae su `platformio.ini`, y el `.ino` se llama igual que la carpeta para que también
se abra tal cual con el **Arduino IDE** (Placa → Teensy 4.1).

Desde la carpeta del programa:

```bash
pio run -e teensy41 -t upload
```

Este robot usa `upload_protocol = teensy-gui`, así que **la app Teensy Loader tiene que estar
abierta** (`~/.platformio/packages/tool-teensy/teensy.exe`). Si no lo está, el cargador se queda
esperando que aprietes el botón de la placa.

> El arquero usa `teensy-cli`, que no necesita la app abierta. Vale la pena probarlo acá alguna
> vez — **pero no el mismo día que se prueba otra cosa.**

## Cuatro cosas que muerden

**🚨 UN SOLO ROBOT CONECTADO POR VEZ.** Ésta es la importante. El cargador **no elige a qué placa
le manda el programa**: agarra la que encuentra. Con los dos robots enchufados, la mesa del
arquero puede flashear el delantero sin enterarse. Antes de cargar:

```bash
pio device list
```

Si aparecen dos, desenchufá uno. No hay forma de elegir desde el `platformio.ini`.

**Cargar un sketch reinicia el Teensy.** Este firmware entra en BUSCANDO 3 segundos después del
reset: si tiene batería y las ruedas en el piso, sale andando solo apenas termina la carga.

**Para probar en el piso, desenchufá el USB después de cargar.** El programa queda en el Teensy y
corre con la batería. Con el cable puesto, el robot lo arranca al moverse.

**Que compile no prueba nada.** Y el `SUCCESS` del cargador tampoco: prueba que el archivo salió,
no que el robot hace lo que querés. Para confirmar qué está corriendo, abrí el monitor a 19200 y
leé el banner de arranque.

## Regla de oro

**Nada está validado hasta que alguien lo ve andar.** Anotá los resultados en
[`bitacora/`](bitacora/) — con los números, no con "anduvo".
