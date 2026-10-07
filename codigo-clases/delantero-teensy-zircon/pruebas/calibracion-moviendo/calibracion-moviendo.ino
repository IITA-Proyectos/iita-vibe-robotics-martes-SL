// =====================================================================
//  CALIBRACION MOVIENDO — un numero por cada cosa que hace el robot
//  Robot DELANTERO (Teensy 4.1 / placa Zircon)
// =====================================================================
//
//  🚨 ESTE PROGRAMA MUEVE EL ROBOT. Va EN EL PISO o EN LA CANCHA, con espacio
//  libre alrededor. NO en la mesa: se cae. Y nadie con las manos cerca.
//
//  PARA QUE ES. El companero de pruebas/calibracion-quieto/. Aquel mide lo que
//  NO se mueve (linea, giroscopio, camara); este mide, uno por uno, los
//  movimientos que el robot hace jugando, con las MISMAS constantes del
//  firmware. Sirve para saber si el piso de esta cancha le cambia la vida:
//
//    FASE 1  PISO DE PWM, rueda por rueda   con cuanta potencia arranca cada una
//    FASE 2  AVANZAR                        se va derecho o curva?
//    FASE 3  GIRAR BUSCANDO (continuo)      cuantos grados por segundo barre
//    FASE 4  CENTRAR A PULSOS               cuantos grados gira EN UN PULSO
//    FASE 5  ORBITAR                        grados/segundo y cuanto tarda una vuelta
//    FASE 6  ESCAPAR DE LINEA               para donde sale con cada sensor
//
//  LA FASE 4 ES LA QUE MAS FALTABA. "Cuantos grados gira en un pulso" es el
//  numero con el que se decide si la banda de histeresis del centrado tiene
//  sentido: si la ventana es mas angosta que un pulso, el robot no puede
//  quedarse adentro y se zarandea. Hasta hoy ese numero se discutia sin
//  medirlo.
//
//  ⚠️ LA PATADA NO ESTA ACA, A PROPOSITO. Ya se mide en dos lugares:
//  pruebas/patada-derecha/ y el firmware mismo, que desde el 06/10 guarda el
//  desvio de las ultimas 5 patadas. Y el 06/10 aprendimos por que una tercera
//  copia seria peor que nada: patada-derecha mide SIN trim, SIN derivativo y a
//  420 ms, mientras el firmware patea CON trim, CON derivativo y a 500 ms. Dos
//  versiones distintas ya nos hicieron sacar conclusiones equivocadas. Una
//  tercera, peor. La patada se mide donde se patea.
//
//  COMO SE USA SIN CABLE: igual que calibracion-quieto. Se carga, se desenchufa
//  el USB con la BATERIA PUESTA, se lo deja hacer las fases en el piso, y se
//  vuelve a enchufar SIN APAGAR LA BATERIA para leer el informe (queda en RAM
//  y se repite cada 3 s).
//
//  TODO SE MIDE CON EL GIROSCOPIO. El robot no tiene encoders, asi que no
//  puede medir CUANTO se movio: solo cuanto GIRO. Por eso la fase 2 dice si
//  va derecho, no cuantos centimetros avanzo.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <math.h>

// ---------- pines, IGUALES a los del firmware ----------
// Si estos no coinciden con funciona/delantero, la medicion no sirve de nada.
// pruebas/piso-de-pwm/ tiene los pines del ARQUERO y por eso nunca se pudo
// correr: movia los motores que no eran. Este los tiene bien.
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6
#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4
#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

const int LED = 13;

// ---------- lo que hace el robot jugando, copiado del firmware ----------
const int VEL_AVANCE        = 95;
const int VEL_CENT          = 78;
const int MS_PULSO_CENT     = 32;
const int MS_ESPERA_CENT    = 320;
const int VEL_GIRO_CONTINUO = 50;
const int RAMPA_GIRO_PASO   = 10;
const unsigned long RAMPA_GIRO_MS = 10;
const int VEL_ORB_FRENTE    = 30;
const int VEL_ORB_IMPULSO   = 120;
const int VEL_ORB_TRASERA   = 67;
const int MS_ORB_IMPULSO    = 300;
const unsigned long MS_RETROCESO_LINEA = 210;
const int VEL_ESCAPE_FUERTE = 170;
const bool GIRO_INVERTIDO   = true;

