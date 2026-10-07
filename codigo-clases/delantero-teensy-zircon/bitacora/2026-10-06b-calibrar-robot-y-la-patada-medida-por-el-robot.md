# 2026-10-06 (segunda parte) — `CALIBRAR-ROBOT.bat`, y la patada la mide el robot

> Continuación de `2026-10-06-el-destrabe-andando-y-la-orbita-que-no-pierde-la-pelota.md`.
> Robot **delantero**, Teensy 4.1 / Zircon, serie `15708680`.

---

## 📋 RESUMEN

Dos cosas, y las dos son del mismo tipo: **dejar de medir a ojo**.

1. **La patada ahora se mide sola.** El robot guarda cuánto se torció en cada una de
   las últimas 5 patadas y lo repite en la telemetría, para leerlo sin cable. La primera
   medición ya cambió dos decisiones y encontró un bug.
2. **`CALIBRAR-ROBOT.bat`**, pedido por el equipo: llegar a una cancha nueva y calibrar todo,
   con dos programas que graban en la RAM y se leen después sin cable.

---

## 1. 🎯 La patada: lo que dijeron los números

El equipo reportó *"está un poco torcida, pero mejoró"*, y después *"hay algunas patadas que se
notó que se torcieron mucho"*. Se instrumentó el firmware y midió esto:

| patada | desvío final | **pico** |
|---|---|---|
| la última | -4,1° | **-12,2°** |
| anterior | **-47,1°** | **-47,1°** |
| | +6,2° | +6,4° |
| | +3,7° | **-19,4°** |
| | +4,8° | **-14,9°** |

**Tres cosas salieron, y ninguna era la esperada:**

**a) No es el trim.** Los signos están mezclados (dos negativas, tres positivas). Un desvío
sistemático sería siempre del mismo lado. El trim igual se bajó de **15 a 5**, porque la
medición nueva de `pruebas/patada-derecha/` dio **-11,9° crudo y +1,9° con el giroscopio solo,
sin nada de trim**: el giroscopio ya hace el trabajo completo y hasta cruza el cero. El 15 se
había elegido cuando el robot curvaba -28,4°, una curva que ya no existe.

**b) El número que importa es el PICO, no el final.** Mirá la cuarta: termina en +3,7° (parece
perfecta) pero en el medio se fue a -19,4°. El robot se tuerce fuerte y después el corrector lo
endereza... **pero la pelota ya salió**. Para dónde va la pelota lo decide el pico.

Por eso se subió **`KD_PATADA` de 0,3 a 0,6**: el patrón "se va lejos y vuelve" es
sobreoscilación, y el derivativo es el que la amortigua. Subir `KP` haría lo contrario.

**c) La patada de -47,1° es otra cosa, y descubrió un bug.** Todas las demás vuelven; ésa no
volvió nunca (pico = final). Eso no es el corrector quedándose corto: es el corrector
**apuntando a otro lado**. Y el código decía por qué:

```cpp
if (nuevo == PATEA_ADEL && giroscopoSano()) rumboAlPatear = rumboActual();
```

El rumbo objetivo **sólo se actualizaba si el giroscopio estaba sano en ese instante**. Si no lo
estaba, `rumboAlPatear` **se quedaba con el valor de una patada anterior** — y si durante el
golpe el giroscopio volvía, el corrector enderezaba el robot hacia **el rumbo de una patada
vieja**, a decenas de grados. El robot obedecía y se torcía muchísimo, a propósito.

**Arreglado:** una bandera `rumboPatadaValido`. Si no hay rumbo confiable, esa patada va con
compensación fija y **sin lazo**, y lo avisa. Además queda registrado por patada si tuvo lazo o
no, así la próxima patada rara se diagnostica en vez de teorizarse.

> Es una hipótesis con una sola muestra y así quedó anotada. Pero enderezar hacia un rumbo viejo
> no es correcto en ningún caso, así que el arreglo vale igual.

---

## 2. 🆕 `CALIBRAR-ROBOT.bat` — llegar a una cancha nueva

Pedido del equipo: un botón para calibrar todo lo necesario en una competencia, con los
programas **en la cancha, sin cable**, y leerlos después enchufando el robot encendido.

### Los tres `.bat`, que ahora hay que no confundir

| lanzador | para qué |
|---|---|
| `CARGAR-ROBOT.bat` | **el partido**: arco fijo azul o amarillo |
| `MEDIR-ROBOT.bat` | las 15 pruebas, de a una, para investigar |
| **`CALIBRAR-ROBOT.bat`** | **cancha nueva: qué medir y en qué orden** |

### Los dos programas nuevos

**`pruebas/calibracion-quieto/`** — tres fases en una corrida de ~1,5 min, **sin mover un motor**:

| fase | qué mide |
|---|---|
| giroscopio | si contesta, `SYS_STATUS`, y **cuánto deriva quieto** |
| cámara | % de cuadros que ve **cada arco**, a qué distancia y ángulo |
| línea | verde / blanco / negro → **propone los 3 umbrales** |

