/* =====================================================================
   GIRO-IMU — ¿arranca el giroscopio, si o no? Una sola vez, en modo IMU
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-22
   =====================================================================

   LO MAS SIMPLE POSIBLE. Inicializa el giroscopio UNA VEZ al prender, en
   modo IMUPLUS (sin brujula), y despues solo parpadea:

        LED FIJO PRENDIDO   ->  esta descansando y calibrandose, NO LO TOQUES
        PARPADEO LENTO      ->  ANDUVO     (1 s prendido, 1 s apagado)
        PARPADEO RAPIDO     ->  NO ARRANCO (10 veces por segundo)

   No reintenta, no revive nada, no tiene teclas. La prueba es APAGAR Y
   PRENDER EL ROBOT ENTERO y mirar el LED: asi se ve de una cuantas veces
   de cada diez arranca bien.

   ---------------------------------------------------------------------
   LOS DOS DESCANSOS, Y POR QUE SON DISTINTOS
   ---------------------------------------------------------------------
   ANTES de hablarle (2 segundos). El BNO055 tarda su medio segundo largo
   en terminar de arrancar solo. El Teensy alimentado por USB puede
   despertarse antes que el, y preguntarle algo a un chip que todavia no
   termino de prender es la forma mas facil de que conteste cualquier cosa.

   DESPUES de inicializarlo (hasta 5 segundos). Este es el que dice la hoja
   de datos de Bosch (BST-BNO055-DS000, seccion 3.11.2): el giroscopio se
   calibra solo QUEDANDOSE QUIETO unos segundos, y esa calibracion se
   pierde en cada apagado. El chip se puntua a si mismo de 0 a 3; aca se
   espera a que llegue a 3, mirando de a poco.

   🚨 MIENTRAS EL LED ESTA FIJO, NO LO MUEVAS. Ese es el rato en que se
   esta calibrando. Es la diferencia entre el 5 de 5 del 21/09 y los
   arranques torcidos de antes.

   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA: el giroscopio come de la
   bateria, no del USB. Y prender la bateria con el USB ya puesto NO
   reinicia el Teensy, asi que para que la prueba valga hay que reiniciar
   de verdad: bateria apagada -> prender bateria.

   ESTE PROGRAMA NO MUEVE EL ROBOT. Los motores se apagan en setup().
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// Los dos descansos. Cortos a proposito: esto se prueba apagando y
// prendiendo diez veces, y cada segundo de mas se paga diez veces.
const unsigned long MS_DESCANSO_INICIAL   = 2000;   // que arranque el chip
const unsigned long MS_DESCANSO_CALIBRAR  = 5000;   // quieto, calibrandose

bool anduvo = false;
unsigned long msHastaElDato = 0, msHastaCalibrar = 0;
uint8_t calGiro = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// ⚠️ Un chequeo de tres lineas, y no es capricho: adentro de bno.begin()
// la libreria de Adafruit resetea el chip y despues espera a que vuelva
// con un `while` QUE NO TIENE SALIDA. Si el chip contesta a medias, el
// programa se queda trabado ahi y el LED no parpadea NUNCA — ni lento ni
// rapido — y no nos enteramos de nada. Preguntandole primero quien es,
// eso no puede pasar: si no contesta, ni lo intentamos.
bool contestaElChip() {
  Wire.beginTransmission(0x28);
  Wire.write(0x00);                       // registro CHIP_ID
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(0x28, 1) != 1)     return false;
  return Wire.read() == 0xA0;             // el BNO055 contesta 0xA0
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();

  Serial.begin(19200);

  // LED fijo = "estoy descansando, no me toques". Se prende ANTES del
  // primer descanso para que se vea desde el segundo cero.
  digitalWrite(LED, HIGH);

  delay(MS_DESCANSO_INICIAL);          // que el chip termine de arrancar

  Wire.begin();
  delay(100);

  if (contestaElChip() && bno.begin(OPERATION_MODE_IMUPLUS)) {
    delay(1000);
    bno.setExtCrystalUse(true);

    // "Arrancar" no es contestar el saludo: el BNO055 dice que si mucho
    // antes de tener un rumbo, y mientras tanto devuelve CEROS. Asi que se
    // espera hasta 3 segundos a que de un dato de verdad.
    unsigned long t0 = millis();
    while (millis() - t0 < 3000) {
      sensors_event_t e;
      bno.getEvent(&e);
      if (e.orientation.x != 0.0 || e.orientation.y != 0.0
          || e.orientation.z != 0.0) { anduvo = true; break; }
      delay(50);
    }
    msHastaElDato = millis() - t0;

    // Y ahora el descanso que manda la hoja de datos: quieto hasta que el
    // giroscopio se de a si mismo un 3 de 3. Sale antes si llega.
    if (anduvo) {
      unsigned long t1 = millis();
      while (millis() - t1 < MS_DESCANSO_CALIBRAR) {
        uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
        bno.getCalibration(&sis, &gir, &ace, &mag);
        calGiro = gir;
        if (gir >= 3) break;
        delay(100);
      }
      msHastaCalibrar = millis() - t1;
    }
  }

  if (anduvo) {
    Serial.println(">> GIROSCOPIO OK en modo IMU (parpadeo lento)");
    Serial.print("   tardo "); Serial.print(msHastaElDato);
    Serial.println(" ms en dar el primer dato");
    Serial.print("   calibracion del giroscopo: "); Serial.print(calGiro);
    Serial.print("/3 despues de "); Serial.print(msHastaCalibrar);
    Serial.println(" ms quieto");
    if (calGiro < 3) {
      Serial.println("   (no llego a 3: o lo movieron, o necesita mas rato)");
    }
  } else {
    Serial.println("!! NO ARRANCO (parpadeo rapido)");
  }
}

void loop() {
  // lento = anduvo    rapido = no arranco
  unsigned long periodo = anduvo ? 1000 : 100;
  digitalWrite(LED, ((millis() / periodo) % 2) ? HIGH : LOW);

  // Por si hay cable puesto: el rumbo, una vez por segundo. Sin cable esto
  // se descarta solo y no molesta.
  static unsigned long t = 0;
  if (anduvo && millis() - t >= 1000) {
    t = millis();
    sensors_event_t e;
    bno.getEvent(&e);
    uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
    bno.getCalibration(&sis, &gir, &ace, &mag);
    Serial.print("yaw ");      Serial.print(e.orientation.x, 1);
    Serial.print("   pitch "); Serial.print(e.orientation.y, 1);
    Serial.print("   roll ");  Serial.print(e.orientation.z, 1);
    Serial.print("   calib sis "); Serial.print(sis);
    Serial.print(" giro ");    Serial.print(gir);
    Serial.print(" acel ");    Serial.println(ace);
  }
}
