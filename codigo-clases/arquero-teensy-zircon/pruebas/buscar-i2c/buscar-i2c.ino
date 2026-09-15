/* =====================================================================
   BUSCAR-I2C — ¿esta el giroscopio conectado, si o no?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-15
   =====================================================================

   PARA QUE SIRVE
   El 08/09 el robot dejo de encontrar el giroscopio:

        !! SIN GIROSCOPIO despues de 10 intentos
           giroscopio: NUNCA APARECIO al encender

   O sea que el chip no contesta. Y sabemos tres cosas mas:
     - a las 19:40 SI contestaba, y a las 20:10 ya no;
     - en el medio el robot viajo a la cancha y volvio;
     - la camara y los sensores de linea siguen andando perfecto.

   Todo eso apunta a algo FISICO, no a codigo. Este programa lo confirma o
   lo descarta sin que haya que adivinar.

   ---------------------------------------------------------------------
   QUE HACE
   ---------------------------------------------------------------------
   1. Mira el estado electrico de los dos cables de datos, ANTES de hablar.
   2. Le pregunta "¿estas ahi?" a las 127 direcciones posibles del bus.
   3. Repite todo cada 2 segundos.

   🎯 LO DEL PASO 3 ES LO MAS UTIL: con el escaneo repitiendose, MOVE EL
   CONECTOR CON LA MANO y mira la pantalla. Si el giroscopio aparece y
   desaparece mientras lo movés, el problema es ese cable y no hay nada
   mas que discutir.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   Con el cable USB puesto y LA BATERIA PRENDIDA (el giroscopio se
   alimenta de la bateria). Abrir `mirar.bat` y leer.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>

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

// Los dos cables de datos del bus. En el Teensy 4.1, Wire usa estos.
#define PIN_SDA 18
#define PIN_SCL 19

const unsigned long BAUDIOS = 19200;
const unsigned long MS_ENTRE_ESCANEOS = 2000;

int vuelta = 0;
int vecesQueAparecio = 0;
int vecesQueFalto = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Que aparato vive en cada direccion, para los que conocemos.
const char* quienEs(byte dir) {
  if (dir == 0x28) return "  <-- BNO055 (EL GIROSCOPIO)";
  if (dir == 0x29) return "  <-- BNO055 en su direccion alternativa";
  return "";
}


// ---- PASO 1: el estado electrico de los cables, antes de hablar ----
//
// Un bus I2C en reposo tiene los dos cables en ALTO, sostenidos por dos
// resistencias que viven en la placa del sensor y se alimentan de SU
// alimentacion. Por eso esta prueba dice mas de lo que parece:
//
//    los dos en ALTO  -> las resistencias estan vivas, o sea que la placa
//                        del sensor TIENE CORRIENTE. Si igual no contesta,
//                        el problema es el chip o el cable de datos.
//    alguno en BAJO   -> un cable pegado a masa: cortocircuito o pelado.
//    los dos en BAJO  -> lo mas probable es que la placa del sensor este
//                        SIN ALIMENTACION (VIN o GND flojos).
//
// Se leen SIN activar las resistencias internas del Teensy, a proposito:
// si las activaramos, darian ALTO siempre y la prueba no diria nada.
void mirarLosCables() {
  Wire.end();                       // soltar el bus para poder mirarlo
  pinMode(PIN_SDA, INPUT);
  pinMode(PIN_SCL, INPUT);
  delay(5);
  int sda = digitalRead(PIN_SDA);
  int scl = digitalRead(PIN_SCL);

  Serial.print("  cables:  SDA(18)="); Serial.print(sda ? "ALTO" : "BAJO");
  Serial.print("   SCL(19)=");         Serial.print(scl ? "ALTO" : "BAJO");

  if (sda && scl) {
    Serial.println("   -> el bus esta sano y la placa tiene corriente");
  } else if (!sda && !scl) {
    Serial.println("   -> 🚨 los DOS en bajo: revisar VIN y GND del sensor");
  } else {
    Serial.println("   -> 🚨 uno pegado a masa: revisar ese cable");
  }

  Wire.begin();                     // devolver el bus a su dueño
  delay(5);
}


// ---- PASO 2: preguntarle a las 127 direcciones ----
void escanear() {
  int encontrados = 0;
  bool estaElGiroscopo = false;

  for (byte dir = 1; dir < 127; dir++) {
    Wire.beginTransmission(dir);
    byte error = Wire.endTransmission();
    if (error == 0) {
      encontrados++;
      if (dir == 0x28 || dir == 0x29) estaElGiroscopo = true;
      Serial.print("  ENCONTRADO en 0x");
      if (dir < 16) Serial.print("0");
      Serial.print(dir, HEX);
      Serial.println(quienEs(dir));
    }
  }

  if (encontrados == 0) {
    Serial.println("  NO HAY NADA EN EL BUS. Ni un solo aparato contesta.");
  }

  if (estaElGiroscopo) {
    vecesQueAparecio++;
    digitalWrite(LED, HIGH);
  } else {
    vecesQueFalto++;
    digitalWrite(LED, LOW);
    Serial.println("  ❌ EL GIROSCOPIO (0x28) NO ESTA");
  }

  // La cuenta de apariciones es lo que delata un cable flojo: si mientras
  // movés el conector el numero de una columna y de la otra van subiendo
  // las dos, no es que el chip este roto — es que el contacto va y viene.
  Serial.print("  --- vuelta "); Serial.print(vuelta);
  Serial.print(": aparecio "); Serial.print(vecesQueAparecio);
  Serial.print(" veces, falto "); Serial.print(vecesQueFalto);
  Serial.println(" veces ---");
}


void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar

  Serial.begin(BAUDIOS);
  Wire.begin();
  delay(500);

  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" BUSCAR-I2C — ¿esta el giroscopio? (el robot NO se mueve)");
  Serial.println("=========================================================");
  Serial.println(" Escanea cada 2 segundos.");
  Serial.println();
  Serial.println(" 🎯 MOVE EL CONECTOR DEL GIROSCOPIO CON LA MANO mientras");
  Serial.println("    esto corre. Si aparece y desaparece, es ese cable.");
  Serial.println();
  Serial.println(" El LED de la placa se prende cuando el 0x28 contesta,");
  Serial.println(" aunque este medio tapado por la bateria.");
  Serial.println();
  Serial.println(" 🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA: el giroscopio se");
  Serial.println("    alimenta de ahi, no del USB.");
  Serial.println("=========================================================");
}


void loop() {
  vuelta++;
  Serial.println();
  Serial.print("========== ESCANEO "); Serial.print(vuelta);
  Serial.println(" ==========");
  mirarLosCables();
  escanear();
  delay(MS_ENTRE_ESCANEOS);
}
