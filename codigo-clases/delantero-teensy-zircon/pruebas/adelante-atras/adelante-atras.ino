/* =====================================================================
   ADELANTE Y ATRAS — mapeo medido en banco (2026-07-28)
   =====================================================================
   Mueven la IZQUIERDA y la DERECHA. La TRASERA queda quieta.

   MAPEO — medido, no deducido. Dos pruebas del tipo "apago una y miro
   cual queda quieta", que coinciden entre si:

        pines  8 / 7 / 6   = rueda IZQUIERDA
        pines 11 / 12 / 4  = rueda DERECHA
        pines  2 / 5 / 3   = rueda TRASERA    <-- esta queda apagada

   Este mapeo corresponde al DELANTERO (#define ROBOT2 del codigo 2025),
   porque ahi M3 (el motor de atras) son los pines 2/5/3.

   "Apagar" = las DOS patas de direccion en 0, no solo PWM en 0. En esta
   placa PWM=0 no alcanza para apagar un motor.

   Las dos de adelante van con polaridad OPUESTA entre si porque estan
   montadas espejadas.

   >>> SI GIRA SOBRE SI MISMO EN VEZ DE IR DERECHO: avisame, es una sola
   >>> linea la que hay que dar vuelta.

   POTENCIA: bajada a 70. Si alguna rueda zumba y no llega a arrancar,
   subi VEL de a 10. Es la unica perilla.
   ===================================================================== */

#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

const int VEL = 70;          // <<< PERILLA de potencia


void traseraQuieta() {
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  traseraQuieta();
}

void adelante() {
  analogWrite(IZQ_PWM, VEL); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, VEL); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  traseraQuieta();
}

void atras() {
  analogWrite(IZQ_PWM, VEL); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, VEL); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  traseraQuieta();
}


void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }
  Serial.println();
  Serial.println("ADELANTE Y ATRAS v3 — izquierda(8/7/6) + derecha(11/12/4).");
  Serial.println("Trasera (2/5/3) QUIETA.");
  Serial.print("Potencia: "); Serial.println(VEL);
  Serial.println("Arranca en 3 segundos.");
  delay(3000);
}


void loop() {
  Serial.println(">>> ADELANTE");
  adelante();
  delay(2000);

  parar();
  delay(1000);

  Serial.println("<<< ATRAS");
  atras();
  delay(2000);

  parar();
  delay(1000);
}
