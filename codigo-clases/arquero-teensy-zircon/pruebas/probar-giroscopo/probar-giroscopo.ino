/* =====================================================================
   PROBAR-GIROSCOPO — solo el BNO055, nada mas
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-15
   =====================================================================

   Basado en el ejemplo oficial de Adafruit:
   https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor/arduino-code

   ---------------------------------------------------------------------
   TRES COSAS QUE SE LE CAMBIARON AL EJEMPLO DE ADAFRUIT, Y POR QUE
   ---------------------------------------------------------------------
   1. NO SE CUELGA SI NO LO ENCUENTRA.
      El original hace `while(1);` — se queda trabado para siempre y no
      dice nada mas. Justo lo contrario de lo que necesitamos: nosotros
      queremos MENEAR EL CONECTOR y ver si aparece. Aca reintenta cada 2
      segundos y avisa en cada vuelta.

   2. PRUEBA LAS DOS DIRECCIONES, 0x28 y 0x29.
      El BNO055 puede estar en cualquiera de las dos segun como quede una
      patita de la placa. Si se movio, buscarlo solo en 0x28 da "no esta"
      cuando en realidad esta al lado.

   3. EL BUS VA MAS LENTO, a 100 kHz.
      La documentacion de Adafruit avisa que el BNO055 es quisquilloso con
      los TIEMPOS del bus I2C, y que las fallas intermitentes se arreglan
      con resistencias mas fuertes. Bajar la velocidad del bus es la
      version por software del mismo arreglo, y no cuesta nada.
      ⚠️ Si con esto anda y sin esto no, el problema son los tiempos del
      bus — y ahi conviene el arreglo de hardware (resistencias de 2,2k a
      4,7k en SDA y SCL).

   ---------------------------------------------------------------------
   ADEMAS MUESTRA LA CALIBRACION, que nunca habiamos mirado
   ---------------------------------------------------------------------
   El BNO055 se autocalibra solo y puntua de 0 a 3 cada parte. Importa
   porque **con el sistema en 0 el rumbo no significa nada**, aunque el
   chip conteste. Es otra forma de "estar mudo" que no habiamos visto:
   contesta numeros, pero son basura.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   Con el USB puesto y 🚨 LA BATERIA PRENDIDA (el giroscopio se alimenta
   de la bateria, no del USB). Abrir `mirar.bat` y leer.

   Giralo con la mano: el rumbo tiene que cambiar. Si el numero no se
   mueve al girarlo, contesta pero no esta midiendo.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

// Motores: se apagan una vez y no se tocan nunca mas.
#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

const unsigned long BAUDIOS = 19200;

Adafruit_BNO055 bno28 = Adafruit_BNO055(55, 0x28);
Adafruit_BNO055 bno29 = Adafruit_BNO055(55, 0x29);
Adafruit_BNO055 *bno = NULL;       // el que haya contestado
byte direccionUsada = 0;

int vuelta = 0;
int vecesQueContesto = 0;
int vecesQueFalto = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// ---- LA PRUEBA MAS PRIMITIVA DE TODAS: preguntarle quien es ----
//
// Sin librerias, sin Adafruit, sin DFRobot. Se le pregunta al chip su
// numero de identificacion, que esta en el registro 0x00 y tiene que
// contestar 0xA0. Ese numero sale de la hoja de datos del BNO055 y vale
// para CUALQUIER placa que lo lleve — Adafruit, DFRobot, la que sea.
//
// 🎯 Para que sirve: saca la libreria del medio. Si esto contesta 0xA0,
// el chip esta vivo y bien conectado, y cualquier problema que quede es
// de configuracion o de libreria. Si no contesta, no hay libreria que lo
// arregle.
//
// Devuelve el ID leido, o -1 si no contesto.
int preguntarQuienEs(byte dir) {
  Wire.beginTransmission(dir);
  Wire.write(0x00);                    // registro CHIP_ID
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom((int)dir, 1) != 1) return -1;
  return Wire.read();
}

void informarIdentidad() {
  for (byte dir = 0x28; dir <= 0x29; dir++) {
    int id = preguntarQuienEs(dir);
    Serial.print("   0x"); Serial.print(dir, HEX); Serial.print(": ");
    if (id < 0) {
      Serial.println("no contesta");
    } else {
      Serial.print("contesta 0x"); Serial.print(id, HEX);
      if (id == 0xA0) Serial.println("  <-- ES UN BNO055 DE VERDAD ✅");
      else            Serial.println("  <-- hay algo, pero NO es un BNO055");
    }
  }
}

// Intenta las dos direcciones. Devuelve true si alguna contesto.
bool buscarElSensor() {
  if (bno28.begin()) { bno = &bno28; direccionUsada = 0x28; return true; }
  if (bno29.begin()) { bno = &bno29; direccionUsada = 0x29; return true; }
  bno = NULL; direccionUsada = 0;
  return false;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar

  Serial.begin(BAUDIOS);
  Wire.begin();
  Wire.setClock(100000);    // bus lento: el BNO055 es quisquilloso

  delay(500);
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" PROBAR-GIROSCOPO — solo el BNO055 (el robot NO se mueve)");
  Serial.println("=========================================================");
  Serial.println(" Reintenta hasta encontrarlo. NO se cuelga.");
  Serial.println();
  Serial.println(" 🎯 Si no aparece: MENEA EL CONECTOR mientras esto corre.");
  Serial.println(" 🎯 Si aparece: GIRA EL ROBOT CON LA MANO y mira el rumbo.");
  Serial.println();
  Serial.println(" 🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA.");
  Serial.println("=========================================================");

  // Primero la prueba sin librerias: ¿hay alguien, y quien es?
  Serial.println(" Identificacion cruda, sin librerias:");
  informarIdentidad();
  Serial.println();

  if (buscarElSensor()) {
    Serial.print(">> ENCONTRADO en 0x"); Serial.println(direccionUsada, HEX);
    delay(1000);
    bno->setExtCrystalUse(true);
  } else {
    Serial.println("!! NO ESTA. Ni en 0x28 ni en 0x29. Sigo buscando...");
  }
}

void loop() {
  vuelta++;

  // ---- si todavia no apareció, seguir buscandolo ----
  if (bno == NULL) {
    vecesQueFalto++;
    digitalWrite(LED, LOW);
    Serial.print("["); Serial.print(vuelta); Serial.print("] ");
    Serial.print("NO ESTA   (contesto ");   Serial.print(vecesQueContesto);
    Serial.print(" veces, falto ");          Serial.print(vecesQueFalto);
    Serial.println(" veces)   menea el conector...");
    informarIdentidad();               // la prueba cruda, en cada vuelta
    if (buscarElSensor()) {
      Serial.print(">> ¡APARECIO! en 0x"); Serial.println(direccionUsada, HEX);
      Serial.println(">> Si aparecio justo al mover el cable, ESE es el problema.");
      delay(1000);
      bno->setExtCrystalUse(true);
    }
    delay(2000);
    return;
  }

  // ---- ya lo tenemos: leerlo ----
  sensors_event_t e;
  bno->getEvent(&e);

  // Los tres en cero exactos = no esta midiendo, aunque conteste.
  bool mudo = (e.orientation.x == 0.0 && e.orientation.y == 0.0
               && e.orientation.z == 0.0);

  // La autocalibracion, de 0 a 3 cada una. Con sistema en 0 el rumbo no
  // significa nada, por mas que el chip conteste numeros.
  uint8_t sistema = 0, giro = 0, acel = 0, mag = 0;
  bno->getCalibration(&sistema, &giro, &acel, &mag);

  if (mudo) { vecesQueFalto++; digitalWrite(LED, LOW); }
  else      { vecesQueContesto++; digitalWrite(LED, HIGH); }

  Serial.print("["); Serial.print(vuelta); Serial.print("] ");
  if (mudo) {
    Serial.print("MUDO (puros ceros)                    ");
  } else {
    Serial.print("rumbo "); Serial.print(e.orientation.x, 1);
    Serial.print("   inclinacion "); Serial.print(e.orientation.y, 1);
    Serial.print(" / ");             Serial.print(e.orientation.z, 1);
  }
  Serial.print("   calib sis="); Serial.print(sistema);
  Serial.print(" giro=");        Serial.print(giro);
  Serial.print(" acel=");        Serial.print(acel);
  Serial.print(" mag=");         Serial.print(mag);
  Serial.print("   (ok ");       Serial.print(vecesQueContesto);
  Serial.print(" / mudo ");      Serial.print(vecesQueFalto);
  Serial.println(")");

  delay(300);
}
