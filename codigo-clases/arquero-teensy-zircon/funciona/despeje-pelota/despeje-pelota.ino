/* =====================================================================
   DESPEJE — ve la pelota naranja, la saca de un empujon y se vuelve
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-04
   =====================================================================

   QUE HACE
   Se queda quieto mirando. Cuando la camara ve la pelota naranja cerca y
   de frente: pega un empujon rapido de ~30 cm hacia adelante para sacarla,
   y vuelve ~30 cm hacia atras, a la posicion de antes (al arco).

   ---------------------------------------------------------------------
   🚨 LOS 30 cm SON POR TIEMPO, NO POR MEDICION
   ---------------------------------------------------------------------
   Este robot NO PUEDE MEDIR CUANTO SE MOVIO. No tiene encoders en las
   ruedas ni sensor de distancia. Lo unico que sabe es cuanto rato tuvo los
   motores prendidos.

   O sea: "30 cm" en realidad es "el tiempo que, medido con una regla, dio
   30 cm". Y ese tiempo cambia con:
       - la bateria (mas cargada = mas rapido = mas distancia)
       - el piso (alfombra vs. ceramica)
       - el peso que tenga encima

   ---------------------------------------------------------------------
   DE DONDE SALE EL NUMERO (medido 2026-08-04, potencia 200)
   ---------------------------------------------------------------------
   Con `pruebas/calibrar-15cm/` se midieron tres saltos con regla:

        150 ms -> 13 cm        250 ms -> 19 cm        350 ms -> 33 cm

   Ajustando una recta a esos tres puntos:

        distancia_cm = tiempo_ms / 10  -  3.3

   Es decir: avanza 1 cm cada 10 ms, PERO pierde ~33 ms al arrancar (el
   robot tarda en ponerse en movimiento; ese retardo pesa mas que el desliz
   del final). Por eso la regla de tres directa da mal — el mismo error que
   nos comimos con los giros por cronometro.

        Para 30 cm  ->  (30 + 3.3) * 10  =  333 ms

   ⚠️ Los datos tienen ruido: entre el 1er y 2do salto avanzo 6 cm por cada
   100 ms, y entre el 2do y 3ro avanzo 14. Tomar 333 ms como punto de
   partida, no como verdad revelada. Verificar con 'c' y ajustar.

   👉 Modo calibracion con el cable: tecla 'c'. Sin cable: el sketch
      `pruebas/calibrar-15cm/`.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        g = ACTIVAR (5 segundos de aviso antes de quedar armado)
        0 = PARAR y desactivar

        c = CALIBRAR: da UN empujon para adelante y nada mas. Medí con la
            regla cuanto avanzo y ajustá con las teclas de abajo.
        v = CALIBRAR al reves: un empujon para atras.

        w / s = tiempo de ida     +20 / -20 ms
        r / f = tiempo de vuelta  +20 / -20 ms
        d / a = potencia          +20 / -20

        x / z = distancia a la que dispara  +5 / -5 cm
        n / b = cuanto desvio tolera        +5 / -5 cm

        i = decir que esta viendo la camara ahora mismo
        ? = ayuda

   ---------------------------------------------------------------------
   ESTE SKETCH ES DEL ARQUERO (ROBOT1)
   ---------------------------------------------------------------------
   Pines y direcciones sacados de `arquero.ino`. Ruedas segun lo MEDIDO en
   banco el 2026-07-28 (los comentarios del codigo 2025 estan espejados):
        U5  = pines 2/5/3    -> IZQUIERDA
        U17 = pines 8/7/6    -> DERECHA
        U7  = pines 11/12/4  -> TRASERA (no empuja al ir recto)

   LA CAMARA (de vision-openmv/README.md): 9 bytes a 19200 por Serial1,
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

const unsigned long BAUDIOS = 19200;


// ---- lo que se ajusta con las teclas ----
int potencia   = 200;            // rapido: es un despeje, no un paseo
int msAdelante = 333;            // = 30 cm, de la calibracion de arriba

// MEDIDO 2026-08-04: con 333 ms para atras se paso 23 cm de la largada, o
// sea que retrocedio ~53 cm mientras que para adelante hacia ~30. Con el
// MISMO tiempo y la MISMA potencia.
//
// Por que va mas rapido de reculada: para adelante le pega a la pelota y la
// empuja — eso le come velocidad. La vuelta es libre, no empuja nada.
//
// Con el mismo retardo de arranque (~33 ms):
//     velocidad atras = 53 cm / (333-33) ms = 0.177 cm/ms
//     para 30 cm  ->  33 + 30/0.177  =  ~200 ms
int msAtras    = 200;
int umbralCm     = 30;           // dispara si la pelota esta a esto o menos
int umbralDesvio = 15;           // ...y si no esta demasiado al costado

const unsigned long MS_PAUSA_MEDIO   = 150;   // entre la ida y la vuelta
const unsigned long MS_ENFRIAMIENTO  = 1500;  // no encadenar despejes
const unsigned long MS_AVISO_ARMADO  = 10000;
const int VECES_PARA_CREERLE = 3;   // frames seguidos viendo la pelota

// true  -> al recibir energia se arma solo despues de la cuenta regresiva.
//          Hace falta para probarlo en el piso, donde el cable no llega.
// false -> arranca apagado y hay que mandarle 'g'. Mas seguro en la mesa.
//
// 🚨 Con esto en true y sin computadora, LA UNICA FORMA DE PARARLO ES LA
//    LLAVE DE LA BATERIA. Y este sketch pega saltos de 30 cm hacia adelante.
const bool ARRANCA_SOLO = true;


// ---- ACOMODARSE AL VOLVER (giroscopio) ----
// Un salto a potencia 200 tuerce el robot: las ruedas no empujan identico.
// Antes de salir se anota hacia donde miraba, y al volver se gira hasta
// recuperar ese rumbo. Sin esto, cada despeje lo deja un poco mas ladeado
// y despues de varios ya no esta mirando a la cancha.
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool acomodarActivado = true;        // tecla 'k'

const float TOLERANCIA_ACOMODO = 3.0;   // grados
const float KP_ACOMODO      = 1.5;      // PWM por grado
const int   PWM_MIN_ACOMODO = 35;       // abajo de esto no vence el roce
const int   PWM_MAX_ACOMODO = 60;
const unsigned long MS_ASENTAR_ACOMODO = 300;
const unsigned long MS_TIMEOUT_ACOMODO = 3000;
const int MAX_REINTENTOS_ACOMODO = 3;

// Rampa de arranque del giro. El 2026-08-04 comprobamos que arrancar las
// tres ruedas de golpe deja mudo al giroscopio; subiendo de a poco paso de
// fallar al 5to giro a aguantar 10 cuadrados.
const int RAMPA_PASO = 3;
const unsigned long RAMPA_MS = 15;
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;

float rumboAntes = 0;
int reintentosAcomodo = 0;


// ---- estado ----
enum Fase { APAGADO, ARMANDOSE, VIGILANDO, ADELANTE, PAUSA_MEDIO, ATRAS,
            ACOMODANDO, ACOMODO_ASENTAR, ENFRIANDO, CAL_ADELANTE, CAL_ATRAS };
Fase fase = APAGADO;
unsigned long t_fase = 0;

int  vecesSeguidas = 0;
int  despejesHechos = 0;

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

// Mismas direcciones que avanzar() de arquero.ino. La trasera va en 0:
// en un omni de tres ruedas no aporta al avance recto.
void adelante() {
  analogWrite(PWM1, potencia); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, potencia); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Lo mismo con las dos direcciones dadas vuelta.
void atras() {
  analogWrite(PWM1, potencia); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, potencia); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


// ---------------------------------------------------------------- rumbo

// Devuelve el rumbo, o -1 si el sensor esta mudo. Devolver un numero
// imposible en vez de 0 evita la confusion de siempre: 0.0 es a la vez un
// rumbo valido y el sintoma de que el chip no contesta.
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

// La diferencia mas corta entre dos rumbos, en (-180, 180]. Sin esto, ir de
// 350 a 10 grados se leeria como un giro de -340.
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// Gira sobre el eje hacia `rumboAntes`. Devuelve true cuando llego.
bool girarHaciaRumbo() {
  float ahora_rumbo = rumboActual();
  if (ahora_rumbo < 0) { parar(); return true; }   // sin sensor, no insistir

  float error = diferencia(rumboAntes, ahora_rumbo);
  if (fabs(error) <= TOLERANCIA_ACOMODO) { parar(); return true; }

  int pwm = (int)(fabs(error) * KP_ACOMODO);
  if (pwm > PWM_MAX_ACOMODO) pwm = PWM_MAX_ACOMODO;
  if (pwm < PWM_MIN_ACOMODO) pwm = PWM_MIN_ACOMODO;

  // arranque suave
  unsigned long ms = millis();
  if (pwm < pwmAcomodoAplicado) {
    pwmAcomodoAplicado = pwm;
  } else if (ms - t_rampa >= RAMPA_MS) {
    t_rampa = ms;
    pwmAcomodoAplicado += RAMPA_PASO;
    if (pwmAcomodoAplicado > pwm) pwmAcomodoAplicado = pwm;
  }

  bool sentido = (error > 0);
  // Las tres al mismo sentido = rotacion pura, como girar() de arquero.ino
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

      // Contar cuantas veces SEGUIDAS la vimos donde nos interesa. Un solo
      // frame no alcanza: cualquier reflejo naranja lanzaria al robot.
      if (Xp > 0 && Xp <= umbralCm && abs(Yp) <= umbralDesvio) {
        if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
      } else {
        vecesSeguidas = 0;
      }
    }
  }
}

bool hayPelotaParaDespejar() {
  return vecesSeguidas >= VECES_PARA_CREERLE;
}


// ---------------------------------------------------------------- consola

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" DESPEJE — ve la pelota naranja y la saca");
  Serial.println("=================================================");
  Serial.println("  g = ACTIVAR (avisa 5 s)     0 = PARAR");
  Serial.println("  c = calibrar IDA (un empujon adelante, medí con regla)");
  Serial.println("  v = calibrar VUELTA (un empujon atras)");
  Serial.println("  w/s = tiempo ida +-20    r/f = tiempo vuelta +-20");
  Serial.println("  d/a = potencia +-20");
  Serial.println("  x/z = distancia que dispara +-5 cm");
  Serial.println("  n/b = desvio tolerado +-5 cm");
  Serial.println("  i = que ve la camara ahora");
  Serial.println("-------------------------------------------------");
  Serial.print("  potencia=");   Serial.print(potencia);
  Serial.print("  ida=");        Serial.print(msAdelante);
  Serial.print("ms  vuelta=");   Serial.print(msAtras);
  Serial.println("ms");
  Serial.print("  dispara si esta a <= "); Serial.print(umbralCm);
  Serial.print(" cm y desviada <= ");      Serial.print(umbralDesvio);
  Serial.println(" cm");
  Serial.println("=================================================");
}

void desactivar(const char* motivo) {
  parar();
  fase = APAGADO;
  vecesSeguidas = 0;
  digitalWrite(LED, LOW);
  Serial.print(">> APAGADO — "); Serial.println(motivo);
}

void leerConsola() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'g':
      fase = ARMANDOSE; t_fase = millis(); vecesSeguidas = 0;
      Serial.println(">> ARMANDOSE — quedo listo en 5 segundos. Sacá las manos.");
      break;

    case '0': desactivar("orden manual"); break;

    case 'c':
      parar();
      fase = CAL_ADELANTE; t_fase = millis();
      Serial.print(">> CALIBRACION IDA: un empujon de ");
      Serial.print(msAdelante); Serial.println(" ms. Medí con la regla.");
      break;

    case 'v':
      parar();
      fase = CAL_ATRAS; t_fase = millis();
      Serial.print(">> CALIBRACION VUELTA: un empujon de ");
      Serial.print(msAtras); Serial.println(" ms para atras.");
      break;

    case 'w': msAdelante += 20; Serial.print("   ida = ");
              Serial.println(msAdelante); break;
    case 's': if (msAdelante > 20) msAdelante -= 20;
              Serial.print("   ida = "); Serial.println(msAdelante); break;

    case 'r': msAtras += 20; Serial.print("   vuelta = ");
              Serial.println(msAtras); break;
    case 'f': if (msAtras > 20) msAtras -= 20;
              Serial.print("   vuelta = "); Serial.println(msAtras); break;

    case 'd': potencia += 20; if (potencia > 255) potencia = 255;
              Serial.print("   potencia = "); Serial.println(potencia); break;
    case 'a': potencia -= 20; if (potencia < 0) potencia = 0;
              Serial.print("   potencia = "); Serial.println(potencia); break;

    case 'x': umbralCm += 5; Serial.print("   dispara a <= ");
              Serial.print(umbralCm); Serial.println(" cm"); break;
    case 'z': if (umbralCm > 5) umbralCm -= 5;
              Serial.print("   dispara a <= "); Serial.print(umbralCm);
              Serial.println(" cm"); break;

    case 'n': umbralDesvio += 5; Serial.print("   desvio tolerado <= ");
              Serial.print(umbralDesvio); Serial.println(" cm"); break;
    case 'b': if (umbralDesvio > 5) umbralDesvio -= 5;
              Serial.print("   desvio tolerado <= "); Serial.print(umbralDesvio);
              Serial.println(" cm"); break;

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
      break;

    case 'k':
      acomodarActivado = !acomodarActivado;
      Serial.print("   acomodarse al volver: ");
      Serial.println(acomodarActivado ? "SI" : "NO");
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
  parar();

  Serial.begin(BAUDIOS);          // USB, para nosotros
  Serial1.begin(BAUDIOS);         // camara, pines 0 y 1

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
      unsigned long periodo = (falta <= 2000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_AVISO_ARMADO) {
        fase = VIGILANDO; t_fase = ahora; vecesSeguidas = 0;
        Serial.println(">> ARMADO — vigilando. Mostrale la pelota.");
      }
      break;
    }

    case VIGILANDO:
      parar();
      // LED con latido suave: armado pero sin actuar
      digitalWrite(LED, ((ahora / 800) % 2) ? HIGH : LOW);
      if (hayPelotaParaDespejar()) {
        // Anotar el rumbo ANTES de salir: es el que vamos a recuperar.
        rumboAntes = rumboActual();
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
      adelante();
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        parar();
        fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;

    case PAUSA_MEDIO:
      // Sin freno, el robot sigue de largo. Esta pausa deja que se detenga
      // antes de mandarlo para atras; si no, la vuelta arranca peleando
      // contra la inercia de la ida y queda mas corta.
      parar();
      if (ahora - t_fase >= MS_PAUSA_MEDIO) {
        fase = ATRAS; t_fase = ahora;
      }
      break;

    case ATRAS:
      atras();
      if (ahora - t_fase >= (unsigned long)msAtras) {
        parar();
        if (acomodarActivado && rumboAntes >= 0 && rumboActual() >= 0) {
          reintentosAcomodo = 0;
          pwmAcomodoAplicado = 0; t_rampa = ahora;
          fase = ACOMODANDO; t_fase = ahora;
          float desvio = diferencia(rumboAntes, rumboActual());
          Serial.print("   volvio torcido "); Serial.print(desvio, 1);
          Serial.println(" grados — acomodandose");
        } else {
          despejesHechos++;
          fase = ENFRIANDO; t_fase = ahora;
          Serial.print(">> despeje n. "); Serial.print(despejesHechos);
          Serial.println(" terminado (sin acomodar)");
        }
      }
      break;

    case ACOMODANDO:
      if (girarHaciaRumbo()) {
        fase = ACOMODO_ASENTAR; t_fase = ahora;
      } else if (ahora - t_fase >= MS_TIMEOUT_ACOMODO) {
        parar();
        Serial.println("   no llego a acomodarse en 3 s — sigo igual");
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
          despejesHechos++;
          fase = ENFRIANDO; t_fase = ahora;
          Serial.print(">> despeje n. "); Serial.print(despejesHechos);
          Serial.print(" terminado — quedo a "); Serial.print(error, 1);
          Serial.println(" grados del rumbo original");
        }
      }
      break;

    case ENFRIANDO:
      // Si la pelota sigue ahi adelante, sin esta pausa el robot encadenaria
      // despejes sin parar.
      parar();
      vecesSeguidas = 0;
      if (ahora - t_fase >= MS_ENFRIAMIENTO) {
        fase = VIGILANDO; t_fase = ahora;
      }
      break;

    case CAL_ADELANTE:
      adelante();
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        parar();
        fase = APAGADO;
        Serial.println("   listo. Medí. 'w' alarga, 's' acorta, 'c' repite.");
      }
      break;

    case CAL_ATRAS:
      atras();
      if (ahora - t_fase >= (unsigned long)msAtras) {
        parar();
        fase = APAGADO;
        Serial.println("   listo. Medí. 'r' alarga, 'f' acorta, 'v' repite.");
      }
      break;
  }
}
