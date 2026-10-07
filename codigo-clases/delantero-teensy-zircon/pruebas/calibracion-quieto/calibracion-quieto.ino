// =====================================================================
//  CALIBRACION EN QUIETO — linea, giroscopio y camara, de una sola vez
//  Robot DELANTERO (Teensy 4.1 / placa Zircon)
// =====================================================================
//
//  PARA QUE ES. Llegar a una cancha que no conocemos y, en una sola corrida,
//  medir las tres cosas que CAMBIAN al cambiar de cancha o de luz:
//    FASE 1  GIROSCOPIO  anda o no anda, y cuanto deriva quieto
//    FASE 2  CAMARA      ve los dos arcos? hasta que distancia?
//    FASE 3  LINEA       verde / blanco / negro -> propone los 3 umbrales
//
//  🚨 ESTE PROGRAMA NO MUEVE LOS MOTORES. Ni uno. Se puede tener en la mano,
//  en la mesa o en la cancha sin que se escape. Los motores van en el otro:
//  pruebas/calibracion-moviendo/.
//
//  COMO SE USA SIN CABLE. En la cancha no llega el USB, asi que el robot se
//  guarda TODO en la RAM y al final repite el informe cada 3 segundos. O sea:
//    1. se carga este programa
//    2. se desenchufa el USB, con la BATERIA PUESTA
//    3. se lleva a la cancha y se siguen las fases (el robot avisa con el LED
//       y con cuentas; cada fase arranca sola, por tiempo, no hay que apretar
//       nada porque no hay teclado en la cancha)
//    4. se vuelve y se enchufa el USB **SIN APAGAR LA BATERIA**
//       (si se corta la energia, el informe se pierde: son datos en RAM)
//    5. se lee con CALIBRAR-ROBOT.bat, opcion L
//
//  LO QUE HAY QUE HACER EN CADA FASE, porque el robot no se mueve solo:
//    FASE 1  dejarlo QUIETO sobre la cancha. No tocarlo.
//    FASE 2  APUNTARLO a un arco, despues al otro. El robot avisa cuando
//            cambiar. Hay que moverlo de a poco, no de golpe.
//    FASE 3  ponerlo sobre el VERDE; despues pasarlo LENTO por encima de la
//            LINEA BLANCA, cruzandola, para que los tres sensores la toquen;
//            y por ultimo sobre algo NEGRO.
//
//  LOS NUMEROS SE LEEN IGUAL QUE EN EL FIRMWARE. Mismo protocolo de camara
//  (paquetes de 9 bytes: 201 xp yp 202 xam yam 203 xaz yaz, con las Y
//  corridas -100), misma autodeteccion de placa por el pin 32, mismos pines
//  de sensores. Si se mide distinto que el firmware, la medicion no sirve.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <math.h>

// ---------------------------------------------------------------------
//  LO QUE SE PUEDE TOCAR
// ---------------------------------------------------------------------

// Cuanto dura cada cosa. Los "aviso" son para que alguien alcance a mover el
// robot; las "graba" son los segundos en que de verdad se mide.
const unsigned long MS_AVISO_FASE   = 6000;   // antes de cada fase
const unsigned long MS_GIRO_QUIETO  = 6000;   // fase 1: midiendo la deriva
const unsigned long MS_ARCO_CADA    = 12000;  // fase 2: por arco
const unsigned long MS_VERDE        = 5000;   // fase 3
const unsigned long MS_BLANCO       = 10000;  // cruzando la linea, LENTO
const unsigned long MS_NEGRO        = 5000;

// Margen minimo aceptable entre el verde y el blanco, en cuentas del ADC. Si
// un sensor queda abajo de esto, el umbral que salga es fragil: cualquier
// sombra lo cruza. Hoy en la tela el margen es de ~300 por sensor.
const int MARGEN_MINIMO = 120;

