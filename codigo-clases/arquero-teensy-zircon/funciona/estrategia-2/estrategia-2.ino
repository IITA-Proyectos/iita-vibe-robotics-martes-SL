/* =====================================================================
   ESTRATEGIA 2 (version B) — el arquero con los sensores de atras
   ARRIBA DE LA LINEA DEL AREA
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-09
   =====================================================================

   LA IDEA DEL EQUIPO: que el robot solamente se desplace de costado por
   la linea del area, para nunca quedarse lejos y dejar el arco descubierto,
   y que el limite sea la punta de la linea. Al arrancar, que retroceda
   hasta la linea.

   POR QUE VERSION B: la version A (a caballo: la linea ENTRE el sensor de
   adelante y los de atras) anduvo, pero el margen era casi cero y los de
   atras veian la linea a cada rato y no avanzaba. El equipo eligio la idea
   5 (09/11): los de atras ARRIBA de la linea blanca. Asi ver blanco es lo
   NORMAL, y lo que avisa es dejar de verlo. La version A quedo en
   respaldos/estrategia-2-a-caballo/.

                 cancha
        ·  ·  ·  [A]  ·  ·  ·      A = sensor de ADELANTE: en el verde de la cancha
   ═══[I]════════════════[D]═══   I, D = sensores de ATRAS: ARRIBA de la linea
        ·  ·  ·  area  ·  ·  ·
                 arco

   ---------------------------------------------------------------------
   QUE HACE (SIN patrulla; el DESPEJE se agrego el 09/11, ver el PANEL)
   ---------------------------------------------------------------------
   1. A los 3 s de prenderlo (LED lento, y rapido el ultimo segundo: manos
      afuera) RETROCEDE hasta que un sensor de atras ve la linea, y frena.
   2. SIGUE LA PELOTA DE COSTADO, con los numeros de la 0921 (70 a 160,
      zona muerta 1,4 cm). Sin pelota, se queda quieto.
   3. SE ACOMODA ADELANTE/ATRAS (solo si dura un ratito, no por un toque):
        - el de ADELANTE ve blanco: se fue para ATRAS (los de atras quedaron
          adentro del area) -> va para ADELANTE hasta que un sensor de atras
          vuelva a ver la linea.
        - los DOS de atras ven verde (y el de adelante tambien): se fue para
          ADELANTE -> va para ATRAS hasta que un sensor de atras vuelva a ver
          la linea (o hasta que el de adelante vea blanco: se paso, y vuelve).
      Con que UN sensor de atras vea blanco, esta bien: no corrige nada.
      Si corrigiendo no encuentra la linea en 0,7 s, deja de corregir 1 s
      (para no irse lejos) y sigue cuidando a la pelota.
   4. LA PUNTA DE LA LINEA (la esquina): yendo de costado, si el sensor de
      atras del lado hacia el que va deja de ver blanco y el otro sigue en
      blanco, se termino la linea... o se corrio un poquito para adelante.
      Para saber: frena y hace un PULSITO PARA ATRAS. Si el sensor vuelve a
      ver blanco, se habia corrido (y ya quedo acomodado). Si sigue en verde,
      es la PUNTA: vuelve para adelante y, si se paso, vuelve de costado a
      pisar la linea. En la punta no sigue para ese lado; cuando la pelota
      lo lleva un rato (0,3 s) para el otro lado, se olvida de la punta.
   5. El giroscopio lo mantiene derecho. Sin giroscopio no arranca.

   EL LED: lento antes de arrancar; temblor = sin giroscopio; 2 destellos
   repitiendo = retrocedio 3 s y no encontro la linea (se queda quieto);
   prendido = ve la pelota; apagado = no la ve.

   OJO CON LOS UMBRALES: aca ver blanco es lo normal. Si un sensor de atras
   cree que el verde es blanco, NO se da cuenta de la punta y se pasa. Medir
   con pruebas/calibrar-linea y ponerle a cada sensor su umbral.

   Las piezas de moverse (rueda, aplicar, frenar, rampa, rumbo, leer la
   camara) son COPIA de funciona/partido-0921 (0921i), con los mismos
   numeros. CARGAR: pedirselo a Claude (o cargar.py con esta carpeta).
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// ---- pines (los del arquero, ROBOT1) ----
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

/* ======================== PANEL: lo que se ajusta ======================= */
// ---- los sensores de linea: un umbral para CADA uno ----
// Los dos de atras no leen igual el verde (el 11/08 habia 110 puntos de
// diferencia). Medir con pruebas/calibrar-linea.
int umbralIzq      = 425;
int umbralDer      = 425;
int umbralAdelante = 425;

