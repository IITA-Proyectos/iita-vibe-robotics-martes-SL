/* =====================================================================
   MEDIR-ANCHO-AREA — cuanto tarda de un costado del area al otro
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-09
   =====================================================================

   PEDIDO DEL EQUIPO (textual): "El robot va a estar adentro del area y va
   a tener que ir hacia un lateral del area y el sensor de adelante va a
   tener que detectar la linea blanca para luego ir hacia el otro lado,
   esto nos servira para la patrulla que se medira por tiempo. Hace un
   programa aparte." (Antes de eso: "primero espera 5 segundos".)

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Cargarlo con la bateria APAGADA (arranca solo apenas termina).
   2. Apoyar el robot ADENTRO DEL AREA, mirando derecho a la cancha, con el
      sensor de ADELANTE sobre el VERDE (no arriba de una linea). Prender
      la bateria.
   3. Espera 5 s (LED lento, y rapido los ultimos 2: sacar las manos).
   4. Va de costado a la IZQUIERDA hasta que el sensor de adelante ve la
      linea blanca del COSTADO del area. Frena.
   5. Va a la DERECHA hasta que el sensor de adelante ve la linea del OTRO
      costado del area, CONTANDO EL TIEMPO. Frena y se queda quieto.
   6. Guarda el tiempo en la EEPROM (no se borra al apagar).

   PARA LEERLO: con la bateria apagada, enchufar el USB y abrir
   pruebas/herramientas/mirar.bat. Al arrancar escribe:
        MEDIR-ANCHO-AREA: la ultima medicion fue XXXX ms de costado a costado
   (y si el USB esta enchufado mientras mide, lo escribe ahi mismo).
   La MITAD de ese tiempo es lo que tarda del costado izquierdo al medio.

   EL LED AL FINAL
        latido lento (cada 2 s)  -> midio bien
        2 destellos, repitiendo  -> al arrancar el sensor de adelante estaba
                                    sobre una linea blanca: no mide
        3 destellos, repitiendo  -> no encontro una linea a tiempo (4 s)
        temblor rapido           -> sin giroscopio: no se mueve

   Las piezas de moverse (rueda, aplicar, frenar, rampa, rumbo con el
   giroscopio, moverDeCostado) son COPIA de funciona/partido-0921 (0921i),
   con los mismos numeros. VEL_LINEA es la MISMA de la patrulla: si se
   cambia alla, cambiarla aca, o el tiempo no sirve.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <EEPROM.h>

// ---- pines (los del arquero, ROBOT1) ----
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = IZQUIERDA (U5)
#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = DERECHA (U17)
#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = TRASERA (U7)
#define LED 13
#define LINEA_ADELANTE A12

// ---- los numeros ----
const int   umbralBlanco = 425;                 // el del partido
const int   VEL_LINEA = 90;                     // 🚨 la MISMA que la patrulla de la 0921i
const unsigned long MS_CUENTA = 5000;           // "primero espera 5 segundos"
// La LINEA del costado: blanco todo este rato seguido. Es corto a proposito:
// yendo a 90 el sensor pasa por arriba de la linea en muy poco tiempo. Si
// frena antes de llegar (por un reflejo): subir. Si se pasa de la linea sin
// verla: bajar.
const unsigned long MS_BLANCO_SOSTENIDO = 15;
// Al arrancar cada tramo no mira la linea durante esto.
const unsigned long MS_MIN_ANTES_DE_LA_LINEA = 200;
// Tope de cada tramo: si no ve la linea, se para (adentro del area son
// menos de 2 s; 4 s y ya esta saliendo del area).
const unsigned long MS_TOPE_TRAMO = 4000;
const unsigned long MS_QUIETO = 500;            // quieto entre los dos tramos
const unsigned long MS_FRENO = 200;             // freno electrico sostenido
const int LADO_FRENTE  = 50;                    // (del partido) la mezcla de costado
const int LADO_TRASERA = 89;
const bool lateralInvertido = false;            // (del partido)
const int RAMPA_MOV_PASO = 10;                  // (del partido) arranque suave
const unsigned long RAMPA_MOV_MS = 10;
const float KP_RUMBO = 5.0;                     // (del partido) correccion de rumbo
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;
const unsigned long MS_ESPERA_DATOS_GIRO = 5000;

// ---- la EEPROM: 3600..3611 (libre en el mapa de medir-ancho) ----
const int DIR_MEDICION = 3600;
const uint32_t MAGIA_MEDICION = 0x4D414131;     // "MAA1"
struct Medicion { uint32_t magia; uint32_t ms; uint32_t veces; };

// ---- estado ----
enum Fase { CUENTA, SIN_GIRO, A_LA_IZQUIERDA, QUIETO, A_LA_DERECHA, TERMINADO, ERROR_LINEA, ERROR_TOPE };
Fase fase = CUENTA;
unsigned long t_fase = 0;
unsigned long t_blancoDesde = 0;
bool salioDelBlanco = false;
unsigned long msMedido = 0;
// 2026-11-09 (2), lo que vio el equipo: "detecta con el sensor de adelante
// pero se va un poco del blanco (con el envion del freno), y cuando quiere ir
// hacia el otro lateral detecta la MISMA linea pensando que es el otro
// lateral y se frena ahi". Entonces el tramo de vuelta primero tiene que
// DEJAR ATRAS la linea de la que salio:
//   - si arranca ENCIMA del blanco: salir del blanco;
//   - si arranca en el VERDE (se paso): cruzarla entera (blanco y verde).
// Recien despues, el proximo blanco es el OTRO costado.
bool pasoLaPrimeraLinea = false;   // ya dejo atras la linea del costado izquierdo
bool vioLaPrimeraLinea = false;    // en la vuelta, ya paso por arriba de esa linea
unsigned long t_verdeDesde = 0;
// Para dar por "dejada atras" la primera linea: verde todo este rato.
const unsigned long MS_VERDE_SOSTENIDO = 30;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
float rumboBase = -1;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;


// ================================================== motores (del partido)
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

void rueda(int ina, int inb, int pwm, int v) {
  if (v > 255)  v = 255;
  if (v < -255) v = -255;
  digitalWrite(ina, v >= 0 ? 1 : 0);
  digitalWrite(inb, v >= 0 ? 0 : 1);
  analogWrite(pwm, abs(v));
}

void aplicar(int v1, int v2, int v3) {
  rueda(INA1, INB1, PWM1, v1);
  rueda(INA2, INB2, PWM2, v2);
  rueda(INA3, INB3, PWM3, v3);
}

void reiniciarRampaMovimiento() {
  potenciaRampa = 0;
  t_rampaMov = millis();
}

int rampa(int objetivo) {
  unsigned long ahora = millis();
  if (objetivo < potenciaRampa) {
    potenciaRampa = objetivo;
  } else if (ahora - t_rampaMov >= RAMPA_MOV_MS) {
    t_rampaMov = ahora;
    potenciaRampa += RAMPA_MOV_PASO;
    if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  }
  return potenciaRampa;
}


// ================================================== rumbo (del partido)
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0 && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

int correccionDeRumbo() {
  if (rumboBase < 0) return 0;
  float r = rumboActual();
  if (r < 0) return 0;
  float error = diferencia(rumboBase, r);
  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;
  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;
  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;
  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  return (int)c;
}

// s > 0 = a la DERECHA (del partido).
void moverDeCostado(int s) {
  if (lateralInvertido) s = -s;
  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;
  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;
  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}


// ================================================== la medicion
bool adelanteVeBlanco() { return analogRead(LINEA_ADELANTE) >= umbralBlanco; }

// Un paso de un tramo hacia `lado`. Devuelve true al llegar a la LINEA del
// costado del area: el sensor de adelante estuvo en verde en este tramo y
// despues ve blanco MS_BLANCO_SOSTENIDO seguidos (sin mirar los primeros
// MS_MIN_ANTES_DE_LA_LINEA). Lo de "estuvo en verde" es para el segundo
// tramo: arranca con el sensor todavia arriba de la primera linea.
bool tramo(int lado, unsigned long ahora) {
  moverDeCostado(lado * VEL_LINEA);
  bool blanco = adelanteVeBlanco();

  // Solo en la vuelta: primero dejar atras la linea de la que salio.
  if (fase == A_LA_DERECHA && !pasoLaPrimeraLinea) {
    if (blanco) { vioLaPrimeraLinea = true; t_verdeDesde = 0; }
    else if (vioLaPrimeraLinea) {
      if (t_verdeDesde == 0) t_verdeDesde = ahora;
      if (ahora - t_verdeDesde >= MS_VERDE_SOSTENIDO) pasoLaPrimeraLinea = true;
    }
    t_blancoDesde = 0;
    return false;
  }

  if (!blanco) salioDelBlanco = true;
  if (!blanco || !salioDelBlanco || ahora - t_fase < MS_MIN_ANTES_DE_LA_LINEA) { t_blancoDesde = 0; return false; }
  if (t_blancoDesde == 0) t_blancoDesde = ahora;
  return ahora - t_blancoDesde >= MS_BLANCO_SOSTENIDO;
}

void empezar(Fase f) {
  fase = f;
  t_fase = millis();
  t_blancoDesde = 0;
  salioDelBlanco = false;
  pasoLaPrimeraLinea = false;
  vioLaPrimeraLinea = adelanteVeBlanco();   // ¿arranca encima de la linea, o se paso?
  t_verdeDesde = 0;
  reiniciarRampaMovimiento();
  reiniciarCorreccion();
}

void contarLoGuardado() {
  if (!Serial) return;
  Medicion m;
  EEPROM.get(DIR_MEDICION, m);
  Serial.print("MEDIR-ANCHO-AREA: ");
  if (m.magia == MAGIA_MEDICION) {
    Serial.print("la ultima medicion fue "); Serial.print(m.ms);
    Serial.print(" ms de costado a costado del area, a VEL_LINEA "); Serial.print(VEL_LINEA);
    Serial.print(" -> al medio "); Serial.print(m.ms / 2);
    Serial.print(" ms (mediciones hechas: "); Serial.print(m.veces); Serial.println(")");
  } else {
    Serial.println("todavia no hay ninguna medicion guardada");
  }
}

void guardar(unsigned long ms) {
  Medicion m;
  EEPROM.get(DIR_MEDICION, m);
  uint32_t veces = (m.magia == MAGIA_MEDICION) ? m.veces + 1 : 1;
  m.magia = MAGIA_MEDICION; m.ms = ms; m.veces = veces;
  EEPROM.put(DIR_MEDICION, m);
}

// N destellos y una pausa, repitiendo.
void destellos(int n, unsigned long ahora) {
  unsigned long ciclo = (unsigned long)n * 400 + 1200;
  unsigned long t = ahora % ciclo;
  digitalWrite(LED, (t < (unsigned long)n * 400 && (t % 400) < 150) ? HIGH : LOW);
}


// ================================================== programa
void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ADELANTE, INPUT);
  parar();
  Serial.begin(19200);

  // El giroscopio, con reintentos (como el partido).
  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    unsigned long t0 = millis();
    while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) delay(50);
  }

  if (Serial) {
    Serial.println("=== MEDIR-ANCHO-AREA (2026-11-09) — 🚨 EL ROBOT SE MUEVE ===");
    Serial.println(" adentro del area, sensor de ADELANTE en verde; 5 s, izquierda hasta la linea, derecha hasta la otra.");
  }
  contarLoGuardado();
  fase = CUENTA; t_fase = millis();
}

void loop() {
  unsigned long ahora = millis();

  switch (fase) {
    case CUENTA: {
      parar();
      unsigned long falta = (ahora - t_fase >= MS_CUENTA) ? 0 : MS_CUENTA - (ahora - t_fase);
      digitalWrite(LED, ((ahora / ((falta <= 2000) ? 100 : 500)) % 2) ? HIGH : LOW);
      if (falta > 0) break;
      float r = rumboActual();
      if (r < 0) { fase = SIN_GIRO; break; }
      rumboBase = r;
      if (adelanteVeBlanco()) {
        if (Serial) Serial.println("!! el sensor de ADELANTE esta sobre una linea blanca: ponerlo en el verde");
        fase = ERROR_LINEA; break;
      }
      empezar(A_LA_IZQUIERDA);
      break;
    }

    case A_LA_IZQUIERDA:
      digitalWrite(LED, HIGH);
      if (tramo(-1, ahora)) {
        frenar();
        fase = QUIETO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_TOPE_TRAMO) { frenar(); fase = ERROR_TOPE; t_fase = ahora; }
      break;

    case QUIETO:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      parar();
      if (ahora - t_fase >= MS_FRENO + MS_QUIETO) empezar(A_LA_DERECHA);
      break;

    case A_LA_DERECHA:
      digitalWrite(LED, HIGH);
      if (tramo(+1, ahora)) {
        msMedido = ahora - t_fase;
        frenar();
        guardar(msMedido);
        if (Serial) {
          Serial.print("MEDIR-ANCHO-AREA: de costado a costado tardo "); Serial.print(msMedido);
          Serial.print(" ms (guardado en la memoria) -> al medio "); Serial.print(msMedido / 2);
          Serial.println(" ms");
        }
        fase = TERMINADO; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_TOPE_TRAMO) { frenar(); fase = ERROR_TOPE; t_fase = ahora; }
      break;

    case TERMINADO:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      parar();
      digitalWrite(LED, (ahora % 2000) < 120 ? HIGH : LOW);   // latido lento: midio bien
      break;

    case ERROR_LINEA:
      parar();
      destellos(2, ahora);
      break;

    case ERROR_TOPE:
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      parar();
      destellos(3, ahora);
      break;

    case SIN_GIRO:
      parar();
      digitalWrite(LED, ((ahora / 50) % 2) ? HIGH : LOW);
      break;
  }
}
