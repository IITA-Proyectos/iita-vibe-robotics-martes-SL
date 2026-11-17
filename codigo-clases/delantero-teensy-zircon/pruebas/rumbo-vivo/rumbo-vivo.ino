// =====================================================================
//  RUMBO EN VIVO — y la comparacion MODO A contra MODO B
//  Robot DELANTERO (Teensy 4.1 / placa Zircon).  NO mueve los motores.
// =====================================================================
//
//  QUE SON LOS DOS MODOS. El BNO055 puede calcular hacia donde mira el robot
//  de dos maneras distintas:
//
//    MODO A — CON BRUJULA  (OPERATION_MODE_NDOF)
//      Usa la brujula que tiene adentro, asi que tiene una referencia EXTERNA:
//      el norte. No se va corriendo con el tiempo, porque la brujula lo vuelve
//      a acomodar. Pero una brujula al lado de tres motores MIENTE (los motores
//      tienen imanes), y cuando termina de calibrarse sola CORRIGE DE GOLPE: el
//      rumbo salta de un numero a otro en un instante.
//
//    MODO B — SIN BRUJULA  (OPERATION_MODE_IMUPLUS)
//      Ignora la brujula. Solo mide CUANTO GIRA y lo va sumando. La referencia
//      es "hacia donde estaba mirando cuando se prendio la bateria". Los
//      motores no lo pueden enganar y NUNCA salta. Pero se va corriendo
//      despacito, porque no hay nada externo que lo acomode.
//
//  POR QUE ESTA PRUEBA. El 12/10, en el partido 1, "el 0 grados se descalibro y
//  pateaba casi hacia nuestro arco". El principal sospechoso es el salto del
//  modo A. Despues del partido se puso el firmware en MODO B
//  (GIRO_SIN_BRUJULA = true) y quedo ACTIVO SIN HABERSE PROBADO NUNCA.
//
//  Y habia una trampa: esta prueba, como las otras 7 que usan el giroscopio,
//  arrancaba el sensor con bno.begin() pelado, o sea en MODO A. Probarla asi
//  era revisar los frenos de otro auto. Ahora mide los dos y los compara.
//
//  QUE DECIDE ESTA PRUEBA
//    - cuanto se corre cada modo ESTANDO QUIETO (lo que mato el partido 1 fue
//      un corrimiento, asi que este numero es el que importa)
//    - si cada modo VUELVE al mismo numero despues de girar el robot y
//      devolverlo a su lugar
//    - de paso, lo de siempre: si girar a la DERECHA hace SUBIR o BAJAR
//
//  COMO LEER EL VEREDICTO
//    Modo B deriva poco (digamos menos de 2 grados en 10 s)  -> sirve, y es mas
//      seguro que el A porque no puede saltar.
//    Modo B deriva mucho -> en un partido largo el cero se va a correr igual,
//      y hay que volver a pensarlo.
//    Modo A da un SALTO grande de golpe -> queda confirmado el sospechoso del
//      partido 1.
//
//  🚨 EL RITUAL DEL ARRANQUE, que con el modo B pesa el doble: la unica
//  referencia que existe es hacia donde miraba el robot al prender la bateria.
//  Apoyarlo DERECHO mirando al arco rival, y recien ahi prender. Cada vez.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <math.h>

// Los pines de los motores se dejan apagados a proposito: esta prueba no mueve
// nada y asi no hay forma de que se escape de la mesa.
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

// ---------- lo que se puede tocar ----------
const unsigned long MS_AVISO    = 6000;   // para leer la instruccion y sacar la mano
const unsigned long MS_QUIETO   = 10000;  // midiendo la deriva, sin tocarlo
const unsigned long MS_GIRANDO  = 15000;  // para girarlo y devolverlo

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiro = false;

struct Resultado {
  bool  pudo      = false;
  int   sys       = -1;
  float derivaQuieto = 0;    // cuanto se movio en MS_QUIETO sin tocarlo
  float saltoMayor   = 0;    // el mayor salto entre dos lecturas seguidas
  float giroMaximo   = 0;    // lo mas que se alejo cuando lo giraron
  float errorAlVolver = 0;   // cuanto le falto para volver al punto de partida
};
Resultado resA, resB;

bool enModoB = false;        // en que modo quedo al final (para el modo vivo)
float refVivo = 0, peorVivo = 0;


