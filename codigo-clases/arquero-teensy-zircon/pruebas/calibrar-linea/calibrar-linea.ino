/* =====================================================================
   CALIBRAR LA LINEA — cuanto marca el verde y cuanto la linea blanca
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-11
   =====================================================================

   PARA QUE SIRVE
   El despeje frena el retroceso cuando un sensor de atras "ve blanco". Pero
   "ver blanco" es en realidad "el numero paso de tal valor", y ese valor
   hay que sacarlo de la cancha de verdad.

   Medimos en banco que una mesa blanca da ~765 y un pad negro ~120. Pero
   el VERDE de la cancha no es negro: va a dar algo intermedio, y nadie sabe
   cuanto. Si el verde diera 620 y el umbral estuviera en 600, el robot
   creeria que toda la cancha es linea y frenaria apenas arranca.

   Este sketch NO MUEVE NINGUN MOTOR y NO NECESITA COMPUTADORA. Mide solo,
   se acuerda, y despues se le pregunta.

   ---------------------------------------------------------------------
   COMO SE USA — EN LA CANCHA, SIN CABLE
   ---------------------------------------------------------------------
   Prende con la bateria y seguí el LED del Teensy:

     1 parpadeo, repetido   -> APOYALO SOBRE EL VERDE (tenes 8 segundos)
     LED FIJO               -> midiendo el verde, no lo muevas (4 s)

     2 parpadeos, repetido  -> PONELO SOBRE UNA LINEA BLANCA (8 segundos)
                               Los DOS sensores de atras tienen que quedar
                               sobre el blanco.
     LED FIJO               -> midiendo el blanco, no lo muevas (4 s)

     parpadeo lento sin fin -> TERMINO

   Despues llevalo a la compu **con la bateria PRENDIDA** (si la apagas se
   borra), enchufa el USB y pedí los numeros con la tecla 'm'.

   ---------------------------------------------------------------------
   QUE VA A CALCULAR
   ---------------------------------------------------------------------
   El umbral no se elige a ojo: va JUSTO EN EL MEDIO entre el verde mas
   claro que vio y el blanco mas oscuro que vio. Asi queda la mayor
   distancia posible a los dos lados.

   Y si el verde y el blanco se SUPERPONEN — si el verde llego a marcar mas
   que el blanco en algun momento — entonces no hay ningun umbral que
   funcione, y el sketch lo avisa. Eso pasaria si el sensor esta muy alto,
   muy sucio, o si le pega el sol. Mejor enterarse ahora.

   ---------------------------------------------------------------------
   SENSORES (medido en banco el 2026-08-11)
   ---------------------------------------------------------------------
        A12 = adelante        A13 = atras IZQUIERDA      A11 = atras DERECHA
        mas claro = numero mas alto
   ===================================================================== */

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12
#define LED 13

const unsigned long MS_PARA_ACOMODAR = 8000;   // tiempo para apoyarlo
const unsigned long MS_MIDIENDO      = 4000;   // tiempo midiendo

// 0 = atras izq, 1 = atras der, 2 = adelante
const int PINES[3] = { LINEA_ATRAS_IZQ, LINEA_ATRAS_DER, LINEA_ADELANTE };
const char* NOMBRES[3] = { "atras-IZQ (A13)", "atras-DER (A11)",
                           "adelante  (A12)" };

int minVerde[3],  maxVerde[3];   long sumaVerde[3];   long nVerde = 0;
int minBlanco[3], maxBlanco[3];  long sumaBlanco[3];  long nBlanco = 0;
bool hayMedicion = false;

enum Fase { AVISO_VERDE, MIDIENDO_VERDE, AVISO_BLANCO, MIDIENDO_BLANCO, LISTO };
Fase fase = AVISO_VERDE;
unsigned long t_fase = 0;