// ---- arranque ----
const unsigned long MS_ARRANQUE = 3000;     // desde que se PRENDE hasta que se mueve
const int VEL_ATRAS_A_LA_LINEA = 80;        // retroceso hasta la linea. No bajar de ~75
const unsigned long MS_TOPE_BUSCAR_LINEA = 3000;   // si no ve la linea, se para

// ---- seguir la pelota (los de la 0921) ----
float kpLateral = 11.5;                     // PWM por cm de desvio
int   pwmMinLateral = 70;
int   pwmMaxLateral = 130;   // 10/11: 160 -> 130, pedido del equipo ("va muy rapido y se desvia al golpear la pelota")
const float ZONA_MUERTA_PELOTA = 1.4;       // cm: casi enfrente, no se mueve

// ---- acomodarse adelante/atras ----
// Empujoncito para acomodarse. Parado, menos de ~70 no mueve el robot.
// Si no se mueve: subir. Si se pasa de largo la linea: bajar.
const int PROF_CORRECCION = 80;
// Cuanto tiempo seguido tiene que durar para corregir (un toque no cuenta).
// Si corrige de mas (tiembla): subir. Si tarda en acomodarse: bajar.
const unsigned long MS_ADELANTE_BLANCO = 30;   // el de adelante en blanco
const unsigned long MS_ATRAS_VERDE     = 40;   // los dos de atras en verde
// Si corrigiendo no vuelve a encontrar la linea en este tiempo, deja de
// corregir un rato (para no irse lejos del arco).
const unsigned long MS_TOPE_CORRECCION = 700;
const unsigned long MS_DESCANSO_CORRECCION = 1000;

// ---- la punta de la linea (la esquina) ----
// El sensor de atras del lado hacia el que va, en verde, este rato seguido.
// Si frena antes de la punta: subir. Si se pasa mucho: bajar.
const unsigned long MS_PUNTA_SOSTENIDA = 60;
// true = antes de creer que es la punta, hace el pulsito para atras.
// false = le cree de una (mas rapido, pero un corrimiento lo puede frenar).
bool confirmarPunta = true;
const int PWM_PULSO_CHEQUEO = 100;          // el pulsito para atras (parado: menos de ~75 no se mueve)
const unsigned long MS_TOPE_CHEQUEO = 250;  // si en este tiempo no vuelve a ver blanco: es la punta
const int VEL_VOLVER_A_LA_LINEA = 80;       // si se paso de la punta, vuelve de costado a pisarla
const unsigned long MS_TOPE_VOLVER = 1000;
// Despues de la punta, para volver a ir para ese lado tiene que haberse
// ido para el otro lado este rato seguido (si no, chequea a cada ratito).
const unsigned long MS_SOLTAR_PUNTA = 300;

