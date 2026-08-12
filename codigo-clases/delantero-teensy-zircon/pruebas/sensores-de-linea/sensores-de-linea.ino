/* =====================================================================
   SENSORES DE LINEA — medir antes de creer
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO MUEVE LOS MOTORES. Es sólo para mirar numeros. Se puede correr con
   el robot en la mano, sin bateria, enchufado al USB.

   CONTESTA TRES PREGUNTAS, y las tres hacen falta antes de que el robot
   pueda escapar de la linea:

     1. Que version de placa es, y por lo tanto en que pines estan los
        sensores. Se autodetecta leyendo el pin 32, igual que la libreria
        Zircon (zirconLib.cpp:52-60).

     2. Cuanto lee cada sensor sobre el VERDE y cuanto sobre la LINEA
        BLANCA, HOY, con la luz de hoy. Los umbrales del 2025 (650/650/750)
        son de la luz del laboratorio del año pasado. Ya nos paso con los
        umbrales de color de la camara: no los repitamos a ciegas.

     3. Cual sensor es cual lado del robot. El codigo 2025 los comenta como
        "izquierdo / centro / derecho" y sus rotulos izquierda/derecha
        estan ESPEJADOS, asi que de eso no nos fiamos.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   Monitor serie a 19200.

   PASO 1 — apoyalo sobre el VERDE, quieto, 5 segundos.
            Anota los tres numeros. Ese es el piso.

   PASO 2 — pasa UN lado del robot por la LINEA BLANCA, despacio.
            El numero que salta es el sensor de ESE lado. Anotalo:
            ese es el dato que dice cual sensor es cual.
            Repetilo con los otros dos lados.

   PASO 3 — mira la tabla de MINIMO y MAXIMO que imprime. El umbral que
            conviene es el punto medio entre el verde y el blanco.
            El programa ya lo calcula solo.

   La pregunta facil es "¿cual salta?", no "¿cual es?". Un solo sensor
   cambiando mientras los otros dos no se mueven no se puede confundir.
   Es la misma idea con la que se mapearon las ruedas el 2026-07-28.
   ===================================================================== */

// ---- autodeteccion de la placa (igual que zirconLib.cpp:52-60) ----
const int PIN_VERSION = 32;

int L1, L2, L3;                 // los pines analogicos, se eligen en setup
const char* versionPlaca = "?";

// min y max vistos, para separar verde de blanco
int minimo[3] = {9999, 9999, 9999};
int maximo[3] = {-1, -1, -1};

unsigned long t_ultimoAviso = 0;

void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }

  pinMode(PIN_VERSION, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION) == LOW) {
    versionPlaca = "Mark1";
    L1 = A11; L2 = A13; L3 = A12;
  } else {
    versionPlaca = "Naveen1";
    L1 = A8;  L2 = A9;  L3 = A12;
  }

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" SENSORES DE LINEA — no mueve los motores");
  Serial.println("=================================================");
  Serial.print("Placa detectada (pin 32): "); Serial.println(versionPlaca);
  Serial.print("Pines de los sensores:  1="); Serial.print(L1);
  Serial.print("  2="); Serial.print(L2);
  Serial.print("  3="); Serial.println(L3);
  Serial.println();
  Serial.println("Umbrales del 2025 para este robot: 650 / 650 / 750");
  Serial.println("(de la luz del laboratorio 2025 — hay que re-medirlos)");
  Serial.println();
  Serial.println("PASO 1: apoyalo sobre el VERDE y esperá 5 segundos.");
  Serial.println("PASO 2: pasá UN lado por la LINEA BLANCA. El que salta");
  Serial.println("        es el sensor de ese lado. Repetí con los otros.");
  Serial.println("=================================================");
  Serial.println();
}

void loop() {
  int v[3];
  v[0] = analogRead(L1);
  v[1] = analogRead(L2);
  v[2] = analogRead(L3);

  for (int i = 0; i < 3; i++) {
    if (v[i] < minimo[i]) minimo[i] = v[i];
    if (v[i] > maximo[i]) maximo[i] = v[i];
  }

  // Cada 250 ms: rapido para ver el salto al pasar por la linea, pero
  // no tanto como para que no se pueda leer.
  if (millis() - t_ultimoAviso < 250) return;
  t_ultimoAviso = millis();

  Serial.print("S1="); Serial.print(v[0]);
  Serial.print("\tS2="); Serial.print(v[1]);
  Serial.print("\tS3="); Serial.print(v[2]);

  // Barra para el que mas subio respecto de su minimo: se ve de reojo
  // cual esta saltando sin leer los numeros.
  int cual = 0, mejor = v[0] - minimo[0];
  for (int i = 1; i < 3; i++) {
    if (v[i] - minimo[i] > mejor) { mejor = v[i] - minimo[i]; cual = i; }
  }
  Serial.print("\t  el que mas subio: S"); Serial.print(cual + 1);
  Serial.print(" (+"); Serial.print(mejor); Serial.print(")");
  Serial.println();

  Serial.print("        rango visto   ");
  for (int i = 0; i < 3; i++) {
    Serial.print("S"); Serial.print(i + 1); Serial.print(": ");
    Serial.print(minimo[i]); Serial.print("..."); Serial.print(maximo[i]);
    if (maximo[i] - minimo[i] > 50) {
      Serial.print(" -> umbral "); Serial.print((minimo[i] + maximo[i]) / 2);
    } else {
      Serial.print(" (todavia no vio blanco)");
    }
    Serial.print("   ");
  }
  Serial.println();
}
