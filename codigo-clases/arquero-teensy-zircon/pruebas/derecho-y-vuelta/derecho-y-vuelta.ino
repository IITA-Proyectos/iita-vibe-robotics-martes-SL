/* =====================================================================
   DERECHO Y VUELTA — ¿se tuerce el robot al avanzar? ¿cuanto lo arregla
   el truco del delantero?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-29
   =====================================================================

   DE DONDE SALE
   La otra mesa (delantero) descubrio el 2026-09-01 que su robot NO VA
   DERECHO cuando patea, y lo midio:

        sin correccion:   10.1 grados de desvio   (pico 11.3)
        con heading-hold:  4.2 grados             (pico  5.4)

   Y el diagnostico es de manual: avanzar() manda el MISMO PWM a las dos
   ruedas de adelante, pero el mismo PWM no es la misma velocidad — son dos
   motores distintos. Y la rueda trasera queda SUELTA, asi que no hay nada
   que se oponga al giro. Cualquier desbalance se convierte en curva y
   nadie la corrige.

   🎯 NUESTRO DESPEJE HACE EXACTAMENTE ESO: sale a 200 de potencia, derecho,
   533 ms. Si el robot se curva 10 grados mientras empuja, la pelota no sale
   para donde apuntamos — y hasta ahora lo veniamos echando a la punteria.

   Este programa mide las dos cosas EN LA MISMA CORRIDA, con los mismos
   motores, la misma bateria y el mismo piso. Asi la comparacion es justa.

   ---------------------------------------------------------------------
   EL TRUCO DEL DELANTERO: LA CORRECCION SOLO FRENA, NUNCA ACELERA
   ---------------------------------------------------------------------
   Lo normal seria: "si me voy para la izquierda, acelero la rueda
   izquierda". El problema es que a 200 sobre un maximo de 255 casi no
   queda lugar para subir, y a 240 no queda nada: el lazo pediria acelerar,
   saturaria, y no pasaria nada.

   Ellos lo dieron vuelta: se FRENA la rueda del lado contrario. Restar
   siempre se puede, incluso a fondo. Por eso funciona a plena potencia.

        error = rumbo que quiero - rumbo que tengo
        resta = |error| * KP        (con tope)
        si error > 0 -> frena la derecha ; si no -> frena la izquierda

   ⚠️ AL RETROCEDER, EL LADO SE DA VUELTA. Frenar la rueda izquierda
   yendo para adelante hace girar al robot para un lado; yendo para atras,
   la misma rueda frenada lo hace girar para el OTRO. Aca esta contemplado.

   ---------------------------------------------------------------------
   QUE HACE, EN ORDEN
   ---------------------------------------------------------------------
        1. arranca el giroscopio y espera a que se calibre QUIETO
        2. 10 segundos de espera (LED lento, despues rapido)
        3. ADELANTE sin corregir   -> mide cuanto se torcio
        4. ATRAS    sin corregir   -> mide
        5. ADELANTE corrigiendo    -> mide
        6. ATRAS    corrigiendo    -> mide
        7. informa la tabla comparativa y se queda quieto

   Entre tramo y tramo frena y espera a que la inercia termine ANTES de
   medir: si se midiera con el robot todavia girando, el numero seria del
   frenado y no del tramo.

   ---------------------------------------------------------------------
   EL HISTORIAL: SE GUARDA SOLO, EN LA EEPROM
   ---------------------------------------------------------------------
   En la cancha no hay cable, asi que todo lo que el robot diga por USB se
   tira a la basura. Por eso cada corrida se guarda en la EEPROM, que es la
   memoria que NO se borra: ni al apagar, ni al sacar la bateria, ni al
   cargar otro programa encima.

   Se guardan las ultimas 12 corridas, y ademas dos contadores que no se
   pierden nunca: cuantas veces se corrio en total y en cuantas ARRANCO EL
   GIROSCOPIO. Ese segundo numero es el que veniamos contando a mano
   (hoy: 5 de 7) y ahora lo lleva el robot solo.

   🎯 Las corridas en que el giroscopio NO arranco tambien se anotan. Son
   la mitad del dato: sin ellas la cuenta de "cuantas de cada diez" no sale.

   Para leerlo: enchufar el USB y abrir mirar.bat. La tabla entera se
   imprime SOLA apenas arranca, antes de la cuenta regresiva.

   Con el cable puesto, dos teclas:
        i = volver a mostrar el historial
        x = borrarlo (pide confirmacion: hay que apretar X mayuscula)

   ---------------------------------------------------------------------
   🚨 EL ROBOT SE MUEVE — NECESITA BASTANTE PISO
   ---------------------------------------------------------------------
   Cada tramo son ~2 METROS, a velocidad casi minima (80 de potencia, 5
   segundos). Va y vuelve cuatro veces, asi que termina cerca de donde
   arranco — pero si se tuerce, no. Darle 3 metros libres adelante y un
   metro a cada costado.

   Son cuatro tramos de 5 s mas las pausas: la corrida entera dura como 35
   segundos. Con la llave de la bateria se corta cuando quieran.

   Sin la computadora, la unica forma de pararlo es LA LLAVE DE LA BATERIA.

        LED lento   -> faltan mas de 3 s, todavia lo podes tocar
        LED rapido  -> faltan menos de 3 s, SOLTALO
        LED fijo    -> en movimiento
        LED parpadeo RAPIDO -> no hay giroscopio, no me muevo
                               (el mismo aviso que el programa de juego)

   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA y hay que prenderla ANTES de
   enchufar el USB, si no el Teensy arranca sin giroscopio.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <EEPROM.h>

#define LED 13

// Ruedas del arquero (medido en banco el 2026-07-28).
#define IZQ_INA 2
#define IZQ_INB 5
#define IZQ_PWM 3
#define DER_INA 8
#define DER_INB 7
#define DER_PWM 6
#define TRA_INA 11
#define TRA_INB 12
#define TRA_PWM 4

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// ---- los numeros del experimento ----
//
// 2026-09-29, pedido del equipo: CASI LA VELOCIDAD MINIMA y tramos LARGOS,
// de unos 2 metros. Las dos cosas ayudan a ver el desvio: despacio se puede
// seguir con la vista, y largo deja que el error se acumule hasta que se note.
//
// 🚨 EL PISO DE ARRANQUE DE ESTOS MOTORES ES ~70 PWM. Debajo de eso no
// vencen el roce y no arrancan. 80 esta apenas 10 arriba, asi que la
// diferencia entre los dos motores pesa MUCHO mas que a 200: puede que uno
// arranque y el otro no. Por eso hay un empujoncito de arranque (ver abajo).
// Si aun asi una rueda no arranca, subir VEL de a 10.
const int VEL          = 80;     // casi el minimo (el piso es ~70)
const int VEL_ARRANQUE = 140;    // empujoncito para vencer el roce parado
const int MS_ARRANQUE  = 120;    // lo que dura el empujoncito

// ~2 metros. La cuenta sale de la calibracion con regla del 04/08: a
// potencia 200 el robot hace 1 cm por cada 10 ms, o sea 100 cm/s. A 80
// deberia hacer unos 40 cm/s, asi que 200 cm son unos 5 segundos.
// ⚠️ ESA REGLA DE TRES ES UNA ESTIMACION, no una medicion: cerca del piso
// de arranque la velocidad NO es proporcional al PWM (pesa el roce). Muy
// probablemente ande mas lento que 40 cm/s. Miren cuanto avanza de verdad y
// ajusten este numero: si hizo 1,5 m, subanlo a 6700.
const int MS_TRAMO     = 5000;

// El tope de la correccion, en proporcion. El delantero usa 120 sobre una
// patada de 200, o sea el 60%. Aca la marcha es de 80, asi que el 60% son
// 48. Dejarlo en 120 no tendria sentido: con restar 80 la rueda ya queda en
// cero y el robot pivotea en el lugar en vez de corregir.
const float KP       = 4.0;    // PWM que se resta por grado (el del delantero)
const int RESTA_MAX  = 45;     // 120 era para potencia 200

// El aviso de "no hay giroscopio", igual que en el programa de juego:
// 50 ms prendido + 50 apagado = 10 destellos por segundo.
const unsigned long MS_PARPADEO_AVISO = 50;
const unsigned long MS_ESPERA_INICIAL = 10000;
const unsigned long MS_ASENTAR        = 600;   // que termine la inercia
const unsigned long MS_ENTRE_TRAMOS   = 1500;

// ⚠️ SI EL ROBOT SE CURVA CADA VEZ MAS EN VEZ DE IR DERECHO, DALO VUELTA.
// No sabemos de antemano que sentido de giro hace subir el rumbo en ESTE
// robot: depende de como quedo montado el sensor. La otra mesa lo midio
// para el suyo, dedujo la regla... y en cancha estaba al reves. Asi que
// aca es un booleano y se prueba, no se deduce. El sintoma es inconfundible:
// con el signo bien el robot va derecho, con el signo mal se va cerrando
// como un caracol.
const bool CORRECCION_INVERTIDA = false;

bool hayGiroscopo = false;
uint8_t calAlArrancar = 0;       // 0..3, la calibracion al fijar el cero

// Resultados: [0]=adelante sin, [1]=atras sin, [2]=adelante con, [3]=atras con
float desvioFinal[4] = {0, 0, 0, 0};
float desvioPico[4]  = {0, 0, 0, 0};
const char* nombreTramo[4] = {
  "ADELANTE sin corregir", "ATRAS    sin corregir",
  "ADELANTE corrigiendo  ", "ATRAS    corrigiendo  "
};


// ------------------------------------------------- EL HISTORIAL EN LA EEPROM
//
// La EEPROM del Teensy 4.1 son ~4 KB que sobreviven al apagado. Aca entra
// todo con muchisimo lugar de sobra.
//
// La "magia" es un numero inventado que se escribe al principio. Sirve para
// saber si lo que hay guardado es NUESTRO: una EEPROM virgen (o la que dejo
// otro programa, como medir-ancho) tiene basura, y sin esta marca leeriamos
// esa basura como si fueran corridas. Si algun dia cambia el formato de la
// tabla, se cambia este numero y el historial viejo se descarta solo.
// Formato 2 (2026-09-29): los tramos ahora pueden valer -1 = "el
// giroscopio se cayo, este numero no sirve". Al cambiar el numero de
// magia, el historial del formato viejo se descarta solo — y hay que
// descartarlo, porque ahi los ceros eran caidas disfrazadas de exito.
const uint32_t MAGIA = 0x44565432;      // "DVT2" = derecho-y-vuelta, formato 2
const int MAX_GUARDADAS = 12;

// Los angulos se guardan en DECIMAS de grado y como enteros: un float ocupa
// el doble y aca no hace falta esa precision.
struct Corrida {
  uint16_t numero;       // que numero de corrida fue, desde siempre
  uint8_t  arranco;      // 1 = el giroscopio dio datos
  uint8_t  calGiro;      // calibracion del giroscopo al empezar (0..3)
  int16_t  fin[4];       // desvio final de cada tramo, en decimas de grado
  int16_t  pico[4];      // el peor desvio de cada tramo
};

struct Cabecera {
  uint32_t magia;
  uint16_t total;        // corridas desde que se borro el historial
  uint16_t arrancaron;   // en cuantas arranco el giroscopio
  uint8_t  guardadas;    // cuantas hay en la tabla (hasta MAX_GUARDADAS)
  uint8_t  proxima;      // en que ranura escribir la que viene
};

// 🚨 LA EEPROM ES UNA SOLA Y LA COMPARTEN TODOS LOS PROGRAMAS.
//
// Este historial arrancaba en la direccion 0... que es justo donde
// `pruebas/medir-ancho/` guarda los tiempos de cruce de la cancha. Resultado:
// la primera corrida de este programa PISO la medicion del 21/09 y
// medir-ancho quedo diciendo "no hay ninguna medicion guardada".
//
// Reparto de la EEPROM del arquero (4 KB en el Teensy 4.1), para que no
// vuelva a pasar. Si alguien agrega otro programa que guarde algo, que se
// anote aca y que use una direccion libre:
//
//      0  ..  255   medir-ancho / medir-ancho-sin-giro  (tiempos de cruce)
//    256  ..  767   derecho-y-vuelta  (este historial)
//    768  .. 4095   libre
const int DIR_CABECERA = 256;
const int DIR_TABLA    = DIR_CABECERA + sizeof(Cabecera);

Cabecera cab;

void leerCabecera() {
  EEPROM.get(DIR_CABECERA, cab);
  if (cab.magia != MAGIA) {          // virgen, o de otro programa
    cab.magia = MAGIA;
    cab.total = 0; cab.arrancaron = 0;
    cab.guardadas = 0; cab.proxima = 0;
    EEPROM.put(DIR_CABECERA, cab);
  }
}

// Se llama UNA vez por corrida, incluso cuando el giroscopio no arranco.
void guardarCorrida(bool arranco) {
  Corrida c;
  cab.total++;
  if (arranco) cab.arrancaron++;
  c.numero  = cab.total;
  c.arranco = arranco ? 1 : 0;
  c.calGiro = calAlArrancar;
  for (int i = 0; i < 4; i++) {
    c.fin[i]  = (int16_t)(desvioFinal[i] * 10.0);
    c.pico[i] = (int16_t)(desvioPico[i]  * 10.0);
  }
  EEPROM.put(DIR_TABLA + cab.proxima * (int)sizeof(Corrida), c);

  // Tabla circular: cuando se llena, la mas vieja se pisa. Los contadores
  // de arriba NO se pisan nunca, asi que la estadistica sigue siendo de
  // todas las corridas aunque solo queden las ultimas 12 en detalle.
  cab.proxima = (cab.proxima + 1) % MAX_GUARDADAS;
  if (cab.guardadas < MAX_GUARDADAS) cab.guardadas++;
  EEPROM.put(DIR_CABECERA, cab);
}

void unDecimal(int16_t decimas) {
  Serial.print(decimas / 10); Serial.print('.'); Serial.print(abs(decimas) % 10);
}

void mostrarHistorial() {
  Serial.println();
  Serial.println("---------------- HISTORIAL (en la EEPROM) ----------------");
  if (cab.total == 0) {
    Serial.println(" Todavia no hay ninguna corrida guardada.");
    Serial.println("----------------------------------------------------------");
    return;
  }
  Serial.print(" Corridas: "); Serial.print(cab.total);
  Serial.print("    arranco el giroscopio en ");
  Serial.print(cab.arrancaron); Serial.print(" de ellas");
  if (cab.total > 0) {
    Serial.print("  ("); Serial.print((cab.arrancaron * 100) / cab.total);
    Serial.print("%)");
  }
  Serial.println();
  Serial.println();
  Serial.println(" n   giro  cal    adel-sin  atras-sin  adel-con  atras-con");
  Serial.println(" --  ----  ---    --------  ---------  --------  ---------");

  // De la mas vieja a la mas nueva. Si la tabla se lleno, la mas vieja es
  // justo la que sigue a la ultima escrita.
  int inicio = (cab.guardadas < MAX_GUARDADAS)
             ? 0 : cab.proxima;
  for (int k = 0; k < cab.guardadas; k++) {
    int ranura = (inicio + k) % MAX_GUARDADAS;
    Corrida c;
    EEPROM.get(DIR_TABLA + ranura * (int)sizeof(Corrida), c);
    Serial.print(" ");
    if (c.numero < 10) Serial.print(" ");
    Serial.print(c.numero);
    Serial.print(c.arranco ? "    SI " : "    no ");
    Serial.print("   ");  Serial.print(c.calGiro);
    if (!c.arranco) {
      Serial.println("     (no se movio)");
      continue;
    }
    for (int i = 0; i < 4; i++) {
      if (c.fin[i] < 0) {
        Serial.print("      mudo");     // se cayo en ese tramo
      } else {
        Serial.print("      ");
        unDecimal(c.fin[i]);
      }
    }
    Serial.println();
  }
  Serial.println();
  Serial.println(" Los numeros son el desvio FINAL de cada tramo, en grados.");
  Serial.println(" \"mudo\" = el giroscopio se cayo en ese tramo y el numero no");
  Serial.println(" sirve. Ojo: NO es que fue derecho.");
  Serial.println(" Los dos primeros van SIN corregir (el control) y los dos");
  Serial.println(" ultimos CORRIGIENDO. Si el truco sirve, los de la derecha");
  Serial.println(" tienen que ser mas chicos que los de la izquierda.");
  Serial.println("----------------------------------------------------------");
}

// Con el cable puesto: i = mostrar, X = borrar (mayuscula a proposito, para
// que no se borre de un dedazo).
void leerTeclado() {
  while (Serial.available()) {
    char t = Serial.read();
    if (t == 'i') mostrarHistorial();
    else if (t == 'X') {
      cab.magia = MAGIA; cab.total = 0; cab.arrancaron = 0;
      cab.guardadas = 0; cab.proxima = 0;
      EEPROM.put(DIR_CABECERA, cab);
      Serial.println();
      Serial.println(">> historial borrado");
    } else if (t == 'x') {
      Serial.println();
      Serial.println(">> para borrar el historial apreta X (mayuscula)");
    }
  }
}



// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Frenar de verdad: las dos patas en bajo y PWM a fondo cortocircuita el
// motor y lo detiene. parar() solo lo suelta, y el robot sigue de largo.
void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

// El corazon del asunto, copiado del delantero y adaptado.
//
// `adelante` decide el sentido de marcha. `corregir` es lo que se esta
// comparando: con false hace exactamente lo de siempre (las dos ruedas al
// mismo PWM y la trasera suelta), con true le suma el heading-hold.
void marchar(bool adelante, bool corregir, float rumboObjetivo, int vel) {
  int vi = vel, vd = vel;

  float r = rumboActual();
  // Sin rumbo no se corrige: mejor ir derecho a lo que salga que empujar una
  // rueda por una cuenta hecha con un numero inventado.
  if (corregir && r >= 0 && rumboObjetivo >= 0) {
    float err = diferencia(rumboObjetivo, r);
    int resta = (int)(fabs(err) * KP);
    if (resta > RESTA_MAX) resta = RESTA_MAX;

    // Al retroceder, frenar la misma rueda tuerce al robot para el otro
    // lado: el lado a frenar se da vuelta.
    bool frenarDerecha = (err > 0);
    if (!adelante)            frenarDerecha = !frenarDerecha;
    if (CORRECCION_INVERTIDA) frenarDerecha = !frenarDerecha;

    if (frenarDerecha) vd -= resta; else vi -= resta;
    if (vi < 0) vi = 0;
    if (vd < 0) vd = 0;
  }

  // Las dos de adelante empujan; la trasera queda SUELTA, igual que en el
  // despeje y que en la patada del delantero. Es a proposito: lo que se
  // esta midiendo es justamente ese caso.
  analogWrite(IZQ_PWM, vi);
  digitalWrite(IZQ_INA, adelante ? 1 : 0); digitalWrite(IZQ_INB, adelante ? 0 : 1);
  analogWrite(DER_PWM, vd);
  digitalWrite(DER_INA, adelante ? 0 : 1); digitalWrite(DER_INB, adelante ? 1 : 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}


// ---------------------------------------------------------------- rumbo

// La diferencia mas corta entre dos rumbos, en (-180, 180]. Sin esto, ir de
// 350 a 10 grados se leeria como un giro de -340 en vez de +20.
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// 🚨 EL AGUJERO QUE TENIA ESTA FUNCION, Y POR QUE IMPORTA TANTO
//
// Antes devolvia el angulo y listo. El problema es que la libreria Adafruit
// devuelve 0.0 EN LOS TRES ANGULOS cuando no puede leer el chip. Entonces,
// si el giroscopio se caia en medio de un tramo, la cuenta del desvio daba
//
//      0.0 - 0.0 = 0.0
//
// y el programa lo anotaba como "salio PERFECTO". O sea que confundia
// "fue derechito" con "se murio el sensor" — y justo con este giroscopio,
// que se suelta cada dos por tres, eso arruina la medicion entera.
//
// Ahora devuelve -1 cuando el sensor no contesta, y ademas lleva la cuenta
// de cuantas lecturas se perdieron. Un tramo con UNA sola lectura mala ya
// no se da por bueno: durante ese rato la correccion estuvo manejando a
// ciegas, asi que el numero no significa nada.
long lecturasMudas = 0;

float rumboActual() {
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    lecturasMudas++;
    return -1;
  }
  return e.orientation.x;
}


// ---------------------------------------------------------------- un tramo

void correrTramo(int indice, bool adelante, bool corregir) {
  lecturasMudas = 0;
  float rumbo0 = rumboActual();
  float pico = 0;

  digitalWrite(LED, HIGH);
  unsigned long t0 = millis();
  while (millis() - t0 < MS_TRAMO) {
    // Empujoncito: los primeros MS_ARRANQUE ms va mas fuerte, para vencer
    // el roce de estar parado. Despues baja a la velocidad lenta de verdad.
    // Sin esto, a 80 desde quieto puede que ni arranque.
    int vel = (millis() - t0 < MS_ARRANQUE) ? VEL_ARRANQUE : VEL;
    marchar(adelante, corregir, rumbo0, vel);
    float r = rumboActual();
    if (r >= 0 && rumbo0 >= 0) {
      float d = fabs(diferencia(rumbo0, r));
      if (d > pico) pico = d;
    }
  }

  // Frenar y DESPUES medir. Si se midiera con el robot todavia girando por
  // la inercia, el numero seria del frenado y no del tramo.
  frenar();
  delay(MS_ASENTAR);
  parar();
  digitalWrite(LED, LOW);

  float rf = rumboActual();
  bool vale = (rumbo0 >= 0 && rf >= 0 && lecturasMudas == 0);

  Serial.print("  "); Serial.print(nombreTramo[indice]);
  if (!vale) {
    // -1 es la marca de "este numero no sirve". Se guarda igual, porque
    // saber que el sensor se cayo EN ESE TRAMO tambien es un dato.
    desvioFinal[indice] = -1;
    desvioPico[indice]  = -1;
    Serial.print("   NO VALE: el giroscopio se cayo (");
    Serial.print(lecturasMudas); Serial.println(" lecturas perdidas)");
  } else {
    desvioPico[indice]  = pico;
    desvioFinal[indice] = fabs(diferencia(rumbo0, rf));
    Serial.print("   desvio final "); Serial.print(desvioFinal[indice], 1);
    Serial.print(" gr   (pico ");     Serial.print(desvioPico[indice], 1);
    Serial.println(")");
  }

  delay(MS_ENTRE_TRAMOS);
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Wire.begin();
  delay(500);

  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" DERECHO Y VUELTA — ¿cuanto se tuerce, y cuanto lo arregla");
  Serial.println(" el truco del delantero? 🚨 EL ROBOT SE MUEVE, va al piso.");
  Serial.println("=========================================================");

  // Lo primero: mostrar lo que ya hay guardado. Asi, si vienen de correrlo
  // sin cable, con solo enchufar el USB ya ven todo.
  leerCabecera();
  mostrarHistorial();

  // Arranque del giroscopio: lo mejor de las dos mesas junto.
  //   - los 10 reintentos son nuestros (la carrera de encendido del 01/09)
  //   - el orden del delay y el SYS_STATUS son del delantero
  //   - esperar la calibracion no lo hace ninguno de los dos, y es lo que
  //     evita que el cero se corra solo mientras medimos
  Serial.print(" giroscopio: ");
  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (hayGiroscopo) {
    bno.setExtCrystalUse(true);
    delay(700);                  // DESPUES: setExtCrystalUse reinicia la fusion
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    if (sys != 5) {
      Serial.print("contesta pero la fusion NO corre (SYS_STATUS=");
      Serial.print(sys); Serial.println(")");
      hayGiroscopo = false;
    }
  }
  if (!hayGiroscopo) {
    Serial.println("NO CONTESTA — no me muevo. ¿Esta prendida la bateria?");
    // 🎯 Se anota igual. Las corridas fallidas son la mitad del dato: sin
    // ellas no sale la cuenta de "cuantas de cada diez arranca".
    guardarCorrida(false);
    Serial.print(" (anotado: van "); Serial.print(cab.arrancaron);
    Serial.print(" de "); Serial.print(cab.total); Serial.println(")");
    return;                      // el loop hace el parpadeo rapido
  }

  Serial.println("OK");
  Serial.print(" esperando que se calibre (quieto): ");
  unsigned long tc = millis();
  uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
  while (millis() - tc < 8000) {
    bno.getCalibration(&sis, &gir, &ace, &mag);
    if (gir >= 3) break;
    delay(100);
  }
  calAlArrancar = gir;
  Serial.print("giroscopo "); Serial.print(gir); Serial.print("/3 en ");
  Serial.print(millis() - tc); Serial.println(" ms");

  Serial.println();
  Serial.println(" 10 segundos y arranca. NO LO TOQUES.");

  // Cuenta de arranque: lento, y rapido en los ultimos 3 segundos.
  unsigned long t0 = millis();
  while (millis() - t0 < MS_ESPERA_INICIAL) {
    unsigned long falta = MS_ESPERA_INICIAL - (millis() - t0);
    unsigned long periodo = (falta <= 3000) ? 100 : 500;
    digitalWrite(LED, ((millis() / periodo) % 2) ? HIGH : LOW);
  }

  Serial.println();
  correrTramo(0, true,  false);
  correrTramo(1, false, false);
  correrTramo(2, true,  true);
  correrTramo(3, false, true);

  parar();

  // ---- la tabla, que es para lo que se hizo todo esto ----
  //
  // Los tramos que no valen quedan AFUERA del promedio. Meterlos como cero
  // era justamente el error: hacia parecer que el robot iba perfecto cuando
  // en realidad el sensor se habia caido.
  float sumaSin = 0, sumaCon = 0;
  int nSin = 0, nCon = 0;
  for (int i = 0; i < 2; i++) if (desvioFinal[i] >= 0) { sumaSin += desvioFinal[i]; nSin++; }
  for (int i = 2; i < 4; i++) if (desvioFinal[i] >= 0) { sumaCon += desvioFinal[i]; nCon++; }

  Serial.println();
  Serial.println("=========================================================");
  if (nSin == 0 || nCon == 0) {
    Serial.println(" NO SE PUEDE COMPARAR: el giroscopio se cayo en el medio.");
    Serial.print(" Tramos utiles: "); Serial.print(nSin);
    Serial.print(" de 2 sin corregir, "); Serial.print(nCon);
    Serial.println(" de 2 corrigiendo.");
    Serial.println(" Hay que repetirlo con el sensor firme.");
    Serial.println("=========================================================");
    guardarCorrida(true);
    Serial.print(" guardado como corrida "); Serial.println(cab.total);
    mostrarHistorial();
    return;
  }
  float sinCorregir = sumaSin / nSin;
  float corrigiendo = sumaCon / nCon;
  Serial.print(" SIN CORREGIR:  "); Serial.print(sinCorregir, 1);
  Serial.print(" grados de desvio promedio  (");
  Serial.print(nSin); Serial.println(" tramo/s utiles)");
  Serial.print(" CORRIGIENDO:   "); Serial.print(corrigiendo, 1);
  Serial.print(" grados de desvio promedio  (");
  Serial.print(nCon); Serial.println(" tramo/s utiles)");
  Serial.println();
  if (corrigiendo < sinCorregir - 1.0) {
    Serial.println(" >> EL TRUCO SIRVE. Conviene meterlo en el despeje.");
  } else if (corrigiendo > sinCorregir + 1.0) {
    Serial.println(" >> EMPEORO. Casi seguro el signo esta al reves:");
    Serial.println("    da vuelta CORRECCION_INVERTIDA y volve a probar.");
  } else {
    Serial.println(" >> NO CAMBIA NADA. O este robot ya va derecho, o KP es");
    Serial.println("    muy chico. Fijate primero el desvio sin corregir: si");
    Serial.println("    ya era menos de 2 grados, no habia nada que arreglar.");
  }
  Serial.println(" (referencia del delantero: 10.1 sin corregir -> 4.2 con,");
  Serial.println("  pero eso fue a potencia 240 y 1 s; aca vamos a 80 y 5 s,");
  Serial.println("  asi que los numeros no son comparables uno a uno)");
  Serial.println("=========================================================");

  // Y a la memoria, que es lo unico que sobrevive a desenchufar el cable.
  guardarCorrida(true);
  Serial.print(" guardado como corrida "); Serial.print(cab.total);
  Serial.print(". Arranco el giroscopio en "); Serial.print(cab.arrancaron);
  Serial.print(" de "); Serial.print(cab.total); Serial.println(".");
  mostrarHistorial();
}

void loop() {
  leerTeclado();               // i = ver el historial, X = borrarlo

  if (!hayGiroscopo) {
    parar();
    // PARPADEO RAPIDO = no hay giroscopio. Es el mismo aviso que el
    // programa de juego usa desde el 2026-09-01 y el que el equipo ya tiene
    // incorporado: 10 destellos por segundo, el patron mas rapido de todos.
    digitalWrite(LED, ((millis() / MS_PARPADEO_AVISO) % 2) ? HIGH : LOW);
    return;
  }
  // Termino: quieto y a oscuras, para que se note que ya esta.
  parar();
  digitalWrite(LED, LOW);
}
