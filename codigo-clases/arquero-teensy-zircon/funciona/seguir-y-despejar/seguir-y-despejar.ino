/* =====================================================================
   ARQUERO — sigue la pelota de costado y la despeja
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-11
   =====================================================================

   QUE HACE
   El robot se para en su area mirando la cancha, con el arco a la espalda,
   y NUNCA gira: siempre mira al frente.

        1. Sin pelota a la vista: quieto.
        2. Ve la pelota lejos  -> se corre DE COSTADO para ponerse enfrente
                                  de ella, sin dejar de mirar al frente.
        3. La pelota se acerca -> DESPEJA: sale 30 cm, la saca, vuelve hasta
                                  pisar la linea, se endereza, avanza 10 cm.

   El movimiento de costado se frena solo al pisar la linea blanca del area:
   yendo a la izquierda mira el sensor de atras-izquierda, yendo a la
   derecha el de atras-derecha.

   ---------------------------------------------------------------------
   POR QUE EL GIROSCOPIO NO PUEDE MEDIR CUANTO SE MOVIO DE COSTADO
   ---------------------------------------------------------------------
   Vale la pena que quede escrito porque es una confusion facil.

   El giroscopio mide HACIA DONDE MIRA el robot, no DONDE ESTA. Si el robot
   se corre 20 cm al costado sin girar, el giroscopio marca exactamente lo
   mismo antes y despues. Y este robot no tiene encoders en las ruedas ni
   sensor de distancia, asi que no hay ninguna forma de medir cuanto se
   desplazo.

   Por eso el limite del movimiento lateral es la LINEA BLANCA — una marca
   fisica de la cancha — y no una cuenta del giroscopio.

   Lo que SI hace el giroscopio, y es lo que pidio el profe: mantenerlo
   DERECHO. Al moverse de costado, un omni de tres ruedas tiende a girar
   un poco (la rueda trasera empuja distinto que las dos de adelante). El
   giroscopio detecta ese giro y lo corrige mientras se mueve.

   ---------------------------------------------------------------------
   COMO SE MEZCLAN MOVERSE Y ENDEREZARSE
   ---------------------------------------------------------------------
   El codigo 2025 resolvia esto con tablas de valores fijos: tres ramas de
   `if` con numeros escritos a mano (50/50/89, 65/40/100...) segun cuanto
   estaba torcido. Funciona, pero no se puede ajustar sin reescribir la
   tabla.

   Aca se hace sumando dos cosas separadas:

        lo que cada rueda tiene que hacer para IR DE COSTADO
      + lo que cada rueda tiene que hacer para NO GIRAR
      = lo que se le manda a esa rueda

   Moverse de costado son las tres ruedas en la proporcion 50/50/89 (esa
   proporcion sale de la geometria del robot y es la del codigo 2025).
   No girar son las tres ruedas parejas, en la misma direccion.

   Como son dos cosas independientes, se calculan por separado y se suman.
   Asi se puede tocar una sin romper la otra.

   ---------------------------------------------------------------------
   🚨 DOS SIGNOS QUE HAY QUE DESCUBRIR PROBANDO
   ---------------------------------------------------------------------
   No se pueden deducir leyendo el codigo, hay que medirlos:

     1. Cuando el programa dice "andá a la derecha", ¿el robot va a la
        derecha? Los comentarios del codigo 2025 estan espejados, asi que
        no sirven de referencia.   -> tecla 'v' hace una prueba corta
                                    -> tecla 'V' invierte el sentido

     2. Cuando la camara dice que la pelota esta desviada +5, ¿esta a la
        derecha o a la izquierda?  -> tecla 'i' muestra el numero
                                    -> tecla 'Y' invierte

   Si alguno esta al reves, el robot se aleja de la pelota en vez de
   seguirla. Se nota en dos segundos.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        g = ACTIVAR (avisa 10 s)      0 = PARAR
        i = que ve la camara          L = que ven los sensores de linea
        v = prueba corta de movimiento lateral
        V = invertir el sentido lateral
        Y = invertir el signo de la camara
        k = enderezarse si/no
        f/F = fuerza del seguimiento lateral  -/+
        u/j = umbral de blanco  -/+
        x/z = distancia a la que despeja  +/-
        ? = ayuda

   ---------------------------------------------------------------------
   MEDICIONES QUE USA (todas hechas en banco o en cancha)
   ---------------------------------------------------------------------
   Ruedas    U5=2/5/3 izquierda · U17=8/7/6 derecha · U7=11/12/4 trasera
   Linea     A12 adelante · A13 atras-IZQ · A11 atras-DER
             verde ~356 y ~465 · blanco ~760 · umbral 620
   Distancia 1 cm cada 10 ms a potencia 200, con ~33 ms de arranque
   Camara    9 bytes a 19200 por Serial1. Xp=0 significa NO LA VEO
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

#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13
#define LINEA_ADELANTE  A12

const unsigned long BAUDIOS = 19200;


// ---- los dos signos, YA DESCUBIERTOS (2026-08-11, en banco) ----
//
// Lateral: se mando la prueba de la tecla 'v' y el robot fue efectivamente
// a la derecha. Queda como esta.
bool lateralInvertido = false;
//
// Camara: con la pelota puesta a la DERECHA del robot, la camara mandaba
// Yp = -24. O sea que el negativo es la derecha, al reves de lo que
// suponia el codigo. Por eso va invertido.
bool camaraYInvertida = true;

// ---- seguimiento lateral ----
// La proporcion 50/50/89 entre las ruedas sale de la geometria del robot;
// es la que usa el codigo 2025 en ai/adproporcional. Lo que cambia es la
// FUERZA con que se aplica, que aca es proporcional a lo desviada que esta
// la pelota en vez de ser un numero fijo.
const int LADO_FRENTE  = 50;     // las dos de adelante
const int LADO_TRASERA = 89;     // la de atras

float kpLateral   = 4.0;         // PWM por cm de desvio de la pelota
int   pwmMinLateral = 60;        // abajo de esto no se mueve, solo zumba
int   pwmMaxLateral = 120;
const float ZONA_MUERTA_PELOTA = 4.0;   // cm: no perseguir migajas

// ---- despeje ----
int potenciaDespeje   = 200;
int potenciaRetroceso = 110;
// La ida tiene que ALCANZAR a la pelota: si el robot dispara a 48 cm y solo
// avanza 30, frena 18 cm antes y no la toca nunca. Por eso este numero sube
// junto con umbralCm.
// Calibrado 2026-08-04 con regla: distancia_cm = tiempo_ms/10 - 3.3
//     50 cm -> (50 + 3.3) * 10 = 533 ms
int msAdelante        = 533;     // ~50 cm
int msAdelanteChico   = 133;     // 10 cm
int umbralBlanco      = 620;     // medido en cancha el 2026-08-11
// Historia de este numero, probando en cancha el 2026-08-11:
//   30 -> 20 -> 48. Se subio para que salga a buscarla mas lejos.
//
// 🚨 REGLA: umbralCm NUNCA puede ser mayor que el alcance de la ida. Si el
// robot dispara a 48 cm y la ida son 30, frena antes de llegar y no toca
// la pelota. Si se cambia uno, hay que mirar el otro.
int umbralCm          = 48;      // despeja si la pelota esta a esto o menos
int umbralDesvio      = 15;

const unsigned long MS_PAUSA_MEDIO   = 150;
const unsigned long MS_MAX_RETROCESO = 1200;
const unsigned long MS_ENFRIAMIENTO  = 1500;
const unsigned long MS_AVISO_ARMADO  = 10000;
const int VECES_PARA_CREERLE = 3;
const unsigned long MS_PRUEBA_LATERAL = 400;

// 🚨 Con esto en true y sin computadora, la unica forma de pararlo es la
// llave de la bateria.
const bool ARRANCA_SOLO = true;


// ---- giroscopio: mantenerlo derecho ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;
bool enderezarActivado = true;

float rumboBase = -1;            // el rumbo que hay que sostener siempre

// Correccion continua de rumbo (proporcional + acumulada). Los valores
// vienen de lo medido el 2026-08-04 en las rectas del cuadrado: con solo
// proporcional quedaba un error fijo de 7 grados; con el termino acumulado
// bajo a menos de 2.
const float KP_RUMBO = 5.0;
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 70;
const float ZONA_MUERTA_RUMBO = 1.0;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Enderezado al terminar el despeje
const float TOLERANCIA_ACOMODO = 3.0;
const float KP_ACOMODO = 1.5;
const int   PWM_MIN_ACOMODO = 35;
const int   PWM_MAX_ACOMODO = 60;
const unsigned long MS_ASENTAR_ACOMODO = 300;
const unsigned long MS_TIMEOUT_ACOMODO = 3000;
const int MAX_REINTENTOS_ACOMODO = 3;
const int RAMPA_PASO = 3;
const unsigned long RAMPA_MS = 15;
int pwmAcomodoAplicado = 0;
unsigned long t_rampa = 0;
int reintentosAcomodo = 0;


// ---- estado ----
enum Fase { APAGADO, ARMANDOSE, ESPERANDO, SIGUIENDO,
            ADELANTE, PAUSA_MEDIO, ATRAS_HASTA_LINEA,
            ACOMODANDO, ACOMODO_ASENTAR, ADELANTE_CHICO, ENFRIANDO,
            PRUEBA_LATERAL };
Fase fase = APAGADO;
unsigned long t_fase = 0;

int despejesHechos = 0;
int vecesSeguidas = 0;
bool ultimoRetrocesoEncontroLinea = false;
bool frenadoPorLineaIzq = false;
bool frenadoPorLineaDer = false;

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

// Aplica un valor CON SIGNO a una rueda. Positivo = pata A alta.
// Tener valores con signo es lo que permite sumar movimientos: no se puede
// sumar "50 hacia alla" con "20 hacia el otro lado" si no hay signos.
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

// Avanzar recto: mismas direcciones que avanzar() de arquero.ino, escritas
// con signos. Las dos de adelante van opuestas entre si porque estan
// montadas espejadas; la trasera no aporta al avance recto.
void adelante(int potencia) { aplicar(+potencia, -potencia, 0); }
void atras(int potencia)    { aplicar(-potencia, +potencia, 0); }


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

void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

// Cuanto hay que sumarle a las TRES ruedas para que el robot no gire.
// Devuelve 0 si no hay giroscopio: sin rumbo no se corrige nada, pero el
// movimiento lateral tiene que seguir funcionando igual.
int correccionDeRumbo() {
  if (!enderezarActivado || rumboBase < 0) return 0;
  float r = rumboActual();
  if (r < 0) return 0;

  float error = diferencia(rumboBase, r);

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;

  // Anti-windup: el acumulador tiene tope. Sin esto, un rato torcido lo
  // infla tanto que despues el robot se pasa para el otro lado.
  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  return (int)c;
}


// ---------------------------------------------------------------- linea

bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= umbralBlanco; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= umbralBlanco; }
bool algunoDeAtrasVeBlanco() { return veBlancoIzq() || veBlancoDer(); }

void mostrarLinea() {
  int iz = analogRead(LINEA_ATRAS_IZQ);
  int de = analogRead(LINEA_ATRAS_DER);
  int ad = analogRead(LINEA_ADELANTE);
  Serial.print("   atras-IZQ(A13)="); Serial.print(iz);
  Serial.print(iz >= umbralBlanco ? " BLANCO" : "       ");
  Serial.print("  atras-DER(A11)="); Serial.print(de);
  Serial.print(de >= umbralBlanco ? " BLANCO" : "       ");
  Serial.print("  adelante(A12)="); Serial.println(ad);
  Serial.print("   umbral = "); Serial.println(umbralBlanco);
}


// ------------------------------------------------------- moverse de costado

// s > 0 pide ir a la DERECHA, s < 0 a la izquierda. El valor es la fuerza.
// Suma el movimiento lateral con la correccion de rumbo: son dos cosas
// independientes y por eso se pueden calcular por separado.
void moverDeCostado(int s) {
  if (lateralInvertido) s = -s;

  int frente  = (s * LADO_FRENTE)  / 100;
  int trasera = (s * LADO_TRASERA) / 100;

  // Direcciones sacadas de adproporcional() de arquero.ino: las dos de
  // adelante para el mismo lado, la trasera al reves y mas fuerte.
  int v1 = +frente;
  int v2 = +frente;
  int v3 = -trasera;

  int c = correccionDeRumbo();
  aplicar(v1 + c, v2 + c, v3 + c);
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

      // Un solo cuadro no alcanza para lanzar al robot: cualquier reflejo
      // naranja lo dispararia.
      if (Xp > 0 && Xp <= umbralCm && abs(Yp) <= umbralDesvio) {
        if (vecesSeguidas < VECES_PARA_CREERLE) vecesSeguidas++;
      } else {
        vecesSeguidas = 0;
      }
    }
  }
}

bool veLaPelota()  { return Xp > 0 && (millis() - t_ultimoPaquete < 500); }
bool hayQueDespejar() { return vecesSeguidas >= VECES_PARA_CREERLE; }

// Desvio de la pelota en cm, con el signo ya corregido: positivo = esta a
// la derecha del robot.
float desvioPelota() {
  return camaraYInvertida ? -(float)Yp : (float)Yp;
}


// ---------------------------------------------------------------- consola

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" ARQUERO — sigue la pelota de costado y despeja");
  Serial.println("=================================================");
  Serial.println("  g = ACTIVAR (avisa 10 s)     0 = PARAR");
  Serial.println("  i = camara      L = sensores de linea");
  Serial.println("  v = prueba lateral corta    V = invertir lateral");
  Serial.println("  Y = invertir signo camara   k = enderezarse si/no");
  Serial.println("  f/F = fuerza del seguimiento -/+");
  Serial.println("  u/j = umbral blanco +/-     x/z = despeja a +/- cm");
  Serial.println("-------------------------------------------------");
  Serial.print("  kpLateral="); Serial.print(kpLateral, 1);
  Serial.print("  pwm "); Serial.print(pwmMinLateral);
  Serial.print("-");      Serial.print(pwmMaxLateral);
  Serial.print("  zona muerta "); Serial.print(ZONA_MUERTA_PELOTA, 0);
  Serial.println(" cm");
  Serial.print("  lateralInvertido="); Serial.print(lateralInvertido);
  Serial.print("  camaraYInvertida="); Serial.println(camaraYInvertida);
  Serial.print("  despeja a <= "); Serial.print(umbralCm);
  Serial.print(" cm   umbral blanco "); Serial.println(umbralBlanco);
  Serial.println("=================================================");
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

    case '0':
      parar(); fase = APAGADO; digitalWrite(LED, LOW);
      Serial.println(">> APAGADO");
      break;

    case 'v':
      parar();
      rumboBase = rumboActual();
      reiniciarCorreccion();
      fase = PRUEBA_LATERAL; t_fase = millis();
      Serial.println(">> prueba: me muevo a lo que YO llamo DERECHA");
      Serial.println("   si va para la izquierda, apretá 'V'");
      break;

    case 'V':
      lateralInvertido = !lateralInvertido;
      Serial.print("   lateral invertido = "); Serial.println(lateralInvertido);
      break;

    case 'Y':
      camaraYInvertida = !camaraYInvertida;
      Serial.print("   signo de camara invertido = ");
      Serial.println(camaraYInvertida);
      break;

    case 'k':
      enderezarActivado = !enderezarActivado;
      Serial.print("   enderezarse: ");
      Serial.println(enderezarActivado ? "SI" : "NO");
      break;

    case 'F': kpLateral += 0.5; Serial.print("   kpLateral = ");
              Serial.println(kpLateral, 1); break;
    case 'f': if (kpLateral > 0.5) kpLateral -= 0.5;
              Serial.print("   kpLateral = "); Serial.println(kpLateral, 1); break;

    case 'u': umbralBlanco += 25; Serial.print("   umbral = ");
              Serial.println(umbralBlanco); break;
    case 'j': if (umbralBlanco > 25) umbralBlanco -= 25;
              Serial.print("   umbral = "); Serial.println(umbralBlanco); break;

    case 'x': umbralCm += 5; Serial.print("   despeja a <= ");
              Serial.print(umbralCm); Serial.println(" cm"); break;
    case 'z': if (umbralCm > 5) umbralCm -= 5;
              Serial.print("   despeja a <= "); Serial.print(umbralCm);
              Serial.println(" cm"); break;

    case 'L': mostrarLinea(); break;

    case 'i':
      if (millis() - t_ultimoPaquete > 1000) {
        Serial.println("   la camara no manda nada");
      } else if (Xp == 0) {
        Serial.println("   la camara anda, pero no ve la pelota");
      } else {
        Serial.print("   pelota a "); Serial.print(Xp);
        Serial.print(" cm, Yp crudo "); Serial.print(Yp);
        Serial.print("  -> la leo como ");
        Serial.print(desvioPelota() > 0 ? "DERECHA" : "IZQUIERDA");
        Serial.print(" ("); Serial.print(fabs(desvioPelota()), 0);
        Serial.println(" cm)");
      }
      Serial.print("   despejes: "); Serial.println(despejesHechos);
      Serial.print("   rumbo: ");
      { float r = rumboActual();
        if (r < 0) Serial.println("GIROSCOPIO MUDO");
        else { Serial.print(r, 1); Serial.print("  base ");
               Serial.println(rumboBase, 1); } }
      if (frenadoPorLineaIzq) Serial.println("   ojo: frene contra la linea IZQUIERDA");
      if (frenadoPorLineaDer) Serial.println("   ojo: frene contra la linea DERECHA");
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
  pinMode(LINEA_ADELANTE,  INPUT);
  parar();

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS);

  // Sin giroscopio el robot igual sigue y despeja: lo unico que pierde es
  // mantenerse derecho. No vale la pena bloquear todo por una mejora.
  hayGiroscopo = bno.begin();
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    Serial.println(">> giroscopio OK");
  } else {
    Serial.println("!! sin giroscopio — se mueve igual pero no se endereza");
  }

  ayuda();
  mostrarLinea();

  if (ARRANCA_SOLO) {
    fase = ARMANDOSE; t_fase = millis();
    Serial.println(">> ARMANDOSE — 10 segundos. Para pararlo: la bateria.");
  } else {
    fase = APAGADO;
    Serial.println("APAGADO. Mandá 'g'.");
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
        // El rumbo de referencia se fija ACA, con el robot ya quieto y
        // apuntando a la cancha. Es el que va a sostener siempre.
        rumboBase = rumboActual();
        reiniciarCorreccion();
        fase = ESPERANDO; t_fase = ahora; vecesSeguidas = 0;
        Serial.print(">> ARMADO — mirando. Rumbo base ");
        if (rumboBase < 0) Serial.println("SIN GIROSCOPIO");
        else Serial.println(rumboBase, 1);
      }
      break;
    }

    case PRUEBA_LATERAL:
      moverDeCostado(pwmMaxLateral);
      if (ahora - t_fase >= MS_PRUEBA_LATERAL) {
        parar(); fase = APAGADO;
        Serial.println("   listo. Fue a la derecha? Si no, apretá 'V'.");
      }
      break;

    case ESPERANDO:
      parar();
      digitalWrite(LED, ((ahora / 800) % 2) ? HIGH : LOW);
      frenadoPorLineaIzq = false; frenadoPorLineaDer = false;
      if (hayQueDespejar()) {
        Serial.print(">> pelota a "); Serial.print(Xp);
        Serial.println(" cm — DESPEJANDO");
        fase = ADELANTE; t_fase = ahora;
        digitalWrite(LED, HIGH);
      } else if (veLaPelota()) {
        fase = SIGUIENDO; t_fase = ahora;
        reiniciarCorreccion();
      }
      break;

    case SIGUIENDO: {
      digitalWrite(LED, HIGH);

      if (hayQueDespejar()) {
        parar();
        Serial.print(">> pelota a "); Serial.print(Xp);
        Serial.println(" cm — DESPEJANDO");
        fase = ADELANTE; t_fase = ahora;
        break;
      }
      if (!veLaPelota()) {
        parar();
        fase = ESPERANDO; t_fase = ahora;
        break;
      }

      float desvio = desvioPelota();

      // Zona muerta: si la pelota esta casi enfrente, quedarse quieto. Sin
      // esto el robot tiembla persiguiendo el ruido de la camara.
      if (fabs(desvio) < ZONA_MUERTA_PELOTA) {
        parar();
        break;
      }

      int fuerza = (int)(fabs(desvio) * kpLateral);
      if (fuerza > pwmMaxLateral) fuerza = pwmMaxLateral;
      if (fuerza < pwmMinLateral) fuerza = pwmMinLateral;

      bool hayQueIrDerecha = (desvio > 0);

      // El limite es la LINEA, no una cuenta: yendo a la izquierda mira el
      // sensor de atras-izquierda, y al reves. Mirar solo el del lado hacia
      // donde va evita que el otro lo frene por nada.
      if (hayQueIrDerecha && veBlancoDer()) {
        parar(); frenadoPorLineaDer = true;
        break;
      }
      if (!hayQueIrDerecha && veBlancoIzq()) {
        parar(); frenadoPorLineaIzq = true;
        break;
      }
      frenadoPorLineaIzq = false; frenadoPorLineaDer = false;

      moverDeCostado(hayQueIrDerecha ? fuerza : -fuerza);
      break;
    }

    case ADELANTE:
      adelante(potenciaDespeje);
      if (ahora - t_fase >= (unsigned long)msAdelante) {
        parar(); fase = PAUSA_MEDIO; t_fase = ahora;
      }
      break;

    case PAUSA_MEDIO:
      parar();
      if (ahora - t_fase >= MS_PAUSA_MEDIO) {
        fase = ATRAS_HASTA_LINEA; t_fase = ahora;
        ultimoRetrocesoEncontroLinea = false;
        Serial.println("   volviendo hasta la linea...");
      }
      break;

    case ATRAS_HASTA_LINEA:
      atras(potenciaRetroceso);
      if (algunoDeAtrasVeBlanco()) {
        parar();
        ultimoRetrocesoEncontroLinea = true;
        Serial.print("   linea a los "); Serial.print(ahora - t_fase);
        Serial.println(" ms");
        fase = ACOMODANDO; t_fase = ahora;
        reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = ahora;
        break;
      }
      // Freno de emergencia: un robot que retrocede sin limite se va de la
      // cancha. El codigo 2025 tiene ese bug exacto en el arquero.
      if (ahora - t_fase >= MS_MAX_RETROCESO) {
        parar();
        Serial.println("!! no encontre la linea — freno igual");
        fase = ACOMODANDO; t_fase = ahora;
        reintentosAcomodo = 0; pwmAcomodoAplicado = 0; t_rampa = ahora;
      }
      break;

    case ACOMODANDO: {
      if (!enderezarActivado) { fase = ADELANTE_CHICO; t_fase = ahora; break; }
      float r = rumboActual();
      if (rumboBase < 0 || r < 0) {
        Serial.println("!! no me puedo enderezar — giroscopio mudo");
        fase = ADELANTE_CHICO; t_fase = ahora;
        break;
      }
      float error = diferencia(rumboBase, r);
      if (fabs(error) <= TOLERANCIA_ACOMODO) {
        parar(); fase = ACOMODO_ASENTAR; t_fase = ahora;
        break;
      }
      if (ahora - t_fase >= MS_TIMEOUT_ACOMODO) {
        parar();
        Serial.println("   no llego a enderezarse en 3 s");
        fase = ACOMODO_ASENTAR; t_fase = ahora;
        break;
      }
      int pwm = (int)(fabs(error) * KP_ACOMODO);
      if (pwm > PWM_MAX_ACOMODO) pwm = PWM_MAX_ACOMODO;
      if (pwm < PWM_MIN_ACOMODO) pwm = PWM_MIN_ACOMODO;
      // arranque suave: las tres de golpe dejan mudo al giroscopio
      if (pwm < pwmAcomodoAplicado) pwmAcomodoAplicado = pwm;
      else if (ahora - t_rampa >= RAMPA_MS) {
        t_rampa = ahora;
        pwmAcomodoAplicado += RAMPA_PASO;
        if (pwmAcomodoAplicado > pwm) pwmAcomodoAplicado = pwm;
      }
      int g = (error > 0) ? pwmAcomodoAplicado : -pwmAcomodoAplicado;
      aplicar(g, g, g);          // las tres parejas = rotacion pura
      break;
    }

    case ACOMODO_ASENTAR: {
      // Frenar y esperar que pase la inercia ANTES de volver a medir. Sin
      // esta pausa el robot cree que llego cuando todavia esta girando.
      parar();
      if (ahora - t_fase < MS_ASENTAR_ACOMODO) break;
      float error = diferencia(rumboBase, rumboActual());
      if (fabs(error) > TOLERANCIA_ACOMODO
          && reintentosAcomodo < MAX_REINTENTOS_ACOMODO) {
        reintentosAcomodo++;
        pwmAcomodoAplicado = 0; t_rampa = ahora;
        fase = ACOMODANDO; t_fase = ahora;
      } else {
        Serial.print("   derecho, a "); Serial.print(error, 1);
        Serial.println(" grados");
        fase = ADELANTE_CHICO; t_fase = ahora;
      }
      break;
    }

    case ADELANTE_CHICO:
      // Va DESPUES de enderezarse, para que los 10 cm salgan derechos.
      adelante(potenciaDespeje);
      if (ahora - t_fase >= (unsigned long)msAdelanteChico) {
        parar();
        despejesHechos++;
        fase = ENFRIANDO; t_fase = ahora;
        Serial.print(">> despeje n. "); Serial.println(despejesHechos);
      }
      break;

    case ENFRIANDO:
      parar();
      vecesSeguidas = 0;
      if (ahora - t_fase >= MS_ENFRIAMIENTO) {
        reiniciarCorreccion();
        fase = ESPERANDO; t_fase = ahora;
      }
      break;
  }
}
