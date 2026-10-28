/* =====================================================================
   DELANTERO - programa de partido - Roboliga 2026 - IITA Salta
   =====================================================================
   Robot 2 (ROBOT2 del codigo 2025) - Teensy 4.1 + Zircon - serie 15708680
   Ruedas (medido en banco): 8/7/6 IZQUIERDA - 11/12/4 DERECHA - 2/5/3 TRASERA

   QUE HACE:  BUSCANDO -> CENTRANDO -> AVANZANDO -> ORBITANDO -> PATEA_ADEL
              -> PATEA_ATRAS -> BUSCANDO. La LINEA BLANCA interrumpe todo.

   CARGAR:    CARGAR-ROBOT.bat (1 azul / 2 amarillo) y leer el cartel del
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

// ---- BUSCAR, CENTRAR Y AVANZAR ----
const int VEL_GIRO_CONTINUO            = 35;               // fuerza del giro buscando. Si zumba y no gira: subir a 45 o 50
const int VEL_CENT                     = 78;               // fuerza de cada pulsito. Tambien al recentrarla orbitando y al apuntar al cero
const int VEL_AVANCE                   = 95;               // fuerza yendo hacia la pelota cuando esta LEJOS
const int VEL_AVANCE_CERCA             = 70;               // fuerza al llegar. Si la empuja al llegar: bajar de a 5, pero debajo de ~70 puede quedarse plantado
const int XP_FRENAR                    = 70;               // desde estos cm empieza a aflojar, de VEL_AVANCE a VEL_AVANCE_CERCA

// ---- RODEAR LA PELOTA (ORBITA) ----
const int XP_ORBITA                    = 32;               // a estos cm deja de avanzar y empieza a rodearla. Si la choca al empezar: subir
const int XP_SUELTA                    = 55;               // si se aleja mas que esto (cm) vuelve a avanzar. Siempre MAYOR que XP_ORBITA
const int VEL_ORB_IMPULSO              = 120;              // golpe de la rueda trasera para arrancar la orbita
const int VEL_ORB_TRASERA              = 67;               // fuerza de la vuelta, y al rearrancar despues de recentrar (ahi no hay golpe). Mas = mas rapida. Si se planta: subir
const unsigned long MS_ORBITA_MAX      = 10000;            // cuanto orbita buscando el arco antes de pasar al plan B (ms)

// ---- PATEAR ----
const int VEL_PATADA                   = 180;              // fuerza del golpe. Mas = llega mas lejos, pero se levanta y vuelca mas facil
const int MS_PATADA                    = 450;              // cuanto dura el golpe (ms). Mas = llega mas lejos y se pasa mas cerca de la linea
const int TRIM_PATADA                  = 5;                // compensacion fija. Si se va siempre a la DERECHA: subir. A la IZQUIERDA: bajar
const float KP_PATADA                  = 6.0;              // corrige por GRADO torcido. Mas = corrige mas fuerte (y puede zigzaguear)
const float KD_PATADA                  = 0.6;              // corrige por GRADO/SEGUNDO: frena el giro antes de que crezca (baja el pico)
const int VEL_RETROCESO                = 110;              // despues del golpe, marcha atras con esta fuerza...
const int MS_RETROCESO                 = 700;              // ...durante esto (ms), y vuelve a buscar

// ---- PUNTERIA (con el arco lejos, que es casi siempre) ----
const float TOL_ANG_LEJOS_PELOTA       = 12.0;             // grados, pelota adelante, con el arco lejos. Tambien para patear al rumbo cero
const float TOL_ANG_LEJOS_ARCO         = 8.0;              // grados, punteria con el arco lejos. Si orbita y no patea: subir a 10 o 12

// ---- RUMBO CERO (plan B: el cero es hacia donde mira AL PRENDER la bateria) ----
const float VUELTAS_PARA_RUMBO0        = 1.0;              // vueltas alrededor de la pelota sin patear antes del plan B

// ---- LINEA BLANCA (UMBRAL_LINEA se MIDE en cada cancha con grabar-linea: 1 DER, 2 IZQ, 3 DEL) ----
int UMBRAL_LINEA[3]                    = { 424, 434, 440 }; // [MEDIDO 20/10, base nueva] desde que lectura cada sensor dice "esto es blanco"
const unsigned long MS_RETROCESO_LINEA = 210;              // al verla, retrocede esto (ms)...
const int VEL_ESCAPE_FUERTE            = 135;              // ...con esta fuerza. Mas = se despega mas rapido, pero se levanta
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
const float TOL_ANG_ALINEADO           = 8.0;              // grados: punteria SOLO con el arco CERCA (menos de ARCO_LEJOS_MIN). Lejos (casi siempre) manda TOL_ANG_LEJOS_ARCO
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
const unsigned long MS_ESPERA_ARRANQUE = 2000;             // cuenta regresiva al prenderlo, para sacar la mano (ms)
const unsigned long MS_MIRAR_ARCOS     = 2000;             // sin arco fijo: cuanto mira al prenderse para elegir el arco (ms)...
const int MUESTRAS_MINIMAS_ARCO        = 3;                // ...y cuantas veces tiene que ver un arco para creerle

// ---- PARA QUE LADO GIRA: tocar SOLO si gira para el lado equivocado ----
const bool GIRO_INVERTIDO              = true;             // sentido al buscar, centrar y recentrar orbitando. Cambiar SOLO si al centrar se aleja de la pelota
const bool ORBITA_INVERTIDA            = true;             // lado de la orbita: SIEMPRE si ORBITA_CAMINO_CORTO = false; si no, solo sin ver el arco ni giroscopio
const bool SENTIDO_ORBITA_INVERTIDO    = false;            // [01/09] cambiar SOLO si para quedar detras de la pelota da la vuelta larga (pasa entre pelota y arco). Irse al lado contrario al arco esta BIEN
const bool GIRO_RUMBO_INVERTIDO        = false;            // [sin verificar] si al apuntar al cero girando en el lugar se aleja del cero

// ---- CAMARA Y GIROSCOPIO ----
const int XP_MAX                       = 150;              // mas lejos que esto (cm) no le cree a la PELOTA: suelen ser manchas naranjas
const int XARCO_MAX                    = 200;              // el techo para el ARCO. 200 = lo maximo que manda la camara: acepta todo
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
              DESTRABANDO };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

int  pwmGiroBusc = 0;              // PWM actual de la rampa del giro
unsigned long t_rampaGiro    = 0;
unsigned long t_frenoAlVer   = 0;  // 0 = no esta frenando el envion
unsigned long t_sinAcercarse = 0;  // 0 = el reloj del destrabe esta parado
unsigned long t_perdidaDesde = 0;  // desde cuando no ve la pelota (0 = la ve)
unsigned long t_orbSinVerla  = 0;  // idem pero DENTRO de la orbita (0 = la ve)
bool recentrandoOrbita = false;    // <- la histeresis de la banda de angulo
float giradoOrbita   = 0;          // grados de rumbo girados en esta orbita (con signo)
float rumboOrbPrevio = 0;          // ultima lectura del rumbo, orbitando
unsigned long t_muestraOrb  = 0;   // cuando se leyo el rumbo por ultima vez, orbitando
unsigned long t_buscaRumbo0 = 0;   // 0 = espera al arco; si no, desde cuando busca el rumbo cero
int  mejorXp    = 0;               // lo mas cerca que estuvo en este intento
int  nDestrabes = 0;

bool avisadoSinCamara = false;

unsigned long nPaquetes = 0, nTirados = 0, nLoops = 0;
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
  }
  return "?";
}

bool arrancarGiroscopo() {
  if (!bno.begin(GIRO_SIN_BRUJULA ? OPERATION_MODE_IMUPLUS : OPERATION_MODE_NDOF)) return false;
  bno.setExtCrystalUse(true);
  delay(700);                      // AHORA despues: setExtCrystalUse reinicia la
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
    pwmRampaPatada += RAMPA_PATADA_PASO;
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
  if (nuevo == ESCAPA_LINEA) t_sinAcercarse = 0;
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
}

// ===== ARRANQUE =====
// CARGAR-ROBOT.bat lee la linea 'ARCO:' del cartel: no cambiarla.
void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Serial1.begin(19200);

  while (!Serial && millis() < 3000) { }
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
  Serial.print("Arranca en "); Serial.print(MS_ESPERA_ARRANQUE / 1000);
  Serial.println(" segundos.");
  delay(MS_ESPERA_ARRANQUE);

  t_ultimoPaquete = millis();
  t_contadores    = millis();   // si no, la primera medicion sale con dt enorme
  cambiarA(BUSCANDO);
}

// Para que lado orbitar: segun el arco si lo ve; si no, segun el rumbo cero.
bool sentidoParaOrbitar() {
  bool porDefecto = !ORBITA_INVERTIDA;
  if (!ORBITA_CAMINO_CORTO) return porDefecto;

  bool haciaElPositivo;
  if (millis() - arcoT() < MS_GRACIA) {
    haciaElPositivo = (anguloDe(arcoX(), arcoY()) > 0);
  } else if (giroscopoSano()) {
    haciaElPositivo = (diferencia(rumboCero, rumboActual()) > 0);
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

  // ---- PATEA_ADEL: el golpe (solo lo corta la linea) ----
  else if (estado == PATEA_ADEL) {
    int vel = rampaPatada(VEL_PATADA);
    bool conLazo = (giroscopoSano() && rumboPatadaValido);
    if (conLazo) avanzarDerecho(vel, rumboAlPatear);
    else         motoresPatada(vel - TRIM_PATADA, vel + TRIM_PATADA);

    float errP = conLazo ? diferencia(rumboAlPatear, rumboActual()) : 0.0;
    if (fabs(errP) > fabs(picoEnCurso)) picoEnCurso = errP;

    if (enEstado >= (unsigned long)MS_PATADA) {
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
    retroceder(VEL_RETROCESO);
    if (enEstado >= (unsigned long)MS_RETROCESO) cambiarA(BUSCANDO);
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
      if (t_buscaRumbo0 == 0 && fabs(giradoOrbita) >= 360.0 * VUELTAS_PARA_RUMBO0) {
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
    else if (veoArco && pelotaAdelante && arcoAlineado) {
      Serial.print("*** ALINEADO con el arco "); Serial.print(arcoNombre());
      Serial.print("  (pelota a "); Serial.print(angPelota, 1);
      Serial.print(" grados, arco a "); Serial.print(angArco, 1);
      Serial.print(", separados "); Serial.print(fabs(diferencia(angArco, angPelota)), 1);
      Serial.print(", tolerancia "); Serial.print(tolPelota, 0);
      if (arcoLejos) Serial.print(" por arco LEJOS");
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (t_buscaRumbo0 != 0 && giroscopoSano()
             && fabs(angPelota) <= TOL_ANG_LEJOS_PELOTA
             && fabs(diferencia(rumboCero, rumboOrbPrevio)) <= TOL_RUMBO
             && !(VETO_ARCO_PROPIO && veoArcoPropio)) {
      Serial.print("*** mirando al rumbo cero con la pelota adelante (rumbo ");
      Serial.print(rumboOrbPrevio, 0); Serial.print(", cero "); Serial.print(rumboCero, 0);
      Serial.print(", pelota a "); Serial.print(angPelota, 1);
      Serial.println(" grados)  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (t_buscaRumbo0 != 0 && millis() - t_buscaRumbo0 > MS_RUMBO0_MAX) {
      Serial.println("... no llegue a quedar mirando al rumbo cero -> vuelvo a buscar");
      cambiarA(BUSCANDO);
    }
    else if (t_buscaRumbo0 == 0 && enEstado > MS_ORBITA_MAX) {   // se le acabo el tiempo y no lo vio
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
        bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
        orbitar(sentidoOrbita, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
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
        cambiarA(PATEA_ADEL);
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
      Serial.println(" /s");
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
