/* =====================================================================
   PROBAR EL FRENO — tres formas de detenerse, medidas con regla
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-18
   =====================================================================

   EL PROBLEMA
   Volviendo del despeje, el robot detecta la linea blanca pero **se pasa**:
   queda mas adelante o mas atras, distinto cada vez. La causa es que hoy no
   frena — solo suelta los motores, y el robot sigue de largo por inercia.

   Se puede pedir un freno electrico, que cortocircuita los bornes del motor
   para que la corriente que el mismo genera lo detenga. Pero **hay dos
   formas de pedirlo y no todas las placas soportan las dos**. Hay que medir
   cual funciona en esta.

   ---------------------------------------------------------------------
   POR QUE ESTE SKETCH Y NO LA PRUEBA EN LA MESA
   ---------------------------------------------------------------------
   Ya se intento con el cable puesto sobre la mesa, con un retroceso de
   250 ms, y el robot **se movio menos de 3 cm**. Con eso no se puede medir
   nada: un freno solo se nota si hay algo que frenar, y en 3 cm el robot
   nunca llego a agarrar velocidad. Estuvo acelerando todo el tiempo y se
   detuvo casi solo. Con o sin freno iba a quedar igual.

   👉 **Un experimento que no puede distinguir las dos respuestas no es una
   medicion.** Aca la corrida dura 1 segundo, para que el robot llegue a la
   velocidad que realmente tiene cuando vuelve del despeje.

   Y como el cable USB no llega a la cancha, el robot hace las tres corridas
   solo y avisa con el LED.

   ---------------------------------------------------------------------
   COMO SE USA — EN EL PISO, SIN CABLE
   ---------------------------------------------------------------------
   Necesita como 2 metros libres HACIA ATRAS del robot (retrocede tres
   veces) y algo para marcar.

     1. Apoyalo y MARCA DONDE ESTA. Esa marca es el punto de partida de las
        tres corridas.
     2. Prendé la bateria y sacá las manos. Espera 10 segundos.

     1 parpadeo  -> corrida 1: retrocede y frena con la VARIANTE 1
     2 parpadeos -> corrida 2: retrocede y frena con la VARIANTE 2
     3 parpadeos -> corrida 3: retrocede y SUELTA (como esta hoy)

     Despues de cada corrida se queda quieto 20 segundos: ahi **marcás donde
     quedo y lo volves a la marca de partida**. Las tres tienen que arrancar
     del mismo lugar o no se pueden comparar.

     Parpadeo lento sin fin -> termino.

     3. Medí las tres distancias desde la marca de partida.

   ---------------------------------------------------------------------
   COMO SE LEE EL RESULTADO
   ---------------------------------------------------------------------
   La corrida 3 (soltar) es la referencia: es lo que hace el robot hoy.

     - Si alguna de las dos primeras quedo CLARAMENTE MAS CERCA -> esa
       variante frena, y esa va al despeje.
     - Si las tres quedaron parecidas -> esta placa no soporta el freno
       electrico. La salida es la contramarcha: un golpe corto de motor en
       sentido contrario. Funciona en cualquier driver, pero hay que
       calibrar cuanto dura.

   ---------------------------------------------------------------------
   🚨 Sin computadora, la unica forma de pararlo es la llave de la bateria.
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


// La misma potencia con la que el robot vuelve del despeje, para que la
// prueba reproduzca la situacion real.
const int POTENCIA = 110;

// 1 segundo: lo suficiente para que llegue a velocidad. La prueba de 250 ms
// en la mesa fallo justamente por corta.
const unsigned long MS_CORRIDA = 1000;

const unsigned long MS_ESPERA_INICIAL = 10000;
const unsigned long MS_FRENO          = 200;   // cuanto se sostiene el freno
const unsigned long MS_PARA_MEDIR     = 20000; // marcar y reposicionar
const unsigned long MS_ANTES_DE_ARRANCAR = 2000;

const int CUANTAS = 3;


void parar() {   // soltar: las dos patas en 0 y el PWM en 0
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// VARIANTE 1: las dos patas en ALTO, PWM al maximo.
void frenarVariante1() {
  digitalWrite(INA1, 1); digitalWrite(INB1, 1); analogWrite(PWM1, 255);
  digitalWrite(INA2, 1); digitalWrite(INB2, 1); analogWrite(PWM2, 255);
  digitalWrite(INA3, 1); digitalWrite(INB3, 1); analogWrite(PWM3, 255);
}

// VARIANTE 2: las dos patas en BAJO, PWM al maximo.
// NO es lo mismo que soltar: al soltar el PWM queda en CERO, lo que apaga
// la salida del driver. Aca el PWM va al maximo.
void frenarVariante2() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

// Retroceder recto: avanzar() de arquero.ino con las direcciones dadas
// vuelta. La trasera no aporta al movimiento recto.
void atras() {
  analogWrite(PWM1, POTENCIA); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, POTENCIA); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);        digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


enum Fase { ESPERANDO, AVISANDO, POR_ARRANCAR, CORRIENDO, FRENANDO,
            MIDIENDO, TERMINADO };
Fase fase = ESPERANDO;

int corrida = 0;               // 0, 1 o 2
unsigned long t_fase = 0;


const char* nombreCorrida(int c) {
  switch (c) {
    case 0:  return "FRENO VARIANTE 1 (las dos patas en alto)";
    case 1:  return "FRENO VARIANTE 2 (las dos patas en bajo)";
    default: return "SOLTAR (como esta hoy)";
  }
}


// Parpadea `veces` y despues una pausa larga, para poder contarlo de lejos.
void parpadearVeces(int veces, unsigned long ahora) {
  unsigned long largo = (unsigned long)veces * 500 + 1500;
  unsigned long ciclo = ahora % largo;
  if (ciclo < (unsigned long)veces * 500) {
    digitalWrite(LED, ((ciclo % 500) < 220) ? HIGH : LOW);
  } else {
    digitalWrite(LED, LOW);
  }
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
  Serial.println(" PROBAR EL FRENO — tres corridas iguales");
  Serial.println("=================================================");
  Serial.print(" retroceso de "); Serial.print(MS_CORRIDA);
  Serial.print(" ms a potencia ");  Serial.println(POTENCIA);
  Serial.println(" 1 parpadeo = variante 1");
  Serial.println(" 2 parpadeos = variante 2");
  Serial.println(" 3 parpadeos = soltar (referencia)");
  Serial.println(" 20 s despues de cada una: marcá y volvelo a la partida");
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
        corrida = 0;
        fase = AVISANDO; t_fase = ahora;
      }
      break;
    }

    case AVISANDO: {
      int veces = corrida + 1;
      parpadearVeces(veces, ahora);
      // avisa durante 4 segundos antes de arrancar
      if (ahora - t_fase >= 4000) {
        fase = POR_ARRANCAR; t_fase = ahora;
        Serial.print(">> corrida "); Serial.print(veces);
        Serial.print(" de 3 — ");    Serial.println(nombreCorrida(corrida));
      }
      break;
    }

    case POR_ARRANCAR:
      parar();
      digitalWrite(LED, LOW);
      if (ahora - t_fase >= MS_ANTES_DE_ARRANCAR) {
        fase = CORRIENDO; t_fase = ahora;
        digitalWrite(LED, HIGH);
      }
      break;

    case CORRIENDO:
      atras();
      if (ahora - t_fase >= MS_CORRIDA) {
        // Aca esta toda la diferencia entre las tres corridas.
        if      (corrida == 0) frenarVariante1();
        else if (corrida == 1) frenarVariante2();
        else                   parar();
        fase = FRENANDO; t_fase = ahora;
      }
      break;

    case FRENANDO:
      // Sostener el freno un ratito y despues soltar, asi no queda el driver
      // cortocircuitado de gusto. En la corrida de "soltar" no hace nada
      // porque ya estaba suelto.
      if (ahora - t_fase >= MS_FRENO) {
        parar();
        fase = MIDIENDO; t_fase = ahora;
        Serial.println("   ...marcá donde quedo y volvelo a la partida (20 s)");
      }
      break;

    case MIDIENDO:
      parar();
      digitalWrite(LED, ((ahora / 1000) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_PARA_MEDIR) {
        corrida++;
        if (corrida >= CUANTAS) {
          fase = TERMINADO; t_fase = ahora;
          Serial.println(">> LISTO — compará las tres distancias.");
          Serial.println("   la mas corta gana. Si las tres son parecidas,");
          Serial.println("   esta placa no frena y hay que ir a contramarcha.");
        } else {
          fase = AVISANDO; t_fase = ahora;
        }
      }
      break;

    case TERMINADO:
      parar();
      digitalWrite(LED, ((ahora / 1500) % 2) ? HIGH : LOW);
      break;
  }
}
