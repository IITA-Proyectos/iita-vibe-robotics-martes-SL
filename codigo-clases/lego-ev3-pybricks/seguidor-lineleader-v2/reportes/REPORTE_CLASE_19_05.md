# Reporte de Clase: 19 de Mayo de 2026

## Objetivos de la Clase
El objetivo principal de hoy fue implementar la **detección de marcadores verdes (intersecciones)** utilizando el LightSensorArray (sensor de escala de grises), y lograr que el robot gire 90 o 180 grados según corresponda, sin usar un sensor de color dedicado.

## Trabajo Realizado y Logros

1. **Nueva Lógica de Detección (Opción B):**
   * Se abandonó el sistema inestable que frenaba al robot aleatoriamente.
   * Se implementó un algoritmo de reconocimiento de patrón espacial: dividimos el array en Izquierda (0, 1, 2), Centro (3, 4) y Derecha (5, 6, 7).
   * **Regla estricta "Blanco-Negro-Gris":** Para evitar que el borde difuminado de la línea negra se confunda con un marcador verde, el robot ahora exige que:
     * El verde sea visto por **al menos 2 sensores** simultáneos.
     * El centro esté firmemente apoyado sobre negro.
     * El lado opuesto al verde esté leyendo **blanco puro**.

2. **Calibración Independiente y Avanzada:**
   * Descubrimos que la iluminación no es simétrica para ambos lados del robot.
   * Modificamos la calibración para que tome 4 pasos: **Blanco, Negro, Verde Izquierdo y Verde Derecho**.
   * Ahora cada lado tiene su propio rango dinámico (`GREEN_L_MIN` / `GREEN_L_MAX` y `GREEN_R_MIN` / `GREEN_R_MAX`), lo que aumenta drásticamente la precisión frente a sombras asimétricas.
   * Añadimos una interfaz visual en la pantalla del EV3 que muestra el array completo de 8 valores al terminar cada paso de calibración, permitiendo depurar problemas físicos de luz en tiempo real.

3. **Sistema de Persistencia de Memoria:**
   * Para ahorrar tiempo de competencia, programamos un sistema de guardado y carga usando archivos `json`.
   * Al encender el robot, el EV3 pregunta: **ARRIBA (Cargar Memoria)** o **ABAJO (Calibración Manual)**.
   * Al hacer la calibración manual, el robot escribe automáticamente el archivo `calibracion.json` en su memoria interna.
   * En reinicios posteriores, si la iluminación del cuarto no ha cambiado, el botón ARRIBA carga los datos en 1 segundo y el robot está listo para correr.

## Problemas Pendientes (Para la próxima clase)

**El problema:** A pesar de los filtros matemáticos estrictos, el robot todavía se confunde a cada rato y detecta verdes falsos, especialmente en curvas.

**Diagnóstico y Posibles Soluciones:**
1. **Filtro de Tiempo (Debounce):** El robot lee los sensores cientos de veces por segundo. Un ruido eléctrico de 1 milisegundo puede disparar un falso verde. *Solución para la próxima:* Exigir que el patrón verde se mantenga constante durante al menos `0.05` segundos (o X ciclos del `while`) antes de dar la orden de giro.
2. **Umbral de Blanco demasiado bajo:** Si el piso no está perfectamente limpio, el robot podría no estar leyendo "blanco puro" en el lado opuesto, invalidando nuestra regla estricta. *Solución:* Revisar en la pantalla del EV3 qué valores exactos tira el piso en las curvas.
3. **Escudo Físico (Hardware):** La interferencia de la luz del sol o focos del techo cambia el valor de gris del borde de la línea mientras dobla. *Solución:* Ponerle una "pollerita" o faldón de cartulina negra alrededor del sensor para aislarlo de la luz externa.

---
**Nota para el equipo (y para Juanse):** La lógica matemática ya está armada a la perfección. La falla actual es puramente por ruido físico del sensor en movimiento. ¡La próxima clase lo solucionamos filtrando los datos en el tiempo!
