# Pruebas de banco

Programas cortos para **medir, diagnosticar y probar comportamientos**. Cada uno responde una
pregunta concreta.

> 🖱️ **La forma fácil de correrlas: doble clic en [`../MEDIR-ROBOT.bat`](../MEDIR-ROBOT.bat).**
> Menú con todas, avisa cuál necesita cable, verifica que el robot enchufado sea el delantero,
> y con la opción **L** lee lo que el robot tiene para decir al volver de la cancha.
> *(Para jugar es [`../CARGAR-ROBOT.bat`](../CARGAR-ROBOT.bat), que es otro y no se toca.)*

### 🟩 Línea blanca

| Prueba | Responde / hace | cable |
|---|---|---|
| [`grabar-linea/`](grabar-linea/) | ⭐ **La de siempre.** Verde, blanco y negro; clasifica las mesetas solo y propone los 3 umbrales | **no** |
| [`identificar-sensores/`](identificar-sensores/) | ¿Qué número de sensor es cada posición física? | sí |

### 🧭 Giroscopio

| Prueba | Responde / hace | cable |
|---|---|---|
| [`giroscopo-crudo/`](giroscopo-crudo/) | ¿Qué chip es y corre la fusión? El que descubrió que nunca estuvo roto | sí |
| [`rumbo-vivo/`](rumbo-vivo/) | El rumbo en vivo, sin mover motores. El de **un minuto** | sí |
| [`giroscopo-recupera/`](giroscopo-recupera/) | ¿El lazo recupera de un empujón? | sí |
| [`signos/`](signos/) | Los tres signos: giro, órbita y cámara | sí |

### ⚙️ Motores

| Prueba | Responde / hace | cable |
|---|---|---|
| [`piso-de-pwm/`](piso-de-pwm/) | ⭐ **A partir de qué PWM arranca cada rueda.** 🐛 tiene los pines del **ARQUERO** | sí |
| [`quien-es-quien/`](quien-es-quien/) | ¿Qué rueda cuelga de qué pines? Mueve dos y apaga una | sí |
| [`motores-a-mano/`](motores-a-mano/) | Escribís una tecla y ese motor arranca | sí |
| [`diagnostico-motores/`](diagnostico-motores/) | Los 3 motores en los 2 sentidos, paso a paso | sí |

### 📷 Cámara, patada y otros

| Prueba | Responde / hace | cable |
|---|---|---|
| [`tabla-camara/`](tabla-camara/) | ⭐ ¿Cuántos cm es un `Xp`? Cierra lo de `XP_ORBITA` | sí |
| [`patada-derecha/`](patada-derecha/) | ¿Cuántos grados se tuerce al patear? ⚠️ mide 240×1000 y hoy patea 215×420 | sí |
| [`identificar-robot/`](identificar-robot/) | ¿Este robot es el arquero o el delantero? | sí |

> 🗑️ **Borrados el 2026-09-22.** `grabar-verde`, `medir-verde`, `calibrar-linea` y
> `sensores-de-linea` quedaron reemplazados por `grabar-linea`, que hace todo eso y clasifica
> solo. `adelante-atras` y `tres-ruedas` los cubre `diagnostico-motores`. `buscar-pelota` era
> una foto congelada del firmware del 28/07.
> **Siguen en el historial de git**: `git log --diff-filter=D -- pruebas/` los encuentra.

## Cómo se cargan

Cada carpeta trae su `platformio.ini`. Desde la carpeta de la prueba:

```bash
pio run -e teensy41 -t upload
```

También se abren tal cual con el **Arduino IDE** (Placa → Teensy 4.1). La carpeta y el `.ino` se
llaman igual justamente para eso.

## Cosas que aprendimos probando (2026-07-28)

Ninguna de estas está en el código 2025 ni se podía deducir leyéndolo. Salieron del banco.

**Diseñá el test alrededor de la pregunta más fácil de contestar.** "¿Cuál falta?" es mucho más
confiable que "¿cuál es?". Las pruebas de una-rueda-por-vez cronometradas dieron resultados
contradictorios; las de apagar-una-y-mirar dieron el mapeo a la primera.

**Para apagar un motor, las dos patas de dirección en 0.** Poner solo `PWM = 0` no es confiable
en esta placa.

**Las dos ruedas de adelante están montadas espejadas.** Para avanzar derecho necesitan polaridad
**opuesta** entre sí. Las tres iguales = el robot rota.

**Hay piso de arranque.** Abajo de ~70 de PWM las ruedas zumban y no giran. Para ir **lento no se
baja el PWM**: se mandan **pulsos cortos con pausas**, cada uno por arriba del piso.

**Para decidir sobre una medición con ruido, usá dos umbrales (histéresis).** Con uno solo el
robot entra y sale del estado sin parar y queda temblando.

**La cámara tiene zona muerta justo adelante.** Cuando la pelota se le mete encima manda 0, no un
número chico. Perder de vista una pelota que tenías pegada no es perderla: es tenerla.

## Regla de oro

**Nada de esto está validado hasta que alguien lo ve andar.** Anoten los resultados en
[`../bitacora/`](../bitacora/) — con los números, no con "anduvo".
