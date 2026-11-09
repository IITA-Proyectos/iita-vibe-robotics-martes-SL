/* =====================================================================
   TECNICA CHALLENGE — al medio, de un arco al otro, y a despejar
   (se llamaba recorrido-arcos; el equipo le cambio el nombre el 08/11)
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-08
   =====================================================================

   🚨 ES UNA PRUEBA, NO EL PROGRAMA DE PARTIDO. Para jugar hay que volver
   a cargar la 0921f (funciona/partido-0921). De ahi salen casi todas las
   piezas de este programa, copiadas con los MISMOS numeros: moverse con
   rampa, sostener el rumbo, la rutina de ir al medio, el enderezado y el
   despeje. Si se cambia un numero aca, alla NO cambia.

   EL PEDIDO DEL EQUIPO (2026-11-08), textual y con su correccion:
     "crea un programa nuevo aparte de el de juego, el robot tiene que en
      cualquier parte de la cancha ir por tiempo hacia el medio de la
      cancha [correccion: el robot debe ir hacia el borde blanco de la
      cancha y luego debe ir hacia el medio] y el robot debe de
      identificar el arco ya sea azul o amarillo y se debe acercar hasta
      tocar el borde de la linea blanca del area. luego el robot debera
      dar una vuelta de 180 grados con ayuda del giroscopo y tendria que
      detectar el otro arco ya sea amarillo o azul y por ultimo el robot
      giraria en su eje para encontrar la pelota como lo que hace el
      robot delantero"
     + "y por ultimo tiene que despejar la pelota"

   ---------------------------------------------------------------------
   EL RECORRIDO, PASO POR PASO
   ---------------------------------------------------------------------
   0. LA CUENTA. Lo apoyan MIRANDO HACIA UN ARCO (cualquiera de los dos)
      y prenden la bateria. El LED parpadea lento y en los ultimos 3 s
      rapido: ahi hay que sacar las manos. Al terminar la cuenta anota
      hacia donde mira. Ese es el RUMBO DE REFERENCIA.

   1. AL LATERAL Y AL MEDIO. De costado hacia su IZQUIERDA hasta que un
      sensor de atras ve el blanco del lateral (el borde de la cancha).
      Frena, se endereza con el giroscopio y vuelve de costado hacia la
      derecha POR TIEMPO: 2228 ms a fuerza 100, que es lo que se midio el
      06/10 desde el lateral izquierdo hasta el medio. Es la rutina de ir
      al medio del partido (MEDIO_AL_LATERAL + MEDIO_AL_MEDIO).
        ¿Por que primero el lateral? Un tiempo solo sirve si se sabe
        DESDE DONDE se empezo a contar. El lateral es ese "desde donde".

   2. EL PRIMER ARCO. Se queda quieto mirando medio segundo. Si ve un
      arco (azul o amarillo: se queda con el que vio MAS veces), lo
      centra girando de a poquito con el giroscopio. Despues avanza
      derecho hacia el, corriendose de costado para tenerlo enfrente,
      hasta que el sensor de ADELANTE ve el blanco de la linea del area.
      Frena y anota de que color era.
      Si no ve ningun arco, gira despacio sobre su eje hasta encontrar uno.

   3. LA VUELTA Y EL OTRO ARCO. Gira 180 grados con el giroscopio (es el
      enderezado del partido, pero con el objetivo dado vuelta) y mira:
      tiene que ver el arco del OTRO color. Si lo ve, LED fijo un ratito.
      Si no, avisa con destellos largos y sigue igual.
      2026-11-08, pedido del equipo: despues AVANZA HASTA LA LINEA DEL AREA
      DEL OTRO ARCO (como el paso 2: si lo vio, se corrige con el; si no,
      derecho con el giroscopio), y ahi da OTRA VUELTA de 180 grados.
      Recien despues va a buscar la pelota. Todo esto cuenta como paso 3
      en el LED (3 destellos cortos).

   4. LA PELOTA. Primero mira quieto (capaz que ya la tiene enfrente). Si
      no la ve, gira sobre su eje despacio y siempre para el mismo lado,
      como el delantero. Al verla FRENA (girando lleva envion y se pasaria
      de largo), mira quieto y la centra girando con el giroscopio.
        Cerca (menos de ~70 cm) gira de una el angulo que calcula, como el
        delantero. LEJOS NO: la camara del arquero recorta la X en 200
        (= 70 cm) y el angulo sale 2 o 3 veces mas grande que el de
        verdad. Ahi la centra como al arco: de a pasitos de 6 grados hasta
        que la pelota cambia de lado. (Arreglado el 2026-11-08: antes, con
        la pelota lejos, iba y venia hasta el tope de 45 s sin despejar.)

   5. EL DESPEJE. Va hacia la pelota corriendose de costado para tenerla
      enfrente. Cuando la tiene a 30 cm y de frente (3 cuadros seguidos,
      igual que el partido) hace la ida del despeje: 533 ms a 200,
      derecho. Frena y se queda QUIETO. Si en el camino pisa una linea
      blanca con cualquier sensor, frena ahi.

   Cada paso tiene tope de tiempo. Si algo no aparece a tiempo (una
   linea, un arco, la pelota) lo avisa con el LED y por el cable, y sigue
   con el paso siguiente o se para, lo que sea mas seguro:

        paso 1  no ve el lateral en 6 s     -> sigue: ya esta contra el
                                               borde (lo mismo que hace
                                               el partido)
        paso 2  da la vuelta y no ve arco   -> vuelve a mirar hacia donde
                                               lo pusieron y va derecho
                no ve la linea del area     -> sigue con la vuelta
        paso 3  no ve el otro arco          -> sigue
        paso 4  2 vueltas sin ver la pelota -> SE PARA
        paso 5  pisa una linea, o no llega  -> SE PARA
        4 y 5   45 s en total con la pelota -> SE PARA (por si se queda
                                               yendo y viniendo)
        en cualquiera: el giroscopio se calla en pleno giro -> SE PARA

   ---------------------------------------------------------------------
   🎯 CADA VEZ QUE GIRA A PROPOSITO, CAMBIA EL RUMBO DE REFERENCIA
   ---------------------------------------------------------------------
   En el partido el robot NUNCA gira: siempre mira al frente. Por eso
   correccionDeRumbo() empuja SIEMPRE hacia el mismo rumbo (rumboBase).

   Aca el robot gira tres veces a proposito (para centrar el arco, la
   vuelta de 180 y para centrar la pelota). Si despues de girar rumboBase
   siguiera siendo el viejo, al avanzar la correccion lo volveria a
   torcer hacia donde miraba ANTES, y saldria chueco. Por eso, al empezar
   cada avance (hacia el arco y hacia la pelota) se hace
        rumboBase = hacia donde mira AHORA
   y se borra lo acumulado de la correccion (reiniciarCorreccion).

   ---------------------------------------------------------------------
   EL IDIOMA DEL LED (sin cable es lo unico que habla)
   ---------------------------------------------------------------------
        lento, y al final rapido   -> la cuenta: saquen las manos
        N destellos CORTOS y una   -> estoy haciendo el paso N (1 a 5)
          pausa, repitiendo
        LED FIJO 1,5 s             -> paso 3: vi el arco del otro color
        N destellos LARGOS, una    -> algo no cerro en el paso N. Lo avisa
          sola vez, quieto            y sigue (o se para: ver la tabla)
        latido lento (un destello  -> TERMINO y todo salio bien
          cortito cada 2 s)
        N destellos LARGOS,        -> TERMINO, pero el paso N fallo (es el
          repitiendo, quieto          PRIMERO que fallo)
        temblor (10 por segundo)   -> SIN GIROSCOPIO: no me muevo. Manda
                                      sobre todas las demas senales
        apagado                    -> lo pararon por el cable (tecla 0)

   El LED de la CAMARA (no el del robot) tambien sirve: rojo = ve la
   pelota, verde = ve el arco AMARILLO, azul = ve el arco AZUL, y los
   colores se mezclan. Si en el paso 2 el LED de la camara no tiene ni
   verde ni azul, la camara no esta viendo el arco.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Cargarlo con CARGAR-TECNICA-CHALLENGE.bat (doble clic), con la bateria
      APAGADA: el robot arranca solo apenas termina la carga.
   2. Desenchufar el USB. Apoyarlo MIRANDO A UN ARCO, quieto, y prender la
      bateria. Sacar las manos durante la cuenta.
   3. Mirar el LED. Cuando termina queda quieto: latido lento = salio
      todo bien; destellos largos = el numero de destellos es el paso
      que fallo.
   4. Para ver QUE vio y POR QUE hizo lo que hizo: con la bateria
      prendida, enchufar el USB y abrir pruebas/herramientas/mirar.bat.
      Escribe cada paso, lo que vio la camara, y cada medio segundo una
      linea de estado. Si lo enchufan recien al final, la tecla i vuelve
      a mostrar el informe.
        0 = PARAR     r = empezar de nuevo (otra cuenta)     i = informe
        e = callar / mostrar la linea de estado               ? = ayuda
      (Cerrar MIRAR con Ctrl+C le manda un 0: el robot se para.)

   🚨 DONDE APOYARLO. En la parte del MEDIO de la cancha (a lo largo):
   entre las dos areas, no al costado de un area, y no sobre una linea.
      - Al costado de un area, yendo hacia la izquierda puede pisar
        antes la linea del costado del area, que es paralela al lateral,
        y creer que es el lateral. El partido tiene el mismo problema.
      - Sobre una linea, primero se despega hacia la izquierda (como el
        partido). Si esta parado A LO LARGO de una linea no se despega
        nunca, y a los 6 s hace de cuenta que llego al lateral.
   Sin la computadora, la unica forma de pararlo es LA LLAVE DE LA BATERIA.

   ---------------------------------------------------------------------
   LO QUE TODAVIA NO SABEMOS (y como comprobarlo)
   ---------------------------------------------------------------------
   Esto NO se puede deducir leyendo codigo. Cada duda tiene su numero o
   su interruptor en el PANEL, con la explicacion de como comprobarla.
     a. Si la camara ve el arco del otro lado de la cancha, sobre todo el
        AMARILLO (pide 600 pixeles; el azul, 300). Lo del 25/08 fue con
        el azul y en la cancha vieja.
     b. El signo del AMARILLO nunca se midio: sale de la misma cuenta que
        el azul, que si se midio. -> amarilloInvertido
     c. Que script tiene hoy la camara del arquero. El del delantero ve
        mejor el amarillo. Sin PC se chequea con el LED de la camara
        (verde) mirando el arco amarillo.
     d. El AZUL da falsos: sombras, ropa oscura o robots negros. Por eso
        se le cree recien con 3 cuadros, y se elige el color visto MAS.
     e. Como se lee el arco pegado a la linea del area (puede llenar la
        imagen). Por eso la correccion de costado hacia el arco es suave
        y con tope.
     f. Que tan bien ve la pelota girando a 80 (el delantero gira a 35,
        pero sus motores son otros: en el arquero, debajo de ~75 las
        ruedas zumban y no giran).
     g. Con la pelota justo enfrente, el 25/08 la camara dio Yp = 10-11 en
        vez de 0 (bitacora de ese dia, sin explicar). Si se repite, centra
        la pelota torcida. -> OFFSET_Y_PELOTA
   Del arco solo se usa el SIGNO de su Y (de que lado esta) y donde
   cruza el cero. Sus X e Y NO son centimetros: la tabla de la camara es
   para una camara a 18,7 cm, el arco no esta apoyado en el piso y la X se
   recorta en 200 (bitacora 2026-08-25). La pelota si: cm = X / 2,87.

   ---------------------------------------------------------------------
   DE DONDE SALE CADA PIEZA
   ---------------------------------------------------------------------
   partido-0921 (0921f): motores, rampa, rumbo, lineas, camara, el
     arranque del giroscopio, el enderezado (ACOMODANDO), la rutina de ir
     al medio (MEDIO_*) y el despeje. Los pines son los del ARQUERO.
   seguir-y-despejar de antes del 21/09 y pruebas/ver-arcos: la lectura
     de los dos arcos y su signo.
   delantero: SOLO LA IDEA de buscar girando (rampa, frenar al verla,
     centrar por angulo). 🚨 Ni sus pines, ni su mapa de ruedas, ni sus
     signos, ni sus fuerzas: el delantero esta cableado DISTINTO.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


// ---- ARQUERO (ROBOT1) ---- copiados de la 0921f. 🚨 NO son los del delantero.
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
#define PIN_AVISO 13          // el del "sin giroscopio": es el mismo LED

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12

const unsigned long BAUDIOS = 19200;     // la camara (y el cable)


/* =====================================================================
   PANEL: TODOS LOS NUMEROS QUE SE PUEDEN TOCAR, JUNTOS
   =====================================================================
   Cada uno dice que hace y PARA QUE LADO moverlo. Los marcados
   "(del partido)" son los de la 0921f, con el mismo valor.
   Regla de oro: TOCAR UNO POR VEZ, probar, y anotar lo que paso.
   ===================================================================== */

