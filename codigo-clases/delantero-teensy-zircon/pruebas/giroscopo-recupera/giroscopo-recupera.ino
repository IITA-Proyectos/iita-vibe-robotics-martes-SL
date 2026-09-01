/* =====================================================================
   ¿EL GIROSCOPO RECUPERA? — prueba de perturbacion
   IITA Salta — taller de los martes — 2026-09-01
   =====================================================================

   LA IDEA ES DEL EQUIPO, y es mejor prueba que la de patada-derecha:
   en vez de mirar si el robot se desvia solo, se lo EMPUJA A PROPOSITO
   mientras anda y se mira si el lazo lo trae de vuelta al rumbo.

   POR QUE ES MEJOR. Un desvio chico puede ser suerte — quiza esa vez las
   ruedas salieron parejas. Recuperarse de un empujon no se puede fingir:
   o el lazo corrige, o no.

   COMO SE CORRE. Anda 4 segundos a VEL = 75 tratando de mantener el rumbo
   con el que arranco. Durante esos 4 segundos hay que TOCARLO: desviarlo a
   mano un poco para un lado, soltarlo, y ver si vuelve. Dos o tres veces.

   POR QUE 75 Y NO 240. Mas lento es mas seguro, recorre menos, y da tiempo
   a perturbarlo varias veces.

   DOS COSAS QUE HAY QUE RESOLVER A ESTA VELOCIDAD, las dos del README:
     - EL ARRANQUE ESTA AL LIMITE. El piso desde quieto es ~70 y estamos en
       75: justo encima. Por eso hay un IMPULSO de 120 durante 300 ms y recien
       despues baja a 75, para que despegue parejo. Es el truco de la orbita.
     - LA CORRECCION SOLO ACELERA. Restar dejaria la rueda cerca del piso de
       rodadura (~40) y se plantaria — y una rueda plantada no corrige. Es el
       espejo exacto de la patada a 240, donde solo podia FRENAR porque no
       habia techo. Aca no hay piso. Sumando siempre, el lazo tiene autoridad
       para los dos lados: para un lado se acelera una rueda, para el otro la
       otra.

   QUE SE MIDE. El error de rumbo cada 20 ms, guardado en RAM, y despues
   dibujado como un grafico de barras. Ahi se ve cada empujon y si el robot
   volvio o se quedo torcido.

   🚨 MUEVE EL ROBOT. A 100 durante 4 segundos recorre bastante. En el piso
   🚨 o en la cancha, con espacio. Los resultados se leen despues, enchufando
   🚨 el USB SIN APAGAR LA BATERIA.

   ===================================================================== */

#include <Arduino.h>
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
#define LED 13

// --------- perillas ---------
const int   VEL      = 75;     // <<< la velocidad de la prueba
const float KP       = 4.0;    // PWM por grado de error
const int   CORR_MAX = 120;    // tope de la correccion

// IMPULSO DE ARRANQUE. Un motor parado necesita ~70 de PWM para despegar, pero
// YA RODANDO se sostiene con ~40 (README del robot, medido). A 50 desde quieto
// zumba y no arranca. Se le da un golpe fuerte y despues baja a VEL: es el
// mismo truco que usa la orbita del firmware (99 x 300 ms -> 48).
const int   VEL_IMPULSO = 120;
const unsigned long MS_IMPULSO = 300;
const unsigned long MS_ANDANDO = 4000;
const unsigned long MS_ANTES   = 8000;
const int   MS_MUESTRA = 20;
const int   N_MUESTRAS = MS_ANDANDO / MS_MUESTRA;   // 200

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiro = false;
float traza[N_MUESTRAS];
int   nTraza = 0;
bool  listo = false;

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void empujar(int vi, int vd) {
  analogWrite(IZQ_PWM, vi); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vd); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

float rumbo() { sensors_event_t e; bno.getEvent(&e); return e.orientation.x; }

