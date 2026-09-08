/* =====================================================================
   GRABAR LINEA — mide en la cancha SIN CABLE, y vuelca al volver
   IITA Salta — delantero, Roboliga 2026
   =====================================================================

   NO TOCA LOS MOTORES. Ni un analogWrite. Anda con la bateria puesta.

   ---------------------------------------------------------------------
   EL PROBLEMA QUE RESUELVE
   ---------------------------------------------------------------------
   Los umbrales de linea SOLO valen si se miden donde el robot juega. El
   25/08 se midieron en la mesa, transfirieron mal, y dos de los tres
   sensores leian el verde de la cancha como si fuera blanco.

   Pero el cable USB no llega a la cancha. Y su hermano
   pruebas/calibrar-linea/ necesita el cable, porque espera un ENTER
   entre fase y fase.

   Este graba SIN NADIE MIRANDO y vuelca todo cuando lo reenchufas.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Cargalo con el cable. Apenas termina, EMPIEZA A GRABAR.
   2. Desenchufa el USB. LA BATERIA QUEDA PRENDIDA — si la apagas se
      corta la alimentacion y se pierde todo lo grabado.
      El LED de la placa (pin 13) parpadea: mientras parpadea, graba.
   3. Llevalo a la cancha y hace esta secuencia, DEJANDOLO QUIETO unos
      5 segundos en cada posicion y MOVIENDOLO entre una y otra:

         VERDE   punto 1        (quieto 5 s)
         VERDE   punto 2        (quieto 5 s)   lejos del anterior
         VERDE   punto 3        (quieto 5 s)   lo mas lejos que puedas
         BLANCO  sensor 1 izq   (quieto 5 s)
         BLANCO  sensor 2 centro(quieto 5 s)
         BLANCO  sensor 3 adel. (quieto 5 s)
         NEGRO   los tres       (quieto 5 s)

      El programa no sabe cual es cual — las reconoce como MESETAS y las
      numera en orden. El orden lo pones vos con la secuencia de arriba.

   4. Volve y enchufa el USB. SIGUE GRABANDO: no vuelca solo. Recien
      cuando le mandas cualquier tecla por el monitor, vuelca todo y
      para. Es a proposito — asi nadie le corta la grabacion por espiar
      el puerto.

   ---------------------------------------------------------------------
   POR QUE MESETAS Y NO FASES POR RELOJ
   ---------------------------------------------------------------------
   Dos clases seguidas se perdieron porque el programa decidia por
   tiempo y la persona no llegaba. Aca no hay reloj que cumplir: dejas
   el robot quieto el tiempo que quieras y eso solo ya es una meseta.
   Si una te sale mal, repetila — van a aparecer las dos y elegimos.
   [bitacoras 2026-08-25, 2026-09-01]

   El firmware no llama a analogReadResolution(), asi que corre en el
   default del Teensy 4.1: 10 bits, 0..1023. Aca tampoco se toca, para
   que los numeros se comparen sin convertir nada.
   ===================================================================== */

const int PIN_VERSION = 32;
const int PIN_LED     = 13;

int  pinLinea[3];
const char* versionPlaca = "?";

const int UMBRAL_ACTUAL[3] = { 707, 582, 795 };

// ---- la traza ----
// Un bloque cada 100 ms con minimo, maximo y promedio de cada sensor.
// Guardar min y max ademas del promedio es lo que deja ver los PICOS,
// que es lo que sospechamos que dispara el escape en falso.
const unsigned long MS_BLOQUE   = 100;
const int           MAX_BLOQUES = 6000;         // 10 minutos

struct Bloque {
  uint16_t minimo[3];
  uint16_t maximo[3];
  uint16_t media[3];
};

DMAMEM Bloque traza[MAX_BLOQUES];               // ~108 KB en RAM2
int    nBloques = 0;

// acumulador del bloque en curso
unsigned long t_bloque = 0;
long     nMuestras = 0;
uint32_t sumaBloque[3];
uint16_t minBloque[3], maxBloque[3];

bool yaVolque = false;

// Que tan quieto tiene que estar para contar como meseta.
const int  QUIETO_MAX      = 45;   // cuentas de variacion dentro del bloque
const int  DERIVA_MAX      = 45;   // cuanto puede correrse entre bloques
const int  BLOQUES_MINIMOS = 20;   // 20 x 100 ms = 2 segundos

