/* =====================================================================
   DELANTERO - PARTIDO CON SAQUE - Roboliga 2026 - IITA Salta
   =====================================================================
   Robot 2 (ROBOT2 del codigo 2025) - Teensy 4.1 + Zircon - serie 15708680
   Ruedas (medido en banco): 8/7/6 IZQUIERDA - 11/12/4 DERECHA - 2/5/3 TRASERA

   QUE HACE:  BUSCANDO -> CENTRANDO -> AVANZANDO -> ORBITANDO -> PATEA_ADEL
              -> PATEA_ATRAS -> BUSCANDO. La LINEA BLANCA interrumpe todo.

   ESTE ES:   una COPIA del programa de partido (funciona/delantero del
              09/11, commit fc557f2) con un SAQUE al arrancar. El de partido
              NO se toco: lo que se cambie aca NO pasa alla (y al reves).
   SAQUE:     al arrancar, AL AZAR (SAQUE_AL_AZAR): 20% CENTRO, 40% IZQUIERDA,
              40% DERECHA. [09/11] Es A CIEGAS: no mira la camara.
              [10/11] ANTES mira si ve la pelota (DECIDIR_SACA_O_DEFIENDE):
              si la VE, esta lejos porque saca el rival -> DEFIENDE: patea
              derecho al frente (como el CENTRO). Si NO la ve, esta pegada
              (la camara no la alcanza) -> SACA, con el sorteo.
              CENTRO: patea derecho apenas arranca. IZQUIERDA/DERECHA: rodea
              la pelota (la orbita del partido, sin camara: solo el giroscopio
              dice cuando llego) hasta quedar GRADOS_SAQUE torcido, y patea.
              Despues juega normal.
              PONERLO MIRANDO AL ARCO RIVAL con la pelota adelante, a la
              distancia a la que la ORBITA (si queda lejos o cerca, la orbita
              la deja de costado). El rumbo cero (y el del saque) sale de
              hacia donde mira al prenderse.
              Todo lo del saque esta marcado [saque].
   CARGAR:    PARTIDO-CON-SAQUE.bat (1 azul / 2 amarillo) y leer el cartel del
              robot (monitor a 19200). El SUCCESS del cargador no prueba nada.
   REGLAS:    un cambio por vez, y anotar en la bitacora que se vio.

   La historia de cada numero esta en bitacora/ y en
   respaldos/delantero-2026-10-27-con-comentarios-largos.ino
   ===================================================================== */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// =====================================================================
//   LO ESENCIAL: lo que se ajusta en clase y en la cancha
//   ms = milisegundos - cm = centimetros de la camara - fuerza 0..255
//   (desde QUIETO una rueda necesita ~70 para arrancar; YA RODANDO ~40)
// =====================================================================

// ---- [saque] SAQUE AL ARRANCAR (lo nuevo de esta copia, 09/11) ----
const bool SAQUE_AL_ARRANCAR           = true;             // false = juega igual que el de partido, sin saque
const float GRADOS_SAQUE               = 60.0;             // [09/11: era 45] cuanto se tuerce del arco hacia la pared para patear (grados). Cuenta del rebote desde el centro: pared a ~91 cm, arco a ~110 -> ~59. Si rebota antes del arco: subir. Si se pasa: bajar
const bool DECIDIR_SACA_O_DEFIENDE     = true;             // [10/11] al arrancar mira si ve la pelota: si la VE, esta lejos (saca el rival) -> DEFIENDE: patea derecho al frente. Si NO la ve, esta pegada -> SACA (el sorteo de abajo). false = siempre saca
const unsigned long MS_MIRAR_PELOTA    = 300;              // [10/11] cuanto mira (ms), con la camara ya andando, para decidir si saca o defiende
const bool SAQUE_AL_AZAR               = true;             // [09/11] cada vez que se prende SORTEA para donde va el saque (los % de abajo). false = siempre para el lado de SAQUE_A_LA_DERECHA
const int  PROB_SAQUE_CENTRO           = 20;               // [09/11] % de las veces que patea DERECHO al arco (sin torcerse)
const int  PROB_SAQUE_IZQUIERDA        = 40;               // [09/11] % de las veces a la IZQUIERDA. A la DERECHA va el resto: 100 - 20 - 40 = 40
const bool SAQUE_A_LA_DERECHA          = true;             // solo con SAQUE_AL_AZAR = false: hacia que pared. [medido 15/09: girando a la DERECHA el rumbo SUBE]. Si se tuerce para el otro lado: cambiar
const float TOL_SAQUE                  = 8.0;              // grados: "ya mira hacia la pared"
const unsigned long MS_SAQUE_MAX       = 8000;             // si en este tiempo no pateo el saque, se olvida del saque y juega normal (ms)
const int VEL_SAQUE                    = 255;              // [09/11: era 215] fuerza del golpe del SAQUE (probado en el TABLERO). Si se vuelca: bajar de a 10 o bajar RAMPA_SAQUE_PASO
const int MS_SAQUE                     = 900;              // [09/11: era 500] cuanto dura el golpe del saque (ms). Mas = la pelota sale mas fuerte, y el robot llega mas cerca de la linea
const int RAMPA_SAQUE_PASO             = 15;               // [09/11: era 8] el golpe del saque sube de a esto cada RAMPA_PATADA_MS. 15 = como en el TABLERO, donde se probaron estos numeros. Si se vuelca: 8
// [09/11] numeros PROPIOS del saque (probados en el TABLERO). La patada normal usa los suyos, mas abajo
const int TRIM_SAQUE                   = 10;               // compensacion fija del saque. Si se va siempre a la DERECHA: subir. A la IZQUIERDA: bajar
const float KP_SAQUE                   = 2.5;              // corrige por GRADO torcido, en el saque
const float KD_SAQUE                   = 0.0;              // corrige por GRADO/SEGUNDO, en el saque (0 = no usa)
const int VEL_RETROCESO_SAQUE          = 155;              // despues del saque, marcha atras con esta fuerza...
const int MS_RETROCESO_SAQUE           = 300;              // ...durante esto (ms), y juega normal
const int MS_FRENO_SAQUE               = 200;              // [10/11] al llegar a los grados del saque FRENA esto (ms) y recien ahi patea: sin el envion de la orbita, el golpe no se desvia. 0 = sin freno

// ---- BUSCAR, CENTRAR Y AVANZAR ----
const int VEL_GIRO_CONTINUO            = 42;                // [09/11: era 35, +20% pedido por el equipo, SOLO en esta copia] fuerza del giro buscando. Si zumba y no gira: subir a 45 o 50
const int VEL_CENT                     = 94;                // [09/11: era 78, +20% pedido por el equipo, SOLO en esta copia] fuerza de cada pulsito. Tambien al recentrarla orbitando y al apuntar al cero
const int VEL_AVANCE                   = 114;               // [09/11: era 95, +20% pedido por el equipo, SOLO en esta copia] fuerza yendo hacia la pelota cuando esta LEJOS
const int VEL_AVANCE_CERCA             = 84;                // [09/11: era 70, +20% pedido por el equipo, SOLO en esta copia] fuerza al llegar. Si la empuja al llegar: bajar de a 5, pero debajo de ~70 puede quedarse plantado
const int XP_FRENAR                    = 70;               // desde estos cm empieza a aflojar, de VEL_AVANCE a VEL_AVANCE_CERCA

// ---- RODEAR LA PELOTA (ORBITA) ----
const int XP_ORBITA                    = 32;               // a estos cm deja de avanzar y empieza a rodearla. Si la choca al empezar: subir
const int XP_SUELTA                    = 55;               // si se aleja mas que esto (cm) vuelve a avanzar. Siempre MAYOR que XP_ORBITA
const int VEL_ORB_IMPULSO              = 120;              // golpe de la rueda trasera para arrancar la orbita
const int VEL_ORB_TRASERA              = 80;                // [09/11: era 67, +20% pedido por el equipo, SOLO en esta copia] fuerza de la vuelta, y al rearrancar despues de recentrar (ahi no hay golpe). Mas = mas rapida. Si se planta: subir
const unsigned long MS_ORBITA_MAX      = 10000;            // cuanto orbita buscando el arco antes de pasar al plan B (ms)

// ---- PATEAR ----
const int MS_FRENO_PATADA              = 200;              // [10/11] antes de la patada normal (despues de rodear) FRENA esto (ms): sin el envion de la orbita no se desvia. Mas = mas derecha, pero le da tiempo al rival. 0 = sin freno
const int VEL_PATADA                   = 190;                // [10/11: era 235, pedido por el equipo, SOLO en esta copia] fuerza del golpe. Mas = llega mas lejos, pero se levanta y vuelca mas facil
const int MS_PATADA                    = 500;                // [10/11: era 525, pedido por el equipo, SOLO en esta copia] cuanto dura el golpe (ms). Mas = llega mas lejos y se pasa mas cerca de la linea
const int TRIM_PATADA                  = 6;                  // [10/11: era 9, pedido por el equipo, SOLO en esta copia] compensacion fija. Si se va siempre a la DERECHA: subir. A la IZQUIERDA: bajar
const float KP_PATADA                  = 5.0;                // [10/11: era 6.5, pedido por el equipo, SOLO en esta copia] corrige por GRADO torcido. Mas = corrige mas fuerte (y puede zigzaguear)
const float KD_PATADA                  = 0.3;               // [09/11: era 0.6, pedido por el equipo, SOLO en esta copia] corrige por GRADO/SEGUNDO: frena el giro antes de que crezca (baja el pico)
const int VEL_RETROCESO                = 100;               // [09/11: era 90, pedido por el equipo, SOLO en esta copia] despues del golpe, marcha atras con esta fuerza...
const int MS_RETROCESO                 = 400;               // [09/11: era 700, pedido por el equipo, SOLO en esta copia] ...durante esto (ms), y vuelve a buscar

// ---- PUNTERIA (con el arco lejos, que es casi siempre) ----
const float TOL_ANG_LEJOS_PELOTA       = 12.0;             // grados, pelota adelante, con el arco lejos. Tambien para patear al rumbo cero
const float TOL_ANG_LEJOS_ARCO         = 12.0;             // [09/11: era 8; se probo 15 y quedo en 12, el equipo: tardaba mucho en alinear] grados, punteria con el arco lejos (arco y pelota separados menos que esto -> patea)

// ---- RUMBO CERO (plan B: el cero es hacia donde mira AL PRENDER la bateria) ----
const float VUELTAS_PARA_RUMBO0        = 1.0;              // vueltas alrededor de la pelota sin patear antes del plan B

// ---- LINEA BLANCA (UMBRAL_LINEA se MIDE en cada cancha con grabar-linea: 1 DER, 2 IZQ, 3 DEL) ----
int UMBRAL_LINEA[3]                    = { 300, 300, 440 }; // [09/11: DER e IZQ bajados a 300 por el equipo; eran 424/434 del 20/10] desde que lectura cada sensor dice "esto es blanco". En el verde tienen que leer MENOS que esto
const unsigned long MS_RETROCESO_LINEA = 210;              // al verla, retrocede esto (ms)...
const int VEL_ESCAPE_FUERTE            = 155;               // [09/11: era 135, pedido por el equipo, SOLO en esta copia] ...con esta fuerza. Mas = se despega mas rapido, pero se levanta
const unsigned long MS_CIEGO_LINEA     = 300;              // despues de verla no la mira durante esto (ms). NUNCA menos que MS_RETROCESO_LINEA

// =====================================================================
//   OTROS AJUSTES: casi nunca se tocan
// =====================================================================

