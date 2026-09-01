/* =====================================================================
   PATADA DERECHA — ¿cuanto se tuerce el robot al patear?
   IITA Salta — taller de los martes — 2026-09-01
   =====================================================================

   LA PREGUNTA: "cuando patea se desvia y no le pega recto a la pelota".
   ¿Es que EL ROBOT CURVA, o es que le pega a la pelota de costado?

   Son dos problemas distintos con arreglos opuestos, y una sola medicion
   los separa: si el robot se tuerce SIN pelota, curva el robot. Si va
   derecho, entonces la pelota sale mal por el contacto y lo que hay que
   corregir es la tolerancia de la patada, no las ruedas.

   COMO SE MIDE, SIN CINTA METRICA Y SIN CABLE. El giroscopio (que quedo
   andando el 2026-09-01) dice cuantos GRADOS giro el robot durante la
   patada. Es la medicion directa de lo que preguntamos.

   HACE DOS PATADAS, PARA COMPARAR:
     PATADA A — igual que el firmware de hoy: las dos ruedas de adelante
                con el MISMO PWM, sin ninguna realimentacion.
     PATADA B — igual, pero corrigiendo con el giroscopio: si se va para
                un lado, se FRENA la rueda de ese lado.

   Si B se tuerce mucho menos que A, el heading-hold sirve y lo llevamos
   al firmware. Si las dos se tuercen igual, el problema es mecanico.

   >>> POR QUE LA CORRECCION SOLO FRENA Y NUNCA ACELERA. La patada corre a
   >>> 240 sobre un maximo de 255: si el lazo pidiera acelerar, no habria
   >>> lugar. Restando siempre, la correccion funciona a plena potencia.

   🚨 ESTO MUEVE EL ROBOT A FONDO. Un segundo a 240 son mas de un metro.
   🚨 NO SE PUEDE CORRER SOBRE LA MESA — se cae. Va en el piso o en la
   🚨 cancha, con espacio libre adelante y atras, y sin el cable USB.
   🚨 Los resultados quedan guardados y se leen despues en la mesa,
   🚨 enchufando el USB SIN APAGAR LA BATERIA.

   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

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
#define LED 13

// --------- perillas ---------
const int  VEL_PATADA = 240;   // el mismo que el firmware. No tocar si se
                               // quiere reproducir la patada de verdad.
const unsigned long MS_PATADA = 1000;   // idem
const float KP = 4.0;          // correccion: PWM que se resta por grado de
                               // desvio. 4.0 = 10 grados -> 40 de PWM menos.
const int  RESTA_MAX = 120;    // tope de la correccion, por las dudas

const unsigned long MS_ANTES   = 8000;   // para apoyarlo y sacar la mano
const unsigned long MS_ENTRE   = 15000;  // para volver a ponerlo en su lugar

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiro = false;

// resultados, que quedan en RAM para leerlos despues
float desvioA = 0, desvioB = 0;
float maxA = 0, maxB = 0;
bool  hechoA = false, hechoB = false;

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Las de adelante van con polaridad OPUESTA entre si porque estan montadas
// espejadas. La trasera queda suelta, igual que en avanzar() del firmware.
void empujar(int velIzq, int velDer) {
  analogWrite(IZQ_PWM, velIzq); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, velDer); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);      digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

float rumbo() {
  sensors_event_t e;
  bno.getEvent(&e);
  return e.orientation.x;
}

// diferencia mas corta entre dos angulos, en (-180, 180]
float dif(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void cuenta(const char *que, unsigned long ms) {
  Serial.print(">>> "); Serial.print(que);
  Serial.print(" — arranca en "); Serial.print(ms / 1000); Serial.println(" s");
  unsigned long t0 = millis();
  int ultimo = -1;
  while (millis() - t0 < ms) {
    int falta = (ms - (millis() - t0)) / 1000;
    if (falta != ultimo) {
      ultimo = falta;
      Serial.print("    "); Serial.println(falta + 1);
    }
    // el LED se apura sobre el final
    unsigned long per = (ms - (millis() - t0) < 3000) ? 100 : 500;
    digitalWrite(LED, ((millis() / per) % 2) ? HIGH : LOW);
  }
  digitalWrite(LED, HIGH);
}

// Una patada. Devuelve cuantos grados se torcio, y deja el maximo en *pico.
float patear(bool conGiroscopo, float *pico) {
  float r0 = hayGiro ? rumbo() : 0;
  float peor = 0;
  unsigned long t0 = millis();

  while (millis() - t0 < MS_PATADA) {
    int vi = VEL_PATADA, vd = VEL_PATADA;
    if (conGiroscopo && hayGiro) {
      float err = dif(r0, rumbo());          // >0 = se fue para un lado
      if (fabs(err) > fabs(peor)) peor = err;
      int resta = (int)(fabs(err) * KP);
      if (resta > RESTA_MAX) resta = RESTA_MAX;
      // SOLO SE FRENA, nunca se acelera: a 240 no hay lugar para subir.
      if (err > 0) vd -= resta; else vi -= resta;
      if (vi < 0) vi = 0;
      if (vd < 0) vd = 0;
    } else if (hayGiro) {
      float err = dif(r0, rumbo());
      if (fabs(err) > fabs(peor)) peor = err;
    }
    empujar(vi, vd);
  }
  parar();
  delay(400);                                 // que se asiente antes de medir
  float total = hayGiro ? dif(r0, rumbo()) : 0;
  *pico = peor;
  return total;
}

void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.println(" RESULTADO — cuanto se torcio el robot al patear");
  Serial.println("=====================================================");
  if (!hayGiro) {
    Serial.println(" SIN GIROSCOPO: no se pudo medir. Corre primero");
    Serial.println(" pruebas/giroscopo-crudo/ para ver que pasa.");
    return;
  }
  Serial.print(" A) SIN correccion  (como el firmware de hoy):  ");
  Serial.print(desvioA, 1); Serial.print(" grados   (pico ");
  Serial.print(maxA, 1); Serial.println(")");
  Serial.print(" B) CON giroscopo   (heading-hold, KP=");
  Serial.print(KP, 1); Serial.print("):        ");
  Serial.print(desvioB, 1); Serial.print(" grados   (pico ");
  Serial.print(maxB, 1); Serial.println(")");
  Serial.println();

  float a = fabs(desvioA), b = fabs(desvioB);
  if (a < 3) {
    Serial.println(" -> EL ROBOT VA DERECHO. Menos de 3 grados en un segundo");
    Serial.println("    a fondo es practicamente nada. Entonces la pelota NO");
    Serial.println("    sale torcida porque el robot curve: sale torcida por el");
    Serial.println("    CONTACTO. Con la pelota 15 grados al costado, el frente");
    Serial.println("    plano la toca de refilon. Lo que hay que bajar es");
    Serial.println("    TOL_ANG_PELOTA, no tocar las ruedas.");
  } else if (b < a / 2) {
    Serial.println(" -> EL ROBOT CURVA, Y EL GIROSCOPO LO ARREGLA.");
    Serial.print("    Paso de "); Serial.print(a, 1);
    Serial.print(" a "); Serial.print(b, 1); Serial.println(" grados.");
    Serial.println("    Llevar este heading-hold al estado PATEA_ADEL del firmware.");
  } else {
    Serial.println(" -> EL ROBOT CURVA Y LA CORRECCION NO ALCANZA.");
    Serial.println("    Subir KP, o mirar la mecanica: si una rueda de adelante");
    Serial.println("    empuja mucho mas que la otra, ningun lazo lo tapa a 240.");
    Serial.println("    Probar tambien bajando VEL_PATADA a 200: a 240 no queda");
    Serial.println("    margen para corregir.");
  }
  Serial.println();
  Serial.println(" (para repetir: desenchufa y enchufa)");
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Wire.begin();
  while (!Serial && millis() < 3000) { }

  Serial.println();
  Serial.println("#####################################################");
  Serial.println("#  PATADA DERECHA — cuanto se tuerce el robot        #");
  Serial.println("#  🚨 MUEVE EL ROBOT A FONDO. En el piso, no en la   #");
  Serial.println("#     mesa. Espacio libre adelante.                   #");
  Serial.println("#####################################################");
  Serial.print("Patada: "); Serial.print(VEL_PATADA);
  Serial.print(" durante "); Serial.print(MS_PATADA); Serial.println(" ms");

  hayGiro = bno.begin();
  if (hayGiro) {
    bno.setExtCrystalUse(true);
    delay(700);
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    hayGiro = (sys == 5);
    Serial.print("Giroscopo: SYS_STATUS="); Serial.print(sys);
    Serial.println(hayGiro ? " (fusion corriendo) OK" : " -> NO SIRVE");
  } else {
    Serial.println("Giroscopo: no contesta");
  }

  cuenta("PATADA A (sin correccion)", MS_ANTES);
  desvioA = patear(false, &maxA);
  hechoA = true;
  Serial.print("    A: se torcio "); Serial.print(desvioA, 1); Serial.println(" grados");

  cuenta("PONELO DE NUEVO EN SU LUGAR — PATADA B (con giroscopo)", MS_ENTRE);
  desvioB = patear(true, &maxB);
  hechoB = true;
  Serial.print("    B: se torcio "); Serial.print(desvioB, 1); Serial.println(" grados");

  informe();
  digitalWrite(LED, LOW);
}

void loop() {
  // El resultado se reimprime cada 3 s para poder leerlo DESPUES, enchufando
  // el USB en la mesa sin apagar la bateria.
  static unsigned long t = 0;
  if (millis() - t < 3000) return;
  t = millis();
  if (hechoA && hechoB) informe();
}