// ---------------- los signos (interruptores) ----------------
//
// Lateral: (del partido, medido el 11/08) moverDeCostado(+) va a la DERECHA.
bool lateralInvertido = false;

// Camara: (del partido, medido el 11/08) con la pelota a la DERECHA la
// camara manda Yp NEGATIVO. Vale igual para los arcos.
bool camaraYInvertida = true;

// Arco AZUL: medido en cancha el 25/08, el centrado "iba para el lado
// correcto" con false. No tocar.
bool azulInvertido = false;

// Arco AMARILLO: ⚠️ NUNCA SE MIDIO. Sale de la misma funcion y la misma
// tabla de la camara que el azul, asi que deberia ser igual.
// COMO COMPROBARLO: en el paso 2, mirando el arco amarillo, el robot
// tiene que girar HACIA el arco al centrarlo (por el cable dice
// "el arco esta a la DERECHA/IZQUIERDA"). Si gira para el otro lado y
// lo pierde, poner true.
bool amarilloInvertido = false;

// Que byte es cada arco. El script de la camara manda
//      [201, Xp, Yp+100,  202, Xam, Yam+100,  203, Xaz, Yaz+100]
// o sea 202 = AMARILLO y 203 = AZUL (leido en el script, no medido).
// COMO COMPROBARLO: con MIRAR abierto, poner el robot mirando el arco
// AMARILLO: en la linea de estado tiene que cambiar "am" (no "az").
// Si cambia "az", poner true.
bool arcosCambiados = false;

// Para que lado crece el rumbo del giroscopio. (Medido a mano el 28/07:
// girando a la DERECHA paso de 359,9 a 99,7, o sea que CRECE.) Solo se usa
// para traducir "el arco/la pelota esta a la derecha" a un rumbo objetivo.
// COMO COMPROBARLO: con MIRAR abierto, girar el robot a mano hacia la
// derecha (como las agujas del reloj, visto de arriba): "rumbo" tiene que
// subir. Si baja, poner false.
bool rumboCreceALaDerecha = true;

// Para que lado gira buscando (el arco si no lo ve, y la pelota).
// +1 = aplicar(+,+,+) = horario visto de arriba (probar-ruedas, 'g').
// -1 = al reves. Da lo mismo: es para probar los dos lados.
const int SENTIDO_GIRO_BUSCAR = +1;


// ---------------- 0. la cuenta ----------------
// Tiempo para apoyarlo y sacar las manos. Los ultimos 3 s parpadea rapido.
// Mas largo si no les da el tiempo. (El partido usa 2000.)
const unsigned long MS_CUENTA = 5000;
// (del partido) si al final de la cuenta el giroscopio todavia no da
// datos, espera esto mas antes de rendirse.
const unsigned long MS_ESPERA_EXTRA_GIRO = 2000;
const unsigned long MS_ESPERA_DATOS_GIRO = 5000;   // (del partido) en setup()


// ---------------- 1. al lateral y al medio ----------------
// -1 = va a buscar el lateral de su IZQUIERDA, como el partido (LADO_MEDIO).
// +1 = el de la derecha. ⚠️ El tiempo al medio cambia segun el lado.
const int LADO_LATERAL = -1;
// (del partido, VEL_MEDIO_BUSCAR) de costado hasta ver el lateral. Termina
// al VER la linea, asi que la velocidad no cambia DONDE frena.
const int VEL_BUSCAR_LINEA = 90;     // 2026-11-08: 110 -> 90, "mas lento en todas las partes" (frena al VER la linea: no cambia donde)
// 🚨 (del partido, VEL_MEDIO_MEDIR) ATADA A LOS MS DE ABAJO. Los 2228 ms
// se midieron a 100: a otra fuerza el robot hace otro camino en el mismo
// tiempo y queda en cualquier lado. NO es una perilla de gusto.
const int VEL_AL_MEDIO = 100;
// (del partido, MS_MEDIO_AL_MEDIO) del lateral IZQUIERDO al medio, yendo
// a la derecha a 100. Medido el 06/10 (2213 y 2243), antes de mover los
// motores. Si queda corto del medio: subir. Si se pasa: bajar.
const unsigned long MS_MEDIO_DESDE_IZQ = 2228;
// Desde el DERECHO (si LADO_LATERAL = +1). Es la cuenta del 06/10, NO se
// probo en cancha.
const unsigned long MS_MEDIO_DESDE_DER = 2331;
// (del partido, MS_MEDIO_ALEJARSE) si arranca sobre una linea: despues de
// dejar de verla, se sigue alejando esto, para no quedar justo en el borde.
const unsigned long MS_DESPEGARSE = 400;
// (del partido, MS_MEDIO_TOPE_LATERAL) tope para llegar al lateral. El
// cruce entero de la cancha son ~4450 ms. Si se cumple, da por hecho que
// esta contra el borde y sigue (lo avisa).
const unsigned long MS_TOPE_LATERAL = 6000;


// ---------------- 2. el primer arco ----------------
// Cuanto mira QUIETO antes de decidir. La camara manda ~26 cuadros por
// segundo: 500 ms son ~13 cuadros. Mas largo = mas seguro y mas lento.
const unsigned long MS_MIRAR_ARCO = 500;
// Cuantos cuadros tiene que ver un color en esa mirada para creerle (como
// MUESTRAS_MINIMAS_ARCO del delantero). Si se le cuelan azules falsos:
// subir a 5. Si no ve un arco que esta: bajar a 2.
const int MUESTRAS_MINIMAS_ARCO = 3;
// Girando: cuantos cuadros SEGUIDOS con un arco para frenar a mirarlo.
const int CUADROS_PARA_FRENAR_ARCO = 2;
// Tope de la busqueda girando: grados girados (lo cuenta el giroscopio)
// o tiempo, lo que pase primero. 400 = una vuelta y un poquito.
const float GRADOS_MAX_BUSCAR_ARCO = 400;
const unsigned long MS_TOPE_BUSCAR_ARCO = 15000;
// Para centrar el arco gira de a ESTE PASO y vuelve a mirar. Del arco no
// sirve el tamano de su Y (de lejos salta de -100 a +100 con nada), solo
// su signo: por eso gira de a poquito hasta que el arco cambia de lado.
// Termina, como mucho, a medio paso del centro.
// Si tarda mucho en centrar: subir (8, 10). Si se pasa de un lado al otro
// sin parar: bajar (4). Con menos de ~4 el giro no llega a moverlo.
const float PASO_GIRO_ARCO = 6.0;
// (del 25/08) si el arco esta a menos de esto del centro, ya esta.
// Unidades de CAMARA, no cm.
const float ZONA_MUERTA_ARCO = 6.0;
// Topes del centrado. Si se cumplen, avanza igual (y lo corrige de costado).
const int MAX_PASOS_CENTRAR_ARCO = 10;
const unsigned long MS_TOPE_CENTRAR_ARCO = 15000;
// Si mirando quieto no ve el arco elegido esta cantidad de veces seguidas,
// lo da por perdido y vuelve a buscar girando.
const int MIRADAS_PARA_PERDER_ARCO = 3;
// Fuerza para ir derecho hacia el arco. La de ir derecho en la rutina del
// medio (VEL_MEDIO_RECTO del partido). Debajo de ~70 no arranca.
const int VEL_HACIA_ARCO = 85;       // 2026-11-08: 110 -> 85, "mas lento en todas las partes". No bajar de ~75: parado no arranca
// Correccion de costado yendo hacia el arco: PWM por unidad de camara de
// desvio (el kpArco del 25/08). Si hace eses: bajar (1,0). Si no se
// corrige: subir (3,0).
const float KP_ARCO_LATERAL = 2.0;
// Tope de esa correccion. El mismo tope de la persecucion del partido.
const int MAX_LATERAL_ARCO = 60;
// (del 25/08) el arco lejos tiembla: cuanto pesa cada lectura nueva en
// el promedio. Mas chico = mas suave y mas lento en reaccionar.
const float SUAVIZADO_ARCO = 0.3;
// Tope para llegar a la linea del area. A 110 hace 1 cm cada 18-24 ms:
// 3500 ms son 1,5 a 2 metros, mas que media cancha.
// 2026-11-08: 3500 -> 4500, porque ahora va a 85 y no a 110.
const unsigned long MS_TOPE_AL_AREA = 4500;
// 2026-11-08 (pedido del equipo): despues de la vuelta tambien va hasta la
// linea del area del OTRO arco, que esta a casi toda la cancha: 1,8 m son
// 3,2 a 4,3 s a 110, y a 85 mas (~5,5 s). Tope mas largo, para que no se
// corte antes.
const unsigned long MS_TOPE_AL_OTRO_AREA = 8000;


// ---------------- 3. la vuelta ----------------
// Tope de cada intento del giro de 180 (despues reintenta, hasta 3 veces,
// igual que el enderezado). A 75-85 la vuelta es lenta: no achicarlo.
const unsigned long MS_TOPE_GIRO_180 = 6000;
// Cuanto mira quieto despues de la vuelta. Mas largo que en el paso 2:
// el otro arco esta LEJOS y se ve chiquito (el amarillo, peor).
const unsigned long MS_MIRAR_OTRO_ARCO = 1500;
// Cuanto queda el LED fijo cuando lo ve.
const unsigned long MS_AVISO_OK = 1500;
// Si el giro quedo mas torcido que esto, lo avisa por el cable.
const float GRADOS_GIRO_TORCIDO = 15.0;


// ---------------- 4. buscar la pelota ----------------
// Fuerza del giro buscando (el arco del paso 2 tambien). 🚨 NO copiar el
// 35 del delantero: este robot tiene otros motores, y debajo de ~70-75 las
// ruedas zumban y no giran (PWM_MIN_ACOMODO del partido, 75).
// Si zumba y no gira: subir a 85-90. Si pasa por la pelota y no la ve:
// bajar, pero no menos de 75.
const int VEL_GIRO_BUSCAR = 80;
// 🎯 2026-11-08, lo que vio el equipo: "el robot gira hasta que detecte la
// pelota, nunca para" y "bajale para que sea mas lento". Girando, la camara
// casi no la ve (el delantero lo midio el 29/09: frenado la ve el 98% de las
// veces, girando el 45%). Y mas despacio que 75 no se puede: las ruedas
// zumban. Entonces busca DE A PASITOS: gira PASO_BUSCAR_PELOTA grados,
// FRENA, mira QUIETO (MS_MIRAR_PELOTA), y si no la ve gira otro pasito.
// Mas lento en promedio, y mirando siempre quieto.
bool buscarDeAPasitos = true;                   // false = como antes, girando sin parar
// El pasito. Mas chico que lo que ve la camara de ancho (~60 grados), para
// no saltearse la pelota. Si tarda mucho: subir (30). Si se saltea la
// pelota: bajar (15).
const float PASO_BUSCAR_PELOTA = 20.0;
// Cuantos cuadros SEGUIDOS con pelota para creerle mientras gira.
const int CUADROS_PARA_CREER_PELOTA = 2;
// Cuanto mira quieto para centrarla (despues del freno).
const unsigned long MS_MIRAR_PELOTA = 300;
// Centrada = a menos de estos grados del frente (el TOL_ANG_SALE del
// delantero, 6). Siempre MAS que TOLERANCIA_ACOMODO (3): si no, el giro
// nunca la deja dentro.
const float TOL_ANG_PELOTA = 6.0;
// CERCA (menos de ~70 cm): gira de una el angulo que calcula, con este
// tope, y vuelve a mirar. Hasta MAX_INTENTOS_CENTRAR_PELOTA veces.
const float GIRO_MAX_CENTRAR = 30.0;
const int MAX_INTENTOS_CENTRAR_PELOTA = 6;
// 🚨 LEJOS (2026-11-08, revision): la camara NO manda una X mas grande que
// 200, que son 200 / 2,87 = 70 cm. Una pelota a 1,8 m llega como "70 cm",
// pero su Y si llega entera. Entonces el angulo calculado sale 2 o 3 veces
// MAS GRANDE que el de verdad: si gira eso, se pasa al otro lado, vuelve
// a pasarse, y se queda yendo y viniendo hasta el tope de 45 s sin
// despejar nunca. (El delantero si puede usar el angulo: su camara esta
// mas alta y su 200 son 2 metros de verdad, no 70 cm.)
// Por eso, con la X recortada hace LO MISMO QUE CON EL ARCO: usa solo el
// SIGNO (de que lado esta) y gira de a un pasito hasta que la pelota
// cambia de lado (queda a menos de un paso) o entra en TOL_ANG_PELOTA.
// (TOL_ANG_PELOTA sirve igual: si el angulo exagerado ya es chico, el de
// verdad es mas chico todavia.)
// Desde que X cuenta como recortada. 195 y no 200: es el PROMEDIO de la
// mirada, y un solo cuadro que llegue en 199 ya lo baja de 200.
const int X_PELOTA_RECORTADA = 195;
// El pasito de lejos. Si tarda mucho en centrarla: subir (8). Si se pasa
// de un lado al otro sin parar: bajar (4). Con menos de ~4 el giro no
// llega a mover el robot (es el mismo pasito del arco).
const float PASO_GIRO_PELOTA_LEJOS = 6.0;
// Cuantos pasitos como mucho (10 x 6 = 60 grados). Despues va igual y la
// corrige de costado.
const int MAX_PASOS_PELOTA_LEJOS = 10;
// Tope de una busqueda girando: grados o tiempo, lo que pase primero.
const float GRADOS_MAX_BUSCAR_PELOTA = 720;     // dos vueltas
// 2026-11-08: 20000 -> 60000 y 45000 -> 90000: de a pasitos, cada 20 grados
// tarda ~1,3 s (arrancar, girar, frenar, mirar), o sea ~25 s por vuelta.
const unsigned long MS_TOPE_BUSCAR_PELOTA = 60000;
// Tope de TODO lo de la pelota junto (pasos 4 y 5, con idas y vueltas).
const unsigned long MS_TOPE_PELOTA_TOTAL = 90000;
// ⚠️ El corrimiento del cero de la camara, en unidades de camara crudas.
// El 25/08, con la pelota justo enfrente, dio Yp = 10-11 en vez de 0.
// COMO COMPROBARLO: pelota quieta justo enfrente a ~30 cm, MIRAR abierto,
// leer "pel X,Y" en la linea de estado. Poner aca esa Y (con su signo).
// 0 = como el partido.
const int OFFSET_Y_PELOTA = 0;