// ---- INTERRUPTORES (true = prendido) ----
const bool BUSCA_CONTINUO              = true;             // buscar la pelota girando continuo (true) o a pulsitos (false)
const bool LINEA_ACTIVA                = true;             // escapar de la linea blanca. En false puede salirse de la cancha
const bool PROTECCION_ARRANQUE         = false;            // true = si al prenderse ya ve blanco, APAGA el escape de linea todo el partido
const bool USAR_GIROSCOPO              = true;             // usar el giroscopio (patada derecha, rumbo cero, lado de la orbita)
const bool GIRO_SIN_BRUJULA            = true;             // giroscopio sin brujula (modo IMU, desde el 12/10). false = el modo viejo, con brujula
const bool PATEAR_AL_RUMBO0            = true;             // plan B: si orbitando no logra patear al arco (a las VUELTAS o a MS_ORBITA_MAX), patea al rumbo cero. Necesita giroscopio
const bool RUMBO0_ORBITANDO            = true;             // plan B orbitando (true) o girando en el lugar (false). En false NO cuenta vueltas NI usa VETO_ARCO_PROPIO
const bool VETO_ARCO_PROPIO            = true;             // no patear al rumbo cero si la camara ve NUESTRO arco. Solo sirve con RUMBO0_ORBITANDO = true
const bool ORBITA_CAMINO_CORTO         = true;             // elegir para que lado orbitar (true) o ir siempre para el mismo (false)
const bool TOLERANCIA_ADAPTATIVA       = true;             // con el arco lejos, apuntar con las tolerancias de "lejos" (seccion 7)
const bool ELEGIR_ARCO_AL_ENCENDER     = true;             // true: al prenderse mira y elige el arco (si no ve ninguno: AZUL). false: siempre AZUL. Con CARGAR-ROBOT.bat se ignora

// ---- BUSCAR Y CENTRAR ----
const int RAMPA_GIRO_PASO              = 10;               // el giro arranca suave: sube esto de fuerza...
const unsigned long RAMPA_GIRO_MS      = 10;               // ...cada esta cantidad de ms
const unsigned long MS_FRENO_AL_VER    = 200;              // al verla frena esto (ms), para no pasarse de largo
const int VEL_GIRO                     = 95;               // fuerza buscando A PULSOS (solo si BUSCA_CONTINUO = false)
const int MS_PULSO_BUSC                = 60;               // a pulsos: cuanto dura cada empujon (ms)
const int MS_ESPERA_BUSC               = 200;              // a pulsos: pausa frenada entre empujones (ms). Menos = barre mas rapido
const int MS_PULSO_CENT                = 32;               // cuanto dura cada pulsito (ms)
const int MS_ESPERA_CENT               = 320;              // pausa frenada entre pulsitos (ms): es cuando la camara mira
const float TOL_ANG_ENTRA              = 12.0;             // grados: mas torcida que esto -> se pone a centrar
const float TOL_ANG_SALE               = 6.0;              // grados: mas derecha que esto -> avanza. Siempre MENOR que TOL_ANG_ENTRA

// ---- DESTRABE ----
const unsigned long MS_LIMITE_CENTRAR  = 3000;             // si pasa esto (ms) viendola sin acercarse...
const int MEJORA_DESTRABE              = 10;               // ...("acercarse" = recortar al menos estos cm)...
const unsigned long MS_AVANCE_DESTRABE = 500;              // ...avanza a lo bruto esto (ms)...
const int VEL_DESTRABE                 = 150;              // ...con esta fuerza, y vuelve a buscar
const int MAX_DESTRABES                = 5;                // destrabes por tanda. Gastados, espera otro MS_LIMITE_CENTRAR y vuelve a 0: NO es un tope y NO suelta la pelota. 0 = sin destrabe
const unsigned long MS_PERDIDA_REAL    = 1000;             // solo para el destrabe: sin verla esto (ms, mas 300 de MS_GRACIA) su reloj y su cupo vuelven a 0. No cambia cuando vuelve a buscar

// ---- ORBITA ----
const int MS_ORB_IMPULSO               = 300;              // cuanto dura ese golpe (ms), SOLO al entrar a orbitar. Si no arranca: subir esto. Si se planta despues: VEL_ORB_TRASERA
const int VEL_ORB_FRENTE               = 30;               // las dos de adelante, DEBAJO del piso a proposito para que no giren. Nunca cerca de 70
const unsigned long MS_AGUANTE_ORBITA  = 1000;             // si deja de verla orbitando, frena y la espera esto (ms) antes de volver a buscar
const float TOL_ANG_ORB_ENTRA          = 18.0;             // grados: si la pelota se corre mas que esto, deja de rodear y la recentra
const float TOL_ANG_ORB_SALE           = 9.0;              // grados: ya mas derecha que esto, sigue rodeando. Siempre MENOR que ENTRA

// ---- PUNTERIA CON EL ARCO CERCA ----
const float TOL_ANG_PELOTA             = 8.0;              // grados: la pelota a menos de esto del frente. Solo con el arco CERCA (menos de ARCO_LEJOS_MIN); lejos manda TOL_ANG_LEJOS_PELOTA
const float TOL_ANG_ALINEADO           = 10.0;             // [09/11: era 8; se probo 15 y 12, quedo en 10] grados: punteria SOLO con el arco CERCA (menos de ARCO_LEJOS_MIN). Lejos manda TOL_ANG_LEJOS_ARCO
const int ARCO_LEJOS_MIN               = 50;               // con el arco entre MIN y MAX (cm) se usan las dos de "lejos" de abajo...
const int ARCO_LEJOS_MAX               = 200;              // ...(200 es el tope de la camara: o sea "de MIN para arriba")

// ---- PATADA (finos) ----
const int RAMPA_PATADA_PASO            = 15;               // el golpe arranca suave: sube esto de fuerza...
const unsigned long RAMPA_PATADA_MS    = 5;                // ...cada esta cantidad de ms (para que no patinen las ruedas)
const int RESTA_MAX                    = 120;              // tope de la correccion, en fuerza

// ---- RUMBO CERO (finos) ----
const unsigned long MS_RUMBO0_MAX      = 15000;            // si en este tiempo (ms) no queda mirando al cero, vuelve a buscar
const float TOL_RUMBO                  = 12.0;             // grados: "ya estoy mirando al cero"
const unsigned long MS_APUNTAR_MAX     = 6000;             // solo con RUMBO0_ORBITANDO = false: cuanto gira en el lugar buscando el cero (ms)

// ---- LINEA (finos) ----
const unsigned long MS_LINEA_CONFIRMA  = 5;                // tiene que verla seguida esto (ms) para creerle

// ---- ARRANQUE ----
const unsigned long MS_ESPERA_ARRANQUE = 0;                // [09/11, era 2000] espera EXTRA al prenderlo (ms). Ya no hace falta: entre el USB y el giroscopio tarda ~3 s en llegar aca, la mano ya no esta
const unsigned long MS_ESPERA_CAMARA_MAX = 7000;           // [09/11] arranca recien cuando la camara manda datos (si no, saldria girando a ciegas). Tope contado desde que se prendio (ms)
const unsigned long MS_DETECTAR_USB    = 500;              // [09/11] minimo (ms desde que se prendio) para ver si esta enchufado a la compu
const unsigned long MS_DESPERTAR_MAX   = 1500;             // [09/11] tope (ms desde que se prendio) esperando que el giroscopio conteste. Se despierta a los ~650 ms
const unsigned long MS_FUSION_MAX      = 1500;             // [09/11] tope (ms) esperando que arranque la fusion del giroscopio. Antes eran 700 + 200 fijos
const unsigned long MS_MIRAR_ARCOS     = 2000;             // sin arco fijo: cuanto mira al prenderse para elegir el arco (ms)...
const int MUESTRAS_MINIMAS_ARCO        = 3;                // ...y cuantas veces tiene que ver un arco para creerle

// ---- PARA QUE LADO GIRA: tocar SOLO si gira para el lado equivocado ----
const bool GIRO_INVERTIDO              = true;             // sentido al buscar, centrar y recentrar orbitando. Cambiar SOLO si al centrar se aleja de la pelota
const bool ORBITA_INVERTIDA            = true;             // lado de la orbita: SIEMPRE si ORBITA_CAMINO_CORTO = false; si no, solo sin ver el arco ni giroscopio
const bool SENTIDO_ORBITA_INVERTIDO    = false;            // [01/09] cambiar SOLO si para quedar detras de la pelota da la vuelta larga (pasa entre pelota y arco). Irse al lado contrario al arco esta BIEN
const bool GIRO_RUMBO_INVERTIDO        = false;            // [sin verificar] si al apuntar al cero girando en el lugar se aleja del cero

// ---- CAMARA Y GIROSCOPIO ----
const int XP_MAX                       = 200;              // [03/11: era 150] mas lejos que esto (cm) no le cree a la PELOTA. 200 = lo maximo que manda la camara: le cree a todo
const int XARCO_MAX                    = 200;              // el techo para el ARCO. 200 = lo maximo que manda la camara: acepta todo
const bool FILTRO_BRILLO_PELOTA        = true;             // [03/11] un arco amarillo PEGADO a la pelota es su brillo: no creerle. false = como antes
const float BRILLO_ANG                 = 10.0;             // grados: "pegado" = en la misma direccion que la pelota (menos que esto)...
const float BRILLO_FACTOR              = 1.5;              // ...y a menos de esta cantidad de veces la distancia de la pelota
const unsigned long MS_GRACIA          = 300;              // sin ver la pelota (o el arco) menos que esto (ms), cuenta como que la ve
const unsigned long SIN_DATOS_MS       = 1500;             // sin datos de la camara este tiempo (ms), avisa por el monitor
const unsigned long MS_CHEQUEO_GIRO    = 500;              // cada cuanto pregunta si el giroscopio sigue vivo (ms). OJO: cada pregunta clava el programa 200 ms (bitacora 13/10)
const unsigned long MS_DERIVADA_PATADA = 10;               // cada cuanto calcula el KD de la patada (ms). Es el ritmo del sensor: no tocar

// OJO: no escribir ninguna funcion antes de 'enum Estado' (mas abajo): el
// preprocesador de Arduino pone los prototipos antes de la primera funcion y
// el build se rompe con "'Estado' was not declared".

// ===== PINES (medidos en banco el 28/07) =====
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

unsigned long t_disparoLinea = 0;   // cuando vio la linea por ultima vez

// Version de placa: pin 32 en LOW = Mark1 (linea en A11/A13/A12).
const int PIN_VERSION_PLACA = 32;

int Xp = 0, Yp = 0;
int Xam = 0, Yam = 0;              // arco AMARILLO (byte 202)
int Xaz = 0, Yaz = 0;              // arco AZUL     (byte 203)

int XpBueno = 0, YpBueno = 0;      // ultima posicion BUENA de la pelota
int XamBueno = 0, YamBueno = 0;
int XazBueno = 0, YazBueno = 0;

unsigned long t_ultimaPelota   = 0;
unsigned long t_ultimoAmarillo = 0;
unsigned long t_ultimoAzul     = 0;
unsigned long t_ultimoPaquete  = 0;
unsigned long t_ultimoAviso    = 0;
unsigned long t_cicloPulso     = 0;
unsigned long t_entroEstado    = 0;

// ARCO FIJO: lo decide platformio.ini (env azul / amarillo, CARGAR-ROBOT.bat).
// Sin ninguno de los dos (env teensy41) elige el arco mirando al prenderse.
#if defined(ATACAR_AZUL) && defined(ATACAR_AMARILLO)
  #error "ATACAR_AZUL y ATACAR_AMARILLO a la vez: elegi uno solo"