// ---------- lo que se puede tocar ----------
const unsigned long MS_AVISO_FASE = 8000;   // para sacar las manos y mirar
const unsigned long MS_AVANZAR    = 1500;   // fase 2
const unsigned long MS_GIRO_CONT  = 3000;   // fase 3
const int           PULSOS_CENT   = 5;      // fase 4: cuantos pulsos promedia
const unsigned long MS_ORBITA     = 4000;   // fase 5

// Fase 1: la rampa que busca el piso de arranque de cada rueda.
const int PISO_DESDE = 20;     // arranca bien abajo, seguro no se mueve
const int PISO_HASTA = 160;    // si a 160 no arranco, hay un problema mecanico
const int PISO_PASO  = 5;
const unsigned long PISO_MS_POR_PASO = 250;  // cuanto espera en cada escalon
const float PISO_GRADOS_MINIMOS = 4.0;       // con esto ya se considera que giro

// ---------------------------------------------------------------------

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiro = false;

// --- resultados, que son el informe ---
int   pisoRueda[3]  = { -1, -1, -1 };
const char* nombreRueda[3] = { "IZQUIERDA", "DERECHA  ", "TRASERA  " };

float avanzarDesvio = 0, avanzarPico = 0;
float giroContGradPorSeg = 0;
float centGradosPorPulso = 0;
float orbitaGradPorSeg = 0;
float escapeGiro[3] = { 0, 0, 0 };

bool listo = false;


// ---------- motores ----------
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// MISMO ESTADO DE LAS PATAS QUE parar(), EFECTO OPUESTO: con PWM 255 el driver
// cortocircuita el motor y lo traba. parar() suelta, frenar() frena.
void frenar() {
  analogWrite(IZQ_PWM, 255); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 255); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 255); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void motoresRotando(bool sentidoA, int vel) {
  int a = sentidoA ? 1 : 0;
  int b = sentidoA ? 0 : 1;
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, a); digitalWrite(TRA_INB, b);
}

void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera);     digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

// Copia exacta del firmware: la mascara dice QUE sensor vio blanco.
void escaparDeLinea(int m, int velocidad) {
  int v[3] = { 0, 0, 0 };
  if (m & 1) { v[1] -= 1; v[2] += 1; }
  if (m & 2) { v[0] += 1; v[2] -= 1; }
  if (m & 4) { v[0] -= 1; v[1] += 1; }

  int pico = 0;
  for (int i = 0; i < 3; i++) if (abs(v[i]) > pico) pico = abs(v[i]);
  if (pico == 0) { frenar(); return; }

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

// Una rueda sola, para buscarle el piso de arranque.
void unaRueda(int cual, int vel) {
  parar();
  if (cual == 0) { analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0); }
  if (cual == 1) { analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0); }
  if (cual == 2) { analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, 1); digitalWrite(TRA_INB, 0); }
}


// ---------- giroscopio ----------
float rumbo() {
  sensors_event_t ev;
  bno.getEvent(&ev);
  return ev.orientation.x;
}

// La diferencia mas corta entre dos rumbos, en -180..180. Sin esto, pasar de
// 359 a 1 grado se leeria como un giro de 358.
float diferencia(float a, float b) {
  float d = a - b;
  while (d >  180) d -= 360;
  while (d < -180) d += 360;
  return d;
}

void cuenta(const char* que, unsigned long ms) {
  parar();
  Serial.println();
  Serial.print(">>> "); Serial.println(que);
  Serial.println("    SACATE Y DEJALE ESPACIO.");
  for (long s = ms / 1000; s > 0; s--) {
    Serial.print("    "); Serial.print(s); Serial.println("...");
    for (int k = 0; k < 10; k++) { digitalWrite(LED, (k % 2) ? HIGH : LOW); delay(100); }
  }
  digitalWrite(LED, HIGH);
  Serial.println("    MIDIENDO.");
}


// ---------- FASE 1: el piso de PWM de cada rueda ----------
void fasePiso() {
  cuenta("FASE 1 de 6: PISO DE PWM. Cada rueda sola, subiendo de a poco.", MS_AVISO_FASE);
  for (int r = 0; r < 3; r++) {
    pisoRueda[r] = -1;
    for (int v = PISO_DESDE; v <= PISO_HASTA; v += PISO_PASO) {
      float r0 = rumbo();
      unaRueda(r, v);
      unsigned long t0 = millis();
      float mayor = 0;
      while (millis() - t0 < PISO_MS_POR_PASO) {
        float d = fabs(diferencia(rumbo(), r0));
        if (d > mayor) mayor = d;
        delay(10);
      }
      parar();
      delay(150);
      if (mayor >= PISO_GRADOS_MINIMOS) { pisoRueda[r] = v; break; }
    }
    Serial.print("    rueda "); Serial.print(nombreRueda[r]);
    if (pisoRueda[r] < 0) Serial.println(": NO ARRANCO ni a 160");
    else { Serial.print(": arranco con PWM "); Serial.println(pisoRueda[r]); }
  }
  parar();
  digitalWrite(LED, LOW);
}


