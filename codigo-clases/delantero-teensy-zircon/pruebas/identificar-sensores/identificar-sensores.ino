/* =====================================================================
   IDENTIFICAR SENSORES — cual numero es cual posicion, MEDIDO
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO TOCA LOS MOTORES. Ni un analogWrite. En la mesa, enchufado.

   ---------------------------------------------------------------------
   LA PREGUNTA
   ---------------------------------------------------------------------
   UMBRAL_LINEA[0], [1] y [2] son "sensor 1, 2 y 3". Pero cual de esos
   numeros es el de ADELANTE, cual el IZQUIERDO y cual el DERECHO?

   No es una pregunta de rotulos: escaparDeLinea() decide para donde salir
   apagando una rueda, y si la posicion esta corrida el robot escapa para
   el lado equivocado.

   RESUELTO EL 2026-09-15 CON ESTE MISMO PROGRAMA:
       sensor 1 -> DERECHO     (A11, pin 25)
       sensor 2 -> IZQUIERDO   (A13, pin 27)
       sensor 3 -> DELANTERO   (A12, pin 26)

   Se apoyo un sensor por vez sobre el blanco con los otros dos en negro.
   Siempre salto UNO solo, ~700 cuentas, y los otros dos ni se movieron.
   El delantero dio +522 contra +24 del segundo: inconfundible.

   Antes de medir habia TRES versiones distintas dando vueltas — el
   dibujo del equipo, la bitacora del 18/08 y los comentarios del codigo
   2025 — y las tres decian cosas diferentes. Ninguna era la correcta.

   Y quedo claro que escaparDeLinea() SI estaba bien: cada sensor esta en
   un LADO del triangulo, entre dos ruedas, o sea enfrentado a la tercera.
   El escape apaga justo esa tercera. Lo que estaba mal eran los rotulos
   que leemos nosotros, no el comportamiento del robot.

   ---------------------------------------------------------------------
   POR QUE ESTE METODO Y NO OTRO
   ---------------------------------------------------------------------
   El 18/08 ya se hizo algo parecido y quedo a medias. Esa medicion probo
   bien QUE PIN ES QUE NUMERO (uno sobre blanco por vez, salta uno solo:
   eso no se puede confundir). Pero la POSICION FISICA se dedujo suponiendo
   que escaparDeLinea() ya estaba bien escrito — o sea que se dio por
   cierto justo lo que habia que verificar.

   Aca la persona dice PRIMERO que sensor fisico va a apoyar, y el programa
   contesta que numero salto. Esa es la vuelta que faltaba.

   La pregunta facil es "cual salta", no "cual es". Un sensor cambiando
   mientras los otros dos no se mueven no se puede confundir. Es el mismo
   metodo con el que se mapearon las ruedas el 28/07.

   ---------------------------------------------------------------------
   COMO SE USA — monitor serie a 19200
   ---------------------------------------------------------------------
   Hacen falta una muestra de BLANCO y una de NEGRO sobre la mesa.

     PASO 0 — los TRES sensores sobre el NEGRO. Es la linea de base.
     PASO 1 — el sensor de ADELANTE sobre el BLANCO, los otros dos en negro.
     PASO 2 — el IZQUIERDO sobre el blanco, los otros dos en negro.
     PASO 3 — el DERECHO sobre el blanco, los otros dos en negro.

   Si alguno no se puede apoyar solo, no pasa nada: el programa avisa que
   no pudo concluir en vez de inventar una respuesta.
   ===================================================================== */

const int PIN_VERSION = 32;

int  pinLinea[3];
const char* versionPlaca = "?";

// Lo que asume el codigo de hoy, para comparar al final.
// indice 0,1,2 = sensor 1,2,3
const char* SEGUN_EL_CODIGO[3] = { "DERECHO", "IZQUIERDO", "ADELANTE" };