// Topes de recorte de la camara, IGUALES a los del firmware. Arriba de esto
// son manchas, no objetos: 200 es el techo que manda la camara.
const int XP_MAX     = 150;
const int XARCO_MAX  = 200;

const int PIN_VERSION_PLACA = 32;
const int LED = 13;

// ---------------------------------------------------------------------

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

int  pinLinea[3] = { A11, A13, A12 };     // Mark1; se corrige en setup()
const char* versionPlaca = "?";
const char* nombreSensor[3] = { "S1 DERECHO  ", "S2 IZQUIERDO", "S3 DELANTERO" };

// --- camara, leida igual que el firmware ---
int Xp = 0, Yp = 0, Xam = 0, Yam = 0, Xaz = 0, Yaz = 0;
unsigned long nPaquetes = 0, nTirados = 0;

// --- lo que se guarda de cada fase (esto es el informe) ---
bool  giroContesta = false;
int   giroSys = -1;
float rumboMin = 999, rumboMax = -999, rumboPrimero = 0, rumboUltimo = 0;

struct MedArco {
  long  cuadros = 0;        // cuadros en que lo vio
  long  total   = 0;        // cuadros mirados en esa fase
  int   xMin = 9999, xMax = -1;
  float angMin = 999, angMax = -999;
};
MedArco medAm, medAz;
long cuadrosPelota = 0, totalPelota = 0;
int  xpMin = 9999, xpMax = -1;

int verdeMin[3]  = { 9999, 9999, 9999 }, verdeMax[3]  = { -1, -1, -1 };
int blancoMin[3] = { 9999, 9999, 9999 }, blancoMax[3] = { -1, -1, -1 };
int negroMin[3]  = { 9999, 9999, 9999 }, negroMax[3]  = { -1, -1, -1 };
int umbralPropuesto[3] = { -1, -1, -1 };

bool listo = false;       // ya termino todo: el informe se puede imprimir


// ---------------------------------------------------------------------
//  utilidades
// ---------------------------------------------------------------------

// El angulo al que esta un objeto, en grados, con 0 = justo adelante.
// Mismo calculo que anguloDe() del firmware.
float anguloDe(int x, int y) {
  if (x == 0 && y == 0) return 0;
  return atan2((float)y, (float)x) * 180.0 / PI;
}

// Lee todo lo que haya llegado de la camara. Copia exacta del firmware: si se
// parsea distinto, se mide otra cosa.
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
      nPaquetes++;
    }
  }
}

void cuenta(const char* que, unsigned long ms) {
  Serial.println();
  Serial.print(">>> "); Serial.println(que);
  for (long s = ms / 1000; s > 0; s--) {
    Serial.print("    "); Serial.print(s); Serial.println("...");
    // El LED parpadea rapido durante la cuenta y queda FIJO mientras mide,
    // asi se sabe desde lejos si ya esta grabando.
    for (int k = 0; k < 10; k++) {
      digitalWrite(LED, (k % 2) ? HIGH : LOW);
      delay(100);
      leerCamara();              // se sigue leyendo para no desincronizar
    }
  }
  digitalWrite(LED, HIGH);
  Serial.println("    GRABANDO.");
}


// ---------------------------------------------------------------------
//  FASE 1 — GIROSCOPIO
// ---------------------------------------------------------------------
void faseGiroscopio() {
  cuenta("FASE 1 de 3: GIROSCOPIO. Dejalo QUIETO y no lo toques.", MS_AVISO_FASE);

  giroContesta = bno.begin();
  if (giroContesta) {
    delay(700);
    bno.setExtCrystalUse(true);
    uint8_t sys, autotest, err;
    bno.getSystemStatus(&sys, &autotest, &err);
    giroSys = sys;

    unsigned long t0 = millis();
    bool primero = true;
    while (millis() - t0 < MS_GIRO_QUIETO) {
      sensors_event_t ev;
      bno.getEvent(&ev);
      float r = ev.orientation.x;
      if (primero) { rumboPrimero = r; primero = false; }
      rumboUltimo = r;
      if (r < rumboMin) rumboMin = r;
      if (r > rumboMax) rumboMax = r;
      leerCamara();
      delay(20);
    }
  }
  digitalWrite(LED, LOW);
}