// ---- despeje (09/11, los numeros de la 0921; 10/11: mas lento y sin saltito) ----
bool  despejeActivado = true;
float umbralCm        = 30.0;      // despeja si la pelota esta a esto o menos
float umbralDesvio    = 20.0;      // y a esto o menos de costado
const int VECES_PARA_CREERLE = 3;  // cuadros seguidos de camara
// 10/11, pedido del equipo ("solo el despeje" mas lento): 200 -> 160, y la
// ida 533 -> 666 ms para llegar igual de lejos (la misma cuenta que la
// tecnica challenge). 🚨 umbralCm nunca mas grande que lo que llega la ida.
int   potenciaDespeje = 160;
unsigned long msAdelante = 666;    // la ida (~50 cm)
// 10/11: "a la hora de despejar se va para la izquierda: un saltito al
// arrancar", con el robot QUIETO antes de salir. Primero se probo arrancar
// la ida ya en 70 (la teoria del 03/11): no lo saco. Despues el equipo pidio
// "una rampa de velocidad para cada motor":
//
// 🎯 RAMPA EN CADA MOTOR (solo en el despeje: la ida, el enderezado y la
// vuelta). CADA RUEDA sube de a poco con TODO lo que le llega: el avance,
// el correrse hacia la pelota y la correccion del giroscopio. Antes esas dos
// ultimas llegaban DE GOLPE, y la rueda de atras es la que empuja de costado.
// Bajar es en el acto. Seguir la pelota de costado NO usa esta rampa.
//   Si sigue el tiron: bajar PASO_RAMPA_RUEDA (5 = el doble de suave).
//   Si sale muy lento y no llega a la pelota: subirlo.
const int PASO_RAMPA_RUEDA = 10;            // cuanto sube cada rueda...
const unsigned long MS_RAMPA_RUEDA = 10;    // ...cada este tiempo (10 cada 10 ms = de 0 a 160 en 0,16 s)
// Correrse hacia la pelota sube en el MISMO tiempo que el avance: asi la
// direccion no cambia mientras arranca.
const float KP_PERSECUCION = 6.0;  // en la ida se corre de costado hacia la pelota
const int PWM_MAX_PERSECUCION = 60;  // 10/11: 80 -> 60, el de la 0921
const unsigned long MS_PAUSA_DESPEJE = 300;   // frena y espera antes de volver (minimo)
// 10/11: "en el retroceso lo hace chueco" (zigzag). Como la 0921 (03/11):
// antes de enderezarse y volver, ESPERA A QUE QUEDE PLANO. Al frenar
// despues de la ida el robot se hamaca, y mientras se hamaca el giroscopio
// da cualquier cosa. Espera a que la inclinacion vuelva a la de siempre (la
// aprende quieto en la linea) un ratito seguido; nunca mas de 0,8 s.
bool esperarPlanoAntesDeVolver = true;          // false = solo los 300 ms
const float GRADOS_PLANO = 1.5;                 // "plano" = a esto o menos de la inclinacion de siempre
const unsigned long MS_PLANO_SOSTENIDO = 100;   // ...y quedarse asi este tiempo
const unsigned long MS_MAX_ESPERA_PLANO = 800;  // tope: a los 800 ms vuelve igual
// Al volver: primero se ENDEREZA con el giroscopio (para quedar alineado),
// despues va DERECHO para atras hasta la linea, y ahi se acomoda arriba.
const float GRADOS_DERECHO = 3.0;
const unsigned long MS_TOPE_ENDEREZAR = 1500;
const int PWM_MIN_GIRO = 70;
const int PWM_MAX_GIRO = 85;    // 10/11: 120 -> 85, pedido del equipo ("al enderezarse se pasa"). Menos de 70 no gira
int   potenciaRetroceso = 80;    // 10/11: 110 -> 90 -> 80, pedido del equipo ("vuelve muy rapido"). Como el retroceso del arranque. Si se traba o hace zigzag: volver a 90
// Tope de la vuelta (es solo un seguro: frena al ver la linea). 10/11: 1500 -> 2500,
// porque a 80 tarda mas en llegar (la revision vio que a 90 ya cortaba justo).
const unsigned long MS_MAX_RETROCESO = 2500;

// ---- fijos del partido (no son perillas) ----
const unsigned long BAUDIOS = 19200;
const bool  camaraYInvertida = true;
const float CAMARA_POR_CM = 2.87;
const int LADO_FRENTE  = 50;
const int LADO_TRASERA = 89;
const bool lateralInvertido = false;
const int RAMPA_MOV_PASO = 10;
const unsigned long RAMPA_MOV_MS = 10;
const float KP_RUMBO = 5.0;
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;
const unsigned long MS_ESPERA_DATOS_GIRO = 5000;
const unsigned long MS_FRENO = 200;
/* ============================= FIN DEL PANEL ============================ */

enum Fase { CUENTA, SIN_GIRO, ATRAS_A_LA_LINEA, FRENANDO, EN_LA_LINEA,
            CHEQUEO_PUNTA, VOLVER_ADELANTE, VOLVER_A_LA_LINEA, SIN_LINEA,
            DESPEJE_IDA, DESPEJE_PAUSA, ENDEREZAR, VOLVER_ATRAS };
