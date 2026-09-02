# 2026-09-01 — El giroscopio nunca estuvo roto, y lo que se destrabó al arreglarlo

**Quiénes:** Máximo (cancha y mesa) + Claude (código y lectura del serie)
**Robot:** delantero (`ROBOT2`) · Teensy `15708680`
**Programa cargado al cerrar:** el firmware bueno, con todo lo de hoy

---

## 📋 RESUMEN DE LA CLASE

| | |
|---|---|
| 🎯 **EL GIROSCOPIO NUNCA ESTUVO ROTO** | tres semanas dado por muerto. Fallaba la verificación |
| ✅ **La patada va derecha** | 10,1° → 4,2° de desvío, medido |
| ✅ **Patada más corta y menos bruta** | 240×1000 ms → 215×420 ms |
| ✅ **`PATEAR_AL_RUMBO0` destrabada** | ya estaba escrita desde el 11/08, esperando el giroscopio |
| ✅ **`ORBITA_CAMINO_CORTO` encendida** | y andando, después de dos bugs |
| ✅ **El escape de línea se compromete** | 400 ms sin cambiar de dirección |
| 🐛 **Dos bugs latentes destapados** | los dos por decidir 17.000 veces por segundo |
| ⚠️ **El heading-hold sobrepasa** | vuelve de 33° pero se pasa a −9,4° |
| 🔴 **24 commits sin pushear** | sigue faltando la deploy key |

---

## 1. 🎯 El giroscopio nunca estuvo roto

Desde el 11/08 el firmware lo daba por muerto con el mensaje
`contesta pero da ceros (9/20 lecturas utiles)`. El 18/08 se perdió una clase
persiguiendo `setExtCrystalUse(true)` como culpable.

Se hizo `pruebas/giroscopo-crudo/`, que le pregunta al chip en crudo por I2C en
vez de deducir. Lo que salió:

```
dispositivo en 0x28  ✅        CHIP_ID = 0xA0 ✅   (BNO055 real)
ACC_ID = 0xFB ✅   MAG_ID = 0x32 ✅   GYR_ID = 0x0F ✅
SYS_STATUS = 5 (FUSION CORRIENDO)   SYS_ERR = 0
giroscopo calibrado 3/3
girando el robot a mano: RECORRIDO 97.8 grados     ← EL RUMBO SIGUE
```

**Los cuatro IDs correctos** — un clon barato suele acertar el `CHIP_ID` y
errarle a los sub-IDs. La fusión corriendo, sin errores. Y el rumbo sigue al
giro. **El sensor está sano.**

### La causa real

```cpp
for (int i = 0; i < 20; i++) {
  bno.getEvent(&e);
  if (e.orientation.x != 0.0 || e.orientation.y != 0.0
      || e.orientation.z != 0.0) buenas++;      // <-- ACA
  delay(50);
}
if (buenas < 10) return false;                  // saca 9
```

**Un rumbo de 0,0° es una postura válida, no una falla.** Y este robot, apoyado
como se lo apoya siempre, arranca justo en el borde: el rumbo en reposo oscila
entre **359,9 y 0,0**. Cuando cae en 0,0 los tres ángulos son cero (está plano,
así que cabeceo y alabeo también) y la lectura se contaba como "sensor caído".

Oscilando entre esos dos valores, de 20 lecturas la mitad salen cero: **da 9.**

Ese *"siempre 9 de 20"* que se anotó el 18/08 **no era la fusión convergiendo:
era el rumbo bailando sobre el 0/360.** Y explica por qué al arquero le anda el
mismo código — su rumbo de arranque no cae ahí.

### 🔴 El mismo bug corría EN PARTIDO

`rumboActual()` usaba la misma lógica con `CEROS_PARA_DARLO_POR_CAIDO = 10`: si
el robot quedaba apuntando al rumbo 0 diez lecturas seguidas, **daba el
giroscopio por muerto en pleno juego** y abandonaba `APUNTA_RUMBO0`.

### El arreglo, en los dos lugares

Preguntarle **al chip** por su registro de estado del sistema (`0x39`):
**5 = algoritmo de fusión corriendo**. Es un dato que el sensor da sobre sí
mismo y no se puede confundir con una postura.