// ---------------- 5. ir a la pelota y despejar ----------------
// Fuerza para ir hacia la pelota (la de ir derecho del partido).
const int VEL_HACIA_PELOTA = 85;     // 2026-11-08: 110 -> 85, "mas lento en todas las partes"
// (del partido) correccion de costado hacia la pelota: PWM por cm real.
// Si hace eses: bajar (4). Si se le escapa de costado: subir (8).
const float kpPersecucion = 6.0;
const int PWM_MAX_PERSECUCION = 60;            // (del partido)
const float ZONA_MUERTA_PELOTA = 1.4;          // (del partido) cm reales
// Si la pelota se va mas al costado que esto (grados) un rato, frena y la
// vuelve a centrar girando. Es el TOL_ANG_ENTRA del delantero, pero mas
// ancho: el arquero ademas se corre de costado.
// Solo CERCA (X menos que X_PELOTA_RECORTADA): de lejos el angulo sale
// exagerado y lo haria frenar y volver al paso 4 a cada rato. De lejos la
// corrige solo la correccion de costado (esa usa los cm de costado, que
// si llegan bien, y tiene el signo bien).
const float TOL_ANG_RECENTRAR = 20.0;
const unsigned long MS_TORCIDA_PARA_RECENTRAR = 300;
// Sin verla menos que esto, cuenta como que la ve (MS_GRACIA del delantero).
const unsigned long MS_GRACIA_PELOTA = 300;
// La camara pierde la pelota cuando la tiene PEGADA (manda X = 0). Si la
// ultima vez estaba a umbralCm + esto o menos, "perderla es tenerla" y
// despeja igual.
const float MARGEN_PEGADA_CM = 5.0;
// Tope para llegar a la pelota sin despejar.
const unsigned long MS_TOPE_IR_A_PELOTA = 6500;   // 2026-11-08: 5000 -> 6500 (ahora va a 85)
// (del partido) despeja si esta a esto o menos, en cm REALES...
float umbralCm = 30.0;
// ...y a esto o menos de costado (cm reales)...
float umbralDesvio = 20.0;
// ...durante esta cantidad de cuadros seguidos.
const int VECES_PARA_CREERLE = 3;
// (del partido) la ida del despeje: 533 ms a 200 = ~50 cm (regla del 04/08).
// 🚨 REGLA: umbralCm NUNCA mas grande que el alcance de la ida (~50).
// Aca la ida arranca ya andando (viene de ir a 110), asi que hace unos cm
// mas que en el partido, que arranca quieto.
// 2026-11-08: "mas lento en todas las partes": 200 -> 160, y la ida se
// estira (533 -> 666 ms) para que llegue mas o menos igual de lejos
// (160 x 666 = 200 x 533; es una cuenta, no esta medido).
int potenciaDespeje = 160;
int msAdelante      = 666;
// La ida, ¿derecha o corrigiendo con la camara? Se decidio DERECHA. La
// 0921f usa true (va corrigiendo de costado, perseguirEnElDespeje).
bool perseguirEnElDespeje = false;


// ---------------- las lineas ----------------
// (del partido, medido en cancha el 15/09) verde 56-95, blanco ~760.
// El de adelante (A12) dio verde 56 y blanco 759 el 15/09: le sirve igual.
int umbralBlanco = 425;
// Cuantas lecturas SEGUIDAS en blanco para creerle (pasos 2 y 5).
const int LECTURAS_BLANCO = 2;


// ---------------- girar en el lugar (el enderezado del partido) ----------------
// Todos (del partido), explicados alla en ACOMODANDO. 🚨 El minimo NO se
// baja: debajo de ~70 las ruedas zumban y no giran (era el bug del 01/09).
// Con un maximo de 110 giraba tan rapido que se pasaba y oscilaba.
const float TOLERANCIA_ACOMODO = 3.0;          // grados
const float KP_ACOMODO = 1.5;
const int   PWM_MIN_ACOMODO = 75;
const int   PWM_MAX_ACOMODO = 85;
const unsigned long MS_ASENTAR_ACOMODO = 300;
const unsigned long MS_TIMEOUT_ACOMODO = 3000;  // tope de cada intento (el 180 tiene el suyo)
const int MAX_REINTENTOS_ACOMODO = 3;
// Arranque suave del giro: "las tres de golpe dejan mudo al giroscopio".
const int RAMPA_PASO = 3;
const unsigned long RAMPA_MS = 15;
// Si girando el giroscopio deja de contestar este rato, se para todo.
const unsigned long MS_GIRO_MUDO_MAX = 1000;


// ---------------- frenos y pausas ----------------
unsigned long msFreno = 200;                    // (del partido) freno electrico
const unsigned long MS_QUIETO_ENTRE_PASOS = 300; // (del partido, MS_MEDIO_QUIETO)


// ---------------- el cable ----------------
// La linea de estado cada medio segundo (tecla e para callarla).
bool mostrarEstado = true;
const unsigned long MS_ESTADO = 500;


// ---------------- el LED ----------------
const unsigned long CORTO_ON = 120, CORTO_OFF = 250, CORTO_PAUSA = 1000;
const unsigned long LARGO_ON = 450, LARGO_OFF = 300, LARGO_PAUSA = 1200;
const unsigned long MS_PARPADEO_AVISO = 50;     // (del partido) sin giroscopio
/* ======================== FIN DEL PANEL ============================ */


// ---- cosas fijas del partido (no son perillas) ----
const float CAMARA_POR_CM = 2.87;    // unidades de camara por cm real (medido el 25/08, 10 a 50 cm)
const int LADO_FRENTE  = 50;         // la mezcla de costado: las dos de adelante...
const int LADO_TRASERA = 89;         // ...y la de atras (geometria del robot)
const float KP_RUMBO = 5.0;          // la correccion de rumbo (medida el 04/08)
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;
const int RAMPA_MOV_PASO = 10;       // la rampa de arranque (medida el 18/08)
const unsigned long RAMPA_MOV_MS = 10;


// ---- giroscopio ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool giroscopoRespondiendo = false;
unsigned long t_chequeoGiro = 0;
float rumboBase = -1;          // el rumbo que se sostiene al moverse
float rumboInicial = -1;       // hacia donde lo pusieron (el primer arco)
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// ---- rampa de los movimientos ----
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;

// ---- lineas ----
// "Armado": un sensor recien cuenta el blanco despues de haber visto verde
// en este movimiento. Asi, si arranca parado sobre una linea (despues de la
// vuelta queda sobre la linea del area), no frena en el acto.
bool armado[3];
int  lecturasEnBlanco[3];
unsigned long t_soltoLinea = 0;

// ---- camara ----
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0,  Yp = 0;
int  Xam = 0, Yam = 0;
int  Xaz = 0, Yaz = 0;
unsigned long t_ultimoPaquete = 0, t_ultimaPelota = 0;
unsigned long t_ultimoAmarillo = 0, t_ultimoAzul = 0;
unsigned long paquetesTotales = 0;
int  seguidosPelota = 0, seguidosAmarillo = 0, seguidosAzul = 0;
int  vecesSeguidas = 0;               // para el despeje, como el partido
float ultimaDistPelotaCm = 999;

// Lo que se junta en una "mirada" quieto.
struct Vista {
  int  vistas;
  long xSuma;
  long ySuma;
};
bool  contando = false;
int   paquetesVentana = 0;
Vista vPelota, vAmarillo, vAzul;

enum ColorArco { NINGUNO, AMARILLO, AZUL };
ColorArco colorArco = NINGUNO;        // el del paso 2 (y en el paso 3, el del otro arco)
ColorArco colorPrimerArco = NINGUNO;  // 2026-11-08: el del paso 2, guardado para el informe
float arcoSuave = 0;
bool  hayArcoSuave = false;

// ---- las fases ----
enum Fase {
  CUENTA,            // 0. la cuenta del arranque
  SIN_GIRO,          // no hay rumbo: quieto
  PAUSA,             // entre paso y paso: frena y queda quieto un momento
  GIRANDO,           // girar en el lugar hasta un rumbo (el ACOMODANDO del partido)
  GIRO_ASENTAR,      // ...y esperar que pase la inercia (ACOMODO_ASENTAR)
  AVISO_FALLA,       // algo no cerro: quieto, destellos largos
  AVISO_OK,          // vio el otro arco: LED fijo
  P1_DESPEGARSE,     // 1. si arranco sobre una linea
  P1_AL_LATERAL,     // 1. de costado hasta el lateral
  P1_AL_MEDIO,       // 1. al medio por tiempo
  P2_MIRAR_ARCO,     // 2. quieto, mirando el arco
  P2_BUSCAR_ARCO,    // 2. girando despacio, buscando un arco
  P2_AL_AREA,        // 2. hacia el arco hasta la linea del area
  P3_MIRAR_OTRO,     // 3. despues de la vuelta: ¿veo el otro color?
  P4_MIRAR_PELOTA,   // 4. quieto, mirando la pelota
  P4_BUSCAR_PELOTA,  // 4. girando, buscando la pelota
  P5_A_LA_PELOTA,    // 5. hacia la pelota
  P5_DESPEJE,        // 5. la ida del despeje
  FIN,               // termino: quieto
  DETENIDO           // lo pararon por el cable
};
Fase fase = CUENTA;
unsigned long t_fase = 0;
Fase faseDespuesDePausa = CUENTA;
Fase faseDespuesDelGiro = CUENTA;
Fase faseDespuesDelAviso = CUENTA;
// 2026-11-08: P2_AL_AREA se usa DOS veces. 1 = hacia el primer arco (paso
// 2). 2 = despues de la vuelta, hacia el OTRO arco (paso 3), y al tocar su
// linea da la segunda vuelta y recien ahi va a buscar la pelota.
int idaAlArea = 1;

// ---- el giro en el lugar ----
float rumboObjetivo = 0;
unsigned long msTopeGiro = 3000;
int   pasoDelGiro = 0;
int   reintentosGiro = 0;
int   pwmGiroAplicado = 0;
unsigned long t_rampaGiro = 0;
unsigned long t_mudoDesde = 0;
float errorFinalGiro = 0;

