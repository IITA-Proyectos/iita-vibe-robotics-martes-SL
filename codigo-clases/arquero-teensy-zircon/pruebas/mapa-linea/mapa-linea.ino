/* =====================================================================
   MAPA-LINEA — el robot trae el dibujo de todo lo que vieron los sensores
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-15
   =====================================================================

   PARA QUE SIRVE
   La cancha cambio (2026-09-15): es mas chica, el verde es MAS OSCURO y
   las lineas blancas son distintas. El umbral viejo (620) salio de medir
   la cancha vieja, asi que no vale mas. Hay que medir de nuevo.

   ---------------------------------------------------------------------
   EL PROBLEMA QUE ESTE PROGRAMA RESUELVE
   ---------------------------------------------------------------------
   Para medir en la cancha tenemos las dos manos atadas:

     - el cable USB NO LLEGA a la cancha, asi que no se puede mirar en vivo;
     - el LED del robot esta TAPADO por la bateria, asi que no sirve para
       guiar "ahora apoyalo en el verde, ahora en el blanco".

   La otra idea —guardar el minimo y el maximo— tampoco sirve: mientras uno
   lleva el robot en la mano, los sensores ven el piso, la mesa y los dedos,
   y esas lecturas se cuelan en el minimo y el maximo y arruinan la medicion.

   ---------------------------------------------------------------------
   LA SOLUCION: UN HISTOGRAMA
   ---------------------------------------------------------------------
   En vez de guardar solo el minimo y el maximo, el robot cuenta CUANTAS
   VECES vio cada valor. El resultado es un dibujo, y en ese dibujo las
   cosas se separan solas:

        verde        blanco
          ▇▇▇▇         ▇▇▇▇▇
        ▇▇▇▇▇▇▇      ▇▇▇▇▇▇▇▇
      --------------------------
        350          760

   Los dos MONTONES grandes son las dos superficies donde el robot estuvo
   apoyado un rato largo. Lo que vio de paso —el piso, los dedos, la mesa—
   aparece como pelitos sueltos, porque duro poquito.

   🎯 Por eso no hace falta ni LED ni cable ni cronometro: no importa CUANDO
   estuvo sobre cada cosa. El monton se forma solo.

   Y el umbral sale del VALLE entre los dos montones, que es el punto mas
   lejos de los dos a la vez.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Cargarlo y llevar el robot a la cancha CON LA BATERIA PRENDIDA.
   2. Apoyarlo sobre el VERDE y dejarlo quieto ~15 segundos.
   3. Apoyarlo sobre una LINEA BLANCA, con los dos sensores de atras bien
      sobre el blanco, y dejarlo quieto ~15 segundos.
      (El orden da igual, y se puede repetir en varios lugares de la cancha
       — cuantos mas lugares, mejor.)
   4. 🚨 VOLVER CON LA BATERIA PRENDIDA. Si se corta, se borra todo.
   5. Enchufar el USB, abrir `mirar.bat` y apretar la tecla 'm'.

   ---------------------------------------------------------------------
   EL LED TE DICE QUE ESTA VIENDO, EN VIVO
   ---------------------------------------------------------------------
        cuanto MAS CLARO ve, MAS RAPIDO parpadea

          sobre el verde  -> lento, casi un latido
          sobre el blanco -> rapidisimo, casi temblando

   No necesita ningun umbral, asi que se puede usar para guiarse ANTES de
   haber medido nada. Sirve para confirmar que el robot esta apoyado donde
   uno cree, y sobre todo para ver si DE VERDAD distingue las dos
   superficies: si el parpadeo cambia poco entre el verde y el blanco, los
   sensores no las separan, y conviene enterarse ahi mismo.

   TECLAS
        m = mostrar el dibujo y el umbral que sugiere
        r = borrar y empezar de nuevo
        ? = ayuda
   ===================================================================== */

#include <Arduino.h>

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12
#define LED 13

// Motores: se apagan una vez y no se tocan nunca mas.
#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

const unsigned long BAUDIOS = 19200;

// El sensor da de 0 a 1023. Se agrupa de a 32 valores, asi quedan 32
// casilleros. Menos casilleros perderian detalle; mas harian un dibujo
// lleno de huecos que no se entiende.
const int CASILLEROS = 32;
const int ANCHO      = 1024 / CASILLEROS;   // 32 valores por casillero

