/* =====================================================================
   DESPEJE — ve la pelota naranja, la saca, y vuelve al area
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-11
   =====================================================================

   QUE HACE
   Se queda quieto mirando la cancha, con su arco a la espalda. Cuando la
   camara ve la pelota naranja cerca y de frente:

        1. sale disparado ~30 cm hacia adelante y la saca
        2. retrocede hasta que un sensor de atras pisa la LINEA BLANCA
        3. se endereza al rumbo que tenia antes de salir
        4. avanza 10 cm y queda listo para el proximo

   ---------------------------------------------------------------------
   POR QUE VOLVER POR LA LINEA Y NO POR TIEMPO
   ---------------------------------------------------------------------
   La version anterior retrocedia un tiempo fijo. Eso obliga a calibrar en
   centimetros, y el resultado se desajusta con la bateria, con el piso y
   con si la ida choco la pelota o no. Ya nos paso: con el mismo tiempo, la
   vuelta daba 53 cm donde la ida daba 30.

   La linea blanca del area es una marca FISICA. Siempre esta en el mismo
   lugar de la cancha. El robot no necesita saber cuanto se movio: llega,
   la ve, y frena. No hay nada que recalibrar cuando cambia la bateria.

   ---------------------------------------------------------------------
   SENSORES DE LINEA — MEDIDO en banco el 2026-08-11
   ---------------------------------------------------------------------
   Los tres andan. Cual es cual se midio apoyando el robot mitad sobre una
   mesa blanca y mitad sobre un pad negro, girandolo 180 grados para ver los
   numeros darse vuelta, y por ultimo dejando solo el izquierdo sobre el pad:

        A12  ->  ADELANTE
        A13  ->  ATRAS IZQUIERDA   } estos dos frenan el retroceso
        A11  ->  ATRAS DERECHA     }

        blanco ~= 765        oscuro ~= 120        (mas claro = mas alto)

   La primera prueba dio mal — parecia que dos sensores estaban rotos —
   porque la tapa blanca cubria solo uno. Lo que funciono fue apoyar el
   robot a caballo entre dos superficies de colores conocidos: nada que
   sostener con la mano, nada que pase rapido, y se lee tranquilo.

   ⚠️ Esos 765 y 120 son de una mesa blanca y un pad negro, NO de la
   cancha. El verde de la cancha va a dar un numero intermedio. **Antes de
   confiar en esto hay que medir en la cancha**: apoyarlo en el verde,
   apoyarlo sobre una linea, y poner el umbral justo en el medio. La tecla
   'L' muestra los tres sensores en vivo para eso.

   ---------------------------------------------------------------------
   🚨 LO QUE PASA SI NO ENCUENTRA LA LINEA
   ---------------------------------------------------------------------
   Frena a los 800 ms y avisa. NO sigue retrocediendo.

   Esto no es la decision de que hacer en ese caso — eso quedo pendiente
   con el profe. Es un freno de emergencia: un robot que retrocede sin
   limite se cae de la mesa o se estrella. El codigo 2025 tiene ese bug
   exacto en el arquero (`PATEANDO_atras_arquero` sale SOLO por linea, sin
   timeout: si nunca ve blanco, retrocede para siempre).

   ---------------------------------------------------------------------
   TECLAS (solo con la PC conectada)
   ---------------------------------------------------------------------
        g = ACTIVAR (avisa 10 s)      0 = PARAR
        i = que ve la camara ahora
        L = que ven los sensores de linea ahora
        c = probar solo el empujon de ida
        u / j = umbral de blanco  +25 / -25
        p / o = potencia del retroceso  +10 / -10
        w / s = empujon de ida    +20 / -20 ms
        e / d = empujon final     +10 / -10 ms
        x / z = distancia que dispara  +5 / -5 cm
        n / b = desvio tolerado        +5 / -5 cm
        k = acomodarse al volver, si/no
        ? = ayuda

   ---------------------------------------------------------------------
   ESTE SKETCH ES DEL ARQUERO (ROBOT1)
   ---------------------------------------------------------------------
   Ruedas segun lo MEDIDO en banco (los comentarios del codigo 2025 estan
   espejados):
        U5  = pines 2/5/3    -> IZQUIERDA
        U17 = pines 8/7/6    -> DERECHA
        U7  = pines 11/12/4  -> TRASERA (no empuja al ir recto)

   LA CAMARA: 9 bytes a 19200 por Serial1,
        [201][Xp][Yp+100] [202][Xam][Yam+100] [203][Xaz][Yaz+100]
   Xp = distancia de la pelota en cm. **Xp = 0 significa NO LA VEO.**
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


// ---- ARQUERO (ROBOT1) ----
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

// ---- sensores de linea (MEDIDO en banco el 2026-08-11) ----
#define LINEA_ATRAS_DER A11   // el "s1" de la libreria
#define LINEA_ATRAS_IZQ A13   // el "s2"
#define LINEA_ADELANTE  A12   // el "s3" — no frena, se lee solo para mirar

const unsigned long BAUDIOS = 19200;


// ---- lo que se ajusta con las teclas ----
int potenciaDespeje = 200;       // la ida: rapido, es un despeje

// El retroceso va MAS LENTO a proposito. El robot no tiene freno: cuando
// corta los motores sigue de largo. A la velocidad de la ida se pasaria
// varios centimetros de la linea antes de detenerse. Mas lento = frena
// mas cerca de donde la vio.
//
// 2026-08-11, probado en cancha: 140 resulto "muy rapido". Bajado a 110.
// Piso de arranque medido por la otra mesa: abajo de ~70 las ruedas zumban
// y no giran, asi que 110 todavia tiene margen. Si hiciera falta ir mas
// lento que eso, no se baja mas el PWM: se manda a pulsos cortos.
int potenciaRetroceso = 110;

int msAdelante = 333;            // 30 cm. Calibrado con regla el 2026-08-04:
                                 // distancia_cm = tiempo_ms/10 - 3.3

int msAdelanteChico = 133;       // los 10 cm del final, con la misma cuenta.
                                 // ⚠️ 33 de esos 133 ms son el retardo de
                                 // arranque: en un movimiento tan corto,
                                 // cualquier variacion pesa mucho. Esperar
                                 // menos precision que en el empujon largo.

// MEDIDO EN LA CANCHA el 2026-08-11 con pruebas/calibrar-linea:
//     verde:  atras-IZQ 356 [355-359]   atras-DER 465 [463-468]
//     blanco: ~760 cuando el sensor esta de verdad sobre la linea
// El umbral va en el medio del verde mas claro (468) y el blanco (760).
//
// Ojo: los dos sensores de atras NO son iguales — sobre el mismo verde uno
// marca 356 y el otro 465. Con 620 los dos quedan holgados, pero si alguno
// empieza a fallar, esa diferencia de altura o suciedad es el sospechoso.
int umbralBlanco = 620;

int umbralCm     = 30;           // dispara si la pelota esta a esto o menos
int umbralDesvio = 15;           // ...y si no esta demasiado al costado

const unsigned long MS_PAUSA_MEDIO      = 150;

// Freno de emergencia del retroceso. Subido de 800 a 1200 ms al bajar la
// potencia de 140 a 110: mas lento tarda mas en llegar a la linea, y el
// tope tiene que seguir dejando la misma distancia de margen.
const unsigned long MS_MAX_RETROCESO    = 1200;
const unsigned long MS_ENFRIAMIENTO     = 1500;
const unsigned long MS_AVISO_ARMADO     = 10000;
const int VECES_PARA_CREERLE = 3;

// true -> se arma solo al recibir energia. Hace falta para probarlo en el
// piso, donde el cable no llega.
// 🚨 Sin computadora, la unica forma de pararlo es la llave de la bateria.
const bool ARRANCA_SOLO = true;


// ---- acomodarse al volver (giroscopio) ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool acomodarActivado = true;

const float TOLERANCIA_ACOMODO = 3.0;
const float KP_ACOMODO      = 1.5;
const int   PWM_MIN_ACOMODO = 35;
const int   PWM_MAX_ACOMODO = 60;
const unsigned long MS_ASENTAR_ACOMODO = 300;
const unsigned long MS_TIMEOUT_ACOMODO = 3000;
const int MAX_REINTENTOS_ACOMODO = 3;

// Rampa de arranque del giro: arrancar las tres ruedas de golpe deja mudo
// al giroscopio (medido el 2026-08-04).
const int RAMPA_PASO = 3;
const unsigned long RAMPA_MS = 15;
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;

float rumboAntes = 0;
int reintentosAcomodo = 0;


// ---- estado ----
enum Fase { APAGADO, ARMANDOSE, VIGILANDO, ADELANTE, PAUSA_MEDIO,
            ATRAS_HASTA_LINEA, ACOMODANDO, ACOMODO_ASENTAR, ADELANTE_CHICO,
            ENFRIANDO, CAL_ADELANTE };
Fase fase = APAGADO;
unsigned long t_fase = 0;

int despejesHechos = 0;
int vecesSeguidas = 0;
bool ultimoRetrocesoEncontroLinea = false;

// Que paso con el enderezado del ultimo despeje. Sin cable no hay forma de
// enterarse en el momento, y "no se enderezo" puede ser por motivos muy
// distintos: que ni lo intentara (giroscopio mudo) o que lo intentara y no
// llegara. El robot se lo acuerda y lo cuenta con la tecla 'i'.
enum ResultadoAcomodo { ACOMODO_SIN_DATOS, ACOMODO_OK, ACOMODO_NO_LLEGO,
                        ACOMODO_SIN_GIROSCOPO, ACOMODO_APAGADO };
ResultadoAcomodo ultimoAcomodo = ACOMODO_SIN_DATOS;
float errorAcomodoFinal = 0;
float desvioAntesDeAcomodar = 0;
int reintentosUsados = 0;

const char* textoAcomodo(int r) {
  switch (r) {
    case ACOMODO_OK:            return "se enderezo bien";
    case ACOMODO_NO_LLEGO:      return "LO INTENTO Y NO LLEGO";
    case ACOMODO_SIN_GIROSCOPO: return "NI LO INTENTO — giroscopio mudo";
    case ACOMODO_APAGADO:       return "desactivado con la tecla k";
    default:                    return "todavia no hubo ninguno";
  }
}

// ---- camara ----
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;


// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Mismas direcciones que avanzar() de arquero.ino. La trasera va en 0: en
// un omni de tres ruedas no aporta al avance recto.
void adelante(int potencia) {
  analogWrite(PWM1, potencia); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, potencia); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void atras(int potencia) {
  analogWrite(PWM1, potencia); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, potencia); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


// ---------------------------------------------------------------- linea

// Solo los DOS DE ATRAS deciden el frenado: son los que cruzan la linea
// primero cuando el robot va marcha atras. El de adelante se lee nada mas
// que para mirarlo con la tecla 'L'.
bool algunSensorDeAtrasVeBlanco() {
  return (analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco)
      || (analogRead(LINEA_ATRAS_DER) >= umbralBlanco);
}

void mostrarLinea() {
  int iz = analogRead(LINEA_ATRAS_IZQ);
  int de = analogRead(LINEA_ATRAS_DER);
  int ad = analogRead(LINEA_ADELANTE);
  Serial.print("   atras-IZQ(A13)="); Serial.print(iz);
  Serial.print(iz >= umbralBlanco ? " BLANCO" : "       ");
  Serial.print("  atras-DER(A11)="); Serial.print(de);
  Serial.print(de >= umbralBlanco ? " BLANCO" : "       ");
  Serial.print("  adelante(A12)="); Serial.print(ad);
  Serial.println(ad >= umbralBlanco ? " BLANCO" : "");
  Serial.print("   umbral = "); Serial.println(umbralBlanco);
}


// ---------------------------------------------------------------- rumbo

// Devuelve el rumbo, o -1 si el sensor esta mudo. Un numero imposible en
// vez de 0 evita la confusion de siempre: 0.0 es a la vez un rumbo valido
// y el sintoma de que el chip no contesta.
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

bool girarHaciaRumbo() {
  float r = rumboActual();
  if (r < 0) { parar(); return true; }        // sin sensor, no insistir

  float error = diferencia(rumboAntes, r);
  if (fabs(error) <= TOLERANCIA_ACOMODO) { parar(); return true; }

  int pwm = (int)(fabs(error) * KP_ACOMODO);
  if (pwm > PWM_MAX_ACOMODO) pwm = PWM_MAX_ACOMODO;
  if (pwm < PWM_MIN_ACOMODO) pwm = PWM_MIN_ACOMODO;

  unsigned long ms = millis();
  if (pwm < pwmAcomodoAplicado) {
    pwmAcomodoAplicado = pwm;
  } else if (ms - t_rampa >= RAMPA_MS) {
    t_rampa = ms;
    pwmAcomodoAplicado += RAMPA_PASO;
    if (pwmAcomodoAplicado > pwm) pwmAcomodoAplicado = pwm;
  }

  bool sentido = (error > 0);
  digitalWrite(INA1, sentido ? 1 : 0); digitalWrite(INB1, sentido ? 0 : 1);
  digitalWrite(INA2, sentido ? 1 : 0); digitalWrite(INB2, sentido ? 0 : 1);
  digitalWrite(INA3, sentido ? 1 : 0); digitalWrite(INB3, sentido ? 0 : 1);
  analogWrite(PWM1, pwmAcomodoAplicado);
  analogWrite(PWM2, pwmAcomodoAplicado);
  analogWrite(PWM3, pwmAcomodoAplicado);
  return false;
}


// ---------------------------------------------------------------- camara

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
    if (paquete[0] == 201 && paquete[3] == 202 && paquete[6] == 203) {
      Xp = paquete[1];
      Yp = paquete[2] - 100;
      t_ultimoPaquete = millis();

      // Cuantas veces SEGUIDAS la vimos donde nos interesa. Un solo cuadro
      // no alcanza: cualquier reflejo naranja lanzaria al robot.
      if (Xp > 0 && Xp <= umbralCm && abs(Yp) <= umbralDesvio) {
        if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
      } else {
        vecesSeguidas = 0;
      }
    }
  }
}

bool hayPelotaParaDespejar() { return vecesSeguidas >= VECES_PARA_CREERLE; }


// ---------------------------------------------------------------- consola

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" DESPEJE — saca la pelota y vuelve por la linea");
  Serial.println("=================================================");
  Serial.println("  g = ACTIVAR (avisa 10 s)     0 = PARAR");
  Serial.println("  i = que ve la camara     L = que ven los sensores");
  Serial.println("  c = probar solo el empujon de ida");
  Serial.println("  u/j = umbral blanco +-25   k = acomodarse si/no");
  Serial.println("  p/o = potencia del retroceso +-10");
  Serial.println("  w/s = ida +-20ms           e/d = empujon final +-10ms");
  Serial.println("  x/z = dispara a +-5cm      n/b = desvio +-5cm");
  Serial.println("-------------------------------------------------");
  Serial.print("  ida=");            Serial.print(msAdelante);
  Serial.print("ms (pot ");          Serial.print(potenciaDespeje);
  Serial.print(")   final=");        Serial.print(msAdelanteChico);
  Serial.println("ms");
  Serial.print("  retroceso: hasta linea, pot "); Serial.print(potenciaRetroceso);
  Serial.print(", umbral ");         Serial.print(umbralBlanco);
  Serial.print(", tope ");           Serial.print(MS_MAX_RETROCESO);
  Serial.println("ms");
  Serial.print("  dispara si esta a <= "); Serial.print(umbralCm);
  Serial.print(" cm y desviada <= ");      Serial.print(umbralDesvio);
  Serial.println(" cm");
  Serial.println("=================================================");
}

void desactivar(const char* motivo) {
  parar(); fase = APAGADO; vecesSeguidas = 0; digitalWrite(LED, LOW);
  Serial.print(">> APAGADO — "); Serial.println(motivo);
}

void leerConsola() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'g':
      fase = ARMANDOSE; t_fase = millis(); vecesSeguidas = 0;
      Serial.println(">> ARMANDOSE — listo en 10 segundos. Sacá las manos.");
      break;

    case '0': desactivar("orden manual"); break;

    case 'c':
      parar(); fase = CAL_ADELANTE; t_fase = millis();
      Serial.print(">> empujon de ida de "); Serial.print(msAdelante);
      Serial.println(" ms. Medí con la regla.");
      break;

    case 'L': mostrarLinea(); break;

    case 'u': umbralBlanco += 25; Serial.print("   umbral = ");
              Serial.println(umbralBlanco); break;
    case 'j': if (umbralBlanco > 25) umbralBlanco -= 25;
              Serial.print("   umbral = "); Serial.println(umbralBlanco); break;

    case 'w': msAdelante += 20; Serial.print("   ida = ");
              Serial.println(msAdelante); break;
    case 's': if (msAdelante > 20) msAdelante -= 20;
              Serial.print("   ida = "); Serial.println(msAdelante); break;

    case 'e': msAdelanteChico += 10; Serial.print("   empujon final = ");
              Serial.println(msAdelanteChico); break;
    case 'd': if (msAdelanteChico > 10) msAdelanteChico -= 10;
              Serial.print("   empujon final = ");
              Serial.println(msAdelanteChico); break;

    case 'x': umbralCm += 5; Serial.print("   dispara a <= ");
              Serial.print(umbralCm); Serial.println(" cm"); break;
    case 'z': if (umbralCm > 5) umbralCm -= 5;
              Serial.print("   dispara a <= "); Serial.print(umbralCm);
              Serial.println(" cm"); break;

    case 'n': umbralDesvio += 5; Serial.print("   desvio <= ");
              Serial.print(umbralDesvio); Serial.println(" cm"); break;
    case 'b': if (umbralDesvio > 5) umbralDesvio -= 5;
              Serial.print("   desvio <= "); Serial.print(umbralDesvio);
              Serial.println(" cm"); break;

    case 'p': potenciaRetroceso += 10; if (potenciaRetroceso > 255) potenciaRetroceso = 255;
              Serial.print("   potencia retroceso = ");
              Serial.println(potenciaRetroceso); break;
    case 'o': if (potenciaRetroceso > 10) potenciaRetroceso -= 10;
              Serial.print("   potencia retroceso = ");
              Serial.print(potenciaRetroceso);
              if (potenciaRetroceso < 80) Serial.print("  <-- ojo, cerca del piso de arranque");
              Serial.println(); break;

    case 'k': acomodarActivado = !acomodarActivado;
              Serial.print("   acomodarse al volver: ");
              Serial.println(acomodarActivado ? "SI" : "NO"); break;

    case 'i':
      if (millis() - t_ultimoPaquete > 1000) {
        Serial.println("   la camara no esta mandando nada");
      } else if (Xp == 0) {
        Serial.println("   la camara anda, pero no ve la pelota");
      } else {
        Serial.print("   pelota a "); Serial.print(Xp);
        Serial.print(" cm, desviada "); Serial.print(Yp);
        Serial.print(" cm  -> ");
        Serial.println(hayPelotaParaDespejar() ? "DENTRO del umbral"
                                               : "fuera del umbral");
      }
      Serial.print("   despejes hechos: "); Serial.println(despejesHechos);
      Serial.print("   ultimo retroceso: ");
      Serial.println(ultimoRetrocesoEncontroLinea ? "encontro la linea"
                                                  : "NO encontro la linea");
      Serial.print("   ultimo enderezado: ");
      Serial.println(textoAcomodo(ultimoAcomodo));
      if (ultimoAcomodo == ACOMODO_OK || ultimoAcomodo == ACOMODO_NO_LLEGO) {
        Serial.print("      venia torcido "); Serial.print(desvioAntesDeAcomodar, 1);
        Serial.print(" grados, quedo en ");   Serial.print(errorAcomodoFinal, 1);
        Serial.print(" (");                   Serial.print(reintentosUsados);
        Serial.println(" reintentos)");
      }
      Serial.print("   rumbo ahora: ");
      { float r = rumboActual();
        if (r < 0) Serial.println("GIROSCOPIO MUDO");
        else       Serial.println(r, 1); }
      break;

    case '?': ayuda(); break;

    default: Serial.print("   tecla '"); Serial.print(c);
             Serial.println("' no hace nada. ? para la ayuda."); break;
  }
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);
  pinMode(LINEA_ADELANTE, INPUT);
  parar();

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS);

  // Si el giroscopio no esta, el despeje igual funciona: lo unico que se
  // pierde es acomodarse al volver. No vale la pena bloquear la funcion
  // principal por una mejora.
  hayGiroscopo = bno.begin();
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    Serial.println(">> giroscopio OK — se va a acomodar al volver");
  } else {
    Serial.println("!! sin giroscopio — despeja igual, pero no se acomoda");
  }

  ayuda();
  mostrarLinea();

  if (ARRANCA_SOLO) {
    fase = ARMANDOSE; t_fase = millis();
    Serial.println(">> ARMANDOSE — 10 segundos y queda vigilando.");
    Serial.println("   Apoyalo, sacá las manos. Para pararlo: la bateria.");
  } else {
    fase = APAGADO;
    Serial.println("APAGADO. Mandá 'g' para activarlo.");
  }
}


void loop() {

  leerConsola();
  leerCamara();
  unsigned long ahora = millis();

  switch (fase) {

    case APAGADO:
      digitalWrite(LED, LOW);
      break;

    case ARMANDOSE: {
      unsigned long falta = (ahora - t_fase >= MS_AVISO_ARMADO)
                            ? 0 : MS_AVISO_ARMADO - (ahora - t_fase);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_AVISO_ARMADO) {
        fase = VIGILANDO; t_fase = ahora; vecesSeguidas = 0;
        Serial.println(">> ARMADO — vigilando. Mostrale la pelota.");
      }
      break;
    }

    case VIGILANDO:
      parar();
      digitalWrite(LED, ((ahora / 800) % 2) ? HIGH : LOW);
      if (hayPelotaParaDespejar()) {
        rumboAntes = rumboActual();     // el rumbo a recuperar despues
        Serial.print(">> PELOTA a "); Serial.print(Xp);
        Serial.print(" cm, desviada "); Serial.print(Yp);
        Serial.print(" cm — DESPEJANDO");
        if (rumboAntes >= 0) { Serial.print(" (rumbo ");
                               Serial.print(rumboAntes, 1); Serial.print(")"); }
        Serial.println();
        fase = ADELANTE; t_fase = ahora;
        digitalWrite(LED, HIGH);
      }
      break;

    case ADELANTE:
      adelante(potenciaDespeje);
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        parar();
        fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;

    case PAUSA_MEDIO:
      // Sin freno, el robot sigue de largo. Esta pausa lo deja detenerse
      // antes de mandarlo para atras.
      parar();
      if (ahora - t_fase >= MS_PAUSA_MEDIO) {
        fase = ATRAS_HASTA_LINEA; t_fase = ahora;
        ultimoRetrocesoEncontroLinea = false;
        Serial.println("   volviendo hasta la linea...");
      }
      break;

    case ATRAS_HASTA_LINEA:
      atras(potenciaRetroceso);

      if (algunSensorDeAtrasVeBlanco()) {
        parar();
        ultimoRetrocesoEncontroLinea = true;
        Serial.print("   linea encontrada a los ");
        Serial.print(ahora - t_fase); Serial.println(" ms");
        fase = ACOMODANDO; t_fase = ahora;
        reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = ahora;
        desvioAntesDeAcomodar = 0;
        break;
      }

      // Freno de emergencia. NO es la decision de que hacer si no hay
      // linea — eso quedo pendiente. Es para que el robot no se vaya de
      // la cancha ni de la mesa.
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        parar();
        ultimoRetrocesoEncontroLinea = false;
        Serial.println("!! NO ENCONTRE LA LINEA en 800 ms — freno igual");
        Serial.println("   revisar el umbral con 'L', o si quedo lejos del area");
        fase = ACOMODANDO; t_fase = ahora;
        reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = ahora;
        desvioAntesDeAcomodar = 0;
      }
      break;

    case ACOMODANDO:
      // Antes se salteaba EN SILENCIO cuando no habia giroscopio, y desde
      // afuera eso se ve igual que "se enderezo mal". Ahora deja dicho por
      // que no lo hizo.
      if (!acomodarActivado) {
        ultimoAcomodo = ACOMODO_APAGADO;
        Serial.println("   enderezado desactivado (tecla k)");
        fase = ADELANTE_CHICO; t_fase = ahora;
        break;
      }
      if (rumboAntes < 0 || rumboActual() < 0) {
        ultimoAcomodo = ACOMODO_SIN_GIROSCOPO;
        Serial.println("!! NO ME PUEDO ENDEREZAR — el giroscopio esta mudo");
        Serial.println("   (lo primero que se revisa es la bateria)");
        fase = ADELANTE_CHICO; t_fase = ahora;
        break;
      }
      if (reintentosAcomodo == 0 && desvioAntesDeAcomodar == 0) {
        desvioAntesDeAcomodar = diferencia(rumboAntes, rumboActual());
      }
      if (girarHaciaRumbo()) {
        fase = ACOMODO_ASENTAR; t_fase = ahora;
      } else if (ahora - t_fase >= MS_TIMEOUT_ACOMODO) {
        parar();
        Serial.println("   no llego a enderezarse en 3 s — sigo igual");
        fase = ACOMODO_ASENTAR; t_fase = ahora;
      }
      break;

    case ACOMODO_ASENTAR:
      // Frenar y esperar que termine la inercia ANTES de volver a medir.
      // Sin esta pausa el robot cree que llego cuando todavia esta girando.
      parar();
      if (ahora - t_fase >= MS_ASENTAR_ACOMODO) {
        float error = diferencia(rumboAntes, rumboActual());
        if (fabs(error) > TOLERANCIA_ACOMODO
            && reintentosAcomodo < MAX_REINTENTOS_ACOMODO) {
          reintentosAcomodo++;
          pwmAcomodoAplicado = 0; t_rampa = ahora;
          fase = ACOMODANDO; t_fase = ahora;
        } else {
          errorAcomodoFinal = error;
          reintentosUsados = reintentosAcomodo;
          ultimoAcomodo = (fabs(error) <= TOLERANCIA_ACOMODO)
                          ? ACOMODO_OK : ACOMODO_NO_LLEGO;
          Serial.print("   quedo a "); Serial.print(error, 1);
          Serial.print(" grados del rumbo original (venia torcido ");
          Serial.print(desvioAntesDeAcomodar, 1);
          Serial.print(", "); Serial.print(reintentosAcomodo);
          Serial.println(" reintentos)");
          fase = ADELANTE_CHICO; t_fase = ahora;
        }
      }
      break;

    case ADELANTE_CHICO:
      // El empujoncito final: se hace DESPUES de enderezarse, para que vaya
      // en la direccion correcta y no repita el desvio.
      adelante(potenciaDespeje);
      if (ahora - t_fase >= (unsigned long)msAdelanteChico) {
        parar();
        despejesHechos++;
        fase = ENFRIANDO; t_fase = ahora;
        Serial.print(">> despeje n. "); Serial.print(despejesHechos);
        Serial.println(" terminado");
      }
      break;

    case ENFRIANDO:
      // Si la pelota sigue ahi adelante, sin esta pausa encadenaria
      // despejes sin parar.
      parar();
      vecesSeguidas = 0;
      if (ahora - t_fase >= MS_ENFRIAMIENTO) {
        fase = VIGILANDO; t_fase = ahora;
      }
      break;

    case CAL_ADELANTE:
      adelante(potenciaDespeje);
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        parar(); fase = APAGADO;
        Serial.println("   listo. Medí. 'w' alarga, 's' acorta, 'c' repite.");
      }
      break;
  }
}
