/* =====================================================================
   TACTICAL CHALLENGE - DELANTERO - programa APARTE del de partido
   =====================================================================
   Robot 2 (ROBOT2) - Teensy 4.1 + Zircon - serie 15708680

   El desafio del profe: desde cualquier lugar de la cancha, SIEMPRE en
   este orden y sin saltear ninguno:

       PASO 1   ir al arco AMARILLO   -> frena al ver la LINEA del arco
       PASO 2   ir al arco AZUL       -> frena al ver la LINEA del arco
       PASO 3   tocar la PELOTA despacio  -> y se queda quieto

   Un paso empieza recien cuando termino el anterior. Mientras va al
   amarillo no le importa si ve el azul o la pelota (y asi con los otros).

   COMO:    cada paso hace lo mismo que el principal con la pelota: gira
            buscando el objetivo, se endereza a pulsitos y avanza aflojando
            al llegar. Lo unico que cambia es QUE mira.
   LINEA:   [08/11] yendo a un arco, CUALQUIER linea que vea es que llego
            (CUALQUIER_LINEA_ES_ARCO): frena, espera MS_PAUSA_PASO, se
            despega de la linea y va al siguiente paso.
            Yendo a la pelota, la linea interrumpe todo: retrocede y vuelve
            a buscar, SIN cambiar de paso.

   CARGAR:  TACTICAL-CHALLENGE.bat. Para volver a jugar: CARGAR-ROBOT.bat.
   OJO:     lo marcado "COPIA DEL PRINCIPAL" son los numeros del principal
            del 03/11. Si se cambian aca, en el partido NO cambian (y al
            reves).
   ===================================================================== */

// =====================================================================
//   LO DEL DESAFIO: lo que se ajusta en la cancha
//   ms = milisegundos - cm = centimetros de la camara - fuerza 0..255
// =====================================================================
const bool CUALQUIER_LINEA_ES_ARCO = true; // [08/11] yendo a un arco, CUALQUIER linea cuenta como "llegue". false = solo con el arco a menos de CM_ARCO_CERCA (se quedaba trabado en el amarillo)
const int CM_ARCO_CERCA           = 80;    // solo con CUALQUIER_LINEA_ES_ARCO = false: la linea cuenta solo con el arco a menos de esto (cm)
const unsigned long MS_ARCO_VISTO = 1000;  // ...y con el arco visto hace menos que esto (ms)
const unsigned long MS_PAUSA_PASO = 1000;  // al llegar a un arco queda frenado esto (ms), despues se despega de la linea y va al siguiente
const int CM_EMPIEZA_TOQUE        = 30;    // con la pelota a esto (cm) y derecha, deja de mirarla y va derecho a tocarla
const int CM_PELOTA_PEGADA        = 40;    // si la pierde de vista avanzando mas cerca que esto, quedo pegada (de muy cerca la camara no la ve): la toca igual
const int VEL_TOQUE               = 75;    // fuerza del toque. Si la empuja lejos: bajar de a 5 (debajo de ~70 se puede quedar plantado)
const unsigned long MS_TOQUE      = 500;   // cuanto dura el toque (ms). Si no llega a tocarla: subir. Si la empuja lejos: bajar

// =====================================================================
//   COPIA DEL PRINCIPAL (03/11): buscar, enderezarse, avanzar y la linea
// =====================================================================
const int VEL_GIRO_CONTINUO            = 35;    // fuerza del giro buscando. Si zumba y no gira: subir a 45 o 50
const int RAMPA_GIRO_PASO              = 10;    // el giro arranca suave: sube esto de fuerza...
const unsigned long RAMPA_GIRO_MS      = 10;    // ...cada esta cantidad de ms
const unsigned long MS_FRENO_AL_VER    = 200;   // al verlo frena esto (ms), para no pasarse de largo
const int VEL_CENT                     = 78;    // fuerza de cada pulsito al enderezarse
const int MS_PULSO_CENT                = 32;    // cuanto dura cada pulsito (ms)
const int MS_ESPERA_CENT               = 320;   // pausa frenada entre pulsitos (ms): es cuando la camara mira
const float TOL_ANG_ENTRA              = 12.0;  // grados: mas torcido que esto -> se pone a enderezar
const float TOL_ANG_SALE               = 6.0;   // grados: mas derecho que esto -> avanza. Siempre MENOR que TOL_ANG_ENTRA
const int VEL_AVANCE                   = 95;    // fuerza yendo al objetivo cuando esta LEJOS
const int VEL_AVANCE_CERCA             = 70;    // fuerza al llegar
const int XP_FRENAR                    = 70;    // desde estos cm empieza a aflojar, de VEL_AVANCE a VEL_AVANCE_CERCA
const bool GIRO_INVERTIDO              = true;  // sentido al buscar y al enderezar. Cambiar SOLO si al enderezar se aleja