// ---- buscar girando ----
int   pwmBusqueda = 0;
float giradoAlEmpezarPaso = 0;   // 2026-11-08: buscar de a pasitos (cuanto llevaba girado al arrancar el pasito)
unsigned long t_rampaBusqueda = 0;
float giradoBusqueda = 0;
float rumboPrevio = -1;
unsigned long t_cuentaGiro = 0;
unsigned long t_empezoBusqueda = 0;

// ---- el centrado del arco ----
int   signoAnteriorArco = 0;
int   pasosCentradoArco = 0;
unsigned long t_empezoCentrado = 0;
int   miradasSinArco = 0;

// ---- la pelota ----
int   intentosCentrarPelota = 0;
int   signoAnteriorPelota = 0;   // de que lado estaba antes del ultimo PASITO de lejos (0 = no hubo)
unsigned long t_empezoPelota = 0;
unsigned long t_torcidaDesde = 0;

// ---- lo que va pasando (para el LED y el informe) ----
int   pasoActual = 0;
unsigned long t_empezoPaso = 0;
unsigned long msPaso[6];
const char* motivoFalla[6];
int   primerPasoFallado = 0;
int   pasoDelAviso = 0;
bool  vioElOtroArco = false;
ColorArco colorDelOtro = NINGUNO;
float errorDelGiro180 = 0;
bool  hizoElGiro180 = false;
bool  despejo = false;
unsigned long t_ultimoEstado = 0;


// ================================================================ cable

// ¿Hay alguien mirando por el cable? En la cancha no hay USB: no se
// escribe nada, asi el robot no pierde tiempo esperando a un cable que no
// esta.
bool cable() { return (bool)Serial; }

void decir(const char* texto) {
  if (cable()) Serial.println(texto);
}

const char* nombreColor(ColorArco c) {
  if (c == AMARILLO) return "AMARILLO";
  if (c == AZUL)     return "AZUL";
  return "ninguno";
}

const char* nombreFase(Fase f) {
  switch (f) {
    case CUENTA:           return "cuenta";
    case SIN_GIRO:         return "SIN GIROSCOPIO";
    case PAUSA:            return "pausa";
    case GIRANDO:          return "girando";
    case GIRO_ASENTAR:     return "asentando el giro";
    case AVISO_FALLA:      return "aviso de falla";
    case AVISO_OK:         return "aviso: lo vi";
    case P1_DESPEGARSE:    return "despegarse de la linea";
    case P1_AL_LATERAL:    return "al lateral";
    case P1_AL_MEDIO:      return "al medio por tiempo";
    case P2_MIRAR_ARCO:    return "mirando el arco";
    case P2_BUSCAR_ARCO:   return "buscando un arco girando";
    case P2_AL_AREA:       return "hacia el arco, hasta la linea del area";
    case P3_MIRAR_OTRO:    return "mirando el otro arco";
    case P4_MIRAR_PELOTA:  return "mirando la pelota";
    case P4_BUSCAR_PELOTA: return "buscando la pelota girando";
    case P5_A_LA_PELOTA:   return "hacia la pelota";
    case P5_DESPEJE:       return "DESPEJE";
    case FIN:              return "FIN";
    case DETENIDO:         return "DETENIDO por el cable";
  }
  return "?";
}

const char* nombrePaso(int p) {
  switch (p) {
    case 1: return "al lateral y al medio";
    case 2: return "el primer arco, hasta la linea del area";
    case 3: return "la vuelta de 180 y el otro arco";
    case 4: return "buscar la pelota girando";
    case 5: return "ir a la pelota y despejar";
  }
  return "";
}


// ================================================================ motores

// SOLTAR los motores (del partido). No es frenar: sigue de largo.
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// FRENO ELECTRICO (del partido, la "variante 2" medida el 18/08): las
// patas en 0 con el PWM al MAXIMO cortocircuita el motor. Ojo que se
// parece a parar(): la unica diferencia es el PWM, y el efecto es opuesto.
void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

// Un valor CON SIGNO a una rueda (del partido). Con signos se pueden SUMAR
// movimientos: avanzar + correrse + no girar.
void rueda(int ina, int inb, int pwm, int v) {
  if (v > 255)  v = 255;
  if (v < -255) v = -255;
  digitalWrite(ina, v >= 0 ? 1 : 0);
  digitalWrite(inb, v >= 0 ? 0 : 1);
  analogWrite(pwm, abs(v));
}

// Izquierda, derecha, trasera. Las mezclas del arquero:
//     avanzar     (+p, -p,  0)      de costado a la derecha (+fr, +fr, -tr)
//     retroceder  (-p, +p,  0)      girar                   ( g,   g,   g)
// (Sin el reloj de la patrulla, que aca no hay.)
void aplicar(int v1, int v2, int v3) {
  rueda(INA1, INB1, PWM1, v1);
  rueda(INA2, INB2, PWM2, v2);
  rueda(INA3, INB3, PWM3, v3);
}


// ================================================================ rampa

// (del partido) arrancando de golpe las ruedas patinan y el robot se
// tuerce. Sube de a 10 cada 10 ms: a 110 llega en ~110 ms. Bajar es de
// golpe (pedir menos fuerza no hace patinar nada).
void reiniciarRampaMovimiento() {
  potenciaRampa = 0;
  t_rampaMov = millis();
}

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


// ================================================================ rumbo

// Devuelve el rumbo, o -1 si el sensor esta mudo (del partido). Un numero
// imposible en vez de 0, porque 0.0 es a la vez un rumbo valido y lo que
// devuelve el chip cuando no contesta.
float rumboActual() {
  if (!hayGiroscopo) { giroscopoRespondiendo = false; return -1; }
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    giroscopoRespondiendo = false;
    return -1;
  }
  giroscopoRespondiendo = true;
  return e.orientation.x;
}

// objetivo - actual, llevado a (-180, 180]. Con exactamente 180 da +180:
// la vuelta del paso 3 arranca girando para el lado de los positivos.
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// Un rumbo cualquiera llevado a [0, 360).
float normalizar(float a) {
  while (a >= 360.0) a -= 360.0;
  while (a < 0.0)    a += 360.0;
  return a;
}

// "Girar tantos grados hacia la DERECHA" en grados de rumbo. Si el rumbo
// crece a la derecha (medido el 28/07) es lo mismo; si no, al reves.
float haciaLaDerecha(float grados) {
  return rumboCreceALaDerecha ? grados : -grados;
}

void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

// Cuanto sumarle a las TRES ruedas para que el robot no gire (del partido).
// 🚨 Empuja siempre hacia rumboBase: ver el encabezado.
int correccionDeRumbo() {
  if (rumboBase < 0) return 0;
  float r = rumboActual();
  if (r < 0) return 0;

  float error = diferencia(rumboBase, r);

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;

  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  return (int)c;
}

// Antes de cada avance: "de aca en adelante, derecho es hacia donde miro
// AHORA". Ver el encabezado: sin esto el robot se tuerce hacia el rumbo
// viejo.
void fijarRumboAca() {
  float r = rumboActual();
  if (r >= 0) rumboBase = r;
  reiniciarCorreccion();
}


// ================================================================ lineas

// (del partido) los dos de atras, sin vueltas. Los usa el paso 1.
bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= umbralBlanco; }
bool algunoDeAtrasVeBlanco() { return veBlancoIzq() || veBlancoDer(); }

// En el partido no hay funcion para el de ADELANTE (A12): solo se declara.
// Esta es nueva. 0 = adelante, 1 = atras-izq, 2 = atras-der.
void armarSensoresDeLinea() {
  for (int i = 0; i < 3; i++) { armado[i] = false; lecturasEnBlanco[i] = 0; }
}

bool sensorPisaBlanco(int i, int pin) {
  bool blanco = analogRead(pin) >= umbralBlanco;
  if (!blanco) {
    armado[i] = true;            // vio verde: de aca en mas el blanco cuenta
    lecturasEnBlanco[i] = 0;
    return false;
  }
  if (!armado[i]) return false;  // arranco encima de esta linea: no cuenta
  if (lecturasEnBlanco[i] < 100) lecturasEnBlanco[i]++;
  return lecturasEnBlanco[i] >= LECTURAS_BLANCO;
}

bool blancoAdelante() { return sensorPisaBlanco(0, LINEA_ADELANTE); }

// Cualquiera de los tres. Se leen LOS TRES siempre (sin cortar en el
// primero) para que los tres vayan "armandose".
bool algunSensorPisaBlanco() {
  bool a = sensorPisaBlanco(0, LINEA_ADELANTE);
  bool b = sensorPisaBlanco(1, LINEA_ATRAS_IZQ);
  bool c = sensorPisaBlanco(2, LINEA_ATRAS_DER);
  return a || b || c;
}


// ================================================================ moverse

// (del partido) s > 0 = a la DERECHA. La fuerza pasa por la rampa; la
// correccion de rumbo no (tiene que actuar enseguida, y es chica).
void moverDeCostado(int s) {
  if (lateralInvertido) s = -s;
  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;
  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;
  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}

// (del partido) avanzar sosteniendo el rumbo.
void adelanteControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(+p + c, -p + c, 0 + c);
}

// (del partido) avanzar Y correrse de costado a la vez, sin girar.
// lateral > 0 = correrse a la DERECHA. El costado no pasa por la rampa.
void avanzarPersiguiendo(int potencia, int lateral) {
  if (lateralInvertido) lateral = -lateral;
  int p  = rampa(potencia);
  int c  = correccionDeRumbo();
  int fr = (lateral * LADO_FRENTE)  / 100;
  int tr = (lateral * LADO_TRASERA) / 100;
  aplicar(+p + fr + c, -p + fr + c, -tr + c);
}

// Girar sobre su eje buscando, con rampa (la idea del delantero). La
// rampa es la del enderezado (de a 3 cada 15 ms) y no la del delantero:
// en el arquero, arrancar las tres ruedas de golpe deja mudo al
// giroscopio, y aca el giroscopio cuenta cuanto se giro.
void girarBuscando(int sentido) {
  unsigned long ahora = millis();
  if (ahora - t_rampaBusqueda >= RAMPA_MS) {
    t_rampaBusqueda = ahora;
    if (pwmBusqueda < VEL_GIRO_BUSCAR) {
      pwmBusqueda += RAMPA_PASO;
      if (pwmBusqueda > VEL_GIRO_BUSCAR) pwmBusqueda = VEL_GIRO_BUSCAR;
    }
  }
  int g = (sentido >= 0) ? pwmBusqueda : -pwmBusqueda;
  aplicar(g, g, g);       // las tres parejas = rotacion pura
}

// Suma lo que se giro buscando (como giradoOrbita del delantero). Asi el
// tope es "una vuelta" y no "tantos segundos", que depende de la bateria.
void contarGiro() {
  unsigned long ahora = millis();
  if (ahora - t_cuentaGiro < 20) return;
  t_cuentaGiro = ahora;
  float r = rumboActual();
  if (r < 0) return;
  if (rumboPrevio >= 0) giradoBusqueda += fabs(diferencia(r, rumboPrevio));
  rumboPrevio = r;
}

void reiniciarCuentaDeBusqueda() {
  giradoBusqueda = 0;
  rumboPrevio = -1;
  t_empezoBusqueda = millis();
}


// ================================================================ camara

// Paquete de 9 bytes, a 19200 por Serial1 (~26 por segundo):
//   [201, Xp, Yp+100,  202, Xam, Yam+100,  203, Xaz, Yaz+100]
//    pelota            arco AMARILLO        arco AZUL
// X = 0 quiere decir NO LO VEO (la Y llega como 0). Ojo: Y = 0 tambien
// quiere decir "centrado", por eso la unica prueba de "no lo veo" es X.
// Como todos los datos van de 0 a 200, el 201 no se confunde con un dato
// y sincronizar con el 201 es seguro.

void anotar(Vista &v, int x, int y) {
  if (x <= 0) return;
  v.vistas++;
  v.xSuma += x;
  v.ySuma += y;
}

void empezarMirada() {
  vPelota.vistas = 0;   vPelota.xSuma = 0;   vPelota.ySuma = 0;
  vAmarillo.vistas = 0; vAmarillo.xSuma = 0; vAmarillo.ySuma = 0;
  vAzul.vistas = 0;     vAzul.xSuma = 0;     vAzul.ySuma = 0;
  paquetesVentana = 0;
  contando = true;
}

