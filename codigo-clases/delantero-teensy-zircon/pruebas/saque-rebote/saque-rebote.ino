/* =====================================================================
   SAQUE CON REBOTE - DELANTERO - prueba aparte (08/11)
   =====================================================================
   Robot 2 (ROBOT2) - Teensy 4.1 + Zircon - serie 15708680

   La idea: apenas arranca, una PATADA de saque contra la pared para que
   la pelota rebote y vaya al arco. Despues, en el partido, seguiria
   jugando normal. Aca, como es una PRUEBA, patea, retrocede y se queda
   quieto, para mirar como salio.

   COMO SE USA:
     1. Poner la pelota adelante del robot, y el robot APUNTADO A MANO
        hacia la pared (para el rebote).
     2. Prenderlo. A los ~5 s patea derecho para donde mira (el
        giroscopio lo mantiene derecho), retrocede y frena.
     3. Para repetir: apagar y prender. Con el USB enchufado tambien
        sirve la tecla  p  en la consola.
   LINEA:  si pisa la linea blanca en el golpe o al retroceder, corta y
           se escapa (como en el partido).

   OJO para cuando pase al principal: el giroscopio toma como "rumbo
   cero" (= hacia el arco rival) para donde mira al PRENDERSE. Si se lo
   apunta a mano a la pared, el plan B del rumbo cero apuntaria a la
   pared todo el partido. Hay que resolverlo antes de llevarlo.

   CARGAR: PROBAR-SAQUE.bat. Para jugar: CARGAR-ROBOT.bat.
   Lo marcado "COPIA DEL PRINCIPAL" es del principal del 08/11: si se
   cambia aca, en el partido NO cambia.
   ===================================================================== */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// =====================================================================
//   LO DEL SAQUE: lo que se ajusta en la cancha
//   ms = milisegundos - fuerza 0..255
// =====================================================================
const int VEL_SAQUE                    = 180;   // fuerza del saque. Empieza igual a la patada del partido. Si no llega a la pared: subir (con 190 se volco un poco, 08/11)
const int MS_SAQUE                     = 450;   // cuanto dura el golpe (ms). Mas = la pelota sale mas fuerte, y el robot se acerca mas a la linea
const int VEL_RETROCESO                = 110;   // despues del golpe, marcha atras con esta fuerza...
const int MS_RETROCESO                 = 700;   // ...durante esto (ms), y frena
const unsigned long MS_ESPERA_ARRANQUE = 2000;  // cuenta regresiva al prenderlo, para sacar la mano (ms)

// =====================================================================
//   COPIA DEL PRINCIPAL (08/11): la patada derecha y la linea
// =====================================================================
const int TRIM_PATADA                  = 5;     // compensacion fija. Si se va siempre a la DERECHA: subir. A la IZQUIERDA: bajar
const float KP_PATADA                  = 6.0;   // corrige por GRADO torcido
const float KD_PATADA                  = 0.6;   // corrige por GRADO/SEGUNDO
const int RAMPA_PATADA_PASO            = 15;    // el golpe arranca suave: sube esto de fuerza...
const unsigned long RAMPA_PATADA_MS    = 5;     // ...cada esta cantidad de ms
const int RESTA_MAX                    = 120;   // tope de la correccion, en fuerza
const unsigned long MS_DERIVADA_PATADA = 10;    // cada cuanto calcula el KD (ms). Es el ritmo del sensor: no tocar
const bool GIRO_SIN_BRUJULA            = true;  // giroscopio sin brujula (modo IMU)

const bool LINEA_ACTIVA                = true;
int UMBRAL_LINEA[3]                    = { 300, 300, 440 }; // [09/11: DER e IZQ bajados a 300 por el equipo; eran 424/434 del 20/10]
const unsigned long MS_RETROCESO_LINEA = 210;   // al verla, se escapa esto (ms)...
const int VEL_ESCAPE_FUERTE            = 135;   // ...con esta fuerza
const unsigned long MS_LINEA_CONFIRMA  = 5;     // tiene que verla seguida esto (ms) para creerle

// ===== PINES (medidos en banco el 28/07) =====
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6

#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4

#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3

const int PIN_VERSION_PLACA = 32;     // pin 32 en LOW = Mark1 (linea en A11/A13/A12)

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;

int  pinLinea[3] = { A11, A13, A12 };   // Mark1; se corrige en setup()
bool lineaHabilitada = false;
unsigned long t_sensorDesde[3] = { 0, 0, 0 };

// Lo que paso en el ultimo saque (se repite por el monitor).
int   nSaques       = 0;
float desvioSaque   = 0;     // grados torcido al terminar el golpe
float picoSaque     = 0;     // lo peor durante el golpe
bool  conLazo       = false; // pateo con el giroscopio corrigiendo?
const char* comoTermino = "-";
unsigned long t_ultimoAviso = 0;

// ===== MOTORES (COPIA DEL PRINCIPAL) =====
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

