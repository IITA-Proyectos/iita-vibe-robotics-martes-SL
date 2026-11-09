# 2026-11-08 (domingo) — La técnica challenge y el tablero con la pestaña PARTIDO

**Quiénes:** Diego y Laureano (cancha, cámara, pruebas y pedidos)
+ Claude (código, revisión y carga)
**Robot:** el **ARQUERO** — Teensy con número de serie `13348550`
**Programas nuevos:** `pruebas/tecnica-challenge/` (con `CARGAR-TECNICA-CHALLENGE.bat`),
`pruebas/tablero/` (con `PARTIDO.bat`, `TABLERO.bat` y `TABLERO-DEMO.bat`),
`funciona/partido-0921h/` y `respaldos/partido-0921f/` (copia exacta de la 0921f)
**Modificados:** `funciona/partido-0921` (de la 0921f a la **0921g**), el README del arquero y el
viejo `funciona/seguir-y-despejar/CARGAR-PARTIDO.bat` (ahora avisa que carga la 0.1103)
**PC:** la notebook que viaja. Plataforma `teensy` 6.0.0, Teensyduino 1.62.

---

## 🎯 EL RESUMEN DEL DÍA

**1. Hay un programa nuevo, aparte del de juego: la técnica challenge** (§1). Recorre la cancha
de un arco al otro, busca la pelota y la despeja. **En la última prueba buscó la pelota de a
pasitos y la despejó.**

**2. El tablero está terminado, con la pestaña PARTIDO** (§3). Desde ahí se cambian los valores
del programa de juego y se carga con un botón. **Todavía no se usó con el robot de verdad.**

**3. El programa de juego tiene una versión nueva, la 0921g** (§4). Es la misma 0921f, más una
línea por el cable que dice la versión. Hay también una **0921h**, con la patrulla para los dos
lados, **sin cargar**.

**4. En el robot quedó la 0921f** (§5), la del 03/11.

---

## 1. La técnica challenge

**El pedido del equipo**, con sus correcciones:
*"crea un programa nuevo aparte de el de juego"* — *"el robot debe ir hacia el borde blanco de la
cancha y luego debe ir hacia el medio"* — *"identificar el arco ya sea azul o amarillo y se debe
acercar hasta tocar el borde de la línea blanca del área"* — *"dar una vuelta de 180 grados con
ayuda del giroscopio"* — *"detectar el otro arco"* — *"girar en su eje para encontrar la pelota
como lo que hace el robot delantero"* — *"y por último tiene que despejar la pelota"*.

**Cómo quedó** (`pruebas/tecnica-challenge/`):

| Paso | Qué hace | LED |
|---|---|---|
| 0 | Cuenta de 5 s. Lo ponen **mirando hacia un arco**, entre las dos áreas | lento, y al final rápido |
| 1 | De costado a la izquierda hasta el lateral, se endereza, y al medio por tiempo (2228 ms a 100) | 1 destello corto |
| 2 | Busca un arco, lo centra y avanza hasta que el sensor de adelante toca la línea del área | 2 destellos |
| 3 | Gira 180°, mira el otro arco, **avanza hasta la línea de su área** y **gira otros 180°** | 3 destellos (fijo 1,5 s si ve el otro arco) |
| 4 | Busca la pelota **de a pasitos**: gira 20°, frena, mira quieto | 4 destellos |
| 5 | Va hacia la pelota y la despeja (160 × 666 ms). Se queda quieto | 5 destellos |

Termina con un **latido lento** si salió todo bien, o con **N destellos largos** si falló el paso N.
Con el USB y `pruebas/herramientas/mirar.bat`, la tecla `i` muestra el informe.

**Cómo se llegó a esto:**
- **La primera versión** la armaron agentes con revisión. La revisión encontró **un problema
  real**, y lo descartó en otros 8: con la pelota lejos (más de unos 70 cm), centrarla girando se
  pasaba de lado. La cámara no manda una distancia mayor que 200 (~70 cm), así que el ángulo salía
  2 o 3 veces más grande que el real. Se arregló: de lejos centra de a pasitos de 6°.
- **Primera prueba:** *"llega hasta la línea del área del arco, se queda quieto apuntando al arco
  del otro lado, y después empieza a girar en su eje sin parar"*. Dio bien la vuelta de 180, **no
  vio el otro arco** (§2), y después giraba buscando la pelota sin encontrarla.
