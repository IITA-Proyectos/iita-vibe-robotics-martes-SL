/* =====================================================================
   GRABAR VERDE — 5 puntos de la cancha, sin cable
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO TOCA LOS MOTORES. Ni un analogWrite.

   ---------------------------------------------------------------------
   POR QUE EXISTE
   ---------------------------------------------------------------------
   2026-09-08. Se cambiaron los umbrales a {663, 661, 725} con el verde
   medido en TRES puntos de la cancha, y el robot SIGUE dando falsos
   blancos. O sea: hay puntos del verde MAS CLAROS que los tres que
   medimos. Tres puntos no alcanzan.

   No hace falta volver a medir el blanco: dio casi igual en la mesa y en
   la cancha (764/762/764 contra 763/757/762) y los tres sensores tienen
   el mismo techo de ~766. El unico numero que cambia de una superficie a
   otra es el verde. Por eso este programa mide SOLO EL VERDE.

   Y no hace falta el cable: no llega a la cancha. Graba con la bateria.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Cargalo con el cable. Apenas termina, EMPIEZA A GRABAR.
      El LED de la placa (pin 13) parpadea: mientras parpadea, graba.

   2. Desenchufa el USB. LA BATERIA QUEDA PRENDIDA.
      🔋 Esto no es un detalle: si se corta la alimentacion el Teensy se
      reinicia y se pierde TODO lo grabado. La corrida buena del 08/09
      sobrevivio por la bateria; la que se perdio fue la que estaba
      alimentada solo por USB.

   3. En la cancha, apoyalo sobre el verde en CINCO PUNTOS distintos,
      quieto ~5 segundos en cada uno, moviendolo entre uno y otro.
      Buscá A PROPOSITO los peores: donde el verde se vea mas claro,
      donde pegue la luz directa, las esquinas, cerca de las lineas.
      El umbral tiene que aguantar el punto mas brillante de la cancha.

   4. Volve y enchufa el USB. SIGUE GRABANDO: no vuelca solo. Recien
      cuando le mandan una tecla vuelca y para.

   Las posiciones se reconocen como MESETAS (tramos quieto) y se numeran
   en orden. No hay reloj que cumplir: si una sale mal, repetila.
   ===================================================================== */

const int PIN_VERSION = 32;
const int PIN_LED     = 13;

int  pinLinea[3];
const char* versionPlaca = "?";

// Lo que el robot tiene cargado AHORA (cambiado el 2026-09-08).
const int UMBRAL_ACTUAL[3] = { 663, 661, 725 };

// Blanco de la cancha, medido el 2026-09-08 con 54.000 muestras por
// sensor. Se usa para proponer el umbral nuevo. Es el numero mas estable
// que tenemos: dio lo mismo en la mesa y en la cancha.
const int BLANCO_CANCHA[3] = { 763, 757, 762 };

const unsigned long MS_BLOQUE   = 100;
const int           MAX_BLOQUES = 6000;      // 10 minutos

struct Bloque {
  uint16_t minimo[3];
  uint16_t maximo[3];
  uint16_t media[3];
};

DMAMEM Bloque traza[MAX_BLOQUES];
int    nBloques = 0;

unsigned long t_bloque = 0;
long     nMuestras = 0;
uint32_t sumaBloque[3];
uint16_t minBloque[3], maxBloque[3];

bool yaVolque = false;

const int QUIETO_MAX      = 45;
const int DERIVA_MAX      = 45;
const int BLOQUES_MINIMOS = 20;   // 2 segundos

// Peor verde visto en una meseta valida, por sensor. Se llena al volcar.
int verdeAlto[3] = { -1, -1, -1 };
int verdeBajo[3] = { 32767, 32767, 32767 };
int disparan[3]  = { 0, 0, 0 };
int mesetasVerdes = 0;

void arrancarBloque() {
  nMuestras = 0;
  for (int i = 0; i < 3; i++) { sumaBloque[i] = 0; minBloque[i] = 65535; maxBloque[i] = 0; }
}

void setup() {
  Serial.begin(19200);
  pinMode(PIN_LED, OUTPUT);

  pinMode(PIN_VERSION, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12;
  }

  arrancarBloque();
  t_bloque = millis();
}

