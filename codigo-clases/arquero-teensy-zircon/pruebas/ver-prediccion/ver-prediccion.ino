/* =====================================================================
   VER-PREDICCION — ¿la prediccion de la pelota acierta o no?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-10-06
   =====================================================================

   DE QUE SE TRATA
   El 2026-09-08 se programo que el robot no vaya a donde ESTA la pelota
   sino a donde VA A ESTAR. La cuenta es esta:

        desvio_predicho = desvio_de_ahora + velocidad * 0,25 segundos

   Esta escrito en el programa de juego desde ese dia... y NUNCA SE PROBO.
   Y no se pudo probar por un motivo concreto: el programa de juego no tiene
   consola, asi que la prediccion trabaja a ciegas. Desde afuera no hay forma
   de saber si acierta, si llega tarde, o si apunta para el lado contrario.

   ---------------------------------------------------------------------
   🚨 EL RIESGO QUE ESTO VIENE A DESCARTAR
   ---------------------------------------------------------------------
   Si el SIGNO de la velocidad estuviera invertido, la prediccion apuntaria
   DETRAS de la pelota en vez de adelante — y seria PEOR que no predecir.
   Esa cuenta nunca se verifico contra la realidad.

   Por eso este programa dibuja una regla con DOS marcas:

        izq |----------o---------P-------------| der
                       ^         ^
                   donde esta  donde se predice

   🎯 Si hacen rodar la pelota de IZQUIERDA A DERECHA, la P tiene que ir
   ADELANTE de la o, o sea a la DERECHA. Si va atras, el signo esta al
   revés y hay que darlo vuelta antes de usarlo en un partido.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar. Es para mirar,
   con el robot apoyado en la mesa y la pelota rodando delante de la camara.

   COMO SE USA
   Abrir `MIRAR.bat` de esta carpeta (o mirar.bat de herramientas) y hacer
   rodar la pelota cruzando el campo de vision, despacio y rapido.

   Teclas:
        p   pausar / seguir
        0   poner los maximos en cero
        a / s   bajar / subir la anticipacion de a 50 ms
   ===================================================================== */

#include <Arduino.h>

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

const unsigned long BAUDIOS        = 19200;
const unsigned long BAUDIOS_CAMARA = 19200;

// ---- TODO ESTO ES COPIA EXACTA DEL PROGRAMA DE JUEGO ----
// Si alguno de estos numeros cambia alla, hay que cambiarlo aca tambien, o
// la prueba estaria midiendo otro robot.
const float CAMARA_POR_CM       = 2.87;   // unidades de camara por cm real
const bool  camaraYInvertida    = true;
const float SUAVIZADO_VELOCIDAD = 0.3;
const float ZONA_MUERTA_PELOTA  = 1.4;    // cm: no perseguir migajas
const float umbralCm            = 30.0;   // despeja si esta a esto o menos
const float umbralDesvio        = 7.0;    // cm de desvio tolerado
const int   VECES_PARA_CREERLE  = 3;
float msAnticipacion            = 250;    // se puede mover con a / s

// ---- la camara ----
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;
unsigned long paquetesTotales = 0;
float paquetesPorSegundo = 0;

// ---- lo que se calcula, igual que el juego ----
float distanciaCm = 0, desvioCm = 0;
float velocidadLateral = 0, velocidadAcercamiento = 0;
float desvioAnterior = 0, distanciaAnterior = 0;
unsigned long t_desvioAnterior = 0;
bool  hayDesvioAnterior = false;
int   vecesSeguidas = 0;

// ---- maximos, para saber que tan rapido llego a ir la pelota ----
float velMax = 0, velMaxAcerc = 0;
int   vecesQueDispararia = 0, vecesQueDispararíaSinPredecir = 0;

bool pausado = false;


void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


// =====================================================================
//  LA CAMARA — el mismo parser del programa de juego
//  9 bytes: [201, Xp, Yp+100, 202, Xam, Yam+100, 203, Xaz, Yaz+100]
// =====================================================================

