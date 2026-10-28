/* =====================================================================
   ARQUERO — sigue la pelota de costado y la despeja
   =====================================================================
            V E R S I O N   0 . 1 0 2 7 d   -   E N   P R U E B A
   =====================================================================
   IITA Salta — taller de los martes — Roboliga 2026
   Ultimo cambio: 2026-10-27 (el panel de control, el despeje en diagonal y
   dos arreglos de la revision de la diagonal)
   (Es la ESTABLE 0.1013, la copia congelada de
   `respaldos/estable1013conlimiteslaterales/`, con DOS diferencias: el freno
   antes de volver del despeje (20/10) y el despeje en DIAGONAL (27/10). La
   prediccion del seguimiento sigue APAGADA, como en la estable. La ESTABLE
   anterior, la 0.0929, sigue en `respaldos/estable-0.0929/`.)

   🎛️ TODO LO QUE SE AJUSTA ESTA JUSTO ABAJO, en el PANEL DE CONTROL.
   La historia de cada cambio sigue despues del panel.
   ===================================================================== */

// =====================================================================
//                       🎛️  PANEL DE CONTROL
// =====================================================================
//   TODO lo que se ajusta del arquero esta aca, junto. Para cambiar algo:
//   cambiar SOLO el numero, cargar con CARGAR-PARTIDO.bat (en esta misma
//   carpeta) y anotar en la bitacora de cuanto a cuanto y QUE SE VIO.
//
//   ANTES DE CARGAR: bateria APAGADA, o el robot en el piso. Apenas termina
//   la carga el robot ARRANCA SOLO. La ventana del cargador tiene que
//   mostrar '=== ARQUERO  version ...': si no, no quedo cargado.
//   PARA JUGAR: desenchufar el USB, apoyarlo QUIETO mirando la cancha,
//   prender la bateria y sacar la mano. A los 2 s guarda hacia donde mira.
//
//   UNIDADES
//     ms   = milisegundos (1000 ms = 1 segundo)
//     cm   = centimetros REALES (la camara ya viene traducida)
//     fuerza (PWM) = 0 nada, 255 a fondo. Desde QUIETO una rueda necesita
//            ~70 para arrancar; YA RODANDO alcanza ~40 (medido antes de
//            mover los motores)
//     true / false = prendido / apagado
//     decimales con PUNTO: 4.5, no 4,5 (con coma NO compila)
//     los que dicen int son enteros: 47.5 ahi no va
//
//   REGLAS
//     1. UN CAMBIO POR VEZ. Si se tocan tres y empeora, no se sabe cual fue.
//     2. Lo marcado [MEDIDO] salio de una prueba: se vuelve a medir, no se tantea.
//     3. Antes de cambiar un numero, buscar su nombre mas abajo (Ctrl+F): ahi
//        esta la historia de por que vale lo que vale.
//     4. Muchos se prueban sin tocar este archivo con pruebas/ajustar-movimientos
//        (AJUSTAR.bat): las teclas cambian los numeros y la v dice que copiar aca.
//        OJO: AJUSTAR.bat REEMPLAZA al programa de partido en el robot. Al
//        terminar: cerrar la consola (Ctrl+C) y CARGAR-PARTIDO.bat, o el robot
//        NO juega. Y la herramienta tiene su PROPIA copia de los numeros (sus
//        DEF_): lo que se cambie aca, cambiarlo alla tambien.
// =====================================================================

// ---------- 1. INTERRUPTORES: que cosas estan prendidas (true = si, false = no) ----------
bool volverAlMedioEnElLateral                    = true;  // siguiendo la pelota, al pisar el lateral vuelve al medio (13/10). false = sin limite: se puede ir contra el muro. NECESITA GIROSCOPIO
bool patrullarSinPelota                          = true;  // sin ver la pelota y quieto un rato, sale a patrullar (que el juez vea que esta vivo). NECESITA GIROSCOPIO
bool patrullaPorElLateral                        = true;  // la patrulla va al lateral y vuelve al medio. false = plan B: solo retrocede a la linea y avanza el poquito
bool avanzarAlUbicarse                           = true;  // al encontrar la linea del area, avanza ~10 cm para no quedar encima del blanco
bool irAlMedioSiNoHayLinea                       = true;  // si al arrancar no encuentra la linea del area, hace la rutina de ir al medio. NECESITA GIROSCOPIO
bool frenarAntesDeVolver                         = true;  // despeje: al terminar la ida FRENA y espera (20/10). false = suelta los motores 150 ms, como antes
bool empujonFinal                                = true;  // despues de volver del despeje, avanza el poquito (~10 cm)
bool predecirTrayectoria                         = false; // seguir adonde VA a estar la pelota. APAGADA el 20/10: con la pelota quieta lo hacia ir y venir
bool enderezarActivado                           = true;  // usar el giroscopio para ir derecho y enderezarse. false = como si no hubiera giroscopio: TAMPOCO hay limite lateral, ni patrulla, ni ir al medio, ni diagonal, aunque digan true. Lo mismo si el LED tiembla rapido (giroscopio muerto)
bool perseguirEnElDespeje                        = false; // en la ida del despeje, corregir hacia la pelota. Nunca se probo
bool despejeEnDiagonal                           = true;  // despejar en DIAGONAL, mirando siempre al frente (ver seccion 4b). PRENDIDO el 27/10, SIN PROBAR en cancha. Necesita giroscopio: sin el, sale derecho solo. false = sale derecho, como hasta el 20/10
bool predecirEnDiagonal                          = true;  // en la diagonal, apuntar adonde VA a estar la pelota (solo si el robot estaba quieto). false = apunta adonde esta

// ---------- 2. LO QUE SE MIDE EN CADA CANCHA (no se tantea: se mide) ----------
// En una cancha nueva: 1ro umbralBlanco, 2do MS_MEDIO_AL_MEDIO. Las pruebas estan
// en pruebas/<nombre>/ con las instrucciones arriba de su .ino.
// 🚨 TIENEN COPIA ESCRITA A MANO en las pruebas, y alla se cambian a mano:
//    umbralBlanco: en ajustar-movimientos y monitor-robot, y como UMBRAL_BLANCO en
//       ir-al-medio, medir-ancho y medir-ancho-sin-giro. Cambiarlo ALLA ANTES de
//       medir MS_MEDIO_AL_MEDIO, o medir-ancho mide mal.
//    CAMARA_POR_CM: en ajustar-movimientos, monitor-robot y ver-prediccion.
//    MS_MEDIO_AL_MEDIO y MS_MEDIO_FONDO_POR_TIEMPO: en ir-al-medio, como
//       MS_AL_MEDIO y MS_FONDO_POR_TIEMPO.
int umbralBlanco                                 = 425;   // [MEDIDO 15/09] desde que lectura un sensor de linea dice blanco. En cada cancha: medirlo con calibrar-linea y copiarlo TAMBIEN en las pruebas (ver arriba)
const float CAMARA_POR_CM                        = 2.87;  // [MEDIDO 25/08] unidades de camara por cm real, pelota enfrente. Es de la CAMARA, no de la cancha: se mide de nuevo solo si se movio la camara (tabla-camara). Cambiarlo cambia TODOS los cm
const unsigned long MS_MEDIO_AL_MEDIO            = 2228;  // [MEDIDO 06/10, antes de mover los motores] del lateral izquierdo al medio a velocidad 100 (medir-ancho). Lo usan ir al medio, la patrulla y el limite lateral. Vale con los numeros de mezcla, rampa y rumbo de hoy: si se tocan, medirlo de nuevo
const unsigned long MS_MEDIO_FONDO_POR_TIEMPO    = 2000;  // [a ojo, no medido] ir al medio: cuanto retrocede buscando el fondo antes de darlo por encontrado (la rampita no lo deja pisar la linea)

// ---------- 3. SEGUIR LA PELOTA DE COSTADO ----------
// Se prueban con pruebas/ajustar-movimientos (movimientos 1 a 5 y 8).
float kpLateral                                  = 11.5;  // fuerza por cada cm que la pelota esta al costado
int pwmMinLateral                                = 45;    // lo minimo que empuja. Si se queda zumbando sin moverse: subir de a 5
int pwmMaxLateral                                = 150;   // lo maximo que empuja = la velocidad de costado. Si se pasa de largo y va y viene alrededor de la pelota: bajar a 135 o 125. Mas de 150 nunca se probo (a 160 choco la madera)
const float ZONA_MUERTA_PELOTA                   = 1.4;   // cm al costado que deja pasar sin moverse. Si tiembla de a poquito con la pelota quieta enfrente: subir
float msAnticipacionSeguir                       = 500;   // solo con predecirTrayectoria = true: cuanto adelanta la prediccion (ms)
const int LADO_FRENTE                            = 50;    // de costado, fuerza de las dos ruedas de adelante (%)
const int LADO_TRASERA                           = 89;    // de costado, fuerza de la rueda de atras (%). Si de costado gira solo: probar 2 mas o 2 menos con ajustar-movimientos. Cambia MS_MEDIO_AL_MEDIO

// ---------- 4. EL DESPEJE ----------
// Se prueba con pruebas/ajustar-movimientos (movimiento 6). OJO: potenciaDespeje, msAdelante y
// RAMPA_MOV_PASO cambian CUANTO SALE: medirlo con regla. Tiene que pasar los cm de umbralCm.
float umbralCm                                   = 30.0;  // sale a despejar con la pelota a esta distancia o menos (cm). ATADO a msAdelante: la ida tiene que llegar MAS LEJOS que esto. Si se sube, subir msAdelante
float umbralDesvio                               = 20.0;  // ...y a esto o menos al costado (cm). 27/10: de 7 a 20 PARA PROBAR la diagonal (pedido del equipo). El tope de 30 grados igual la frena: a 30 cm sale hasta ~17 cm, a 20 cm hasta ~11. OJO: saliendo derecho (sin giroscopio o sin diagonal) mas de 7 no la alcanza: volver a 7
const int VECES_PARA_CREERLE                     = 3;     // ...vista asi en esta cantidad de cuadros seguidos de la camara
int potenciaDespeje                              = 200;   // fuerza de la ida
int msAdelante                                   = 383;   // cuanto dura la ida (ms). [CUENTA, SIN PROBAR] para X cm: ms = (X + 3.3) x 10, a potencia 200 (04/08, antes de la rampa y del freno: puede salir menos). Si no toca la pelota: subir y medir con regla
const unsigned long MS_ESPERA_ANTES_DE_VOLVER    = 400;   // con frenarAntesDeVolver: cuanto se queda frenado antes de volver (ms). 400 lo eligio el equipo el 20/10
const unsigned long MS_PAUSA_MEDIO               = 150;   // con frenarAntesDeVolver = false: cuanto queda suelto antes de volver (ms)
int potenciaRetroceso                            = 110;   // fuerza de la vuelta hasta la linea (tambien al arrancar el partido)
const unsigned long MS_MAX_RETROCESO             = 1200;  // si no ve la linea, deja de retroceder a los (ms). Para que no se vaya de la cancha. Tambien decide al ARRANCAR si hace la rutina de ir al medio
unsigned long msFreno                            = 200;   // cuanto sostiene el freno electrico cada vez que frena (ms)
int potenciaEmpujon                              = 100;   // fuerza del empujoncito final. ATADA a msEmpujonFinal (los 10 cm son a 100)
int msEmpujonFinal                               = 320;   // cuanto dura el empujoncito (ms). [CUENTA de lo MEDIDO el 18/08: 400 ms = 13 cm a potencia 100] 320 = ~10 cm
const unsigned long MS_ENFRIAMIENTO              = 1500;  // despues de un despeje se queda QUIETO esto (ms): no sigue la pelota ni despeja. Mas = mas tiempo parado

// ---------- 4b. EL DESPEJE EN DIAGONAL (con despejeEnDiagonal = true) ----------
// Sale en diagonal hacia la pelota, mirando siempre al frente (el giroscopio no dirige: impide que gire).
// Cada rueda = avance + costado + correccion de rumbo. El costado sale de donde esta la pelota (regla de
// tres) y, si el robot estaba quieto, de cuanto se mueve. Vuelve por la misma diagonal. Si la cuenta dice
// que no llega, o que tendria que salir muy abierto, NO SALE y sigue acomodandose.
// Se prueba con pruebas/ajustar-movimientos (movimiento 9).
float rendimientoCostado                         = 0.80;  // [DE FE, nunca medido] cuanto rinde el costado contra el avance. OJO, va AL REVES: mas chico = MAS costado. Si no llega de costado (la pelota le queda mas afuera): BAJAR de a 0.05. Si se pasa de costado: SUBIR
const float MAX_ANGULO_DIAGONAL                  = 30.0;  // grados: si para encontrarse con la pelota tiene que salir mas abierto que esto, NO SALE (27/10, a ojo)
const unsigned long MS_QUIETO_PARA_PREDECIR      = 400;   // la velocidad de la pelota se usa solo si no se movio de costado en estos ms (con el robot moviendose, la camara se confunde)
const float PWM_POR_CM_S                         = 2.0;   // fuerza de costado por cada cm/s que se mueve la pelota (medido con regla)
const float PISO_VELOCIDAD                       = 4.0;   // velocidad de la pelota: debajo de esto (cm/s) es ruido de la camara y se toma como quieta
const float SUAVIZADO_VELOCIDAD                  = 0.3;   // velocidad de la pelota: cuanto pesa la lectura nueva (0 a 1)
const int PRESUPUESTO_RUEDA                      = 210;   // lo maximo por rueda sin contar la correccion. Es 255 - 45 escrito a mano: no se recalcula solo
const int PWM_MIN_AVANCE_DESPEJE                 = 140;   // si para que entre el costado el avance queda debajo de esto, NO SALE
const unsigned long MS_MAX_ADELANTE              = 800;   // tope duro de la ida en diagonal (ms)
const int COSTADO_MINIMO_UTIL                    = 20;    // menos costado que esto y sale derecho

// ---------- 4c. MIRAR ANTES DE SALIR (idea del equipo, 27/10) ----------
// Con la pelota a distanciaParaMirar o menos, el robot FRENA y se queda QUIETO mirandola. Quieto, la camara
// no se confunde con el movimiento del robot: mide la velocidad REAL de la pelota. Recien cuando la pelota
// esta a umbralCm o menos, sale en diagonal hacia donde VA a estar. Si no llega a umbralCm, vuelve a seguirla.
bool medirAntesDeDespejar                        = true;  // false = como antes: sale apenas la ve a umbralCm
float distanciaParaMirar                         = 40.0;  // cm: aca empieza a mirar. 0.1027c: 45 (esperaba de mas). Mas = ve venir pelotas mas rapidas, pero frena mas seguido y la camara ve peor de lejos
const unsigned long MS_ASENTAR_MIRADA            = 60;    // primero frena esto (ms) SIN medir: el robot todavia se esta frenando. 0.1027c: 100
const unsigned long MS_MIRAR                     = 120;   // y despues mide quieto esto (ms) como minimo, y 3 cuadros como minimo. 0.1027c: 200
const unsigned long MS_MAX_MIRANDO               = 500;   // tope de la mirada. Si la pelota NO VIENE (o no llega a umbralCm antes de esto), vuelve a seguirla enseguida. 0.1027c: 700, y esperaba siempre
const unsigned long MS_DESCANSO_MIRADA           = 500;   // despues de eso, no vuelve a frenarse a mirar hasta que pase esto (salvo que la pelota ya este a umbralCm)

// ---------- 5. IR DERECHO: la correccion de rumbo con el giroscopio ----------
// Se ajustan con pruebas/ajustar-movimientos (AJUSTAR.bat): las teclas los cambian sin cargar. Lo que ande, se copia aca.
const float KP_RUMBO                             = 5.0;   // cuanto corrige por cada grado torcido. Si zigzaguea: bajar de a 0.5. Si se le escapa: subir de a 0.5
const float KI_RUMBO                             = 1.5;   // corrige la torcida que queda fija para un lado
const float LIMITE_INTEGRAL                      = 30.0;  // tope de lo que acumula KI_RUMBO
const int MAX_CORRECCION_RUMBO                   = 100;   // tope de la correccion, en fuerza
const float ZONA_MUERTA_RUMBO                    = 1.0;   // grados que deja pasar sin corregir
const int RAMPA_MOV_PASO                         = 10;    // arranque suave de TODOS los movimientos: sube esto de fuerza... (menos = mas suave, pero cambia las distancias por tiempo: msAdelante, MS_MEDIO_AL_MEDIO)
const unsigned long RAMPA_MOV_MS                 = 10;    // ...cada esta cantidad de ms

// ---------- 6. ENDEREZARSE ----------
const float TOLERANCIA_ACOMODO                   = 3.0;   // grados: mas derecho que esto ya esta enderezado
const float KP_ACOMODO                           = 1.5;   // fuerza de giro por cada grado torcido. Hoy casi no hace nada: los topes de abajo (75 y 85) deciden
const int PWM_MIN_ACOMODO                        = 75;    // fuerza minima de giro. NO bajar de ~70: parado no arranca (01/09)
const int PWM_MAX_ACOMODO                        = 85;    // fuerza maxima de giro (con 110 oscilaba)
const unsigned long MS_ASENTAR_ACOMODO           = 300;   // quieto esto (ms) antes de volver a medir, para que pase la inercia
const unsigned long MS_TIMEOUT_ACOMODO           = 3000;  // si no se endereza en esto (ms), corta el intento
const int MAX_REINTENTOS_ACOMODO                 = 3;     // cuantas veces mas lo intenta
const int RAMPA_PASO                             = 3;     // el giro arranca suave: sube esto de fuerza...
const unsigned long RAMPA_MS                     = 15;    // ...cada esta cantidad de ms
const float TOLERANCIA_ESPERA                    = 8.0;   // esperando la pelota: mas torcido que esto (grados) y se endereza solo. Siempre BASTANTE mas grande que TOLERANCIA_ACOMODO: si quedan cerca, se la pasa enderezandose
const unsigned long MS_CHEQUEO_ESPERA            = 300;   // esperando: cada cuanto mira si esta torcido (ms)
const unsigned long MS_DESCANSO_ENDEREZAR        = 6000;  // esperando: tiempo minimo entre dos enderezados (ms)

// ---------- 7. PATRULLA, IR AL MEDIO Y LIMITE LATERAL ----------
// Los de 'ir al medio' son COPIA de pruebas/ir-al-medio, donde se llaman SIN "MEDIO_" (MS_ALEJARSE,
// MS_ADELANTE, MS_TOPE_LATERAL, MS_QUIETO, VEL_COSTADO_BUSCAR, VEL_COSTADO_MEDIR, VEL_ATRAS, LADO):
// si se cambia uno aca, cambiarlo alla tambien.
const unsigned long MS_QUIETO_PARA_PATRULLAR     = 10000; // patrulla si lleva esto quieto (ms)... (15000 -> 10000 el 13/10). Menos = el juez lo ve vivo, pero el arco queda solo mas seguido
const unsigned long MS_SIN_PELOTA_PARA_PATRULLAR = 3000;  // ...y esto seguido sin ver la pelota (ms)
const int CUADROS_PARA_CORTAR_PATRULLA           = 5;     // patrullando, si ve la pelota esta cantidad de cuadros seguidos, la corta y la sigue
const int PWM_CUENTA_COMO_MOVERSE                = 60;    // fuerza desde la que cuenta como 'se movio' para el reloj de la patrulla
const int LECTURAS_PARA_CREER_LATERAL            = 3;     // limite lateral: vueltas seguidas viendo el lateral para creerle
const int VEL_MEDIO_BUSCAR                       = 110;   // ir al medio: velocidad de costado buscando una linea
const int VEL_MEDIO_MEDIR                        = 100;   // ir al medio: velocidad del tramo por tiempo. ATADA a MS_MEDIO_AL_MEDIO (medido a 100)
const int VEL_MEDIO_RECTO                        = 110;   // ir al medio: velocidad para atras y adelante. Atada a MS_MEDIO_ADELANTE
const int LADO_MEDIO                             = -1;    // ir al medio: -1 = busca el lateral IZQUIERDO
const unsigned long MS_MEDIO_ALEJARSE            = 400;   // ir al medio: cuanto se despega del lateral (ms)
const unsigned long MS_MEDIO_ADELANTE            = 943;   // ir al medio: cuanto avanza para salir del area (ms, ~50 cm a 110)
const unsigned long MS_MEDIO_TOPE_LATERAL        = 6000;  // ir al medio: si no ve el lateral en esto (ms), lo da por encontrado
const unsigned long MS_MEDIO_QUIETO              = 300;   // ir al medio: quieto entre paso y paso (ms)

