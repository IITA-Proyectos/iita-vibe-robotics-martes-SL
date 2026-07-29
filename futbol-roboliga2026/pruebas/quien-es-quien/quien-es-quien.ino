/* =====================================================================
   QUIEN ES QUIEN — que rueda cuelga de que par de pines
   IITA Salta — martes — 2026-07-28
   =====================================================================

   POR QUE ASI
   Las pruebas anteriores preguntaban "¿cual rueda se mueve?" en una
   secuencia, y es facil confundirse. Esta pregunta al reves:

        muevo DOS ruedas y apago UNA. ¿Cual quedo QUIETA?

   Una rueda quieta entre dos que giran se ve de una, sin dudar y sin
   tener que acordarse de ningun orden. Son 3 fases y listo.

   ---------------------------------------------------------------------
   COMO SE CORRE
   ---------------------------------------------------------------------
   ROBOT LEVANTADO, ruedas al aire. Bateria puesta.
   Cada fase dura 5 s, con 2 s de pausa entre medio. Repite para siempre,
   asi lo podes mirar las veces que quieras.

   ANOTA SOLO ESTO — tres palabras:
        FASE A -> quieta la: ______   (izquierda / derecha / trasera)
        FASE B -> quieta la: ______
        FASE C -> quieta la: ______

   Con esas tres palabras queda cerrado el mapeo y no hay que volver a
   discutirlo nunca mas.

   NOTA: "apagar" acá es poner las DOS patas de direccion en 0, no solo
   el PWM en 0. En esta placa PWM=0 no alcanza para apagar un motor.
   ===================================================================== */

// Los tres pares de pines de motor de la placa Zircon.
// NO les pongo nombre de rueda a proposito: eso es lo que estamos midiendo.
#define A_INA 2
#define A_INB 5
#define A_PWM 3

#define B_INA 8
#define B_INB 7
#define B_PWM 6

#define C_INA 11
#define C_INB 12
#define C_PWM 4

const int VEL = 130;          // arriba del piso de arranque medido hoy
const int MS_FASE  = 5000;
const int MS_PAUSA = 2000;


void apagar(int ina, int inb, int pwm) {
  analogWrite(pwm, 0);
  digitalWrite(ina, 0);
  digitalWrite(inb, 0);
}

void encender(int ina, int inb, int pwm) {
  digitalWrite(ina, 1);
  digitalWrite(inb, 0);
  analogWrite(pwm, VEL);
}

void apagarTodo() {
  apagar(A_INA, A_INB, A_PWM);
  apagar(B_INA, B_INB, B_PWM);
  apagar(C_INA, C_INB, C_PWM);
}


void setup() {
  pinMode(A_INA, OUTPUT); pinMode(A_INB, OUTPUT); pinMode(A_PWM, OUTPUT);
  pinMode(B_INA, OUTPUT); pinMode(B_INB, OUTPUT); pinMode(B_PWM, OUTPUT);
  pinMode(C_INA, OUTPUT); pinMode(C_INB, OUTPUT); pinMode(C_PWM, OUTPUT);
  apagarTodo();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }
  Serial.println();
  Serial.println("==========================================");
  Serial.println("QUIEN ES QUIEN — robot levantado");
  Serial.println("En cada fase: 2 ruedas giran, 1 queda QUIETA.");
  Serial.println("Anota cual es la QUIETA en cada fase.");
  Serial.println("==========================================");
  Serial.println("Arranca en 4 segundos.");
  delay(4000);
}


void loop() {

  // ---- FASE A: apago el par de pines 2/5/3 ----
  apagarTodo(); delay(MS_PAUSA);
  Serial.println();
  Serial.println("FASE A  — apagado el par de pines 2 / 5 / 3");
  Serial.println("          ¿CUAL RUEDA QUEDO QUIETA?");
  apagar  (A_INA, A_INB, A_PWM);
  encender(B_INA, B_INB, B_PWM);
  encender(C_INA, C_INB, C_PWM);
  delay(MS_FASE);

  // ---- FASE B: apago el par de pines 8/7/6 ----
  apagarTodo(); delay(MS_PAUSA);
  Serial.println();
  Serial.println("FASE B  — apagado el par de pines 8 / 7 / 6");
  Serial.println("          ¿CUAL RUEDA QUEDO QUIETA?");
  encender(A_INA, A_INB, A_PWM);
  apagar  (B_INA, B_INB, B_PWM);
  encender(C_INA, C_INB, C_PWM);
  delay(MS_FASE);

  // ---- FASE C: apago el par de pines 11/12/4 ----
  apagarTodo(); delay(MS_PAUSA);
  Serial.println();
  Serial.println("FASE C  — apagado el par de pines 11 / 12 / 4");
  Serial.println("          ¿CUAL RUEDA QUEDO QUIETA?");
  encender(A_INA, A_INB, A_PWM);
  encender(B_INA, B_INB, B_PWM);
  apagar  (C_INA, C_INB, C_PWM);
  delay(MS_FASE);

  Serial.println();
  Serial.println("--- vuelta completa, empieza de nuevo ---");
}