void leerCamara() {
  while (Serial1.available()) {
    byte b = Serial1.read();
    if (!sincronizado) {
      if (b == 201) { sincronizado = true; paquete[0] = b; cuantos = 1; }
      continue;
    }
    paquete[cuantos++] = b;
    if (cuantos < 9) continue;
    cuantos = 0; sincronizado = false;
    if (paquete[0] != 201 || paquete[3] != 202 || paquete[6] != 203) continue;

    Xp = paquete[1];
    Yp = paquete[2] - 100;
    t_ultimoPaquete = millis();
    paquetesTotales++;

    // ---- de unidades de camara a centimetros reales ----
    distanciaCm = Xp / CAMARA_POR_CM;
    desvioCm = (camaraYInvertida ? -(float)Yp : (float)Yp) / CAMARA_POR_CM;

    // ---- la velocidad, calculada con cada paquete nuevo ----
    //
    // Se hace ACA y no en el loop a proposito: en el loop se leeria el mismo
    // dato muchas veces y la velocidad daria un cero falso.
    if (Xp > 0) {
      unsigned long t = millis();
      if (hayDesvioAnterior) {
        float dt = (t - t_desvioAnterior) / 1000.0;
        // dt muy chico agranda cualquier error; dt muy grande quiere decir
        // que se perdio la pelota en el medio. Los dos se tiran.
        if (dt > 0.005 && dt < 0.5) {
          float v = (desvioCm - desvioAnterior) / dt;
          velocidadLateral = velocidadLateral * (1.0 - SUAVIZADO_VELOCIDAD)
                           + v * SUAVIZADO_VELOCIDAD;
          // La de acercamiento se saca al reves, porque la distancia BAJA
          // cuando la pelota se viene encima.
          float va = (distanciaAnterior - distanciaCm) / dt;
          velocidadAcercamiento = velocidadAcercamiento * (1.0 - SUAVIZADO_VELOCIDAD)
                                + va * SUAVIZADO_VELOCIDAD;
          if (fabs(velocidadLateral) > velMax) velMax = fabs(velocidadLateral);
          if (velocidadAcercamiento > velMaxAcerc) velMaxAcerc = velocidadAcercamiento;
        }
      }
      desvioAnterior    = desvioCm;
      distanciaAnterior = distanciaCm;
      t_desvioAnterior  = t;
      hayDesvioAnterior = true;
    } else {
      // Sin pelota a la vista la velocidad vieja no vale nada: cuando
      // reaparezca puede estar en cualquier lado.
      hayDesvioAnterior = false;
      velocidadLateral = 0;
      velocidadAcercamiento = 0;
    }

    // ---- la decision del despeje, con y sin prediccion ----
    float predicho = desvioCm + velocidadLateral * (msAnticipacion / 1000.0);
    bool conPred = (Xp > 0 && distanciaCm <= umbralCm && fabs(predicho) <= umbralDesvio);
    bool sinPred = (Xp > 0 && distanciaCm <= umbralCm && fabs(desvioCm) <= umbralDesvio);
    if (conPred) vecesSeguidas++; else vecesSeguidas = 0;
    if (conPred) vecesQueDispararia++;
    if (sinPred) vecesQueDispararíaSinPredecir++;
  }
}

float desvioPredicho() {
  return desvioCm + velocidadLateral * (msAnticipacion / 1000.0);
}


// =====================================================================
//  LA REGLA CON DOS MARCAS — es lo mas importante de este programa
// =====================================================================
//
// Dibuja el campo de vision de costado, con una 'o' donde esta la pelota y
// una 'P' donde se predice que va a estar. Si las dos coinciden se dibuja
// una '*'.
//
// 🎯 Rodando la pelota de IZQUIERDA a DERECHA, la P tiene que quedar a la
// DERECHA de la o. Si queda a la izquierda, el signo de la velocidad esta
// invertido y la prediccion esta apuntando detras de la pelota.