float dif(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void correr() {
  float r0 = rumbo();
  unsigned long t0 = millis(), tm = millis();
  nTraza = 0;

  while (millis() - t0 < MS_ANDANDO) {
    bool enImpulso = (millis() - t0 < MS_IMPULSO);
    int base = enImpulso ? VEL_IMPULSO : VEL;

    float err = dif(r0, rumbo());
    int corr = (int)(fabs(err) * KP);
    if (corr > CORR_MAX) corr = CORR_MAX;

    // LA CORRECCION SOLO ACELERA, nunca frena. Es el espejo de lo que se hace
    // en la patada a 240: alla no habia TECHO para subir, aca no hay PISO para
    // bajar. A 50, restar dejaria la rueda por debajo del piso de arranque y se
    // plantaria — y una rueda plantada no corrige nada. Sumando siempre, el
    // lazo tiene autoridad completa para los dos lados: para girar a un lado
    // se acelera una rueda, para el otro lado se acelera la otra.
    int vi = base, vd = base;
    if (err > 0) vi += corr; else vd += corr;
    if (vi > 255) vi = 255;
    if (vd > 255) vd = 255;
    empujar(vi, vd);

    if (millis() - tm >= (unsigned long)MS_MUESTRA && nTraza < N_MUESTRAS) {
      tm = millis();
      traza[nTraza++] = err;
    }
  }
  parar();
  listo = true;
}

void informe() {
  Serial.println();
  Serial.println("==========================================================");
  Serial.println(" ERROR DE RUMBO durante la prueba — un renglon cada 100 ms");
  Serial.println(" El 0 es el rumbo con el que arranco. Cada '#' = 1 grado.");
  Serial.println("==========================================================");

  float peor = 0, suma = 0;
  int   fuera = 0;
  for (int i = 0; i < nTraza; i++) {
    if (fabs(traza[i]) > fabs(peor)) peor = traza[i];
    suma += fabs(traza[i]);
    if (fabs(traza[i]) > 5) fuera++;
  }

  // se dibuja 1 de cada 5 muestras para que entre en pantalla
  for (int i = 0; i < nTraza; i += 5) {
    float e = traza[i];
    Serial.print("  ");
    if (i * MS_MUESTRA < 1000) Serial.print(" ");
    Serial.print(i * MS_MUESTRA); Serial.print(" ms  ");
    if (e < 0) Serial.print("-"); else Serial.print("+");
    int n = (int)fabs(e);
    if (n > 40) n = 40;
    for (int k = 0; k < n; k++) Serial.print("#");
    Serial.print("  "); Serial.println(e, 1);
  }

  Serial.println();
  Serial.print(" peor desvio: "); Serial.print(peor, 1); Serial.println(" grados");
  Serial.print(" error medio: "); Serial.print(suma / nTraza, 1); Serial.println(" grados");
  Serial.print(" muestras con mas de 5 grados: "); Serial.print(fuera);
  Serial.print(" de "); Serial.println(nTraza);
  Serial.println();
  Serial.println(" COMO LEERLO:");
  Serial.println("  - Cada empujon tuyo tiene que verse como una barra que crece.");
  Serial.println("  - Si despues de cada empujon las barras VUELVEN a achicarse,");
  Serial.println("    EL LAZO FUNCIONA: el robot se acomoda solo.");
  Serial.println("  - Si crece y se queda grande, el lazo NO recupera: subir KP.");
  Serial.println("  - Si oscila (cambia de + a - todo el tiempo), KP es DEMASIADO");
  Serial.println("    alto: bajarlo.");
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
  Serial.println("##########################################################");
  Serial.println("#  ¿EL GIROSCOPO RECUPERA? — prueba de perturbacion       #");
  Serial.println("#  🚨 MUEVE EL ROBOT. En el piso, con espacio.            #");
  Serial.println("##########################################################");
  Serial.print("Velocidad "); Serial.print(VEL);
  Serial.print(" (impulso "); Serial.print(VEL_IMPULSO);
  Serial.print(" x "); Serial.print(MS_IMPULSO); Serial.print(" ms)");
  Serial.print(", "); Serial.print(MS_ANDANDO / 1000);
  Serial.print(" s, KP="); Serial.println(KP, 1);

  hayGiro = bno.begin();
  if (hayGiro) {
    bno.setExtCrystalUse(true);
    delay(700);
    uint8_t sys = 0, at = 0, er = 0;
    bno.getSystemStatus(&sys, &at, &er);
    hayGiro = (sys == 5);
    Serial.print("Giroscopo: SYS_STATUS="); Serial.print(sys);
    Serial.println(hayGiro ? " (fusion corriendo) OK" : "  -> NO SIRVE");
  }
  if (!hayGiro) {
    Serial.println("SIN GIROSCOPO: no tiene sentido la prueba. Corre");
    Serial.println("pruebas/giroscopo-crudo/ para ver que pasa.");
    return;
  }

  Serial.println();
  Serial.println(">>> Apoyalo en el piso. Arranca en 8 segundos.");
  Serial.println(">>> MIENTRAS ANDA, TOCALO: desvialo a mano un poco para un");
  Serial.println(">>> lado, soltalo, y mira si vuelve. Dos o tres veces.");
  unsigned long t0 = millis();
  int ult = -1;
  while (millis() - t0 < MS_ANTES) {
    int falta = (MS_ANTES - (millis() - t0)) / 1000;
    if (falta != ult) { ult = falta; Serial.print("    "); Serial.println(falta + 1); }
    unsigned long per = (MS_ANTES - (millis() - t0) < 3000) ? 100 : 500;
    digitalWrite(LED, ((millis() / per) % 2) ? HIGH : LOW);
  }
  digitalWrite(LED, HIGH);

  Serial.println(">>> ANDANDO — tocalo!");
  correr();
  digitalWrite(LED, LOW);
  informe();
}

void loop() {
  static unsigned long t = 0;
  if (millis() - t < 5000) return;
  t = millis();
  if (listo) informe();
}