- **Cambio 1, pedido del equipo:** después de la vuelta, ir hasta la línea del área del otro arco y
  dar otra vuelta. Si no ve el otro arco, va derecho con el giroscopio. Tope para esa ida: 8 s.
- **Cambio 2, pedido del equipo:** *"al giro para encontrar la pelota bajale para que sea más
  lento"* y *"el robot tiene que ir más lento en todas las partes"*.
  - **Buscar de a pasitos.** Girar más despacio que 75 no se puede: las ruedas zumban. Y girando, la
    cámara casi no ve la pelota (el delantero midió el 29/09: frenado la ve el 98% de las veces,
    girando el 45%).
  - Al lateral 110 → 90. Hacia los arcos y hacia la pelota 110 → 85. Despeje 200 × 533 → 160 × 666
    ms (para llegar más o menos igual de lejos; es una cuenta).
  - **No se tocaron:** el tramo al medio por tiempo (100 durante 2228 ms; ese tiempo se midió a
    100) y los giros de 180 (ya van al mínimo, 75 a 85).
- **Última prueba:** *"gira de a pasitos y despeja"*. ✅
- **Se llamaba `recorrido-arcos`.** El equipo le puso **técnica challenge**.

**No se sabe:** si en la última prueba vio el otro arco y llegó hasta su línea (§2).

---

## 2. La cámara no ve el arco lejano

Después de la primera vuelta de 180, el robot **no vio el arco del otro lado**. La sospecha: el
programa de la cámara cuenta un arco solo si ocupa muchos píxeles (`pixels_threshold=600` para el
amarillo y 300 para el azul), y de lejos el arco se ve chico.

El equipo lo revisó con la **OpenMV IDE**. Dijeron que el programa *"tiene algunos errores"*, pero
no llegó el texto del error. Revisé la copia de esta PC (`enviar_coordenadas_2_arcos_y_pelota.py`)
y no encontré nada que lo haga fallar: el paquete no se sale de rango y no hay divisiones por cero.
**No se cambió nada de la cámara.**

---

## 3. El tablero, terminado

El 03/11 la sesión se cortó a la mitad del último arreglo. Hoy se terminó y se agregó lo que pidió
el equipo: *"poder cargar todo fácil y cambiar todo fácil es la prioridad más grande"*.

**La pestaña ★ PARTIDO** (se abre directo con `PARTIDO.bat`):
- Muestra los valores de `funciona/partido-0921`. Arriba los de la cancha (`umbralBlanco`,
  `MS_MEDIO_AL_MEDIO`...), después los interruptores, el despeje y el seguimiento.
- **▶ CARGAR EL PARTIDO**: guarda una copia, escribe los valores, compila y carga, con los
  controles de `cargar.py` (solo al arquero, un solo Teensy).
- **Quedó cargado si están las 3 cosas:** LISTO en verde abajo; arriba dice "Cargado y confirmado
  por el robot"; y en "EL ROBOT TIENE" dice "✔ coincide con el archivo".
- **"Volver a como estaba"** y **"Volver a los de fábrica"**.
- Lo medido en CANCHA se pasa con **"PASAR A PARTIDO →"**.
- **"Volver al partido"** ahora lleva a la 0921. Antes cargaba la 0.1103.

**La revisión** (3 lentes: cargar seguro, el editor de valores, que se entienda) encontró **10
problemas confirmados y se arreglaron 9**. El que quedó: el viejo
`funciona/seguir-y-despejar/CARGAR-PARTIDO.bat` cargaba la 0.1103 con el título "programa de
partido". **Ahora avisa y pide una tecla antes de cargar.**

**El demo** (`TABLERO-DEMO.bat`) trabaja sobre una **copia de práctica** del programa de juego y
no carga nada. El equipo lo vio hoy.

---

## 4. La 0921g y la 0921h

- **0921g** (`funciona/partido-0921/`): la 0921f + una línea por el cable al arrancar:
  `#PARTIDO version=0921g sello=fabrica ... umbralBlanco=425 ... interruptores=...`. **No cambia
  nada de cómo juega.** Es para que el tablero confirme qué quedó cargado. El equipo lo autorizó el
  03/11.
