# La cámara OpenMV y cómo le habla al Teensy

La cámara **hace la visión sola**. El Teensy no ve nada: recibe números ya masticados por el
cable serie. Entender este contrato es la mitad de entender el robot.

- Archivo: [`enviar_coordenadas_2_arcos_y_pelota.py`](enviar_coordenadas_2_arcos_y_pelota.py)
- Modelo: **OpenMV H7 Plus** (hay una versión casi igual para la H7 normal en el repo 2025)
- Lenguaje: **MicroPython**, se edita y se carga con el **OpenMV IDE** (no con Arduino IDE)

---

## El protocolo: 9 bytes, siempre

La cámara manda **9 bytes de corrido, sin parar**, por `UART(3)` a **19200 baudios**
(`enviar_coordenadas_2_arcos_y_pelota.py:6`, `:149-155`):

| Byte | Valor | Qué es |
|---|---|---|
| 0 | **201** | marca de inicio de la **pelota** |
| 1 | `Xp` | distancia de la pelota, en cm |
| 2 | `Yp + 100` | desvío lateral de la pelota, en cm, **corrido +100** |
| 3 | **202** | marca del **arco amarillo** |
| 4 | `Xam` | distancia del arco amarillo |
| 5 | `Yam + 100` | desvío lateral del arco amarillo, corrido +100 |
| 6 | **203** | marca del **arco azul** |
| 7 | `Xaz` | distancia del arco azul |
| 8 | `Yaz + 100` | desvío lateral del arco azul, corrido +100 |

### Por qué el +100

Un byte solo puede llevar números de 0 a 255: **no existen los negativos**. Pero el desvío lateral
sí es negativo cuando el objeto está a un lado. La solución del equipo 2025 fue sumarle 100 antes
de mandarlo y restárselo del otro lado. Así, `Y = 0` viaja como `100`, `Y = -30` viaja como `70`.

**Del lado del Teensy hay que restar 100.** Si algún día ves que el robot cree que la pelota está
siempre a la derecha, este es el primer lugar donde mirar.

### Si no ve algo, manda 0

`procesar_blob()` devuelve `0, 0` cuando no encuentra el blob (`:79-81`). O sea: **la cámara nunca
deja de transmitir**. Un `Xp` de 0 no significa "la pelota está pegada al robot", significa
"no la veo". El programa del Teensy tiene que distinguir esos dos casos — fijate cómo lo hace en
el `COMO-FUNCIONA.md` de cada robot.

---

## Cómo detecta los colores

Por **umbrales LAB** (`:58-60`), que es una forma de describir un color que aguanta mejor los
cambios de luz que el RGB común:

```python
naranja_threshold  = (21, 67, 18, 79, -32, 127)   # la pelota
amarillo_threshold = (17, 70, -27, 14, 38, 111)   # un arco
azul_threshold     = (4, 36, -13, 57, -64, -4)    # el otro arco
```

⚠️ **Estos números son de la luz del laboratorio de 2025.** La luz de la cancha de la Roboliga
va a ser distinta y muy probablemente haya que recalibrarlos. Es normal, no es que se rompió nada.
En el repo 2025 hay un `Calibrar_Treshold.py` para eso.

Después de encontrar los blobs, se queda con **el más grande de cada color** (`:84`) y convierte
el pixel a centímetros con una **matriz homográfica** (`:65-76`) — una tabla de conversión que
compensa que la cámara mira en diagonal, no desde arriba. Esa matriz está calibrada para la
**altura de cámara de 18,7 cm** (`:48`). Si movés la cámara de lugar, la matriz deja de servir.

---

## Los LEDs son tu diagnóstico gratis

La cámara enciende un LED por cada cosa que ve (`:124-135`):

| LED | Significa |
|---|---|
| 🔴 rojo | está viendo la **pelota** |
| 🟢 verde | está viendo el **arco amarillo** |
| 🔵 azul | está viendo el **arco azul** |

Antes de debuggear el Teensy, mirá los LEDs de la cámara. Si están apagados, el problema es de
visión (luz, umbrales, la cámara tapada) y no del programa del robot.

---

## Cuidado con esto

- **`pyb.LED` y `UART(3)`** son de la OpenMV H7. Si algún día se cambia la cámara por una N6,
  este código **no arranca**. (El equipo de Incheon ya pasó por eso.)
- El protocolo **no tiene forma de recuperarse si se desincroniza**: no lleva verificación de
  errores ni marca de fin. Si en el cable se pierde un byte, el Teensy puede quedar leyendo
  corrido y creer cualquier cosa. Está anotado como problema conocido.
- La cámara arranca con **balance de blancos y ganancia automáticos** (`:31-32`) y espera 2
  segundos (`:36`). Si la encendés apuntando a algo raro, se calibra mal. Encendela mirando la
  cancha.
