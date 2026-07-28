/* =====================================================================
   IDENTIFICAR ROBOT  —  ¿este es el ARQUERO o el DELANTERO?
   IITA Salta — taller de los martes — Roboliga 2026
   =====================================================================

   PARA QUE SIRVE
   Los dos robots son iguales por fuera, pero tienen las ruedas enchufadas
   en drivers distintos. El programa 2025 lo compensa con un #define:

       ROBOT1 (ARQUERO)      ROBOT2 (DELANTERO)
       M1 -> pines 2/5/3     M1 -> pines 8/7/6
       M2 -> pines 8/7/6     M2 -> pines 11/12/4
       M3 -> pines 11/12/4   M3 -> pines 2/5/3

   Los pines son LOS MISMOS en los dos. Lo que cambia es que cada rueda
   fisica cuelga de un driver distinto. Este programa energiza una rueda
   por vez y vos anotas CUAL se movio: con ese orden se sabe cual robot es.

   ---------------------------------------------------------------------
   ANTES DE CARGAR — LEER, SON 30 SEGUNDOS
   ---------------------------------------------------------------------
   1. LAS RUEDAS NO SE MUEVEN SIN LA BATERIA. El USB alimenta al Teensy,
      pero los drivers de motor van por la bateria. Si cargas el programa
      y no pasa NADA, lo primero que se revisa es la bateria, no el codigo.

   2. ESTE ROBOT NO TIENE BOTON DE ARRANQUE. Arranca solo. Orden correcto:
      cargar con la bateria APAGADA -> apoyar el robot -> sacar las manos
      -> recien ahi prender la bateria.

   3. MODO 2 (el que viene puesto): el robot va LEVANTADO, apoyado sobre
      una caja o dos libros, con las TRES RUEDAS AL AIRE. Asi no se mueve
      de lugar y se ve clarisimo cual rueda gira. Riesgo: cero.

   4. MODO 1 (adelante/atras): ese SI se desplaza. Va al PISO, con 3 metros
      libres, y con alguien con la mano en la llave de la bateria. Nunca
      sobre una mesa. Para que no arranque de sorpresa, el modo 1 no
      empieza hasta que le escribas algo en el monitor serie: es a proposito.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   - Dejalo como viene (ROBOT2 + MODO 2), carga, y corre la prueba con el
     robot levantado. Con UNA sola corrida ya tenes la respuesta: no hace
     falta recompilar ni probar el otro #define.
   - Anota el orden de las ruedas y compara con la tabla de abajo.

   ESTE PROGRAMA NO USA zirconLib NI EL GIROSCOPIO, a proposito:
   - zirconLib hoy NO COMPILA (ver bugs-conocidos.md).
   - Si el BNO055 no contesta, el programa 2025 se cuelga en un while(1).
   Aca no hay nada de eso: solo pines y motores. Compila y corre siempre.

   Sobre el delay(): en el firmware de competencia delay() esta PROHIBIDO
   porque frena el lazo de control. Aca no hay lazo ni sensores que leer.
   La regla es "nada de delay() en el LAZO DE CONTROL", no "nunca".
   ===================================================================== */


// ---------------------------------------------------------------------
// ELEGI 1 ROBOT  (descomenta uno solo)
// ---------------------------------------------------------------------
#define ROBOT2      // DELANTERO  <-- el que probamos primero
//#define ROBOT1    // ARQUERO


// ---------------------------------------------------------------------
// ELEGI 1 MODO  (descomenta uno solo)
// ---------------------------------------------------------------------
#define MODO_UNA_RUEDA            // una rueda por vez — ROBOT LEVANTADO
//#define MODO_ADELANTE_ATRAS     // 2 tramos — EN EL PISO, con espacio


// ---------------------------------------------------------------------
// Guardas: si te equivocas al descomentar, NO compila y te avisa.
// Sin esto, descomentar los dos robots compila igual y gana ROBOT1 en
// silencio — que es justo el error mas facil de cometer.
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
  #error "No elegiste ningun modo. Descomenta MODO_UNA_RUEDA o MODO_ADELANTE_ATRAS."
#endif


// ---------------------------------------------------------------------
// Potencia. 100 es la que usa avanzar() del codigo 2025 (arquero.ino:152).
// Si una rueda zumba y no arranca, subi de a 20 — pero SIEMPRE con el
// robot levantado. Cuando encuentres el minimo que mueve las tres, recien
// ahi lo bajas al piso. No pases de 180 en esta prueba.
// ---------------------------------------------------------------------
const int VEL = 100;


// ---------------------------------------------------------------------
// PINES — copiados textuales del codigo 2025 (arquero.ino:13-24 y :37-48)
// Verificados uno por uno contra arquero.ino, delantero.ino y
// mapa-pines-teensy.md: las tres fuentes coinciden.
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