void reiniciar() {
  for (int i = 0; i < 3; i++) {
    minVerde[i] = 9999;  maxVerde[i] = -1;  sumaVerde[i] = 0;
    minBlanco[i] = 9999; maxBlanco[i] = -1; sumaBlanco[i] = 0;
  }
  nVerde = 0; nBlanco = 0;
  hayMedicion = false;
  fase = AVISO_VERDE;
  t_fase = millis();
  Serial.println();
  Serial.println(">> 1 parpadeo = APOYALO SOBRE EL VERDE");
}


// Parpadea `veces` y despues hace una pausa larga, para poder contarlo
// de lejos y sin computadora.
void parpadearVeces(int veces, unsigned long ahora) {
  unsigned long ciclo = ahora % ((unsigned long)veces * 500 + 1500);
  if (ciclo < (unsigned long)veces * 500) {
    digitalWrite(LED, ((ciclo % 500) < 220) ? HIGH : LOW);
  } else {
    digitalWrite(LED, LOW);
  }
}


void informe() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" CALIBRACION DE LINEA");
  Serial.println("=================================================");
  if (!hayMedicion) {
    Serial.println("  todavia no termino de medir");
    Serial.println("=================================================");
    return;
  }
  Serial.print("  muestras: verde "); Serial.print(nVerde);
  Serial.print(", blanco ");          Serial.println(nBlanco);
  Serial.println();

  int peorVerde = -1;      // el verde MAS CLARO de los dos de atras
  int peorBlanco = 9999;   // el blanco MAS OSCURO de los dos de atras

  for (int i = 0; i < 3; i++) {
    int promV = nVerde  ? (int)(sumaVerde[i]  / nVerde)  : 0;
    int promB = nBlanco ? (int)(sumaBlanco[i] / nBlanco) : 0;
    Serial.print("  "); Serial.print(NOMBRES[i]);
    Serial.print("   verde ");  Serial.print(promV);
    Serial.print(" ["); Serial.print(minVerde[i]);
    Serial.print("-");  Serial.print(maxVerde[i]); Serial.print("]");
    Serial.print("   blanco "); Serial.print(promB);
    Serial.print(" ["); Serial.print(minBlanco[i]);
    Serial.print("-");  Serial.print(maxBlanco[i]); Serial.print("]");

    int separacion = minBlanco[i] - maxVerde[i];
    Serial.print("   separacion "); Serial.print(separacion);
    if (separacion <= 0)      Serial.print("  <-- SE SUPERPONEN!");
    else if (separacion < 60) Serial.print("  <-- muy justo");
    Serial.println();

    // Solo los dos de atras deciden el umbral: son los que frenan.
    //
    // Del verde tomamos el MAXIMO (el verde mas claro que vimos) y del
    // blanco tambien el MAXIMO — no el minimo, aunque suene raro.
    //
    // Por que: durante los 4 s del blanco es facil que el sensor pase un
    // rato FUERA de la linea, sobre el verde. Esas lecturas bajas ensucian
    // el minimo y hacen creer que verde y blanco se superponen. Nos paso
    // el 2026-08-11: el blanco dio un rango de 288 a 768. El 288 era verde
    // colado en la muestra; el 768 era la linea de verdad.
    //
    // El blanco es lo mas claro que hay en la cancha, asi que el maximo de
    // esa fase SI es la linea. El promedio y el minimo se siguen mostrando
    // arriba para poder darse cuenta de que la muestra salio sucia.
    if (i < 2) {
      if (maxVerde[i]  > peorVerde)  peorVerde  = maxVerde[i];
      if (maxBlanco[i] < peorBlanco) peorBlanco = maxBlanco[i];
    }
  }

  Serial.println();
  Serial.println("  --- para los DOS DE ATRAS, que son los que frenan ---");
  Serial.print("  verde mas claro que vieron:   "); Serial.println(peorVerde);
  Serial.print("  blanco de verdad (el maximo): "); Serial.println(peorBlanco);
  Serial.println("  (si el promedio del blanco de arriba quedo MUY por");
  Serial.println("   debajo de este maximo, el robot estuvo un rato fuera");
  Serial.println("   de la linea mientras media — la muestra salio sucia,");
  Serial.println("   pero el maximo sigue sirviendo)");

  if (peorBlanco <= peorVerde) {
    Serial.println();
    Serial.println("  🚨 SE SUPERPONEN — no hay umbral que sirva.");
    Serial.println("  Ni siquiera el blanco mas claro supero al verde.");
    Serial.println("  Revisar altura de los sensores, suciedad, o luz");
    Serial.println("  directa. O que la linea medida fuera realmente blanca.");
  } else {
    int umbral = (peorVerde + peorBlanco) / 2;
    Serial.println();
    Serial.print("  >>> UMBRAL RECOMENDADO: "); Serial.println(umbral);
    Serial.print("      (queda a "); Serial.print(umbral - peorVerde);
    Serial.print(" del verde y a ");  Serial.print(peorBlanco - umbral);
    Serial.println(" del blanco)");
  }
  Serial.println("=================================================");
  Serial.println("  'r' = medir de nuevo");
  Serial.println("=================================================");
}