// ---------- FASE 2: avanzar derecho ----------
void faseAvanzar() {
  cuenta("FASE 2 de 6: AVANZAR. Se va derecho o curva?", MS_AVISO_FASE);
  float r0 = rumbo();
  unsigned long t0 = millis();
  avanzarPico = 0;
  while (millis() - t0 < MS_AVANZAR) {
    avanzar(VEL_AVANCE);
    float d = diferencia(rumbo(), r0);
    if (fabs(d) > fabs(avanzarPico)) avanzarPico = d;
    delay(10);
  }
  avanzarDesvio = diferencia(rumbo(), r0);
  frenar(); delay(300); parar();
  digitalWrite(LED, LOW);
}


// ---------- FASE 3: girar buscando, continuo con rampa ----------
void faseGiroContinuo() {
  cuenta("FASE 3 de 6: GIRAR BUSCANDO (continuo a 50 con rampa).", MS_AVISO_FASE);
  float r0 = rumbo();
  float acumulado = 0, anterior = r0;
  int pwm = 0;
  unsigned long t0 = millis(), tRampa = millis();
  while (millis() - t0 < MS_GIRO_CONT) {
    if (millis() - tRampa >= RAMPA_GIRO_MS) {
      tRampa = millis();
      if (pwm < VEL_GIRO_CONTINUO) {
        pwm += RAMPA_GIRO_PASO;
        if (pwm > VEL_GIRO_CONTINUO) pwm = VEL_GIRO_CONTINUO;
      }
    }
    motoresRotando(!GIRO_INVERTIDO, pwm);
    float r = rumbo();
    acumulado += fabs(diferencia(r, anterior));   // se acumula: puede dar varias vueltas
    anterior = r;
    delay(10);
  }
  frenar(); delay(300); parar();
  giroContGradPorSeg = acumulado * 1000.0 / (float)MS_GIRO_CONT;
  digitalWrite(LED, LOW);
}


// ---------- FASE 4: centrar a pulsos ----------
// EL NUMERO QUE MAS FALTABA: cuantos grados mueve UN pulso de centrado.
void faseCentrar() {
  cuenta("FASE 4 de 6: CENTRAR A PULSOS. Cuantos grados mueve UN pulso.", MS_AVISO_FASE);
  float total = 0;
  for (int p = 0; p < PULSOS_CENT; p++) {
    float r0 = rumbo();
    unsigned long t0 = millis();
    while (millis() - t0 < (unsigned long)MS_PULSO_CENT) {
      motoresRotando(!GIRO_INVERTIDO, VEL_CENT);
      delay(2);
    }
    frenar();
    // La pausa del firmware es FRENADA, no suelta: hay que esperarla entera
    // para medir lo que de verdad se movio, inercia incluida.
    delay(MS_ESPERA_CENT);
    float movido = fabs(diferencia(rumbo(), r0));
    total += movido;
    Serial.print("    pulso "); Serial.print(p + 1);
    Serial.print(": "); Serial.print(movido, 1); Serial.println(" grados");
  }
  parar();
  centGradosPorPulso = total / (float)PULSOS_CENT;
  digitalWrite(LED, LOW);
}


// ---------- FASE 5: orbitar ----------
void faseOrbitar() {
  cuenta("FASE 5 de 6: ORBITAR. Necesita MAS espacio: gira alrededor de un punto adelante.",
         MS_AVISO_FASE);
  float anterior = rumbo();
  float acumulado = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < MS_ORBITA) {
    bool enImpulso = (millis() - t0 < (unsigned long)MS_ORB_IMPULSO);
    orbitar(true, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
    float r = rumbo();
    acumulado += fabs(diferencia(r, anterior));
    anterior = r;
    delay(10);
  }
  frenar(); delay(300); parar();
  orbitaGradPorSeg = acumulado * 1000.0 / (float)MS_ORBITA;
  digitalWrite(LED, LOW);
}


