/* =====================================================================
   MONITOR DEL ROBOT ARQUERO — una pantalla que se refresca en su lugar
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-22
   =====================================================================

   QUE ES
   Un tablero. Siempre las mismas lineas, en el mismo lugar, y los numeros
   cambiando adentro. NO va imprimiendo dato tras dato hacia abajo: la
   pantalla se queda quieta y solo cambian los valores, como el tablero de
   un auto.

        === MONITOR DEL ROBOT ARQUERO ==================== 123 s ===

        GIROSCOPIO                                       [ SANO ]
          yaw     ->    123.4 grados
          pitch   ->     -2.1 grados
          ...
        SENSORES DE LUZ
          frente     ->    500   BLANCO
          ...
        OPCIONES  1 reiniciar giro   2 reiniciar Teensy   ...

   ---------------------------------------------------------------------
   COMO HACE PARA NO IRSE HACIA ABAJO
   ---------------------------------------------------------------------
   Manda unos codigos especiales que la consola entiende como ordenes en
   vez de texto (se llaman codigos ANSI, y empiezan con el caracter 27):

        ESC [ H     ->  "volve arriba de todo con el cursor"
        ESC [ K     ->  "borra lo que quede a la derecha en esta linea"
        ESC [ 2J    ->  "borra toda la pantalla"

   Cada vuelta: vuelve arriba y reescribe las 25 lineas encima de las
   viejas. Como cada linea se borra hasta el final, nunca quedan restos de
   un numero mas largo de la vuelta anterior.

   🚨 Windows NO interpreta esos codigos hasta que el programa de la PC se
   lo pide. Por eso `mirar.py` ahora lo pide al abrirse. Si aun asi se ve
   basura con corchetes, la tecla `l` cambia a lista simple.

   ⚠️ La ventana tiene que tener al menos 26 renglones. Si se ve cortada o
   temblando, agrandala un poco.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup(). La UNICA excepcion es la tecla `m`
   (el sacudon), que hay que apretar dos veces a proposito y avisa antes.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA: el giroscopio y la camara se
   alimentan de ahi, no del USB. Y por la trampa del 21/09, prender la
   bateria con el USB ya puesto NO reinicia el Teensy. Entonces:

        prender la bateria  ->  enchufar el USB  ->  abrir mirar.bat

   Si lo hacen al reves van a ver el giroscopio AUSENTE aunque este
   perfecto. La tecla `1` lo arregla sin desenchufar nada.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

// Motores: se apagan en setup() y solo los toca el sacudon (tecla m).
#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

// Sensores de linea. Los pines salen del programa de juego.
#define LINEA_ADELANTE  A12
#define LINEA_ATRAS_IZQ A13
#define LINEA_ATRAS_DER A11

const unsigned long BAUDIOS        = 19200;
const unsigned long BAUDIOS_CAMARA = 19200;

// Medir rapido (para no perderse una caida corta) y DIBUJAR mas despacio
// (para que los numeros se puedan leer sin que tiemblen).
const unsigned long MS_MEDIDA   = 100;
const unsigned long MS_DIBUJO   = 200;

// Del programa de juego, para que lo que se ve aca sea lo mismo que ve el
// robot cuando juega.
const float CAMARA_POR_CM = 2.87;    // unidades de camara por cm real
int umbralBlanco = 425;              // medido en cancha el 2026-09-15


// ------------------------------------------------------- el giroscopio

// Dos objetos, uno por direccion posible: el BNO055 queda en 0x28 o en
// 0x29 segun como este una patita de la placa.
Adafruit_BNO055 bno28 = Adafruit_BNO055(55, 0x28);
Adafruit_BNO055 bno29 = Adafruit_BNO055(55, 0x29);
Adafruit_BNO055 *bno  = &bno28;
byte direccion = 0x28;

// Registros del BNO055, verificados contra Adafruit_BNO055.h.
const byte REG_CHIP_ID    = 0x00;   // tiene que dar 0xA0
const byte REG_PAGE_ID    = 0x07;   // si no es 0, todo lo demas se lee mal
const byte REG_TEMP       = 0x34;
const byte REG_CALIB_STAT = 0x35;
const byte REG_ST_RESULT  = 0x36;
const byte REG_SYS_STAT   = 0x39;
const byte REG_SYS_ERR    = 0x3A;
const byte REG_OPR_MODE   = 0x3D;
const byte REG_PWR_MODE   = 0x3E;

// Perillas que se mueven con el teclado, para ir probando hipotesis.
adafruit_bno055_opmode_t modoPedido = OPERATION_MODE_NDOF;  // el del juego
bool     usarCristal   = true;
uint32_t velocidadBus  = 400000;
bool     revivirSolo   = false;     // arranca APAGADO, ver abajo
bool     pausado       = false;
bool     pantallaAnsi  = true;      // tecla l: pasar a lista simple
float    ceroManual    = -1;

// ⚠️ Por que `revivirSolo` arranca APAGADO:
// si el programa lo revive apenas se cae, nunca vamos a saber si el sensor
// vuelve SOLO. Y esa es justo la pregunta. Primero se mira, despues se
// automatiza.


// ------------------------------------------------------- lo que se mide

enum Salud { AUSENTE, MUDO, SANO };
Salud salud = AUSENTE;
Salud saludAnterior = AUSENTE;

float yaw = 0, pitch = 0, roll = 0;
byte  calSis = 0, calGir = 0, calAce = 0, calMag = 0;
byte  modoReal = 0xFF, sysStat = 0xFF, sysErr = 0xFF;
int   temperatura = 0;

unsigned long lecturasOk = 0, lecturasMalas = 0;
unsigned long t_ultimoDato = 0;

int luzFrente = 0, luzIzq = 0, luzDer = 0;

// Camara
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0, Xam = 0, Yam = 0, Xaz = 0, Yaz = 0;
unsigned long t_ultimoPaquete = 0;
unsigned long paquetesTotales = 0;
float paquetesPorSegundo = 0;

int pwmMotores = 0;

// Relojes del lazo. Van declarados ACA arriba, y no junto a loop(), porque
// setup() y leerTeclado() tambien los tocan: en C++ una variable no existe
// para el codigo que esta escrito mas arriba que ella.
unsigned long t_medida = 0, t_dibujo = 0, t_cuenta = 0;
unsigned long paquetesEnLaVuelta = 0;

// Frenos del revivir automatico (tecla a).
const unsigned long MS_ENTRE_INTENTOS     = 6000;
const int           MAX_INTENTOS_SEGUIDOS = 5;
unsigned long t_ultimoIntento  = 0;
int           intentosSeguidos = 0;


// ------------------------------------------------------- historial

// Lo mas valioso va a ser QUE estaba pasando en el momento exacto de la
// caida. Si no se guarda ahi mismo, se pierde.
struct Caida {
  unsigned long ms;
  unsigned long duracion;    // 0 = todavia no volvio
  Salud         tipo;
  byte          modo;        // 🎯 el dato clave: ¿quedo en CONFIG?
  byte          sysStat, sysErr;
  byte          calSis;
  int           pwm;         // ¿motores andando en ese momento?
};
const int MAX_CAIDAS = 12;
Caida caidas[MAX_CAIDAS];
int   nCaidas = 0;
int   caidasTotales = 0;

// A cual de las caidas guardadas hay que escribirle la duracion cuando el
// sensor vuelva. -1 = no hay ninguna abierta, o la que esta abierta no
// entro en la tabla porque ya estaba llena. Sin esto, con la tabla llena
// la duracion se le escribia a la caida equivocada.
int   caidaAbierta = -1;

// El PWM que tenian los motores JUSTO cuando se cayo. Va aparte de
// pwmMotores (que es el de ahora) porque lo primero que hace el sacudon al
// detectar la caida es apagarlos: si se leyera el de ahora, la caida
// quedaria anotada como "motores en 0" y se perderia el dato clave.
int   pwmAlCaer = 0;

// Los dos ultimos avisos, que se muestran al pie del tablero. Como la
// pantalla no scrollea, si no se guardan no quedan en ningun lado.
char aviso1[75] = "";
char aviso2[75] = "";

void avisar(const char *texto) {
  strncpy(aviso2, aviso1, sizeof(aviso2) - 1); aviso2[sizeof(aviso2) - 1] = 0;
  strncpy(aviso1, texto,  sizeof(aviso1) - 1); aviso1[sizeof(aviso1) - 1] = 0;
}


// =====================================================================
//  FORMATO DE NUMEROS
// =====================================================================
//
// Se escriben a mano en vez de usar printf con %f porque asi se controla
// el ANCHO EXACTO. En un tablero que se refresca solo, que un numero pase
// de 3 a 4 digitos y corra toda la linea hace que sea imposible de leer.
// Aca cada numero ocupa siempre el mismo lugar.

void textoFloat(char *buf, int ancho, float v, int dec) {
  bool neg = v < 0;
  if (neg) v = -v;
  long escala = 1;
  for (int i = 0; i < dec; i++) escala *= 10;
  long total = (long)(v * escala + 0.5);
  long ent = total / escala;
  long fra = total % escala;

  char tmp[24];
  int n = 0;
  for (int i = 0; i < dec; i++) { tmp[n++] = '0' + (char)(fra % 10); fra /= 10; }
  if (dec > 0) tmp[n++] = '.';
  if (ent == 0) tmp[n++] = '0';
  while (ent > 0 && n < 22) { tmp[n++] = '0' + (char)(ent % 10); ent /= 10; }
  if (neg && n < 23) tmp[n++] = '-';

  int k = 0;
  for (int i = n; i < ancho; i++) buf[k++] = ' ';
  for (int i = n - 1; i >= 0; i--) buf[k++] = tmp[i];
  buf[k] = 0;
}

void textoEntero(char *buf, int ancho, long v) {
  textoFloat(buf, ancho, (float)v, 0);
}


// =====================================================================
//  LECTURA CRUDA DEL CHIP, SIN LIBRERIA
// =====================================================================
//
// Se le habla al chip directamente. Sirve por dos motivos: saca la
// libreria del medio como sospechosa, y sobre todo sigue funcionando
// cuando la libreria ya perdio el hilo — que es exactamente lo que pasa
// si el chip se reinicia solo: la libreria cree que esta en NDOF y el
// chip volvio a CONFIG.

bool leerReg(byte reg, byte *valor) {
  Wire.beginTransmission(direccion);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)direccion, 1) != 1) return false;
  *valor = Wire.read();
  return true;
}

bool escribirReg(byte reg, byte valor) {
  Wire.beginTransmission(direccion);
  Wire.write(reg);
  Wire.write(valor);
  return Wire.endTransmission() == 0;
}

const char* nombreModo(byte m) {
  switch (m) {
    case 0x00: return "CONFIG";     // 🎯 el chip se reinicio solo
    case 0x01: return "ACCONLY";
    case 0x02: return "MAGONLY";
    case 0x03: return "GYRONLY";
    case 0x04: return "ACCMAG";
    case 0x05: return "ACCGYRO";
    case 0x06: return "MAGGYRO";
    case 0x07: return "AMG";
    case 0x08: return "IMUPLUS";    // sin brujula
    case 0x09: return "COMPASS";
    case 0x0A: return "M4G";
    case 0x0B: return "NDOF_FMC";
    case 0x0C: return "NDOF";       // el que usa el programa de juego
    default:   return "?";
  }
}

const char* nombreSysStat(byte s) {
  switch (s) {
    case 0: return "en reposo";
    case 1: return "ERROR DE SISTEMA";
    case 2: return "iniciando perifericos";
    case 3: return "iniciando el sistema";
    case 4: return "corriendo el autotest";
    case 5: return "fusionando, todo bien";
    case 6: return "andando SIN fusion";
    default: return "sin dato";
  }
}

const char* nombreSysErr(byte e) {
  switch (e) {
    case 0:  return "sin error";
    case 1:  return "fallo al iniciar un periferico";
    case 2:  return "fallo al iniciar el sistema";
    case 3:  return "el autotest dio mal";
    case 4:  return "valor fuera de rango";
    case 5:  return "direccion fuera de rango";
    case 6:  return "error de escritura";
    case 7:  return "ese modo no admite bajo consumo";
    case 8:  return "consumo del acelerometro no disponible";
    case 9:  return "error de configuracion de la fusion";
    case 10: return "error de configuracion de un sensor";
    default: return "sin dato";
  }
}


// =====================================================================
//  MEDIR
// =====================================================================

void medirGiroscopo() {
  byte v;

  // Primero lo mas barato y lo mas decisivo: ¿hay alguien en el bus?
  if (!leerReg(REG_CHIP_ID, &v)) {
    salud = AUSENTE;
    lecturasMalas++;
    modoReal = 0xFF; sysStat = 0xFF; sysErr = 0xFF;
    return;
  }

  // Lo de adentro del chip, crudo. Si una lectura suelta falla se deja el
  // valor anterior: perder una lectura no es lo mismo que un chip caido.
  if (leerReg(REG_OPR_MODE, &v)) modoReal = v & 0x0F;
  if (leerReg(REG_SYS_STAT, &v)) sysStat  = v;
  if (leerReg(REG_SYS_ERR,  &v)) sysErr   = v;
  if (leerReg(REG_TEMP,     &v)) temperatura = (int8_t)v;
  if (leerReg(REG_CALIB_STAT, &v)) {
    calSis = (v >> 6) & 0x03;
    calGir = (v >> 4) & 0x03;
    calAce = (v >> 2) & 0x03;
    calMag =  v       & 0x03;
  }

  // Los angulos por la libreria, igual que el programa de juego.
  sensors_event_t e;
  bno->getEvent(&e);

  // Los tres en cero EXACTO es la firma de "no puedo leer el chip" de la
  // libreria Adafruit. Que los tres angulos den 0.000 de verdad, al mismo
  // tiempo, no pasa.
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    salud = MUDO;
    lecturasMalas++;
    return;
  }

  yaw   = e.orientation.x;
  pitch = e.orientation.y;
  roll  = e.orientation.z;
  salud = SANO;
  lecturasOk++;
  t_ultimoDato = millis();
}

void medirLuz() {
  luzFrente = analogRead(LINEA_ADELANTE);
  luzIzq    = analogRead(LINEA_ATRAS_IZQ);
  luzDer    = analogRead(LINEA_ATRAS_DER);
}

// El mismo parser del programa de juego: 9 bytes, con tres marcas.
//   [201, Xp, Yp+100, 202, Xam, Yam+100, 203, Xaz, Yaz+100]
//   201 = pelota    202 = arco amarillo    203 = arco azul
void medirCamara() {
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
      Xp  = paquete[1]; Yp  = paquete[2] - 100;
      Xam = paquete[4]; Yam = paquete[5] - 100;
      Xaz = paquete[7]; Yaz = paquete[8] - 100;
      t_ultimoPaquete = millis();
      paquetesTotales++;
    }
  }
}


// =====================================================================
//  ANOTAR LAS CAIDAS
// =====================================================================

void anotarCaida(unsigned long ahora) {
  caidasTotales++;

  char t[75];
  char n[8], s[10];
  textoEntero(n, 0, caidasTotales);
  textoFloat(s, 0, ahora / 1000.0, 1);
  if (salud == AUSENTE) {
    snprintf(t, sizeof(t), "CAIDA %s a los %s s: AUSENTE."
                           " Revisar VIN GND SDA SCL", n, s);
  } else if (modoReal == 0x00) {
    snprintf(t, sizeof(t), "CAIDA %s a los %s s: MUDO y en CONFIG"
                           " -> EL CHIP SE REINICIO SOLO", n, s);
  } else {
    snprintf(t, sizeof(t), "CAIDA %s a los %s s: MUDO pero sigue en %s"
                           " -> NO se reinicio", n, s, nombreModo(modoReal));
  }
  avisar(t);

  if (pwmAlCaer > 0) {
    char m[75];
    char q[8]; textoEntero(q, 0, pwmAlCaer);
    snprintf(m, sizeof(m), "   ... y los motores estaban andando a PWM %s", q);
    avisar(m);
  }

  if (nCaidas < MAX_CAIDAS) {
    Caida &c = caidas[nCaidas];
    c.ms = ahora; c.duracion = 0; c.tipo = salud;
    c.modo = modoReal; c.sysStat = sysStat; c.sysErr = sysErr;
    c.calSis = calSis; c.pwm = pwmAlCaer;
    caidaAbierta = nCaidas;
    nCaidas++;
  } else {
    caidaAbierta = -1;        // esta caida no entro en la tabla
  }
  pwmAlCaer = 0;              // se consumio: no contamina las siguientes
}

void anotarVuelta(unsigned long ahora) {
  unsigned long dur = 0;
  if (caidaAbierta >= 0 && caidaAbierta < nCaidas) {
    dur = ahora - caidas[caidaAbierta].ms;
    caidas[caidaAbierta].duracion = dur;
    caidaAbierta = -1;
  }
  char t[75], s[10];
  textoFloat(s, 0, dur / 1000.0, 1);
  if (revivirSolo)
    snprintf(t, sizeof(t), "VOLVIO despues de %s s (lo revivio el programa)", s);
  else
    snprintf(t, sizeof(t), "VOLVIO despues de %s s SOLO, sin que nadie lo tocara", s);
  avisar(t);
}


// =====================================================================
//  REVIVIR EL GIROSCOPIO
// =====================================================================
//
// Mismo procedimiento que `cuadrado-giroscopo`, y el orden importa: se
// reinicia el BUS antes que el chip, porque si lo que quedo trabado es el
// bus, reiniciar el chip solo no alcanza.
//
// Y no se da por bueno con una lectura: el sintoma de este sensor es
// justamente contestar "si, existo" y devolver ceros. Con una sola lectura
// no se distingue de uno sano. Por eso se piden 20 y tienen que servir 10.

bool revivir() {
  avisar("reiniciando el giroscopio...");

  Wire.end();
  delay(50);
  Wire.begin();
  Wire.setClock(velocidadBus);
  delay(50);

  // 🚨 POR QUE NO SE LLAMA A begin() DE ENTRADA
  //
  // Adafruit_BNO055::begin() resetea el chip y despues espera a que vuelva
  // asi (linea 40 de Adafruit_BNO055.cpp):
  //
  //      while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) { delay(10); }
  //
  // Ese lazo NO TIENE SALIDA. Si el chip no vuelve, el programa se queda
  // ahi para siempre: la pantalla se congela y ni el teclado responde. Y
  // justo esta herramienta se usa cuando el sensor anda mal, que es cuando
  // eso puede pasar.
  //
  // Asi que primero hacemos el reset NOSOTROS, con reloj:
  //   1. ¿contesta y dice que es un BNO055?
  //   2. se lo resetea y se espera a que vuelva, con tope de 1 segundo.
  // Recien si las dos dan bien se llama a begin(), que entonces encuentra
  // el chip despierto y su lazo sale en la primera vuelta.

  byte v;
  if (!leerReg(REG_CHIP_ID, &v) || v != 0xA0) {
    avisar("NO contesta en el bus. Eso es electrico: VIN GND SDA SCL");
    return false;
  }

  escribirReg(0x3F, 0x20);              // SYS_TRIGGER: resetear el chip
  delay(30);
  bool volvio = false;
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    if (leerReg(REG_CHIP_ID, &v) && v == 0xA0) { volvio = true; break; }
    delay(10);
  }
  if (!volvio) {
    avisar("se resetea pero NO vuelve en 1 s: el chip se cuelga al arrancar");
    return false;
  }
  delay(50);

  if (!bno->begin(modoPedido)) {
    avisar("NO se pudo levantar. Proba la tecla 9 (escanear los buses).");
    return false;
  }

  // ⚠️ El setClock va DESPUES de begin(), no antes: adentro de begin() la
  // libreria hace Wire.begin() de nuevo, y eso devuelve el bus a 100 kHz.
  // Puesto antes, la tecla 7 no cambiaba nada.
  Wire.setClock(velocidadBus);

  delay(700);                  // el BNO tarda en arrancar la fusion
  if (usarCristal) bno->setExtCrystalUse(true);

  int buenas = 0;
  for (int i = 0; i < 20; i++) {
    sensors_event_t e;
    bno->getEvent(&e);
    if (e.orientation.x != 0.0 || e.orientation.y != 0.0
        || e.orientation.z != 0.0) buenas++;
    delay(50);
  }

  char t[75], n[6];
  textoEntero(n, 0, buenas);
  if (buenas < 10) {
    snprintf(t, sizeof(t), "NO lo doy por bueno: contesta pero da ceros (%s/20)", n);
    avisar(t);
    return false;
  }
  snprintf(t, sizeof(t), "LEVANTADO (%s/20 lecturas utiles)", n);
  avisar(t);
  t_ultimoDato = millis();
  return true;
}


// =====================================================================
//  LA PANTALLA
// =====================================================================
//
// El truco esta en irArriba() + finDeLinea(): se vuelve al principio y se
// reescriben las mismas lineas encima de las viejas. Como cada una se
// borra hasta el final, nunca queda la cola de un numero mas largo.
// No se borra la pantalla entera cada vuelta a proposito: eso haria que
// parpadee.

void irArriba()     { Serial.print("\033[H"); }
void limpiarTodo()  { Serial.print("\033[2J\033[H"); }
void finDeLinea()   { Serial.print("\033[K\r\n"); }
void ocultarCursor(){ Serial.print("\033[?25l"); }

// Cuantas lineas ocupa el tablero. Si se agrega una, actualizar el
// comentario del encabezado que dice cuantos renglones hace falta.
const int LINEAS_TABLERO = 25;

void dibujarTablero() {
  char b[24];
  irArriba();

  // ---- 1: titulo y reloj -------------------------------------------
  Serial.print("=== MONITOR DEL ROBOT ARQUERO ");
  Serial.print("=============================");
  textoFloat(b, 8, millis() / 1000.0, 0);
  Serial.print(b); Serial.print(" s ===");
  finDeLinea();
  finDeLinea();

  // ---- 3-9: giroscopio ---------------------------------------------
  // ANINGUNA linea del tablero puede pasar de 78 columnas. Si se pasa, la
  // consola la parte en dos renglones, el "volve arriba" queda apuntando
  // al segundo, y la pantalla se va yendo hacia abajo sola: justo lo que
  // este programa existe para evitar.
  Serial.print("GIROSCOPIO");
  for (int i = 0; i < 20; i++) Serial.write(' ');
  if (salud == SANO)         Serial.print("[ SANO ]");
  else if (salud == MUDO)    Serial.print("[ MUDO: contesta y da ceros ]");
  else                       Serial.print("[ AUSENTE: no contesta ]");
  finDeLinea();

  if (salud == SANO) {
    textoFloat(b, 9, yaw, 1);
    Serial.print("  yaw     -> "); Serial.print(b); Serial.print(" grados");
    if (ceroManual >= 0) {
      float d = yaw - ceroManual;
      while (d > 180.0)  d -= 360.0;
      while (d <= -180.0) d += 360.0;
      char c[24];
      textoFloat(c, 7, d, 1);
      Serial.print("     cero puesto en ");
      textoFloat(b, 0, ceroManual, 1); Serial.print(b);
      Serial.print(" -> torcido "); Serial.print(c);
    }
    finDeLinea();
    textoFloat(b, 9, pitch, 1);
    Serial.print("  pitch   -> "); Serial.print(b); Serial.print(" grados");
    finDeLinea();
    textoFloat(b, 9, roll, 1);
    Serial.print("  roll    -> "); Serial.print(b); Serial.print(" grados");
    finDeLinea();
  } else {
    Serial.print("  yaw     ->        --");                    finDeLinea();
    Serial.print("  pitch   ->        --");                    finDeLinea();
    Serial.print("  roll    ->        --");                    finDeLinea();
  }

  Serial.print("  calibracion  sistema "); Serial.print(calSis);
  Serial.print("   giro ");                Serial.print(calGir);
  Serial.print("   acel ");                Serial.print(calAce);
  Serial.print("   brujula ");             Serial.print(calMag);
  if (calSis == 0) Serial.print("   (con 0 el rumbo no vale)");
  finDeLinea();

  Serial.print("  modo "); Serial.print(nombreModo(modoReal));
  if (modoReal == 0x00) Serial.print(" <- SE REINICIO SOLO");
  Serial.print("    estado: "); Serial.print(nombreSysStat(sysStat));
  finDeLinea();

  Serial.print("  error: ");   Serial.print(nombreSysErr(sysErr));
  finDeLinea();

  Serial.print("  ");           Serial.print(temperatura); Serial.print(" C");
  Serial.print("   bus ");      Serial.print(velocidadBus / 1000);
  Serial.print(" kHz en 0x");   Serial.print(direccion, HEX);
  Serial.print("   lecturas "); Serial.print(lecturasOk);
  Serial.print(" ok / ");       Serial.print(lecturasMalas);
  Serial.print(" malas");
  finDeLinea();

  Serial.print("  CAIDAS ");    Serial.print(caidasTotales);
  if (revivirSolo) Serial.print("   (lo revivo solo)");
  if (pausado)     Serial.print("   PAUSADO");
  finDeLinea();
  finDeLinea();

  // ---- 11-14: sensores de luz --------------------------------------
  Serial.print("SENSORES DE LUZ");
  for (int i = 0; i < 25; i++) Serial.write(' ');
  Serial.print("es BLANCO si pasa de "); Serial.print(umbralBlanco);
  finDeLinea();

  const char *nombres[3] = { "  frente    -> ", "  atras izq -> ",
                             "  atras der -> " };
  int valores[3] = { luzFrente, luzIzq, luzDer };
  for (int i = 0; i < 3; i++) {
    textoEntero(b, 5, valores[i]);
    Serial.print(nombres[i]); Serial.print(b);
    Serial.print(valores[i] >= umbralBlanco ? "   BLANCO" : "   verde ");
    finDeLinea();
  }
  finDeLinea();

  // ---- 16-18: camara -----------------------------------------------
  bool camaraViva = (millis() - t_ultimoPaquete) < 500;
  Serial.print("CAMARA");
  for (int i = 0; i < 34; i++) Serial.write(' ');
  if (camaraViva) {
    textoFloat(b, 5, paquetesPorSegundo, 1);
    Serial.print(b); Serial.print(" paquetes por segundo");
  } else {
    Serial.print("NO ESTA HABLANDO");
  }
  finDeLinea();

  if (camaraViva && Xp > 0) {
    textoFloat(b, 6, Xp / CAMARA_POR_CM, 1);
    Serial.print("  pelota    -> "); Serial.print(b);
    Serial.print(" cm de frente,");
    textoFloat(b, 6, Yp / CAMARA_POR_CM, 1);
    Serial.print(b); Serial.print(" cm al costado");
  } else if (camaraViva) {
    Serial.print("  pelota    -> no la ve  (o la tiene justo encima)");
  } else {
    Serial.print("  pelota    -> --");
  }
  finDeLinea();

  Serial.print("  arcos     -> azul ");
  Serial.print((camaraViva && Xaz > 0) ? "SI" : "no");
  Serial.print("     amarillo ");
  Serial.print((camaraViva && Xam > 0) ? "SI" : "no");
  finDeLinea();
  finDeLinea();

  // ---- 20-22: opciones ---------------------------------------------
  Serial.print("OPCIONES  1 reiniciar giro   2 reiniciar Teensy   3 yaw a cero");
  finDeLinea();
  Serial.print("          4 NDOF   5 IMUPLUS   6 cristal   7 bus   8 direccion");
  finDeLinea();
  Serial.print("          9 escanear I2C   h historial   m sacudon   ");
  Serial.print(pausado ? "0 SEGUIR" : "0 pausar");
  Serial.print("   ? ayuda");
  finDeLinea();
  finDeLinea();

  // ---- 24-25: los dos ultimos avisos -------------------------------
  Serial.print("> "); Serial.print(aviso1); finDeLinea();
  Serial.print("> "); Serial.print(aviso2); finDeLinea();
}

// Respaldo por si la consola no entiende los codigos ANSI: los mismos
// datos, en texto liso, una tanda por segundo. Scrollea, pero se lee.
void dibujarListaSimple() {
  Serial.println();
  Serial.print("--- "); Serial.print(millis() / 1000);
  Serial.println(" s ----------------------------------------");
  Serial.print("giroscopio: ");
  if (salud == SANO) {
    Serial.print("yaw "); Serial.print(yaw, 1);
    Serial.print("  pitch "); Serial.print(pitch, 1);
    Serial.print("  roll ");  Serial.print(roll, 1);
  } else {
    Serial.print(salud == MUDO ? "MUDO" : "AUSENTE");
  }
  Serial.print("   modo "); Serial.print(nombreModo(modoReal));
  Serial.print("   cal S"); Serial.print(calSis);
  Serial.print(" G"); Serial.print(calGir);
  Serial.print(" A"); Serial.print(calAce);
  Serial.print(" M"); Serial.println(calMag);
  Serial.print("luz: frente "); Serial.print(luzFrente);
  Serial.print("  izq ");       Serial.print(luzIzq);
  Serial.print("  der ");       Serial.print(luzDer);
  Serial.print("   (blanco si pasa de "); Serial.print(umbralBlanco);
  Serial.println(")");
  Serial.print("camara: ");
  if ((millis() - t_ultimoPaquete) < 500) {
    Serial.print("pelota x "); Serial.print(Xp);
    Serial.print(" y ");       Serial.print(Yp);
    Serial.print("   azul ");  Serial.print(Xaz > 0 ? "si" : "no");
    Serial.print("   amarillo "); Serial.println(Xam > 0 ? "si" : "no");
  } else {
    Serial.println("no esta hablando");
  }
  Serial.print("caidas "); Serial.print(caidasTotales);
  Serial.print("   ultimo aviso: "); Serial.println(aviso1);
}


// =====================================================================
//  PANTALLAS DE INFORME
// =====================================================================
//
// Los informes largos no entran en el tablero. En vez de escupirlos
// encima (que romperia la pantalla quieta), se muestran en su propia
// pantalla y con cualquier tecla se vuelve.

bool enInforme = false;

void abrirInforme(const char *titulo) {
  enInforme = true;
  limpiarTodo();
  Serial.println();
  Serial.print("=== "); Serial.print(titulo);
  Serial.println(" ===");
  Serial.println();
}

void cerrarInforme() {
  Serial.println();
  Serial.println("--- apreta cualquier tecla para volver al tablero ---");
}

void informeEstado() {
  abrirInforme("ESTADO INTERNO DEL CHIP");
  byte v;
  Serial.print(" chip_id : ");
  if (leerReg(REG_CHIP_ID, &v)) {
    Serial.print("0x"); Serial.print(v, HEX);
    Serial.println(v == 0xA0 ? "   es un BNO055 de verdad"
                             : "   hay algo, pero NO es un BNO055");
  } else {
    Serial.println("no contesta  <- el chip no esta en el bus");
    cerrarInforme();
    return;
  }
  if (leerReg(REG_PAGE_ID, &v)) {
    Serial.print(" page_id : "); Serial.print(v);
    Serial.println(v == 0 ? "   (correcto)"
                          : "   <- tiene que ser 0, si no todo se lee mal");
  }
  if (leerReg(REG_OPR_MODE, &v)) {
    v &= 0x0F;
    Serial.print(" modo    : "); Serial.print(nombreModo(v));
    if (v == 0x00)      Serial.println("   <- CONFIG: el chip se reinicio solo");
    else if (v == 0x0C) Serial.println("   (con brujula, el que usa el juego)");
    else if (v == 0x08) Serial.println("   (sin brujula)");
    else                Serial.println();
  }
  if (leerReg(REG_PWR_MODE, &v)) {
    Serial.print(" consumo : "); Serial.print(v);
    Serial.println(v == 0 ? "   (normal)" : "   <- no es el normal");
  }
  if (leerReg(REG_SYS_STAT, &v)) {
    Serial.print(" estado  : "); Serial.print(v);
    Serial.print("   ");         Serial.println(nombreSysStat(v));
  }
  if (leerReg(REG_SYS_ERR, &v)) {
    Serial.print(" error   : "); Serial.print(v);
    Serial.print("   ");         Serial.println(nombreSysErr(v));
  }
  if (leerReg(REG_ST_RESULT, &v)) {
    Serial.print(" autotest: acelerometro "); Serial.print((v & 0x01) ? "OK" : "MAL");
    Serial.print("   magnetometro ");         Serial.print((v & 0x02) ? "OK" : "MAL");
    Serial.print("   giroscopo ");            Serial.print((v & 0x04) ? "OK" : "MAL");
    Serial.print("   micro ");                Serial.println((v & 0x08) ? "OK" : "MAL");
  }
  Serial.println();
  Serial.print(" temperatura "); Serial.print(temperatura); Serial.println(" C");
  Serial.print(" bus "); Serial.print(velocidadBus / 1000);
  Serial.print(" kHz   direccion 0x"); Serial.print(direccion, HEX);
  Serial.print("   cristal externo "); Serial.println(usarCristal ? "SI" : "NO");
  Serial.println();
  Serial.println(" LA CALIBRACION (0 a 3) se pierde en cada apagado.");
  Serial.print(" sistema "); Serial.print(calSis);
  Serial.println(calSis == 0 ? "   <- con 0 el rumbo NO significa nada" : "");
  Serial.print(" giroscopo "); Serial.print(calGir);
  Serial.println("   sube solo QUEDANDOSE QUIETO unos segundos");
  Serial.print(" acelerometro "); Serial.print(calAce);
  Serial.println("   se calibra apoyandolo en varias posiciones");
  Serial.print(" brujula "); Serial.print(calMag);
  Serial.println("   se calibra moviendolo en ocho en el aire");
  Serial.println();
  Serial.println(" Para el arquero el que importa es el GIROSCOPO: eso explica");
  Serial.println(" el 5 de 5 del 21/09 apoyandolo quieto antes de prender.");
  cerrarInforme();
}

void informeHistorial() {
  abrirInforme("HISTORIAL DE CAIDAS");
  if (caidasTotales == 0) {
    Serial.print(" Ninguna en "); Serial.print(millis() / 1000.0, 1);
    Serial.println(" s. El sensor viene aguantando.");
    cerrarInforme();
    return;
  }
  Serial.print(" "); Serial.print(caidasTotales);
  Serial.print(" caida(s) en "); Serial.print(millis() / 1000.0, 1);
  Serial.println(" s:");
  Serial.println();
  for (int i = 0; i < nCaidas; i++) {
    Caida &c = caidas[i];
    Serial.print("  "); Serial.print(i + 1); Serial.print(") a los ");
    Serial.print(c.ms / 1000.0, 1); Serial.print(" s   ");
    Serial.print(c.tipo == AUSENTE ? "AUSENTE" : "MUDO   ");
    Serial.print("  modo ");  Serial.print(nombreModo(c.modo));
    Serial.print("  estado ");Serial.print(c.sysStat);
    Serial.print(" error ");  Serial.print(c.sysErr);
    Serial.print("  calS ");  Serial.print(c.calSis);
    if (c.pwm > 0) { Serial.print("  MOTORES PWM "); Serial.print(c.pwm); }
    if (c.duracion > 0) {
      Serial.print("  -> volvio en "); Serial.print(c.duracion / 1000.0, 1);
      Serial.print(" s");
    } else {
      Serial.print("  -> NO volvio");
    }
    Serial.println();
  }
  if (caidasTotales > nCaidas) {
    Serial.print("  (la tabla guarda "); Serial.print(MAX_CAIDAS);
    Serial.println("; las demas solo se contaron)");
  }
  cerrarInforme();
}

void informeEscaneo() {
  abrirInforme("ESCANEO DE LOS TRES BUSES I2C");
  // ⚠️ Sin begin(), el Teensy ni siquiera le conecta los pines al modulo
  // I2C: el bus queda apagado y el escaneo contesta "vacio" con total
  // seguridad, aunque el giroscopio este ahi. Ademas tarda una eternidad
  // porque cada una de las 126 direcciones espera su tiempo de descarte.
  Wire1.begin();
  Wire2.begin();

  TwoWire *buses[3] = { &Wire, &Wire1, &Wire2 };
  const char *nombres[3] = { "Wire  (18/19)", "Wire1 (16/17)", "Wire2 (24/25)" };
  for (int b = 0; b < 3; b++) {
    int encontrados = 0;
    Serial.print(" "); Serial.print(nombres[b]); Serial.print(": ");
    for (byte d = 1; d < 127; d++) {
      buses[b]->beginTransmission(d);
      if (buses[b]->endTransmission() == 0) {
        encontrados++;
        Serial.print("0x"); Serial.print(d, HEX);
        if (d == 0x28 || d == 0x29) Serial.print(" (BNO055)");
        Serial.print("  ");
      }
    }
    if (encontrados == 0) Serial.print("vacio");
    Serial.println();
  }
  Serial.println();
  Serial.println(" La placa Zircon Mark1 lleva el giroscopio en Wire (18/19).");
  Serial.println(" Si aparece en otro bus, esta enchufado donde no va.");
  Wire.setClock(velocidadBus);        // el escaneo no tiene que cambiarla
  cerrarInforme();
}

void informeAyuda() {
  abrirInforme("TECLAS");
  Serial.println("  1  reiniciar el giroscopio (bus + chip + verificar 20 lecturas)");
  Serial.println("  2  reiniciar el TEENSY entero");
  Serial.println("     se cae el puerto USB: hay que volver a abrir mirar.bat");
  Serial.println("  3  poner el yaw de ahora como cero");
  Serial.println("  4  modo NDOF    (con brujula) - el que usa el programa de juego");
  Serial.println("  5  modo IMUPLUS (sin brujula) - el pendiente del 21/09");
  Serial.println("  6  cristal externo si / no");
  Serial.println("  7  bus a 100 kHz / 400 kHz");
  Serial.println("  8  probar la otra direccion (0x28 <-> 0x29)");
  Serial.println("  9  escanear los tres buses I2C");
  Serial.println("  0  pausar / seguir");
  Serial.println();
  Serial.println("  e  estado interno del chip, explicado");
  Serial.println("  h  historial de caidas");
  Serial.println("  a  revivir el giroscopio solo cuando se cae: si / no");
  Serial.println("  x  borrar contadores e historial");
  Serial.println("  l  lista simple (si la pantalla se ve con basura)");
  Serial.println("  m  SACUDON DE MOTORES - apretar dos veces. EL ROBOT GIRA.");
  Serial.println("  ?  esta ayuda");
  cerrarInforme();
}


// =====================================================================
//  MOTORES — solo para el sacudon
// =====================================================================

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
  pwmMotores = 0;
}

// Las tres ruedas al mismo sentido = rotacion pura sobre el eje, igual que
// girarHaciaObjetivo() del cuadrado. No se traslada: gira en el lugar.
void girarEnElLugar(bool sentido, int pwm) {
  digitalWrite(INA1, sentido ? 1 : 0); digitalWrite(INB1, sentido ? 0 : 1);
  digitalWrite(INA2, sentido ? 1 : 0); digitalWrite(INB2, sentido ? 0 : 1);
  digitalWrite(INA3, sentido ? 1 : 0); digitalWrite(INB3, sentido ? 0 : 1);
  analogWrite(PWM1, pwm); analogWrite(PWM2, pwm); analogWrite(PWM3, pwm);
  pwmMotores = pwm;
}

// 🚨 LA PRUEBA QUE FALTA HACER.
//
// La sospecha del 21/09 es que un bajon de tension tumba al chip, y lo que
// mas hunde la tension es el ARRANQUE de los tres motores desde quieto.
// Esto lo provoca a proposito: tirones cortos alternando el sentido.
//
// Alterna por dos razones: cada cambio de sentido es un arranque nuevo
// desde cero (que es el momento del tiron), y como gira para un lado y
// para el otro, el robot queda donde estaba.
//
// Entre tiron y tiron pasa por PWM 0 un instante: invertir un motor a
// fondo de golpe castiga el puente H sin necesidad.
const int TIRONES   = 10;
const int PWM_TIRON = 200;
const int MS_TIRON  = 150;
const int MS_RESPIRO = 60;

void sacudon() {
  // Si ya estaba caido, el sacudon no prueba nada: se caeria en el primer
  // tiron y le echariamos la culpa a los motores sin motivo.
  if (salud != SANO) {
    avisar("el giroscopio ya esta caido: primero levantalo con la tecla 1");
    return;
  }
  avisar("SACUDON: girando a los tirones. Cualquier tecla lo corta.");
  dibujarTablero();

  for (int i = 0; i < TIRONES; i++) {
    girarEnElLugar(i % 2 == 0, PWM_TIRON);
    unsigned long t0 = millis();
    while (millis() - t0 < MS_TIRON) {
      medirGiroscopo();
      if (salud != SANO) {
        pwmAlCaer = PWM_TIRON;      // antes de apagarlos, si no se pierde
        apagarMotores();
        char t[75], n[6];
        textoEntero(n, 0, i + 1);
        snprintf(t, sizeof(t), "SE CAYO EN EL TIRON %s DE 10"
                               " -> es la corriente de los motores", n);
        avisar(t);
        return;
      }
      // Cualquier tecla corta la prueba. Sin esto no habria forma de
      // pararla desde el teclado hasta que termine.
      if (Serial.available()) {
        apagarMotores();
        while (Serial.available()) Serial.read();
        avisar("SACUDON cortado a mano");
        return;
      }
    }
    apagarMotores();
    delay(MS_RESPIRO);
  }
  apagarMotores();
  avisar("aguanto los 10 tirones. El arranque solo no lo tumba.");
}


// =====================================================================
//  TECLADO
// =====================================================================

unsigned long t_pidioSacudon = 0;

void reiniciarTeensy() {
  apagarMotores();
  limpiarTodo();
  Serial.println();
  Serial.println("  Reiniciando el Teensy.");
  Serial.println("  El puerto USB se va a caer: volve a abrir mirar.bat.");
  Serial.flush();
  delay(300);
  SCB_AIRCR = 0x05FA0004;      // reset por software del Cortex-M7
}

void volverAlTablero() {
  enInforme = false;
  limpiarTodo();
  ocultarCursor();
}

void leerTeclado() {
  while (Serial.available()) {
    char t = Serial.read();
    if (t == '\n' || t == '\r') continue;

    // Dentro de un informe, cualquier tecla vuelve al tablero y no hace
    // nada mas. Asi no se dispara una accion sin querer al salir.
    if (enInforme) {
      while (Serial.available()) Serial.read();
      volverAlTablero();
      return;
    }

    // El sacudon pide confirmacion: dos `m` seguidas, dentro de 4 s.
    if (t == 'm') {
      if (t_pidioSacudon != 0 && millis() - t_pidioSacudon < 4000) {
        t_pidioSacudon = 0;
        sacudon();
      } else {
        t_pidioSacudon = millis();
        avisar("EL ROBOT VA A GIRAR. Sacale las manos y apreta m otra vez.");
      }
      continue;
    }
    t_pidioSacudon = 0;        // cualquier otra tecla cancela la confirmacion

    switch (t) {
      case '1':
        intentosSeguidos = 0;        // la mano reinicia la paciencia
        if (revivir()) {
          salud = SANO; anotarVuelta(millis()); saludAnterior = SANO;
        }
        break;

      case '2': reiniciarTeensy(); break;

      case '3':
        if (salud == SANO) {
          ceroManual = yaw;
          avisar("cero puesto en el yaw de ahora");
        } else {
          avisar("no puedo poner el cero: ahora no esta dando datos");
        }
        break;

      case '4':
        modoPedido = OPERATION_MODE_NDOF;
        avisar("modo NDOF (con brujula), el que usa el programa de juego");
        revivir();
        break;

      case '5':
        modoPedido = OPERATION_MODE_IMUPLUS;
        avisar("modo IMUPLUS (sin brujula). Ojo: el yaw se corre despacio.");
        revivir();
        break;

      case '6':
        usarCristal = !usarCristal;
        avisar(usarCristal ? "cristal externo: SI" : "cristal externo: NO");
        revivir();
        break;

      case '7':
        velocidadBus = (velocidadBus == 400000) ? 100000 : 400000;
        avisar(velocidadBus == 100000 ? "bus I2C a 100 kHz (el lento)"
                                      : "bus I2C a 400 kHz (el rapido)");
        revivir();
        break;

      case '8':
        direccion = (direccion == 0x28) ? 0x29 : 0x28;
        bno = (direccion == 0x28) ? &bno28 : &bno29;
        avisar(direccion == 0x29 ? "probando la direccion 0x29"
                                 : "probando la direccion 0x28");
        revivir();
        break;

      case '9': informeEscaneo(); break;

      case '0':
        pausado = !pausado;
        avisar(pausado ? "pausado (0 para seguir)" : "siguiendo");
        if (!pausado) volverAlTablero();
        break;

      case 'e': informeEstado();    break;
      case 'h': informeHistorial(); break;
      case '?': informeAyuda();     break;

      case 'a':
        revivirSolo = !revivirSolo;
        avisar(revivirSolo
               ? "lo revivo solo cuando se cae"
               : "NO lo revivo solo: asi se ve si vuelve por su cuenta");
        break;

      case 'x':
        lecturasOk = lecturasMalas = 0;
        nCaidas = caidasTotales = 0;
        paquetesTotales = 0;
        aviso1[0] = aviso2[0] = 0;
        avisar("contadores e historial borrados");
        break;

      case 'l':
        pantallaAnsi = !pantallaAnsi;
        if (pantallaAnsi) volverAlTablero();
        else {
          limpiarTodo();
          Serial.println("Lista simple. La tecla l vuelve al tablero.");
        }
        break;

      default: {
        char m[75];
        snprintf(m, sizeof(m), "no conozco la tecla '%c'. Apreta ? para la ayuda.", t);
        avisar(m);
        break;
      }
    }
  }
}


// =====================================================================
//  PROGRAMA
// =====================================================================

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();

  pinMode(LINEA_ADELANTE,  INPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS_CAMARA);
  delay(800);                        // que mirar.bat alcance a engancharse

  limpiarTodo();
  Serial.println("=== MONITOR DEL ROBOT ARQUERO ===");
  Serial.println();

  // El Teensy 4.1 guarda por que se murio la vez anterior. Si el problema
  // fuera un bajon que reinicia TODO (no solo el giroscopio), la huella
  // queda aca.
  if (CrashReport) {
    Serial.println(" EL TEENSY SE REINICIO SOLO LA VEZ ANTERIOR:");
    Serial.print(CrashReport);
    Serial.println();
  }

  Wire.begin();
  Wire.setClock(velocidadBus);
  delay(100);

  if (bno->begin(modoPedido)) {
    delay(700);
    if (usarCristal) bno->setExtCrystalUse(true);
    avisar("giroscopio encontrado en 0x28");
  } else {
    avisar("el giroscopio no contesta. ¿Esta prendida la bateria? Proba la tecla 1.");
  }

  t_ultimoDato = millis();
  t_cuenta = millis();
  salud = AUSENTE;
  saludAnterior = AUSENTE;           // el primer cambio real se va a anotar

  ocultarCursor();
  limpiarTodo();
}



void loop() {
  leerTeclado();
  medirCamara();                     // hay que vaciar el buffer seguido

  unsigned long ahora = millis();

  // ---- medir ----
  if (ahora - t_medida >= MS_MEDIDA) {
    t_medida = ahora;
    medirGiroscopo();
    medirLuz();

    digitalWrite(LED, salud == SANO ? HIGH : LOW);

    if (salud != saludAnterior) {
      // Al arrancar, saludAnterior vale AUSENTE. Si el giroscopio esta
      // bien, la primera lectura buena es un cambio AUSENTE->SANO que no
      // es ninguna "vuelta": nunca se cayo. Por eso se pide que haya
      // habido una caida de verdad antes de cantarla.
      if (salud == SANO) { if (caidasTotales > 0) anotarVuelta(ahora); }
      else if (saludAnterior == SANO) anotarCaida(ahora);
      else {
        // AUSENTE <-> MUDO: no es una caida nueva, pero cambia el diagnostico
        avisar(salud == AUSENTE ? "paso de MUDO a AUSENTE (dejo de contestar)"
                                : "paso de AUSENTE a MUDO (contesta pero da ceros)");
      }
      saludAnterior = salud;
    }

    // Revivir automatico, con freno de mano. revivir() tarda casi 2
    // segundos, y durante ese rato la pantalla se congela y el teclado no
    // contesta: sin un descanso entre intentos, un sensor que no vuelve
    // dejaria la herramienta inutilizable justo cuando mas se la necesita.
    if (revivirSolo && salud != SANO
        && intentosSeguidos < MAX_INTENTOS_SEGUIDOS
        && ahora - t_ultimoIntento >= MS_ENTRE_INTENTOS) {
      t_ultimoIntento = ahora;
      intentosSeguidos++;
      if (revivir()) {
        salud = SANO;
        anotarVuelta(millis());      // cerrar la caida en el historial
        saludAnterior = SANO;
        intentosSeguidos = 0;
      } else if (intentosSeguidos >= MAX_INTENTOS_SEGUIDOS) {
        avisar("me rindo de revivirlo solo. La tecla 1 vuelve a intentar.");
      }
      t_medida = millis();           // revivir() tarda ~2 s
    }
  }

  // ---- paquetes por segundo de la camara ----
  if (ahora - t_cuenta >= 1000) {
    paquetesPorSegundo = (paquetesTotales - paquetesEnLaVuelta)
                       * 1000.0 / (ahora - t_cuenta);
    paquetesEnLaVuelta = paquetesTotales;
    t_cuenta = ahora;
  }

  // ---- dibujar ----
  if (!pausado && !enInforme && ahora - t_dibujo >= MS_DIBUJO) {
    t_dibujo = ahora;
    if (pantallaAnsi) dibujarTablero();
    else if (ahora % 1000 < MS_DIBUJO) dibujarListaSimple();
  }
}