#endif
#if defined(ATACAR_AMARILLO)
  const bool ARCO_FIJO = true;
  bool objetivoEsAmarillo = true;
#elif defined(ATACAR_AZUL)
  const bool ARCO_FIJO = true;
  bool objetivoEsAmarillo = false;
#else
  const bool ARCO_FIJO = false;
  bool objetivoEsAmarillo = false;   // si al encender no ve ningun arco, ataca este
#endif

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiroscopo = false;
float rumboCero    = 0;    // hacia donde miraba al encenderse = hacia el arco rival
float ultimoRumbo  = 0;
bool          giroCaido = false;      // lo dice el propio chip, no lo adivinamos
float         rumboAlPatear = 0;      // rumbo al empezar la patada (heading-hold)

bool  saquePendiente  = false;  // [saque] true hasta que patea el saque (o se le acaba MS_SAQUE_MAX)
float rumboSaque      = 0;      // [saque] hacia donde patea = rumboCero +- GRADOS_SAQUE
unsigned long t_inicioSaque = 0;
bool  saqueVueltaDada = false;  // [saque] ya dio vuelta la orbita porque iba para el lado largo
float errSaqueInicial = 0;      // [saque] cuanto le faltaba al empezar a orbitar
bool  pateandoSaque   = false;  // [saque] el golpe en curso es el saque (fuerza VEL_SAQUE)
bool  retrocesoDeSaque = false; // [saque] la marcha atras en curso es la del saque (VEL_RETROCESO_SAQUE)
const char* saqueElegido = "-"; // [saque] lo que salio en el sorteo: CENTRO, IZQUIERDA o DERECHA

bool          sentidoOrbita = false;    // congelado al entrar a ORBITANDO
bool sentidoParaOrbitar();              // prototipo: cambiarA() lo usa
unsigned long t_chequeoGiro = 0;      // ultima vez que se le pregunto

int  pinLinea[3]  = { A11, A13, A12 };   // Mark1; se corrige en setup()
const char* versionPlaca = "?";
bool lineaHabilitada = false;
bool frenoFuerte = false;    // entramos al escape viniendo de la patada?

int lineaArranque[3] = { -1, -1, -1 };          // lo que leyo al encenderse
int lineaMin[3] = { 9999, 9999, 9999 };         // minimo visto en toda la corrida
int lineaMax[3] = { -1, -1, -1 };               // maximo visto en toda la corrida
int  mascaraLinea = 0;
unsigned long t_ultimaLinea = 0;
unsigned long t_sensorDesde[3] = { 0, 0, 0 };

// Los estados del robot.
enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO,
              APUNTA_RUMBO0, PATEA_ADEL, PATEA_ATRAS, ESCAPA_LINEA,
              DESTRABANDO, SAQUE_GIRA, FRENA_PATADA };   // SAQUE_GIRA: [saque] rodea la pelota a ciegas. FRENA_PATADA: [10/11] frena antes de patear
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

int  pwmGiroBusc = 0;              // PWM actual de la rampa del giro
unsigned long t_rampaGiro    = 0;
unsigned long t_frenoAlVer   = 0;  // 0 = no esta frenando el envion
unsigned long t_sinAcercarse = 0;  // 0 = el reloj del destrabe esta parado
unsigned long t_perdidaDesde = 0;  // desde cuando no ve la pelota (0 = la ve)
unsigned long t_orbSinVerla  = 0;  // idem pero DENTRO de la orbita (0 = la ve)
bool recentrandoOrbita = false;    // <- la histeresis de la banda de angulo
bool  giroOrbInvertido  = false;   // [09/11] aprendio en la cancha que "hacia el 0" iba al reves
bool  giroOrbAprendido  = false;
float errCeroOrbInicial = 0;       // [09/11] cuanto le faltaba para el 0 al empezar a orbitar...
float errCeroOrbMin     = 0;       // ...y lo mas cerca que estuvo
float giradoOrbita   = 0;          // grados de rumbo girados en esta orbita (con signo)
float rumboOrbPrevio = 0;          // ultima lectura del rumbo, orbitando
unsigned long t_muestraOrb  = 0;   // cuando se leyo el rumbo por ultima vez, orbitando
unsigned long t_buscaRumbo0 = 0;   // 0 = espera al arco; si no, desde cuando busca el rumbo cero
int  mejorXp    = 0;               // lo mas cerca que estuvo en este intento
int  nDestrabes = 0;

bool avisadoSinCamara = false;
unsigned long msArranque = 0;   // [09/11] a los cuantos ms de prenderlo arranco a jugar (sale en la telemetria)
unsigned long msDespierto = 0;  // [09/11] a los cuantos ms contesto el giroscopio
unsigned long msFusion = 0;     // [09/11] cuanto tardo en arrancar la fusion (ms)

// [09/11] distinto de cero cuando esta enchufado a una compu (lo pone el USB de la Teensy)
extern "C" volatile uint8_t usb_configuration;

unsigned long nPaquetes = 0, nTirados = 0, nLoops = 0;
unsigned long nBrillos = 0;   // arcos amarillos descartados por ser el brillo de la pelota (total)
unsigned long t_contadores = 0;

bool rumboPatadaValido = false;

#define PATADAS_GUARDADAS 5
float desvioPatada[PATADAS_GUARDADAS];   // grados al terminar el golpe
float picoPatada[PATADAS_GUARDADAS];     // lo peor durante el golpe
bool  conLazoPatada[PATADAS_GUARDADAS];  // tuvo giroscopo y rumbo bueno?
int   nPatadas    = 0;                   // cuantas pateo en toda la corrida
float picoEnCurso = 0;                   // pico de la patada que esta en vuelo

// ===== MOTORES =====
// parar() SUELTA las ruedas (sigue de largo). frenar() las TRABA: las mismas
// patas en 0 pero con el PWM a 255. Mismas patas, efecto opuesto.
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

void motoresRotando(bool sentidoA, int vel) {
  int a = sentidoA ? 1 : 0;
  int b = sentidoA ? 0 : 1;
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, a); digitalWrite(TRA_INB, b);
}

float         errPatadaPrevio = 0;
unsigned long t_derivPatada   = 0;
float         derivPatada     = 0;

void reiniciarDerivadaPatada() {
  errPatadaPrevio = 0;
  t_derivPatada   = 0;
  derivPatada     = 0;
}

// La patada: empujan las dos de adelante y la TRASERA va trabada (resiste
// que el robot gire, sin frenar el avance).
void motoresPatada(int vi, int vd) {
  if (vi < 0) vi = 0;   if (vi > 255) vi = 255;
  if (vd < 0) vd = 0;   if (vd > 255) vd = 255;
  analogWrite(IZQ_PWM, vi); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vd); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

// Golpe derecho: trim fijo + corrector PD del giroscopio, repartido entre las
// dos ruedas (a una le suma y a la otra le resta).
void avanzarDerecho(int vel, float rumboObjetivo) {
  int trim = pateandoSaque ? TRIM_SAQUE : TRIM_PATADA;   // [saque] numeros propios
  int vi = vel - trim;
  int vd = vel + trim;

  float err = diferencia(rumboObjetivo, rumboActual());
  unsigned long ahora = millis();
  if (t_derivPatada == 0) {
    errPatadaPrevio = err;
    t_derivPatada   = ahora;
  } else if (ahora - t_derivPatada >= MS_DERIVADA_PATADA) {
    derivPatada     = (err - errPatadaPrevio) * 1000.0 / (float)(ahora - t_derivPatada);
    errPatadaPrevio = err;
    t_derivPatada   = ahora;
  }
  float kp = pateandoSaque ? KP_SAQUE : KP_PATADA;        // [saque] numeros propios
  float kd = pateandoSaque ? KD_SAQUE : KD_PATADA;
  float mando = kp * err + kd * derivPatada;

  int mitad = (int)(fabs(mando) / 2.0);
  if (mitad > RESTA_MAX / 2) mitad = RESTA_MAX / 2;
  if (mando > 0) { vd -= mitad; vi += mitad; }
  else           { vi -= mitad; vd += mitad; }

  motoresPatada(vi, vd);
}

void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Rapido de lejos y suave al llegar (entre XP_FRENAR y XP_ORBITA).
int velAcercarse(int xp) {
  if (xp >= XP_FRENAR) return VEL_AVANCE;
  if (xp <= XP_ORBITA) return VEL_AVANCE_CERCA;
  long sobra = (long)(xp - XP_ORBITA) * (VEL_AVANCE - VEL_AVANCE_CERCA);
  return VEL_AVANCE_CERCA + (int)(sobra / (XP_FRENAR - XP_ORBITA));
}

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// ORBITA: las dos de adelante DEBAJO del piso (no giran) y empuja la trasera.
// Gira alrededor de un punto a R = 2*L adelante (L era 8,75 cm; sin medir
// desde que se achico el robot). La fuerza cambia la velocidad, no el radio.
void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;    // las dos de adelante
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE);  digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE);  digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera);      digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

// ===== LINEA BLANCA =====
// bit 0 = sensor 1 DERECHO, bit 1 = sensor 2 IZQUIERDO, bit 2 = sensor 3 DELANTERO.
int leerLineas() {
  int m = 0;
  for (int i = 0; i < 3; i++) {
    int v = analogRead(pinLinea[i]);
    if (v < lineaMin[i]) lineaMin[i] = v;      // para saber cuanto da el VERDE
    if (v > lineaMax[i]) lineaMax[i] = v;      // y cuanto da el BLANCO, en cancha
    if (v >= UMBRAL_LINEA[i]) m |= (1 << i);
  }
  return m;
}

// Escapa para el lado contrario al sensor: apaga la rueda de enfrente y las
// otras dos empujan al reves entre si. Sensor 3 (adelante) = derecho para atras.
void escaparDeLinea(int m, int velocidad) {
  int v[3] = {   0,      0,      0   };
  if (m & 1) { v[1] -= 1; v[2] += 1; }   // hacia la DI  -> IZQ apagada
  if (m & 2) { v[0] += 1; v[2] -= 1; }   // hacia la DD  -> DER apagada
  if (m & 4) { v[0] -= 1; v[1] += 1; }   // hacia la T   -> TRA apagada

  int pico = 0;
  for (int i = 0; i < 3; i++) if (abs(v[i]) > pico) pico = abs(v[i]);

  if (pico == 0) {
    frenar();
    return;
  }

  int pwm[3];
  for (int i = 0; i < 3; i++) pwm[i] = (v[i] * velocidad) / pico;

  analogWrite(IZQ_PWM, abs(pwm[0]));
  digitalWrite(IZQ_INA, pwm[0] > 0 ? 1 : 0);
  digitalWrite(IZQ_INB, pwm[0] < 0 ? 1 : 0);

  analogWrite(DER_PWM, abs(pwm[1]));
  digitalWrite(DER_INA, pwm[1] > 0 ? 1 : 0);
  digitalWrite(DER_INB, pwm[1] < 0 ? 1 : 0);

  analogWrite(TRA_PWM, abs(pwm[2]));
  digitalWrite(TRA_INA, pwm[2] > 0 ? 1 : 0);
  digitalWrite(TRA_INB, pwm[2] < 0 ? 1 : 0);
}

// Gira a pulsos: empuja y despues FRENA, para que la camara mire quieta.
void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso)                    motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera))  frenar();
  else                                                  t_cicloPulso = millis();
}

