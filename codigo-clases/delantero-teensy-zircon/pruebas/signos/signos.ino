/* =====================================================================
   LOS SIGNOS — cerrar las dos perillas que nunca se verificaron
   IITA Salta — taller de los martes — 2026-09-01
   =====================================================================

   EL PROBLEMA. En el firmware hay dos booleanos marcados
   "[SIN VERIFICAR EN BANCO]" desde el 11/08:

       const bool SENTIDO_ORBITA_INVERTIDO = false;
       const bool GIRO_RUMBO_INVERTIDO     = false;

   Nadie sabe de que lado del robot es "Y positivo" de la camara, ni hacia
   donde gira el robot cuando se le pide sentidoA = true. Son 50 y 50: si
   estan al reves, ORBITA_CAMINO_CORTO hace que el robot orbite ALEJANDOSE
   del arco, que es peor que dejarlo apagado.

   ESTE PROGRAMA NO ADIVINA: MIDE. Ahora que el giroscopio anda (arreglado
   el 2026-09-01) el robot puede decir hacia donde giro de verdad.

   TRES MEDICIONES, y con las tres queda todo determinado:

     1. LA CONVENCION DEL GIROSCOPO. Te pide girar el robot a mano HACIA LA
        DERECHA y mira si el rumbo sube o baja. Eso fija que significa
        "rumbo positivo" para ESTE sensor, sin suponer nada.

     2. HACIA DONDE GIRA sentidoA = true. Manda orbitar un ratito y mide el
        cambio de rumbo. Cruzado con (1), queda: orbitar(true) gira a la
        derecha o a la izquierda.

     3. LA CONVENCION DE LA CAMARA. Te pide poner la pelota a la DERECHA del
        robot y lee el signo de Yp. Eso fija de que lado es "Y positivo".

   Con las tres, el programa IMPRIME que valor tiene que llevar cada
   booleano. No hay que interpretarlo a ojo.

   🚨 EN LA MEDICION 2 EL ROBOT SE MUEVE (orbita ~1,5 s). Espacio libre.
   🚨 Las otras dos son a mano, con el robot quieto.

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

// mismos valores que la orbita del firmware, para que sea la misma maniobra
const int VEL_ORB_FRENTE  = 30;
const int VEL_ORB_IMPULSO = 99;
const int VEL_ORB_TRASERA = 48;
const unsigned long MS_ORB_IMPULSO = 300;
const unsigned long MS_ORBITANDO   = 1500;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiro = false;

// resultados
float giroManoDerecha = 0;    // cuanto cambio el rumbo al girarlo a la derecha
float giroSentidoA    = 0;    // cuanto cambio con orbitar(true)
int   ypDerecha       = 0;    // Yp con la pelota a la derecha
bool  listo = false;

int Xp = 0, Yp = 0;

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// COPIADA TAL CUAL del firmware, para medir la maniobra de verdad
void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE);  digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE);  digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera);      digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

float rumbo() { sensors_event_t e; bno.getEvent(&e); return e.orientation.x; }

float dif(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// mismo protocolo de 9 bytes del firmware
void leerCamara() {
  while (Serial1.available() >= 9) {
    if (Serial1.read() != 201) continue;
    int xp = Serial1.read(), yp = Serial1.read();
    int h2 = Serial1.read();
    Serial1.read(); Serial1.read();
    int h3 = Serial1.read();
    Serial1.read(); Serial1.read();
    if (h2 == 202 && h3 == 203) { Xp = xp; Yp = yp - 100; }
  }
}

void esperar(const char *que, unsigned long ms) {
  Serial.println();
  Serial.print(">>> "); Serial.println(que);
  unsigned long t0 = millis();
  int ult = -1;
  while (millis() - t0 < ms) {
    leerCamara();
    int falta = (ms - (millis() - t0)) / 1000;
    if (falta != ult) { ult = falta; Serial.print("    "); Serial.println(falta + 1); }
    unsigned long per = (ms - (millis() - t0) < 3000) ? 100 : 500;
    digitalWrite(LED, ((millis() / per) % 2) ? HIGH : LOW);
  }
  digitalWrite(LED, HIGH);
}

// Minimos para que una medicion cuente como valida. Sin esto el programa
// sacaba conclusiones de mediciones en CERO — que es lo que paso la primera
// vez que se corrio: nadie llego a girar el robot y escupio un veredicto
// inventado. Una medicion que no se hizo no es un dato: es una trampa.
const float MIN_GIRO_MANO = 20.0;   // grados que hay que girarlo a mano
const float MIN_GIRO_ORB  = 10.0;   // grados que tiene que girar orbitando
const int   MIN_YP        = 8;      // Yp minimo para creerle a la camara

void informe() {
  Serial.println();
  Serial.println("==========================================================");
  Serial.println(" LOS SIGNOS — resultado");
  Serial.println("==========================================================");

  // ---- lo primero: ¿las mediciones sirven? ----
  bool ok1 = fabs(giroManoDerecha) >= MIN_GIRO_MANO;
  bool ok2 = fabs(giroSentidoA)    >= MIN_GIRO_ORB;
  bool ok3 = abs(ypDerecha)        >= MIN_YP;

  if (!ok1 || !ok2 || !ok3) {
    Serial.println();
    Serial.println(" 🔴 MEDICION INVALIDA — NO SE PUEDE CONCLUIR NADA.");
    Serial.println();
    if (!ok1) {
      Serial.print("   1. Girandolo a mano dio "); Serial.print(giroManoDerecha, 1);
      Serial.print(" grados (hacen falta "); Serial.print(MIN_GIRO_MANO, 0);
      Serial.println("). No lo giraste, o lo giraste tarde.");
    }
    if (!ok2) {
      Serial.print("   2. Orbitando dio "); Serial.print(giroSentidoA, 1);
      Serial.print(" grados (hacen falta "); Serial.print(MIN_GIRO_ORB, 0);
      Serial.println("). El robot no llego a girar: ¿estaba en el aire o trabado?");
    }
    if (!ok3) {
      Serial.print("   3. La camara dio Yp="); Serial.print(ypDerecha);
      Serial.print(" (hacen falta "); Serial.print(MIN_YP);
      Serial.println("). No vio la pelota, o estaba muy centrada.");
    }
    Serial.println();
    Serial.println("   Desenchufa y enchufa, y esta vez segui las consignas");
    Serial.println("   cuando el programa las pida. NO cambies ningun booleano");
    Serial.println("   con este resultado.");
    Serial.println();
    return;
  }

  // --- 1. la convencion del giroscopo ---
  bool derechaSuma = (giroManoDerecha > 0);
  Serial.print(" 1. Girandolo a mano a la DERECHA, el rumbo cambio ");
  Serial.print(giroManoDerecha, 1); Serial.println(" grados.");
  Serial.print("    -> en este robot, DERECHA = rumbo ");
  Serial.println(derechaSuma ? "que SUBE" : "que BAJA");

  // --- 2. hacia donde gira sentidoA = true ---
  bool aSuma = (giroSentidoA > 0);
  bool aEsDerecha = (aSuma == derechaSuma);
  Serial.print(" 2. Con orbitar(sentidoA=true) el rumbo cambio ");
  Serial.print(giroSentidoA, 1); Serial.println(" grados.");
  Serial.print("    -> sentidoA = true gira hacia la ");
  Serial.println(aEsDerecha ? "DERECHA" : "IZQUIERDA");

  // --- 3. la convencion de la camara ---
  bool derechaEsYpos = (ypDerecha > 0);
  Serial.print(" 3. Con la pelota a la DERECHA, Yp dio ");
  Serial.println(ypDerecha);
  Serial.print("    -> para la camara, DERECHA = Y ");
  Serial.println(derechaEsYpos ? "POSITIVO" : "NEGATIVO");

  Serial.println();
  Serial.println("----------------------------------------------------------");
  Serial.println(" LA CONCLUSION");
  Serial.println("----------------------------------------------------------");
  Serial.println(" El firmware hace: si el arco tiene angulo POSITIVO,");
  Serial.println(" llama a orbitar(sentidoA = true). Para que eso lo lleve");
  Serial.println(" HACIA el arco, sentidoA=true tiene que girar para el mismo");
  Serial.println(" lado donde la camara ve los angulos positivos.");
  Serial.println();

  // lado fisico donde la camara ve positivo
  bool camPositivoEsDerecha = derechaEsYpos;
  bool coincide = (aEsDerecha == camPositivoEsDerecha);

  Serial.print("    camara: angulo POSITIVO esta a la ");
  Serial.println(camPositivoEsDerecha ? "DERECHA" : "IZQUIERDA");
  Serial.print("    motores: sentidoA=true gira a la ");
  Serial.println(aEsDerecha ? "DERECHA" : "IZQUIERDA");
  Serial.println();

  if (coincide) {
    Serial.println("    ✅ COINCIDEN. Dejar como esta:");
    Serial.println("           const bool SENTIDO_ORBITA_INVERTIDO = false;");
  } else {
    Serial.println("    🔴 NO COINCIDEN. Sin invertir, el robot orbitaria");
    Serial.println("       ALEJANDOSE del arco. Hay que poner:");
    Serial.println("           const bool SENTIDO_ORBITA_INVERTIDO = true;");
  }
  Serial.println();
  Serial.println("    Recien con esto puesto conviene encender");
  Serial.println("           const bool ORBITA_CAMINO_CORTO = true;");
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
  Serial1.begin(19200);
  Wire.begin();
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("##########################################################");
  Serial.println("#  LOS SIGNOS — se miden, no se adivinan                  #");
  Serial.println("##########################################################");

  hayGiro = bno.begin();
  if (hayGiro) {
    bno.setExtCrystalUse(true);
    delay(700);
    uint8_t sys = 0, at = 0, er = 0;
    bno.getSystemStatus(&sys, &at, &er);
    hayGiro = (sys == 5);
  }
  Serial.println(hayGiro ? "Giroscopo OK (fusion corriendo)"
                         : "!! SIN GIROSCOPO — no se puede medir nada");
  if (!hayGiro) return;

  // ---------- MEDICION 1: la convencion del giroscopio ----------
  Serial.println();
  Serial.println("=== MEDICION 1: hacia donde crece el rumbo ===");
  Serial.println("Vas a girar el robot A MANO, sobre la mesa, claramente");
  Serial.println("HACIA LA DERECHA (en sentido horario, visto desde arriba),");
  Serial.println("como un cuarto de vuelta. No hace falta que sea exacto.");
  // NO hay ventana de tiempo: se espera hasta que EFECTIVAMENTE lo gires.
  // Las tres primeras corridas dieron 0.0 porque el aviso salia por el monitor
  // y el equipo estaba mirando el robot, no la pantalla. Depender de que la
  // persona adivine el momento es un mal diseno: que espere el programa.
  float r0 = rumbo();
  Serial.println();
  Serial.println(">>> GIRA EL ROBOT A MANO HACIA LA DERECHA.");
  Serial.println(">>> Sin apuro: espero hasta que lo hagas (hasta 60 s).");
  Serial.println(">>> El LED parpadea MIENTRAS ESPERO y queda FIJO cuando");
  Serial.println(">>> ya lo detecte. Ahi podes soltarlo.");

  unsigned long t0 = millis();
  float peor = 0;
  int ultAviso = -1;
  while (millis() - t0 < 60000) {
    float d = dif(rumbo(), r0);
    if (fabs(d) > fabs(peor)) peor = d;
    digitalWrite(LED, ((millis() / 150) % 2) ? HIGH : LOW);   // parpadeo rapido
    if (fabs(peor) >= MIN_GIRO_MANO) break;                   // listo, ya alcanza
    int seg = (millis() - t0) / 5000;
    if (seg != ultAviso) {
      ultAviso = seg;
      Serial.print("    esperando... llevas "); Serial.print(peor, 1);
      Serial.print(" de "); Serial.print(MIN_GIRO_MANO, 0); Serial.println(" grados");
    }
  }
  digitalWrite(LED, HIGH);
  giroManoDerecha = peor;
  Serial.print("    LISTO. cambio de rumbo: "); Serial.println(giroManoDerecha, 1);

  // ---------- MEDICION 2: hacia donde gira sentidoA = true ----------
  Serial.println();
  Serial.println("=== MEDICION 2: hacia donde gira sentidoA = true ===");
  Serial.println("🚨 EL ROBOT SE VA A MOVER. Dejalo libre, con espacio.");
  esperar("Solta el robot y sacate de encima.", 8000);

  float r1 = rumbo();
  Serial.println(">>> ORBITANDO...");
  t0 = millis();
  while (millis() - t0 < MS_ORBITANDO) {
    bool imp = (millis() - t0 < MS_ORB_IMPULSO);
    orbitar(true, imp ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
  }
  parar();
  delay(500);
  giroSentidoA = dif(rumbo(), r1);
  Serial.print("    cambio de rumbo: "); Serial.println(giroSentidoA, 1);

  // ---------- MEDICION 3: la convencion de la camara ----------
  Serial.println();
  Serial.println("=== MEDICION 3: de que lado ve la camara los Y positivos ===");
  Serial.println("Pone la PELOTA claramente a la DERECHA del robot, adelante");
  Serial.println("y a un costado, donde la camara la vea bien.");
  // Igual que la 1: se espera hasta que la camara VEA la pelota bien al
  // costado, en vez de contar 12 segundos y cruzar los dedos.
  Serial.println();
  Serial.println(">>> PONE LA PELOTA A LA DERECHA del robot, adelante y al");
  Serial.println(">>> costado. Espero hasta verla bien (hasta 60 s).");
  Serial.println(">>> El LED queda FIJO cuando la vea. Ahi no la muevas mas.");

  t0 = millis();
  ultAviso = -1;
  while (millis() - t0 < 60000) {
    leerCamara();
    digitalWrite(LED, ((millis() / 150) % 2) ? HIGH : LOW);
    if (Xp > 0 && abs(Yp) >= MIN_YP) break;
    int seg = (millis() - t0) / 5000;
    if (seg != ultAviso) {
      ultAviso = seg;
      Serial.print("    esperando... Xp="); Serial.print(Xp);
      Serial.print(" Yp="); Serial.print(Yp);
      Serial.println(Xp > 0 ? "  (la veo, pero muy centrada: corrila al costado)"
                            : "  (no la veo)");
    }
  }
  digitalWrite(LED, HIGH);

  Serial.println(">>> LA VEO — leyendo 3 s, no muevas nada");
  t0 = millis();
  long suma = 0; int n = 0;
  while (millis() - t0 < 3000) {
    leerCamara();
    if (Xp > 0) { suma += Yp; n++; }
  }
  if (n == 0) {
    Serial.println("    !! NO VIO LA PELOTA. Repetir: desenchufa y enchufa.");
    return;
  }
  ypDerecha = suma / n;
  Serial.print("    Yp promedio: "); Serial.print(ypDerecha);
  Serial.print("   ("); Serial.print(n); Serial.println(" lecturas)");

  listo = true;
  digitalWrite(LED, LOW);
  informe();
}

void loop() {
  static unsigned long t = 0;
  if (millis() - t < 5000) return;
  t = millis();
  if (listo) informe();
}
