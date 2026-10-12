/* =====================================================================
   PROBAR RUEDAS — cual es cual, para donde gira, y con cuanto arranca
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-10-12
   =====================================================================

   PARA QUE SIRVE
   Los tres motores se metieron mas adentro con soportes impresos en 3D,
   para que el robot entre en el diametro del reglamento. Para eso hubo que
   desarmar y volver a enchufar. Antes de apoyarlo en la cancha hay que
   contestar tres preguntas faciles:

     1. ¿Cada motor quedo en SU conector?            teclas 1, 2 y 3
     2. ¿El robot se mueve para donde se le pide?    teclas a, d y g
     3. ¿Con cuanta potencia arranca?                teclas + y -

   Si un conector quedo cruzado o dado vuelta, el programa de partido
   compila y se carga igual, pero el robot gira cuando deberia avanzar. Es
   mejor verlo aca, con las ruedas en el aire, que en la cancha.

   NO ARRANCA SOLO. No mueve nada hasta que apretes una tecla, y cada
   prueba se corta sola. Cualquier tecla mientras se mueve = PARAR.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   Primero CON LAS RUEDAS EN EL AIRE (apoyado sobre algo, que ninguna
   rueda toque la mesa). Bateria prendida, cable USB, y abrir
   pruebas\herramientas\mirar.bat.

   1. Tecla 1: tiene que girar SOLO la rueda IZQUIERDA (mirando el robot
      desde atras, con la camara apuntando para adelante).
      Tecla 2: solo la DERECHA.   Tecla 3: solo la TRASERA.
      Si gira otra, ese motor esta enchufado en el conector equivocado.

   2. Tecla a (avanzar): giran las dos de adelante, la trasera queda
      quieta. Apoya un dedo ARRIBA de cada rueda de adelante: las dos te
      lo tienen que empujar hacia ADELANTE (en diagonal, pero para el lado
      de la camara). Si una empuja para atras, ese motor quedo al reves.

   3. Tecla d (derecha): giran las tres. El dedo ARRIBA de la trasera se
      va hacia la DERECHA del robot.

   Recien si todo eso dio bien, EN EL PISO y con lugar alrededor:

   4. Teclas a, d y g. Son empujones de medio segundo. El robot tiene que
      AVANZAR, correrse a la DERECHA, y GIRAR EN EL LUGAR hacia la derecha
      (sentido horario visto de arriba). Si al girar se va corriendo de
      lugar, las tres ruedas no quedaron a la misma distancia del centro.

   5. El piso de arranque: bajar la potencia con - y repetir g hasta que el
      robot zumbe y no gire. El ultimo valor con el que SI giro es el piso.
      Con la base vieja era ~70; de ese numero salen PWM_MIN_ACOMODO = 75 y
      pwmMinLateral = 45 del programa de partido. Lo mismo con d.

   OJO: aca los motores arrancan DE GOLPE, sin la rampa del programa de
   partido. Sirve para ver SI arranca y PARA DONDE va. No sirve para medir
   centimetros.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        1 = rueda IZQUIERDA sola        a = avanzar
        2 = rueda DERECHA sola          d = de costado a la derecha
        3 = rueda TRASERA sola          g = girar en el lugar (horario)
        + = mas potencia                - = menos potencia
        ? = ayuda                       cualquier otra = PARAR

   ---------------------------------------------------------------------
   PINES Y MEZCLAS (los mismos de funciona/seguir-y-despejar)
   ---------------------------------------------------------------------
        U5  = 2/5/3   izquierda      avanzar   ( +p, -p,   0 )
        U17 = 8/7/6   derecha        derecha   ( fr, fr, -tr )   50/50/89
        U7  = 11/12/4 trasera        girar     (  g,  g,   g )
   ===================================================================== */

#include <Arduino.h>


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

// La proporcion para ir de costado, COPIADA del programa de partido. Si alla
// cambia, hay que cambiarla aca: si no, esta prueba valida otra cosa.
const int LADO_FRENTE  = 50;
const int LADO_TRASERA = 89;

// Arranca en 90 porque el piso de arranque medido con la base vieja era ~70.
int potencia = 90;
const int PASO_POTENCIA = 5;
const int POTENCIA_MIN  = 20;
const int POTENCIA_MAX  = 200;

