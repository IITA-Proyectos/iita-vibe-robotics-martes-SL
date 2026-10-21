/* =====================================================================
   AJUSTAR-MOVIMIENTOS — ¿se mueve prolijo? Medirlo y corregirlo solos
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-10-20
   =====================================================================

   PARA QUE SIRVE
   Para dejar prolijos los movimientos del arquero sin tener que cambiar
   el programa de partido y volver a cargarlo en cada prueba.

   El robot hace UN movimiento (de costado, el despeje, el retroceso, o
   seguir la pelota) con el MISMO codigo del programa de partido. Mientras
   se mueve, el giroscopio anota hacia donde mira. Al terminar dice:

        - cuanto se torcio como maximo, y cuando
        - cuantas veces zigzagueo (cruzo de un lado al otro)
        - como termino
        - cuanto tuvo que corregir, en promedio
        - siguiendo la pelota: cuantas veces fue y vino

   y una nota: PROLIJO, CASI, ZIGZAGUEA o SE TUERCE, con que numero probar
   tocar. Los numeros se cambian con teclas, SIN COMPILAR. Cuando queda
   bien, la tecla v imprime las lineas para copiar al programa de partido.

   🎯 EL PRIMER USO: los numeros de la correccion de rumbo (KP 5, KI 1,5)
   se ajustaron el 04/08 en el cuadrado, cuando la correccion iba SOLO a la
   rueda de atras. En el partido va a las tres ruedas y nunca se volvieron
   a ajustar. Y el 12/10 se movieron los motores. Lo primero es correr cada
   movimiento 3 veces SIN TOCAR NADA, para saber como esta hoy.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Robot quieto en el piso. BATERIA PRENDIDA (el giroscopio y los
      motores comen de ahi) y DESPUES el USB.
   2. Doble clic en AJUSTAR.bat: carga este programa y abre la consola.
   3. El robot en el PISO DE LA CANCHA, mirando al frente, con lugar:
      de costado se mueve hasta ~50 cm para cada lado, y el despeje sale
      ~35 cm para adelante y vuelve hasta la linea.
   4. Apretar el numero del movimiento. Cuenta 3 segundos y arranca.
   5. Leer el informe, cambiar UN numero, repetir. Anotar en la bitacora.
   6. Al terminar: CERRAR esta consola (Ctrl+C) y despues
      CARGAR-PARTIDO.bat (en funciona/seguir-y-despejar), porque con este
      programa cargado el robot NO juega. Con la consola abierta, el
      cargador no puede reiniciar el Teensy.

   Despues del 6 o el 7 el robot queda SOBRE la linea del area: para los
   de costado hay que correrlo ~10 cm para adelante (si no, no arranca).

   SIN CABLE: la tecla c prende el modo "sin cable". El proximo movimiento
   cuenta 10 segundos para que desenchufen el USB. Al terminar, el LED dice
   la nota (destellos: 1 = PROLIJO, 2 = CASI, 3 = ZIGZAGUEA, 4 = SE TUERCE,
   temblando = no vale). Al volver a enchufar, la consola se reconecta sola
   y la tecla r muestra el informe. Queda guardado aunque se apague.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        1 .. 8     los movimientos (la lista sale con ?)
        0 / espacio / cualquier tecla mientras se mueve:   PARAR
                   (Enter NO lo para. Ctrl+C cierra la consola y antes lo para.)
        n / b      elegir el numero siguiente / anterior
        + / -      subir / bajar el numero elegido
        l          ver todos los numeros
        v          imprimir lo que hay que copiar al programa de partido
        r          repetir el ultimo informe
        c          prender / apagar el modo sin cable
        x          volver a los numeros del programa de partido
        ?          ayuda

   ---------------------------------------------------------------------
   🚨 ESTE PROGRAMA MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Solo cuando se aprieta un numero, despues de la cuenta, y con tope de
   tiempo. Cualquier tecla lo para. Sin giroscopio no arranca: lo que mide
   es el giroscopio, sin el no hay nada que medir.

   ⚠️ LO QUE NO ES IGUAL AL PARTIDO
   - El rumbo que sostiene es el que tenia el robot AL APRETAR LA TECLA, no
     el de cuando se armo. Asi se mide solo el movimiento.
   - De costado (1..5) va a fuerza FIJA, no proporcional a la pelota.
   - Siguiendo la pelota (8) NO DESPEJA aunque la tenga encima, y si pisa
     el lateral FRENA y termina la prueba, en vez de volver al medio.

   ⚠️ Los criterios de la nota son los que el equipo dejo escritos, mas
   algunos a ojo. Estan todos juntos en "CRITERIOS DE LA NOTA".
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

const char* VERSION = "AJUSTAR-MOVIMIENTOS 2026-10-20";

// ⚠️ NO MOVER. PlatformIO escribe sus declaraciones automaticas antes de la
// primera funcion que encuentra. Con la tabla de numeros larga de abajo se
// confundia y las metia ADENTRO de la tabla (no compilaba). Esta funcion va
// aca arriba solo para que las ponga aca.
void paraQueCompile() {}

// ---- ARQUERO (ROBOT1): COPIA EXACTA DEL PROGRAMA DE PARTIDO ----
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


// =====================================================================
// LOS NUMEROS QUE SE AJUSTAN
// =====================================================================
//
// 🚨 LOS VALORES DE ARRANQUE SON COPIA EXACTA DEL PROGRAMA DE PARTIDO
// (funciona/seguir-y-despejar, version 0.1013). Si alla cambia alguno, hay
// que cambiarlo aca tambien: si no, esta herramienta mide otro robot.
//
// Los que se cambian con teclas quedan guardados en la EEPROM: se apaga el
// robot y siguen. La tecla x vuelve a estos.

// Correccion de rumbo (correccionDeRumbo del partido)
const float DEF_KP_RUMBO             = 5.0;
const float DEF_KI_RUMBO             = 1.5;
const float DEF_LIMITE_INTEGRAL      = 30.0;
const int   DEF_MAX_CORRECCION_RUMBO = 100;
const float DEF_ZONA_MUERTA_RUMBO    = 1.0;

// Arranque suave (rampa del partido)
const int   DEF_RAMPA_MOV_PASO = 10;
const unsigned long RAMPA_MOV_MS = 10;        // fijo: no se ajusta aca

// De costado (moverDeCostado y SIGUIENDO del partido)
const int   LADO_FRENTE          = 50;        // fijo: las dos de adelante
const int   DEF_LADO_TRASERA     = 89;        // la de atras
const int   DEF_PWM_MAX_LATERAL  = 150;       // el tope del seguimiento
const int   DEF_PWM_MIN_LATERAL  = 45;        // el piso del seguimiento
const float DEF_KP_LATERAL       = 11.5;      // PWM por cm de desvio de la pelota
const float DEF_ZONA_MUERTA_PELOTA = 1.4;     // cm: no perseguir migajas
const float DEF_MS_ANTICIPACION_SEGUIR = 500; // ms de adelanto de la prediccion
const int   DEF_PREDECIR         = 0;         // predecirTrayectoria = false (20/10)
const bool  lateralInvertido     = false;

// Despeje y retroceso
const int   DEF_POTENCIA_DESPEJE   = 200;
const int   DEF_MS_ADELANTE        = 383;     // ~35 cm
const int   DEF_POTENCIA_RETROCESO = 110;
const unsigned long MS_PAUSA_MEDIO   = 150;     // si NO frena antes de volver
const int   DEF_FRENAR_ANTES_DE_VOLVER = 1;      // 2026-10-20: frenarAntesDeVolver = true
const int   DEF_MS_ESPERA_ANTES_DE_VOLVER = 400; // MS_ESPERA_ANTES_DE_VOLVER (era 100)
const unsigned long MS_MAX_RETROCESO = 1200;
const unsigned long msFreno          = 200;
const int   umbralBlanco             = 425;   // medido en cancha el 2026-09-15

// Camara (leerCamara del partido)
const float CAMARA_POR_CM       = 2.87;
const bool  camaraYInvertida    = true;
const float SUAVIZADO_VELOCIDAD = 0.3;
const float PISO_VELOCIDAD      = 4.0;

// Solo de esta prueba (no existen en el partido)
const int   DEF_VEL_LENTA  = 70;     // de costado, despacio
const int   DEF_MS_COSTADO = 1000;   // cuanto dura cada tramo de costado
const int   DEF_MS_SEGUIR  = 10000;  // cuanto dura la prueba de seguir la pelota

// Los numeros de verdad, los que se tocan con teclas.
float kpRumbo, kiRumbo, limiteIntegral, zonaMuertaRumbo;
int   maxCorreccion, rampaPaso, ladoTrasera, pwmMaxLateral, pwmMinLateral;
float kpLateral, zonaMuertaPelota, msAnticipacionSeguir;
int   predecirTrayectoria;
int   velLenta, msCostado, msSeguir;
int   potenciaDespeje, msAdelante, potenciaRetroceso;

// 🧪 HIPOTESIS PARA PROBAR (2026-10-20). En el partido, el retroceso del
// despeje NO reinicia la rampa: la rampa quedo arriba desde la ida (200),
// y como 110 es menos, rampa() lo deja pasar de golpe. O sea que el robot
// pasa de ir para adelante a ir para atras a 110 sin arranque suave: es el
// pendiente del 21/09, y el tiron que el 18/08 se vio torcer al robot. No
// esta comprobado que haga dano. Con esto en 1 la prueba arranca el
// retroceso con rampa, y se pueden comparar las dos. En 0 es igual al
// partido.
int rampaEnRetroceso = 0;

// Al terminar la ida del despeje: 1 = frena y se queda frenado
// msEsperaAntesDeVolver (el partido desde el 20/10); 0 = suelta los motores
// MS_PAUSA_MEDIO, como el partido hasta el 13/10.
int frenarAntesDeVolver, msEsperaAntesDeVolver;


// =====================================================================
// CRITERIOS DE LA NOTA
// =====================================================================
// De lo que el equipo dejo escrito: el 06/10 de costado "menos de 4
// grados" fue el exito; TOLERANCIA_ACOMODO del partido es 3; y medir-ancho
// dice "menos de 5 fue derecho, de 5 a 10 hay que mirarlo, mas de 10
// oscilo". El numero de cruces es a ojo: no hay ninguno escrito.
const float NOTA_MAX_PROLIJO   = 4.0;   // grados de torcida maxima
const int   NOTA_CRUCES_OK     = 2;     // cruces de un lado al otro
const float NOTA_FINAL_OK      = 3.0;   // grados al terminar
const int   NOTA_CRUCES_ZIGZAG = 4;     // desde aca es zigzag   (a ojo)
const float NOTA_MAX_TUERCE    = 10.0;  // desde aca se tuerce   (medir-ancho)
const float BANDA_CRUCE        = 2.0;   // para contar un cruce hay que pasar
                                        // de +2 a -2 grados (o al reves). Era
                                        // 1: el 20/10, siguiendo la pelota,
                                        // contaba como zigzag bamboleos de
                                        // 1 o 2 grados en cada cambio de lado
const int   CORR_PROMEDIO_ALTA = 15;    // de costado: ver ladoTrasera
const unsigned long HUECO_RETARDO = 40; // ms: una vuelta del loop mas lenta
                                        // que esto es un RETARDO (a ojo; la
                                        // leccion del delay escondido del 06/10)
const int   IDAS_Y_VUELTAS_MUCHAS = 3;  // siguiendo con la pelota quieta (a ojo)


// =====================================================================
// LA TABLA DE NUMEROS (para las teclas n, b, +, -, l, v)
// =====================================================================
// Rangos y pasos: los que propuso la lectura de las bitacoras del 20/10.
// Los nuevos van SIEMPRE AL FINAL de la tabla: la EEPROM los guarda por
// orden, y uno metido en el medio correria a todos los de abajo.
struct Ajuste {
  const char* nombre;     // como se llama aca
  char tipo;              // 'f' float, 'i' int, 'b' si/no
  void* p;
  float paso, minimo, maximo;
  float defecto;
  const char* enElPartido;  // como se escribe alla ("" = no existe)
  int decimales;
  const char* queHace;
};

Ajuste ajustes[] = {
  { "kpRumbo",          'f', &kpRumbo,          0.5,  1,   10,  DEF_KP_RUMBO,
    "const float KP_RUMBO = ",              1, "fuerza de la correccion de rumbo (mas = corrige mas fuerte)" },
  { "kiRumbo",          'f', &kiRumbo,          0.25, 0,   3,   DEF_KI_RUMBO,
    "const float KI_RUMBO = ",              2, "corrige lo acumulado (saca la torcida que queda fija)" },
  { "maxCorreccion",    'i', &maxCorreccion,    10,   40,  120, DEF_MAX_CORRECCION_RUMBO,
    "const int   MAX_CORRECCION_RUMBO = ",  0, "tope de la correccion de rumbo" },
  { "zonaMuertaRumbo",  'f', &zonaMuertaRumbo,  0.25, 0.25, 3,  DEF_ZONA_MUERTA_RUMBO,
    "const float ZONA_MUERTA_RUMBO = ",     2, "grados que se dejan pasar sin corregir" },
  { "limiteIntegral",   'f', &limiteIntegral,   5,    10,  40,  DEF_LIMITE_INTEGRAL,
    "const float LIMITE_INTEGRAL = ",       1, "tope de lo acumulado" },
  { "rampaPaso",        'i', &rampaPaso,        1,    2,   20,  DEF_RAMPA_MOV_PASO,
    "const int RAMPA_MOV_PASO = ",          0, "cuanto sube la potencia cada 10 ms al arrancar (menos = mas suave; cambia las distancias por tiempo)" },
  { "ladoTrasera",      'i', &ladoTrasera,      1,    80,  100, DEF_LADO_TRASERA,
    "const int LADO_TRASERA = ",            0, "fuerza de la rueda de atras de costado, en % (si gira solo de costado)" },
  { "pwmMaxLateral",    'i', &pwmMaxLateral,    10,   80,  170, DEF_PWM_MAX_LATERAL,
    "int   pwmMaxLateral = ",               0, "velocidad de costado RAPIDA = tope del seguimiento (mas de 150 nunca se probo)" },
  { "pwmMinLateral",    'i', &pwmMinLateral,    5,    30,  80,  DEF_PWM_MIN_LATERAL,
    "int   pwmMinLateral = ",               0, "piso del seguimiento: lo minimo que empuja para seguir la pelota" },
  { "kpLateral",        'f', &kpLateral,        0.5,  5,   20,  DEF_KP_LATERAL,
    "float kpLateral   = ",                 1, "siguiendo: fuerza por cada cm que la pelota esta al costado" },
  { "zonaMuertaPelota", 'f', &zonaMuertaPelota, 0.2,  1,   4,   DEF_ZONA_MUERTA_PELOTA,
    "const float ZONA_MUERTA_PELOTA = ",    1, "siguiendo: cm al costado que deja pasar sin moverse" },
  { "msAnticipacion",   'f', &msAnticipacionSeguir, 50, 0, 800, DEF_MS_ANTICIPACION_SEGUIR,
    "float msAnticipacionSeguir = ",        0, "siguiendo: cuanto adelanta la prediccion, en ms" },
  { "predecir",         'b', &predecirTrayectoria, 1, 0,  1,   DEF_PREDECIR,
    "bool  predecirTrayectoria = ",         0, "siguiendo: 1 = prediccion prendida, 0 = mira donde ESTA la pelota" },
  { "velLenta",         'i', &velLenta,         5,    40,  150, DEF_VEL_LENTA,
    "",                                     0, "velocidad de costado LENTA (solo de esta prueba)" },
  { "msCostado",        'i', &msCostado,        100,  300, 1500, DEF_MS_COSTADO,
    "",                                     0, "cuanto dura cada tramo de costado, en ms (solo de esta prueba)" },
  { "msSeguir",         'i', &msSeguir,         1000, 3000, 20000, DEF_MS_SEGUIR,
    "",                                     0, "cuanto dura la prueba de seguir la pelota, en ms (solo de esta prueba)" },
  { "potenciaDespeje",  'i', &potenciaDespeje,  10,   140, 230, DEF_POTENCIA_DESPEJE,
    "int potenciaDespeje   = ",             0, "potencia de la ida del despeje (cambia cuanto sale: msAdelante se calibro a 200)" },
  { "msAdelante",       'i', &msAdelante,       10,   100, 500, DEF_MS_ADELANTE,
    "int msAdelante        = ",             0, "cuanto dura la ida del despeje, en ms (383 = ~35 cm; tiene que pasar los 30 cm del disparo)" },
  { "potenciaRetroceso",'i', &potenciaRetroceso,10,   80,  140, DEF_POTENCIA_RETROCESO,
    "int potenciaRetroceso = ",             0, "potencia del retroceso hasta la linea" },
  { "rampaEnRetroceso", 'b', &rampaEnRetroceso, 1,    0,   1,   0,
    "",                                     0, "1 = el retroceso del despeje arranca con rampa (hipotesis, no es el partido)" },
  { "frenarAntesVolver",'b', &frenarAntesDeVolver, 1, 0,   1,   DEF_FRENAR_ANTES_DE_VOLVER,
    "bool frenarAntesDeVolver = ",          0, "despeje: 1 = al terminar la ida FRENA y espera; 0 = suelta los motores 150 ms" },
  { "msEsperaVolver",   'i', &msEsperaAntesDeVolver, 25, 0, 800, DEF_MS_ESPERA_ANTES_DE_VOLVER,
    "const unsigned long MS_ESPERA_ANTES_DE_VOLVER = ", 0, "despeje: cuanto se queda frenado antes de volver, en ms" },
};
const int N_AJUSTES = sizeof(ajustes) / sizeof(ajustes[0]);
const int MAX_AJUSTES = 24;          // lugar en la EEPROM
int elegido = 0;

float valorDe(int i) {
  return (ajustes[i].tipo == 'f') ? *(float*)ajustes[i].p : (float)*(int*)ajustes[i].p;
}
void ponerValor(int i, float v) {
  if (isnan(v)) v = ajustes[i].defecto;     // EEPROM con basura
  if (v < ajustes[i].minimo) v = ajustes[i].minimo;
  if (v > ajustes[i].maximo) v = ajustes[i].maximo;
  if (ajustes[i].tipo == 'f') *(float*)ajustes[i].p = v;
  else *(int*)ajustes[i].p = (int)lround(v);
}
// Para comparar con el valor del partido sin que un 0,1 de float moleste.
bool cambiado(int i) {
  return fabs(valorDe(i) - ajustes[i].defecto) > 0.001;
}
bool cambiadoVar(void* p) {
  for (int i = 0; i < N_AJUSTES; i++) if (ajustes[i].p == p) return cambiado(i);
  return false;
}
// Al arrancar: si los numeros guardados no son los del partido, decirlo.
void avisarSiNoSonLosDelPartido() {
  int n = 0;
  for (int i = 0; i < N_AJUSTES; i++) if (cambiado(i)) n++;
  if (n == 0) return;
  Serial.println();
  Serial.print("⚠️ OJO: hay "); Serial.print(n);
  Serial.println(" numeros guardados que NO son los del partido (tecla l para");
  Serial.println("   verlos, x para volver a los del partido).");
}


// =====================================================================
// EEPROM
// =====================================================================
//
// 🚨 LA EEPROM ES UNA SOLA Y LA COMPARTEN TODOS LOS PROGRAMAS. Reparto:
//
//      0  ..  255   medir-ancho / medir-ancho-sin-giro
//    256  ..  767   derecho-y-vuelta
//   1024  .. 1535   ajustar-movimientos (este)
//    resto          libre
//
// Si agregan otro programa que guarde algo, anotenlo aca Y en los otros dos.
const int DIR_VALORES = 1024;
const int DIR_INFORME = 1024 + 128;
const uint32_t MAGIA_VALORES = 0x414A5633;   // "AJV3" (20/10: + freno antes de volver)
const uint32_t MAGIA_INFORME = 0x414A4933;   // "AJI3"

struct Guardado {
  uint32_t magia;
  float v[MAX_AJUSTES];
};

void guardarValores() {
  Guardado g;
  g.magia = MAGIA_VALORES;
  for (int i = 0; i < MAX_AJUSTES; i++) g.v[i] = (i < N_AJUSTES) ? valorDe(i) : 0;
  EEPROM.put(DIR_VALORES, g);
}

void valoresDelPartido() {
  for (int i = 0; i < N_AJUSTES; i++) ponerValor(i, ajustes[i].defecto);
}

void leerValores() {
  valoresDelPartido();
  Guardado g;
  EEPROM.get(DIR_VALORES, g);
  if (g.magia != MAGIA_VALORES) return;          // virgen o de otro programa
  for (int i = 0; i < N_AJUSTES && i < MAX_AJUSTES; i++) ponerValor(i, g.v[i]);
}


// =====================================================================
// MOTORES — COPIA EXACTA DEL PROGRAMA DE PARTIDO
// =====================================================================
// Lo unico distinto: aplicar() no lleva el reloj de la patrulla (aca no
// hay patrulla) y los numeros son variables en vez de constantes.

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

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

int potenciaRampa = 0;
unsigned long t_rampaMov = 0;

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
    potenciaRampa += rampaPaso;
    if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  }
  return potenciaRampa;
}


// =====================================================================
// RUMBO — COPIA EXACTA DEL PROGRAMA DE PARTIDO, mas lo que anota
// =====================================================================
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool giroscopoRespondiendo = false;
const bool enderezarActivado = true;
float rumboBase = -1;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Lo que anota la ultima lectura del giroscopio, para medir. Se lee UNA
// sola vez por vuelta y se mide con ese mismo valor: medir no puede cambiar
// lo que se mide (la leccion de medir-ancho).
float ultimoError = 0;
int   ultimaCorreccion = 0;
bool  ultimaMuda = false;

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

int correccionDeRumbo() {
  ultimaCorreccion = 0;
  if (!enderezarActivado || rumboBase < 0) return 0;
  float r = rumboActual();
  if (r < 0) { ultimaMuda = true; return 0; }
  ultimaMuda = false;

  float error = diferencia(rumboBase, r);
  ultimoError = error;

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < zonaMuertaRumbo) return 0;

  integralRumbo += error * dt;
  if (integralRumbo >  limiteIntegral) integralRumbo =  limiteIntegral;
  if (integralRumbo < -limiteIntegral) integralRumbo = -limiteIntegral;

  float c = error * kpRumbo + integralRumbo * kiRumbo;
  if (c >  maxCorreccion) c =  maxCorreccion;
  if (c < -maxCorreccion) c = -maxCorreccion;
  ultimaCorreccion = (int)c;
  return (int)c;
}

// Para cuando el robot esta quieto y no se corrige nada: solo mirar.
void medirSinCorregir() {
  ultimaCorreccion = 0;
  float r = rumboActual();
  if (r < 0) { ultimaMuda = true; return; }
  ultimaMuda = false;
  ultimoError = diferencia(rumboBase, r);
}


// =====================================================================
// LINEA Y MOVIMIENTOS — COPIA EXACTA DEL PROGRAMA DE PARTIDO
// =====================================================================
bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= umbralBlanco; }
bool algunoDeAtrasVeBlanco() { return veBlancoIzq() || veBlancoDer(); }

bool pisaElLateral(int lado) {
  bool izq = veBlancoIzq();
  bool der = veBlancoDer();
  return (lado < 0) ? (izq && !der) : (der && !izq);
}
const int LECTURAS_PARA_CREER_LATERAL = 3;

// s > 0 = a la DERECHA, s < 0 = a la izquierda.
void moverDeCostado(int s) {
  if (lateralInvertido) s = -s;

  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;

  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * ladoTrasera) / 100;

  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}

void adelanteControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(+p + c, -p + c, 0 + c);
}

void atrasControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(-p + c, +p + c, 0 + c);
}


// =====================================================================
// CAMARA — COPIA DE leerCamara() DEL PARTIDO (sin lo del despeje)
// =====================================================================
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;
float velocidadLateral = 0;      // cm/s, + = a la derecha
float desvioAnterior = 0;
unsigned long t_desvioAnterior = 0;
bool  hayDesvioAnterior = false;

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

      float dLat = (camaraYInvertida ? -(float)Yp : (float)Yp) / CAMARA_POR_CM;
      if (Xp > 0) {
        unsigned long t = millis();
        if (hayDesvioAnterior) {
          float dt = (t - t_desvioAnterior) / 1000.0;
          if (dt > 0.005 && dt < 0.5) {
            float v = (dLat - desvioAnterior) / dt;
            velocidadLateral = velocidadLateral * (1.0 - SUAVIZADO_VELOCIDAD)
                             + v * SUAVIZADO_VELOCIDAD;
          }
        }
        desvioAnterior    = dLat;
        t_desvioAnterior  = t;
        hayDesvioAnterior = true;
      } else {
        hayDesvioAnterior = false;
        velocidadLateral  = 0;
      }
    }
  }
}

bool veLaPelota() { return Xp > 0 && (millis() - t_ultimoPaquete < 500); }

float desvioPelota() {
  float y = camaraYInvertida ? -(float)Yp : (float)Yp;
  return y / CAMARA_POR_CM;
}

float desvioPredicho() {
  if (!predecirTrayectoria) return desvioPelota();
  float v = velocidadLateral;
  if (fabs(v) < PISO_VELOCIDAD) v = 0;
  return desvioPelota() + v * (msAnticipacionSeguir / 1000.0);
}


// =====================================================================
// LOS MOVIMIENTOS DE LA PRUEBA
// =====================================================================
//
//   1  de costado a la IZQUIERDA, lento      (velLenta, msCostado)
//   2  de costado a la DERECHA, lento
//   3  de costado a la IZQUIERDA, rapido     (pwmMaxLateral, msCostado)
//   4  de costado a la DERECHA, rapido
//   5  ida y vuelta rapido: izquierda y sin frenar derecha, como cuando la
//      pelota cruza al otro lado en el seguimiento
//   6  DESPEJE: adelante msAdelante, pausa, atras hasta la linea
//   7  RETROCESO: atras hasta la linea, como al arrancar el partido
//   8  SEGUIR LA PELOTA durante msSeguir, como SIGUIENDO del partido
//
// Los de costado frenan solos si pisan el lateral (la misma regla del
// partido). Todos terminan frenando y quedandose quietos medio segundo,
// para medir como quedo despues de la inercia.

const int N_MOVIMIENTOS = 8;
const char* NOMBRES[] = { "",
  "costado IZQUIERDA lento", "costado DERECHA lento",
  "costado IZQUIERDA rapido", "costado DERECHA rapido",
  "ida y vuelta rapido", "DESPEJE", "RETROCESO hasta la linea",
  "SEGUIR LA PELOTA" };

enum Paso { NADA, CUENTA, COSTADO, COSTADO_VUELTA, IDA, PAUSA, ATRAS,
            SEGUIR, FRENO, ASENTAR };
Paso paso = NADA;
unsigned long t_paso = 0, t_inicio = 0;
int movimiento = 0;
int ladoCostado = 0;          // -1 izquierda, +1 derecha
int velCostado = 0;
int lecturasDeLateral = 0;
bool sinCable = false;

// Seguimiento
bool ibaDerecha = true;
bool seMovia = false;         // para contar arranques
bool estabaSinPelota = true;

const unsigned long MS_CUENTA           = 3000;
const unsigned long MS_CUENTA_SIN_CABLE = 10000;
const unsigned long MS_ASENTAR          = 500;
const unsigned long MS_TOPE_TOTAL       = 8000;   // salvo seguir la pelota
const int MAX_TRAZA = 80;


// =====================================================================
// LO QUE SE MIDE
// =====================================================================
struct Informe {
  uint32_t magia;
  uint8_t  movimiento;
  uint8_t  nota;            // 1 prolijo, 2 casi, 3 zigzaguea, 4 se tuerce, 5 no vale
  uint8_t  termino;         // 0 normal, 1 paro en el lateral, 2 vio la linea,
                            // 3 tope sin linea, 4 lo pararon (tecla o tope)
  uint8_t  nTraza;
  uint16_t msTraza;         // cada cuanto se anoto una marca
  uint16_t msTotal;
  uint16_t msHastaLinea;    // retroceso: cuanto tardo en ver la linea
  uint16_t msFinMov;        // cuando dejo de moverse (empezo a frenar)
  int16_t  maxDecimas;      // torcida maxima (con signo), decimas de grado
  uint16_t msDelMax;
  int16_t  finalDecimas;
  int16_t  medioDecimas;    // error promedio CON signo, en movimiento
  uint16_t cruces;
  uint16_t muestras;        // en movimiento
  uint16_t fuera3;          // muestras en movimiento con mas de 3 grados
  int16_t  corrPromedio;
  uint16_t alTope;          // muestras con la correccion al tope
  uint16_t mudas;           // lecturas del giroscopio que no contestaron
  uint16_t peorRachaMuda;   // la mayor cantidad de mudas SEGUIDAS
  uint16_t msPrimeraMuda;   // cuando fue la primera
  uint16_t msInicioAtras;   // despeje: cuando arranco el retroceso
  uint16_t peorHueco;       // la vuelta del loop mas lenta, en ms
  // Siguiendo la pelota
  uint16_t idasYVueltas;    // veces que cambio de lado
  uint16_t arranques;       // veces que salio de quieto
  uint16_t muestrasSeguir;  // todas las vueltas de la prueba
  uint16_t conPelota;       // vueltas viendo la pelota
  uint16_t enZonaMuerta;    // vueltas quieto porque la tenia enfrente
  int16_t  velMediaDecimas; // |velocidad de la pelota| promedio, cm/s x10
  int16_t  traza[MAX_TRAZA];// decimas de grado, una cada msTraza
  float    usados[MAX_AJUSTES]; // los numeros CON LOS QUE se corrio
};
Informe inf;
bool hayInforme = false;

int  ladoDelError = 0;
int  rachaMuda = 0;
long sumaCorr = 0;
float sumaErr = 0;
float sumaFinal = 0;
int   nFinal = 0;
float sumaVel = 0;
unsigned long t_traza = 0, t_vueltaAnterior = 0;
bool enMovimiento = false;

void empezarMedicion() {
  memset(&inf, 0, sizeof(inf));
  inf.magia = MAGIA_INFORME;
  inf.movimiento = movimiento;
  // Seguir la pelota dura mas: una marca cada 250 ms para que entre.
  inf.msTraza = (movimiento == 8) ? 250 : 100;
  for (int i = 0; i < MAX_AJUSTES; i++) inf.usados[i] = (i < N_AJUSTES) ? valorDe(i) : 0;
  ladoDelError = 0;
  rachaMuda = 0;
  sumaCorr = 0; sumaErr = 0;
  sumaFinal = 0; nFinal = 0;
  sumaVel = 0;
  t_traza = millis();
  t_inicio = millis();
  t_vueltaAnterior = millis();
}

// Se llama en cada vuelta mientras dura la prueba, DESPUES de mover (o de
// medirSinCorregir si esta quieto). Usa la lectura que ya se hizo.
void anotar(unsigned long ahora) {
  unsigned long hueco = ahora - t_vueltaAnterior;
  t_vueltaAnterior = ahora;
  if (enMovimiento && hueco > inf.peorHueco) inf.peorHueco = (uint16_t)hueco;

  if (ultimaMuda) {
    if (inf.mudas == 0) inf.msPrimeraMuda = (uint16_t)(ahora - t_inicio);
    if (inf.mudas < 65535) inf.mudas++;
    rachaMuda++;
    if (rachaMuda > inf.peorRachaMuda) inf.peorRachaMuda = rachaMuda;
    return;
  }
  rachaMuda = 0;
  float e = ultimoError;

  if (enMovimiento) {
    if (inf.muestras < 65535) inf.muestras++;
    if (fabs(e) > 3.0 && inf.fuera3 < 65535) inf.fuera3++;
    sumaCorr += ultimaCorreccion;
    sumaErr += e;
    if (abs(ultimaCorreccion) >= maxCorreccion && inf.alTope < 65535) inf.alTope++;
  }

  if (fabs(e) * 10 > abs(inf.maxDecimas)) {
    inf.maxDecimas = (int16_t)lround(e * 10);
    inf.msDelMax = (uint16_t)(ahora - t_inicio);
  }

  // Un cruce: pasar de mas de +BANDA a menos de -BANDA, o al reves.
  int lado = (e > BANDA_CRUCE) ? 1 : (e < -BANDA_CRUCE) ? -1 : 0;
  if (lado != 0) {
    if (ladoDelError != 0 && lado != ladoDelError && inf.cruces < 65535) inf.cruces++;
    ladoDelError = lado;
  }

  if (paso == ASENTAR && ahora - t_paso >= MS_ASENTAR - 150) {
    sumaFinal += e; nFinal++;               // el promedio de los ultimos 150 ms
  }

  if (ahora - t_traza >= inf.msTraza && inf.nTraza < MAX_TRAZA) {
    t_traza = ahora;
    inf.traza[inf.nTraza++] = (int16_t)lround(e * 10);
  }
}

uint8_t calcularNota() {
  float maxAbs = abs(inf.maxDecimas) / 10.0;
  float fin = fabs(inf.finalDecimas / 10.0);
  // Una lectura muda suelta no tira la prueba: NO VALE si se callo varias
  // veces SEGUIDAS, o mas del 2 % del total. Si no, nota normal y se avisa.
  if (inf.peorRachaMuda >= 3 || inf.mudas * 50 > inf.muestras + inf.mudas
      || inf.muestras < 5) return 5;
  if (inf.cruces >= NOTA_CRUCES_ZIGZAG) return 3;
  if (maxAbs > NOTA_MAX_TUERCE || fin > NOTA_MAX_TUERCE) return 4;
  if (maxAbs <= NOTA_MAX_PROLIJO && inf.cruces <= NOTA_CRUCES_OK && fin <= NOTA_FINAL_OK) return 1;
  return 2;
}

void terminarMedicion(unsigned long ahora, uint8_t comoTermino) {
  parar();
  inf.termino = comoTermino;
  inf.msTotal = (uint16_t)(ahora - t_inicio);
  if (inf.msFinMov == 0) inf.msFinMov = inf.msTotal;
  inf.finalDecimas = (nFinal > 0) ? (int16_t)lround(sumaFinal / nFinal * 10)
                                  : (int16_t)lround(ultimoError * 10);
  inf.corrPromedio = (inf.muestras > 0) ? (int16_t)(sumaCorr / (long)inf.muestras) : 0;
  inf.medioDecimas = (inf.muestras > 0) ? (int16_t)lround(sumaErr / inf.muestras * 10) : 0;
  inf.velMediaDecimas = (inf.conPelota > 0) ? (int16_t)lround(sumaVel / inf.conPelota * 10) : 0;
  inf.nota = calcularNota();
  EEPROM.put(DIR_INFORME, inf);
  hayInforme = true;
  paso = NADA;
  enMovimiento = false;
  imprimirInforme();
}


// =====================================================================
// EL INFORME
// =====================================================================
const char* NOTAS[] = { "", "PROLIJO", "CASI", "ZIGZAGUEA", "SE TUERCE",
                        "NO VALE (el giroscopio no contesto, o casi no se movio)" };

void imprimirDecimas(int16_t d) {
  if (d >= 0) Serial.print('+');
  Serial.print(d / 10.0, 1);
}

bool vaYViene() {
  // Muchas idas y vueltas con la pelota casi quieta: el problema del 12/10.
  return inf.movimiento == 8 && inf.idasYVueltas >= IDAS_Y_VUELTAS_MUCHAS
         && inf.velMediaDecimas < PISO_VELOCIDAD * 10;
}

void imprimirSugerencia() {
  bool costado = (inf.movimiento >= 1 && inf.movimiento <= 5);
  Serial.println("QUE PROBAR (de a UN numero por vez, y repetir 3 veces):");
  if (inf.nota != 5 && inf.peorHueco > HUECO_RETARDO) {
    Serial.print("  ⚠️ RETARDO: una vuelta del programa tardo ");
    Serial.print(inf.peorHueco);
    Serial.println(" ms. Con eso zigzaguea igual,");
    Serial.println("  toquen lo que toquen (el delay escondido del 06/10). Avisen antes de");
    Serial.println("  cambiar numeros.");
  }
  switch (inf.nota) {
    case 1:
      Serial.println("  Nada: anoten los numeros en la bitacora. Prueben tambien el mismo");
      Serial.println("  movimiento para el otro lado.");
      break;
    case 2:
      if (inf.msDelMax < 300) {
        Serial.println("  La peor torcida fue al ARRANCAR (primeros 0,3 s):");
        Serial.print  ("  -> bajar rampaPaso de a 1 (ahora "); Serial.print(rampaPaso); Serial.println(") para arrancar mas suave");
      } else if (fabs(inf.medioDecimas / 10.0) > 2.0) {
        Serial.println("  Se queda torcido siempre para el MISMO lado:");
        Serial.print  ("  -> subir kiRumbo de a 0.25 (ahora "); Serial.print(kiRumbo, 2); Serial.println(")");
      } else {
        Serial.println("  Esta cerca. Repitan 2 o 3 veces: si da parecido, es asi.");
      }
      break;
    case 3:
      Serial.println("  Corrige de mas y se pasa para el otro lado:");
      Serial.print  ("  -> bajar kpRumbo de a 0.5 (ahora "); Serial.print(kpRumbo, 1); Serial.println(")");
      Serial.println("     si con kpRumbo bajo sigue igual, bajar kiRumbo de a 0.25");
      break;
    case 4:
      if (inf.muestras > 0 && inf.alTope * 5 > inf.muestras) {
        Serial.println("  La correccion esta AL TOPE mucho rato y no alcanza:");
        Serial.print  ("  -> subir maxCorreccion de a 10 (ahora "); Serial.print(maxCorreccion); Serial.println(")");
        Serial.println("     o ir mas despacio");
      } else if (inf.movimiento == 6 && inf.msInicioAtras > 0
                 && inf.msDelMax >= inf.msInicioAtras && inf.msDelMax < inf.msInicioAtras + 300) {
        Serial.println("  La peor torcida fue al ARRANCAR EL RETROCESO (el tiron de ir para");
        Serial.println("  adelante a ir para atras de golpe):");
        Serial.println("  -> probar rampaEnRetroceso = 1 y comparar");
      } else if (inf.movimiento == 6 && inf.msInicioAtras > 0 && inf.msDelMax < inf.msInicioAtras
                 && inf.msDelMax >= (uint16_t)msAdelante) {
        Serial.println("  Se torcio en la PAUSA, al terminar la ida. Ahi la correccion no actua:");
        Serial.println("  no es kpRumbo.");
        if (frenarAntesDeVolver) {
          Serial.print  ("  -> probar msEsperaVolver de a 25 (ahora "); Serial.print(msEsperaAntesDeVolver); Serial.println(" ms)");
        } else {
          Serial.println("  -> probar frenarAntesVolver = 1");
        }
      } else if (inf.msDelMax >= inf.msFinMov) {
        Serial.println("  Se tuerce al FRENAR, no mientras va (la peor torcida fue ya frenando).");
        Serial.println("  El freno es igual al del partido: anotenlo, no hay numero para tocar aca.");
      } else {
        Serial.println("  Corrige flojo y se le escapa:");
        Serial.print  ("  -> subir kpRumbo de a 0.5 (ahora "); Serial.print(kpRumbo, 1); Serial.println(")");
      }
      break;
    case 5:
      Serial.println("  El resultado no vale. Revisar: bateria prendida ANTES del USB,");
      Serial.println("  y el cable del giroscopio (el 29/09 estaba roto).");
      break;
  }
  if (costado && inf.nota != 5 && abs(inf.corrPromedio) >= CORR_PROMEDIO_ALTA) {
    Serial.print("  Ademas: tuvo que corregir en promedio ");
    Serial.print(inf.corrPromedio);
    Serial.println(" todo el tiempo: de costado tiende a girar");
    Serial.println("  solo. Corran el MISMO movimiento para el otro lado: si la correccion");
    Serial.println("  promedio cambia de signo, es la mezcla (probar ladoTrasera 2 mas o 2");
    Serial.println("  menos); si tiene el mismo signo, es un motor que tira mas (kiRumbo).");
  }
  if (vaYViene()) {
    Serial.println("  SIGUIENDO: VA Y VIENE con la pelota casi quieta. De a uno:");
    Serial.print  ("  -> subir zonaMuertaPelota de a 0.2 (ahora "); Serial.print(zonaMuertaPelota, 1); Serial.println(" cm)");
    Serial.println("  -> o probar predecir = 0 (si deja de ir y venir, era la prediccion)");
    Serial.print  ("  -> o bajar pwmMinLateral de a 5 (ahora "); Serial.print(pwmMinLateral); Serial.println(")");
  }
}

void imprimirTraza() {
  // Una regla de 10 grados para cada lado, como la de ver-prediccion. El *
  // va para el lado al que apunta la nariz del robot: el giroscopio cuenta
  // positivo en sentido horario (bitacora del 28/07), asi que un error
  // positivo (rumboBase - rumbo) es que giro a la IZQUIERDA.
  Serial.println("  tiempo  grados  izq 10    izq 5     0         der 5     der 10");
  for (int i = 0; i < inf.nTraza; i++) {
    float g = inf.traza[i] / 10.0;
    Serial.print("  ");
    float t = (i + 1) * inf.msTraza / 1000.0;
    if (t < 10) Serial.print(' ');
    Serial.print(t, 1);
    Serial.print(" s  ");
    if (inf.traza[i] >= 0) Serial.print(' ');
    if (abs(inf.traza[i]) < 100) Serial.print(' ');
    Serial.print(g, 1);
    Serial.print("  ");
    int col = 20 - (int)lround(g * 2);     // media columna por grado; + = izquierda
    char fila[42];
    for (int k = 0; k < 41; k++) fila[k] = (k == 20) ? '|' : (k % 10 == 0 ? ':' : '.');
    fila[41] = 0;
    if (col < 0) fila[0] = '<';
    else if (col > 40) fila[40] = '>';
    else fila[col] = '*';
    Serial.println(fila);
  }
}

void imprimirInforme() {
  if (!hayInforme) { Serial.println("todavia no hay ningun informe"); return; }
  Serial.println();
  Serial.print("=============== INFORME: ");
  Serial.print(inf.movimiento); Serial.print(" "); Serial.print(NOMBRES[inf.movimiento]);
  Serial.println(" ===============");
  Serial.print("NOTA: "); Serial.print(NOTAS[inf.nota]);
  if (vaYViene()) Serial.print("   +   SIGUIENDO: VA Y VIENE");
  Serial.println();
  Serial.println();
  Serial.print("torcida maxima       "); imprimirDecimas(inf.maxDecimas);
  Serial.print(" grados  (a los "); Serial.print(inf.msDelMax / 1000.0, 1); Serial.println(" s)");
  Serial.print("zigzag               "); Serial.print(inf.cruces); Serial.println(" cruces de un lado al otro");
  Serial.print("como termino         "); imprimirDecimas(inf.finalDecimas); Serial.println(" grados (quieto, despues de frenar)");
  if (inf.muestras > 0) {
    Serial.print("torcida promedio     "); imprimirDecimas(inf.medioDecimas); Serial.println(" grados mientras se movia (con signo)");
    Serial.print("fuera de +-3 grados  "); Serial.print((100UL * inf.fuera3) / inf.muestras); Serial.println(" % del tiempo en movimiento");
    Serial.print("correccion promedio  "); Serial.print(inf.corrPromedio);
    Serial.print("   (al tope el "); Serial.print((100UL * inf.alTope) / inf.muestras); Serial.println(" % del tiempo)");
  }
  if (inf.movimiento == 8 && inf.muestrasSeguir > 0) {
    Serial.print("vio la pelota        "); Serial.print((100UL * inf.conPelota) / inf.muestrasSeguir); Serial.println(" % del tiempo");
    Serial.print("quieto enfrente      "); Serial.print((100UL * inf.enZonaMuerta) / inf.muestrasSeguir); Serial.println(" % del tiempo (zona muerta)");
    Serial.print("idas y vueltas       "); Serial.print(inf.idasYVueltas); Serial.println(" veces que cambio de lado");
    Serial.print("arranques            "); Serial.print(inf.arranques); Serial.println(" veces que salio de quieto");
    Serial.print("pelota               se movia en promedio a "); Serial.print(inf.velMediaDecimas / 10.0, 1);
    Serial.print(" cm/s");
    if (inf.velMediaDecimas < PISO_VELOCIDAD * 10) Serial.print("  (casi quieta)");
    Serial.println();
  }
  Serial.print("giroscopio           ");
  if (inf.mudas == 0) Serial.println("bien");
  else {
    Serial.print(inf.mudas); Serial.print(" lecturas SIN RESPUESTA (la primera a los ");
    Serial.print(inf.msPrimeraMuda); Serial.print(" ms; hasta "); Serial.print(inf.peorRachaMuda);
    Serial.println(" seguidas)");
  }
  Serial.print("vuelta mas lenta     "); Serial.print(inf.peorHueco); Serial.print(" ms");
  if (inf.peorHueco > HUECO_RETARDO) Serial.print("   <- RETARDO");
  Serial.println();
  Serial.print("duro                 "); Serial.print(inf.msTotal / 1000.0, 2); Serial.print(" s");
  switch (inf.termino) {
    case 1: Serial.println("  -- FRENO EN EL LATERAL (vio la linea)"); break;
    case 2: Serial.print("  -- vio la linea a los "); Serial.print(inf.msHastaLinea); Serial.println(" ms de retroceso"); break;
    case 3: Serial.println("  -- NO VIO LA LINEA: freno por el tope de 1200 ms"); break;
    case 4: Serial.println("  -- LO PARARON (tecla o tope de tiempo)"); break;
    default: Serial.println(); break;
  }
  Serial.println();
  imprimirTraza();
  Serial.println("  (* = hacia donde apuntaba; | = derecho, el rumbo del arranque;");
  Serial.println("   en los numeros, + = se torcio a la izquierda, - = a la derecha)");
  Serial.println();
  imprimirSugerencia();
  Serial.println();
  Serial.println("Numeros CON LOS QUE SE CORRIO (para la bitacora):");
  for (int i = 0; i < N_AJUSTES; i++) {
    float u = inf.usados[i];
    float ahoraV = valorDe(i);
    if (ajustes[i].enElPartido[0] == 0 && fabs(u - ajustes[i].defecto) < 0.001) continue;
    Serial.print("    ");
    Serial.print(ajustes[i].nombre);
    for (int k = strlen(ajustes[i].nombre); k < 18; k++) Serial.print(' ');
    if (ajustes[i].tipo == 'f') Serial.print(u, ajustes[i].decimales); else Serial.print((int)lround(u));
    if (fabs(u - ajustes[i].defecto) > 0.001) Serial.print("   <- distinto del partido");
    if (fabs(u - ahoraV) > 0.001) { Serial.print("   (ahora vale "); imprimirValor(i); Serial.print(")"); }
    Serial.println();
  }
  Serial.print("RESUMEN: "); Serial.print(NOMBRES[inf.movimiento]); Serial.print(" -> ");
  Serial.print(NOTAS[inf.nota]); Serial.print(", torcida maxima "); imprimirDecimas(inf.maxDecimas);
  Serial.print(", "); Serial.print(inf.cruces); Serial.println(" cruces");
}


// =====================================================================
// PANTALLA Y TECLAS
// =====================================================================
void imprimirValor(int i) {
  if (ajustes[i].tipo == 'f') Serial.print(valorDe(i), ajustes[i].decimales);
  else Serial.print((int)valorDe(i));
}

// Como se escribe en el programa de partido: los si/no son true/false.
void imprimirValorParaCopiar(int i) {
  if (ajustes[i].tipo == 'b') Serial.print(valorDe(i) != 0 ? "true" : "false");
  else imprimirValor(i);
}

void imprimirDefecto(int i) {
  if (ajustes[i].tipo == 'b') Serial.print(ajustes[i].defecto != 0 ? "true" : "false");
  else if (ajustes[i].tipo == 'f') Serial.print(ajustes[i].defecto, ajustes[i].decimales);
  else Serial.print((int)ajustes[i].defecto);
}

void imprimirLista(bool conDescripcion) {
  for (int i = 0; i < N_AJUSTES; i++) {
    Serial.print(i == elegido ? "  > " : "    ");
    Serial.print(ajustes[i].nombre);
    for (int k = strlen(ajustes[i].nombre); k < 18; k++) Serial.print(' ');
    imprimirValor(i);
    if (cambiado(i)) {
      Serial.print("   (en el partido: ");
      imprimirDefecto(i);
      Serial.print(")");
    }
    if (conDescripcion) { Serial.print("   "); Serial.print(ajustes[i].queHace); }
    Serial.println();
  }
}

void imprimirElegido() {
  Serial.print("> ");
  Serial.print(ajustes[elegido].nombre);
  Serial.print(" = ");
  imprimirValor(elegido);
  Serial.print("   (");
  Serial.print(ajustes[elegido].queHace);
  Serial.println(")");
}

void imprimirParaCopiar() {
  Serial.println();
  Serial.println("=============== PARA COPIAR AL PROGRAMA DE PARTIDO ===============");
  Serial.println("Archivo: funciona/seguir-y-despejar/seguir-y-despejar.ino");
  Serial.println("Buscar cada nombre (Ctrl+F) y cambiar SOLO el numero:");
  Serial.println();
  int cambiados = 0;
  for (int i = 0; i < N_AJUSTES; i++) {
    if (!cambiado(i)) continue;
    if (ajustes[i].enElPartido[0] == 0) continue;
    Serial.print("    ");
    Serial.print(ajustes[i].enElPartido);
    imprimirValorParaCopiar(i);
    Serial.print(";     // era ");
    imprimirDefecto(i);
    Serial.println();
    cambiados++;
  }
  if (cambiados == 0) Serial.println("    (nada: todo esta igual que en el partido)");
  if (cambiadoVar(&rampaPaso) || cambiadoVar(&potenciaDespeje) || cambiadoVar(&msAdelante)) {
    Serial.println();
    Serial.println("    🚨 rampaPaso, potenciaDespeje y msAdelante cambian CUANTO SALE EL");
    Serial.println("    DESPEJE (y rampaPaso tambien la vuelta al medio por tiempo). Medirlo con");
    Serial.println("    regla antes de jugar: la ida tiene que pasar de los 30 cm a los que");
    Serial.println("    dispara, o no toca la pelota (el 15/09, con 0 cm de sobra, no la tocaba).");
  }
  if (cambiados > 0) {
    Serial.println();
    Serial.println("    Y DESPUES, en ESTA herramienta (pruebas/ajustar-movimientos/");
    Serial.println("    ajustar-movimientos.ino), los valores DEF_ tienen que quedar igual que");
    Serial.println("    el partido, o la tecla x vuelve a numeros viejos:");
    for (int i = 0; i < N_AJUSTES; i++) {
      if (!cambiado(i) || ajustes[i].enElPartido[0] == 0) continue;
      Serial.print("        DEF_ de "); Serial.print(ajustes[i].nombre); Serial.print(" -> ");
      imprimirValor(i); Serial.println();
    }
  }
  if (rampaEnRetroceso) {
    Serial.println();
    Serial.println("    rampaEnRetroceso = 1 NO es un numero del partido: si sirvio,");
    Serial.println("    en el partido hay que agregar reiniciarRampaMovimiento() al");
    Serial.println("    pasar a ATRAS_HASTA_LINEA. Es un cambio de codigo: pidanlo.");
  }
  if (velLenta != DEF_VEL_LENTA || msCostado != DEF_MS_COSTADO || msSeguir != DEF_MS_SEGUIR) {
    Serial.println();
    Serial.println("    (velLenta, msCostado y msSeguir son solo de esta prueba: no se copian)");
  }
  Serial.println();
  Serial.println("Despues de cambiarlos: cargar el partido y ANOTAR EN LA BITACORA");
  Serial.println("los numeros y que nota daba cada movimiento.");
  Serial.println("==================================================================");
}

void imprimirAyuda() {
  Serial.println();
  Serial.print("=============== "); Serial.print(VERSION); Serial.println(" ===============");
  Serial.println("MOVIMIENTOS (el robot SE MUEVE, en el piso y con lugar):");
  for (int m = 1; m <= N_MOVIMIENTOS; m++) {
    Serial.print("  "); Serial.print(m); Serial.print("  "); Serial.print(NOMBRES[m]);
    if (m == 8) { Serial.print(" ("); Serial.print(msSeguir / 1000); Serial.print(" s, muevan la pelota; NO despeja)"); }
    Serial.println();
  }
  Serial.println("  0 o espacio: PARAR. Mientras se mueve, cualquier tecla (menos Enter) lo para.");
  Serial.println();
  Serial.println("NUMEROS:  n / b elegir   + / - cambiar   l lista   v para copiar");
  Serial.println("OTRAS:    r ultimo informe   c sin cable   x volver al partido   ? ayuda");
  Serial.print  ("Modo sin cable: "); Serial.println(sinCable ? "PRENDIDO (10 s para desenchufar)" : "apagado");
  Serial.print  ("Giroscopio: ");
  if (!hayGiroscopo) Serial.println("NO ESTA. Sin giroscopio no se puede medir.");
  else if (rumboActual() < 0) Serial.println("esta, pero NO DA DATOS (bateria prendida?)");
  else Serial.println("bien");
  Serial.print  ("Camara: ");
  Serial.println((millis() - t_ultimoPaquete < 500 && t_ultimoPaquete > 0) ? "manda datos" : "NO MANDA DATOS (solo hace falta para el 8)");
  Serial.println();
  imprimirElegido();
}

unsigned long t_parada = 0;
const unsigned long MS_ESPERA_DESPUES_DE_PARAR = 1500;

void empezarMovimiento(int m) {
  // Recien se lo paro: una segunda tecla apretada de mas NO lo vuelve a
  // arrancar (lo encontro la revision del 20/10).
  if (t_parada > 0 && millis() - t_parada < MS_ESPERA_DESPUES_DE_PARAR) {
    Serial.println("espera: recien se paro. Volve a apretar el numero en un segundo.");
    return;
  }
  if (!hayGiroscopo || rumboActual() < 0) {
    Serial.println("NO ARRANCO: el giroscopio no da datos. Bateria prendida? Cable?");
    return;
  }
  // De costado y siguiendo: si arranca SOBRE una linea, no puede ver el
  // lateral (la regla pide que el otro sensor este en verde) y se iria de
  // largo. Pasa justo despues del 6 o el 7, que terminan sobre la linea.
  bool deCostado = (m >= 1 && m <= 5) || m == 8;
  if (deCostado && algunoDeAtrasVeBlanco()) {
    Serial.println("NO ARRANCO: un sensor de atras esta sobre una linea blanca. Asi no");
    Serial.println("ve el lateral y se iria de largo. Corranlo ~10 cm para adelante.");
    return;
  }
  movimiento = m;
  paso = CUENTA;
  t_paso = millis();
  Serial.println();
  Serial.print(">>> "); Serial.print(m); Serial.print(" "); Serial.print(NOMBRES[m]);
  if (sinCable) Serial.println(": arranca en 10 s. DESENCHUFEN EL USB ahora.");
  else Serial.println(": arranca en 3 s. Cualquier tecla lo para.");
}

void arrancarDeVerdad(unsigned long ahora) {
  rumboBase = rumboActual();
  if (rumboBase < 0) {             // se cayo durante la cuenta
    parar(); paso = NADA;
    Serial.println("NO ARRANCO: el giroscopio dejo de contestar durante la cuenta.");
    return;
  }
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  lecturasDeLateral = 0;
  empezarMedicion();
  enMovimiento = true;
  t_paso = ahora;
  switch (movimiento) {
    case 1: ladoCostado = -1; velCostado = velLenta;      paso = COSTADO; break;
    case 2: ladoCostado = +1; velCostado = velLenta;      paso = COSTADO; break;
    case 3: ladoCostado = -1; velCostado = pwmMaxLateral; paso = COSTADO; break;
    case 4: ladoCostado = +1; velCostado = pwmMaxLateral; paso = COSTADO; break;
    case 5: ladoCostado = -1; velCostado = pwmMaxLateral; paso = COSTADO_VUELTA; break;
    case 6: paso = IDA; break;
    case 7: paso = ATRAS; break;
    case 8:
      ibaDerecha = true; seMovia = false; estabaSinPelota = true;
      enMovimiento = false;          // se prende cuando de verdad se mueve
      paso = SEGUIR; break;
  }
}

void pasarAFrenar(unsigned long ahora) {
  frenar();
  enMovimiento = false;
  inf.msFinMov = (uint16_t)(ahora - t_inicio);
  paso = FRENO; t_paso = ahora;
}

void pararYa(unsigned long ahora) {
  parar();
  t_parada = ahora;
  delay(20);                                   // que llegue el resto de lo tecleado
  while (Serial.available()) Serial.read();    // y tirarlo: no es un pedido nuevo
  if (paso == NADA) return;
  if (paso == CUENTA) {
    paso = NADA;
    Serial.println("cancelado");
    return;
  }
  terminarMedicion(ahora, 4);
}

void tecla(char k) {
  if (k >= '1' && k <= '0' + N_MOVIMIENTOS) { empezarMovimiento(k - '0'); return; }
  switch (k) {
    case '0': case ' ': parar(); Serial.println("PARADO"); break;
    case 'n': elegido = (elegido + 1) % N_AJUSTES; imprimirElegido(); break;
    case 'b': elegido = (elegido + N_AJUSTES - 1) % N_AJUSTES; imprimirElegido(); break;
    case '+': case '=':
      ponerValor(elegido, valorDe(elegido) + ajustes[elegido].paso);
      guardarValores(); imprimirElegido(); break;
    case '-': case '_':
      ponerValor(elegido, valorDe(elegido) - ajustes[elegido].paso);
      guardarValores(); imprimirElegido(); break;
    case 'l': Serial.println(); imprimirLista(true); break;
    case 'v': imprimirParaCopiar(); break;
    case 'r': imprimirInforme(); break;
    case 'c':
      sinCable = !sinCable;
      Serial.println(sinCable ? "modo sin cable PRENDIDO: el proximo movimiento cuenta 10 s"
                              : "modo sin cable apagado");
      break;
    case 'x':
      valoresDelPartido(); guardarValores();
      Serial.println("volvieron los numeros del programa de partido:");
      imprimirLista(false);
      break;
    case '?': case 'h': imprimirAyuda(); break;
    default: break;
  }
}


// =====================================================================
// LED
// =====================================================================
// Despues de una prueba, el LED repite la nota en destellos (sirve sin cable).
void ledNota(unsigned long ahora) {
  if (!hayInforme) { digitalWrite(LED, (ahora / 1000) % 2); return; }
  if (inf.nota == 5) { digitalWrite(LED, (ahora / 50) % 2); return; }
  int veces = inf.nota;
  unsigned long ciclo = ahora % ((unsigned long)veces * 400 + 1200);
  digitalWrite(LED, (ciclo < (unsigned long)veces * 400 && (ciclo % 400) < 200) ? HIGH : LOW);
}


// =====================================================================
// SEGUIR LA PELOTA — lo mismo que SIGUIENDO del partido
// =====================================================================
// Sin el despeje y sin volver al medio en el lateral: en el lateral frena
// y termina la prueba.
void pasoSeguir(unsigned long ahora) {
  digitalWrite(LED, HIGH);
  if (inf.muestrasSeguir < 65535) inf.muestrasSeguir++;

  if (!veLaPelota()) {
    // Como el partido: parar() y esperar. Cuando reaparece, rampa y
    // correccion de cero (lo que hace ESPERANDO -> SIGUIENDO).
    parar();
    enMovimiento = false;
    seMovia = false;
    estabaSinPelota = true;
    medirSinCorregir();
    anotar(ahora);
    return;
  }
  if (inf.conPelota < 65535) inf.conPelota++;
  sumaVel += fabs(velocidadLateral);
  if (estabaSinPelota) {
    estabaSinPelota = false;
    reiniciarRampaMovimiento();
    reiniciarCorreccion();
  }

  float desvio = desvioPredicho();

  if (fabs(desvio) < zonaMuertaPelota) {
    parar();
    reiniciarRampaMovimiento();
    enMovimiento = false;
    seMovia = false;
    if (inf.enZonaMuerta < 65535) inf.enZonaMuerta++;
    medirSinCorregir();
    anotar(ahora);
    return;
  }

  int fuerza = (int)(fabs(desvio) * kpLateral);
  if (fuerza > pwmMaxLateral) fuerza = pwmMaxLateral;
  if (fuerza < pwmMinLateral) fuerza = pwmMinLateral;

  bool hayQueIrDerecha = (desvio > 0);
  if (hayQueIrDerecha != ibaDerecha) {
    ibaDerecha = hayQueIrDerecha;
    reiniciarRampaMovimiento();
    if (inf.idasYVueltas < 65535) inf.idasYVueltas++;
  }
  if (!seMovia) {
    seMovia = true;
    if (inf.arranques < 65535) inf.arranques++;
  }

  // El lateral, con la misma regla del partido. Aca no vuelve al medio:
  // frena y se termina la prueba.
  int lado = hayQueIrDerecha ? +1 : -1;
  if (pisaElLateral(lado)) lecturasDeLateral++;
  else                     lecturasDeLateral = 0;
  if (lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
    inf.termino = 1;
    pasarAFrenar(ahora);
    return;
  }

  enMovimiento = true;
  moverDeCostado(hayQueIrDerecha ? fuerza : -fuerza);
  anotar(ahora);
}


// =====================================================================
// SETUP Y LOOP
// =====================================================================
void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);
  parar();

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS);       // la camara
  delay(300);
  Serial.println();
  Serial.print("=== "); Serial.print(VERSION); Serial.println(" — el robot SE MUEVE solo al apretar 1..8 ===");

  leerValores();
  Informe guardado;
  EEPROM.get(DIR_INFORME, guardado);
  if (guardado.magia == MAGIA_INFORME && guardado.movimiento >= 1
      && guardado.movimiento <= N_MOVIMIENTOS && guardado.nota >= 1 && guardado.nota <= 5
      && guardado.nTraza <= MAX_TRAZA) {
    inf = guardado;
    hayInforme = true;
  }

  // Igual que el partido, pero con 3 intentos: aca no hay apuro, y si no
  // aparece se dice fuerte.
  for (int i = 0; i < 3 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < 5000) delay(50);
  }

  imprimirAyuda();
  avisarSiNoSonLosDelPartido();
  if (hayInforme) Serial.println("(hay un informe guardado: tecla r)");
}

void loop() {
  leerCamara();
  unsigned long ahora = millis();

  // Teclas. Mientras se mueve, cualquier tecla lo para.
  while (Serial.available()) {
    char k = Serial.read();
    if (k == '\n' || k == '\r') continue;
    if (paso != NADA) { pararYa(ahora); continue; }
    tecla(k);
  }

  // Tope duro: nada dura mas que esto.
  unsigned long tope = (movimiento == 8) ? (unsigned long)msSeguir + 2000 : MS_TOPE_TOTAL;
  if (paso != NADA && paso != CUENTA && ahora - t_inicio >= tope) {
    terminarMedicion(ahora, 4);
    Serial.println("(se corto por el tope de tiempo)");
    return;
  }

  switch (paso) {
    case NADA:
      ledNota(ahora);
      break;

    case CUENTA: {
      unsigned long cuenta = sinCable ? MS_CUENTA_SIN_CABLE : MS_CUENTA;
      digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);
      if (ahora - t_paso >= cuenta) arrancarDeVerdad(ahora);
      break;
    }

    case COSTADO:
    case COSTADO_VUELTA:
      digitalWrite(LED, HIGH);
      moverDeCostado(ladoCostado * velCostado);
      anotar(ahora);
      // La misma regla del partido: sensor de ese lado en blanco y el otro no.
      if (pisaElLateral(ladoCostado)) lecturasDeLateral++;
      else                            lecturasDeLateral = 0;
      if (lecturasDeLateral >= LECTURAS_PARA_CREER_LATERAL) {
        inf.termino = 1;
        pasarAFrenar(ahora);
        break;
      }
      if (ahora - t_paso >= (unsigned long)msCostado) {
        if (paso == COSTADO_VUELTA) {
          // Como en el seguimiento cuando la pelota cruza: se da vuelta sin
          // frenar, reiniciando la rampa.
          ladoCostado = -ladoCostado;
          reiniciarRampaMovimiento();
          lecturasDeLateral = 0;
          paso = COSTADO; t_paso = ahora;
        } else {
          pasarAFrenar(ahora);
        }
      }
      break;

    case IDA:
      digitalWrite(LED, HIGH);
      adelanteControlado(potenciaDespeje);
      anotar(ahora);
      if (ahora - t_paso >= (unsigned long)msAdelante) {
        if (frenarAntesDeVolver) frenar(); else parar();
        enMovimiento = false;          // en la pausa no se corrige: no cuenta
        paso = PAUSA; t_paso = ahora;
      }
      break;

    case PAUSA: {
      // Igual que PAUSA_MEDIO del partido: frenado (o suelto), sin corregir.
      if (frenarAntesDeVolver) frenar(); else parar();
      medirSinCorregir();
      anotar(ahora);
      unsigned long espera = frenarAntesDeVolver ? (unsigned long)msEsperaAntesDeVolver
                                                 : MS_PAUSA_MEDIO;
      if (ahora - t_paso >= espera) {
        if (rampaEnRetroceso) reiniciarRampaMovimiento();
        enMovimiento = true;
        inf.msInicioAtras = (uint16_t)(ahora - t_inicio);
        paso = ATRAS; t_paso = ahora;
      }
      break;
    }

    case ATRAS: {
      digitalWrite(LED, HIGH);
      atrasControlado(potenciaRetroceso);
      anotar(ahora);
      if (algunoDeAtrasVeBlanco()) {
        inf.termino = 2;
        inf.msHastaLinea = (uint16_t)(ahora - t_paso);
        pasarAFrenar(ahora);
        break;
      }
      // El tope del partido es 1200 ms A 110. Si aca se sube la potencia, el
      // tope baja para que el robot no pueda ir mas lejos que en el partido.
      unsigned long topeAtras = MS_MAX_RETROCESO * (unsigned long)DEF_POTENCIA_RETROCESO
                                / (unsigned long)potenciaRetroceso;
      if (topeAtras > MS_MAX_RETROCESO) topeAtras = MS_MAX_RETROCESO;
      if (ahora - t_paso >= topeAtras) {
        inf.termino = 3;
        pasarAFrenar(ahora);
      }
      break;
    }

    case SEGUIR:
      pasoSeguir(ahora);
      if (paso == SEGUIR && ahora - t_paso >= (unsigned long)msSeguir) pasarAFrenar(ahora);
      break;

    case FRENO:
      frenar();
      medirSinCorregir();
      anotar(ahora);
      if (ahora - t_paso >= msFreno) {
        parar();
        paso = ASENTAR; t_paso = ahora;
      }
      break;

    case ASENTAR:
      parar();
      medirSinCorregir();
      anotar(ahora);
      if (ahora - t_paso >= MS_ASENTAR) terminarMedicion(ahora, inf.termino);
      break;
  }
}
