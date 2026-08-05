/* =====================================================================
   CALIBRAR LOS 15 cm — tres saltos, para medir con una regla
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-04
   =====================================================================

   EL PROBLEMA QUE RESUELVE
   El robot no puede medir cuanto se movio: no tiene encoders ni sensor de
   distancia. Solo sabe cuanto rato tuvo los motores prendidos. Asi que para
   que avance 15 cm hay que averiguar, con una regla, que tiempo da 15 cm.

   Y no alcanza con probar UN tiempo, por esto:

   El robot NO TIENE FRENO. Cuando se cortan los motores sigue de largo. Esa
   distancia de desliz es casi la misma siempre, no depende del tiempo que
   estuvo acelerando. Entonces:

        distancia  =  (velocidad x tiempo)  +  desliz

   Hay DOS incognitas. Con una sola medicion no se pueden separar, y si uno
   hace regla de tres directa se equivoca — justo el error que cometimos con
   los giros por cronometro (700 ms daban 160 grados y 400 no dieron 90).

   Con TRES saltos de distintos tiempos se despejan las dos y se calcula el
   tiempo justo para 15 cm.

   ---------------------------------------------------------------------
   COMO SE USA — NO NECESITA COMPUTADORA
   ---------------------------------------------------------------------
   1. Robot en el piso, con como 1,5 metros libres adelante.
   2. Poné algo para marcar (cinta, monedas, lo que sea) a mano.
   3. Prendé la bateria y sacá las manos.
   4. Espera 10 segundos parpadeando. Despues hace TRES saltos.

   Antes de cada salto, el LED parpadea para avisar CUAL viene:
        1 parpadeo  -> viene el salto corto
        2 parpadeos -> viene el mediano
        3 parpadeos -> viene el largo

   Despues de cada salto se queda 15 segundos quieto: ahi marcás donde
   quedo. NO lo muevas, solo marcá.

   5. Al final el LED queda fijo. Ahi medís las tres distancias:
        de la marca de largada a la marca 1
        de la marca 1 a la marca 2
        de la marca 2 a la marca 3

   6. Pasame los tres numeros y calculo el tiempo para 15 cm.

   ⚠️ La bateria cambia el resultado. Anotá con que bateria calibraste.
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

#define LED 13


const int POTENCIA = 200;              // la misma del despeje
const int TIEMPOS[] = { 150, 250, 350 };   // los tres saltos, en ms
const int CUANTOS_SALTOS = 3;

const unsigned long MS_ESPERA_INICIAL = 10000;
const unsigned long MS_ANTES_DEL_SALTO = 3000;   // despues de los parpadeos
const unsigned long MS_PARA_MEDIR      = 15000;  // quieto, para que marques


enum Fase { ESPERANDO, AVISANDO, POR_SALTAR, SALTANDO, MIDIENDO, TERMINADO };
Fase fase = ESPERANDO;

int salto = 0;                 // cual va, 0..CUANTOS_SALTOS-1
unsigned long t_fase = 0;


void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Mismas direcciones que avanzar() de arquero.ino.
void adelante() {
  analogWrite(PWM1, POTENCIA); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, POTENCIA); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  parar();

  Serial.begin(19200);      // por si hay cable; no lo espera
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" CALIBRAR 15 cm — tres saltos");
  Serial.println("=================================================");
  Serial.print(" potencia "); Serial.println(POTENCIA);
  Serial.println(" saltos de 150, 250 y 350 ms");
  Serial.println(" 10 s de espera, y 15 s despues de cada salto");
  Serial.println("=================================================");

  t_fase = millis();
  fase = ESPERANDO;
}


void loop() {
  unsigned long ahora = millis();

  switch (fase) {

    case ESPERANDO: {
      unsigned long falta = (ahora - t_fase >= MS_ESPERA_INICIAL)
                            ? 0 : MS_ESPERA_INICIAL - (ahora - t_fase);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_ESPERA_INICIAL) {
        salto = 0;
        fase = AVISANDO; t_fase = ahora;
        Serial.println(">> arranca");
      }
      break;
    }

    case AVISANDO: {
      // Parpadea (salto+1) veces, con una pausa larga al final, para que se
      // pueda contar de lejos y sin computadora.
      unsigned long ciclo = ahora - t_fase;
      unsigned long total = (unsigned long)(salto + 1) * 600 + 1200;
      if (ciclo < (unsigned long)(salto + 1) * 600) {
        digitalWrite(LED, ((ciclo % 600) < 250) ? HIGH : LOW);
      } else {
        digitalWrite(LED, LOW);
      }
      if (ciclo >= total) {
        fase = POR_SALTAR; t_fase = ahora;
        Serial.print(">> salto "); Serial.print(salto + 1);
        Serial.print(" de "); Serial.print(CUANTOS_SALTOS);
        Serial.print(" — "); Serial.print(TIEMPOS[salto]);
        Serial.println(" ms");
      }
      break;
    }

    case POR_SALTAR:
      parar();
      if (ahora - t_fase >= MS_ANTES_DEL_SALTO) {
        fase = SALTANDO; t_fase = ahora;
        digitalWrite(LED, HIGH);
      }
      break;

    case SALTANDO:
      adelante();
      if (ahora - t_fase >= (unsigned long)TIEMPOS[salto]) {
        parar();
        fase = MIDIENDO; t_fase = ahora;
        Serial.println("   ...ahora marcá donde quedo (15 s)");
      }
      break;

    case MIDIENDO:
      parar();
      // parpadeo lento y parejo: "estoy esperando que midas"
      digitalWrite(LED, ((ahora / 1000) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_PARA_MEDIR) {
        salto++;
        if (salto >= CUANTOS_SALTOS) {
          fase = TERMINADO; t_fase = ahora;
          Serial.println(">> LISTO — medí las tres distancias y pasalas");
        } else {
          fase = AVISANDO; t_fase = ahora;
        }
      }
      break;

    case TERMINADO:
      parar();
      digitalWrite(LED, HIGH);      // fijo = termine
      break;
  }
}