// ---------------------------------------------------------------------
//  FASE 2 — CAMARA
// ---------------------------------------------------------------------
void mirarArco(MedArco &m, int x, int y) {
  m.total++;
  if ((x > 0) && (x <= XARCO_MAX) && (abs(y) < 100)) {
    m.cuadros++;
    if (x < m.xMin) m.xMin = x;
    if (x > m.xMax) m.xMax = x;
    float a = anguloDe(x, y);
    if (a < m.angMin) m.angMin = a;
    if (a > m.angMax) m.angMax = a;
  }
}

void grabarArco(MedArco &m, unsigned long ms) {
  unsigned long t0 = millis();
  unsigned long ultimo = 0;
  while (millis() - t0 < ms) {
    leerCamara();
    // Un "cuadro mirado" cada 25 ms: la camara manda ~45 por segundo, asi que
    // esto no pierde nada y hace que los porcentajes signifiquen tiempo.
    if (millis() - ultimo >= 25) {
      ultimo = millis();
      if (&m == &medAm) mirarArco(medAm, Xam, Yam);
      else              mirarArco(medAz, Xaz, Yaz);
      // La pelota se mira en las dos mitades: sirve para saber si la camara
      // esta sana en general, aunque no haya pelota en la cancha.
      totalPelota++;
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        cuadrosPelota++;
        if (Xp < xpMin) xpMin = Xp;
        if (Xp > xpMax) xpMax = Xp;
      }
    }
  }
}

void faseCamara() {
  cuenta("FASE 2 de 3: CAMARA. Apuntalo al arco AMARILLO y movelo de a poco.",
         MS_AVISO_FASE);
  grabarArco(medAm, MS_ARCO_CADA);
  digitalWrite(LED, LOW);

  cuenta("Ahora apuntalo al arco AZUL. Movelo de a poco.", MS_AVISO_FASE);
  grabarArco(medAz, MS_ARCO_CADA);
  digitalWrite(LED, LOW);
}


// ---------------------------------------------------------------------
//  FASE 3 — LINEA
// ---------------------------------------------------------------------
void grabarLinea(int *mn, int *mx, unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    for (int i = 0; i < 3; i++) {
      int v = analogRead(pinLinea[i]);
      if (v < mn[i]) mn[i] = v;
      if (v > mx[i]) mx[i] = v;
    }
    leerCamara();
    delay(5);
  }
}

void faseLinea() {
  cuenta("FASE 3 de 3: LINEA. Apoyalo sobre el VERDE de la cancha.", MS_AVISO_FASE);
  grabarLinea(verdeMin, verdeMax, MS_VERDE);
  digitalWrite(LED, LOW);

  cuenta("Pasalo LENTO por encima de la LINEA BLANCA, cruzandola, para que los TRES sensores la toquen.",
         MS_AVISO_FASE);
  grabarLinea(blancoMin, blancoMax, MS_BLANCO);
  digitalWrite(LED, LOW);

  cuenta("Ahora apoyalo sobre algo NEGRO.", MS_AVISO_FASE);
  grabarLinea(negroMin, negroMax, MS_NEGRO);
  digitalWrite(LED, LOW);

  // EL UMBRAL VA EN EL MEDIO entre el verde mas claro que se vio y el blanco
  // mas brillante que se vio. Se usa el PEOR verde (su maximo) a proposito:
  // es el que esta mas cerca de confundirse con blanco, y es el que manda.
  for (int i = 0; i < 3; i++) {
    if (verdeMax[i] >= 0 && blancoMax[i] >= 0 && blancoMax[i] > verdeMax[i])
      umbralPropuesto[i] = (verdeMax[i] + blancoMax[i]) / 2;
  }
}