// Giro de busqueda continuo, con rampa para no patinar al arrancar.
void girarContinuo(bool sentidoA) {
  if (millis() - t_rampaGiro >= RAMPA_GIRO_MS) {
    t_rampaGiro = millis();
    if (pwmGiroBusc < VEL_GIRO_CONTINUO) {
      pwmGiroBusc += RAMPA_GIRO_PASO;
      if (pwmGiroBusc > VEL_GIRO_CONTINUO) pwmGiroBusc = VEL_GIRO_CONTINUO;
    }
  }
  motoresRotando(sentidoA, pwmGiroBusc);
}

// ===== CAMARA =====
// Paquete de 9 bytes: 201,Xp,Yp+100, 202,Xam,Yam+100, 203,Xaz,Yaz+100 (cm).
// X adelante, Y al costado (Y positivo = izquierda). 200 y +-100 = recortes.
// [03/11] Con mucha luz, el brillo de la pelota la camara lo ve amarillo y lo
// manda como ARCO, pegado a la pelota (se vio: pelota a 19 cm, "arco" a 23 cm
// en la misma direccion) y el robot pateaba. Por color no se separan (se midio:
// el L, A y B del brillo y del arco se pisan). Por LUGAR si: un arco de verdad
// nunca esta pegado a la pelota. Al robot la camara no le manda el tamano ni
// la forma de la mancha, solo donde esta: por eso se decide por la posicion.
bool esBrilloDeLaPelota(int xa, int ya) {
  if (!FILTRO_BRILLO_PELOTA) return false;
  if (!((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100))) return false;   // sin pelota en este paquete
  float dif = fabs(diferencia(anguloDe(xa, ya), anguloDe(Xp, Yp)));
  return (dif <= BRILLO_ANG) && (xa <= Xp * BRILLO_FACTOR);
}

void leerCamara() {
  while (Serial1.available() >= 9) {
    int h1 = Serial1.read();
    if (h1 != 201) { nTirados++; continue; }

    int xp  = Serial1.read();
    int yp  = Serial1.read();
    int h2  = Serial1.read();
    int xam = Serial1.read();
    int yam = Serial1.read();
    int h3  = Serial1.read();
    int xaz = Serial1.read();
    int yaz = Serial1.read();

    if (h2 == 202 && h3 == 203) {
      Xp  = xp;   Yp  = yp  - 100;
      Xam = xam;  Yam = yam - 100;    // antes se tiraba: ahora hace falta para elegir arco
      Xaz = xaz;  Yaz = yaz - 100;
      t_ultimoPaquete = millis();
      nPaquetes++;

      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xam > 0) && (Xam <= XARCO_MAX) && (abs(Yam) < 100)) {
        if (esBrilloDeLaPelota(Xam, Yam)) {
          nBrillos++;                       // es el brillo de la pelota: no es el arco
        } else {
          XamBueno = Xam; YamBueno = Yam;
          t_ultimoAmarillo = millis();
        }
      }
      if ((Xaz > 0) && (Xaz <= XARCO_MAX) && (abs(Yaz) < 100)) {
        XazBueno = Xaz; YazBueno = Yaz;
        t_ultimoAzul = millis();
      }
    }
  }
}

// ===== ANGULOS =====
// Se compara en grados y no en cm: un angulo no depende de la distancia.
float anguloDe(int X, int Y) {
  if (X <= 0) return 0.0;                    // sin dato, no invento un angulo
  return atan2((float)Y, (float)X) * 180.0 / PI;
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

int   arcoX()       { return objetivoEsAmarillo ? XamBueno : XazBueno; }
int   arcoY()       { return objetivoEsAmarillo ? YamBueno : YazBueno; }
unsigned long arcoT() { return objetivoEsAmarillo ? t_ultimoAmarillo : t_ultimoAzul; }
unsigned long arcoPropioT() { return objetivoEsAmarillo ? t_ultimoAzul : t_ultimoAmarillo; }
const char* arcoNombre() { return objetivoEsAmarillo ? "AMARILLO" : "AZUL"; }

// ===== GIROSCOPIO =====
// Cada MS_CHEQUEO_GIRO le pregunta al chip si la fusion sigue (SYS_STATUS 5).
// OJO: bno.getSystemStatus() trae un delay(200) adentro (bitacora 13/10).
float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  ultimoRumbo = evento.orientation.x;        // 0..360

  if (millis() - t_chequeoGiro > MS_CHEQUEO_GIRO) {
    t_chequeoGiro = millis();
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    giroCaido = (sys != 5);                  // 5 = algoritmo de fusion corriendo
  }
  return ultimoRumbo;
}

bool giroscopoSano() {
  return hayGiroscopo && !giroCaido;
}

const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO:    return "BUSCANDO";
    case CENTRANDO:   return "CENTRANDO";
    case AVANZANDO:   return "AVANZANDO";
    case ORBITANDO:   return "ORBITANDO";
    case APUNTA_RUMBO0: return "al rumbo 0";
    case PATEA_ADEL:  return "PATEANDO!";
    case PATEA_ATRAS: return "retrocede";
    case ESCAPA_LINEA: return "!LINEA!";
    case DESTRABANDO: return "destrabando";
    case SAQUE_GIRA:  return "SAQUE: rodeando a ciegas";
    case FRENA_PATADA: return "frena para patear";
  }
  return "?";
}

// [09/11] Lee un registro del BNO055 directo por I2C, sin las esperas de la libreria
// (getSystemStatus trae un delay(200) adentro). Devuelve -1 si el chip no contesta.
int leerRegistroBNO(uint8_t reg) {
  Wire.beginTransmission(0x28);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return -1;
  if (Wire.requestFrom(0x28, 1) != 1) return -1;
  return Wire.read();
}

bool bnoDespierto() { return leerRegistroBNO(0x00) == 0xA0; }   // registro 0x00 = 0xA0: "soy un BNO055"

bool arrancarGiroscopo() {
  if (!bno.begin(GIRO_SIN_BRUJULA ? OPERATION_MODE_IMUPLUS : OPERATION_MODE_NDOF)) return false;
  bno.setExtCrystalUse(true);
  // [09/11] setExtCrystalUse reinicia la fusion (bitacora 18/08 y 25/08). Antes se esperaba
  // 700 ms fijos y despues se preguntaba (200 ms mas adentro de la libreria). Ahora pregunta
  // cada 20 ms y sigue apenas la fusion corre: SYS_STATUS (registro 0x39) = 5, DOS veces
  // seguidas. Mismo control que antes, sin esperar de mas.
  uint8_t sys = 0;
  int seguidos = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < MS_FUSION_MAX) {
    int s = leerRegistroBNO(0x39);
    sys = (s < 0) ? 0 : (uint8_t)s;
    seguidos = (sys == 5) ? seguidos + 1 : 0;
    if (seguidos >= 2) break;
    delay(20);
  }
  msFusion = millis() - t0;
  if (sys != 5) {
    int err = leerRegistroBNO(0x3A);   // SYS_ERR
    Serial.print("   la fusion NO esta corriendo (SYS_STATUS="); Serial.print(sys);
    Serial.print(", SYS_ERR="); Serial.print(err);
    Serial.println("). Corre pruebas/giroscopo-crudo/ para ver que pasa.");
    return false;
  }
  Serial.print("(fusion corriendo, SYS_STATUS=5) ");
  giroCaido = false;
  t_chequeoGiro = millis();
  return true;
}

void elegirArcoMirando() {
  long   nAm = 0, nAz = 0;
  double sumAm = 0, sumAz = 0;
  unsigned long t0 = millis();

  Serial.print("Mirando "); Serial.print(MS_MIRAR_ARCOS / 1000);
  Serial.println(" s para ver a que arco apunto. NO LO MUEVAS.");

  unsigned long visto_am = 0, visto_az = 0;
  while (millis() - t0 < MS_MIRAR_ARCOS) {
    leerCamara();
    if (t_ultimoAmarillo != visto_am) {          // llego un dato NUEVO del amarillo
      visto_am = t_ultimoAmarillo;
      nAm++;  sumAm += fabs(anguloDe(XamBueno, YamBueno));
    }
    if (t_ultimoAzul != visto_az) {
      visto_az = t_ultimoAzul;
      nAz++;  sumAz += fabs(anguloDe(XazBueno, YazBueno));
    }
  }

  float medAm = nAm ? (float)(sumAm / nAm) : 999.0;
  float medAz = nAz ? (float)(sumAz / nAz) : 999.0;

  Serial.print("   amarillo: "); Serial.print(nAm); Serial.print(" muestras");
  if (nAm) { Serial.print(", a "); Serial.print(medAm, 1); Serial.print(" grados"); }
  Serial.println();
  Serial.print("   azul:     "); Serial.print(nAz); Serial.print(" muestras");
  if (nAz) { Serial.print(", a "); Serial.print(medAz, 1); Serial.print(" grados"); }
  Serial.println();

  bool sirveAm = (nAm >= MUESTRAS_MINIMAS_ARCO);
  bool sirveAz = (nAz >= MUESTRAS_MINIMAS_ARCO);

  if (!sirveAm && !sirveAz) {
    Serial.print("   NO VI NINGUN ARCO -> me quedo con el de siempre: ");
    Serial.println(arcoNombre());
    return;
  }
  if (sirveAm && !sirveAz)      objetivoEsAmarillo = true;
  else if (sirveAz && !sirveAm) objetivoEsAmarillo = false;
  else                          objetivoEsAmarillo = (medAm < medAz);

  Serial.print("   *** ATACO EL ARCO "); Serial.print(arcoNombre()); Serial.println(" ***");
}

int           pwmRampaPatada = 0;
unsigned long t_rampaPatada  = 0;

// Rampa del golpe: sube de a escalones para que no patinen las ruedas.
void reiniciarRampaPatada() {
  pwmRampaPatada = 0;
  t_rampaPatada  = millis();
}

int rampaPatada(int objetivo) {
  unsigned long ahora = millis();
  if (objetivo < pwmRampaPatada) {
    pwmRampaPatada = objetivo;
  } else if (ahora - t_rampaPatada >= RAMPA_PATADA_MS) {
    t_rampaPatada = ahora;
    pwmRampaPatada += pateandoSaque ? RAMPA_SAQUE_PASO : RAMPA_PATADA_PASO;   // [saque] mas suave
    if (pwmRampaPatada > objetivo) pwmRampaPatada = objetivo;
  }
  return pwmRampaPatada;
}

