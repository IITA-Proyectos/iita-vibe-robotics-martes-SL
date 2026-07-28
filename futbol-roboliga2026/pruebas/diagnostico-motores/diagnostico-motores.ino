/* =====================================================================
   DIAGNOSTICO DE MOTORES v2 — los 3 motores en los DOS sentidos
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-07-28
   =====================================================================

   DE DONDE VIENE ESTO (banco, Gustavo, 2026-07-28)

   v1 dio dos cosas:
   (a) La rueda TRASERA gira con PWM3 = 0, si las patas de direccion estan
       en 1/0. O sea: en esta placa el PWM NO apaga el motor. Manda la
       direccion. Consecuencia grande: en el firmware 2025 la trasera nunca
       estuvo realmente libre durante el avance.
   (b) La rueda DERECHA no giro con INA2=0 / INB2=1 ... pero SI habia girado
       en la prueba de identificacion, que la manejaba con INA2=1 / INB2=0.
       Sospecha: anda en un sentido y no en el otro.

   ESTA VERSION responde (b) en serio: prueba los TRES motores en los DOS
   sentidos, y ademas repite la pregunta del PWM en cada motor.

   Si el resultado es "M1 y M3 andan en los dos sentidos, M2 solo en uno",
   el problema es de HARDWARE en el canal de la rueda derecha (medio puente
   H quemado, o la pata 7 sin llegar al driver), no del programa.
   Si los TRES fallan en el mismo sentido, entonces lo que esta mal es como
   entendemos el driver, y hay que mirar el esquematico de la Zircon.

   ---------------------------------------------------------------------
   COMO SE CORRE
   ---------------------------------------------------------------------
   ROBOT LEVANTADO, las TRES RUEDAS AL AIRE. Riesgo cero.
   Bateria puesta. Monitor serie a 19200. Apreta RESET.

   Son 8 pasos de 3 s con 2 s de pausa entre medio (~40 s en total).
   Para CADA paso anota una sola cosa: GIRA o NO GIRA.
   No mires para que lado gira, no importa.
   ===================================================================== */


// ARQUERO (ROBOT1) — mapeo confirmado en banco 2026-07-28
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = rueda IZQUIERDA

#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = rueda DERECHA

#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = rueda TRASERA

const int VEL      = 100;
const int MS       = 3000;
const int MS_PAUSA = 2000;

// Tabla de pasos: motor, INA, INB, PWM, texto
struct Paso {
  const char* titulo;
  int ina, inb, pwmPin;
  int valINA, valINB, valPWM;
};

const Paso PASOS[] = {
  { "1) IZQUIERDA  sentido A   (INA=1 INB=0  PWM=100)", INA1, INB1, PWM1, 1, 0, VEL },
  { "2) IZQUIERDA  sentido B   (INA=0 INB=1  PWM=100)", INA1, INB1, PWM1, 0, 1, VEL },
  { "3) DERECHA    sentido A   (INA=1 INB=0  PWM=100)", INA2, INB2, PWM2, 1, 0, VEL },
  { "4) DERECHA    sentido B   (INA=0 INB=1  PWM=100)  <<< la que usa avanzar()", INA2, INB2, PWM2, 0, 1, VEL },
  { "5) TRASERA    sentido A   (INA=1 INB=0  PWM=100)", INA3, INB3, PWM3, 1, 0, VEL },
  { "6) TRASERA    sentido B   (INA=0 INB=1  PWM=100)", INA3, INB3, PWM3, 0, 1, VEL },
  { "7) IZQUIERDA  sentido A   (INA=1 INB=0  PWM=0)    <<< prueba del PWM", INA1, INB1, PWM1, 1, 0, 0 },
  { "8) DERECHA    sentido A   (INA=1 INB=0  PWM=0)    <<< prueba del PWM", INA2, INB2, PWM2, 1, 0, 0 },
};
const int N_PASOS = sizeof(PASOS) / sizeof(PASOS[0]);


void todoApagado() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  todoApagado();

  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("=================================================");
  Serial.println("DIAGNOSTICO v2 — 3 motores, 2 sentidos");
  Serial.println("ROBOT LEVANTADO, ruedas al aire.");
  Serial.println("Para cada paso anota solo: GIRA / NO GIRA");
  Serial.println("=================================================");
  Serial.println("Arranca en 5 segundos.");
  for (int s = 5; s >= 1; s--) { Serial.print(s); Serial.println("..."); delay(1000); }
}


void loop() {

  for (int i = 0; i < N_PASOS; i++) {
    todoApagado();
    delay(MS_PAUSA);

    Serial.println();
    Serial.println(PASOS[i].titulo);

    digitalWrite(PASOS[i].ina, PASOS[i].valINA);
    digitalWrite(PASOS[i].inb, PASOS[i].valINB);
    analogWrite(PASOS[i].pwmPin, PASOS[i].valPWM);
    delay(MS);
  }

  todoApagado();
  Serial.println();
  Serial.println("=== TERMINO. Apreta RESET para repetir. ===");
  Serial.println();
  Serial.println("COMO SE LEE:");
  Serial.println(" - Si 1,2,5,6 giran y 4 NO: el canal de la rueda DERECHA");
  Serial.println("   anda en un solo sentido. Es hardware, no software.");
  Serial.println("   avanzar() no puede funcionar hasta arreglarlo.");
  Serial.println(" - Si 2,4,6 (todos los sentido B) NO giran: no es una rueda,");
  Serial.println("   es como entendemos el driver. Mirar el esquematico Zircon.");
  Serial.println(" - Si 7 y 8 GIRAN: confirmado que PWM=0 no apaga el motor;");
  Serial.println("   para apagar de verdad hay que poner INA=0 e INB=0.");
  Serial.println(" - Si 3 gira y 4 no, proba cambiar el conector del motor");
  Serial.println("   derecho por el de otra rueda para separar motor de driver.");

  while (true) { todoApagado(); delay(1000); }
}
