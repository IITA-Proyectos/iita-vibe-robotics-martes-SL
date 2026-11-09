/* =====================================================================
   TABLERO DEL ARQUERO — probar los movimientos del PARTIDO de a uno,
   medirlos con el giroscopio, ajustar los numeros SIN RECARGAR, y medir
   una cancha nueva sin internet
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-03
   =====================================================================

   QUE ES
   Es el programa del ROBOT que va con la VENTANA DEL TABLERO del arquero
   (pruebas/tablero/tablero.py, se abre con TABLERO.bat). La ventana le
   manda ordenes cortas por el cable USB y este programa las cumple: mueve
   el robot, mide y contesta. Tambien se usa SIN la ventana, desde
   cualquier consola serie a 19200, escribiendo las mismas ordenes.

   Es la idea del tablero del DELANTERO (27/10), pero con LOS MOVIMIENTOS
   DEL ARQUERO: los del programa de partido de hoy (0.1103, con MIRAR
   ANTES DE SALIR), cada uno por separado, con una NOTA y un CONSEJO de que
   numero tocar (la idea de pruebas/ajustar-movimientos). Y dos cosas que
   el delantero no tiene:
     - el MODO SIN CABLE: en la cancha no hay USB. Se pide la prueba, hay
       10 s para desenchufar, el robot la hace solo, la GUARDA en la
       EEPROM y el LED dice la nota. Al volver a enchufar (sin apagar la
       bateria) la ventana la pide y la muestra.
     - las MEDICIONES DE CANCHA: la linea, el ancho, el fondo, la camara e
       ir al medio, para preparar una cancha nueva sin internet.

   LOS 10 MOVIMIENTOS (todo lo que mueve el robot es COPIA DEL PARTIDO)
     1 COSTADO_IZQ   de costado a la IZQUIERDA, lento (velLenta)
     2 COSTADO_DER   de costado a la DERECHA, lento (la prueba de lateralInvertido)
     3 RAPIDO_IZQ    de costado a la IZQUIERDA a pwmMaxLateral
     4 RAPIDO_DER    de costado a la DERECHA a pwmMaxLateral
     5 IDA_Y_VUELTA  rapido a la izquierda y se da vuelta SIN frenar
     6 DESPEJE       el despeje derecho completo del partido, sin pelota
     7 UBICARSE      el arranque del partido: atras hasta la linea, enderezarse
                     y el poquito (SIN la rutina de ir al medio)
     8 SEGUIR        seguir la pelota de costado como el partido, sin despejar
     9 ATAJAR        seguir + MIRAR + despejar en diagonal, como el partido.
                     Termina en el primer despeje que sale
    10 IDA           solo la ida del despeje (msPruebaIda): frena y se queda,
                     para medir con regla cuanto sale
   LAS 5 MEDICIONES DE CANCHA
     L  la linea (4 s por fase: verde, blanco IZQ, blanco DER; o guiada por
        el LED, sin cable). No mueve.
     A  el ancho y MS_MEDIO_AL_MEDIO (la logica de medir-ancho). MUEVE.
     F  el fondo: MS_MEDIO_FONDO_POR_TIEMPO (con TOCO) o comprobarlo. MUEVE.
     C  3 s de camara con esta luz. No mueve.
     I  la rutina de ir al medio del partido, para comprobar. MUEVE.

   QUE MIDE
   En cada vuelta del programa, el giroscopio dice cuanto se torcio contra
   el rumbo que tenia al terminar la cuenta. Al terminar (NUNCA mientras se
   mueve, para no cambiarle el ritmo al programa) manda todo junto: las
   muestras para el grafico, la NOTA (PROLIJO, CASI, ZIGZAGUEA, SE_TUERCE,
   NO_SALIO o NO_VALE), el pico, los cruces, como termino, cuanto corrigio,
   la vuelta del programa mas larga (el HUECO: un retardo hace zigzaguear
   toquen lo que toquen), y en la 9 cada intento de despeje (si salio o por
   que no). Grados: NEGATIVO = se fue a la DERECHA, POSITIVO = a la
   IZQUIERDA.

   COMO SE USA
   CON CABLE
     1. Robot en el PISO, con lugar. BATERIA PRENDIDA y DESPUES el USB (el
        giroscopio come de la bateria).
     2. TABLERO.bat: carga este programa y abre la ventana.
     3. Elegir un movimiento y PROBAR. Cuenta 3 s y arranca. Cualquier tecla
        (o PARAR) lo frena.
     4. Mirar la nota y el consejo. Repetir 3 veces SIN TOCAR NADA. Recien
        ahi cambiar UN numero.
     5. Lo que ande: "Copiar al panel" (la orden v) y pegarlo en el PANEL DE
        CONTROL del partido. Anotar todo en la bitacora.
     6. Al terminar: VOLVER AL PARTIDO (o CARGAR-PARTIDO.bat). Con este
        programa cargado el robot NO juega.
   SIN CABLE (en la cancha no hay USB)
     1. Tildar SIN CABLE y PROBAR (la orden termina en " c").
     2. Cuenta 10 s: DESENCHUFAR EL USB en ese rato.
     3. El robot hace la prueba solo y frena solo. Desenchufar NO lo frena:
        se frena con PARAR (si el cable esta), con su tope, o al terminar.
        Sin cable, para frenarlo de golpe: levantarlo o apagar la bateria.
     4. Al terminar el LED dice la nota: 1 destello PROLIJO, 2 CASI,
        3 ZIGZAGUEA, 4 SE_TUERCE, 5 NO_SALIO; temblando = NO VALE.
     5. Enchufar SIN APAGAR LA BATERIA: la ventana lo pide sola y lo
        muestra. (Si se apago igual queda guardado, con menos muestras.)

   QUE NO HACE, A PROPOSITO
   - NUNCA arranca solo: se mueve solo con una orden "!", despues de la
     cuenta, y con tope de tiempo. No hay ARMANDOSE ni patrulla.
   - No escribe el REGISTRO DE DESPEJES del partido (EEPROM 3072..3583): el
     renglon #MIRE lo reemplaza.
   - Si su Teensy no es el del ARQUERO (serie 13348550) no mueve NADA: los
     pines del delantero estan cruzados.
   - Sin giroscopio no se mueve: lo que mide es el giroscopio.

   🚨 ESTE PROGRAMA MUEVE EL ROBOT. En el piso, nunca sobre la mesa, con
   lugar, y el cable flojo (o sin cable).

   ⚠️ LO QUE NO ES IGUAL AL PARTIDO (la ventana lo dice)
   - El rumbo que sostiene es el de cuando TERMINA LA CUENTA, no el del armado.
   - No hay patrulla. La rutina de ir al medio solo esta en !I (y en la
     vuelta del lateral de 8 y 9, como el partido).
   - No escribe el registro de despejes.

   ---------------------------------------------------------------------
   ⚠ COPIAS DEL PRINCIPAL
   ---------------------------------------------------------------------
   Lo marcado "COPIA del partido" es copia de
        funciona/seguir-y-despejar/seguir-y-despejar.ino  version 0.1103
   y los numeros arrancan con el valor de su PANEL DE CONTROL. SI EL
   PARTIDO CAMBIA, CAMBIARLO ACA TAMBIEN: si no, el tablero mide un robot
   que ya no existe. Lo UNICO que se cambio de las copias esta marcado
   "// TABLERO (2026-11-03): Mn" (M1 a M13 del DISENO.md, seccion 7.3).

   ---------------------------------------------------------------------
   🚨 LA EEPROM ES UNA SOLA Y LA COMPARTEN TODOS LOS PROGRAMAS DEL ARQUERO
   ---------------------------------------------------------------------
        0 ..  255   medir-ancho / medir-ancho-sin-giro     (aca: NO se escribe)
      256 ..  767   derecho-y-vuelta                       (no se toca)
      768 .. 1023   libre
     1024 .. 2047   ajustar-movimientos                    (no se toca)
     2048 .. 2943   TABLERO: el ultimo MOVIMIENTO  (magia "ATM1")
     2944 .. 3071   TABLERO: la ultima MEDICION DE CANCHA (magia "ATC1")
     3072 .. 3583   registro de despejes del partido 0.1103 (NUNCA se escribe)
     3584 .. 4283   libre
   La EEPROM se escribe SOLO con el robot quieto y frenado.

   ---------------------------------------------------------------------
   EL CONTRATO CON LA VENTANA (no cambiarlo sin cambiar el otro lado tambien)
   ---------------------------------------------------------------------
   Es la seccion 2 de pruebas/tablero/DISENO.md, que es la que manda.

   TRANSPORTE. USB a 19200. Cada renglon termina en "\n" solo. Un renglon
   es para la ventana si empieza con "#"; los demas son para personas.
   Campos NOMBRE=valor separados por UN espacio, listas con comas, punto
   decimal, sin "-0.0". El robot no imprime tildes. Las ordenes son ASCII
   de 71 caracteres como mucho (el bufer es de 72). Los nombres de los
   numeros son los del panel (umbralCm, KP_RUMBO) y se comparan SIN
   distinguir mayusculas. El robot habla SOLO si hay alguien escuchando
   (if (Serial)), y mientras se mueve no manda NADA, salvo los #PASO de A e
   I con el robot frenado entre tramo y tramo.
   Las 26 etiquetas: HOLA S V DEF OK ERROR GIRO CUENTA INICIO PASO FRENO M
   MIRE R USADOS CONSEJO FIN REPITE GUARDADO LIN ANCHO FONDO CAM MEDIO
   PANEL FINPANEL.

   CODIGOS: 1 COSTADO_IZQ, 2 COSTADO_DER, 3 RAPIDO_IZQ, 4 RAPIDO_DER,
   5 IDA_Y_VUELTA, 6 DESPEJE, 7 UBICARSE, 8 SEGUIR, 9 ATAJAR, 10 IDA;
   L (LINEA_VERDE, LINEA_BLANCO_IZQ, LINEA_BLANCO_DER, LINEA_GUIADA),
   A ANCHO, F (FONDO, FONDO_COMPROBAR), C CAMARA, I IR_AL_MEDIO.

   PC -> ROBOT (solo se leen en ESPERA)
     !1 .. !10        la prueba con cable (cuenta 3 s)
     !1 c .. !10 c    lo mismo SIN CABLE (cuenta 10 s). !6c = !6 c
     !L v|i|d         medir la linea 4 s (no mueve, sin cuenta)
     !L c             la linea GUIADA por el LED, sin cable
     !A [c]  !F [c]  !C [c]  !I [c]     (en !F la "c" es COMPROBAR, sin cable;
                                         midiendo con !F, la letra t = TOCO)
     x                en ESPERA: frena 400 ms y suelta
     =NOMBRE valor    cambiar un numero en la RAM (tambien =NOMBRE=valor)
     ?  ayuda (#V, #DEF y texto)     v  lineas para el panel (#PANEL)
     0  volver a los del partido     h/H  saludo (#HOLA + 2 #GUARDADO)
     g  buscar el giroscopio         r  repetir el ultimo MOVIMIENTO
     rc repetir la ultima MEDICION DE CANCHA
   En CUENTA o MOVIENDO, el primer byte que no sea \r ni \n FRENA y no se
   interpreta (despues se tira lo que llegue durante 50 ms).

   ROBOT -> PC
     #HOLA tablero-arquero 2026-11-03 serie=13348550 partido=0.1103
         (en setup, con h, y cada vez que se abre el puerto en ESPERA;
          siempre lo siguen 2 #GUARDADO)
     #GUARDADO que=mov|cancha hay=0
     #GUARDADO que=mov hay=1 n= nombre= nota= sincable= nuevo= mismo= hace=
     #GIRO buscando  /  #GIRO ok=1 estado=5
     #S t= l=izq,der,ade u= r= g= cal=s,g,a,m p=Xp,Yp,edad d= y= vy= va= ve=
        n= dg=sale,motivo,avance,lateral,msIda am=X,Y,edad az=X,Y,edad q= e=ESPERA
         (cada 100 ms, solo en ESPERA; y + = DERECHA)
     #V NOMBRE=valor ... (46, en el orden de la tabla)   #DEF igual, los del partido
     #OK NOMBRE=valor   /   #ERROR texto (textos fijos, ver cambiarNumero)
     #CUENTA 3.0 cable (0,5 s) / #CUENTA 10.0 sincable (1 s)
     #INICIO codigo NOMBRE tope=ms sincable=0|1
     #PASO A calibrando giro=N / #PASO A tramo= ms= desvio= endereza=
     #PASO I paso= ms= como=L|T
     #FRENO tecla | tope de tiempo | sin giroscopio | sin camara |
            sobre la linea | recien parado | no es el arquero
     #M t grados fase        (fases: l s m i p a f e c v q)
     #MIRE k= t= res= miro= cuadros= d= y= vy= va= av= lat= ms= ang= recto=
     #R n mov nota como pico picoms fin medio cruces fuera3 corr tope mudas
        racha hueco huecoms duro lineams idams atrasms acomodo laterales
        idas arranques conpelota zonamuerta velmedia intentos sale  (29, fijos)
     #USADOS NOMBRE=valor ...   #CONSEJO texto   #FIN codigo (el ULTIMO)
     #REPITE codigo NOMBRE sincable= apagado=
     #LIN fase= n= izq=min,max,prom der= ade=
     #ANCHO ok= error= medioizq= izqder= derizq= almedioizq= almedioder=
            desvio=a,b,c,d endereza= golpe= vel= rampa= umbral= calgiro=
     #FONDO ms= como=TOCO|LINEA|TOPE|TIEMPO|PARADO desvio= umbral= probado=
     #CAM n= cero= val= xp=min,max,prom yp= am= az=
     #MEDIO ok= pasos=p1..p8 como=c1..c8 desvio= acomodo=
     #PANEL declaracion nombre = valor; ... #FINPANEL

   SECUENCIAS
     normal:  >>> texto, #CUENTA..., #INICIO, (silencio), #M..., (#MIRE... en
              la 9), #R, #USADOS, #CONSEJO..., RESULTADO: ..., #FIN n
     tecla en la cuenta:  #CUENTA..., #FRENO tecla, "cancelado: no se movio"
     tecla moviendose:    #INICIO, #FRENO tecla, la tanda con NO_VALE, #FIN n
     no arranca:          #FRENO motivo + texto, sin #CUENTA
     sin giro al terminar la cuenta: #CUENTA..., #FRENO sin giroscopio
     tope:                #INICIO, #FRENO tope de tiempo, la tanda con NO_VALE
     SIN CABLE: #CUENTA 10.0 sincable..., se desenchufa (es lo esperado), hace
              todo solo, lo guarda (nuevo=1); al enchufar: #HOLA + #GUARDADO
              nuevo=1 -> la ventana manda r -> #REPITE + la tanda + #FIN.
              Una prueba SIN CABLE queda nuevo=1 aunque la tanda haya
              salido en vivo (el Teensy no sabe si lo desenchufaron:
              ver terminarPrueba). Solo r / rc la pasan a nuevo=0.
     cancha:  igual, pero la tanda es #LIN/#ANCHO/#FONDO/#CAM/#MEDIO + #FIN.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

const char*    SALUDO_PROGRAMA = "tablero-arquero";
const char*    FECHA_CONTRATO  = "2026-11-03";
const char*    PARTIDO_COPIADO = "0.1103";      // la VERSION del partido de la que son las copias
const uint32_t SERIE_ARQUERO   = 13348550;      // el numero de serie USB del Teensy del ARQUERO

// ---- ARQUERO (ROBOT1): COPIA del partido ----
// ⚠️ El DELANTERO tiene estos mismos pines CRUZADOS (IZQ 8/7/6, DER 11/12/4,
// TRA 2/5/3): este programa en el delantero moveria las ruedas equivocadas.
// Por eso se niega a mover si su serie no es la del arquero (ver soyElArquero).
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
#define PIN_AVISO 13

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12

const unsigned long BAUDIOS = 19200;


// =====================================================================
//   TIPOS PROPIOS. Van ANTES de cualquier funcion, a proposito:
//   PlatformIO pone sus declaraciones automaticas antes de la PRIMERA
//   funcion que encuentra, y si una de esas declaraciones usa un tipo que
//   todavia no aparecio, no compila (paso en ajustar-movimientos y en el
//   tablero del delantero).
// =====================================================================

enum Modo { ESPERA, CUENTA, MOVIENDO };

// El enum Fase del partido, COPIA, con las fases de la prueba agregadas al
// final (TABLERO 2026-11-03). APAGADO y ARMANDOSE quedan en la lista para
// que la copia sea igual, pero el tablero NUNCA los usa (M12).
enum Fase { APAGADO, ARMANDOSE, ESPERANDO, SIGUIENDO,
            ADELANTE, PAUSA_MEDIO, ATRAS_HASTA_LINEA, FRENANDO,
            ACOMODANDO, ACOMODO_ASENTAR, ADELANTE_CHICO, ENFRIANDO,
            UBIC_ATRAS, UBIC_FRENANDO,
            MEDIO_PAUSA, MEDIO_AL_LATERAL, MEDIO_ALEJARSE, MEDIO_AL_FONDO,
            MEDIO_ADELANTE, MEDIO_AL_LATERAL_2, MEDIO_AL_MEDIO,
            MEDIO_AL_AREA, MEDIO_DESPEGARSE,
            LATERAL_AL_MEDIO, MIRANDO,
            // ---- TABLERO (2026-11-03): las fases de la prueba ----
            COSTADO, COSTADO_VUELTA,     // 1 a 5 (de ajustar-movimientos)
            P_FRENO, P_QUIETO,           // el freno y el quieto del final
            P_LINEA, P_ANCHO, P_FONDO, P_FONDO_FRENO, P_CAMARA };   // cancha

// COPIA del partido (0.1103): por que salio o no salio un despeje.
enum Resultado { R_SALE_MIRANDO, R_SALE_SIN_MIRAR, R_MUY_ABIERTO, R_NO_ALCANZA,
                 R_NO_VIENE, R_POCOS_CUADROS, R_CANTIDAD };

// La nota (seccion 4.4 del diseno) y como termino.
enum Nota { NOTA_NINGUNA, NOTA_PROLIJO, NOTA_CASI, NOTA_ZIGZAGUEA, NOTA_SE_TUERCE,
            NOTA_NO_SALIO, NOTA_NO_VALE };
enum Como { COMO_NORMAL, COMO_LATERAL, COMO_SIN_LINEA, COMO_NO_SALIO, COMO_TECLA, COMO_TOPE };

const int MAX_MIRE            = 10;    // intentos de despeje que se guardan (los ultimos)
const int MAX_USADOS_EEPROM   = 12;    // numeros cambiados que entran en la EEPROM
const int MAX_MUESTRAS_EEPROM = 100;   // muestras del grafico que entran en la EEPROM

// Un numero de la tabla (para "=NOMBRE valor", "?", "v" y "0").
struct Ajuste {
  const char* nombre;      // el MISMO nombre del panel del partido
  char        tipo;        // 'i' int, 'u' unsigned long, 'f' float, 'b' bool (0/1)
  void*       p;           // la variable (para los bool, apunta a un bool)
  float       minimo, maximo;
  const char* enElPanel;   // como se declara en el panel ("" = solo de esta prueba)
  float       defecto;     // se llena en setup() con el valor de arriba
};

// Un intento de despeje de la 9: un renglon #MIRE. 24 bytes.
struct Intento {
  uint16_t k;              // numero de intento (1..)
  uint16_t t;              // ms desde el #INICIO
  uint8_t  res;            // un Resultado
  uint8_t  miro;           // 1 = paso por MIRANDO
  uint8_t  cuadros;        // cuadros de la mirada (0 = no miro)
  uint8_t  recto;          // si salio DERECHO, por que (0 = diagonal o no salio)
  int16_t  d, y, vy, va;   // decimas de cm y de cm/s (y + = derecha)
  int16_t  av, lat;        // avance y costado con que salio (0 = no salio)
  uint16_t ms;             // msIda con que salio
  int16_t  ang;            // decimas de grado que NECESITABA (+ = derecha)
};

// La cabecera de los dos bloques de la EEPROM. 12 bytes.
struct Cabecera {
  uint32_t magia;          // "ATM1" o "ATC1": si no esta, es basura
  uint8_t  version;        // 1
  uint8_t  codigo;         // 1..10, o 'L' 'A' 'F' 'C' 'I'
  uint8_t  nuevo;          // 1 = nadie lo vio todavia
  uint8_t  sinCable;       // 1 = se pidio sin cable
  uint16_t largo;          // sizeof de lo guardado (para validar)
  uint16_t libre;          // 0
};

// El resultado de un MOVIMIENTO, tal como se guarda (y tal como vive en la
// RAM: las muestras de la RAM, que son hasta 1200, van aparte).
struct DatosMov {
  uint8_t  n, como, nota, sale;
  uint8_t  acomodo, laterales, fuera3, tope;
  uint8_t  conpelota, zonamuerta, picoFase, nIntentos;
  uint8_t  nUsados, usadosDeMas, nMuestras, sinLinea;
  int16_t  pico, fin, medio, corr, velmedia;        // decimas (corr: PWM)
  uint16_t picoms, cruces, mudas, racha, hueco, huecoms, duro;
  uint16_t lineams, idams, atrasms, idas, arranques, intentos;
  uint16_t inicioMov, muestras, topeMs;
  uint16_t contRes[R_CANTIDAD];                     // cuantos intentos de cada Resultado
  Intento  mire[MAX_MIRE];
  uint8_t  usadoIdx[MAX_USADOS_EEPROM];
  float    usadoVal[MAX_USADOS_EEPROM];
  uint16_t mT[MAX_MUESTRAS_EEPROM];                 // las muestras raleadas
  int16_t  mG[MAX_MUESTRAS_EEPROM];
  char     mF[MAX_MUESTRAS_EEPROM];
};
struct BloqueMov { Cabecera c; DatosMov d; };

// Las mediciones de cancha.
struct LinFase    { char fase; uint8_t libre; uint16_t n; uint16_t mn[3], mx[3], prom10[3]; };
struct DatosLin   { uint8_t nFases, libre; LinFase f[3]; };
struct DatosAncho { uint8_t error, endereza, calgiro, libre;
                    uint16_t medioizq, izqder, derizq, almedioizq, almedioder, vel, rampa, umbral;
                    int16_t desvio[4]; int16_t golpe; };
struct DatosFondo { uint8_t como, libre; uint16_t ms, umbral, probado; int16_t desvio; };
struct DatosCam   { uint16_t n, cero, val, am, az; uint8_t xpmin, xpmax; uint16_t xpprom10; int16_t yp10; };
struct DatosMedio { uint8_t acomodo, libre; uint16_t pasos[8]; char como[8]; int16_t desvio; };
struct DatosCancha {
  char    codigo;          // 'L' 'A' 'F' 'C' 'I'
  char    variante;        // L: v i d c; F: 0 medir, c comprobar; los demas 0
  uint8_t ok;              // 1 = OK, 0 = ERROR
  uint8_t libre;
  union { DatosLin lin; DatosAncho an; DatosFondo fo; DatosCam ca; DatosMedio me; } u;
};
struct BloqueCancha { Cabecera c; DatosCancha d; };

// ⚠ NO MOVER. Es la PRIMERA funcion del archivo a proposito: PlatformIO
// mete sus declaraciones automaticas aca, despues de los tipos y ANTES de
// la tabla de numeros (a ajustar-movimientos se las metia adentro de su
// tabla y no compilaba).
void paraQueCompile() {}


// =====================================================================
//   LA EEPROM DEL TABLERO (ver el mapa arriba)
// =====================================================================
const int      DIR_MOV      = 2048;
const int      TAM_MOV      = 896;          // 2048 .. 2943
const int      DIR_CANCHA   = 2944;
const int      TAM_CANCHA   = 128;          // 2944 .. 3071
const uint32_t MAGIA_MOV    = 0x41544D31;   // "ATM1"
const uint32_t MAGIA_CANCHA = 0x41544331;   // "ATC1"
const uint8_t  VERSION_EEPROM = 1;

static_assert(sizeof(Intento) == 24, "Intento tiene que ocupar 24 bytes");
static_assert(sizeof(BloqueMov) <= TAM_MOV, "el bloque de movimiento no entra en 2048..2943");
static_assert(sizeof(BloqueCancha) <= TAM_CANCHA, "el bloque de cancha no entra en 2944..3071");
static_assert(DIR_CANCHA + TAM_CANCHA <= 3072, "el tablero NUNCA escribe el registro del partido (3072..)");


// =====================================================================
//   LOS 46 NUMEROS QUE SE CAMBIAN DESDE LA VENTANA
// =====================================================================
//   Valores por defecto: los del PANEL DE CONTROL del partido 0.1103
//   (funciona/seguir-y-despejar). Se cambian con "=NOMBRE valor" SIN
//   recargar, y la orden "0" vuelve a ESTOS. Viven en la RAM: al apagar
//   el robot vuelven solos a los del partido.
//   TABLERO (2026-11-03): M1. Los que en el panel son "const" aca son
//   variables con el MISMO nombre, para que el codigo copiado compile sin
//   tocarlo. La orden "v" igual escribe la declaracion del panel (con const).
// =====================================================================

// ---------- 2. lo que se mide en cada cancha ----------
int           umbralBlanco              = 425;    // del PANEL DE CONTROL de la 0.1103
float         CAMARA_POR_CM             = 2.87;   // del PANEL DE CONTROL de la 0.1103
unsigned long MS_MEDIO_AL_MEDIO         = 2228;   // del PANEL DE CONTROL de la 0.1103
unsigned long MS_MEDIO_FONDO_POR_TIEMPO = 2000;   // del PANEL DE CONTROL de la 0.1103
// ---------- 3. seguir la pelota de costado ----------
float         kpLateral                 = 11.5;   // del PANEL DE CONTROL de la 0.1103
int           pwmMinLateral             = 45;     // del PANEL DE CONTROL de la 0.1103
int           pwmMaxLateral             = 150;    // del PANEL DE CONTROL de la 0.1103
float         ZONA_MUERTA_PELOTA        = 1.4;    // del PANEL DE CONTROL de la 0.1103
bool          predecirTrayectoria       = false;  // del PANEL DE CONTROL de la 0.1103 (seccion 1)
float         msAnticipacionSeguir      = 500;    // del PANEL DE CONTROL de la 0.1103
int           LADO_TRASERA              = 89;     // del PANEL DE CONTROL de la 0.1103
bool          volverAlMedioEnElLateral  = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
// ---------- 4. el despeje ----------
float         umbralCm                  = 30.0;   // del PANEL DE CONTROL de la 0.1103
float         umbralDesvio              = 20.0;   // del PANEL DE CONTROL de la 0.1103
int           potenciaDespeje           = 200;    // del PANEL DE CONTROL de la 0.1103
int           msAdelante                = 383;    // del PANEL DE CONTROL de la 0.1103
bool          frenarAntesDeVolver       = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
unsigned long MS_ESPERA_ANTES_DE_VOLVER = 400;    // del PANEL DE CONTROL de la 0.1103
int           potenciaRetroceso         = 110;    // del PANEL DE CONTROL de la 0.1103
unsigned long msFreno                   = 200;    // del PANEL DE CONTROL de la 0.1103
bool          empujonFinal              = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
int           potenciaEmpujon           = 100;    // del PANEL DE CONTROL de la 0.1103
int           msEmpujonFinal            = 320;    // del PANEL DE CONTROL de la 0.1103
bool          avanzarAlUbicarse         = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
// ---------- 4b. el despeje en diagonal ----------
bool          despejeEnDiagonal         = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
bool          predecirEnDiagonal        = true;   // del PANEL DE CONTROL de la 0.1103 (seccion 1)
float         rendimientoCostado        = 0.80;   // del PANEL DE CONTROL de la 0.1103. ⚠️ AL REVES: mas chico = MAS costado
float         MAX_ANGULO_DIAGONAL       = 30.0;   // del PANEL DE CONTROL de la 0.1103
// ---------- 4c. mirar antes de salir ----------
bool          medirAntesDeDespejar      = true;   // del PANEL DE CONTROL de la 0.1103
float         distanciaParaMirar        = 40.0;   // del PANEL DE CONTROL de la 0.1103 (no 45: eso era la 0.1027c)
unsigned long MS_ASENTAR_MIRADA         = 60;     // del PANEL DE CONTROL de la 0.1103
unsigned long MS_MIRAR                  = 120;    // del PANEL DE CONTROL de la 0.1103
unsigned long MS_MAX_MIRANDO            = 500;    // del PANEL DE CONTROL de la 0.1103
unsigned long MS_DESCANSO_MIRADA        = 500;    // del PANEL DE CONTROL de la 0.1103
// ---------- 5. ir derecho ----------
float         KP_RUMBO                  = 5.0;    // del PANEL DE CONTROL de la 0.1103
float         KI_RUMBO                  = 1.5;    // del PANEL DE CONTROL de la 0.1103
float         LIMITE_INTEGRAL           = 30.0;   // del PANEL DE CONTROL de la 0.1103
int           MAX_CORRECCION_RUMBO      = 100;    // del PANEL DE CONTROL de la 0.1103
float         ZONA_MUERTA_RUMBO         = 1.0;    // del PANEL DE CONTROL de la 0.1103
int           RAMPA_MOV_PASO            = 10;     // del PANEL DE CONTROL de la 0.1103
// ---------- 9. para que lado ----------
bool          lateralInvertido          = false;  // del PANEL DE CONTROL de la 0.1103
bool          camaraYInvertida          = true;   // del PANEL DE CONTROL de la 0.1103
// ---------- solo de esta prueba (no existen en el partido) ----------
int           velLenta                  = 70;     // solo de esta prueba: de costado LENTO (1 y 2)
int           msCostado                 = 1000;   // solo de esta prueba: cuanto dura cada tramo de costado (ms)
int           msSeguir                  = 10000;  // solo de esta prueba: cuanto dura SEGUIR y cuanto espera ATAJAR (ms)
int           msPruebaIda               = 383;    // solo de esta prueba: cuanto dura la IDA del movimiento 10 (ms)

// ---------- FIJOS: COPIA del partido 0.1103. Aca NO se cambian ----------
const bool          enderezarActivado            = true;
const bool          perseguirEnElDespeje         = false;
const bool          irAlMedioSiNoHayLinea        = true;
const bool          patrullaPorElLateral         = true;
const float         kpPersecucion                = 6.0;
const int           PWM_MAX_PERSECUCION          = 60;
const float         msAnticipacion               = 0;
const int           VECES_PARA_CREERLE           = 3;
const unsigned long MS_PAUSA_MEDIO               = 150;
const unsigned long MS_MAX_RETROCESO             = 1200;
const unsigned long MS_ENFRIAMIENTO              = 1500;
const unsigned long MS_QUIETO_PARA_PREDECIR      = 400;
const float         PWM_POR_CM_S                 = 2.0;
const float         PISO_VELOCIDAD               = 4.0;
const float         SUAVIZADO_VELOCIDAD          = 0.3;
const int           PRESUPUESTO_RUEDA            = 210;
const int           PWM_MIN_AVANCE_DESPEJE       = 140;
const unsigned long MS_MAX_ADELANTE              = 800;
const int           COSTADO_MINIMO_UTIL          = 20;
const int           LADO_FRENTE                  = 50;
const unsigned long RAMPA_MOV_MS                 = 10;
// seccion 6: enderezarse
const float         TOLERANCIA_ACOMODO           = 3.0;
const float         KP_ACOMODO                   = 1.5;
const int           PWM_MIN_ACOMODO              = 75;
const int           PWM_MAX_ACOMODO              = 85;
const unsigned long MS_ASENTAR_ACOMODO           = 300;
const unsigned long MS_TIMEOUT_ACOMODO           = 3000;
const int           MAX_REINTENTOS_ACOMODO       = 3;
const int           RAMPA_PASO                   = 3;
const unsigned long RAMPA_MS                     = 15;
const float         TOLERANCIA_ESPERA            = 8.0;
const unsigned long MS_CHEQUEO_ESPERA            = 300;
const unsigned long MS_DESCANSO_ENDEREZAR        = 6000;
// seccion 7: ir al medio y limite lateral
const int           CUADROS_PARA_CORTAR_PATRULLA = 5;
const int           PWM_CUENTA_COMO_MOVERSE      = 60;
const int           LECTURAS_PARA_CREER_LATERAL  = 3;
const int           VEL_MEDIO_BUSCAR             = 110;
const int           VEL_MEDIO_MEDIR              = 100;
const int           VEL_MEDIO_RECTO              = 110;
const int           LADO_MEDIO                   = -1;
const unsigned long MS_MEDIO_ALEJARSE            = 400;
const unsigned long MS_MEDIO_ADELANTE            = 943;
const unsigned long MS_MEDIO_TOPE_LATERAL        = 6000;
const unsigned long MS_MEDIO_QUIETO              = 300;
// seccion 8: giroscopio
const unsigned long MS_ESPERA_DATOS_GIRO         = 5000;
const unsigned long MS_PARPADEO_AVISO            = 50;


// =====================================================================
//   LA TABLA DE LOS NUMEROS: el orden es el de #V, #DEF y la ventana
//   (seccion 3 del diseno). Los rangos son lo que acepta "=NOMBRE valor".
// =====================================================================
Ajuste ajustes[] = {
  { "umbralBlanco",              'i', &umbralBlanco,              100,  1000,  "int",                 0 },
  { "CAMARA_POR_CM",             'f', &CAMARA_POR_CM,             2.0,  4.0,   "const float",         0 },
  // MS_MEDIO_AL_MEDIO hasta 3200 y no 6000 (2026-11-03, revision): en !I
  // el paso 6 va de costado ESE tiempo sin mirar la linea, desde el
  // lateral. Un cruce entero son ~4400 ms: con un numero mal escrito
  // (4228 en vez de 2228) se iba de la cancha. !A nunca puede medir mas de
  // ~3150 (msHastaElMedio de un cruce de 6000, el tope del tramo).
  { "MS_MEDIO_AL_MEDIO",         'u', &MS_MEDIO_AL_MEDIO,         500,  3200,  "const unsigned long", 0 },
  { "MS_MEDIO_FONDO_POR_TIEMPO", 'u', &MS_MEDIO_FONDO_POR_TIEMPO, 500,  6000,  "const unsigned long", 0 },
  { "kpLateral",                 'f', &kpLateral,                 5,    20,    "float",               0 },
  { "pwmMinLateral",             'i', &pwmMinLateral,             30,   80,    "int",                 0 },
  { "pwmMaxLateral",             'i', &pwmMaxLateral,             80,   170,   "int",                 0 },
  { "ZONA_MUERTA_PELOTA",        'f', &ZONA_MUERTA_PELOTA,        1,    4,     "const float",         0 },
  { "predecirTrayectoria",       'b', &predecirTrayectoria,       0,    1,     "bool",                0 },
  { "msAnticipacionSeguir",      'f', &msAnticipacionSeguir,      0,    800,   "float",               0 },
  { "LADO_TRASERA",              'i', &LADO_TRASERA,              80,   100,   "const int",           0 },
  { "volverAlMedioEnElLateral",  'b', &volverAlMedioEnElLateral,  0,    1,     "bool",                0 },
  { "umbralCm",                  'f', &umbralCm,                  20,   45,    "float",               0 },
  { "umbralDesvio",              'f', &umbralDesvio,              3,    30,    "float",               0 },
  { "potenciaDespeje",           'i', &potenciaDespeje,           140,  230,   "int",                 0 },
  { "msAdelante",                'i', &msAdelante,                100,  800,   "int",                 0 },
  { "frenarAntesDeVolver",       'b', &frenarAntesDeVolver,       0,    1,     "bool",                0 },
  { "MS_ESPERA_ANTES_DE_VOLVER", 'u', &MS_ESPERA_ANTES_DE_VOLVER, 0,    800,   "const unsigned long", 0 },
  { "potenciaRetroceso",         'i', &potenciaRetroceso,         80,   140,   "int",                 0 },
  { "msFreno",                   'u', &msFreno,                   100,  400,   "unsigned long",       0 },
  { "empujonFinal",              'b', &empujonFinal,              0,    1,     "bool",                0 },
  { "potenciaEmpujon",           'i', &potenciaEmpujon,           60,   150,   "int",                 0 },
  { "msEmpujonFinal",            'i', &msEmpujonFinal,            0,    600,   "int",                 0 },
  { "avanzarAlUbicarse",         'b', &avanzarAlUbicarse,         0,    1,     "bool",                0 },
  { "despejeEnDiagonal",         'b', &despejeEnDiagonal,         0,    1,     "bool",                0 },
  { "predecirEnDiagonal",        'b', &predecirEnDiagonal,        0,    1,     "bool",                0 },
  { "rendimientoCostado",        'f', &rendimientoCostado,        0.4,  1.2,   "float",               0 },
  { "MAX_ANGULO_DIAGONAL",       'f', &MAX_ANGULO_DIAGONAL,       10,   45,    "const float",         0 },
  { "medirAntesDeDespejar",      'b', &medirAntesDeDespejar,      0,    1,     "bool",                0 },
  { "distanciaParaMirar",        'f', &distanciaParaMirar,        25,   60,    "float",               0 },
  { "MS_ASENTAR_MIRADA",         'u', &MS_ASENTAR_MIRADA,         0,    300,   "const unsigned long", 0 },
  { "MS_MIRAR",                  'u', &MS_MIRAR,                  40,   600,   "const unsigned long", 0 },
  { "MS_MAX_MIRANDO",            'u', &MS_MAX_MIRANDO,            200,  1500,  "const unsigned long", 0 },
  { "MS_DESCANSO_MIRADA",        'u', &MS_DESCANSO_MIRADA,        0,    2000,  "const unsigned long", 0 },
  { "KP_RUMBO",                  'f', &KP_RUMBO,                  1,    10,    "const float",         0 },
  { "KI_RUMBO",                  'f', &KI_RUMBO,                  0,    3,     "const float",         0 },
  { "LIMITE_INTEGRAL",           'f', &LIMITE_INTEGRAL,           10,   40,    "const float",         0 },
  { "MAX_CORRECCION_RUMBO",      'i', &MAX_CORRECCION_RUMBO,      40,   120,   "const int",           0 },
  { "ZONA_MUERTA_RUMBO",         'f', &ZONA_MUERTA_RUMBO,         0.25, 3,     "const float",         0 },
  { "RAMPA_MOV_PASO",            'i', &RAMPA_MOV_PASO,            2,    20,    "const int",           0 },
  { "lateralInvertido",          'b', &lateralInvertido,          0,    1,     "bool",                0 },
  { "camaraYInvertida",          'b', &camaraYInvertida,          0,    1,     "bool",                0 },
  { "velLenta",                  'i', &velLenta,                  40,   150,   "",                    0 },
  { "msCostado",                 'i', &msCostado,                 300,  1500,  "",                    0 },
  { "msSeguir",                  'i', &msSeguir,                  3000, 20000, "",                    0 },
  { "msPruebaIda",               'i', &msPruebaIda,               100,  800,   "",                    0 },
};
const int N_AJUSTES = sizeof(ajustes) / sizeof(ajustes[0]);
static_assert(sizeof(ajustes) / sizeof(ajustes[0]) == 46, "tienen que ser los 46 numeros de la tabla 3");

// Los nombres de las pruebas, de las notas, de como termino y de los intentos.
const char* NOMBRE_MOV[11] = { "", "COSTADO_IZQ", "COSTADO_DER", "RAPIDO_IZQ", "RAPIDO_DER",
                               "IDA_Y_VUELTA", "DESPEJE", "UBICARSE", "SEGUIR", "ATAJAR", "IDA" };
const char* NOMBRE_NOTA[7] = { "?", "PROLIJO", "CASI", "ZIGZAGUEA", "SE_TUERCE", "NO_SALIO", "NO_VALE" };
const char* NOMBRE_COMO[6] = { "NORMAL", "LATERAL", "SIN_LINEA", "NO_SALIO", "TECLA", "TOPE" };
const char* CODIGO_RESULTADO[R_CANTIDAD] = { "SALE_MIRANDO", "SALE_SIN_MIRAR", "MUY_ABIERTO",
                                             "NO_ALCANZA", "NO_VIENE", "POCOS_CUADROS" };
// Como termino el fondo (!F).
const uint8_t FO_TOCO = 0, FO_LINEA = 1, FO_TOPE = 2, FO_TIEMPO = 3, FO_PARADO = 4;
const char* NOMBRE_FONDO[5] = { "TOCO", "LINEA", "TOPE", "TIEMPO", "PARADO" };

// ---------- tiempos y criterios de la prueba ----------
const unsigned long MS_CUENTA                  = 3000;   // cuenta con cable
const unsigned long MS_CUENTA_SIN_CABLE        = 10000;  // cuenta sin cable: para desenchufar
const unsigned long MS_QUIETO_FINAL            = 500;    // la fase q del final
const unsigned long MS_FRENO_AL_TERMINAR       = 400;    // en ESPERA frena esto y despues suelta
const unsigned long MS_ESPERA_DESPUES_DE_PARAR = 1500;   // "recien parado": una tecla de mas no lo arranca
const unsigned long MS_ESTADO                  = 100;    // cada cuanto el #S
const unsigned long HUECO_RETARDO              = 40;     // ms: una vuelta mas larga es un RETARDO
const unsigned long MS_RETARDO_ARRANQUE        = 33;     // medir-ancho: lo que tarda el motor (04/08)
const float         NOTA_MAX_PROLIJO           = 4.0;    // criterios de ajustar-movimientos
const int           NOTA_CRUCES_OK             = 2;
const float         NOTA_FINAL_OK              = 3.0;
const int           NOTA_CRUCES_ZIGZAG         = 4;
const float         NOTA_MAX_TUERCE            = 10.0;
const float         BANDA_CRUCE                = 2.0;
#define MAX_MUESTRAS 1200


// =====================================================================
//   ESTADO COPIADO DEL PARTIDO (mismos nombres, mismos valores iniciales)
// =====================================================================
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool giroscopoRespondiendo = false;
unsigned long t_chequeoGiro = 0;
float rumboBase = -1;                  // el rumbo que hay que sostener (en ESPERA vale -1)
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;
unsigned long t_chequeoEspera = 0;
unsigned long t_ultimoIntentoEnderezar = 0;
bool enderezandoEnEspera = false;
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;
int reintentosAcomodo = 0;
bool ubicandose = false;
bool encontroLaLinea = false;
bool empujonDeArranque = false;
unsigned long t_ultimoMovimiento = 0;
unsigned long t_soltoLinea = 0;
bool patrullando = false;
int  cuadrosConPelota = 0;
int lecturasDeLateral = 0;
int ladoDelLateral = 0;
Fase fase = APAGADO;
unsigned long t_fase = 0;
Fase faseDespuesDePausa = ESPERANDO;
int vecesSeguidas = 0;
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;
float velocidadLateral    = 0;         // cm/s, + = a la derecha
float velocidadAcercamiento = 0;       // cm/s, + = se viene encima
int   nMirada = 0;
float sT = 0, sY = 0, sD = 0, sTT = 0, sTY = 0, sTD = 0;
float vyMirada = 0;
float vaMirada = 0;
bool  hayMirada = false;
unsigned long t_finMirada = 0;
int   motivoNoSale = 0;                // 1 el avance no alcanza, 2 muy abierto
int lateralDelDespeje  = 0;
int pwmAvanceDespeje   = 200;
unsigned long msIdaDespeje = 533;
float desvioAnterior = 0;
float distanciaAnterior = 0;
unsigned long t_desvioAnterior = 0;
bool  hayDesvioAnterior = false;
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;
unsigned long t_ultimoCostado = 0;

// TABLERO (2026-11-03): M2. Lo que anota la ultima lectura del giroscopio,
// para medir. Se lee UNA vez por vuelta y se mide con ese mismo valor:
// medir no puede cambiar lo que se mide (la leccion de medir-ancho).
float ultimoError = 0;
int   ultimaCorreccion = 0;
bool  ultimaMuda = false;
bool  leyoGiroEnEstaVuelta = false;    // = en esta vuelta se llamo a correccionDeRumbo()

// TABLERO (2026-11-03): M3. Lo que vio calcularDiagonal() la ultima vez.
float diagD = 0, diagY = 0, diagVy = 0, diagVa = 0, diagCostado = 0, diagAngulo = 0;
int   diagAvance = 0;
unsigned long diagMsIda = 0;
uint8_t diagRecto = 0;                 // 0 diagonal, 1 costado chico, 2 diagonal apagada, 3 sin giro, 4 no la veia

// TABLERO (2026-11-03): M4. Lo que leerCamara() guarda de mas.
int XpBueno = 0, YpBueno = 0;          // la ultima vez que vio la pelota (Xp > 0), crudos
unsigned long t_pelotaBuena = 0;
int Xam = 0, Yam = 0, Xaz = 0, Yaz = 0;   // los arcos, crudos (Y ya con el -100)
unsigned long t_amarillo = 0, t_azul = 0;
unsigned long nPaquetes = 0, paquetesPorSegundo = 0, t_contadores = 0;


// =====================================================================
//   ESTADO DEL TABLERO
// =====================================================================
Modo modo = ESPERA;
int  prueba = 0;                       // 1..10, o 'L' 'A' 'F' 'C' 'I'
char variante = 0;                     // L: v i d c; F: 0 medir, c comprobar
bool sinCablePedido = false;
unsigned long t_cuenta = 0;
unsigned long msDeCuenta = 0;
int  avisosCuenta = 0;
unsigned long t_inicio = 0;            // cuando arranco (el #INICIO)
unsigned long topePrueba = 0;          // el tope duro de esta prueba (ms)
unsigned long t_freno = 0;             // cuando termino la ultima prueba (0 = nunca)
unsigned long t_parada = 0;            // cuando lo frenaron con una tecla (0 = nunca)
unsigned long t_estado = 0;            // ultimo #S
char linea[72];                        // la orden que se esta escribiendo
int  largoLinea = 0;
bool lineaLarga = false;
bool estabaConectado = false;          // para saber cuando se abre el puerto
uint32_t serieMia = 0;
bool soyElArquero = false;
int  ladoCostado = 0, velCostado = 0;  // 1 a 5
Como comoFinal = COMO_NORMAL;          // como termina (lo decide quien pasa a P_FRENO / P_QUIETO)
// la vista previa de la diagonal (el dg= del #S)
int  dgSale = 1, dgMotivo = 0, dgAvance = 200, dgLateral = 0;
unsigned long dgMs = 383;

// ---------- lo que se mide en un movimiento ----------
uint16_t muestraT[MAX_MUESTRAS];       // ms desde el #INICIO
int16_t  muestraG[MAX_MUESTRAS];       // decimas de grado
char     muestraF[MAX_MUESTRAS];       // la fase
int      nMuestras = 0;
unsigned long t_muestra = 0;
unsigned long msMuestra = 10;
float pico = 0;
unsigned long picoMs = 0;
char  picoFase = 'q';
int   ladoDelError = 0;
unsigned long cruces = 0;
unsigned long muestrasEnMov = 0, fuera3 = 0, alTope = 0;
long  sumaCorr = 0;
float sumaErr = 0;
unsigned long mudas = 0, racha = 0, peorRacha = 0;
unsigned long huecoMax = 0, huecoMs = 0, msLoopPrevio = 0;
bool  saltearHueco = false;
bool  enMovAnterior = false;
float sumaFinal = 0;
int   nFinal = 0;
float peorEnMov = 0;                   // el peor |desvio| moviendose (F e I)
unsigned long idaMs = 0, atrasMs = 0, lineaMs = 0;
bool  sinLinea = false;
int   acomodoMax = 0, laterales = 0;
unsigned long vueltasSeguir = 0, conPelota = 0, enZonaMuerta = 0, idas = 0, arranques = 0;
float sumaVel = 0;
bool  seMovia = false, ladoSeguir = true;
bool  salio = false;                   // 9: ya salio a despejar
unsigned long inicioMov = 0;           // 9: cuando arranco la ida (ms desde el #INICIO)
float usadosRAM[46];                   // los numeros CON LOS QUE se corrio
// los intentos de la 9 (los ultimos MAX_MIRE, en anillo)
Intento anillo[MAX_MIRE];
unsigned long nIntentosTotal = 0;
uint16_t contRes[R_CANTIDAD];
uint8_t ultimoRes = 0;
unsigned long t_ultimoIntento = 0;

// ---------- los resultados guardados ----------
BloqueMov bMov;                        // el ultimo MOVIMIENTO (bMov.d vive tambien en la RAM)
bool hayMov = false, movMismo = false, movNuevo = false;
unsigned long t_finMov = 0;
BloqueCancha bCancha;                  // la ultima MEDICION DE CANCHA
bool hayCancha = false, canchaMismo = false, canchaNuevo = false;
unsigned long t_finCancha = 0;
char ultimoResultado = 0;              // 'm' o 'c': lo que muestra el LED (de este encendido)

// ---------- las mediciones de cancha ----------
// L
int  linMin[3], linMax[3];
uint32_t linSuma[3], linN = 0;
unsigned long t_linMuestra = 0;
int  linPaso = 0;
DatosLin linAcum;
// A
const int AN_CALIBRA = 0, AN_TRAMO = 1, AN_PAUSA = 2, AN_ENDEREZA = 3, AN_PAUSA2 = 4;
int   anPaso = AN_CALIBRA, anTramo = 0, anDir = -1, anCeros = 0, anEndereza = 0, anError = 0, anCalGiro = 0;
unsigned long anMsTramo = 0, anMsUltimo = 0, t_anCal = 0, t_anAviso = 0;
unsigned long anMedioIzq = 0, anIzqDer = 0, anDerIzq = 0;
float anDesvio[4], anGolpe = 0;
// F
uint8_t foComo = FO_TOPE;
unsigned long foMs = 0;
// C
bool camMidiendo = false;
unsigned long camN = 0, camCero = 0, camVal = 0, camAm = 0, camAz = 0, camXpSuma = 0;
long camYpSuma = 0;
int  camXpMin = 255, camXpMax = 0;
// I
uint16_t mePasos[8];
char     meComo[8];


// =====================================================================
//                    MOTORES — COPIA del partido
// =====================================================================

// SOLTAR los motores. No es frenar: el robot sigue de largo por inercia.
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// FRENO ELECTRICO: las dos patas en BAJO con el PWM al MAXIMO (medido el
// 18/08). Mismo estado de las patas que parar(), efecto opuesto: no confundirlas.
void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

// Un valor CON SIGNO por rueda. Positivo = pata A alta.
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
  // El reloj de "hace cuanto que no me muevo" de la patrulla. Aca no hay
  // patrulla, pero se deja igual para que la copia no cambie.
  if (abs(v1) >= PWM_CUENTA_COMO_MOVERSE || abs(v2) >= PWM_CUENTA_COMO_MOVERSE
      || abs(v3) >= PWM_CUENTA_COMO_MOVERSE) {
    t_ultimoMovimiento = millis();
  }
}

// Arranque suave (18/08): subiendo la potencia de a poco las ruedas no patinan.
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


// =====================================================================
//                     RUMBO — COPIA del partido
// =====================================================================

// El rumbo, o -1 si el sensor esta mudo (un numero imposible: 0.0 es a la
// vez un rumbo valido y el sintoma de que el chip no contesta).
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
// TABLERO (2026-11-03): M2. Ademas anota ultimaMuda, ultimoError,
// ultimaCorreccion y leyoGiroEnEstaVuelta (como ajustar-movimientos). La
// cuenta es la misma del partido.
int correccionDeRumbo() {
  ultimaCorreccion = 0;                          // TABLERO M2
  if (!enderezarActivado || rumboBase < 0) return 0;
  float r = rumboActual();
  leyoGiroEnEstaVuelta = true;                   // TABLERO M2
  if (r < 0) { ultimaMuda = true; return 0; }    // TABLERO M2: anota la muda
  ultimaMuda = false;                            // TABLERO M2

  float error = diferencia(rumboBase, r);
  ultimoError = error;                           // TABLERO M2

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;

  // Anti-windup: el acumulador tiene tope.
  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  ultimaCorreccion = (int)c;                     // TABLERO M2
  return (int)c;
}


// =====================================================================
//                     LINEA — COPIA del partido
// =====================================================================
bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= umbralBlanco; }
bool algunoDeAtrasVeBlanco() { return veBlancoIzq() || veBlancoDer(); }

// ¿Pisa el lateral de ese lado? (-1 izquierdo, +1 derecho). Solo el sensor
// de ese lado en blanco y el otro en verde (los dos = la linea del area).
bool pisaElLateral(int lado) {
  bool izq = veBlancoIzq();
  bool der = veBlancoDer();
  return (lado < 0) ? (izq && !der) : (der && !izq);
}


// =====================================================================
//                 MOVERSE — COPIA del partido
// =====================================================================

// s > 0 pide ir a la DERECHA, s < 0 a la izquierda. La fuerza pasa por la
// rampa; la correccion de rumbo NO (tiene que actuar enseguida).
void moverDeCostado(int s) {
  if (s != 0) t_ultimoCostado = millis();
  if (lateralInvertido) s = -s;

  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;

  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;

  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}

// Avanzar sosteniendo el rumbo, con arranque suave.
void adelanteControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(+p + c, -p + c, 0 + c);
}

// Avanzar y correrse de costado a la vez. El costado sube con la rampa,
// junto con el avance (revision del 27/10).
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


// =====================================================================
//                    CAMARA — COPIA del partido
// =====================================================================
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
      t_ultimoPaquete = millis();

      // TABLERO (2026-11-03): M4. Ademas guarda los arcos (el partido no
      // los usa: son solo para mirar), la ultima pelota buena, cuenta los
      // paquetes (para el q del #S) y le suma a la medicion de !C si esta
      // andando. No cambia nada de lo que ya hacia.
      nPaquetes++;
      int xAm = paquete[4], yAm = paquete[5] - 100;
      int xAz = paquete[7], yAz = paquete[8] - 100;
      if (xAm > 0) { Xam = xAm; Yam = yAm; t_amarillo = t_ultimoPaquete; }
      if (xAz > 0) { Xaz = xAz; Yaz = yAz; t_azul = t_ultimoPaquete; }
      if (Xp > 0)  { XpBueno = Xp; YpBueno = Yp; t_pelotaBuena = t_ultimoPaquete; }
      if (camMidiendo) anotarCamara(xAm, xAz);

      float dCm  = Xp / CAMARA_POR_CM;
      float dLat = (camaraYInvertida ? -(float)Yp : (float)Yp) / CAMARA_POR_CM;

      // ---- velocidad lateral y de acercamiento de la pelota ----
      if (Xp > 0) {
        if (cuadrosConPelota < 1000) cuadrosConPelota++;
        unsigned long t = millis();
        if (hayDesvioAnterior) {
          float dt = (t - t_desvioAnterior) / 1000.0;
          if (dt > 0.005 && dt < 0.5) {
            float v = (dLat - desvioAnterior) / dt;
            velocidadLateral = velocidadLateral * (1.0 - SUAVIZADO_VELOCIDAD)
                             + v * SUAVIZADO_VELOCIDAD;
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

// Desvio de la pelota en cm reales: positivo = a la DERECHA del robot.
float desvioPelota() {
  float y = camaraYInvertida ? -(float)Yp : (float)Yp;
  return y / CAMARA_POR_CM;
}

// Adonde VA A ESTAR la pelota (solo con predecirTrayectoria). Debajo del
// piso de velocidad, la velocidad es ruido y se toma como cero.
float desvioPredicho() {
  if (!predecirTrayectoria) return desvioPelota();
  float v = velocidadLateral;
  if (fabs(v) < PISO_VELOCIDAD) v = 0;
  return desvioPelota() + v * (msAnticipacionSeguir / 1000.0);
}

// ¿Con que angulo tengo que salir, y llego? COPIA del partido.
// TABLERO (2026-11-03): M3. Al principio pone en cero lo que anota, y antes
// de cada return anota lo que vio (diagCostado ANTES de que
// COSTADO_MINIMO_UTIL lo pase a 0; diagRecto segun por que salio derecho).
// No cambia ninguna cuenta. Asi el angulo del informe es el de ESTA pasada
// (el problema 3 de ajustar-movimientos).
bool calcularDiagonal(int &avance, int &lateral, unsigned long &msIda) {
  avance  = potenciaDespeje;
  lateral = 0;
  msIda   = msAdelante;
  diagD = 0; diagY = 0; diagVy = 0; diagVa = 0; diagCostado = 0; diagAngulo = 0;   // TABLERO M3
  diagRecto = 0; diagAvance = avance; diagMsIda = msIda;                            // TABLERO M3

  if (!despejeEnDiagonal) { diagRecto = 2; return true; }     // modo viejo: sale derecho (TABLERO M3: anota)
  if (!veLaPelota())      { diagRecto = 4; return true; }     // sin datos, sale derecho (TABLERO M3: anota)

  // Sin giroscopio, derecho (revision del 27/10).
  if (!enderezarActivado || rumboBase < 0 || !giroscopoRespondiendo) { diagRecto = 3; return true; }   // TABLERO M3: anota

  float d = distanciaPelota();
  if (d < 1.0) d = 1.0;                    // no dividir por cero
  float y  = desvioPelota();               // + = a la derecha
  motivoNoSale = 0;
  float vy = hayMirada ? vyMirada : velocidadLateral;
  if (fabs(vy) < PISO_VELOCIDAD) vy = 0;   // eso es ruido, no movimiento
  float va = hayMirada ? vaMirada : 0;     // acercamiento: solo si lo midio quieto
  if (va < PISO_VELOCIDAD) va = 0;         // (alejandose no suma)

  // La velocidad de la pelota, solo con el robot quieto (la camara va arriba
  // del robot). Con la mirada estuvo quieto a proposito.
  if (!predecirEnDiagonal || (!hayMirada && millis() - t_ultimoCostado < MS_QUIETO_PARA_PREDECIR)) vy = 0;
  if (!predecirEnDiagonal) va = 0;
  diagD = d; diagY = y; diagVy = vy; diagVa = va;   // TABLERO M3

  // MOTIVO 1: la pelota esta corrida (regla de tres, no necesita calibrar).
  float k = y / d;
  // MOTIVO 2: la pelota se esta corriendo sola (2 de PWM por cada cm/s).
  float m = vy * PWM_POR_CM_S;
  // MOTIVO 3 (con la mirada): se viene encima, se encuentran antes.
  m += k * va * PWM_POR_CM_S;

  // ¿Entra en el motor? avance + 0,5 x costado <= PRESUPUESTO.
  float r = rendimientoCostado;
  float denom = 1.0 + 0.5 * fabs(k) / r;
  float techo = (PRESUPUESTO_RUEDA - 0.5 * fabs(m) / r) / denom;

  if (techo < avance) avance = (int)techo;
  diagAvance = avance;                                     // TABLERO M3

  // Si no entra, se achican los dos juntos: si el avance no alcanza, NO SALE.
  if (avance < PWM_MIN_AVANCE_DESPEJE) { motivoNoSale = 1; return false; }

  float costado = (k * avance + m) / r;
  diagCostado = costado;                                   // TABLERO M3: antes del COSTADO_MINIMO_UTIL
  diagAngulo  = atan(costado * r / avance) * 180.0 / PI;   // TABLERO M3: el angulo que necesitaba

  // Tope de angulo (27/10): mas abierto que MAX_ANGULO_DIAGONAL, NO SALE.
  float tangenteSalida = fabs(costado * r / avance);
  if (tangenteSalida > tan(MAX_ANGULO_DIAGONAL * PI / 180.0)) { motivoNoSale = 2; return false; }

  if (fabs(costado) < COSTADO_MINIMO_UTIL) diagRecto = 1;   // TABLERO M3: anota por que sale derecho
  if (fabs(costado) < COSTADO_MINIMO_UTIL) costado = 0;     // sale derecho
  lateral = (int)costado;

  // Si el avance bajo, la ida se estira para recorrer la misma distancia.
  msIda = (unsigned long)((float)msAdelante * potenciaDespeje / avance);
  if (msIda > MS_MAX_ADELANTE) msIda = MS_MAX_ADELANTE;
  diagMsIda = msIda;                                       // TABLERO M3
  return true;
}

// Volver por la MISMA diagonal por la que salio.
void atrasEnDiagonal(int potencia, int lateral) {
  if (lateralInvertido) lateral = -lateral;

  int p  = rampa(potencia);
  int c  = correccionDeRumbo();
  int fr = (lateral * LADO_FRENTE)  / 100;
  int tr = (lateral * LADO_TRASERA) / 100;

  aplicar(-p + fr + c, +p + fr + c, -tr + c);
}


// =====================================================================
//           MIRAR ANTES DE SALIR — COPIA del partido (0.1103)
// =====================================================================
void empezarMirada() {
  nMirada = 0; sT = sY = sD = sTT = sTY = sTD = 0;
  hayMirada = false;
}

void agregarMuestraMirada(unsigned long t, float y, float d) {
  float s = (t - t_fase) / 1000.0;
  nMirada++; sT += s; sY += y; sD += d; sTT += s * s; sTY += s * y; sTD += s * d;
}

// Pendientes de las dos rectas (cuadrados minimos). false = no hay datos.
bool calcularMirada() {
  if (nMirada < 3) return false;
  float den = nMirada * sTT - sT * sT;
  if (den < 1e-6) return false;
  vyMirada =  (nMirada * sTY - sT * sY) / den;
  vaMirada = -(nMirada * sTD - sT * sD) / den;   // la distancia BAJA al acercarse
  return true;
}


// =====================================================================
//       LOS PASOS DEL PARTIDO — COPIA del partido
// =====================================================================

// Se llama al terminar de enderezarse.
void pasarAEsperar() {
  fase = ESPERANDO;
  t_fase = millis();
  vecesSeguidas = 0;
}

// La ubicacion inicial: atras hasta la linea y enderezarse.
void arrancarUbicacion() {
  ubicandose      = true;
  encontroLaLinea = false;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  fase = UBIC_ATRAS; t_fase = millis();
}

// Entre un paso y el otro de la rutina de ir al medio: frenar, soltar y
// quedarse quieto un momento (sin delay, para no dejar de leer la camara).
void pausaMedio(Fase siguiente) {
  frenar();
  faseDespuesDePausa = siguiente;
  fase = MEDIO_PAUSA; t_fase = millis();
}

// La rutina no cerro: frena, se endereza y juega desde donde quedo.
void abandonarMedio() {
  encontroLaLinea = false;
  frenar();
  fase = UBIC_FRENANDO; t_fase = millis();
}

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

// Aparecio la pelota volviendo del lateral: a jugar.
void cortarPatrulla() {
  patrullando = false;
  ubicandose  = false;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  fase = SIGUIENDO; t_fase = millis();
}

// Piso el lateral siguiendo la pelota: frena y vuelve al medio por tiempo.
void arrancarVueltaDelLateral(int lado) {
  ladoDelLateral    = lado;
  lecturasDeLateral = 0;
  patrullando       = false;
  ubicandose        = true;
  encontroLaLinea   = false;
  pausaMedio(LATERAL_AL_MEDIO);
}

void pasarAEnderezarse() {
  parar();
  fase = ACOMODANDO; t_fase = millis();
  reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = millis();
}

// Se llama cuando la camara ya dijo 3 cuadros seguidos que la pelota esta
// cerca y de frente. Decide el ANGULO y, sobre todo, SI SALIR.
// TABLERO (2026-11-03): M5. registrarIntento() -> anotarIntento() (misma
// firma y misma regla; guarda en la RAM para el #MIRE, no en la EEPROM).
bool intentarDespejar(unsigned long ahora) {
  // MIRAR ANTES DE SALIR: en vez de salir, frena y mira quieto (MIRANDO).
  // Si el robot YA estaba quieto y la pelota ya esta a umbralCm, sale ya.
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
    // La cuenta dice que no llego: no salir (decision del equipo).
    vecesSeguidas = 0;
    if (!hayMirada)
      anotarIntento(motivoNoSale == 2 ? R_MUY_ABIERTO : R_NO_ALCANZA, 0,
                    distanciaPelota(), desvioPelota(), velocidadLateral, 0, 0);   // TABLERO M5
    return false;
  }

  if (!hayMirada)
    anotarIntento(R_SALE_SIN_MIRAR, 0, distanciaPelota(), desvioPelota(),
                  velocidadLateral, 0, lateral);                                  // TABLERO M5
  lateralDelDespeje  = lateral;
  pwmAvanceDespeje   = avance;
  msIdaDespeje       = msIda;

  fase = ADELANTE; t_fase = ahora;
  reiniciarRampaMovimiento();   // arrancar suave: si patina, se tuerce
  reiniciarCorreccion();
  return true;
}

void terminarDespeje() {
  // Un enderezado de los que se hace solo mientras espera: vuelve a esperar.
  if (enderezandoEnEspera) {
    enderezandoEnEspera = false;
    pasarAEsperar();
    return;
  }
  // La ubicacion (o la vuelta del lateral): si encontro la linea, el poquito.
  if (ubicandose) {
    ubicandose = false;
    patrullando = false;
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

// El switch del loop() del partido, COPIA, adentro de una funcion.
// TABLERO (2026-11-03): sin APAGADO ni ARMANDOSE (M12: nunca arranca solo),
// sin el registro de despejes (M11), y con M5, M6, M7, M8 marcados. Por
// donde ENTRA cada prueba y donde TERMINA lo decide el tablero desde
// afuera (observarPartido), sin tocar estos pasos (M9).
void pasoDelPartido(unsigned long ahora) {

  // Volviendo del lateral y aparecio la pelota: se corta y se la sigue.
  if (patrullando && enFaseDeMedio() && veLaPelota()
      && cuadrosConPelota >= CUADROS_PARA_CORTAR_PATRULLA) {
    cortarPatrulla();
  }

  switch (fase) {

    // ---------------- ubicacion inicial: atras hasta la linea ----------------
    case UBIC_ATRAS:
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atrasControlado(potenciaRetroceso);
      if (algunoDeAtrasVeBlanco()) {
        encontroLaLinea = true;
        frenar();
        fase = UBIC_FRENANDO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        // TABLERO (2026-11-03): M8. En la prueba 7 no hace la rutina del
        // medio: frena y se endereza ahi (la rutina se prueba con !I).
        if (prueba != 7 && irAlMedioSiNoHayLinea && enderezarActivado && rumboBase >= 0
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
        pasarAEnderezarse();
      }
      break;

    // ---------- sin linea: ir al medio usando los laterales ----------
    //   1 al lateral, 2 alejarse, 3 al fondo (linea o tiempo), 4 adelante,
    //   5 al mismo lateral, 6 al medio por TIEMPO, 7 atras hasta el area.
    case MEDIO_PAUSA:
      if (ahora - t_fase < msFreno) { frenar(); break; }
      parar();
      if (ahora - t_fase >= msFreno + MS_MEDIO_QUIETO) {
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
        if (faseDespuesDePausa == MEDIO_AL_LATERAL && algunoDeAtrasVeBlanco()) {
          encontroLaLinea = true;
          pasarAEnderezarse();
          break;
        }
        fase = faseDespuesDePausa; t_fase = ahora;
        t_soltoLinea = ahora;
      }
      break;

    case MEDIO_DESPEGARSE:
      digitalWrite(LED, HIGH);
      moverDeCostado(LADO_MEDIO * VEL_MEDIO_BUSCAR);
      if (algunoDeAtrasVeBlanco()) {
        t_soltoLinea = ahora;
      } else if (ahora - t_soltoLinea >= MS_MEDIO_ALEJARSE) {
        pausaMedio(MEDIO_AL_LATERAL_2);
        break;
      }
      if (ahora - t_fase >= MS_MEDIO_TOPE_LATERAL) pausaMedio(MEDIO_AL_MEDIO);
      break;

    case MEDIO_AL_LATERAL:            // paso 1
    case MEDIO_AL_LATERAL_2:          // paso 5
      digitalWrite(LED, HIGH);
      moverDeCostado(LADO_MEDIO * VEL_MEDIO_BUSCAR);
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

    // La vuelta al medio despues de pisar un lateral siguiendo la pelota.
    case LATERAL_AL_MEDIO:
      digitalWrite(LED, HIGH);
      moverDeCostado(-ladoDelLateral * VEL_MEDIO_MEDIR);   // 🚨 100, no 110
      if (pisaElLateral(-ladoDelLateral)) lecturasDeLateral++;
      else                                lecturasDeLateral = 0;
      if (ahora - t_fase >= MS_MEDIO_AL_MEDIO
          || lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
        lecturasDeLateral = 0;
        patrullando = true;
        pausaMedio(MEDIO_AL_AREA);
      }
      break;

    case ESPERANDO:
      parar();
      digitalWrite(LED, ((ahora / 800) % 2) ? HIGH : LOW);

      // Enderezarse mientras espera (15/09), con descanso entre intentos.
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
      // TABLERO (2026-11-03): M6. Despejar SOLO en la prueba 9 (en la 8 la
      // sigue sin despejar). Y sin la rama de la patrulla: nunca sale sola.
      if (prueba == 9 && hayQueDespejar()) {
        if (intentarDespejar(ahora)) digitalWrite(LED, HIGH);
      } else if (veLaPelota()) {
        fase = SIGUIENDO; t_fase = ahora;
        reiniciarRampaMovimiento();
        reiniciarCorreccion();
      }
      break;

    case SIGUIENDO: {
      digitalWrite(LED, HIGH);

      // TABLERO (2026-11-03): M7. Despejar SOLO en la prueba 9.
      if (prueba == 9 && hayQueDespejar()) {
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

      float desvio = desvioPredicho();

      // Zona muerta: si la pelota esta casi enfrente, quedarse quieto.
      if (fabs(desvio) < ZONA_MUERTA_PELOTA) {
        parar();
        reiniciarRampaMovimiento();
        break;
      }

      int fuerza = (int)(fabs(desvio) * kpLateral);
      if (fuerza > pwmMaxLateral) fuerza = pwmMaxLateral;
      if (fuerza < pwmMinLateral) fuerza = pwmMinLateral;

      bool hayQueIrDerecha = (desvio > 0);

      // Si la pelota cruzo al otro lado, reiniciar la rampa.
      static bool ibaDerecha = true;
      if (hayQueIrDerecha != ibaDerecha) {
        ibaDerecha = hayQueIrDerecha;
        reiniciarRampaMovimiento();
      }

      // Limite lateral (13/10): al pisar el lateral vuelve al medio.
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
      // El angulo YA se decidio al salir. El giroscopio sostiene el frente.
      int lateral = lateralDelDespeje;
      if (perseguirEnElDespeje && veLaPelota()) {
        float d = desvioPelota();
        if (fabs(d) > ZONA_MUERTA_PELOTA) {
          lateral = (int)(d * kpPersecucion);
          if (lateral >  PWM_MAX_PERSECUCION) lateral =  PWM_MAX_PERSECUCION;
          if (lateral < -PWM_MAX_PERSECUCION) lateral = -PWM_MAX_PERSECUCION;
        }
      }
      avanzarPersiguiendo(pwmAvanceDespeje, lateral);

      if (ahora - t_fase >= msIdaDespeje) {
        if (frenarAntesDeVolver) frenar(); else parar();
        fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;
    }

    case PAUSA_MEDIO:
      // 20/10: frena y se queda quieto antes de volver. Con false, suelta.
      if (frenarAntesDeVolver) frenar(); else parar();
      if (ahora - t_fase >= (frenarAntesDeVolver ? MS_ESPERA_ANTES_DE_VOLVER
                                                 : MS_PAUSA_MEDIO)) {
        fase = ATRAS_HASTA_LINEA; t_fase = ahora;
      }
      break;

    case ATRAS_HASTA_LINEA: {
      // Volver por la misma diagonal, con el costado achicado en la misma
      // proporcion que la potencia (para que el ANGULO sea el mismo).
      int lateralVuelta = -(lateralDelDespeje * potenciaRetroceso)
                          / pwmAvanceDespeje;
      atrasEnDiagonal(potenciaRetroceso, lateralVuelta);

      if (algunoDeAtrasVeBlanco()) {
        frenar();
        fase = FRENANDO; t_fase = ahora;
        break;
      }
      // Freno de emergencia: sin limite se va de la cancha.
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        frenar();
        fase = FRENANDO; t_fase = ahora;
      }
      break;
    }

    case FRENANDO:
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
      // Esperar que pase la inercia ANTES de volver a medir, con el freno.
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
      // TABLERO (2026-11-03): M10. El delay(msFreno) de aca queda (es del
      // partido), pero esa vuelta NO cuenta como hueco (ver observarPartido).
      adelanteControlado(potenciaEmpujon);
      if (ahora - t_fase >= (unsigned long)msEmpujonFinal) {
        frenar();
        delay(msFreno);
        parar();
        if (empujonDeArranque) {
          empujonDeArranque = false;
          reiniciarCorreccion();
          pasarAEsperar();
        } else {
          fase = ENFRIANDO; t_fase = ahora;
        }
      }
      break;

    case MIRANDO: {
      // Frena, despues mide QUIETO, y sale recien con la pelota a umbralCm.
      // TABLERO (2026-11-03): M5. Sin los Serial.print (mientras se mueve no
      // se manda nada) y anotarIntento() en vez de registrarIntento().
      digitalWrite(LED, HIGH);
      if (ahora - t_fase < MS_ASENTAR_MIRADA) { frenar(); break; }
      parar();
      if (!veLaPelota()) { hayMirada = false; pasarAEsperar(); break; }

      bool vencio = (ahora - t_fase >= MS_MAX_MIRANDO);
      bool yaMidio = (ahora - t_fase >= MS_ASENTAR_MIRADA + MS_MIRAR) && calcularMirada();

      // Ya midio y la pelota todavia esta lejos: esperar SOLO si viene.
      if (yaMidio && distanciaPelota() > umbralCm) {
        unsigned long pasaron = ahora - t_fase;
        float quedanS = (pasaron < MS_MAX_MIRANDO) ? (MS_MAX_MIRANDO - pasaron) / 1000.0 : 0;
        bool viene = vaMirada > PISO_VELOCIDAD
                  && (distanciaPelota() - umbralCm) / vaMirada <= quedanS;
        if (!viene) {
          anotarIntento(R_NO_VIENE, nMirada, distanciaPelota(), desvioPelota(), vyMirada, vaMirada, 0);   // TABLERO M5
          vencio = true;
        }
      }

      if (yaMidio && distanciaPelota() <= umbralCm) {
        hayMirada = true;
        bool salio = intentarDespejar(ahora);
        anotarIntento(salio ? R_SALE_MIRANDO : (motivoNoSale == 2 ? R_MUY_ABIERTO : R_NO_ALCANZA),
                      nMirada, distanciaPelota(), desvioPelota(), vyMirada, vaMirada,
                      salio ? lateralDelDespeje : 0);                                             // TABLERO M5
        hayMirada = false;
        if (salio) break;
        vencio = true;                     // no sale: a seguirla
      }
      if (vencio) {
        if (!yaMidio) {
          anotarIntento(R_POCOS_CUADROS, nMirada, distanciaPelota(), desvioPelota(), 0, 0, 0);    // TABLERO M5
        }
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

    default:
      break;
  }

  // AVISO "ESTOY SIN GIROSCOPIO", del final del loop() del partido.
  // TABLERO (2026-11-03): M13. Corre solo en MOVIENDO (aca adentro); en
  // ESPERA el LED lo maneja el tablero (ledEnEspera).
  if (ahora - t_chequeoGiro >= 200) {
    t_chequeoGiro = ahora;
    rumboActual();                 // actualiza giroscopoRespondiendo
  }
  bool sinRumbo = (!hayGiroscopo) || (!giroscopoRespondiendo) || (rumboBase < 0);
  if (sinRumbo) {
    digitalWrite(PIN_AVISO, ((ahora / MS_PARPADEO_AVISO) % 2) ? HIGH : LOW);
  }
}


// =====================================================================
//   LOS INTENTOS DE LA 9 (el renglon #MIRE)
// =====================================================================

// TABLERO (2026-11-03): M5. Reemplaza a registrarIntento() del partido, con
// la MISMA firma y la MISMA regla: un "no sale" igual al anterior y a menos
// de 1 s es UNA sola situacion y no se anota. Guarda en la RAM los ultimos
// MAX_MIRE (los ultimos, porque el que importa es el que termino saliendo),
// y cuenta TODOS por resultado (para los consejos).
void anotarIntento(uint8_t resultado, int cuadros, float d, float y,
                   float vy, float va, int lateral) {
  if (prueba != 9 || modo != MOVIENDO) return;
  unsigned long ahora = millis();
  bool sale = (resultado == R_SALE_MIRANDO || resultado == R_SALE_SIN_MIRAR);
  if (!sale && nIntentosTotal > 0) {
    if (ultimoRes == resultado && ahora - t_ultimoIntento < 1000) return;
  }
  Intento &n = anillo[nIntentosTotal % MAX_MIRE];
  n.k       = a16(nIntentosTotal + 1);
  n.t       = a16(ahora - t_inicio);
  n.res     = resultado;
  // Paso por MIRANDO todo lo que se anota desde MIRANDO: los que traen
  // cuadros, y NO_VIENE / POCOS_CUADROS / SALE_MIRANDO aunque traigan 0.
  n.miro    = (resultado == R_SALE_MIRANDO || resultado == R_NO_VIENE
               || resultado == R_POCOS_CUADROS || cuadros > 0) ? 1 : 0;
  n.cuadros = (uint8_t)constrain(cuadros, 0, 255);
  n.d       = s16(lround(d * 10));
  n.y       = s16(lround(y * 10));
  n.vy      = s16(lround(vy * 10));
  n.va      = s16(lround(va * 10));
  // Lo que anoto calcularDiagonal() en esta misma pasada (M3).
  n.av      = sale ? (int16_t)diagAvance : 0;
  n.lat     = sale ? (int16_t)lateral : 0;
  n.ms      = sale ? a16(diagMsIda) : 0;
  n.ang     = (sale || resultado == R_MUY_ABIERTO) ? s16(lround(diagAngulo * 10)) : 0;
  n.recto   = sale ? diagRecto : 0;
  nIntentosTotal++;
  if (resultado < R_CANTIDAD && contRes[resultado] < 65535) contRes[resultado]++;
  ultimoRes = resultado;
  t_ultimoIntento = ahora;
}


// =====================================================================
//   LAS FASES DE LA PRUEBA (de ajustar-movimientos)
// =====================================================================

// El freno y el quieto del final: f = frenar() msFreno, q = parar() 500 ms
// midiendo sin corregir (asi se ve como quedo despues de la inercia).
void entrarFreno(Como como, unsigned long ahora) {
  comoFinal = como;
  frenar();
  fase = P_FRENO; t_fase = ahora;
}

void entrarQuieto(Como como, unsigned long ahora) {
  comoFinal = como;
  parar();
  fase = P_QUIETO; t_fase = ahora;
}

void pasoDeFaseDePrueba(unsigned long ahora) {
  switch (fase) {
    case COSTADO:
    case COSTADO_VUELTA:
      // 1 a 5: de costado a fuerza FIJA con el moverDeCostado() del partido.
      digitalWrite(LED, HIGH);
      moverDeCostado(ladoCostado * velCostado);
      // La misma regla del partido: sensor de ese lado en blanco y el otro no.
      if (pisaElLateral(ladoCostado)) lecturasDeLateral++;
      else                            lecturasDeLateral = 0;
      if (lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
        entrarFreno(COMO_LATERAL, ahora);
        break;
      }
      if (ahora - t_fase >= (unsigned long)msCostado) {
        if (fase == COSTADO_VUELTA) {
          // Como cuando la pelota cruza: se da vuelta sin frenar, con rampa.
          ladoCostado = -ladoCostado;
          reiniciarRampaMovimiento();
          lecturasDeLateral = 0;
          fase = COSTADO; t_fase = ahora;
        } else {
          entrarFreno(COMO_NORMAL, ahora);
        }
      }
      break;

    case P_FRENO:
      frenar();
      if (ahora - t_fase >= msFreno) {
        parar();
        fase = P_QUIETO; t_fase = ahora;
      }
      break;

    case P_QUIETO:
      parar();
      if (ahora - t_fase >= MS_QUIETO_FINAL) terminarPrueba(comoFinal);
      break;

    default:
      break;
  }
}

// Por que letra va cada fase en el grafico (seccion 2.4 del diseno).
char letraDeFase(Fase f) {
  switch (f) {
    case COSTADO: case COSTADO_VUELTA:            return 'l';
    case ESPERANDO: case SIGUIENDO:               return 's';
    case MIRANDO:                                 return 'm';
    case ADELANTE:                                return 'i';
    case PAUSA_MEDIO:                             return 'p';
    case ATRAS_HASTA_LINEA: case UBIC_ATRAS:
    case P_FONDO:                                 return 'a';
    case ACOMODANDO: case ACOMODO_ASENTAR:        return 'e';
    case ADELANTE_CHICO:                          return 'c';
    case MEDIO_DESPEGARSE: case MEDIO_AL_LATERAL: case MEDIO_ALEJARSE:
    case MEDIO_AL_FONDO: case MEDIO_ADELANTE: case MEDIO_AL_LATERAL_2:
    case MEDIO_AL_MEDIO: case MEDIO_AL_AREA: case LATERAL_AL_MEDIO:
    case P_ANCHO:                                 return 'v';
    case ENFRIANDO: case P_QUIETO:
    case P_LINEA: case P_CAMARA:                  return 'q';
    default:                                      return 'f';   // los frenos y las pausas
  }
}

// Las fases que corrigen el rumbo CON MOTORES: ahi se cuentan las muestras,
// fuera3, corr, tope, medio y el hueco. ("s" solo cuando de verdad se movio:
// eso lo dice que se haya llamado a correccionDeRumbo en esa vuelta.)
bool letraEnMovimiento(char c) {
  return c == 'l' || c == 's' || c == 'i' || c == 'a' || c == 'c' || c == 'v';
}

bool esCancha(int p) { return p == 'L' || p == 'A' || p == 'F' || p == 'C' || p == 'I'; }

// L y C no mueven y A mide a su manera: en esas no se usa anotar().
bool midoElGiro() { return prueba != 'L' && prueba != 'C' && prueba != 'A'; }


// =====================================================================
//   LO QUE SE MIDE (como anotar() de ajustar-movimientos)
// =====================================================================

void empezarMedicion(unsigned long ahora) {
  nMuestras = 0;
  msMuestra = (prueba == 8 || prueba == 9) ? 20 : 10;
  t_muestra = ahora - msMuestra;          // la primera muestra sale enseguida
  pico = 0; picoMs = 0; picoFase = 'q';
  ladoDelError = 0; cruces = 0;
  muestrasEnMov = 0; fuera3 = 0; alTope = 0; sumaCorr = 0; sumaErr = 0;
  mudas = 0; racha = 0; peorRacha = 0;
  huecoMax = 0; huecoMs = 0; msLoopPrevio = ahora; saltearHueco = false; enMovAnterior = false;
  sumaFinal = 0; nFinal = 0; peorEnMov = 0;
  idaMs = 0; atrasMs = 0; lineaMs = 0; sinLinea = false;
  acomodoMax = 0; laterales = 0;
  vueltasSeguir = 0; conPelota = 0; enZonaMuerta = 0; idas = 0; arranques = 0; sumaVel = 0;
  seMovia = false; ladoSeguir = true;
  salio = false; inicioMov = 0;
  nIntentosTotal = 0; ultimoRes = 0; t_ultimoIntento = 0;
  for (int r = 0; r < R_CANTIDAD; r++) contRes[r] = 0;
  ultimaMuda = false; ultimoError = 0; ultimaCorreccion = 0;
  for (int i = 0; i < N_AJUSTES; i++) usadosRAM[i] = valorDe(i);
}

// En la 9, la nota se mide desde que arranca la ida (lo de antes es seguir
// la pelota). Las #M si van desde el principio.
void reiniciarCuentasDeNota() {
  pico = 0; picoMs = 0; picoFase = 'q';
  ladoDelError = 0; cruces = 0;
  muestrasEnMov = 0; fuera3 = 0; alTope = 0; sumaCorr = 0; sumaErr = 0;
  mudas = 0; racha = 0; peorRacha = 0;
  huecoMax = 0; huecoMs = 0;
  sumaFinal = 0; nFinal = 0;
  acomodoMax = 0;
}

// Para cuando en esta vuelta nadie corrigio: solo mirar, sin mover nada.
void medirSinCorregir() {
  ultimaCorreccion = 0;
  float r = rumboActual();
  if (r < 0) { ultimaMuda = true; return; }
  ultimaMuda = false;
  ultimoError = diferencia(rumboBase, r);
}

// Una vez por vuelta, DESPUES de mover. Usa la lectura que ya se hizo.
void anotar(unsigned long ahora, Fase antes, char letra, bool enMov) {
  enMovAnterior = enMov;
  if (ultimaMuda) {
    mudas++;
    racha++;
    if (racha > peorRacha) peorRacha = racha;
    return;
  }
  racha = 0;
  float e = ultimoError;
  unsigned long enPrueba = ahora - t_inicio;

  if (enMov) {
    muestrasEnMov++;
    if (fabs(e) > 3.0) fuera3++;
    sumaCorr += ultimaCorreccion;
    sumaErr += e;
    if (abs(ultimaCorreccion) >= MAX_CORRECCION_RUMBO) alTope++;
    if (fabs(e) > peorEnMov) peorEnMov = fabs(e);
  }
  if (fabs(e) > fabs(pico)) { pico = e; picoMs = enPrueba; picoFase = letra; }

  // Un cruce: pasar de mas de +2 a menos de -2 grados, o al reves.
  int lado = (e > BANDA_CRUCE) ? 1 : (e < -BANDA_CRUCE) ? -1 : 0;
  if (lado != 0) {
    if (ladoDelError != 0 && lado != ladoDelError) cruces++;
    ladoDelError = lado;
  }
  // fin = el promedio de los ultimos 150 ms de la fase q.
  if (antes == P_QUIETO && ahora - t_fase >= MS_QUIETO_FINAL - 150) {
    sumaFinal += e; nFinal++;
  }
  // Las muestras del grafico (las mudas no se mandan).
  if (ahora - t_muestra >= msMuestra && nMuestras < MAX_MUESTRAS) {
    t_muestra = ahora;
    muestraT[nMuestras] = a16(enPrueba);
    muestraG[nMuestras] = s16(lround(e * 10));
    muestraF[nMuestras] = letra;
    nMuestras++;
  }
}

// El numero de paso de la rutina de ir al medio (para !I).
int numeroDePasoMedio(Fase f) {
  switch (f) {
    case MEDIO_AL_LATERAL:   return 1;
    case MEDIO_ALEJARSE:     return 2;
    case MEDIO_AL_FONDO:     return 3;
    case MEDIO_ADELANTE:     return 4;
    case MEDIO_AL_LATERAL_2: return 5;
    case MEDIO_AL_MEDIO:     return 6;
    case MEDIO_AL_AREA:      return 7;
    case ADELANTE_CHICO:     return 8;
    default:                 return 0;
  }
}

// TABLERO (2026-11-03): M9. Lo que el tablero mira DESDE AFUERA despues de
// cada paso del partido, sin tocar los pasos: los cambios de fase (cuando
// termino la ida, cuando vio la linea...), las cuentas del seguimiento y,
// sobre todo, DONDE TERMINA cada prueba (tabla 4.3 del diseno).
void observarPartido(Fase antes, unsigned long tAntes, unsigned long ahora, bool corrigio) {
  unsigned long enPrueba = ahora - t_inicio;

  if (antes != fase) {
    if (antes == ADELANTE && fase == PAUSA_MEDIO) idaMs = enPrueba;
    if (antes == PAUSA_MEDIO && fase == ATRAS_HASTA_LINEA) atrasMs = enPrueba;
    if (antes == ATRAS_HASTA_LINEA && fase == FRENANDO) {
      // Freno por la linea o por el tope de MS_MAX_RETROCESO.
      bool vio = (ahora - tAntes < MS_MAX_RETROCESO) || algunoDeAtrasVeBlanco();
      if (vio) { lineaMs = ahora - tAntes; if (lineaMs == 0) lineaMs = 1; }
      else     { lineaMs = 0; sinLinea = true; }
    }
    if (prueba == 7 && antes == UBIC_ATRAS && fase == UBIC_FRENANDO) {
      if (encontroLaLinea) { lineaMs = ahora - tAntes; if (lineaMs == 0) lineaMs = 1; }
      else                 { lineaMs = 0; sinLinea = true; }
    }
    if (fase == LATERAL_AL_MEDIO) laterales++;
    // M10: la vuelta del delay(msFreno) de ADELANTE_CHICO no es un hueco.
    if (antes == ADELANTE_CHICO) saltearHueco = true;
    // !I: cada paso de la rutina, y #PASO con el robot ya frenado.
    if (prueba == 'I') {
      int n = numeroDePasoMedio(antes);
      if (n > 0) {
        unsigned long dur = ahora - tAntes;
        char como = 'T';
        if (n == 1 || n == 5) como = (dur < MS_MEDIO_TOPE_LATERAL || algunoDeAtrasVeBlanco()) ? 'L' : 'T';
        if (n == 3)           como = (dur < MS_MEDIO_FONDO_POR_TIEMPO || algunoDeAtrasVeBlanco()) ? 'L' : 'T';
        if (n == 7)           como = encontroLaLinea ? 'L' : 'T';
        mePasos[n - 1] = a16(dur);
        meComo[n - 1]  = como;
        if (n <= 7 && Serial) {
          Serial.print("#PASO I paso="); Serial.print(n);
          Serial.print(" ms="); Serial.print(dur);
          Serial.print(" como="); Serial.print(como);
          finDeLinea();
        }
      }
    }
    // 9: la nota se mide desde que arranca la ida.
    if (prueba == 9 && fase == ADELANTE && !salio) {
      salio = true;
      reiniciarCuentasDeNota();
      inicioMov = enPrueba;
    }
  }
  if ((fase == ACOMODANDO || fase == ACOMODO_ASENTAR) && reintentosAcomodo > acomodoMax)
    acomodoMax = reintentosAcomodo;

  // 8 y 9: las cuentas del seguimiento (como pasoSeguir de ajustar).
  if ((prueba == 8 || prueba == 9) && (antes == ESPERANDO || antes == SIGUIENDO)) {
    vueltasSeguir++;
    if (veLaPelota()) {
      conPelota++;
      sumaVel += fabs(velocidadLateral);
    }
    bool seMueve = (antes == SIGUIENDO && corrigio);
    if (seMueve) {
      bool derecha = (desvioPredicho() > 0);
      if (derecha != ladoSeguir) { ladoSeguir = derecha; idas++; }
      if (!seMovia) arranques++;
    } else if (antes == SIGUIENDO && veLaPelota() && fabs(desvioPredicho()) < ZONA_MUERTA_PELOTA) {
      enZonaMuerta++;
    }
    seMovia = seMueve;
  }

  // ---- DONDE TERMINA CADA PRUEBA ----
  if ((prueba == 6 || prueba == 9) && fase == ENFRIANDO) {
    entrarQuieto((prueba == 6 && sinLinea) ? COMO_SIN_LINEA : COMO_NORMAL, ahora);
  } else if (prueba == 10 && fase == ATRAS_HASTA_LINEA) {
    entrarQuieto(COMO_NORMAL, ahora);            // NO vuelve: se queda donde llego
  } else if ((prueba == 7 || prueba == 'I') && fase == ESPERANDO) {
    entrarQuieto((prueba == 7 && sinLinea) ? COMO_SIN_LINEA : COMO_NORMAL, ahora);
  } else if ((prueba == 8 || (prueba == 9 && !salio)) && enPrueba >= (unsigned long)msSeguir
             && (fase == ESPERANDO || fase == SIGUIENDO)) {
    entrarFreno(prueba == 9 ? COMO_NO_SALIO : COMO_NORMAL, ahora);
  }
}


// =====================================================================
//   LA NOTA Y EL CIERRE DE UN MOVIMIENTO
// =====================================================================

// La calcularNota() de ajustar-movimientos, con nombres (seccion 4.4).
uint8_t calcularNota(const DatosMov &d) {
  if (d.como == COMO_TECLA || d.como == COMO_TOPE) return NOTA_NO_VALE;
  // TABLERO: en la 9 sin salir no hubo despeje que medir. Ahi solo la tira
  // una racha muda del giroscopio; si no, es NO_SALIO (ver el informe).
  if (d.n == 9 && !d.sale) return (d.racha >= 3) ? NOTA_NO_VALE : NOTA_NO_SALIO;
  if (d.racha >= 3 || (unsigned long)d.mudas * 50 > (unsigned long)d.muestras + d.mudas
      || d.muestras < 5) return NOTA_NO_VALE;
  float picoAbs = abs(d.pico) / 10.0;
  float finAbs  = abs(d.fin) / 10.0;
  if (d.cruces >= NOTA_CRUCES_ZIGZAG) return NOTA_ZIGZAGUEA;
  if (picoAbs > NOTA_MAX_TUERCE || finAbs > NOTA_MAX_TUERCE) return NOTA_SE_TUERCE;
  if (picoAbs <= NOTA_MAX_PROLIJO && d.cruces <= NOTA_CRUCES_OK && finAbs <= NOTA_FINAL_OK) return NOTA_PROLIJO;
  return NOTA_CASI;
}

void cerrarMovimiento(Como como, unsigned long ahora) {
  DatosMov &d = bMov.d;
  memset(&d, 0, sizeof(d));
  d.n        = (uint8_t)prueba;
  d.como     = (uint8_t)como;
  d.sale     = salio ? 1 : 0;
  d.pico     = s16(lround(pico * 10));
  d.picoms   = a16(picoMs);
  d.picoFase = picoFase;
  d.fin      = (nFinal > 0) ? s16(lround(sumaFinal / nFinal * 10)) : s16(lround(ultimoError * 10));
  d.muestras = a16(muestrasEnMov);
  if (muestrasEnMov > 0) {
    d.medio  = s16(lround(sumaErr / muestrasEnMov * 10));
    d.corr   = s16(sumaCorr / (long)muestrasEnMov);
    d.fuera3 = (uint8_t)((100UL * fuera3) / muestrasEnMov);
    d.tope   = (uint8_t)((100UL * alTope) / muestrasEnMov);
  }
  d.cruces   = a16(cruces);
  d.mudas    = a16(mudas);
  d.racha    = a16(peorRacha);
  d.hueco    = a16(huecoMax);
  d.huecoms  = a16(huecoMs);
  d.duro     = a16(ahora - t_inicio);
  d.lineams  = a16(lineaMs);
  d.sinLinea = sinLinea ? 1 : 0;
  d.idams    = a16(idaMs);
  d.atrasms  = a16(atrasMs);
  d.acomodo  = (uint8_t)constrain(acomodoMax, 0, 255);
  d.laterales = (uint8_t)constrain(laterales, 0, 255);
  if (prueba == 8 || prueba == 9) {
    d.idas      = a16(idas);
    d.arranques = a16(arranques);
    if (vueltasSeguir > 0) {
      d.conpelota  = (uint8_t)((100UL * conPelota) / vueltasSeguir);
      d.zonamuerta = (uint8_t)((100UL * enZonaMuerta) / vueltasSeguir);
    }
    d.velmedia = (conPelota > 0) ? s16(lround(sumaVel / conPelota * 10)) : 0;
  }
  d.intentos  = a16(nIntentosTotal);
  d.inicioMov = a16(inicioMov);
  d.topeMs    = a16(topePrueba);
  for (int r = 0; r < R_CANTIDAD; r++) d.contRes[r] = contRes[r];
  // Los intentos: los ultimos MAX_MIRE, en orden.
  unsigned long cuantosHay = (nIntentosTotal < (unsigned long)MAX_MIRE) ? nIntentosTotal : (unsigned long)MAX_MIRE;
  unsigned long primero = nIntentosTotal - cuantosHay;
  for (unsigned long k = 0; k < cuantosHay; k++) d.mire[k] = anillo[(primero + k) % MAX_MIRE];
  d.nIntentos = (uint8_t)cuantosHay;
  // Los numeros cambiados (hasta 12 en la EEPROM; si hubo mas, cuantos).
  for (int i = 0; i < N_AJUSTES; i++) {
    if (fabs(usadosRAM[i] - ajustes[i].defecto) <= 0.001) continue;
    if (d.nUsados < MAX_USADOS_EEPROM) {
      d.usadoIdx[d.nUsados] = (uint8_t)i;
      d.usadoVal[d.nUsados] = usadosRAM[i];
      d.nUsados++;
    } else if (d.usadosDeMas < 255) {
      d.usadosDeMas++;
    }
  }
  // Las muestras para la EEPROM: una de cada k, hasta 100.
  int k = (nMuestras + MAX_MUESTRAS_EEPROM - 1) / MAX_MUESTRAS_EEPROM;
  if (k < 1) k = 1;
  int j = 0;
  for (int i = 0; i < nMuestras && j < MAX_MUESTRAS_EEPROM; i += k) {
    d.mT[j] = muestraT[i]; d.mG[j] = muestraG[i]; d.mF[j] = muestraF[i];
    j++;
  }
  d.nMuestras = (uint8_t)j;
  d.nota = calcularNota(d);
}


// =====================================================================
//   EL INFORME: #M, #MIRE, #R, #USADOS, #CONSEJO, RESULTADO, #FIN
// =====================================================================

void consejo(const char* texto) {
  Serial.print("#CONSEJO "); Serial.print(texto); finDeLinea();
}

// Los consejos (seccion 4.5), con los numeros de AHORA entre parentesis.
void mandarConsejos(const DatosMov &d, bool apagado) {
  int  n     = d.n;
  bool tecla = (d.como == COMO_TECLA);
  bool tope  = (d.como == COMO_TOPE);
  bool noVale = (d.nota == NOTA_NO_VALE);

  if (apagado && (d.nUsados > 0 || d.usadosDeMas > 0))
    consejo("Se corrio con numeros cambiados que se perdieron al apagar: los consejos usan los de ahora.");

  // 1. tecla o tope
  if (tecla) consejo("Lo frenaron antes de terminar: este resultado no vale.");
  if (tope) {
    Serial.print("#CONSEJO Se corto por el tope de tiempo ("); Serial.print(d.topeMs);
    Serial.print(" ms): no deberia pasar. Avisar."); finDeLinea();
  }
  // 2. NO_VALE por el giroscopio (o porque casi no se movio)
  bool giroMalo = d.racha >= 3 || (unsigned long)d.mudas * 50 > (unsigned long)d.muestras + d.mudas;
  if (noVale && !tecla && !tope) {
    if (giroMalo) {
      Serial.print("#CONSEJO El giroscopio no contesto ("); Serial.print(d.mudas);
      Serial.print(" lecturas mudas, la peor racha "); Serial.print(d.racha);
      Serial.print(" seguidas): bateria prendida ANTES del USB, y el cable del giroscopio (29/09).");
      finDeLinea();
    } else {
      Serial.print("#CONSEJO Casi no se movio ("); Serial.print(d.muestras);
      Serial.print(" muestras moviendose): no hay nada que medir. Repetirlo."); finDeLinea();
    }
  }
  // 3. el hueco
  if (d.hueco > HUECO_RETARDO) {
    Serial.print("#CONSEJO Una vuelta del programa tardo "); Serial.print(d.hueco);
    Serial.print(" ms (a los "); Serial.print(d.huecoms);
    Serial.print(" ms): eso es un RETARDO y con eso zigzaguea igual, toquen lo que toquen. Avisar antes de cambiar numeros.");
    finDeLinea();
  }
  // 4. segun la nota
  long picoRel = (long)d.picoms - (long)d.inicioMov;
  switch (d.nota) {
    case NOTA_PROLIJO:
      consejo("Nada que tocar: anotar los numeros en la bitacora. Probar tambien para el otro lado.");
      break;
    case NOTA_CASI:
      if (picoRel < 300 && ((n >= 1 && n <= 5) || n == 8)) {
        // Nunca en 6, 9 y 10: en el despeje, bajar la rampa cambia cuanto
        // sale y no endereza nada.
        Serial.print("#CONSEJO La peor torcida fue al ARRANCAR: bajar RAMPA_MOV_PASO de a 1 (ahora ");
        Serial.print(RAMPA_MOV_PASO);
        Serial.print("). OJO: cambia msAdelante y MS_MEDIO_AL_MEDIO (hay que medirlos de nuevo).");
        finDeLinea();
      } else if (abs(d.medio) > 20) {
        Serial.print("#CONSEJO Queda torcido siempre para el mismo lado: subir KI_RUMBO de a 0.25 (ahora ");
        imprimirDecimal(KI_RUMBO); Serial.print(")."); finDeLinea();
      } else {
        consejo("Esta cerca. Repetir 2 o 3 veces sin tocar nada: si da parecido, es asi.");
      }
      break;
    case NOTA_ZIGZAGUEA:
      Serial.print("#CONSEJO Corrige de mas y se pasa: bajar KP_RUMBO de a 0.5 (ahora ");
      imprimirDecimal(KP_RUMBO); Serial.print("). Si sigue, bajar KI_RUMBO de a 0.25 (ahora ");
      imprimirDecimal(KI_RUMBO); Serial.print(")."); finDeLinea();
      break;
    case NOTA_SE_TUERCE: {
      bool despeje = (n == 6 || n == 9) && d.idams > 0 && d.atrasms > 0;
      if (d.tope > 20) {
        Serial.print("#CONSEJO La correccion estuvo al tope el "); Serial.print(d.tope);
        Serial.print("% del tiempo y no alcanzo: subir MAX_CORRECCION_RUMBO de a 10 (ahora ");
        Serial.print(MAX_CORRECCION_RUMBO); Serial.print("), o ir mas despacio."); finDeLinea();
      } else if (despeje && d.idams <= d.picoms && d.picoms < d.atrasms) {
        if (frenarAntesDeVolver) {
          Serial.print("#CONSEJO Se torcio en la PAUSA, frenado al terminar la ida (ahi no corrige): probar MS_ESPERA_ANTES_DE_VOLVER de a 25 (ahora ");
          Serial.print(MS_ESPERA_ANTES_DE_VOLVER); Serial.print(")."); finDeLinea();
        } else {
          consejo("Se torcio en la PAUSA, al terminar la ida (ahi no corrige): probar frenarAntesDeVolver = 1.");
        }
      } else if (despeje && d.picoms >= d.atrasms
                 && (unsigned long)d.picoms < (unsigned long)d.atrasms + 300) {
        consejo("Se torcio al ARRANCAR PARA ATRAS: el partido no reinicia la rampa ahi. No hay numero para eso: anotarlo en la bitacora.");
      } else if (d.picoFase == 'f') {
        consejo("Se tuerce al FRENAR: el freno es igual al del partido, no hay numero para tocar.");
      } else {
        Serial.print("#CONSEJO Corrige flojo y se le escapa: subir KP_RUMBO de a 0.5 (ahora ");
        imprimirDecimal(KP_RUMBO); Serial.print(")."); finDeLinea();
      }
      break;
    }
    default:
      break;
  }
  // 5. de costado, corrigiendo siempre para el mismo lado
  if (n >= 1 && n <= 5 && !noVale && abs(d.corr) >= 15) {
    Serial.print("#CONSEJO Corrigio en promedio "); Serial.print(d.corr);
    Serial.print(" todo el tiempo: de costado tiende a girar solo. Correr el MISMO movimiento para el otro lado: si la correccion cambia de signo es la mezcla (LADO_TRASERA 2 mas o 2 menos, ahora ");
    Serial.print(LADO_TRASERA);
    Serial.print("; OJO: cambia MS_MEDIO_AL_MEDIO); si tiene el mismo signo, es un motor que tira mas (KI_RUMBO).");
    finDeLinea();
  }
  // 6. no vio la linea al volver
  if ((n == 6 || n == 7 || n == 9) && d.sinLinea && !tecla && !tope) {
    Serial.print("#CONSEJO No vio la linea al volver: freno por tiempo ("); Serial.print(MS_MAX_RETROCESO);
    Serial.print(" ms). Mirar umbralBlanco (CANCHA, paso 1) y que haya linea atras."); finDeLinea();
  }
  // 7. el acomodo
  if (d.acomodo >= 3) consejo("No se pudo enderezar en 3 intentos: mirar la bateria y el piso.");
  // 8 y 9. el seguimiento
  if ((n == 8 || n == 9) && d.idas >= 3 && d.velmedia < 40) {
    Serial.print("#CONSEJO VA Y VIENE con la pelota casi quieta. De a uno: subir ZONA_MUERTA_PELOTA de a 0.2 (ahora ");
    imprimirDecimal(ZONA_MUERTA_PELOTA);
    Serial.print("), o probar predecirTrayectoria = 0, o bajar pwmMinLateral de a 5 (ahora ");
    Serial.print(pwmMinLateral); Serial.print(")."); finDeLinea();
  }
  if ((n == 8 || n == 9) && d.conpelota < 50) {
    Serial.print("#CONSEJO Vio la pelota solo el "); Serial.print(d.conpelota);
    Serial.print("% del tiempo: ponerla de frente, a menos de 60 cm."); finDeLinea();
  }
  // 10. la 9: por los intentos (uno por cada resultado que aparezca)
  if (n == 9) {
    if (d.contRes[R_MUY_ABIERTO] > 0) {
      Serial.print("#CONSEJO "); Serial.print(d.contRes[R_MUY_ABIERTO]);
      Serial.print(" veces no salio por MUY ABIERTO (tenia que salir a mas de "); Serial.print(lround(MAX_ANGULO_DIAGONAL));
      Serial.print(" grados). La perilla es MAX_ANGULO_DIAGONAL (ahora "); imprimirDecimal(MAX_ANGULO_DIAGONAL);
      Serial.print("), de a 5. OJO: salir abierto y no llegar es peor que no salir."); finDeLinea();
    }
    if (d.contRes[R_NO_ALCANZA] > 0) {
      Serial.print("#CONSEJO "); Serial.print(d.contRes[R_NO_ALCANZA]);
      Serial.print(" veces no salio porque el avance no alcanzaba (muy al costado o muy rapida). No se arregla con rendimientoCostado: ese numero tiene que ser el de verdad. Anotarlo.");
      finDeLinea();
    }
    if (d.contRes[R_NO_VIENE] > 0) {
      Serial.print("#CONSEJO "); Serial.print(d.contRes[R_NO_VIENE]);
      Serial.print(" veces miro y la pelota no venia (o no llegaba a umbralCm a tiempo). Con la pelota quieta lejos es normal. Si pasa con pelotas que SI vienen: subir MS_MAX_MIRANDO de a 50 (ahora ");
      Serial.print(MS_MAX_MIRANDO); Serial.print(")."); finDeLinea();
    }
    if (d.contRes[R_POCOS_CUADROS] > 0) {
      Serial.print("#CONSEJO "); Serial.print(d.contRes[R_POCOS_CUADROS]);
      Serial.print(" veces no junto 3 cuadros a tiempo: la camara manda pocos (mirar q en SENSORES). Revisar la camara antes de tocar MS_MIRAR.");
      finDeLinea();
    }
    // el intento que salio (el ultimo que salio)
    for (int k = d.nIntentos - 1; k >= 0; k--) {
      const Intento &it = d.mire[k];
      if (it.res != R_SALE_MIRANDO && it.res != R_SALE_SIN_MIRAR) continue;
      if (it.recto == 3) consejo("Salio DERECHO porque el giroscopio no contestaba.");
      if (it.recto == 2) consejo("Salio DERECHO porque despejeEnDiagonal esta apagado.");
      break;
    }
    if (d.sale) {
      Serial.print("#CONSEJO Si NO LLEGO de costado (la pelota le quedo mas afuera): BAJAR rendimientoCostado de a 0.05 (ahora ");
      Serial.print(rendimientoCostado, 2);
      Serial.print("). Si se PASO de costado: SUBIRLO. Va al reves: mas chico = MAS costado."); finDeLinea();
    }
  }
  // 11. la 9 sin salir
  if (n == 9 && d.nota == NOTA_NO_SALIO) {
    Serial.print("#CONSEJO En msSeguir ("); Serial.print(msSeguir);
    Serial.print(" ms) no salio a despejar. Mirar los intentos: dicen por que."); finDeLinea();
  }
  // 12. la 10
  if (n == 10) consejo("Medir con regla desde la cinta hasta el frente del chasis y anotarlo en CANCHA (paso 5).");
}

// La tanda de un movimiento, en vivo (conFreno) o repetida (sin el #FRENO).
void mandarTandaMov(bool conFreno) {
  const DatosMov &d = bMov.d;
  bool apagado = !movMismo;
  if (conFreno) {
    if (d.como == COMO_TECLA) decir("#FRENO tecla");
    else if (d.como == COMO_TOPE) decir("#FRENO tope de tiempo");
  }
  // #M: de la RAM, todas; de la EEPROM, las raleadas.
  int cuantas = movMismo ? nMuestras : d.nMuestras;
  for (int i = 0; i < cuantas; i++) {
    Serial.print("#M ");
    Serial.print(movMismo ? muestraT[i] : d.mT[i]); Serial.print(' ');
    imprimirDecimas(movMismo ? muestraG[i] : d.mG[i]); Serial.print(' ');
    Serial.print(movMismo ? muestraF[i] : d.mF[i]);
    finDeLinea();
  }
  // #MIRE: solo en la 9.
  if (d.n == 9) {
    for (int k = 0; k < d.nIntentos; k++) {
      const Intento &it = d.mire[k];
      Serial.print("#MIRE k="); Serial.print(it.k);
      Serial.print(" t=");       Serial.print(it.t);
      Serial.print(" res=");     Serial.print(it.res < R_CANTIDAD ? CODIGO_RESULTADO[it.res] : "?");
      Serial.print(" miro=");    Serial.print(it.miro);
      Serial.print(" cuadros="); Serial.print(it.cuadros);
      Serial.print(" d=");       imprimirDecimas(it.d);
      Serial.print(" y=");       imprimirDecimas(it.y);
      Serial.print(" vy=");      imprimirDecimas(it.vy);
      Serial.print(" va=");      imprimirDecimas(it.va);
      Serial.print(" av=");      Serial.print(it.av);
      Serial.print(" lat=");     Serial.print(it.lat);
      Serial.print(" ms=");      Serial.print(it.ms);
      Serial.print(" ang=");     imprimirDecimas(it.ang);
      Serial.print(" recto=");   Serial.print(it.recto);
      finDeLinea();
    }
  }
  // #R: siempre los 29 campos, en este orden.
  Serial.print("#R n="); Serial.print(d.n);
  Serial.print(" mov=");  Serial.print(NOMBRE_MOV[d.n]);
  Serial.print(" nota="); Serial.print(NOMBRE_NOTA[d.nota]);
  Serial.print(" como="); Serial.print(NOMBRE_COMO[d.como]);
  campoDecimas("pico", d.pico);         campo("picoms", d.picoms);
  campoDecimas("fin", d.fin);           campoDecimas("medio", d.medio);
  campo("cruces", d.cruces);            campo("fuera3", d.fuera3);
  campo("corr", d.corr);                campo("tope", d.tope);
  campo("mudas", d.mudas);              campo("racha", d.racha);
  campo("hueco", d.hueco);              campo("huecoms", d.huecoms);
  campo("duro", d.duro);                campo("lineams", d.lineams);
  campo("idams", d.idams);              campo("atrasms", d.atrasms);
  campo("acomodo", d.acomodo);          campo("laterales", d.laterales);
  campo("idas", d.idas);                campo("arranques", d.arranques);
  campo("conpelota", d.conpelota);      campo("zonamuerta", d.zonamuerta);
  campoDecimas("velmedia", d.velmedia); campo("intentos", d.intentos);
  campo("sale", d.sale);
  finDeLinea();
  // #USADOS: los distintos de los del partido CUANDO se corrio.
  Serial.print("#USADOS");
  if (movMismo) {
    for (int i = 0; i < N_AJUSTES; i++) {
      if (fabs(usadosRAM[i] - ajustes[i].defecto) <= 0.001) continue;
      Serial.print(' '); Serial.print(ajustes[i].nombre); Serial.print('=');
      imprimirNumero(usadosRAM[i], ajustes[i].tipo);
    }
  } else {
    for (int j = 0; j < d.nUsados; j++) {
      int i = d.usadoIdx[j];
      Serial.print(' '); Serial.print(ajustes[i].nombre); Serial.print('=');
      imprimirNumero(d.usadoVal[j], ajustes[i].tipo);
    }
  }
  finDeLinea();
  if (!movMismo && d.usadosDeMas > 0) {
    Serial.print("(y "); Serial.print(d.usadosDeMas);
    Serial.print(" numeros cambiados mas, que no entraron en la EEPROM)"); finDeLinea();
  }
  mandarConsejos(d, apagado);
  // Para personas, en una linea.
  Serial.print("RESULTADO: "); Serial.print(d.n); Serial.print(' '); Serial.print(NOMBRE_MOV[d.n]);
  Serial.print(" -> "); Serial.print(NOMBRE_NOTA[d.nota]);
  Serial.print("  pico "); imprimirDecimasConSigno(d.pico);
  Serial.print(" grados a los "); Serial.print(d.picoms);
  Serial.print(" ms, final "); imprimirDecimasConSigno(d.fin);
  Serial.print(", cruces "); Serial.print(d.cruces);
  Serial.print(", duro "); Serial.print(d.duro);
  Serial.print(" ms, vuelta mas larga "); Serial.print(d.hueco); Serial.print(" ms");
  finDeLinea();
  decir("Grados: NEGATIVO = a la DERECHA, POSITIVO = a la IZQUIERDA.   ? = ayuda");
  Serial.print("#FIN "); Serial.print(d.n); finDeLinea();
}


// =====================================================================
//   LA EEPROM: guardar, leer, validar, marcar como visto
// =====================================================================

void guardarMov() {
  bMov.c.magia    = MAGIA_MOV;
  bMov.c.version  = VERSION_EEPROM;
  bMov.c.codigo   = bMov.d.n;
  bMov.c.nuevo    = 1;                    // hasta que alguien lo vea
  bMov.c.sinCable = sinCablePedido ? 1 : 0;
  bMov.c.largo    = sizeof(DatosMov);
  bMov.c.libre    = 0;
  EEPROM.put(DIR_MOV, bMov);              // solo reescribe los bytes que cambiaron
}

void guardarCancha() {
  bCancha.c.magia    = MAGIA_CANCHA;
  bCancha.c.version  = VERSION_EEPROM;
  bCancha.c.codigo   = (uint8_t)bCancha.d.codigo;
  bCancha.c.nuevo    = 1;
  bCancha.c.sinCable = sinCablePedido ? 1 : 0;
  bCancha.c.largo    = sizeof(DatosCancha);
  bCancha.c.libre    = 0;
  EEPROM.put(DIR_CANCHA, bCancha);
}

bool faseValida(char f) {
  return f == 'l' || f == 's' || f == 'm' || f == 'i' || f == 'p' || f == 'a'
      || f == 'f' || f == 'e' || f == 'c' || f == 'v' || f == 'q';
}

// Una EEPROM virgen viene con 0xFF: si algo no da, es "no hay nada guardado".
// Nunca se imprime basura.
bool movValido() {
  const Cabecera &c = bMov.c;
  const DatosMov &d = bMov.d;
  if (c.magia != MAGIA_MOV || c.version != VERSION_EEPROM || c.largo != sizeof(DatosMov)) return false;
  if (c.codigo < 1 || c.codigo > 10 || d.n != c.codigo) return false;
  if (d.nota < NOTA_PROLIJO || d.nota > NOTA_NO_VALE || d.como > COMO_TOPE) return false;
  if (d.nMuestras > MAX_MUESTRAS_EEPROM || d.nIntentos > MAX_MIRE || d.nUsados > MAX_USADOS_EEPROM) return false;
  for (int i = 0; i < d.nMuestras; i++) if (!faseValida(d.mF[i])) return false;
  for (int k = 0; k < d.nIntentos; k++) if (d.mire[k].res >= R_CANTIDAD) return false;
  for (int j = 0; j < d.nUsados; j++) {
    if (d.usadoIdx[j] >= N_AJUSTES) return false;
    if (isnan(d.usadoVal[j]) || isinf(d.usadoVal[j])) return false;
  }
  return true;
}

bool canchaValida() {
  const Cabecera &c = bCancha.c;
  const DatosCancha &d = bCancha.d;
  if (c.magia != MAGIA_CANCHA || c.version != VERSION_EEPROM || c.largo != sizeof(DatosCancha)) return false;
  if (!esCancha(c.codigo) || d.codigo != (char)c.codigo || d.ok > 1) return false;
  if (d.codigo == 'L') {
    if (d.u.lin.nFases > 3) return false;
    for (int i = 0; i < d.u.lin.nFases; i++) {
      char f = d.u.lin.f[i].fase;
      if (f != 'v' && f != 'i' && f != 'd') return false;
    }
  }
  if (d.codigo == 'F' && d.u.fo.como > FO_PARADO) return false;
  if (d.codigo == 'A' && d.u.an.error > 7) return false;
  if (d.codigo == 'I') {
    for (int i = 0; i < 8; i++) {
      char x = d.u.me.como[i];
      if (x != 'L' && x != 'T' && x != '-') return false;
    }
  }
  return true;
}

void leerGuardados() {
  EEPROM.get(DIR_MOV, bMov);
  if (movValido()) {
    hayMov = true; movMismo = false; movNuevo = (bMov.c.nuevo == 1);
  } else {
    memset(&bMov, 0, sizeof(bMov));
    hayMov = false; movMismo = false; movNuevo = false;
  }
  EEPROM.get(DIR_CANCHA, bCancha);
  if (canchaValida()) {
    hayCancha = true; canchaMismo = false; canchaNuevo = (bCancha.c.nuevo == 1);
  } else {
    memset(&bCancha, 0, sizeof(bCancha));
    hayCancha = false; canchaMismo = false; canchaNuevo = false;
  }
}

// Despues de mandarlo con alguien escuchando: nuevo = 0 (en la RAM y en el
// byte de la cabecera). Se escribe con el robot quieto (estamos en ESPERA,
// o recien frenado).
void marcarMovVisto() {
  movNuevo = false;
  bMov.c.nuevo = 0;
  if (hayMov) EEPROM.update(DIR_MOV + offsetof(Cabecera, nuevo), 0);
}

void marcarCanchaVista() {
  canchaNuevo = false;
  bCancha.c.nuevo = 0;
  if (hayCancha) EEPROM.update(DIR_CANCHA + offsetof(Cabecera, nuevo), 0);
}

// #GUARDADO: lo que quedo guardado (RAM o EEPROM).
void mandarGuardado(bool esMov) {
  Serial.print("#GUARDADO que="); Serial.print(esMov ? "mov" : "cancha");
  Serial.print(" hay=");
  bool hay = esMov ? hayMov : hayCancha;
  if (!hay) { Serial.print('0'); finDeLinea(); return; }
  Serial.print("1 n=");
  bool mismo = false, nuevo = false;
  uint8_t sc = 0;
  unsigned long tFin = 0;
  if (esMov) {
    Serial.print(bMov.d.n);
    Serial.print(" nombre="); Serial.print(NOMBRE_MOV[bMov.d.n]);
    Serial.print(" nota=");   Serial.print(NOMBRE_NOTA[bMov.d.nota]);
    mismo = movMismo; nuevo = movNuevo; sc = bMov.c.sinCable; tFin = t_finMov;
  } else {
    Serial.print(bCancha.d.codigo);
    Serial.print(" nombre="); Serial.print(nombreDe(bCancha.d.codigo, bCancha.d.variante));
    Serial.print(" nota=");   Serial.print(bCancha.d.ok ? "OK" : "ERROR");
    mismo = canchaMismo; nuevo = canchaNuevo; sc = bCancha.c.sinCable; tFin = t_finCancha;
  }
  Serial.print(" sincable="); Serial.print(sc ? 1 : 0);
  Serial.print(" nuevo=");    Serial.print(nuevo ? 1 : 0);
  Serial.print(" mismo=");    Serial.print(mismo ? 1 : 0);
  Serial.print(" hace=");
  if (mismo) Serial.print(millis() - tFin); else Serial.print("-1");
  finDeLinea();
}

void repetirMov() {
  if (!hayMov) { mandarError("no hay nada guardado", "", ""); return; }
  Serial.print("#REPITE "); Serial.print(bMov.d.n); Serial.print(' ');
  Serial.print(NOMBRE_MOV[bMov.d.n]);
  Serial.print(" sincable="); Serial.print(bMov.c.sinCable ? 1 : 0);
  Serial.print(" apagado=");  Serial.print(movMismo ? 0 : 1);
  finDeLinea();
  mandarTandaMov(false);
  marcarMovVisto();
}

void repetirCancha() {
  if (!hayCancha) { mandarError("no hay nada guardado", "", ""); return; }
  Serial.print("#REPITE "); Serial.print(bCancha.d.codigo); Serial.print(' ');
  Serial.print(nombreDe(bCancha.d.codigo, bCancha.d.variante));
  Serial.print(" sincable="); Serial.print(bCancha.c.sinCable ? 1 : 0);
  Serial.print(" apagado=");  Serial.print(canchaMismo ? 0 : 1);
  finDeLinea();
  mandarTandaCancha(false, COMO_NORMAL);
  marcarCanchaVista();
}


// =====================================================================
//   LAS MEDICIONES DE CANCHA (L A F C I)
// =====================================================================

const char* nombreDe(int p, char v) {
  if (p >= 1 && p <= 10) return NOMBRE_MOV[p];
  switch (p) {
    case 'L':
      if (v == 'v') return "LINEA_VERDE";
      if (v == 'i') return "LINEA_BLANCO_IZQ";
      if (v == 'd') return "LINEA_BLANCO_DER";
      return "LINEA_GUIADA";
    case 'A': return "ANCHO";
    case 'F': return (v == 'c') ? "FONDO_COMPROBAR" : "FONDO";
    case 'C': return "CAMARA";
    case 'I': return "IR_AL_MEDIO";
    default:  return "?";
  }
}

void imprimirCodigo(int p) {
  if (p >= 1 && p <= 10) Serial.print(p);
  else                   Serial.print((char)p);
}

// ---------- L: los sensores de linea (la logica de calibrar-linea) ----------
void empezarFaseLinea() {
  for (int i = 0; i < 3; i++) { linMin[i] = 9999; linMax[i] = -1; linSuma[i] = 0; }
  linN = 0;
  t_linMuestra = 0;
}

// Una muestra por milisegundo de los 3 sensores: A13 izq, A11 der, A12 ade.
void muestrearLinea(unsigned long ahora) {
  if (ahora == t_linMuestra) return;
  t_linMuestra = ahora;
  int v[3] = { analogRead(LINEA_ATRAS_IZQ), analogRead(LINEA_ATRAS_DER), analogRead(LINEA_ADELANTE) };
  for (int i = 0; i < 3; i++) {
    if (v[i] < linMin[i]) linMin[i] = v[i];
    if (v[i] > linMax[i]) linMax[i] = v[i];
    linSuma[i] += v[i];
  }
  linN++;
}

void guardarFaseLinea(char f) {
  if (linAcum.nFases >= 3) return;
  LinFase &x = linAcum.f[linAcum.nFases];
  x.fase = f; x.libre = 0;
  x.n = a16(linN);
  for (int i = 0; i < 3; i++) {
    x.mn[i] = (linN > 0) ? (uint16_t)linMin[i] : 0;
    x.mx[i] = (linN > 0) ? (uint16_t)linMax[i] : 0;
    x.prom10[i] = (linN > 0) ? a16(lround(linSuma[i] * 10.0 / linN)) : 0;
  }
  linAcum.nFases++;
}

// Con cable: 4 s de la fase pedida. Guiada (sin cable), como calibrar-linea:
// 1 destello 8 s (al VERDE) y LED fijo 4 s midiendo; 2 destellos (el de
// atras IZQUIERDO sobre la linea) y fijo; 3 destellos (el DERECHO) y fijo.
// Despues el latido lento (en ESPERA) dice que termino bien.
void pasoLinea(unsigned long ahora) {
  parar();
  if (variante != 'c') {
    digitalWrite(LED, HIGH);
    muestrearLinea(ahora);
    if (ahora - t_fase >= 4000) {
      guardarFaseLinea(variante);
      terminarPrueba(COMO_NORMAL);
    }
    return;
  }
  int cual = linPaso / 2;                    // 0 verde, 1 blanco izq, 2 blanco der
  if (linPaso % 2 == 0) {
    destellos(cual + 1, ahora);
    if (ahora - t_fase >= 8000) { empezarFaseLinea(); linPaso++; t_fase = ahora; }
  } else {
    digitalWrite(LED, HIGH);
    muestrearLinea(ahora);
    if (ahora - t_fase >= 4000) {
      const char fases[3] = { 'v', 'i', 'd' };
      guardarFaseLinea(fases[cual]);
      linPaso++; t_fase = ahora;
      if (linPaso >= 6) terminarPrueba(COMO_NORMAL);
    }
  }
}

// ---------- A: el ancho (la logica de medir-ancho) ----------

// Cuanto andar desde un costado para quedar en la mitad. NO es la mitad
// justa del cruce: cada tramo arranca desde quieto y el arranque rinde
// menos (la leccion del empujoncito del 18/08). A 100 con la rampa 10/10
// da cruce/2 + 41: 4344 -> 2213, 4405 -> 2243, 4547 -> 2314, 4615 -> 2348.
unsigned long msHastaElMedio(unsigned long msCruce) {
  unsigned long msRampa = ((unsigned long)VEL_MEDIO_MEDIR / (unsigned long)RAMPA_MOV_PASO) * RAMPA_MOV_MS;
  unsigned long perdida = msRampa / 2 + MS_RETARDO_ARRANQUE;
  return msCruce / 2 + perdida / 2;
}

// Tope de cada tramo, de lo que ya midio (6 s de costado son casi 2 m).
unsigned long topeDelTramo() {
  unsigned long tope = 6000;
  if (anTramo == 0) tope = 4000;
  if (anTramo == 1) tope = anMedioIzq * 3 + 500;
  if (anTramo == 2) tope = anIzqDer * 13 / 10 + 300;
  if (tope > 6000) tope = 6000;
  return tope;
}

// ¿Los numeros tienen sentido? Arrancando del medio, el cruce dura entre
// 1,4 y 3 veces la ida al primer costado, y la vuelta entre 0,7 y 1,43
// veces el cruce. Si no, se fue torcido y piso otra linea.
bool numerosCreibles() {
  if (anTramo == 1) return anIzqDer * 10 >= anMedioIzq * 14 && anIzqDer * 10 <= anMedioIzq * 30;
  return anDerIzq * 10 >= anIzqDer * 7 && anDerIzq * 7 <= anIzqDer * 10;
}

void empezarTramoAncho(int n, unsigned long ahora) {
  anTramo = n;
  anDir = (n == 1 || n == 3) ? +1 : -1;
  if (n == 3) anMsTramo = msHastaElMedio(anIzqDer);
  anCeros = 0;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  anPaso = AN_TRAMO; t_fase = ahora;
}

void mandarPasoAncho() {
  if (!Serial) return;
  Serial.print("#PASO A tramo="); Serial.print(anTramo);
  Serial.print(" ms=");      Serial.print(anMsUltimo);
  Serial.print(" desvio=");  imprimirUnDecimal(anDesvio[anTramo]);
  Serial.print(" endereza="); Serial.print(anEndereza);
  finDeLinea();
}

void fallaAncho(int error) {
  frenar();
  anError = error;
  terminarPrueba(COMO_NORMAL);
}

void pasoAncho(unsigned long ahora) {
  switch (anPaso) {
    case AN_CALIBRA: {
      // Quieto hasta que el giro este calibrado (3), como mucho 10 s.
      parar();
      digitalWrite(LED, ((ahora / 250) % 2) ? HIGH : LOW);
      if (ahora - t_anCal >= 100) {
        t_anCal = ahora;
        uint8_t s = 0, g = 0, a = 0, m = 0;
        bno.getCalibration(&s, &g, &a, &m);
        anCalGiro = g;
      }
      if (ahora - t_anAviso >= 1000) {
        t_anAviso = ahora;
        if (Serial) { Serial.print("#PASO A calibrando giro="); Serial.print(anCalGiro); finDeLinea(); }
      }
      if (anCalGiro < 3 && ahora - t_fase < 10000) break;
      if (algunoDeAtrasVeBlanco()) { fallaAncho(2); return; }
      float r = rumboActual();
      if (r < 0) { fallaAncho(3); return; }
      rumboBase = r;                        // el rumbo a sostener, quieto
      empezarTramoAncho(0, ahora);
      break;
    }
    case AN_TRAMO: {
      digitalWrite(LED, HIGH);
      moverDeCostado(anDir * VEL_MEDIO_MEDIR);   // tambien lee el giroscopio
      if (ultimaMuda) anCeros++;
      else {
        anCeros = 0;
        float e = fabs(ultimoError);
        if (e > anDesvio[anTramo]) anDesvio[anTramo] = e;
      }
      if (anCeros >= 10) { fallaAncho(3); return; }   // el giroscopio se cayo
      unsigned long ms = ahora - t_fase;
      if (anTramo < 3) {
        if (pisaElLateral(anDir)) {
          frenar();                         // frenar, no soltar
          if (anTramo == 0) anMedioIzq = ms;
          if (anTramo == 1) anIzqDer = ms;  // 🎯 las mediciones
          if (anTramo == 2) anDerIzq = ms;
          anMsUltimo = ms;
          if (anTramo >= 1 && !numerosCreibles()) { fallaAncho(5); return; }
          anPaso = AN_PAUSA; t_fase = ahora;
        } else if (ms >= topeDelTramo()) {
          fallaAncho(4); return;
        }
      } else if (ms >= anMsTramo) {
        frenar();
        anMsUltimo = ms;
        anPaso = AN_PAUSA; t_fase = ahora;
      }
      break;
    }
    case AN_PAUSA: {
      // Freno 200 ms y quieto 300: cada tramo arranca desde quieto.
      if (ahora - t_fase < 200) { frenar(); break; }
      parar();
      if (ahora - t_fase < 500) break;
      if (anTramo < 3) {
        // ¿Quedo torcido por el golpe contra el borde? Enderezarse QUIETO.
        float r = rumboActual();
        if (r >= 0) {
          float err = fabs(diferencia(rumboBase, r));
          if (err > anGolpe) anGolpe = err;
          if (err > 3.0) { anEndereza++; anPaso = AN_ENDEREZA; t_fase = ahora; break; }
        }
        mandarPasoAncho();
        empezarTramoAncho(anTramo + 1, ahora);
      } else {
        mandarPasoAncho();
        anError = 0;
        terminarPrueba(COMO_NORMAL);
      }
      break;
    }
    case AN_ENDEREZA: {
      // Gira en el lugar (PWM 75..85, hasta 2500 ms), como medir-ancho.
      digitalWrite(LED, ((ahora / 150) % 2) ? HIGH : LOW);
      float r = rumboActual();
      bool listo = false;
      if (r < 0) {
        listo = true;
      } else {
        float err = diferencia(rumboBase, r);
        if (fabs(err) <= 3.0) listo = true;
        else {
          int pwm = (int)(fabs(err) * KP_RUMBO);
          if (pwm > 85) pwm = 85;
          if (pwm < 75) pwm = 75;
          int c = (err > 0) ? pwm : -pwm;
          aplicar(c, c, c);
        }
      }
      if (ahora - t_fase >= 2500) listo = true;
      if (listo) { frenar(); anPaso = AN_PAUSA2; t_fase = ahora; }
      break;
    }
    case AN_PAUSA2: {
      if (ahora - t_fase < 200) { frenar(); break; }
      parar();
      if (ahora - t_fase < 500) break;
      mandarPasoAncho();
      empezarTramoAncho(anTramo + 1, ahora);
      break;
    }
    default:
      break;
  }
}

// ---------- F: el fondo ----------
// Medir (!F): para atras a VEL_MEDIO_RECTO hasta que aprieten TOCO (t), vea
// la linea, o 4 s. Comprobar (!F c): lo mismo que el paso 3 de ir al medio
// del partido: hasta la linea o MS_MEDIO_FONDO_POR_TIEMPO.
void fondoParo(uint8_t como, unsigned long ms) {
  foComo = como;
  foMs = ms;
  frenar();
  fase = P_FONDO_FRENO; t_fase = millis();
}

void pasoFondo(unsigned long ahora) {
  if (fase == P_FONDO) {
    digitalWrite(LED, HIGH);
    atrasControlado(VEL_MEDIO_RECTO);
    unsigned long ms = ahora - t_fase;
    bool comprobar = (variante == 'c');
    if (algunoDeAtrasVeBlanco())                               fondoParo(FO_LINEA, ms);
    else if (!comprobar && ms >= 4000)                         fondoParo(FO_TOPE, ms);
    else if (comprobar && ms >= MS_MEDIO_FONDO_POR_TIEMPO)     fondoParo(FO_TIEMPO, ms);
  } else {
    frenar();
    if (ahora - t_fase >= msFreno) { parar(); terminarPrueba(COMO_NORMAL); }
  }
}

// ---------- C: la camara ----------
// TABLERO M4: leerCamara() llama a esto con cada paquete mientras se mide.
void anotarCamara(int xAm, int xAz) {
  camN++;
  if (Xp == 0) camCero++;
  else {
    camVal++;
    camXpSuma += Xp;
    camYpSuma += Yp;
    if (Xp < camXpMin) camXpMin = Xp;
    if (Xp > camXpMax) camXpMax = Xp;
  }
  if (xAm > 0) camAm++;
  if (xAz > 0) camAz++;
}

void pasoCamara(unsigned long ahora) {
  parar();
  digitalWrite(LED, HIGH);
  if (ahora - t_fase >= 3000) {
    camMidiendo = false;
    terminarPrueba(COMO_NORMAL);
  }
}

// Arma el resultado de cancha en bCancha.d.
void cerrarCancha(Como como, unsigned long ahora) {
  DatosCancha &c = bCancha.d;
  memset(&c, 0, sizeof(c));
  c.codigo   = (char)prueba;
  c.variante = variante;
  bool corte = (como == COMO_TECLA || como == COMO_TOPE);
  switch (prueba) {
    case 'L': {
      c.u.lin = linAcum;
      int pedidas = (variante == 'c') ? 3 : 1;
      c.ok = (!corte && linAcum.nFases == pedidas) ? 1 : 0;
      break;
    }
    case 'A': {
      DatosAncho &a = c.u.an;
      int err = anError;
      if (como == COMO_TECLA) err = 6;
      if (como == COMO_TOPE)  err = 7;
      a.error      = (uint8_t)err;
      a.medioizq   = a16(anMedioIzq);
      a.izqder     = a16(anIzqDer);
      a.derizq     = a16(anDerIzq);
      a.almedioizq = anIzqDer ? a16(msHastaElMedio(anIzqDer)) : 0;
      a.almedioder = anDerIzq ? a16(msHastaElMedio(anDerIzq)) : 0;
      for (int i = 0; i < 4; i++) a.desvio[i] = s16(lround(anDesvio[i] * 10));
      a.endereza   = (uint8_t)constrain(anEndereza, 0, 255);
      a.golpe      = s16(lround(anGolpe * 10));
      a.vel        = VEL_MEDIO_MEDIR;
      a.rampa      = (uint16_t)RAMPA_MOV_PASO;
      a.umbral     = (uint16_t)umbralBlanco;
      a.calgiro    = (uint8_t)constrain(anCalGiro, 0, 255);
      c.ok = (err == 0) ? 1 : 0;
      break;
    }
    case 'F': {
      DatosFondo &f = c.u.fo;
      f.como = corte ? FO_PARADO : foComo;
      f.ms   = a16(corte ? ahora - t_inicio : foMs);
      f.desvio  = s16(lround(peorEnMov * 10));
      f.umbral  = (uint16_t)umbralBlanco;
      f.probado = (variante == 'c') ? a16(MS_MEDIO_FONDO_POR_TIEMPO) : 0;
      c.ok = (f.como == FO_TOCO || f.como == FO_LINEA || f.como == FO_TIEMPO) ? 1 : 0;
      break;
    }
    case 'C': {
      DatosCam &k = c.u.ca;
      k.n = a16(camN); k.cero = a16(camCero); k.val = a16(camVal);
      k.am = a16(camAm); k.az = a16(camAz);
      if (camVal > 0) {
        k.xpmin = (uint8_t)camXpMin; k.xpmax = (uint8_t)camXpMax;
        k.xpprom10 = a16(lround(camXpSuma * 10.0 / camVal));
        k.yp10 = s16(lround(camYpSuma * 10.0 / camVal));
      }
      c.ok = (!corte && camN > 0) ? 1 : 0;
      break;
    }
    case 'I': {
      DatosMedio &m = c.u.me;
      for (int i = 0; i < 8; i++) { m.pasos[i] = mePasos[i]; m.como[i] = meComo[i]; }
      m.desvio  = s16(lround(peorEnMov * 10));
      m.acomodo = (uint8_t)constrain(acomodoMax, 0, 255);
      c.ok = (!corte && encontroLaLinea) ? 1 : 0;
      break;
    }
    default:
      break;
  }
}

const char* textoErrorAncho(int e) {
  switch (e) {
    case 0: return "midio bien";
    case 2: return "arranco con un sensor de atras sobre una linea";
    case 3: return "sin giroscopio (no aparecio, o se cayo andando)";
    case 4: return "no encontro la linea a tiempo (freno de emergencia)";
    case 5: return "los numeros no tienen sentido (estaba en el medio? iba torcido?)";
    case 6: return "lo frenaron";
    case 7: return "se paso del tope de tiempo";
    default: return "?";
  }
}

void mandarLin(const LinFase &x) {
  Serial.print("#LIN fase="); Serial.print(x.fase);
  Serial.print(" n="); Serial.print(x.n);
  const char* nombres[3] = { " izq=", " der=", " ade=" };
  for (int i = 0; i < 3; i++) {
    Serial.print(nombres[i]);
    Serial.print(x.mn[i]); Serial.print(','); Serial.print(x.mx[i]); Serial.print(',');
    imprimirDecimas(x.prom10[i]);
  }
  finDeLinea();
}

// La tanda de cancha: #LIN/#ANCHO/#FONDO/#CAM/#MEDIO, texto y #FIN.
void mandarTandaCancha(bool conFreno, Como como) {
  const DatosCancha &c = bCancha.d;
  if (conFreno) {
    if (como == COMO_TECLA) decir("#FRENO tecla");
    else if (como == COMO_TOPE) decir("#FRENO tope de tiempo");
  }
  switch (c.codigo) {
    case 'L':
      // Solo las fases COMPLETAS: una a medias no sirve para el umbral.
      for (int i = 0; i < c.u.lin.nFases; i++) mandarLin(c.u.lin.f[i]);
      break;
    case 'A': {
      const DatosAncho &a = c.u.an;
      Serial.print("#ANCHO ok="); Serial.print(c.ok);
      Serial.print(" error=");      Serial.print(a.error);
      Serial.print(" medioizq=");   Serial.print(a.medioizq);
      Serial.print(" izqder=");     Serial.print(a.izqder);
      Serial.print(" derizq=");     Serial.print(a.derizq);
      Serial.print(" almedioizq="); Serial.print(a.almedioizq);
      Serial.print(" almedioder="); Serial.print(a.almedioder);
      Serial.print(" desvio=");
      for (int i = 0; i < 4; i++) { if (i) Serial.print(','); imprimirDecimas(a.desvio[i]); }
      Serial.print(" endereza=");   Serial.print(a.endereza);
      Serial.print(" golpe=");      imprimirDecimas(a.golpe);
      Serial.print(" vel=");        Serial.print(a.vel);
      Serial.print(" rampa=");      Serial.print(a.rampa);
      Serial.print(" umbral=");     Serial.print(a.umbral);
      Serial.print(" calgiro=");    Serial.print(a.calgiro);
      finDeLinea();
      Serial.print("RESULTADO: A ANCHO -> "); Serial.print(c.ok ? "OK, " : "ERROR, ");
      Serial.print(textoErrorAncho(a.error));
      if (c.ok) { Serial.print(". Al medio desde la izquierda: "); Serial.print(a.almedioizq); Serial.print(" ms"); }
      finDeLinea();
      break;
    }
    case 'F': {
      const DatosFondo &f = c.u.fo;
      Serial.print("#FONDO ms="); Serial.print(f.ms);
      Serial.print(" como=");    Serial.print(NOMBRE_FONDO[f.como]);
      Serial.print(" desvio=");  imprimirDecimas(f.desvio);
      Serial.print(" umbral=");  Serial.print(f.umbral);
      Serial.print(" probado="); Serial.print(f.probado);
      finDeLinea();
      break;
    }
    case 'C': {
      const DatosCam &k = c.u.ca;
      if (!c.ok) { decir("(la camara no mando nada, o la frenaron: no hay medicion)"); break; }
      Serial.print("#CAM n="); Serial.print(k.n);
      Serial.print(" cero=");  Serial.print(k.cero);
      Serial.print(" val=");   Serial.print(k.val);
      Serial.print(" xp=");    Serial.print(k.xpmin); Serial.print(','); Serial.print(k.xpmax);
      Serial.print(',');       imprimirDecimas(k.xpprom10);
      Serial.print(" yp=");    imprimirDecimas(k.yp10);
      Serial.print(" am=");    Serial.print(k.am);
      Serial.print(" az=");    Serial.print(k.az);
      finDeLinea();
      break;
    }
    case 'I': {
      const DatosMedio &m = c.u.me;
      Serial.print("#MEDIO ok="); Serial.print(c.ok);
      Serial.print(" pasos=");
      for (int i = 0; i < 8; i++) { if (i) Serial.print(','); Serial.print(m.pasos[i]); }
      Serial.print(" como=");
      for (int i = 0; i < 8; i++) { if (i) Serial.print(','); Serial.print(m.como[i]); }
      Serial.print(" desvio=");  imprimirDecimas(m.desvio);
      Serial.print(" acomodo="); Serial.print(m.acomodo);
      finDeLinea();
      break;
    }
    default:
      break;
  }
  Serial.print("#FIN "); Serial.print(c.codigo); finDeLinea();
}


// =====================================================================
//   ESPERA / CUENTA / MOVIENDO
// =====================================================================

// Vacia lo que haya llegado. true si habia alguna tecla de verdad (un "\r"
// o "\n" sueltos no cuentan). De patada-derecha, via el delantero.
bool hayTeclaDeVerdad() {
  bool hay = false;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c != '\r' && c != '\n') hay = true;
  }
  return hay;
}

// Moviendose: 0 nada, 1 tecla (FRENA), 2 la t de TOCO (solo midiendo el fondo).
int leerTeclaMoviendo() {
  int r = 0;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') continue;
    if (prueba == 'F' && fase == P_FONDO_FRENO) continue;   // ya esta frenando con su resultado
    if (r != 0) continue;
    bool toco = (prueba == 'F' && variante != 'c' && fase == P_FONDO && (c == 't' || c == 'T'));
    r = toco ? 2 : 1;
  }
  return r;
}

// Despues de un freno a tecla: tirar lo que se mando junto (el "\n" de
// "x\n"), para que no se lea como una orden nueva.
void tirarLoQueLlego() {
  unsigned long t0 = millis();
  while (millis() - t0 < 50) { while (Serial.available()) Serial.read(); }
  largoLinea = 0;
  lineaLarga = false;
}

unsigned long topeDe(int p) {
  switch (p) {
    case 1: case 2: case 3: case 4: return (unsigned long)msCostado + 3000;
    case 5:           return 2UL * (unsigned long)msCostado + 3000;
    case 6: case 7:   return 20000;
    case 8: case 9:   return (unsigned long)msSeguir + 20000;
    case 10:          return (unsigned long)msPruebaIda + 3000;
    case 'L':         return (variante == 'c') ? 50000 : 5000;
    case 'A':         return 45000;
    case 'F':         return (variante == 'c') ? 8000 : 6000;
    case 'C':         return 4000;
    case 'I':         return 40000;
    default:          return 3000;
  }
}

// No arranco: ni #INICIO ni informe.
void noArranca(const char* motivo) {
  modo = ESPERA;
  digitalWrite(LED, LOW);
  if (Serial) { Serial.print("#FRENO "); decir(motivo); }
}

// Los chequeos de antes de arrancar (seccion 4.3), y la cuenta.
void empezarCuenta(int p, char var, bool sinCable) {
  // 1. ¿soy el arquero?
  if (!soyElArquero) {
    noArranca("no es el arquero");
    Serial.print("ESTE TEENSY NO ES EL ARQUERO (serie "); Serial.print(serieMia);
    Serial.print("): no muevo nada."); finDeLinea();
    return;
  }
  // 2. recien parado
  if (t_parada != 0 && millis() - t_parada < MS_ESPERA_DESPUES_DE_PARAR) {
    noArranca("recien parado");
    decir("espera: recien se paro. Volve a pedirlo en un segundo.");
    return;
  }
  bool noMueve = (p == 'L' || p == 'C');
  // 3. el giroscopio (L y C no mueven: no lo necesitan)
  if (!noMueve && (!hayGiroscopo || rumboActual() < 0)) {
    decir("Sin giroscopio. Lo busco de nuevo (tarda hasta 6 s)...");
    buscarGiroscopio(1);
    if (!hayGiroscopo || rumboActual() < 0) {
      noArranca("sin giroscopio");
      decir("No se mueve sin giroscopio. Bateria prendida ANTES del USB? Cable del giroscopio?");
      return;
    }
  }
  // 4. sobre la linea (si arranca sobre una linea, no ve el lateral)
  bool miraLinea = (p >= 1 && p <= 5) || p == 8 || p == 9 || p == 'A' || p == 'I' || p == 'F';
  if (miraLinea && algunoDeAtrasVeBlanco()) {
    noArranca("sobre la linea");
    decir("un sensor de atras esta sobre una linea blanca: corranlo ~10 cm para adelante");
    return;
  }
  // 5. la 9 necesita la camara
  if (p == 9 && (t_ultimoPaquete == 0 || millis() - t_ultimoPaquete >= 500)) {
    noArranca("sin camara");
    decir("la camara no manda datos (bateria prendida? cable de la camara?)");
    return;
  }
  // 6. a contar
  prueba = p;
  variante = var;
  sinCablePedido = sinCable;
  Serial.print(">>> "); imprimirCodigo(p); Serial.print(' '); Serial.print(nombreDe(p, var));
  if (noMueve && !sinCable) {
    // !L y !C con cable no mueven: no tienen cuenta.
    if (p == 'L') Serial.print(": midiendo 4 s, no lo muevas.");
    else          Serial.print(": 3 s mirando la camara, no lo muevas.");
    finDeLinea();
    arrancarMovimiento();
    return;
  }
  if (sinCable) {
    Serial.print(" SIN CABLE: arranca en 10 s. DESENCHUFEN EL USB.");
    if (p == 'L') Serial.print(" Despues segui el LED: 1 destello = al VERDE, 2 = blanco IZQ, 3 = blanco DER, fijo = midiendo.");
    else if (esCancha(p)) Serial.print(" Al terminar, el LED dice como salio.");
    else Serial.print(" Al terminar, el LED dice la nota.");
  } else {
    Serial.print(" en 3 s. Saca la mano. Cualquier tecla lo frena.");
  }
  finDeLinea();
  modo = CUENTA;
  t_cuenta = millis();
  msDeCuenta = sinCable ? MS_CUENTA_SIN_CABLE : MS_CUENTA;
  avisosCuenta = 0;
  pasoDeLaCuenta(t_cuenta);           // el primer #CUENTA sale ya
}

void pasoDeLaCuenta(unsigned long ahora) {
  digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);
  if (hayTeclaDeVerdad()) {
    noArranca("tecla");
    if (Serial) decir("cancelado: no se movio");
    t_parada = millis();
    tirarLoQueLlego();
    return;
  }
  unsigned long pasado = ahora - t_cuenta;
  unsigned long periodo = sinCablePedido ? 1000 : 500;
  while ((unsigned long)avisosCuenta * periodo <= pasado
         && (unsigned long)avisosCuenta * periodo < msDeCuenta) {
    if (Serial) {
      Serial.print("#CUENTA ");
      imprimirUnDecimal((msDeCuenta - (unsigned long)avisosCuenta * periodo) / 1000.0);
      Serial.print(sinCablePedido ? " sincable" : " cable");
      finDeLinea();
    }
    avisosCuenta++;
  }
  if (pasado >= msDeCuenta) arrancarMovimiento();
}

// Al terminar la cuenta: deja todo listo y arranca (seccion 4.3).
void arrancarMovimiento() {
  unsigned long ahora = millis();
  bool noMueve = (prueba == 'L' || prueba == 'C');
  if (!noMueve) {
    float r = rumboActual();
    if (!hayGiroscopo || r < 0) { noArranca("sin giroscopio"); return; }
    rumboBase = r;                    // el rumbo que tenia al terminar la cuenta
  }
  // Ultima oportunidad de frenar ANTES de mover.
  if (hayTeclaDeVerdad()) {
    noArranca("tecla");
    if (Serial) decir("cancelado: no se movio");
    t_parada = millis();
    tirarLoQueLlego();
    rumboBase = -1;
    return;
  }
  reiniciarCorreccion();
  reiniciarRampaMovimiento();
  lecturasDeLateral = 0;
  vecesSeguidas = 0;
  hayMirada = false;
  t_finMirada = 0;                    // asi no arranca "descansando"
  enderezandoEnEspera = false; ubicandose = false; patrullando = false; empujonDeArranque = false;
  encontroLaLinea = false;
  t_chequeoEspera = 0; t_ultimoIntentoEnderezar = 0;
  topePrueba = topeDe(prueba);
  empezarMedicion(ahora);
  // Lo de las mediciones de cancha
  memset(&linAcum, 0, sizeof(linAcum));
  empezarFaseLinea(); linPaso = 0;
  anPaso = AN_CALIBRA; anTramo = 0; anDir = -1; anCeros = 0; anEndereza = 0; anError = 0; anCalGiro = 0;
  anMsTramo = 0; anMsUltimo = 0; t_anCal = 0; t_anAviso = 0;
  anMedioIzq = 0; anIzqDer = 0; anDerIzq = 0; anGolpe = 0;
  for (int i = 0; i < 4; i++) anDesvio[i] = 0;
  foComo = FO_TOPE; foMs = 0;
  camN = 0; camCero = 0; camVal = 0; camAm = 0; camAz = 0; camXpSuma = 0; camYpSuma = 0;
  camXpMin = 255; camXpMax = 0; camMidiendo = false;
  for (int i = 0; i < 8; i++) { mePasos[i] = 0; meComo[i] = '-'; }

  if (Serial) {
    Serial.print("#INICIO "); imprimirCodigo(prueba); Serial.print(' ');
    Serial.print(nombreDe(prueba, variante));
    Serial.print(" tope="); Serial.print(topePrueba);
    Serial.print(" sincable="); Serial.print(sinCablePedido ? 1 : 0);
    finDeLinea();
  }
  // De aca hasta terminar no se manda NADA (salvo los #PASO de A e I).
  modo = MOVIENDO;
  t_inicio = ahora;
  msLoopPrevio = ahora;
  t_fase = ahora;
  switch (prueba) {
    case 1: ladoCostado = -1; velCostado = velLenta;      fase = COSTADO; break;
    case 2: ladoCostado = +1; velCostado = velLenta;      fase = COSTADO; break;
    case 3: ladoCostado = -1; velCostado = pwmMaxLateral; fase = COSTADO; break;
    case 4: ladoCostado = +1; velCostado = pwmMaxLateral; fase = COSTADO; break;
    case 5: ladoCostado = -1; velCostado = pwmMaxLateral; fase = COSTADO_VUELTA; break;
    case 6:
    case 10:
      // Lo mismo que hace intentarDespejar() al salir derecho.
      lateralDelDespeje = 0;
      pwmAvanceDespeje  = potenciaDespeje;
      msIdaDespeje      = (prueba == 10) ? (unsigned long)msPruebaIda : (unsigned long)msAdelante;
      fase = ADELANTE;
      break;
    case 7: arrancarUbicacion(); break;
    case 8:
    case 9: pasarAEsperar(); break;
    case 'L': fase = P_LINEA; break;
    case 'A': fase = P_ANCHO; break;
    case 'F': fase = P_FONDO; break;
    case 'C': fase = P_CAMARA; camMidiendo = true; break;
    case 'I':
      // Como el partido cuando no encuentra la linea al ubicarse.
      ubicandose = true;
      encontroLaLinea = false;
      pausaMedio(MEDIO_AL_LATERAL);
      break;
    default:
      noArranca("tecla");
      return;
  }
  t_fase = ahora;
  digitalWrite(LED, HIGH);
}

// Una vuelta del loop mientras se mueve: el paso, la medicion y lo que se
// mira desde afuera.
void pasoDeLaPrueba(unsigned long ahora) {
  Fase antes = fase;
  unsigned long tAntes = t_fase;
  leyoGiroEnEstaVuelta = false;
  switch (fase) {
    case COSTADO: case COSTADO_VUELTA: case P_FRENO: case P_QUIETO:
      pasoDeFaseDePrueba(ahora); break;
    case P_LINEA:  pasoLinea(ahora);  break;
    case P_ANCHO:  pasoAncho(ahora);  break;
    case P_FONDO: case P_FONDO_FRENO: pasoFondo(ahora); break;
    case P_CAMARA: pasoCamara(ahora); break;
    default:       pasoDelPartido(ahora); break;
  }
  if (modo != MOVIENDO) return;            // termino en esta vuelta
  bool corrigio = leyoGiroEnEstaVuelta;
  if (midoElGiro()) {
    if (!corrigio) medirSinCorregir();
    char letra = letraDeFase(antes);
    anotar(ahora, antes, letra, letraEnMovimiento(letra) && corrigio);
  }
  if (antes < COSTADO) observarPartido(antes, tAntes, ahora, corrigio);
}

// Termina la prueba: en este orden (seccion 7.7).
void terminarPrueba(Como como) {
  frenar();                                 // lo primero: quieto
  unsigned long ahora = millis();
  enMovAnterior = false;
  camMidiendo = false;
  bool cancha = esCancha(prueba);
  if (cancha) cerrarCancha(como, ahora);
  else        cerrarMovimiento(como, ahora);
  // La EEPROM, con el robot ya frenado. Siempre, con o sin cable.
  if (cancha) {
    guardarCancha();
    hayCancha = true; canchaMismo = true; canchaNuevo = true; t_finCancha = ahora;
    ultimoResultado = 'c';
  } else {
    guardarMov();
    hayMov = true; movMismo = true; movNuevo = true; t_finMov = ahora;
    ultimoResultado = 'm';
  }
  // Se manda solo si hay alguien escuchando; si no, queda nuevo = 1.
  // SIN CABLE (2026-11-03, revision): aunque "haya alguien", NO se marca
  // como visto. POR QUE: el Teensy 4.1 NO se entera de que lo desenchufaron.
  // Serial dice true si la PC levanto el DTR, y eso solo cambia cuando la PC
  // lo manda; el USB del core no mira el VBUS ni la suspension (usb.c:
  // "TODO: return to low power mode when VBUS removed"). Asi que al
  // desenchufar Serial SIGUE en true y la tanda se manda AL VACIO. Con
  // nuevo = 1, al volver a enchufar el #GUARDADO dice nuevo=1 y la ventana
  // lo pide con r / rc. Si la ventana SI lo vio en vivo (no lo
  // desenchufaron), se da cuenta por el hace= y no lo pide de nuevo.
  if (Serial) {
    if (cancha) { mandarTandaCancha(true, como); if (!sinCablePedido) marcarCanchaVista(); }
    else        { mandarTandaMov(true);          if (!sinCablePedido) marcarMovVisto(); }
  }
  modo = ESPERA;
  rumboBase = -1;
  fase = APAGADO;
  t_freno = millis();                       // frena 400 ms y despues suelta
  if (como == COMO_TECLA) { t_parada = millis(); tirarLoQueLlego(); }
}


// =====================================================================
//   EL LED EN ESPERA (seccion 7.8)
// =====================================================================

// `veces` destellos de 200 ms cada 400 ms y una pausa de 1200 ms: se puede
// contar de lejos y sin computadora (la destellos() de medir-ancho).
void destellos(int veces, unsigned long ahora) {
  unsigned long ciclo = ahora % ((unsigned long)veces * 400 + 1200);
  if (ciclo < (unsigned long)veces * 400) {
    digitalWrite(LED, ((ciclo % 400) < 200) ? HIGH : LOW);
  } else {
    digitalWrite(LED, LOW);
  }
}

void ledEnEspera(unsigned long ahora) {
  bool temblar = (ahora / MS_PARPADEO_AVISO) % 2;
  // Sin giroscopio: temblando, como el partido. Manda sobre todo lo demas.
  if (!hayGiroscopo || !giroscopoRespondiendo) { digitalWrite(LED, temblar ? HIGH : LOW); return; }
  if (ultimoResultado == 'm') {
    if (bMov.d.nota == NOTA_NO_VALE) digitalWrite(LED, temblar ? HIGH : LOW);
    else destellos(bMov.d.nota, ahora);      // 1 PROLIJO .. 5 NO_SALIO
    return;
  }
  if (ultimoResultado == 'c') {
    if (bCancha.d.ok) {
      digitalWrite(LED, ((ahora % 2000) < 120) ? HIGH : LOW);   // latido lento = bien
    } else if (bCancha.d.codigo == 'A' && bCancha.d.u.an.error >= 2 && bCancha.d.u.an.error <= 5) {
      destellos(bCancha.d.u.an.error, ahora);                   // el error del ancho
    } else {
      digitalWrite(LED, temblar ? HIGH : LOW);                  // no salio
    }
    return;
  }
  digitalWrite(LED, ((ahora / 1000) % 2) ? HIGH : LOW);         // sin resultado todavia
}


// =====================================================================
//   AYUDITAS PARA ESCRIBIR (sin String y sin tildes)
// =====================================================================
// Todas las lineas terminan en "\n" solo (println agrega "\r\n").
void finDeLinea() { Serial.print('\n'); }
void decir(const char* s) { Serial.print(s); finDeLinea(); }

uint16_t a16(unsigned long v) { return (v > 65535UL) ? 65535 : (uint16_t)v; }
int16_t  s16(long v) { if (v > 32767) return 32767; if (v < -32767) return -32767; return (int16_t)v; }

// Con punto decimal y 1 a 2 decimales: 4.0, 0.8, 0.75, 2.87 (como el panel).
void imprimirDecimal(float v) {
  if (isnan(v) || isinf(v)) { Serial.print("0.0"); return; }
  long c = lround(v * 100.0f);
  if (c < 0) { Serial.print('-'); c = -c; }
  Serial.print(c / 100);
  Serial.print('.');
  Serial.print((c / 10) % 10);
  if (c % 10 != 0) Serial.print(c % 10);
}

// Exactamente 1 decimal y sin "-0.0".
void imprimirUnDecimal(float v) {
  if (isnan(v) || isinf(v)) { Serial.print("0.0"); return; }
  long d = lround(v * 10.0f);
  if (d < 0) { Serial.print('-'); d = -d; }
  Serial.print(d / 10); Serial.print('.'); Serial.print(d % 10);
}

// Un numero guardado en decimas, con 1 decimal (y sin "-0.0").
void imprimirDecimas(long d) {
  if (d < 0) { Serial.print('-'); d = -d; }
  Serial.print(d / 10); Serial.print('.'); Serial.print(d % 10);
}

// Igual, con "+" adelante si no es negativo (para personas).
void imprimirDecimasConSigno(long d) {
  if (d >= 0) Serial.print('+');
  imprimirDecimas(d);
}

void imprimirNumero(float v, char tipo) {
  if (tipo == 'f')      imprimirDecimal(v);
  else if (tipo == 'b') Serial.print(v > 0.5f ? 1 : 0);
  else                  Serial.print(lround(v));
}

void campo(const char* nombre, long v) {
  Serial.print(' '); Serial.print(nombre); Serial.print('='); Serial.print(v);
}

void campoDecimas(const char* nombre, long d) {
  Serial.print(' '); Serial.print(nombre); Serial.print('='); imprimirDecimas(d);
}

long edad(unsigned long t) {
  if (t == 0) return -1;              // nunca se vio
  return (long)(millis() - t);
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
//   LOS NUMEROS (la tabla)
// =====================================================================
float valorDe(int i) {
  switch (ajustes[i].tipo) {
    case 'f': return *(float*)ajustes[i].p;
    case 'u': return (float)*(unsigned long*)ajustes[i].p;
    case 'b': return *(bool*)ajustes[i].p ? 1.0f : 0.0f;
    default:  return (float)*(int*)ajustes[i].p;
  }
}

void ponerValor(int i, float v) {
  switch (ajustes[i].tipo) {
    case 'f': *(float*)ajustes[i].p = (float)lround(v * 100.0f) / 100.0f; break;   // 2 decimales
    case 'u': *(unsigned long*)ajustes[i].p = (unsigned long)lround(v); break;
    case 'b': *(bool*)ajustes[i].p = (v > 0.5f); break;
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

// #V (los de ahora) o #DEF (las copias del partido), en un solo renglon.
void mandarValores(bool defectos) {
  Serial.print(defectos ? "#DEF" : "#V");
  for (int i = 0; i < N_AJUSTES; i++) {
    Serial.print(' ');
    Serial.print(ajustes[i].nombre);
    Serial.print('=');
    imprimirNumero(defectos ? ajustes[i].defecto : valorDe(i), ajustes[i].tipo);
  }
  finDeLinea();
}

void volverAlPrincipal() {
  for (int i = 0; i < N_AJUSTES; i++) ponerValor(i, ajustes[i].defecto);
}

// La orden "v": las lineas para pegar en el PANEL DE CONTROL del partido.
// Van SOLO las que cambiaron y existen alla: pegar las demas no cambiaria
// nada, y si mientras tanto alguien toco el partido, lo pisaria.
void mandarPanel() {
  decir("--- PARA COPIAR AL PANEL DE CONTROL (funciona/seguir-y-despejar) ---");
  decir("Buscar cada nombre (Ctrl+F) y cambiar SOLO el numero.");
  int cuantas = 0, deLaPrueba = 0;
  for (int i = 0; i < N_AJUSTES; i++) {
    if (!cambiado(i)) continue;
    if (ajustes[i].enElPanel[0] == 0) deLaPrueba++;
    else cuantas++;
  }
  if (cuantas == 0) decir("(nada para copiar: estan todos como en el partido)");
  if (deLaPrueba > 0) {
    Serial.print("(no se copian porque son solo de esta prueba:");
    for (int i = 0; i < N_AJUSTES; i++) {
      if (!cambiado(i) || ajustes[i].enElPanel[0] != 0) continue;
      Serial.print(' '); Serial.print(ajustes[i].nombre);
    }
    Serial.print(')'); finDeLinea();
  }
  for (int i = 0; i < N_AJUSTES; i++) {
    if (!cambiado(i) || ajustes[i].enElPanel[0] == 0) continue;
    Serial.print("#PANEL ");
    Serial.print(ajustes[i].enElPanel);
    Serial.print(' ');
    Serial.print(ajustes[i].nombre);
    Serial.print(" = ");
    if (ajustes[i].tipo == 'b') Serial.print(valorDe(i) > 0.5f ? "true" : "false");
    else                        imprimirNumero(valorDe(i), ajustes[i].tipo);
    Serial.print(';');
    finDeLinea();
  }
  decir("#FINPANEL");
}

void ayuda() {
  finDeLinea();
  Serial.print("------------------- TABLERO DEL ARQUERO (copias del partido ");
  Serial.print(PARTIDO_COPIADO); Serial.print(") -------------------"); finDeLinea();
  decir("MOVIMIENTOS (el robot SE MUEVE despues de la cuenta de 3 s; con c al final: 10 s y SIN CABLE):");
  decir("  !1 COSTADO_IZQ   de costado a la izquierda, lento (velLenta)");
  decir("  !2 COSTADO_DER   de costado a la derecha, lento (prueba de lateralInvertido)");
  decir("  !3 RAPIDO_IZQ    de costado a la izquierda a pwmMaxLateral");
  decir("  !4 RAPIDO_DER    de costado a la derecha a pwmMaxLateral");
  decir("  !5 IDA_Y_VUELTA  rapido a la izquierda y vuelta sin frenar");
  decir("  !6 DESPEJE       el despeje derecho del partido, sin pelota");
  decir("  !7 UBICARSE      atras hasta la linea, enderezarse y el poquito");
  decir("  !8 SEGUIR        seguir la pelota de costado, sin despejar (msSeguir)");
  decir("  !9 ATAJAR        seguir + MIRAR + despejar en diagonal: tirar la pelota despues de la cuenta");
  decir("  !10 IDA          solo la ida (msPruebaIda), frena y se queda: medir con regla");
  decir("CANCHA:");
  decir("  !L v / !L i / !L d   la linea 4 s: verde / blanco IZQ / blanco DER (no mueve)");
  decir("  !L c                 la linea guiada por el LED, SIN CABLE");
  decir("  !A  (!A c)           el ancho y MS_MEDIO_AL_MEDIO (mueve de costado)");
  decir("  !F                   el fondo: para atras hasta TOCO (la letra t), la linea o 4 s");
  decir("  !F c                 comprobar MS_MEDIO_FONDO_POR_TIEMPO, sin cable");
  decir("  !C  (!C c)           3 s de camara (no mueve)");
  decir("  !I  (!I c)           ir al medio como el partido, para comprobar (mueve)");
  decir("PARAR: x   (y CUALQUIER tecla en la cuenta o mientras se mueve; Enter solo no).");
  decir("       SIN CABLE, PARAR no llega: levantarlo o apagar la bateria.");
  decir("CAMBIAR UN NUMERO: =NOMBRE valor   por ejemplo  =umbralCm 32");
  decir("OTRAS: ? ayuda   v copiar al panel   0 volver a los del partido   h saludo");
  decir("       g buscar el giroscopio   r repetir el ultimo movimiento   rc repetir la ultima de cancha");
  Serial.print("Serie de este Teensy: "); Serial.print(serieMia);
  decir(soyElArquero ? " (ARQUERO)" : " (NO ES EL ARQUERO: no muevo nada)");
  Serial.print("Giroscopio: ");
  if (!hayGiroscopo)               decir("NO CONTESTA. Sin el no se mueve: bateria prendida? g para buscarlo.");
  else if (!giroscopoRespondiendo) decir("contesta, pero NO DA DATOS (bateria prendida?). g para buscarlo.");
  else                             decir("bien (modo NDOF, como el partido)");
  Serial.print("Camara: ");
  if (paquetesPorSegundo > 0) { Serial.print(paquetesPorSegundo); decir(" paquetes por segundo"); }
  else                        decir("no manda datos (solo hace falta para 8, 9 y la C)");
  decir("NUMEROS (entre parentesis, el del partido si no es igual):");
  for (int i = 0; i < N_AJUSTES; i++) {
    Serial.print("  ");
    Serial.print(ajustes[i].nombre);
    for (int k = strlen(ajustes[i].nombre); k < 27; k++) Serial.print(' ');
    imprimirNumero(valorDe(i), ajustes[i].tipo);
    if (cambiado(i)) {
      Serial.print("   (partido: ");
      imprimirNumero(ajustes[i].defecto, ajustes[i].tipo);
      Serial.print(')');
    }
    if (ajustes[i].enElPanel[0] == 0) Serial.print("   [solo de esta prueba]");
    finDeLinea();
  }
  decir("Grados: NEGATIVO = a la DERECHA, POSITIVO = a la IZQUIERDA.");
}


// =====================================================================
//   EL GIROSCOPIO Y LA SERIE
// =====================================================================

// SYS_STATUS (registro 0x39) leido a mano por I2C: 5 = la fusion corre, -1
// = no contesta. NUNCA bno.getSystemStatus(): termina con delay(200) y
// clava el programa (la causa del zigzag de medir-ancho del 06/10).
int estadoDelGiroscopio() {
  Wire.beginTransmission(0x28);
  Wire.write(0x39);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(0x28, 1) != 1) return -1;
  return Wire.read();
}

// Arranca el giroscopio COMO EL PARTIDO (bno.begin() = modo NDOF; 1 s;
// setExtCrystalUse; espera hasta 5 s a que de un dato: saludar no alcanza),
// pero con varios intentos si se pide (aca no hay apuro).
void buscarGiroscopio(int intentos) {
  if (Serial) decir("#GIRO buscando");
  hayGiroscopo = false;
  int n = 0;
  while (!hayGiroscopo && n < intentos) {
    n++;
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo && n < intentos) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) delay(50);
  } else {
    giroscopoRespondiendo = false;
  }
  if (Serial) {
    Serial.print("#GIRO ok="); Serial.print((hayGiroscopo && giroscopoRespondiendo) ? 1 : 0);
    Serial.print(" estado="); Serial.print(estadoDelGiroscopio());
    finDeLinea();
  }
}

// El numero de serie de ESTE Teensy, calculado igual que Teensyduino
// (usb_init_serialnumber del Teensy 4). Si no es el del arquero, no mueve nada.
uint32_t serieDeEsteTeensy() {
  uint32_t num = HW_OCOTP_MAC0 & 0xFFFFFF;
  if (num < 10000000) num = num * 10;
  return num;
}

void saludar() {
  Serial.print("#HOLA "); Serial.print(SALUDO_PROGRAMA); Serial.print(' ');
  Serial.print(FECHA_CONTRATO);
  Serial.print(" serie="); Serial.print(serieMia);
  Serial.print(" partido="); Serial.print(PARTIDO_COPIADO);
  finDeLinea();
  mandarGuardado(true);
  mandarGuardado(false);
}


// =====================================================================
//   LO QUE SE MANDA EN ESPERA (#S)
// =====================================================================

// Cada 100 ms en ESPERA: el rumbo (tambien para el LED, aunque no haya
// nadie), la vista previa de la diagonal, y el #S si hay alguien.
void cadaCienMs() {
  float r = rumboActual();            // actualiza giroscopoRespondiendo
  // dg: "si saliera AHORA" (seccion 7.4). calcularDiagonal necesita un
  // rumboBase: se pone el de ahora y despues se restaura (en ESPERA es -1).
  float guardado = rumboBase;
  bool  miradaGuardada = hayMirada;
  if (veLaPelota() && r >= 0) { rumboBase = r; hayMirada = false; }
  int av = 0, lat = 0;
  unsigned long ms = 0;
  bool sale = calcularDiagonal(av, lat, ms);
  rumboBase = guardado;
  hayMirada = miradaGuardada;
  dgSale = sale ? 1 : 0;
  dgMotivo = sale ? 0 : motivoNoSale;
  dgAvance = av; dgLateral = lat; dgMs = ms;
  if (Serial) mandarEstado(r);
}

void mandarEstado(float r) {
  Serial.print("#S t="); Serial.print(millis());
  Serial.print(" l=");
  Serial.print(analogRead(LINEA_ATRAS_IZQ)); Serial.print(',');
  Serial.print(analogRead(LINEA_ATRAS_DER)); Serial.print(',');
  Serial.print(analogRead(LINEA_ADELANTE));
  Serial.print(" u="); Serial.print(umbralBlanco);
  Serial.print(" r="); if (r < 0) Serial.print("-1"); else Serial.print(r, 1);
  Serial.print(" g="); Serial.print(hayGiroscopo ? estadoDelGiroscopio() : -1);
  Serial.print(" cal=");
  if (hayGiroscopo) {
    uint8_t s = 0, g = 0, a = 0, m = 0;
    bno.getCalibration(&s, &g, &a, &m);
    Serial.print(s); Serial.print(','); Serial.print(g); Serial.print(',');
    Serial.print(a); Serial.print(','); Serial.print(m);
  } else {
    Serial.print("-1,-1,-1,-1");
  }
  bool nunca = (t_pelotaBuena == 0);
  Serial.print(" p="); Serial.print(XpBueno); Serial.print(','); Serial.print(YpBueno);
  Serial.print(',');   Serial.print(edad(t_pelotaBuena));
  Serial.print(" d="); if (nunca) Serial.print("-1.0"); else imprimirUnDecimal(XpBueno / CAMARA_POR_CM);
  Serial.print(" y=");
  if (nunca) Serial.print("0.0");
  else       imprimirUnDecimal((camaraYInvertida ? -YpBueno : YpBueno) / CAMARA_POR_CM);
  Serial.print(" vy="); imprimirUnDecimal(velocidadLateral);
  Serial.print(" va="); imprimirUnDecimal(velocidadAcercamiento);
  Serial.print(" ve="); Serial.print(veLaPelota() ? 1 : 0);
  Serial.print(" n=");  Serial.print(vecesSeguidas);
  Serial.print(" dg="); Serial.print(dgSale); Serial.print(','); Serial.print(dgMotivo);
  Serial.print(',');    Serial.print(dgAvance); Serial.print(','); Serial.print(dgLateral);
  Serial.print(',');    Serial.print(dgMs);
  Serial.print(" am="); Serial.print(Xam); Serial.print(','); Serial.print(Yam);
  Serial.print(',');    Serial.print(edad(t_amarillo));
  Serial.print(" az="); Serial.print(Xaz); Serial.print(','); Serial.print(Yaz);
  Serial.print(',');    Serial.print(edad(t_azul));
  Serial.print(" q=");  Serial.print(paquetesPorSegundo);
  Serial.print(" e=ESPERA");
  finDeLinea();
}


// =====================================================================
//   LAS ORDENES
// =====================================================================

void mandarError(const char* a, const char* b, const char* c) {
  Serial.print("#ERROR "); Serial.print(a); Serial.print(b); Serial.print(c);
  finDeLinea();
}

// "=NOMBRE valor" (llega sin el "="). Las validaciones son las del
// delantero, mas los si/no.
void cambiarNumero(char* s) {
  while (*s == ' ' || *s == '\t') s++;
  char* sep = s;
  while (*sep && *sep != ' ' && *sep != '=' && *sep != '\t') sep++;
  char* val = sep;
  if (*sep) { *sep = 0; val = sep + 1; }
  while (*val == ' ' || *val == '=' || *val == '\t') val++;

  if (*s == 0) { mandarError("falta el nombre. Se escribe: =NOMBRE valor   (ej: =umbralCm 32)", "", ""); return; }
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
  if (ajustes[i].tipo == 'b') {
    if (fabs(v) > 1e-6 && fabs(v - 1.0) > 1e-6) { mandarError(ajustes[i].nombre, " es si o no: va 0 o 1", ""); return; }
  } else if (ajustes[i].tipo != 'f' && fabs(v) < 1e9 && fabs(v - floor(v + 0.5)) > 1e-6) {
    mandarError(ajustes[i].nombre, " va sin decimales", ""); return;
  }
  if (v < ajustes[i].minimo - 1e-6 || v > ajustes[i].maximo + 1e-6) {
    Serial.print("#ERROR "); Serial.print(ajustes[i].nombre);
    Serial.print(" fuera de rango: va de "); imprimirNumero(ajustes[i].minimo, ajustes[i].tipo);
    Serial.print(" a "); imprimirNumero(ajustes[i].maximo, ajustes[i].tipo);
    finDeLinea();
    return;
  }
  ponerValor(i, (float)v);
  Serial.print("#OK "); Serial.print(ajustes[i].nombre); Serial.print('=');
  imprimirNumero(valorDe(i), ajustes[i].tipo);
  finDeLinea();
  mandarValores(false);
}

// "!..." (llega sin el "!"): !1..!10 [c], !L v|i|d|c, !A !F !C !I [c].
void pedirMovimiento(char* r) {
  while (*r == ' ' || *r == '\t') r++;
  if (*r == 0) { mandarError("falta el numero del movimiento: !1 a !10, o !L !A !F !C !I", "", ""); return; }
  char* original = r;
  if (isdigit((unsigned char)*r)) {
    char* resto;
    long k = strtol(r, &resto, 10);
    while (*resto == ' ' || *resto == '\t') resto++;
    bool sc = false;
    if (*resto == 'c' || *resto == 'C') {
      sc = true; resto++;
      while (*resto == ' ' || *resto == '\t') resto++;
    }
    if (*resto != 0 || k < 1 || k > 10) {
      mandarError("no hay movimiento ", original, ": son del !1 al !10, y los de cancha !L !A !F !C !I");
      return;
    }
    empezarCuenta((int)k, 0, sc);
    return;
  }
  char letra = (char)toupper((unsigned char)*r);
  char* resto = r + 1;
  while (*resto == ' ' || *resto == '\t') resto++;
  bool hayArg = (*resto != 0);
  char arg = (char)tolower((unsigned char)*resto);
  if (hayArg) { resto++; while (*resto == ' ' || *resto == '\t') resto++; }
  bool sobra = (*resto != 0);
  switch (letra) {
    case 'L':
      if (!hayArg || sobra || (arg != 'v' && arg != 'i' && arg != 'd' && arg != 'c')) {
        mandarError("!L necesita la fase: !L v (verde), !L i (blanco izq), !L d (blanco der) o !L c (sin cable)", "", "");
        return;
      }
      empezarCuenta('L', arg, arg == 'c');
      return;
    case 'A': case 'F': case 'C': case 'I':
      if (sobra || (hayArg && arg != 'c')) break;
      empezarCuenta(letra, hayArg ? 'c' : 0, hayArg);
      return;
    default:
      break;
  }
  mandarError("no hay movimiento ", original, ": son del !1 al !10, y los de cancha !L !A !F !C !I");
}

void procesarLinea(char* s) {
  // sacar espacios de los costados
  while (*s == ' ' || *s == '\t') s++;
  int n = strlen(s);
  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = 0;
  if (n == 0) return;

  if (s[0] == '=') { cambiarNumero(s + 1); return; }
  if (s[0] == '!') { pedirMovimiento(s + 1); return; }
  if (n == 2 && (s[0] == 'r' || s[0] == 'R') && (s[1] == 'c' || s[1] == 'C')) { repetirCancha(); return; }

  if (n == 1) {
    switch (s[0]) {
      case '?':
        mandarValores(false);
        mandarValores(true);
        ayuda();
        return;
      case 'h': case 'H':
        saludar();
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
        Serial.print("Volvieron los numeros del PANEL DE CONTROL del partido (");
        Serial.print(PARTIDO_COPIADO); Serial.print(")."); finDeLinea();
        mandarValores(false);
        return;
      case 'g': case 'G':
        buscarGiroscopio(1);
        return;
      case 'r': case 'R':
        repetirMov();
        return;
      default:
        break;
    }
  }
  mandarError("no entiendo '", s, "'. ? = ayuda");
}

// Junta lo que llega hasta el fin de linea, y recien ahi lo cumple. Solo
// en ESPERA (en la cuenta o moviendose, cualquier tecla FRENA).
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
  // 1. Los valores de arriba son "los del partido": la orden 0 vuelve a estos.
  for (int i = 0; i < N_AJUSTES; i++) ajustes[i].defecto = valorDe(i);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);
  parar();

  // 2. El USB (para la ventana) y la camara.
  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS);
  while (!Serial && millis() < 3000) { }

  // 3. ¿Soy el arquero? Y lo guardado, para que el #HOLA lleve sus dos
  // #GUARDADO enseguida (la EEPROM se lee en un instante).
  serieMia = serieDeEsteTeensy();
  soyElArquero = (serieMia == SERIE_ARQUERO);
  leerGuardados();
  if (Serial) {
    saludar();
    Serial.print("=== TABLERO DEL ARQUERO "); Serial.print(FECHA_CONTRATO);
    Serial.print(" (copias del partido "); Serial.print(PARTIDO_COPIADO);
    Serial.print(") - NO ES EL DE PARTIDO ==="); finDeLinea();
    decir("Se mueve SOLO al pedirlo, despues de la cuenta. Cualquier tecla lo frena.");
    if (!soyElArquero) {
      Serial.print("ESTE TEENSY NO ES EL ARQUERO (serie "); Serial.print(serieMia);
      Serial.print("): no muevo nada."); finDeLinea();
    }
  }

  // 4. El giroscopio, como el partido, con 3 intentos.
  buscarGiroscopio(3);

  // 5. Los numeros y la ayuda.
  t_contadores = millis();
  if (Serial) {
    mandarValores(false);
    mandarValores(true);
    ayuda();
  }

  // 6. Asi el loop() no manda un segundo #HOLA enseguida.
  estabaConectado = (bool)Serial;
}

void loop() {
  unsigned long ahora = millis();

  // ---------- MOVIENDOSE: lo minimo, y nada por el cable ----------
  if (modo == MOVIENDO) {
    // Cuanto tardo la vuelta anterior: si el programa se clavo, aca se ve.
    unsigned long vuelta = ahora - msLoopPrevio;
    if (saltearHueco) saltearHueco = false;
    else if (enMovAnterior && vuelta > huecoMax) { huecoMax = vuelta; huecoMs = msLoopPrevio - t_inicio; }
    msLoopPrevio = ahora;

    int k = leerTeclaMoviendo();
    if (k == 1) { frenar(); terminarPrueba(COMO_TECLA); return; }   // cualquier tecla = FRENO
    if (k == 2) fondoParo(FO_TOCO, ahora - t_fase);                 // la t del fondo = TOCO
    leerCamara();
    ahora = millis();
    if (ahora - t_inicio > topePrueba) { frenar(); terminarPrueba(COMO_TOPE); return; }
    pasoDeLaPrueba(ahora);
    return;
  }

  // ---------- QUIETO (espera o cuenta) ----------
  leerCamara();
  // Al terminar frena de verdad un momento y despues suelta las ruedas, para
  // poder acomodarlo a mano.
  if (t_freno != 0 && ahora - t_freno < MS_FRENO_AL_TERMINAR) frenar();
  else                                                        parar();

  if (modo == CUENTA) { pasoDeLaCuenta(ahora); return; }

  // ---------- ESPERA ----------
  ledEnEspera(ahora);
  // Se abrio el puerto (enchufaron, o abrieron la ventana): saludar.
  bool conectado = (bool)Serial;
  if (conectado && !estabaConectado) saludar();
  estabaConectado = conectado;

  leerOrdenes();
  if (modo != ESPERA) return;

  if (ahora - t_contadores >= 1000) {
    paquetesPorSegundo = nPaquetes * 1000UL / (ahora - t_contadores);
    nPaquetes = 0;
    t_contadores = ahora;
  }
  if (ahora - t_estado >= MS_ESTADO) {
    t_estado = ahora;
    cadaCienMs();
  }
}