- **0921h** (`funciona/partido-0921h/`): la 0921f + **la patrulla para los dos lados**. Pedido:
  *"la patrulla siempre se va para la izquierda, hacé que también pueda ir hacia la derecha"*.
  Ahora una patrulla va al izquierdo y la siguiente al derecho. Ir al medio al arrancar sigue yendo
  por el izquierdo. Desde el derecho vuelve con los mismos 2228 ms; si se queda corto, el número
  de ese lado es 2331. **El equipo pidió no cargarla.** El tablero no la maneja.

---

## 5. Lo que se cargó hoy, en orden

1. La técnica challenge, varias veces, una por cambio.
2. La **0921g**, sin querer: el armado del tablero ya la había generado. Se reemplazó enseguida.
3. La **0921f**, sacada tal cual de GitHub. Pedido: *"cargá el partido que utilizamos la clase
   pasada"*.
4. La **0921d**: *"cargá la d"*. Es **la última que se probó en cancha** (03/11).
5. La **0921f** otra vez.

| En el robot | Estado |
|---|---|
| **0921f** | 🚨 **LA QUE ESTÁ EN EL ROBOT.** El límite lateral, la patrulla e ir al medio **todavía no se probaron en cancha** |

---

## 6. La explicación del profe (issue 17)

El profe Franco subió el issue 17 en GitHub, con una explicación del robot para que el equipo la
pueda contar: Teensy, ruedas omni, motores sin encoder y PWM, el BNO055, la cámara (blobs y el
modelo LAB), los sensores de línea, `setup` y `loop`, la máquina de estados y los números que se
ajustan. Se resumió en la sesión.

---

## 7. Errores del día (míos)

1. **La primera técnica challenge no iba hasta el otro arco.** Lo había entendido como "mirar el
   otro arco y seguir"; el equipo lo corrigió.
2. **La búsqueda de la pelota girando sin parar a 80.** El delantero ya había medido que girando la
   cámara casi no ve la pelota: había que hacerla de a pasitos desde el principio.
3. **Cargué la 0921g sin querer.** No me fijé que el armado del tablero ya había cambiado el archivo
   del programa de juego. La reemplacé por la 0921f, pero esa vez el robot corrió un programa sin
   revisar.
4. **El tablero tardó muchísimo**: se cortó el 03/11, hoy se pausó para hacer la técnica challenge
   y se retomó.

---

## 8. Qué queda pendiente

### 🔴 Antes del partido

- 🔴 **Elegir entre la d (probada) y la f (con límite lateral, patrulla e ir al medio, sin probar).**
  Si es la f: probar que el límite lateral frene antes de salirse **a 160**.
- 🔴 **Usar el tablero con el robot de verdad por primera vez**: cambiar un valor, CARGAR, y ver
  las 3 señales (§3).

### ⬜ La técnica challenge

- ⬜ **¿Ve el otro arco y llega a su línea?** Si no lo ve, bajar `pixels_threshold` en el programa
  de la cámara (600 → ~200 el amarillo, 300 → ~150 el azul) y probar con ▶ en la OpenMV IDE antes
  de guardarlo. El azul puede dar falsos con sombras.
- ⬜ **El texto del error** que mostró la OpenMV IDE.
- ⬜ **El signo del amarillo** (`amarilloInvertido`) nunca se midió.

### ⬜ El programa de juego

- ⬜ **La patrulla para los dos lados (0921h):** probarla; si anda, pasarla a la que maneja el
  tablero (`funciona/partido-0921`).
- ⬜ **`MS_MEDIO_AL_MEDIO` (2228)** se midió antes de mover los motores. Volver a medirlo (la
  pestaña CANCHA del tablero lo hace).

### ⬜ Arrastres

- ⬜ Quedan textos que mandan a la 0.1103: `AJUSTAR.bat`, `tablero.ino:71`,
  `ajustar-movimientos.ino:43`, `pruebas/herramientas/README.md:47` y `cargar.py:9` (anotado en
  `pruebas/tablero/DISENO.md` 16.10).
- ⬜ `cargar.py` no reconoce un Teensy en modo carga (el tablero sí).
- ⬜ El script de la cámara sigue con el umbral naranja cambiado solo en esta PC (79 → 78).