// ---------- 8. ARRANQUE Y GIROSCOPIO ----------
const unsigned long MS_AVISO_ARMADO              = 2000;  // al prenderlo, cuenta esto (ms) antes de moverse: para sacar la mano
const int INTENTOS_GIRO                          = 1;     // al prenderlo, cuantas veces pregunta por el giroscopio (29/09: 1; antes 10)
const unsigned long MS_ESPERA_DATOS_GIRO         = 5000;  // al prenderlo, cuanto espera a que el giroscopio de un dato (ms)
const unsigned long MS_ESPERA_EXTRA_GIRO         = 2000;  // antes de armarse, cuanto mas espera al giroscopio si no da rumbo (ms)
const unsigned long MS_PARPADEO_AVISO            = 50;    // parpadeo del LED cuando no hay giroscopio (ms prendido y ms apagado)

// ---------- 9. PARA QUE LADO: tocar SOLO si el robot hace lo contrario ----------
bool lateralInvertido                            = false; // [MEDIDO 11/08] se prueba SIN pelota: ajustar-movimientos, movimiento 2 (costado DERECHA). Si va a la IZQUIERDA, cambiar esto
bool camaraYInvertida                            = true;  // [MEDIDO 11/08] cambiar SOLO si el movimiento 2 de ajustar-movimientos va bien a la derecha y aun asi, siguiendo la pelota, se aleja de ella

// ---------- 10. AVANZADO: casi nunca se tocan ----------
float kpPersecucion                              = 6.0;   // perseguirEnElDespeje (apagado): fuerza de costado por cada cm de desvio
const int PWM_MAX_PERSECUCION                    = 60;    // perseguirEnElDespeje (apagado): tope de esa fuerza
float msAnticipacion                             = 0;     // prediccion para DECIDIR el despeje (ms). Solo con predecirTrayectoria = true. En 0 no cambia nada
const int RESERVA_RUMBO                          = 45;    // ⚠️ no se usa en ningun lado: cambiarlo no hace nada (el que manda es PRESUPUESTO_RUEDA)

// =====================================================================
//                       fin del panel de control
// =====================================================================

