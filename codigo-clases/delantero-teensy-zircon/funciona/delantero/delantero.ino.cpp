# 1 "C:\\Users\\violl\\AppData\\Local\\Temp\\tmpz98ycas0"
#include <Arduino.h>
# 1 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
# 70 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3




const bool GIRO_INVERTIDO = true;
const bool ORBITA_INVERTIDA = true;


const int TOL_ENTRA = 10;
const int TOL_SALE = 5;


const int XP_ORBITA = 22;
const int XP_SUELTA = 55;
const int XP_MAX = 150;
# 117 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const int XARCO_MAX = 200;


const int VEL_GIRO = 80;
const int MS_PULSO_BUSC = 60;
const int MS_ESPERA_BUSC = 380;


const int VEL_CENT = 78;
const int MS_PULSO_CENT = 32;
const int MS_ESPERA_CENT = 320;


const int VEL_AVANCE = 55;
# 202 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const int VEL_ORB_FRENTE = 30;
const int VEL_ORB_IMPULSO = 99;
const int MS_ORB_IMPULSO = 300;
const int VEL_ORB_TRASERA = 48;





const unsigned long MS_ORBITA_MAX = 20000;


const int VEL_PATADA = 240;
const int MS_PATADA = 1000;
const int VEL_RETROCESO = 110;
const int MS_RETROCESO = 700;

const unsigned long MS_GRACIA = 300;
const unsigned long SIN_DATOS_MS = 1500;
# 257 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const float TOL_ANG_PELOTA = 10.0;
const float TOL_ANG_ALINEADO = 10.0;
# 288 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const bool ELEGIR_ARCO_AL_ENCENDER = true;
const unsigned long MS_MIRAR_ARCOS = 2000;
const int MUESTRAS_MINIMAS_ARCO = 3;
# 303 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const bool USAR_GIROSCOPO = true;
const bool PATEAR_AL_RUMBO0 = true;
const bool ORBITA_CAMINO_CORTO = false;



const float TOL_RUMBO = 12.0;
const unsigned long MS_APUNTAR_MAX = 6000;
# 320 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const bool SENTIDO_ORBITA_INVERTIDO = false;
const bool GIRO_RUMBO_INVERTIDO = false;
# 352 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
const bool LINEA_ACTIVA = true;
const int VEL_ESCAPE = 100;
const unsigned long MS_ESCAPE_EXTRA = 400;
# 368 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
int UMBRAL_LINEA[3] = { 620, 620, 620 };


const int PIN_VERSION_PLACA = 32;
# 408 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
int Xp = 0, Yp = 0;
int Xam = 0, Yam = 0;
int Xaz = 0, Yaz = 0;

int XpBueno = 0, YpBueno = 0;
int XamBueno = 0, YamBueno = 0;
int XazBueno = 0, YazBueno = 0;

unsigned long t_ultimaPelota = 0;
unsigned long t_ultimoAmarillo = 0;
unsigned long t_ultimoAzul = 0;
unsigned long t_ultimoPaquete = 0;
unsigned long t_ultimoAviso = 0;
unsigned long t_cicloPulso = 0;
unsigned long t_entroEstado = 0;



bool objetivoEsAmarillo = false;


Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
float rumboCero = 0;
float ultimoRumbo = 0;
int contadorCeros = 0;
const int CEROS_PARA_DARLO_POR_CAIDO = 10;


int pinLinea[3] = { A11, A13, A12 };
const char* versionPlaca = "?";
bool lineaHabilitada = false;
int mascaraLinea = 0;
unsigned long t_ultimaLinea = 0;

enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO,
              APUNTA_RUMBO0, PATEA_ADEL, PATEA_ATRAS, ESCAPA_LINEA };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

bool avisadoSinCamara = false;
# 457 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
unsigned long nPaquetes = 0, nTirados = 0, nLoops = 0;
unsigned long t_contadores = 0;
void parar();
void motoresRotando(bool sentidoA, int vel);
void avanzar(int vel);
void retroceder(int vel);
void orbitar(bool sentidoA, int velTrasera);
int leerLineas();
void escaparDeLinea(int m);
void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera);
void leerCamara();
float anguloDe(int X, int Y);
float diferencia(float objetivo, float actual);
int arcoX();
int arcoY();
unsigned long arcoT();
const char* arcoNombre();
float rumboActual();
bool giroscopoSano();
const char* nombreEstado(Estado e);
bool arrancarGiroscopo();
void elegirArcoMirando();
void cambiarA(Estado nuevo);
void setup();
bool sentidoParaOrbitar();
void loop();
#line 463 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
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
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}