// ---------- FASE 6: escapar de linea ----------
void faseEscape() {
  const char* quien[3] = { "sensor 1 (DERECHO)", "sensor 2 (IZQUIERDO)", "sensor 3 (DELANTERO)" };
  for (int i = 0; i < 3; i++) {
    char msg[90];
    snprintf(msg, sizeof(msg), "FASE 6 de 6: ESCAPE como si viera blanco el %s.", quien[i]);
    cuenta(msg, MS_AVISO_FASE);
    float r0 = rumbo();
    unsigned long t0 = millis();
    while (millis() - t0 < MS_RETROCESO_LINEA) {
      escaparDeLinea(1 << i, VEL_ESCAPE_FUERTE);
      delay(5);
    }
    frenar(); delay(400); parar();
    escapeGiro[i] = diferencia(rumbo(), r0);
    Serial.print("    "); Serial.print(quien[i]);
    Serial.print(": giro "); Serial.print(escapeGiro[i], 1); Serial.println(" grados");
    digitalWrite(LED, LOW);
  }
}


// ---------- el informe ----------
void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.println(" CALIBRACION MOVIENDO - informe");
  Serial.println("=====================================================");
  if (!hayGiro) {
    Serial.println(" SIN GIROSCOPIO: casi nada de esto se pudo medir.");
    Serial.println(" Todo lo de abajo se mide contando grados, y sin el chip");
    Serial.println(" no hay grados. Corre primero calibracion-quieto.");
  }

  Serial.println();
  Serial.println(" 1) PISO DE PWM, rueda por rueda");
  Serial.println("    (con cuanta potencia ARRANCA cada una desde quieto)");
  for (int r = 0; r < 3; r++) {
    Serial.print("    "); Serial.print(nombreRueda[r]); Serial.print(":  ");
    if (pisoRueda[r] < 0) Serial.println("NO ARRANCO ni a 160  <- revisar mecanica o cable");
    else                  Serial.println(pisoRueda[r]);
  }
  int pisoPeor = -1;
  for (int r = 0; r < 3; r++) if (pisoRueda[r] > pisoPeor) pisoPeor = pisoRueda[r];
  if (pisoPeor > 0) {
    Serial.print("    EL QUE MANDA es el mas alto: "); Serial.println(pisoPeor);
    Serial.println("    Cualquier VEL_ de arranque por debajo de ese numero no mueve el robot.");
    Serial.print("    Hoy el firmware usa VEL_AVANCE="); Serial.print(VEL_AVANCE);
    Serial.print(", VEL_CENT="); Serial.print(VEL_CENT);
    Serial.print(", VEL_GIRO_CONTINUO="); Serial.println(VEL_GIRO_CONTINUO);
    if (VEL_GIRO_CONTINUO < pisoPeor)
      Serial.println("    OJO: VEL_GIRO_CONTINUO quedo DEBAJO del piso. Solo arranca por la rampa.");
  }

  Serial.println();
  Serial.println(" 2) AVANZAR");
  Serial.print  ("    se torcio "); Serial.print(avanzarDesvio, 1);
  Serial.print  (" grados (pico "); Serial.print(avanzarPico, 1);
  Serial.print  (") en "); Serial.print(MS_AVANZAR); Serial.println(" ms a VEL_AVANCE");
  if (fabs(avanzarDesvio) < 3.0) Serial.println("    VEREDICTO: va derecho.");
  else                           Serial.println("    VEREDICTO: CURVA. Mismo sintoma que la patada torcida.");
  Serial.println("    (no mide centimetros: el robot no tiene encoders, solo giroscopio)");

  Serial.println();
  Serial.println(" 3) GIRAR BUSCANDO (continuo con rampa)");
  Serial.print  ("    "); Serial.print(giroContGradPorSeg, 0);
  Serial.println(" grados por segundo");
  if (giroContGradPorSeg > 1.0) {
    Serial.print("    una vuelta entera le lleva ");
    Serial.print(360.0 / giroContGradPorSeg, 1); Serial.println(" s");
  } else {
    Serial.println("    CASI NO GIRO. El PWM del giro continuo no alcanza en este piso.");
  }

  Serial.println();
  Serial.println(" 4) CENTRAR A PULSOS  <- el numero que mas faltaba");
  Serial.print  ("    UN pulso mueve "); Serial.print(centGradosPorPulso, 1);
  Serial.println(" grados");
  Serial.println("    PARA QUE SIRVE: la banda de histeresis del centrado tiene que ser");
  Serial.println("    MAS ANCHA que esto, o el robot no puede quedarse adentro y se");
  Serial.println("    zarandea. Hoy el firmware usa entra 12 / sale 6, o sea una banda de 6.");
  if (centGradosPorPulso > 6.0)
    Serial.println("    OJO: un pulso es MAS GRANDE que la banda. Hay que ensanchar la banda");
  else
    Serial.println("    OK: un pulso entra en la banda.");

  Serial.println();
  Serial.println(" 5) ORBITAR");
  Serial.print  ("    "); Serial.print(orbitaGradPorSeg, 0);
  Serial.println(" grados por segundo");
  if (orbitaGradPorSeg > 1.0) {
    Serial.print("    media vuelta (180 grados) le lleva ");
    Serial.print(180.0 / orbitaGradPorSeg, 1); Serial.println(" s");
    Serial.println("    Comparar con MS_ORBITA_MAX del firmware (20 s): si media vuelta");
    Serial.println("    tarda mas que eso, se rinde antes de encontrar el arco.");
  } else {
    Serial.println("    CASI NO ORBITO. Subir MS_ORB_IMPULSO antes que VEL_ORB_IMPULSO:");
    Serial.println("    mas golpe empuja la pelota.");
  }

  Serial.println();
  Serial.println(" 6) ESCAPAR DE LINEA (cuanto giro en cada escape)");
  const char* quien[3] = { "sensor 1 (DERECHO)  ", "sensor 2 (IZQUIERDO)", "sensor 3 (DELANTERO)" };
  for (int i = 0; i < 3; i++) {
    Serial.print("    "); Serial.print(quien[i]);
    Serial.print(": "); Serial.print(escapeGiro[i], 1); Serial.println(" grados");
  }
  Serial.println("    COMO LEERLO: el escape tiene que sacar al robot HACIA ATRAS del");
  Serial.println("    sensor que vio blanco, no hacerlo girar en el lugar. Numeros chicos");
  Serial.println("    son buena senal; uno muy grande dice que ese escape gira en vez de");
  Serial.println("    retroceder. Y mirando el robot: con el sensor 3 (delantero) tiene");
  Serial.println("    que irse derecho para atras.");

  Serial.println("=====================================================");
  Serial.println(" (para repetir: desenchufa y enchufa, o RESET)");
}