// Desvio de un arco, con el signo arreglado: positivo = a la DERECHA.
// En UNIDADES DE CAMARA (no son cm: ver el encabezado).
float desvioArco(ColorArco c, float yCruda) {
  float y = camaraYInvertida ? -yCruda : yCruda;
  bool inv = (c == AMARILLO) ? amarilloInvertido : azulInvertido;
  return inv ? -y : y;
}

// Desvio de la pelota en cm REALES, positivo = a la DERECHA (del partido,
// mas el corrimiento del cero del panel).
float desvioPelotaDe(float yCruda) {
  float y = yCruda - OFFSET_Y_PELOTA;
  if (camaraYInvertida) y = -y;
  return y / CAMARA_POR_CM;
}
float desvioPelota() { return desvioPelotaDe((float)Yp); }

// Hacia donde esta la pelota, en GRADOS: positivo = a la derecha. El 2,87
// se cancela (esta arriba y abajo). Si no la ve, 0.
// ⚠️ Con la X recortada (pelota a mas de ~70 cm) sale EXAGERADO: el signo
// esta bien, el tamano no. Ver X_PELOTA_RECORTADA en el panel.
float anguloPelota(float x, float yCruda) {
  if (x <= 0) return 0;
  return atan2(desvioPelotaDe(yCruda), x / CAMARA_POR_CM) * 180.0 / PI;
}

// Centrar la pelota desde cero: sin intentos gastados y sin "lado de
// antes" (si no, un lado viejo haria creer que ya cambio de lado).
void reiniciarCentradoPelota() {
  intentosCentrarPelota = 0;
  signoAnteriorPelota = 0;
}

bool veoArco(ColorArco c) {
  if (c == AMARILLO) return Xam > 0 && (millis() - t_ultimoAmarillo < 300);
  if (c == AZUL)     return Xaz > 0 && (millis() - t_ultimoAzul < 300);
  return false;
}

bool hayQueDespejar() { return vecesSeguidas >= VECES_PARA_CREERLE; }

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
    if (!(paquete[0] == 201 && paquete[3] == 202 && paquete[6] == 203)) continue;

    unsigned long ahora = millis();
    Xp = paquete[1];  Yp = paquete[2] - 100;
    // Los arcos: en el partido se tiran desde el 21/09. Aca hacen falta.
    int xA = paquete[4], yA = paquete[5] - 100;    // byte 202
    int xB = paquete[7], yB = paquete[8] - 100;    // byte 203
    if (!arcosCambiados) { Xam = xA; Yam = yA; Xaz = xB; Yaz = yB; }
    else                 { Xam = xB; Yam = yB; Xaz = xA; Yaz = yA; }
    t_ultimoPaquete = ahora;
    paquetesTotales++;

    if (Xp > 0) {
      t_ultimaPelota = ahora;
      ultimaDistPelotaCm = Xp / CAMARA_POR_CM;
      if (seguidosPelota < 1000) seguidosPelota++;
    } else {
      seguidosPelota = 0;
    }
    if (Xam > 0) { t_ultimoAmarillo = ahora; if (seguidosAmarillo < 1000) seguidosAmarillo++; }
    else         seguidosAmarillo = 0;
    if (Xaz > 0) { t_ultimoAzul = ahora;     if (seguidosAzul < 1000) seguidosAzul++; }
    else         seguidosAzul = 0;

    // El arco elegido, suavizado (el de lejos tiembla).
    if (colorArco != NINGUNO) {
      int x = (colorArco == AMARILLO) ? Xam : Xaz;
      int y = (colorArco == AMARILLO) ? Yam : Yaz;
      if (x > 0) {
        float d = desvioArco(colorArco, (float)y);
        if (!hayArcoSuave) { arcoSuave = d; hayArcoSuave = true; }
        else arcoSuave = arcoSuave * (1.0 - SUAVIZADO_ARCO) + d * SUAVIZADO_ARCO;
      }
    }

    // (del partido) para despejar: cerca Y de frente, varios cuadros
    // seguidos. Un cuadro solo no alcanza: un reflejo naranja lo dispararia.
    float dCm  = Xp / CAMARA_POR_CM;
    float dLat = desvioPelotaDe((float)Yp);
    if (Xp > 0 && dCm <= umbralCm && fabs(dLat) <= umbralDesvio) {
      if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
    } else {
      vecesSeguidas = 0;
    }

    if (contando) {
      paquetesVentana++;
      anotar(vPelota,   Xp,  Yp);
      anotar(vAmarillo, Xam, Yam);
      anotar(vAzul,     Xaz, Yaz);
    }
  }
}

void contarMirada(const char* que, Vista &v) {
  if (!cable()) return;
  Serial.print("     "); Serial.print(que); Serial.print(": ");
  if (v.vistas == 0) { Serial.println("no"); return; }
  Serial.print(v.vistas); Serial.print(" cuadros, X medio ");
  Serial.print((float)v.xSuma / v.vistas, 0);
  Serial.print(", Y medio ");
  Serial.println((float)v.ySuma / v.vistas, 1);
}

void informarMirada(unsigned long ms) {
  if (!cable()) return;
  Serial.print("   miro "); Serial.print(ms); Serial.print(" ms quieto: ");
  Serial.print(paquetesVentana); Serial.println(" paquetes de la camara");
  if (paquetesVentana == 0) {
    Serial.println("   !! LA CAMARA NO MANDA NADA. ¿Esta prendida? ¿El cable a Serial1?");
    return;
  }
  contarMirada("arco amarillo", vAmarillo);
  contarMirada("arco azul    ", vAzul);
  contarMirada("pelota       ", vPelota);
}


// ================================================================ fases

int pasoDeFase(Fase f) {
  switch (f) {
    case P1_DESPEGARSE: case P1_AL_LATERAL: case P1_AL_MEDIO:           return 1;
    case P2_MIRAR_ARCO: case P2_BUSCAR_ARCO: case P2_AL_AREA:           return (idaAlArea == 2) ? 3 : 2;
    case P3_MIRAR_OTRO:                                                 return 3;
    case P4_MIRAR_PELOTA: case P4_BUSCAR_PELOTA:                        return 4;
    case P5_A_LA_PELOTA: case P5_DESPEJE:                               return 5;
    case GIRANDO: case GIRO_ASENTAR:                                    return pasoDelGiro;
    default:                                                            return 0;
  }
}

void cambiarDePaso(int p) {
  unsigned long ahora = millis();
  if (pasoActual >= 1 && pasoActual <= 5) msPaso[pasoActual] += ahora - t_empezoPaso;
  pasoActual = p;
  t_empezoPaso = ahora;          // el LED cuenta los destellos desde aca
  if (cable()) {
    Serial.println();
    Serial.print("===== PASO "); Serial.print(p); Serial.print(": ");
    Serial.print(nombrePaso(p));
    Serial.print("   (a los "); Serial.print(ahora / 1000.0, 1); Serial.println(" s)");
  }
}

// Pasar a una fase. Todas arrancan suave (rampa en 0) y con la correccion
// de rumbo borrada, MENOS el despeje: ese viene de ir a 110 hacia la
// pelota, y bajar a 0 para volver a subir seria un tiron al pedo.
void entrarA(Fase f) {
  fase = f;
  t_fase = millis();
  if (f != P5_DESPEJE) { reiniciarRampaMovimiento(); reiniciarCorreccion(); }

  int p = pasoDeFase(f);
  if (p > 0 && p != pasoActual) cambiarDePaso(p);

  if (cable() && f != GIRO_ASENTAR) {
    Serial.print(" > "); Serial.println(nombreFase(f));
  }

  switch (f) {
    case P1_DESPEGARSE:
    case P1_AL_LATERAL:
      t_soltoLinea = millis();
      break;
    case P2_MIRAR_ARCO:
    case P3_MIRAR_OTRO:
    case P4_MIRAR_PELOTA:
      parar();
      empezarMirada();
      break;
    case P2_BUSCAR_ARCO:
    case P4_BUSCAR_PELOTA:
      // La rampa del giro desde 0, y los "seguidos" en 0: tiene que verlo
      // de nuevo GIRANDO, no valen los cuadros de antes.
      pwmBusqueda = 0; t_rampaBusqueda = millis();
      seguidosAmarillo = 0; seguidosAzul = 0; seguidosPelota = 0;
      rumboPrevio = -1;
      giradoAlEmpezarPaso = giradoBusqueda;
      break;
    case P2_AL_AREA:
    case P5_A_LA_PELOTA:
      fijarRumboAca();             // 🚨 ver el encabezado
      armarSensoresDeLinea();
      hayArcoSuave = false;
      vecesSeguidas = 0;
      t_torcidaDesde = 0;
      if (cable()) {
        Serial.print("   derecho = rumbo "); Serial.println(rumboBase, 1);
      }
      break;
    case GIRANDO:
      reintentosGiro = 0; pwmGiroAplicado = 0; t_rampaGiro = millis();
      t_mudoDesde = 0;
      parar();
      if (cable()) {
        Serial.print("   girar hasta el rumbo "); Serial.print(rumboObjetivo, 1);
        Serial.print(" (estoy en "); Serial.print(rumboActual(), 1); Serial.println(")");
      }
      break;
    case FIN:
      parar();
      if (pasoActual >= 1 && pasoActual <= 5) {
        msPaso[pasoActual] += millis() - t_empezoPaso;
        t_empezoPaso = millis();
      }
      informeFinal();
      break;
    case DETENIDO:
    case SIN_GIRO:
      frenar();
      break;
    default:
      break;
  }
}

// Entre paso y paso: frenar, soltar y quedarse quieto un momento (del
// partido, pausaMedio). Sin delay(), para no dejar de leer la camara.
void pausa(Fase siguiente) {
  frenar();
  faseDespuesDePausa = siguiente;
  fase = PAUSA;
  t_fase = millis();
}

// Deja listo un giro en el lugar. Arranca cuando se entre a GIRANDO
// (directo, o despues de una pausa o de un aviso).
void prepararGiro(float objetivo, unsigned long tope, Fase despues, int paso) {
  rumboObjetivo = normalizar(objetivo);
  msTopeGiro = tope;
  faseDespuesDelGiro = despues;
  pasoDelGiro = paso;
}

// Algo no cerro en el paso N: lo anota, lo dice por el cable, frena, y
// muestra N destellos largos UNA vez. Despues sigue con "despues".
void avisarFalla(int paso, const char* motivo, Fase despues) {
  frenar();
  if (paso >= 1 && paso <= 5 && motivoFalla[paso] == nullptr) motivoFalla[paso] = motivo;
  if (primerPasoFallado == 0) primerPasoFallado = paso;
  pasoDelAviso = paso;
  faseDespuesDelAviso = despues;
  fase = AVISO_FALLA;
  t_fase = millis();
  if (cable()) {
    Serial.print("!! PASO "); Serial.print(paso); Serial.print(": ");
    Serial.println(motivo);
    Serial.print("   (lo aviso con "); Serial.print(paso);
    Serial.print(" destellos largos y sigo con: ");
    Serial.print(despues == FIN ? "NADA, me paro" : nombreFase(despues));
    Serial.println(")");
  }
}

unsigned long duracionAvisoFalla(int paso) {
  return (unsigned long)paso * (LARGO_ON + LARGO_OFF) + 800;
}

void avisarOk(Fase despues) {
  frenar();
  faseDespuesDelAviso = despues;
  fase = AVISO_OK;
  t_fase = millis();
}


// ================================================================ decisiones