// Cuanto tiene que subir un sensor para creerle que es el que esta sobre
// el blanco. Con el blanco en ~750 y el negro en ~70, una subida de
// verdad son ~600 cuentas: 200 es holgado y a la vez inconfundible.
const int SUBIDA_MINIMA = 200;

// Y cuanto tiene que sacarle al segundo. Si dos suben parecido es que
// habia dos sobre el blanco: no se concluye.
const int VENTAJA_MINIMA = 150;

const unsigned long MS_CAPTURA = 2500;

int base[3]    = { 0, 0, 0 };
int medido[3]  = { -1, -1, -1 };   // por posicion fisica: 0=adel 1=izq 2=der
const char* POSICION[3] = { "ADELANTE", "IZQUIERDO", "DERECHO" };

void vaciarSerie() { while (Serial.available()) Serial.read(); }

void esperarEnter(const char* queHacer) {
  Serial.println();
  Serial.print(">>> "); Serial.println(queHacer);
  Serial.println(">>> Cuando este listo, apreta ENTER.");
  vaciarSerie();
  unsigned long t = 0;
  while (!Serial.available()) {
    if (millis() - t > 400) {
      t = millis();
      Serial.print("      en vivo:  ");
      for (int i = 0; i < 3; i++) {
        Serial.print("S"); Serial.print(i + 1); Serial.print("=");
        Serial.print(analogRead(pinLinea[i])); Serial.print("   ");
      }
      Serial.println();
    }
  }
  vaciarSerie();
}

// Promedia MS_CAPTURA de lecturas y deja el resultado en destino[].
void capturar(int destino[3]) {
  double suma[3] = { 0, 0, 0 };
  long n = 0;
  Serial.println("    midiendo, no lo muevas...");
  unsigned long t0 = millis();
  while (millis() - t0 < MS_CAPTURA) {
    for (int i = 0; i < 3; i++) suma[i] += analogRead(pinLinea[i]);
    n++;
  }
  Serial.print("    ->  ");
  for (int i = 0; i < 3; i++) {
    destino[i] = (int)(suma[i] / n);
    Serial.print("S"); Serial.print(i + 1); Serial.print("=");
    Serial.print(destino[i]); Serial.print("   ");
  }
  Serial.println();
}

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
  Serial.println(" IDENTIFICAR SENSORES - cual numero es cual lugar");
  Serial.println(" NO mueve los motores.");
  Serial.println("===================================================");
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   pines "); Serial.print(pinLinea[0]);
  Serial.print(", "); Serial.print(pinLinea[1]);
  Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.println();
  Serial.println("Vos decis que sensor FISICO apoyas; el programa contesta");
  Serial.println("que NUMERO salto. Hacen falta una muestra de blanco y una");
  Serial.println("de negro.");
  Serial.println("===================================================");
}

bool termine = false;