// La patada: empujan las dos de adelante y la TRASERA va trabada.
void motoresPatada(int vi, int vd) {
  if (vi < 0) vi = 0;   if (vi > 255) vi = 255;
  if (vd < 0) vd = 0;   if (vd > 255) vd = 255;
  analogWrite(IZQ_PWM, vi); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vd); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Escapa para el lado contrario al sensor (COPIA DEL PRINCIPAL).
void escaparDeLinea(int m, int velocidad) {
  int v[3] = { 0, 0, 0 };
  if (m & 1) { v[1] -= 1; v[2] += 1; }
  if (m & 2) { v[0] += 1; v[2] -= 1; }
  if (m & 4) { v[0] -= 1; v[1] += 1; }

  int pico = 0;
  for (int i = 0; i < 3; i++) if (abs(v[i]) > pico) pico = abs(v[i]);
  if (pico == 0) { frenar(); return; }

  int pwm[3];
  for (int i = 0; i < 3; i++) pwm[i] = (v[i] * velocidad) / pico;

  analogWrite(IZQ_PWM, abs(pwm[0]));
  digitalWrite(IZQ_INA, pwm[0] > 0 ? 1 : 0);
  digitalWrite(IZQ_INB, pwm[0] < 0 ? 1 : 0);
  analogWrite(DER_PWM, abs(pwm[1]));
  digitalWrite(DER_INA, pwm[1] > 0 ? 1 : 0);
  digitalWrite(DER_INB, pwm[1] < 0 ? 1 : 0);
  analogWrite(TRA_PWM, abs(pwm[2]));
  digitalWrite(TRA_INA, pwm[2] > 0 ? 1 : 0);
  digitalWrite(TRA_INB, pwm[2] < 0 ? 1 : 0);
}

// ===== GIROSCOPIO =====
// Sin el chequeo de estado del principal: ese chequeo clava el programa
// 200 ms (bitacora 13/10), y en un golpe de 450 ms eso es mucho.
float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  return evento.orientation.x;               // 0..360
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

bool arrancarGiroscopo() {
  if (!bno.begin(GIRO_SIN_BRUJULA ? OPERATION_MODE_IMUPLUS : OPERATION_MODE_NDOF)) return false;
  bno.setExtCrystalUse(true);
  delay(700);
  uint8_t sys = 0, autotest = 0, err = 0;
  bno.getSystemStatus(&sys, &autotest, &err);
  if (sys != 5) {
    Serial.print("   la fusion NO esta corriendo (SYS_STATUS="); Serial.print(sys);
    Serial.println("). Pateo sin giroscopio, con el trim solo.");
    return false;
  }
  return true;
}

// ===== LINEA =====
// Devuelve el sensor que la vio PRIMERO (como mascara), o 0 si no hay linea.
int lineaVista() {
  if (!lineaHabilitada) return 0;
  int primero = -1;
  for (int i = 0; i < 3; i++) {
    if (analogRead(pinLinea[i]) >= UMBRAL_LINEA[i]) {
      if (t_sensorDesde[i] == 0) t_sensorDesde[i] = millis();
    } else {
      t_sensorDesde[i] = 0;
    }
    if (t_sensorDesde[i] != 0 && millis() - t_sensorDesde[i] >= MS_LINEA_CONFIRMA)
      if (primero < 0 || t_sensorDesde[i] < t_sensorDesde[primero]) primero = i;
  }
  return (primero < 0) ? 0 : (1 << primero);
}

void escaparYFrenar(int m, const char* cuando) {
  Serial.print("!!! LINEA BLANCA "); Serial.print(cuando);
  Serial.print(" (sensor "); Serial.print(m == 1 ? "1 DERECHO" : (m == 2 ? "2 IZQUIERDO" : "3 DELANTERO"));
  Serial.println(") -> me escapo y freno");
  unsigned long t0 = millis();
  while (millis() - t0 < MS_RETROCESO_LINEA) escaparDeLinea(m, VEL_ESCAPE_FUERTE);
  frenar();
  for (int i = 0; i < 3; i++) t_sensorDesde[i] = 0;
}

