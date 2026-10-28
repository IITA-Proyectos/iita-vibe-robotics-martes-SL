/* =====================================================================
   TABLERO DEL DELANTERO — probar los movimientos del partido de a uno,
   medirlos con el giroscopio y ajustar los numeros SIN RECARGAR
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-10-27
   =====================================================================

   QUE ES
   Es el programa del ROBOT que va con la VENTANA DEL TABLERO (el programa
   de la compu, con botones). La ventana le manda ordenes cortas por el
   cable USB y este programa las cumple: mueve el robot, mide y contesta.
   Tambien se usa SIN la ventana, desde cualquier consola serie a 19200
   (por ejemplo la de PlatformIO), escribiendo las mismas ordenes.

   Junta en un solo programa la patada medida de pruebas/patada-derecha y
   los otros movimientos del partido, con la idea de la herramienta del
   arquero (arquero-teensy-zircon/pruebas/ajustar-movimientos): una NOTA
   por movimiento y un CONSEJO de que numero tocar.

   LOS 7 MOVIMIENTOS (el codigo de los motores es COPIA DEL PRINCIPAL)
     1 PATADA      el golpe (rampa + trim + corrector KP/KD del giroscopio,
                   trasera trabada) y despues el retroceso, sin freno en el
                   medio. Igual que en el partido.
     2 AVANCE      avanzar derecho, como cuando va hacia la pelota
     3 ESCAPE_S1   el escape de la linea blanca del sensor 1 (DERECHO):
                   sale en diagonal, hacia adelante a la IZQUIERDA
     4 ESCAPE_S2   el del sensor 2 (IZQUIERDO): adelante a la DERECHA
     5 ESCAPE_S3   el del sensor 3 (DELANTERO): derecho para ATRAS
     6 ORBITA      la vuelta alrededor de la pelota (golpe de arranque y
                   despues crucero). Se prueba sin pelota.
     7 GIRO        el giro continuo de busqueda, con su rampa

   QUE MIDE
   El giroscopio anota cada 10 ms hacia donde mira el robot, contra el
   rumbo con el que arranco. Al terminar (NUNCA mientras se mueve, para no
   cambiarle el ritmo al programa) manda:
     - todas las muestras juntas, para el grafico
     - la NOTA:
         derechos (1 a 5):  PROLIJO, CASI, ZIGZAGUEA o SE_TUERCE
         giros (6 y 7):     GIRA_BIEN, SE_PLANTA o NO_ARRANCA
         y NO_VALE si se quedo sin giroscopio o lo frenaron
     - el PICO (lo mas torcido) y cuando fue, el pico de los primeros
       200 ms (en la patada es lo que ve la pelota) y como termino
     - cuantas veces cruzo de un lado al otro (zigzag)
     - orbita y giro: cuanto giro, a cuantos grados por segundo, y el
       rato mas largo que se quedo PLANTADO (casi sin girar)
     - el HUECO: la vuelta del programa mas larga. El chequeo del
       giroscopio clava el programa 200 ms (bitacora del 13/10): si pasa
       en medio del movimiento, aca se ve.
     - un CONSEJO: que numero tocar
   Grados: NEGATIVO = se fue a la DERECHA, POSITIVO = a la IZQUIERDA
   (medido el 01/09).

   Mientras espera, cada 100 ms manda lo que ven los sensores: las tres
   lineas, el rumbo, la camara (pelota y arcos). Sirve para ver que todo
   ande ANTES de mover nada.

   COMO SE USA
   1. Robot en el PISO, con lugar (~1 m adelante y a los costados).
      BATERIA PRENDIDA y DESPUES el USB (el giroscopio come de la bateria).
   2. Cargar este programa y abrir la ventana del tablero (o una consola
      a 19200).
   3. Apretar un movimiento. Cuenta 1,5 s y arranca.
   4. Mirar la nota y el consejo. Repetir 3 veces SIN TOCAR NADA para
      saber como esta hoy. Recien ahi cambiar UN numero.
   5. Cuando quede bien: la orden "v" da las lineas para pegar en el
      PANEL DE CONTROL del principal. Anotar todo en la bitacora.
   6. Al terminar, volver a cargar el programa de partido con
      CARGAR-ROBOT.bat: con este cargado el robot NO juega.

   LAS ORDENES (cada una es una linea; la ventana las manda sola)
     !1 .. !7          hacer un movimiento (arranca 1,5 s despues)
     x                 PARAR
     =NOMBRE valor     cambiar un numero      ej:  =TRIM_PATADA 7
     ?                 ayuda y todos los numeros
     v                 las lineas para copiar al principal
     0                 volver todos los numeros a los del principal
     h                 saludo (para saber que programa esta cargado)
   Los numeros cambiados viven en la memoria: al apagar el robot vuelven
   solos a los del principal. No se guarda nada.

   QUE NO HACE, a proposito
   - NO MIRA LA LINEA BLANCA. Si se va de la cancha, se va: dejarle lugar.
   - No busca la pelota: cada movimiento dura un tiempo fijo. La camara
     solo se muestra, no se usa para moverse.
   - Sin giroscopio no se mueve: no habria nada que medir.
   - No guarda los numeros: al apagar vuelven a los del principal.

   🚨 MUEVE EL ROBOT, pero SOLO cuando se pide un movimiento, despues de la
   cuenta de 1,5 s, y con tope de tiempo. CUALQUIER tecla o boton durante
   la cuenta o mientras se mueve lo FRENA (un Enter solo, no). En el piso,
   nunca sobre la mesa, y con el cable flojo.

   ⚠ COPIAS DEL PRINCIPAL. Las funciones marcadas "COPIA DEL PRINCIPAL"
   son copia textual de funciona/delantero/delantero.ino, y los numeros
   arrancan con el valor del PANEL DE CONTROL del principal (27/10). Si el
   principal cambia, hay que hacer el MISMO cambio aca, o el tablero mide
   un robot que ya no existe (con patada-derecha ya paso el 22/09 y el
   06/10). Y al reves: lo que ande aca se lleva alla con la orden "v".

   ---------------------------------------------------------------------
   EL IDIOMA CON LA VENTANA (no cambiarlo sin cambiar la ventana tambien)
   ---------------------------------------------------------------------
   Lineas que terminan en "\n". Las que empiezan con "#" son para la
   ventana; las demas son para personas (la ventana las muestra en su
   consola). Campos NOMBRE=valor separados por un espacio, listas con
   comas, numeros con punto decimal.

   ROBOT -> COMPU
     #HOLA tablero-delantero 2026-10-27        al arrancar y con "h"
     #S t= l= u= r= g= p= am= az= q= e=        cada 100 ms, SOLO en espera
     #V NOMBRE=valor ...                       todos los numeros
     #OK NOMBRE=valor                          confirma un cambio
     #ERROR texto                              orden que no entiende
     #CUENTA 1.5 / 1.0 / 0.5                   la cuenta antes de moverse
     #INICIO n NOMBRE                          arranca a moverse
     #FRENO motivo                             tecla / sin giroscopio / tope de tiempo
     #M t_ms grados fase                       las muestras (g golpe, r retroceso,
                                               m movimiento), despues de moverse
     #R n= mov= nota= pico= picoms= pico200= fin= fin2= cruces= giro= gps=
        duro= hueco= huecoms= plantado=        el informe, despues de las #M
     #CONSEJO texto                            que numero tocar
     #PANEL linea de C++ ... #FINPANEL         con "v"
   Despues de cada #INICIO vienen las #M, un #R y los #CONSEJO, aunque lo
   frenen. Un #FRENO SIN #INICIO antes (en la cuenta, o sin giroscopio)
   no trae informe: el robot no se llego a mover.
   En #INICIO y en #R, n es el NUMERO DEL MOVIMIENTO (1 a 7), el mismo de
   "!n". duro = cuanto duro el movimiento; en la PATADA, cuanto duro el
   GOLPE (para compararlo con MS_PATADA: si el programa se clava, se
   alarga). Los grados de la orbita y el giro son lo girado EN TOTAL desde
   el arranque (pasan de 180), con el mismo signo.
   La "v" manda SOLO los numeros que cambiaron y existen en el principal.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

const char* SALUDO = "#HOLA tablero-delantero 2026-10-27";

// Mapeo MEDIDO en banco 2026-07-28 (robot DELANTERO) — COPIA DEL PRINCIPAL
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

#define LED 13

const unsigned long BAUDIOS = 19200;


// =====================================================================
//          LOS NUMEROS QUE SE CAMBIAN DESDE LA VENTANA
// =====================================================================
//   Valores por defecto: del PANEL DE CONTROL del principal (27/10).
//   Se cambian con "=NOMBRE valor" sin recargar, y la orden "0" vuelve a
//   ESTOS. Si en el principal cambia alguno, cambiarlo aca tambien.
//   Los que dicen "solo de esta prueba" no existen en el principal.
// =====================================================================

// ---------- 1 PATADA (secciones 8 y 9 del panel del principal) ----------
int   VEL_PATADA       = 180;   // del PANEL DE CONTROL del principal (27/10). Fuerza del golpe
int   MS_PATADA        = 450;   // del PANEL DE CONTROL del principal (27/10). Cuanto dura el golpe (ms)
int   TRIM_PATADA      = 5;     // del PANEL DE CONTROL del principal (27/10). Se va a la DERECHA: subir
float KP_PATADA        = 6.0;   // del PANEL DE CONTROL del principal (27/10). Corrige por GRADO torcido
float KD_PATADA        = 0.6;   // del PANEL DE CONTROL del principal (27/10). Corrige por GRADO/SEGUNDO
int   VEL_RETROCESO    = 110;   // del PANEL DE CONTROL del principal (27/10). Marcha atras despues del golpe
int   MS_RETROCESO     = 700;   // del PANEL DE CONTROL del principal (27/10). ...durante esto (ms)
int   LAZO             = 1;     // solo de esta prueba. 1 = corrector del giroscopio prendido (como juega).
                                //   0 = golpe con el trim solo: sirve para ajustar TRIM_PATADA
int   TRASERA_TRABADA  = 1;     // solo de esta prueba. 1 = trasera trabada en el golpe (como juega). 0 = suelta
int   SIN_ESPERA       = 0;     // solo de esta prueba. 1 = el chequeo del giroscopio SIN el delay(200)
                                //   de la libreria (ver estadoGiroSinEspera). 0 = como juega hoy

// ---------- 2 AVANCE ----------
int   VEL_AVANCE       = 95;    // del PANEL DE CONTROL del principal (27/10). Fuerza yendo a la pelota de LEJOS
int   MS_PRUEBA_AVANCE = 800;   // solo de esta prueba. Cuanto avanza (ms)

// ---------- 3, 4 y 5 ESCAPES DE LA LINEA ----------
int           VEL_ESCAPE_FUERTE  = 135;   // del PANEL DE CONTROL del principal (27/10). Fuerza del escape
unsigned long MS_RETROCESO_LINEA = 210;   // del PANEL DE CONTROL del principal (27/10). Cuanto dura (ms)

// ---------- 6 ORBITA ----------
int   VEL_ORB_IMPULSO  = 120;   // del PANEL DE CONTROL del principal (27/10). Golpe de la trasera al arrancar
int   MS_ORB_IMPULSO   = 300;   // del PANEL DE CONTROL del principal (27/10). Cuanto dura ese golpe (ms)
int   VEL_ORB_TRASERA  = 67;    // del PANEL DE CONTROL del principal (27/10). Fuerza de la vuelta (crucero)
int   VEL_ORB_FRENTE   = 30;    // del PANEL DE CONTROL del principal (27/10). Las de adelante, DEBAJO del piso
int   MS_PRUEBA_ORBITA = 3000;  // solo de esta prueba. Cuanto orbita (ms)
const bool ORBITA_INVERTIDA = true;          // del PANEL DE CONTROL del principal (27/10). NO se cambia aca
int   SENTIDO_ORBITA   = !ORBITA_INVERTIDA;  // solo de esta prueba. Para que lado orbita (0 o 1). Arranca
                                             //   en el lado "de siempre" del principal (!ORBITA_INVERTIDA)

// ---------- 7 GIRO DE BUSQUEDA ----------
int           VEL_GIRO_CONTINUO = 35;    // del PANEL DE CONTROL del principal (27/10). Fuerza del giro
int           RAMPA_GIRO_PASO   = 10;    // del PANEL DE CONTROL del principal (27/10). Sube esto de fuerza...
unsigned long RAMPA_GIRO_MS     = 10;    // del PANEL DE CONTROL del principal (27/10). ...cada esto (ms)
int           MS_PRUEBA_GIRO    = 2000;  // solo de esta prueba. Cuanto gira (ms)

// ---------- FIJOS: del PANEL DE CONTROL del principal (27/10). Aca NO se cambian ----------
const int           RAMPA_PATADA_PASO  = 15;
const unsigned long RAMPA_PATADA_MS    = 5;
const int           RESTA_MAX          = 120;
const unsigned long MS_DERIVADA_PATADA = 10;
const unsigned long MS_CHEQUEO_GIRO    = 500;
const unsigned long MS_CIEGO_LINEA     = 300;    // del panel: NUNCA menos que MS_RETROCESO_LINEA.
                                                 //   Por eso MS_RETROCESO_LINEA no puede pasar de esto aca
const bool          GIRO_SIN_BRUJULA   = true;   // giroscopio en modo IMU, como el principal desde el 12/10
const bool          GIRO_INVERTIDO     = true;   // el giro de busqueda es girarContinuo(!GIRO_INVERTIDO)
int UMBRAL_LINEA[3] = { 424, 434, 440 };         // [MEDIDO 20/10] solo para mostrarlos: aca no se mira la linea
const int           XP_MAX             = 150;
const int           XARCO_MAX          = 200;


// =====================================================================
//   CRITERIOS DE LA NOTA (los mismos del arquero, ajustar-movimientos)
// =====================================================================
// Derechos (1 a 5), sobre el golpe en la patada:
//   PROLIJO   si |pico| <= 4 grados, cruces <= 2 y |final| <= 3
//   ZIGZAGUEA si cruces >= 4
//   SE_TUERCE si |pico| >= 10
//   CASI      todo lo demas
// Un CRUCE es pasar de mas de +2 grados a menos de -2, o al reves.
// Giros (6 y 7):
//   NO_ARRANCA si giro en total menos de 20 grados
//   SE_PLANTA  si estuvo mas de 400 ms girando menos de 3 grados, despues
//              del golpe de arranque (orbita) o de la rampa (giro)
//   GIRA_BIEN  todo lo demas
// NO_VALE: se quedo sin giroscopio, lo frenaron, o se paso del tope.
const float         NOTA_PICO_PROLIJO  = 4.0;
const int           NOTA_CRUCES_OK     = 2;
const float         NOTA_FIN_PROLIJO   = 3.0;
const int           NOTA_CRUCES_ZIGZAG = 4;
const float         NOTA_PICO_TUERCE   = 10.0;
const float         BANDA_CRUCE        = 2.0;
const float         GIRO_MINIMO        = 20.0;
const unsigned long MS_PLANTADO        = 400;
const float         GRADOS_PLANTADO    = 3.0;
const unsigned long HUECO_AVISO        = 50;    // ms: una vuelta mas larga que esto = programa clavado
const float         ESCAPE_GIRA_MUCHO  = 10.0;

// ---------- tiempos de la prueba ----------
const unsigned long MS_CUENTA            = 1500;  // cuenta antes de moverse
const unsigned long MS_FRENO_AL_TERMINAR = 400;   // al terminar frena esto y despues suelta las ruedas
const unsigned long MS_ESTADO            = 100;   // cada cuanto manda #S en espera
const unsigned long MS_MUESTRA           = 10;    // una muestra del giroscopio cada esto
const unsigned long MS_TOPE_EXTRA        = 1000;  // tope: lo pedido + esto, y se corta

#define MAX_MUESTRAS 300


// =====================================================================
//   TIPOS PROPIOS. Van ANTES de cualquier funcion: PlatformIO pone sus
//   declaraciones automaticas antes de la PRIMERA funcion que encuentra,
//   y si esa queda arriba de un tipo propio no compila (el principal
//   tiene el mismo aviso, al lado de velAcercarse).
// =====================================================================
enum Modo { ESPERA, CUENTA, MOVIENDO };

struct Ajuste {
  const char* nombre;
  char        tipo;            // 'i' entero, 'u' entero largo, 'f' con decimales, 'b' 1/0
  void*       p;               // la variable de arriba
  float       minimo, maximo;  // lo que acepta "=NOMBRE valor"
  const char* enElPrincipal;   // como se declara en el PANEL DE CONTROL ("" = no existe alla)
  float       defecto;         // se llena en setup() con el valor de arriba
};

// ⚠ NO MOVER. Es la primera funcion del archivo a proposito: PlatformIO
// mete sus declaraciones automaticas aca, despues de los tipos y ANTES de
// la tabla de abajo (al arquero se las metia adentro de su tabla y no
// compilaba).
void paraQueCompile() {}


// =====================================================================
//   LA TABLA DE LOS NUMEROS (para "=NOMBRE valor", "?", "v" y "0")
// =====================================================================
// Los rangos son lo que acepta la orden "=". Fuera de eso: #ERROR.
//   MS_RETROCESO_LINEA hasta MS_CIEGO_LINEA (300): en el principal la ventana
//     ciega tiene que durar por lo menos lo que el escape. Si el escape dura
//     mas, a mitad de camino vuelve a mirar la linea y puede cambiar de
//     direccion: el robot patalea en el borde.
//   MS_PRUEBA_ORBITA y MS_PRUEBA_GIRO hasta 3000: es lo que entra en el
//     grafico (MAX_MUESTRAS 300 x una muestra cada 10 ms). Mas tiempo no
//     mide nada nuevo y es mas rato girando sin mirar la linea.
Ajuste ajustes[] = {
  { "VEL_PATADA",         'i', &VEL_PATADA,          60,  255, "const int",           0 },
  { "MS_PATADA",          'i', &MS_PATADA,          100, 1000, "const int",           0 },
  { "TRIM_PATADA",        'i', &TRIM_PATADA,        -60,   60, "const int",           0 },
  { "KP_PATADA",          'f', &KP_PATADA,            0,   20, "const float",         0 },
  { "KD_PATADA",          'f', &KD_PATADA,            0,    5, "const float",         0 },
  { "VEL_RETROCESO",      'i', &VEL_RETROCESO,        0,  255, "const int",           0 },
  { "MS_RETROCESO",       'i', &MS_RETROCESO,         0, 1500, "const int",           0 },
  { "LAZO",               'b', &LAZO,                 0,    1, "",                    0 },
  { "TRASERA_TRABADA",    'b', &TRASERA_TRABADA,      0,    1, "",                    0 },
  { "SIN_ESPERA",         'b', &SIN_ESPERA,           0,    1, "",                    0 },
  { "VEL_AVANCE",         'i', &VEL_AVANCE,           0,  255, "const int",           0 },
  { "MS_PRUEBA_AVANCE",   'i', &MS_PRUEBA_AVANCE,   100, 3000, "",                    0 },
  { "VEL_ESCAPE_FUERTE",  'i', &VEL_ESCAPE_FUERTE,    0,  255, "const int",           0 },
  { "MS_RETROCESO_LINEA", 'u', &MS_RETROCESO_LINEA,  50, MS_CIEGO_LINEA, "const unsigned long", 0 },
  { "VEL_ORB_IMPULSO",    'i', &VEL_ORB_IMPULSO,      0,  255, "const int",           0 },
  { "MS_ORB_IMPULSO",     'i', &MS_ORB_IMPULSO,       0, 2000, "const int",           0 },
  { "VEL_ORB_TRASERA",    'i', &VEL_ORB_TRASERA,      0,  255, "const int",           0 },
  { "VEL_ORB_FRENTE",     'i', &VEL_ORB_FRENTE,       0,   69, "const int",           0 },
  { "MS_PRUEBA_ORBITA",   'i', &MS_PRUEBA_ORBITA,   500, 3000, "",                    0 },
  { "SENTIDO_ORBITA",     'b', &SENTIDO_ORBITA,       0,    1, "",                    0 },
  { "VEL_GIRO_CONTINUO",  'i', &VEL_GIRO_CONTINUO,    0,  255, "const int",           0 },
  { "RAMPA_GIRO_PASO",    'i', &RAMPA_GIRO_PASO,      1,  255, "const int",           0 },
  { "RAMPA_GIRO_MS",      'u', &RAMPA_GIRO_MS,        1,  200, "const unsigned long", 0 },
  { "MS_PRUEBA_GIRO",     'i', &MS_PRUEBA_GIRO,     300, 3000, "",                    0 },
};
const int N_AJUSTES = sizeof(ajustes) / sizeof(ajustes[0]);

const int   N_MOVIMIENTOS = 7;
const char* NOMBRE_MOV[] = { "", "PATADA", "AVANCE", "ESCAPE_S1", "ESCAPE_S2",
                             "ESCAPE_S3", "ORBITA", "GIRO" };

// Como termino un movimiento
const int TERMINO_BIEN     = 0;
const int FRENO_TECLA      = 1;
const int FRENO_SIN_GIRO   = 2;
const int FRENO_TOPE       = 3;

// Las notas
const int NOTA_PROLIJO    = 0;
const int NOTA_CASI       = 1;
const int NOTA_ZIGZAGUEA  = 2;
const int NOTA_SE_TUERCE  = 3;
const int NOTA_GIRA_BIEN  = 4;
const int NOTA_SE_PLANTA  = 5;
const int NOTA_NO_ARRANCA = 6;
const int NOTA_NO_VALE    = 7;
const char* NOMBRE_NOTA[] = { "PROLIJO", "CASI", "ZIGZAGUEA", "SE_TUERCE",
                              "GIRA_BIEN", "SE_PLANTA", "NO_ARRANCA", "NO_VALE" };


// =====================================================================
//   ESTADO DEL PROGRAMA
// =====================================================================

// ---------- giroscopio (igual que el principal) ----------
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiroscopo      = false;
bool  giroCaido         = false;      // lo dice el propio chip, no lo adivinamos
float ultimoRumbo       = 0;
float rumboAlPatear     = 0;          // rumbo al empezar la patada (heading-hold)
bool  rumboPatadaValido = false;
unsigned long t_chequeoGiro = 0;      // ultima vez que se le pregunto

// ---------- sensores de linea (solo se muestran) ----------
const int PIN_VERSION_PLACA = 32;
int  pinLinea[3]  = { A11, A13, A12 };   // Mark1; se corrige en setup()
const char* versionPlaca = "?";

// ---------- camara (COPIA DEL PRINCIPAL) ----------
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
unsigned long nPaquetes = 0, nTirados = 0;
unsigned long t_contadores = 0;
unsigned long paquetesPorSegundo = 0;

// ---------- el giro de busqueda (COPIA DEL PRINCIPAL) ----------
int  pwmGiroBusc = 0;              // PWM actual de la rampa del giro
unsigned long t_rampaGiro = 0;

// ---------- la prueba ----------
Modo modo = ESPERA;
int  mov  = 0;                     // el movimiento pedido (1..7)
unsigned long t_cuenta = 0;        // cuando empezo la cuenta
int  avisosCuenta = 0;             // cuantos #CUENTA se mandaron
unsigned long t_inicio = 0;        // cuando arranco a moverse
unsigned long t_fase   = 0;        // patada: cuando empezo el golpe o el retroceso
int  fasePatada = 0;               // 1 = golpe, 2 = retroceso
unsigned long t_freno  = 0;        // cuando termino el ultimo movimiento (0 = nunca)
unsigned long t_estado = 0;        // ultimo #S
unsigned long t_chequeoRapido = 0; // ultimo chequeo del giroscopio sin espera
char linea[72];                    // la orden que se esta escribiendo
int  largoLinea = 0;
bool lineaLarga = false;

// ---------- lo que se mide ----------
int      nMuestras = 0;
uint16_t muestraT[MAX_MUESTRAS];   // ms desde que arranco
float    muestraG[MAX_MUESTRAS];   // grados
char     muestraF[MAX_MUESTRAS];   // g golpe, r retroceso, m movimiento
unsigned long t_muestra = 0;
float rumbo0 = 0, rumboPrevio = 0; // el rumbo al arrancar, y la ultima lectura
float girado = 0;                  // orbita y giro: grados girados en total (con signo)
float pico = 0, pico200 = 0, fin = 0, fin2 = 0;
unsigned long picoMs = 0;
int   cruces = 0, ladoDelError = 0;
unsigned long huecoMax = 0, huecoMs = 0, msLoopPrevio = 0;
bool  plantadoArmado = false;
float anclaGiro = 0;
unsigned long t_ancla = 0, plantadoMax = 0;
unsigned long duro = 0;            // lo que duro (en la patada: el golpe)
unsigned long demoraEntrada = 0;   // patada: programa clavado ANTES del golpe


// =====================================================================
//                    MOTORES — COPIA DEL PRINCIPAL
// =====================================================================

// SOLTAR los motores. NO es frenar: el robot sigue de largo por inercia.
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// FRENO ELECTRICO: las mismas patas en 0 que parar(), pero con el PWM a fondo.
void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

// COPIA DEL PRINCIPAL.
void motoresRotando(bool sentidoA, int vel) {
  int a = sentidoA ? 1 : 0;
  int b = sentidoA ? 0 : 1;
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, a); digitalWrite(TRA_INB, b);
}

// COPIA DEL PRINCIPAL. Las dos de adelante empujan y la TRASERA va TRABADA.
// Lo unico agregado es el "if (TRASERA_TRABADA)", igual que en
// patada-derecha (su tecla b): deja comparar con la trasera suelta.
void motoresPatada(int vi, int vd) {
  if (vi < 0) vi = 0;   if (vi > 255) vi = 255;
  if (vd < 0) vd = 0;   if (vd > 255) vd = 255;
  analogWrite(IZQ_PWM, vi); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vd); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  if (TRASERA_TRABADA) {
    digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
  } else {
    analogWrite(TRA_PWM, 0);  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
  }
}

// COPIA DEL PRINCIPAL. Avanzar sin corregir nada: trasera suelta.
void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// COPIA DEL PRINCIPAL. Marcha atras: las dos de adelante al reves, trasera suelta.
void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// COPIA DEL PRINCIPAL. La orbita: adelante suave (debajo del piso, para que
// no giren) y la trasera fuerte.
void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;    // las dos de adelante
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE);  digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE);  digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera);      digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

// COPIA DEL PRINCIPAL. Escapa de la(s) linea(s) de la mascara:
//   bit 0 = sensor 1 (DERECHO)   -> IZQ apagada, sale hacia la delantera izquierda
//   bit 1 = sensor 2 (IZQUIERDO) -> DER apagada, sale hacia la delantera derecha
//   bit 2 = sensor 3 (DELANTERO) -> TRA apagada, sale derecho para atras
void escaparDeLinea(int m, int velocidad) {
  //          IZQ(M1) DER(M2) TRA(M3)
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

// COPIA DEL PRINCIPAL. Giro CONTINUO con rampa para BUSCAR la pelota: sube
// el PWM de a RAMPA_GIRO_PASO hasta VEL_GIRO_CONTINUO y se queda ahi.
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


// =====================================================================
//                  GIROSCOPIO — COPIA DEL PRINCIPAL
// =====================================================================

// COPIA DEL PRINCIPAL. Diferencia mas corta entre dos angulos, en (-180, 180].
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// ---------- EL CHEQUEO QUE CLAVA EL PROGRAMA (de patada-derecha) ----------
// bno.getSystemStatus() de la libreria (Adafruit BNO055 1.6.4) TERMINA CON
// delay(200): 200 ms con el programa entero clavado. Esta funcion lee el
// MISMO dato (el registro 0x39 del chip, SYS_STATUS) directo por I2C, sin
// la espera. 5 = la fusion esta corriendo. Devuelve -1 si el chip no
// contesta (en patada-derecha devolvia 0: para el chequeo es lo mismo,
// porque los dos son "distinto de 5").
int estadoGiroSinEspera() {
  Wire.beginTransmission(0x28);
  Wire.write(0x39);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(0x28, 1) != 1) return -1;
  return Wire.read();
}

// COPIA DEL PRINCIPAL (lo unico agregado es el "if (SIN_ESPERA)", como la
// tecla c de patada-derecha).
float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  ultimoRumbo = evento.orientation.x;        // 0..360

  if (millis() - t_chequeoGiro > MS_CHEQUEO_GIRO) {
    t_chequeoGiro = millis();
    if (SIN_ESPERA) {
      giroCaido = (estadoGiroSinEspera() != 5);
      return ultimoRumbo;
    }
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    giroCaido = (sys != 5);                  // 5 = algoritmo de fusion corriendo
  }
  return ultimoRumbo;
}

// Solo el rumbo, SIN chequeo de salud: para medir los movimientos que en el
// partido no le preguntan nada al giroscopio (avance, escapes, giro y el
// retroceso de la patada). Asi medir no les cambia el ritmo.
float rumboSinChequeo() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  return evento.orientation.x;
}

// COPIA DEL PRINCIPAL.
bool giroscopoSano() {
  return hayGiroscopo && !giroCaido;
}

// COPIA DEL PRINCIPAL. Enciende el BNO055 (modo IMU con GIRO_SIN_BRUJULA).
bool arrancarGiroscopo() {
  if (!bno.begin(GIRO_SIN_BRUJULA ? OPERATION_MODE_IMUPLUS : OPERATION_MODE_NDOF)) return false;
  bno.setExtCrystalUse(true);
  delay(700);                      // AHORA despues: setExtCrystalUse reinicia la
                                   // fusion, asi que esperar antes no servia
  uint8_t sys = 0, autotest = 0, err = 0;
  bno.getSystemStatus(&sys, &autotest, &err);
  if (sys != 5) {
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


// =====================================================================
//           EL GOLPE DERECHO DE LA PATADA — COPIA DEL PRINCIPAL
// =====================================================================

float         errPatadaPrevio = 0;
unsigned long t_derivPatada   = 0;
float         derivPatada     = 0;

// COPIA DEL PRINCIPAL.
void reiniciarDerivadaPatada() {
  errPatadaPrevio = 0;
  t_derivPatada   = 0;
  derivPatada     = 0;
}

// COPIA DEL PRINCIPAL: trim fijo + proporcional + derivativo, repartido
// entre las dos ruedas (a una le suma y a la otra le resta).
void avanzarDerecho(int vel, float rumboObjetivo) {
  int vi = vel - TRIM_PATADA;
  int vd = vel + TRIM_PATADA;

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
  float mando = KP_PATADA * err + KD_PATADA * derivPatada;

  int mitad = (int)(fabs(mando) / 2.0);
  if (mitad > RESTA_MAX / 2) mitad = RESTA_MAX / 2;
  if (mando > 0) { vd -= mitad; vi += mitad; }
  else           { vi -= mitad; vd += mitad; }

  motoresPatada(vi, vd);
}

// COPIA DEL PRINCIPAL: la rampa de arranque del golpe.
int           pwmRampaPatada = 0;
unsigned long t_rampaPatada  = 0;

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
    pwmRampaPatada += RAMPA_PATADA_PASO;
    if (pwmRampaPatada > objetivo) pwmRampaPatada = objetivo;
  }
  return pwmRampaPatada;
}


// =====================================================================
//                    CAMARA — COPIA DEL PRINCIPAL
// =====================================================================
// Aca la camara NO mueve nada: solo se muestra en el #S.
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
      Xam = xam;  Yam = yam - 100;
      Xaz = xaz;  Yaz = yaz - 100;
      t_ultimoPaquete = millis();
      nPaquetes++;

      // 200 y +-100 son los TOPES de recorte de la camara: casi siempre manchas.
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xam > 0) && (Xam <= XARCO_MAX) && (abs(Yam) < 100)) {
        XamBueno = Xam; YamBueno = Yam;
        t_ultimoAmarillo = millis();
      }
      if ((Xaz > 0) && (Xaz <= XARCO_MAX) && (abs(Yaz) < 100)) {
        XazBueno = Xaz; YazBueno = Yaz;
        t_ultimoAzul = millis();
      }
    }
  }
}


// =====================================================================
//              AYUDITAS PARA ESCRIBIR (sin String, sin acentos)
// =====================================================================
// Todas las lineas terminan en "\n" solo (println agrega "\r\n").
void finDeLinea() { Serial.print('\n'); }
void decir(const char* s) { Serial.print(s); finDeLinea(); }

// Con punto decimal y 1 a 2 decimales: 4.0, 0.6, 0.65 (como el panel).
void imprimirDecimal(float v) {
  long c = lround(v * 100.0f);
  if (c < 0) { Serial.print('-'); c = -c; }
  Serial.print(c / 100);
  Serial.print('.');
  Serial.print((c / 10) % 10);
  if (c % 10 != 0) Serial.print(c % 10);
}

// Grados con 1 decimal. Sin "-0.0".
void imprimirGrados(float g) {
  if (fabs(g) < 0.05) Serial.print("0.0");
  else                Serial.print(g, 1);
}

void imprimirNumero(float v, char tipo) {
  if (tipo == 'f') imprimirDecimal(v);
  else             Serial.print(lround(v));
}

const char* haciaDonde(float g) {
  return (g < 0) ? "la DERECHA" : "la IZQUIERDA";
}

// Para comparar nombres sin que importen las mayusculas.
bool mismoNombre(const char* a, const char* b) {
  while (*a && *b) {
    if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) return false;
    a++; b++;
  }
  return *a == 0 && *b == 0;
}


// =====================================================================
//                       LOS NUMEROS (la tabla)
// =====================================================================

float valorDe(int i) {
  switch (ajustes[i].tipo) {
    case 'f': return *(float*)ajustes[i].p;
    case 'u': return (float)*(unsigned long*)ajustes[i].p;
    default:  return (float)*(int*)ajustes[i].p;
  }
}

void ponerValor(int i, float v) {
  switch (ajustes[i].tipo) {
    case 'f': *(float*)ajustes[i].p = (float)lround(v * 100.0f) / 100.0f; break;   // 2 decimales
    case 'u': *(unsigned long*)ajustes[i].p = (unsigned long)lround(v); break;
    default:  *(int*)ajustes[i].p = (int)lround(v); break;
  }
}

bool cambiado(int i) {
  return fabs(valorDe(i) - ajustes[i].defecto) > 0.001;
}

int buscarAjuste(const char* nombre) {
  for (int i = 0; i < N_AJUSTES; i++) if (mismoNombre(nombre, ajustes[i].nombre)) return i;
  return -1;
}

// #V con TODOS los numeros, en una sola linea.
void mandarValores() {
  Serial.print("#V");
  for (int i = 0; i < N_AJUSTES; i++) {
    Serial.print(' ');
    Serial.print(ajustes[i].nombre);
    Serial.print('=');
    imprimirNumero(valorDe(i), ajustes[i].tipo);
  }
  finDeLinea();
}

void volverAlPrincipal() {
  for (int i = 0; i < N_AJUSTES; i++) ponerValor(i, ajustes[i].defecto);
}

// La orden "v": las lineas para pegar en el PANEL DE CONTROL del principal.
// Van SOLO las que cambiaron y existen alla: pegar las demas no cambiaria
// nada, y si mientras tanto alguien toco el principal, lo pisaria.
void mandarPanel() {
  finDeLinea();
  decir("--- PARA COPIAR AL PANEL DE CONTROL del principal (funciona/delantero/delantero.ino) ---");
  decir("Buscar cada nombre (Ctrl+F) y cambiar SOLO esa linea. UN CAMBIO POR VEZ, y anotarlo en la bitacora.");
  int cuantas = 0, deLaPrueba = 0;
  for (int i = 0; i < N_AJUSTES; i++) {
    if (!cambiado(i)) continue;
    if (ajustes[i].enElPrincipal[0] == 0) { deLaPrueba++; continue; }
    cuantas++;
  }
  if (cuantas == 0) decir("(nada para copiar: los numeros del principal estan todos igual que aca)");
  if (deLaPrueba > 0) {
    Serial.print("(no se copian porque son solo de esta prueba:");
    for (int i = 0; i < N_AJUSTES; i++) {
      if (!cambiado(i) || ajustes[i].enElPrincipal[0] != 0) continue;
      Serial.print(' '); Serial.print(ajustes[i].nombre);
    }
    Serial.print(')'); finDeLinea();
  }
  if (SIN_ESPERA) {
    decir("SIN_ESPERA = 1 no es un numero: en el principal es un cambio de CODIGO en rumboActual().");
    decir("Si sirvio, pedirlo (no se arregla pegando una linea).");
  }
  for (int i = 0; i < N_AJUSTES; i++) {
    if (!cambiado(i) || ajustes[i].enElPrincipal[0] == 0) continue;
    Serial.print("#PANEL ");
    Serial.print(ajustes[i].enElPrincipal);
    Serial.print(' ');
    Serial.print(ajustes[i].nombre);
    Serial.print(" = ");
    imprimirNumero(valorDe(i), ajustes[i].tipo);
    Serial.print(';');
    finDeLinea();
  }
  decir("#FINPANEL");
}

void ayuda() {
  finDeLinea();
  decir("------------------- TABLERO DEL DELANTERO -------------------");
  decir("MOVIMIENTOS (el robot SE MUEVE 1,5 s despues de pedirlo):");
  decir("  !1 PATADA     golpe + retroceso, como en el partido");
  decir("  !2 AVANCE     avanzar derecho (el del partido no corrige el rumbo)");
  decir("  !3 ESCAPE_S1  escape de linea del sensor 1 (DERECHO): sale adelante a la izquierda");
  decir("  !4 ESCAPE_S2  escape del sensor 2 (IZQUIERDO): sale adelante a la derecha");
  decir("  !5 ESCAPE_S3  escape del sensor 3 (DELANTERO): sale derecho para atras");
  decir("  !6 ORBITA     la vuelta alrededor de la pelota (sin pelota)");
  decir("  !7 GIRO       el giro continuo de busqueda");
  decir("PARAR: x   (y CUALQUIER tecla en la cuenta o mientras se mueve; Enter solo no)");
  decir("CAMBIAR UN NUMERO: =NOMBRE valor   por ejemplo  =TRIM_PATADA 7");
  decir("OTRAS: ? esta ayuda   v copiar al principal   0 volver a los del principal   h saludo");
  decir("OJO: NO MIRA LA LINEA BLANCA. En el piso, con lugar alrededor, y el cable flojo.");
  Serial.print("Giroscopio: ");
  if (!hayGiroscopo)    decir("NO CONTESTA. Sin el no se mueve: prender la bateria y pedir un movimiento (lo busca de nuevo).");
  else if (giroCaido)   decir("contesta, pero la fusion NO esta corriendo.");
  else                  decir("OK (modo IMU, sin brujula)");
  Serial.print("Camara: ");
  if (paquetesPorSegundo > 0) { Serial.print(paquetesPorSegundo); decir(" paquetes por segundo"); }
  else                        decir("no manda datos (para moverse no hace falta)");
  Serial.print("Placa: "); Serial.print(versionPlaca);
  Serial.print("   sensores de linea en pines ");
  Serial.print(pinLinea[0]); Serial.print(", ");
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.print(pinLinea[2]);
  decir("  (S1 DERECHO, S2 IZQUIERDO, S3 DELANTERO)");
  decir("NUMEROS (entre parentesis, el del principal si no es igual):");
  for (int i = 0; i < N_AJUSTES; i++) {
    Serial.print("  ");
    Serial.print(ajustes[i].nombre);
    for (int k = strlen(ajustes[i].nombre); k < 20; k++) Serial.print(' ');
    imprimirNumero(valorDe(i), ajustes[i].tipo);
    if (cambiado(i)) {
      Serial.print("   (principal: ");
      imprimirNumero(ajustes[i].defecto, ajustes[i].tipo);
      Serial.print(')');
    }
    if (ajustes[i].enElPrincipal[0] == 0) Serial.print("   [solo de esta prueba]");
    finDeLinea();
  }
}


// =====================================================================
//                    LO QUE SE MANDA EN ESPERA (#S)
// =====================================================================

long edad(unsigned long t) {
  if (t == 0) return -1;              // nunca se vio
  return (long)(millis() - t);
}

void mandarEstado() {
  Serial.print("#S t="); Serial.print(millis());
  Serial.print(" l=");
  for (int i = 0; i < 3; i++) { if (i) Serial.print(','); Serial.print(analogRead(pinLinea[i])); }
  Serial.print(" u=");
  for (int i = 0; i < 3; i++) { if (i) Serial.print(','); Serial.print(UMBRAL_LINEA[i]); }

  // En espera el rumbo sale de getEvent() y el estado del registro 0x39
  // leido directo: NUNCA getSystemStatus(), que clava 200 ms.
  int g = -1;
  Serial.print(" r=");
  if (hayGiroscopo) {
    Serial.print(rumboSinChequeo(), 1);
    g = estadoGiroSinEspera();
    giroCaido = (g != 5);
  } else {
    Serial.print("-1");
  }
  Serial.print(" g="); Serial.print(g);

  Serial.print(" p=");  Serial.print(XpBueno);  Serial.print(','); Serial.print(YpBueno);
  Serial.print(',');    Serial.print(edad(t_ultimaPelota));
  Serial.print(" am="); Serial.print(XamBueno); Serial.print(','); Serial.print(YamBueno);
  Serial.print(',');    Serial.print(edad(t_ultimoAmarillo));
  Serial.print(" az="); Serial.print(XazBueno); Serial.print(','); Serial.print(YazBueno);
  Serial.print(',');    Serial.print(edad(t_ultimoAzul));
  Serial.print(" q=");  Serial.print(paquetesPorSegundo);
  Serial.print(" e=");
  Serial.print(modo == ESPERA ? "ESPERA" : (modo == CUENTA ? "CUENTA" : "MOVIENDO"));
  finDeLinea();
}


// =====================================================================
//                         LO QUE SE ANOTA
// =====================================================================

void reiniciarMedicion() {
  nMuestras = 0;
  t_muestra = 0;                       // la primera muestra sale enseguida
  girado = 0;
  pico = 0; pico200 = 0; fin = 0; fin2 = 0; picoMs = 0;
  cruces = 0; ladoDelError = 0;
  huecoMax = 0; huecoMs = 0;
  plantadoArmado = false; anclaGiro = 0; t_ancla = 0; plantadoMax = 0;
  duro = 0; demoraEntrada = 0;
}

void guardarMuestra(unsigned long t, float g, char fase) {
  if (nMuestras >= MAX_MUESTRAS) return;       // la RAM alcanza para 3 s
  muestraT[nMuestras] = (uint16_t)t;
  muestraG[nMuestras] = g;
  muestraF[nMuestras] = fase;
  nMuestras++;
}

// Movimientos derechos: el pico, el de los primeros 200 ms, los cruces y
// como va terminando.
void anotarDesvio(float e, unsigned long enMov) {
  if (fabs(e) > fabs(pico)) { pico = e; picoMs = enMov; }
  if (enMov <= 200 && fabs(e) > fabs(pico200)) pico200 = e;
  int lado = (e > BANDA_CRUCE) ? 1 : (e < -BANDA_CRUCE) ? -1 : 0;
  if (lado != 0) {
    if (ladoDelError != 0 && lado != ladoDelError) cruces++;
    ladoDelError = lado;
  }
  fin = e;
}

// Orbita y giro: cuanto giro en total y el rato mas largo PLANTADO (menos
// de GRADOS_PLANTADO de giro). Se arma despues del golpe de arranque.
void armarPlantado(unsigned long ahora) {
  plantadoArmado = true;
  anclaGiro = girado;
  t_ancla = ahora;
}

void anotarGiro(float r, unsigned long ahora) {
  girado += diferencia(rumboPrevio, r);      // NEGATIVO = gira a la DERECHA
  rumboPrevio = r;
  if (!plantadoArmado) return;
  if (fabs(girado - anclaGiro) >= GRADOS_PLANTADO) {
    anclaGiro = girado;
    t_ancla = ahora;
  } else if (ahora - t_ancla > plantadoMax) {
    plantadoMax = ahora - t_ancla;
  }
}

// En los movimientos que no usan rumboActual(), cada MS_CHEQUEO_GIRO se le
// pregunta al chip si sigue vivo, pero SIN la espera de 200 ms.
void chequeoRapido(unsigned long ahora) {
  if (ahora - t_chequeoRapido < MS_CHEQUEO_GIRO) return;
  t_chequeoRapido = ahora;
  giroCaido = (estadoGiroSinEspera() != 5);
}


// =====================================================================
//                      EL INFORME (#M, #R, #CONSEJO)
// =====================================================================

int calcularNota(int comoTermino) {
  if (comoTermino != TERMINO_BIEN) return NOTA_NO_VALE;
  if (mov >= 6) {
    if (fabs(girado) < GIRO_MINIMO) return NOTA_NO_ARRANCA;
    if (plantadoMax > MS_PLANTADO)  return NOTA_SE_PLANTA;
    return NOTA_GIRA_BIEN;
  }
  if (fabs(pico) <= NOTA_PICO_PROLIJO && cruces <= NOTA_CRUCES_OK
      && fabs(fin) <= NOTA_FIN_PROLIJO) return NOTA_PROLIJO;
  if (cruces >= NOTA_CRUCES_ZIGZAG)    return NOTA_ZIGZAGUEA;
  if (fabs(pico) >= NOTA_PICO_TUERCE)  return NOTA_SE_TUERCE;
  return NOTA_CASI;
}

void campoGrados(const char* nombre, float v, bool aplica) {
  Serial.print(' '); Serial.print(nombre); Serial.print('=');
  if (aplica) imprimirGrados(v); else Serial.print('0');
}

void campoEntero(const char* nombre, long v, bool aplica) {
  Serial.print(' '); Serial.print(nombre); Serial.print('=');
  Serial.print(aplica ? v : 0L);
}

float gradosPorSegundo() {
  if (duro == 0) return 0;
  return girado * 1000.0 / (float)duro;
}

void consejo(const char* s) { Serial.print("#CONSEJO "); decir(s); }

void consejoPatada(int nota) {
  if (LAZO == 0) {
    if (fabs(fin) <= NOTA_FIN_PROLIJO) {
      Serial.print("#CONSEJO Con LAZO = 0 (trim solo) el golpe salio casi derecho: TRIM_PATADA = ");
      Serial.print(TRIM_PATADA); decir(" sirve. Volver LAZO a 1 y medir otra vez.");
    } else {
      Serial.print("#CONSEJO Con LAZO = 0 (trim solo) se fue a "); Serial.print(haciaDonde(fin));
      Serial.print(fin < 0 ? ": subir" : ": bajar");
      Serial.print(" TRIM_PATADA de a 1 (ahora "); Serial.print(TRIM_PATADA); decir(").");
    }
  } else if (nota == NOTA_PROLIJO) {
    consejo("La patada salio derecha. Repetirla 3 veces: si da parecido, anotar los numeros en la bitacora y copiarlos con v.");
  } else if (nota == NOTA_ZIGZAGUEA) {
    Serial.print("#CONSEJO Zigzaguea ("); Serial.print(cruces);
    Serial.print(" cruces): corrige de mas. Bajar KP_PATADA de a 0.5 (ahora "); imprimirDecimal(KP_PATADA);
    Serial.print(") o subir KD_PATADA de a 0.1 (ahora "); imprimirDecimal(KD_PATADA); decir("). De a uno.");
  } else {
    // CASI o SE_TUERCE: se quedo torcida, o se torcio y volvio?
    bool quedoTorcida = (fabs(fin) > NOTA_FIN_PROLIJO && fin * pico > 0
                         && fabs(fin) * 2 >= fabs(pico));
    if (quedoTorcida) {
      Serial.print("#CONSEJO Se fue a "); Serial.print(haciaDonde(pico));
      Serial.print(" y no volvio (termino en "); imprimirGrados(fin);
      Serial.print(pico < 0 ? "): subir" : "): bajar");
      Serial.print(" TRIM_PATADA de a 1 (ahora "); Serial.print(TRIM_PATADA); decir(").");
    } else if (fabs(pico) > NOTA_PICO_PROLIJO) {
      Serial.print("#CONSEJO Se torcio "); imprimirGrados(pico);
      Serial.print(" grados a los "); Serial.print(picoMs);
      Serial.print(" ms y despues se enderezo: la pelota ve el pico. Subir KD_PATADA de a 0.1 (ahora ");
      imprimirDecimal(KD_PATADA); decir(").");
    } else {
      consejo("Esta cerca. Repetirla 2 o 3 veces: si da parecido, es asi.");
    }
    consejo("Si TODAS se van para el mismo lado es el TRIM_PATADA; si para los dos lados, KP_PATADA y KD_PATADA.");
  }
  if (TRASERA_TRABADA == 0)
    consejo("OJO: TRASERA_TRABADA = 0 no es como juega el partido: es solo para comparar.");
}

void consejoAvance(int nota) {
  if (nota == NOTA_PROLIJO) {
    Serial.print("#CONSEJO El avance va derecho (lo mas torcido: "); imprimirGrados(pico); decir(" grados).");
  } else {
    Serial.print("#CONSEJO El avance se torcio "); imprimirGrados(pico);
    Serial.print(" grados, para "); Serial.print(haciaDonde(pico));
    decir(". El avance del partido no corrige el rumbo: anotarlo en la bitacora.");
  }
}

void consejoEscape(int nota) {
  int sensor = mov - 2;
  if (fabs(pico) > ESCAPE_GIRA_MUCHO) {
    Serial.print("#CONSEJO El escape del sensor "); Serial.print(sensor);
    Serial.print(" giro "); imprimirGrados(pico);
    decir(" grados: tendria que salir sin girar. Avisar y anotarlo en la bitacora.");
  } else if (nota == NOTA_PROLIJO) {
    Serial.print("#CONSEJO El escape del sensor "); Serial.print(sensor);
    Serial.print(" sale sin girar (lo mas torcido: "); imprimirGrados(pico); decir(" grados).");
  } else {
    Serial.print("#CONSEJO El escape del sensor "); Serial.print(sensor);
    Serial.print(" giro un poco ("); imprimirGrados(pico);
    decir(" grados). Anotarlo y comparar con los otros dos sensores.");
  }
}

void consejoGiro(int nota) {
  bool orbita = (mov == 6);
  if (nota == NOTA_NO_ARRANCA) {
    Serial.print("#CONSEJO No arranco (giro "); imprimirGrados(girado);
    if (orbita) { Serial.print(" grados): subir MS_ORB_IMPULSO de a 50 (ahora "); Serial.print(MS_ORB_IMPULSO); }
    else        { Serial.print(" grados): subir VEL_GIRO_CONTINUO de a 5 (ahora "); Serial.print(VEL_GIRO_CONTINUO); }
    decir(").");
  } else if (nota == NOTA_SE_PLANTA) {
    Serial.print("#CONSEJO Se planto "); Serial.print(plantadoMax);
    if (orbita) { Serial.print(" ms casi sin girar: subir VEL_ORB_TRASERA de a 5 (ahora "); Serial.print(VEL_ORB_TRASERA); }
    else        { Serial.print(" ms casi sin girar: subir VEL_GIRO_CONTINUO de a 5 (ahora "); Serial.print(VEL_GIRO_CONTINUO); }
    decir(").");
  } else {
    Serial.print("#CONSEJO Gira bien, hacia "); Serial.print(haciaDonde(girado));
    Serial.print(": "); Serial.print(fabs(gradosPorSegundo()), 0);
    decir(" grados por segundo. Anotarlo en la bitacora.");
  }
}

void mandarConsejos(int nota, int comoTermino) {
  if (comoTermino == FRENO_TECLA)
    consejo("Lo frenaron antes de terminar: los numeros son de lo que llego a hacer. Repetir sin tocar nada.");
  else if (comoTermino == FRENO_SIN_GIRO)
    consejo("El giroscopio dejo de contestar: no se pudo medir. Revisar la bateria (prendida ANTES del USB) y el cable del giroscopio.");
  else if (comoTermino == FRENO_TOPE)
    consejo("Se corto por el tope de tiempo: el programa se trabo. Avisar.");

  if (huecoMax > HUECO_AVISO) {
    Serial.print("#CONSEJO El programa se clavo "); Serial.print(huecoMax);
    Serial.print(" ms (a los "); Serial.print(huecoMs); Serial.print(" ms)");
    if (mov == 1 || mov == 6) {
      if (SIN_ESPERA) decir(" aunque SIN_ESPERA = 1: es otra cosa. Avisar.");
      else            decir(": en ese rato no corrige nada. Probar SIN_ESPERA = 1.");
    } else {
      decir(": en este movimiento no deberia pasar. Avisar.");
    }
  }
  if (comoTermino != TERMINO_BIEN) return;

  if (mov == 1)                   consejoPatada(nota);
  else if (mov == 2)              consejoAvance(nota);
  else if (mov >= 3 && mov <= 5)  consejoEscape(nota);
  else                            consejoGiro(nota);
}

// Lo mismo, para personas, en una linea.
void resumenParaPersonas(int nota) {
  Serial.print("RESULTADO: "); Serial.print(mov); Serial.print(' ');
  Serial.print(NOMBRE_MOV[mov]); Serial.print(" -> "); Serial.print(NOMBRE_NOTA[nota]);
  if (mov <= 5) {
    Serial.print("   pico "); imprimirGrados(pico);
    Serial.print(" a los "); Serial.print(picoMs); Serial.print(" ms (200 ms: ");
    imprimirGrados(pico200); Serial.print(")   final "); imprimirGrados(fin);
    if (mov == 1) { Serial.print("   despues del retroceso "); imprimirGrados(fin2); }
    Serial.print("   cruces "); Serial.print(cruces);
  } else {
    Serial.print("   giro "); imprimirGrados(girado);
    Serial.print(" grados en "); Serial.print(duro); Serial.print(" ms (");
    Serial.print(gradosPorSegundo(), 1); Serial.print(" por segundo)   plantado ");
    Serial.print(plantadoMax); Serial.print(" ms");
  }
  Serial.print(mov == 1 ? "   el golpe duro " : "   duro ");
  Serial.print(duro); Serial.print(" ms   vuelta mas larga "); Serial.print(huecoMax); Serial.print(" ms");
  finDeLinea();
  if (mov == 1 && demoraEntrada > HUECO_AVISO) {
    Serial.print("   (ANTES de arrancar el golpe el programa estuvo clavado ");
    Serial.print(demoraEntrada); decir(" ms)");
  }
  decir("Grados: NEGATIVO = a la DERECHA, POSITIVO = a la IZQUIERDA.   ? = ayuda");
}

void mandarInforme(int comoTermino) {
  int nota = calcularNota(comoTermino);
  bool derecho = (mov <= 5);
  bool gira    = (mov >= 6);

  for (int i = 0; i < nMuestras; i++) {
    Serial.print("#M "); Serial.print(muestraT[i]); Serial.print(' ');
    imprimirGrados(muestraG[i]); Serial.print(' '); Serial.print(muestraF[i]);
    finDeLinea();
  }

  Serial.print("#R n="); Serial.print(mov);
  Serial.print(" mov="); Serial.print(NOMBRE_MOV[mov]);
  Serial.print(" nota="); Serial.print(NOMBRE_NOTA[nota]);
  campoGrados("pico",    pico,    derecho);
  campoEntero("picoms",  (long)picoMs, derecho);
  campoGrados("pico200", pico200, derecho);
  campoGrados("fin",     fin,     derecho);
  campoGrados("fin2",    fin2,    mov == 1);
  campoEntero("cruces",  cruces,  derecho);
  campoGrados("giro",    girado,  gira);
  Serial.print(" gps=");
  if (gira) Serial.print(gradosPorSegundo(), 1); else Serial.print('0');
  campoEntero("duro",     (long)duro,        true);
  campoEntero("hueco",    (long)huecoMax,    true);
  campoEntero("huecoms",  (long)huecoMs,     true);
  campoEntero("plantado", (long)plantadoMax, gira);
  finDeLinea();

  mandarConsejos(nota, comoTermino);
  resumenParaPersonas(nota);
}


// =====================================================================
//                  ARRANCAR, MOVER Y TERMINAR
// =====================================================================

// Vacia lo que haya llegado. Devuelve true si habia alguna tecla de verdad
// (un "\r" o "\n" sueltos no cuentan). De patada-derecha.
bool hayTeclaDeVerdad() {
  bool hay = false;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c != '\r' && c != '\n') hay = true;
  }
  return hay;
}

// Despues de un freno a tecla: tirar el resto de lo que se mando junto
// (ej. el "\n" de "x\n"), para que no se lea como una orden nueva.
void tirarLoQueLlego() {
  unsigned long t0 = millis();
  while (millis() - t0 < 50) { while (Serial.available()) Serial.read(); }
  largoLinea = 0;
  lineaLarga = false;
}

unsigned long duracionPedida() {
  switch (mov) {
    case 1: return (unsigned long)MS_PATADA + (unsigned long)MS_RETROCESO;
    case 2: return (unsigned long)MS_PRUEBA_AVANCE;
    case 3: case 4: case 5: return MS_RETROCESO_LINEA;
    case 6: return (unsigned long)MS_PRUEBA_ORBITA;
    case 7: return (unsigned long)MS_PRUEBA_GIRO;
  }
  return 0;
}

void terminarMovimiento(int comoTermino) {
  frenar();                                    // lo primero: quieto
  unsigned long ahora = millis();
  t_freno = ahora;                             // frena 400 ms y despues suelta
  modo = ESPERA;
  digitalWrite(LED, LOW);
  if (mov != 1 || fasePatada == 1) duro = ahora - t_inicio;   // la patada: lo que duro el golpe
  if (comoTermino == FRENO_TECLA) {
    decir("#FRENO tecla");
    tirarLoQueLlego();
  } else if (comoTermino == FRENO_SIN_GIRO) {
    decir("#FRENO sin giroscopio");
  } else if (comoTermino == FRENO_TOPE) {
    decir("#FRENO tope de tiempo");
  }
  mandarInforme(comoTermino);
}

// No arranco: ni #INICIO ni informe.
void noArranca(const char* motivo) {
  modo = ESPERA;
  digitalWrite(LED, LOW);
  Serial.print("#FRENO "); decir(motivo);
}

void empezarCuenta(int n) {
  if (!hayGiroscopo) {
    decir("Sin giroscopio. Lo busco de nuevo (tarda ~1 s)...");
    Serial.print("Giroscopio: ");
    hayGiroscopo = arrancarGiroscopo();
    if (hayGiroscopo) decir("OK, aparecio.");
    else              decir("NO CONTESTA.");
  }
  if (!hayGiroscopo) {
    noArranca("sin giroscopio");
    decir("No se mueve sin giroscopio. Bateria prendida ANTES del USB? Cable del giroscopio?");
    return;
  }
  mov = n;
  modo = CUENTA;
  t_cuenta = millis();
  avisosCuenta = 1;
  Serial.print(">>> "); Serial.print(n); Serial.print(' '); Serial.print(NOMBRE_MOV[n]);
  decir(" en 1,5 s. Saca la mano. Cualquier tecla lo frena.");
  decir("#CUENTA 1.5");
}

// Al terminar la cuenta. Deja todo listo y arranca.
void arrancarMovimiento() {
  reiniciarMedicion();

  if (mov == 1) {
    // ---- como empezarGolpe() de patada-derecha ----
    // En espera nadie le pregunta al giroscopio si sigue vivo con la
    // libreria, asi que se le pregunta aca. Tarda 200 ms, pero el robot
    // todavia esta quieto.
    if (hayGiroscopo) {
      uint8_t sys = 0, autotest = 0, err = 0;
      bno.getSystemStatus(&sys, &autotest, &err);
      giroCaido = (sys != 5);
    }
    // EN EL PARTIDO se llega a la patada desde la orbita, que le pregunta el
    // rumbo cada 20 ms: el chequeo de 200 ms se repite cada ~500-520 ms. Y
    // mientras esta clavado en el chequeo no puede decidir patear, asi que
    // al entrar a la patada el ultimo chequeo fue hace entre ~200 y ~520 ms.
    // Por eso en el partido el chequeo cae en los primeros ~300 ms del golpe
    // (o al entrar, todavia quieto, si ya pasaron mas de 500). Aca se sortea
    // esa EDAD del ultimo chequeo, entre 200 y 520 ms: random(a, b) da de a
    // hasta b-1. Igual que en pruebas/patada-derecha, para poder comparar.
    randomSeed(micros());
    t_chequeoGiro = millis() - random(200, MS_CHEQUEO_GIRO + 21);
    unsigned long t0 = millis();

    // ---- COPIA DEL PRINCIPAL: lo que hace cambiarA(PATEA_ADEL) ----
    rumboPatadaValido = giroscopoSano();
    if (rumboPatadaValido) rumboAlPatear = rumboActual();
    reiniciarRampaPatada();
    reiniciarDerivadaPatada();
    // ----------------------------------------------------------------

    demoraEntrada = millis() - t0;
    // El partido patearia igual, con el trim solo. La prueba no: sin
    // giroscopio no hay nada que medir.
    if (!rumboPatadaValido) { noArranca("sin giroscopio"); return; }
    rumbo0 = rumboAlPatear;
    fasePatada = 1;
  } else if (mov == 6) {
    giroCaido = (estadoGiroSinEspera() != 5);
    if (!giroscopoSano()) { noArranca("sin giroscopio"); return; }
    // La orbita del partido lee el rumbo con rumboActual() (con su chequeo
    // de 200 ms). Alla se llega a ORBITANDO desde AVANZANDO, donde casi
    // nadie le pregunta al giroscopio, asi que al entrar (cambiarA, con
    // rumboOrbPrevio = rumboActual()) casi siempre toca el chequeo, ANTES de
    // que empiece el golpe de arranque. El siguiente cae a los ~300 ms, justo
    // cuando el golpe termina. Aca se hace lo mismo, sin sortear: se fuerza
    // el chequeo ahora, con el robot quieto. (Sorteado, a veces caia en medio
    // del golpe y lo alargaba 200 ms: se media un golpe que el partido no da.)
    t_chequeoGiro = millis() - MS_CHEQUEO_GIRO - 1;   // "ya toca preguntarle"
    rumbo0 = rumboActual();          // como cambiarA(ORBITANDO): clava 200 ms, QUIETO
    if (!giroscopoSano()) { noArranca("sin giroscopio"); return; }
  } else {
    giroCaido = (estadoGiroSinEspera() != 5);
    if (!giroscopoSano()) { noArranca("sin giroscopio"); return; }
    rumbo0 = rumboSinChequeo();
  }
  rumboPrevio = rumbo0;
  if (mov == 7) pwmGiroBusc = 0;     // como el principal: la busqueda arranca suave

  // Ultima oportunidad de frenar ANTES de mover (llego algo durante el chequeo).
  if (hayTeclaDeVerdad()) { noArranca("tecla"); tirarLoQueLlego(); return; }

  Serial.print("#INICIO "); Serial.print(mov); Serial.print(' ');
  Serial.print(NOMBRE_MOV[mov]); finDeLinea();
  // De aca hasta terminar no se manda NADA por el cable.
  modo = MOVIENDO;
  t_inicio = millis();
  t_fase = t_inicio;
  msLoopPrevio = t_inicio;
  t_chequeoRapido = t_inicio;
  digitalWrite(LED, HIGH);
}

// ---------- 1 PATADA: golpe + retroceso ----------
void pasoPatada() {
  unsigned long enFase = millis() - t_fase;

  if (fasePatada == 1) {
    // ---- COPIA DEL PRINCIPAL: el estado PATEA_ADEL ----
    int vel = rampaPatada(VEL_PATADA);
    bool conLazo = (giroscopoSano() && rumboPatadaValido);
    if (conLazo && LAZO) avanzarDerecho(vel, rumboAlPatear);     // "&& LAZO" es de la prueba
    else                 motoresPatada(vel - TRIM_PATADA, vel + TRIM_PATADA);
    float errP = conLazo ? diferencia(rumboAlPatear, rumboActual()) : 0.0;
    // ----------------------------------------------------

    if (!conLazo) { terminarMovimiento(FRENO_SIN_GIRO); return; }
    anotarDesvio(errP, enFase);
    if (millis() - t_muestra >= MS_MUESTRA) {
      t_muestra = millis();
      guardarMuestra(enFase, errP, 'g');
    }
    if (enFase >= (unsigned long)MS_PATADA) {
      fasePatada = 2;                  // en el principal: cambiarA(PATEA_ATRAS), sin freno
      t_fase = millis();
      duro = t_fase - t_inicio;        // lo que duro DE VERDAD el golpe
    }
  } else {
    retroceder(VEL_RETROCESO);         // COPIA DEL PRINCIPAL: el estado PATEA_ATRAS

    // El retroceso del partido NO mira el giroscopio. Aca se lo lee aparte,
    // cada 10 ms y sin el chequeo de 200 ms: es solo para anotar.
    chequeoRapido(millis());
    if (!giroscopoSano()) { terminarMovimiento(FRENO_SIN_GIRO); return; }
    if (millis() - t_muestra >= MS_MUESTRA) {
      t_muestra = millis();
      float err = diferencia(rumboAlPatear, rumboSinChequeo());
      fin2 = err;
      guardarMuestra(duro + enFase, err, 'r');
    }
    if (enFase >= (unsigned long)MS_RETROCESO) terminarMovimiento(TERMINO_BIEN);
  }
}

// ---------- 2 a 5: AVANCE y ESCAPES (derechos, sin corrector) ----------
void pasoDerecho(unsigned long ahora, unsigned long enMov) {
  if (mov == 2) avanzar(VEL_AVANCE);                                // COPIA: AVANZANDO, de lejos
  else          escaparDeLinea(1 << (mov - 3), VEL_ESCAPE_FUERTE);  // COPIA: ESCAPA_LINEA

  chequeoRapido(ahora);
  if (!giroscopoSano()) { terminarMovimiento(FRENO_SIN_GIRO); return; }
  if (ahora - t_muestra >= MS_MUESTRA) {
    t_muestra = ahora;
    float e = diferencia(rumbo0, rumboSinChequeo());
    anotarDesvio(e, enMov);
    guardarMuestra(enMov, e, 'm');
  }
  unsigned long dura = (mov == 2) ? (unsigned long)MS_PRUEBA_AVANCE : MS_RETROCESO_LINEA;
  if (enMov >= dura) terminarMovimiento(TERMINO_BIEN);
}

// ---------- 6 ORBITA ----------
void pasoOrbita(unsigned long ahora, unsigned long enMov) {
  // COPIA DEL PRINCIPAL: la rama de ORBITANDO que empuja la orbita.
  bool enImpulso = (enMov < (unsigned long)MS_ORB_IMPULSO);
  orbitar(SENTIDO_ORBITA != 0, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);

  if (!enImpulso && !plantadoArmado) armarPlantado(ahora);
  if (ahora - t_muestra >= MS_MUESTRA) {
    t_muestra = ahora;
    float r = rumboActual();           // como el partido: con su chequeo
    if (!giroscopoSano()) { terminarMovimiento(FRENO_SIN_GIRO); return; }
    anotarGiro(r, millis());
    guardarMuestra(enMov, girado, 'm');
  }
  if (enMov >= (unsigned long)MS_PRUEBA_ORBITA) terminarMovimiento(TERMINO_BIEN);
}

// ---------- 7 GIRO de busqueda ----------
void pasoGiro(unsigned long ahora, unsigned long enMov) {
  girarContinuo(!GIRO_INVERTIDO);      // COPIA DEL PRINCIPAL: BUSCANDO con BUSCA_CONTINUO

  if (!plantadoArmado && pwmGiroBusc >= VEL_GIRO_CONTINUO) armarPlantado(ahora);
  chequeoRapido(ahora);
  if (!giroscopoSano()) { terminarMovimiento(FRENO_SIN_GIRO); return; }
  if (ahora - t_muestra >= MS_MUESTRA) {
    t_muestra = ahora;
    anotarGiro(rumboSinChequeo(), ahora);
    guardarMuestra(enMov, girado, 'm');
  }
  if (enMov >= (unsigned long)MS_PRUEBA_GIRO) terminarMovimiento(TERMINO_BIEN);
}

// Una vuelta del loop mientras se mueve.
void pasoDelMovimiento(unsigned long ahora) {
  unsigned long enMov = ahora - t_inicio;
  if (mov == 1)      pasoPatada();
  else if (mov <= 5) pasoDerecho(ahora, enMov);
  else if (mov == 6) pasoOrbita(ahora, enMov);
  else               pasoGiro(ahora, enMov);
}

void pasoDeLaCuenta(unsigned long ahora) {
  digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);
  if (hayTeclaDeVerdad()) {
    noArranca("tecla");
    decir("cancelado: no se movio");
    tirarLoQueLlego();
    return;
  }
  unsigned long pasado = ahora - t_cuenta;
  if (avisosCuenta == 1 && pasado >= 500)  { decir("#CUENTA 1.0"); avisosCuenta = 2; }
  if (avisosCuenta == 2 && pasado >= 1000) { decir("#CUENTA 0.5"); avisosCuenta = 3; }
  if (pasado >= MS_CUENTA) arrancarMovimiento();
}


// =====================================================================
//                           LAS ORDENES
// =====================================================================

void mandarError(const char* a, const char* b, const char* c) {
  Serial.print("#ERROR "); Serial.print(a); Serial.print(b); Serial.print(c);
  finDeLinea();
}

// "=NOMBRE valor" (llega sin el "=")
void cambiarNumero(char* s) {
  while (*s == ' ') s++;
  char* sep = s;
  while (*sep && *sep != ' ' && *sep != '=' && *sep != '\t') sep++;
  char* val = sep;
  if (*sep) { *sep = 0; val = sep + 1; }
  while (*val == ' ' || *val == '=' || *val == '\t') val++;

  if (*s == 0) { mandarError("falta el nombre. Se escribe: =NOMBRE valor   (ej: =TRIM_PATADA 7)", "", ""); return; }
  int i = buscarAjuste(s);
  if (i < 0)   { mandarError("no existe el numero ", s, ". Con ? salen todos los nombres"); return; }
  if (*val == 0) { mandarError("falta el valor de ", ajustes[i].nombre, ". Se escribe: =NOMBRE valor"); return; }
  if (strchr(val, ',')) { mandarError(ajustes[i].nombre, ": usar punto decimal (4.5, no 4,5)", ""); return; }

  char* resto;
  double v = strtod(val, &resto);
  while (*resto == ' ' || *resto == '\t') resto++;
  // (isnan/isinf: "nan" pasaria el control de rango, porque con NaN toda comparacion da falso)
  if (resto == val || *resto != 0 || isnan(v) || isinf(v)) {
    mandarError(ajustes[i].nombre, ": no es un numero: ", val); return;
  }
  if (ajustes[i].tipo != 'f' && fabs(v - (double)lround(v)) > 1e-6) {
    mandarError(ajustes[i].nombre, " va sin decimales", ""); return;
  }
  if (v < ajustes[i].minimo - 1e-6 || v > ajustes[i].maximo + 1e-6) {
    Serial.print("#ERROR "); Serial.print(ajustes[i].nombre);
    Serial.print(" fuera de rango: va de "); imprimirNumero(ajustes[i].minimo, ajustes[i].tipo);
    Serial.print(" a "); imprimirNumero(ajustes[i].maximo, ajustes[i].tipo);
    // Por que esos topes (ver arriba de la tabla):
    if (ajustes[i].p == &MS_RETROCESO_LINEA) {
      Serial.print(" (mas no: en el principal MS_CIEGO_LINEA = "); Serial.print(MS_CIEGO_LINEA);
      Serial.print(" tiene que durar por lo menos lo mismo)");
    } else if (ajustes[i].p == &MS_PRUEBA_ORBITA || ajustes[i].p == &MS_PRUEBA_GIRO) {
      Serial.print(" (mas no entra en el grafico)");
    }
    finDeLinea();
    return;
  }

  // SIN_ESPERA: antes de confiar en la lectura rapida se la compara con la
  // de la libreria. Si no dicen lo mismo, la rapida esta mal y no se prende.
  // (De patada-derecha, tecla c.)
  if (&SIN_ESPERA == (int*)ajustes[i].p && v > 0.5 && !SIN_ESPERA) {
    if (!hayGiroscopo) { mandarError("SIN_ESPERA necesita el giroscopio, y no contesta", "", ""); return; }
    uint8_t lento = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&lento, &autotest, &err);
    int rapido = estadoGiroSinEspera();
    if (rapido != (int)lento) {
      Serial.print("#ERROR SIN_ESPERA no se prende: la lectura rapida dio "); Serial.print(rapido);
      Serial.print(" y la de la libreria "); Serial.print(lento); Serial.print(". Avisar.");
      finDeLinea();
      return;
    }
  }

  ponerValor(i, (float)v);
  Serial.print("#OK "); Serial.print(ajustes[i].nombre); Serial.print('=');
  imprimirNumero(valorDe(i), ajustes[i].tipo);
  finDeLinea();
  mandarValores();
}

void procesarLinea(char* s) {
  // sacar espacios de los costados
  while (*s == ' ' || *s == '\t') s++;
  int n = strlen(s);
  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = 0;
  if (n == 0) return;

  if (s[0] == '=') { cambiarNumero(s + 1); return; }

  if (s[0] == '!') {
    if (s[1] == 0) { mandarError("falta el numero del movimiento: !1 a !7", "", ""); return; }
    char* resto;
    long k = strtol(s + 1, &resto, 10);
    while (*resto == ' ') resto++;
    if (resto == s + 1 || *resto != 0 || k < 1 || k > N_MOVIMIENTOS) {
      mandarError("no hay movimiento ", s + 1, ": son del !1 al !7");
      return;
    }
    empezarCuenta((int)k);
    return;
  }

  if (n == 1) {
    switch (s[0]) {
      case '?':
        mandarValores();
        ayuda();
        return;
      case 'h': case 'H':
        decir(SALUDO);
        return;
      case 'x': case 'X':
        frenar();
        t_freno = millis();            // frena 400 ms y despues suelta
        decir("PARADO (frena 0,4 s y suelta las ruedas)");
        return;
      case 'v': case 'V':
        mandarPanel();
        return;
      case '0':
        volverAlPrincipal();
        decir("Volvieron los numeros del PANEL DE CONTROL del principal (27/10).");
        mandarValores();
        return;
    }
  }
  mandarError("no entiendo '", s, "'. ? = ayuda");
}

// Junta lo que llega hasta el fin de linea, y recien ahi lo cumple.
void leerOrdenes() {
  while (modo == ESPERA && Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      if (lineaLarga)          mandarError("orden demasiado larga", "", "");
      else if (largoLinea > 0) { linea[largoLinea] = 0; procesarLinea(linea); }
      largoLinea = 0;
      lineaLarga = false;
    } else if (largoLinea < (int)sizeof(linea) - 1) {
      linea[largoLinea++] = c;
    } else {
      lineaLarga = true;
    }
  }
}


// =====================================================================
//                          SETUP Y LOOP
// =====================================================================

void setup() {
  // Los valores de arriba son los "del principal": la orden 0 vuelve a estos.
  for (int i = 0; i < N_AJUSTES; i++) ajustes[i].defecto = valorDe(i);

  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(BAUDIOS);
  Serial1.begin(19200);                // la camara
  while (!Serial && millis() < 3000) { }

  decir(SALUDO);
  decir("=============================================================");
  decir(" TABLERO DEL DELANTERO - probar y medir los movimientos");
  decir(" NO ES EL DE PARTIDO: no busca la pelota, espera ordenes.");
  decir(" Se mueve SOLO al pedir un movimiento (!1 a !7), 1,5 s despues.");
  decir(" NO MIRA LA LINEA BLANCA. En el piso, con lugar, cable flojo.");
  decir("=============================================================");

  // --- sensores de linea: COPIA DEL PRINCIPAL (la placa se ve en el pin 32) ---
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
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.print(pinLinea[2]);
  finDeLinea();

  // --- giroscopio ---
  Serial.print("Giroscopio: ");
  hayGiroscopo = arrancarGiroscopo();
  if (hayGiroscopo) decir(GIRO_SIN_BRUJULA ? "OK (modo IMU, sin brujula)" : "OK (modo NDOF, con brujula)");
  else              decir("NO CONTESTA. Sin giroscopio no se mueve (se puede mirar igual).");

  t_contadores = millis();
  mandarValores();
  ayuda();
}

void loop() {
  unsigned long ahora = millis();

  // ---------- MOVIENDOSE: lo minimo, y nada por el cable ----------
  if (modo == MOVIENDO) {
    // Cuanto tardo la vuelta anterior: si el programa se clavo, aca se ve.
    unsigned long hueco = ahora - msLoopPrevio;
    if (hueco > huecoMax) { huecoMax = hueco; huecoMs = msLoopPrevio - t_inicio; }
    msLoopPrevio = ahora;

    if (hayTeclaDeVerdad()) { terminarMovimiento(FRENO_TECLA); return; }   // cualquier tecla = FRENO
    leerCamara();
    if (ahora - t_inicio > duracionPedida() + MS_TOPE_EXTRA) { terminarMovimiento(FRENO_TOPE); return; }
    pasoDelMovimiento(ahora);
    return;
  }

  // ---------- QUIETO (espera o cuenta) ----------
  leerCamara();
  // Al terminar un movimiento frena de verdad un momento y despues suelta,
  // para poder acomodarlo a mano.
  if (t_freno != 0 && ahora - t_freno < MS_FRENO_AL_TERMINAR) frenar();
  else                                                        parar();

  if (modo == CUENTA) { pasoDeLaCuenta(ahora); return; }

  digitalWrite(LED, LOW);
  leerOrdenes();

  if (ahora - t_contadores >= 1000) {
    paquetesPorSegundo = nPaquetes * 1000UL / (ahora - t_contadores);
    nPaquetes = 0;
    nTirados  = 0;
    t_contadores = ahora;
  }
  if (modo == ESPERA && ahora - t_estado >= MS_ESTADO) {
    t_estado = ahora;
    mandarEstado();
  }
}
