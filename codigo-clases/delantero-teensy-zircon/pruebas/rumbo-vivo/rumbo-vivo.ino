/* =====================================================================
   RUMBO EN VIVO — ¿hacia donde crece el rumbo de ESTE robot?
   IITA Salta — taller de los martes — 2026-09-01
   =====================================================================

   Es la MEDICION 1 de pruebas/signos/, sola y sin cuentas regresivas,
   porque las otras dos ya se midieron:

       orbitar(sentidoA = true)  ->  el rumbo SUBIO 74.1 grados
       pelota a la DERECHA       ->  Yp = -33  (o sea: Y positivo = IZQUIERDA)

   Falta lo unico que traduce ese "+74.1" a un lado fisico: saber si el
   rumbo sube girando a la derecha o a la izquierda.

   NO se puede dar por sentado. La convencion del BNO055 es de brujula (el
   rumbo sube girando a la derecha), pero eso vale para el chip apoyado en
   su orientacion de referencia — y no sabemos como quedo montado en esta
   placa. Si esta al reves, el signo se da vuelta y con el la conclusion.

   COMO SE USA: cargarlo, mirar el numero, y GIRAR EL ROBOT A MANO HACIA LA
   DERECHA (horario visto desde arriba). Si el numero sube, derecha = subir.
   Si baja, derecha = bajar. Listo.

   NO MUEVE LOS MOTORES. Se corre sobre la mesa.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6
#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4
#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiro = false;
float r0 = -1;                 // rumbo de referencia, el del arranque
float peor = 0;                // el desvio mas grande visto

float dif(float a, float b) {
  float d = a - b;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void setup() {
  int pines[9] = { IZQ_INA, IZQ_INB, IZQ_PWM, DER_INA, DER_INB, DER_PWM,
                   TRA_INA, TRA_INB, TRA_PWM };
  for (int i = 0; i < 9; i++) { pinMode(pines[i], OUTPUT); digitalWrite(pines[i], LOW); }

  Serial.begin(19200);
  Wire.begin();
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("############################################################");
  Serial.println("#  RUMBO EN VIVO — no mueve los motores                     #");
  Serial.println("#  GIRA EL ROBOT A MANO HACIA LA DERECHA y mira el numero.  #");
  Serial.println("############################################################");

  hayGiro = bno.begin();
  if (hayGiro) {
    bno.setExtCrystalUse(true);
    delay(700);
    uint8_t sys = 0, at = 0, er = 0;
    bno.getSystemStatus(&sys, &at, &er);
    hayGiro = (sys == 5);
  }
  if (!hayGiro) { Serial.println("!! SIN GIROSCOPO"); return; }

  sensors_event_t e; bno.getEvent(&e);
  r0 = e.orientation.x;
  Serial.print("Rumbo de referencia (el de ahora): "); Serial.println(r0, 1);
  Serial.println();
  Serial.println("Ahora girá el robot a la DERECHA. Cada renglon muestra");
  Serial.println("cuanto te alejaste de la referencia:");
  Serial.println("    positivo = el rumbo SUBE girando a la derecha");
  Serial.println("    negativo = el rumbo BAJA girando a la derecha");
  Serial.println();
}

void loop() {
  if (!hayGiro) return;
  static unsigned long t = 0;
  if (millis() - t < 300) return;
  t = millis();

  sensors_event_t e; bno.getEvent(&e);
  float d = dif(e.orientation.x, r0);
  if (fabs(d) > fabs(peor)) peor = d;

  Serial.print("rumbo="); Serial.print(e.orientation.x, 1);
  Serial.print("   desde la referencia: ");
  if (d >= 0) Serial.print("+");
  Serial.print(d, 1);
  Serial.print("   maximo visto: ");
  if (peor >= 0) Serial.print("+");
  Serial.print(peor, 1);
  if (fabs(peor) >= 20) {
    Serial.print(peor > 0 ? "   -> DERECHA = SUBE" : "   -> DERECHA = BAJA");
  }
  Serial.println();
}