int vecesSeguidas = 0;     // cuadros seguidos con la pelota cerca y de frente
Fase fase = CUENTA;
unsigned long t_fase = 0;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
float rumboBase = -1;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;
int ruedaActual[3] = {0, 0, 0};   // la rampa de cada motor (10/11): lo que tiene cada rueda ahora
unsigned long t_rampaRuedas = 0;

// camara
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;

// la linea
unsigned long t_buscando = 0;           // retroceso del arranque
unsigned long t_adelanteBlanco = 0;     // el de adelante en blanco, desde cuando
unsigned long t_ambosVerde = 0;         // los dos de atras en verde, desde cuando
unsigned long t_punta = 0;              // el de la punta en verde, desde cuando
unsigned long t_soltarPunta = 0;
unsigned long t_volvioBlanco = 0;       // en el chequeo y al volver
int  corrigiendo = 0;                   // +1 = yendo para adelante, -1 = para atras, 0 = nada
unsigned long t_corrigiendo = 0;
unsigned long t_sinCorregirHasta = 0;   // el descanso si no encontro la linea
int  limiteLado = 0;                    // -1 / +1: llego a la punta de ese lado
int  dirPunta = 0;                      // el lado de la punta que se esta chequeando
bool yaMiroDespuesDelFreno = false;
int  ultimoLado = 0;                    // para reiniciar la rampa al cambiar de lado


// ================================================== motores (del partido)
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

void reiniciarRampaMovimiento() { potenciaRampa = 0; t_rampaMov = millis(); }

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


// ---- la rampa de CADA motor (10/11, solo en el despeje) ----
void reiniciarRuedas() {
  ruedaActual[0] = ruedaActual[1] = ruedaActual[2] = 0;
  t_rampaRuedas = millis();
}

// Una rueda: sube de a PASO_RAMPA_RUEDA (si `toca`), baja en el acto, y si
// cambia de sentido pasa primero por 0.
int pasoRueda(int actual, int objetivo, bool toca) {
  if (objetivo == 0) return 0;
  if ((actual > 0 && objetivo < 0) || (actual < 0 && objetivo > 0)) actual = 0;
  if (abs(objetivo) <= abs(actual)) return objetivo;
  if (!toca) return actual;
  int sube = abs(actual) + PASO_RAMPA_RUEDA;
  if (sube > abs(objetivo)) sube = abs(objetivo);
  return (objetivo > 0) ? sube : -sube;
}

// Como aplicar(), pero cada rueda llega a su fuerza con su propia rampa.
void aplicarSuave(int v1, int v2, int v3) {
  unsigned long ahora = millis();
  bool toca = (ahora - t_rampaRuedas >= MS_RAMPA_RUEDA);
  if (toca) t_rampaRuedas = ahora;
  ruedaActual[0] = pasoRueda(ruedaActual[0], v1, toca);
  ruedaActual[1] = pasoRueda(ruedaActual[1], v2, toca);
  ruedaActual[2] = pasoRueda(ruedaActual[2], v3, toca);
  aplicar(ruedaActual[0], ruedaActual[1], ruedaActual[2]);
}


// ================================================== rumbo (del partido)
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0 && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void reiniciarCorreccion() { integralRumbo = 0; t_ultimoControl = millis(); }

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

// ---- ¿quedo plano? (10/11, copia de la 0921) ----
float incBaseY = 0, incBaseZ = 0;   // la inclinacion "de siempre", quieto en el piso
bool  hayIncBase = false;
unsigned long t_planoDesde = 0;     // desde cuando esta plano (0 = no lo esta)

bool leerInclinacion(float &y, float &z) {
  if (!hayGiroscopo) return false;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0 && e.orientation.z == 0.0) return false;
  y = e.orientation.y;
  z = e.orientation.z;
  return true;
}

// Promedia despacito la inclinacion con el robot QUIETO (si la cancha esta
// en pendiente, la "de siempre" la sigue).
void aprenderInclinacion() {
  static unsigned long t_ultima = 0;
  if (millis() - t_ultima < 20) return;
  t_ultima = millis();
  float y, z;
  if (!leerInclinacion(y, z)) return;
  if (!hayIncBase) { incBaseY = y; incBaseZ = z; hayIncBase = true; return; }
  incBaseY += 0.02 * diferencia(y, incBaseY);
  incBaseZ += 0.02 * diferencia(z, incBaseZ);
}