const int ANCHO_REGLA = 61;        // casilleros
const float REGLA_CM  = 30.0;      // +-30 cm de lado a lado

int casillero(float cm) {
  int medio = ANCHO_REGLA / 2;
  int n = medio + (int)(cm / REGLA_CM * medio);
  if (n < 0) n = 0;
  if (n >= ANCHO_REGLA) n = ANCHO_REGLA - 1;
  return n;
}

void dibujarRegla() {
  int cAhora = casillero(desvioCm);
  int cPred  = casillero(desvioPredicho());
  int medio  = ANCHO_REGLA / 2;
  // El pasillo de punteria: si la pelota (predicha) cae adentro, despeja.
  int cIzq = casillero(-umbralDesvio);
  int cDer = casillero(+umbralDesvio);

  Serial.print("izq ");
  for (int i = 0; i < ANCHO_REGLA; i++) {
    if (i == cAhora && i == cPred) Serial.write('*');
    else if (i == cAhora)          Serial.write('o');
    else if (i == cPred)           Serial.write('P');
    else if (i == medio)           Serial.write('|');
    else if (i == cIzq || i == cDer) Serial.write(':');
    else                           Serial.write('-');
  }
  Serial.println(" der");
}


// =====================================================================
//  PANTALLA
// =====================================================================

void irArriba()   { Serial.print("\033[H"); }
void limpiarTodo(){ Serial.print("\033[2J\033[H"); }
void finDeLinea() { Serial.print("\033[K\r\n"); }

void dibujar() {
  bool viva = (millis() - t_ultimoPaquete) < 500;
  irArriba();

  Serial.print("=== VER-PREDICCION ==================================");
  Serial.print("  "); Serial.print(millis() / 1000);
  Serial.print(" s"); finDeLinea();
  finDeLinea();

  if (!viva) {
    Serial.print("LA CAMARA NO ESTA HABLANDO. ¿Bateria prendida?");
    finDeLinea();
    for (int i = 0; i < 18; i++) finDeLinea();
    return;
  }

  Serial.print("camara: "); Serial.print(paquetesPorSegundo, 1);
  Serial.print(" paquetes/s"); finDeLinea();
  finDeLinea();

  if (Xp == 0) {
    Serial.print("PELOTA: no la ve  (o la tiene justo encima: Xp=0 es las dos)");
    finDeLinea();
  } else {
    Serial.print("PELOTA   de frente "); Serial.print(distanciaCm, 1);
    Serial.print(" cm      al costado "); Serial.print(desvioCm, 1);
    Serial.print(" cm"); finDeLinea();
  }

  Serial.print("VELOCIDAD de costado "); Serial.print(velocidadLateral, 1);
  Serial.print(" cm/s    acercandose "); Serial.print(velocidadAcercamiento, 1);
  Serial.print(" cm/s"); finDeLinea();

  // 🚨 El piso de ruido: medido el 2026-09-08 con la pelota QUIETA dio 3,6
  // cm/s. Abajo de eso, lo que se ve no es movimiento: es temblor de la
  // camara, y la prediccion estaria corrigiendo por nada.
  if (fabs(velocidadLateral) < 3.6) {
    Serial.print("   (debajo del piso de ruido de 3,6 cm/s: no es movimiento real)");
  } else {
    Serial.print("   maximos de hoy: costado ");  Serial.print(velMax, 1);
    Serial.print("   acercamiento ");             Serial.print(velMaxAcerc, 1);
  }
  finDeLinea();
  finDeLinea();

  Serial.print("anticipa "); Serial.print(msAnticipacion, 0);
  Serial.print(" ms  ->  predice el costado en ");
  Serial.print(desvioPredicho(), 1); Serial.print(" cm");
  float salto = desvioPredicho() - desvioCm;
  Serial.print("   (adelanta "); Serial.print(salto, 1); Serial.print(" cm)");
  finDeLinea();
  finDeLinea();

  dibujarRegla();
  Serial.print("    o = donde esta     P = donde se predice     ");
  Serial.print(": = el pasillo de punteria (+-");
  Serial.print(umbralDesvio, 0); Serial.print(" cm)");
  finDeLinea();
  finDeLinea();

  // La decision, con y sin prediccion: es la comparacion que importa.
  bool conPred = (Xp > 0 && distanciaCm <= umbralCm
                  && fabs(desvioPredicho()) <= umbralDesvio);
  bool sinPred = (Xp > 0 && distanciaCm <= umbralCm
                  && fabs(desvioCm) <= umbralDesvio);
  Serial.print("DESPEJARIA?   prediciendo: ");
  Serial.print(conPred ? "SI " : "no ");
  Serial.print("       sin predecir: ");
  Serial.print(sinPred ? "SI " : "no ");
  if (conPred != sinPred) Serial.print("   <-- LA PREDICCION CAMBIA LA DECISION");
  finDeLinea();
  Serial.print("   cuadros seguidos creyendo: "); Serial.print(vecesSeguidas);
  Serial.print(" de ");                           Serial.print(VECES_PARA_CREERLE);
  if (vecesSeguidas >= VECES_PARA_CREERLE) Serial.print("   >> SALDRIA A DESPEJAR");
  finDeLinea();
  Serial.print("   veces que disparo: prediciendo ");
  Serial.print(vecesQueDispararia);
  Serial.print(", sin predecir ");
  Serial.print(vecesQueDispararíaSinPredecir);
  finDeLinea();
  finDeLinea();

  Serial.print("🎯 LA PRUEBA DEL SIGNO: roda la pelota de IZQUIERDA a DERECHA.");
  finDeLinea();
  Serial.print("   La P tiene que quedar a la DERECHA de la o. Si queda a la");
  finDeLinea();
  Serial.print("   izquierda, el signo esta invertido y la prediccion apunta");
  finDeLinea();
  Serial.print("   DETRAS de la pelota.");
  finDeLinea();
  finDeLinea();
  Serial.print("teclas:  p pausar    0 borrar maximos    a/s anticipacion -+50");
  finDeLinea();
}