const bool LINEA_ACTIVA                = true;  // escapar de la linea blanca. En false puede salirse de la cancha
int UMBRAL_LINEA[3]                    = { 300, 300, 440 }; // [09/11: DER e IZQ bajados a 300 por el equipo; eran 424/434 del 20/10] desde que lectura cada sensor dice "esto es blanco"
const unsigned long MS_RETROCESO_LINEA = 210;   // al verla, retrocede esto (ms)...
const int VEL_ESCAPE_FUERTE            = 135;   // ...con esta fuerza
const unsigned long MS_CIEGO_LINEA     = 300;   // despues de verla no la mira durante esto (ms). NUNCA menos que MS_RETROCESO_LINEA
const unsigned long MS_LINEA_CONFIRMA  = 5;     // tiene que verla seguida esto (ms) para creerle

const int XP_MAX                       = 200;   // mas lejos que esto (cm) no le cree a la PELOTA. 200 = le cree a todo
const int XARCO_MAX                    = 200;   // lo mismo para los ARCOS
const bool FILTRO_BRILLO_PELOTA        = true;  // [03/11] un arco amarillo PEGADO a la pelota es su brillo: no creerle
const float BRILLO_ANG                 = 10.0;  // grados: "pegado" = en la misma direccion que la pelota...
const float BRILLO_FACTOR              = 1.5;   // ...y a menos de esta cantidad de veces la distancia de la pelota
const unsigned long MS_GRACIA          = 300;   // sin verlo menos que esto (ms), cuenta como que lo ve
const unsigned long SIN_DATOS_MS       = 1500;  // sin datos de la camara este tiempo (ms), avisa por el monitor
const unsigned long MS_ESPERA_ARRANQUE = 2000;  // cuenta regresiva al prenderlo, para sacar la mano (ms)

// OJO: no escribir ninguna funcion antes de los 'enum' (mas abajo): el
// preprocesador de Arduino pone los prototipos antes de la primera funcion.

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

const int PIN_VERSION_PLACA = 32;     // pin 32 en LOW = Mark1 (linea en A11/A13/A12)

// ===== LOS PASOS DEL DESAFIO (en este orden, siempre) =====
enum Paso   { PASO_AMARILLO, PASO_AZUL, PASO_PELOTA };
enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, LLEGUE_ARCO, DESPEGANDO,
              TOCANDO, ESCAPA_LINEA, TERMINADO };

Paso   paso           = PASO_AMARILLO;
Estado estado         = BUSCANDO;
Estado estadoAnterior = TERMINADO;    // distinto a BUSCANDO: asi avisa el primero

// ===== LO QUE MANDA LA CAMARA =====
int Xp = 0, Yp = 0;
int Xam = 0, Yam = 0;              // arco AMARILLO (byte 202)
int Xaz = 0, Yaz = 0;              // arco AZUL     (byte 203)

int XpBueno = 0, YpBueno = 0;      // ultima posicion BUENA de cada uno
int XamBueno = 0, YamBueno = 0;
int XazBueno = 0, YazBueno = 0;

unsigned long t_ultimaPelota   = 0;
unsigned long t_ultimoAmarillo = 0;
unsigned long t_ultimoAzul     = 0;
unsigned long t_ultimoPaquete  = 0;

unsigned long nPaquetes = 0, nTirados = 0, nLoops = 0, nBrillos = 0;
unsigned long t_contadores = 0;
bool avisadoSinCamara = false;