const int SENSORES = 3;
const char* NOMBRES[SENSORES] = { "atras-IZQ (A13)", "atras-DER (A11)", "adelante  (A12)" };
const int PINES[SENSORES]     = { LINEA_ATRAS_IZQ, LINEA_ATRAS_DER, LINEA_ADELANTE };

unsigned long cuenta[SENSORES][CASILLEROS];
unsigned long muestras = 0;

const unsigned long MS_ENTRE_MUESTRAS = 20;   // 50 por segundo
unsigned long t_ultimaMuestra = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void borrar() {
  for (int s = 0; s < SENSORES; s++)
    for (int c = 0; c < CASILLEROS; c++) cuenta[s][c] = 0;
  muestras = 0;
}


// ---------------------------------------------------------- el dibujo

// Dibuja los casilleros que tienen algo, con una barra proporcional.
void dibujar(int s) {
  unsigned long mayor = 1;
  for (int c = 0; c < CASILLEROS; c++)
    if (cuenta[s][c] > mayor) mayor = cuenta[s][c];

  for (int c = 0; c < CASILLEROS; c++) {
    if (cuenta[s][c] == 0) continue;
    int desde = c * ANCHO;
    Serial.print("   ");
    if (desde < 100) Serial.print(" ");
    if (desde < 10)  Serial.print(" ");
    Serial.print(desde);
    Serial.print("-");
    Serial.print(desde + ANCHO - 1);
    Serial.print(" |");
    // barra de hasta 40 caracteres
    int largo = (int)((cuenta[s][c] * 40) / mayor);
    if (largo < 1) largo = 1;
    for (int i = 0; i < largo; i++) Serial.print("#");
    Serial.print("  "); Serial.println(cuenta[s][c]);
  }
}

// Busca los dos montones y el valle entre ellos.
//
// Primero el monton mas grande. Despues el mas grande de los que estan
// LEJOS del primero — si no se exigiera distancia, el "segundo monton"
// seria el casillero de al lado del primero, que es parte del mismo.
// Y el umbral va en el valle: el casillero mas vacio entre los dos.
//
// Devuelve el umbral sugerido, o -1 si no encontro dos montones.
int sugerirUmbral(int s) {
  const int LEJOS = 4;              // al menos 4 casilleros = 128 unidades
  int pico1 = -1; unsigned long may1 = 0;
  for (int c = 0; c < CASILLEROS; c++)
    if (cuenta[s][c] > may1) { may1 = cuenta[s][c]; pico1 = c; }
  if (pico1 < 0 || may1 == 0) return -1;

  int pico2 = -1; unsigned long may2 = 0;
  for (int c = 0; c < CASILLEROS; c++) {
    if (abs(c - pico1) < LEJOS) continue;
    if (cuenta[s][c] > may2) { may2 = cuenta[s][c]; pico2 = c; }
  }
  // El segundo monton tiene que ser de verdad, no un pelito suelto.
  if (pico2 < 0 || may2 < may1 / 10) return -1;

  int a = pico1 < pico2 ? pico1 : pico2;
  int b = pico1 < pico2 ? pico2 : pico1;

  int valle = a; unsigned long menor = cuenta[s][a];
  for (int c = a; c <= b; c++)
    if (cuenta[s][c] <= menor) { menor = cuenta[s][c]; valle = c; }

  return valle * ANCHO + ANCHO / 2;   // el medio de ese casillero
}

void mostrar() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" LO QUE VIERON LOS SENSORES");
  Serial.print(" "); Serial.print(muestras);
  Serial.println(" muestras guardadas");
  Serial.println("=========================================================");

  int umbrales[SENSORES];
  for (int s = 0; s < SENSORES; s++) {
    Serial.println();
    Serial.print(" "); Serial.println(NOMBRES[s]);
    dibujar(s);
    umbrales[s] = sugerirUmbral(s);
    Serial.print("   -> ");
    if (umbrales[s] < 0) {
      Serial.println("NO VEO DOS MONTONES. ¿Estuvo sobre las dos superficies?");
    } else {
      Serial.print("el valle esta en "); Serial.println(umbrales[s]);
    }
  }

  // Los que deciden el despeje son los DOS DE ATRAS. El de adelante hoy
  // solo se mira, asi que no entra en la cuenta del umbral.
  Serial.println();
  Serial.println("---------------------------------------------------------");
  if (umbrales[0] > 0 && umbrales[1] > 0) {
    int u = (umbrales[0] + umbrales[1]) / 2;
    Serial.print(" 🎯 UMBRAL SUGERIDO PARA umbralBlanco: ");
    Serial.println(u);
    Serial.println(" (promedio de los dos sensores de atras, que son los");
    Serial.println("  que frenan el retroceso del despeje)");
    Serial.print(" El viejo, de la cancha anterior, era 620.");
    Serial.println();
    if (abs(umbrales[0] - umbrales[1]) > 80) {
      Serial.println(" ⚠️ Los dos de atras no coinciden: mas de 80 de");
      Serial.println("    diferencia. Ya sabiamos que no son iguales entre");
      Serial.println("    si (110 de diferencia sobre el mismo verde el");
      Serial.println("    11/08). Con uno solo de umbral, mirar cual sufre.");
    }
  } else {
    Serial.println(" No pude sugerir umbral: falta alguno de los montones.");
    Serial.println(" Hay que apoyarlo un rato largo sobre el VERDE y otro");
    Serial.println(" rato largo sobre una LINEA BLANCA.");
  }
  Serial.println("=========================================================");
}

