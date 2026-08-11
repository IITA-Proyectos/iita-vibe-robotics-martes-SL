# 2026-08-11 — Alineación por ángulo, elección de arco y giroscopio

**Quiénes:** Gustavo Viollaz (piso) + Claude (código)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa cargado:** `funciona/delantero/delantero.ino` — **cargado y verificado corriendo**

## Qué queríamos probar

Que patee mejor. Gustavo pidió revisar la lógica de alineación usando de guía el delantero
campeón 2025, y de paso dejar preparadas la elección de arco y el giroscopio.

## El hallazgo: la cuenta de alineación estaba mal

El criterio de patada era:

```cpp
abs(Yp - Yarco) <= 12        // centimetros
```

**Eso resta centímetros medidos a distancias distintas.** `Yp` se mide a la distancia de la
pelota (~17-22 cm) y `Yarco` a la del arco (~100 cm o más). Es restar "3 pasos míos" menos
"3 pasos de un gigante".

**El número que lo muestra:** una pelota con `Yp = 12 cm` a 17 cm de distancia está a **35°**.
Un arco con `Yarco = 12 cm` a 100 cm está a **6,8°**. La cuenta vieja da `|12−12| = 0` y canta
"alineado" cuando en realidad están a **28° uno del otro**.

La solución ya estaba en el código campeón 2025 (`delantero.ino:311-313`) y no la habíamos
copiado:

```cpp
anguloPelota    = atan2(Yp,  Xp)  * 180.0 / PI;
anguloArco_Azul = atan2(Yaz, Xaz) * 180.0 / PI;
```

Un ángulo **no depende de la distancia**. Ahora se comparan ángulos con ángulos.

Y se agregó una segunda condición que el 2025 también tenía (`tolerancia_apuntado = 15°`): la
pelota tiene que estar **adelante**. Si la pelota está a 40° y el arco también, la resta da 0 —
pero el robot al avanzar derecho ni la toca.

## Qué se agregó (tres cosas, una sola activa)

| | Qué | Interruptor | Estado |
|---|---|---|---|
| **A** | Patada por ángulo | — | **ACTIVA** |
| **B** | Elegir arco al encender | `ELEGIR_ARCO_AL_ENCENDER` | apagada |
| **C1** | Patear al rumbo 0 si no ve el arco | `USAR_GIROSCOPO` | apagada |
| **C2** | Orbitar por el camino más corto | `ORBITA_CAMINO_CORTO` | apagada |

Se prenden **de a una**, en ese orden. El procedimiento de cada paso está en el bloque
`COMO PROBAR ESTO` dentro del `.ino`.

**B — el ritual:** se apoya el robot **mirando al arco rival** y se lo enciende. Mira 2 s sin
moverse y se queda con el arco más centrado. El mismo gesto define el "cero" del giroscopio: un
solo ritual, dos funciones.

**C — degradación con gracia:** si el BNO055 no contesta, lo dice y sigue andando como hasta
ayer. La detección de "sensor congelado" (10 lecturas de 0.000 exacto seguidas) es prestada del
`cuadrado-giroscopo` del arquero, que la pagó caro el 28/7.

**Perillas de signo, `[SIN VERIFICAR EN BANCO]`:** no sabemos de qué lado es "Y positivo" ni qué
sentido de giro da `sentidoA = true`. Hay dos booleanos para darlo vuelta —
`SENTIDO_ORBITA_INVERTIDO` y `GIRO_RUMBO_INVERTIDO`— y se prueban en dos intentos cada uno.

## Números del monitor serie (9 s, robot andando)

```
>>> AVANZANDO   Xp=140 Yp=-3  (a  -1.2 grados)  arco AZUL: no lo veo
>>> CENTRANDO   Xp=34  Yp=-11 (a -17.9 grados)  arco AZUL: no lo veo
>>> BUSCANDO    Xp=143 Yp=-23 (a  -9.1 grados)  arco AZUL: no lo veo
*** llegue a 17 cm -> a orbitar
>>> ORBITANDO   Xp=17  Yp=7   (a  22.4 grados)  arco AZUL: no lo veo
>>> AVANZANDO   Xp=57  Yp=23  (a  22.0 grados)
... perdi la pelota orbitando
```

**Dos cosas, y ninguna es la lógica nueva:**

**1. `angArco = --` en TODAS las líneas.** En 9 segundos el arco azul **no se vio ni una vez**.
Mientras eso pase, el robot **no puede patear nunca**, con la cuenta vieja o con la nueva. O sea:
**la mejora A está cargada pero todavía no se pudo evaluar.** Es la calibración de cámara, que
viene postergada desde el 28/7.

**2. Entra a orbitar a 17-20 cm y ahí pierde la pelota.** Las cuatro entradas a `ORBITANDO`
fueron a **17, 20, 19 y 17 cm** — y el piso de la imagen calculado el 28/7 es **17,4 cm**.
Inmediatamente después, la pelota o se pierde o salta a 40-62 cm.

Eso es **consistente** con la sospecha abierta desde la primera clase: la órbita pone la pelota
justo en el borde de lo que la cámara puede ver (`R = 2·L = 17,5 cm` contra un piso de imagen de
17,4 cm). **Sigue sin confirmarse** — falta saber desde dónde mide la homografía, que se
contesta poniendo la pelota a 30 cm exactos y leyendo `Xp`.

## Qué queda pendiente

1. **Calibrar la cámara.** Ya no es "lo primero de la lista": es lo único que destraba lo demás.
   El arco no se ve, y sin arco no hay patada. Herramienta y protocolo en
   [`../vision/`](../vision/).
2. Medir los 30 cm que cierran la duda de la zona ciega.
3. Recién con el arco a la vista, evaluar la patada por ángulo y anotar **a cuántos grados
   pateó**.
4. Después, los pasos 2, 3 y 4 del bloque `COMO PROBAR ESTO`.

**Compila y carga no prueban nada de esto.** Lo único validado hoy es que el firmware corre y
que imprime los ángulos.
