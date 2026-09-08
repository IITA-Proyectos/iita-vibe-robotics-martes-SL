/* =====================================================================
   CALIBRAR LINEA — verde, blanco y negro, con veredicto
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO TOCA LOS MOTORES. Ni un analogWrite. Se corre con el robot en la
   mano o apoyado, enchufado al USB, con la bateria APAGADA.

   ---------------------------------------------------------------------
   PARA QUE ES
   ---------------------------------------------------------------------
   Para decidir los tres numeros de UMBRAL_LINEA[] con la luz, la cancha
   y el montaje de HOY. Y sobre todo para contestar la pregunta que
   aparecio el 2026-09-08: por que el robot ve blancos que no existen.

   ---------------------------------------------------------------------
   LAS CUATRO TRAMPAS QUE ESQUIVA (todas ya documentadas)
   ---------------------------------------------------------------------
   1) EL NEGRO NO ENTRA EN LA CUENTA DEL UMBRAL.
      pruebas/sensores-de-linea/ propone el umbral como (min+max)/2 de
      TODO lo que vio. Si en la corrida se midio negro, propone ~420,
      que esta POR DEBAJO del verde: el robot escaparia parado en medio
      de la cancha. El umbral se calcula VERDE<->BLANCO y nada mas.
      [bitacora 2026-08-25]

   2) EL VERDE NO ES UN NUMERO, ES UN RANGO.
      Dos puntos de la misma cancha dieron -55 / -71 / -111 de
      diferencia. Por eso el verde se mide en TRES PUNTOS distintos y el
      umbral se calcula contra el verde MAS ALTO, no contra el promedio.
      [bitacora 2026-08-25]

   3) EL PROGRAMA ESPERA A LA PERSONA, NO AL RELOJ.
      Dos clases seguidas se perdieron porque el aviso salia por el
      monitor y el equipo miraba el robot. Aca ninguna fase arranca sola:
      cada una espera que aprietes ENTER. [bitacoras 2026-08-25, 09-01]

   4) SI FALTA UNA MEDICION, NO HAY VEREDICTO.
      Las tres primeras corridas de pruebas/signos/ concluyeron a partir
      de una medicion en cero, y el veredicto salio al reves del
      correcto. Aca, si una fase no junto muestras, ese sensor se declara
      SIN DATO y no se propone umbral. [bitacora 2026-09-01]

   ---------------------------------------------------------------------
   COMO SE USA — monitor serie a 19200
   ---------------------------------------------------------------------
   El programa te va pidiendo cada paso y espera ENTER. En total:

     VERDE   x3 puntos distintos de la cancha (no de la mesa: los
             umbrales medidos en la mesa YA fallaron en la cancha
             el 25/08)
     BLANCO  x3, un sensor por vez sobre la linea
     NEGRO   x1, los tres sobre el negro

   Al final imprime la linea lista para copiar y pegar en el firmware.

   IMPORTANTE: el umbral se mide donde el robot va a jugar. Si el cable
   no llega a la cancha, hay que acercar la cancha o alargar el cable:
   medir en la mesa y esperar que transfiera ya fallo una vez.
   ===================================================================== */

// El firmware NO llama a analogReadResolution(), asi que corre en el
// default del Teensy 4.1: 10 bits, 0..1023. Aca tampoco se toca, para
// que estos numeros se puedan comparar contra los del firmware sin
// convertir nada.

const int PIN_VERSION = 32;

int  pinLinea[3];
const char* versionPlaca = "?";

// Lo que el robot tiene cargado HOY, para comparar al final.
const int UMBRAL_ACTUAL[3] = { 707, 582, 795 };

const unsigned long MS_POR_MUESTRA = 3000;  // cuanto dura cada captura
const long MUESTRAS_MINIMAS = 500;          // menos que esto: no hay dato

// Cuantos puntos distintos de verde se miden.
//
// EN LA CANCHA VAN 3, y no es un capricho: el verde se movio 111 cuentas
// entre dos puntos de la misma cancha [bitacora 2026-08-25], y el umbral
// tiene que aguantar el punto MAS CLARO, no el que nos toco medir.
//
// En la mesa se puede bajar a 1 porque no hay espacio para separar los
// puntos. El programa entonces NO puede medir cuanto varia el verde, y lo
// dice en el veredicto en vez de callarselo. Un umbral sacado de un solo
// punto de mesa NO sirve para jugar. [2026-09-08]
const int PUNTOS_VERDE = 1;

struct Medicion {
  long   n;
  int    minimo;
  int    maximo;
  double suma;
  double sumaCuad;
};

Medicion verde[3][3];   // [punto][sensor]
Medicion blanco[3][3];  // [paso][sensor]  — en el paso i interesa el sensor i
Medicion negro[3];      // [sensor]