// ===== RELOJES =====
unsigned long t_inicio      = 0;   // cuando arranco el desafio
unsigned long t_inicioPaso  = 0;   // cuando empezo el paso en que esta
unsigned long t_entroEstado = 0;
unsigned long t_cicloPulso  = 0;
unsigned long t_rampaGiro   = 0;
unsigned long t_frenoAlVer  = 0;   // 0 = no esta frenando el envion
unsigned long t_ultimoAviso = 0;
int  pwmGiroBusc = 0;              // fuerza actual de la rampa del giro
float segundosPaso[3] = { 0, 0, 0 };   // a los cuantos segundos termino cada paso

// ===== LINEA BLANCA =====
int  pinLinea[3]  = { A11, A13, A12 };   // Mark1; se corrige en setup()
const char* versionPlaca = "?";
bool lineaHabilitada = false;
int  lineaArranque[3] = { -1, -1, -1 };
int  mascaraLinea = 0;
unsigned long t_disparoLinea = 0;
unsigned long t_sensorDesde[3] = { 0, 0, 0 };

// ===== MOTORES (COPIA DEL PRINCIPAL) =====
// parar() SUELTA las ruedas (sigue de largo). frenar() las TRABA.
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

void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Rapido de lejos y suave al llegar: desde XP_FRENAR baja de VEL_AVANCE
// hasta VEL_AVANCE_CERCA, que la alcanza justo en la distancia de 'llegada'.
int velAcercarse(int dist, int llegada) {
  if (dist >= XP_FRENAR) return VEL_AVANCE;
  if (dist <= llegada || llegada >= XP_FRENAR) return VEL_AVANCE_CERCA;
  long sobra = (long)(dist - llegada) * (VEL_AVANCE - VEL_AVANCE_CERCA);
  return VEL_AVANCE_CERCA + (int)(sobra / (XP_FRENAR - llegada));
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

// ===== LINEA BLANCA (COPIA DEL PRINCIPAL) =====
// bit 0 = sensor 1 DERECHO, bit 1 = sensor 2 IZQUIERDO, bit 2 = sensor 3 DELANTERO.
int leerLineas() {
  int m = 0;
  for (int i = 0; i < 3; i++)
    if (analogRead(pinLinea[i]) >= UMBRAL_LINEA[i]) m |= (1 << i);
  return m;
}

// Escapa para el lado contrario al sensor: apaga la rueda de enfrente y las
// otras dos empujan al reves entre si. Sensor 3 (adelante) = derecho para atras.
void escaparDeLinea(int m, int velocidad) {
  int v[3] = { 0, 0, 0 };
  if (m & 1) { v[1] -= 1; v[2] += 1; }   // hacia la DI  -> IZQ apagada
  if (m & 2) { v[0] += 1; v[2] -= 1; }   // hacia la DD  -> DER apagada
  if (m & 4) { v[0] -= 1; v[1] += 1; }   // hacia la T   -> TRA apagada

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

// ===== ANGULOS Y DISTANCIAS =====
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

// Distancia de verdad (cm), no solo lo de adelante: asi "a 40 cm del arco"
// vale aunque el arco este un poco de costado.
int distanciaDe(int X, int Y) {
  if (X <= 0) return 999;
  return (int)(sqrt((float)X * X + (float)Y * Y) + 0.5);
}

// ===== CAMARA (COPIA DEL PRINCIPAL) =====
// Paquete de 9 bytes: 201,Xp,Yp+100, 202,Xam,Yam+100, 203,Xaz,Yaz+100 (cm).
// X adelante, Y al costado (Y positivo = izquierda). 200 y +-100 = recortes.
bool esBrilloDeLaPelota(int xa, int ya) {
  if (!FILTRO_BRILLO_PELOTA) return false;
  if (!((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100))) return false;
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
      Xam = xam;  Yam = yam - 100;
      Xaz = xaz;  Yaz = yaz - 100;
      t_ultimoPaquete = millis();
      nPaquetes++;

      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xam > 0) && (Xam <= XARCO_MAX) && (abs(Yam) < 100)) {
        if (esBrilloDeLaPelota(Xam, Yam)) {
          nBrillos++;
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

// ===== EL OBJETIVO DEL PASO EN QUE ESTA =====
// Esto es lo unico que cambia entre un paso y otro: QUE mira.
int objX() {
  if (paso == PASO_AMARILLO) return XamBueno;
  if (paso == PASO_AZUL)     return XazBueno;
  return XpBueno;
}
int objY() {
  if (paso == PASO_AMARILLO) return YamBueno;
  if (paso == PASO_AZUL)     return YazBueno;
  return YpBueno;
}
unsigned long objT() {
  if (paso == PASO_AMARILLO) return t_ultimoAmarillo;
  if (paso == PASO_AZUL)     return t_ultimoAzul;
  return t_ultimaPelota;
}
const char* objNombre() {
  if (paso == PASO_AMARILLO) return "arco AMARILLO";
  if (paso == PASO_AZUL)     return "arco AZUL";
  return "PELOTA";
}

const char* elLa() { return (paso == PASO_PELOTA) ? "la " : "el "; }

const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO:     return "BUSCANDO";
    case CENTRANDO:    return "ENDEREZANDO";
    case AVANZANDO:    return "AVANZANDO";
    case LLEGUE_ARCO:  return "LLEGUE (pausa)";
    case DESPEGANDO:   return "me despego de la linea";
    case TOCANDO:      return "TOCANDO";
    case ESCAPA_LINEA: return "!LINEA!";
    case TERMINADO:    return "TERMINADO";
  }
  return "?";
}

float segundos() { return (millis() - t_inicio) / 1000.0; }

void cambiarA(Estado nuevo) {
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
  if (nuevo == BUSCANDO) { t_frenoAlVer = 0; pwmGiroBusc = 0; }   // la busqueda arranca suave
}

void empezarPaso(Paso p) {
  paso = p;
  t_inicioPaso = millis();
  Serial.println();
  Serial.print("=== PASO "); Serial.print((int)p + 1); Serial.print(" de 3: ");
  Serial.print(p == PASO_PELOTA ? "TOCAR la " : "ir al ");
  Serial.print(objNombre());
  Serial.print("   (a los "); Serial.print(segundos(), 1); Serial.println(" s) ===");
  cambiarA(BUSCANDO);
}

void imprimirResultado() {
  Serial.println("==============================================");
  Serial.print("TACTICAL CHALLENGE TERMINADO en "); Serial.print(segundosPaso[2], 1); Serial.println(" s");
  Serial.print("   1) arco AMARILLO  a los "); Serial.print(segundosPaso[0], 1); Serial.println(" s");
  Serial.print("   2) arco AZUL      a los "); Serial.print(segundosPaso[1], 1); Serial.println(" s");
  Serial.print("   3) PELOTA tocada  a los "); Serial.print(segundosPaso[2], 1); Serial.println(" s");
  Serial.println("Para repetir: apagar y prender el robot.");
  Serial.println("==============================================");
}

// La linea blanca. La direccion del escape la decide el sensor que la vio
// PRIMERO (COPIA DEL PRINCIPAL).
// [08/11] Yendo a un arco, cualquier linea: llego (frena).
// Yendo a la pelota: retrocede y sigue con el mismo paso.
void revisarLinea() {
  bool lineaCiega = (t_disparoLinea != 0 && millis() - t_disparoLinea < MS_CIEGO_LINEA);
  if (lineaCiega) return;

  int mCrudo = leerLineas();
  for (int i = 0; i < 3; i++) {
    if (mCrudo & (1 << i)) { if (t_sensorDesde[i] == 0) t_sensorDesde[i] = millis(); }
    else                     t_sensorDesde[i] = 0;
  }

  int m = 0;
  for (int i = 0; i < 3; i++)
    if (t_sensorDesde[i] != 0 && millis() - t_sensorDesde[i] >= MS_LINEA_CONFIRMA)
      m |= (1 << i);
  if (m == 0) return;

  int primero = -1;
  for (int i = 0; i < 3; i++) {
    if (!(m & (1 << i))) continue;
    if (primero < 0 || t_sensorDesde[i] < t_sensorDesde[primero]) primero = i;
  }
  mascaraLinea = (1 << primero);

  if (estado != ESCAPA_LINEA) {
    const char* sensor = (primero == 0) ? "DERECHO" : ((primero == 1) ? "IZQUIERDO" : "DELANTERO");
    bool arcoVisto = (millis() - objT() < MS_ARCO_VISTO);
    int  distArco  = distanciaDe(objX(), objY());

    // ---- yendo a un arco: la linea es SU linea -> llegue ----
    // Solo si ya vio ESE arco en este paso: si no, recien despegado del
    // amarillo, girando buscando el azul, contaria la linea del amarillo.
    bool vioEnEstePaso = (objT() > t_inicioPaso);
    bool esSuLinea = vioEnEstePaso
                     && (CUALQUIER_LINEA_ES_ARCO || (arcoVisto && distArco <= CM_ARCO_CERCA));
    if (paso != PASO_PELOTA && esSuLinea) {
      frenar();
      segundosPaso[paso] = segundos();
      Serial.print("*** LLEGUE: vi la LINEA yendo al "); Serial.print(objNombre());
      Serial.print(" (sensor "); Serial.print(primero + 1); Serial.print(" "); Serial.print(sensor);
      if (arcoVisto) { Serial.print(", arco a "); Serial.print(distArco); Serial.print(" cm"); }
      else           { Serial.print(", el arco no lo veo"); }
      Serial.print(") a los "); Serial.print(segundosPaso[paso], 1); Serial.println(" s");
      cambiarA(LLEGUE_ARCO);
      return;
    }

    // ---- cualquier otra linea: retrocede y sigue con el mismo paso ----
    Serial.print("!!! LINEA BLANCA (sensor "); Serial.print(primero + 1);
    Serial.print(" "); Serial.print(sensor);
    Serial.print(") yendo hacia "); Serial.print(elLa()); Serial.print(objNombre());
    bool loVeoAhora = (paso == PASO_PELOTA) ? (millis() - objT() < MS_GRACIA) : arcoVisto;
    if (loVeoAhora) {
      Serial.print(" a "); Serial.print(distArco); Serial.print(" cm");
    } else {
      Serial.print(" (no lo veo)");
    }
    Serial.println(" -> retrocedo y sigo con el mismo paso");
    if (paso != PASO_PELOTA && !vioEnEstePaso) {
      Serial.print("    (no la cuento como llegada: en este paso todavia no vi el ");
      Serial.print(objNombre()); Serial.println(")");
    } else if (paso != PASO_PELOTA) {
      Serial.print("    (no la cuento como llegada: hace falta el arco a menos de ");
      Serial.print(CM_ARCO_CERCA); Serial.println(" cm)");
    }
    t_disparoLinea = millis();
    cambiarA(ESCAPA_LINEA);
  }
}

// ===== ARRANQUE =====
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
  Serial.println("TACTICAL CHALLENGE - DELANTERO (NO es el de partido)");
  Serial.println("   1) arco AMARILLO   2) arco AZUL   3) tocar la PELOTA");
  if (CUALQUIER_LINEA_ES_ARCO) {
    Serial.print("   arcos: frena en CUALQUIER LINEA (despues de haber visto ese arco)");
  } else {
    Serial.print("   arcos: frena al ver la LINEA con el arco a menos de "); Serial.print(CM_ARCO_CERCA);
    Serial.print(" cm");
  }
  Serial.print(", espera "); Serial.print(MS_PAUSA_PASO);
  Serial.println(" ms y se despega");
  Serial.print("   pelota: a "); Serial.print(CM_EMPIEZA_TOQUE);
  Serial.print(" cm la toca a "); Serial.print(VEL_TOQUE);
  Serial.print(" durante "); Serial.print(MS_TOQUE); Serial.println(" ms y queda quieto");
  Serial.println(FILTRO_BRILLO_PELOTA ? "   camara: un arco amarillo pegado a la pelota es su brillo -> NO le creo"
                                      : "   camara: filtro del brillo de la pelota APAGADO");
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
  Serial.print("Placa (pin 32): "); Serial.println(versionPlaca);

  if (LINEA_ACTIVA) {
    for (int i = 0; i < 3; i++) lineaArranque[i] = analogRead(pinLinea[i]);
    Serial.print("Linea: sensores leen ");
    Serial.print(lineaArranque[0]); Serial.print(" / ");
    Serial.print(lineaArranque[1]); Serial.print(" / ");
    Serial.print(lineaArranque[2]);
    Serial.print("   umbrales "); Serial.print(UMBRAL_LINEA[0]);
    Serial.print(" / "); Serial.print(UMBRAL_LINEA[1]);
    Serial.print(" / "); Serial.println(UMBRAL_LINEA[2]);
    lineaHabilitada = true;
    if (leerLineas() != 0)
      Serial.println("!!! OJO: ya lee blanco al arrancar. Si escapa sin parar, los umbrales no sirven aca.");
    else
      Serial.println("Linea: OK, escape ACTIVADO (anula todo lo demas).");
  } else {
    Serial.println("Linea: apagada por configuracion.");
    Serial.println("!!! SIN LINEA NO PUEDE LLEGAR A LOS ARCOS: frena al verla. Poner LINEA_ACTIVA = true");
  }

  Serial.print("Arranca en "); Serial.print(MS_ESPERA_ARRANQUE / 1000);
  Serial.println(" segundos.");
  delay(MS_ESPERA_ARRANQUE);

  t_ultimoPaquete = millis();
  t_contadores    = millis();
  t_inicio        = millis();
  empezarPaso(PASO_AMARILLO);
}