// ---------------------------------------------------------------------
void setup() {
  pinMode(LED, OUTPUT);
  int pines[9] = { IZQ_INA, IZQ_INB, IZQ_PWM, DER_INA, DER_INB, DER_PWM,
                   TRA_INA, TRA_INB, TRA_PWM };
  for (int i = 0; i < 9; i++) pinMode(pines[i], OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }

  Serial.println();
  Serial.println("#####################################################");
  Serial.println("#  CALIBRACION MOVIENDO                             #");
  Serial.println("#  un numero por cada cosa que hace el robot         #");
  Serial.println("#  🚨 MUEVE EL ROBOT: en el PISO, NO en la mesa.      #");
  Serial.println("#####################################################");
  Serial.println("Las 6 fases arrancan solas, por tiempo. El LED parpadea");
  Serial.println("en la cuenta y queda FIJO mientras mide.");
  Serial.println("La PATADA no esta aca a proposito: se mide en el firmware.");

  hayGiro = bno.begin();
  if (hayGiro) {
    delay(700);
    bno.setExtCrystalUse(true);
    uint8_t sys, autotest, err;
    bno.getSystemStatus(&sys, &autotest, &err);
    hayGiro = (sys == 5);
    Serial.print("Giroscopo: SYS_STATUS="); Serial.print(sys);
    Serial.println(hayGiro ? " (fusion corriendo) OK" : " -> sin el no hay grados que medir");
  } else {
    Serial.println("Giroscopo: no contesta -> sin el no hay grados que medir");
  }

  fasePiso();
  faseAvanzar();
  faseGiroContinuo();
  faseCentrar();
  faseOrbitar();
  faseEscape();

  parar();
  listo = true;
  informe();
}

void loop() {
  // El informe se repite cada 3 s para leerlo DESPUES, enchufando el USB en la
  // mesa SIN APAGAR LA BATERIA. Si se corta la energia, se pierde.
  parar();
  static unsigned long t = 0;
  if (!listo) return;
  if (millis() - t < 3000) return;
  t = millis();
  informe();
}
