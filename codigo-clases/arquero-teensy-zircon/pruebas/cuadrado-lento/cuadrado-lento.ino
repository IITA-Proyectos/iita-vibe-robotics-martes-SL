/* =====================================================================
   CUADRADO LENTO — prueba CON CARGA, en el piso, SIN COMPUTADORA
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-07-28
   =====================================================================

   PARA QUE SIRVE
   En `motores-a-mano` probamos los tres motores con las ruedas AL AIRE y
   los tres andaban. Eso demuestra que el camino Teensy -> driver -> motor
   esta sano, pero NO dice nada de que pasa con el peso del robot encima.
   Un motor que gira impecable en el aire puede patinar, rozar o frenarse
   apoyado. Este sketch es para ver eso, en el piso y sin cable.

   Repite el cuadrado indefinidamente: cuatro lados y cuatro giros, pausa,
   y de nuevo. Asi se puede mirar tranquilo si el error se repite igual
   todas las vueltas (mecanico) o cambia cada vez (giro mal calibrado).

   ---------------------------------------------------------------------
   🚨 COMO SE PARA — LEER ESTO PRIMERO
   ---------------------------------------------------------------------
   SIN LA COMPUTADORA CONECTADA, LA UNICA FORMA DE PARARLO ES LA LLAVE DE
   LA BATERIA. No hay boton de freno. No se cansa. No se detiene solo.

   Por eso arranca con 10 SEGUNDOS de espera, y avisa con el LED de la
   placa (el chiquito del Teensy):

        parpadeo LENTO  -> quedan mas de 3 segundos, todavia lo podes tocar
        parpadeo RAPIDO -> quedan menos de 3 segundos, SOLTALO YA
        LED FIJO        -> esta en movimiento

   PROTOCOLO: apoyar el robot en el piso -> prender la bateria -> sacar
   las manos. Nunca al reves. Nunca sobre una mesa.

   ---------------------------------------------------------------------
   SI ESTA CONECTADO A LA PC (opcional)
   ---------------------------------------------------------------------
   Con el cable puesto se puede ajustar sin recompilar:

        0 = PARAR TODO          g = volver a arrancar
        d / a = velocidad avance  +10 / -10
        t / r = tiempo lado       +200 / -200 ms
        y / u = tiempo giro       +50 / -50 ms
        ? = ayuda

   Los valores vuelven a los de abajo cada vez que se corta la energia:
   cuando encuentres los que sirven, ANOTALOS y cambialos aca.

   ---------------------------------------------------------------------
   QUE ESTAMOS TRATANDO DE AVERIGUAR
   ---------------------------------------------------------------------
   1. Con carga, arrancan las tres ruedas o alguna se queda?
   2. Los lados salen rectos, o el robot se va siempre para el mismo lado?
      SIEMPRE PARA EL MISMO LADO, todas las vueltas -> rueda floja con
      carga, es mecanico. Para cualquier lado -> es el giro que no cierra.
   3. Cuanto tarda de verdad un giro de 90 grados? El valor de abajo es
      una estimacion a ojo; depende del piso y de cuanta bateria queda.
      No busques el numero "correcto": buscá el que cierre el cuadrado HOY,
      en ESTE piso, y anotalo en la bitacora.

   ---------------------------------------------------------------------
   ESTE SKETCH ES DEL ARQUERO (ROBOT1)
   ---------------------------------------------------------------------
   Los pines y las direcciones salen de `arquero.ino`. En el delantero los
   mismos pines mueven OTRAS ruedas — no lo cargues ahi sin revisarlo.

   Nombres de rueda segun lo MEDIDO en banco el 2026-07-28, no segun los
   comentarios del codigo 2025, que estan espejados:
        U5  = pines 2/5/3    -> rueda IZQUIERDA
        U17 = pines 8/7/6    -> rueda DERECHA
        U7  = pines 11/12/4  -> rueda TRASERA (no empuja al avanzar recto)
   ===================================================================== */


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

#define LED 13      // LED chiquito de la placa del Teensy


// ---- lo que se puede tocar en caliente ----
int velAvance = 70;              // PWM de las dos ruedas de adelante
int velGiro   = 45;              // PWM de las tres al girar sobre el eje
unsigned long msLado = 1500;     // cuanto dura cada lado
// MEDIDO en el piso el 2026-07-28: 700 ms daba ~160 grados. Regla de tres
// -> 700*90/160 = 394. Pero parte de esos 160 grados es INERCIA (al soltar
// los motores el robot sigue de largo), asi que el tiempo real de motor es
// menor que el proporcional: 400 probablemente siga quedando largo.
unsigned long msGiro = 400;      // cuanto dura cada giro (a calibrar!)

const int LADOS = 4;
const unsigned long MS_ESPERA_INICIAL = 10000;  // para apoyarlo y salir
const unsigned long MS_PAUSA_ENTRE_VUELTAS = 2000;


enum Fase { ESPERANDO, LADO, GIRO, PAUSA, QUIETO };
Fase fase = ESPERANDO;

