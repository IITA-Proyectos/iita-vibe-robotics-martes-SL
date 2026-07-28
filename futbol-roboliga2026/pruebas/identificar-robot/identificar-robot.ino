/* =====================================================================
   IDENTIFICAR ROBOT / PROBAR MOVIMIENTO
   IITA Salta — taller de los martes — Roboliga 2026
   =====================================================================

   DOS MODOS:

   MODO 1 — ADELANTE / ATRAS. Usa el avanzar() del codigo 2025 tal cual.
            Sirve para ver si el robot va DERECHO con el #define elegido.
            Se corre EN EL PISO, con espacio.

   MODO 2 — UNA RUEDA POR VEZ. Sirve para saber que rueda cuelga de que
            driver, y con eso cual robot es. Se corre LEVANTADO.

   ---------------------------------------------------------------------
   YA MEDIDO EN BANCO (2026-07-28) — no hace falta volver a averiguarlo
   ---------------------------------------------------------------------
   El robot que el equipo llama "robot 2" es el ARQUERO -> #define ROBOT1.
   Confirmado dos veces, con los dos #define, mismo resultado:

       pines 2/5/3   (U5)  = rueda IZQUIERDA
       pines 8/7/6   (U17) = rueda DERECHA
       pines 11/12/4 (U7)  = rueda TRASERA

   OJO: los comentarios del codigo 2025 dicen "M1 = motor derecho" y
   "M2 = motor izquierdo". Estan ESPEJADOS (escritos mirando al robot de
   frente). La medicion de banco manda.

   ---------------------------------------------------------------------
   ANTES DE CARGAR — LEER, SON 30 SEGUNDOS
   ---------------------------------------------------------------------
   1. LAS RUEDAS NO SE MUEVEN SIN LA BATERIA. El USB alimenta al Teensy,
      pero los drivers de motor van por la bateria. Si cargas y no pasa
      NADA, lo primero que se revisa es la bateria, no el codigo.

   2. NO HAY BOTON DE ARRANQUE. El programa arranca al resetear el Teensy.
      Y como el Teensy se alimenta del USB, al terminar la carga YA esta
      corriendo, aunque la bateria este apagada.

      >>> PROTOCOLO: apoyar el robot -> prender la bateria -> sacar las
      >>> manos -> apretar RESET (el boton blanco del Teensy). Cuenta 5
      >>> segundos y arranca.

   3. MODO 1 va al PISO, con 3 metros libres en cada sentido y alguien con
      la mano en la llave de la bateria. NUNCA sobre una mesa: 2 segundos
      a PWM 100 cruzan cualquier mesa de taller.

   4. MODO 2 va LEVANTADO, sobre una caja, con las tres ruedas al aire.

   Este programa NO usa zirconLib ni el giroscopio a proposito: esquiva el
   problema de compilacion de la libreria y el while(1) mudo del BNO055.
   ===================================================================== */


// ---------------------------------------------------------------------
// ELEGI 1 ROBOT  (descomenta uno solo)
// ---------------------------------------------------------------------
//#define ROBOT2    // DELANTERO
#define ROBOT1      // ARQUERO  <-- CONFIRMADO en banco para "robot 2"


// ---------------------------------------------------------------------
// ELEGI 1 MODO  (descomenta uno solo)
// ---------------------------------------------------------------------
#define MODO_ADELANTE_ATRAS       // adelante/atras — EN EL PISO
//#define MODO_UNA_RUEDA          // una rueda por vez — ROBOT LEVANTADO


// ---------------------------------------------------------------------
// PERILLAS del modo adelante/atras
// ---------------------------------------------------------------------
const int  MS_TRAMO  = 2000;   // cuanto dura cada tramo (ms)
const int  MS_FRENO  = 1000;   // pausa entre adelante y atras (ms).
                               // 1 s es lo que usa el codigo 2025 entre
                               // la patada y el retroceso (arquero.ino:1177)
const int  CICLOS    = 3;      // cuantas idas y vueltas. 0 = infinito.
                               // OJO con 0: el ida y vuelta NO vuelve al
                               // punto de partida, va derivando de a poco.

const int  VEL = 100;          // el mismo PWM que avanzar() del 2025
                               // (arquero.ino:152). Si una rueda zumba y
                               // no arranca, subi de a 20 — SIEMPRE con
                               // el robot levantado primero.


// ---------------------------------------------------------------------
// Guardas: si te equivocas al descomentar, NO compila y te avisa.
// ---------------------------------------------------------------------
#if defined(ROBOT1) && defined(ROBOT2)
  #error "Elegiste LOS DOS robots. Comenta ROBOT1 o comenta ROBOT2, uno solo."
#endif
#if !defined(ROBOT1) && !defined(ROBOT2)
  #error "No elegiste ningun robot. Descomenta ROBOT1 o ROBOT2."
#endif
#if defined(MODO_UNA_RUEDA) && defined(MODO_ADELANTE_ATRAS)
  #error "Elegiste LOS DOS modos. Dejate uno solo."
