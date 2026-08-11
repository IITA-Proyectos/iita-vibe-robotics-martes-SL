# Delantero — punto de partida de la sesión

Robot: el que el equipo llama **"robot 2"**. Es el **DELANTERO** → `#define ROBOT2`.
Worktree: `C:\Users\violl\iita-martes-delantero` · rama `robot/delantero`.

---

## Lo que YA está medido (no lo vuelvas a discutir)

**El mapeo de ruedas.** Medido dos veces en banco el 2026-07-28, con dos pruebas independientes
que coincidieron:

| Pines | Rueda | En el código 2025 es |
|---|---|---|
| **8 / 7 / 6** | IZQUIERDA | `M1` |
| **11 / 12 / 4** | DERECHA | `M2` |
| **2 / 5 / 3** | TRASERA | `M3` |

**Los rótulos izq/der de los comentarios del código 2025 están ESPEJADOS.** `M1` dice *"motor
derecho"* y es el izquierdo; `M2` dice *"izquierdo"* y es el derecho. Fueron escritos mirando al
robot de frente. **No razones sobre izquierda/derecha usando esos comentarios.**

**Cinco cosas del hardware que no están en el código y hay que respetar:**

1. **Para apagar un motor, las dos patas de dirección en 0.** Sólo `PWM = 0` no es confiable.
2. **Las ruedas de adelante están montadas espejadas.** Para avanzar derecho necesitan polaridad
   **opuesta** entre sí. Las tres iguales = el robot rota sobre su eje.
3. **Hay piso de arranque, alrededor de 70 de PWM.** Abajo de eso zumban y no giran.
4. **Para ir lento NO se baja el PWM: se mandan pulsos cortos con pausas**, cada uno por arriba
   del piso. Durante las pausas el robot sigue leyendo la cámara, así que reacciona a tiempo.
5. **La cámara tiene zona muerta justo adelante.** Cuando la pelota se le mete encima manda
   `Xp = 0`, no un número chico. *Perder de vista una pelota que tenías pegada no es perderla:
   es tenerla.*

**Para decidir sobre una medición con ruido, dos umbrales (histéresis).** Con uno solo el robot
entra y sale del estado sin parar y queda temblando.

---

## Lo que NO está confirmado

- **Si `PWM = 0` apaga el motor o no.** Se afirmó que no, pero salió de una rueda mal
  identificada. Hay que re-medirlo limpio. Mientras tanto se apaga con las dos patas en 0, que
  funciona seguro.
- **El piso de PWM real de cada rueda y cada sentido.** Sabemos que ronda 70 porque a 55 no
  arrancaba y a 70 sí, pero los seis números no están medidos.
  [`../../pruebas/piso-de-pwm/`](../../pruebas/piso-de-pwm/) está escrito para eso y nunca se corrió.
- **El signo de `Yp`**: qué valor corresponde a cada lado. Se ajustó por prueba y error con
  `GIRO_INVERTIDO`, no se midió.

---

## 🎥 Lo primero que conviene hacer: recalibrar la cámara

Es el **techo de todo lo demás**. Con la pelota quieta, `Xp` pega saltos no físicos — se vio ir
de 60 a 146 cm — y aparecen los valores de recorte (`Xp = 200`, `Yp = ±100`) que casi siempre son
manchas naranjas del entorno, no la pelota.

Los umbrales de color son de la **luz del laboratorio de 2025**. Mientras sigan así, ningún
ajuste de control va a andar bien: el robot está obedeciendo mediciones falsas.

Se hace con el **OpenMV IDE**: se conecta la cámara, play, y se ve la imagen con los recuadros de
lo que detecta. En diez minutos se ajustan los umbrales a la luz de hoy. El script vive en
[`../../robots-2025/vision-openmv/`](../../../../futbol-roboliga2026/robots-2025/vision-openmv/) y su README explica el
protocolo de 9 bytes.

Probablemente resuelva más que cualquier cosa que se toque en el firmware.

---

## Lo que hace hoy el firmware

Máquina de estados, validada en banco:

```
BUSCANDO → CENTRANDO → AVANZANDO → ORBITANDO → PATEA_ADEL → PATEA_ATRAS → BUSCANDO
```

La **órbita** y el **criterio de patada** no se inventaron: salen del delantero 2025
(`delantero.ino:613-617` y `:621`), que es este mismo robot. La órbita manda las dos de adelante
suave y la trasera fuerte al revés — esa asimetría es lo que hace que rodee la pelota en vez de
girar sobre su eje. El criterio de patada es `|Yp − Yarco| ≤ tolerancia`: pelota y arco en la
misma dirección vistos desde el robot.

### Las perillas, y qué hace cada una

| Perilla | Qué pasa si la subís |
|---|---|
| `TOL_ENTRA` / `TOL_SALE` | más tolerante al ruido, menos temblor, menos precisión |
| `XP_ORBITA` | empieza a orbitar desde más lejos |
| `TOL_ALINEADO` | patea con menos precisión. Subilo si **nunca** patea |
| `MS_ESPERA_*` | gira **más lento** (es la perilla correcta para "más despacio") |
| `VEL_ORB_TRASERA` | orbita **más rápido**. **No cambia el radio** — el radio lo fija la geometría |
| `MS_ORB_IMPULSO` | golpe de arranque más largo. Es la perilla de "no arranca la órbita" |
| `VEL_*` | más fuerza. Desde quieto el piso es ~70; **ya rodando basta ~40** |

> ⚠️ Esta tabla decía antes que `VEL_ORB_TRASERA` cambiaba el tamaño del círculo, y que
> estaba en 130. Las dos cosas quedaron viejas con la órbita pegada del 2026-08-04: el radio
> sale de `R = 2·L` y **no depende de la velocidad**. Corregido el 2026-08-04.

**Órbita, estado al 2026-08-04:** gira bastante bien, pero a veces se descentra y se le escapa
la ventana en la que ve el arco. Se le agregó el **impulso de arranque** del campeón 2025
(golpe 99 durante 300 ms → crucero 48) para poder orbitar por debajo del piso de arranque.
**Falta probarlo en el piso.**

---

---

## 📋 Lo que sigue

Todo lo pendiente, ordenado y con lo que hay que confirmar antes de escribir código:
**[`MEJORAS-PENDIENTES.md`](../../MEJORAS-PENDIENTES.md)**.

Lo primero de la próxima clase: **calibrar la cámara** — no detecta el arco de lejos, y la
pelota da saltos imposibles. Protocolo y herramienta en [`../../vision/`](../../vision/).

---

## Reglas de trabajo

1. **Un cambio por vez.** Si tocás tres cosas y empeora, no sabés cuál fue.
2. **Nada está arreglado hasta que se ve andar.** Que compile no prueba nada.
3. **Una entrada de bitácora por sesión**, en [`../../bitacora/`](../../bitacora/), con los
   números. No "anduvo".
4. **Sólo un robot conectado por vez al cargar** — ver [`../README.md`](../../README.md). El cargador
   no elige placa: podés flashear el arquero sin querer.
5. Esta sesión toca **sólo** `firmware/delantero/`. Lo compartido, con aviso a la otra sesión.
