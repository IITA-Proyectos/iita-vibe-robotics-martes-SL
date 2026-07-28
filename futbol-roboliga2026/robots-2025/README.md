# Los robots del Nacional 2025 — tal cual están

Esta carpeta es **la foto** de los dos robots con los que se ganó el Nacional 2025.
No se edita. Si querés probar un cambio, copiá el archivo a tu carpeta `alumnos/<tunombre>/`.

---

## Qué hay

| Archivo | Qué es | Líneas |
|---|---|---|
| [`mapa-pines-teensy.md`](mapa-pines-teensy.md) | ⭐ Qué pin del Teensy va a qué cosa, en los dos robots | — |
| [`arquero/arquero-2025.ino`](arquero/arquero-2025.ino) | El programa del arquero | 1207 |
| [`arquero/COMO-FUNCIONA.md`](arquero/COMO-FUNCIONA.md) | Qué hace, línea por línea: estados, tiempos, PWM, umbrales | — |
| [`delantero/delantero-2025.ino`](delantero/delantero-2025.ino) | El programa del delantero | 1214 |
| [`delantero/COMO-FUNCIONA.md`](delantero/COMO-FUNCIONA.md) | Ídem para el delantero | — |
| [`delantero/variantes/delantero-sin-zirconlib.ino`](delantero/variantes/delantero-sin-zirconlib.ino) | Versión del delantero **sin la librería Zircon** — ver abajo, puede salvarte | — |
| [`vision-openmv/`](vision-openmv/) | El programa de la cámara y el protocolo con el Teensy | 116 |
| [`libreria-zircon/`](libreria-zircon/) | La librería de la placa. ⚠️ **No compila tal cual** | 356 |

---

## ⭐ La variante sin librería puede ser tu atajo

`libreria-zircon/zirconLib.cpp` **no compila** (tiene una llave de más, ver su README). Eso puede
frenarte el primer día.

`delantero/variantes/delantero-sin-zirconlib.ino` es una versión del delantero que **no usa la
librería**: lee los sensores de línea con `analogRead()` directo y maneja los pines a mano.
Si te trabás con la librería, arrancá por ahí para tener el robot moviéndose, y volvé después.

> Ojo: esta variante **no** está en el repo original de 2025 — apareció en el repo del equipo de
> Incheon. No sabemos si llegó a correr en un robot. Tratala como punto de partida, no como
> código probado.

---

## Procedencia — de dónde salió cada archivo

Copiado el **28-jul-2026**. Los **programas** son copias **byte a byte**: se verificó el hash de
git de cada archivo copiado contra el blob del repo de origen, y coinciden exactamente.

| Archivo acá | Hash git (SHA-1) | Origen |
|---|---|---|
| `arquero/arquero-2025.ino` | `8a8453c8941aaf97ef8c93fbbc2f4ba03b075be1` | repo 2025 → `ARQUERO/6-9-2026 viernes` |
| `delantero/delantero-2025.ino` | `528bdbc997de6d09ff4c57d3524edba75c69945c` | repo 2025 → `DELANTERO/bueno/un-solo-programa/DEFINITIVO` |
| `vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py` | `17ea8419f5d4b70faa7e7a930388674403d1899e` | repo 2025 → `OpenMV/H7 plus/enviar coordenadas 2 arcos y pelota` |
| `libreria-zircon/zirconLib.cpp` | `9712b73fb572f215eaf0f30e1f4cb2b1840452de` | repo 2025 → `Librerias/zirconLib/zirconLib.cpp` |
| `libreria-zircon/zirconLib.h` | `7f9de772495a82dc51c0b872ab755c5b78355146` | repo 2025 → `Librerias/zirconLib/zirconLib.h` |
| `delantero/variantes/delantero-sin-zirconlib.ino` | `bed9fc0307363a7848a6abe25fb457e0c0b6585c` | repo 2026 (no existe en el de 2025) |

**Lo único que cambió es el nombre del archivo.** Los originales no tenían extensión
(`DEFINITIVO`, `6-9-2026 viernes`), así que ni el editor ni la IA los reconocían como C++.
Les pusimos `.ino`. El contenido es idéntico: por eso **los números de línea del
`COMO-FUNCIONA.md` coinciden exactamente** con estos archivos (verificado: L10 `#define ROBOT1`,
L131 `Estado estado = impulso_inicial;`, L1016 `case impulso_inicial:`, L1184
`case PATEANDO_atras_arquero:`).

Los **documentos** (`COMO-FUNCIONA.md`, `mapa-pines-teensy.md`) sí se tocaron: se corrigieron las
rutas, que apuntaban a carpetas del otro repo que acá no existen. Cada corrección está marcada
en el propio archivo.

Repos de origen:
[RoboCupJunior-Soccer-Open-League-2025](https://github.com/IITA-Proyectos/RoboCupJunior-Soccer-Open-League-2025)
·
[open-soccer-robocup-team2026](https://github.com/IITA-Proyectos/open-soccer-robocup-team2026)

### Cómo comprobar vos mismo que la copia es fiel

```bash
git hash-object futbol-roboliga2026/robots-2025/arquero/arquero-2025.ino
```

Tiene que devolver `8a8453c8941aaf97ef8c93fbbc2f4ba03b075be1`. Si no coincide, alguien editó el
archivo (y no debía).

---

## El hardware, en dos líneas

**Teensy 4.1** (procesador ARM a 600 MHz) montado sobre una **placa Zircon Rev v15** (una PCB
comercial de Robomov que trae los drivers de motor y los conectores). Encima, una **cámara
OpenMV H7** que hace la visión por su cuenta y le manda coordenadas al Teensy por cable serie.

- **3 motores** en configuración omni (el robot se mueve en cualquier dirección sin girar).
- **3 sensores de línea** analógicos, para no salirse de la cancha.
- **8 sensores infrarrojos** de pelota (TSSP58038) alrededor del robot.
- **1 giroscopio BNO055** por I²C, para saber hacia dónde está mirando.
- Kicker: **no hay solenoide**. La "patada" es un avance fuerte de las ruedas.

Los pines exactos están en [`mapa-pines-teensy.md`](mapa-pines-teensy.md). **Los motores están
cableados distinto en cada robot** — por eso existe el `#define ROBOT1` / `ROBOT2`. Está explicado
en la tabla de equivalencias de ese archivo.

---

## Sobre el giroscopio: SÍ se usaba

Vas a encontrar documentación vieja del equipo anterior que dice que el giroscopio estaba
"deshabilitado en 2025" o que "ganaron sin IMU". **Es falso.** El BNO055 se inicializa, se lee y
participa del control — está verificado contra el código, con números de línea, en los
`COMO-FUNCIONA.md` de cada robot. Los detalles finos (dónde entra al control y dónde no) están
ahí. Si ves un doc que dice lo contrario, es un error heredado: corregilo o avisá.
