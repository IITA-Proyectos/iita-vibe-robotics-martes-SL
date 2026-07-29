/* =====================================================================
   TRES RUEDAS — para un lado, y despues para el otro. Nada mas.
   IITA Salta — martes — 2026-07-28
   =====================================================================

   Las TRES ruedas 2 s para un lado, pausa, las TRES 2 s para el otro,
   pausa, y repite. Para siempre.

   ROBOT LEVANTADO, ruedas al aire.
   (Si lo apoyas en el piso va a girar sobre si mismo: con las tres ruedas
   en el mismo sentido, un robot omni rota. No se va a ningun lado, pero
   se ve peor.)

   Bateria puesta. Monitor serie a 19200 si lo queres ver escrito.

   QUE MIRAR: en cada tanda tienen que girar LAS TRES.
   Si en una tanda gira una menos, esa rueda anda en un solo sentido.
   ===================================================================== */

// ARQUERO (ROBOT1)
#define INA1 2      // rueda IZQUIERDA
#define INB1 5
#define PWM1 3

#define INA2 8      // rueda DERECHA
#define INB2 7
#define PWM2 6

#define INA3 11     // rueda TRASERA
#define INB3 12
#define PWM3 4

const int VEL = 120;


void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// a/b son los valores de las dos patas de direccion de cada motor.
void tresRuedas(int a, int b) {
  digitalWrite(INA1, a); digitalWrite(INB1, b); analogWrite(PWM1, VEL);
  digitalWrite(INA2, a); digitalWrite(INB2, b); analogWrite(PWM2, VEL);
  digitalWrite(INA3, a); digitalWrite(INB3, b); analogWrite(PWM3, VEL);
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }
  Serial.println();
  Serial.println("TRES RUEDAS — robot levantado, ruedas al aire.");
  Serial.print("Potencia: "); Serial.println(VEL);
  Serial.println("Arranca en 3 segundos.");
  delay(3000);
}


void loop() {

  Serial.println(">>> LAS TRES — sentido A");
  tresRuedas(1, 0);
  delay(2000);

  parar();
  delay(1000);

  Serial.println("<<< LAS TRES — sentido B");
  tresRuedas(0, 1);
  delay(2000);

  parar();
  delay(1000);
}
