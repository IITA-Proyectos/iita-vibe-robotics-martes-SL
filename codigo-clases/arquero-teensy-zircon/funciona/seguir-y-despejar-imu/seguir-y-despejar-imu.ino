/* =====================================================================
   ARQUERO en MODO IMU — igual al de siempre, pero SIN BRUJULA
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-22
   =====================================================================

   🚨 ESTA ES UNA COPIA. El original sigue intacto en
      `funciona/seguir-y-despejar/` y no se toco ni una linea.

   QUE CAMBIA RESPECTO DEL ORIGINAL — SOLO EL GIROSCOPIO
   El original arranca el BNO055 con `bno.begin()`, que por defecto usa el
   modo NDOF: acelerometro + giroscopio + MAGNETOMETRO. O sea, con brujula.
   Esta version usa IMUPLUS, que no usa la brujula.

   POR QUE
   La brujula se deja torcer por cualquier metal, iman o cable con
   corriente — y el robot tiene TRES MOTORES justo abajo del sensor, juega
   adentro de una cancha con marco de metal y enfrente tiene otros robots.
   Ademas Adafruit documenta que en NDOF el rumbo puede SALTAR DE GOLPE
   cuando el magnetometro termina de calibrarse. Ese salto es el principal
   sospechoso de los 80 grados del 2026-09-01.

   QUE SE PIERDE
   En IMUPLUS el rumbo es RELATIVO: arranca en 0 donde este el robot al
   prenderse y mide cuanto giro desde ahi. No apunta al norte, y se va
   corriendo despacio con el tiempo.

   Al programa el cero arbitrario no le molesta: nunca uso el rumbo
   absoluto. Guarda `rumboBase` al armarse y despues sostiene ESE.

   LA DERIVA SI MOLESTA, Y HUBO QUE AGREGAR DOS COSAS
   En el original, `rumboBase` se fijaba UNA sola vez al armarse y nunca
   mas se tocaba. Con brujula eso estaba bien, porque el rumbo absoluto no
   envejece. Sin brujula es un problema serio, y no pasivo: el programa
   CORRIGE contra `rumboBase`, asi que la deriva del sensor se convierte en
   giro de verdad. A un grado y medio por minuto, a los seis minutos el
   robot -- parado y quieto en su arco -- empieza a girar solo porque el
   numero se corrio, y termina mirando al lateral. Visto de afuera es
   igualito al bug de los 80 grados que este modo venia a arreglar.

   Por eso esta version agrega:

     1. ESPERAR LA CALIBRACION DEL GIROSCOPO ANTES DE FIJAR EL CERO.
        El BNO055 mide su propio error de reposo quedandose quieto unos
        segundos, y recien ahi lo descuenta. Sin eso la deriva no es de un
        grado por minuto sino de hasta UNO POR SEGUNDO. Es la diferencia
        entre andar y no andar.

     2. VOLVER A TOMAR EL CERO EN LA LINEA BLANCA.
        Cuando el robot vuelve del despeje y LOS DOS sensores de atras ven
        blanco en la misma vuelta, esta cuadrado con la linea del arco por
        construccion. Ese es un dato fisico de la cancha, que no deriva.
        Ahi se vuelve a fijar `rumboBase`. Asi la deriva nunca acumula mas
        que lo que pase entre dos despejes: segundos, no minutos.

        ⚠️ Con UN solo sensor no alcanza: con uno el robot esta en diagonal
        y volveria a fijar el cero torcido. Por eso se piden los dos, y si
        entra uno solo se deja `rumboBase` como estaba.

   Un error que crece despacio y se borra en cada despeje es mucho mejor
   que un salto de 80 grados que llega sin aviso.

   =====================================================================
   ARQUERO — sigue la pelota de costado y la despeja
   =====================================================================

   QUE HACE
   El robot se para en su area mirando la cancha, con el arco a la espalda,
   y NUNCA gira: siempre mira al frente.

        1. Sin pelota a la vista: quieto.
        2. Ve la pelota lejos  -> se corre DE COSTADO para ponerse enfrente
                                  de ella, sin dejar de mirar al frente.
        3. La pelota llega a 30 cm -> DESPEJA: sale ~50 cm, la saca, vuelve
                                  hasta pisar la linea, se endereza, y da
                                  un empujoncito de 10 cm.

   ⚠️ Sale 50 cm para una pelota que estaba a 30. Queda mas lejos del arco
   de lo necesario. Achicar la ida es el proximo ajuste pendiente, y es una
   decision de estrategia de arquero, no una cuenta.

   ⚠️ EL SEGUIMIENTO DE COSTADO NO TIENE LIMITE (cambio del 2026-08-18).
   Lo unico que lo detiene es perder de vista la pelota. Antes se frenaba al
   pisar la linea del costado del area, pero el robot vuelve chueco de
   algunos despejes y estando torcido pisaba la linea antes de tiempo.

   La linea SIGUE usandose para volver del despeje. Son dos usos distintos
   de los mismos dos sensores de atras, y solo se saco el primero.

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
bool lateralInvertido = false;
//
// Camara: con la pelota puesta a la DERECHA del robot, la camara mandaba
// Yp = -24. O sea que el negativo es la derecha, al reves de lo que
// suponia el codigo. Por eso va invertido.
bool camaraYInvertida = true;


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
const float CAMARA_POR_CM = 2.87;    // unidades de camara por cm real

// ---- seguimiento lateral ----
// La proporcion 50/50/89 entre las ruedas sale de la geometria del robot;
// es la que usa el codigo 2025 en ai/adproporcional. Lo que cambia es la
// FUERZA con que se aplica, que aca es proporcional a lo desviada que esta
// la pelota en vez de ser un numero fijo.
const int LADO_FRENTE  = 50;     // las dos de adelante
const int LADO_TRASERA = 89;     // la de atras

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
float kpLateral   = 11.5;        // PWM por cm REAL de desvio de la pelota
int   pwmMinLateral = 60;        // abajo de esto no se mueve, solo zumba
int   pwmMaxLateral = 120;
// 4.0 de camara / 2,87 = 1,4 cm reales. Mismo comportamiento que antes.
const float ZONA_MUERTA_PELOTA = 1.4;   // cm REALES: no perseguir migajas

// ---- despeje ----
int potenciaDespeje   = 200;
int potenciaRetroceso = 110;
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
int msAdelante        = 533;     // ~50 cm
int msAdelanteChico   = 133;     // 10 cm
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
int umbralBlanco      = 425;     // medido en cancha el 2026-09-15
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
float umbralCm        = 30.0;    // cm REALES: despeja si esta a esto o menos

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
float umbralDesvio    = 7.0;     // cm REALES de desvio tolerado


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
bool perseguirEnElDespeje = false;

// PWM de costado por cada cm REAL que la pelota esta desviada.
// 6.0 -> con la pelota 5 cm al costado empuja 30; con 10 cm, 60 (el tope).
// ⚠️ PUESTO A OJO, primera version. Se ajusta mirando al robot.
float kpPersecucion = 6.0;

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
const int PWM_MAX_PERSECUCION = 60;

const unsigned long MS_PAUSA_MEDIO   = 150;
const unsigned long MS_MAX_RETROCESO = 1200;

// Cuanto se sostiene el freno electrico. Despues se sueltan los motores:
// mantenerlos cortocircuitados sin necesidad solo calienta el driver.
unsigned long msFreno = 200;


// El empujoncito de 10 cm al final, para que el robot termine donde arranco
// y no pegado a la linea. Se saco un rato el 2026-08-18 mientras se probaba
// el freno, y se volvio a poner el mismo dia una vez que el freno funciono.
bool empujonFinal = true;

// Va DESPACIO, no a la potencia del despeje: son 10 cm y el robot ya esta
// bien parado sobre la linea. Yendo rapido patina y arruina justo lo que
// acaba de lograr.
int potenciaEmpujon = 100;

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
int msEmpujonFinal = 320;
const unsigned long MS_ENFRIAMIENTO  = 1500;
// 2026-09-21: bajado de 10 s a 2 s a pedido del equipo. Con 2 s toda la
// cuenta queda en parpadeo rapido (el lento era para los primeros 7 s).
const unsigned long MS_AVISO_ARMADO  = 2000;
const int VECES_PARA_CREERLE = 3;


// ---- giroscopio: mantenerlo derecho ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool enderezarActivado = true;

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
const unsigned long MS_PARPADEO_AVISO = 50;   // 50 encendido + 50 apagado

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
const unsigned long MS_ESPERA_DATOS_GIRO  = 5000;   // en setup()
const unsigned long MS_ESPERA_EXTRA_GIRO  = 2000;   // antes de armarse (era 15 s, bajado el 2026-09-21)

float rumboBase = -1;            // el rumbo que hay que sostener siempre

// Cuanto se espera, al armarse, a que el giroscopio se calibre solo estando
// quieto. Sin brujula esto es lo que separa una deriva de un grado por
// minuto de una de un grado por segundo.
const unsigned long MS_ESPERA_CALIBRACION = 6000;

// Cuantas veces la linea blanca corrigio el cero. No se usa para decidir
// nada: es para saber, al mirar la memoria, si el mecanismo llego a actuar.
int vecesQueSeReancloRumbo = 0;

// Correccion continua de rumbo (proporcional + acumulada). Los valores
// vienen de lo medido el 2026-08-04 en las rectas del cuadrado: con solo
// proporcional quedaba un error fijo de 7 grados; con el termino acumulado
// bajo a menos de 2.
const float KP_RUMBO = 5.0;
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
// 2026-09-01: subido de 70 a 100. Con 70, durante el despeje (potencia 200)
// la correccion era demasiado floja para evitar que el robot se torciera.
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Enderezado al terminar el despeje
const float TOLERANCIA_ACOMODO = 3.0;
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
const float KP_ACOMODO = 1.5;
// 2026-09-01, segunda pasada: con MAX=110 el robot SI giraba, pero giraba
// DEMASIADO RAPIDO y se pasaba: oscilaba varias veces antes de quedarse
// quieto, y a veces terminaba mal acomodado. A pedido del equipo el techo
// baja a la mitad del cambio (110 -> 85): menos velocidad, menos inercia.
//
// ⚠️ EL MINIMO NO SE PUEDE BAJAR. No es una perilla de ajuste fino: es lo
// que hace que el robot se mueva ALGO. Debajo de ~70 las ruedas zumban y
// no giran, que era exactamente el bug de antes.
const int   PWM_MIN_ACOMODO = 75;    // era 35: debajo del piso de arranque
const int   PWM_MAX_ACOMODO = 85;    // era 60, despues 110
const unsigned long MS_ASENTAR_ACOMODO = 300;
const unsigned long MS_TIMEOUT_ACOMODO = 3000;
const int MAX_REINTENTOS_ACOMODO = 3;

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
const float TOLERANCIA_ESPERA = 8.0;          // grados para decidir enderezarse
const unsigned long MS_CHEQUEO_ESPERA = 300;  // no preguntarle al sensor a cada rato
unsigned long t_chequeoEspera = 0;
// Descanso obligatorio entre intentos de enderezarse esperando. Mientras
// descansa, el robot vuelve a mirar la pelota. Sin esto puede quedarse
// reintentando una maniobra que no converge y no atajar nunca.
const unsigned long MS_DESCANSO_ENDEREZAR = 6000;
unsigned long t_ultimoIntentoEnderezar = 0;
bool enderezandoEnEspera = false;
const int RAMPA_PASO = 3;
const unsigned long RAMPA_MS = 15;
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;
int reintentosAcomodo = 0;


// ---- UBICACION INICIAL ----
//
// Al armarse, el robot retrocede hasta pisar la linea del area y se
// endereza. Nada mas.
//
// 2026-09-21: SE SACO EL CENTRADO CON EL ARCO DEL RIVAL, a pedido del
// equipo: "lo centramos nosotros". El robot se apoya a mano en el centro
// del arco antes de prenderlo.
bool  ubicandose      = false;


// ---- estado ----
enum Fase { APAGADO, ARMANDOSE, ESPERANDO, SIGUIENDO,
            ADELANTE, PAUSA_MEDIO, ATRAS_HASTA_LINEA, FRENANDO,
            ACOMODANDO, ACOMODO_ASENTAR, ADELANTE_CHICO, ENFRIANDO,
            UBIC_ATRAS, UBIC_FRENANDO };
Fase fase = APAGADO;
unsigned long t_fase = 0;

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
bool  predecirTrayectoria = true;
float velocidadLateral    = 0;      // cm reales por segundo, + = a la derecha
float velocidadAcercamiento = 0;    // cm/s, + = se viene encima del robot
const float SUAVIZADO_VELOCIDAD = 0.3;


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
bool despejeEnDiagonal = false;

// MEDIDO el 2026-08-04 con regla: a potencia 200 el robot hace 1 cm cada
// 10 ms. O sea 100 cm por segundo.
const float VEL_ROBOT_CM_S = 100.0;

// 200 de PWM dan 100 cm/s. O sea 2 de PWM por cada cm/s.
// NO es un ajuste: es la medicion del 04/08 escrita en otra unidad.
const float PWM_POR_CM_S = 2.0;

// Debajo de esta velocidad se considera que la pelota esta QUIETA y no se
// arma diagonal por velocidad. El ruido medido con la pelota parada llega
// a 3,6 cm/s, asi que 4 esta apenas arriba.
const float PISO_VELOCIDAD = 4.0;

// ⚠️ EL UNICO NUMERO DE FE QUE QUEDA.
// Con el MISMO PWM, ¿el robot se corre de costado tan rapido como avanza?
// No: rinde menos. Avanzando, las dos ruedas de adelante tiran enteras.
// De costado, con la proporcion 50/50/89, parte del empuje de cada rueda
// se va para donde no sirve. De la geometria sale ~0,80 (o sea, el 80%).
// Esperar que la cancha pida bajarlo a 0,60-0,70.
//
// Lo bueno de que sea UN SOLO multiplicador: tambien tapa el otro agujero
// conocido (que al eje Y de la camara se le aplica el mismo factor 2,87
// que al X sin haberlo medido). Si ese estuviera errado, los dos terminos
// de la cuenta se escalan igual, y esta misma perilla los corrige a los
// dos. Una perilla, dos agujeros — aunque nunca sepamos cual era.
float rendimientoCostado = 0.80;

// 🚨 EL REPARTO DEL MOTOR. Una rueda no pasa de 255. La mas cargada
// recibe el avance ENTERO mas la MITAD del costado. Se le reservan 45
// para el giroscopio (con KP_RUMBO = 5,0 eso son 9 grados de error
// corregibles sin recortar nada), y quedan 210 para el movimiento.
const int RESERVA_RUMBO      = 45;
const int PRESUPUESTO_RUEDA  = 210;   // 255 - RESERVA_RUMBO

// Debajo de este avance ya no es un despeje: es un paseo.
const int PWM_MIN_AVANCE_DESPEJE = 140;
const unsigned long MS_MAX_ADELANTE = 800;   // tope duro de la ida

// Debajo de este costado el rozamiento se lo come: no vale la pena pagar
// avance por una diagonal que no se va a cumplir. Sale derecho.
const int COSTADO_MINIMO_UTIL = 20;

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
float msAnticipacion = 250;

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
const int RAMPA_MOV_PASO = 10;
const unsigned long RAMPA_MOV_MS = 10;
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
// 🚨 LA TRAMPA DEL MODO IMU, Y POR QUE ESTA FUNCION CAMBIA
//
// Todo el programa se da cuenta de que el giroscopio se cayo asi: la
// libreria Adafruit devuelve 0.0 en los tres angulos cuando no puede leer
// el chip, y tres ceros exactos al mismo tiempo no son una postura real.
//
// En NDOF eso es cierto. En IMUPLUS NO: el rumbo es relativo y arranca
// EXACTAMENTE en 0.0. Si ademas el robot esta bien apoyado y plano, los
// otros dos tambien pueden dar 0.0. O sea que justo al arrancar, un
// giroscopio PERFECTO se ve igual que uno muerto — y el programa se
// quedaria esperando un dato que ya tiene.
//
// La salida es preguntarle al chip. El registro 0x39 (SYS_STATUS) dice en
// que anda: 5 = "fusionando, todo bien". Si contesta 5, los tres ceros son
// una lectura legitima y no una caida. Esa pregunta extra se hace SOLO en
// el caso raro de los tres ceros, asi que no cuesta nada en el juego
// normal.
bool chipSigueFusionando() {
  Wire.beginTransmission(0x28);
  Wire.write(0x39);                      // SYS_STATUS
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(0x28, 1) != 1)    return false;
  return Wire.read() == 5;               // 5 = fusionando
}

float rumboActual() {
  if (!hayGiroscopo) { giroscopoRespondiendo = false; return -1; }
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    if (!chipSigueFusionando()) {
      giroscopoRespondiendo = false;     // se cayo de verdad
      return -1;
    }
    // Contesta y esta fusionando: el cero es de verdad. Se devuelve un
    // pelin mas que cero porque el programa usa el -1 para "no hay dato" y
    // compara con >= 0; 0.0 exacto pasa igual, pero esto deja claro que es
    // un rumbo valido y no un valor sin inicializar.
    giroscopoRespondiendo = true;
    return 0.01;
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


// ------------------------------------------------------- moverse de costado

// s > 0 pide ir a la DERECHA, s < 0 a la izquierda. El valor es la fuerza.
// Suma el movimiento lateral con la correccion de rumbo: son dos cosas
// independientes y por eso se pueden calcular por separado.
//
// La fuerza pasa por la rampa; la correccion de rumbo NO. La correccion
// tiene que actuar enseguida, y ademas es chica: no hace patinar nada.
void moverDeCostado(int s) {
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
// El costado NO pasa por la rampa, por el mismo motivo que la correccion
// de rumbo: tiene que actuar en el acto, y es chico al lado del avance.
void avanzarPersiguiendo(int potencia, int lateral) {
  if (lateralInvertido) lateral = -lateral;

  int p  = rampa(potencia);
  int c  = correccionDeRumbo();
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
      } else {
        // Sin pelota a la vista, la velocidad vieja no vale nada: cuando
        // reaparezca puede estar en cualquier lado.
        hayDesvioAnterior = false;
        velocidadLateral  = 0;
        velocidadAcercamiento = 0;
      }

      // Para decidir el despeje se usa DONDE VA A ESTAR, no donde esta.
      float dLatPredicho = predecirTrayectoria
                         ? dLat + velocidadLateral * (msAnticipacion / 1000.0)
                         : dLat;
      float desvCm = fabs(dLatPredicho);
      if (Xp > 0 && dCm <= umbralCm && desvCm <= umbralDesvio) {
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
  return desvioPelota() + velocidadLateral * (msAnticipacion / 1000.0);
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

  float d = distanciaPelota();
  if (d < 1.0) d = 1.0;                    // no dividir por cero
  float y  = desvioPelota();               // + = a la derecha
  float vy = velocidadLateral;
  if (fabs(vy) < PISO_VELOCIDAD) vy = 0;   // eso es ruido, no movimiento

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
  if (avance < PWM_MIN_AVANCE_DESPEJE) return false;   // no llego: no salgo

  float costado = (k * avance + m) / r;
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
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  fase = UBIC_ATRAS; t_fase = millis();
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
  // Si lo que acaba de terminar era la UBICACION INICIAL y no un despeje,
  // no corresponde el empujoncito de 10 cm: el robot ya esta donde tiene
  // que estar. Se reusa toda la maquinaria de enderezarse, solo cambia
  // adonde va despues.
  if (ubicandose) {
    ubicandose = false;
    pasarAEsperar();
    return;
  }
  if (empujonFinal) {
    fase = ADELANTE_CHICO; t_fase = millis();
  } else {
    fase = ENFRIANDO; t_fase = millis();
  }
}



// ---------------------------------------------------------------- programa

void setup() {
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
  int intentos = 0;
  while (!hayGiroscopo && intentos < 10) {
    intentos++;
    // 🎯 LA UNICA LINEA QUE CAMBIA DE VERDAD: el modo.
    // El original llama a bno.begin() sin argumento, y el que la libreria
    // pone por defecto es NDOF (con brujula).
    hayGiroscopo = bno.begin(OPERATION_MODE_IMUPLUS);
    if (!hayGiroscopo) delay(300);
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

        // 🚨 SIN BRUJULA, ESPERAR LA CALIBRACION NO ES OPCIONAL.
        //
        // El BNO055 arranca sin saber cuanto se equivoca cuando esta
        // quieto. Lo aprende quedandose quieto unos segundos: ahi mide ese
        // error de reposo y lo descuenta. Hasta que no lo hace, el rumbo
        // se corre solo a razon de hasta UN GRADO POR SEGUNDO.
        //
        // En NDOF esto no importaba porque la brujula lo ataba al norte y
        // no lo dejaba irse. En IMUPLUS no hay nada que lo ate: si se fija
        // el cero antes de que calibre, a los dos minutos el robot esta
        // mirando para cualquier lado.
        //
        // El chip se puntua a si mismo de 0 a 3. Se espera a que el
        // giroscopo llegue a 3, con tope: si no llega, se juega igual
        // (mejor torcido que quieto) pero el LED sigue avisando.
        if (hayGiroscopo) {
          unsigned long tc = millis();
          while (millis() - tc < MS_ESPERA_CALIBRACION) {
            uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
            bno.getCalibration(&sis, &gir, &ace, &mag);
            if (gir >= 3) break;
            delay(100);
          }
          r = rumboActual();          // releer: paso tiempo desde la de arriba
        }

        // El rumbo de referencia se fija ACA, con el robot ya quieto y
        // apuntando a la cancha. Es el que va a sostener siempre... salvo
        // que la linea blanca lo corrija (ver ATRAS_HASTA_LINEA).
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
        frenar();
        fase = UBIC_FRENANDO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        frenar();
        fase = UBIC_FRENANDO; t_fase = ahora;
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
        parar(); fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;
    }

    case PAUSA_MEDIO:
      parar();
      if (ahora - t_fase >= MS_PAUSA_MEDIO) {
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

      // 🎯 EL UNICO PUNTO DE LA CANCHA QUE NO DERIVA.
      //
      // Si los DOS sensores de atras ven blanco en la misma vuelta, el
      // robot esta apoyado contra la linea del arco y por lo tanto
      // CUADRADO con ella. Eso es una referencia fisica de verdad, no un
      // numero que se va corriendo. Aca se vuelve a fijar el cero.
      //
      // Con uno solo NO se hace: con uno el robot esta en diagonal (por eso
      // frenar usa "alguno" pero re-anclar exige "los dos"), y fijar el
      // cero ahi seria guardar un rumbo torcido para el resto del partido.
      if (veBlancoIzq() && veBlancoDer()) {
        float rl = rumboActual();
        if (rl >= 0) {
          rumboBase = rl;
          reiniciarCorreccion();
          vecesQueSeReancloRumbo++;
        }
      }

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
        fase = ENFRIANDO; t_fase = ahora;
      }
      break;

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