// Paso 2, al terminar de mirar quieto: ¿que arco veo, y esta centrado?
void decidirArco() {
  informarMirada(MS_MIRAR_ARCO);

  if (colorArco == NINGUNO) {
    bool sirveAm = vAmarillo.vistas >= MUESTRAS_MINIMAS_ARCO;
    bool sirveAz = vAzul.vistas     >= MUESTRAS_MINIMAS_ARCO;
    if (!sirveAm && !sirveAz) {
      decir("   no veo ningun arco: giro despacio hasta encontrar uno");
      entrarA(P2_BUSCAR_ARCO);
      return;
    }
    // Se queda con el que vio MAS veces: el azul da falsos (sombras, ropa
    // oscura), y uno falso suele aparecer menos que el de verdad.
    colorArco = (vAmarillo.vistas > vAzul.vistas) ? AMARILLO : AZUL;
    signoAnteriorArco = 0;
    pasosCentradoArco = 0;
    miradasSinArco = 0;
    t_empezoCentrado = millis();
    if (cable()) {
      Serial.print("   ELIJO EL ARCO "); Serial.println(nombreColor(colorArco));
    }
  }

  Vista &v = (colorArco == AMARILLO) ? vAmarillo : vAzul;
  if (v.vistas < MUESTRAS_MINIMAS_ARCO) {
    miradasSinArco++;
    if (miradasSinArco >= MIRADAS_PARA_PERDER_ARCO) {
      decir("   perdi el arco: vuelvo a buscar girando");
      colorArco = NINGUNO;
      entrarA(P2_BUSCAR_ARCO);
    } else {
      entrarA(P2_MIRAR_ARCO);      // mirar de nuevo
    }
    return;
  }
  miradasSinArco = 0;

  float d = desvioArco(colorArco, (float)v.ySuma / v.vistas);   // + = derecha
  int signo = (d > 0) ? +1 : -1;
  bool cruzo = (signoAnteriorArco != 0 && signo != signoAnteriorArco);
  bool centrado = (fabs(d) <= ZONA_MUERTA_ARCO) || cruzo;

  if (cable()) {
    Serial.print("   el arco esta a la "); Serial.print(d > 0 ? "DERECHA" : "IZQUIERDA");
    Serial.print(" (desvio "); Serial.print(d, 1); Serial.println(" de camara)");
  }

  if (centrado) {
    decir(cruzo ? "   cambio de lado: queda centrado (a menos de un paso)"
                : "   CENTRADO");
    entrarA(P2_AL_AREA);
    return;
  }
  float r = rumboActual();
  if (pasosCentradoArco >= MAX_PASOS_CENTRAR_ARCO
      || millis() - t_empezoCentrado >= MS_TOPE_CENTRAR_ARCO || r < 0) {
    decir("   no termine de centrarlo: voy igual y lo corrijo de costado");
    entrarA(P2_AL_AREA);
    return;
  }
  // Un pasito hacia el arco y volver a mirar.
  signoAnteriorArco = signo;
  pasosCentradoArco++;
  float giro = haciaLaDerecha(signo > 0 ? PASO_GIRO_ARCO : -PASO_GIRO_ARCO);
  prepararGiro(r + giro, MS_TIMEOUT_ACOMODO, P2_MIRAR_ARCO, 2);
  entrarA(GIRANDO);
}

// Paso 3, despues de la vuelta: ¿veo el OTRO color?
// 2026-11-08: la SEGUNDA vuelta, al llegar a la linea del area del otro
// arco. Lo que viene despues es el paso 4: se arrancan sus relojes aca.
void prepararSegundaVuelta() {
  t_empezoPelota = millis();
  reiniciarCuentaDeBusqueda();
  reiniciarCentradoPelota();
  prepararGiro(rumboBase + 180.0, MS_TOPE_GIRO_180, P4_MIRAR_PELOTA, 3);
}

// 2026-11-08 (pedido del equipo): despues de mirar el otro arco, va HASTA
// la linea de su area, como en el paso 2. Si lo vio, se corrige con el para
// tenerlo enfrente; si no, va derecho con el giroscopio (despues de la
// vuelta ya queda mirandolo).
void irAlOtroArco(ColorArco color) {
  idaAlArea = 2;
  colorPrimerArco = colorArco;
  colorArco = color;              // NINGUNO = derecho, sin corregir con la camara
}

void decidirOtroArco() {
  informarMirada(MS_MIRAR_OTRO_ARCO);

  bool veoAm = vAmarillo.vistas >= MUESTRAS_MINIMAS_ARCO;
  bool veoAz = vAzul.vistas     >= MUESTRAS_MINIMAS_ARCO;
  ColorArco otro = (colorArco == AMARILLO) ? AZUL
                 : (colorArco == AZUL)     ? AMARILLO : NINGUNO;

  if (otro == NINGUNO) {
    // En el paso 2 no supo el color: dice lo que ve y sigue.
    if (veoAm || veoAz) {
      colorDelOtro = (vAmarillo.vistas > vAzul.vistas) ? AMARILLO : AZUL;
      vioElOtroArco = true;
      if (cable()) {
        Serial.print("   veo el arco "); Serial.print(nombreColor(colorDelOtro));
        Serial.println(" (en el paso 2 no supe el color del primero)");
      }
      irAlOtroArco(colorDelOtro);
      avisarOk(P2_AL_AREA);
    } else {
      irAlOtroArco(NINGUNO);
      avisarFalla(3, "despues de la vuelta no veo ningun arco: voy derecho hasta su linea igual", P2_AL_AREA);
    }
    return;
  }

  bool veoOtro  = (otro == AMARILLO) ? veoAm : veoAz;
  bool veoMismo = (otro == AMARILLO) ? veoAz : veoAm;
  if (veoOtro) {
    colorDelOtro = otro;
    vioElOtroArco = true;
    if (cable()) {
      Serial.print("   VEO EL OTRO ARCO: "); Serial.print(nombreColor(otro));
      Serial.println(". La vuelta dio.");
    }
    irAlOtroArco(otro);
    avisarOk(P2_AL_AREA);
  } else {
    if (veoMismo) {
      decir("   veo el MISMO color del paso 2: ¿la vuelta no dio? ¿o un azul falso?");
    }
    irAlOtroArco(NINGUNO);
    avisarFalla(3, "despues de la vuelta no veo el arco del otro color: voy derecho hasta su linea igual", P2_AL_AREA);
  }
}

// Paso 4, al terminar de mirar quieto: ¿donde esta la pelota?
void decidirPelota() {
  informarMirada(MS_MIRAR_PELOTA);

  if (vPelota.vistas < CUADROS_PARA_CREER_PELOTA) {
    decir("   no veo la pelota: a buscarla girando");
    entrarA(P4_BUSCAR_PELOTA);
    return;
  }
  float x = (float)vPelota.xSuma / vPelota.vistas;
  float y = (float)vPelota.ySuma / vPelota.vistas;
  float ang = anguloPelota(x, y);                    // + = derecha
  // ¿Lejos? Con la X recortada el angulo sale exagerado: solo vale su
  // signo (ver X_PELOTA_RECORTADA en el panel).
  bool lejos = (x >= X_PELOTA_RECORTADA);
  int signo = (ang > 0) ? +1 : -1;
  if (cable()) {
    Serial.print("   pelota a "); Serial.print(x / CAMARA_POR_CM, 0);
    Serial.print(" cm (");      Serial.print(lejos ? "o mas: X recortada" : "real");
    Serial.print("), ");        Serial.print(fabs(ang), 1);
    Serial.print(" grados a la "); Serial.print(ang > 0 ? "DERECHA" : "IZQUIERDA");
    Serial.println(lejos ? " (exagerado: solo uso el lado)" : "");
  }

  if (fabs(ang) <= TOL_ANG_PELOTA) {
    decir("   CENTRADA: voy");
    entrarA(P5_A_LA_PELOTA);
    return;
  }
  // Lejos, como el arco: si despues del pasito quedo del OTRO lado, la
  // pelota esta a menos de un paso del centro. Ya esta.
  if (lejos && signoAnteriorPelota != 0 && signo != signoAnteriorPelota) {
    decir("   cambio de lado: queda centrada (a menos de un pasito)");
    entrarA(P5_A_LA_PELOTA);
    return;
  }
  int maxIntentos = lejos ? MAX_PASOS_PELOTA_LEJOS : MAX_INTENTOS_CENTRAR_PELOTA;
  if (intentosCentrarPelota >= maxIntentos) {
    decir("   no termino de centrarla: voy igual, la corrijo de costado");
    entrarA(P5_A_LA_PELOTA);
    return;
  }
  float r = rumboActual();
  if (r < 0) {
    avisarFalla(4, "el giroscopio no contesta: no puedo centrar la pelota", FIN);
    return;
  }
  float giro;
  if (lejos) {
    // Un pasito fijo hacia su lado, y volver a mirar.
    giro = (signo > 0) ? PASO_GIRO_PELOTA_LEJOS : -PASO_GIRO_PELOTA_LEJOS;
    signoAnteriorPelota = signo;
  } else {
    // Cerca el angulo es de verdad: girarlo de una, con tope.
    giro = ang;
    if (giro >  GIRO_MAX_CENTRAR) giro =  GIRO_MAX_CENTRAR;
    if (giro < -GIRO_MAX_CENTRAR) giro = -GIRO_MAX_CENTRAR;
    // El "cambio de lado" solo vale despues de un pasito chico: despues
    // de un giro grande, cambiar de lado NO quiere decir que este cerca.
    signoAnteriorPelota = 0;
  }
  intentosCentrarPelota++;
  prepararGiro(r + haciaLaDerecha(giro), MS_TIMEOUT_ACOMODO, P4_MIRAR_PELOTA, 4);
  entrarA(GIRANDO);
}


// ================================================================ cable: informe y teclas

void informeFinal() {
  if (!cable()) return;
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" TECNICA CHALLENGE — COMO TERMINO");
  Serial.println("=========================================================");
  for (int p = 1; p <= 5; p++) {
    Serial.print("  paso "); Serial.print(p); Serial.print(" (");
    Serial.print(nombrePaso(p)); Serial.print("): ");
    Serial.print(msPaso[p]); Serial.print(" ms  ");
    if (motivoFalla[p] != nullptr) {
      Serial.print("-> FALLO: "); Serial.println(motivoFalla[p]);
    } else if (msPaso[p] == 0) {
      Serial.println("-> no llego");
    } else {
      Serial.println("-> bien");
    }
  }
  Serial.println("---------------------------------------------------------");
  Serial.print("  arco del paso 2: "); Serial.println(nombreColor(idaAlArea == 2 ? colorPrimerArco : colorArco));
  Serial.print("  vuelta de 180: ");
  if (hizoElGiro180) {
    Serial.print("quedo a "); Serial.print(errorDelGiro180, 1);
    Serial.println(" grados del objetivo");
  } else {
    Serial.println("no se hizo");
  }
  Serial.print("  el otro arco: ");
  Serial.println(vioElOtroArco ? nombreColor(colorDelOtro) : "NO LO VI");
  Serial.print("  despejo: "); Serial.println(despejo ? "SI" : "NO");
  Serial.println("  (i = volver a mostrar esto, r = empezar de nuevo)");
  Serial.println("=========================================================");
}

void ayuda() {
  if (!cable()) return;
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" TECNICA CHALLENGE (2026-11-08) — 🚨 EL ROBOT SE MUEVE");
  Serial.println("=========================================================");
  Serial.println("  0 = PARAR            r = empezar de nuevo (otra cuenta)");
  Serial.println("  i = informe          e = callar/mostrar la linea de estado");
  Serial.println("  ? = esta ayuda");
  Serial.println("---------------------------------------------------------");
  Serial.println("  LED: N destellos cortos = paso N.  N largos = fallo el paso N.");
  Serial.println("       fijo = vi el otro arco.  latido lento = termino bien.");
  Serial.println("       temblor = SIN GIROSCOPIO.");
  Serial.println("  Estado: pel/am/az = X,Y crudos de la camara (X=0: no lo ve)");
  Serial.println("          lin = sensores adelante/atras-izq/atras-der (blanco >= 425)");
  Serial.println("=========================================================");
}

void mostrarEstadoPorCable(unsigned long ahora) {
  if (!mostrarEstado || !cable() || ahora - t_ultimoEstado < MS_ESTADO) return;
  if (fase == FIN || fase == DETENIDO || fase == SIN_GIRO) return;
  t_ultimoEstado = ahora;
  float r = rumboActual();
  Serial.print("   . p"); Serial.print(pasoActual);
  Serial.print(" "); Serial.print(nombreFase(fase));
  Serial.print(" | rumbo ");
  if (r < 0) Serial.print("MUDO"); else Serial.print(r, 1);
  Serial.print(" base "); Serial.print(rumboBase, 1);
  if (ahora - t_ultimoPaquete > 1000) {
    Serial.print(" | camara: NO LLEGA NADA");
  } else {
    Serial.print(" | pel ");  Serial.print(Xp);  Serial.print(","); Serial.print(Yp);
    Serial.print(" am ");     Serial.print(Xam); Serial.print(","); Serial.print(Yam);
    Serial.print(" az ");     Serial.print(Xaz); Serial.print(","); Serial.print(Yaz);
  }
  Serial.print(" | lin ");
  Serial.print(analogRead(LINEA_ADELANTE));  Serial.print("/");
  Serial.print(analogRead(LINEA_ATRAS_IZQ)); Serial.print("/");
  Serial.println(analogRead(LINEA_ATRAS_DER));
}

