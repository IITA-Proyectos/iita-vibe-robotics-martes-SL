/* =====================================================================
   CALIBRACION BUSQUEDA — buscar la pelota y llegar, y nada mas
   IITA Salta — delantero, Roboliga 2026 — 2026-09-29
   =====================================================================

   NO PATEA. NO ORBITA. NO ESCAPA DE LA LINEA. Busca la pelota, la centra,
   avanza hasta ella y SE DETIENE. Nada mas. Todo lo demas del firmware
   esta sacado a proposito, para que no ensucie la medicion.

   ---------------------------------------------------------------------
   LA PREGUNTA
   ---------------------------------------------------------------------
   El 29/09 se cambio la pausa de rotarPulsado() para que FRENE en vez de
   SOLTAR las ruedas. La idea era que el robot dejara de pasarse de la
   pelota al encontrarla. El equipo lo probo y dijo que "no resulta mucho",
   y sospecha que la causa real es otra: LA PELOTA ES MAS BLANCA que la de
   siempre, asi que la camara la ve peor.

   Este programa corre las DOS versiones, una despues de la otra, y las
   compara con numeros en vez de con impresiones:

     RONDA A — la pausa FRENA        (como quedo el firmware hoy)
     RONDA B — la pausa SUELTA       (como estaba antes)

   ---------------------------------------------------------------------
   QUE MIDE, Y POR QUE CADA COSA
   ---------------------------------------------------------------------
     1. CUANTO TARDA EN VERLA          -> si frenar la hace mas lenta
     2. GRADOS POR SEGUNDO de barrido  -> el numero objetivo de "¿es mas
                                          rapido?". Lo da el giroscopio
     3. CUANTAS VECES LA PERDIO        -> el sintoma exacto que se reporto:
                                          "la detecta, se pasa, y la busca
                                          de vuelta". Cada vuelta a buscar
                                          cuenta una
     4. CUANTO TARDA EN LLEGAR         -> el total, de arrancar a estar
                                          encima de la pelota
     5. EN QUE % DE LOS CUADROS LA VE  -> ESTA ES LA DE LA PELOTA BLANCA.
                                          La camara manda 46 cuadros por
                                          segundo; si la pelota se ve mal,
                                          este numero va a estar bajo en
                                          LAS DOS rondas por igual, y ahi
                                          el problema no es el freno: es
                                          el color

   Si A y B dan parecido en 1-4 pero el 5 esta bajo, la hipotesis de la
   pelota gana y lo que hay que calibrar es la CAMARA, no los motores.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   🚨 EN EL PISO O EN LA CANCHA, NO EN LA MESA: el robot se desplaza.

   1. Cargalo. Apaga la bateria mientras cargas.
   2. Desenchufa el USB, llevalo al piso y PRENDE LA BATERIA.
   3. Cada ronda avisa con el LED y arranca sola:
        - 8 segundos para poner la pelota LEJOS y sacar la mano
        - el robot gira, la busca, la centra, se le acerca y PARA
        - si en 30 segundos no llega, esa ronda se da por perdida
   4. Entre las dos rondas hay otros 8 segundos para volver a poner la
      pelota en el mismo lugar. Poneia PARECIDO a la vez anterior: si no,
      la comparacion no vale.
   5. Volve, enchufa el USB SIN APAGAR LA BATERIA, y el informe se
      reimprime cada 3 segundos.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// ---- pines, MEDIDOS en banco el 2026-07-28 (robot DELANTERO) ----
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6
#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4
#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3
#define LED     13

// ---- los MISMOS numeros que el firmware. Si alla cambian, aca tambien ----
const bool GIRO_INVERTIDO = true;
const int  VEL_GIRO        = 95;
const int  MS_PULSO_BUSC   = 60;
// 2026-09-29: 380 -> 200 en las DOS rondas, a pedido del equipo.
// La primera corrida dio que frenar barre 2,6 veces mas lento
// (48 contra 125 grados/s) pero llega antes igual (10,2 contra
// 17,5 s). Bajar la pausa es la forma de recuperar barrido SIN
// subir la velocidad: mas pulsos por segundo, y cada uno termina
// donde debe. Subir VEL_GIRO seria al reves — mas inercia.
// La camara manda 46 cuadros/s, asi que en 200 ms alcanza a ver
// unos 9: sigue siendo tiempo de sobra para decidir.
const int  MS_ESPERA_BUSC  = 200;   // era 380
const int  VEL_CENT        = 78;
const int  MS_PULSO_CENT   = 32;
const int  MS_ESPERA_CENT  = 320;
const int  VEL_AVANCE      = 95;
const int  VEL_AVANCE_CERCA = 70;
const int  XP_FRENAR       = 70;
const int  XP_ORBITA       = 34;
const int  XP_MAX          = 150;
// ---- CENTRAR POR ANGULO, NO POR Yp CRUDO (29/09) ----
// El firmware compara abs(Yp) contra TOL_ENTRA=10 / TOL_SALE=5. Pero Yp es
// "cuanto esta corrida para el costado" en unidades de camara, y el ANGULO
// es atan2(Yp, Xp): el MISMO Yp significa un angulo muy distinto segun la
// distancia.
//
//     pelota LEJOS  (Xp=140):  Yp=10  ->   4 grados
//     media cancha  (Xp= 70):  Yp=10  ->   8 grados
//     CERCA         (Xp= 40):  Yp=10  ->  14 grados
//
// O sea que el robot se exige alinearse a 4 grados cuando esta lejos y le
// alcanza con 14 cuando esta cerca: TRES VECES Y MEDIA mas precision justo
// cuando mas le cuesta. Reportado por el equipo como "cuando la pelota esta
// lejos le cuesta centrarse y avanzar".
//
// Es EL MISMO ERROR del 11/08, cuando la patada comparaba centimetros
// medidos a 17 cm con centimetros medidos a 100. Alla se arreglo pasando a
// ANGULOS (TOL_ANG_PELOTA = 8 grados) — pero el centrado quedo con el
// metodo viejo. El arreglo se aplico donde dolia y no en el resto.
//
// Los valores salen de lo que hoy exige CERCA, que es donde el robot
// efectivamente se destraba: Yp=10 a Xp=40 son ~14 grados, Yp=5 son ~7.
const float TOL_ANG_ENTRA  = 12.0;   // mas torcida que esto -> a centrar
const float TOL_ANG_SALE   =  6.0;   // mas derecha que esto -> a avanzar
const unsigned long MS_GRACIA = 300;

// ---- de la prueba ----
// ---- RONDA B: GIRO CONTINUO CON RAMPA (propuesta del equipo, 29/09) ----
// En vez de pulsos, gira SIN PARAR hasta ver la pelota. La rampa evita el
// tiron del arranque (que hace patinar y torcer, como en la patada).
// Cuando la ve: FRENA, y recien despues centra y avanza.
//
// LA DUDA QUE RESUELVE ESTA PRUEBA: girando, la camara la ve en 45% de los
// cuadros; quieto, en 98%. Girando continuo la camara NUNCA esta quieta,
// asi que puede barrer rapidisimo y pasarle por al lado sin verla. El
// numero que lo dice es "grados hasta verla": si da mucho mas de 360, la
// esta salteando.
// La ronda B gira MAS LENTO a proposito (70 contra los 95 de la A). No es
// para que sea justa la comparacion — no lo es, y esta bien que no lo sea:
// girando continuo la camara nunca esta quieta, asi que la velocidad es
// justamente lo que hay que bajar para que llegue a ver la pelota. Si aun
// asi necesita mas de una vuelta para encontrarla, el problema no es la
// velocidad: es que hay que PARAR para ver. [pedido del equipo, 29/09]
const int           VEL_GIRO_CONTINUO = 50;  // 70 -> 50. La A usa VEL_GIRO = 95
// ⚠ OJO CON 50: el piso de PWM desde quieto es ~70 en la cancha vieja, y
// sobre la TELA nunca se midio (pruebas/piso-de-pwm sigue sin correrse).
// Si el robot zumba y NO GIRA, es esto: quedo por debajo del piso. La
// rampa ayuda a despegar, pero una vez rodando tiene que alcanzarle para
// seguir. Si no gira, subir de a 5.
const int           RAMPA_GIRO_PASO = 10;    // cuanto sube el PWM por escalon
const unsigned long RAMPA_GIRO_MS   = 10;    // cada cuanto sube
const unsigned long MS_FRENO_AL_VER = 200;   // cuanto frena al verla

// ---- DESTRABE: si no logra centrarse, acercarse igual (29/09) ----
// Propuesta del equipo: "si por 5 segundos no centra la pelota, que se
// frene, avance un poco (tipo 20 cm) y busque de vuelta; cuando esta mas
// cerca la centra mas rapido y facil".
//
// Es una red de seguridad. Con el centrado por ANGULO el problema de fondo
// deberia desaparecer, pero si igual se traba (la pelota contra una pared,
// una rueda que patina, el arco tapandola) esto lo saca del pozo en vez de
// dejarlo zapateando hasta que se acabe el tiempo.
//
// ⚠ LOS 20 CM SON UNA ESTIMACION, NO UNA MEDICION. No sabemos cuantos cm
// por segundo hace el robot: el piso de PWM sobre la tela nunca se midio
// (pruebas/piso-de-pwm sigue sin correrse desde julio) y la escala de la
// camara tampoco (pruebas/tabla-camara). De las corridas de hoy sale algo
// asi como 50 cm/s, y de ahi los 400 ms. Si se ve que avanza mucho mas o
// mucho menos que 20 cm, este es el numero a tocar.
// UNA SOLA REGLA: 5 segundos sin lograr centrarse -> avanza 1 segundo.
// Y si despues de avanzar sigue sin poder, otros 5 segundos y avanza de
// nuevo. Siempre igual.
//
// (Habia puesto que el reintento fuera al segundo en vez de a los cinco,
//  por mi cuenta. El equipo lo marco: el "1 segundo" era cuanto AVANZA, no
//  cuanto espera. Una regla sola es mas facil de predecir mirando el robot,
//  y avanza menos de gusto.)
const unsigned long MS_LIMITE_CENTRAR   = 5000;  // sin lograr centrarse, siempre
const unsigned long MS_AVANCE_DESTRABE  = 1000;  // cuanto avanza cada vez
// El destrabe empuja MAS FUERTE que el avance normal (100 contra los 70 de
// VEL_AVANCE_CERCA). Es a proposito: no es un acercamiento fino, es sacarlo
// del pozo. Si va lento tarda mas en cambiar de perspectiva y se come otros
// 5 segundos antes de volver a intentar. [pedido del equipo, 29/09]
const int VEL_DESTRABE = 150;
const unsigned long MS_FRENO_DESTRABE   = 250;   // frena antes y despues de avanzar
// Cuanto tiene que acercarse (EN CENTIMETROS: Xp de la camara son cm) para
// que el reloj del destrabe vuelva a cero. Si en MS_LIMITE_CENTRAR no logro
// recortar esto, esta trabado aunque se este moviendo. [29/09]
const int MEJORA_DESTRABE = 10;

// Tope de destrabes seguidos. Sin esto, una pelota que quedo de costado
// haria que el robot avance, no la centre, avance de nuevo... y se cruce
// la cancha entera de a un segundo por vez. Con 5 son como maximo 5
// segundos de avance extra, y despues sigue intentando centrar sin moverse.
const int MAX_DESTRABES = 5;

// ---- QUE RONDAS CORRER ----
// 2026-09-29: la ronda A (pulsado) queda APAGADA a pedido del equipo. La B
// (giro continuo con rampa) gano las dos comparaciones — 3,5 s contra 5,7 —
// y hasta al profe le gusto como se mueve, asi que de aca en mas se ajusta
// B sola y no hay que esperar a que corra A cada vez.
//
// Para volver a compararlas, poner HACER_RONDA_A en true. La ronda A sigue
// entera en el codigo: no se borro nada.
const bool HACER_RONDA_A = false;

const unsigned long MS_ANTES   = 8000;    // para poner la pelota y salir
const unsigned long MS_LIMITE  = 30000;   // si no llega, ronda perdida

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiro = false;

// ---- camara ----
int Xp = 0, Yp = 0;
int XpBueno = 0, YpBueno = 0;
unsigned long t_ultimaPelota = 0;
long nCuadros = 0, nCuadrosConPelota = 0;

// ---- la medida HONESTA de si la camara ve la pelota ----
// La primera version conto los cuadros de TODA la ronda, y estaba mal: en
// la busqueda la pelota casi nunca esta a la vista, asi que el porcentaje
// dependia de cuanto habia durado cada fase, no de que tan bien se ve la
// pelota. Mezclaba deteccion con duracion. (Mismo error que el 11/08 con
// los centimetros medidos a distancias distintas.)
//
// Estos cuentan SOLO mientras se acerca, o sea con la pelota adelante.
// Ahi si: deberia verse en casi el 100% de los cuadros. Si da mucho menos,
// la camara la ve mal — y eso no lo arregla ningun freno.
bool midiendoAcerc = false;
long nCuadrosAcerc = 0, nConPelotaAcerc = 0;

// ---- resultado de cada ronda ----
struct Ronda {
  bool  llego;
  long  msHastaVerla;
  long  msTotal;
  float gradosBarridos;
  long  msBuscando;
  int   perdidas;
  long  cuadros;
  long  cuadrosConPelota;
  long  cuadrosAcerc;
  long  conPelotaAcerc;
  int   destrabes;        // cuantas veces hubo que sacarlo del pozo
};
Ronda rA, rB;
bool hechoA = false, hechoB = false;

// =====================================================================
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// Freno electrico: mismas patas que parar(), PERO CON EL PWM AL MAXIMO.
// Con el PWM en 0 el driver apaga la salida y la rueda queda suelta; con
// el PWM en 255 queda cortocircuitada. Mismo estado de patas, efecto
// opuesto. No confundirlas al leer.
void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

void motoresRotando(bool sentidoA, int vel) {
  int a = sentidoA ? 1 : 0, b = sentidoA ? 0 : 1;
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, a); digitalWrite(TRA_INB, b);
}

void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// LA UNICA DIFERENCIA ENTRE LAS DOS RONDAS esta aca: que hace en la pausa.
unsigned long t_cicloPulso = 0;
void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera, bool pausaFrena) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso)                        motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera))      { if (pausaFrena) frenar(); else parar(); }
  else                                                      t_cicloPulso = millis();
}

int velAcercarse(int xp) {
  if (xp >= XP_FRENAR) return VEL_AVANCE;
  if (xp <= XP_ORBITA) return VEL_AVANCE_CERCA;
  long sobra = (long)(xp - XP_ORBITA) * (VEL_AVANCE - VEL_AVANCE_CERCA);
  return VEL_AVANCE_CERCA + (int)(sobra / (XP_FRENAR - XP_ORBITA));
}

void leerCamara() {
  while (Serial1.available() >= 9) {
    int h1 = Serial1.read();
    if (h1 != 201) continue;
    int xp = Serial1.read(), yp = Serial1.read();
    int h2 = Serial1.read();
    Serial1.read(); Serial1.read();            // arco amarillo: no se usa aca
    int h3 = Serial1.read();
    Serial1.read(); Serial1.read();            // arco azul: tampoco
    if (h2 == 202 && h3 == 203) {
      Xp = xp;  Yp = yp - 100;
      nCuadros++;
      if (midiendoAcerc) nCuadrosAcerc++;
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
        nCuadrosConPelota++;
        if (midiendoAcerc) nConPelotaAcerc++;
      }
    }
  }
}

// Angulo al que esta un objeto, en grados. Positivo o negativo segun el
// lado. Si X<=0 no hay dato y no se inventa un angulo.
float anguloDe(int X, int Y) {
  if (X <= 0) return 0.0;
  return atan2((float)Y, (float)X) * 180.0 / PI;
}

float rumbo() {
  if (!hayGiro) return 0;
  sensors_event_t e;
  bno.getEvent(&e);
  return e.orientation.x;
}

float dif(float a, float b) {
  float d = a - b;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

void cuenta(const char *que, unsigned long ms) {
  Serial.println();
  Serial.print(">>> "); Serial.println(que);
  unsigned long t0 = millis();
  int ultimo = -1;
  while (millis() - t0 < ms) {
    leerCamara();
    int faltan = (int)((ms - (millis() - t0)) / 1000) + 1;
    if (faltan != ultimo) {
      ultimo = faltan;
      Serial.print("    "); Serial.print(faltan); Serial.println("...");
    }
    digitalWrite(LED, (millis() / 150) % 2);
  }
  digitalWrite(LED, HIGH);
}

// =====================================================================
// UNA RONDA: busca la pelota, la centra, se le acerca y PARA.
// =====================================================================
// modo 0 = PULSADO con la pausa frenando (el que gano las dos corridas)
// modo 1 = CONTINUO con rampa, y freno al ver la pelota
Ronda correr(int modo) {
  bool pausaFrena = true;
  bool continuo   = (modo == 1);
  int  pwmGiro    = 0;
  unsigned long t_rampaGiro = millis();
  Ronda r;
  r.llego = false; r.msHastaVerla = -1; r.msTotal = -1;
  r.gradosBarridos = 0; r.msBuscando = 0; r.perdidas = 0; r.destrabes = 0;

  nCuadros = 0; nCuadrosConPelota = 0;
  midiendoAcerc = false; nCuadrosAcerc = 0; nConPelotaAcerc = 0;
  XpBueno = 0; YpBueno = 0; t_ultimaPelota = 0;
  t_cicloPulso = millis();

  unsigned long t0 = millis();
  bool laViAlgunaVez = false;
  bool estabaBuscando = true;

  unsigned long t_centrandoDesde = 0;
  int  mejorXp = 0;            // lo mas cerca que estuvo en este intento
  bool centrando = false;      // <- LA HISTERESIS. Ver abajo.
  float rumboPrevio = rumbo();
  unsigned long tBuscarDesde = millis();

  while (millis() - t0 < MS_LIMITE) {
    leerCamara();
    bool laVeo = (millis() - t_ultimaPelota) < MS_GRACIA;

    if (!laVeo) {
      // ---- BUSCANDO ----
      if (!estabaBuscando) {
        estabaBuscando = true;
        tBuscarDesde = millis();
        if (laViAlgunaVez) r.perdidas++;   // la tenia y la perdio: EL SINTOMA
        // La perdio: el reloj del destrabe vuelve a cero. El destrabe es
        // para "la veo y no consigo centrarla", no para "la perdi": si no,
        // el tiempo de volver a buscarla contaria como tiempo centrando.
        t_centrandoDesde = 0;
        centrando = false;
        // Y dejan de contar los cuadros de "acercandose": mientras busca,
        // la pelota NO esta adelante, asi que contarlos hunde el porcentaje
        // y hace parecer que la camara ve mal. (Tercera vez que esta
        // metrica me sale confundida. Ahora si mide solo lo que dice medir.)
        midiendoAcerc = false;
      }
      if (hayGiro) {
        float ahora = rumbo();
        r.gradosBarridos += fabs(dif(ahora, rumboPrevio));
        rumboPrevio = ahora;
      }
      if (continuo) {
        // rampa hasta VEL_GIRO, y de ahi sin parar
        if (millis() - t_rampaGiro >= RAMPA_GIRO_MS) {
          t_rampaGiro = millis();
          pwmGiro += RAMPA_GIRO_PASO;
          if (pwmGiro > VEL_GIRO_CONTINUO) pwmGiro = VEL_GIRO_CONTINUO;
        }
        motoresRotando(!GIRO_INVERTIDO, pwmGiro);
      } else {
        rotarPulsado(!GIRO_INVERTIDO, VEL_GIRO, MS_PULSO_BUSC, MS_ESPERA_BUSC, pausaFrena);
      }
    } else {
      // ---- LA VE ----
      if (estabaBuscando) {
        estabaBuscando = false;
        r.msBuscando += (long)(millis() - tBuscarDesde);
        if (!laViAlgunaVez) {
          laViAlgunaVez = true;
          r.msHastaVerla = (long)(millis() - t0);
        }
        midiendoAcerc = true;       // la ve: desde aca cuentan los cuadros
        if (continuo) {
          // Venia girando a fondo: hay que sacarle el envion antes de
          // centrar, o se pasa de largo y la pierde.
          frenar();
          delay(MS_FRENO_AL_VER);
          pwmGiro = 0;              // la proxima busqueda arranca de cero
          leerCamara();
        }
        rumboPrevio = rumbo();
      }

      if (XpBueno < XP_ORBITA) {           // LLEGO: aca se detiene y listo
        frenar();
        delay(300);
        parar();
        r.llego = true;
        r.msTotal = (long)(millis() - t0);
        break;
      }

      // EL RELOJ DEL DESTRABE mide "la veo y no me acerco", NO "estoy girando".
      // Arranca en cuanto la ve, y solo vuelve a cero cuando de verdad se
      // acerco (MEJORA_DESTRABE cm) o cuando la perdio.
      //
      // Antes lo reiniciaba cada vez que el robot quedaba alineado. Con la
      // histeresis eso pasa a cada rato (gira, baja de TOL_ANG_SALE, avanza,
      // se tuerce, vuelve a girar), asi que el reloj nunca llegaba a los 5 s
      // y el destrabe casi no saltaba: en la corrida del 29/09 salto 0 veces
      // mientras perdia la pelota 11. Estaba midiendo lo que no era.
      if (t_centrandoDesde == 0) { t_centrandoDesde = millis(); mejorXp = XpBueno; }
      if (XpBueno < mejorXp - MEJORA_DESTRABE) {   // se acerco de verdad
        mejorXp = XpBueno;
        t_centrandoDesde = millis();               // progresa: se le da mas tiempo
      }

      // DESTRABE: lleva MS_LIMITE_CENTRAR viendola sin acercarse. Frena, se
      // acerca a lo bruto y reintenta desde mas cerca, que es donde le sale
      // facil. Va AFUERA del if de centrar, porque la falla de verdad es
      // oscilar entre girar y avanzar, no quedarse girando.
      if (r.destrabes < MAX_DESTRABES
          && millis() - t_centrandoDesde > MS_LIMITE_CENTRAR) {
        r.destrabes++;
        frenar();
        delay(MS_FRENO_DESTRABE);
        avanzar(VEL_DESTRABE);
        unsigned long tA = millis();
        while (millis() - tA < MS_AVANCE_DESTRABE) leerCamara();
        frenar();
        delay(MS_FRENO_DESTRABE);
        t_centrandoDesde = 0;        // el reloj y mejorXp se recargan al verla
        centrando = false;
        leerCamara();
        continue;
      }

      // EL CENTRADO MIDE EN GRADOS, y CON HISTERESIS: entra a centrar si se
      // torcio mas de TOL_ANG_ENTRA, y no vuelve a avanzar hasta estar mejor
      // que TOL_ANG_SALE. Sin esa banda, justo en el umbral el robot alterna
      // entre girar y avanzar muchas veces por segundo: se zarandea, la
      // camara ve peor y termina perdiendo la pelota. El firmware siempre
      // tuvo la histeresis (10 y 5 sobre Yp); al pasar a angulos me la
      // comi, y de ahi salieron las 11 perdidas de la corrida del 29/09.
      float desvio = fabs(anguloDe(XpBueno, YpBueno));
      if (centrando) { if (desvio < TOL_ANG_SALE)  centrando = false; }
      else           { if (desvio > TOL_ANG_ENTRA) centrando = true;  }

      if (centrando) {
        bool haciaUnLado = (YpBueno > 0);
        if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT, pausaFrena);
      } else {
        avanzar(velAcercarse(XpBueno));   // OJO: aca NO se toca t_centrandoDesde
      }
    }
  }

  parar();
  if (!r.llego) r.msTotal = (long)(millis() - t0);
  midiendoAcerc = false;
  r.cuadros = nCuadros;
  r.cuadrosConPelota = nCuadrosConPelota;
  r.cuadrosAcerc = nCuadrosAcerc;
  r.conPelotaAcerc = nConPelotaAcerc;
  return r;
}

// =====================================================================
void mostrarRonda(const char *nombre, Ronda &r) {
  Serial.print(" "); Serial.print(nombre); Serial.println(":");
  Serial.print("    la vio a los ....... ");
  if (r.msHastaVerla < 0) Serial.println("NUNCA LA VIO");
  else { Serial.print(r.msHastaVerla / 1000.0, 1); Serial.println(" s"); }

  Serial.print("    llego en ........... ");
  if (r.llego) { Serial.print(r.msTotal / 1000.0, 1); Serial.println(" s"); }
  else         { Serial.print("NO LLEGO (corte a los "); Serial.print(r.msTotal / 1000.0, 1); Serial.println(" s)"); }

  Serial.print("    barrido ............ ");
  if (r.msBuscando > 0) {
    Serial.print(r.gradosBarridos * 1000.0 / (float)r.msBuscando, 0);
    Serial.print(" grados/s   ("); Serial.print(r.gradosBarridos, 0);
    Serial.print(" grados en "); Serial.print(r.msBuscando / 1000.0, 1); Serial.println(" s)");
  } else Serial.println("no llego a buscar");

  Serial.print("    grados hasta verla . "); Serial.print(r.gradosBarridos, 0);
  Serial.println("   (una vuelta son 360; mucho mas = le pasa por al lado sin verla)");
  Serial.print("    se destrabo ........ "); Serial.print(r.destrabes);
  Serial.print(" veces de "); Serial.print(MAX_DESTRABES);
  Serial.println(" (no lograba centrarse y se acerco igual)");
  Serial.print("    la perdio .......... "); Serial.print(r.perdidas);
  Serial.println(" veces (la vio y tuvo que buscarla de nuevo)");

  Serial.print("    la vio en ......... ");
  if (r.cuadrosAcerc > 0) {
    Serial.print(r.conPelotaAcerc * 100 / r.cuadrosAcerc);
    Serial.print("% de los cuadros MIENTRAS SE ACERCABA   (");
    Serial.print(r.conPelotaAcerc); Serial.print(" de "); Serial.print(r.cuadrosAcerc);
    Serial.println(")   <- deberia ser casi 100%");
  } else Serial.println("no llego a acercarse");
  Serial.println();
}

void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.println(" CALIBRACION BUSQUEDA — pulsado contra giro continuo");
  Serial.println("=====================================================");
  if (hechoA) mostrarRonda("RONDA A  (pulsado con la pausa FRENADA)", rA);
  else        Serial.println(" RONDA A: apagada (HACER_RONDA_A = false)");
  char rotB[70];
  snprintf(rotB, sizeof(rotB), "RONDA B  (giro CONTINUO a %d con rampa, frena al verla)", VEL_GIRO_CONTINUO);
  mostrarRonda(rotB, rB);

  Serial.println("-----------------------------------------------------");
  // La pelota blanca: si las dos rondas la ven poco, no es el freno
  long pa = (hechoA && rA.cuadrosAcerc > 0) ? rA.conPelotaAcerc * 100 / rA.cuadrosAcerc : -1;
  long pb = (rB.cuadrosAcerc > 0) ? rB.conPelotaAcerc * 100 / rB.cuadrosAcerc : 100;
  long mejor = (pa > pb) ? pa : pb;
  if (!hechoA) mejor = pb;

  // 2026-09-29: ANTES ESTE VEREDICTO MIRABA LA RONDA PEOR, y concluia "la
  // camara ve mal la pelota" cuando en realidad una de las rondas la veia
  // al 98%. Error de logica, no de medicion — la segunda vez en el dia.
  // Lo que importa es si EXISTE una forma de verla bien: si la mejor ronda
  // llega a ~100%, la camara y la pelota estan bien y lo que falla es como
  // se mueve el robot.
  if (mejor < 80) {
    Serial.println(" >>> LA CAMARA VE MAL LA PELOTA EN LAS DOS FORMAS.");
    Serial.print("     La mejor la vio en "); Serial.print(mejor);
    Serial.println("% de los cuadros, con la pelota ADELANTE.");
    Serial.println("     Eso no lo arregla ningun freno ni ninguna rampa:");
    Serial.println("     hay que calibrar la CAMARA. Si la pelota es mas");
    Serial.println("     blanca que la de siempre, los umbrales de naranja");
    Serial.println("     ya no le sirven. Ver vision/calibrar-umbrales.py");
  } else {
    Serial.print(" La camara VE BIEN la pelota: la mejor ronda la vio en ");
    Serial.print(mejor); Serial.println("% de los cuadros.");
    Serial.println(" Asi que el problema, si lo hay, es COMO SE MUEVE el robot.");
    Serial.println();
    if (hechoA) {
      Serial.print("   grados hasta verla:  A "); Serial.print(rA.gradosBarridos, 0);
      Serial.print("   B "); Serial.println(rB.gradosBarridos, 0);
      Serial.println("   (mucho mas de 360 = le pasa por al lado sin verla)");
      Serial.print("   la perdio:           A "); Serial.print(rA.perdidas);
      Serial.print("   B "); Serial.println(rB.perdidas);
      Serial.print("   la vio en:           A "); Serial.print(pa);
      Serial.print("%   B "); Serial.print(pb); Serial.println("%");
      Serial.println();
    }
    if (hechoA && rA.llego && rB.llego) {
      Serial.print(" >>> LLEGAR LE COSTO:  A "); Serial.print(rA.msTotal / 1000.0, 1);
      Serial.print(" s    B "); Serial.print(rB.msTotal / 1000.0, 1); Serial.println(" s");
      if (rA.msTotal < rB.msTotal) Serial.println("     GANA A (pulsado con freno).");
      else                          Serial.println("     GANA B (giro continuo con rampa).");
    } else if (rB.llego) {
      Serial.print(" >>> LLEGO EN "); Serial.print(rB.msTotal / 1000.0, 1);
      Serial.println(" s.   (referencias: 3,5 s la mejor B; 5,3 s la mejor A)");
    } else {
      Serial.println(" >>> NO LLEGO. Repetir: capaz la pelota quedo fuera de la vuelta,");
      Serial.println("     o el PWM del giro continuo quedo debajo del piso y no arranco.");
    }
  }
  Serial.println("=====================================================");
  Serial.println(" (para repetir: desenchufa y enchufa, o RESET)");
}

// =====================================================================
void setup() {
  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Serial1.begin(19200);
  Wire.begin();
  while (!Serial && millis() < 3000) { }

  Serial.println();
  Serial.println("#####################################################");
  Serial.println("#  CALIBRACION BUSQUEDA                              #");
  Serial.println("#  NO patea, NO orbita, NO escapa de la linea.        #");
  Serial.println("#  🚨 EN EL PISO. El robot se desplaza.               #");
  Serial.println("#####################################################");
  Serial.print("Destrabe: si en "); Serial.print(MS_LIMITE_CENTRAR / 1000);
  Serial.print(" s no se acerca "); Serial.print(MEJORA_DESTRABE);
  Serial.print(" cm, avanza "); Serial.print(MS_AVANCE_DESTRABE / 1000.0, 1);
  Serial.print(" s a "); Serial.print(VEL_DESTRABE);
  Serial.print(" y reintenta (hasta "); Serial.print(MAX_DESTRABES);
  Serial.println(" veces)");
  Serial.print("Centrado POR ANGULO: entra a "); Serial.print(TOL_ANG_ENTRA, 0);
  Serial.print(" grados, sale a "); Serial.print(TOL_ANG_SALE, 0);
  Serial.println(" (el firmware usa Yp crudo)");
  Serial.print("Ronda A: pulsado a "); Serial.print(VEL_GIRO);
  Serial.print("   Ronda B: continuo a "); Serial.println(VEL_GIRO_CONTINUO);
  Serial.print("Giro "); Serial.print(VEL_GIRO);
  Serial.print("  pulso "); Serial.print(MS_PULSO_BUSC);
  Serial.print(" ms  pausa "); Serial.print(MS_ESPERA_BUSC);
  Serial.println(" ms");

  hayGiro = bno.begin();
  if (hayGiro) {
    delay(700);
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    hayGiro = (sys == 5);
    Serial.print("Giroscopo: SYS_STATUS="); Serial.print(sys);
    Serial.println(hayGiro ? " (fusion corriendo) OK" : " -> sin el, no hay grados/s");
  } else {
    Serial.println("Giroscopo: no contesta -> sin el, no hay grados/s");
  }

  if (HACER_RONDA_A) {
    cuenta("RONDA A (pulsado). Pone la pelota LEJOS y sacate.", MS_ANTES);
    rA = correr(0);
    hechoA = true;
    Serial.println("    ronda A terminada.");
  } else {
    Serial.println();
    Serial.println("   (ronda A APAGADA: solo se corre la B)");
  }

  cuenta("RONDA B (giro CONTINUO con rampa). Pone la pelota LEJOS y sacate.", MS_ANTES);
  rB = correr(1);
  hechoB = true;
  Serial.println("    ronda B terminada.");

  informe();
  digitalWrite(LED, LOW);
}

void loop() {
  // El informe se reimprime cada 3 s para poder leerlo DESPUES, enchufando
  // el USB en la mesa sin apagar la bateria.
  static unsigned long t = 0;
  if (millis() - t < 3000) return;
  t = millis();
  if (hechoB) informe();
}