/* =====================================================================
   HISTORIA: lo que cambio en cada clase (lo mas nuevo arriba)
   =====================================================================

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-27 (5): MIRAR MAS CORTO (0.1027d)
   ---------------------------------------------------------------------
   Probaron la c: "el robot espera mucho para poder despejar y despeja
   muchas menos pelotas que antes". Tres causas, tres arreglos:
     - miraba minimo 300 ms (100 frenando + 200 midiendo): ahora 180
       (60 + 120, y 3 cuadros como minimo);
     - si la pelota no llegaba a umbralCm, esperaba SIEMPRE hasta 700 ms:
       ahora espera solo si la pelota VIENE y va a llegar antes de 500 ms;
       si no, vuelve a seguirla enseguida;
     - si el robot YA estaba quieto y la pelota ya esta a umbralCm, no se
       frena a mirar: sale ya, como la b (quieto, la velocidad ya es buena).
   Y empieza a mirar a 40 cm (era 45).

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-27 (4): MIRAR ANTES DE SALIR (0.1027c)
   ---------------------------------------------------------------------
   Probaron la 0.1027b: sale un poco mas en diagonal, pero a las pelotas
   rapidas no llega (sale, pero apunta adonde la pelota ESTABA). Idea del
   equipo: que con la pelota en rango el robot se quede quieto 100-300 ms y
   ahi calcule la trayectoria real (quieto, la camara no se confunde).

   Como quedo (seccion 4c del panel):
     - a distanciaParaMirar (45 cm) FRENA (100 ms) y MIRA quieto (200 ms
       minimo), trazando la recta de la pelota con todos los cuadros;
     - sale recien con la pelota a umbralCm (30), con la velocidad medida
       de costado Y de acercamiento (si se viene encima, se encuentran antes);
     - si en 700 ms no llego a umbralCm, vuelve a seguirla, y por 500 ms no
       se vuelve a frenar a mirar (salvo que ya este a umbralCm);
     - por el cable (MIRAR.bat) dice que vio y si salio o por que no.

        medirAntesDeDespejar   true -> false   (para volver: sale como la b)

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-27 (3): ARREGLOS DE LA REVISION (0.1027b)
   ---------------------------------------------------------------------
   Una revision del codigo de la diagonal (solo leyendo, sin robot) encontro
   dos cosas que no hacian lo que se habia dicho:
     - SIN GIROSCOPIO salia en diagonal igual, y sin rumbo sale girando.
       Ahora sale derecho, como el limite lateral, la patrulla e ir al medio.
     - En la ida el costado entraba entero y el avance subia con la rampa:
       salia ~1,3 veces mas abierto que lo calculado. Ahora el costado sube
       con la rampa (avanzarPersiguiendo).
   Y un texto al reves: rendimientoCostado se DIVIDE, asi que bajarlo da
   MAS costado. El panel decia lo contrario.

   Con giroscopio y la pelota centrada no cambia nada: sale derecho igual.

   Y PARA PROBAR, a pedido del equipo (vieron que la diagonal no pasaba de
   ~10 grados): umbralDesvio 7 -> 20 cm. El tope de 30 grados sigue.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-27 (2): EL DESPEJE EN DIAGONAL, PRENDIDO
   ---------------------------------------------------------------------
   Idea del equipo: como la prediccion no anduvo en el seguimiento, usarla
   solo para despejar. Eligieron la diagonal mirando siempre al frente (y no
   girar y despejar derecho), con el giroscopio sosteniendo el frente.

   La diagonal estaba escrita desde el 08/09 (tambien pedido del equipo) y
   apagada desde el 15/09 porque no habia giroscopio. Se prendio con dos
   cambios en calcularDiagonal():
     - la velocidad de la pelota se usa SOLO si el robot no se movio de
       costado en los ultimos MS_QUIETO_PARA_PREDECIR (la camara va arriba
       del robot y confunde su movimiento con el de la pelota);
     - si tiene que salir mas abierto que MAX_ANGULO_DIAGONAL, no sale.

        despejeEnDiagonal    true   ->   false   (para volver: sale derecho)
        predecirEnDiagonal   true   ->   false   (apunta adonde esta la pelota)

   ⚠️ SIN PROBAR EN CANCHA. rendimientoCostado (0,80) es "de fe": es lo
   primero que hay que ajustar. Se prueba con ajustar-movimientos (9).

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-27: EL PANEL DE CONTROL
   ---------------------------------------------------------------------
   Pedido del profe (13/10) y del equipo: todo lo que se ajusta, junto
   arriba, para encontrarlo rapido y sin internet. Se MOVIERON 86
   declaraciones al PANEL DE CONTROL, sin cambiar ningun valor: un script
   las copio tal cual y comparo las 166 declaraciones del archivo, antes y
   despues. Donde estaba cada una quedo una nota "en el PANEL DE CONTROL
   (arriba)" y, alrededor, su historia de siempre.

   El panel solo no cambia nada de lo que hace el robot: compilado con el
   mismo cartel, mide exactamente igual que la 0.1020. Lo que si cambia es
   el despeje en diagonal, que se prendio el mismo dia (ver arriba).

   INTENTOS_GIRO estaba adentro de setup(); ahora esta en el panel.
   msAdelanteChico y VEL_ROBOT_CM_S no se usan en ningun lado: quedaron
   donde estaban, con un aviso, y no van al panel.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-20 (2): LA PREDICCION SE APAGA DE NUEVO
   ---------------------------------------------------------------------
   Con la pelota quieta enfrente, prendida hacia ir y venir al robot sin
   centrarse (lo vio el equipo y lo midio ajustar-movimientos). Ver donde
   se declara.

        predecirTrayectoria   true   ->   false   (para volver: true)

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-20: EL DESPEJE FRENA ANTES DE VOLVER
   ---------------------------------------------------------------------
   Pedido del equipo: al llegar al tope de la ida el robot se inclina y
   recien despues vuelve al piso. Ahora, al terminar la ida, FRENA, se
   queda frenado 400 ms y recien ahi retrocede hasta la linea. Antes
   soltaba los motores 150 ms y arrancaba para atras de golpe.

        frenarAntesDeVolver         true   ->   false   (para volver)
        MS_ESPERA_ANTES_DE_VOLVER   400 ms (cuanto se queda frenado)

   Primero se puso 100; el equipo probo en `pruebas/ajustar-movimientos`
   (movimiento 6) y con 400 anduvo "decentemente mejor". Falta verlo en el
   programa de partido, en cancha.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-13 (4): LA PREDICCION VUELVE A ESTAR PRENDIDA
   ---------------------------------------------------------------------
   Pedido del equipo, despues de congelar la ESTABLE 0.1013. Es UN
   interruptor:

        predecirTrayectoria   false   ->   true   (para volver: false)

   Se habia apagado el 12/10 porque el robot oscilaba de lado a lado con la
   pelota lejos; no quedo anotado si apagada dejo de oscilar. Si prendida
   vuelve a oscilar, el paso intermedio es dejarla en true y bajar
   msAnticipacionSeguir de 500 a 250.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-13: LA PATRULLA SALE A LOS 10 s
   ---------------------------------------------------------------------
   Pedido del equipo. Es UN numero: cuanto espera quieto y sin ver la
   pelota antes de salir al lateral y volver al medio.

        MS_QUIETO_PARA_PATRULLAR   15000   ->   10000   (para volver: 15000)

   La patrulla en si no cambia (~8 s, unos 5 con el arco descubierto), pero
   ahora sale mas seguido, asi que el arco queda descubierto mas seguido.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-13 (2): LIMITE LATERAL, VUELVE AL MEDIO
   ---------------------------------------------------------------------
   Pedido del equipo: siguiendo la pelota se iba de largo por el costado,
   chocaba el muro de afuera y el juez lo sacaba 1 minuto. Ahora, si
   siguiendo la pelota pisa la linea blanca del lateral:

        frena  ->  vuelve al medio por tiempo  ->  retrocede hasta la linea
        del area  ->  se endereza y avanza el poquito

   Cuenta como lateral solo si ve blanco el sensor de atras del lado hacia
   el que va y el otro NO (los dos juntos es la linea del area). El tiempo
   de vuelta es el MISMO desde los dos laterales (MS_MEDIO_AL_MEDIO, 2228),
   decision del equipo. Desde el derecho la cuenta del 06/10 da 2331.

        volverAlMedioEnElLateral = false   ->  sin limite, como antes

   El equipo lo probo en cancha el 13/10 y dijo que anda bien (no quedaron
   numeros anotados). ⚠️ Mientras vuelve al medio (~2,7 s) no mira la
   pelota. Sin giroscopio no hay limite (no puede volver por tiempo).

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-13 (3): EL DESPEJE SALE 35 cm EN VEZ DE 50
   ---------------------------------------------------------------------
   Pedido del equipo: sale a buscar una pelota que esta a 30 cm y se iba de
   largo hasta los 50. Es UN numero. El disparo sigue en 30 cm.

        msAdelante   533 (~50 cm)   ->   383 (~35 cm)   (para volver: 533)

   ⚠️ SIN PROBAR EN CANCHA. Le quedan 5 cm para empujar la pelota (antes
   20). El 15/09, con 0 de sobra, no llegaba a tocarla.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-12: COMO SE UBICA AL ARRANCAR
   ---------------------------------------------------------------------
   Pedido del equipo. Al armarse el robot sigue retrocediendo hasta la linea
   del area, igual que antes. Lo que cambia es lo que hace DESPUES:

        la ENCUENTRA     ->  se endereza y avanza un poquito (~10 cm), para
                             no quedar sentado sobre el blanco
        NO la encuentra  ->  hace la rutina de `pruebas/ir-al-medio`: busca
                             el lateral, el fondo, vuelve al lateral, va al
                             medio por tiempo y retrocede hasta la linea del
                             area. Ahi se endereza y avanza el poquito

   El FONDO se busca por linea O POR TIEMPO (2000 ms): la rampita del borde
   de la cancha no deja que el robot llegue a pisar esa linea.

   ---------------------------------------------------------------------
   📋 CAMBIO DEL 2026-10-12 (2): PATRULLA, PARA QUE EL JUEZ NO LO SAQUE
   ---------------------------------------------------------------------
   Pedido del equipo despues de ganar 2-0: sin pelota a la vista el robot
   se quedaba quieto para siempre y el juez lo saco creyendo que estaba
   roto. Ahora, si pasa 15 s sin moverse y sin ver la pelota:

        va al lateral  ->  vuelve al medio por tiempo  ->  retrocede hasta
        la linea del area  ->  se endereza y avanza el poquito

   Si aparece la pelota en el camino (5 cuadros seguidos), corta y la sigue.
   Y los laterales ahora tambien terminan POR TIEMPO si no ven la linea,
   en vez de abandonar y quedar estacionado en el borde.

        patrullarSinPelota = false          ->  vuelve a quedarse quieto
        MS_QUIETO_PARA_PATRULLAR = 15000    ->  cada cuanto patrulla
                                                (10000 desde el 13/10)

   ⚠️ SIN PROBAR EN CANCHA. Cada patrulla son ~8 s y unos 5 con el arco
   descubierto.

   Antes, en los dos casos se quedaba donde habia frenado.

   Son DOS interruptores, uno por cada cosa, donde dice "UBICACION INICIAL":
        avanzarAlUbicarse      = false   ->  vuelve a quedarse sobre la linea
        irAlMedioSiNoHayLinea  = false   ->  vuelve a quedarse donde freno

   ⚠️ TODAVIA NO SE PROBO EN CANCHA. Los tiempos de la rutina son los de
   `pruebas/ir-al-medio`, medidos el 06/10, ANTES de meter los motores mas
   adentro. Si termina corrido del medio, hay que volver a medir.

   ---------------------------------------------------------------------
   🧪 PRUEBA DEL 2026-10-12: LA PREDICCION ESTA APAGADA
   ---------------------------------------------------------------------
   El robot oscila de lado a lado cuando la pelota esta lejos. Para saber si
   la culpa es de la prediccion, se apago:

        predecirTrayectoria   false   ->   true   (para volver a como estaba)

   Es UN interruptor. La explicacion esta donde se declara. Si con esto se
   queda quieto, era la prediccion; si oscila igual, no era y se vuelve atras.

   ---------------------------------------------------------------------
   📋 CAMBIOS DEL 2026-10-06, Y COMO VOLVER ATRAS
   ---------------------------------------------------------------------
   Se tocaron DOS numeros, los dos para que el robot se adelante mejor a la
   pelota. Si algo anda peor que antes, estos son los valores de vuelta:

        pwmMaxLateral             150   ->   120   (era asi antes de hoy)
        msAnticipacionSeguir      500   ->   250   (o borrar la constante y
                                                    usar msAnticipacion)

   Los dos estan con su explicacion completa en el lugar donde se declaran.
   Y los dos son UN NUMERO: volver atras es cambiarlo, nada mas.

   🧊 Y si se quiere volver a TODO como estaba, sin pensar: esta la copia
   congelada en `respaldos/estable-0.0929/`, que es este mismo programa tal
   como quedo el 29/09. Se carga desde su propia carpeta y su banner avisa
   "RESPALDO ESTABLE 0.0929" para no confundirla con esta.

   POR QUE LOS DOS JUNTOS Y NO DE A UNO: porque estaban peleados. Subir la
   anticipacion sola no hacia nada con pelota rapida, porque el tope de 120
   ya recortaba la fuerza (120/11,5 = 10,4 cm de desvio y ya saturaba). Y
   subir el tope solo tampoco alcanzaba, porque el robot seguia mirando a
   donde la pelota ESTABA. Hacian falta los dos.

   ---------------------------------------------------------------------

   ESTE ES EL PROGRAMA DE PARTIDO. Es el que se carga para jugar.
   Todo lo que esta en `pruebas/` son herramientas de medicion: no juegan.

   El numero de version es la fecha: 0.1013 = 2026-10-13. Hay una copia
   congelada de esta misma version en
        funciona/seguir-y-despejar/../../respaldos/estable1013conlimiteslaterales/
   que se carga tal cual y no se toca nunca mas. Si algo se rompe mientras
   se experimenta aca, se vuelve a esa carpeta y listo. (La ESTABLE anterior,
   la 0.0929, sigue en respaldos/estable-0.0929/.)

   ⚠️ QUE TAN "ESTABLE" ES, dicho con honestidad:
   La declaro estable el equipo el 2026-10-13, probandola en cancha ese
   mismo dia: dijo que andan bien el despeje, la vuelta al medio del area y
   el limite lateral ("la version actual esta buena"). Eso fue ANTES del
   ultimo cambio, el despeje de 50 a 35 cm, que se cargo junto con este
   cartel: al congelarla no quedo anotado si se probo. No jugo ningun
   partido, no hay numeros medidos de ese dia, y los tiempos de costado son
   de antes de mover los motores (12/10).

   Lo que este lugar decia de la ESTABLE anterior (0.0929), por si se vuelve
   a ella: la version que gano los amistosos del 2026-09-21 (5-0 y 3-2, con
   una derrota 3-1) era la 0.0929 con UNA diferencia: aquella preguntaba por
   el giroscopio hasta 10 veces al arrancar y la 0.0929 pregunta UNA sola
   (INTENTOS_GIRO, pedido del equipo el 29/09; sigue asi). Ese cambio no se
   probo en cancha antes de congelarla. Y el 2026-09-29 se descubrio que el
   cable del giroscopio estaba roto: sin rumbo el LED parpadea rapido, y eso
   no es culpa del programa.
   =====================================================================

   QUE HACE
   El robot se para en su area mirando la cancha, con el arco a la espalda,
   y NUNCA gira: siempre mira al frente.

        1. Sin pelota a la vista: quieto.
        2. Ve la pelota lejos  -> se corre DE COSTADO para ponerse enfrente
                                  de ella, sin dejar de mirar al frente.
        3. La pelota llega a 30 cm -> DESPEJA: sale ~35 cm, la saca, vuelve
                                  hasta pisar la linea, se endereza, y da
                                  un empujoncito de 10 cm.

   ⚠️ Sale 35 cm para una pelota que estaba a 30 (desde el 2026-10-13; antes
   salia 50 y quedaba lejos del arco). Le sobran 5 cm para empujarla: si no
   llega a tocarla, hay que subir la ida.

   ⚠️ EL LIMITE DEL SEGUIMIENTO DE COSTADO. Hasta el 2026-08-18 se frenaba
   al pisar una linea; se saco porque el robot vuelve chueco de algunos
   despejes y estando torcido la pisaba antes de tiempo. Desde el
   2026-10-13 hay limite otra vez, distinto: al pisar el LATERAL vuelve al
   medio (ver el cambio del 13/10 (2), arriba).

   La linea tambien se usa para volver del despeje. Son dos usos distintos
   de los mismos dos sensores de atras.

   ---------------------------------------------------------------------
   POR QUE EL GIROSCOPIO NO PUEDE MEDIR CUANTO SE MOVIO DE COSTADO
   ---------------------------------------------------------------------
   Vale la pena que quede escrito porque es una confusion facil.

   El giroscopio mide HACIA DONDE MIRA el robot, no DONDE ESTA. Si el robot
   se corre 20 cm al costado sin girar, el giroscopio marca exactamente lo
   mismo antes y despues. Y este robot no tiene encoders en las ruedas ni
   sensor de distancia, asi que no hay ninguna forma de medir cuanto se
   desplazo.

   Por eso el limite del movimiento lateral es la LINEA BLANCA — una marca
   fisica de la cancha — y no una cuenta del giroscopio.

   Lo que SI hace el giroscopio, y es lo que pidio el profe: mantenerlo
   DERECHO. Al moverse de costado, un omni de tres ruedas tiende a girar
   un poco (la rueda trasera empuja distinto que las dos de adelante). El
   giroscopio detecta ese giro y lo corrige mientras se mueve.

   ---------------------------------------------------------------------
   COMO SE MEZCLAN MOVERSE Y ENDEREZARSE
   ---------------------------------------------------------------------
   El codigo 2025 resolvia esto con tablas de valores fijos: tres ramas de
   `if` con numeros escritos a mano (50/50/89, 65/40/100...) segun cuanto
   estaba torcido. Funciona, pero no se puede ajustar sin reescribir la
   tabla.

   Aca se hace sumando dos cosas separadas:

        lo que cada rueda tiene que hacer para IR DE COSTADO
      + lo que cada rueda tiene que hacer para NO GIRAR
      = lo que se le manda a esa rueda

   Moverse de costado son las tres ruedas en la proporcion 50/50/89 (esa
   proporcion sale de la geometria del robot y es la del codigo 2025).
   No girar son las tres ruedas parejas, en la misma direccion.

   Como son dos cosas independientes, se calculan por separado y se suman.
   Asi se puede tocar una sin romper la otra.

   ---------------------------------------------------------------------
   🚨 DOS SIGNOS QUE HAY QUE DESCUBRIR PROBANDO
   ---------------------------------------------------------------------
   No se pueden deducir leyendo el codigo, hay que medirlos:

     1. Cuando el programa dice "andá a la derecha", ¿el robot va a la
        derecha? Los comentarios del codigo 2025 estan espejados, asi que
        no sirven de referencia.         -> lateralInvertido

     2. Cuando la camara dice que la pelota esta desviada +5, ¿esta a la
        derecha o a la izquierda?        -> camaraYInvertida

   Si alguno esta al reves, el robot se aleja de la pelota en vez de
   seguirla. Se nota en dos segundos. Los dos ya estan medidos (abajo).

   ---------------------------------------------------------------------
   SIN TECLAS NI TERMINAL (2026-09-21)
   ---------------------------------------------------------------------
   A pedido del equipo se saco todo lo de la consola: teclas, mensajes,
   monitor en vivo y pruebas por tecla. En la cancha no hay cable y nunca
   se usaban. El robot arranca solo al prenderlo y se para con la llave
   de la bateria. Lo unico que avisa es el LED. Para cambiar un valor, se
   cambia aca en el codigo y se vuelve a cargar.

   ---------------------------------------------------------------------
   MEDICIONES QUE USA (todas hechas en banco o en cancha)
   ---------------------------------------------------------------------
   Ruedas    U5=2/5/3 izquierda · U17=8/7/6 derecha · U7=11/12/4 trasera
   Linea     A12 adelante · A13 atras-IZQ · A11 atras-DER
             verde ~356 y ~465 · blanco ~760 · umbral 620
   Distancia 1 cm cada 10 ms a potencia 200, con ~33 ms de arranque
   Camara    9 bytes a 19200 por Serial1. Xp=0 significa NO LA VEO
             2,87 unidades de camara = 1 cm real (medido el 2026-08-25 con
             regla, 5 posiciones). La camara esta a ~8 cm del piso, no a
             los 18,7 para los que la calibraron en 2025.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


// ---- ARQUERO (ROBOT1) ----
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = IZQUIERDA (U5)

#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = DERECHA (U17)

#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = TRASERA (U7)

#define LED 13

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12

const unsigned long BAUDIOS = 19200;


// ---- los dos signos, YA DESCUBIERTOS (2026-08-11, en banco) ----
//
// Lateral: se mando una prueba corta de costado y el robot fue efectivamente
// a la derecha. Queda como esta.
// lateralInvertido: en el PANEL DE CONTROL (arriba).
//
// Camara: con la pelota puesta a la DERECHA del robot, la camara mandaba
// Yp = -24. O sea que el negativo es la derecha, al reves de lo que
// suponia el codigo. Por eso va invertido.
// camaraYInvertida: en el PANEL DE CONTROL (arriba).


// ---- 🎯 LA CAMARA NO HABLA EN CENTIMETROS: HAY QUE TRADUCIRLA ----
//
// La camara no mide distancia. Ve en que pixel aparecio la mancha naranja y
// lo traduce a centimetros con una tabla de conversion que quedo escrita en
// su programa en 2025, calibrada para una camara a 18,7 cm del piso.
//
// La camara de ESTE robot esta a ~8 cm. Por eso exagera: dice numeros mucho
// mas grandes que la distancia real.
//
// MEDIDO EN CANCHA EL 2026-08-25, cinco posiciones con regla:
//
//       pelota a  10 cm  ->  la camara dice   28     factor 2,80
//       pelota a  20 cm  ->  la camara dice   58     factor 2,90
//       pelota a  30 cm  ->  la camara dice   89     factor 2,97
//       pelota a  40 cm  ->  la camara dice  117     factor 2,93
//       pelota a  50 cm  ->  la camara dice  142     factor 2,84
//
// El factor es PAREJO. Eso es lo que dice que cambio la ALTURA y no la
// inclinacion: una inclinacion distinta daria un factor que crece con la
// distancia. Por eso alcanza con dividir, sin tabla ni interpolacion.
//
// Confirmacion independiente: despejando la cuenta, la camara tendria que
// estar a 7,9 cm. Se midio con regla y da 7-8. Cierra.
//
// ⚠️ Medido entre 10 y 50 cm. Fuera de ese rango no esta comprobado.
//
// 🚨 DE ACA EN ADELANTE, TODO EL PROGRAMA HABLA EN CENTIMETROS REALES.
// Lo unico en unidades de camara son Xp e Yp, que son lo que llega crudo
// por el cable.
// CAMARA_POR_CM: en el PANEL DE CONTROL (arriba).  unidades de camara por cm real

// ---- seguimiento lateral ----
// La proporcion 50/50/89 entre las ruedas sale de la geometria del robot;
// es la que usa el codigo 2025 en ai/adproporcional. Lo que cambia es la
// FUERZA con que se aplica, que aca es proporcional a lo desviada que esta
// la pelota en vez de ser un numero fijo.
// LADO_FRENTE: en el PANEL DE CONTROL (arriba).  las dos de adelante
// LADO_TRASERA: en el PANEL DE CONTROL (arriba).  la de atras

// 🚨 kpLateral SUBIO DE 4.0 A 11.5 EL 2026-08-25, Y NO ES UN AJUSTE.
// Es la misma fuerza de antes, escrita en la unidad nueva. Antes el desvio
// llegaba en unidades de camara; ahora llega en centimetros reales, que son
// numeros 2,87 veces mas chicos. Para que el robot empuje igual, el numero
// que los multiplica tiene que ser 2,87 veces mas grande:
//
//       4.0 x 2,87 = 11,5
//
// Si algun dia alguien "corrige" esto de vuelta a 4, el robot va a seguir la
// pelota casi tres veces mas flojo y va a parecer que se rompio.
// kpLateral: en el PANEL DE CONTROL (arriba).  PWM por cm REAL de desvio de la pelota
// 🎯 2026-10-06: 60 -> 45, buscando que el robot no salte con la pelota quieta.
//
// ⚠️ ACLARACION: este numero NUNCA SE CAMBIO HOY, venia en 60 de antes. Si
// cuando pidieron "bajar el piso al que tenia antes" se referian a otro
// numero, hay que decirlo.
//
// POR QUE BAJARLO IGUAL, que la idea del equipo es buena: con el piso en 60
// NO EXISTE LA CORRECCION SUAVE. Cualquier desvio que apenas pase la zona
// muerta recibe el mismo empujon que un desvio grande:
//
//      desvio de 1,5 cm  ->  1,5 x 11,5 = 17  ->  se sube a 60
//      desvio de 5,0 cm  ->  5,0 x 11,5 = 58  ->  se sube a 60
//
// O sea que el robot pega el mismo salto por 1,5 cm que por 5 cm. Con la
// pelota quieta y el ruido de la camara entrando y saliendo de la zona
// muerta, eso es exactamente un robot saltando de lado a lado.
//
// ⚠️ EL RIESGO: el 60 estaba ahi porque abajo de eso "no se mueve, solo
// zumba". Si con 45 el robot zumba sin moverse cuando el desvio es chico,
// subirlo de a 5. El piso de arranque de los motores desde quieto es ~70,
// pero de costado la rueda mas cargada recibe 89% del valor, asi que 45 le
// da 40 a la trasera: justo el piso de RODADURA. Moviendose alcanza; desde
// parado puede no arrancar.
// pwmMinLateral: en el PANEL DE CONTROL (arriba).

// 🎯 2026-10-06: 120 -> 150, a pedido del equipo.
//
// POR QUE. Lo destapo una pregunta del equipo: "¿el seguimiento lateral tiene
// un tope de velocidad?". Si, y la cuenta de cuando satura es esta:
//
//       120 / kpLateral  =  120 / 11,5  =  10,4 cm de desvio
//
// O sea que con la pelota a mas de 10 cm al costado, el robot YA IBA AL TOPE:
// 10 cm o 30 cm daban lo mismo. Y eso explica algo que acababamos de hacer:
// subir la anticipacion del seguimiento de 250 a 500 ms NO cambiaba nada con
// pelota rapida, porque el desvio ya pasaba los 10,4 cm y la fuerza ya estaba
// recortada. La prediccion cambiaba hacia DONDE mira, pero el robot no podia
// ir mas rapido. Con pelota rapida el que manda es ESTE numero.
//
//       150 / 11,5 = 13 cm antes de saturar
//
// Como queda el reparto de las ruedas con 150 (mezcla 50/50/89):
//       las dos de adelante:  75     (el piso de arranque es ~70: justo arriba)
//       la de atras:         133
// Y la correccion de rumbo puede sumar hasta 100 encima: 233, debajo de 255.
// Entra.
//
// ⚠️ EL COSTO, PARA SABER QUE MIRAR: mas velocidad es mas envion, y al llegar
// al desvio cero el robot se pasa de largo. Si empieza a oscilar de lado a
// lado alrededor de la pelota en vez de quedarse, bajarlo (135, 125). La zona
// muerta de 1,4 cm ayuda, pero no tapa un envion grande.
// pwmMaxLateral: en el PANEL DE CONTROL (arriba).
// 4.0 de camara / 2,87 = 1,4 cm reales. Mismo comportamiento que antes.
// ZONA_MUERTA_PELOTA: en el PANEL DE CONTROL (arriba).  cm REALES: no perseguir migajas

// ---- despeje ----
// potenciaDespeje: en el PANEL DE CONTROL (arriba).
// potenciaRetroceso: en el PANEL DE CONTROL (arriba).
// La ida tiene que ALCANZAR a la pelota: si el robot dispara a 48 cm y solo
// avanza 30, frena 18 cm antes y no la toca nunca. Por eso este numero sube
// junto con umbralCm.
// Calibrado 2026-08-04 con regla: distancia_cm = tiempo_ms/10 - 3.3
//     50 cm -> (50 + 3.3) * 10 = 533 ms
// 2026-09-15, DOS PASADAS EN LA MISMA CLASE:
//
//   533 ms (~50 cm) -> 333 (~30) : la cancha nueva es mas chica y salir
//                                  medio metro dejaba el arco muy solo.
//   333 ms (~30 cm) -> 433 (~40) : PROBADO EN CANCHA y quedaba CORTO,
//                                  "ni llegaba a tocar la pelota".
//
// La cuenta sale de la calibracion con regla del 04/08:
//        distancia_cm = tiempo_ms / 10 - 3,3
//        40 cm  ->  (40 + 3,3) x 10 = 433 ms
//
// 🎯 POR QUE QUEDABA CORTO, Y POR QUE AHORA CIERRA. Con la ida en 30 y el
// disparo tambien en 30, el robot llegaba JUSTO a donde estaba la pelota
// y no le sobraba ni un centimetro para empujarla. Ahora dispara a 20 y
// avanza 40: le sobran 20 cm de empuje. La regla vuelve a cumplirse con
// margen, que era lo que faltaba.
// 2026-09-21: +10 cm a pedido del equipo, junto con el disparo (+10 cm),
// para que quede parejo: dispara a 30 y avanza 50 -> sobran 20 de empuje,
// igual que antes (20 y 40).
//        50 cm  ->  (50 + 3,3) x 10 = 533 ms
// 2026-10-13: 533 (~50 cm) -> 383 (~35), pedido del equipo: salia a buscar
// una pelota que estaba a 30 y se iba de largo hasta los 50. El disparo
// sigue en 30, asi que ahora sobran 5 cm de empuje (antes 20).
// ⚠️ SIN PROBAR EN CANCHA. El 15/09, con 0 cm de sobra (30 y 30), no llegaba
// a tocar la pelota. Si con 5 pasa lo mismo, subir este numero.
//        35 cm  ->  (35 + 3,3) x 10 = 383 ms
// msAdelante: en el PANEL DE CONTROL (arriba).  ~35 cm (era 533, ~50)
int msAdelanteChico   = 133;     // 10 cm   // ⚠️ no se usa en ningun lado: cambiarlo no hace nada
// 🚨 2026-09-15 — CANCHA NUEVA, UMBRAL NUEVO. Era 620.
//
// La cancha cambio: mas chica, el verde MUCHO mas oscuro, las lineas
// blancas mas angostas. Medido con `pruebas/calibrar-linea`, 79.000
// muestras:
//
//              cancha vieja      cancha nueva
//     verde     356 / 465          60 a 95
//     blanco       ~760              ~755
//
// El blanco quedo igual y el verde se oscurecio muchisimo, asi que el
// contraste MEJORO. El punto medio entre 97 y 753 da 425.
//
// ⚠️ Y hay un motivo mas fuerte que la cuenta para bajarlo tanto. El mismo
// sensor, sobre la misma linea, dio 557 o 763 segun que tan centrado
// estaba. El robot vuelve del despeje EN MOVIMIENTO y muchas veces en
// diagonal, asi que nunca cruza perfectamente centrado: el umbral tiene
// que aguantar una lectura mediocre. Con 620, una pasada floja no
// disparaba y el robot se pasaba de largo.
// umbralBlanco: en el PANEL DE CONTROL (arriba).  medido en cancha el 2026-09-15
// Historia de este numero, en UNIDADES DE CAMARA mientras no sabiamos que
// no eran centimetros:  30 -> 20 -> 48 (2026-08-11).
//
// 2026-08-25: ahora esta en CENTIMETROS REALES. Los 48 de camara de antes
// eran 16,7 cm reales — el robot esperaba a tenerla casi encima. El equipo
// pidio que salga a buscarla a 30 cm reales.
//
// 🚨 REGLA: umbralCm NUNCA puede ser mayor que el alcance de la ida. Si el
// robot dispara a 30 cm y la ida son 20, frena antes de llegar y no toca
// la pelota. Si se cambia uno, hay que mirar el otro.
// Hoy: dispara a 30, la ida son ~50. La regla se cumple.
// 2026-09-15: BAJADO DE 30 A 20 a pedido del equipo despues de probar en
// cancha — que salga a despejar con la pelota mas cerca. En la cancha
// nueva, que es mas chica, salir a buscarla a 30 cm era salir demasiado.
// 2026-09-21: 20 -> 30 cm a pedido del equipo, junto con la ida (+10 cm).
// 2026-10-13: la ida bajo a ~35. Dispara a 30, la ida son ~35: se cumple.
// umbralCm: en el PANEL DE CONTROL (arriba).  cm REALES: despeja si esta a esto o menos

// Cuan de frente tiene que estar la pelota para disparar. 15 de camara /
// 2,87 = 5,2 cm reales: es lo mismo que venia haciendo.
//
// ⚠️ Ojo con este al subir umbralCm: 5,2 cm de costado a 16,7 cm de
// distancia es un angulo ancho, pero los mismos 5,2 cm a 30 cm de distancia
// son un angulo mucho mas angosto. O sea que disparando de mas lejos el
// robot se vuelve MAS exigente con la alineacion. Si en cancha se lo ve
// dudar y no despejar, este es el primer numero a mirar.
// 2026-09-08: SUBIDO DE 5,2 A 7,0 a pedido del equipo.
// Los 5,2 venian de cuando el despeje salia DERECHO: no tenia sentido
// salir si la pelota no estaba justo enfrente, porque yendo derecho no la
// alcanzaba nunca. Ahora que sale en DIAGONAL si puede alcanzar pelotas
// corridas, y ese limite se habia vuelto la traba que impedia usar la
// diagonal justo en los casos para los que se hizo.
//
// 7,0 ensancha el pasillo de ~10 a ~14 cm (unos +-13 grados desde 30 cm).
// Se puede ensanchar sin miedo PORQUE si la cuenta dice que no llega, el
// robot NO SALE. Sin esa proteccion, subir esto seria peligroso.
// umbralDesvio: en el PANEL DE CONTROL (arriba).  cm REALES de desvio tolerado


// ---- 🎯 PERSEGUIR LA PELOTA DURANTE EL DESPEJE (2026-09-08) ----
//
// EL PROBLEMA, contado por el equipo:
//   "cuando la pelota viene rapido en diagonal, el robot despeja pero la
//    pelota a veces pasa de largo, porque el robot se movio hacia adelante
//    en un eje QUE YA QUEDO ANTIGUO"
//
// Tenian razon y se ve en el codigo. La fase ADELANTE era una sola linea:
//        adelanteControlado(potenciaDespeje);
// Ni menciona la camara. El robot decidia UNA vez y despues manejaba 533 ms
// con los ojos cerrados. Una pelota rapida se corre muchisimo en medio
// segundo, asi que el robot llegaba al lugar donde la pelota ESTABA.
//
// LA SOLUCION: que no vaya a ciegas. Este robot es un omni de tres ruedas,
// asi que puede AVANZAR Y CORRERSE AL MISMO TIEMPO. Ahora, mientras carga,
// sigue mirando la pelota 26 veces por segundo y se va corriendo hacia
// donde ella este. En vez de ir a un punto viejo, PERSIGUE.
//
// Se eligio esto antes que "calcular la velocidad y predecir" porque
// predecir es apostar una vez: si la pelota cambia de direccion, erraste.
// Corregir todo el tiempo no le tiene que acertar a nada.
//
// Es el mismo truco de sumar movimientos independientes que ya usa todo el
// programa. Ahora son tres sumandos en vez de dos:
//        avanzar + correrse hacia la pelota + no girar
// 🚨 APAGADO A PROPOSITO EL 2026-09-08. Esto (la "opcion B") quedo escrito
// y compilado pero SIN PROBAR: el equipo decidio probar primero la
// prediccion (la "opcion A") por separado, para no mezclar dos cosas
// nuevas en la misma corrida y no saber cual hizo que.
// Se prende cambiando este false por true.
// perseguirEnElDespeje: en el PANEL DE CONTROL (arriba).

// PWM de costado por cada cm REAL que la pelota esta desviada.
// 6.0 -> con la pelota 5 cm al costado empuja 30; con 10 cm, 60 (el tope).
// ⚠️ PUESTO A OJO, primera version. Se ajusta mirando al robot.
// kpPersecucion: en el PANEL DE CONTROL (arriba).

// Tope del empujon lateral. No es solo por prolijidad:
//
// 🚨 CUIDADO CON QUEDARSE SIN MOTOR. El maximo que aguanta una rueda es
// 255. Durante el despeje ya se le esta pidiendo 200 para avanzar, mas la
// correccion de rumbo. Si el costado pide mucho mas, la cuenta se pasa de
// 255, la rueda se queda corta, y el robot no hace bien NINGUNA de las dos
// cosas — ni avanza derecho ni se corre.
// Si en cancha se lo ve flojo o raro justo cuando persigue fuerte, el
// primer sospechoso es ese: la salida es BAJAR potenciaDespeje para
// dejarle lugar al volantazo.
// PWM_MAX_PERSECUCION: en el PANEL DE CONTROL (arriba).

// MS_PAUSA_MEDIO: en el PANEL DE CONTROL (arriba).

// 2026-10-20, pedido del equipo, mirando el despeje: al llegar al tope de
// la ida el robot "hace como una inclinacion y recien vuelve al piso".
// Hasta hoy, al terminar la ida SOLTABA los motores yendo a 200 (parar()),
// esperaba MS_PAUSA_MEDIO suelto y arrancaba para atras de golpe. Ahora
// FRENA, se queda frenado MS_ESPERA_ANTES_DE_VOLVER para asentarse, y recien
// ahi retrocede.
//
// Lo midio el giroscopio con `pruebas/ajustar-movimientos` el mismo dia, con
// el despeje como estaba: 6 corridas, ninguna prolija, y la peor torcida
// SIEMPRE a los 0,3-0,4 s, o sea justo en ese cambio (7 a 25 grados). El
// retroceso solo, arrancando quieto, dio 2 grados. ⚠️ SIN PROBAR con esto.
//
//      frenarAntesDeVolver = false   ->   vuelve a soltar 150 ms, como antes
//
// 2026-10-20: 100 -> 400 ms. El equipo lo probo en ajustar-movimientos
// (movimiento 6) y con 400 "funciona decentemente mejor". Cuesta 0,25 s mas
// de despeje que antes (400 contra los 150 de la pausa vieja). No quedaron
// anotados los grados de cada corrida.
// frenarAntesDeVolver: en el PANEL DE CONTROL (arriba).
// MS_ESPERA_ANTES_DE_VOLVER: en el PANEL DE CONTROL (arriba).
// MS_MAX_RETROCESO: en el PANEL DE CONTROL (arriba).

// Cuanto se sostiene el freno electrico. Despues se sueltan los motores:
// mantenerlos cortocircuitados sin necesidad solo calienta el driver.
// msFreno: en el PANEL DE CONTROL (arriba).


// El empujoncito de 10 cm al final, para que el robot termine donde arranco
// y no pegado a la linea. Se saco un rato el 2026-08-18 mientras se probaba
// el freno, y se volvio a poner el mismo dia una vez que el freno funciono.
// empujonFinal: en el PANEL DE CONTROL (arriba).

// Va DESPACIO, no a la potencia del despeje: son 10 cm y el robot ya esta
// bien parado sobre la linea. Yendo rapido patina y arruina justo lo que
// acaba de lograr.
// potenciaEmpujon: en el PANEL DE CONTROL (arriba).

// MEDIDO con regla el 2026-08-18, a potencia 100 y con rampa:
//     400 ms  ->  13 cm      (dos corridas)
//
// Para 10 cm NO alcanza la regla de tres (daria 308 ms). Los primeros
// 100 ms el robot esta acelerando por la rampa, asi que ese tramo rinde
// menos distancia que el resto. Al acortar el tiempo total, la rampa pasa
// a ser una porcion mas grande del viaje y se pierde proporcionalmente mas.
//
// Descontando la rampa (~50 ms de recorrido equivalente) y el retardo
// mecanico de arranque (~33 ms, medido el 04/08):
//     velocidad = 13 cm / (400 - 50 - 33) ms = 0.041 cm/ms
//     para 10 cm  ->  10/0.041 + 50 + 33  =  ~320 ms
// msEmpujonFinal: en el PANEL DE CONTROL (arriba).
// MS_ENFRIAMIENTO: en el PANEL DE CONTROL (arriba).
// 2026-09-21: bajado de 10 s a 2 s a pedido del equipo. Con 2 s toda la
// cuenta queda en parpadeo rapido (el lento era para los primeros 7 s).
// MS_AVISO_ARMADO: en el PANEL DE CONTROL (arriba).
// VECES_PARA_CREERLE: en el PANEL DE CONTROL (arriba).


// ---- giroscopio: mantenerlo derecho ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
// enderezarActivado: en el PANEL DE CONTROL (arriba).

// ---- AVISO DE "ESTOY SIN GIROSCOPIO" ----
//
// Pedido del equipo el 2026-09-01: en cancha el robot no andaba derecho y
// no habia forma de saber si era porque el giroscopio no estaba
// funcionando. Ahora avisa con el LED, con un parpadeo MUCHO mas rapido
// que cualquier otro del programa (10 por segundo), para que no se
// confunda con las otras senales.
//
// ⚠️ El LED del pin 13 es de UN SOLO COLOR: no puede ser rojo. Si quieren
// una luz roja de verdad, hay que soldar un LED con su resistencia a un
// pin libre (el 9 o el 10 estan sin usar) y cambiar PIN_AVISO aca abajo.
// El codigo ya esta preparado: es cambiar este numero y nada mas.
#define PIN_AVISO 13
// MS_PARPADEO_AVISO: en el PANEL DE CONTROL (arriba).  50 encendido + 50 apagado

// Se actualiza adentro de rumboActual(), asi el aviso refleja el estado de
// verdad sin agregar lecturas extra al bus I2C.
bool giroscopoRespondiendo = false;

unsigned long t_chequeoGiro = 0;

// 🚨 2026-09-08 — CONTESTAR QUE EXISTE Y DAR DATOS SON DOS COSAS DISTINTAS.
//
// La clase pasada se arreglo que bno.begin() insistiera 10 veces. Pero
// begin() solo pregunta "¿estas ahi?". El BNO055 contesta que si mucho
// ANTES de tener un rumbo valido: primero tiene que arrancar su fusion de
// sensores, y mientras tanto devuelve CEROS — que es exactamente lo que el
// programa lee como "esta mudo".
//
// Por que no se veia en la mesa: ahi el robot lleva minutos encendido y el
// sensor ya esta caliente. En la cancha lo prenden y a los 10 segundos ya
// esta jugando.
//
// Lo que lo destapo: el equipo midio la bateria y dio 8,23 V — mas que la
// nominal. O sea que la explicacion de "bateria floja" no alcanzaba, y
// habia que buscar otra cosa.
//
// Ahora se espera a que el sensor DE UN DATO, no a que salude.
// MS_ESPERA_DATOS_GIRO: en el PANEL DE CONTROL (arriba).  en setup()
// MS_ESPERA_EXTRA_GIRO: en el PANEL DE CONTROL (arriba).  antes de armarse (era 15 s, bajado el 2026-09-21)

float rumboBase = -1;            // el rumbo que hay que sostener siempre

// Correccion continua de rumbo (proporcional + acumulada). Los valores
// vienen de lo medido el 2026-08-04 en las rectas del cuadrado: con solo
// proporcional quedaba un error fijo de 7 grados; con el termino acumulado
// bajo a menos de 2.
// KP_RUMBO: en el PANEL DE CONTROL (arriba).
// KI_RUMBO: en el PANEL DE CONTROL (arriba).
// LIMITE_INTEGRAL: en el PANEL DE CONTROL (arriba).
// 2026-09-01: subido de 70 a 100. Con 70, durante el despeje (potencia 200)
// la correccion era demasiado floja para evitar que el robot se torciera.
// MAX_CORRECCION_RUMBO: en el PANEL DE CONTROL (arriba).
// ZONA_MUERTA_RUMBO: en el PANEL DE CONTROL (arriba).
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Enderezado al terminar el despeje
// TOLERANCIA_ACOMODO: en el PANEL DE CONTROL (arriba).
// 🚨 2026-09-01 — EL ENDEREZADO NO TENIA FUERZA PARA GIRAR EL ROBOT.
//
// En cancha el robot termino 80 grados torcido y no se enderezaba nunca.
// El robot SI sabia que estaba torcido: conectado por USB contesto
// "rumbo 274.6, base 354.7" y "no llego a enderezarse en 3 s". O sea que
// el giroscopio andaba y el rumbo de referencia estaba bien guardado.
//
// El problema era la POTENCIA. La cuenta con 80 grados de error daba:
//        80 x KP_ACOMODO (1,5) = 120  ->  recortado a PWM_MAX_ACOMODO = 60
// Y un motor PARADO necesita ~70 de PWM para arrancar (medido en banco).
// Con 60 las ruedas zumban y no giran. Intentaba 3 segundos, se rendia,
// reintentaba 3 veces y seguia de largo — 80 grados torcido.
//
// El minimo tenia el mismo problema, peor: con 35 no se movia NUNCA, asi
// que los errores chicos tampoco se corregian jamas.
//
// Los dos suben por encima del piso de arranque. Mejor pasarse un poco y
// que el reintento lo acomode, que no moverse nunca.
// KP_ACOMODO: en el PANEL DE CONTROL (arriba).
// 2026-09-01, segunda pasada: con MAX=110 el robot SI giraba, pero giraba
// DEMASIADO RAPIDO y se pasaba: oscilaba varias veces antes de quedarse
// quieto, y a veces terminaba mal acomodado. A pedido del equipo el techo
// baja a la mitad del cambio (110 -> 85): menos velocidad, menos inercia.
//
// ⚠️ EL MINIMO NO SE PUEDE BAJAR. No es una perilla de ajuste fino: es lo
// que hace que el robot se mueva ALGO. Debajo de ~70 las ruedas zumban y
// no giran, que era exactamente el bug de antes.
// PWM_MIN_ACOMODO: en el PANEL DE CONTROL (arriba).  era 35: debajo del piso de arranque
// PWM_MAX_ACOMODO: en el PANEL DE CONTROL (arriba).  era 60, despues 110
// MS_ASENTAR_ACOMODO: en el PANEL DE CONTROL (arriba).
// MS_TIMEOUT_ACOMODO: en el PANEL DE CONTROL (arriba).
// MAX_REINTENTOS_ACOMODO: en el PANEL DE CONTROL (arriba).

// ---- 🎯 ENDEREZARSE TAMBIEN CUANDO NO VE LA PELOTA (2026-09-15) ----
//
// LO QUE NOTO EL EQUIPO EN CANCHA:
//   "el robot hace las correcciones cuando esta viendo la pelota. Si no la
//    esta viendo o la perdio, queremos que mire hacia adelante, porque a
//    veces queda viendo al lateral y la pelota esta en otro lado y por
//    estar mirando al costado no entra en su campo de vision."
//
// Tenian razon y es un circulo vicioso feo: el robot quedaba torcido, y
// justamente POR estar torcido no volvia a ver la pelota — y sin ver la
// pelota no hacia nada que lo enderezara. Se quedaba trabado mirando al
// costado para siempre.
//
// Por que pasaba: la correccion de rumbo viaja SUMADA al movimiento, asi
// que solo actua cuando el robot se esta moviendo. En ESPERANDO el robot
// hace parar() — motores apagados — y ahi no se corrige nada.
//
// Ahora, mientras espera, si se da cuenta de que esta torcido se endereza
// solo y vuelve a esperar. Reusa la misma maniobra del final del despeje.
//
// ⚠️ LOS DOS NUMEROS SON DISTINTOS A PROPOSITO: se endereza hasta quedar a
// menos de 3 grados (TOLERANCIA_ACOMODO), pero no vuelve a arrancar hasta
// pasarse de 8. Si los dos fueran iguales, el robot estaria todo el dia
// corrigiendo de a un grado, temblando en el arco.
// TOLERANCIA_ESPERA: en el PANEL DE CONTROL (arriba).  grados para decidir enderezarse
// MS_CHEQUEO_ESPERA: en el PANEL DE CONTROL (arriba).  no preguntarle al sensor a cada rato
unsigned long t_chequeoEspera = 0;
// Descanso obligatorio entre intentos de enderezarse esperando. Mientras
// descansa, el robot vuelve a mirar la pelota. Sin esto puede quedarse
// reintentando una maniobra que no converge y no atajar nunca.
// MS_DESCANSO_ENDEREZAR: en el PANEL DE CONTROL (arriba).
unsigned long t_ultimoIntentoEnderezar = 0;
bool enderezandoEnEspera = false;
// RAMPA_PASO: en el PANEL DE CONTROL (arriba).
// RAMPA_MS: en el PANEL DE CONTROL (arriba).
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;
int reintentosAcomodo = 0;


// ---- UBICACION INICIAL ----
//
// Al armarse, el robot retrocede hasta pisar la linea del area y se
// endereza. (Desde el 2026-10-12 hace algo mas: ver abajo.)
//
// 2026-09-21: SE SACO EL CENTRADO CON EL ARCO DEL RIVAL, a pedido del
// equipo: "lo centramos nosotros". El robot se apoya a mano en el centro
// del arco antes de prenderlo.
bool  ubicandose      = false;

// 2026-10-12, pedido del equipo: DOS COSAS NUEVAS en la ubicacion inicial.
//
//   1. Si ENCONTRO la linea del area, despues de enderezarse avanza el mismo
//      empujoncito de ~10 cm que hace al volver de un despeje. Asi no queda
//      sentado sobre el blanco, y arranca parado en el mismo lugar en el que
//      va a quedar despues de cada despeje.
//
//   2. Si NO la encontro (retrocedio MS_MAX_RETROCESO y nada), en vez de
//      quedarse ahi hace la rutina de `pruebas/ir-al-medio`, la que usa las
//      lineas del costado y del fondo para saber donde esta parado.
//
// Cada una tiene su interruptor: se pueden probar y apagar por separado.
// avanzarAlUbicarse: en el PANEL DE CONTROL (arriba).
// irAlMedioSiNoHayLinea: en el PANEL DE CONTROL (arriba).

// Como termino la ubicacion: si hay linea debajo, el robot sabe donde esta.
bool encontroLaLinea = false;

// El empujoncito del arranque usa la misma fase que el del despeje
// (ADELANTE_CHICO), pero al terminar NO tiene que enfriarse: el enfriamiento
// existe para no volver a disparar un despeje, y aca no hubo ninguno. Sin
// esto el arquero arrancaba ~1,3 s mas tarde sin mirar la pelota.
bool empujonDeArranque = false;


// ---- PATRULLA: no quedarse quieto para siempre ----
//
// 2026-10-12, pedido del equipo, despues de ganar 2-0: el robot hacia la
// rutina, quedaba en el medio y sin pelota a la vista NO SE MOVIA MAS. El
// juez penso que estaba roto y lo saco. (El 21/09 ya habia pasado lo mismo.)
//
// Ahora, si pasa MS_QUIETO_PARA_PATRULLAR sin moverse y sin ver la pelota,
// va hasta el lateral, vuelve al medio por tiempo, retrocede hasta la linea
// del area y avanza el poquito. O sea: los pasos 5, 6 y 7 de la rutina de
// ir al medio. Sirve para dos cosas: el juez ve que esta vivo, y el robot
// vuelve a quedar CENTRADO (el seguimiento lo va corriendo y nada lo traia).
//
// Cuatro cuidados, sacados de la revision del 21/09:
//   - El reloj cuenta desde la ultima vez que el robot SE MOVIO, no desde la
//     ultima vez que vio la pelota. Si no, un reflejo naranja cada tanto lo
//     dejaria quieto para siempre.
//   - Si aparece la pelota varios cuadros SEGUIDOS, corta la patrulla y la
//     sigue. Un cuadro suelto no alcanza.
//   - Termina en la linea del area, asi no se va corriendo en profundidad.
//   - Sin giroscopio no patrulla: de costado y sin rumbo gira solo.
//
// ⚠️ COSTO: cada patrulla son unos 8 s, y 5 de esos el arco queda
// descubierto. Si el juez aguanta mas de 10 s quieto, conviene subirlo.
// patrullarSinPelota: en el PANEL DE CONTROL (arriba).
// MS_QUIETO_PARA_PATRULLAR: en el PANEL DE CONTROL (arriba).  era 15000, bajado el 2026-10-13
// CUADROS_PARA_CORTAR_PATRULLA: en el PANEL DE CONTROL (arriba).  ~0,2 s viendo la pelota

// Ademas de los 10 s sin moverse, tiene que llevar este rato SEGUIDO sin ver
// la pelota. Sin esto, si el arquero estuvo 10 s quieto MIRANDO una pelota
// trabada enfrente y un rival se la tapa un instante, salia al lateral en el
// acto, justo cuando le llevan la pelota.
// MS_SIN_PELOTA_PARA_PATRULLAR: en el PANEL DE CONTROL (arriba).

// Con menos que esto en las tres ruedas, el robot no se mueve a la vista.
// PWM_CUENTA_COMO_MOVERSE: en el PANEL DE CONTROL (arriba).

// PLAN B, por si la patrulla por el lateral hace algo raro en la cancha: con
// esto en false la "patrulla" es solo retroceder hasta la linea del area,
// enderezarse y avanzar el poquito. Dura ~1,3 s y NO descubre el arco; lo
// que se pierde es que no vuelve a centrarlo de costado.
// patrullaPorElLateral: en el PANEL DE CONTROL (arriba).

unsigned long t_ultimoMovimiento = 0;   // lo actualiza aplicar()
unsigned long t_soltoLinea = 0;         // para el paso de despegarse
bool patrullando = false;
int  cuadrosConPelota = 0;              // cuadros seguidos con pelota a la vista

// ---- la rutina de ir al medio ----
//
// 🚨 SON LOS NUMEROS DE `pruebas/ir-al-medio`, COPIADOS TAL CUAL. Alla estan
// explicados uno por uno y alla se midieron (2026-10-06). Si se cambia uno
// en un lado hay que cambiarlo en el otro, o la prueba deja de decir lo que
// va a hacer el partido.
//
// Hay DOS velocidades de costado y no es un descuido: los pasos que
// terminan al VER una linea pueden ir a cualquier velocidad, pero el que
// termina por TIEMPO tiene que ir a la velocidad con la que se midio ese
// tiempo. Los 2228 ms son a velocidad 100.
// VEL_MEDIO_BUSCAR: en el PANEL DE CONTROL (arriba).  de costado, hasta ver una linea
// VEL_MEDIO_MEDIR: en el PANEL DE CONTROL (arriba).  🚨 atada a MS_MEDIO_AL_MEDIO
// VEL_MEDIO_RECTO: en el PANEL DE CONTROL (arriba).  atras y adelante. Atada a MS_MEDIO_ADELANTE
// LADO_MEDIO: en el PANEL DE CONTROL (arriba).  -1 = va a buscar el lateral IZQUIERDO

// MS_MEDIO_ALEJARSE: en el PANEL DE CONTROL (arriba).  despegarse del lateral
// MS_MEDIO_ADELANTE: en el PANEL DE CONTROL (arriba).  ~50 cm a 110 (regla de tres)
// MS_MEDIO_AL_MEDIO: en el PANEL DE CONTROL (arriba).  del lateral izquierdo al medio
// MS_MEDIO_TOPE_LATERAL: en el PANEL DE CONTROL (arriba).
// MS_MEDIO_QUIETO: en el PANEL DE CONTROL (arriba).  quieto entre paso y paso

// 2026-10-12, dato del equipo: EL FONDO SE MIDE POR TIEMPO.
//
// En los bordes de la cancha hay una rampita, puesta para que la pelota
// vuelva sola al campo. El robot NO la sube, asi que los sensores de atras
// no llegan nunca a pisar la linea del fondo y "retroceder hasta ver blanco"
// no terminaba: se cumplia el tope y la rutina se daba por fallada.
//
// Ahora el paso 3 termina con LO QUE PASE PRIMERO: ve la linea, o se cumple
// este tiempo. Si se cumple el tiempo se da por hecho que el robot esta
// apoyado contra la rampa del fondo —que es una marca fisica tan buena como
// la linea— y la rutina SIGUE.
//
// ⚠️ ES UN NUMERO A OJO, NO UNA MEDICION. Tiene que alcanzar para llegar al
// fondo desde donde este (ya viene de retroceder 1200 ms antes de empezar
// la rutina) y no ser tan largo que se quede empujando la rampa de gusto.
// A 110 el robot hace 1 cm cada 18 a 24 ms: 2000 ms son entre 80 y 110 cm.
// Como medirlo: cronometrar cuanto tarda en tocar la rampa desde el lugar
// mas lejano en el que se lo vaya a apoyar, y sumarle medio segundo.
// MS_MEDIO_FONDO_POR_TIEMPO: en el PANEL DE CONTROL (arriba).


// ---- LIMITE LATERAL: al pisar el lateral, volver al medio ----
//
// 2026-10-13, pedido del equipo: siguiendo la pelota el robot se iba de
// largo por el costado, chocaba el muro de afuera de la cancha y el juez lo
// sacaba 1 minuto. Ahora, si mientras sigue la pelota pisa la linea blanca
// del lateral, frena y vuelve al medio.
//
// COMO SABE QUE ES EL LATERAL Y NO LA LINEA DEL AREA. El limite que hubo
// del 11/08 al 18/08 ya miraba solo el sensor del lado hacia el que iba, y
// igual se saco porque cortaba de mas: torcido, ese sensor pisaba otra
// linea antes de tiempo. Este pide ademas que el OTRO sensor siga en verde
// y varias lecturas seguidas. Las dos lineas se distinguen por como cruzan
// el par de sensores:
//
//     lateral   (va de adelante hacia atras)  ->  la pisa PRIMERO el sensor
//               del lado hacia el que va el robot; el otro sigue en verde
//     del area  (va de costado a costado)     ->  la pisan LOS DOS
//
// Por eso cuenta solo si ve blanco el sensor del lado hacia el que va Y el
// otro no. ⚠️ No es infalible: corrido para atras Y torcido puede quedar
// un solo sensor sobre la linea del area. Para eso esta el cuidado de la
// vuelta: si volviendo al medio pisa el OTRO lateral, frena ahi.
//
// EL MISMO TIEMPO DESDE LOS DOS LATERALES, decision del equipo: usa
// MS_MEDIO_AL_MEDIO (2228), que sale de los cruces desde el IZQUIERDO.
// Desde el derecho la misma cuenta del 06/10 da 2331 ms (103 mas): si de
// ese lado queda corto del medio, ese es el numero. Los dos son de antes
// de mover los motores.
//
// Mientras vuelve al medio NO mira la pelota (~2,7 s): la pelota esta de
// ese lado y lo llevaria derecho de vuelta al lateral. Una vez en el medio,
// si la ve, la sigue ahi mismo.
//
// SIN GIROSCOPIO NO HAY LIMITE: de costado y sin rumbo gira solo, asi que
// no puede volver por tiempo. Es lo mismo que pasa con la patrulla.
// volverAlMedioEnElLateral: en el PANEL DE CONTROL (arriba).  false -> sin limite, como antes
// LECTURAS_PARA_CREER_LATERAL: en el PANEL DE CONTROL (arriba).  vueltas seguidas viendolo
int lecturasDeLateral = 0;
int ladoDelLateral = 0;                     // -1 = piso el izquierdo, +1 = el derecho


// ---- estado ----
enum Fase { APAGADO, ARMANDOSE, ESPERANDO, SIGUIENDO,
            ADELANTE, PAUSA_MEDIO, ATRAS_HASTA_LINEA, FRENANDO,
            ACOMODANDO, ACOMODO_ASENTAR, ADELANTE_CHICO, ENFRIANDO,
            UBIC_ATRAS, UBIC_FRENANDO,
            MEDIO_PAUSA, MEDIO_AL_LATERAL, MEDIO_ALEJARSE, MEDIO_AL_FONDO,
            MEDIO_ADELANTE, MEDIO_AL_LATERAL_2, MEDIO_AL_MEDIO,
            MEDIO_AL_AREA, MEDIO_DESPEGARSE,
            LATERAL_AL_MEDIO, MIRANDO };
Fase fase = APAGADO;
unsigned long t_fase = 0;

// A que paso de la rutina se va cuando termine la pausa entre pasos.
Fase faseDespuesDePausa = ESPERANDO;

int vecesSeguidas = 0;

// ---- camara ----
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;


unsigned long t_ultimoPaquete = 0;


// ---- 🎯 PREDECIR ADONDE VA LA PELOTA (2026-09-08) ----
//
// La camara dice DONDE ESTA la pelota. Pero al robot le sirve mas saber
// DONDE VA A ESTAR, porque para cuando termina de reaccionar la pelota ya
// se movio. Eso es lo que noto el equipo: el robot apuntaba a un lugar
// "que ya quedo antiguo".
//
// COMO SE SACA LA VELOCIDAD, que es mas simple de lo que suena:
// la camara manda 26 veces por segundo. Si en un cuadro la pelota estaba
// a 10 cm a la derecha y en el siguiente a 12, se corrio 2 cm en ~38 ms.
// Dividiendo, la velocidad. Nada mas que eso: cuanto cambio, dividido el
// tiempo que paso.
//
//        velocidad = (donde esta ahora - donde estaba antes) / tiempo
//
// Y despues, para adivinar el futuro:
//
//        donde va a estar = donde esta + velocidad x tiempo de adelanto
//
// Es lo mismo que hace un arquero de verdad cuando se tira ANTES de que
// la pelota llegue: no se tira a donde la ve, se tira a donde calcula que
// va a pasar.
//
// ⚠️ POR QUE HAY QUE SUAVIZAR. Restar dos lecturas seguidas es la forma
// mas ruidosa de medir que existe: si cada lectura tiene un error chico,
// la resta se lleva los dos errores juntos y ademas se divide por un
// tiempo muy corto, lo que agranda la equivocacion. Por eso la velocidad
// no se usa cruda: se promedia con las anteriores, pesando mas lo nuevo.
//
// 🧪 2026-10-12: APAGADA PARA PROBAR (era true). El robot oscila de lado a
// lado cuando la pelota esta LEJOS. HIPOTESIS, todavia sin probar: la camara
// va montada en el robot, asi que la "velocidad de la pelota" es en realidad
// la velocidad de la pelota RESPECTO DEL ROBOT. Con la pelota quieta, lo que
// se mide es el movimiento y el giro del propio robot, y girar 1 grado corre
// la pelota 0,5 cm en la imagen si esta a 30 cm pero 1,75 cm si esta a 1 m.
// El robot terminaria persiguiendo su propio giro.
//
// Con esto en false el seguimiento mira donde la pelota ESTA y nada mas. Es
// una prueba de si o no: si deja de oscilar, era la prediccion; si oscila
// igual, no era, y se vuelve a true.
//
// No cambia la decision del despeje MIENTRAS msAnticipacion siga en 0.
// El paso intermedio, si se quiere recuperar algo de adelanto: dejar esto en
// true y bajar msAnticipacionSeguir de 500 a 250.
//
// 2026-10-13: PRENDIDA de nuevo, pedido del equipo. La ESTABLE 0.1013
// congelada quedo con esto en false.
//
// 2026-10-20: APAGADA OTRA VEZ, y ahora con datos. El equipo puso la pelota
// QUIETA enfrente: con esto en true el robot iba y venia sin centrarse y se
// alejaba del arco; con la estable (false) no. La herramienta
// `pruebas/ajustar-movimientos` (movimiento 8) midio, con la pelota quieta,
// que la pelota "se movia" a 6,2 cm/s (el piso de ruido es 4) y 11 cambios de
// lado. Es la hipotesis del 12/10 confirmada: la camara va arriba del robot,
// el robot se mueve, la pelota parece moverse, y la prediccion lo manda mas
// lejos.
// predecirTrayectoria: en el PANEL DE CONTROL (arriba).
float velocidadLateral    = 0;      // cm reales por segundo, + = a la derecha
float velocidadAcercamiento = 0;    // cm/s, + = se viene encima del robot
// SUAVIZADO_VELOCIDAD: en el PANEL DE CONTROL (arriba).

// ---- 👀 MIRAR ANTES DE SALIR (2026-10-27, idea del equipo) ----
// Con el robot QUIETO, todo lo que se mueve en la imagen es la pelota. Se
// juntan los cuadros de la mirada y se traza la RECTA que mejor pasa por
// ellos (cuadrados minimos): la pendiente es la velocidad. Con 5 cuadros es
// mucho menos ruidosa que restar dos lecturas seguidas, y no arrastra nada
// de antes de frenar (el promedio de velocidadLateral si).
int   nMirada = 0;
float sT = 0, sY = 0, sD = 0, sTT = 0, sTY = 0, sTD = 0;
float vyMirada = 0;                 // cm/s, + = la pelota va a la derecha
float vaMirada = 0;                 // cm/s, + = se viene encima
bool  hayMirada = false;            // true = calcularDiagonal usa la velocidad de la mirada
unsigned long t_finMirada = 0;      // cuando termino la ultima mirada sin salir
int   motivoNoSale = 0;             // 1 el avance no alcanza, 2 muy abierto

void empezarMirada() {
  nMirada = 0; sT = sY = sD = sTT = sTY = sTD = 0;
  hayMirada = false;
}

void agregarMuestraMirada(unsigned long t, float y, float d) {
  float s = (t - t_fase) / 1000.0;
  nMirada++; sT += s; sY += y; sD += d; sTT += s * s; sTY += s * y; sTD += s * d;
}

// Pendientes de las dos rectas. Devuelve false si no hay datos para creerle.
bool calcularMirada() {
  if (nMirada < 3) return false;
  float den = nMirada * sTT - sT * sT;
  if (den < 1e-6) return false;
  vyMirada =  (nMirada * sTY - sT * sY) / den;
  vaMirada = -(nMirada * sTD - sT * sD) / den;   // la distancia BAJA al acercarse
  return true;
}


// ---- 🎯 DESPEJE EN DIAGONAL (2026-09-08) ----
//
// PEDIDO DEL EQUIPO, textual:
//   "el robot cuando despeja va recto y a veces la pelota la pierde, o si
//    esta muy cerca hace mal el despeje. Nos parecia buena idea que
//    despeje usando el giroscopio EN DIAGONAL si es necesario,
//    PREDICIENDO EL ANGULO con el que debe salir a despejar."
//
// La idea, y es mejor que corregir sobre la marcha: en el instante de
// disparar, con la pelota todavia bien a la vista, el robot calcula
// ADONDE SE VAN A ENCONTRAR los dos, saca de ahi el angulo, y se lanza
// comprometido con esa diagonal. Despues no necesita ver nada mas.
//
// Por que importa que no necesite ver: la camara PIERDE la pelota justo
// cuando la tiene encima (manda Xp = 0 tanto si no la ve como si la tiene
// pegada). Un despeje que depende de mirar se queda sin datos en el peor
// momento. Este no.
//
// LA CUENTA, en castellano:
//   1. ¿En cuanto tiempo nos encontramos? La pelota se me viene encima y
//      yo voy hacia ella, asi que la distancia se cierra con la SUMA de
//      las dos velocidades.
//          tiempo = distancia / (mi velocidad + la de acercamiento de ella)
//   2. ¿Donde va a estar de costado en ese momento?
//          desvio_final = desvio_ahora + velocidad_lateral x tiempo
//   3. ¿Que tan rapido me tengo que correr para llegar ahi?
//          velocidad_lateral_necesaria = desvio_final / tiempo
//   4. Si esa velocidad es mas de lo que puedo, NO SALGO. Decision del
//      equipo: un arquero que sale y no llega queda fuera de posicion Y
//      ademas le hacen el gol. Mejor quedarse y seguir acomodandose.
//
// El giroscopio no dirige nada: IMPIDE QUE EL ROBOT GIRE. Eso es lo que
// hace que la diagonal salga recta y no curva, y que el robot llegue
// mirando al frente.
// 🚨 2026-09-15: APAGADA, PORQUE EL GIROSCOPIO ESTA MUERTO.
//
// Sin giroscopio la diagonal es PEOR que ir derecho: la mezcla lateral
// 50/50/89 genera una rotacion parasita chica, y el que la corrige es
// justamente el giroscopio. Sin el, el robot sale en diagonal Y GIRANDO,
// y encima despues no se puede enderezar.
//
// Ir derecho sin giroscopio es lo que el robot venia haciendo en agosto:
// no es ideal, pero es predecible.
//
// ⚠️ APENAS EL GIROSCOPIO VUELVA, PRENDER ESTO (poner true).
// La diagonal esta escrita, compilada y sin probar.
//
// 2026-10-27: PRENDIDA, a pedido del equipo, con dos cuidados nuevos (ver
// calcularDiagonal): la velocidad de la pelota solo con el robot quieto, y
// un tope de angulo. Sigue sin probar en cancha.
// despejeEnDiagonal: en el PANEL DE CONTROL (arriba).

// MEDIDO el 2026-08-04 con regla: a potencia 200 el robot hace 1 cm cada
// 10 ms. O sea 100 cm por segundo.
const float VEL_ROBOT_CM_S = 100.0;   // ⚠️ no se usa en ningun lado: cambiarlo no hace nada

// 200 de PWM dan 100 cm/s. O sea 2 de PWM por cada cm/s.
// NO es un ajuste: es la medicion del 04/08 escrita en otra unidad.
// PWM_POR_CM_S: en el PANEL DE CONTROL (arriba).

// Debajo de esta velocidad se considera que la pelota esta QUIETA y no se
// arma diagonal por velocidad. El ruido medido con la pelota parada llega
// a 3,6 cm/s, asi que 4 esta apenas arriba.
// PISO_VELOCIDAD: en el PANEL DE CONTROL (arriba).

// ⚠️ EL UNICO NUMERO DE FE QUE QUEDA.
// Con el MISMO PWM, ¿el robot se corre de costado tan rapido como avanza?
// No: rinde menos. Avanzando, las dos ruedas de adelante tiran enteras.
// De costado, con la proporcion 50/50/89, parte del empuje de cada rueda
// se va para donde no sirve. De la geometria sale ~0,80 (o sea, el 80%).
// Esperar que la cancha pida bajarlo a 0,60-0,70.
// ⚠️ VA AL REVES de lo que parece: el costado se DIVIDE por este numero.
// Bajarlo = pedir MAS costado. Si no llega de costado: bajar. Si se pasa:
// subir. (Hasta la revision del 27/10 el panel lo decia al reves.)
//
// Lo bueno de que sea UN SOLO multiplicador: tambien tapa el otro agujero
// conocido (que al eje Y de la camara se le aplica el mismo factor 2,87
// que al X sin haberlo medido). Si ese estuviera errado, los dos terminos
// de la cuenta se escalan igual, y esta misma perilla los corrige a los
// dos. Una perilla, dos agujeros — aunque nunca sepamos cual era.
// rendimientoCostado: en el PANEL DE CONTROL (arriba).

// 🚨 EL REPARTO DEL MOTOR. Una rueda no pasa de 255. La mas cargada
// recibe el avance ENTERO mas la MITAD del costado. Se le reservan 45
// para el giroscopio (con KP_RUMBO = 5,0 eso son 9 grados de error
// corregibles sin recortar nada), y quedan 210 para el movimiento.
// RESERVA_RUMBO: en el PANEL DE CONTROL (arriba).
// PRESUPUESTO_RUEDA: en el PANEL DE CONTROL (arriba).  255 - RESERVA_RUMBO

// Debajo de este avance ya no es un despeje: es un paseo.
// PWM_MIN_AVANCE_DESPEJE: en el PANEL DE CONTROL (arriba).
// MS_MAX_ADELANTE: en el PANEL DE CONTROL (arriba).  tope duro de la ida

// Debajo de este costado el rozamiento se lo come: no vale la pena pagar
// avance por una diagonal que no se va a cumplir. Sale derecho.
// COSTADO_MINIMO_UTIL: en el PANEL DE CONTROL (arriba).

// Lo que se decidio para ESTE despeje. Se guarda para poder volver por la
// MISMA diagonal, que es lo que pidio el equipo: si sali torcido para
// alla, vuelvo torcido para aca y caigo donde arranque.
int lateralDelDespeje  = 0;
int pwmAvanceDespeje   = 200;    // puede bajar para dejarle lugar al costado
unsigned long msIdaDespeje = 533;

// Cuanto adelanto. Es EL TIEMPO QUE TARDA EL ROBOT EN REACCIONAR, sumando:
//    ~115 ms  -> los 3 cuadros seguidos que exige antes de creerle
//    ~135 ms  -> arrancar los motores y que la rampa suba
// ⚠️ ESTIMADO. Si el robot se adelanta de mas (queda del otro lado de la
// pelota), este numero esta alto.
//
// 🎯 2026-10-06, pedido del equipo: EN CERO. Quieren probar que el despeje
// decida con la pelota donde ESTA de verdad —que "despeje siempre"— y dejar
// la prediccion unicamente para el seguimiento lateral.
//
// Con 0, dLatPredicho = dLat: la decision del despeje usa el desvio real y
// nada mas. La prediccion del seguimiento no se toca y sigue en
// msAnticipacionSeguir.
//
// Para volver a los 250 (que es lo que el robot tarda en reaccionar, ver la
// cuenta de abajo) basta con cambiar este numero:
//      ~115 ms  -> los 3 cuadros seguidos que exige antes de creerle
//      ~135 ms  -> arrancar los motores y que la rampa suba
// msAnticipacion: en el PANEL DE CONTROL (arriba).

// 🎯 2026-10-06 — SEGUIR Y DISPARAR NO QUIEREN EL MISMO ADELANTO.
//
// Los 250 ms de arriba estan calculados para DECIDIR EL DESPEJE: son lo que
// el robot tarda en reaccionar, asi que sale hacia donde la pelota va a estar
// cuando el llegue. Para eso, 250 es el numero correcto — mas seria pasarse
// de largo.
//
// Pero SEGUIR no es lo mismo. Siguiendo, el robot esta SIEMPRE atras: la
// pelota se mueve y el la persigue, y nunca termina de alcanzarla. Ahi
// conviene adelantarse mas.
//
// El equipo lo vio en cancha y lo dijo asi: "si la pelota va lenta tiene que
// ir siguiendola igual que el movimiento de la pelota, y si va rapido que
// vaya mas adelantado". Eso es exactamente lo que hace la cuenta
// (velocidad x tiempo): con pelota lenta adelanta poco y con pelota rapida
// adelanta mucho, solo.
//
// LO QUE FALTABA ERA GANANCIA. Con 250 ms el adelanto era:
//       5 cm/s  ->  1,2 cm   (¡menos que ZONA_MUERTA_PELOTA = 1,4!)
//      15 cm/s  ->  3,8 cm
//      35 cm/s  ->  8,7 cm
// O sea que con pelota normal adelantaba dos o tres centimetros: estaba
// prediciendo, pero tan poco que a ojo no se notaba. Y con pelota lenta el
// adelanto caia dentro de la zona muerta y desaparecia del todo.
//
// Con 500 ms se duplica, y recien ahi el robot va visiblemente adelantado.
//
// ⚠️ COMO SABER SI QUEDO BIEN:
//    pasa de largo y la pelota le entra por detras  ->  bajarlo (400, 350)
//    sigue corriendo atras de la pelota             ->  subirlo (600, 700)
// Y ojo: esto NO toca la decision del despeje, que sigue con los 250.
// msAnticipacionSeguir: en el PANEL DE CONTROL (arriba).

// Para la cuenta de la velocidad hay que acordarse de la lectura anterior.
float desvioAnterior = 0;
float distanciaAnterior = 0;
unsigned long t_desvioAnterior = 0;
bool  hayDesvioAnterior = false;


// ---------------------------------------------------------------- motores

// SOLTAR los motores. No es frenar: el robot sigue de largo por inercia.
// Las dos patas de direccion en 0 (no alcanza con poner el PWM en 0, eso ya
// lo midio la otra mesa).
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// FRENO ELECTRICO. La idea es cortocircuitar los bornes del motor: la
// corriente que el propio motor genera al girar lo frena a el mismo. Es
// instantaneo y no hay que calibrar nada.
//
// Hay DOS formas de pedirselo al driver. MEDIDO en el piso el 2026-08-18
// con `pruebas/probar-freno` (tres corridas iguales de 1 segundo, marcando
// donde quedaba cada una):
//
//   VARIANTE 1 — las dos patas en ALTO, PWM al maximo.   ✅ FRENA
//   VARIANTE 2 — las dos patas en BAJO, PWM al maximo.   ✅ FRENA  <-- activa
//   Soltar (lo que habia antes)                          quedo MAS LEJOS
//
// Las dos frenan parecido. Se usa la 2 porque es la que quedo probada en
// este camino del codigo.
//
// ⚠️ La variante 2 se parece peligrosamente a parar(): la unica diferencia
// es el PWM. Con el PWM en CERO el driver apaga la salida y el motor queda
// suelto; con el PWM al maximo, cortocircuitado. **Mismo estado de las patas
// de direccion, efecto opuesto.** No confundirlas al leer.
//
// 🚨 Antes de esto hubo una prueba en la MESA que dijo que la variante 1 no
// frenaba. Estaba mal: el retroceso era de 250 ms y el robot se movia menos
// de 3 cm, o sea que nunca agarraba velocidad. **Un freno solo se puede
// medir si hay inercia que frenar.** El experimento no podia distinguir las
// dos respuestas, asi que no era una medicion.
void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

// Aplica un valor CON SIGNO a una rueda. Positivo = pata A alta.
// Tener valores con signo es lo que permite sumar movimientos: no se puede
// sumar "50 hacia alla" con "20 hacia el otro lado" si no hay signos.
void rueda(int ina, int inb, int pwm, int v) {
  if (v > 255)  v = 255;
  if (v < -255) v = -255;
  digitalWrite(ina, v >= 0 ? 1 : 0);
  digitalWrite(inb, v >= 0 ? 0 : 1);
  analogWrite(pwm, abs(v));
}

void aplicar(int v1, int v2, int v3) {
  rueda(INA1, INB1, PWM1, v1);
  rueda(INA2, INB2, PWM2, v2);
  rueda(INA3, INB3, PWM3, v3);
  // Todo movimiento pasa por aca (parar() y frenar() no): es el reloj de
  // "hace cuanto que no me muevo" que usa la patrulla.
  //
  // Solo cuenta si alguna rueda recibe fuerza de verdad. Sin este piso, un
  // reflejo naranja de UN cuadro mandaba al robot a seguir "la pelota", se
  // llamaba aca con la rampa todavia en cero, y el reloj se reiniciaba sin
  // que el robot se hubiera movido: la patrulla no salia nunca. Todos los
  // movimientos de verdad pasan de 60 en alguna rueda.
  if (abs(v1) >= PWM_CUENTA_COMO_MOVERSE || abs(v2) >= PWM_CUENTA_COMO_MOVERSE
      || abs(v3) >= PWM_CUENTA_COMO_MOVERSE) {
    t_ultimoMovimiento = millis();
  }
}

// Avanzar recto: mismas direcciones que avanzar() de arquero.ino, escritas
// con signos. Las dos de adelante van opuestas entre si porque estan
// montadas espejadas; la trasera no aporta al avance recto.


// ---------------------------------------------------- arranque suave

// MEDIDO en cancha el 2026-08-18: arrancando de golpe a potencia 200, las
// ruedas PATINAN. Y no patinan igual las dos — una agarra antes que la
// otra, y ese instante de diferencia tuerce al robot. De ahi en mas se va
// chueco, y como el avance no corregia el rumbo, llegaba torcido y volvia
// por la misma diagonal.
//
// Subiendo la potencia de a poco (200 ms hasta el fondo) las ruedas
// alcanzan a agarrar. Llega a la misma velocidad, sin el tiron.
// RAMPA_MOV_PASO: en el PANEL DE CONTROL (arriba).
// RAMPA_MOV_MS: en el PANEL DE CONTROL (arriba).
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;

void reiniciarRampaMovimiento() {
  potenciaRampa = 0;
  t_rampaMov = millis();
}

// Devuelve la potencia que corresponde AHORA, subiendo de a escalones.
// Bajar puede ser de golpe: pedir menos fuerza nunca hace patinar.
int rampa(int objetivo) {
  unsigned long ahora = millis();
  if (objetivo < potenciaRampa) {
    potenciaRampa = objetivo;
  } else if (ahora - t_rampaMov >= RAMPA_MOV_MS) {
    t_rampaMov = ahora;
    potenciaRampa += RAMPA_MOV_PASO;
    if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  }
  return potenciaRampa;
}


// ---------------------------------------------------------------- rumbo

// Devuelve el rumbo, o -1 si el sensor esta mudo. Un numero imposible en
// vez de 0 evita la confusion de siempre: 0.0 es a la vez un rumbo valido
// y el sintoma de que el chip no contesta.
float rumboActual() {
  if (!hayGiroscopo) { giroscopoRespondiendo = false; return -1; }
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    giroscopoRespondiendo = false;       // se cayo
    return -1;
  }
  giroscopoRespondiendo = true;
  return e.orientation.x;
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

// Cuanto hay que sumarle a las TRES ruedas para que el robot no gire.
// Devuelve 0 si no hay giroscopio: sin rumbo no se corrige nada, pero el
// movimiento lateral tiene que seguir funcionando igual.
int correccionDeRumbo() {
  if (!enderezarActivado || rumboBase < 0) return 0;
  float r = rumboActual();
  if (r < 0) return 0;

  float error = diferencia(rumboBase, r);

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;

  // Anti-windup: el acumulador tiene tope. Sin esto, un rato torcido lo
  // infla tanto que despues el robot se pasa para el otro lado.
  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  return (int)c;
}


// ---------------------------------------------------------------- linea

bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= umbralBlanco; }
bool algunoDeAtrasVeBlanco() { return veBlancoIzq() || veBlancoDer(); }

// ¿Pisa el lateral de ese lado? (-1 izquierdo, +1 derecho). Solo el sensor
// de ese lado en blanco y el otro en verde: ver "LIMITE LATERAL".
bool pisaElLateral(int lado) {
  bool izq = veBlancoIzq();
  bool der = veBlancoDer();
  return (lado < 0) ? (izq && !der) : (der && !izq);
}


// ------------------------------------------------------- moverse de costado

// s > 0 pide ir a la DERECHA, s < 0 a la izquierda. El valor es la fuerza.
// Suma el movimiento lateral con la correccion de rumbo: son dos cosas
// independientes y por eso se pueden calcular por separado.
//
// La fuerza pasa por la rampa; la correccion de rumbo NO. La correccion
// tiene que actuar enseguida, y ademas es chica: no hace patinar nada.
// Cuando se movio de costado por ultima vez: la usa el despeje en diagonal
// para no confundir el movimiento del robot con el de la pelota.
unsigned long t_ultimoCostado = 0;

void moverDeCostado(int s) {
  if (s != 0) t_ultimoCostado = millis();
  if (lateralInvertido) s = -s;

  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;

  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;

  // Direcciones sacadas de adproporcional() de arquero.ino: las dos de
  // adelante para el mismo lado, la trasera al reves y mas fuerte.
  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}

// Avanzar y retroceder SOSTENIENDO EL RUMBO, y con arranque suave.
//
// Antes el avance del despeje salia a ciegas: si patinaba al arrancar y se
// torcia, nadie lo corregia en los 50 cm siguientes. Y el retroceso tampoco
// corregia, asi que volvia por la misma diagonal torcida — de ahi que el
// robot terminara "apuntando a donde termino el avance".
//
// La correccion se SUMA a las tres ruedas, igual que en el movimiento
// lateral: girar es las tres parejas, avanzar son las dos de adelante
// opuestas entre si. Como son movimientos independientes, se suman.
void adelanteControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(+p + c, -p + c, 0 + c);
}

// AVANZAR PERSIGUIENDO: avanzar y correrse de costado a la vez.
//
// Es adelanteControlado() con un sumando mas. Las tres cosas se calculan
// por separado y se suman, que es como esta armado todo el programa:
//
//     avanzar          ->  (+p, -p,  0)     las dos de adelante opuestas
//     correrse         ->  (fr, fr, -tr)    la proporcion 50/50/89
//     no girar         ->  ( c,  c,  c)     las tres parejas
//
// `lateral` positivo = correrse a la DERECHA del robot, igual criterio que
// moverDeCostado().
//
// 2026-10-27 (revision): EL COSTADO SUBE CON LA RAMPA, JUNTO CON EL AVANCE.
// Antes el costado entraba entero desde el milisegundo 0 y el avance tardaba
// ~180 ms en subir: los primeros ~70 ms iba casi puro de costado, y la
// diagonal salia ~1,3 veces mas abierta que la calculada. Escalandolo con la
// rampa, la direccion es la calculada desde el arranque. La correccion de
// rumbo (c) sigue sin rampa: tiene que actuar en el acto.
void avanzarPersiguiendo(int potencia, int lateral) {
  if (lateralInvertido) lateral = -lateral;

  int p  = rampa(potencia);
  int c  = correccionDeRumbo();
  if (potencia > 0) lateral = (lateral * p) / potencia;
  int fr = (lateral * LADO_FRENTE)  / 100;
  int tr = (lateral * LADO_TRASERA) / 100;

  aplicar(+p + fr + c, -p + fr + c, -tr + c);
}

void atrasControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(-p + c, +p + c, 0 + c);
}


// ---------------------------------------------------------------- camara

void leerCamara() {
  while (Serial1.available()) {
    byte b = Serial1.read();
    if (!sincronizado) {
      if (b == 201) { sincronizado = true; paquete[0] = b; cuantos = 1; }
      continue;
    }
    paquete[cuantos++] = b;
    if (cuantos < 9) continue;

    cuantos = 0; sincronizado = false;
    if (paquete[0] == 201 && paquete[3] == 202 && paquete[6] == 203) {
      Xp = paquete[1];
      Yp = paquete[2] - 100;
      // El paquete trae tambien los dos arcos (202 amarillo, 203 azul),
      // pero desde 2026-09-21 no se usan: el equipo centra el robot a mano.
      t_ultimoPaquete = millis();

      // Un solo cuadro no alcanza para lanzar al robot: cualquier reflejo
      // naranja lo dispararia.
      //
      // La comparacion se hace en CENTIMETROS REALES: lo que llega por el
      // cable son unidades de camara y hay que traducirlo antes de decidir.
      float dCm  = Xp / CAMARA_POR_CM;
      float dLat = (camaraYInvertida ? -(float)Yp : (float)Yp) / CAMARA_POR_CM;

      // ---- velocidad lateral de la pelota ----
      // Se actualiza ACA, con cada paquete nuevo, que es cuando hay dato
      // fresco. Hacerlo en el loop() daria lecturas repetidas y una
      // velocidad falsa de cero.
      if (Xp > 0) {
        if (cuadrosConPelota < 1000) cuadrosConPelota++;
        unsigned long t = millis();
        if (hayDesvioAnterior) {
          float dt = (t - t_desvioAnterior) / 1000.0;
          // dt muy chico agranda cualquier error; dt muy grande quiere
          // decir que se perdio la pelota en el medio. Los dos se tiran.
          if (dt > 0.005 && dt < 0.5) {
            float v = (dLat - desvioAnterior) / dt;
            velocidadLateral = velocidadLateral * (1.0 - SUAVIZADO_VELOCIDAD)
                             + v * SUAVIZADO_VELOCIDAD;

            // Y la velocidad de ACERCAMIENTO, igual pero con la distancia.
            // Positiva = se viene encima. Se saca al reves que la lateral
            // (antes menos ahora) porque la distancia BAJA al acercarse.
            float va = (distanciaAnterior - dCm) / dt;
            velocidadAcercamiento = velocidadAcercamiento * (1.0 - SUAVIZADO_VELOCIDAD)
                                  + va * SUAVIZADO_VELOCIDAD;
          }
        }
        desvioAnterior    = dLat;
        distanciaAnterior = dCm;
        t_desvioAnterior  = t;
        hayDesvioAnterior = true;

        // MIRANDO: cuadro tomado con el robot ya quieto -> a la recta.
        if (fase == MIRANDO && t - t_fase >= MS_ASENTAR_MIRADA)
          agregarMuestraMirada(t, dLat, dCm);
      } else {
        // Sin pelota a la vista, la velocidad vieja no vale nada: cuando
        // reaparezca puede estar en cualquier lado.
        cuadrosConPelota  = 0;
        hayDesvioAnterior = false;
        velocidadLateral  = 0;
        velocidadAcercamiento = 0;
      }

      // Para decidir el despeje se usa DONDE VA A ESTAR, no donde esta.
      float dLatPredicho = predecirTrayectoria
                         ? dLat + velocidadLateral * (msAnticipacion / 1000.0)
                         : dLat;
      float desvCm = fabs(dLatPredicho);
      // Con medirAntesDeDespejar se dispara MAS LEJOS: a esa distancia
      // empieza a MIRAR. Salir, sale recien a umbralCm (ver MIRANDO).
      float distDisparo = medirAntesDeDespejar ? distanciaParaMirar : umbralCm;
      if (Xp > 0 && dCm <= distDisparo && desvCm <= umbralDesvio) {
        if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
      } else {
        vecesSeguidas = 0;
      }
    }
  }
}

bool veLaPelota()  { return Xp > 0 && (millis() - t_ultimoPaquete < 500); }
bool hayQueDespejar() { return vecesSeguidas >= VECES_PARA_CREERLE; }

// A que distancia esta la pelota, en CENTIMETROS REALES.
float distanciaPelota() {
  return Xp / CAMARA_POR_CM;
}

// Desvio de la pelota en CENTIMETROS REALES, con el signo ya corregido:
// positivo = esta a la derecha del robot.
//
// ⚠️ Al eje Y se le aplica el MISMO factor que al eje X. Bajar la camara
// achica los dos ejes por igual, asi que en teoria corresponde — pero eso
// es teoria, NO esta medido. Se mide igual que la distancia: pelota corrida
// 10, 20 y 30 cm al costado, a distancia fija. Anotado como pendiente.
float desvioPelota() {
  float y = camaraYInvertida ? -(float)Yp : (float)Yp;
  return y / CAMARA_POR_CM;
}

// Adonde VA A ESTAR la pelota dentro de msAnticipacion, en cm reales.
// Es la posicion de ahora mas lo que se va a mover en ese rato.
// Si la prediccion esta apagada, devuelve la posicion de ahora y listo.
float desvioPredicho() {
  if (!predecirTrayectoria) return desvioPelota();

  // 🚨 EL PISO DE VELOCIDAD, Y POR QUE HACE FALTA ACA (2026-10-06)
  //
  // Con la pelota QUIETA el robot se iba de lado a lado. La cuenta de por
  // que: el ruido de la camara con la pelota parada llega a 3,6 cm/s
  // (medido el 08/09), y con la anticipacion en 500 ms ese ruido se
  // convierte en un desvio inventado de
  //
  //      3,6 cm/s x 0,5 s = 1,8 cm
  //
  // ...que es MAS que ZONA_MUERTA_PELOTA (1,4 cm). O sea que el robot creia
  // que la pelota se movia, la perseguia, el ruido cambiaba de signo, y la
  // perseguia para el otro lado. No corregia de mas: se le estaba
  // amplificando el ruido.
  //
  // (Con los 250 ms de antes el ruido daba 0,9 cm y quedaba ADENTRO de la
  // zona muerta, asi que no se notaba. Subir la anticipacion lo saco afuera.)
  //
  // PISO_VELOCIDAD ya existia en este programa —4,0, puesto justo apenas
  // arriba de esos 3,6— pero se usaba solo para armar la diagonal del
  // despeje. Aca faltaba. Debajo del piso, la velocidad se toma como CERO y
  // la prediccion no adelanta nada: con la pelota quieta el robot mira donde
  // la pelota esta, y se queda.
  float v = velocidadLateral;
  if (fabs(v) < PISO_VELOCIDAD) v = 0;

  // Esta la usa el SEGUIMIENTO lateral, con su propia anticipacion. La
  // decision del despeje tiene su cuenta aparte, en leerCamara().
  return desvioPelota() + v * (msAnticipacionSeguir / 1000.0);
}

// ¿Con que angulo tengo que salir, y llego?
//
// Devuelve true si el despeje es posible, y deja en `lateral` el empuje de
// costado que hay que sumarle al avance. Si devuelve false, NO HAY QUE
// SALIR: la pelota se va a escapar igual y el robot queda fuera del arco.
bool calcularDiagonal(int &avance, int &lateral, unsigned long &msIda) {
  avance  = potenciaDespeje;
  lateral = 0;
  msIda   = msAdelante;

  if (!despejeEnDiagonal) return true;     // modo viejo: sale derecho
  if (!veLaPelota())      return true;     // sin datos, sale derecho

  // 2026-10-27 (revision): SIN GIROSCOPIO, DERECHO. Sin rumbo no hay quien
  // sostenga el frente: la diagonal sale girando y despues no se puede
  // enderezar (por eso se apago el 15/09). Es la misma condicion que piden
  // el limite lateral, la patrulla e ir al medio.
  if (!enderezarActivado || rumboBase < 0 || !giroscopoRespondiendo) return true;

  float d = distanciaPelota();
  if (d < 1.0) d = 1.0;                    // no dividir por cero
  float y  = desvioPelota();               // + = a la derecha
  motivoNoSale = 0;
  float vy = hayMirada ? vyMirada : velocidadLateral;
  if (fabs(vy) < PISO_VELOCIDAD) vy = 0;   // eso es ruido, no movimiento
  float va = hayMirada ? vaMirada : 0;     // acercamiento: solo si lo midio quieto
  if (va < PISO_VELOCIDAD) va = 0;         // (alejandose no suma)

  // 2026-10-27: LA VELOCIDAD DE LA PELOTA, SOLO CON EL ROBOT QUIETO.
  // La camara va arriba del robot: si el robot se esta moviendo de costado,
  // la pelota PARECE moverse para el otro lado aunque este quieta. Es lo que
  // hacia ir y venir al seguimiento con la prediccion prendida (20/10). Aca
  // haria salir la diagonal torcida para el lado contrario. Asi que si en los
  // ultimos MS_QUIETO_PARA_PREDECIR se movio de costado, se apunta a donde la
  // pelota ESTA, sin adelantarse.
  // 2026-10-27 (2): con la MIRADA el robot estuvo quieto a proposito y la
  // velocidad es de verdad, asi que no se mira t_ultimoCostado.
  if (!predecirEnDiagonal || (!hayMirada && millis() - t_ultimoCostado < MS_QUIETO_PARA_PREDECIR)) vy = 0;
  if (!predecirEnDiagonal) va = 0;

  // ---- HAY DOS MOTIVOS PARA IRSE EN DIAGONAL, Y SE SUMAN ----
  //
  // MOTIVO 1 — LA PELOTA ESTA CORRIDA. Es una regla de tres y nada mas:
  // si esta a 30 cm adelante y 6 al costado, hay que correrse 6 por cada
  // 30 que se avanza.
  //         costado = (6/30) x avance
  // 🎯 Lo lindo: los dos numeros se DIVIDEN entre si, asi que cualquier
  // error de escala de la camara SE CANCELA. Este termino no necesita
  // ninguna calibracion. Es puntería con una regla.
  float k = y / d;                          // sin unidades

  // MOTIVO 2 — LA PELOTA SE ESTA CORRIENDO SOLA. Si cruza a 20 cm/s, el
  // robot tiene que correrse a esos MISMOS 20 cm/s: si los dos van al
  // mismo ritmo de costado, se encuentran si o si, tarden lo que tarden.
  // Aca si hay que pasar de cm/s a PWM, y para eso se usa lo unico
  // medido con regla: 2 de PWM por cada cm/s.
  float m = vy * PWM_POR_CM_S;

  // MOTIVO 3 (27/10, con la mirada) — LA PELOTA SE VIENE ENCIMA. Entonces se
  // encuentran ANTES, y el costado hay que hacerlo en menos tiempo: la regla
  // de tres se hace con avance + lo que se acerca la pelota (en PWM).
  m += k * va * PWM_POR_CM_S;

  // ---- ¿ENTRA EN EL MOTOR? ----
  // La rueda mas cargada recibe el avance entero mas MEDIO costado, y no
  // puede pasar del presupuesto. Despejando el avance de:
  //        avance + 0,5 x costado <= PRESUPUESTO
  //        costado = (k x avance + m) / rendimiento
  float r = rendimientoCostado;
  float denom = 1.0 + 0.5 * fabs(k) / r;
  float techo = (PRESUPUESTO_RUEDA - 0.5 * fabs(m) / r) / denom;

  if (techo < avance) avance = (int)techo;

  // 🚨 SI NO ENTRA, SE ACHICAN LOS DOS JUNTOS — nunca solo el costado.
  // Recortar solo el costado dejaria al robot yendo en una direccion que
  // NO es la que calculo: ni derecho ni en la diagonal buena, el peor de
  // los mundos. Achicando los dos en la misma proporcion, la diagonal
  // apunta exactamente igual, solo que se recorre mas despacio.
  if (avance < PWM_MIN_AVANCE_DESPEJE) { motivoNoSale = 1; return false; }   // no llego: no salgo

  float costado = (k * avance + m) / r;

  // 2026-10-27: TOPE DE ANGULO. Para donde sale de verdad: por cada parte de
  // avance, costado x rendimiento de costado. Si para encontrarse con la
  // pelota tiene que salir mas abierto que MAX_ANGULO_DIAGONAL, NO SALE (la
  // misma decision de siempre: un arquero que sale y no llega es lo peor).
  float tangenteSalida = fabs(costado * r / avance);
  if (tangenteSalida > tan(MAX_ANGULO_DIAGONAL * PI / 180.0)) { motivoNoSale = 2; return false; }

  if (fabs(costado) < COSTADO_MINIMO_UTIL) costado = 0;   // sale derecho
  lateral = (int)costado;

  // Si el avance bajo, la ida se estira para recorrer la misma distancia.
  msIda = (unsigned long)((float)msAdelante * potenciaDespeje / avance);
  if (msIda > MS_MAX_ADELANTE) msIda = MS_MAX_ADELANTE;
  return true;
}

// Volver por la MISMA diagonal por la que salio, que es lo que pidio el
// equipo: si me fui torcido para alla, vuelvo torcido para aca y caigo
// donde arranque.
//
// Se invierten LAS DOS cosas: el avance pasa a retroceso y el costado
// cambia de lado. Y el costado se achica en la misma proporcion en que se
// achico la potencia (la ida va a 200 y la vuelta a 110), para que el
// ANGULO sea el mismo y no solo el sentido.
void atrasEnDiagonal(int potencia, int lateral) {
  if (lateralInvertido) lateral = -lateral;

  int p  = rampa(potencia);
  int c  = correccionDeRumbo();
  int fr = (lateral * LADO_FRENTE)  / 100;
  int tr = (lateral * LADO_TRASERA) / 100;

  aplicar(-p + fr + c, +p + fr + c, -tr + c);
}

// Se llama al terminar de enderezarse, desde las tres salidas de ACOMODANDO.
void pasarAEsperar() {
  fase = ESPERANDO;
  t_fase = millis();
  vecesSeguidas = 0;
}

// Arranca la ubicacion inicial: atras hasta la linea y enderezarse.
void arrancarUbicacion() {
  ubicandose      = true;
  encontroLaLinea = false;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  fase = UBIC_ATRAS; t_fase = millis();
}

// Entre un paso y el otro de la rutina de ir al medio: frenar, soltar y
// quedarse quieto un momento. Es lo que hace pasarA() en `pruebas/ir-al-medio`,
// pero sin delay(), para no dejar de leer la camara.
void pausaMedio(Fase siguiente) {
  frenar();
  faseDespuesDePausa = siguiente;
  fase = MEDIO_PAUSA; t_fase = millis();
}

// La rutina no cerro: no aparecio una linea antes del tope. En la prueba el
// robot se quedaba avisando con el LED; en un partido eso es regalar el
// arco. Aca frena, se endereza y juega desde donde quedo.
void abandonarMedio() {
  encontroLaLinea = false;
  frenar();
  fase = UBIC_FRENANDO; t_fase = millis();
}

// ¿Esta en alguno de los pasos de la rutina de ir al medio?
bool enFaseDeMedio() {
  switch (fase) {
    case MEDIO_PAUSA:        case MEDIO_DESPEGARSE:  case MEDIO_AL_LATERAL:
    case MEDIO_ALEJARSE:     case MEDIO_AL_FONDO:    case MEDIO_ADELANTE:
    case MEDIO_AL_LATERAL_2: case MEDIO_AL_MEDIO:    case MEDIO_AL_AREA:
      return true;
    default:
      return false;
  }
}

// La patrulla entra a la rutina de ir al medio por el paso 5 (ir al lateral)
// y la termina por el mismo camino que la ubicacion inicial: linea del area,
// enderezarse y avanzar el poquito.
//
// Si arranca parado sobre una linea, primero se despega (el paso 0 de
// `pruebas/ir-al-medio`). Sin eso el paso 5 terminaria al instante, creyendo
// que ya llego al lateral, y "volveria al medio" desde cualquier lado.
void arrancarPatrulla() {
  cuadrosConPelota = 0;         // los cuadros del corte se cuentan desde aca
  if (!patrullaPorElLateral) {
    // Plan B: lo mismo que al arrancar el partido (atras hasta la linea,
    // enderezarse, avanzar el poquito).
    arrancarUbicacion();
    patrullando = true;
    return;
  }
  patrullando     = true;
  ubicandose      = true;
  encontroLaLinea = false;
  pausaMedio(algunoDeAtrasVeBlanco() ? MEDIO_DESPEGARSE : MEDIO_AL_LATERAL_2);
}

// Aparecio la pelota en plena patrulla: a jugar. Queda donde esta, corrido
// del medio; la proxima patrulla lo vuelve a centrar.
void cortarPatrulla() {
  patrullando = false;
  ubicandose  = false;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  fase = SIGUIENDO; t_fase = millis();
}

// Piso el lateral siguiendo la pelota: frena y vuelve al medio por tiempo.
// No es una patrulla (patrullando = false): mientras vuelve, la pelota NO
// lo corta. Termina por el camino de la ubicacion, igual que la patrulla.
void arrancarVueltaDelLateral(int lado) {
  ladoDelLateral    = lado;
  lecturasDeLateral = 0;
  patrullando       = false;
  ubicandose        = true;
  encontroLaLinea   = false;
  pausaMedio(LATERAL_AL_MEDIO);
}

// Pasa al enderezado. Lo usan la ubicacion inicial y el enderezado que
// se hace mientras espera.
void pasarAEnderezarse() {
  parar();
  fase = ACOMODANDO; t_fase = millis();
  reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = millis();
}


// Adonde ir despues del enderezado. Esta en una funcion y no repetido en
// cada rama porque hay TRES caminos que salen del enderezado (termino bien,
// estaba desactivado, o el giroscopio estaba mudo) y los tres tienen que
// respetar el interruptor del empujon final. Cuando estaba escrito tres
// veces, dos se lo salteaban.
// Se llama cuando la camara ya dijo 3 cuadros seguidos que la pelota esta
// cerca y de frente. Aca se decide el ANGULO y, sobre todo, SI SALIR.
// Devuelve true si arranco el despeje.
bool intentarDespejar(unsigned long ahora) {
  // 2026-10-27 (2): MIRAR ANTES DE SALIR. En vez de salir, frena y mira la
  // pelota quieto (fase MIRANDO); de ahi se vuelve a llamar a esta funcion.
  // Devuelve true porque cambio de fase. Despues de una mirada que no salio,
  // descansa MS_DESCANSO_MIRADA, salvo que la pelota ya este a umbralCm.
  // 0.1027d: si el robot YA estaba quieto (no se corrio de costado en
  // MS_QUIETO_PARA_PREDECIR) y la pelota ya esta a umbralCm, no hace falta
  // frenarse a mirar: la velocidad que tiene es de verdad. Sale ya, como la b.
  bool yaEstabaQuieto = (millis() - t_ultimoCostado >= MS_QUIETO_PARA_PREDECIR);
  if (medirAntesDeDespejar && fase != MIRANDO
      && !(yaEstabaQuieto && distanciaPelota() <= umbralCm)) {
    if (ahora - t_finMirada < MS_DESCANSO_MIRADA && distanciaPelota() > umbralCm) {
      vecesSeguidas = 0;
      return false;
    }
    fase = MIRANDO; t_fase = ahora;
    empezarMirada();
    frenar();
    return true;
  }

  int avance = potenciaDespeje;
  int lateral = 0;
  unsigned long msIda = msAdelante;

  if (!calcularDiagonal(avance, lateral, msIda)) {
    // La cuenta dice que no llego. Decision del equipo: no salir.
    // Un arquero que sale y no llega queda fuera de posicion Y ademas le
    // hacen el gol: es lo peor de los dos mundos.
    vecesSeguidas = 0;          // que vuelva a juntar cuadros desde cero
    return false;
  }

  lateralDelDespeje  = lateral;
  pwmAvanceDespeje   = avance;
  msIdaDespeje       = msIda;

  fase = ADELANTE; t_fase = ahora;
  reiniciarRampaMovimiento();   // arrancar suave: si patina, se tuerce
  reiniciarCorreccion();
  return true;
}

void terminarDespeje() {
  // Si lo que acaba de terminar fue un enderezado de los que se hace solo
  // mientras espera, vuelve a esperar y listo: ni empujoncito ni cuenta de
  // despeje. No despejo nada, solo se acomodo.
  if (enderezandoEnEspera) {
    enderezandoEnEspera = false;
    pasarAEsperar();
    return;
  }
  // Si lo que acaba de terminar era la UBICACION INICIAL y no un despeje.
  // Se reusa toda la maquinaria de enderezarse, solo cambia adonde va
  // despues.
  //
  // 2026-10-12, pedido del equipo: si encontro la linea, avanza el poquito.
  // (Antes no: se quedaba parado sobre la linea.) Si NO la encontro no
  // avanza nada, porque no sabe donde esta parado.
  if (ubicandose) {
    ubicandose = false;
    patrullando = false;        // la patrulla termina por este mismo camino
    if (avanzarAlUbicarse && encontroLaLinea) {
      empujonDeArranque = true;
      fase = ADELANTE_CHICO; t_fase = millis();
    } else {
      pasarAEsperar();
    }
    return;
  }
  if (empujonFinal) {
    fase = ADELANTE_CHICO; t_fase = millis();
  } else {
    fase = ENFRIANDO; t_fase = millis();
  }
}



// ---------------------------------------------------------------- programa

// La version, tambien como texto en el programa: asi se puede saber que
// firmware tiene puesto el robot sin adivinar. Se imprime UNA vez al
// arrancar y nada mas — el 21/09 se saco toda la consola del programa de
// juego porque en la cancha no hay cable, y eso sigue igual: esta linea no
// se lee en la cancha, se lee cuando el robot vuelve a la mesa.
// 2026-10-12: dejo de decir ESTABLE. Desde el 06/10 este archivo ya no es el
// del 29/09, y el cartel no lo distinguia del respaldo congelado.
// 2026-10-13: vuelve a decir ESTABLE, pedido del equipo. La copia congelada
// de esta version es `respaldos/estable1013conlimiteslaterales/`; su cartel dice RESPALDO.
// Con la prediccion prendida ya no es igual a la copia congelada: EN PRUEBA.
const char* VERSION = "0.1027d EN PRUEBA (diagonal + MIRAR CORTO + umbralDesvio 20)";

void setup() {
  Serial.begin(19200);
  delay(300);
  Serial.println();
  Serial.print("=== ARQUERO  version ");
  Serial.print(VERSION);
  Serial.println(" ===");

  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);
  parar();

  Serial1.begin(BAUDIOS);       // la camara

  // 🚨 2026-09-01 — EL ARRANQUE DEL GIROSCOPIO ES UNA CARRERA, Y SE PERDIA.
  //
  // Antes aca habia UNA sola linea: `hayGiroscopo = bno.begin();`. Se
  // preguntaba una vez y, si contestaba que no, no se volvia a preguntar
  // nunca mas.
  //
  // El problema: cuando se prende la bateria, el Teensy arranca en
  // milisegundos pero el BNO055 tarda casi un segundo en estar listo para
  // contestar por I2C. Si el Teensy pregunta antes, recibe "no estoy" —
  // y queda `hayGiroscopo = false` para TODA la corrida. Sin correccion de
  // rumbo en ningun movimiento.
  //
  // Se noto porque dos corridas seguidas del MISMO programa dieron
  // distinto: la primera el robot no anduvo derecho y termino en cualquier
  // lado; la segunda anduvo bien. Mismo codigo, dos resultados = algo que
  // a veces sale bien y a veces mal. Era esta carrera.
  //
  // Ahora insiste: hasta 10 intentos separados por 300 ms (3 segundos en
  // total), que sobra para que el sensor despierte.
  // ---- cuantas veces se le pregunta al giroscopio ----
  //
  // 2026-09-29, pedido del equipo: UN SOLO INTENTO, como hace el delantero.
  //
  // ⚠️ Los 10 intentos se agregaron el 2026-09-01 para tapar la carrera de
  // encendido: el BNO055 tarda en despertar y, si se le pregunta demasiado
  // pronto, contesta que no esta aunque este perfecto. Ese dia el equipo
  // confirmo 3 corridas buenas de 3 con los reintentos puestos.
  //
  // Con UNO solo, si el sensor todavia no desperto el robot juega el partido
  // entero sin rumbo (avisando con el parpadeo rapido). A cambio, lo que se
  // gana es que el arranque sea inmediato y que no se disfrace un problema
  // fisico: el 29/09 se descubrio que EL CABLE del giroscopio esta roto, y
  // los reintentos hacen que "a veces arranque" en vez de fallar parejo.
  //
  // Es un solo numero: volver a 10 es cambiar este 1.
  // INTENTOS_GIRO: en el PANEL DE CONTROL (arriba).

  int intentos = 0;
  while (!hayGiroscopo && intentos < INTENTOS_GIRO) {
    intentos++;
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo && intentos < INTENTOS_GIRO) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);

    // Y ahora lo importante: esperar a que DE UN DATO. Saludar no alcanza.
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) {
      delay(50);
    }
  }
  // Si el giroscopio no aparece, el robot juega igual pero sin enderezarse,
  // y avisa con el LED temblando (10 por segundo).

  // Arranca solo: la unica forma de pararlo es la llave de la bateria.
  fase = ARMANDOSE; t_fase = millis();
}


void loop() {

  leerCamara();
  unsigned long ahora = millis();

  // Patrullando y aparecio la pelota: se corta la patrulla y se la sigue.
  // Varios cuadros SEGUIDOS, para que no la corte un reflejo suelto. Solo
  // la patrulla: la ubicacion del arranque se deja terminar.
  if (patrullando && enFaseDeMedio() && veLaPelota()
      && cuadrosConPelota >= CUADROS_PARA_CORTAR_PATRULLA) {
    cortarPatrulla();
  }

  switch (fase) {

    case APAGADO:
      digitalWrite(LED, LOW);
      break;

    case ARMANDOSE: {
      unsigned long falta = (ahora - t_fase >= MS_AVISO_ARMADO)
                            ? 0 : MS_AVISO_ARMADO - (ahora - t_fase);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_AVISO_ARMADO) {
        // 🚨 NO ARMARSE SIN RUMBO. Si el sensor todavia no da datos, se
        // sigue esperando en vez de salir a jugar a ciegas. El LED ya esta
        // temblando (10 destellos por segundo), asi que desde afuera se ve
        // que esta ESPERANDO y no que se colgo.
        float r = rumboActual();
        if (r < 0 && (ahora - t_fase) < MS_AVISO_ARMADO + MS_ESPERA_EXTRA_GIRO) {
          break;                    // todavia no da datos: seguir esperando
        }

        // El rumbo de referencia se fija ACA, con el robot ya quieto y
        // apuntando a la cancha. Es el que va a sostener siempre.
        rumboBase = r;
        reiniciarCorreccion();
        // Antes se pasaba derecho a ESPERANDO, o sea que el robot se
        // quedaba donde lo hubieran apoyado. Ahora primero se ubica.
        arrancarUbicacion();
      }
      break;
    }

    // ---------------- ubicacion inicial: atras hasta la linea ----------------

    case UBIC_ATRAS:
      // Igual que el regreso del despeje, y por el mismo motivo: la linea
      // del area es una marca fisica, siempre esta en el mismo lugar.
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atrasControlado(potenciaRetroceso);
      if (algunoDeAtrasVeBlanco()) {
        encontroLaLinea = true;
        frenar();
        fase = UBIC_FRENANDO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        // 2026-10-12: no aparecio la linea. Antes se quedaba aca; ahora va
        // a buscar el medio con los laterales. Sin giroscopio NO: de costado
        // y sin rumbo el robot gira solo y la rutina no tiene sentido. Se
        // mira que conteste AHORA, no solo que haya contestado al armarse.
        //
        // ⚠️ Si el giroscopio se cae A MITAD de la rutina, sigue igual hasta
        // el tope de ese paso, sin corregir el rumbo (lo mismo que hace la
        // prueba). No se corta ahi a proposito: la unica senal que hay de
        // "se cayo" son tres ceros seguidos, y con el robot quieto cerca del
        // rumbo 0 esa senal puede dar falsa (bitacora del 29/09).
        if (irAlMedioSiNoHayLinea && enderezarActivado && rumboBase >= 0
            && giroscopoRespondiendo) {
          pausaMedio(MEDIO_AL_LATERAL);
        } else {
          frenar();
          fase = UBIC_FRENANDO; t_fase = ahora;
        }
      }
      break;

    case UBIC_FRENANDO:
      if (ahora - t_fase >= msFreno) {
        parar();
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
        // Se queda donde encontro la linea y pasa a enderezarse.
        pasarAEnderezarse();
      }
      break;

    // ---------- sin linea: ir al medio usando los laterales ----------
    //
    // Es la rutina de `pruebas/ir-al-medio`, con los mismos pasos y los
    // mismos numeros. Arranca cuando el retroceso de arriba no encontro la
    // linea del area, o sea que el robot NO esta parado sobre ningun blanco
    // (por eso aca no hace falta el paso 0 de la prueba, el de despegarse).
    //
    //   1. de costado hasta la linea LATERAL       fija el ANCHO
    //   2. se aleja un poquito                     para que "ver blanco"
    //                                              vuelva a significar algo
    //   3. retrocede hasta el FONDO                fija la PROFUNDIDAD
    //      (hasta ver la linea o, si la rampita del borde no lo deja
    //      llegar, por tiempo: MS_MEDIO_FONDO_POR_TIEMPO)
    //   4. avanza ~50 cm                           sale del area
    //   5. vuelve al MISMO lateral                 por si se corrio
    //   6. al medio por TIEMPO                     ahora si
    //   7. atras hasta la linea del AREA           igual que el arranque
    //
    // Y de ahi sigue por el camino de siempre: frena, se endereza y avanza
    // el poquito. Si en algun paso no aparece la linea antes del tope, no se
    // queda clavado: abandonarMedio() lo manda a enderezarse y a jugar desde
    // donde quedo.

    case MEDIO_PAUSA:
      if (ahora - t_fase < msFreno) { frenar(); break; }
      parar();
      if (ahora - t_fase >= msFreno + MS_MEDIO_QUIETO) {
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
        // El retroceso miro el blanco EN MOVIMIENTO. Si con el envion del
        // freno termino parado sobre una linea, es la que venia buscando:
        // la encontro tarde, pero la encontro. Sin este chequeo el paso 1
        // terminaria al instante creyendo que ya llego al lateral.
        if (faseDespuesDePausa == MEDIO_AL_LATERAL && algunoDeAtrasVeBlanco()) {
          encontroLaLinea = true;
          pasarAEnderezarse();
          break;
        }
        fase = faseDespuesDePausa; t_fase = ahora;
        t_soltoLinea = ahora;     // el margen de despegarse cuenta desde ACA
      }
      break;

    // Paso 0, solo si la patrulla arranco parado sobre una linea: irse para
    // el mismo lado al que va a ir despues, hasta dejar de ver blanco, y un
    // rato mas (MS_MEDIO_ALEJARSE) para no quedar justo en el borde.
    case MEDIO_DESPEGARSE:
      digitalWrite(LED, HIGH);
      moverDeCostado(LADO_MEDIO * VEL_MEDIO_BUSCAR);
      if (algunoDeAtrasVeBlanco()) {
        t_soltoLinea = ahora;                    // sigue encima del blanco
      } else if (ahora - t_soltoLinea >= MS_MEDIO_ALEJARSE) {
        pausaMedio(MEDIO_AL_LATERAL_2);
        break;
      }
      // No logro salirse: lleva MS_MEDIO_TOPE_LATERAL yendo para ese lado,
      // asi que esta contra el lateral. Vuelve al medio por tiempo.
      if (ahora - t_fase >= MS_MEDIO_TOPE_LATERAL) pausaMedio(MEDIO_AL_MEDIO);
      break;

    case MEDIO_AL_LATERAL:            // paso 1
    case MEDIO_AL_LATERAL_2:          // paso 5
      digitalWrite(LED, HIGH);
      moverDeCostado(LADO_MEDIO * VEL_MEDIO_BUSCAR);
      // Termina al VER la linea o, si no aparece, POR TIEMPO: despues de
      // MS_MEDIO_TOPE_LATERAL empujando para ese lado el robot esta contra
      // el lateral aunque no haya visto el blanco. Antes aca abandonaba y
      // quedaba estacionado en el borde; con una patrulla que se repite
      // eso era un robot atascado.
      if (algunoDeAtrasVeBlanco()
          || ahora - t_fase >= MS_MEDIO_TOPE_LATERAL) {
        pausaMedio(fase == MEDIO_AL_LATERAL ? MEDIO_ALEJARSE : MEDIO_AL_MEDIO);
      }
      break;

    case MEDIO_ALEJARSE:              // paso 2
      digitalWrite(LED, HIGH);
      moverDeCostado(-LADO_MEDIO * VEL_MEDIO_BUSCAR);
      if (ahora - t_fase >= MS_MEDIO_ALEJARSE) pausaMedio(MEDIO_AL_FONDO);
      break;

    case MEDIO_AL_FONDO:              // paso 3
      digitalWrite(LED, HIGH);
      atrasControlado(VEL_MEDIO_RECTO);
      if (algunoDeAtrasVeBlanco()) { pausaMedio(MEDIO_ADELANTE); break; }
      // Sin linea a la vista: por tiempo. La rampita del borde no lo deja
      // llegar a pisarla, asi que a esta altura ya esta apoyado contra ella.
      if (ahora - t_fase >= MS_MEDIO_FONDO_POR_TIEMPO) pausaMedio(MEDIO_ADELANTE);
      break;

    case MEDIO_ADELANTE:              // paso 4
      digitalWrite(LED, HIGH);
      adelanteControlado(VEL_MEDIO_RECTO);
      if (ahora - t_fase >= MS_MEDIO_ADELANTE) pausaMedio(MEDIO_AL_LATERAL_2);
      break;

    case MEDIO_AL_MEDIO:              // paso 6
      digitalWrite(LED, HIGH);
      moverDeCostado(-LADO_MEDIO * VEL_MEDIO_MEDIR);   // 🚨 100, no 110
      if (ahora - t_fase >= MS_MEDIO_AL_MEDIO) pausaMedio(MEDIO_AL_AREA);
      break;

    case MEDIO_AL_AREA:               // paso 7
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atrasControlado(VEL_MEDIO_RECTO);
      if (algunoDeAtrasVeBlanco()) {
        encontroLaLinea = true;
        frenar();
        fase = UBIC_FRENANDO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_MAX_RETROCESO) abandonarMedio();
      break;

    // 2026-10-13: la vuelta al medio despues de pisar un lateral siguiendo
    // la pelota. Es el paso 6 de arriba, pero para el lado que haga falta.
    case LATERAL_AL_MEDIO:
      digitalWrite(LED, HIGH);
      moverDeCostado(-ladoDelLateral * VEL_MEDIO_MEDIR);   // 🚨 100, no 110
      // Si aparece el OTRO lateral, no salio de donde creia: frena ahi.
      if (pisaElLateral(-ladoDelLateral)) lecturasDeLateral++;
      else                                lecturasDeLateral = 0;
      if (ahora - t_fase >= MS_MEDIO_AL_MEDIO
          || lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
        lecturasDeLateral = 0;
        // Desde aca es el final de una patrulla: si ve la pelota la sigue,
        // y si no, se acomoda contra la linea del area.
        patrullando = true;
        pausaMedio(MEDIO_AL_AREA);
      }
      break;

    case ESPERANDO:
      parar();
      digitalWrite(LED, ((ahora / 800) % 2) ? HIGH : LOW);

      // 🎯 ENDEREZARSE MIENTRAS ESPERA (2026-09-15).
      // Si quedo torcido, se acomoda solo aunque no vea la pelota. Sin
      // esto se quedaba mirando al costado para siempre: torcido no veia
      // la pelota, y sin ver la pelota no hacia nada que lo enderezara.
      //
      // Va ANTES de mirar la pelota: si esta torcido, primero se acomoda.
      // De todos modos no pierde nada — si la pelota aparece mientras se
      // endereza, la va a ver apenas termine, que son menos de 2 segundos.
      //
      // 🚨 CON DESCANSO OBLIGATORIO ENTRE INTENTOS. Sin esto quedaba un
      // lazo feo: si el enderezado no llegaba a converger, volvia a
      // ESPERANDO todavia torcido, medía de nuevo, y disparaba la MISMA
      // maniobra que acababa de fracasar — sin tope y sin descanso. Y como
      // este bloque corta antes de mirar la pelota, un robot atrapado ahi
      // SE QUEDA SIN ATAJAR. Con el descanso, en el peor caso pierde unos
      // segundos y despues vuelve a jugar.
      if (enderezarActivado && rumboBase >= 0
          && ahora - t_chequeoEspera >= MS_CHEQUEO_ESPERA
          && ahora - t_ultimoIntentoEnderezar >= MS_DESCANSO_ENDEREZAR) {
        t_chequeoEspera = ahora;
        float r = rumboActual();
        if (r >= 0 && fabs(diferencia(rumboBase, r)) > TOLERANCIA_ESPERA) {
          t_ultimoIntentoEnderezar = ahora;
          enderezandoEnEspera = true;
          pasarAEnderezarse();
          break;
        }
      }
      if (hayQueDespejar()) {
        if (intentarDespejar(ahora)) digitalWrite(LED, HIGH);
      } else if (veLaPelota()) {
        fase = SIGUIENDO; t_fase = ahora;
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
      } else if (patrullarSinPelota && enderezarActivado && rumboBase >= 0
                 && giroscopoRespondiendo
                 && ahora - t_fase >= MS_SIN_PELOTA_PARA_PATRULLAR
                 && ahora - t_ultimoMovimiento >= MS_QUIETO_PARA_PATRULLAR) {
        // 2026-10-12: sin pelota a la vista y hace rato que no se mueve.
        // A patrullar, que el juez vea que esta vivo (y de paso se centra).
        arrancarPatrulla();
      }
      break;

    case SIGUIENDO: {
      digitalWrite(LED, HIGH);

      if (hayQueDespejar()) {
        // Si la cuenta dice que no llega, intentarDespejar() no cambia de
        // fase y el robot sigue en SIGUIENDO, acomodandose. No sale a
        // perderse.
        int faseAntes = fase;
        parar();
        intentarDespejar(ahora);
        if (fase != faseAntes) break;
      }
      if (!veLaPelota()) {
        parar();
        fase = ESPERANDO; t_fase = ahora;
        break;
      }

      // 2026-09-08: ahora se sigue ADONDE VA A ESTAR la pelota, no donde
      // esta. Asi el robot llega a tiempo en vez de correr siempre atras.
      float desvio = desvioPredicho();

      // Zona muerta: si la pelota esta casi enfrente, quedarse quieto. Sin
      // esto el robot tiembla persiguiendo el ruido de la camara.
      if (fabs(desvio) < ZONA_MUERTA_PELOTA) {
        parar();
        // 🚨 2026-09-01: FALTABA ESTO, y era una fuente de torceduras.
        // Sin reiniciar la rampa, el proximo arranque salia A FONDO DE
        // GOLPE — justo lo que hace patinar las ruedas y torcer al robot.
        // Y pasa muchas veces por minuto, cada vez que la pelota entra y
        // sale de la zona muerta.
        reiniciarRampaMovimiento();
        break;
      }

      int fuerza = (int)(fabs(desvio) * kpLateral);
      if (fuerza > pwmMaxLateral) fuerza = pwmMaxLateral;
      if (fuerza < pwmMinLateral) fuerza = pwmMinLateral;

      bool hayQueIrDerecha = (desvio > 0);

      // Si la pelota cruzo al otro lado, el robot tiene que invertir la
      // marcha. Sin reiniciar la rampa, saldria para el otro lado a fondo
      // de una — el mismo tiron que hace patinar al arrancar, pero peor,
      // porque ademas viene con velocidad en contra.
      static bool ibaDerecha = true;
      if (hayQueIrDerecha != ibaDerecha) {
        ibaDerecha = hayQueIrDerecha;
        reiniciarRampaMovimiento();
      }

      // 2026-08-18: SIN LIMITE LATERAL, a pedido del equipo.
      //
      // Antes el seguimiento se frenaba al pisar la linea del costado del
      // area. Se saco porque el robot vuelve chueco de algunos despejes, y
      // estando torcido sus dos sensores de atras quedan en diagonal
      // respecto de la linea: uno la pisa ANTES de que el robot este
      // realmente en el borde, y el seguimiento se cortaba de mas.
      //
      // ⚠️ Esto saca el SINTOMA, no la causa. Lo que hay que arreglar es el
      // enderezado (ver la bitacora del 11/08). Si algun dia se arregla,
      // conviene volver a probar con el limite puesto.
      //
      // ⚠️ Consecuencia: lo unico que detiene el seguimiento ahora es perder
      // de vista la pelota. El robot puede terminar lejos del arco, y ahi se
      // queda — todavia no vuelve solo al centro.
      //
      // La linea SIGUE usandose para volver del despeje (ATRAS_HASTA_LINEA).
      // Eso no se toco.
      //
      // 2026-10-13: VUELVE A HABER LIMITE, pero distinto del que se saco.
      // Mira solo el sensor del lado hacia el que va (ver "LIMITE LATERAL",
      // donde esta el interruptor) y en vez de frenarse vuelve al medio.
      // Sin giroscopio no: no puede volver por tiempo.
      if (volverAlMedioEnElLateral && enderezarActivado && rumboBase >= 0
          && giroscopoRespondiendo) {
        int lado = hayQueIrDerecha ? +1 : -1;
        if (pisaElLateral(lado)) lecturasDeLateral++;
        else                     lecturasDeLateral = 0;
        if (lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
          arrancarVueltaDelLateral(lado);
          break;
        }
      }

      moverDeCostado(hayQueIrDerecha ? fuerza : -fuerza);
      break;
    }

    case ADELANTE: {
      // Antes esto era una sola linea que ni miraba la camara: el robot
      // manejaba 533 ms con los ojos cerrados. Ahora persigue.
      // El angulo YA se decidio al salir, con la pelota bien a la vista.
      // El robot se compromete con esa diagonal y el giroscopio se la
      // sostiene derecha. No necesita seguir viendo la pelota — que es lo
      // bueno, porque justo cuando la tiene encima la camara la pierde.
      int lateral = lateralDelDespeje;

      // Ademas del angulo fijo, la version de corregir sobre la marcha.
      // Apagada por defecto: se prueba una cosa por vez.
      if (perseguirEnElDespeje && veLaPelota()) {
        float d = desvioPelota();          // cm reales, + = a la derecha
        if (fabs(d) > ZONA_MUERTA_PELOTA) {
          lateral = (int)(d * kpPersecucion);
          if (lateral >  PWM_MAX_PERSECUCION) lateral =  PWM_MAX_PERSECUCION;
          if (lateral < -PWM_MAX_PERSECUCION) lateral = -PWM_MAX_PERSECUCION;
        }
      }

      // El avance puede ser menor que potenciaDespeje: si la diagonal
      // pedia mucho costado, se achicaron LOS DOS JUNTOS para que la
      // direccion siga siendo la calculada. Por eso la ida tambien se
      // estira, para recorrer los mismos centimetros.
      avanzarPersiguiendo(pwmAvanceDespeje, lateral);

      if (ahora - t_fase >= msIdaDespeje) {
        if (frenarAntesDeVolver) frenar(); else parar();
        fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;
    }

    case PAUSA_MEDIO:
      // 2026-10-20: frena y se queda quieto antes de volver (ver donde se
      // declara frenarAntesDeVolver). Con false, suelta como antes.
      if (frenarAntesDeVolver) frenar(); else parar();
      if (ahora - t_fase >= (frenarAntesDeVolver ? MS_ESPERA_ANTES_DE_VOLVER
                                                 : MS_PAUSA_MEDIO)) {
        fase = ATRAS_HASTA_LINEA; t_fase = ahora;
      }
      break;

    case ATRAS_HASTA_LINEA: {
      // 2026-09-08, pedido del equipo: VOLVER POR LA MISMA DIAGONAL.
      // Si la ida fue torcida y la vuelta fuera derecha, el robot
      // terminaria corrido de costado y fuera del centro del arco.
      //
      // El costado se achica en la misma proporcion en que se achica la
      // potencia (la ida va a 200 y la vuelta a 110) para que el ANGULO
      // sea el mismo, no solo el sentido. Y va con el signo cambiado,
      // porque se esta desandando el camino.
      int lateralVuelta = -(lateralDelDespeje * potenciaRetroceso)
                          / pwmAvanceDespeje;
      atrasEnDiagonal(potenciaRetroceso, lateralVuelta);

      if (algunoDeAtrasVeBlanco()) {
        // FRENAR, no soltar. Antes aca iba parar(), y el robot seguia de
        // largo y se pasaba de la linea: quedaba en un lugar distinto cada
        // vez. Frenando se detiene practicamente donde la vio.
        frenar();
        fase = FRENANDO; t_fase = ahora;
        break;
      }
      // Freno de emergencia: un robot que retrocede sin limite se va de la
      // cancha. El codigo 2025 tiene ese bug exacto en el arquero.
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        frenar();
        fase = FRENANDO; t_fase = ahora;
      }
      break;
    }

    case FRENANDO:
      // El freno se sostiene un ratito y despues se sueltan los motores:
      // dejarlos cortocircuitados de gusto solo calienta el driver.
      if (ahora - t_fase >= msFreno) {
        parar();
        fase = ACOMODANDO; t_fase = ahora;
        reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = ahora;
      }
      break;

    case ACOMODANDO: {
      if (!enderezarActivado) { terminarDespeje(); break; }
      float r = rumboActual();
      if (rumboBase < 0 || r < 0) {
        terminarDespeje();
        break;
      }
      float error = diferencia(rumboBase, r);
      if (fabs(error) <= TOLERANCIA_ACOMODO) {
        // FRENAR, no soltar: llego al punto y hay que clavarlo ahi mismo.
        frenar(); fase = ACOMODO_ASENTAR; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_TIMEOUT_ACOMODO) {
        frenar();
        fase = ACOMODO_ASENTAR; t_fase = ahora;
        break;
      }
      int pwm = (int)(fabs(error) * KP_ACOMODO);
      if (pwm > PWM_MAX_ACOMODO) pwm = PWM_MAX_ACOMODO;
      if (pwm < PWM_MIN_ACOMODO) pwm = PWM_MIN_ACOMODO;
      // arranque suave: las tres de golpe dejan mudo al giroscopio
      if (pwm < pwmAcomodoAplicado) pwmAcomodoAplicado = pwm;
      else if (ahora - t_rampa >= RAMPA_MS) {
        t_rampa = ahora;
        pwmAcomodoAplicado += RAMPA_PASO;
        if (pwmAcomodoAplicado > pwm) pwmAcomodoAplicado = pwm;
      }
      int g = (error > 0) ? pwmAcomodoAplicado : -pwmAcomodoAplicado;
      aplicar(g, g, g);          // las tres parejas = rotacion pura
      break;
    }

    case ACOMODO_ASENTAR: {
      // Esperar que pase la inercia ANTES de volver a medir. Sin esta pausa
      // el robot cree que llego cuando todavia esta girando.
      //
      // 🚨 2026-09-01: ACA HABIA UN parar() Y ESO ERA PARTE DEL PROBLEMA.
      // parar() SUELTA los motores: el robot llegaba al punto, se cortaba
      // la corriente, y la inercia lo seguia llevando de largo. Por eso se
      // pasaba y despues tenia que volver — la oscilacion.
      // Ahora se sostiene el FRENO ELECTRICO (cortocircuita los motores y
      // se frenan solos) los primeros msFreno, y recien despues se sueltan.
      // Es el mismo freno que se midio el 18/08 para el regreso del despeje.
      if (ahora - t_fase < msFreno) { frenar(); break; }
      parar();
      if (ahora - t_fase < MS_ASENTAR_ACOMODO) break;
      float error = diferencia(rumboBase, rumboActual());
      if (fabs(error) > TOLERANCIA_ACOMODO
          && reintentosAcomodo < MAX_REINTENTOS_ACOMODO) {
        reintentosAcomodo++;
        pwmAcomodoAplicado = 0; t_rampa = ahora;
        fase = ACOMODANDO; t_fase = ahora;
      } else {
        terminarDespeje();
      }
      break;
    }

    case ADELANTE_CHICO:
      // Va DESPUES de enderezarse, para que los 10 cm salgan derechos.
      // Despacio y con rumbo sostenido: la gracia es terminar bien parado,
      // no llegar rapido.
      adelanteControlado(potenciaEmpujon);
      if (ahora - t_fase >= (unsigned long)msEmpujonFinal) {
        // Frenar, no soltar. Es corto: si suelta, la inercia se lleva
        // puesta buena parte de los 10 cm.
        frenar();
        delay(msFreno);
        parar();
        if (empujonDeArranque) {
          // Era el poquito del arranque, no el final de un despeje: a
          // esperar la pelota ya mismo, sin enfriarse.
          empujonDeArranque = false;
          reiniciarCorreccion();
          pasarAEsperar();
        } else {
          fase = ENFRIANDO; t_fase = ahora;
        }
      }
      break;

    case MIRANDO: {
      // 2026-10-27 (2): ver "MIRAR ANTES DE SALIR". Frena, despues mide
      // QUIETO, y sale recien con la pelota a umbralCm.
      digitalWrite(LED, HIGH);
      if (ahora - t_fase < MS_ASENTAR_MIRADA) { frenar(); break; }
      parar();
      if (!veLaPelota()) { hayMirada = false; pasarAEsperar(); break; }

      bool vencio = (ahora - t_fase >= MS_MAX_MIRANDO);
      bool yaMidio = (ahora - t_fase >= MS_ASENTAR_MIRADA + MS_MIRAR) && calcularMirada();

      // 0.1027d: ya midio y la pelota todavia esta lejos. Esperar SOLO si
      // viene y va a llegar a umbralCm antes del tope; si no, a seguirla YA
      // (la c esperaba siempre hasta el tope, y por eso despejaba menos).
      if (yaMidio && distanciaPelota() > umbralCm) {
        unsigned long pasaron = ahora - t_fase;
        float quedanS = (pasaron < MS_MAX_MIRANDO) ? (MS_MAX_MIRANDO - pasaron) / 1000.0 : 0;
        bool viene = vaMirada > PISO_VELOCIDAD
                  && (distanciaPelota() - umbralCm) / vaMirada <= quedanS;
        if (!viene) {
          Serial.print("MIRE y la pelota no viene (se acerca a "); Serial.print(vaMirada, 0);
          Serial.println(" cm/s): vuelvo a seguirla");
          vencio = true;
        }
      }

      if (yaMidio && distanciaPelota() <= umbralCm) {
        hayMirada = true;
        bool salio = intentarDespejar(ahora);
        Serial.print("MIRE "); Serial.print(nMirada); Serial.print(" cuadros: a ");
        Serial.print(distanciaPelota(), 0); Serial.print(" cm, costado ");
        Serial.print(desvioPelota(), 1); Serial.print(" cm, va de costado a ");
        Serial.print(vyMirada, 0); Serial.print(" cm/s, se acerca a ");
        Serial.print(vaMirada, 0); Serial.print(" cm/s -> ");
        if (salio) { Serial.print("SALE, costado "); Serial.println(lateralDelDespeje); }
        else Serial.println(motivoNoSale == 2 ? "NO SALE (muy abierto, mas de MAX_ANGULO_DIAGONAL)"
                                              : "NO SALE (el avance no alcanza)");
        hayMirada = false;
        if (salio) break;
        vencio = true;                     // no sale: a seguirla
      }
      if (vencio) {
        if (!yaMidio) Serial.println("MIRE y no junte 3 cuadros a tiempo: vuelvo a seguirla");
        t_finMirada = ahora;
        vecesSeguidas = 0;
        fase = SIGUIENDO; t_fase = ahora;
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
      }
      break;
    }

    case ENFRIANDO:
      parar();
      vecesSeguidas = 0;
      if (ahora - t_fase >= MS_ENFRIAMIENTO) {
        reiniciarCorreccion();
        fase = ESPERANDO; t_fase = ahora;
      }
      break;
  }

  // ------------------------------------------------------------------
  // AVISO: "ESTOY SIN GIROSCOPIO"
  // ------------------------------------------------------------------
  // Va DESPUES del switch a proposito: asi PISA el LED que haya puesto la
  // fase. Cuando no hay rumbo, esta senal manda sobre todas las demas.
  //
  // Parpadeo de 10 por segundo — el mas rapido de todo el programa. El que
  // le sigue es de 5 por segundo (las cuentas regresivas), asi que se
  // distingue: este se ve casi como una luz temblando.
  //
  // Se chequea el sensor cada 200 ms y no en cada vuelta, para no cargar
  // el bus I2C al pedo.

  if (ahora - t_chequeoGiro >= 200) {
    t_chequeoGiro = ahora;
    rumboActual();                 // actualiza giroscopoRespondiendo
  }

  bool sinRumbo = (!hayGiroscopo)
               || (!giroscopoRespondiendo)
               || (fase != ARMANDOSE && rumboBase < 0);

  if (sinRumbo && fase != APAGADO) {
    digitalWrite(PIN_AVISO, ((ahora / MS_PARPADEO_AVISO) % 2) ? HIGH : LOW);
  }
}