Y quedó **descartado `setExtCrystalUse(true)`**: la fusión arranca igual con el
cristal externo. Se movió el `delay(700)` a después de la llamada, que es donde
corresponde, pero no era el problema.

> **La lección:** el código 2025 que funcionaba **no verificaba nada** —
> `bno.begin()`, `setExtCrystalUse`, y a leer. La verificación de las 20
> lecturas es un agregado de 2026, y estuvo rechazando un sensor sano durante
> tres semanas. A veces el chequeo que agregaste para protegerte es el que te
> rompe.

---

## 2. La patada va derecha

Se hizo `pruebas/patada-derecha/`, que reproduce la patada exacta y usa el
giroscopio para medir cuántos grados se tuerce el robot. Hace dos patadas
seguidas, una sin corrección y otra con heading-hold, para comparar directo:

```
A) sin correccion (como venia):  10.1 grados   (pico 11.3)
B) con heading-hold, KP = 4.0:    4.2 grados   (pico  5.4)
```

**El robot curva.** Diez grados en un segundo a fondo explica que la pelota
saliera desviada: el robot ya está girando mientras la empuja. Y quedó
**descartada la otra hipótesis** — no era el contacto de refilón con la pelota.

**Por qué curva:** `avanzar()` manda el mismo PWM a las dos ruedas de adelante,
pero el mismo PWM no es la misma velocidad. Y la trasera queda **suelta**
durante la patada, así que no hay nada que se oponga al giro.

**El arreglo:** al entrar a `PATEA_ADEL` se guarda el rumbo y se corrige contra
él durante el golpe. Si no hay giroscopio sano, patea como antes — no hay
regresión posible.

> **Un detalle de diseño que se repite:** a 240 la corrección **solo puede
> frenar**, porque no hay techo para subir (máximo 255). A 75, en la prueba de
> perturbación, **solo puede acelerar**, porque restar dejaría la rueda debajo
> del piso de rodadura y se plantaría. Es el mismo problema visto desde los dos
> extremos.

---

## 3. La patada, más corta y menos bruta

```
240 x 1000 ms   ->   215 x 420 ms
```

Ataca **tres** cosas a la vez:

1. **Salirse de la cancha.** La pelota se va del robot en los primeros ~200 ms.
   Los otros 800 ms ya no empujaban la pelota: empujaban al **robot**.
2. **El rebote del heading-hold.** El lazo vuelve al rumbo en ~700 ms pero se
   pasa de largo. Con la patada en 420 ms, **el rebote ni siquiera llega a
   entrar** en la ventana de la patada.
3. **Margen para corregir.** A 240 sobre 255 casi no quedaba lugar; a 215
   quedan 40 puntos.

**El precio:** la pelota llega menos lejos. Es un canje deliberado — puntería y
no salirse, a cambio de alcance. Si queda corta, subir `VEL_PATADA` de a 10
**antes** que alargar el tiempo.

---

## 4. ¿El lazo recupera? — la prueba que propuso el equipo

**La idea fue de Máximo**, y es mejor prueba que la anterior: en vez de mirar si
el robot se desvía solo, **empujarlo a propósito** y ver si vuelve. Un desvío
chico puede ser suerte; recuperarse de un empujón no se puede fingir.

`pruebas/giroscopo-recupera/`, a VEL 75, 4 s, grabando el error cada 20 ms:

```
   0-300 ms   +#####  5.6        el impulso de arranque lo sacude
   400-600    → +0.6             🟢 recupera solo
   700-2000   ±0.8               🟢 1,3 s con MENOS DE 1 GRADO de error
   2100-2400  +#########...33.2  ← empujón a mano
   2600       16.9               🟢 corrigiendo
   2700        2.4               🟢 casi en cero
   2800       -8.2               ⚠️ se pasó
   2900       -9.4               ⚠️ el rebote
   3100       -0.3               🟢 estabilizado
```

**Se recupera de 33 grados en 700 ms**, y sin tocarlo mantiene el rumbo con
menos de un grado.

### ⚠️ Pero sobrepasa