// El arranque del giroscopio (del partido, con los reintentos del 01/09 y
// la espera de datos del 08/09): contestar "estoy" no es dar un rumbo.
void iniciarGiroscopio() {
  int intentos = 0;
  while (!hayGiroscopo && intentos < 10) {
    intentos++;
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) {
      delay(50);
    }
  }
  if (cable()) {
    Serial.print(" giroscopio: ");
    if (!hayGiroscopo)               Serial.println("NO CONTESTA (¿bateria apagada?)");
    else if (rumboActual() < 0)      Serial.println("contesta pero todavia no da rumbo");
    else { Serial.print("OK, rumbo "); Serial.println(rumboActual(), 1); }
  }
}

void reiniciarRecorrido() {
  parar();
  if (!hayGiroscopo) iniciarGiroscopio();
  for (int p = 0; p < 6; p++) { msPaso[p] = 0; motivoFalla[p] = nullptr; }
  primerPasoFallado = 0;
  pasoActual = 0;
  colorArco = NINGUNO;
  colorPrimerArco = NINGUNO;
  idaAlArea = 1;
  colorDelOtro = NINGUNO;
  vioElOtroArco = false;
  hizoElGiro180 = false;
  despejo = false;
  rumboBase = -1;
  rumboInicial = -1;
  ultimaDistPelotaCm = 999;
  contando = false;
  entrarA(CUENTA);
  if (cable()) {
    Serial.print(" arranca en "); Serial.print(MS_CUENTA / 1000);
    Serial.println(" s: apoyalo MIRANDO A UN ARCO y saca las manos.");
  }
}

void leerTeclas() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r' || c == ' ') continue;
    switch (c) {
      case '0':
        entrarA(DETENIDO);
        decir("   PARADO. r = empezar de nuevo");
        break;
      case 'r': reiniciarRecorrido(); break;
      case 'i': informeFinal(); break;
      case '?': ayuda(); break;
      case 'e':
        mostrarEstado = !mostrarEstado;
        decir(mostrarEstado ? "   linea de estado: SI" : "   linea de estado: NO");
        break;
      default:
        if (cable()) {
          Serial.print("   tecla '"); Serial.print(c);
          Serial.println("' no hace nada. ? para la ayuda.");
        }
        break;
    }
  }
}


// ================================================================ LED

// N destellos de "on" ms (separados por "off") y una pausa, repitiendo,
// contando desde t0. Asi el primer destello sale justo al empezar.
bool destellos(unsigned long ahora, unsigned long t0, int n,
               unsigned long on, unsigned long off, unsigned long pausaMs) {
  if (n < 1) n = 1;
  unsigned long grupo = (unsigned long)n * (on + off);
  unsigned long t = (ahora - t0) % (grupo + pausaMs);
  if (t >= grupo) return false;
  return (t % (on + off)) < on;
}

void mostrarLed(unsigned long ahora) {
  bool prendido = false;
  switch (fase) {
    case CUENTA: {
      // Igual que ARMANDOSE del partido: lento, y los ultimos 3 s rapido.
      unsigned long pasado = ahora - t_fase;
      unsigned long falta = (pasado >= MS_CUENTA) ? 0 : MS_CUENTA - pasado;
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      prendido = (ahora / periodo) % 2;
      break;
    }
    case AVISO_FALLA:
      prendido = destellos(ahora, t_fase, pasoDelAviso, LARGO_ON, LARGO_OFF, LARGO_PAUSA);
      break;
    case AVISO_OK:
      prendido = true;
      break;
    case FIN:
      if (primerPasoFallado == 0) prendido = (ahora % 2000) < 120;   // latido lento
      else prendido = destellos(ahora, t_fase, primerPasoFallado, LARGO_ON, LARGO_OFF, LARGO_PAUSA);
      break;
    case DETENIDO:
    case SIN_GIRO:
      prendido = false;      // en SIN_GIRO lo pisa el temblor de abajo
      break;
    default:
      prendido = destellos(ahora, t_empezoPaso, pasoActual, CORTO_ON, CORTO_OFF, CORTO_PAUSA);
      break;
  }
  digitalWrite(LED, prendido ? HIGH : LOW);

  // (del partido) AVISO "ESTOY SIN GIROSCOPIO": va al final y PISA lo de
  // arriba. Es el parpadeo mas rapido del programa (ningun otro usa 50 ms).
  if (ahora - t_chequeoGiro >= 200) {
    t_chequeoGiro = ahora;
    rumboActual();                     // actualiza giroscopoRespondiendo
  }
  bool sinRumbo = (!hayGiroscopo) || (!giroscopoRespondiendo)
               || (fase != CUENTA && fase != DETENIDO && rumboBase < 0);
  if (sinRumbo) {
    digitalWrite(PIN_AVISO, ((ahora / MS_PARPADEO_AVISO) % 2) ? HIGH : LOW);
  }
}


// ================================================================ programa

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);
  parar();

  Serial.begin(BAUDIOS);         // el cable (para MIRAR)
  Serial1.begin(BAUDIOS);        // la camara

  // Si hay cable, se espera un poquito a que la compu abra el puerto, asi
  // el cartel de arranque se ve. Sin cable son 2 s mas de espera, que el
  // giroscopio igual necesita para despertarse.
  while (!Serial && millis() < 2000) delay(10);

  if (cable()) {
    Serial.println();
    Serial.println("=========================================================");
    Serial.println(" TECNICA CHALLENGE (2026-11-08) — 🚨 EL ROBOT SE MUEVE");
    Serial.println(" lateral -> medio -> un arco -> vuelta -> el otro arco");
    Serial.println(" -> la pelota -> despeje. ? = ayuda, 0 = PARAR");
    Serial.println("=========================================================");
  }

  iniciarGiroscopio();
  reiniciarRecorrido();
}


