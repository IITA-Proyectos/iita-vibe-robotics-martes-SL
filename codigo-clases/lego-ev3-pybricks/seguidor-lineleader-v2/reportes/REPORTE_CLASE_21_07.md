# Reporte de Clase: 21 de Julio de 2026

## Objetivos de la Clase
El objetivo principal de la sesión fue implementar el sistema dinámico de aprendizaje de verdes en pista, agregar señales visuales gráficas en la pantalla del EV3 (178x128 px), crear un menú de calibración modular por botones, implementar la función de pausa no bloqueante en pista y establecer una selección de modos de arranque (Modo Competición vs. Modo Recolección de Datos).

---

## 1. Trabajo Realizado y Funcionalidades Desarrolladas

### A. Pantalla Gráfica EV3 para Eventos en Pista (`dibujar_pantalla_evento`)
* **Implementación:** Se diseñó una interfaz gráfica completa para la pantalla del EV3 (178x128 píxeles).
* **Gráficos Activos:** Muestra íconos y textos descriptivos para 9 eventos: `VERDE_IZQ` (Flecha Izq + V), `VERDE_DER` (Flecha Der + V), `VERDE_DOBLE` (U-Turn 180° + VV), `CRUZ` (Cruz negra +), `T_IZQ`, `T_DER`, `CURVA_DESCARTE`, `RECOVERY` y `PAUSA` (ícono ||).

### B. Botón de Pausa No Bloqueante
* **Mecanismo:** Presionar `Button.LEFT` o `Button.RIGHT` en cualquier momento del bucle `follow_line()` detiene los motores inmediatamente, emite un tono de alerta y muestra el ícono de Pausa en pantalla.
* **Reanudación:** El programa permanece en espera sin detener la ejecución del script ni descalibrar el PID. Se reanuda suavemente al presionar y soltar `Button.CENTER`.

### C. Menú Modular de Calibración de 4 Opciones
* **Estructura:** Se reestructuró el menú de inicio para permitir calibraciones parciales o completas:
  - **Boton ARRIBA:** Carga directa de datos desde la memoria JSON (`calibracion_verdes.json`).
  - **Boton ABAJO:** Calibración completa de 7 pasos (LSA + Sensores de Color Frontales).
  - **Boton IZQUIERDA:** Solo calibración del array LSA de piso (4 pasos: Blanco, Negro, T Izq, T Der).
  - **Boton DERECHA:** Solo calibración de los sensores de color frontales S2/S4 (4 pasos: Blanco Color, Negro Color, Verde Izq, Verde Der).

### D. Aprendizaje Progresivo 70/30 e Historial de Pista (`experiencia_pista.json`)
* **Algoritmo:** En el Modo Recolección de Datos, cuando se detecta un verde y el usuario confirma con `IZQ (SI)`, el robot actualiza el target HSV mediante promedio ponderado:
  $$\text{Nuevo Target} = 0.70 \times \text{Target Previo} + 0.30 \times \text{Medición Actual}$$
* **Persistencia:** Guarda las calibraciones refinadas en `calibracion_verdes.json` y registra cada muestra medida en `experiencia_pista.json`.

### E. Menú de Inicio de Pista Dual (`=== LISTO A PISTA ===`)
* **Selección de Modo:**
  - **Boton CENTRO:** **Modo Normal (Competición)**. Usa la calibración guardada en memoria JSON. Detecta los verdes, alinea los 88 mm y gira automáticamente sin frenar ni pedir confirmación en pantalla.
  - **Cualquier OTRO BOTÓN:** **Modo Recolección (Data)**. Detecta los verdes, frena sobre la marca y abre el modal interactivo `¿Es VERDE REAL?` (`IZQ: SI` / `DER: NO`) para validar muestras y actualizar `experiencia_pista.json`.

---

## 2. Estado de Archivos y Repositorio

* **[seguidor_verdes.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/seguidor_verdes.py):** Código principal optimizado con menú modular, gráficos en pantalla, pausa no bloqueante y selector dual de modos.
* **[calibracion_verdes.json](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/calibracion_verdes.json):** Archivo JSON de calibración activa.
* **[experiencia_pista.json](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/codigo-clases/lego-ev3-pybricks/seguidor-lineleader-v2/codigos/experiencia_pista.json):** Registro de muestras capturadas durante la práctica en pista.

---

## 3. Tareas Pendientes y Recordatorio para la Próxima Clase

> [!CAUTION]
> **RECORDATORIO CRÍTICO OBLIGATORIO PARA LA PRÓXIMA CLASE:**
> **Se debe volver a calibrar los sensores de verde antes de la primera prueba.**
> Durante las pruebas de hoy, se registró una muestra sobre una sombra/línea negra oscura (Hue: 252°, Valor: 8%), lo cual desvió temporalmente el valor de `GREEN_L_HSV` a un tono inapropiado, causando falsos positivos de `VERDE IZQUIERDA` sobre el negro.
> 
> **Pasos requeridos al iniciar la próxima clase:**
> 1. Encender el EV3 y ejecutar `seguidor_verdes.py`.
> 2. En el menú de calibración, presionar **DERECHA** (Solo Sensores de Color) o **ABAJO** (Calibración Completa).
> 3. Calibrar sobre la pista con luz real los sensores frontales S2 y S4 en **Blanco**, **Negro** y **Verde**.

---

### Próximos Pasos en Código:
1. **Prueba de Pista Completa:** Correr el robot en Modo Normal (presionando `CENTRO` al inicio) tras la recalibración limpia de Diego.
2. **Ajuste de Velocidad en Curva en U:** Probar el giro de $180^\circ$ (Doble Verde) para asegurar la salida limpia en la línea.