void setup() {
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);
  pinMode(LED, OUTPUT);

  Serial.begin(19200);      // por si hay cable; no lo espera
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" CALIBRAR LA LINEA — no muevo ningun motor");
  Serial.println("=================================================");
  Serial.println("  1 parpadeo  -> apoyalo en el VERDE");
  Serial.println("  LED fijo    -> midiendo, no lo muevas");
  Serial.println("  2 parpadeos -> ponelo sobre la LINEA BLANCA");
  Serial.println("  LED fijo    -> midiendo");
  Serial.println("  parpadeo lento sin fin -> termino, pedí 'm'");
  Serial.println("=================================================");
  reiniciar();
}


void loop() {

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'm') informe();
    else if (c == 'r') reiniciar();
  }

  unsigned long ahora = millis();

  switch (fase) {

    case AVISO_VERDE:
      parpadearVeces(1, ahora);
      if (ahora - t_fase >= MS_PARA_ACOMODAR) {
        fase = MIDIENDO_VERDE; t_fase = ahora;
        Serial.println(">> midiendo el VERDE — quieto");
      }
      break;

    case MIDIENDO_VERDE: {
      digitalWrite(LED, HIGH);
      for (int i = 0; i < 3; i++) {
        int v = analogRead(PINES[i]);
        if (v < minVerde[i]) minVerde[i] = v;
        if (v > maxVerde[i]) maxVerde[i] = v;
        sumaVerde[i] += v;
      }
      nVerde++;
      if (ahora - t_fase >= MS_MIDIENDO) {
        fase = AVISO_BLANCO; t_fase = ahora;
        Serial.println(">> 2 parpadeos = PONELO SOBRE LA LINEA BLANCA");
      }
      break;
    }

    case AVISO_BLANCO:
      parpadearVeces(2, ahora);
      if (ahora - t_fase >= MS_PARA_ACOMODAR) {
        fase = MIDIENDO_BLANCO; t_fase = ahora;
        Serial.println(">> midiendo el BLANCO — quieto");
      }
      break;

    case MIDIENDO_BLANCO: {
      digitalWrite(LED, HIGH);
      for (int i = 0; i < 3; i++) {
        int v = analogRead(PINES[i]);
        if (v < minBlanco[i]) minBlanco[i] = v;
        if (v > maxBlanco[i]) maxBlanco[i] = v;
        sumaBlanco[i] += v;
      }
      nBlanco++;
      if (ahora - t_fase >= MS_MIDIENDO) {
        fase = LISTO; t_fase = ahora;
        hayMedicion = true;
        Serial.println(">> LISTO");
        informe();
      }
      break;
    }

    case LISTO:
      digitalWrite(LED, ((ahora / 1200) % 2) ? HIGH : LOW);
      break;
  }
}