// Copia TEXTUAL de avanzar() del codigo 2025 (arquero.ino:151-155),
// incluido el INA3=1/INB3=0 con PWM3 en 0. Con PWM 0 la rueda de atras no
// recibe potencia igual, pero lo dejamos identico al original para que la
// trayectoria sea exactamente la del programa que gano el Nacional.
void avanzar() {
  analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, VEL); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

// El espejo exacto: se dan vuelta INA/INB de M1 y M2.
// Mismo patron que retroceder_patear() del 2025 (arquero.ino:180-184).
void retroceder() {
  analogWrite(PWM1, VEL); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, VEL); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

// Mueve UNA sola rueda. parar() primero deja las otras dos realmente
// apagadas (PWM 0 y las dos direcciones en 0), no solo sin potencia.
void soloMotor(int cual) {
  parar();
  if (cual == 1) { analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0); }
  if (cual == 2) { analogWrite(PWM2, VEL); digitalWrite(INA2, 1); digitalWrite(INB2, 0); }
  if (cual == 3) { analogWrite(PWM3, VEL); digitalWrite(INA3, 1); digitalWrite(INB3, 0); }
}


// ---------------------------------------------------------------------

int vueltas = 0;

void cabecera() {
  Serial.println();
  Serial.println("=========================================");
  Serial.print("IDENTIFICAR ROBOT — compilado como ");
  Serial.println(NOMBRE_ROBOT);
  Serial.print("Potencia (PWM): "); Serial.println(VEL);
  Serial.println("=========================================");
}

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);

  parar();                       // lo primero: motores frenados

  Serial.begin(19200);
  while (!Serial && millis() < 8000) { }   // espera al monitor, max 8 s
  cabecera();

#if defined(MODO_UNA_RUEDA)
  Serial.println("MODO 2: una rueda por vez.  ROBOT LEVANTADO, ruedas al aire.");
  Serial.println();
  Serial.println("TABLA DE DECISION (compilado como ROBOT2):");
  Serial.println("  DERECHA -> IZQUIERDA -> ATRAS   = este robot es el DELANTERO");
  Serial.println("  IZQUIERDA -> ATRAS -> DERECHA   = este robot es el ARQUERO");
  Serial.println("  cualquier otro orden, o una rueda que no gira: PARAR y avisar.");
  Serial.println();
  Serial.println("El SENTIDO de giro no significa nada aca: mira solo QUE rueda gira.");
  Serial.println("(el frente del robot es el lado del pateador)");
  Serial.println();
  Serial.println("Arranca en 3 segundos.");
  delay(3000);
#else
  Serial.println("MODO 1: adelante / atras.  EN EL PISO, 3 metros libres.");
  Serial.println("Criterio: si va DERECHO, el #define compilado es el correcto.");
  Serial.println("          si sale en diagonal o girando, es el OTRO robot.");
  Serial.println();
  Serial.println(">>> Apoya el robot, saca las manos, y escribi cualquier cosa");
  Serial.println(">>> aca abajo + ENTER para arrancar.");
  while (Serial.available() == 0) { }        // armado explicito
  while (Serial.available()) Serial.read();  // limpia el buffer
  Serial.println("Arrancando...");
#endif
}


void loop() {

#if defined(MODO_UNA_RUEDA)

  // 3 rondas y para. Para repetir: boton reset del Teensy.
  if (vueltas >= 3) { parar(); return; }
  vueltas++;

  cabecera();
  Serial.print("--- RONDA "); Serial.print(vueltas); Serial.println(" de 3 ---");

  for (int m = 1; m <= 3; m++) {
    Serial.print(">>> Girando SOLO M"); Serial.print(m);
    Serial.println("   — anota QUE rueda se mueve (derecha / izquierda / atras)");
    soloMotor(m);
    delay(2000);

    parar();
    delay(1500);                  // pausa clara entre rueda y rueda
  }

  if (vueltas >= 3) {
    parar();
    Serial.println();
    Serial.println("=== TERMINO. Compara tu orden con la tabla de arriba. ===");
    Serial.println("Para repetir: boton reset del Teensy.");
  }

#else   // MODO_ADELANTE_ATRAS

  // UNA sola pasada y para. Sin esto el robot no deja de moverse nunca:
  // el ida y vuelta NO vuelve al punto de partida, va derivando.
  if (vueltas >= 1) { parar(); return; }
  vueltas++;

  Serial.println(">>> ADELANTE (1 s)");
  avanzar();
  delay(1000);

  Serial.println("    freno (1 s)");
  parar();                        // 1 s: el mismo freno que usa el codigo
  delay(1000);                    // 2025 entre patada y retroceso (:1177)

  Serial.println("<<< ATRAS (1 s)");
  retroceder();
  delay(1000);

  parar();
  Serial.println();
  Serial.println("=== TERMINO. ¿Fue derecho, o salio torcido? ===");
  Serial.println("Para repetir: boton reset del Teensy.");

#endif

}
