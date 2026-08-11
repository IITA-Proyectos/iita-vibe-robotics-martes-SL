/* =====================================================================
   MOTORES A MANO — vos mandás, el robot obedece
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-07-28
   =====================================================================

   PARA QUE SIRVE
   Las pruebas cronometradas obligan a seguir una secuencia con el ojo y es
   facil confundirse de rueda o de paso. Acá no hay secuencia: vos escribis
   una tecla en el monitor serie y ESE motor arranca, y se queda andando
   hasta que mandes otra cosa. Podes mirarlo todo el tiempo que quieras,
   tocarlo, escucharlo, y repetir la misma prueba diez veces seguidas.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. ROBOT LEVANTADO, las tres ruedas al aire. Bateria puesta.
   2. Arduino IDE -> Herramientas -> Monitor Serie, velocidad 19200.
   3. Escribi una tecla en la barra de arriba del monitor y dale ENTER.

        1 = IZQUIERDA sentido A       2 = IZQUIERDA sentido B
        3 = DERECHA   sentido A       4 = DERECHA   sentido B
        5 = TRASERA   sentido A       6 = TRASERA   sentido B

        7 = las TRES juntas, sentido A   (para ver cual NO responde)
        0 = PARAR TODO

        d = subir la potencia (+20)      a = bajar la potencia (-20)
        ? = volver a mostrar esta ayuda

   Cada motor se apaga solo a los 10 segundos, por las dudas.

   ---------------------------------------------------------------------
   QUE ESTAMOS TRATANDO DE AVERIGUAR
   ---------------------------------------------------------------------
   Si la rueda DERECHA (pines 8/7/6) responde en algun sentido, o en
   ninguno. Proba el 3 y el 4 varias veces cada uno, mirando SOLO esa rueda.
   Despues proba el 1, 2, 5 y 6 para tener con que comparar.

   OJO: ya sabemos que en esta placa PWM=0 NO apaga el motor (lo manda la
   direccion). Por eso "parar" pone las dos patas de direccion en 0.
   ===================================================================== */


// ARQUERO (ROBOT1)
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = rueda IZQUIERDA

#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = rueda DERECHA

#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = rueda TRASERA

int vel = 100;
unsigned long t_arranque = 0;
bool andando = false;

const unsigned long LIMITE_MS = 10000;   // apagado automatico


void pararTodo() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
  andando = false;
}

void mover(int ina, int inb, int pwmPin, int a, int b) {
  pararTodo();
  digitalWrite(ina, a);
  digitalWrite(inb, b);
  analogWrite(pwmPin, vel);
  t_arranque = millis();
  andando = true;
}

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" MOTORES A MANO — robot levantado, ruedas al aire");
  Serial.println("=================================================");
  Serial.println("  1 = IZQUIERDA sent.A     2 = IZQUIERDA sent.B");
  Serial.println("  3 = DERECHA   sent.A     4 = DERECHA   sent.B");
  Serial.println("  5 = TRASERA   sent.A     6 = TRASERA   sent.B");
  Serial.println("  7 = las TRES juntas      0 = PARAR TODO");
  Serial.println("  d = potencia +20         a = potencia -20");
  Serial.print  ("  potencia actual: "); Serial.println(vel);
  Serial.println("  (se apaga solo a los 10 s)");
  Serial.println("=================================================");
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pararTodo();

  Serial.begin(19200);
  while (!Serial && millis() < 6000) { }
  ayuda();
  Serial.println("Listo. Escribi una tecla + ENTER.");
}


void loop() {

  // apagado automatico
  if (andando && (millis() - t_arranque > LIMITE_MS)) {
    pararTodo();
    Serial.println("   (10 s cumplidos — apagado automatico)");
  }

  if (Serial.available() == 0) return;

  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {

    case '1': Serial.println(">> IZQUIERDA sentido A  (pin2=ALTO, pin5=bajo)");
              mover(INA1, INB1, PWM1, 1, 0); break;
    case '2': Serial.println(">> IZQUIERDA sentido B  (pin2=bajo, pin5=ALTO)");
              mover(INA1, INB1, PWM1, 0, 1); break;

    case '3': Serial.println(">> DERECHA sentido A    (pin8=ALTO, pin7=bajo)");
              mover(INA2, INB2, PWM2, 1, 0); break;
    case '4': Serial.println(">> DERECHA sentido B    (pin8=bajo, pin7=ALTO)");
              mover(INA2, INB2, PWM2, 0, 1); break;

    case '5': Serial.println(">> TRASERA sentido A    (pin11=ALTO, pin12=bajo)");
              mover(INA3, INB3, PWM3, 1, 0); break;
    case '6': Serial.println(">> TRASERA sentido B    (pin11=bajo, pin12=ALTO)");
              mover(INA3, INB3, PWM3, 0, 1); break;

    case '7': Serial.println(">> LAS TRES juntas, sentido A — mira cual NO gira");
              pararTodo();
              digitalWrite(INA1, 1); digitalWrite(INB1, 0); analogWrite(PWM1, vel);
              digitalWrite(INA2, 1); digitalWrite(INB2, 0); analogWrite(PWM2, vel);
              digitalWrite(INA3, 1); digitalWrite(INB3, 0); analogWrite(PWM3, vel);
              t_arranque = millis(); andando = true;
              break;

    case '0': Serial.println(">> PARADO");
              pararTodo(); break;

    case 'd': vel += 20; if (vel > 255) vel = 255;
              Serial.print("   potencia = "); Serial.println(vel); break;
    case 'a': vel -= 20; if (vel < 0) vel = 0;
              Serial.print("   potencia = "); Serial.println(vel); break;

    case '?': ayuda(); break;

    default:  Serial.print("   tecla '"); Serial.print(c);
              Serial.println("' no hace nada. Escribi ? para la ayuda.");
              break;
  }
}
