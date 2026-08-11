/* =====================================================================
   BUSCAR — CENTRAR — AVANZAR — ORBITAR — PATEAR AL ARCO AZUL
   IITA Salta — martes — 2026-07-28
   =====================================================================

     BUSCANDO    no ve la pelota           -> gira lento a pulsitos
     CENTRANDO   la ve de costado          -> gira a pulsitos HACIA ella
     AVANZANDO   la ve y centrada          -> va derecho hacia ella
     ORBITANDO   la tiene cerca (<25 cm)   -> da la vuelta ALREDEDOR de la
                                              pelota buscando el arco AZUL
     PATEA_ADEL  pelota alineada con arco  -> 1 s a maxima potencia
     PATEA_ATRAS despues de patear         -> retrocede y vuelve a buscar

   ---------------------------------------------------------------------
   LA ORBITA — de donde sale
   ---------------------------------------------------------------------
   No la invente: es la maniobra del delantero que gano el Nacional 2025,
   estado CENTRANDO_horario (delantero.ino:613-617). Manda las dos ruedas
   de adelante SUAVE para un lado y la TRASERA FUERTE para el otro, en
   relacion 1 : 1 : 3.

   Esa asimetria es la clave: si las tres fueran iguales el robot giraria
   sobre su propio eje y la pelota se le escaparia. Con la trasera
   empujando mucho mas, el robot describe un ARCO AMPLIO y la pelota le
   queda adentro de la curva. Por eso "orbita" en vez de "girar".

   Los numeros originales (60/60/180 x c=0.4 => 24/24/72) quedan por
   DEBAJO del piso de arranque de este robot hoy, asi que estan escalados
   manteniendo la relacion.

   ---------------------------------------------------------------------
   CUANDO PATEA — el criterio del 2025
   ---------------------------------------------------------------------
        abs(Yp - Yarco) <= TOL_ALINEADO        (delantero.ino:621)

   Yp es hacia donde esta la PELOTA y Yarco hacia donde esta el ARCO. Si
   los dos numeros son parecidos, es que estan en la MISMA DIRECCION
   vistos desde el robot: o sea, robot -> pelota -> arco alineados. Ahi
   empujar la pelota derecho la manda al arco.

   NO alcanza con "ver el arco": hay que verlo DETRAS de la pelota.

   ---------------------------------------------------------------------
   El arco AZUL es el byte 203 del paquete de la camara.
   Correr en el PISO, con espacio. Monitor serie a 19200.
   ===================================================================== */

// Mapeo MEDIDO en banco 2026-07-28 (robot DELANTERO)
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3


// ================= PERILLAS =================

const bool GIRO_INVERTIDO   = true;   // sentido del giro de busqueda/centrado
const bool ORBITA_INVERTIDA = true;  // <<< para que orbite al otro lado

// --- histeresis del centrado ---
const int TOL_ENTRA = 10;
const int TOL_SALE  = 5;

// --- distancias ---
const int XP_ORBITA = 25;   // mas cerca que esto -> empieza a orbitar
const int XP_SUELTA = 55;   // si se le aleja mas que esto, vuelve a avanzar
const int XP_MAX    = 150;  // arriba de esto no le creo (la camara recorta en 200)

// --- alineacion con el arco ---
const int TOL_ALINEADO = 12;   // |Yp - Yarco| menor a esto = alineado. Subilo si nunca patea.

// --- giro para BUSCAR ---
const int VEL_GIRO       = 80;
const int MS_PULSO_BUSC  = 60;
const int MS_ESPERA_BUSC = 380;

// --- giro para CENTRAR ---
const int VEL_CENT       = 78;
const int MS_PULSO_CENT  = 32;
const int MS_ESPERA_CENT = 320;

// --- avance ---
const int VEL_AVANCE = 55;

// --- ORBITA: relacion 1:1:3 del codigo 2025, escalada arriba del piso ---
const int VEL_ORB_FRENTE  = 72;    // las dos de adelante
const int VEL_ORB_TRASERA = 130;   // la trasera (mucho mas: es la que curva)
const unsigned long MS_ORBITA_MAX = 9000;   // si no encuentra el arco, se rinde

// --- patada ---
const int VEL_PATADA    = 240;
const int MS_PATADA     = 1000;
const int VEL_RETROCESO = 110;
const int MS_RETROCESO  = 700;

const unsigned long MS_GRACIA    = 300;
const unsigned long SIN_DATOS_MS = 1500;

// ============================================

int Xp = 0, Yp = 0, Xaz = 0, Yaz = 0;
int XpBueno = 0, YpBueno = 0;      // ultima posicion BUENA de la pelota
int YazBueno = 0;                  // ultima direccion BUENA del arco azul

unsigned long t_ultimaPelota  = 0;
unsigned long t_ultimoArcoAzul = 0;
unsigned long t_ultimoPaquete = 0;
unsigned long t_ultimoAviso   = 0;
unsigned long t_cicloPulso    = 0;
unsigned long t_entroEstado   = 0;

enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO, PATEA_ADEL, PATEA_ATRAS };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

bool avisadoSinCamara = false;


// ---------- motores ----------

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
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

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// ORBITA: la maniobra del delantero 2025 (delantero.ino:613-617).
// Adelante suave, trasera fuerte y al reves => arco amplio alrededor
// de la pelota, en vez de girar sobre el propio eje.
void orbitar(bool sentidoA) {
  int a = sentidoA ? 0 : 1;    // las dos de adelante
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE);  digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE);  digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, VEL_ORB_TRASERA); digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso)                    motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera))  parar();
  else                                                  t_cicloPulso = millis();
}


// ---------- camara ----------

