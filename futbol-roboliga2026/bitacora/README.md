# Bitácora del equipo de fútbol

**Una entrada por clase.** Archivo `YYYY-MM-DD-tema-corto.md`.

No es burocracia: sin esto, la clase que viene nadie se acuerda de qué valor probaron, y se
repite el mismo experimento tres veces. Ya lo vienen haciendo en sus carpetas personales
(`bitacora_05_05_2026.md` y compañía) — acá es lo mismo, pero de lo que se prueba **en el robot**,
para que lo vea todo el equipo.

## Plantilla

```markdown
# 2026-MM-DD — <tema>

**Quiénes:** 
**Robot:** arquero / delantero
**Programa cargado:** <archivo + qué #define estaba activo>

## Qué queríamos probar
Una frase. Qué esperábamos que pasara ANTES de encender.

## Qué hicimos
Qué cambiamos, con valores concretos. Un cambio por vez.

## Qué pasó de verdad
Lo que vimos, no lo que queríamos ver. Si salió mal, mejor: eso es lo que sirve.

## Números
Valores medidos, tiempos, PWM, umbrales de línea, lo que hayan anotado.

## Qué queda pendiente
Lo que no llegamos a probar, o la duda que quedó abierta.
```

## Reglas

- **Un cambio por vez.** Si tocan tres cosas y el robot empeora, no saben cuál fue.
- **Anotar lo que pasó, no lo que querían que pasara.** Un experimento que sale mal y queda
  anotado vale más que uno que sale bien y no se anota.
- **Los números, aunque parezcan obvios.** El PWM que probaron, el umbral de blanco, el tiempo
  del estado. Dentro de tres semanas nadie se acuerda.
- **Si algo anda, decir con qué versión exacta.** Qué archivo, qué `#define`, qué valores.
- **Si un parche de `correcciones-propuestas.md` se probó**, anotar acá el resultado y marcar
  el estado en ese archivo. Los parches están **propuestos**, no validados: los valida el que
  tiene el robot en la mano.