Y al final escribe la línea lista para copiar:

```
int UMBRAL_LINEA[3] = { 390, 427, 413 };
```

Si algún margen queda por debajo de 120 cuentas, **se niega a proponer** y pide repetir, en vez
de entregar un umbral frágil.

**`pruebas/calibracion-moviendo/`** — seis fases, **en el piso**:

1. **Piso de PWM, rueda por rueda** — rampa hasta que el giroscopio detecta movimiento. Esto
   estaba pendiente desde el 04/08 y nunca se pudo correr porque `pruebas/piso-de-pwm/` tiene
   **los pines del arquero**. Este los tiene bien.
2. **Avanzar** — ¿va derecho o curva?
3. **Girar buscando** (continuo con rampa) — grados/segundo y cuánto tarda una vuelta.
4. **Centrar a pulsos** — **cuántos grados mueve UN pulso.**
5. **Orbitar** — grados/segundo y cuánto tarda media vuelta, para comparar con los 20 s de
   `MS_ORBITA_MAX`.
6. **Escapar de línea** — cuánto gira con cada sensor, para verificar que el escape retrocede
   en vez de girar.

### La fase 4 es la que más faltaba

*"Cuántos grados mueve un pulso de centrado"* es el número con el que se decide si la banda de
histeresis tiene sentido: **si la ventana es más angosta que un pulso, el robot no puede quedarse
adentro y se zarandea**. Eso se viene discutiendo desde el 29/09 sin medirlo. El informe lo
compara solo contra la banda actual (entra 12 / sale 6) y avisa si hay que ensancharla.

### ⚠️ La patada NO está en el de motores, a propósito

Ya se mide en dos lugares: `pruebas/patada-derecha/` y el firmware mismo. Y hoy aprendimos por
qué una tercera copia sería **peor que nada**: `patada-derecha` mide **sin trim, sin derivativo y
a 420 ms**, mientras el firmware patea **con trim, con derivativo y a 500 ms**. Dos versiones
distintas ya nos hicieron sacar conclusiones equivocadas esta misma tarde. **La patada se mide
donde se patea.**

### Una decisión de diseño

`calibrar-robot.ps1` **no duplica la plomería**: llama a `medir-robot.ps1`, que ya tiene resuelto
buscar el robot, comprobar que sea el delantero y no el arquero, compilar, cargar, leer el
puerto y guardar en `mediciones/`. Un solo motor, dos puertas de entrada: si aparece un bug se
arregla para los dos. Había ya dos bugs de variables de PowerShell en ese archivo; una tercera
copia era pedir el tercero.

---

## ⚠️ Para el día de la competencia

**Una carpeta de prueba NUEVA puede no compilar.** El primer compilado de
`calibracion-quieto` falló con el error del antivirus rompiendo el SSL (`HTTPClientError:`
vacío), porque PlatformIO quiso bajar las librerías. Se arregló copiando las que ya estaban en
caché de otro programa. **Las carpetas que ya existen compilan sin internet; una nueva, no.** Si
en la competencia hace falta un programa nuevo, hay que copiarle la carpeta `.pio/libdeps` de
otro.

---

## Correcciones a cosas que dije mal hoy

1. **Dije que no había prueba de cámara en `MEDIR-ROBOT.bat`. Es falso:** está la opción 11,
   `tabla-camara` ("cuántos cm es un Xp"), escrita desde el 25/08 y nunca corrida. Es la opción 4
   del lanzador nuevo.
2. **Dije que el sensor 2 podía estar roto o tener el cable flojo.** No: al mover el robot bajó
   de 753 a 296. Era lo que tenía debajo.
3. **Dije que `sentidoParaOrbitar()` tenía un bug de ramas opuestas.** Lo leí: no lo tiene.

---

## 🔧 Qué queda

- **Correr los dos programas nuevos.** Ninguno se probó todavía: compilan y nada más.
- **Pasar la patada por el nuevo KD.** Pateá 4 o 5 veces y leemos si los picos bajaron de -19 y
  -15, y si vuelve a aparecer una de 40-50° (y si dice `SIN LAZO`, queda confirmada la
  hipótesis del rumbo viejo).
- La cinta métrica contra `Xp`, que ahora es la opción 4 del lanzador.
- Un partido: todo lo de hoy está probado en cancha, no en partido.

## Nota de método

Hoy se repitió tres veces el mismo movimiento, y las tres funcionó: **cuando algo "está un poco
torcido" o "no lo hace", no dar otra explicación — hacer que el robot imprima lo que decide.**
El diagnóstico del destrabe encontró un estado muerto que nadie buscaba. El de la patada
descartó el trim, cambió la perilla a usar, y destapó el bug del rumbo viejo. Ninguna de las
tres cosas salió de leer el código: salieron de mirar los números del robot.