// ¿Esta plano AHORA? Si no se puede saber, dice que si (para no trabarse).
bool estaPlano() {
  float y, z;
  if (!hayIncBase || !leerInclinacion(y, z)) return true;
  return fabs(diferencia(y, incBaseY)) <= GRADOS_PLANO
      && fabs(diferencia(z, incBaseZ)) <= GRADOS_PLANO;
}

// De costado (s > 0 = DERECHA, con rampa) mas un poquito de avance (prof > 0)
// o de retroceso (prof < 0), mas la correccion de rumbo. Avanzar se suma
// igual que en el partido: (+p, -p, 0).
void costadoYProfundidad(int s, int prof) {
  if (lateralInvertido) s = -s;
  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;
  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;
  int c = correccionDeRumbo();
  aplicar(frente + prof + c, frente - prof + c, -trasera + c);
}

void atrasControlado(int potencia) {
  int p = rampa(potencia);
  int c = correccionDeRumbo();
  aplicar(-p + c, +p + c, 0 + c);
}


// ================================================== camara (del partido)
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
      float dCm  = Xp / CAMARA_POR_CM;
      float desv = fabs((camaraYInvertida ? -(float)Yp : (float)Yp) / CAMARA_POR_CM);
      if (Xp > 0 && dCm <= umbralCm && desv <= umbralDesvio) {
        if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
      } else {
        vecesSeguidas = 0;
      }
    }
  }
}

bool veLaPelota() { return Xp > 0 && (millis() - t_ultimoPaquete < 500); }

// cm reales, + = la pelota esta a la DERECHA del robot.
float desvioPelota() {
  float y = camaraYInvertida ? -(float)Yp : (float)Yp;
  return y / CAMARA_POR_CM;
}


// ================================================== los sensores de linea
bool blancoIzq()      { return analogRead(LINEA_ATRAS_IZQ) >= umbralIzq; }
bool blancoDer()      { return analogRead(LINEA_ATRAS_DER) >= umbralDer; }
bool blancoAdelante() { return analogRead(LINEA_ADELANTE)  >= umbralAdelante; }

// true si la condicion lleva `ms` seguido (t = desde cuando; 0 = no esta pasando).
bool sostenido(bool condicion, unsigned long &t, unsigned long ms, unsigned long ahora) {
  if (!condicion) { t = 0; return false; }
  if (t == 0) t = ahora;
  return ahora - t >= ms;
}

void empezar(Fase f) {
  fase = f; t_fase = millis();
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
  reiniciarRuedas();
}

// Frena y despues vuelve a cuidar la linea. NO borra el limite de la punta.
void frenarYVolverALaLinea() {
  frenar();
  fase = FRENANDO; t_fase = millis();
}


// ================================================== programa
void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE, INPUT);
  parar();
  Serial.begin(19200);
  Serial1.begin(BAUDIOS);       // la camara

  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) delay(50);
  }
  if (Serial) {
    Serial.println("=== ESTRATEGIA 2 version B (2026-11-10d: costado 130, giro 85, vuelta 80) — sensores de atras ARRIBA de la linea del area ===");
    Serial.println(" 3 s, retrocede hasta la linea, y sigue la pelota de costado por la linea.");
  }
  fase = CUENTA; t_fase = 0;    // los 3 s cuentan DESDE QUE SE PRENDIO
}