void loop() {
  if (termine) return;

  esperarEnter("Poné los TRES sensores sobre el NEGRO (linea de base).");
  capturar(base);

  for (int p = 0; p < 3; p++) {
    char msg[110];
    snprintf(msg, sizeof(msg),
             "Poné SOLO el sensor de %s sobre el BLANCO. Los otros dos, en negro.",
             POSICION[p]);
    esperarEnter(msg);

    int v[3];
    capturar(v);

    // Cuanto subio cada uno respecto del negro
    int subida[3];
    Serial.print("        subida vs negro:  ");
    for (int i = 0; i < 3; i++) {
      subida[i] = v[i] - base[i];
      Serial.print("S"); Serial.print(i + 1); Serial.print("=+");
      Serial.print(subida[i]); Serial.print("   ");
    }
    Serial.println();

    // El que mas subio, y el segundo
    int primero = 0;
    for (int i = 1; i < 3; i++) if (subida[i] > subida[primero]) primero = i;
    int segundo = -1;
    for (int i = 0; i < 3; i++)
      if (i != primero && (segundo < 0 || subida[i] > subida[segundo])) segundo = i;

    if (subida[primero] < SUBIDA_MINIMA) {
      Serial.print("        NO CONCLUYO: el que mas subio apenas subio ");
      Serial.print(subida[primero]); Serial.println(" cuentas.");
      Serial.println("        Ese sensor no llego a apoyarse bien en el blanco.");
      medido[p] = -1;
    } else if (subida[primero] - subida[segundo] < VENTAJA_MINIMA) {
      Serial.print("        NO CONCLUYO: subieron DOS parecido (S");
      Serial.print(primero + 1); Serial.print(" y S"); Serial.print(segundo + 1);
      Serial.println(").");
      Serial.println("        Habia mas de un sensor sobre el blanco. Repetir.");
      medido[p] = -1;
    } else {
      medido[p] = primero;
      Serial.print("        >>> el de "); Serial.print(POSICION[p]);
      Serial.print(" es el SENSOR "); Serial.println(primero + 1);
    }
  }

  // ---------------- VEREDICTO ----------------
  Serial.println();
  Serial.println("===================================================");
  Serial.println("                  VEREDICTO");
  Serial.println("===================================================");

  bool completo = true;
  bool repetido = false;
  for (int p = 0; p < 3; p++) {
    if (medido[p] < 0) completo = false;
    for (int q = p + 1; q < 3; q++)
      if (medido[p] >= 0 && medido[p] == medido[q]) repetido = true;
  }

  for (int p = 0; p < 3; p++) {
    Serial.print("  "); Serial.print(POSICION[p]);
    Serial.print("  ->  ");
    if (medido[p] < 0) Serial.println("SIN DATO");
    else { Serial.print("sensor "); Serial.println(medido[p] + 1); }
  }

  if (!completo) {
    Serial.println();
    Serial.println("FALTAN MEDICIONES. No doy veredicto: una medicion que no");
    Serial.println("se hizo no es un dato, es una trampa. Repetir los pasos");
    Serial.println("que dicen SIN DATO. (Leccion de pruebas/signos/, 01/09.)");
    Serial.println("===================================================");
    termine = true;
    return;
  }
  if (repetido) {
    Serial.println();
    Serial.println("DOS POSICIONES DIERON EL MISMO SENSOR. Algo se apoyo mal.");
    Serial.println("No doy veredicto. Repetir todo.");
    Serial.println("===================================================");
    termine = true;
    return;
  }

  // Comparar contra lo que asume el codigo
  Serial.println();
  Serial.println("  Contra lo que asume escaparDeLinea() hoy:");
  bool coincide = true;
  for (int p = 0; p < 3; p++) {
    int s = medido[p];                       // numero de sensor (0..2)
    Serial.print("    sensor "); Serial.print(s + 1);
    Serial.print(": el codigo lo trata como "); Serial.print(SEGUN_EL_CODIGO[s]);
    Serial.print(", y esta "); Serial.print(POSICION[p]);
    if (strcmp(SEGUN_EL_CODIGO[s], POSICION[p]) == 0) {
      Serial.println("   OK");
    } else {
      Serial.println("   <<< NO COINCIDE");
      coincide = false;
    }
  }

  Serial.println();
  if (coincide) {
    Serial.println("  EL CODIGO ESTA BIEN. El escape sale para donde debe.");
    Serial.println("  Lo que hay que corregir son los dibujos y los rotulos.");
  } else {
    Serial.println("  EL CODIGO ESCAPA PARA EL LADO EQUIVOCADO.");
    Serial.println("  Hay que corregir escaparDeLinea() en funciona/delantero/,");
    Serial.println("  y de paso el orden de UMBRAL_LINEA[] si hace falta.");
    Serial.println("  OJO: los umbrales estan guardados POR NUMERO de sensor,");
    Serial.println("  asi que si se reordena algo hay que reordenar los dos.");
  }
  Serial.println("  Anotalo en la bitacora CON LOS NUMEROS.");
  Serial.println("===================================================");

  termine = true;
}