// Cambia de estado. Se decide UNA sola vez al entrar: el rumbo de la patada
// y el sentido de la orbita (decidirlo en cada vuelta lo hacia patalear).
void cambiarA(Estado nuevo) {
  if (nuevo == PATEA_ADEL) {
    rumboPatadaValido = giroscopoSano();
    if (rumboPatadaValido) rumboAlPatear = rumboActual();
    else Serial.println("!!! patada sin rumbo del giroscopo -> va con trim fijo, sin lazo");
  }
  if (nuevo == PATEA_ADEL) { reiniciarRampaPatada(); reiniciarDerivadaPatada(); }
  if (nuevo == ORBITANDO) { sentidoOrbita = sentidoParaOrbitar();
                            recentrandoOrbita = false; t_orbSinVerla = 0;
                            giradoOrbita = 0; t_buscaRumbo0 = 0; t_muestraOrb = millis();
                            if (hayGiroscopo) rumboOrbPrevio = rumboActual(); }
  if (nuevo == ORBITANDO && hayGiroscopo) {     // [09/11] para ver si va para el lado largo
    errCeroOrbInicial = fabs(diferencia(rumboCero, rumboActual()));
    errCeroOrbMin     = errCeroOrbInicial;
  }
  if (nuevo == SAQUE_GIRA) {                     // [saque] lado de la orbita y cuanto le falta
    sentidoOrbita   = sentidoParaOrbitar();
    errSaqueInicial = fabs(diferencia(rumboSaque, rumboActual()));   // para ver si va para el lado largo
  }
  if (saquePendiente && nuevo != SAQUE_GIRA && nuevo != PATEA_ADEL && nuevo != FRENA_PATADA) {
    saquePendiente = false;                      // [saque] lo corto algo (la linea, el tiempo): juega normal
    Serial.print("... [saque] cortado (paso a "); Serial.print(nombreEstado(nuevo));
    Serial.println(") -> juego normal");
  }
  if (nuevo == ESCAPA_LINEA) t_sinAcercarse = 0;
  if (nuevo == PATEA_ATRAS) retrocesoDeSaque = pateandoSaque;   // [saque] su marcha atras tiene numeros propios
  if (nuevo != PATEA_ADEL && nuevo != FRENA_PATADA) pateandoSaque = false;   // [saque] termino (o lo corto la linea)
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
}

// ===== ARRANQUE =====
// CARGAR-ROBOT.bat lee la linea 'ARCO:' del cartel: no cambiarla.
// [saque] Decide para donde va el saque y calcula su rumbo (+ = derecha: el rumbo SUBE).
// [09/11] Al azar: la semilla sale de los microsegundos en que termino de arrancar el
// giroscopio (nunca tarda exactamente lo mismo) y del ruido de los sensores de linea.
// Asi cada vez que se prende sale distinto.
void elegirSaque() {
  float grados;
  if (SAQUE_AL_AZAR) {
    uint32_t semilla = micros();
    for (int i = 0; i < 3; i++) semilla = semilla * 31 + analogRead(pinLinea[i]);
    randomSeed(semilla);
    int sorteo = random(100);                        // 0..99
    if (sorteo < PROB_SAQUE_CENTRO) {
      grados = 0;             saqueElegido = "CENTRO";
    } else if (sorteo < PROB_SAQUE_CENTRO + PROB_SAQUE_IZQUIERDA) {
      grados = -GRADOS_SAQUE; saqueElegido = "IZQUIERDA";
    } else {
      grados = GRADOS_SAQUE;  saqueElegido = "DERECHA";
    }
    Serial.print("SAQUE sorteado: "); Serial.print(saqueElegido);
    Serial.print("  (salio el "); Serial.print(sorteo); Serial.println(" de 0 a 99)");
  } else {
    grados = SAQUE_A_LA_DERECHA ? GRADOS_SAQUE : -GRADOS_SAQUE;
    saqueElegido = SAQUE_A_LA_DERECHA ? "DERECHA" : "IZQUIERDA";
  }
  rumboSaque = rumboCero + grados;
  while (rumboSaque >= 360.0) rumboSaque -= 360.0;
  while (rumboSaque < 0.0)    rumboSaque += 360.0;
}

void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Serial1.begin(19200);

  // [09/11] Espera lo justo: que el giroscopio conteste (tarda ~0,65 s en despertarse al
  // prender la bateria) y, mientras tanto, mira si hay una compu por el USB.
  // Con la compu, espera (hasta 3 s) que se abra el monitor, como siempre: el cargador
  // lee este cartel. Con la bateria sola, en la cancha, no espera nada mas.
  Wire.begin();
  while (millis() < MS_DESPERTAR_MAX && !(millis() >= MS_DETECTAR_USB && bnoDespierto())) { }
  msDespierto = millis();
  if (usb_configuration) { while (!Serial && millis() < 3000) { } }
  Serial.println();
  Serial.println("==============================================");
  Serial.println("BUSCAR - CENTRAR - AVANZAR - ORBITAR - PATEAR");
  Serial.print("orbita si Xp<"); Serial.println(XP_ORBITA);
  if (BUSCA_CONTINUO) {
    Serial.print("buscar: giro CONTINUO a "); Serial.print(VEL_GIRO_CONTINUO);
    Serial.print(" con rampa; al verla frena "); Serial.print(MS_FRENO_AL_VER);
    Serial.println(" ms el envion");
  } else {
    Serial.print("buscar: pulsos de "); Serial.print(MS_PULSO_BUSC);
    Serial.print(" ms a "); Serial.print(VEL_GIRO);
    Serial.print(" con pausa FRENADA de "); Serial.print(MS_ESPERA_BUSC);
    Serial.println(" ms");
  }
  Serial.print("centrar POR ANGULO: entra a "); Serial.print(TOL_ANG_ENTRA, 0);
  Serial.print(" grados, sale a "); Serial.print(TOL_ANG_SALE, 0);
  Serial.println("  (pausa FRENADA)");
  Serial.print("destrabe: si en "); Serial.print(MS_LIMITE_CENTRAR / 1000);
  Serial.print(" s no se acerca "); Serial.print(MEJORA_DESTRABE);
  Serial.print(" cm, avanza "); Serial.print(MS_AVANCE_DESTRABE / 1000.0, 1);
  Serial.print(" s a "); Serial.print(VEL_DESTRABE);
  Serial.print(" (hasta "); Serial.print(MAX_DESTRABES); Serial.println(" veces)");
  Serial.print("avance: "); Serial.print(VEL_AVANCE);
  Serial.print(" y afloja hasta "); Serial.print(VEL_AVANCE_CERCA);
  Serial.print(" desde Xp<"); Serial.println(XP_FRENAR);
  Serial.print("patea si pelota a menos de "); Serial.print(TOL_ANG_PELOTA, 0);
  Serial.print(" grados del frente Y el arco a menos de "); Serial.print(TOL_ANG_ALINEADO, 0);
  Serial.print(" grados de la pelota");
  if (TOLERANCIA_ADAPTATIVA) {
    Serial.print("  (lejos: pelota "); Serial.print(TOL_ANG_LEJOS_PELOTA, 0);
    Serial.print(", arco ");            Serial.print(TOL_ANG_LEJOS_ARCO, 0);
    Serial.print(" si arcoX entre "); Serial.print(ARCO_LEJOS_MIN);
    Serial.print(" y "); Serial.print(ARCO_LEJOS_MAX); Serial.print(")");
  }
  Serial.println();
  Serial.print("orbita: impulso "); Serial.print(VEL_ORB_IMPULSO);
  Serial.print(" x "); Serial.print(MS_ORB_IMPULSO);
  Serial.print(" ms  ->  crucero "); Serial.print(VEL_ORB_TRASERA);
  Serial.print("   (max "); Serial.print(MS_ORBITA_MAX / 1000); Serial.println(" s)");
  if (RUMBO0_ORBITANDO && PATEAR_AL_RUMBO0) {
    Serial.print("orbita: a las "); Serial.print(VUELTAS_PARA_RUMBO0, 0);
    Serial.print(" vueltas (o "); Serial.print(MS_ORBITA_MAX / 1000);
    Serial.println(" s) sin patear, patea al quedar mirando al RUMBO CERO");
  }
  Serial.print("orbita, no perder la pelota: aguanta ");
  Serial.print(MS_AGUANTE_ORBITA); Serial.print(" ms sin verla FRENADA, y recentra");
  Serial.print(" si pasa "); Serial.print(TOL_ANG_ORB_ENTRA, 0);
  Serial.print(" grados (vuelve a orbitar bajo "); Serial.print(TOL_ANG_ORB_SALE, 0);
  Serial.println(")");
  Serial.print("patada derecha: trim "); Serial.print(TRIM_PATADA);
  Serial.print("  KP "); Serial.print(KP_PATADA, 1);
  Serial.print("  KD "); Serial.print(KD_PATADA, 1);
  Serial.println("  (trasera trabada)");
  Serial.println("la patada se MIDE: al terminar imprime cuanto se torcio, y la");
  Serial.println("telemetria repite las ultimas 5 para leerlas despues sin cable.");
  Serial.print("patada: "); Serial.print(VEL_PATADA);
  Serial.print(" x "); Serial.print(MS_PATADA); Serial.println(" ms");
  Serial.println(FILTRO_BRILLO_PELOTA ? "camara: un arco amarillo pegado a la pelota es su brillo -> NO le creo"
                                      : "camara: filtro del brillo de la pelota APAGADO");
  if (ARCO_FIJO) {
    Serial.print("ARCO: "); Serial.print(arcoNombre());
    Serial.println("  (fijo por programa)");
  } else {
    Serial.println("ARCO: ?  (sin arco fijo: lo elige mirando al encender)");
  }
  Serial.println("==============================================");

  pinMode(PIN_VERSION_PLACA, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION_PLACA) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12;
  }
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   sensores de linea en pines ");
  Serial.print(pinLinea[0]); Serial.print(", ");
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.println("   S1=DERECHO   S2=IZQUIERDO   S3=DELANTERO  (medido 15/09)");

  if (LINEA_ACTIVA) {
    for (int i = 0; i < 3; i++) lineaArranque[i] = analogRead(pinLinea[i]);
    Serial.print("Linea: sensores leen ");
    Serial.print(lineaArranque[0]); Serial.print(" / ");
    Serial.print(lineaArranque[1]); Serial.print(" / ");
    Serial.print(lineaArranque[2]);
    Serial.print("   umbrales "); Serial.print(UMBRAL_LINEA[0]);
    Serial.print(" / "); Serial.print(UMBRAL_LINEA[1]);
    Serial.print(" / "); Serial.print(UMBRAL_LINEA[2]);
    Serial.print("   confirma "); Serial.print(MS_LINEA_CONFIRMA);
    Serial.println(" ms");
    Serial.print("   al ver linea: RETROCEDE YA ");
    Serial.print(MS_RETROCESO_LINEA);
    Serial.print(" ms a "); Serial.print(VEL_ESCAPE_FUERTE);
    Serial.print("  (sin mirar la linea hasta los ");
    Serial.print(MS_CIEGO_LINEA); Serial.println(" ms)");

    int m = leerLineas();
    if (m != 0 && PROTECCION_ARRANQUE) {
      lineaHabilitada = false;
      Serial.println("!!! YA LEE BLANCO ESTANDO EN EL VERDE -> el umbral esta mal.");
      Serial.println("!!! ESCAPE DE LINEA DESACTIVADO. Corre pruebas/grabar-linea/");
    } else {
      lineaHabilitada = true;
      if (m != 0) {
        Serial.print("!!! OJO: ya lee blanco al arrancar (sensores");
        for (int i = 0; i < 3; i++) if (m & (1 << i)) { Serial.print(" "); Serial.print(i + 1); }
        Serial.println("), pero la autoproteccion esta APAGADA.");
        Serial.println("!!! El escape queda ACTIVADO igual. Si el robot escapa sin");
        Serial.println("!!! parar, la causa es esta: los umbrales no sirven para esta");
        Serial.println("!!! superficie. Volver a medir aca mismo.");
      } else {
        Serial.println("Linea: OK, escape ACTIVADO (anula todo lo demas).");
      }
    }
  } else {
    Serial.println("Linea: apagada por configuracion.");
  }

  if (USAR_GIROSCOPO) {
    Serial.print("Giroscopo: ");
    hayGiroscopo = arrancarGiroscopo();
    if (hayGiroscopo) {
      rumboCero = rumboActual();
      Serial.print("OK. Rumbo cero = "); Serial.print(rumboCero, 1);
      Serial.println(" grados (hacia donde mira AHORA)");
      Serial.println(GIRO_SIN_BRUJULA ? "   giroscopo SIN brujula (modo IMU): el cero no depende de los imanes"
                                      : "   giroscopo CON brujula (modo NDOF, el de siempre)");
    } else {
      Serial.println("NO CONTESTA. Sigo sin el, como hasta ayer.");
    }
  } else {
    Serial.println("Giroscopo: apagado por configuracion.");
  }

  // [saque] el rumbo del saque sale del rumbo cero: por eso se prende MIRANDO AL ARCO
  // (el lado se sortea mas abajo, cuando llega la camara: ver elegirSaque)
  if (SAQUE_AL_ARRANCAR && hayGiroscopo) {
    saquePendiente = true;
    if (SAQUE_AL_AZAR) {
      if (DECIDIR_SACA_O_DEFIENDE) Serial.println("SAQUE o DEFENSA: si al arrancar VE la pelota defiende (patea derecho al frente); si no, saca");
      Serial.print("SAQUE: AL AZAR ("); Serial.print(PROB_SAQUE_CENTRO); Serial.print("% centro, ");
      Serial.print(PROB_SAQUE_IZQUIERDA); Serial.print("% izquierda, ");
      Serial.print(100 - PROB_SAQUE_CENTRO - PROB_SAQUE_IZQUIERDA); Serial.print("% derecha), a ");
      Serial.print(GRADOS_SAQUE, 0); Serial.print(" grados del arco, patea a ");
    } else {
      Serial.print("SAQUE: rodea la pelota hasta mirar "); Serial.print(GRADOS_SAQUE, 0);
      Serial.print(" grados a la "); Serial.print(SAQUE_A_LA_DERECHA ? "DERECHA" : "IZQUIERDA");
      Serial.print(" del arco y patea a ");
    }
    Serial.print(VEL_SAQUE); Serial.print(" x "); Serial.print(MS_SAQUE); Serial.print(" ms");
    Serial.print(" (trim "); Serial.print(TRIM_SAQUE); Serial.print(" KP "); Serial.print(KP_SAQUE, 1);
    Serial.print(" KD "); Serial.print(KD_SAQUE, 1); Serial.print(", atras "); Serial.print(VEL_RETROCESO_SAQUE);
    Serial.print(" x "); Serial.print(MS_RETROCESO_SAQUE); Serial.println(" ms)");
  } else if (SAQUE_AL_ARRANCAR) {
    Serial.println("SAQUE: NO (sin giroscopio no sabe hacia donde patear) -> juega normal");
  } else {
    Serial.println("SAQUE: apagado (SAQUE_AL_ARRANCAR = false)");
  }

  if (ARCO_FIJO) {
    Serial.print("   *** ATACO EL ARCO "); Serial.print(arcoNombre());
    Serial.println(" *** (fijo por programa: no miro al encender)");
  } else if (ELEGIR_ARCO_AL_ENCENDER) {
    elegirArcoMirando();
  } else {
    Serial.print("Arco objetivo: "); Serial.print(arcoNombre());
    Serial.println("  (fijo por configuracion)");
  }

  Serial.println("==============================================");
  // [09/11] Sin saque: arranca apenas la camara manda datos (antes: 2 s fijos); sin datos
  // saldria girando a ciegas. CON saque no la espera: el saque es a ciegas, y cuando
  // termina (~1-2 s) la camara ya esta. MS_ESPERA_ARRANQUE es un extra.
  // [10/11] Para decidir si saca o defiende SI hace falta la camara: la espera.
  bool esperaCamara = (!saquePendiente || DECIDIR_SACA_O_DEFIENDE);
  Serial.println(esperaCamara ? "Arranca en cuanto la camara mande datos."
                              : "Arranca en seguida: el saque no necesita la camara.");
  unsigned long t_espera = millis();
  while (millis() - t_espera < MS_ESPERA_ARRANQUE
         || (esperaCamara && t_ultimoPaquete == 0 && millis() < MS_ESPERA_CAMARA_MAX)) {
    leerCamara();
  }
  if (saquePendiente) {
    // [10/11] SACA o DEFIENDE: mira un ratito con la camara ya andando. Si ve la
    // pelota, esta lejos (en el centro: saca el rival) -> defiende. Si no la ve,
    // esta pegada adelante (la camara no la alcanza) -> saca. Sin camara: saca.
    bool defiende = false;
    if (DECIDIR_SACA_O_DEFIENDE && t_ultimoPaquete != 0) {
      unsigned long t_mira = millis();
      while (millis() - t_mira < MS_MIRAR_PELOTA) leerCamara();
      defiende = (t_ultimaPelota != 0);
      if (defiende) {
        Serial.print("INICIO: VEO la pelota (a "); Serial.print(XpBueno);
        Serial.println(" cm) -> DEFIENDO: patada derecho al frente");
      } else {
        Serial.println("INICIO: NO veo la pelota (esta pegada) -> SACO");
      }
    } else if (DECIDIR_SACA_O_DEFIENDE) {
      Serial.println("INICIO: la camara no manda datos -> SACO (el saque es a ciegas)");
    }
    if (defiende) {
      saqueElegido = "DEFENSA";        // patea derecho al frente, como el CENTRO
      rumboSaque   = rumboCero;
    } else {
      elegirSaque();                   // [saque] el sorteo
    }
  }
  msArranque = millis();
  Serial.print("ARRANCO a los "); Serial.print(msArranque);
  Serial.println(t_ultimoPaquete ? " ms de prenderlo (camara OK)" : " ms de prenderlo (la camara NO MANDA DATOS)");
  Serial.print("   (el giroscopio contesto a los "); Serial.print(msDespierto);
  Serial.print(" ms y su fusion tardo "); Serial.print(msFusion); Serial.println(" ms)");

  t_ultimoPaquete = millis();
  t_contadores    = millis();   // si no, la primera medicion sale con dt enorme
  t_inicioSaque   = millis();   // [saque]
  if (!saquePendiente) {
    cambiarA(BUSCANDO);
  } else if (fabs(diferencia(rumboSaque, rumboCero)) < 1.0) {
    Serial.print("*** "); Serial.print(saqueElegido);   // CENTRO o DEFENSA
    Serial.println(": patea derecho al frente, ya mira al arco  -> PATADA");
    saquePendiente = false;           // [saque] centro: no hay que girar
    pateandoSaque  = true;
    cambiarA(PATEA_ADEL);
  } else {
    cambiarA(SAQUE_GIRA);             // [saque] izquierda/derecha: rodea a ciegas
  }
}

