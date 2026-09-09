/* =====================================================================
   MEDIR VERDE — cinco puntos de la cancha, y nada mas
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO TOCA LOS MOTORES. Ni un analogWrite.

   ---------------------------------------------------------------------
   POR QUE EXISTE
   ---------------------------------------------------------------------
   2026-09-08. Se cambiaron los umbrales a {663, 661, 725} con el verde
   medido en TRES puntos de la cancha, y el robot SIGUE dando falsos
   blancos. O sea: el verde llega mas arriba de lo que vimos en esos tres
   puntos.

   La sospecha no es el sensor — con los motores apagados los tres estan
   quietisimos (desvio 0,2 a 1,8 cuentas sobre 54.000 muestras). La
   sospecha es que TRES PUNTOS NO ALCANZAN para conocer el verde de la
   cancha. Por eso este programa mide CINCO, y no mide nada mas.

   El blanco no hace falta volver a medirlo: dio casi igual en la mesa y
   en la cancha (764/762/764 contra 763/757/762), y ademas los tres
   sensores tienen el mismo techo de ~766. El unico numero que se mueve
   de una superficie a otra es el verde.

   ---------------------------------------------------------------------
   COMO SE USA — monitor serie a 19200
   ---------------------------------------------------------------------
   Apoyalo sobre el verde de la cancha y apreta ENTER. Cinco veces, en
   cinco puntos BIEN SEPARADOS: las esquinas, el medio, cerca de un arco.
   Buscá a proposito los lugares donde el verde se vea MAS CLARO o le
   pegue la luz — el umbral tiene que aguantar el peor punto, no el
   promedio.

   Al final dice, para cada sensor, cuanto llego el verde y que umbral
   haria falta.
   ===================================================================== */

// El firmware no llama a analogReadResolution(): corre en el default del
// Teensy 4.1, 10 bits, 0..1023. Aca tampoco se toca, para comparar sin
// convertir nada.

const int PIN_VERSION = 32;
const int PIN_LED     = 13;

int  pinLinea[3];
const char* versionPlaca = "?";

// Lo que el robot tiene cargado AHORA (cambiado hoy 2026-09-08).
const int UMBRAL_ACTUAL[3] = { 663, 661, 725 };

// Blanco de la cancha, medido el 2026-09-08. Se usa solo para proponer
// el umbral nuevo; NO se vuelve a medir porque es el numero mas estable
// que tenemos (dio lo mismo en la mesa y en la cancha).
const int BLANCO_CANCHA[3] = { 763, 757, 762 };

const int PUNTOS = 5;
const unsigned long MS_POR_MUESTRA = 3000;
const long MUESTRAS_MINIMAS = 500;

struct Medicion {
  long   n;
  int    minimo;
  int    maximo;
  double suma;
  double sumaCuad;
};

Medicion punto[PUNTOS][3];

void reset(Medicion &m) {
  m.n = 0; m.minimo = 32767; m.maximo = -1; m.suma = 0; m.sumaCuad = 0;
}
void acumular(Medicion &m, int v) {
  m.n++;
  if (v < m.minimo) m.minimo = v;
  if (v > m.maximo) m.maximo = v;
  m.suma += v; m.sumaCuad += (double)v * v;
}
float media(const Medicion &m) { return m.n ? (float)(m.suma / m.n) : 0.0; }
float desvio(const Medicion &m) {
  if (m.n < 2) return 0.0;
  double mu = m.suma / m.n;
  double var = (m.sumaCuad / m.n) - (mu * mu);
  return var > 0 ? sqrt(var) : 0.0;
}

void vaciarSerie() { while (Serial.available()) Serial.read(); }

// Espera a la PERSONA, no al reloj. Mientras tanto muestra las lecturas
// en vivo, y avisa si en ese momento ya cruzaria el umbral.
void esperarEnter(int p) {
  Serial.println();
  Serial.print(">>> PUNTO "); Serial.print(p + 1); Serial.print(" de ");
  Serial.print(PUNTOS);
  Serial.println(" — apoyalo sobre el VERDE, en un punto NUEVO.");
  Serial.println(">>> Buscá los lugares mas claros o con mas luz.");
  Serial.println(">>> Cuando este listo, apreta ENTER.");
  vaciarSerie();
  unsigned long t = 0;
  while (!Serial.available()) {
    if (millis() - t > 400) {
      t = millis();
      Serial.print("      en vivo:  ");
      for (int i = 0; i < 3; i++) {
        int v = analogRead(pinLinea[i]);
        Serial.print("S"); Serial.print(i + 1); Serial.print("=");
        Serial.print(v);
        if (v >= UMBRAL_ACTUAL[i]) Serial.print("!");   // ya dispararia
        Serial.print("   ");
      }
      Serial.println();
    }
  }
  vaciarSerie();
}