// Una rueda sola dura mas porque es para mirarla. Los movimientos duran poco
// porque en el piso el robot se va y tironea el cable.
const unsigned long MS_UNA_RUEDA  = 1500;
const unsigned long MS_MOVIMIENTO = 500;

bool moviendo = false;
unsigned long t_inicio = 0;
unsigned long msPrueba = 0;


// ---------------------------------------------------------------- motores

// SOLTAR los motores: las dos patas de direccion en 0 (no alcanza con poner
// el PWM en 0).
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Aplica un valor CON SIGNO a una rueda. Positivo = pata A alta.
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

void cortar() {
  parar();
  digitalWrite(LED, 0);
  moviendo = false;
}

void arrancarReloj(unsigned long ms) {
  digitalWrite(LED, 1);
  moviendo = true;
  t_inicio = millis();
  msPrueba = ms;
}


// ---------------------------------------------------------------- pruebas

// Una rueda sola. Las otras dos quedan SUELTAS de verdad (parar() primero):
// si se mueve mas de una, no es por este programa.
void unaRueda(const char *cual, int ina, int inb, int pwm) {
  parar();
  rueda(ina, inb, pwm, potencia);
  Serial.print("  -> SOLO la rueda "); Serial.print(cual);
  Serial.print("   potencia "); Serial.println(potencia);
  arrancarReloj(MS_UNA_RUEDA);
}

void movimiento(const char *cual, int v1, int v2, int v3) {
  aplicar(v1, v2, v3);
  Serial.print("  -> "); Serial.print(cual);
  Serial.print("   potencia "); Serial.print(potencia);
  Serial.print("   ruedas (");
  Serial.print(v1); Serial.print(", ");
  Serial.print(v2); Serial.print(", ");
  Serial.print(v3); Serial.println(")");
  arrancarReloj(MS_MOVIMIENTO);
}

void cambiarPotencia(int paso) {
  potencia += paso;
  if (potencia > POTENCIA_MAX) potencia = POTENCIA_MAX;
  if (potencia < POTENCIA_MIN) potencia = POTENCIA_MIN;
  Serial.print("  potencia = "); Serial.println(potencia);
}

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" PROBAR RUEDAS — no muevo nada hasta que apretes");
  Serial.println("=================================================");
  Serial.println("  Primero con las RUEDAS EN EL AIRE.");
  Serial.println("  1 = IZQUIERDA sola    a = avanzar");
  Serial.println("  2 = DERECHA sola      d = costado a la derecha");
  Serial.println("  3 = TRASERA sola      g = girar (horario)");
  Serial.println("  + / - = potencia      otra tecla = PARAR");
  Serial.print  ("  potencia = "); Serial.println(potencia);
  Serial.println("=================================================");
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  parar();

  Serial.begin(BAUDIOS);
  while (!Serial && millis() < 4000) { }
  ayuda();
}


void loop() {

  if (moviendo && millis() - t_inicio >= msPrueba) {
    cortar();
    Serial.println("     (corto solo)");
  }

  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r') return;

  // Mientras se mueve, cualquier tecla es el freno de mano.
  if (moviendo) {
    cortar();
    Serial.println("  PARADO");
    return;
  }

  int fr = (potencia * LADO_FRENTE)  / 100;
  int tr = (potencia * LADO_TRASERA) / 100;

  switch (c) {
    case '1': unaRueda("IZQUIERDA (U5)", INA1, INB1, PWM1); break;
    case '2': unaRueda("DERECHA (U17)",  INA2, INB2, PWM2); break;
    case '3': unaRueda("TRASERA (U7)",   INA3, INB3, PWM3); break;
    case 'a': movimiento("AVANZAR",            +potencia, -potencia, 0);        break;
    case 'd': movimiento("COSTADO A DERECHA",  fr,        fr,        -tr);      break;
    case 'g': movimiento("GIRAR HORARIO",      potencia,  potencia,  potencia); break;
    case '+': cambiarPotencia(+PASO_POTENCIA); break;
    case '-': cambiarPotencia(-PASO_POTENCIA); break;
    case '?': ayuda(); break;
    default:
      cortar();
      Serial.println("  PARADO");
  }
}