#endif
#if !defined(MODO_UNA_RUEDA) && !defined(MODO_ADELANTE_ATRAS)
  #error "No elegiste ningun modo. Descomenta MODO_ADELANTE_ATRAS o MODO_UNA_RUEDA."
#endif


// ---------------------------------------------------------------------
// PINES — textuales del codigo 2025 (arquero.ino:13-24 y :37-48)
// ---------------------------------------------------------------------
#if defined(ROBOT2)
  #define INA1 8
  #define INB1 7
  #define PWM1 6

  #define INA2 11
  #define INB2 12
  #define PWM2 4

  #define INA3 2
  #define INB3 5
  #define PWM3 3

  #define NOMBRE_ROBOT "ROBOT2 (DELANTERO)"
#endif

#if defined(ROBOT1)
  #define INA1 2
  #define INB1 5
  #define PWM1 3

  #define INA2 8
  #define INB2 7
  #define PWM2 6

  #define INA3 11
  #define INB3 12
  #define PWM3 4

  #define NOMBRE_ROBOT "ROBOT1 (ARQUERO)"
#endif


// ---------------------------------------------------------------------
// Movimientos
// ---------------------------------------------------------------------

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Copia TEXTUAL de avanzar() del codigo 2025 (arquero.ino:151-155).
// Las dos ruedas de adelante empujan con polaridad OPUESTA entre si
// (porque estan montadas espejadas) y la TRASERA queda sin potencia:
// en un omni de 3 ruedas la trasera no aporta nada al avance recto.
void avanzar() {
  analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, VEL); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

// El espejo exacto: se dan vuelta INA/INB de las dos de adelante.
// Mismo patron que retroceder_patear() del 2025 (arquero.ino:180-184).
void retroceder() {
  analogWrite(PWM1, VEL); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, VEL); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

void soloMotor(int cual) {
  parar();
  if (cual == 1) { analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0); }
  if (cual == 2) { analogWrite(PWM2, VEL); digitalWrite(INA2, 1); digitalWrite(INB2, 0); }
  if (cual == 3) { analogWrite(PWM3, VEL); digitalWrite(INA3, 1); digitalWrite(INB3, 0); }
}


// ---------------------------------------------------------------------

int vueltas = 0;

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);

  parar();                       // lo primero: motores frenados

  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }   // espera al monitor, max 4 s

  Serial.println();
  Serial.println("=========================================");
  Serial.print("compilado como "); Serial.println(NOMBRE_ROBOT);
  Serial.print("PWM: "); Serial.println(VEL);
  Serial.println("=========================================");

#if defined(MODO_ADELANTE_ATRAS)
  Serial.println("MODO 1: adelante / atras.  >>> EN EL PISO, CON ESPACIO <<<");
  Serial.print("  "); Serial.print(MS_TRAMO); Serial.print(" ms adelante, ");
  Serial.print(MS_FRENO); Serial.print(" ms de freno, ");
  Serial.print(MS_TRAMO); Serial.println(" ms atras.");
  if (CICLOS > 0) { Serial.print("  "); Serial.print(CICLOS); Serial.println(" ciclos y para."); }
  else            { Serial.println("  INFINITO — se corta desenchufando la bateria."); }
  Serial.println();
  Serial.println("QUE MIRAR: si va DERECHO, el #define es el correcto para");
  Serial.println("este robot. Si sale en diagonal o girando, es el OTRO robot.");
#else
  Serial.println("MODO 2: una rueda por vez.  >>> ROBOT LEVANTADO <<<");
  Serial.println();
  Serial.println("CRITERIO: si la TERCERA rueda que gira (M3) es la de ATRAS,");
  Serial.println("el #define compilado es el correcto para este robot.");
#endif

  Serial.println();
  Serial.println("Arranca en 5 segundos. Sacá las manos.");
  for (int s = 5; s >= 1; s--) { Serial.print(s); Serial.println("..."); delay(1000); }
  Serial.println("YA");
}


void loop() {

  if (CICLOS > 0 && vueltas >= CICLOS) {
    parar();
    return;                       // termino: apreta RESET para repetir
  }
  vueltas++;

#if defined(MODO_ADELANTE_ATRAS)

  Serial.print("--- ciclo "); Serial.println(vueltas);

  Serial.println(">>> ADELANTE");
  avanzar();
  delay(MS_TRAMO);

  Serial.println("    freno");
  parar();
  delay(MS_FRENO);

  Serial.println("<<< ATRAS");
  retroceder();
  delay(MS_TRAMO);

  Serial.println("    freno");
  parar();
  delay(MS_FRENO);

#else   // MODO_UNA_RUEDA

  Serial.print("--- ronda "); Serial.println(vueltas);

  for (int m = 1; m <= 3; m++) {
    Serial.print(">>> Girando SOLO M"); Serial.print(m);
    Serial.println("   — anota QUE rueda se mueve");
    soloMotor(m);
    delay(2000);
    parar();
    delay(1500);
  }

#endif

  if (CICLOS > 0 && vueltas >= CICLOS) {
    parar();
    Serial.println();
    Serial.println("=== TERMINO. Para repetir: boton RESET del Teensy. ===");
  }
}