void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera); digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}




int leerLineas() {
  int m = 0;
  for (int i = 0; i < 3; i++) {
    if (analogRead(pinLinea[i]) >= UMBRAL_LINEA[i]) m |= (1 << i);
  }
  return m;
}



void escaparDeLinea(int m) {

  int v[3] = { 0, 0, 0 };
  if (m & 1) { v[1] -= 1; v[2] += 1; }
  if (m & 2) { v[0] += 1; v[2] -= 1; }
  if (m & 4) { v[0] -= 1; v[1] += 1; }

  int pico = 0;
  for (int i = 0; i < 3; i++) if (abs(v[i]) > pico) pico = abs(v[i]);

  if (pico == 0) {



    parar();
    return;
  }

  int pwm[3];
  for (int i = 0; i < 3; i++) pwm[i] = (v[i] * VEL_ESCAPE) / pico;

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

void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso) motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera)) parar();
  else t_cicloPulso = millis();
}




void leerCamara() {
  while (Serial1.available() >= 9) {
    int h1 = Serial1.read();
    if (h1 != 201) { nTirados++; continue; }

    int xp = Serial1.read();
    int yp = Serial1.read();
    int h2 = Serial1.read();
    int xam = Serial1.read();
    int yam = Serial1.read();
    int h3 = Serial1.read();
    int xaz = Serial1.read();
    int yaz = Serial1.read();

    if (h2 == 202 && h3 == 203) {
      Xp = xp; Yp = yp - 100;
      Xam = xam; Yam = yam - 100;
      Xaz = xaz; Yaz = yaz - 100;
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
# 609 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
float anguloDe(int X, int Y) {
  if (X <= 0) return 0.0;
  return atan2((float)Y, (float)X) * 180.0 / PI;
}




float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0) d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}



int arcoX() { return objetivoEsAmarillo ? XamBueno : XazBueno; }
int arcoY() { return objetivoEsAmarillo ? YamBueno : YazBueno; }
unsigned long arcoT() { return objetivoEsAmarillo ? t_ultimoAmarillo : t_ultimoAzul; }
const char* arcoNombre() { return objetivoEsAmarillo ? "AMARILLO" : "AZUL"; }
# 640 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  if (evento.orientation.x == 0.0 && evento.orientation.y == 0.0
      && evento.orientation.z == 0.0) {
    if (contadorCeros < CEROS_PARA_DARLO_POR_CAIDO) contadorCeros++;
  } else {
    contadorCeros = 0;
  }
  ultimoRumbo = evento.orientation.x;
  return ultimoRumbo;
}

bool giroscopoSano() {
  return hayGiroscopo && contadorCeros < CEROS_PARA_DARLO_POR_CAIDO;
}


const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO: return "BUSCANDO";
    case CENTRANDO: return "CENTRANDO";
    case AVANZANDO: return "AVANZANDO";
    case ORBITANDO: return "ORBITANDO";
    case APUNTA_RUMBO0: return "al rumbo 0";
    case PATEA_ADEL: return "PATEANDO!";
    case PATEA_ATRAS: return "retrocede";
    case ESCAPA_LINEA: return "!LINEA!";
  }
  return "?";
}






bool arrancarGiroscopo() {
  if (!bno.begin()) return false;
  delay(700);
  bno.setExtCrystalUse(true);
  int buenas = 0;
  for (int i = 0; i < 20; i++) {
    sensors_event_t e;
    bno.getEvent(&e);
    if (e.orientation.x != 0.0 || e.orientation.y != 0.0 || e.orientation.z != 0.0) buenas++;
    delay(50);
  }
  if (buenas < 10) {
    Serial.print("   contesta pero da ceros ("); Serial.print(buenas);
    Serial.println("/20 lecturas utiles) — no lo doy por bueno");
    return false;
  }
  contadorCeros = 0;
  return true;
}