// ===== EL SAQUE: golpe derecho + retroceso =====
void hacerSaque() {
  nSaques++;
  Serial.println();
  Serial.print("*** SAQUE "); Serial.print(nSaques);
  Serial.print(": golpe "); Serial.print(VEL_SAQUE); Serial.print(" x ");
  Serial.print(MS_SAQUE); Serial.println(" ms");

  conLazo = hayGiroscopo;
  float rumboObjetivo = hayGiroscopo ? rumboActual() : 0;
  float errPrevio = 0, deriv = 0;
  unsigned long t_deriv = 0;
  int pwmRampa = 0;
  unsigned long t_rampa = millis();
  picoSaque = 0;
  desvioSaque = 0;
  for (int i = 0; i < 3; i++) t_sensorDesde[i] = 0;

  // ---- el golpe ----
  unsigned long t0 = millis();
  while (millis() - t0 < (unsigned long)MS_SAQUE) {
    int m = lineaVista();
    if (m) { escaparYFrenar(m, "en el golpe"); comoTermino = "lo corto la LINEA en el golpe"; return; }

    // rampa: sube de a escalones para que no patinen las ruedas
    if (millis() - t_rampa >= RAMPA_PATADA_MS) {
      t_rampa = millis();
      pwmRampa += RAMPA_PATADA_PASO;
      if (pwmRampa > VEL_SAQUE) pwmRampa = VEL_SAQUE;
    }

    int vi = pwmRampa - TRIM_PATADA;
    int vd = pwmRampa + TRIM_PATADA;
    if (conLazo) {
      float err = diferencia(rumboObjetivo, rumboActual());
      unsigned long ahora = millis();
      if (t_deriv == 0) { errPrevio = err; t_deriv = ahora; }
      else if (ahora - t_deriv >= MS_DERIVADA_PATADA) {
        deriv = (err - errPrevio) * 1000.0 / (float)(ahora - t_deriv);
        errPrevio = err; t_deriv = ahora;
      }
      float mando = KP_PATADA * err + KD_PATADA * deriv;
      int mitad = (int)(fabs(mando) / 2.0);
      if (mitad > RESTA_MAX / 2) mitad = RESTA_MAX / 2;
      if (mando > 0) { vd -= mitad; vi += mitad; }
      else           { vi -= mitad; vd += mitad; }
      if (fabs(err) > fabs(picoSaque)) picoSaque = err;
      desvioSaque = err;
    }
    motoresPatada(vi, vd);
  }

  Serial.print("    golpe terminado: se torcio "); Serial.print(desvioSaque, 1);
  Serial.print(" grados (pico "); Serial.print(picoSaque, 1);
  Serial.println(conLazo ? ")" : ")  [SIN GIROSCOPO: trim solo]");

  // ---- el retroceso ----
  t0 = millis();
  while (millis() - t0 < (unsigned long)MS_RETROCESO) {
    int m = lineaVista();
    if (m) { escaparYFrenar(m, "al retroceder"); comoTermino = "lo corto la LINEA al retroceder"; return; }
    retroceder(VEL_RETROCESO);
  }
  frenar();
  comoTermino = "completo (golpe + retroceso)";
  Serial.println("    retroceso terminado -> quieto");
}

void imprimirResultado() {
  Serial.print("   [SAQUE "); Serial.print(nSaques); Serial.print("] ");
  Serial.print(comoTermino);
  Serial.print("   desvio "); Serial.print(desvioSaque, 1);
  Serial.print(" grados (pico "); Serial.print(picoSaque, 1); Serial.print(")");
  if (!conLazo) Serial.print("  SIN GIROSCOPO");
  Serial.println("   p = otro saque");
}

// ===== ARRANQUE =====
void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }
  Serial.println();
  Serial.println("==============================================");
  Serial.println("SAQUE CON REBOTE - DELANTERO (prueba, NO es el de partido)");
  Serial.print("   al arrancar: golpe "); Serial.print(VEL_SAQUE);
  Serial.print(" x "); Serial.print(MS_SAQUE);
  Serial.print(" ms derecho para donde mira, retrocede "); Serial.print(VEL_RETROCESO);
  Serial.print(" x "); Serial.print(MS_RETROCESO); Serial.println(" ms y frena");
  Serial.println("   apuntarlo A MANO a la pared, con la pelota adelante");
  Serial.println("==============================================");

  pinMode(PIN_VERSION_PLACA, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION_PLACA) == LOW) { pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12; }
  else                                       { pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12; }

  if (LINEA_ACTIVA) {
    lineaHabilitada = true;
    Serial.print("Linea: sensores leen ");
    for (int i = 0; i < 3; i++) { Serial.print(analogRead(pinLinea[i])); if (i < 2) Serial.print(" / "); }
    Serial.println("   (escape ACTIVADO)");
  }

  Serial.print("Giroscopo: ");
  hayGiroscopo = arrancarGiroscopo();
  Serial.println(hayGiroscopo ? "OK (mantiene el saque derecho)" : "NO -> patea con el trim solo");

  Serial.print("Saque en "); Serial.print(MS_ESPERA_ARRANQUE / 1000);
  Serial.println(" segundos. Saca la mano.");
  delay(MS_ESPERA_ARRANQUE);

  hacerSaque();
  imprimirResultado();
  t_ultimoAviso = millis();
}

// ===== LOOP: quieto. Repite el resultado para leerlo despues sin cable =====
void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'p' || c == 'P') {
      Serial.println("Otro saque en 2 segundos...");
      delay(2000);
      hacerSaque();
      imprimirResultado();
      t_ultimoAviso = millis();
    }
  }
  if (millis() - t_ultimoAviso > 5000) {
    t_ultimoAviso = millis();
    imprimirResultado();
  }
}