void loop() {
  leerCamara();
  unsigned long ahora = millis();

  switch (fase) {
    case CUENTA: {
      parar();
      unsigned long falta = (ahora >= MS_ARRANQUE) ? 0 : MS_ARRANQUE - ahora;
      digitalWrite(LED, ((ahora / ((falta <= 1000) ? 100 : 500)) % 2) ? HIGH : LOW);
      if (falta <= 1000) aprenderInclinacion();   // el ultimo segundo ya esta en el piso, sin manos
      if (falta > 0) break;
      float r = rumboActual();
      if (r < 0) {
        if (ahora < MS_ARRANQUE + 2000) break;   // un ratito mas al giroscopio
        fase = SIN_GIRO; break;
      }
      rumboBase = r;
      t_buscando = 0;
      empezar(ATRAS_A_LA_LINEA);
      break;
    }

    case SIN_GIRO:
      parar();
      digitalWrite(LED, ((ahora / 50) % 2) ? HIGH : LOW);
      break;

    // 1. Atras hasta que un sensor de atras vea la linea. (Si el de adelante
    //    ve blanco, se paso: tambien frena, y el acomodarse lo arregla.)
    case ATRAS_A_LA_LINEA:
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atrasControlado(VEL_ATRAS_A_LA_LINEA);
      if (sostenido(blancoIzq() || blancoDer() || blancoAdelante(), t_buscando, 15, ahora)) {
        limiteLado = 0;
        frenarYVolverALaLinea();
        break;
      }
      if (ahora - t_fase >= MS_TOPE_BUSCAR_LINEA) { frenar(); fase = SIN_LINEA; t_fase = ahora; }
      break;

    case FRENANDO:
      frenar();
      if (ahora - t_fase >= MS_FRENO) {
        parar();
        t_adelanteBlanco = t_ambosVerde = t_punta = t_soltarPunta = 0;
        corrigiendo = 0;
        ultimoLado = 0;
        empezar(EN_LA_LINEA);
      }
      break;

    case SIN_LINEA: {
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      parar();
      unsigned long t = ahora % 2000;            // 2 destellos, repitiendo
      digitalWrite(LED, (t < 800 && (t % 400) < 150) ? HIGH : LOW);
      break;
    }

    // 2-4. Arriba de la linea, siguiendo la pelota de costado.
    case EN_LA_LINEA: {
      bool fBl = blancoAdelante(), iBl = blancoIzq(), dBl = blancoDer();
      bool viendo = veLaPelota();
      digitalWrite(LED, viendo ? HIGH : LOW);

      // DESPEJE: la pelota cerca y de frente, varios cuadros seguidos.
      if (despejeActivado && viendo && vecesSeguidas >= VECES_PARA_CREERLE) {
        vecesSeguidas = 0;
        empezar(DESPEJE_IDA);              // (empezar pone las ruedas en 0: arrancan con su rampa)
        break;
      }

      // Para donde quiere ir la pelota (como el SIGUIENDO de la 0921).
      int lateral = 0;
      if (viendo) {
        float d = desvioPelota();
        if (fabs(d) >= ZONA_MUERTA_PELOTA) {
          int fuerza = (int)(fabs(d) * kpLateral);
          if (fuerza > pwmMaxLateral) fuerza = pwmMaxLateral;
          if (fuerza < pwmMinLateral) fuerza = pwmMinLateral;
          lateral = (d > 0) ? fuerza : -fuerza;
        }
      }
      int lado = (lateral > 0) ? 1 : (lateral < 0 ? -1 : 0);

      // La punta: no seguir para ese lado. Se libera cuando se fue un rato
      // para el otro lado.
      if (limiteLado != 0 && lado == limiteLado) { lateral = 0; lado = 0; }
      if (limiteLado != 0 && sostenido(lado == -limiteLado, t_soltarPunta, MS_SOLTAR_PUNTA, ahora)) {
        limiteLado = 0; t_soltarPunta = 0;
      }

      // Cambio de lado o arranque: la rampa desde cero (no salir de un tiron).
      if (lado != ultimoLado) { reiniciarRampaMovimiento(); ultimoLado = lado; }

      // ---- acomodarse adelante/atras ----
      // Empieza a corregir solo si dura un ratito; y una vez que empezo,
      // sigue hasta que un sensor de atras vuelve a pisar la linea.
      if (corrigiendo == 0 && ahora >= t_sinCorregirHasta) {
        if (sostenido(fBl, t_adelanteBlanco, MS_ADELANTE_BLANCO, ahora)) {
          corrigiendo = +1; t_corrigiendo = ahora;             // se fue para atras
        } else if (sostenido(!iBl && !dBl && !fBl, t_ambosVerde, MS_ATRAS_VERDE, ahora)) {
          corrigiendo = -1; t_corrigiendo = ahora;             // se fue para adelante
        }
      }
      if (corrigiendo != 0) {
        if (corrigiendo < 0 && fBl) { corrigiendo = +1; t_corrigiendo = ahora; }  // se paso para atras
        if (iBl || dBl) {                                      // volvio a la linea: listo
          corrigiendo = 0;
          t_adelanteBlanco = t_ambosVerde = 0;
        } else if (ahora - t_corrigiendo >= MS_TOPE_CORRECCION) {
          corrigiendo = 0;                                     // no la encuentra: descanso
          t_adelanteBlanco = t_ambosVerde = 0;
          t_sinCorregirHasta = ahora + MS_DESCANSO_CORRECCION;
        }
      }

      // ---- la punta de la linea ----
      // Yendo para un lado: el sensor de atras de ESE lado en verde y el
      // otro en blanco = se termino la linea (o se corrio un poquito).
      if (lado != 0 && corrigiendo == 0) {
        bool puntaVerde = (lado > 0) ? !dBl : !iBl;
        bool otroBlanco = (lado > 0) ?  iBl :  dBl;
        if (sostenido(puntaVerde && otroBlanco && !fBl, t_punta, MS_PUNTA_SOSTENIDA, ahora)) {
          dirPunta = lado;
          yaMiroDespuesDelFreno = false;
          t_volvioBlanco = 0;
          frenar();
          empezar(CHEQUEO_PUNTA);
          break;
        }
      } else {
        t_punta = 0;
      }

      int prof = corrigiendo * PROF_CORRECCION;
      if (lateral == 0 && prof == 0) { parar(); aprenderInclinacion(); }   // quieto: aprende "plano"
      else costadoYProfundidad(lateral, prof);
      break;
    }

    // ¿Es la punta o se corrio para adelante?
    case CHEQUEO_PUNTA: {
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      bool ladoBlanco = (dirPunta > 0) ? blancoDer() : blancoIzq();
      if (!yaMiroDespuesDelFreno) {
        yaMiroDespuesDelFreno = true;
        if (ladoBlanco) { frenarYVolverALaLinea(); break; }      // fue un toque: sigue
        if (!confirmarPunta) {                                   // le cree de una
          limiteLado = dirPunta;
          t_volvioBlanco = 0;
          empezar(VOLVER_A_LA_LINEA);
          break;
        }
        reiniciarRampaMovimiento();
      }
      // El pulsito para atras. Si el sensor vuelve a ver blanco, se habia
      // corrido para adelante (y ya quedo acomodado). Si no, es la punta.
      costadoYProfundidad(0, -PWM_PULSO_CHEQUEO);
      if (sostenido(ladoBlanco, t_volvioBlanco, 15, ahora)) {
        frenarYVolverALaLinea();
        break;
      }
      if (ahora - t_fase >= MS_FRENO + MS_TOPE_CHEQUEO) {
        limiteLado = dirPunta;                 // es la punta: no sigue para ahi
        frenar();
        empezar(VOLVER_ADELANTE);
      }
      break;
    }

    // Era la punta: deshace el pulsito (para adelante hasta que un sensor
    // de atras pise la linea, o el mismo tiempo que fue para atras).
    case VOLVER_ADELANTE:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      costadoYProfundidad(0, +PWM_PULSO_CHEQUEO);
      if (blancoIzq() || blancoDer() || ahora - t_fase >= MS_FRENO + MS_TOPE_CHEQUEO) {
        frenar();
        t_volvioBlanco = 0;
        empezar(VOLVER_A_LA_LINEA);
      }
      break;

    // Si al frenar se paso de la punta con los DOS sensores, vuelve de
    // costado (para el medio) hasta que uno vuelva a pisar la linea.
    case VOLVER_A_LA_LINEA:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      if (sostenido(blancoIzq() || blancoDer(), t_volvioBlanco, 15, ahora)
          || ahora - t_fase >= MS_FRENO + MS_TOPE_VOLVER) {
        frenarYVolverALaLinea();
        break;
      }
      costadoYProfundidad(-dirPunta * VEL_VOLVER_A_LA_LINEA, 0);
      break;

    // ---- DESPEJE: ida para adelante (corriendose hacia la pelota) ----
    case DESPEJE_IDA: {
      digitalWrite(LED, HIGH);
      int p = potenciaDespeje;        // la rampa la pone cada rueda (aplicarSuave)
      int lat = 0;
      if (veLaPelota()) {
        float d = desvioPelota();
        if (fabs(d) > ZONA_MUERTA_PELOTA) {
          lat = (int)(d * KP_PERSECUCION);
          if (lat >  PWM_MAX_PERSECUCION) lat =  PWM_MAX_PERSECUCION;
          if (lat < -PWM_MAX_PERSECUCION) lat = -PWM_MAX_PERSECUCION;
        }
      }
      // El costado sube en el MISMO tiempo que el avance (lo que tardan las
      // ruedas de adelante en llegar a potenciaDespeje): la direccion no
      // cambia mientras arranca, y no hay tiron de costado al salir.
      {
        unsigned long msSubida = (unsigned long)(potenciaDespeje / PASO_RAMPA_RUEDA) * MS_RAMPA_RUEDA;
        unsigned long dt = ahora - t_fase;
        if (msSubida > 0 && dt < msSubida) lat = (int)((long)lat * (long)dt / (long)msSubida);
      }
      if (lateralInvertido) lat = -lat;
      int fr = (lat * LADO_FRENTE) / 100;
      int tr = (lat * LADO_TRASERA) / 100;
      int c = correccionDeRumbo();
      aplicarSuave(+p + fr + c, -p + fr + c, -tr + c);
      if (ahora - t_fase >= msAdelante) { frenar(); fase = DESPEJE_PAUSA; t_fase = ahora; t_planoDesde = 0; }
      break;
    }

    // Frena, y espera a que deje de hamacarse (minimo 0,3 s, maximo 0,8 s).
    case DESPEJE_PAUSA: {
      if (ahora - t_fase < MS_FRENO) frenar(); else parar();
      bool listo;
      if (!esperarPlanoAntesDeVolver) {
        listo = (ahora - t_fase >= MS_PAUSA_DESPEJE);
      } else {
        if (estaPlano()) { if (t_planoDesde == 0) t_planoDesde = ahora; }
        else             t_planoDesde = 0;
        listo = (ahora - t_fase >= MS_PAUSA_DESPEJE
                 && t_planoDesde != 0 && ahora - t_planoDesde >= MS_PLANO_SOSTENIDO)
             || (ahora - t_fase >= MS_MAX_ESPERA_PLANO);
      }
      if (listo) { parar(); empezar(ENDEREZAR); }
      break;
    }

    // Girar en el lugar hasta quedar mirando igual que al arrancar.
    case ENDEREZAR: {
      float r = rumboActual();
      float e = (r < 0) ? 0 : diferencia(rumboBase, r);
      if (fabs(e) <= GRADOS_DERECHO || ahora - t_fase >= MS_TOPE_ENDEREZAR) {
        frenar();
        t_buscando = 0;
        empezar(VOLVER_ATRAS);
        break;
      }
      int g = (int)(e * KP_RUMBO);
      if (abs(g) < PWM_MIN_GIRO) g = (e > 0) ? PWM_MIN_GIRO : -PWM_MIN_GIRO;
      if (abs(g) > PWM_MAX_GIRO) g = (e > 0) ? PWM_MAX_GIRO : -PWM_MAX_GIRO;
      aplicarSuave(g, g, g);     // las tres para el mismo lado = girar en el lugar (con rampa en cada una)
      break;
    }

    // Derecho para atras (con giroscopio) hasta la linea; ahi EN_LA_LINEA
    // termina de acomodarlo arriba de la linea.
    case VOLVER_ATRAS:
      if (ahora - t_fase < 150) { frenar(); break; }
      {
        int c = correccionDeRumbo();   // para atras = (-p, +p, 0), con rampa en cada rueda
        aplicarSuave(-potenciaRetroceso + c, +potenciaRetroceso + c, 0 + c);
      }
      if (sostenido(blancoIzq() || blancoDer() || blancoAdelante(), t_buscando, 15, ahora)
          || ahora - t_fase >= 150 + MS_MAX_RETROCESO) {
        limiteLado = 0;
        vecesSeguidas = 0;
        frenarYVolverALaLinea();
      }
      break;
  }
}