void volcar() {
  Serial.println();
  Serial.println("===================================================");
  Serial.println(" GRABAR VERDE - el verde de la cancha, 5 puntos");
  Serial.println("===================================================");
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   pines "); Serial.print(pinLinea[0]);
  Serial.print(", "); Serial.print(pinLinea[1]);
  Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.print("Umbrales cargados AHORA: ");
  Serial.print(UMBRAL_ACTUAL[0]); Serial.print(" / ");
  Serial.print(UMBRAL_ACTUAL[1]); Serial.print(" / ");
  Serial.println(UMBRAL_ACTUAL[2]);
  Serial.print("Grabados "); Serial.print(nBloques);
  Serial.print(" bloques de 100 ms = ");
  Serial.print(nBloques / 10.0, 1); Serial.println(" segundos");
  Serial.println("Sensor 1 = izquierdo   2 = centro   3 = adelante");

  if (nBloques < BLOQUES_MINIMOS) {
    Serial.println();
    Serial.println("!!! CASI NO HAY GRABACION. Si se corto la bateria al");
    Serial.println("!!! desenchufar, el robot se reinicio y se perdio todo.");
    Serial.println("===================================================");
    return;
  }

  Serial.println();
  Serial.println("--- rango de TODA la corrida ---");
  for (int i = 0; i < 3; i++) {
    uint16_t lo = 65535, hi = 0;
    for (int b = 0; b < nBloques; b++) {
      if (traza[b].minimo[i] < lo) lo = traza[b].minimo[i];
      if (traza[b].maximo[i] > hi) hi = traza[b].maximo[i];
    }
    Serial.print("  S"); Serial.print(i + 1); Serial.print(": ");
    Serial.print(lo); Serial.print(" .. "); Serial.println(hi);
  }

  Serial.println();
  Serial.println("--- MESETAS (el robot estuvo quieto) ---");
  Serial.println("Las primeras 5 tendrian que ser los 5 puntos de verde.");
  Serial.println("Lo que venga despues es el viaje de vuelta: NO es verde.");
  Serial.println();

  int meseta = 0, b = 0;
  while (b < nBloques) {
    bool quieto = true;
    for (int i = 0; i < 3; i++)
      if (traza[b].maximo[i] - traza[b].minimo[i] > QUIETO_MAX) quieto = false;
    if (!quieto) { b++; continue; }

    int ini = b;
    uint32_t suma[3] = {0, 0, 0};
    uint16_t lo[3] = {65535, 65535, 65535}, hi[3] = {0, 0, 0};
    int n = 0;
    while (b < nBloques) {
      bool ok = true;
      for (int i = 0; i < 3; i++) {
        if (traza[b].maximo[i] - traza[b].minimo[i] > QUIETO_MAX) ok = false;
        if (n > 0) {
          int refe = suma[i] / n;
          if (abs((int)traza[b].media[i] - refe) > DERIVA_MAX) ok = false;
        }
      }
      if (!ok) break;
      for (int i = 0; i < 3; i++) {
        suma[i] += traza[b].media[i];
        if (traza[b].minimo[i] < lo[i]) lo[i] = traza[b].minimo[i];
        if (traza[b].maximo[i] > hi[i]) hi[i] = traza[b].maximo[i];
      }
      n++; b++;
    }
    if (n < BLOQUES_MINIMOS) continue;

    meseta++;
    Serial.print("MESETA "); Serial.print(meseta);
    Serial.print("   t="); Serial.print(ini / 10.0, 1);
    Serial.print(" s   duro "); Serial.print(n / 10.0, 1); Serial.println(" s");
    for (int i = 0; i < 3; i++) {
      int prom = suma[i] / n;
      Serial.print("    S"); Serial.print(i + 1);
      Serial.print(" = "); Serial.print(prom);
      Serial.print("   (visto "); Serial.print(lo[i]);
      Serial.print(".."); Serial.print(hi[i]); Serial.print(")");
      if (hi[i] >= UMBRAL_ACTUAL[i]) Serial.print("   <<< DISPARA CON EL UMBRAL DE HOY");
      Serial.println();
    }

    // Solo las 5 primeras cuentan como verde. Lo de despues es el viaje.
    if (meseta <= 5) {
      mesetasVerdes = meseta;
      for (int i = 0; i < 3; i++) {
        if (hi[i] > verdeAlto[i]) verdeAlto[i] = hi[i];
        if (lo[i] < verdeBajo[i]) verdeBajo[i] = lo[i];
        if (hi[i] >= UMBRAL_ACTUAL[i]) disparan[i]++;
      }
    }
    Serial.println();
  }

  if (meseta == 0) {
    Serial.println("NO ENCONTRE NINGUNA MESETA. Hace falta dejarlo quieto al");
    Serial.println("menos 2 segundos en cada punto. Repetir apoyandolo.");
    Serial.println("===================================================");
    return;
  }

  // ---------------- VEREDICTO ----------------
  Serial.println("===================================================");
  Serial.print("     VEREDICTO — sobre las primeras ");
  Serial.print(mesetasVerdes); Serial.println(" mesetas");
  Serial.println("===================================================");

  if (mesetasVerdes < 5) {
    Serial.print("OJO: esperaba 5 puntos de verde y encontre ");
    Serial.print(mesetasVerdes); Serial.println(".");
    Serial.println("El veredicto sale igual, pero con menos puntos que los pedidos.");
  }

  int propuesto[3];
  bool todosOk = true;

  for (int i = 0; i < 3; i++) {
    Serial.println();
    Serial.print("---- SENSOR "); Serial.print(i + 1);
    Serial.println(i == 0 ? "  (izquierdo) ----" : (i == 1 ? "  (centro) ----" : "  (adelante) ----"));
    Serial.print("  verde: "); Serial.print(verdeBajo[i]); Serial.print("..");
    Serial.print(verdeAlto[i]);
    Serial.print("   (se movio "); Serial.print(verdeAlto[i] - verdeBajo[i]);
    Serial.println(" cuentas entre puntos)");

    Serial.print("  umbral de hoy "); Serial.print(UMBRAL_ACTUAL[i]);
    Serial.print(" -> margen "); Serial.print(UMBRAL_ACTUAL[i] - verdeAlto[i]);
    if (disparan[i] > 0) {
      Serial.print("   *** DISPARA EN FALSO EN ");
      Serial.print(disparan[i]); Serial.print(" DE ");
      Serial.print(mesetasVerdes); Serial.println(" PUNTOS ***");
    } else {
      Serial.println("   (no disparo en ninguno)");
    }

    int sep = BLANCO_CANCHA[i] - verdeAlto[i];
    if (sep <= 0) {
      Serial.print("  >>> NO SIRVE: el verde llega a "); Serial.print(verdeAlto[i]);
      Serial.print(" y el blanco es "); Serial.print(BLANCO_CANCHA[i]);
      Serial.println(".");
      Serial.println("      Ningun umbral los separa. Hay que SUBIR el sensor.");
      propuesto[i] = -1; todosOk = false;
      continue;
    }
    propuesto[i] = verdeAlto[i] + sep / 2;
    Serial.print("  UMBRAL PROPUESTO: "); Serial.print(propuesto[i]);
    Serial.print("   (blanco "); Serial.print(BLANCO_CANCHA[i]);
    Serial.print(", margen "); Serial.print(sep / 2); Serial.println(")");

    if (sep / 2 <= (verdeAlto[i] - verdeBajo[i])) {
      Serial.println("  >>> OJO: el margen es MENOR que lo que el verde se mueve");
      Serial.println("      entre puntos. Va a volver a disparar en algun lado.");
      Serial.println("      El arreglo de fondo es la ALTURA del sensor.");
    }
  }

  Serial.println();
  Serial.println("===================================================");
  if (todosOk) {
    Serial.println("Para pegar en funciona/delantero/delantero.ino:");
    Serial.print("int UMBRAL_LINEA[3] = { ");
    Serial.print(propuesto[0]); Serial.print(", ");
    Serial.print(propuesto[1]); Serial.print(", ");
    Serial.print(propuesto[2]); Serial.println(" };");
  } else {
    Serial.println("NO doy linea para copiar: al menos un sensor no cerro.");
  }
  Serial.println("Anotalo en la bitacora CON LOS NUMEROS.");
  Serial.println("===================================================");
}