void leerCamara() {
  while (Serial1.available() >= 9) {
    int h1 = Serial1.read();
    if (h1 != 201) continue;

    int xp  = Serial1.read();
    int yp  = Serial1.read();
    int h2  = Serial1.read();
    int xam = Serial1.read();
    int yam = Serial1.read();
    int h3  = Serial1.read();
    int xaz = Serial1.read();
    int yaz = Serial1.read();
    (void)xam; (void)yam;

    if (h2 == 202 && h3 == 203) {
      Xp = xp;  Yp = yp - 100;
      Xaz = xaz; Yaz = yaz - 100;
      t_ultimoPaquete = millis();

      // 200 y +-100 son los TOPES de recorte de la camara: casi siempre manchas.
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xaz > 0) && (Xaz <= XP_MAX) && (abs(Yaz) < 100)) {
        YazBueno = Yaz;
        t_ultimoArcoAzul = millis();
      }
    }
  }
}


const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO:    return "BUSCANDO";
    case CENTRANDO:   return "CENTRANDO";
    case AVANZANDO:   return "AVANZANDO";
    case ORBITANDO:   return "ORBITANDO";
    case PATEA_ADEL:  return "PATEANDO!";
    case PATEA_ATRAS: return "retrocede";
  }
  return "?";
}

void cambiarA(Estado nuevo) {
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
}


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
  Serial.print("patea si |Yp - Yarcoazul| <= "); Serial.println(TOL_ALINEADO);
  Serial.println("==============================================");
  Serial.println("Arranca en 3 segundos.");
  delay(3000);

  t_ultimoPaquete = millis();
  cambiarA(BUSCANDO);
}


void loop() {

  leerCamara();

  bool laVeo    = (millis() - t_ultimaPelota)   < MS_GRACIA;
  bool veoArco  = (millis() - t_ultimoArcoAzul) < MS_GRACIA;
  unsigned long enEstado = millis() - t_entroEstado;

  // ---------- la patada NO se interrumpe ----------
  if (estado == PATEA_ADEL) {
    avanzar(VEL_PATADA);
    if (enEstado >= (unsigned long)MS_PATADA) cambiarA(PATEA_ATRAS);
  }
  else if (estado == PATEA_ATRAS) {
    retroceder(VEL_RETROCESO);
    if (enEstado >= (unsigned long)MS_RETROCESO) cambiarA(BUSCANDO);
  }

  // ---------- ORBITANDO ----------
  else if (estado == ORBITANDO) {

    if (!laVeo) {                                   // se le escapo la pelota
      Serial.println("... perdi la pelota orbitando");
      cambiarA(BUSCANDO);
    }
    else if (XpBueno > XP_SUELTA) {                 // se le alejo: vuelve a ir
      cambiarA(AVANZANDO);
    }
    else if (veoArco && abs(YpBueno - YazBueno) <= TOL_ALINEADO) {
      Serial.print("*** ALINEADO con el arco azul  (Yp="); Serial.print(YpBueno);
      Serial.print("  Yarco="); Serial.print(YazBueno);
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (enEstado > MS_ORBITA_MAX) {            // se rinde
      Serial.println("... orbite 9 s y no encontre el arco azul alineado");
      cambiarA(BUSCANDO);
    }
    else {
      orbitar(!ORBITA_INVERTIDA);
    }
  }

  // ---------- el resto ----------
  else {
    if (!laVeo) {
      if (estado != BUSCANDO) cambiarA(BUSCANDO);
      rotarPulsado(!GIRO_INVERTIDO, VEL_GIRO, MS_PULSO_BUSC, MS_ESPERA_BUSC);
    }
    else if (XpBueno < XP_ORBITA) {
      Serial.print("*** llegue a "); Serial.print(XpBueno);
      Serial.println(" cm -> a orbitar buscando el arco azul");
      cambiarA(ORBITANDO);
    }
    else {
      int desvio = abs(YpBueno);
      if (estado == CENTRANDO) {
        if (desvio < TOL_SALE) cambiarA(AVANZANDO);
      } else {
        if (desvio > TOL_ENTRA)        cambiarA(CENTRANDO);
        else if (estado != AVANZANDO)  cambiarA(AVANZANDO);
      }

      if (estado == CENTRANDO) {
        bool haciaUnLado = (YpBueno > 0);
        if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
      } else {
        avanzar(VEL_AVANCE);
      }
    }
  }

  // ---------- avisos ----------
  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    Serial.print(">>> "); Serial.print(nombreEstado(estado));
    Serial.print("   Xp="); Serial.print(XpBueno);
    Serial.print(" Yp=");  Serial.print(YpBueno);
    Serial.print("  arco: ");
    if (veoArco) { Serial.print("Yaz="); Serial.println(YazBueno); }
    else         { Serial.println("no lo veo"); }
  }

  bool sinDatos = (millis() - t_ultimoPaquete > SIN_DATOS_MS);
  if (sinDatos && !avisadoSinCamara) {
    Serial.println("!!! NO LLEGAN DATOS DE LA CAMARA !!!");
    avisadoSinCamara = true;
  }
  if (!sinDatos) avisadoSinCamara = false;

  if (millis() - t_ultimoAviso > 2000) {
    t_ultimoAviso = millis();
    Serial.print("   ["); Serial.print(nombreEstado(estado));
    Serial.print("]  Xp="); Serial.print(XpBueno);
    Serial.print(" Yp="); Serial.print(YpBueno);
    Serial.print("  Yaz=");
    if (veoArco) Serial.print(YazBueno); else Serial.print("--");
    Serial.print("  dif=");
    if (veoArco) Serial.println(abs(YpBueno - YazBueno)); else Serial.println("--");
  }
}