// ===== LOOP =====
// Orden: 1) camara  2) LINEA BLANCA, que manda sobre todo  3) el paso.
void loop() {
  nLoops++;
  leerCamara();
  unsigned long enEstado = millis() - t_entroEstado;

  // ---- TERMINADO: quieto. Repite el resultado para leerlo despues ----
  if (estado == TERMINADO) {
    if (enEstado < 1000) frenar(); else parar();
    if (millis() - t_ultimoAviso > 5000) { t_ultimoAviso = millis(); imprimirResultado(); }
    return;
  }

  // Al llegar a un arco ya esta EN la linea: ahi no la mira (ni al despegarse).
  if (lineaHabilitada && estado != LLEGUE_ARCO && estado != DESPEGANDO) revisarLinea();
  enEstado = millis() - t_entroEstado;   // por si la linea cambio el estado

  bool  loVeo = (millis() - objT()) < MS_GRACIA;
  int   dist  = distanciaDe(objX(), objY());
  float ang   = anguloDe(objX(), objY());
  // Al arco no frena por distancia: va aflojando hasta VEL_AVANCE_CERCA y frena en la linea.
  int   llegada = (paso == PASO_PELOTA) ? CM_EMPIEZA_TOQUE : 0;

  // ---- ESCAPA_LINEA: retrocede y vuelve a buscar EL MISMO objetivo ----
  if (estado == ESCAPA_LINEA) {
    if (enEstado < MS_RETROCESO_LINEA) escaparDeLinea(mascaraLinea, VEL_ESCAPE_FUERTE);
    else                               cambiarA(BUSCANDO);
  }

  // ---- LLEGUE_ARCO: frenado en la linea un ratito ----
  else if (estado == LLEGUE_ARCO) {
    frenar();
    if (enEstado >= MS_PAUSA_PASO) {
      t_disparoLinea = millis();     // la ventana ciega cubre el despegue
      cambiarA(DESPEGANDO);
    }
  }

  // ---- DESPEGANDO: se aleja de la linea (como el escape) y pasa al siguiente ----
  else if (estado == DESPEGANDO) {
    if (enEstado < MS_RETROCESO_LINEA) {
      escaparDeLinea(mascaraLinea, VEL_ESCAPE_FUERTE);
    } else if (paso == PASO_AMARILLO) {
      empezarPaso(PASO_AZUL);
    } else {
      empezarPaso(PASO_PELOTA);
    }
  }

  // ---- TOCANDO: derecho y despacio, sin mirar (de cerca no la ve) ----
  else if (estado == TOCANDO) {
    if (enEstado < MS_TOQUE) {
      avanzar(VEL_TOQUE);
    } else {
      frenar();
      segundosPaso[2] = segundos();
      Serial.print("*** PELOTA TOCADA a los "); Serial.print(segundosPaso[2], 1);
      Serial.println(" s -> me quedo quieto");
      cambiarA(TERMINADO);
      t_ultimoAviso = millis();
      imprimirResultado();
    }
  }

  // ---- no ve el objetivo: lo busca girando ----
  else if (!loVeo) {
    if (paso == PASO_PELOTA && estado == AVANZANDO && dist <= CM_PELOTA_PEGADA) {
      Serial.print("*** la deje de ver avanzando a "); Serial.print(dist);
      Serial.println(" cm: quedo pegada -> la toco");
      cambiarA(TOCANDO);
    } else {
      if (estado != BUSCANDO) cambiarA(BUSCANDO);
      t_frenoAlVer = 0;
      girarContinuo(!GIRO_INVERTIDO);
    }
  }

  // ---- recien lo vio: frena el envion del giro ----
  else if (estado == BUSCANDO
           && (t_frenoAlVer == 0 || millis() - t_frenoAlVer < MS_FRENO_AL_VER)) {
    if (t_frenoAlVer == 0) {
      t_frenoAlVer = millis();
      pwmGiroBusc  = 0;
      Serial.print("*** vi "); Serial.print(elLa()); Serial.print(objNombre());
      Serial.print(" a "); Serial.print(dist); Serial.println(" cm -> freno el envion");
    }
    frenar();
  }

  // (la llegada al arco la decide revisarLinea(): frena al ver SU linea)

  // ---- la pelota cerca y derecha: a tocarla ----
  else if (paso == PASO_PELOTA && dist <= CM_EMPIEZA_TOQUE && fabs(ang) <= TOL_ANG_ENTRA) {
    Serial.print("*** pelota a "); Serial.print(dist);
    Serial.println(" cm y derecha -> la toco");
    cambiarA(TOCANDO);
  }

  // ---- enderezarse o avanzar (COPIA DEL PRINCIPAL) ----
  else {
    float desvio = fabs(ang);
    if (estado == CENTRANDO) {
      if (desvio < TOL_ANG_SALE) cambiarA(AVANZANDO);
    } else {
      if (desvio > TOL_ANG_ENTRA)    cambiarA(CENTRANDO);
      else if (estado != AVANZANDO)  cambiarA(AVANZANDO);
    }

    if (estado == CENTRANDO) {
      bool haciaUnLado = (objY() > 0);
      if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
      rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
    } else {
      avanzar(velAcercarse(dist, llegada));
    }
  }

  // ---- AVISOS POR EL MONITOR (no cambian lo que hace el robot) ----
  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    Serial.print(">>> "); Serial.print(nombreEstado(estado));
    Serial.print("   "); Serial.print(objNombre()); Serial.print(": ");
    if (millis() - objT() < MS_GRACIA) {
      Serial.print(distanciaDe(objX(), objY())); Serial.print(" cm, a ");
      Serial.print(anguloDe(objX(), objY()), 1); Serial.println(" grados");
    } else {
      Serial.println("no lo veo");
    }
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
    Serial.print("   [PASO "); Serial.print((int)paso + 1); Serial.print("/3 ");
    Serial.print(objNombre()); Serial.print(" | "); Serial.print(nombreEstado(estado));
    Serial.print("]  ");
    if (loVeo) {
      Serial.print("X="); Serial.print(objX());
      Serial.print(" Y="); Serial.print(objY());
      Serial.print("  dist="); Serial.print(dist);
      Serial.print(" cm  ang="); Serial.print(ang, 1);
    } else {
      Serial.print("no lo veo");
    }
    unsigned long dt = millis() - t_contadores;
    if (dt > 0) {
      Serial.print("   camara "); Serial.print(nPaquetes * 1000UL / dt);
      Serial.print(" paq/s  brillos "); Serial.print(nBrillos);
    }
    Serial.print("   t="); Serial.print(segundos(), 1); Serial.println(" s");
    nPaquetes = nTirados = nLoops = 0;
    t_contadores = millis();
  }
}
