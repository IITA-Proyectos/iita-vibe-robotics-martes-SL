/* =====================================================================
   MEDIR-ANCHO-SIN-GIRO — lo mismo que medir-ancho, pero si el giroscopio
   no contesta, mide igual sin el
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-21
   =====================================================================

   🆕 QUE CAMBIA RESPECTO DE `medir-ancho` (pedido del equipo, 21/09)
   `medir-ancho` no se mueve sin giroscopio. Este:
     - al arrancar LO LLAMA VARIAS VECES (INTENTOS_GIRO, ~1 s cada uno);
     - si no contesta, o contesta pero no da datos, MIDE IGUAL SIN EL;
     - si se cae a mitad de la medicion, SIGUE SIN EL en vez de frenar.
   Todo lo demas es igual: los tramos, los topes, los controles de que
   los numeros tengan sentido, la EEPROM y el informe por USB.

   ⚠️ SIN GIROSCOPIO EL NUMERO ES MENOS CONFIABLE. Yendo de costado, la
   mezcla 50/50/89 hace girar al robot si nadie lo corrige, y girando
   recorre otro camino. Los controles (cruce ~2 veces la ida, ida y vuelta
   parecidas) atajan los casos groseros. La EEPROM anota si se midio CON o
   SIN giroscopio, y el informe por USB lo dice.

   PARA QUE SIRVE
   En el partido del 21/09 el robot se quedaba quieto cuando no veia la
   pelota, y el juez lo saco un minuto por "robot que no funciona". La idea
   del equipo: que cuando no vea la pelota vaya a un costado y vuelva a la
   mitad. Para eso el robot tiene que saber CUANTO TIEMPO tarda en cruzar.

   Este programa es SOLO LA HERRAMIENTA DE MEDIR. No juega, no usa la
   camara, no busca la pelota. Mide, guarda los numeros y termina. Como se
   usan esos numeros en el partido se decide despues.

   ---------------------------------------------------------------------
   QUE HACE
   ---------------------------------------------------------------------
        1. Espera el giroscopio y cuenta 3 segundos, quieto.
        2. Va de costado a la IZQUIERDA hasta que el sensor de atras
           izquierdo pisa la linea blanca. Frena.
        3. Cruza a la DERECHA hasta la otra linea, CRONOMETRANDO.   -> ida
        4. Cruza de vuelta a la IZQUIERDA, CRONOMETRANDO.           -> vuelta
        5. Va a la MITAD, con la cuenta del medio cruce.
        6. Guarda los dos tiempos en la memoria del Teensy (EEPROM), que
           NO se borra al cortar la bateria.

   Todo de costado, a velocidad media y sosteniendo el rumbo con el
   giroscopio. El paso 5 es la prueba de que la medicion sirve: si el
   robot termina en la mitad, el numero esta bien. Si termina corrido, no.

   Por que ida Y vuelta: el robot puede no ser igual de rapido para los dos
   lados (la rueda trasera empuja distinto que las de adelante). Con los
   dos numeros se ve si son parecidos. Si no lo son, se usa cada uno para
   su lado.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Apoyar el robot en la MITAD, mirando la cancha, a la distancia de la
      linea de atras en la que juega. Los sensores de atras NO pueden
      quedar arriba de una linea: si no, cree que ya llego (y avisa).
   2. Con el robot ya apoyado y QUIETO, prender la bateria sin tocarlo.
      (Es lo que funciono en el partido del 21/09: 5 de 5 veces anduvo el
      giroscopio apoyando el robot antes de prenderlo.)
   3. Mirar. Al terminar tiene que quedar en la mitad.
   Para medir de nuevo: apagar, volver a apoyarlo en la mitad y prender.
   Cada medicion buena reemplaza a la anterior.

   ---------------------------------------------------------------------
   EL IDIOMA DEL LED
   ---------------------------------------------------------------------
        parpadeo rapido          -> cuenta de 3 s, sacale las manos
        LED fijo                 -> midiendo, no lo toques
        latido lento             -> TERMINO BIEN, con giroscopio
        latido DOBLE             -> TERMINO BIEN, pero SIN giroscopio
        destellos que se repiten -> ERROR, NO se guardo nada:
             2 destellos = arranco con un sensor de atras sobre una linea
             4 destellos = no encontro la linea a tiempo (freno de emergencia)
             5 destellos = los numeros no tienen sentido (ver abajo)

   ---------------------------------------------------------------------
   🛡️ LOS NUMEROS SE CONTROLAN ANTES DE GUARDARLOS
   ---------------------------------------------------------------------
   Una medicion mala es peor que ninguna: el partido la usaria entera. Un
   caso que puede pasar: si el robot va un poco torcido, un sensor de atras
   pisa la linea de ATRAS a mitad del cruce y la toma por la del costado.
   Por eso, arrancando del medio:
     - el cruce tiene que durar entre 1,4 y 3 veces lo que tardo del medio
       al primer costado (lo esperable es ~2);
     - la ida y la vuelta no pueden diferir mas de 30%.
   Si no se cumple: 5 destellos, NO guarda, y por USB se ven los numeros
   que dio igual, para entender que paso.

   Y los topes de tiempo salen de lo que ya midio, no de un numero fijo:
   6 s de costado son casi 2 m, mas que la cancha.

   🚨 SI EL USB ESTA ENCHUFADO, PRENDER LA BATERIA NO REINICIA EL TEENSY.
   El Teensy ya esta prendido por el USB: arranco sin bateria (sin
   giroscopio y con los motores sin corriente) y no se entera de que
   llego la bateria. Para medir: DESENCHUFAR el USB y despues prender.

   ---------------------------------------------------------------------
   🚨 LO QUE SE MIDE VALE PARA ESTA VELOCIDAD Y ESTA FORMA DE MOVERSE
   ---------------------------------------------------------------------
   El robot no mide distancias: mide TIEMPO. El numero sirve solo si en el
   partido se mueve exactamente igual: misma VEL_MEDIR, misma rampa de
   arranque, misma mezcla de ruedas y misma correccion de rumbo. Todo eso
   esta copiado de `funciona/seguir-y-despejar`. Si alli se cambia algo de
   eso, hay que volver a medir. Por eso la EEPROM guarda tambien la
   velocidad con la que se midio.

   ---------------------------------------------------------------------
   COMO VER LOS NUMEROS: POR USB
   ---------------------------------------------------------------------
   Cada 2 segundos, y apenas termina de medir, el robot manda por USB lo
   que tiene guardado en la EEPROM: los dos tiempos de cruce, cuanto tiene
   que andar desde cada costado para quedar en la mitad, y en que anda.
   No tiene teclas: solo informa.

   🎯 LA FORMA SEGURA DE LEERLOS: con la BATERIA APAGADA, enchufar el USB y
   abrir `pruebas/herramientas/mirar.bat`. Sin bateria los motores no
   tienen corriente: el robot NO SE MUEVE, "intenta" medir sin giroscopio,
   termina en "no encontro la linea" (que en este caso es lo esperado) y
   no toca lo guardado. Mientras tanto muestra lo guardado.
   🚨 Con la bateria PRENDIDA y el USB puesto, vuelve a medir: se mueve.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


// ---- ARQUERO (ROBOT1) — ruedas medidas en banco el 2026-07-28 ----
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

// ---- sensores de linea, medidos el 2026-08-11 ----
#define LINEA_ATRAS_DER A11
#define LINEA_ATRAS_IZQ A13

// Umbral de blanco de la cancha nueva, medido el 2026-09-15. El mismo que
// usa el programa de juego.
const int UMBRAL_BLANCO = 425;


// ---- la velocidad de la medicion ----
// "Velocidad media", pedido del equipo. El maximo de costado que usa el
// juego es 120.
//
// ⚠️ Con 100, las dos ruedas de adelante reciben 50 (la mezcla de costado
// es 50/50/89) y un motor parado necesita ~70 para arrancar (medido el
// 01/09). Si en vez de deslizarse de costado el robot GIRA o avanza a los
// tirones, este es el primer numero a subir.
const int VEL_MEDIR = 100;

// Mezcla para ir de costado: la del codigo 2025 (ai/adproporcional), la
// misma que usa el programa de juego.
const int LADO_FRENTE  = 50;
const int LADO_TRASERA = 89;


// ---- tiempos ----
const unsigned long MS_CUENTA        = 3000;   // quieto antes de salir
// Freno de emergencia de cada tramo, si no encuentra la linea.
// Del medio al primer costado: fijo, porque todavia no sabe nada.
// Los cruces: a partir de lo que ya midio (ver topeDelTramo()).
const unsigned long MS_MAX_PRIMER_COSTADO = 4000;
const unsigned long MS_MAX_CRUCE          = 6000;   // nunca mas que esto
const unsigned long MS_FRENO         = 200;    // freno electrico sostenido
const unsigned long MS_QUIETO        = 300;    // quieto entre tramos
// Lo que tarda el motor en empezar a mover, medido el 04/08 (avanzando).
const unsigned long MS_RETARDO_ARRANQUE = 33;


// ---- arranque suave: el MISMO que el programa de juego ----
// Arrancando de golpe las ruedas patinan y tuercen al robot (18/08).
const int RAMPA_MOV_PASO = 10;
const unsigned long RAMPA_MOV_MS = 10;
int potenciaRampa = 0;
unsigned long t_rampaMov = 0;


// ---- giroscopio y correccion de rumbo: los MISMOS que el juego ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiroscopo = false;
float rumboBase = -1;

const float KP_RUMBO = 5.0;
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;
float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Una lectura en cero suelta no alcanza para decir que se cayo: puede ser
// un tropiezo del bus. Recien con varias seguidas se da por caido (es lo
// que hace `cuadrado-giroscopo`).
const int CEROS_PARA_CAIDO = 10;
int cerosSeguidos = 0;

// 🆕 Cuantas veces se lo llama al arrancar. Cada intento fallido tarda
// ~1,15 s (la libreria espera ~850 ms + 300 ms de pausa), asi que 8
// intentos son ~9 s de espera antes de decidir medir sin el.
const int INTENTOS_GIRO = 8;

// 🆕 Si se usa el giroscopio en ESTE momento. Arranca en true; se pone en
// false si no aparece, si no da datos, o si se cae andando.
bool usaGiroscopo = true;
// Si lo tuvo durante TODA la medicion. Es lo que se anota en la EEPROM.
bool giroTodaLaMedicion = true;
bool seCayoAndando = false;
const unsigned long MS_ESPERA_DATOS_GIRO = 5000;

// 🎯 ESPERAR A QUE EL GIROSCOPIO SE CALIBRE, QUIETO.
// La hoja de datos de Bosch (BNO055, seccion 3.11.2) dice que el giroscopio
// se calibra dejando el sensor quieto "unos segundos", y que esa
// calibracion se pierde en cada apagado. El chip dice como va, de 0 a 3
// (getCalibration). Se espera el 3 antes de fijar el rumbo a sostener.
// Nadie publica cuanto tarda, asi que hay un tope: si no llega, mide igual.
// (Relacionado: el 21/09, apoyando el robot quieto ANTES de prenderlo, el
// giroscopio anduvo 5 de 5 veces.)
const unsigned long MS_MAX_CALIBRAR_GIRO = 10000;


// ---- lo que se guarda en la EEPROM ----
// Se guarda una "marca" para que el programa que lo lea sepa que ahi hay
// una medicion de verdad y no basura (una EEPROM nueva viene con 0xFF).
struct MedicionAncho {
  uint16_t marca;        // MARCA_ANCHO = hay una medicion valida
  uint16_t velocidad;    // VEL_MEDIR con la que se midio
  uint32_t msIzqADer;    // cruce de la linea izquierda a la derecha
  uint32_t msDerAIzq;    // cruce de la derecha a la izquierda
  uint32_t msMedioAIzq;  // del medio (donde lo apoyaron) a la izquierda
  uint16_t conGiroscopio; // 1 = sostuvo el rumbo con el giroscopio TODA la
                          // medicion; 0 = lo hizo sin giroscopio (o se cayo)
};
// La marca cambia cada vez que cambia esta estructura. Los campos viejos
// quedan siempre en el mismo lugar, asi que las versiones anteriores se
// pueden leer igual:
//   V1 (21/09, antes del partido): sin msMedioAIzq ni conGiroscopio
//   V2 (21/09): sin conGiroscopio (esa version exigia el giroscopio)
//   V3: la de ahora, la escriben medir-ancho y medir-ancho-sin-giro
const uint16_t MARCA_ANCHO    = 0xA2C3;
const uint16_t MARCA_ANCHO_V2 = 0xA2C2;
const uint16_t MARCA_ANCHO_V1 = 0xA2C1;
const int DIR_EEPROM_ANCHO = 0;

MedicionAncho medicion = { 0, 0, 0, 0, 0, 0 };


// ---- estado ----
enum Fase { CUENTA, MOVIENDO, FRENANDO, TERMINADO, ERROR };
Fase fase = CUENTA;
unsigned long t_fase = 0;

// Los cuatro tramos, en orden:
//   0: a la IZQUIERDA hasta la linea      (no se cronometra: solo llegar)
//   1: a la DERECHA hasta la linea        -> msIzqADer
//   2: a la IZQUIERDA hasta la linea      -> msDerAIzq
//   3: a la DERECHA, al medio, por tiempo
const int TRAMOS = 4;
int tramo = 0;
int dirTramo = -1;              // +1 derecha, -1 izquierda
unsigned long msTramo = 0;      // solo para el tramo por tiempo

// No hay error "sin giroscopio" (el 3): este programa mide igual sin el.
enum Error { ERR_NINGUNO = 0, ERR_SOBRE_LINEA = 2,
             ERR_SIN_LINEA = 4, ERR_RARA = 5 };
Error error = ERR_NINGUNO;


// ---------------------------------------------------------------- motores

// SOLTAR los motores (el robot sigue de largo por inercia).
void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// FRENO ELECTRICO: las dos patas en BAJO con el PWM al MAXIMO. Medido el
// 18/08: frena en el acto. (Con el PWM en 0 seria soltar: mismo estado de
// las patas, efecto opuesto.)
void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

// Un valor CON SIGNO por rueda. Positivo = pata A alta.
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

void reiniciarRampa() {
  potenciaRampa = 0;
  t_rampaMov = millis();
}

// Sube de a escalones hasta el objetivo.
int rampa(int objetivo) {
  unsigned long ahora = millis();
  if (objetivo < potenciaRampa) {
    potenciaRampa = objetivo;
  } else if (ahora - t_rampaMov >= RAMPA_MOV_MS) {
    t_rampaMov = ahora;
    potenciaRampa += RAMPA_MOV_PASO;
    if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  }
  return potenciaRampa;
}


// ---------------------------------------------------------------- rumbo

// El rumbo, o -1 si el sensor devolvio ceros (no contesto).
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) {
    if (cerosSeguidos < CEROS_PARA_CAIDO) cerosSeguidos++;
    return -1;
  }
  cerosSeguidos = 0;
  return e.orientation.x;
}

bool giroscopoCaido() { return cerosSeguidos >= CEROS_PARA_CAIDO; }

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

// Cuanto sumarle a las TRES ruedas para que el robot no gire.
// Proporcional + acumulada con tope (medido el 04/08: solo proporcional
// dejaba 7 grados de error fijo).
int correccionDeRumbo() {
  if (!usaGiroscopo) return 0;          // 🆕 sin giroscopio no hay con que
  float r = rumboActual();
  if (rumboBase < 0 || r < 0) return 0;

  float error = diferencia(rumboBase, r);

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;

  if (fabs(error) < ZONA_MUERTA_RUMBO) return 0;

  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float c = error * KP_RUMBO + integralRumbo * KI_RUMBO;
  if (c >  MAX_CORRECCION_RUMBO) c =  MAX_CORRECCION_RUMBO;
  if (c < -MAX_CORRECCION_RUMBO) c = -MAX_CORRECCION_RUMBO;
  return (int)c;
}

// De costado sosteniendo el rumbo. s > 0 = DERECHA (medido el 11/08).
// Lo que hace ir de costado pasa por la rampa; la correccion de rumbo no.
void moverDeCostado(int s) {
  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;
  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;
  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}


// ---------------------------------------------------------------- linea

bool veBlancoIzq() { return analogRead(LINEA_ATRAS_IZQ) >= UMBRAL_BLANCO; }
bool veBlancoDer() { return analogRead(LINEA_ATRAS_DER) >= UMBRAL_BLANCO; }

// ¿Llego al costado? Mira SOLO el sensor de atras del lado hacia el que va:
// la linea del costado la pisa primero ese. Si los DOS ven blanco no
// cuenta: eso es la linea de atras (el robot se fue para atras mientras
// iba de costado), no la del costado.
bool llegoAlCostado(int dir) {
  bool iz = veBlancoIzq();
  bool de = veBlancoDer();
  return (dir > 0) ? (de && !iz) : (iz && !de);
}


// ---------------------------------------------------------------- cuentas

// Cuanto tiempo andar desde un costado para quedar en la mitad.
//
// NO es la mitad justa del cruce. Cada tramo arranca desde quieto, y el
// arranque rinde menos distancia que el resto: la rampa sube de a poco (en
// promedio va a media potencia) y el motor tarda en empezar a mover. El
// cruce entero tiene UN arranque, y el medio cruce tambien tiene UNO. Si
// se partiera el cruce al medio, al medio cruce le faltaria medio arranque.
// Es la leccion del empujoncito del 18/08: la regla de tres no alcanza
// cuando el arranque pesa.
//
//     perdida  = (tiempo de la rampa) / 2  +  retardo del motor
//     cruce    = perdida + ancho / velocidad
//     al medio = perdida + (ancho / 2) / velocidad
//              = cruce / 2 + perdida / 2
unsigned long msHastaElMedio(unsigned long msCruce, int velocidad) {
  unsigned long msRampa = ((unsigned long)velocidad / RAMPA_MOV_PASO)
                          * RAMPA_MOV_MS;
  unsigned long perdida = msRampa / 2 + MS_RETARDO_ARRANQUE;
  return msCruce / 2 + perdida / 2;
}


// ---------------------------------------------------------------- USB

// En que anda, en palabras. `buscandoGiro` es para el arranque, cuando
// todavia no llego al loop().
int intentoGiro = 0;
bool buscandoGiro = true;

void escribirEstado() {
  Serial.print("ahora: ");
  if (buscandoGiro) {
    Serial.print("arrancando, llamando al giroscopio (intento ");
    Serial.print(intentoGiro); Serial.print(" de "); Serial.print(INTENTOS_GIRO);
    Serial.println(")");
    return;
  }
  Serial.print("giroscopio: ");
  if (usaGiroscopo)       Serial.println("SI, contesta");
  else if (seCayoAndando) Serial.println("SE CAYO a mitad de la medicion: sigue sin el");
  else                    Serial.println("NO contesto: mide SIN giroscopio");
  Serial.print("ahora: ");
  switch (fase) {
    case CUENTA:
      Serial.println(usaGiroscopo
                     ? "cuenta de arranque / esperando que el giroscopio se calibre"
                     : "cuenta de arranque");
      break;
    case MOVIENDO:
    case FRENANDO:
      Serial.print("MIDIENDO, tramo "); Serial.print(tramo + 1);
      Serial.println(" de 4"); break;
    case TERMINADO: Serial.println("TERMINO BIEN. La medicion de arriba es la que acaba de hacer."); break;
    case ERROR:
      Serial.print("ERROR, no se guardo nada: ");
      if (error == ERR_SOBRE_LINEA) {
        Serial.println("arranco con un sensor de atras sobre una linea.");
      } else if (error == ERR_SIN_LINEA) {
        Serial.print("no encontro la linea a tiempo, en el tramo ");
        Serial.print(tramo + 1); Serial.println(" (freno de emergencia).");
        Serial.println("       (si la bateria esta APAGADA es lo esperado: los");
        Serial.println("        motores no tienen corriente y el robot no se movio)");
      } else {
        Serial.println("los numeros no tienen sentido. Los que dio:");
        Serial.print("       medio -> izquierda:     "); Serial.print(medicion.msMedioAIzq);
        Serial.println(" ms");
        Serial.print("       izquierda -> derecha:   "); Serial.print(medicion.msIzqADer);
        Serial.println(" ms   (tendria que ser ~2 veces el de arriba)");
        if (tramo == 2) {
          Serial.print("       derecha -> izquierda:   "); Serial.print(medicion.msDerAIzq);
          Serial.println(" ms   (tendria que ser parecido al de arriba)");
        }
        Serial.println("       ¿Estaba apoyado en el medio? ¿Iba torcido?");
      }
      break;
  }
}

void informe() {
  MedicionAncho g;
  EEPROM.get(DIR_EEPROM_ANCHO, g);

  Serial.println();
  Serial.println("=========== MEDIR-ANCHO-SIN-GIRO ===========");
  if (g.marca != MARCA_ANCHO && g.marca != MARCA_ANCHO_V2
      && g.marca != MARCA_ANCHO_V1) {
    Serial.println("en la memoria: NO hay ninguna medicion guardada");
  } else {
    Serial.println("MEDICION GUARDADA:");
    if (g.marca == MARCA_ANCHO_V1) {
      Serial.println("  (de la primera version: sin control de numeros ni ida del medio)");
    } else {
      Serial.print("  del medio a la izquierda:   "); Serial.print(g.msMedioAIzq);
      Serial.println(" ms");
    }
    // Las V1 y V2 exigian el giroscopio: si guardaron, fue con giroscopio.
    bool conGiro = (g.marca != MARCA_ANCHO) || g.conGiroscopio;
    Serial.println(conGiro ? "  medido CON giroscopio"
                           : "  medido SIN giroscopio (menos confiable: pudo ir girando)");
    Serial.print("  cruce izquierda -> derecha: "); Serial.print(g.msIzqADer);
    Serial.println(" ms");
    Serial.print("  cruce derecha -> izquierda: "); Serial.print(g.msDerAIzq);
    Serial.println(" ms");

    // ¿Es igual de rapido para los dos lados?
    unsigned long mayor = max(g.msIzqADer, g.msDerAIzq);
    unsigned long menor = min(g.msIzqADer, g.msDerAIzq);
    unsigned long dif = mayor - menor;
    Serial.print("  diferencia: "); Serial.print(dif); Serial.print(" ms (");
    Serial.print(mayor ? (100 * dif) / mayor : 0);
    Serial.println("% del mas lento)");

    Serial.print("  al medio desde la IZQUIERDA (yendo a la derecha): ");
    Serial.print(msHastaElMedio(g.msIzqADer, g.velocidad)); Serial.println(" ms");
    Serial.print("  al medio desde la DERECHA (yendo a la izquierda): ");
    Serial.print(msHastaElMedio(g.msDerAIzq, g.velocidad)); Serial.println(" ms");
    Serial.print("  medido a velocidad "); Serial.print(g.velocidad);
    if (g.velocidad != VEL_MEDIR) {
      Serial.print("  <-- OJO: este programa usa "); Serial.print(VEL_MEDIR);
    }
    Serial.println();
  }
  escribirEstado();
  Serial.println("===========================================");
}

const unsigned long MS_ENTRE_INFORMES = 2000;
unsigned long t_informe = 0;

// Informa cada 2 s. Se llama desde el loop() y tambien desde las esperas
// del arranque, que pueden tardar varios segundos.
void informarSiToca() {
  if (millis() - t_informe < MS_ENTRE_INFORMES) return;
  t_informe = millis();
  informe();
}


// ---------------------------------------------------------------- control

// Tope de tiempo del tramo que esta por hacer.
//   0 (medio -> izquierda): fijo.
//   1 (cruce): arrancando del medio, un cruce dura ~2 veces la ida al
//     primer costado. 3 veces + medio segundo ya es "se paso".
//   2 (vuelta): ~lo mismo que la ida. 30% mas + 300 ms ya es "se paso".
unsigned long topeDelTramo() {
  unsigned long tope = MS_MAX_CRUCE;
  if (tramo == 0) tope = MS_MAX_PRIMER_COSTADO;
  if (tramo == 1) tope = medicion.msMedioAIzq * 3 + 500;
  if (tramo == 2) tope = medicion.msIzqADer * 13 / 10 + 300;
  if (tope > MS_MAX_CRUCE) tope = MS_MAX_CRUCE;
  return tope;
}

// ¿Los numeros tienen sentido? Se llama al terminar el cruce (tramo 1) y
// la vuelta (tramo 2).
bool numerosCreibles() {
  if (tramo == 1) {
    // cruce / ida entre 1,4 y 3
    unsigned long c = medicion.msIzqADer, i = medicion.msMedioAIzq;
    return c * 10 >= i * 14 && c * 10 <= i * 30;
  }
  // tramo 2: vuelta / ida entre 0,7 y 1,43 (hasta 30% de diferencia)
  unsigned long v = medicion.msDerAIzq, c = medicion.msIzqADer;
  return v * 10 >= c * 7 && v * 7 <= c * 10;
}


// ---------------------------------------------------------------- pasos

void empezarTramo(int n) {
  tramo = n;
  switch (n) {
    case 0: dirTramo = -1; break;
    case 1: dirTramo = +1; break;
    case 2: dirTramo = -1; break;
    default:                // 3: al medio, yendo a la derecha como en el 1
      dirTramo = +1;
      msTramo = msHastaElMedio(medicion.msIzqADer, VEL_MEDIR);
      break;
  }
  reiniciarRampa();
  reiniciarCorreccion();
  fase = MOVIENDO; t_fase = millis();
}

bool tramoHastaLinea() { return tramo < 3; }

// Un rumbo, insistiendo unas veces: una lectura en cero suelta puede ser un
// tropiezo del bus, y no alcanza para dar al giroscopio por muerto.
float rumboInsistiendo() {
  float r = rumboActual();
  for (int k = 0; k < 5 && r < 0; k++) {
    delay(20);
    r = rumboActual();
  }
  return r;
}

// 🆕 Deja de usar el giroscopio y sigue sin el.
void seguirSinGiroscopo() {
  usaGiroscopo = false;
  giroTodaLaMedicion = false;
  rumboBase = -1;
}

void fallar(Error e) {
  frenar();
  error = e;
  fase = ERROR; t_fase = millis();
}

void guardar() {
  medicion.marca = MARCA_ANCHO;
  medicion.velocidad = VEL_MEDIR;
  medicion.conGiroscopio = giroTodaLaMedicion ? 1 : 0;
  EEPROM.put(DIR_EEPROM_ANCHO, medicion);
}


// ---------------------------------------------------------------- LED

// `veces` destellos y una pausa, repetido: se puede contar de lejos.
void destellos(int veces, unsigned long ahora) {
  unsigned long ciclo = ahora % ((unsigned long)veces * 400 + 1200);
  if (ciclo < (unsigned long)veces * 400) {
    digitalWrite(LED, ((ciclo % 400) < 200) ? HIGH : LOW);
  } else {
    digitalWrite(LED, LOW);
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
  parar();

  Serial.begin(19200);            // USB: solo para informar
  t_informe = millis() - MS_ENTRE_INFORMES;

  // 🆕 Se lo llama varias veces. Si no contesta, se mide SIN el.
  while (!hayGiroscopo && intentoGiro < INTENTOS_GIRO) {
    intentoGiro++;
    informarSiToca();
    digitalWrite(LED, (intentoGiro % 2) ? HIGH : LOW);   // da señales de vida
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }

  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);

    // Saludar no alcanza: hay que esperar a que DE UN DATO.
    unsigned long t0 = millis();
    float r = rumboActual();
    while (r < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) {
      informarSiToca();
      delay(50);
      r = rumboActual();
    }
    if (r < 0) seguirSinGiroscopo();               // saluda pero no da datos
  } else {
    seguirSinGiroscopo();                          // no contesto
  }
  buscandoGiro = false;

  fase = CUENTA; t_fase = millis();
}


void loop() {
  unsigned long ahora = millis();
  informarSiToca();

  switch (fase) {

    case CUENTA:
      digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_CUENTA) {
        // Quieto hasta que el giroscopio este calibrado (o hasta el tope).
        if (usaGiroscopo) {
          uint8_t sis = 0, giro = 0, acel = 0, mag = 0;
          bno.getCalibration(&sis, &giro, &acel, &mag);
          if (giro < 3 && ahora - t_fase < MS_CUENTA + MS_MAX_CALIBRAR_GIRO) break;
        }

        // Si ya esta sobre una linea, "llegar al costado" no significaria
        // nada: mejor avisar que medir mal.
        if (veBlancoIzq() || veBlancoDer()) { fallar(ERR_SOBRE_LINEA); break; }
        // El rumbo a sostener se fija aca, con el robot quieto.
        if (usaGiroscopo) {
          float r = rumboInsistiendo();
          if (r < 0) seguirSinGiroscopo();     // 🆕 se cayo justo ahora
          else       rumboBase = r;
        }
        empezarTramo(0);
      }
      break;

    case MOVIENDO: {
      digitalWrite(LED, HIGH);
      moverDeCostado(dirTramo * VEL_MEDIR);   // tambien lee el giroscopio

      // 🆕 Si se cae andando, NO frena: sigue sin giroscopio.
      if (usaGiroscopo && giroscopoCaido()) {
        seCayoAndando = true;
        seguirSinGiroscopo();
      }

      unsigned long ms = ahora - t_fase;
      if (tramoHastaLinea()) {
        if (llegoAlCostado(dirTramo)) {
          frenar();                           // frenar, no soltar
          if (tramo == 0) medicion.msMedioAIzq = ms;
          if (tramo == 1) medicion.msIzqADer = ms;   // 🎯 las mediciones
          if (tramo == 2) medicion.msDerAIzq = ms;
          if (tramo >= 1 && !numerosCreibles()) { fallar(ERR_RARA); break; }
          fase = FRENANDO; t_fase = ahora;
        } else if (ms >= topeDelTramo()) {
          fallar(ERR_SIN_LINEA);
        }
      } else if (ms >= msTramo) {
        frenar();
        fase = FRENANDO; t_fase = ahora;
      }
      break;
    }

    case FRENANDO:
      // Freno sostenido un ratito, despues soltar y quedarse quieto antes
      // del tramo siguiente: asi cada tramo arranca desde quieto, igual
      // que se cronometro.
      if (ahora - t_fase < MS_FRENO) { frenar(); break; }
      parar();
      if (ahora - t_fase < MS_FRENO + MS_QUIETO) break;
      if (tramo + 1 < TRAMOS) {
        empezarTramo(tramo + 1);
      } else {
        guardar();                             // recien aca, todo salio bien
        fase = TERMINADO; t_fase = ahora;
        informe();                             // y avisa enseguida
      }
      break;

    case TERMINADO: {
      parar();
      // Latido lento: un destello corto cada 2 segundos.
      // 🆕 Latido DOBLE si midio (todo o en parte) sin giroscopio.
      unsigned long tl = ahora % 2000;
      bool on = (tl < 120);
      if (!giroTodaLaMedicion) on = on || (tl >= 300 && tl < 420);
      digitalWrite(LED, on ? HIGH : LOW);
      break;
    }

    case ERROR:
      if (ahora - t_fase < MS_FRENO) frenar();
      else parar();
      destellos((int)error, ahora);
      break;
  }
}