void elegirArcoMirando() {
  long nAm = 0, nAz = 0;
  double sumAm = 0, sumAz = 0;
  unsigned long t0 = millis();

  Serial.print("Mirando "); Serial.print(MS_MIRAR_ARCOS / 1000);
  Serial.println(" s para ver a que arco apunto. NO LO MUEVAS.");





  unsigned long visto_am = 0, visto_az = 0;
  while (millis() - t0 < MS_MIRAR_ARCOS) {
    leerCamara();
    if (t_ultimoAmarillo != visto_am) {
      visto_am = t_ultimoAmarillo;
      nAm++; sumAm += fabs(anguloDe(XamBueno, YamBueno));
    }
    if (t_ultimoAzul != visto_az) {
      visto_az = t_ultimoAzul;
      nAz++; sumAz += fabs(anguloDe(XazBueno, YazBueno));
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
  if (sirveAm && !sirveAz) objetivoEsAmarillo = true;
  else if (sirveAz && !sirveAm) objetivoEsAmarillo = false;
  else objetivoEsAmarillo = (medAm < medAz);

  Serial.print("   *** ATACO EL ARCO "); Serial.print(arcoNombre()); Serial.println(" ***");
}

void cambiarA(Estado nuevo) {
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso = millis();
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
  Serial.print("patea si pelota a menos de "); Serial.print(TOL_ANG_PELOTA, 0);
  Serial.print(" grados del frente Y el arco a menos de "); Serial.print(TOL_ANG_ALINEADO, 0);
  Serial.println(" grados de la pelota");
  Serial.print("orbita: impulso "); Serial.print(VEL_ORB_IMPULSO);
  Serial.print(" x "); Serial.print(MS_ORB_IMPULSO);
  Serial.print(" ms  ->  crucero "); Serial.print(VEL_ORB_TRASERA);
  Serial.print("   (max "); Serial.print(MS_ORBITA_MAX / 1000); Serial.println(" s)");
  Serial.println("==============================================");


  pinMode(PIN_VERSION_PLACA, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION_PLACA) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8; pinLinea[1] = A9; pinLinea[2] = A12;
  }
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   sensores de linea en pines ");
  Serial.print(pinLinea[0]); Serial.print(", ");
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.println(pinLinea[2]);

  if (LINEA_ACTIVA) {




    Serial.print("Linea: sensores leen ");
    Serial.print(analogRead(pinLinea[0])); Serial.print(" / ");
    Serial.print(analogRead(pinLinea[1])); Serial.print(" / ");
    Serial.print(analogRead(pinLinea[2]));
    Serial.print("   umbrales "); Serial.print(UMBRAL_LINEA[0]);
    Serial.print(" / "); Serial.print(UMBRAL_LINEA[1]);
    Serial.print(" / "); Serial.println(UMBRAL_LINEA[2]);

    int m = leerLineas();
    if (m != 0) {
      lineaHabilitada = false;
      Serial.println("!!! YA LEE BLANCO ESTANDO EN EL VERDE -> el umbral esta mal.");
      Serial.println("!!! ESCAPE DE LINEA DESACTIVADO. Corre pruebas/sensores-de-linea/");
    } else {
      lineaHabilitada = true;
      Serial.println("Linea: OK, escape ACTIVADO (anula todo lo demas).");
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
    } else {
      Serial.println("NO CONTESTA. Sigo sin el, como hasta ayer.");
    }
  } else {
    Serial.println("Giroscopo: apagado por configuracion.");
  }


  if (ELEGIR_ARCO_AL_ENCENDER) {
    elegirArcoMirando();
  } else {
    Serial.print("Arco objetivo: "); Serial.print(arcoNombre());
    Serial.println("  (fijo por configuracion)");
  }

  Serial.println("==============================================");
  Serial.println("Arranca en 3 segundos.");
  delay(3000);

  t_ultimoPaquete = millis();
  t_contadores = millis();
  cambiarA(BUSCANDO);
}
# 859 "C:/Users/violl/iita-martes-delantero/codigo-clases/delantero-teensy-zircon/funciona/delantero/delantero.ino"
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


void loop() {

  nLoops++;
  leerCamara();




  if (lineaHabilitada) {
    int m = leerLineas();
    if (m != 0) {
      t_ultimaLinea = millis();
      mascaraLinea = m;
      if (estado != ESCAPA_LINEA) {
        Serial.print("!!! LINEA BLANCA (sensores");
        for (int i = 0; i < 3; i++) if (m & (1 << i)) { Serial.print(" "); Serial.print(i + 1); }
        Serial.print(") estando en "); Serial.print(nombreEstado(estado));
        Serial.println(" -> ESCAPO");
        cambiarA(ESCAPA_LINEA);
      }
    }
  }

  bool laVeo = (millis() - t_ultimaPelota) < MS_GRACIA;
  bool veoArco = (millis() - arcoT()) < MS_GRACIA;
  unsigned long enEstado = millis() - t_entroEstado;




  float angPelota = anguloDe(XpBueno, YpBueno);
  float angArco = anguloDe(arcoX(), arcoY());
  bool pelotaAdelante = (fabs(angPelota) <= TOL_ANG_PELOTA);
  bool arcoAlineado = (fabs(diferencia(angArco, angPelota)) <= TOL_ANG_ALINEADO);


  if (estado == ESCAPA_LINEA) {
    if (millis() - t_ultimaLinea > MS_ESCAPE_EXTRA) {
      Serial.println("... ya me despegue de la linea");
      cambiarA(BUSCANDO);
    } else {
      escaparDeLinea(mascaraLinea);
    }
  }


  else if (estado == PATEA_ADEL) {
    avanzar(VEL_PATADA);
    if (enEstado >= (unsigned long)MS_PATADA) cambiarA(PATEA_ATRAS);
  }
  else if (estado == PATEA_ATRAS) {
    retroceder(VEL_RETROCESO);
    if (enEstado >= (unsigned long)MS_RETROCESO) cambiarA(BUSCANDO);
  }


  else if (estado == ORBITANDO) {

    if (!laVeo) {
      Serial.println("... perdi la pelota orbitando");
      cambiarA(BUSCANDO);
    }
    else if (XpBueno > XP_SUELTA) {
      cambiarA(AVANZANDO);
    }
    else if (veoArco && pelotaAdelante && arcoAlineado) {
      Serial.print("*** ALINEADO con el arco "); Serial.print(arcoNombre());
      Serial.print("  (pelota a "); Serial.print(angPelota, 1);
      Serial.print(" grados, arco a "); Serial.print(angArco, 1);
      Serial.print(", separados "); Serial.print(fabs(diferencia(angArco, angPelota)), 1);
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (enEstado > MS_ORBITA_MAX) {
      Serial.print("... orbite "); Serial.print(MS_ORBITA_MAX / 1000);
      Serial.print(" s y no encontre el arco "); Serial.println(arcoNombre());



      if (PATEAR_AL_RUMBO0 && giroscopoSano()) {
        Serial.print("    -> voy a apuntar al rumbo de arranque (");
        Serial.print(rumboCero, 1); Serial.println(" grados) y patear ahi");
        cambiarA(APUNTA_RUMBO0);
      } else {
        cambiarA(BUSCANDO);
      }
    }
    else {




      bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
      orbitar(sentidoParaOrbitar(), enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
    }
  }





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
    if (!laVeo) {
      if (estado != BUSCANDO) cambiarA(BUSCANDO);
      rotarPulsado(!GIRO_INVERTIDO, VEL_GIRO, MS_PULSO_BUSC, MS_ESPERA_BUSC);
    }
    else if (XpBueno < XP_ORBITA) {
      Serial.print("*** llegue a "); Serial.print(XpBueno);
      Serial.print(" cm -> a orbitar buscando el arco ");
      Serial.println(arcoNombre());
      cambiarA(ORBITANDO);
    }
    else {
      int desvio = abs(YpBueno);
      if (estado == CENTRANDO) {
        if (desvio < TOL_SALE) cambiarA(AVANZANDO);
      } else {
        if (desvio > TOL_ENTRA) cambiarA(CENTRANDO);
        else if (estado != AVANZANDO) cambiarA(AVANZANDO);
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


  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    Serial.print(">>> "); Serial.print(nombreEstado(estado));
    Serial.print("   Xp="); Serial.print(XpBueno);
    Serial.print(" Yp="); Serial.print(YpBueno);
    Serial.print(" (a "); Serial.print(angPelota, 1); Serial.print(" grados)");
    Serial.print("  arco "); Serial.print(arcoNombre()); Serial.print(": ");
    if (veoArco) { Serial.print("a "); Serial.print(angArco, 1); Serial.println(" grados"); }
    else { Serial.println("no lo veo"); }
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
    Serial.print("  angPelota="); Serial.print(angPelota, 1);
    Serial.print("  angArco=");
    if (veoArco) Serial.print(angArco, 1); else Serial.print("--");
    Serial.print("  separacion=");
    if (veoArco) Serial.print(fabs(diferencia(angArco, angPelota)), 1); else Serial.print("--");
    if (giroscopoSano()) { Serial.print("  rumbo="); Serial.print(ultimoRumbo, 0); }
    Serial.println();

    unsigned long dt = millis() - t_contadores;
    if (dt > 0) {
      Serial.print("        camara: "); Serial.print(nPaquetes * 1000UL / dt);
      Serial.print(" paq/s   "); Serial.print(nTirados * 1000UL / dt);
      Serial.print(" bytes tirados/s   loop: "); Serial.print(nLoops * 1000UL / dt);
      Serial.println(" /s");
    }
    nPaquetes = nTirados = nLoops = 0;
    t_contadores = millis();
  }
}