void leerTeclado() {
  while (Serial.available()) {
    char t = Serial.read();
    if (t == 'p') { pausado = !pausado; if (!pausado) limpiarTodo(); }
    else if (t == '0') {
      velMax = 0; velMaxAcerc = 0;
      vecesQueDispararia = 0; vecesQueDispararíaSinPredecir = 0;
    }
    else if (t == 'a') { msAnticipacion -= 50; if (msAnticipacion < 0) msAnticipacion = 0; }
    else if (t == 's') { msAnticipacion += 50; if (msAnticipacion > 1000) msAnticipacion = 1000; }
  }
}


void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS_CAMARA);
  delay(800);
  limpiarTodo();
  Serial.println("=== VER-PREDICCION — el robot NO se mueve ===");
  Serial.println("Haciendo rodar la pelota delante de la camara.");
  delay(700);
  limpiarTodo();
}


unsigned long t_dibujo = 0, t_cuenta = 0;
unsigned long paquetesEnLaVuelta = 0;

void loop() {
  leerTeclado();
  leerCamara();                    // hay que vaciar el buffer seguido

  unsigned long ahora = millis();

  if (ahora - t_cuenta >= 1000) {
    paquetesPorSegundo = (paquetesTotales - paquetesEnLaVuelta)
                       * 1000.0 / (ahora - t_cuenta);
    paquetesEnLaVuelta = paquetesTotales;
    t_cuenta = ahora;
  }

  // El LED prendido cuando ve la pelota: se ve aunque no haya cable.
  digitalWrite(LED, (Xp > 0 && (ahora - t_ultimoPaquete) < 500) ? HIGH : LOW);

  // 10 por segundo: rapido para seguir una pelota, lento para poder leer.
  if (!pausado && ahora - t_dibujo >= 100) {
    t_dibujo = ahora;
    dibujar();
  }
}
