# Los robots del Nacional 2025 — tal cual están

Esta carpeta es **la foto** de los dos robots con los que se ganó el Nacional 2025.
No se edita. Si querés probar un cambio, copiá el archivo a tu carpeta `alumnos/<tunombre>/`.

---

## Qué hay

| Archivo | Qué es | Líneas |
|---|---|---|
| [`mapa-pines-teensy.md`](mapa-pines-teensy.md) | ⭐ Qué pin del Teensy va a qué cosa, en los dos robots | — |
| [`arquero/arquero.ino`](arquero/arquero.ino) | El programa del arquero | 1207 |
| [`arquero/COMO-FUNCIONA.md`](arquero/COMO-FUNCIONA.md) | Qué hace, línea por línea: estados, tiempos, PWM, umbrales | — |
| [`delantero/delantero.ino`](delantero/delantero.ino) | El programa del delantero | 1214 |
| [`delantero/COMO-FUNCIONA.md`](delantero/COMO-FUNCIONA.md) | Ídem para el delantero | — |
| [`delantero/sin-zirconlib/sin-zirconlib.ino`](delantero/sin-zirconlib/sin-zirconlib.ino) | Versión del delantero **sin la librería Zircon** — ver abajo, puede salvarte | — |
| [`vision-openmv/`](vision-openmv/) | El programa de la cámara y el protocolo con el Teensy | 116 |
| [`libreria-zircon/`](libreria-zircon/) | La librería de la placa. ⚠️ **No compila tal cual** | 356 |

---

## ⭐ La variante sin librería puede ser tu atajo

`libreria-zircon/zirconLib.cpp` **no compila** (tiene una llave de más, ver su README). Eso puede
frenarte el primer día.

`delantero/sin-zirconlib/sin-zirconlib.ino` es una versión del delantero que **no usa la
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
| `arquero/arquero.ino` | `8a8453c8941aaf97ef8c93fbbc2f4ba03b075be1` | repo 2025 → `ARQUERO/6-9-2026 viernes` |
| `delantero/delantero.ino` | `528bdbc997de6d09ff4c57d3524edba75c69945c` | repo 2025 → `DELANTERO/bueno/un-solo-programa/DEFINITIVO` |
| `vision-openmv/enviar_coordenadas_2_arcos_y_pelota.py` | `17ea8419f5d4b70faa7e7a930388674403d1899e` | repo 2025 → `OpenMV/H7 plus/enviar coordenadas 2 arcos y pelota` |
| `libreria-zircon/zirconLib.cpp` | `9712b73fb572f215eaf0f30e1f4cb2b1840452de` | repo 2025 → `Librerias/zirconLib/zirconLib.cpp` |
| `libreria-zircon/zirconLib.h` | `7f9de772495a82dc51c0b872ab755c5b78355146` | repo 2025 → `Librerias/zirconLib/zirconLib.h` |
| `delantero/sin-zirconlib/sin-zirconlib.ino` | `bed9fc0307363a7848a6abe25fb457e0c0b6585c` | repo 2026 (no existe en el de 2025) |

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
git hash-object futbol-roboliga2026/robots-2025/arquero/arquero.ino
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

## Sobre el giroscopio: SÍ se usaba (y por qué hay docs que dicen lo contrario)

Vas a encontrar documentación vieja del equipo anterior que dice que el giroscopio estaba
"deshabilitado en 2025" o que "ganaron sin IMU". **Eso es incorrecto sobre los robots** — pero
tiene una explicación, y entenderla te enseña algo útil sobre este código.

**Hay DOS caminos al giroscopio, y solo uno está vivo:**

| Camino | Estado | Detalle |
|---|---|---|
| El de la **librería** (`readCompass()`) | 💀 **realmente apagado** | `CalibrateCompass()` está enteramente comentada, el flag de calibración nunca se pone en true, y `readCompass()` **siempre devuelve 0** |
| El de los **programas** (`.ino`) | ✅ **vivo y obligatorio** | Cada programa crea su **propio** objeto del BNO055 y habla directo con el sensor, salteando la librería |

Ninguno de los dos programas llama nunca a `readCompass()` — cero apariciones en las 1207 y 1214
líneas. **Quien miró solo la librería concluyó "estaba deshabilitado" y tenía razón sobre la
librería. Quien miró los programas dijo "se usaba" y tenía razón sobre los robots.** Los docs
viejos no inventaron nada: describían otro archivo. Por eso se corrigen, no se borran.

### Qué significa exactamente "se usaba"

No había control PID de rumbo. Lo que había es más simple, y distinto en cada robot:

- **Arquero** — el error de rumbo **sí llega a los motores**. Elige entre **tres repartos fijos**
  de potencia entre las tres ruedas (uno para "derecho", uno para cada lado) y así compensa el
  giro parásito mientras se desplaza de costado. Es un *heading-hold de 3 escalones*, no un
  proporcional, pese a que la función se llame `adproporcional()`.
- **Delantero** — el error de rumbo **nunca toca un PWM**. Solo aparece en condiciones que
  **cambian de estado**: decide **cuándo patear** y, al pisar una línea blanca, si patea o si
  sigue orbitando (una salvaguarda **anti-autogol**).

Y hay un detalle honesto que conviene saber: la única cuenta verdaderamente proporcional del
programa, `correccion = error * kp`, **se calcula en cada vuelta del loop y nunca se lee**. Ese
sí es, textualmente, el caso de "se calcula pero no se usa". Alguien empezó un control
proporcional de rumbo, dejó la cuenta hecha, y nunca la conectó a los motores.

Si el BNO055 no responde al arrancar, **el programa se cuelga a propósito** y el robot no hace
nada. Ver [`../bugs-conocidos.md`](../bugs-conocidos.md) A5 y DEL-03.