void arrancarBloque() {
  nMuestras = 0;
  for (int i = 0; i < 3; i++) {
    sumaBloque[i] = 0;
    minBloque[i]  = 65535;
    maxBloque[i]  = 0;
  }
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

// ---------------------------------------------------------------------

void volcar() {
  Serial.println();
  Serial.println("===================================================");
  Serial.println(" GRABAR LINEA - lo que se midio en la cancha");
  Serial.println("===================================================");
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   pines "); Serial.print(pinLinea[0]);
  Serial.print(", "); Serial.print(pinLinea[1]);
  Serial.print(", "); Serial.println(pinLinea[2]);
  Serial.print("Umbrales cargados hoy en el robot: ");
  Serial.print(UMBRAL_ACTUAL[0]); Serial.print(" / ");
  Serial.print(UMBRAL_ACTUAL[1]); Serial.print(" / ");
  Serial.println(UMBRAL_ACTUAL[2]);
  Serial.print("Grabados "); Serial.print(nBloques);
  Serial.print(" bloques de 100 ms = ");
  Serial.print(nBloques / 10.0, 1); Serial.println(" segundos");
  Serial.println("Sensor 1 = izquierdo   2 = centro   3 = adelante");

  if (nBloques < BLOQUES_MINIMOS) {
    Serial.println();
    Serial.println("!!! CASI NO HAY GRABACION. Si apagaste la bateria al");
    Serial.println("!!! desenchufar, el robot se reinicio y se perdio todo.");
    Serial.println("===================================================");
    return;
  }

  // ---- rango total, para tener el panorama ----
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

  // ---- mesetas: tramos donde estuvo quieto ----
  Serial.println();
  Serial.println("--- MESETAS (el robot estuvo quieto) ---");
  Serial.println("En el orden en que las hiciste: VERDE x3, BLANCO x3, NEGRO.");
  Serial.println();

  int meseta = 0;
  int b = 0;
  while (b < nBloques) {
    // un bloque sirve de arranque si adentro estuvo quieto
    bool quieto = true;
    for (int i = 0; i < 3; i++)
      if (traza[b].maximo[i] - traza[b].minimo[i] > QUIETO_MAX) quieto = false;
    if (!quieto) { b++; continue; }

    // extender mientras siga quieto y no se corra de nivel
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

    if (n < BLOQUES_MINIMOS) continue;   // muy corta, era de paso

    meseta++;
    Serial.print("MESETA "); Serial.print(meseta);
    Serial.print("   t="); Serial.print(ini / 10.0, 1);
    Serial.print(" s   duro "); Serial.print(n / 10.0, 1); Serial.println(" s");
    for (int i = 0; i < 3; i++) {
      int prom = suma[i] / n;
      Serial.print("    S"); Serial.print(i + 1);
      Serial.print(" = "); Serial.print(prom);
      Serial.print("   (visto "); Serial.print(lo[i]);
      Serial.print(".."); Serial.print(hi[i]);
      Serial.print(", pico "); Serial.print(hi[i] - prom);
      Serial.print(" arriba)");
      // aviso: con el umbral de hoy, esta meseta ya dispararia
      if (hi[i] >= UMBRAL_ACTUAL[i]) Serial.print("   <<< YA CRUZA EL UMBRAL DE HOY");
      Serial.println();
    }
    Serial.println();
  }

  if (meseta == 0) {
    Serial.println("NO ENCONTRE NINGUNA MESETA.");
    Serial.println("Hace falta dejarlo quieto al menos 2 segundos en cada");
    Serial.println("posicion. Si lo tuviste en la mano todo el tiempo, no hay");
    Serial.println("nada estable que medir. Repetir apoyandolo.");
  } else {
    Serial.print("Total: "); Serial.print(meseta); Serial.println(" mesetas.");
    Serial.println("Si hiciste la secuencia completa tendrian que ser 7.");
    Serial.println("Mas de 7 = alguna posicion quedo partida en dos, o sobra");
    Serial.println("una parada de paso. Menos = alguna no quedo quieta.");
  }

  Serial.println("===================================================");
  Serial.println("Para grabar de nuevo: RESET del Teensy, o recargar.");
  Serial.println("===================================================");
}

// ---------------------------------------------------------------------

void loop() {
  if (yaVolque) return;

  // VUELCA SOLO CUANDO SE LO PIDEN, mandando cualquier tecla por el
  // monitor. A proposito: no adivina si el cable "volvio".
  //
  // El Teensy no puede distinguir "me desenchufaron" de "estoy
  // enchufado pero nadie abrio el monitor" — las dos cosas se ven igual
  // desde adentro. Si volcara al detectar el cable, alcanzaria con que
  // alguien espie el puerto para que corte la grabacion a la mitad y se
  // pierda la salida a la cancha. Asi no: graba hasta que se lo pidan.
  if (Serial && Serial.available()) {
    while (Serial.available()) Serial.read();
    delay(400);                 // que el monitor termine de engancharse
    digitalWrite(PIN_LED, HIGH);
    volcar();
    yaVolque = true;
    return;
  }

  // ---- grabar ----
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
    // parpadeo: mientras parpadea, esta grabando
    digitalWrite(PIN_LED, (nBloques / 5) % 2);
  }
}