// Para que lado orbitar: segun el arco si lo ve; si no, segun el rumbo cero.
bool sentidoParaOrbitar() {
  bool porDefecto = !ORBITA_INVERTIDA;
  if (!ORBITA_CAMINO_CORTO) return porDefecto;

  // [saque] mientras falta el saque, rodea hacia el rumbo del saque (no hacia el arco).
  // Misma cuenta que con el rumbo cero; si igual iba para el lado largo, se da vuelta.
  if (saquePendiente && giroscopoSano()) {
    bool haciaSaque = (diferencia(rumboSaque, rumboActual()) > 0);
    if (SENTIDO_ORBITA_INVERTIDO) haciaSaque = !haciaSaque;
    if (saqueVueltaDada)          haciaSaque = !haciaSaque;
    return haciaSaque;
  }

  // [09/11] SIEMPRE hacia el lado mas corto para volver a mirar al rumbo 0 (el frente con
  // el que arranco el partido). El arco solo se usa si no anda el giroscopio.
  bool haciaElPositivo;
  if (giroscopoSano()) {
    haciaElPositivo = (diferencia(rumboCero, rumboActual()) > 0);
    if (giroOrbInvertido) haciaElPositivo = !haciaElPositivo;   // aprendido en la cancha (ver ORBITANDO)
  } else if (millis() - arcoT() < MS_GRACIA) {
    haciaElPositivo = (anguloDe(arcoX(), arcoY()) > 0);
  } else {
    return porDefecto;
  }

  if (SENTIDO_ORBITA_INVERTIDO) haciaElPositivo = !haciaElPositivo;
  return haciaElPositivo;
}