// La diferencia mas corta entre dos rumbos, en -180..180. Sin esto, pasar de
// 359 a 1 grado se leeria como un giro de 358.
float dif(float a, float b) {
  float d = a - b;
  while (d >   180.0) d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

float rumbo() {
  sensors_event_t e;
  bno.getEvent(&e);
  return e.orientation.x;
}

// Cambiar de modo en caliente: hay que pasar por CONFIG y esperar. El chip
// necesita unos milisegundos para rearmar la fusion; si se lee antes, devuelve
// basura y parece que el modo no sirve.
void ponerModo(adafruit_bno055_opmode_t modo) {
  bno.setMode(OPERATION_MODE_CONFIG);
  delay(30);
  bno.setMode(modo);
  delay(50);
  // Despues de cambiar de modo la fusion arranca de nuevo: se le da un rato
  // para que se acomode antes de medir, o la "deriva" que midamos seria en
  // realidad el arranque del filtro.
  delay(1200);
}

void cuenta(const char* que, unsigned long ms) {
  Serial.println();
  Serial.print(">>> "); Serial.println(que);
  for (long s = ms / 1000; s > 0; s--) {
    Serial.print("    "); Serial.print(s); Serial.println("...");
    for (int k = 0; k < 10; k++) { digitalWrite(LED, (k % 2) ? HIGH : LOW); delay(100); }
  }
  digitalWrite(LED, HIGH);
}


// Mide un modo completo: primero quieto, despues girandolo a mano.
Resultado medirModo(const char* nombre, adafruit_bno055_opmode_t modo) {
  Resultado r;

  Serial.println();
  Serial.println("-----------------------------------------------------");
  Serial.print(" MIDIENDO "); Serial.println(nombre);
  Serial.println("-----------------------------------------------------");

  ponerModo(modo);

  uint8_t sys = 0, at = 0, er = 0;
  bno.getSystemStatus(&sys, &at, &er);
  r.sys = sys;
  Serial.print("    SYS_STATUS = "); Serial.println(sys);

  // ---- a) QUIETO: la deriva ----
  cuenta("NO LO TOQUES. Midiendo cuanto se corre estando quieto.", MS_AVISO);
  float base = rumbo();
  float anterior = base;
  float menor = 0, mayor = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < MS_QUIETO) {
    float a = rumbo();
    float d = dif(a, base);
    if (d < menor) menor = d;
    if (d > mayor) mayor = d;
    float salto = fabs(dif(a, anterior));
    if (salto > fabs(r.saltoMayor)) r.saltoMayor = salto;
    anterior = a;
    delay(50);
  }
  r.derivaQuieto = mayor - menor;
  digitalWrite(LED, LOW);
  Serial.print("    se movio "); Serial.print(r.derivaQuieto, 1);
  Serial.print(" grados quieto, y el salto mas grande entre dos lecturas fue ");
  Serial.print(r.saltoMayor, 1); Serial.println(" grados");

  // ---- b) GIRANDOLO: vuelve al mismo numero? ----
  cuenta("Ahora GIRALO A LA DERECHA como un cuarto de vuelta, y DEVOLVELO a como estaba, antes de que termine la cuenta.",
         MS_AVISO);
  float partida = rumbo();
  r.giroMaximo = 0;
  t0 = millis();
  while (millis() - t0 < MS_GIRANDO) {
    // PARPADEO LENTO mientras mide el giro, para distinguirlo del FIJO de la
    // fase de quieto. Las instrucciones salen por el cable y en la mesa nadie
    // mira el monitor: el LED es lo unico que se ve. La primera corrida salio
    // con los cuatro numeros en 0,0 porque nadie supo cuando girarlo. [13/10]
    digitalWrite(LED, ((millis() / 500) % 2) ? HIGH : LOW);
    float d = dif(rumbo(), partida);
    if (fabs(d) > fabs(r.giroMaximo)) r.giroMaximo = d;
    delay(50);
  }
  r.errorAlVolver = dif(rumbo(), partida);
  digitalWrite(LED, LOW);
  Serial.print("    lo mas que se alejo: "); Serial.print(r.giroMaximo, 1);
  Serial.print(" grados   y al volver quedo a "); Serial.print(r.errorAlVolver, 1);
  Serial.println(" grados del punto de partida");

  r.pudo = true;
  return r;
}


