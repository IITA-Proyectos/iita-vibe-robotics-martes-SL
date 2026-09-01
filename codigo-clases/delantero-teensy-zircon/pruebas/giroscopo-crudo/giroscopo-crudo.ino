/* =====================================================================
   GIROSCOPO EN CRUDO — diagnostico del delantero
   IITA Salta — taller de los martes — 2026-09-01
   =====================================================================

   LA PREGUNTA QUE RESPONDE:
   ¿el giroscopio de ESTE robot esta sano, esta roto, o es otro chip?

   POR QUE HACE FALTA. El giroscopio original del delantero SE QUEMO (les
   paso a los robots que compitieron en 2025) y el que hay ahora es un
   REEMPLAZO del que no se sabe si es el mismo modelo. Indicacion del profe
   Gustavo: antes de tocar una linea de firmware, averiguar QUE chip es y
   leerlo EN CRUDO.

   LO QUE SE SABE HASTA HOY:
   - El firmware dice "contesta pero da ceros (9/20 lecturas utiles)" y lo
     da por muerto.
   - El 18/08 Gustavo giro el robot a mano 20 segundos y el rumbo quedo
     CONGELADO en 360.
   - El ARQUERO tiene EXACTAMENTE el mismo codigo de arranque y a el SI le
     anda. Eso apunta a hardware, no a software.
   - El codigo campeon 2025 arrancaba el sensor SIN NINGUNA verificacion:
     bno.begin(), setExtCrystalUse(true), y a leer. La verificacion de las
     20 lecturas es un agregado de 2026 — puede estar rechazando un sensor
     sano, o el sensor puede estar realmente roto. Eso es lo que se mide aca.

   NO MUEVE LOS MOTORES. Los apaga al arrancar y no los vuelve a tocar.
   Se puede correr arriba de la mesa, con el robot enchufado al USB.

   COMO SE USA: cargarlo, abrir el monitor a 19200, y seguir lo que pide.
   En las fases 3 y 4 hay que GIRAR EL ROBOT A MANO, despacio, como media
   vuelta y volver. Todo lo demas es automatico.

   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// Pines de motor — SOLO para apagarlos. Son los del DELANTERO, medidos en
// banco el 28/07. No importa cual es cual: se ponen todos en cero.
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6
#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4
#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

const uint8_t DIR_BNO_A = 0x28;   // donde deberia estar
const uint8_t DIR_BNO_B = 0x29;   // la alternativa, si le pusieron el puente

// ---- registros del BNO055 (hoja de datos, tabla 4-2) ----
const uint8_t REG_CHIP_ID    = 0x00;   // tiene que dar 0xA0
const uint8_t REG_ACC_ID     = 0x01;
const uint8_t REG_MAG_ID     = 0x02;
const uint8_t REG_GYR_ID     = 0x03;
const uint8_t REG_PAGE_ID    = 0x07;
const uint8_t REG_EUL_H_LSB  = 0x1A;   // rumbo, 16 LSB por grado
const uint8_t REG_GYR_X_LSB  = 0x14;   // giroscopo CRUDO, 16 LSB por grado/s
const uint8_t REG_CALIB_STAT = 0x35;
const uint8_t REG_SYS_STATUS = 0x39;   // <-- EL REGISTRO QUE DECIDE
const uint8_t REG_SYS_ERR    = 0x3A;
const uint8_t REG_OPR_MODE   = 0x3D;

Adafruit_BNO055 bno = Adafruit_BNO055(55, DIR_BNO_A);

uint8_t dirEncontrada = 0;     // donde apareció de verdad
bool    esBNO = false;         // el chip ID dio 0xA0?

// ---------------------------------------------------------------------
// I2C en crudo — sin la libreria de Adafruit de por medio.
// La gracia de leer asi es que no dependemos de que la libreria interprete
// bien: vemos el byte tal cual sale del chip.
// ---------------------------------------------------------------------
bool leerReg(uint8_t dir, uint8_t reg, uint8_t *valor) {
  Wire.beginTransmission(dir);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)dir, 1) != 1) return false;
  *valor = Wire.read();
  return true;
}

bool leerRegs(uint8_t dir, uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(dir);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)dir, (int)n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

void apagarMotores() {
  int pines[9] = { IZQ_INA, IZQ_INB, IZQ_PWM, DER_INA, DER_INB, DER_PWM,
                   TRA_INA, TRA_INB, TRA_PWM };
  for (int i = 0; i < 9; i++) { pinMode(pines[i], OUTPUT); digitalWrite(pines[i], LOW); }
}

void titulo(const char *t) {
  Serial.println();
  Serial.println("======================================================");
  Serial.println(t);
  Serial.println("======================================================");
}

// ---------------------------------------------------------------------
// PASO 1 — que hay en el bus I2C
// ---------------------------------------------------------------------
void escanearBus() {
  titulo("PASO 1 — QUE HAY EN EL BUS I2C");
  Serial.println("Si el BNO055 esta, tiene que aparecer en 0x28 (o 0x29).");
  Serial.println("Si no aparece ahi, ES OTRO CHIP y la libreria no sirve.");
  Serial.println("  De referencia: MPU-6050 y compania viven en 0x68/0x69,");
  Serial.println("  el LSM6DS en 0x6A/0x6B.");
  Serial.println();

  int encontrados = 0;
  for (uint8_t dir = 1; dir < 127; dir++) {
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) {
      encontrados++;
      Serial.print("   dispositivo en 0x");
      if (dir < 16) Serial.print("0");
      Serial.print(dir, HEX);
      if (dir == DIR_BNO_A || dir == DIR_BNO_B) {
        Serial.print("   <-- donde deberia estar el BNO055");
        if (!dirEncontrada) dirEncontrada = dir;
      }
      Serial.println();
    }
  }
  Serial.println();
  if (encontrados == 0) {
    Serial.println("!! NO HAY NADA EN EL BUS I2C.");
    Serial.println("!! Eso NO es un problema de firmware: revisar cableado,");
    Serial.println("!! alimentacion del sensor, y los pines 18 y 19 (SDA/SCL).");
  } else if (!dirEncontrada) {
    Serial.println("!! HAY ALGO, PERO NO EN 0x28 NI 0x29.");
    Serial.println("!! Es otro chip. Anotar la direccion de arriba y buscar cual es.");
  } else {
    Serial.print("OK: hay un dispositivo en 0x");
    Serial.println(dirEncontrada, HEX);
  }
}

// ---------------------------------------------------------------------
// PASO 2 — quien dice ser
// ---------------------------------------------------------------------
void identificar() {
  titulo("PASO 2 — QUE CHIP DICE SER");
  if (!dirEncontrada) { Serial.println("(no hay nada en 0x28/0x29, se saltea)"); return; }

  uint8_t id = 0, acc = 0, mag = 0, gyr = 0;
  leerReg(dirEncontrada, REG_CHIP_ID, &id);
  leerReg(dirEncontrada, REG_ACC_ID,  &acc);
  leerReg(dirEncontrada, REG_MAG_ID,  &mag);
  leerReg(dirEncontrada, REG_GYR_ID,  &gyr);

  Serial.print("   CHIP_ID  = 0x"); Serial.print(id, HEX);
  Serial.print("   (un BNO055 tiene que decir 0xA0)");
  esBNO = (id == 0xA0);
  Serial.println(esBNO ? "   OK" : "   <-- NO COINCIDE");

  Serial.print("   ACC_ID   = 0x"); Serial.print(acc, HEX); Serial.println("   (esperado 0xFB)");
  Serial.print("   MAG_ID   = 0x"); Serial.print(mag, HEX); Serial.println("   (esperado 0x32)");
  Serial.print("   GYR_ID   = 0x"); Serial.print(gyr, HEX); Serial.println("   (esperado 0x0F)");
  Serial.println();
  if (!esBNO) {
    Serial.println("!! NO ES UN BNO055. Todo el codigo que usa Adafruit_BNO055");
    Serial.println("!! esta hablando con el chip equivocado. Anotar los IDs.");
  } else {
    Serial.println("Se presenta como un BNO055. OJO: los clones tambien lo hacen —");
    Serial.println("responden el ID correcto y traen la fusion rota. Eso lo decide");
    Serial.println("el PASO 3, no este.");
  }
}

// ---------------------------------------------------------------------
// El registro que decide: SYS_STATUS
// ---------------------------------------------------------------------
void mostrarEstado(const char *cuando) {
  if (!dirEncontrada) return;
  uint8_t sys = 0, err = 0, cal = 0, modo = 0;
  leerReg(dirEncontrada, REG_SYS_STATUS, &sys);
  leerReg(dirEncontrada, REG_SYS_ERR,    &err);
  leerReg(dirEncontrada, REG_CALIB_STAT, &cal);
  leerReg(dirEncontrada, REG_OPR_MODE,   &modo);

  Serial.print("   ["); Serial.print(cuando); Serial.print("]  SYS_STATUS=");
  Serial.print(sys); Serial.print(" (");
  switch (sys) {
    case 0: Serial.print("dormido");                    break;
    case 1: Serial.print("!! ERROR DE SISTEMA");        break;
    case 2: Serial.print("inicializando perifericos");  break;
    case 3: Serial.print("inicializando sistema");      break;
    case 4: Serial.print("autotest");                   break;
    case 5: Serial.print("FUSION CORRIENDO <-- lo que queremos"); break;
    case 6: Serial.print("!! corriendo SIN FUSION");    break;
    default: Serial.print("valor raro");                break;
  }
  Serial.print(")   SYS_ERR="); Serial.print(err);
  Serial.print("   OPR_MODE=0x"); Serial.print(modo, HEX);
  Serial.print("   calib sys/gyr/acc/mag=");
  Serial.print((cal >> 6) & 3); Serial.print("/");
  Serial.print((cal >> 4) & 3); Serial.print("/");
  Serial.print((cal >> 2) & 3); Serial.print("/");
  Serial.println(cal & 3);
}

// Rumbo leido EN CRUDO de los registros de Euler, sin la libreria.
float rumboCrudo() {
  uint8_t b[2];
  if (!leerRegs(dirEncontrada, REG_EUL_H_LSB, b, 2)) return -1.0;
  int16_t v = (int16_t)((b[1] << 8) | b[0]);
  return v / 16.0;                       // 16 LSB por grado
}

// Velocidad angular CRUDA del eje Z. Esto es el PLAN B: no pasa por la
// fusion, sale directo del giroscopo. Si esto se mueve y el rumbo no, la
// fusion esta rota pero el sensor sirve igual.
float gyroZcrudo() {
  uint8_t b[6];
  if (!leerRegs(dirEncontrada, REG_GYR_X_LSB, b, 6)) return 0.0;
  int16_t z = (int16_t)((b[5] << 8) | b[4]);
  return z / 16.0;                       // 16 LSB por grado/s
}

// ---------------------------------------------------------------------
// Una tanda de observacion: mira el rumbo mientras el equipo gira el robot
// ---------------------------------------------------------------------
void observar(const char *etiqueta, unsigned long segundos) {
  Serial.println();
  Serial.print(">>> "); Serial.print(etiqueta);
  Serial.print(" — GIRA EL ROBOT A MANO, despacio, media vuelta y volve. ");
  Serial.print(segundos); Serial.println(" s.");
  Serial.println("    (si no lo tocas, el rumbo NO tiene por que cambiar)");

  float rMin = 9999, rMax = -9999, gMax = 0;
  float integrado = 0;                 // el plan B, integrando el giro crudo
  unsigned long t0 = millis(), tUlt = millis(), tAviso = 0;
  int muestras = 0, ceros = 0;

  while (millis() - t0 < segundos * 1000UL) {
    float r = rumboCrudo();
    float g = gyroZcrudo();
    unsigned long ahora = millis();
    float dt = (ahora - tUlt) / 1000.0;
    tUlt = ahora;
    integrado += g * dt;                          // grados = grados/s x s
    if (fabs(g) > fabs(gMax)) gMax = g;
    if (r >= 0) {
      muestras++;
      if (r == 0.0) ceros++;
      if (r < rMin) rMin = r;
      if (r > rMax) rMax = r;
    }
    if (ahora - tAviso > 1000) {
      tAviso = ahora;
      Serial.print("      rumbo="); Serial.print(r, 1);
      Serial.print("   gyroZ="); Serial.print(g, 1); Serial.print(" gr/s");
      Serial.print("   integrado="); Serial.print(integrado, 1);
      Serial.println(" gr");
    }
    delay(20);
  }

  float recorrido = (muestras && rMax >= rMin) ? (rMax - rMin) : 0;
  Serial.println();
  Serial.print("   RESULTADO ["); Serial.print(etiqueta); Serial.println("]");
  Serial.print("      rumbo:  min="); Serial.print(rMin, 1);
  Serial.print("  max="); Serial.print(rMax, 1);
  Serial.print("  RECORRIDO="); Serial.print(recorrido, 1); Serial.println(" grados");
  Serial.print("      lecturas en cero: "); Serial.print(ceros);
  Serial.print(" de "); Serial.println(muestras);
  Serial.print("      giroscopo crudo: pico="); Serial.print(gMax, 1);
  Serial.print(" gr/s   integrado="); Serial.print(integrado, 1); Serial.println(" gr");
  Serial.println();

  // La lectura del resultado, dicha en criollo
  if (recorrido < 5 && fabs(gMax) < 20) {
    Serial.println("      -> NO SE MOVIO NADA. O no giraste el robot, o el sensor");
    Serial.println("         esta completamente mudo. Repetir girandolo de verdad.");
  } else if (recorrido < 5 && fabs(gMax) >= 20) {
    Serial.println("      -> 🔴 EL GIROSCOPO CRUDO SE MUEVE PERO EL RUMBO NO.");
    Serial.println("         Esto es la FUSION ROTA, con el sensor fisico SANO.");
    Serial.println("         Ningun arreglo de firmware la va a hacer andar — pero");
    Serial.println("         el PLAN B si funciona: integrar el giro crudo, que es");
    Serial.println("         justo lo que dice la columna 'integrado' de arriba.");
  } else {
    Serial.println("      -> ✅ EL RUMBO SE MUEVE. El sensor mide.");
    Serial.println("         Si el firmware igual lo da por muerto, el problema es");
    Serial.println("         la VERIFICACION de arrancarGiroscopo(), no el sensor.");
  }
}

void setup() {
  apagarMotores();
  Serial.begin(19200);
  Wire.begin();
  delay(50);
  while (!Serial && millis() < 4000) { }

  Serial.println();
  Serial.println("######################################################");
  Serial.println("#  GIROSCOPO EN CRUDO — delantero (Teensy 15708680)  #");
  Serial.println("#  No mueve los motores. Se corre sobre la mesa.      #");
  Serial.println("######################################################");

  escanearBus();
  identificar();

  titulo("PASO 3 — ESTADO ANTES DE TOCAR NADA");
  Serial.println("Asi esta el chip recien alimentado, sin que nadie lo configure.");
  mostrarEstado("crudo");

  // ---------------- fase A: como lo arranca el firmware de hoy ----------
  titulo("PASO 4 — ARRANQUE **CON** setExtCrystalUse(true)");
  Serial.println("Es como lo arranca el firmware de hoy y como lo hacia el 2025.");
  Serial.println("setExtCrystalUse le pide al chip que use un cristal externo de");
  Serial.println("32 kHz. Si esta placa no lo tiene, el reloj queda mal y la");
  Serial.println("fusion se detiene: ese es el sospechoso anotado el 18/08.");
  Serial.println();

  if (!bno.begin()) {
    Serial.println("!! bno.begin() FALLO. La libreria no reconocio el chip.");
  } else {
    Serial.println("bno.begin() OK.");
    mostrarEstado("recien begin");
    delay(700);
    mostrarEstado("700 ms despues");
    bno.setExtCrystalUse(true);
    Serial.println("   -> setExtCrystalUse(true) aplicado");
    delay(1000);
    mostrarEstado("1 s despues del cristal");
    observar("CON cristal externo", 15);
  }

  // ---------------- fase B: sin el cristal externo -----------------------
  titulo("PASO 5 — ARRANQUE **SIN** setExtCrystalUse");
  Serial.println("La misma prueba, pero dejando el reloj interno del chip.");
  Serial.println("Si aca la fusion arranca y antes no, el culpable era esa linea.");
  Serial.println();

  if (!bno.begin()) {
    Serial.println("!! bno.begin() FALLO en el segundo intento.");
  } else {
    Serial.println("bno.begin() OK (sin tocar el cristal).");
    delay(1000);
    mostrarEstado("1 s despues, sin cristal");
    observar("SIN cristal externo", 15);
  }

  titulo("LISTO — QUE MIRAR");
  Serial.println("1. PASO 1: si no aparecio nada en 0x28/0x29, es OTRO CHIP.");
  Serial.println("2. PASO 2: si CHIP_ID no dio 0xA0, es OTRO CHIP.");
  Serial.println("3. SYS_STATUS: 5 = fusion corriendo. 6 = corriendo SIN fusion.");
  Serial.println("   Ese numero decide si el problema es hardware o nuestro codigo.");
  Serial.println("4. RECORRIDO del rumbo: si giraste el robot y dio ~0, esta");
  Serial.println("   congelado (es lo que vio Gustavo el 18/08).");
  Serial.println("5. Si el rumbo NO se mueve pero 'integrado' SI, el sensor sirve");
  Serial.println("   igual: se puede llevar el rumbo integrando el giro crudo.");
  Serial.println();
  Serial.println("Abajo queda una lectura en vivo. Segui girando el robot para");
  Serial.println("mirar como responde. Para repetir todo, desenchufa y enchufa.");
  Serial.println();
}

void loop() {
  static unsigned long t = 0;
  if (millis() - t < 500) return;
  t = millis();
  if (!dirEncontrada) { Serial.println("(sin chip en el bus)"); return; }

  uint8_t sys = 0;
  leerReg(dirEncontrada, REG_SYS_STATUS, &sys);
  Serial.print("rumbo="); Serial.print(rumboCrudo(), 1);
  Serial.print("   gyroZ="); Serial.print(gyroZcrudo(), 1); Serial.print(" gr/s");
  Serial.print("   SYS_STATUS="); Serial.print(sys);
  Serial.println(sys == 5 ? "  (fusion OK)" : "  (fusion NO)");
}