// ===== LOOP =====
// Orden: 1) camara  2) LINEA BLANCA, que manda sobre todo  3) el estado.
void loop() {

  nLoops++;
  leerCamara();

  // [saque] si no lo pudo patear a tiempo (o se cayo el giroscopio), juega normal
  if (saquePendiente && (millis() - t_inicioSaque > MS_SAQUE_MAX || !giroscopoSano())) {
    saquePendiente = false;
    Serial.print("... [saque] no lo pude patear (");
    Serial.print(giroscopoSano() ? "se acabo el tiempo" : "giroscopio caido");
    Serial.println(") -> me olvido del saque y juego normal");
    if (estado == SAQUE_GIRA) cambiarA(BUSCANDO);
  }

  bool lineaCiega = (t_disparoLinea != 0
                     && millis() - t_disparoLinea < MS_CIEGO_LINEA);

  if (lineaHabilitada && !lineaCiega) {
    int mCrudo = leerLineas();

    for (int i = 0; i < 3; i++) {
      if (mCrudo & (1 << i)) { if (t_sensorDesde[i] == 0) t_sensorDesde[i] = millis(); }
      else                     t_sensorDesde[i] = 0;
    }

    int m = 0;
    for (int i = 0; i < 3; i++)
      if (t_sensorDesde[i] != 0 && millis() - t_sensorDesde[i] >= MS_LINEA_CONFIRMA)
        m |= (1 << i);

    if (m != 0) {
      t_ultimaLinea = millis();

      // La direccion del escape la decide el sensor que vio la linea PRIMERO.
      int primero = -1;
      for (int i = 0; i < 3; i++) {
        if (!(m & (1 << i))) continue;
        if (primero < 0 || t_sensorDesde[i] < t_sensorDesde[primero]) primero = i;
      }
      mascaraLinea = (1 << primero);

      if (estado != ESCAPA_LINEA) {
        frenoFuerte = (estado == PATEA_ADEL);
        Serial.print("!!! LINEA BLANCA: la vio PRIMERO el sensor ");
        Serial.print(primero + 1);
        Serial.print(" (");
        Serial.print(primero == 0 ? "DERECHO" : (primero == 1 ? "IZQUIERDO" : "DELANTERO"));
        Serial.print(")");
        if (m != mascaraLinea) {         // habia mas de uno viendola
          Serial.print("   [tambien veian:");
          for (int i = 0; i < 3; i++)
            if ((m & (1 << i)) && i != primero) { Serial.print(" "); Serial.print(i + 1); }
          Serial.print("  -> los ignoro]");
        }
        Serial.println();
        Serial.print("    estando en "); Serial.print(nombreEstado(estado));
        Serial.print(" -> RETROCEDO YA "); Serial.print(MS_RETROCESO_LINEA);
        Serial.print(" ms a "); Serial.print(VEL_ESCAPE_FUERTE);
        Serial.print(", ciego hasta los "); Serial.print(MS_CIEGO_LINEA);
        Serial.println(" ms");
        t_disparoLinea = millis();     // arranca la ventana ciega
        cambiarA(ESCAPA_LINEA);
      }
    }
  }

  bool laVeo    = (millis() - t_ultimaPelota) < MS_GRACIA;
  bool veoArco  = (millis() - arcoT())        < MS_GRACIA;
  bool veoArcoPropio = (millis() - arcoPropioT()) < MS_GRACIA;
  unsigned long enEstado = millis() - t_entroEstado;

  float angPelota = anguloDe(XpBueno, YpBueno);
  float angArco   = anguloDe(arcoX(), arcoY());
  bool  arcoLejos   = (TOLERANCIA_ADAPTATIVA
                       && arcoX() >= ARCO_LEJOS_MIN && arcoX() <= ARCO_LEJOS_MAX);
  float tolPelota   = arcoLejos ? TOL_ANG_LEJOS_PELOTA : TOL_ANG_PELOTA;
  float tolAlineado = arcoLejos ? TOL_ANG_LEJOS_ARCO   : TOL_ANG_ALINEADO;

  bool  pelotaAdelante = (fabs(angPelota) <= tolPelota);
  bool  arcoAlineado   = (fabs(diferencia(angArco, angPelota)) <= tolAlineado);

  // ---- ESCAPA_LINEA: retrocede YA y no mira la linea hasta MS_CIEGO_LINEA ----
  if (estado == ESCAPA_LINEA) {
    if (enEstado < MS_RETROCESO_LINEA) {
      escaparDeLinea(mascaraLinea, VEL_ESCAPE_FUERTE);
    } else {
      Serial.print("... retrocedi "); Serial.print(MS_RETROCESO_LINEA);
      Serial.print(" ms. Sigo sin mirar la linea hasta los ");
      Serial.print(MS_CIEGO_LINEA); Serial.println(" ms.");
      cambiarA(BUSCANDO);
    }
  }

  // ---- [10/11] FRENA_PATADA: quieto un ratito y recien ahi patea ----
  // Viniendo de rodear, el robot todavia gira por el envion de la orbita: si patea
  // asi, el golpe sale desviado. El rumbo de la patada se toma AL TERMINAR de frenar.
  else if (estado == FRENA_PATADA) {
    frenar();
    if (enEstado >= (unsigned long)(pateandoSaque ? MS_FRENO_SAQUE : MS_FRENO_PATADA))
      cambiarA(PATEA_ADEL);
  }

  // ---- PATEA_ADEL: el golpe (solo lo corta la linea) ----
  else if (estado == PATEA_ADEL) {
    int vel = rampaPatada(pateandoSaque ? VEL_SAQUE : VEL_PATADA);   // [saque] el saque va con MUCHA fuerza
    bool conLazo = (giroscopoSano() && rumboPatadaValido);
    if (conLazo) avanzarDerecho(vel, rumboAlPatear);
    else {
      int tr = pateandoSaque ? TRIM_SAQUE : TRIM_PATADA;   // [saque] numeros propios
      motoresPatada(vel - tr, vel + tr);
    }

    float errP = conLazo ? diferencia(rumboAlPatear, rumboActual()) : 0.0;
    if (fabs(errP) > fabs(picoEnCurso)) picoEnCurso = errP;

    if (enEstado >= (unsigned long)(pateandoSaque ? MS_SAQUE : MS_PATADA)) {
      int i = nPatadas % PATADAS_GUARDADAS;
      desvioPatada[i]  = errP;
      picoPatada[i]    = picoEnCurso;
      conLazoPatada[i] = conLazo;
      nPatadas++;
      Serial.print("*** patada "); Serial.print(nPatadas);
      Serial.print(" terminada: se torcio "); Serial.print(errP, 1);
      Serial.print(" grados (pico "); Serial.print(picoEnCurso, 1);
      Serial.println(conLazo ? ")" : ")  [SIN LAZO: trim fijo solo]");
      picoEnCurso = 0;
      cambiarA(PATEA_ATRAS);
    }
  }

  // ---- PATEA_ATRAS: marcha atras, sin freno en el medio ----
  else if (estado == PATEA_ATRAS) {
    retroceder(retrocesoDeSaque ? VEL_RETROCESO_SAQUE : VEL_RETROCESO);              // [saque]
    if (enEstado >= (unsigned long)(retrocesoDeSaque ? MS_RETROCESO_SAQUE : MS_RETROCESO)) cambiarA(BUSCANDO);
  }

  // ---- [saque] SAQUE_GIRA: rodea la pelota A CIEGAS hasta el rumbo sorteado ----
  // Sin camara: la orbita gira alrededor de un punto adelante (donde va la pelota) y
  // el giroscopio dice cuando llego. Si iba para el lado largo, se da vuelta una vez.
  else if (estado == SAQUE_GIRA) {
    float falta = diferencia(rumboSaque, rumboActual());
    if (fabs(falta) <= TOL_SAQUE) {
      Serial.print("*** SAQUE "); Serial.print(saqueElegido);
      Serial.print(": llegue al rumbo "); Serial.print(rumboSaque, 0);
      Serial.print(" (me faltaban "); Serial.print(falta, 1); Serial.println(" grados)  -> PATADA");
      saquePendiente = false;
      pateandoSaque  = true;         // este golpe va con VEL_SAQUE x MS_SAQUE
      cambiarA(FRENA_PATADA);        // [10/11] primero frena (MS_FRENO_SAQUE), despues patea
    }
    else if (!saqueVueltaDada && fabs(falta) > errSaqueInicial + 30.0) {
      Serial.println("... [saque] rodeaba para el lado largo -> doy vuelta la orbita");
      saqueVueltaDada = true;
      cambiarA(SAQUE_GIRA);          // elige el lado de nuevo (ahora al reves), con el golpe de arranque
    }
    else {
      bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
      orbitar(sentidoOrbita, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
    }
  }

  // ---- ORBITANDO ----
  else if (estado == ORBITANDO) {

    if (laVeo) t_orbSinVerla = 0;

    // Cuenta cuanto giro alrededor de la pelota (para el plan del rumbo cero).
    if (RUMBO0_ORBITANDO && PATEAR_AL_RUMBO0 && giroscopoSano()
        && millis() - t_muestraOrb >= 20) {
      t_muestraOrb = millis();
      float r = rumboActual();
      giradoOrbita  += diferencia(r, rumboOrbPrevio);
      rumboOrbPrevio = r;
      if (!saquePendiente && t_buscaRumbo0 == 0 && fabs(giradoOrbita) >= 360.0 * VUELTAS_PARA_RUMBO0) {
        t_buscaRumbo0 = millis();
        Serial.print("... di "); Serial.print(VUELTAS_PARA_RUMBO0, 0);
        Serial.println(" vueltas y no patee -> pateo al quedar mirando al rumbo cero");
      }
    }

    static unsigned long t_diagOrb = 0;
    if (millis() - t_diagOrb > 1000) {
      t_diagOrb = millis();
      Serial.print("    [orbita] Xp=");  Serial.print(XpBueno);
      Serial.print(" ang=");             Serial.print(angPelota, 1);
      Serial.print(recentrandoOrbita ? "  RECENTRANDO" : "  orbitando");
      Serial.print("  girado="); Serial.print(giradoOrbita, 0);
      if (t_buscaRumbo0 != 0) Serial.print(" BUSCA EL CERO");
      if (hayGiroscopo) { Serial.print("  falta para el 0="); Serial.print(diferencia(rumboCero, rumboActual()), 0); }
      if (giroOrbInvertido) Serial.print(" (0 al reves)");
      if (saquePendiente) { Serial.print(" SAQUE: faltan "); Serial.print(diferencia(rumboSaque, rumboActual()), 0); Serial.print(" grados"); }
      if (veoArcoPropio) Serial.print(" VEO NUESTRO ARCO");
      Serial.print("  sin verla=");
      Serial.print(t_orbSinVerla ? (long)(millis() - t_orbSinVerla) : 0L);
      Serial.print("/"); Serial.print(MS_AGUANTE_ORBITA);
      Serial.print(" ms  arco=");
      if (veoArco) { Serial.print(angArco, 1); Serial.println(" grados"); }
      else         { Serial.println("no lo veo"); }
    }

    // Si deja de verla: frena y la espera MS_AGUANTE_ORBITA sin salir de la orbita.
    if (!laVeo) {                                   // no la ve AHORA
      if (t_orbSinVerla == 0) t_orbSinVerla = millis();
      if (millis() - t_orbSinVerla > MS_AGUANTE_ORBITA) {
        Serial.print("... perdi la pelota orbitando (");
        Serial.print(MS_AGUANTE_ORBITA); Serial.println(" ms sin verla)");
        cambiarA(BUSCANDO);
      } else {
        frenar();     // quieto: es cuando la camara la encuentra mejor
      }
    }
    else if (XpBueno > XP_SUELTA) {                 // se le alejo: vuelve a ir
      cambiarA(AVANZANDO);
    }
    else if (!saquePendiente && veoArco && pelotaAdelante && arcoAlineado) {
      Serial.print("*** ALINEADO con el arco "); Serial.print(arcoNombre());
      Serial.print("  (pelota a "); Serial.print(angPelota, 1);
      Serial.print(" grados, arco a "); Serial.print(angArco, 1);
      Serial.print(", separados "); Serial.print(fabs(diferencia(angArco, angPelota)), 1);
      Serial.print(", tolerancia "); Serial.print(tolPelota, 0);
      if (arcoLejos) Serial.print(" por arco LEJOS");
      Serial.println(")  -> PATADA");
      cambiarA(FRENA_PATADA);        // [10/11] primero frena (MS_FRENO_PATADA), despues patea
    }
    else if (t_buscaRumbo0 != 0 && giroscopoSano()
             && fabs(angPelota) <= TOL_ANG_LEJOS_PELOTA
             && fabs(diferencia(rumboCero, rumboOrbPrevio)) <= TOL_RUMBO
             && !(VETO_ARCO_PROPIO && veoArcoPropio)) {
      Serial.print("*** mirando al rumbo cero con la pelota adelante (rumbo ");
      Serial.print(rumboOrbPrevio, 0); Serial.print(", cero "); Serial.print(rumboCero, 0);
      Serial.print(", pelota a "); Serial.print(angPelota, 1);
      Serial.println(" grados)  -> PATADA");
      cambiarA(FRENA_PATADA);        // [10/11] primero frena, despues patea
    }
    else if (t_buscaRumbo0 != 0 && millis() - t_buscaRumbo0 > MS_RUMBO0_MAX) {
      Serial.println("... no llegue a quedar mirando al rumbo cero -> vuelvo a buscar");
      cambiarA(BUSCANDO);
    }
    else if (!saquePendiente && t_buscaRumbo0 == 0 && enEstado > MS_ORBITA_MAX) {   // se le acabo el tiempo y no lo vio
      Serial.print("... orbite "); Serial.print(MS_ORBITA_MAX / 1000);
      Serial.print(" s y no encontre el arco "); Serial.println(arcoNombre());

      if (PATEAR_AL_RUMBO0 && giroscopoSano()) {
        if (RUMBO0_ORBITANDO) {
          t_buscaRumbo0 = millis();
          Serial.println("    -> sigo orbitando y pateo al quedar mirando al rumbo cero");
        } else {
          Serial.print("    -> voy a apuntar al rumbo de arranque (");
          Serial.print(rumboCero, 1); Serial.println(" grados) y patear ahi");
          cambiarA(APUNTA_RUMBO0);
        }
      } else {
        cambiarA(BUSCANDO);
      }
    }
    else {
      float desvOrb = fabs(angPelota);
      if (recentrandoOrbita) { if (desvOrb < TOL_ANG_ORB_SALE)  recentrandoOrbita = false; }
      else                   { if (desvOrb > TOL_ANG_ORB_ENTRA) recentrandoOrbita = true;  }

      if (recentrandoOrbita) {
        bool haciaUnLado = (YpBueno > 0);
        if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
      } else {
        // [09/11] El lado "hacia el 0" con el giroscopio no esta medido en la cancha. Si lo que
        // falta para el 0 CRECE 30 grados sin haberse achicado nunca, iba al reves: se da
        // vuelta una vez y se lo acuerda todo el partido. (Si primero se acerco y despues se
        // paso del 0 siguiendo la orbita, eso esta bien: no cuenta.)
        float faltaCero = giroscopoSano() ? fabs(diferencia(rumboCero, rumboActual())) : 0;
        if (faltaCero < errCeroOrbMin) errCeroOrbMin = faltaCero;
        if (!giroOrbAprendido && giroscopoSano()
            && errCeroOrbMin > errCeroOrbInicial - 5.0
            && faltaCero > errCeroOrbInicial + 30.0) {
          giroOrbInvertido = true;
          giroOrbAprendido = true;
          Serial.println("... [orbita] hacia el 0 iba para el lado largo -> la doy vuelta (y me lo acuerdo)");
          cambiarA(ORBITANDO);         // elige el lado de nuevo (ahora al reves), con el golpe de arranque
        } else {
          bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
          orbitar(sentidoOrbita, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
        }
      }
    }
  }

  // ---- APUNTA_RUMBO0: gira en el lugar hasta el cero (si RUMBO0_ORBITANDO = false) ----
  else if (estado == APUNTA_RUMBO0) {

    if (!laVeo) {
      Serial.println("... perdi la pelota apuntando al rumbo 0");
      cambiarA(BUSCANDO);
    }
    else if (!giroscopoSano()) {
      Serial.println("!!! el giroscopo se quedo mudo apuntando -> vuelvo a buscar");
      cambiarA(BUSCANDO);
    }
    else {
      float err = diferencia(rumboCero, rumboActual());
      if (fabs(err) <= TOL_RUMBO) {
        Serial.print("*** mirando al rumbo de arranque (error ");
        Serial.print(err, 1); Serial.println(" grados) -> PATADA");
        cambiarA(FRENA_PATADA);      // [10/11] primero frena, despues patea
      }
      else if (enEstado > MS_APUNTAR_MAX) {
        Serial.println("... no llegue a apuntar al rumbo 0 -> vuelvo a buscar");
        cambiarA(BUSCANDO);
      }
      else {
        bool haciaUnLado = (err > 0);
        if (GIRO_RUMBO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
      }
    }
  }

  else {
    if (laVeo) t_perdidaDesde = 0;   // la ve: se cancela el cronometro de perdida
    if (!laVeo) {
      if (estado != BUSCANDO) cambiarA(BUSCANDO);
      if (t_perdidaDesde == 0) t_perdidaDesde = millis();
      if (millis() - t_perdidaDesde > MS_PERDIDA_REAL) {
        t_sinAcercarse = 0;
        nDestrabes     = 0;
      }
      t_frenoAlVer   = 0;
      if (BUSCA_CONTINUO) girarContinuo(!GIRO_INVERTIDO);
      else                rotarPulsado(!GIRO_INVERTIDO, VEL_GIRO, MS_PULSO_BUSC, MS_ESPERA_BUSC);
    }
    else if (estado == BUSCANDO && BUSCA_CONTINUO
             && (t_frenoAlVer == 0 || millis() - t_frenoAlVer < MS_FRENO_AL_VER)) {
      if (t_frenoAlVer == 0) {
        t_frenoAlVer = millis();
        pwmGiroBusc  = 0;          // la proxima busqueda arranca suave
        Serial.print("*** la vi a "); Serial.print(XpBueno);
        Serial.println(" cm -> freno el envion antes de centrar");
      }
      frenar();
    }
    else if (XpBueno < XP_ORBITA) {
      Serial.print("*** llegue a "); Serial.print(XpBueno);
      Serial.print(" cm -> a orbitar buscando el arco ");
      Serial.println(arcoNombre());
      nDestrabes     = 0;          // llego: el cupo de destrabes se renueva
      t_sinAcercarse = 0;
      pwmGiroBusc    = 0;
      cambiarA(ORBITANDO);
    }
    else if (estado == DESTRABANDO) {
      if (millis() - t_entroEstado >= MS_AVANCE_DESTRABE) {
        t_sinAcercarse = 0;        // el reloj se recarga con la nueva distancia
        pwmGiroBusc = 0;           // el giro de busqueda arranca suave
        cambiarA(BUSCANDO);
      } else {
        avanzar(VEL_DESTRABE);
      }
    }
    else {

      // DESTRABE: el reloj mide 'la veo y no me acerco'. MAX_DESTRABES no es un
      // tope: gastados, espera otro MS_LIMITE_CENTRAR y vuelve a empezar.
      if (t_sinAcercarse == 0) { t_sinAcercarse = millis(); mejorXp = XpBueno; }
      if (XpBueno < mejorXp - MEJORA_DESTRABE) {
        mejorXp        = XpBueno;
        t_sinAcercarse = millis();     // progresa: se le da mas tiempo
      }

      static unsigned long t_diagDestrabe = 0;
      if (millis() - t_diagDestrabe > 1000) {
        t_diagDestrabe = millis();
        Serial.print("    [destrabe] Xp="); Serial.print(XpBueno);
        Serial.print(" mejor=");           Serial.print(mejorXp);
        Serial.print(" reloj=");           Serial.print((long)(millis() - t_sinAcercarse));
        Serial.print("/");                 Serial.print(MS_LIMITE_CENTRAR);
        Serial.print(" ms  cupo=");        Serial.print(nDestrabes);
        Serial.print("/");                 Serial.print(MAX_DESTRABES);
        Serial.print("  ");                Serial.println(nombreEstado(estado));
      }

      if (nDestrabes >= MAX_DESTRABES
          && millis() - t_sinAcercarse > MS_LIMITE_CENTRAR) {
        Serial.println("... gaste los destrabes y no me acerco -> suelto esta pelota y busco de nuevo");
        nDestrabes     = 0;        // arranca de nuevo la cuenta de destrabes
        t_sinAcercarse = 0;
        pwmGiroBusc    = 0;
        cambiarA(BUSCANDO);
      }
      else if (nDestrabes < MAX_DESTRABES
          && millis() - t_sinAcercarse > MS_LIMITE_CENTRAR) {
        nDestrabes++;
        Serial.print("... la veo a "); Serial.print(XpBueno);
        Serial.print(" cm y no me acerco -> avanzo a lo bruto (destrabe ");
        Serial.print(nDestrabes); Serial.print(" de "); Serial.print(MAX_DESTRABES);
        Serial.println(")");
        cambiarA(DESTRABANDO);
      }
      else {
        float desvio = fabs(angPelota);
        if (estado == CENTRANDO) {
          if (desvio < TOL_ANG_SALE) cambiarA(AVANZANDO);
        } else {
          if (desvio > TOL_ANG_ENTRA)    cambiarA(CENTRANDO);
          else if (estado != AVANZANDO)  cambiarA(AVANZANDO);
        }

        if (estado == CENTRANDO) {
          bool haciaUnLado = (YpBueno > 0);
          if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
          rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
        } else {
          avanzar(velAcercarse(XpBueno));
        }
      }
    }
  }

  // ---- AVISOS POR EL MONITOR (no cambian lo que hace el robot) ----
  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    Serial.print(">>> "); Serial.print(nombreEstado(estado));
    Serial.print("   Xp="); Serial.print(XpBueno);
    Serial.print(" Yp=");  Serial.print(YpBueno);
    Serial.print(" (a "); Serial.print(angPelota, 1); Serial.print(" grados)");
    Serial.print("  arco "); Serial.print(arcoNombre()); Serial.print(": ");
    if (veoArco) { Serial.print("a "); Serial.print(angArco, 1); Serial.println(" grados"); }
    else         { Serial.println("no lo veo"); }
  }

  bool sinDatos = (millis() - t_ultimoPaquete > SIN_DATOS_MS);
  if (sinDatos && !avisadoSinCamara) {
    Serial.println("!!! NO LLEGAN DATOS DE LA CAMARA !!!");
    avisadoSinCamara = true;
  }
  if (!sinDatos) avisadoSinCamara = false;

  // Telemetria cada 2 s: se lee despues, enchufando el USB SIN apagar la bateria.
  if (millis() - t_ultimoAviso > 2000) {
    t_ultimoAviso = millis();
    Serial.print("   ["); Serial.print(nombreEstado(estado));
    Serial.print("]  Xp="); Serial.print(XpBueno);
    Serial.print(" Yp="); Serial.print(YpBueno);
    Serial.print("  angPelota="); Serial.print(angPelota, 1);
    Serial.print("  angArco=");
    if (veoArco) Serial.print(angArco, 1); else Serial.print("--");
    Serial.print("  separacion=");
    if (veoArco) Serial.print(fabs(diferencia(angArco, angPelota)), 1); else Serial.print("--");
    Serial.print("  arcoX="); if (veoArco) Serial.print(arcoX()); else Serial.print("--");
    Serial.print("  tol="); Serial.print(tolPelota, 0);
    if (arcoLejos) Serial.print("(lejos)");
    if (hayGiroscopo) {
      float r = rumboActual();
      Serial.print("  rumbo=");
      if (giroscopoSano()) Serial.print(r, 0);
      else                 Serial.print("CAIDO");
    }
    Serial.println();

    Serial.print("        linea: ");
    Serial.print(lineaHabilitada ? "ON " : "OFF");
    Serial.print("  arranque ");
    for (int i = 0; i < 3; i++) { Serial.print(lineaArranque[i]); if (i < 2) Serial.print("/"); }
    Serial.print("  visto ");
    for (int i = 0; i < 3; i++) {
      if (lineaMax[i] < 0) Serial.print("--");
      else { Serial.print(lineaMin[i]); Serial.print(".."); Serial.print(lineaMax[i]); }
      if (i < 2) Serial.print(" ");
    }
    Serial.print("  umbrales ");
    for (int i = 0; i < 3; i++) { Serial.print(UMBRAL_LINEA[i]); if (i < 2) Serial.print("/"); }
    Serial.println();

    unsigned long dt = millis() - t_contadores;
    if (dt > 0) {
      Serial.print("        camara: "); Serial.print(nPaquetes * 1000UL / dt);
      Serial.print(" paq/s   "); Serial.print(nTirados * 1000UL / dt);
      Serial.print(" bytes tirados/s   loop: "); Serial.print(nLoops * 1000UL / dt);
      Serial.print(" /s   brillos descartados: "); Serial.print(nBrillos);
      Serial.print("   arranco a los "); Serial.print(msArranque);
      Serial.print(" ms (giro contesto "); Serial.print(msDespierto);
      Serial.print(", fusion "); Serial.print(msFusion); Serial.print(" ms)");
      Serial.print("   saque: "); Serial.println(saqueElegido);
    }
    if (nPatadas > 0) {
      Serial.print("        patadas: "); Serial.print(nPatadas);
      Serial.print("   desvio (grados, ultimas primero):");
      int cuantas = (nPatadas < PATADAS_GUARDADAS) ? nPatadas : PATADAS_GUARDADAS;
      for (int k = 1; k <= cuantas; k++) {
        int i = (nPatadas - k) % PATADAS_GUARDADAS;
        Serial.print(" "); Serial.print(desvioPatada[i], 1);
        Serial.print("(pico "); Serial.print(picoPatada[i], 1);
        Serial.print(conLazoPatada[i] ? ")" : ",SIN LAZO)");
      }
      Serial.println();
    }
    nPaquetes = nTirados = nLoops = 0;
    t_contadores = millis();
  }
}