int ladosHechos = 0;
int vuelta = 0;
unsigned long t_fase = 0;


// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Mismas direcciones que avanzar() de arquero.ino, con la velocidad bajada.
// La trasera va en 0: en un omni de tres ruedas no aporta al avance recto.
void avanzar() {
  analogWrite(PWM1, velAvance); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, velAvance); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);         digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

// Mismas direcciones que girar() de arquero.ino: las tres al mismo sentido.
void girar() {
  analogWrite(PWM1, velGiro); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, velGiro); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, velGiro); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
}


// ---------------------------------------------------------------- consola

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" CUADRADO LENTO — arquero (ROBOT1) — BUCLE INFINITO");
  Serial.println("=================================================");
  Serial.println("  SIN LA PC, SOLO PARA CON LA LLAVE DE LA BATERIA");
  Serial.println("  LED lento = por arrancar | rapido = ya sale");
  Serial.println("-------------------------------------------------");
  Serial.println("  0 = PARAR        g = arrancar de nuevo");
  Serial.println("  d / a = velocidad avance  +10 / -10");
  Serial.println("  t / r = tiempo lado       +200 / -200 ms");
  Serial.println("  y / u = tiempo giro       +50 / -50 ms");
  Serial.print  ("  velAvance="); Serial.print(velAvance);
  Serial.print  ("  velGiro=");   Serial.print(velGiro);
  Serial.print  ("  msLado=");    Serial.print(msLado);
  Serial.print  ("  msGiro=");    Serial.println(msGiro);
  Serial.println("=================================================");
}

void leerConsola() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'g':
      ladosHechos = 0; vuelta = 0;
      fase = ESPERANDO; t_fase = millis();
      Serial.println(">> ESPERANDO 10 s — apoyalo y sacá las manos");
      break;

    case '0':
      parar(); fase = QUIETO; digitalWrite(LED, LOW);
      Serial.println(">> PARADO — 'g' para arrancar de nuevo");
      break;

    case 'd': velAvance += 10; if (velAvance > 255) velAvance = 255;
              Serial.print("   velAvance = "); Serial.println(velAvance); break;
    case 'a': velAvance -= 10; if (velAvance < 0) velAvance = 0;
              Serial.print("   velAvance = "); Serial.println(velAvance); break;

    case 't': msLado += 200; Serial.print("   msLado = "); Serial.println(msLado); break;
    case 'r': if (msLado > 200) msLado -= 200;
              Serial.print("   msLado = "); Serial.println(msLado); break;

    case 'y': msGiro += 50; Serial.print("   msGiro = "); Serial.println(msGiro); break;
    case 'u': if (msGiro > 50) msGiro -= 50;
              Serial.print("   msGiro = "); Serial.println(msGiro); break;

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

  Serial.begin(19200);
  // No espera al monitor: tiene que arrancar igual sin computadora.
  ayuda();
  Serial.println(">> ESPERANDO 10 s — apoyalo y sacá las manos");

  t_fase = millis();
  fase = ESPERANDO;
}


void loop() {

  // La consola se lee SIEMPRE. Si hay PC, el '0' es parada de emergencia.
  leerConsola();

  unsigned long ahora = millis();

  switch (fase) {

    case QUIETO:
      digitalWrite(LED, LOW);
      break;

    case ESPERANDO: {
      unsigned long transcurrido = ahora - t_fase;
      unsigned long falta = (transcurrido >= MS_ESPERA_INICIAL)
                            ? 0 : (MS_ESPERA_INICIAL - transcurrido);
      // ultimos 3 segundos: parpadeo rapido
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);

      if (transcurrido >= MS_ESPERA_INICIAL) {
        vuelta = 1; ladosHechos = 0;
        fase = LADO; t_fase = ahora;
        digitalWrite(LED, HIGH);
        Serial.print(">> VUELTA "); Serial.print(vuelta);
        Serial.println(" — LADO 1");
      }
      break;
    }

    case LADO:
      avanzar();
      if (ahora - t_fase >= msLado) {
        parar();
        fase = GIRO; t_fase = ahora;
        Serial.print("   giro "); Serial.println(ladosHechos + 1);
      }
      break;

    case GIRO:
      girar();
      if (ahora - t_fase >= msGiro) {
        parar();
        ladosHechos++;
        if (ladosHechos >= LADOS) {
          fase = PAUSA; t_fase = ahora;
          Serial.println(">> cuadrado cerrado — pausa");
        } else {
          fase = LADO; t_fase = ahora;
          Serial.print("   lado "); Serial.println(ladosHechos + 1);
        }
      }
      break;

    case PAUSA:
      parar();
      if (ahora - t_fase >= MS_PAUSA_ENTRE_VUELTAS) {
        vuelta++; ladosHechos = 0;
        fase = LADO; t_fase = ahora;
        Serial.print(">> VUELTA "); Serial.print(vuelta);
        Serial.println(" — LADO 1");
      }
      break;
  }
}
