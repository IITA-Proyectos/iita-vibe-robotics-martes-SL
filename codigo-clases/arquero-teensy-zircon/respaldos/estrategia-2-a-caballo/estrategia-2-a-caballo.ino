/* =====================================================================
   ESTRATEGIA 2 — el arquero que vive A CABALLO DE LA LINEA DEL AREA
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-09
   =====================================================================

   LA IDEA DEL EQUIPO (textual): "un programa que se desplace lateralmente
   por la linea. El sensor de adelante quede por delante de la linea y los
   sensores de atras por detras de la linea. Que el robot solamente se
   desplace por esa linea del area para nunca quedarse lejos y dejar
   descubierto el arco. Y que el limite lateral sea cuando el robot pisa la
   linea con los sensores de cada costado." "La linea tiene un grosor casi
   justo para que los sensores vean verde, pero es posible que vean la
   linea blanca." "Que se llame estrategia 2. Al arrancar, que retroceda
   hasta la linea."

                 cancha
        ·  ·  ·  [A]  ·  ·  ·      A = sensor de ADELANTE: del lado de la cancha
   ════════════════════════════   la linea del area, ENTRE los sensores
        [I]       ·       [D]      I, D = sensores de ATRAS: adentro del area
                 arco

   ---------------------------------------------------------------------
   QUE HACE (primera version: SIN despeje y SIN patrulla)
   ---------------------------------------------------------------------
   1. A los 3 s de prenderlo (LED lento, y rapido el ultimo segundo: manos
      afuera) RETROCEDE hasta que los sensores de atras CRUZAN la linea del
      area (ven blanco y despues verde otra vez). Frena: queda a caballo.
   2. SIGUE LA PELOTA DE COSTADO por la linea, con los mismos numeros de la
      0921 (70 a 160, zona muerta 1,4 cm). Sin pelota, se queda quieto.
   3. SE ACOMODA ADELANTE/ATRAS, solo si un sensor ve blanco UN RATO SEGUIDO
      (el margen es justo: un toquecito no cuenta):
        - el de ADELANTE ve blanco   -> se fue para atras    -> un poco ADELANTE
        - los DOS de ATRAS ven blanco -> se fue para adelante -> un poco ATRAS
      Mientras los tres ven verde no corrige nada: asi no oscila.
   4. LA ESQUINA: yendo de costado, si SOLO el sensor de atras del lado
      hacia el que va ve blanco, puede ser la linea del COSTADO del area
      (llego a la esquina) o que se fue un poco para adelante. Para saber:
        a) frena y mira: si ya NO ve blanco, se paso de largo de una linea que
           va hacia el arco = la del costado: ES LA ESQUINA.
        b) si sigue en blanco, PULSITO PARA ATRAS: la linea del costado va
           hacia el arco, asi que si SIGUE viendo blanco, ES LA ESQUINA (y
           vuelve el pulsito para adelante). Si deja de verlo, era la linea de
           enfrente: se habia ido para adelante y el pulsito ya lo acomodo.
      En la esquina no sigue para ese lado; cuando la pelota lo lleva un rato
      (0,3 s) para el otro lado, se olvida de la esquina.
   5. El giroscopio lo mantiene derecho mientras se mueve. Sin giroscopio no
      arranca (LED temblando).

   EL LED: lento antes de arrancar; temblor = sin giroscopio; 2 destellos
   repitiendo = retrocedio 3 s y no encontro la linea (se queda quieto);
   prendido = ve la pelota; apagado = no la ve.

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
// diferencia). Si uno "ve blanco" estando en verde: subirle SU umbral.
int umbralIzq      = 425;
int umbralDer      = 425;
int umbralAdelante = 425;

// ---- arranque ----
const unsigned long MS_ARRANQUE = 3000;     // desde que se PRENDE hasta que se mueve
const int VEL_ATRAS_A_LA_LINEA = 80;        // retroceso hasta la linea. No bajar de ~75
const unsigned long MS_TOPE_BUSCAR_LINEA = 3000;   // si no ve la linea, se para
const unsigned long MS_TOPE_CRUZAR_LINEA = 1000;   // tope para terminar de cruzarla

// ---- seguir la pelota (los de la 0921) ----
float kpLateral = 11.5;                     // PWM por cm de desvio
int   pwmMinLateral = 70;
int   pwmMaxLateral = 160;
const float ZONA_MUERTA_PELOTA = 1.4;       // cm: casi enfrente, no se mueve

// ---- acomodarse adelante/atras ----
// Empujoncito para acomodarse. Parado, menos de ~70 no mueve el robot.
// Si al acomodarse se pasa al otro lado de la linea: bajar. Si no se mueve: subir.
const int PROF_CORRECCION = 80;
// Blanco seguido para creerle (el margen es justo: un toquecito no cuenta).
// Si se acomoda de mas (tiembla): subir. Si tarda en acomodarse: bajar.
const unsigned long MS_BLANCO_SOSTENIDO = 40;

// ---- la esquina ----
const unsigned long MS_ESQUINA_SOSTENIDA = 40;  // un solo sensor de atras en blanco, este rato
const int PWM_PULSO_CHEQUEO = 90;               // el pulsito para atras (parado: menos de ~75 no se mueve)
const unsigned long MS_PULSO_CHEQUEO = 150;     // cuanto dura el pulsito
// Despues de la esquina, para volver a ir para ese lado tiene que haberse
// ido para el otro lado este rato seguido (si no, chequea a cada ratito).
const unsigned long MS_SOLTAR_ESQUINA = 300;

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

enum Fase { CUENTA, SIN_GIRO, ATRAS_A_LA_LINEA, ATRAS_CRUZANDO, FRENANDO,
            EN_LA_LINEA, CHEQUEO_ATRAS, CHEQUEO_ADELANTE, SIN_LINEA };
Fase fase = CUENTA;
unsigned long t_fase = 0;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
float rumboBase = -1;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;

// camara
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;

// la linea
unsigned long t_adelanteBlanco = 0, t_ambosAtrasBlanco = 0, t_esquina = 0, t_verdeCruzando = 0;
unsigned long t_blancoBuscando = 0, t_soltarEsquina = 0;
bool yaMiroDespuesDelFreno = false;
bool esEsquina = false;
int limiteLado = 0;          // -1 / +1: llego a la esquina de ese lado (no sigue para ahi)
int dirEsquina = 0;          // el lado de la esquina que se esta chequeando
int ultimoLado = 0;          // para reiniciar la rampa al cambiar de lado


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

// true si `ahora` lleva `ms` seguido con la condicion (t = desde cuando).
bool sostenido(bool condicion, unsigned long &t, unsigned long ms, unsigned long ahora) {
  if (!condicion) { t = 0; return false; }
  if (t == 0) t = ahora;
  return ahora - t >= ms;
}

void empezar(Fase f) {
  fase = f; t_fase = millis();
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
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
    Serial.println("=== ESTRATEGIA 2 (2026-11-09) — el arquero a caballo de la linea del area ===");
    Serial.println(" 3 s, retrocede hasta cruzar la linea, y sigue la pelota de costado por la linea.");
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
      if (falta > 0) break;
      float r = rumboActual();
      if (r < 0) {
        if (ahora < MS_ARRANQUE + 2000) break;   // un ratito mas al giroscopio
        fase = SIN_GIRO; break;
      }
      rumboBase = r;
      empezar(ATRAS_A_LA_LINEA);
      break;
    }

    case SIN_GIRO:
      parar();
      digitalWrite(LED, ((ahora / 50) % 2) ? HIGH : LOW);
      break;

    // 1. atras hasta que un sensor de atras vea la linea...
    case ATRAS_A_LA_LINEA:
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atrasControlado(VEL_ATRAS_A_LA_LINEA);
      if (sostenido(blancoIzq() || blancoDer(), t_blancoBuscando, 15, ahora)) {
        fase = ATRAS_CRUZANDO; t_fase = ahora; t_verdeCruzando = 0; break;
      }
      if (ahora - t_fase >= MS_TOPE_BUSCAR_LINEA) { frenar(); fase = SIN_LINEA; t_fase = ahora; }
      break;

    // ...y un poquito mas, hasta que los DOS de atras vuelvan a ver verde.
    case ATRAS_CRUZANDO:
      atrasControlado(VEL_ATRAS_A_LA_LINEA);
      if (sostenido(!blancoIzq() && !blancoDer(), t_verdeCruzando, 30, ahora)
          || ahora - t_fase >= MS_TOPE_CRUZAR_LINEA) {
        frenar(); fase = FRENANDO; t_fase = ahora;
      }
      break;

    // Frena y se pone a cuidar la linea (al llegar, y despues de cada chequeo).
    // OJO: aca NO se borra el limite de la esquina.
    case FRENANDO:
      frenar();
      if (ahora - t_fase >= MS_FRENO) {
        parar();
        t_adelanteBlanco = t_ambosAtrasBlanco = t_esquina = t_soltarEsquina = 0;
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

    // 2-4. a caballo de la linea, siguiendo la pelota de costado
    case EN_LA_LINEA: {
      bool fBl = blancoAdelante(), iBl = blancoIzq(), dBl = blancoDer();
      bool viendo = veLaPelota();
      digitalWrite(LED, viendo ? HIGH : LOW);

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

      // La esquina: no seguir para ese lado. Se libera cuando se fue un rato
      // para el otro lado.
      if (limiteLado != 0 && lado == limiteLado) { lateral = 0; lado = 0; }
      if (limiteLado != 0 && sostenido(lado == -limiteLado, t_soltarEsquina, MS_SOLTAR_ESQUINA, ahora)) {
        limiteLado = 0; t_soltarEsquina = 0;
      }

      // Cambio de lado o arranque: la rampa desde cero (no salir de un tiron).
      if (lado != ultimoLado) { reiniciarRampaMovimiento(); ultimoLado = lado; }

      // ¿Llego a una esquina? Solo el de atras del lado hacia el que va.
      if (lado != 0) {
        bool delantero = (lado > 0) ? dBl : iBl;
        bool elOtro    = (lado > 0) ? iBl : dBl;
        if (sostenido(delantero && !elOtro && !fBl, t_esquina, MS_ESQUINA_SOSTENIDA, ahora)) {
          dirEsquina = lado;
          yaMiroDespuesDelFreno = false;
          esEsquina = false;
          frenar();
          empezar(CHEQUEO_ATRAS);          // frena, mira, y el pulsito para atras
          break;
        }
      } else {
        t_esquina = 0;
      }

      // Acomodarse adelante/atras (solo con blanco SOSTENIDO).
      int prof = 0;
      if (sostenido(fBl, t_adelanteBlanco, MS_BLANCO_SOSTENIDO, ahora))           prof = +PROF_CORRECCION;
      else if (sostenido(iBl && dBl, t_ambosAtrasBlanco, MS_BLANCO_SOSTENIDO, ahora)) prof = -PROF_CORRECCION;

      if (lateral == 0 && prof == 0) parar();
      else costadoYProfundidad(lateral, prof);
      break;
    }

    // ¿Esquina o se fue para adelante? Un solo sensor de atras vio blanco.
    case CHEQUEO_ATRAS: {
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      bool ladoBlanco = (dirEsquina > 0) ? blancoDer() : blancoIzq();
      // a) Ya frenado, ¿el sensor sigue en blanco? Si NO: al frenar se paso
      //    de largo de una linea que va de adelante hacia atras = la linea
      //    del costado del area. Es la esquina (y no hace falta el pulsito).
      if (!yaMiroDespuesDelFreno) {
        yaMiroDespuesDelFreno = true;
        if (!ladoBlanco) {
          limiteLado = dirEsquina;
          fase = FRENANDO; t_fase = ahora;
          break;
        }
      }
      // b) El pulsito para atras. Si SIGUE en blanco, esta sobre la linea del
      //    costado (que va hacia el arco): la esquina. Si deja de verlo, era la
      //    linea de enfrente: se habia ido para adelante, y el pulsito ya lo
      //    acomodo.
      costadoYProfundidad(0, -PWM_PULSO_CHEQUEO);
      if (ahora - t_fase >= MS_FRENO + MS_PULSO_CHEQUEO) {
        esEsquina = ladoBlanco;
        frenar();
        if (esEsquina) {
          limiteLado = dirEsquina;           // no sigue para ese lado
          empezar(CHEQUEO_ADELANTE);         // y vuelve el pulsito para adelante
        } else {
          fase = FRENANDO; t_fase = ahora;   // se queda donde lo dejo el pulsito
        }
      }
      break;
    }

    // Era la esquina: el mismo pulsito para adelante, a la misma altura de antes.
    case CHEQUEO_ADELANTE:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      costadoYProfundidad(0, +PWM_PULSO_CHEQUEO);
      if (ahora - t_fase >= MS_FRENO + MS_PULSO_CHEQUEO) {
        frenar();
        fase = FRENANDO; t_fase = ahora;      // frena y vuelve a EN_LA_LINEA
      }
      break;
  }
}