void capturar(int p) {
  for (int i = 0; i < 3; i++) reset(punto[p][i]);
  Serial.println("    midiendo 3 s, no lo muevas...");
  unsigned long t0 = millis();
  while (millis() - t0 < MS_POR_MUESTRA) {
    for (int i = 0; i < 3; i++) acumular(punto[p][i], analogRead(pinLinea[i]));
  }
  Serial.print("    PUNTO "); Serial.print(p + 1); Serial.print(":  ");
  for (int i = 0; i < 3; i++) {
    Serial.print("S"); Serial.print(i + 1); Serial.print("=");
    Serial.print(media(punto[p][i]), 0);
    Serial.print(" ("); Serial.print(punto[p][i].minimo);
    Serial.print(".."); Serial.print(punto[p][i].maximo);
    Serial.print(", d="); Serial.print(desvio(punto[p][i]), 1);
    Serial.print(")");
    if (punto[p][i].maximo >= UMBRAL_ACTUAL[i]) Serial.print("  <<< DISPARA");
    Serial.print("   ");
  }
  Serial.println();
  Serial.print("        muestras: "); Serial.println(punto[p][0].n);
}

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }
  delay(300);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  pinMode(PIN_VERSION, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12;
  }

  Serial.println();
  Serial.println("===================================================");
  Serial.println(" MEDIR VERDE - 5 puntos de la cancha, nada mas");
  Serial.println(" NO mueve los motores.");
  Serial.println("===================================================");
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   pines "); Serial.print(pinLinea[0]);
  Serial.print(", "); Serial.print(pinLinea[1]);
  Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.print("Umbrales cargados AHORA: ");
  Serial.print(UMBRAL_ACTUAL[0]); Serial.print(" / ");
  Serial.print(UMBRAL_ACTUAL[1]); Serial.print(" / ");
  Serial.println(UMBRAL_ACTUAL[2]);
  Serial.println("Un ! al lado de la lectura = ese sensor YA dispararia.");
  Serial.println("Sensor 1 = izquierdo   2 = centro   3 = adelante");
  Serial.println("===================================================");
}

bool termine = false;

void loop() {
  if (termine) return;

  for (int p = 0; p < PUNTOS; p++) {
    esperarEnter(p);
    capturar(p);
  }

  // ---------------- VEREDICTO ----------------
  Serial.println();
  Serial.println("===================================================");
  Serial.println("        EL VERDE DE LA CANCHA, EN 5 PUNTOS");
  Serial.println("===================================================");

  int propuesto[3];
  bool hayDato = true;

  for (int i = 0; i < 3; i++) {
    int alto = -1, bajo = 32767, cualAlto = 0;
    long n = 0;
    int disparan = 0;
    for (int p = 0; p < PUNTOS; p++) {
      if (punto[p][i].maximo > alto) { alto = punto[p][i].maximo; cualAlto = p + 1; }
      if (punto[p][i].minimo < bajo) bajo = punto[p][i].minimo;
      n += punto[p][i].n;
      if (punto[p][i].maximo >= UMBRAL_ACTUAL[i]) disparan++;
    }

    Serial.println();
    Serial.print("---- SENSOR "); Serial.print(i + 1);
    Serial.println(i == 0 ? "  (izquierdo) ----" : (i == 1 ? "  (centro) ----" : "  (adelante) ----"));

    if (n < MUESTRAS_MINIMAS * PUNTOS) {
      Serial.println("  SIN DATO - no junte muestras suficientes.");
      hayDato = false; propuesto[i] = -1;
      continue;
    }

    Serial.print("  verde: "); Serial.print(bajo); Serial.print("..");
    Serial.print(alto);
    Serial.print("   (se movio "); Serial.print(alto - bajo);
    Serial.println(" cuentas entre los 5 puntos)");
    Serial.print("  el mas claro fue el punto "); Serial.println(cualAlto);

    Serial.print("  umbral de hoy: "); Serial.print(UMBRAL_ACTUAL[i]);
    Serial.print("   -> margen "); Serial.print(UMBRAL_ACTUAL[i] - alto);
    if (disparan > 0) {
      Serial.print("   *** DISPARA EN FALSO EN ");
      Serial.print(disparan); Serial.println(" DE LOS 5 PUNTOS ***");
    } else {
      Serial.println("   (no disparo en ninguno)");
    }

    // Umbral nuevo: punto medio entre el verde MAS ALTO y el blanco.
    int sep = BLANCO_CANCHA[i] - alto;
    if (sep <= 0) {
      Serial.print("  >>> NO SIRVE: el verde ("); Serial.print(alto);
      Serial.print(") llega al blanco ("); Serial.print(BLANCO_CANCHA[i]);
      Serial.println("). Ningun umbral separa.");
      Serial.println("      Este sensor hay que subirlo de altura. No es un numero.");
      propuesto[i] = -1; hayDato = false;
      continue;
    }
    propuesto[i] = alto + sep / 2;
    Serial.print("  UMBRAL PROPUESTO: "); Serial.print(propuesto[i]);
    Serial.print("   (blanco "); Serial.print(BLANCO_CANCHA[i]);
    Serial.print(", margen al verde "); Serial.print(sep / 2); Serial.println(")");

    if (sep / 2 <= (alto - bajo)) {
      Serial.println("  >>> OJO: el margen es MENOR que lo que el verde se mueve");
      Serial.println("      entre puntos. Va a volver a disparar en falso en algun");
      Serial.println("      lado. El arreglo de fondo es la altura del sensor.");
    }
  }

  Serial.println();
  Serial.println("===================================================");
  if (hayDato) {
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

  termine = true;
}