Después de corregir los +33° se fue a **−9,4°**. Un 28% de rebote. Es el
comportamiento clásico de un control **solo proporcional**: corrige mirando
*dónde está*, pero no *qué tan rápido vuelve*, así que llega al cero con
demasiada velocidad y lo cruza.

**Pendiente:** agregar término derivativo (P → PD). Hoy se tapó acortando la
patada a 420 ms, que termina antes de que el rebote entre — pero el rebote
sigue ahí.

---

## 5. Los signos, medidos — y un error mío

Se hizo `pruebas/signos/` para cerrar el `[SIN VERIFICAR EN BANCO]` que estaba
abierto desde el 11/08. Tres mediciones:

```
1. girando el robot a mano a la DERECHA -> el rumbo SUBE      (+20.6 grados)
2. orbitar(sentidoA = true)             -> gira a la DERECHA  (+38.2 grados)
3. pelota a la DERECHA                  -> Yp NEGATIVO        (-14)
   o sea: para la camara el angulo POSITIVO esta a la IZQUIERDA
```

De ahí deduje `SENTIDO_ORBITA_INVERTIDO = true`. **Y estaba mal.** Probado en
cancha, orbitaba para el lado equivocado. Volvió a `false`.

### 🔴 El error de razonamiento, para no repetirlo

Asumí que *"orbitar hacia el arco"* significa **girar hacia el lado donde está
el arco**. Es **al revés**. Para patear, el robot tiene que quedar con la
**pelota entre él y el arco** — o sea del lado **opuesto** de la pelota respecto
del arco. Si el arco está a la izquierda, el robot tiene que rodear la pelota
hacia la **derecha** para quedar detrás de ella.

**Orbitar para alinearse es irse al lado contrario al arco, no hacia el arco.**

Las tres mediciones eran correctas y siguen anotadas en el código. **El error
estuvo en componerlas en una regla geométrica, no en medirlas.** Medir bien no
alcanza si la regla que armás con los datos está mal.

---

## 6. 🐛 Dos bugs latentes, la misma forma

Los dos estaban escritos hace tiempo y **nadie los había visto** porque las
funciones que los contenían estaban apagadas.

### El sentido de la órbita

```cpp
orbitar(sentidoParaOrbitar(), ...);   // DENTRO del loop: ~17.000 veces/s
```

Decide mirando el **signo** del ángulo al arco. Mientras el robot orbita *hacia*
el arco, ese ángulo se acerca a cero y **lo cruza**: ahí el signo se da vuelta y
el robot invierte el sentido. Enseguida vuelve a cruzar. **Queda pataleando
alrededor del cruce por cero** — arranca para un lado, se va para el otro, y
nunca completa la vuelta.

Síntoma reportado por el equipo apenas se encendió `ORBITA_CAMINO_CORTO`.
Con la perilla apagada la función devolvía una constante y no se veía nunca.

**Arreglo:** el sentido se congela en `cambiarA(ORBITANDO)`.

### La dirección del escape de línea

```cpp
if (m != 0) { t_ultimaLinea = millis(); mascaraLinea = m; ... }   // cada vuelta
```

Si durante el escape otro sensor pisaba la línea, **la dirección se daba vuelta
en pleno escape**, y el robot podía quedar rebotando sin despegarse.

**Probable explicación del escape de 24 segundos del 25/08.** En su momento se
atribuyó a falta de espacio en la mesa; ésta no necesita suponer nada del
entorno. Queda como causa probable, no confirmada.

**Arreglo, a pedido de Máximo:** los primeros `MS_ESCAPE_COMPROMISO = 400 ms` la
máscara queda congelada y los otros sensores se ignoran. Después vuelve a
actualizarse, así que las esquinas siguen funcionando.

> **El patrón:** las dos son **decisiones que hay que tomar una vez y sostener,
> replanteadas a cada instante**. Si vuelve a aparecer un síntoma de "el robot
> no se decide", mirar primero si algo se está recalculando dentro del loop.

---

## 7. Lo que se destrabó con el giroscopio

Nada de esto hubo que escribirlo. **Ya estaba, esperando un giroscopio sano:**