void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.println(" RUMBO: MODO A (con brujula) contra MODO B (sin brujula)");
  Serial.println("=====================================================");
  Serial.println();
  Serial.println("                                 MODO A      MODO B");
  Serial.print  (" SYS_STATUS (5 = fusion OK)        ");
  Serial.print(resA.sys); Serial.print("           "); Serial.println(resB.sys);
  Serial.print  (" se corre QUIETO (grados/10 s)     ");
  Serial.print(resA.derivaQuieto, 1); Serial.print("         "); Serial.println(resB.derivaQuieto, 1);
  Serial.print  (" salto mas grande de golpe         ");
  Serial.print(resA.saltoMayor, 1); Serial.print("         "); Serial.println(resB.saltoMayor, 1);
  Serial.print  (" error al volver a su lugar        ");
  Serial.print(resA.errorAlVolver, 1); Serial.print("         "); Serial.println(resB.errorAlVolver, 1);

  Serial.println();
  Serial.println(" QUE SIGNIFICA CADA FILA");
  Serial.println("   se corre QUIETO: el robot no se movio, asi que todo lo que");
  Serial.println("     cambio es error del sensor. Es LO QUE MATO EL PARTIDO 1.");
  Serial.println("   salto de golpe: el modo A corrige de golpe cuando la brujula");
  Serial.println("     termina de calibrarse. Un salto grande aca confirma al");
  Serial.println("     sospechoso. El modo B no deberia saltar nunca.");
  Serial.println("   error al volver: lo giraste y lo devolviste; si el numero no");
  Serial.println("     vuelve, el sensor perdio la cuenta en el camino.");

  Serial.println();
  Serial.println(" VEREDICTO");
  if (resB.derivaQuieto <= 2.0)
    Serial.println("   MODO B se queda QUIETO bien: sirve, y encima no puede saltar.");
  else if (resB.derivaQuieto <= 6.0)
    Serial.println("   MODO B se corre un poco. Alcanza para una jugada, ojo en un partido largo.");
  else
    Serial.println("   MODO B SE CORRE MUCHO: el cero se va a mover igual. Volver a pensarlo.");

  if (resA.saltoMayor >= 10.0)
    Serial.println("   MODO A dio un SALTO grande: queda confirmado lo del partido 1.");
  else
    Serial.println("   MODO A no salto en esta corrida. No prueba que no pase: el salto");
    Serial.println("   aparece cuando la brujula termina de calibrarse, y puede tardar.");

  if (fabs(resB.errorAlVolver) > 10.0)
    Serial.println("   OJO: el MODO B no volvio a su lugar. Girar le hace perder la cuenta.");

  Serial.println();
  Serial.print(" EL FIRMWARE HOY USA: MODO ");
  Serial.println(enModoB ? "B (GIRO_SIN_BRUJULA = true)" : "A");
  Serial.println(" De aca en adelante queda el modo del firmware, en vivo.");
  Serial.println(" Gira el robot a la DERECHA y mira si el numero SUBE o BAJA.");
  Serial.println("=====================================================");
}


void setup() {
  int pines[9] = { IZQ_INA, IZQ_INB, IZQ_PWM, DER_INA, DER_INB, DER_PWM,
                   TRA_INA, TRA_INB, TRA_PWM };
  for (int i = 0; i < 9; i++) { pinMode(pines[i], OUTPUT); digitalWrite(pines[i], LOW); }
  pinMode(LED, OUTPUT);

  Serial.begin(19200);
  Wire.begin();
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("############################################################");
  Serial.println("#  RUMBO: MODO A (con brujula) contra MODO B (sin brujula)  #");
  Serial.println("#  NO mueve los motores. Va en la mesa.                     #");
  Serial.println("#  Son 4 tandas: quieto y girando, en cada modo. ~1 minuto.  #");
  Serial.println("#                                                            #");
  Serial.println("#  MIRA EL LED, que es lo unico que se ve sin el monitor:     #");
  Serial.println("#    parpadeo RAPIDO = cuenta, preparate                     #");
  Serial.println("#    FIJO            = NO LO TOQUES                          #");
  Serial.println("#    parpadeo LENTO  = GIRALO y DEVOLVELO a su lugar         #");
  Serial.println("############################################################");

  hayGiro = bno.begin();
  if (hayGiro) {
    bno.setExtCrystalUse(true);
    delay(700);
  }
  if (!hayGiro) {
    Serial.println("!! SIN GIROSCOPO: el chip no contesta. Revisar el cable y 0x28.");
    return;
  }

  resA = medirModo("MODO A — CON brujula (NDOF)",     OPERATION_MODE_NDOF);
  resB = medirModo("MODO B — SIN brujula (IMUPLUS)",  OPERATION_MODE_IMUPLUS);

  // Queda en el modo que usa el firmware, asi el modo vivo de abajo muestra lo
  // que el robot de verdad va a sentir.
  enModoB = true;
  ponerModo(OPERATION_MODE_IMUPLUS);
  refVivo = rumbo();

  informe();
}


void loop() {
  if (!hayGiro) return;
  static unsigned long t = 0;
  static unsigned long tInforme = 0;

  // El informe se repite cada 20 s, para poder leerlo si uno llega tarde.
  if (millis() - tInforme > 20000) { tInforme = millis(); informe(); }

  if (millis() - t < 300) return;
  t = millis();

  float a = rumbo();
  float d = dif(a, refVivo);
  if (fabs(d) > fabs(peorVivo)) peorVivo = d;

  Serial.print("rumbo="); Serial.print(a, 1);
  Serial.print("   desde la referencia: ");
  if (d >= 0) Serial.print("+");
  Serial.print(d, 1);
  Serial.print("   maximo visto: ");
  if (peorVivo >= 0) Serial.print("+");
  Serial.print(peorVivo, 1);
  if (fabs(peorVivo) >= 20) {
    Serial.print(peorVivo > 0 ? "   -> DERECHA = SUBE" : "   -> DERECHA = BAJA");
  }
  Serial.println();
}
