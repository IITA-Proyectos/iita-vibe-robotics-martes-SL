/* =====================================================================
   DIAGNOSTICO DE MOTORES — ¿por que avanzar() mueve la rueda equivocada?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-07-28
   =====================================================================

   EL SINTOMA
   Con el ARQUERO compilado como ROBOT1, avanzar() del codigo 2025 deberia
   mover la rueda IZQUIERDA y la DERECHA, y dejar la TRASERA libre. En banco
   se vio moverse la IZQUIERDA y la TRASERA.

   LA SOSPECHA
   avanzar() (arquero.ino:151-155) deja la trasera asi:
        analogWrite(PWM3, 0);  digitalWrite(INA3, 1);  digitalWrite(INB3, 0);
   O sea: PWM en cero, PERO las dos patas de direccion puestas como "adelante".
   Si en la placa Zircon el pin PWM NO es un enable de verdad, esas dos patas
   solas alcanzan para que el motor gire a full. Eso explicaria la trasera.

   ESTE PROGRAMA NO ARREGLA NADA. Separa el problema en 5 pruebas para saber
   QUE esta pasando antes de tocar el codigo de competencia.

   ---------------------------------------------------------------------
   COMO SE CORRE
   ---------------------------------------------------------------------
   ROBOT LEVANTADO, apoyado sobre una caja, las TRES RUEDAS AL AIRE.
   Riesgo cero: el robot no se mueve de lugar.

   Bateria puesta (sin bateria no gira nada), monitor serie a 19200,
   y apreta RESET. Anota rueda por rueda lo que pasa en cada paso.
   ===================================================================== */


// ARQUERO — confirmado en banco 2026-07-28
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = rueda IZQUIERDA (medido)

#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = rueda DERECHA (medido)

#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = rueda TRASERA (medido)

const int VEL = 100;
const int MS  = 3000;   // cuanto dura cada paso
const int MS_PAUSA = 2000;


void todoApagado() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void paso(const char* titulo, const char* preguntar) {
  todoApagado();
  delay(MS_PAUSA);
  Serial.println();
  Serial.println(titulo);
  Serial.print("    PREGUNTA: "); Serial.println(preguntar);
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  todoApagado();

  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("==========================================");
  Serial.println("DIAGNOSTICO DE MOTORES — ARQUERO (ROBOT1)");
  Serial.println("ROBOT LEVANTADO, ruedas al aire.");
  Serial.println("Anota rueda por rueda en cada paso.");
  Serial.println("==========================================");
  Serial.println("Arranca en 5 segundos.");
  for (int s = 5; s >= 1; s--) { Serial.print(s); Serial.println("..."); delay(1000); }
}


void loop() {

  // ---- PASO 1: avanzar() tal cual esta en el codigo 2025 ----
  paso("PASO 1 — avanzar() del codigo 2025, tal cual",
       "¿QUE ruedas giran? (izquierda / derecha / trasera)");
  analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, VEL); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
  delay(MS);

  // ---- PASO 2: LA PRUEBA CLAVE ----
  // Solo la trasera, con PWM 0 y las patas de direccion como las deja
  // avanzar(). Si gira, PWM=0 NO apaga el motor en esta placa.
  paso("PASO 2 — SOLO trasera: PWM3=0 con INA3=1, INB3=0  <<< LA CLAVE",
       "¿Gira la TRASERA? Si gira, PWM=0 NO apaga el motor.");
  analogWrite(PWM3, 0); digitalWrite(INA3, 1); digitalWrite(INB3, 0);
  delay(MS);

  // ---- PASO 3: control del paso 2 ----
  paso("PASO 3 — SOLO trasera: PWM3=0 con INA3=0, INB3=0  (control)",
       "¿Se queda quieta? Si con 0/0 para y con 1/0 giraba, esta confirmado.");
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
  delay(MS);

  // ---- PASO 4: la derecha, con la polaridad que usa avanzar() ----
  paso("PASO 4 — SOLO derecha (M2): PWM=100 con INA2=0, INB2=1",
       "¿Gira la DERECHA? Es la polaridad exacta que usa avanzar().");
  analogWrite(PWM2, VEL); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  delay(MS);

  // ---- PASO 5: la izquierda, con la polaridad que usa avanzar() ----
  paso("PASO 5 — SOLO izquierda (M1): PWM=100 con INA1=1, INB1=0",
       "¿Gira la IZQUIERDA? Es la polaridad exacta que usa avanzar().");
  analogWrite(PWM1, VEL); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  delay(MS);

  // ---- fin ----
  todoApagado();
  Serial.println();
  Serial.println("=== TERMINO. Apreta RESET para repetir. ===");
  Serial.println();
  Serial.println("COMO SE LEE:");
  Serial.println(" - Paso 2 gira y paso 3 no  -> PWM=0 no apaga: hay que apagar");
  Serial.println("   la trasera con INA3=0/INB3=0. Explica todo el sintoma.");
  Serial.println(" - Paso 4 NO gira -> el problema es la rueda derecha o su");
  Serial.println("   driver con esa polaridad, no la trasera.");
  Serial.println(" - Pasos 4 y 5 giran y el paso 1 igual sale mal -> avisar,");
  Serial.println("   hay algo que no entendimos.");
  while (true) { todoApagado(); delay(1000); }
}
