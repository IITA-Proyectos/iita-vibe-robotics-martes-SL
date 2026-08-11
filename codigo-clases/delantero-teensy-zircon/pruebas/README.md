# Pruebas de banco

Programas cortos para **medir, diagnosticar y probar comportamientos**. Cada uno responde una
pregunta concreta.

Ninguno usa `zirconLib` ni el giroscopio, a propósito: así compilan y corren siempre, sin
depender de los dos problemas P0 del código de competencia (ver
[`../bugs-conocidos.md`](../../../futbol-roboliga2026/bugs-conocidos.md)).

| Prueba | Responde / hace |
|---|---|
| [`quien-es-quien/`](quien-es-quien/) | ⭐ **¿Qué rueda cuelga de qué pines?** Mueve dos y apaga una: se ve cuál queda quieta |
| [`identificar-robot/`](identificar-robot/) | ¿Este robot es el arquero o el delantero? |
| [`tres-ruedas/`](tres-ruedas/) | Las tres ruedas para un lado y para el otro. Test más simple posible |
| [`motores-a-mano/`](motores-a-mano/) | Prueba interactiva: escribís una tecla y ese motor arranca |
| [`diagnostico-motores/`](diagnostico-motores/) | Los 3 motores en los 2 sentidos, paso a paso |
| [`piso-de-pwm/`](piso-de-pwm/) | **A partir de qué PWM arranca cada rueda.** Sube de a 10 y anotás |
| [`adelante-atras/`](adelante-atras/) | Izquierda + derecha adelante y atrás, trasera quieta |
| [`buscar-pelota/`](buscar-pelota/) | ⭐ **El comportamiento completo:** buscar → centrar → avanzar → orbitar → patear |

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