| | |
|---|---|
| `PATEAR_AL_RUMBO0` | Si la órbita no encuentra el arco, apunta al rumbo de arranque y patea ahí. **Es exactamente la función que pidió el equipo hoy.** Escrita el 11/08 |
| `ORBITA_CAMINO_CORTO`, rama 2 | Elegir el lado sin ver el arco, usando el rumbo. Nunca había podido correr |
| Heading-hold de la patada | Imposible de intentar con el sensor dado por muerto |

**`PATEAR_AL_RUMBO0` sigue sin probarse en cancha.** Es el pendiente N°1.

---

## 8. 🛠️ Avast rotó su certificado raíz

PlatformIO volvió a fallar con el `HTTPClientError:` vacío de la clase pasada.
**Avast cambió su raíz** entre el 25/08 y hoy:

| | Huella SHA-1 |
|---|---|
| 25/08 | `98EE2D80…773200` |
| hoy | `5CB506EA…4F07A7` |

Como va a volver a pasar, quedó un script que regenera el bundle solo:

```bash
powershell -ExecutionPolicy Bypass -File C:\Users\alumnos\.platformio\regenerar-ca-bundle.ps1
```

Busca todas las raíces de interceptación del almacén de Windows (Avast, AVG,
Kaspersky, ESET, Bitdefender…), las junta con `certifi` y prueba la conexión.

---

## 9. Programas nuevos en `pruebas/`

| Programa | Responde |
|---|---|
| `giroscopo-crudo/` | ¿Qué chip es, corre la fusión, se mueve el rumbo? |
| `patada-derecha/` | ¿Cuántos grados se tuerce al patear, con y sin corrección? |
| `giroscopo-recupera/` | ¿El lazo recupera de un empujón? (idea del equipo) |
| `signos/` | Los tres signos, medidos en vez de supuestos |
| `rumbo-vivo/` | El rumbo en vivo, sin mover motores |

Todos miden **sin cable** o guardan el resultado en RAM para leerlo después.

### Dos lecciones de método que quedaron en el código

**Una medición que no se hizo no es un dato: es una trampa.** Las tres primeras
corridas de `signos/` dieron 0,0 en la medición 1 y el programa igual escupió un
veredicto, derivado de ceros. Si se hubiera aplicado, habría sido el valor
**contrario** al correcto. Ahora se niega a concluir si alguna medición no llega
al mínimo.

**No depender de que la persona adivine el momento.** Fallaba porque el aviso
salía por el monitor y el equipo miraba el robot. Se sacó la ventana de tiempo:
ahora el programa **espera** hasta que lo giren y hasta que vea la pelota. Es la
misma corrección que hubo que hacer el 25/08 con los sensores de línea — **dos
veces el mismo error de diseño en dos clases seguidas.**

---

## Qué queda pendiente

1. 🔴 **Probar `PATEAR_AL_RUMBO0` en cancha.** Dejarlo orbitar sin ver el arco y
   esperar los 20 s. Es la función que el equipo pidió y todavía no se vio
   andar. **Ojo la primera vez:** el 18/08, con el rumbo congelado, ese estado
   hacía girar al robot 6 segundos en el lugar.
2. **Probar el escape con el compromiso de 400 ms**, y confirmar si era eso lo
   del escape de 24 segundos.
3. **Término derivativo en el heading-hold** (P → PD), para el rebote de −9,4°.
4. **Correr `tabla-camara`** — sigue pendiente desde el 25/08, y sigue siendo lo
   que cerraría lo de `XP_ORBITA = 22`.
5. **La deploy key.** 24 commits esperando.
6. **Avisar al arquero del `SPI.h`** — sigue sin arreglarse.

## Nota de método

Dos errores míos hoy, los dos del mismo tipo: **saqué conclusiones de datos que
no tenía.** El veredicto de los signos salió de una medición en cero, y la regla
geométrica salió de una suposición que no verifiqué. En los dos casos el equipo
lo detectó probando en el robot.

Y una que salió bien: **la mejor prueba del día la propuso Máximo** — empujar el
robot a propósito en vez de mirar si se desvía solo. Es un salto de calidad:
pasa de observar a *perturbar y observar la respuesta*, que es lo que de verdad
prueba que un lazo funciona.