// ---------------------------------------------------------------------
//  EL INFORME
// ---------------------------------------------------------------------
void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.println(" CALIBRACION EN QUIETO - informe");
  Serial.print  (" Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print  ("   pines de linea: ");
  Serial.print(pinLinea[0]); Serial.print(", ");
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.println("=====================================================");

  // ---- 1. giroscopio ----
  Serial.println();
  Serial.println(" 1) GIROSCOPIO");
  if (!giroContesta) {
    Serial.println("    NO CONTESTA. Revisar el cable y la direccion I2C (0x28).");
    Serial.println("    Sin giroscopo: no hay patada derecha ni plan B de rumbo.");
  } else {
    Serial.print  ("    SYS_STATUS = "); Serial.print(giroSys);
    Serial.println(giroSys == 5 ? "  (fusion corriendo) OK" : "  <- deberia ser 5");
    Serial.print  ("    rumbo quieto: entre "); Serial.print(rumboMin, 1);
    Serial.print  (" y "); Serial.print(rumboMax, 1);
    float ancho = rumboMax - rumboMin;
    Serial.print  ("  ->  se movio "); Serial.print(ancho, 1);
    Serial.println(" grados sin que nadie lo toque");
    if (giroSys == 5 && ancho <= 2.0)
      Serial.println("    VEREDICTO: sano y estable. Se puede confiar en la patada derecha.");
    else if (giroSys == 5)
      Serial.println("    VEREDICTO: contesta, pero DERIVA. Ojo con la patada derecha.");
    else
      Serial.println("    VEREDICTO: contesta pero la fusion no corre. Esperar o reiniciar.");
  }

  // ---- 2. camara ----
  Serial.println();
  Serial.println(" 2) CAMARA");
  Serial.print  ("    paquetes validos: "); Serial.print(nPaquetes);
  Serial.print  ("   bytes tirados: ");     Serial.println(nTirados);
  if (nPaquetes == 0) {
    Serial.println("    NO LLEGA NADA. Revisar el cable a Serial1 y que la camara este encendida.");
  } else {
    if (nTirados > nPaquetes)
      Serial.println("    OJO: se tiran mas bytes que paquetes buenos -> desincronizacion.");

    const char* nombres[2] = { "ARCO AMARILLO", "ARCO AZUL    " };
    MedArco*    ms[2]      = { &medAm, &medAz };
    for (int k = 0; k < 2; k++) {
      MedArco *m = ms[k];
      Serial.print("    "); Serial.print(nombres[k]); Serial.print(": ");
      if (m->cuadros == 0) {
        Serial.println("NO LO VIO NUNCA");
      } else {
        Serial.print(m->cuadros * 100 / (m->total > 0 ? m->total : 1));
        Serial.print("% de los cuadros   distancia ");
        Serial.print(m->xMin); Serial.print(" a "); Serial.print(m->xMax);
        Serial.print(" cm   angulo "); Serial.print(m->angMin, 0);
        Serial.print(" a "); Serial.print(m->angMax, 0); Serial.println(" grados");
      }
    }
    Serial.print("    PELOTA: ");
    if (cuadrosPelota == 0) {
      Serial.println("no la vio (normal si no habia pelota en la cancha)");
    } else {
      Serial.print(cuadrosPelota * 100 / (totalPelota > 0 ? totalPelota : 1));
      Serial.print("% de los cuadros   distancia ");
      Serial.print(xpMin); Serial.print(" a "); Serial.print(xpMax); Serial.println(" cm");
    }

    if (medAm.cuadros > 0 && medAz.cuadros > 0)
      Serial.println("    VEREDICTO: ve LOS DOS arcos. La eleccion de arco al encender va a funcionar.");
    else if (medAm.cuadros > 0 || medAz.cuadros > 0)
      Serial.println("    VEREDICTO: ve UNO SOLO. Con el otro, el robot va a orbitar y rendirse.");
    else
      Serial.println("    VEREDICTO: no vio ningun arco. Hay que calibrar los colores de la camara.");
  }

  // ---- 3. linea ----
  Serial.println();
  Serial.println(" 3) LINEA");
  Serial.println("    sensor         negro        verde        blanco     ->  UMBRAL");
  for (int i = 0; i < 3; i++) {
    Serial.print("    "); Serial.print(nombreSensor[i]); Serial.print("  ");
    Serial.print(negroMin[i]);  Serial.print("-"); Serial.print(negroMax[i]);
    Serial.print("     ");
    Serial.print(verdeMin[i]);  Serial.print("-"); Serial.print(verdeMax[i]);
    Serial.print("     ");
    Serial.print(blancoMin[i]); Serial.print("-"); Serial.print(blancoMax[i]);
    Serial.print("   ->  ");
    if (umbralPropuesto[i] < 0) Serial.println("NO SE PUEDE (el blanco no quedo arriba del verde)");
    else                        Serial.println(umbralPropuesto[i]);
  }

  Serial.println();
  bool todosBien = true;
  for (int i = 0; i < 3; i++) {
    if (umbralPropuesto[i] < 0) { todosBien = false; continue; }
    int margen = (blancoMax[i] - verdeMax[i]) / 2;
    Serial.print("    "); Serial.print(nombreSensor[i]);
    Serial.print("  margen a cada lado del umbral: "); Serial.print(margen);
    if (margen < MARGEN_MINIMO) { Serial.println("  <- POCO, umbral fragil"); todosBien = false; }
    else                          Serial.println("  OK");
  }

  Serial.println();
  if (todosBien) {
    Serial.println("    PARA COPIAR AL FIRMWARE (funciona/delantero, en el PANEL DE CONTROL):");
    Serial.print  ("    int UMBRAL_LINEA[3] = { ");
    for (int i = 0; i < 3; i++) {
      Serial.print(umbralPropuesto[i]);
      Serial.print(i < 2 ? ", " : " };");
    }
    Serial.println();
  } else {
    Serial.println("    NO COPIAR ESTOS UMBRALES TODAVIA: alguno quedo fragil o imposible.");
    Serial.println("    Volver a correr la fase 3 cruzando la linea mas lento, y revisar que");
    Serial.println("    el negro sea NEGRO de verdad y el verde sea el de ESTA cancha.");
  }

  Serial.println("=====================================================");
  Serial.println(" (para repetir: desenchufa y enchufa, o RESET)");
}


// ---------------------------------------------------------------------
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(19200);
  Serial1.begin(19200);
  while (!Serial && millis() < 3000) { }

  pinMode(PIN_VERSION_PLACA, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION_PLACA) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12;
  }

  Serial.println();
  Serial.println("#####################################################");
  Serial.println("#  CALIBRACION EN QUIETO                            #");
  Serial.println("#  linea + giroscopio + camara, una sola corrida     #");
  Serial.println("#  NO MUEVE LOS MOTORES: no se escapa.               #");
  Serial.println("#####################################################");
  Serial.println("Las fases arrancan solas, por tiempo. El LED parpadea");
  Serial.println("en la cuenta y queda FIJO mientras graba.");
  Serial.print  ("Placa (pin 32): "); Serial.println(versionPlaca);

  faseGiroscopio();
  faseCamara();
  faseLinea();

  listo = true;
  informe();
}

void loop() {
  // El informe se repite cada 3 s para poder leerlo DESPUES, enchufando el USB
  // en la mesa SIN APAGAR LA BATERIA. Si se corta la energia, se pierde.
  static unsigned long t = 0;
  if (!listo) return;
  if (millis() - t < 3000) return;
  t = millis();
  informe();
}