void reset(Medicion &m) {
  m.n = 0; m.minimo = 32767; m.maximo = -1; m.suma = 0; m.sumaCuad = 0;
}

void acumular(Medicion &m, int v) {
  m.n++;
  if (v < m.minimo) m.minimo = v;
  if (v > m.maximo) m.maximo = v;
  m.suma     += v;
  m.sumaCuad += (double)v * v;
}

float media(const Medicion &m) { return m.n ? (float)(m.suma / m.n) : 0.0; }

float desvio(const Medicion &m) {
  if (m.n < 2) return 0.0;
  double mu  = m.suma / m.n;
  double var = (m.sumaCuad / m.n) - (mu * mu);
  return var > 0 ? sqrt(var) : 0.0;
}

// ---------------------------------------------------------------------

void vaciarSerie() { while (Serial.available()) Serial.read(); }

// Espera a la PERSONA. Mientras tanto muestra las lecturas en vivo, asi
// se ve que el robot esta donde tiene que estar ANTES de capturar.
void esperarEnter(const char* queHacer) {
  Serial.println();
  Serial.print(">>> "); Serial.println(queHacer);
  Serial.println(">>> Cuando este listo, apreta ENTER en el monitor.");
  vaciarSerie();
  unsigned long t = 0;
  while (!Serial.available()) {
    if (millis() - t > 400) {
      t = millis();
      Serial.print("      en vivo:  S1="); Serial.print(analogRead(pinLinea[0]));
      Serial.print("   S2=");              Serial.print(analogRead(pinLinea[1]));
      Serial.print("   S3=");              Serial.print(analogRead(pinLinea[2]));
      Serial.println();
    }
  }
  vaciarSerie();
}

// Captura los tres sensores lo mas rapido que puede durante MS_POR_MUESTRA.
// Rapido a proposito: asi entran los PICOS, que es lo que sospechamos que
// dispara el escape en falso.
void capturar(Medicion destino[3], const char* rotulo) {
  for (int i = 0; i < 3; i++) reset(destino[i]);
  Serial.print("    midiendo "); Serial.print(rotulo);
  Serial.print(" ("); Serial.print(MS_POR_MUESTRA / 1000);
  Serial.println(" s, no lo muevas)...");

  unsigned long t0 = millis();
  while (millis() - t0 < MS_POR_MUESTRA) {
    for (int i = 0; i < 3; i++) acumular(destino[i], analogRead(pinLinea[i]));
  }

  Serial.print("    ->  ");
  for (int i = 0; i < 3; i++) {
    Serial.print("S"); Serial.print(i + 1); Serial.print(": ");
    Serial.print(media(destino[i]), 0);
    Serial.print(" ("); Serial.print(destino[i].minimo);
    Serial.print(".."); Serial.print(destino[i].maximo);
    Serial.print(", desvio "); Serial.print(desvio(destino[i]), 1);
    Serial.print(")   ");
  }
  Serial.println();
  Serial.print("        muestras: "); Serial.println(destino[0].n);
}

// ---------------------------------------------------------------------

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }
  delay(300);

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
  Serial.println(" CALIBRAR LINEA - verde, blanco y negro");
  Serial.println(" NO mueve los motores. Bateria APAGADA.");
  Serial.println("===================================================");
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   pines "); Serial.print(pinLinea[0]);
  Serial.print(", "); Serial.print(pinLinea[1]);
  Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.print("Umbrales cargados hoy en el robot: ");
  Serial.print(UMBRAL_ACTUAL[0]); Serial.print(" / ");
  Serial.print(UMBRAL_ACTUAL[1]); Serial.print(" / ");
  Serial.println(UMBRAL_ACTUAL[2]);
  Serial.println();
  Serial.println("Sensor 1 = izquierdo   2 = centro   3 = adelante");
  Serial.println("Se mide en la CANCHA, no en la mesa.");
  Serial.println("===================================================");
}

bool termine = false;

