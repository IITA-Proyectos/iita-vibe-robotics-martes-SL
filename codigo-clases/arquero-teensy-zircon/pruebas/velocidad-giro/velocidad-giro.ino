/* =====================================================================
   VELOCIDAD-GIRO — ¿a que fuerza EMPIEZA a girar sobre su eje?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-17
   =====================================================================

   PARA QUE SIRVE
   El robot gira sobre su eje en varios lugares del programa de partido: al
   enderezarse, al acomodarse, y en LA BARRIDA. Y en todos esos lugares hay
   un numero puesto a ojo:

        PWM_BARRIDA_TRES = 80        (la barrida, con las tres ruedas)
        PWM_MIN/MAX_ACOMODO = 75/85  (enderezarse)

   Vienen de "abajo de ~70 un motor parado no arranca", que se midio el
   2026-09-01 — ANTES de mover los motores adentro del chasis (12/10). O sea
   que hoy nadie sabe si 80 alcanza.

   Esto lo mide: subis y bajas la fuerza con dos teclas, el robot gira sobre
   su eje, y el programa te dice CUANTOS GRADOS POR SEGUNDO esta girando de
   verdad, medidos con el giroscopio. Asi "giro" o "zumbo" deja de ser una
   opinion.

   🎯 Y lleva solo la cuenta de LA FUERZA MINIMA a la que giro, para cada
   sentido. Ese es el numero que se vino a buscar.

   ---------------------------------------------------------------------
   LAS TECLAS
   ---------------------------------------------------------------------
        espacio   girar / parar
        s         subir la fuerza   (+5)
        a         bajar la fuerza   (-5)
        S / A     subir / bajar de a 1 (para afinar)
        d         cambiar el sentido (derecha / izquierda)
        0         borrar los minimos encontrados
        ?         esta ayuda

   ---------------------------------------------------------------------
   🚨 EL ROBOT GIRA, PERO NO SE TRASLADA
   ---------------------------------------------------------------------
   Las tres ruedas al mismo sentido = rotacion pura. Se queda en el lugar,
   asi que se puede hacer sobre la mesa — igual conviene el piso de la
   cancha, porque la superficie cambia el roce y lo que interesa es el roce
   de la cancha.

   Arranca PARADO: no se mueve hasta que aprietes espacio.

   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA (el giroscopio come de ahi), y
   prendida ANTES de enchufar el USB.
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

const unsigned long BAUDIOS = 19200;

// ---- la fuerza con la que arranca la prueba ----
// Bien abajo del piso conocido (~70) a proposito: la idea es SUBIR hasta
// que empiece a girar, no bajar hasta que deje.
const int PWM_INICIAL = 40;
const int PWM_MIN = 0;
const int PWM_MAX = 255;

// "Esta girando de verdad" = mas que esto, en grados por segundo.
// El ruido de la medicion con el robot quieto queda bien abajo de 5.
//   Si dice que gira con el robot quieto: subir (8).
//   Si gira despacito y dice que no: bajar (3).
const float GRADOS_POR_SEG_PARA_GIRAR = 5.0;

// Cada cuanto se mide y cada cuanto se dibuja.
const unsigned long MS_MEDIDA = 50;
const unsigned long MS_DIBUJO = 150;

// Suavizado de los grados por segundo (0 a 1: mas chico = mas suave).
const float SUAVIZADO = 0.25;

bool hayGiroscopo = false;
bool girando = false;
int  pwm = PWM_INICIAL;
int  sentido = +1;                  // +1 = derecha (como el rumbo), -1 = izquierda

float gradosPorSeg = 0;
float rumboAnterior = -1;
unsigned long t_anterior = 0;
float gradosGirados = 0;            // acumulado desde que empezo a girar

// 🎯 Lo que se vino a buscar.
int minimoQueGiro[2] = { 0, 0 };    // [0] = derecha, [1] = izquierda
float gradosAlMinimo[2] = { 0, 0 };


// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Las TRES ruedas al mismo sentido = rotacion pura sobre el eje. Es lo
// mismo que hace la barrida con barridaConTresRuedas.
void girarEnElLugar(int v) {
  bool s = (v >= 0);
  int p = abs(v);
  digitalWrite(INA1, s ? 1 : 0); digitalWrite(INB1, s ? 0 : 1);
  digitalWrite(INA2, s ? 1 : 0); digitalWrite(INB2, s ? 0 : 1);
  digitalWrite(INA3, s ? 1 : 0); digitalWrite(INB3, s ? 0 : 1);
  analogWrite(PWM1, p); analogWrite(PWM2, p); analogWrite(PWM3, p);
}


// ---------------------------------------------------------------- giroscopio

// El rumbo, o -1 si no esta dando datos.
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

// La diferencia mas corta entre dos rumbos. Sin esto, pasar de 359 a 1 se
// leeria como un giro de -358 en vez de +2.
float diferencia(float a, float b) {
  float d = a - b;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void medir() {
  float r = rumboActual();
  unsigned long ahora = millis();
  if (r < 0) { rumboAnterior = -1; return; }
  if (rumboAnterior < 0) { rumboAnterior = r; t_anterior = ahora; return; }

  float dt = (ahora - t_anterior) / 1000.0;
  if (dt <= 0.001) return;
  float d = diferencia(r, rumboAnterior);
  rumboAnterior = r;
  t_anterior = ahora;

  float v = d / dt;
  gradosPorSeg = gradosPorSeg * (1.0 - SUAVIZADO) + v * SUAVIZADO;
  if (girando) gradosGirados += d;

  // 🎯 Anotar el minimo al que giro de verdad, para este sentido.
  if (girando && fabs(gradosPorSeg) >= GRADOS_POR_SEG_PARA_GIRAR) {
    int i = (sentido > 0) ? 0 : 1;
    if (minimoQueGiro[i] == 0 || pwm < minimoQueGiro[i]) {
      minimoQueGiro[i] = pwm;
      gradosAlMinimo[i] = fabs(gradosPorSeg);
    }
  }
}


// ---------------------------------------------------------------- pantalla

void irArriba()    { Serial.print("\033[H"); }
void limpiarTodo() { Serial.print("\033[2J\033[H"); }
void finDeLinea()  { Serial.print("\033[K\r\n"); }

void dibujar() {
  irArriba();
  Serial.print("=== VELOCIDAD DE GIRO SOBRE EL EJE ===================");
  finDeLinea();
  finDeLinea();

  Serial.print("  fuerza (PWM) : ");  Serial.print(pwm);
  Serial.print("        sentido: ");
  Serial.print(sentido > 0 ? "DERECHA" : "IZQUIERDA");
  finDeLinea();

  Serial.print("  estado       : ");
  Serial.print(girando ? "GIRANDO" : "parado (apreta espacio)");
  finDeLinea();
  finDeLinea();

  if (!hayGiroscopo) {
    Serial.print("  🚨 SIN GIROSCOPIO: puedo girar, pero no puedo medir.");
    finDeLinea();
    Serial.print("     ¿Esta prendida la bateria? Mientras tanto, a ojo.");
    finDeLinea();
  } else {
    Serial.print("  grados/seg   : ");  Serial.print(gradosPorSeg, 1);
    if (fabs(gradosPorSeg) >= GRADOS_POR_SEG_PARA_GIRAR) Serial.print("   >> GIRA");
    else if (girando)                                    Serial.print("   >> zumba, no gira");
    finDeLinea();
    Serial.print("  giro total   : ");  Serial.print(gradosGirados, 0);
    Serial.print(" grados");
    finDeLinea();
  }
  finDeLinea();

  Serial.print("  🎯 LA FUERZA MINIMA A LA QUE GIRO:");
  finDeLinea();
  for (int i = 0; i < 2; i++) {
    Serial.print("       ");
    Serial.print(i == 0 ? "a la derecha  : " : "a la izquierda: ");
    if (minimoQueGiro[i] == 0) {
      Serial.print("todavia no giro");
    } else {
      Serial.print(minimoQueGiro[i]);
      Serial.print("   (ahi hacia ");
      Serial.print(gradosAlMinimo[i], 1);
      Serial.print(" grados/seg)");
    }
    finDeLinea();
  }
  finDeLinea();

  // Para comparar con lo que tiene puesto el programa de partido.
  Serial.print("  de referencia: la BARRIDA gira con 80, y enderezarse con 75-85");
  finDeLinea();
  finDeLinea();
  Serial.print("  espacio girar/parar   s/a fuerza +-5   S/A +-1");
  finDeLinea();
  Serial.print("  d sentido   0 borrar minimos   ? ayuda");
  finDeLinea();
}

void ayuda() {
  limpiarTodo();
  Serial.println();
  Serial.println("=== TECLAS ===============================================");
  Serial.println("  espacio   girar / parar");
  Serial.println("  s         subir la fuerza (+5)      S   subir de a 1");
  Serial.println("  a         bajar la fuerza (-5)      A   bajar de a 1");
  Serial.println("  d         cambiar el sentido (derecha / izquierda)");
  Serial.println("  0         borrar los minimos encontrados");
  Serial.println("  ?         esta ayuda");
  Serial.println();
  Serial.println(" COMO SE USA, para que el numero sirva:");
  Serial.println("  1. apoyalo en el PISO DE LA CANCHA (el roce de la mesa es otro)");
  Serial.println("  2. arranca en 40 y subi de a 5 con la tecla s");
  Serial.println("  3. el primer PWM donde diga >> GIRA es el minimo");
  Serial.println("  4. afinalo de a 1 con A, bajando hasta que deje de girar");
  Serial.println("  5. repetilo para el otro sentido con la tecla d: pueden");
  Serial.println("     no dar igual, y el que manda es EL MAS ALTO de los dos");
  Serial.println();
  Serial.println(" (cualquier tecla para volver)");
  Serial.println("==========================================================");
}


// ---------------------------------------------------------------- teclado

bool enAyuda = false;

void leerTeclado() {
  while (Serial.available()) {
    char t = Serial.read();
    if (t == '\n' || t == '\r') continue;

    if (enAyuda) { enAyuda = false; limpiarTodo(); return; }

    switch (t) {
      case ' ':
        girando = !girando;
        if (girando) { gradosGirados = 0; rumboAnterior = -1; }
        else         { parar(); gradosPorSeg = 0; }
        break;
      case 's': pwm += 5; break;
      case 'a': pwm -= 5; break;
      case 'S': pwm += 1; break;
      case 'A': pwm -= 1; break;
      case 'd':
        sentido = -sentido;
        // Parar al cambiar de sentido: invertir de golpe castiga el puente H,
        // y ademas la inercia ensuciaria la medicion del arranque.
        girando = false; parar(); gradosPorSeg = 0;
        break;
      case '0':
        minimoQueGiro[0] = minimoQueGiro[1] = 0;
        gradosAlMinimo[0] = gradosAlMinimo[1] = 0;
        break;
      case '?': enAyuda = true; ayuda(); return;
      default: break;
    }
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    if (pwm < PWM_MIN) pwm = PWM_MIN;
  }
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  parar();                      // arranca PARADO

  Serial.begin(BAUDIOS);
  Wire.begin();
  delay(800);

  limpiarTodo();
  Serial.println("=== VELOCIDAD DE GIRO — arranca PARADO ===");
  Serial.print(" giroscopio: ");
  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    bno.setExtCrystalUse(true);   // primero el cristal: reinicia la fusion
    delay(700);                   // y DESPUES la espera
    Serial.println("OK");
  } else {
    Serial.println("NO CONTESTA (puedo girar, pero no medir)");
  }
  delay(600);
  limpiarTodo();
}


unsigned long t_medida = 0, t_dibujo = 0;

void loop() {
  leerTeclado();
  if (enAyuda) return;

  unsigned long ahora = millis();

  if (ahora - t_medida >= MS_MEDIDA) {
    t_medida = ahora;
    medir();
  }

  if (girando) girarEnElLugar(sentido * pwm);
  else         parar();

  digitalWrite(LED, girando ? HIGH : LOW);

  if (ahora - t_dibujo >= MS_DIBUJO) {
    t_dibujo = ahora;
    dibujar();
  }
}