void loop() {
  if (yaVolque) return;

  // Vuelca SOLO cuando se lo piden. No adivina si el cable volvio: el
  // Teensy no puede distinguir "me desenchufaron" de "nadie abrio el
  // monitor", y si volcara al detectar el cable, cualquiera que espie el
  // puerto le corta la grabacion a la mitad.
  if (Serial && Serial.available()) {
    while (Serial.available()) Serial.read();
    delay(400);
    digitalWrite(PIN_LED, HIGH);
    volcar();
    yaVolque = true;
    return;
  }

  if (nBloques >= MAX_BLOQUES) { digitalWrite(PIN_LED, LOW); return; }

  for (int i = 0; i < 3; i++) {
    uint16_t v = analogRead(pinLinea[i]);
    sumaBloque[i] += v;
    if (v < minBloque[i]) minBloque[i] = v;
    if (v > maxBloque[i]) maxBloque[i] = v;
  }
  nMuestras++;

  if (millis() - t_bloque >= MS_BLOQUE) {
    t_bloque = millis();
    if (nMuestras > 0) {
      for (int i = 0; i < 3; i++) {
        traza[nBloques].minimo[i] = minBloque[i];
        traza[nBloques].maximo[i] = maxBloque[i];
        traza[nBloques].media[i]  = sumaBloque[i] / nMuestras;
      }
      nBloques++;
    }
    arrancarBloque();
    digitalWrite(PIN_LED, (nBloques / 5) % 2);
  }
}