void loop() {
  if (termine) return;

  // ---------- VERDE, en tres puntos distintos ----------
  Serial.println();
  Serial.println("###################  VERDE  ###################");
  if (PUNTOS_VERDE >= 3) {
    Serial.println("El verde no es un numero, es un rango: entre dos");
    Serial.println("puntos de la misma cancha ya cambio 111 cuentas.");
    Serial.println("Por eso van TRES puntos BIEN SEPARADOS.");
  } else {
    Serial.print("Configurado en "); Serial.print(PUNTOS_VERDE);
    Serial.println(" punto(s) de verde. OJO: con menos de 3 no se");
    Serial.println("puede medir cuanto varia el verde de un lado a otro.");
  }

  const char* dondeVerde[3] = {
    "Apoyalo sobre el VERDE, en un punto cualquiera.",
    "Movelo a OTRO punto del verde, lejos del anterior.",
    "Y a un TERCER punto del verde, lo mas lejos que puedas."
  };
  for (int p = 0; p < PUNTOS_VERDE; p++) {
    esperarEnter(dondeVerde[p]);
    capturar(verde[p], "VERDE");
  }

  // ---------- BLANCO, un sensor por vez ----------
  Serial.println();
  Serial.println("###################  BLANCO  ###################");
  Serial.println("Un sensor por vez sobre la linea. Es el metodo");
  Serial.println("diferencial: la pregunta facil es 'cual salta'.");

  const char* dondeBlanco[3] = {
    "Pone SOLO el sensor 1 (IZQUIERDO) sobre la LINEA BLANCA.",
    "Ahora SOLO el sensor 2 (CENTRO) sobre la LINEA BLANCA.",
    "Ahora SOLO el sensor 3 (ADELANTE) sobre la LINEA BLANCA."
  };
  for (int p = 0; p < 3; p++) {
    esperarEnter(dondeBlanco[p]);
    capturar(blanco[p], "BLANCO");
    // Control cruzado: el que mas subio respecto del verde deberia ser
    // el que pusimos sobre la linea. Si no, el sensor no es el que creemos.
    int cual = 0; float mejor = -99999;
    for (int i = 0; i < 3; i++) {
      float sube = media(blanco[p][i]) - media(verde[0][i]);
      if (sube > mejor) { mejor = sube; cual = i; }
    }
    Serial.print("        el que mas subio fue S"); Serial.print(cual + 1);
    if (cual != p) {
      Serial.print("  <<< OJO: esperabamos S"); Serial.print(p + 1);
      Serial.print(". O el sensor no es el que creemos, o hay mas de uno sobre el blanco.");
    }
    Serial.println();
  }

  // ---------- NEGRO ----------
  Serial.println();
  Serial.println("###################  NEGRO  ###################");
  Serial.println("El negro NO se usa para el umbral. Sirve para ver");
  Serial.println("el rango completo del sensor y si alguno esta muerto.");
  esperarEnter("Apoya los TRES sensores sobre el NEGRO.");
  capturar(negro, "NEGRO");

  // ---------- VEREDICTO ----------
  Serial.println();
  Serial.println();
  Serial.println("===================================================");
  Serial.println("                   VEREDICTO");
  Serial.println("===================================================");

  int  propuesto[3];
  bool sirve[3];

  for (int i = 0; i < 3; i++) {
    propuesto[i] = -1;
    sirve[i] = false;

    // Peor caso de cada lado: el verde MAS ALTO que vimos en cualquier
    // punto, y el blanco MAS BAJO. Asi el umbral aguanta la cancha real,
    // no el punto que nos toco medir.
    int  verdeAlto = -1, verdeBajo = 32767;
    long nVerde = 0;
    for (int p = 0; p < PUNTOS_VERDE; p++) {
      if (verde[p][i].maximo > verdeAlto) verdeAlto = verde[p][i].maximo;
      if (verde[p][i].minimo < verdeBajo) verdeBajo = verde[p][i].minimo;
      nVerde += verde[p][i].n;
    }
    int  blancoBajo = blanco[i][i].minimo;
    long nBlanco    = blanco[i][i].n;

    Serial.println();
    Serial.print("---- SENSOR "); Serial.print(i + 1);
    Serial.print(i == 0 ? "  (izquierdo)" : (i == 1 ? "  (centro)" : "  (adelante)"));
    Serial.println(" ----");

    // Trampa 4: sin muestras no hay veredicto.
    if (nVerde < MUESTRAS_MINIMAS * PUNTOS_VERDE || nBlanco < MUESTRAS_MINIMAS) {
      Serial.println("  SIN DATO - no junte muestras suficientes. NO propongo umbral.");
      Serial.print("  (verde n="); Serial.print(nVerde);
      Serial.print(", blanco n="); Serial.print(nBlanco); Serial.println(")");
      continue;
    }

    int variacionVerde = verdeAlto - verdeBajo;
    int separacion     = blancoBajo - verdeAlto;

    Serial.print("  verde:  "); Serial.print(verdeBajo); Serial.print("..");
    Serial.print(verdeAlto);
    if (PUNTOS_VERDE >= 3) {
      Serial.print("   (se movio "); Serial.print(variacionVerde);
      Serial.println(" cuentas entre los tres puntos)");
    } else {
      Serial.print("   (un solo punto: esto es el ruido de la lectura,");
      Serial.println(" NO cuanto cambia el verde de un lado a otro)");
    }
    int blancoAlto = blanco[i][i].maximo;
    Serial.print("  blanco: "); Serial.print(blancoBajo);
    Serial.print(".."); Serial.println(blancoAlto);
    Serial.print("  negro:  "); Serial.println(media(negro[i]), 0);
    Serial.print("  separacion verde->blanco: "); Serial.println(separacion);

    // EL CHEQUEO QUE FALTABA (2026-09-08): si el sensor NUNCA llega al
    // umbral que tiene cargado, no es que dispare de mas o de menos —
    // es que NO PUEDE DISPARAR NUNCA. Con la linea blanca debajo y el
    // sensor saturado, sigue por debajo del umbral. El sensor 3 estaba
    // asi: techo 765 contra un umbral de 795.
    if (blancoAlto < UMBRAL_ACTUAL[i]) {
      Serial.println("  >>> EL UMBRAL DE HOY ES INALCANZABLE.");
      Serial.print("      Con el blanco DEBAJO este sensor llega a ");
      Serial.print(blancoAlto); Serial.print(" y su umbral es ");
      Serial.print(UMBRAL_ACTUAL[i]); Serial.println(".");
      Serial.println("      O sea que HOY este sensor no detecta la linea nunca.");
    }

    if (separacion <= 0) {
      Serial.println("  >>> NO SIRVE: el blanco no supera al verde. Este sensor no");
      Serial.println("      distingue nada. Es altura o montaje, no es un numero.");
      continue;
    }

    propuesto[i] = verdeAlto + separacion / 2;
    int margen = propuesto[i] - verdeAlto;

    Serial.print("  UMBRAL PROPUESTO: "); Serial.print(propuesto[i]);
    Serial.print("   (margen al verde: "); Serial.print(margen); Serial.println(")");

    // La prueba de fuego: el margen tiene que ser mas grande que lo que
    // el verde se mueve solo. Si no, en algun punto de la cancha va a dar
    // blanco sin que haya blanco. Eso es un FALSO BLANCO.
    float ruido = desvio(verde[0][i]);

    // Con un solo punto de verde no hay con que comparar el margen: la
    // prueba de fuego ("el margen tiene que ser mas grande que lo que el
    // verde se mueve solo") necesita al menos dos puntos. Decirlo, en vez
    // de dar un OK que no se gano. En la cancha el verde se movio 115.
    if (PUNTOS_VERDE < 3) {
      Serial.println("  >>> OJO: medido en UN SOLO punto de verde.");
      Serial.print("      En cancha el verde se movio 115 cuentas entre puntos.");
      Serial.println("");
      Serial.print("      Contra eso, este margen de "); Serial.print(margen);
      Serial.println(margen > 115 ? " alcanzaria." : " NO alcanzaria.");
      Serial.println("      Hay que re-medirlo en la cancha antes de jugar.");
    }

    if (margen <= variacionVerde) {
      Serial.println("  >>> FALSOS BLANCOS: el margen es MENOR que lo que el verde");
      Serial.println("      se mueve solo entre puntos. Ningun umbral alcanza.");
      Serial.println("      Hay que levantar/bajar el sensor unos milimetros.");
      Serial.println("      (Al sensor 1 le paso: de 3 puntos de separacion paso a 446)");
    } else if (margen < 6 * ruido) {
      Serial.println("  >>> JUSTO: el margen es chico contra el ruido de la lectura.");
      Serial.println("      Anda, pero un pico lo cruza. Conviene filtrar en el firmware.");
      sirve[i] = true;
    } else {
      Serial.println("  >>> OK: margen comodo.");
      sirve[i] = true;
    }

    Serial.print("  (hoy tiene "); Serial.print(UMBRAL_ACTUAL[i]);
    Serial.print(" -> margen actual al verde: ");
    Serial.print(UMBRAL_ACTUAL[i] - verdeAlto); Serial.println(")");
  }

  Serial.println();
  Serial.println("===================================================");
  if (sirve[0] && sirve[1] && sirve[2]) {
    Serial.println("Para pegar en funciona/delantero/delantero.ino:");
    Serial.print("int UMBRAL_LINEA[3] = { ");
    Serial.print(propuesto[0]); Serial.print(", ");
    Serial.print(propuesto[1]); Serial.print(", ");
    Serial.print(propuesto[2]);
    Serial.println(" };");
  } else {
    Serial.println("NO doy una linea para copiar: al menos un sensor no cerro.");
    Serial.println("Arreglar eso primero. Un umbral inventado es peor que ninguno.");
  }
  Serial.println("Anotalo en la bitacora CON LOS NUMEROS.");
  Serial.println("Para repetir: RESET del Teensy (boton blanco).");
  Serial.println("===================================================");

  termine = true;
}