void ayuda() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" MAPA-LINEA — el robot NO se mueve");
  Serial.println("=========================================================");
  Serial.println("  Llevalo a la cancha CON LA BATERIA PRENDIDA.");
  Serial.println("  Apoyalo ~15 s sobre el VERDE.");
  Serial.println("  Apoyalo ~15 s sobre una LINEA BLANCA.");
  Serial.println("  El orden da igual, y podes repetir en varios lugares.");
  Serial.println("  Volve SIN CORTAR LA BATERIA y apreta 'm'.");
  Serial.println("---------------------------------------------------------");
  Serial.println("  m = mostrar el dibujo    r = borrar y empezar de nuevo");
  Serial.println("=========================================================");
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar

  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE,  INPUT);

  Serial.begin(BAUDIOS);
  borrar();
  ayuda();
  Serial.println(">> MIDIENDO. Llevame a la cancha.");
}

void loop() {
  // Consola
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'm') mostrar();
    else if (c == 'r') { borrar(); Serial.println(">> borrado, empiezo de nuevo"); }
    else if (c == '?') ayuda();
  }

  unsigned long ahora = millis();
  if (ahora - t_ultimaMuestra < MS_ENTRE_MUESTRAS) return;
  t_ultimaMuestra = ahora;

  for (int s = 0; s < SENSORES; s++) {
    int v = analogRead(PINES[s]);
    int casillero = v / ANCHO;
    if (casillero < 0) casillero = 0;
    if (casillero >= CASILLEROS) casillero = CASILLEROS - 1;
    cuenta[s][casillero]++;
  }
  muestras++;

  // ------------------------------------------------------------------
  // EL LED CUENTA QUE ESTAN VIENDO LOS SENSORES DE ATRAS, EN VIVO
  // ------------------------------------------------------------------
  // El equipo aviso que el LED se ve "un poquito", asi que la señal tiene
  // que ser GRUESA: nada de contar destellos.
  //
  //      cuanto MAS CLARO ve, MAS RAPIDO parpadea
  //
  //        verde  -> lento, casi un latido       (~1 por segundo)
  //        blanco -> rapidisimo, casi temblando  (~10 por segundo)
  //
  // 🎯 Y lo mejor: NO NECESITA UN UMBRAL. Es justo lo que venimos a medir,
  // asi que no podiamos usarlo para guiarnos — seria pedirle la respuesta
  // antes de la pregunta. El ritmo es proporcional y no decide nada.
  //
  // Sirve para dos cosas en la cancha, sin cable:
  //   1. confirmar que el robot esta apoyado donde ustedes creen;
  //   2. 🚨 VER SI DE VERDAD DISTINGUE las dos superficies. Si el parpadeo
  //      cambia poco entre el verde y el blanco, los sensores no las
  //      separan bien — y eso hay que saberlo ahi mismo, no al volver.
  //
  // Se usa el MAS CLARO de los dos de atras, porque el despeje frena con
  // cualquiera de los dos.
  int izq = analogRead(LINEA_ATRAS_IZQ);
  int der = analogRead(LINEA_ATRAS_DER);
  int claro = (izq > der) ? izq : der;

  // 0 -> 1200 ms de periodo (lento).  1023 -> 100 ms (rapidisimo).
  unsigned long periodo = 1200 - ((unsigned long)claro * 1100) / 1023;
  if (periodo < 100) periodo = 100;
  digitalWrite(LED, ((ahora / (periodo / 2)) % 2) ? HIGH : LOW);
}
