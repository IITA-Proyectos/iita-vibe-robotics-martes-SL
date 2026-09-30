/* =====================================================================
   CUANDO SE CAE — ¿a que potencia de motor se muere el giroscopio?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-29
   =====================================================================

   LO QUE YA SABEMOS (medido hoy con derecho-y-vuelta)
   El giroscopio arranca bien, llega a calibracion 3/3 estando quieto... y
   se cae en LOS CUATRO tramos, apenas los motores empiezan a andar.

        QUIETO ANDA.  MOVIENDOSE SE CAE.  Siempre, no a veces.

   Eso explica todo lo de hoy y tambien lo que quedo sin explicar el 21/09
   ("el LED que empieza a temblar despues de moverse"). Pero deja DOS causas
   posibles, y las dos dicen lo mismo:

        A) ELECTRICA — el tiron de corriente de los motores hunde la
           tension y el chip se reinicia.
        B) MECANICA  — la vibracion abre un contacto flojo.

   ---------------------------------------------------------------------
   🎯 COMO LAS SEPARA ESTE PROGRAMA: POR RAMPA
   ---------------------------------------------------------------------
   En vez de prender los motores de golpe, les sube la potencia DE A POCO y
   anota EN QUE PWM EXACTO se cayo el giroscopio.

        si se cae siempre alrededor del mismo PWM  ->  es CORRIENTE (A)
        si se cae en cualquier lado, distinto cada vez  ->  es VIBRACION (B)

   Un umbral repetible es la firma de un problema electrico: hay una
   corriente a partir de la cual la tension no alcanza. La vibracion, en
   cambio, no tiene umbral — depende de como pegue el temblor.

   Y ademas prueba con DOS cargas distintas:
        2 ruedas (como el despeje)  ->  menos corriente
        3 ruedas (girando en el lugar)  ->  MAS corriente
   Si con 3 se cae antes que con 2, otra vez apunta a la corriente.

   Despues de cada tanda se queda QUIETO unos segundos, para ver si vuelve
   solo. Que vuelva solo al parar los motores es, de nuevo, electrico.

   ---------------------------------------------------------------------
   🚨 HOY LOS MOTORES ESTAN APAGADOS (MOVER_LOS_MOTORES = false)
   ---------------------------------------------------------------------
   La pregunta "corriente o vibracion" ya se contesto: es EL CABLE. Asi que
   por ahora el programa solo VIGILA el giroscopio y cuenta las caidas, sin
   mover nada. Sirve para menear el cable y ver el numero moverse.

   Con el cable arreglado, poner MOVER_LOS_MOTORES en true y vuelve la
   prueba de rampa que sigue descrita abajo.

   ---------------------------------------------------------------------
   🚨 LA PRUEBA DE RAMPA (hoy apagada) ES DE MESA, CON EL CABLE PUESTO
   ---------------------------------------------------------------------
   LAS RUEDAS GIRAN. El robot tiene que estar SUJETO: pegado con cinta,
   apoyado sobre algo que lo levante, o agarrado con la mano. Si lo dejan
   suelto sobre la mesa, se va.

   Va imprimiendo todo por USB mientras corre: en que etapa esta, que PWM
   tienen los motores, y que contesta el giroscopio en ese momento.

   ---------------------------------------------------------------------
   ESPERA AL GIROSCOPIO, NO SE RINDE
   ---------------------------------------------------------------------
   Hoy (29/09) el sensor va y viene en minutos: anduvo 5 de 7, despues
   desaparecio 30 veces seguidas, despues volvio y calibro 3/3, despues se
   fue otra vez. Si el programa se rindiera al arrancar, habria que
   recargarlo cada vez que aparece — y justo cuando aparece es cuando hay
   que aprovechar.

   Asi que si no lo encuentra, se queda buscandolo cada 2 segundos con el
   LED parpadeando rapido, y ARRANCA LA PRUEBA SOLO en cuanto aparezca.
   Mientras espera, los motores estan apagados: se puede menear el conector
   con la mano tranquilo.

   ---------------------------------------------------------------------
   SE PUEDE REPETIR SIN RECARGAR
   ---------------------------------------------------------------------
   Con el cable puesto, la tecla `g` larga otra vuelta completa. Hace falta
   correrlo VARIAS VECES: un umbral solo no dice si es repetible, y eso es
   justo lo que separa "corriente" de "vibracion".

   Para no tener que apretarla a mano, esta `PROBAR.bat` en esta misma
   carpeta: larga las vueltas que le pidas, junta los resultados y al final
   dice si el umbral se repite o no.

   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA, y prendida ANTES del USB.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

#define IZQ_INA 2
#define IZQ_INB 5
#define IZQ_PWM 3
#define DER_INA 8
#define DER_INB 7
#define DER_PWM 6
#define TRA_INA 11
#define TRA_INB 12
#define TRA_PWM 4

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

const unsigned long BAUDIOS = 19200;

// La rampa: de PWM_DESDE a PWM_HASTA en MS_RAMPA. Arranca en 60 porque
// debajo de ~70 los motores no vencen el roce y no pasa nada.
const int PWM_DESDE = 60;
const int PWM_HASTA = 220;      // un poco arriba del despeje (200)
const unsigned long MS_RAMPA  = 8000;
const unsigned long MS_QUIETO = 4000;
const int  TIRONES     = 8;
const int  PWM_TIRON   = 220;
const int  MS_TIRON    = 200;
const unsigned long MS_MUESTRA = 50;    // cada cuanto imprime

// 🚨 2026-09-29: LOS MOTORES ESTAN APAGADOS A PROPOSITO.
//
// Mover los motores servia para separar "corriente" de "vibracion". Esa
// pregunta ya se contesto: el equipo encontro que EL CABLE DEL GIROSCOPIO
// esta roto, y meneandolo con la mano el sensor aparece y desaparece. Con
// el cable roto, una rampa de motores no prueba nada mas.
//
// Asi que por ahora el programa solo VIGILA: informa el estado del
// giroscopio todo el tiempo y cuenta cuantas veces se cae y cuantas vuelve.
// Eso es exactamente lo que sirve mientras se trabaja el cable: se menea y
// se mira el numero.
//
// Cuando el cable este arreglado, poner true y vuelve la prueba de rampa.
const bool MOVER_LOS_MOTORES = false;

// Cuanto dura una vuelta de vigilancia.
const unsigned long MS_VIGILANCIA = 45000;

bool hayGiroscopo = false;

// Lo que venimos a buscar.
int pwmCaida2 = -1;        // PWM al que se cayo con 2 ruedas
int pwmCaida3 = -1;        // PWM al que se cayo con 3 ruedas
bool volvioSolo2 = false;  // ¿volvio al parar los motores?
bool volvioSolo3 = false;
int caidasEnTirones = 0;


// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Las dos de adelante, como el despeje. La trasera suelta.
void dosRuedas(int pwm) {
  analogWrite(IZQ_PWM, pwm); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, pwm); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Las tres al mismo sentido = gira en el lugar. Mas corriente, y no se
// traslada, asi que es la mas segura para la mesa.
void tresRuedas(int pwm, bool sentido) {
  digitalWrite(IZQ_INA, sentido ? 1 : 0); digitalWrite(IZQ_INB, sentido ? 0 : 1);
  digitalWrite(DER_INA, sentido ? 1 : 0); digitalWrite(DER_INB, sentido ? 0 : 1);
  digitalWrite(TRA_INA, sentido ? 1 : 0); digitalWrite(TRA_INB, sentido ? 0 : 1);
  analogWrite(IZQ_PWM, pwm); analogWrite(DER_PWM, pwm); analogWrite(TRA_PWM, pwm);
}


// ---------------------------------------------------------------- giroscopio

// Devuelve true si esta dando datos de verdad. Los tres angulos en cero
// exacto es la firma de "no puedo leer el chip" de la libreria Adafruit.
bool giroSano(float *yaw) {
  sensors_event_t e;
  bno.getEvent(&e);
  *yaw = e.orientation.x;
  return !(e.orientation.x == 0.0 && e.orientation.y == 0.0
           && e.orientation.z == 0.0);
}

// Ademas del dato, lo que dice el chip de si mismo. Distingue tres casos
// que desde afuera se ven iguales:
//   no contesta         -> se fue del bus (electrico o cable)
//   contesta, modo 0    -> CONFIG: SE REINICIO SOLO (bajon de tension)
//   contesta, modo 12   -> sigue en NDOF pero no fusiona
void informarPorDentro() {
  byte v;
  Wire.beginTransmission(0x28);
  Wire.write(0x3D);                       // OPR_MODE
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(0x28, 1) != 1) {
    Serial.println("      >> el chip NO CONTESTA en el bus");
    return;
  }
  v = Wire.read() & 0x0F;
  Serial.print("      >> el chip contesta, modo ");
  Serial.print(v);
  if (v == 0) Serial.println("  <-- 🎯 CONFIG: SE REINICIO SOLO (bajon de tension)");
  else        Serial.println("  <-- sigue en su modo: NO se reinicio");
}


// ---------------------------------------------------------------- etapas

void imprimir(const char* etapa, int pwm, bool sano, float yaw) {
  Serial.print(millis() / 1000.0, 1); Serial.print(" s  ");
  Serial.print(etapa);
  Serial.print("  pwm ");
  if (pwm < 100) Serial.print(" ");
  if (pwm < 10)  Serial.print(" ");
  Serial.print(pwm);
  Serial.print("  giro ");
  if (sano) { Serial.print("SANO  yaw "); Serial.print(yaw, 1); }
  else      { Serial.print("MUDO"); }
  Serial.println();
}

// Una rampa. Devuelve el PWM al que se cayo, o -1 si aguanto entera.
int rampa(const char* etapa, bool tresMotores) {
  Serial.println();
  Serial.print("===== "); Serial.print(etapa);
  Serial.print(": subiendo de "); Serial.print(PWM_DESDE);
  Serial.print(" a ");            Serial.print(PWM_HASTA);
  Serial.print(" en ");           Serial.print(MS_RAMPA / 1000);
  Serial.println(" s =====");

  int pwmDondeSeCayo = -1;
  bool eraSano = true;
  unsigned long t0 = millis();
  unsigned long tUltima = 0;

  while (millis() - t0 < MS_RAMPA) {
    unsigned long dt = millis() - t0;
    int pwm = PWM_DESDE + (int)((long)(PWM_HASTA - PWM_DESDE) * dt / MS_RAMPA);

    if (tresMotores) tresRuedas(pwm, true);
    else             dosRuedas(pwm);

    if (millis() - tUltima >= MS_MUESTRA) {
      tUltima = millis();
      float yaw = 0;
      bool sano = giroSano(&yaw);
      imprimir(etapa, pwm, sano, yaw);

      // El momento exacto: la primera vez que pasa de sano a mudo.
      if (eraSano && !sano) {
        pwmDondeSeCayo = pwm;
        Serial.print("   🚨 SE CAYO ACA: pwm ");
        Serial.print(pwm);
        Serial.print(", a los "); Serial.print(dt); Serial.println(" ms de la rampa");
        informarPorDentro();
        eraSano = false;
      }
      if (!eraSano && sano) {
        Serial.println("   >> volvio, con los motores todavia andando");
        eraSano = true;
      }
    }
  }
  parar();
  return pwmDondeSeCayo;
}

// Un rato quieto. Devuelve true si el giroscopio volvio solo.
bool descansar(const char* etapa) {
  Serial.println();
  Serial.print("===== "); Serial.print(etapa);
  Serial.println(": motores APAGADOS, a ver si vuelve solo =====");
  bool volvio = false;
  unsigned long t0 = millis(), tUltima = 0;
  while (millis() - t0 < MS_QUIETO) {
    if (millis() - tUltima >= MS_MUESTRA * 4) {   // mas lento, no hace falta tanto
      tUltima = millis();
      float yaw = 0;
      bool sano = giroSano(&yaw);
      imprimir(etapa, 0, sano, yaw);
      if (sano) volvio = true;
    }
  }
  if (volvio) Serial.println("   🎯 VOLVIO SOLO al parar los motores.");
  else        Serial.println("   no volvio con los motores parados.");
  return volvio;
}

void tandaDeTirones() {
  Serial.println();
  Serial.print("===== TIRONES: "); Serial.print(TIRONES);
  Serial.print(" arranques de golpe a pwm "); Serial.print(PWM_TIRON);
  Serial.println(", alternando =====");
  Serial.println(" (el arranque desde quieto es el momento de mas corriente)");

  bool eraSano = true;
  for (int i = 0; i < TIRONES; i++) {
    tresRuedas(PWM_TIRON, i % 2 == 0);
    unsigned long t0 = millis(), tUltima = 0;
    while (millis() - t0 < MS_TIRON) {
      if (millis() - tUltima >= MS_MUESTRA) {
        tUltima = millis();
        float yaw = 0;
        bool sano = giroSano(&yaw);
        imprimir("TIRON  ", PWM_TIRON, sano, yaw);
        if (eraSano && !sano) {
          caidasEnTirones++;
          Serial.print("   🚨 SE CAYO en el tiron "); Serial.println(i + 1);
          informarPorDentro();
          eraSano = false;
        }
        if (!eraSano && sano) eraSano = true;
      }
    }
    parar();
    delay(150);                 // un respiro entre tirones
  }
}


// -------------------------------------------------------------- vigilancia
//
// Mira el giroscopio sin tocar los motores y cuenta las transiciones. Lo que
// importa no es el rumbo: es CUANTAS VECES se cae y si vuelve. Si mientras
// esto corre alguien menea el cable y los numeros se mueven, ese cable es el
// problema — y no hace falta ninguna otra prueba.
int vigCaidas = 0, vigVueltas = 0;
long vigSanas = 0, vigMudas = 0;

void vigilar() {
  Serial.println();
  Serial.println("===== VIGILANCIA: los motores NO se mueven =====");
  Serial.print(" ");  Serial.print(MS_VIGILANCIA / 1000);
  Serial.println(" segundos mirando el giroscopio.");
  Serial.println(" 🎯 MENEA EL CABLE DEL GIROSCOPIO MIENTRAS ESTO CORRE.");
  Serial.println("    Si aparece y desaparece cuando lo movés, es ese cable.");
  Serial.println();

  vigCaidas = 0; vigVueltas = 0; vigSanas = 0; vigMudas = 0;
  float yaw = 0;
  bool eraSano = giroSano(&yaw);
  unsigned long t0 = millis(), tUltima = 0;

  while (millis() - t0 < MS_VIGILANCIA) {
    if (millis() - tUltima < 250) continue;
    tUltima = millis();

    bool sano = giroSano(&yaw);
    if (sano) vigSanas++; else vigMudas++;

    // El LED: fijo cuando esta sano, parpadeo rapido cuando no.
    if (sano) digitalWrite(LED, HIGH);
    else      digitalWrite(LED, ((millis() / 50) % 2) ? HIGH : LOW);

    uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
    bno.getCalibration(&sis, &gir, &ace, &mag);

    Serial.print(millis() / 1000.0, 1); Serial.print(" s  ");
    if (sano) { Serial.print("SANO  yaw "); Serial.print(yaw, 1); }
    else      { Serial.print("MUDO           "); }
    Serial.print("   calib giro "); Serial.print(gir);
    Serial.print("   caidas ");     Serial.print(vigCaidas);
    Serial.print("  vueltas ");     Serial.println(vigVueltas);

    if (eraSano && !sano) {
      vigCaidas++;
      Serial.print("   🚨 SE CAYO (n. "); Serial.print(vigCaidas); Serial.println(")");
      informarPorDentro();
      eraSano = false;
    } else if (!eraSano && sano) {
      vigVueltas++;
      Serial.print("   >> volvio (n. "); Serial.print(vigVueltas); Serial.println(")");
      eraSano = true;
    }
  }

  digitalWrite(LED, LOW);
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" VIGILANCIA TERMINADA");
  Serial.print(" lecturas sanas "); Serial.print(vigSanas);
  Serial.print("   mudas ");         Serial.println(vigMudas);
  Serial.print(" se cayo ");         Serial.print(vigCaidas);
  Serial.print(" veces, volvio ");   Serial.print(vigVueltas);
  Serial.println(" veces");
  Serial.println();
  if (vigCaidas == 0 && vigMudas == 0) {
    Serial.println(" >> AGUANTO TODO EL RATO SIN CAERSE, quieto.");
    Serial.println("    Si ademas lo meneaste y no se cayo, el cable quedo bien.");
  } else if (vigCaidas > 0) {
    Serial.println(" >> SE CAE. Si se cayo justo cuando movias el cable, ES");
    Serial.println("    ESE CABLE. Si se cae solo, sin tocarlo, esta peor.");
  } else {
    Serial.println(" >> Estuvo mudo todo el rato: ni siquiera arranco bien.");
  }
  Serial.println("=========================================================");

  // La misma linea de siempre, para que PROBAR.bat la lea igual. Los pwm van
  // en -1 porque en este modo no hay rampa.
  Serial.print("RESUMEN pwm2=-1 pwm3=-1 tirones="); Serial.print(vigCaidas);
  Serial.println(" volvio2=0 volvio3=0");
  Serial.print("VIGILANCIA caidas="); Serial.print(vigCaidas);
  Serial.print(" vueltas=");          Serial.print(vigVueltas);
  Serial.print(" sanas=");            Serial.print(vigSanas);
  Serial.print(" mudas=");            Serial.println(vigMudas);
  Serial.println("Apreta g para otra vuelta.");
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(BAUDIOS);
  Wire.begin();
  delay(1500);                  // que mirar.bat alcance a engancharse

  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" CUANDO SE CAE — ¿a que potencia se muere el giroscopio?");
  Serial.println("=========================================================");
  Serial.println(" 🚨 LAS RUEDAS GIRAN. El robot tiene que estar SUJETO:");
  Serial.println("    con cinta, levantado, o agarrado con la mano.");
  Serial.println("=========================================================");

  Serial.print(" giroscopio: ");
  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (!hayGiroscopo) {
    Serial.println("NO CONTESTA todavia.");
    Serial.println();
    Serial.println(" 🎯 ME QUEDO BUSCANDOLO cada 2 segundos, con los motores");
    Serial.println("    APAGADOS. Menea el conector con la mano: en cuanto");
    Serial.println("    aparezca, la prueba arranca sola.");
    Serial.println("    (LED parpadeando rapido = todavia no esta)");
    return;                 // el loop sigue buscandolo
  }
  prepararYCorrer();
}


// Termina de poner en marcha el sensor y larga la prueba.
void prepararYCorrer() {
  bno.setExtCrystalUse(true);
  delay(700);
  Serial.println("OK");

  Serial.print(" esperando que se calibre (quieto): ");
  unsigned long tc = millis();
  uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
  while (millis() - tc < 8000) {
    bno.getCalibration(&sis, &gir, &ace, &mag);
    if (gir >= 3) break;
    delay(100);
  }
  Serial.print("giroscopo "); Serial.print(gir); Serial.print("/3 en ");
  Serial.print(millis() - tc); Serial.println(" ms");

  correrPrueba();
}


// Toda la prueba, separada del setup() para poder llamarla en cuanto el
// giroscopio aparezca, sin importar cuanto tarde.
bool yaCorrio = false;

void correrPrueba() {
  yaCorrio = true;
  // Limpiar lo de la vuelta anterior: si no, un "no se cayo" de ahora
  // heredaria el umbral de antes y mentiria.
  pwmCaida2 = -1; pwmCaida3 = -1;
  volvioSolo2 = false; volvioSolo3 = false;
  caidasEnTirones = 0;
  digitalWrite(LED, HIGH);

  if (!MOVER_LOS_MOTORES) { vigilar(); return; }

  // ---- la prueba ----
  // Primero la de 3 ruedas: gira en el lugar y NO se traslada, asi que si
  // el robot no quedo bien sujeto todavia no se va. La de 2 ruedas, que si
  // empuja para adelante, viene despues — da como 25 segundos de margen.
  descansar("QUIETO1");                       // linea de base
  pwmCaida3   = rampa("RAMPA-3R", true);      // 3 ruedas, mas corriente
  volvioSolo3 = descansar("QUIETO2");
  pwmCaida2   = rampa("RAMPA-2R", false);     // 2 ruedas, como el despeje
  volvioSolo2 = descansar("QUIETO3");
  tandaDeTirones();
  descansar("QUIETO4");

  parar();
  digitalWrite(LED, LOW);

  // ---- el veredicto ----
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" RESULTADO");
  Serial.println("=========================================================");
  Serial.print(" con 2 ruedas se cayo a pwm : ");
  if (pwmCaida2 < 0) Serial.println("NO SE CAYO"); else Serial.println(pwmCaida2);
  Serial.print(" con 3 ruedas se cayo a pwm : ");
  if (pwmCaida3 < 0) Serial.println("NO SE CAYO"); else Serial.println(pwmCaida3);
  Serial.print(" caidas en los tirones      : "); Serial.println(caidasEnTirones);
  Serial.print(" volvio solo al parar       : ");
  Serial.print(volvioSolo2 ? "si" : "no");
  Serial.print(" / ");
  Serial.println(volvioSolo3 ? "si" : "no");
  Serial.println();
  Serial.println(" COMO LEERLO:");
  Serial.println(" - Si se cayo y volvio solo al parar los motores, y con 3");
  Serial.println("   ruedas se cayo ANTES que con 2 -> es la CORRIENTE.");
  Serial.println(" - Si no hay un pwm repetible y cambia cada vez que se");
  Serial.println("   corre -> es VIBRACION, o sea el conector.");
  Serial.println(" - Si aparece \"CONFIG: se reinicio solo\", es tension: el");
  Serial.println("   chip se reinicio, no se desconecto.");
  Serial.println();
  Serial.println(" 🎯 CORRERLO 3 VECES. Un umbral solo no dice si es repetible,");
  Serial.println("    y eso es justamente lo que distingue las dos causas.");
  Serial.println("=========================================================");

  // Linea en formato fijo para que PROBAR.bat la lea sola. Que sea siempre
  // igual es a proposito: un programa que tiene que adivinar el formato se
  // rompe en cuanto alguien cambia un texto.
  Serial.print("RESUMEN pwm2="); Serial.print(pwmCaida2);
  Serial.print(" pwm3=");        Serial.print(pwmCaida3);
  Serial.print(" tirones=");     Serial.print(caidasEnTirones);
  Serial.print(" volvio2=");     Serial.print(volvioSolo2 ? 1 : 0);
  Serial.print(" volvio3=");     Serial.println(volvioSolo3 ? 1 : 0);
  Serial.println("Apreta g para otra vuelta.");
}

unsigned long t_busqueda = 0;
int intentosDeBusqueda = 0;

void loop() {
  parar();                            // los motores quietos, siempre

  // ---- todavia no aparecio: buscarlo ----
  if (!hayGiroscopo) {
    digitalWrite(LED, ((millis() / 50) % 2) ? HIGH : LOW);   // rapido = sin giro
    if (millis() - t_busqueda >= 2000) {
      t_busqueda = millis();
      intentosDeBusqueda++;
      hayGiroscopo = bno.begin();
      if (hayGiroscopo) {
        Serial.println();
        Serial.print(">> 🎯 APARECIO, despues de ");
        Serial.print(intentosDeBusqueda);
        Serial.println(" intentos.");
        Serial.println(">> Si aparecio JUSTO cuando movias el cable, ES ESE CABLE.");
        prepararYCorrer();
      } else {
        Serial.print("["); Serial.print(intentosDeBusqueda);
        Serial.println("] sigo sin encontrarlo");
      }
    }
    return;
  }

  // ---- ya corrio la prueba: quieto, esperando otra orden ----
  digitalWrite(LED, LOW);
  while (Serial.available()) {
    char t = Serial.read();
    if (t == 'g' || t == 'G') {
      Serial.println();
      Serial.println(">>>>> OTRA VUELTA <<<<<");
      correrPrueba();
    }
  }
}