void loop() {
  leerTeclas();
  leerCamara();
  unsigned long ahora = millis();

  // Tope de TODO lo de la pelota (pasos 4 y 5, con todas sus idas y
  // vueltas). El despeje ya lanzado se deja terminar.
  bool conLaPelota = (fase == P4_MIRAR_PELOTA || fase == P4_BUSCAR_PELOTA
                      || fase == P5_A_LA_PELOTA
                      || ((fase == GIRANDO || fase == GIRO_ASENTAR) && pasoDelGiro == 4));
  if (conLaPelota && ahora - t_empezoPelota >= MS_TOPE_PELOTA_TOTAL) {
    avisarFalla(pasoActual, "se acabo el tiempo total para la pelota", FIN);
  }

  switch (fase) {

    // ---------------- 0. la cuenta ----------------
    case CUENTA: {
      parar();
      if (ahora - t_fase < MS_CUENTA) break;
      // 🚨 NO ARRANCAR SIN RUMBO (como el partido, pero mas estricto: sin
      // rumbo no se puede ir al medio por tiempo ni dar la vuelta).
      float r = rumboActual();
      if (r < 0) {
        if (ahora - t_fase < MS_CUENTA + MS_ESPERA_EXTRA_GIRO) break;
        decir("!! SIN RUMBO: no me muevo. ¿Bateria prendida ANTES del USB? (r = reintentar)");
        entrarA(SIN_GIRO);
        break;
      }
      rumboInicial = r;
      rumboBase = r;
      reiniciarCorreccion();
      if (cable()) {
        Serial.print(" rumbo de referencia (mirando al primer arco): ");
        Serial.println(r, 1);
      }
      idaAlArea = 1;
      // Si lo apoyaron sobre una linea, primero se despega: si no, el
      // paso del lateral terminaria al instante (como el partido).
      if (algunoDeAtrasVeBlanco()) entrarA(P1_DESPEGARSE);
      else                         entrarA(P1_AL_LATERAL);
      break;
    }

    case SIN_GIRO:
    case DETENIDO:
      if (ahora - t_fase < msFreno) frenar(); else parar();
      break;

    // ---------------- las piezas que usan todos los pasos ----------------

    case PAUSA:
      if (ahora - t_fase < msFreno) { frenar(); break; }
      parar();
      if (ahora - t_fase >= msFreno + MS_QUIETO_ENTRE_PASOS) entrarA(faseDespuesDePausa);
      break;

    case AVISO_FALLA:
      if (ahora - t_fase < msFreno) frenar(); else parar();
      if (ahora - t_fase >= duracionAvisoFalla(pasoDelAviso)) entrarA(faseDespuesDelAviso);
      break;

    case AVISO_OK:
      if (ahora - t_fase < msFreno) frenar(); else parar();
      if (ahora - t_fase >= MS_AVISO_OK) entrarA(faseDespuesDelAviso);
      break;

    // Girar en el lugar hasta rumboObjetivo. Es ACOMODANDO del partido,
    // con el objetivo que se le pida en vez de rumboBase.
    case GIRANDO: {
      float r = rumboActual();
      if (r < 0) {
        // Sin dato no se cambia nada. Si sigue mudo un rato, se para todo:
        // sin giroscopio no hay vuelta, ni centrado, ni rumbo.
        if (t_mudoDesde == 0) t_mudoDesde = ahora;
        if (ahora - t_mudoDesde >= MS_GIRO_MUDO_MAX) {
          avisarFalla(pasoDelGiro, "el giroscopio dejo de contestar en pleno giro", FIN);
        }
        break;
      }
      t_mudoDesde = 0;
      float error = diferencia(rumboObjetivo, r);
      if (fabs(error) <= TOLERANCIA_ACOMODO || ahora - t_fase >= msTopeGiro) {
        // FRENAR, no soltar: llego y hay que clavarlo ahi (del partido).
        frenar();
        fase = GIRO_ASENTAR; t_fase = ahora;
        break;
      }
      int pwm = (int)(fabs(error) * KP_ACOMODO);
      if (pwm > PWM_MAX_ACOMODO) pwm = PWM_MAX_ACOMODO;
      if (pwm < PWM_MIN_ACOMODO) pwm = PWM_MIN_ACOMODO;
      // arranque suave: las tres de golpe dejan mudo al giroscopio
      if (pwm < pwmGiroAplicado) pwmGiroAplicado = pwm;
      else if (ahora - t_rampaGiro >= RAMPA_MS) {
        t_rampaGiro = ahora;
        pwmGiroAplicado += RAMPA_PASO;
        if (pwmGiroAplicado > pwm) pwmGiroAplicado = pwm;
      }
      // error > 0 -> el rumbo tiene que SUBIR -> aplicar(+,+,+) (probado
      // en cancha con el enderezado del partido).
      int g = (error > 0) ? pwmGiroAplicado : -pwmGiroAplicado;
      aplicar(g, g, g);
      break;
    }

    case GIRO_ASENTAR: {
      // Esperar que pase la inercia ANTES de volver a medir, con el freno
      // sostenido los primeros msFreno (del partido, ACOMODO_ASENTAR).
      if (ahora - t_fase < msFreno) { frenar(); break; }
      parar();
      if (ahora - t_fase < MS_ASENTAR_ACOMODO) break;
      float r = rumboActual();
      float error = (r < 0) ? 999 : diferencia(rumboObjetivo, r);
      if (fabs(error) > TOLERANCIA_ACOMODO && reintentosGiro < MAX_REINTENTOS_ACOMODO) {
        reintentosGiro++;
        pwmGiroAplicado = 0; t_rampaGiro = ahora;
        fase = GIRANDO; t_fase = ahora;     // sin entrarA: no borra los reintentos
        break;
      }
      errorFinalGiro = error;
      if (cable()) {
        Serial.print("   giro terminado: ");
        if (r < 0) Serial.print("giroscopio MUDO");
        else { Serial.print("quedo a "); Serial.print(error, 1); Serial.print(" grados"); }
        Serial.print(" (reintentos "); Serial.print(reintentosGiro); Serial.println(")");
      }
      if (pasoDelGiro == 3) {
        hizoElGiro180 = true;
        errorDelGiro180 = error;
        if (fabs(error) > GRADOS_GIRO_TORCIDO) {
          decir("   ⚠️ la vuelta quedo torcida: puede no ver el otro arco");
        }
      }
      entrarA(faseDespuesDelGiro);
      break;
    }

    // ---------------- 1. al lateral y al medio ----------------

    // Si arranco parado sobre una linea: irse para el mismo lado hasta
    // dejar de ver blanco, y un rato mas (del partido, MEDIO_DESPEGARSE).
    case P1_DESPEGARSE:
      moverDeCostado(LADO_LATERAL * VEL_BUSCAR_LINEA);
      if (algunoDeAtrasVeBlanco()) {
        t_soltoLinea = ahora;                  // sigue encima del blanco
      } else if (ahora - t_soltoLinea >= MS_DESPEGARSE) {
        pausa(P1_AL_LATERAL);
        break;
      }
      if (ahora - t_fase >= MS_TOPE_LATERAL) {
        prepararGiro(rumboInicial, MS_TIMEOUT_ACOMODO, P1_AL_MEDIO, 1);
        avisarFalla(1, "arranque sobre una linea y no me despegue: hago de cuenta que estoy contra el lateral",
                    GIRANDO);
      }
      break;

    // (del partido, MEDIO_AL_LATERAL) termina con CUALQUIERA de los dos
    // de atras, como el partido. Yendo a la izquierda el lateral lo pisa
    // primero el izquierdo, pero si ese se lo pierde, el otro lo agarra.
    case P1_AL_LATERAL:
      moverDeCostado(LADO_LATERAL * VEL_BUSCAR_LINEA);
      if (algunoDeAtrasVeBlanco()) {
        if (cable()) {
          Serial.print("   vi el lateral a los "); Serial.print(ahora - t_fase);
          Serial.println(" ms: freno y me enderezo");
        }
        prepararGiro(rumboInicial, MS_TIMEOUT_ACOMODO, P1_AL_MEDIO, 1);
        pausa(GIRANDO);
        break;
      }
      if (ahora - t_fase >= MS_TOPE_LATERAL) {
        prepararGiro(rumboInicial, MS_TIMEOUT_ACOMODO, P1_AL_MEDIO, 1);
        avisarFalla(1, "no vi el lateral en 6 s: sigo como si estuviera contra el", GIRANDO);
      }
      break;

    // (del partido, MEDIO_AL_MEDIO) 🚨 a VEL_AL_MEDIO (100), la fuerza con
    // la que se midieron los ms. El tiempo cuenta desde que termino de
    // enderezarse, quieto contra el lateral.
    case P1_AL_MEDIO: {
      unsigned long msAlMedio = (LADO_LATERAL < 0) ? MS_MEDIO_DESDE_IZQ : MS_MEDIO_DESDE_DER;
      moverDeCostado(-LADO_LATERAL * VEL_AL_MEDIO);
      if (ahora - t_fase >= msAlMedio) {
        decir("   llegue al medio (por tiempo)");
        colorArco = NINGUNO;
        reiniciarCuentaDeBusqueda();
        pausa(P2_MIRAR_ARCO);
      }
      break;
    }

    // ---------------- 2. el primer arco ----------------

    case P2_MIRAR_ARCO:
      parar();
      if (ahora - t_fase >= MS_MIRAR_ARCO) {
        contando = false;
        decidirArco();
      }
      break;

    case P2_BUSCAR_ARCO:
      girarBuscando(SENTIDO_GIRO_BUSCAR);
      contarGiro();
      if (seguidosAmarillo >= CUADROS_PARA_FRENAR_ARCO
          || seguidosAzul >= CUADROS_PARA_FRENAR_ARCO) {
        // Como el delantero: al verlo FRENA (girando lleva envion y se
        // pasaria de largo), y despues mira quieto.
        decir("   vi un arco girando: freno y miro quieto");
        pausa(P2_MIRAR_ARCO);
        break;
      }
      if (giradoBusqueda >= GRADOS_MAX_BUSCAR_ARCO
          || ahora - t_empezoBusqueda >= MS_TOPE_BUSCAR_ARCO) {
        // No hay arco a la vista. Lo pusieron mirando a uno: vuelve a
        // mirar para ahi y va derecho, sin corregir con la camara.
        colorArco = NINGUNO;
        rumboBase = rumboInicial;
        prepararGiro(rumboInicial, MS_TIMEOUT_ACOMODO, P2_AL_AREA, 2);
        avisarFalla(2, "di la vuelta y no vi ningun arco: miro para donde me pusieron y voy derecho",
                    GIRANDO);
      }
      break;

    // Derecho hacia el arco (sostiene el rumbo) y corriendose de costado
    // para tenerlo enfrente, hasta que el sensor de ADELANTE ve blanco.
    case P2_AL_AREA: {
      int lateral = 0;
      if (colorArco != NINGUNO && hayArcoSuave && veoArco(colorArco)
          && fabs(arcoSuave) > ZONA_MUERTA_ARCO) {
        lateral = (int)(arcoSuave * KP_ARCO_LATERAL);   // + = arco a la derecha -> a la derecha
        if (lateral >  MAX_LATERAL_ARCO) lateral =  MAX_LATERAL_ARCO;
        if (lateral < -MAX_LATERAL_ARCO) lateral = -MAX_LATERAL_ARCO;
      }
      avanzarPersiguiendo(VEL_HACIA_ARCO, lateral);

      // 2026-11-08: la PRIMERA vez (idaAlArea 1) despues de la vuelta mira el
      // otro arco; la SEGUNDA (idaAlArea 2, hacia el otro arco) da la segunda
      // vuelta y recien ahi va a buscar la pelota.
      bool segunda = (idaAlArea == 2);
      if (blancoAdelante()) {
        frenar();
        if (cable()) {
          Serial.print("   TOQUE LA LINEA DEL AREA del arco "); Serial.print(nombreColor(colorArco));
          Serial.print(" a los "); Serial.print(ahora - t_fase); Serial.println(" ms");
        }
        if (segunda) {
          prepararSegundaVuelta();
        } else {
          prepararGiro(rumboBase + 180.0, MS_TOPE_GIRO_180, P3_MIRAR_OTRO, 3);
        }
        pausa(GIRANDO);
        break;
      }
      if (ahora - t_fase >= (segunda ? MS_TOPE_AL_OTRO_AREA : MS_TOPE_AL_AREA)) {
        if (segunda) {
          prepararSegundaVuelta();
          avisarFalla(3, "yendo al otro arco, el sensor de adelante no vio la linea del area a tiempo: doy la vuelta igual",
                      GIRANDO);
        } else {
          prepararGiro(rumboBase + 180.0, MS_TOPE_GIRO_180, P3_MIRAR_OTRO, 3);
          avisarFalla(2, "el sensor de adelante no vio la linea del area a tiempo: doy la vuelta igual",
                      GIRANDO);
        }
      }
      break;
    }

    // ---------------- 3. la vuelta (es un GIRANDO) y el otro arco ----------------

    case P3_MIRAR_OTRO:
      parar();
      if (ahora - t_fase >= MS_MIRAR_OTRO_ARCO) {
        contando = false;
        decidirOtroArco();
      }
      break;

    // ---------------- 4. la pelota ----------------

    case P4_MIRAR_PELOTA:
      parar();
      if (ahora - t_fase >= MS_MIRAR_PELOTA) {
        contando = false;
        decidirPelota();
      }
      break;

    // Girar siempre para el mismo lado hasta verla (la idea del delantero).
    case P4_BUSCAR_PELOTA:
      girarBuscando(SENTIDO_GIRO_BUSCAR);
      contarGiro();
      if (seguidosPelota >= CUADROS_PARA_CREER_PELOTA) {
        // El freno de 200 ms al verla es el MS_FRENO_AL_VER del delantero
        // (y el msFreno del arquero). FRENAR, no soltar: el 29/09 el
        // delantero midio que frenado la ve el 98% de las veces, y suelto
        // el 45%. "La camara no veia mal: el robot se movia mientras miraba."
        decir("   vi la pelota girando: freno y miro quieto");
        reiniciarCentradoPelota();
        pausa(P4_MIRAR_PELOTA);
        break;
      }
      if (giradoBusqueda >= GRADOS_MAX_BUSCAR_PELOTA
          || ahora - t_empezoBusqueda >= MS_TOPE_BUSCAR_PELOTA) {
        avisarFalla(4, "di dos vueltas y no vi la pelota: me quedo quieto", FIN);
        break;
      }
      // 2026-11-08: DE A PASITOS (ver buscarDeAPasitos). Ya giro un pasito:
      // frena y mira QUIETO. Si no la ve, decidirPelota() vuelve aca y gira
      // otro pasito (la cuenta de grados y el tope siguen sumando).
      if (buscarDeAPasitos && giradoBusqueda - giradoAlEmpezarPaso >= PASO_BUSCAR_PELOTA) {
        decir("   gire un pasito: freno y miro quieto");
        pausa(P4_MIRAR_PELOTA);
      }
      break;

    // ---------------- 5. ir a la pelota y despejar ----------------

    case P5_A_LA_PELOTA: {
      if (algunSensorPisaBlanco()) {
        frenar();
        avisarFalla(5, "pise una linea blanca yendo a la pelota: freno", FIN);
        break;
      }
      if (hayQueDespejar()) {
        if (cable()) {
          Serial.print("   DESPEJO: pelota a "); Serial.print(Xp / CAMARA_POR_CM, 0);
          Serial.println(" cm y de frente");
        }
        entrarA(P5_DESPEJE);
        break;
      }
      bool laVeo = (ahora - t_ultimaPelota) < MS_GRACIA_PELOTA;
      if (!laVeo) {
        // "Perderla de vista pegada no es perderla, es tenerla" (README
        // del delantero): con la pelota encima la camara manda X = 0.
        if (ultimaDistPelotaCm <= umbralCm + MARGEN_PEGADA_CM) {
          decir("   la perdi de vista PEGADA: la tengo encima, despejo");
          entrarA(P5_DESPEJE);
        } else {
          decir("   la perdi de vista: a buscarla de nuevo");
          reiniciarCuentaDeBusqueda();
          pausa(P4_BUSCAR_PELOTA);
        }
        break;
      }
      // Si se fue muy al costado, correrse no alcanza: frena y la vuelve a
      // centrar girando. Solo CERCA: con la X recortada el angulo sale
      // exagerado (ver X_PELOTA_RECORTADA) y frenaria a cada rato sin
      // avanzar. De lejos la corrige solo la correccion de costado de abajo.
      float ang = (Xp > 0 && Xp < X_PELOTA_RECORTADA) ? anguloPelota(Xp, Yp) : 0;
      if (fabs(ang) > TOL_ANG_RECENTRAR) { if (t_torcidaDesde == 0) t_torcidaDesde = ahora; }
      else t_torcidaDesde = 0;
      if (t_torcidaDesde != 0 && ahora - t_torcidaDesde >= MS_TORCIDA_PARA_RECENTRAR) {
        decir("   se me fue al costado: freno y la vuelvo a centrar girando");
        reiniciarCentradoPelota();
        pausa(P4_MIRAR_PELOTA);
        break;
      }
      // Como la ida del despeje del partido: avanzar + correrse hacia ella.
      int lateral = 0;
      if (Xp > 0) {
        float d = desvioPelota();
        if (fabs(d) > ZONA_MUERTA_PELOTA) {
          lateral = (int)(d * kpPersecucion);
          if (lateral >  PWM_MAX_PERSECUCION) lateral =  PWM_MAX_PERSECUCION;
          if (lateral < -PWM_MAX_PERSECUCION) lateral = -PWM_MAX_PERSECUCION;
        }
      }
      avanzarPersiguiendo(VEL_HACIA_PELOTA, lateral);
      if (ahora - t_fase >= MS_TOPE_IR_A_PELOTA) {
        avisarFalla(5, "no llegue a la pelota a tiempo: me paro", FIN);
      }
      break;
    }

    // La ida del despeje del partido: 533 ms a 200, derecha (o
    // corrigiendo, si perseguirEnElDespeje). La rampa NO arranca de 0:
    // sigue subiendo desde la fuerza con la que venia.
    case P5_DESPEJE: {
      if (algunSensorPisaBlanco()) {
        frenar();
        avisarFalla(5, "pise una linea blanca en el despeje: freno", FIN);
        break;
      }
      int lateral = 0;
      if (perseguirEnElDespeje && Xp > 0 && ahora - t_ultimaPelota < MS_GRACIA_PELOTA) {
        float d = desvioPelota();
        if (fabs(d) > ZONA_MUERTA_PELOTA) {
          lateral = (int)(d * kpPersecucion);
          if (lateral >  PWM_MAX_PERSECUCION) lateral =  PWM_MAX_PERSECUCION;
          if (lateral < -PWM_MAX_PERSECUCION) lateral = -PWM_MAX_PERSECUCION;
        }
      }
      if (lateral == 0) adelanteControlado(potenciaDespeje);
      else              avanzarPersiguiendo(potenciaDespeje, lateral);
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        despejo = true;
        decir("   despeje hecho: freno y me quedo QUIETO");
        pausa(FIN);       // frena, suelta, y queda quieto
      }
      break;
    }

    case FIN:
      parar();
      break;
  }

  mostrarEstadoPorCable(ahora);
  mostrarLed(ahora);
}
