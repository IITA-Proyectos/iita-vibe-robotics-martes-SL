/* =====================================================================
   IR-AL-MEDIO — volver al centro del arco usando las lineas
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-10-06
   =====================================================================

   DE QUE SE TRATA
   El robot no tiene encoders: no sabe cuanto camino. Solo sabe cuanto
   TIEMPO estuvo moviendose. Y un tiempo solo sirve si se sabe DESDE DONDE
   se empezo a contar.

   El 2026-10-06 medimos que del lateral izquierdo al medio son 2228 ms.
   Pero eso vale unicamente si el robot arranca pegado a ese lateral. Si
   arranca de cualquier lado, 2228 ms lo dejan en cualquier lado.

   Idea del equipo: usar LAS DOS LINEAS PERPENDICULARES de la cancha para
   saber donde esta en los dos ejes, y recien entonces cronometrar.

   ---------------------------------------------------------------------
   LA RUTINA, PASO POR PASO
   ---------------------------------------------------------------------
     0. si arranco sobre una linea, despegarse       -> se puede apoyar
        (solo si hace falta)                              EN CUALQUIER LADO
     1. de costado hasta ver la LINEA LATERAL        -> fija el ANCHO
     2. se aleja un poquito del lateral              -> ver (*)
     3. retrocede hasta la LINEA DEL FONDO           -> fija la PROFUNDIDAD
        (la del borde de la cancha, no la del area)
     4. avanza 50 cm                                 -> sale del area, que
                                                        no sea falta
     5. vuelve al MISMO lateral                      -> ver (**)
     6. al medio por TIEMPO                          -> ahora si
     7. atras hasta la LINEA DEL AREA                -> fija la profundidad,
        (igual que el arranque del juego)                ya centrado
     8. un poquito adelante                          -> no quedar sentado
                                                        sobre el blanco

   (*)  ¿Por que alejarse antes de retroceder? Porque los sensores que ven
        el blanco son los de ATRAS, y si el robot quedo apoyado sobre la
        linea lateral, ya estan viendo blanco: retroceder "hasta ver la
        linea del fondo" terminaria enseguida, sin haberse movido. Hay que
        despegarse primero para que "ver blanco" signifique UNA sola cosa.

   (**) ¿Por que volver al lateral si ya habia estado? Porque retroceder y
        avanzar pueden haberlo corrido de costado unos centimetros. El
        cronometro del paso 6 arranca desde el lateral, asi que hay que
        estar seguros de estar ahi. Es barato y saca toda la duda.

   ---------------------------------------------------------------------
   🚨 EL ROBOT SE MUEVE Y VA A LA CANCHA
   ---------------------------------------------------------------------
   No sirve en la mesa: ahi los tres sensores leen como blanco (medimos
   505/651/601 contra un umbral de 425) y el robot creeria estar siempre
   sobre una linea.

   Sin la computadora, la unica forma de pararlo es LA LLAVE DE LA BATERIA.

        LED parpadeo RAPIDO  -> no hay giroscopio, no me muevo
        LED lento            -> cuenta de arranque
        LED fijo             -> moviendose
        LED latido lento     -> termino bien
        LED 5 destellos      -> algo no cerro (no encontro una linea)

   🚨 LA BATERIA PRENDIDA, Y PRENDIDA ANTES DEL USB.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

#define LINEA_ADELANTE  A12
#define LINEA_ATRAS_IZQ A13
#define LINEA_ATRAS_DER A11

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

const unsigned long BAUDIOS = 19200;

// ---- umbral de blanco: medido en cancha el 2026-09-15 ----
const int UMBRAL_BLANCO = 425;

// ---- la mezcla de costado, igual que el juego ----
const int LADO_FRENTE  = 50;
const int LADO_TRASERA = 89;

// ---- velocidades ----
//
// HAY DOS VELOCIDADES DE COSTADO, Y NO ES UN DESCUIDO.
//
// 2026-10-06, pedido del equipo: ir al lateral a la misma velocidad que el
// retroceso. Se puede, PERO NO EN TODOS LOS PASOS, y la razon es la
// diferencia entre moverse hasta VER ALGO y moverse POR TIEMPO:
//
//   BUSCAR (pasos 0, 1, 2 y 5) — terminan cuando los sensores ven la linea.
//      Si el robot va mas rapido, llega antes; el resultado es el mismo.
//      La velocidad aca no cambia DONDE termina. Puede ser 110.
//
//   AL MEDIO (paso 6) — termina cuando se cumple un TIEMPO: 2228 ms.
//      Ese numero se midio a velocidad 100 con `medir-ancho`. A 110 el robot
//      haria mas camino en los mismos milisegundos y se pasaria del medio.
//
// 🚨 POR ESO VEL_COSTADO_MEDIR NO SE TOCA sin volver a medir. No es una
// perilla de gusto: es la que hace que los 2228 ms signifiquen algo.
const int VEL_COSTADO_BUSCAR = 110;   // igual que el retroceso
const int VEL_COSTADO_MEDIR  = 100;   // 🚨 atado a MS_AL_MEDIO
const int VEL_ATRAS     = 110;   // la del retroceso del juego
// 2026-10-06, pedido del equipo: que el avance de 40 cm vaya a la MISMA
// velocidad que el retroceso. Asi los dos movimientos de ida y vuelta son
// simetricos y el robot no da el tiron del despeje en medio de una maniobra
// de acomodarse.
const int VEL_ADELANTE  = VEL_ATRAS;

// ---- los tiempos ----
//
// Cuanto avanza en el paso 4, para salir del area.
// 2026-10-06: el equipo lo subio de 40 a 50 cm.
//
// La unica calibracion con regla que tenemos (2026-08-04) es a POTENCIA 200:
// 1 cm cada 10 ms, mas ~33 ms que el robot tarda en arrancar. Pero este
// avance va a 110, no a 200, asi que hay que estirar el tiempo:
//
//      a 200:  10,0 ms por cm
//      a 110:  10,0 x (200/110) = 18,2 ms por cm
//      50 cm:  18,2 x 50 = 910 ms,  mas 33 de arranque  =  943 ms
//
//      (40 cm eran 760 ms, para comparar)
//
// ⚠️ ESO ES UNA REGLA DE TRES, NO UNA MEDICION. La velocidad no sube
// exactamente parejo con el PWM (pesa el roce, y mas cuanto mas lento se va),
// asi que lo mas probable es que 943 ms se queden CORTOS de 50 cm.
//
// 🎯 COMO ARREGLARLO BIEN, y son dos minutos: marcar con cinta donde arranca
// el robot, correr la rutina, y medir con la regla cuanto avanzo de verdad en
// el paso 4. Si hizo 38 cm en vez de 50, subir este numero en la misma
// proporcion (943 x 50/38 = 1241). Recien ahi deja de ser una estimacion.
const unsigned long MS_ADELANTE = 943;

// Alejarse del lateral antes de retroceder.
// ⚠️ ESTE NUMERO ESTA EN MILISEGUNDOS Y NO SABEMOS A CUANTOS CM EQUIVALE:
// medimos el cruce completo de la cancha (4446 ms) pero nunca medimos el
// ancho con cinta metrica, asi que no tenemos la conversion. 400 ms es como
// el 9% del ancho — unos 10 a 15 cm segun cuanto mida la cancha. Para
// despegarse de una linea alcanza y sobra; no necesita precision.
const unsigned long MS_ALEJARSE = 400;

// 🎯 EL NUMERO QUE VINIMOS A USAR. Medido el 2026-10-06 con `medir-ancho`,
// dos corridas que dieron 2213 y 2243 ms (1,4% de diferencia).
// Desde el lateral IZQUIERDO, yendo a la derecha.
const unsigned long MS_AL_MEDIO = 2228;

// Topes: si no encuentra una linea, mejor parar y avisar que seguir de largo
// hasta salirse de la cancha. El cruce entero son ~4450 ms, asi que 6000 es
// holgado pero no infinito.
const unsigned long MS_TOPE_LATERAL = 6000;
const unsigned long MS_TOPE_FONDO   = 4000;

// ---- el acomodado final, copiado del programa de juego ----
//
// 2026-10-06, pedido del equipo: ya en el medio, retroceder hasta la linea
// del AREA y despues avanzar un poquito. Asi el robot termina en el mismo
// lugar exacto en el que el programa de juego se para al arrancar: centrado
// a lo ancho (paso 6) y a una distancia conocida del fondo (este paso).
//
// El tope sale del juego (MS_MAX_RETROCESO = 1200): si en 1200 ms de
// retroceso no aparecio la linea, algo esta mal y es mejor frenar que seguir
// yendo para atras hasta salirse.
//
// ⚠️ UNA DIFERENCIA CON EL JUEGO, a proposito: el juego se QUEDA donde
// encontro la linea ("Se queda donde encontro la linea", dice su comentario).
// Aca se avanza un poquito despues, para no quedar sentado sobre el blanco —
// si no, los sensores de atras ven linea todo el tiempo y cualquier cosa que
// mire "¿estoy sobre una linea?" se confunde.
const unsigned long MS_TOPE_AREA    = 1200;

// 10 cm para salir de la linea. Misma cuenta que el avance del paso 4:
// a 110 son 18,2 ms por cm, mas 33 de arranque -> 10 x 18,2 + 33 = 215.
// (El juego usa 133 ms para los mismos 10 cm, pero a potencia 200.)
const unsigned long MS_SALIR_LINEA  = 215;

const unsigned long MS_CUENTA        = 3000;
const unsigned long MS_FRENO         = 200;
const unsigned long MS_QUIETO        = 300;
const unsigned long MS_MAX_CALIBRAR  = 8000;

// ---- correccion de rumbo: EXACTAMENTE la del programa de juego ----
//
// Y es exactamente a proposito. El 2026-10-06 se perdio media clase porque
// `medir-ancho` tenia este mismo lazo "mejorado" con un piso de PWM y una
// zona muerta mas ancha, y zigzagueaba. El juego, sin esos agregados, anda
// bien. No tocar.
const float KP_RUMBO = 5.0;
const float KI_RUMBO = 1.5;
const float LIMITE_INTEGRAL = 30.0;
const int   MAX_CORRECCION_RUMBO = 100;
const float ZONA_MUERTA_RUMBO = 1.0;

float integralRumbo = 0;
unsigned long t_ultimoControl = 0;
float rumboBase = -1;
bool  hayGiroscopo = false;

// ---- rampa de arranque, igual que el juego ----
const int RAMPA_PASO = 10;
const unsigned long RAMPA_MS = 10;
int potenciaRampa = 0;
unsigned long t_rampa = 0;


// ⚠️ El enum va ACA ARRIBA, antes de la primera funcion, y no junto a la
// rutina. En un .ino el IDE genera los prototipos de todas las funciones y
// los mete antes de la primera que encuentre definida: si el enum estuviera
// mas abajo, el prototipo de nombrePaso(Paso) quedaria antes del enum y no
// compila.
enum Paso {
  CUENTA,
  P0_DESPEGARSE,     // si arranco sobre una linea, salirse primero
  P1_AL_LATERAL,     // de costado hasta la linea lateral
  P2_ALEJARSE,       // despegarse de esa linea
  P3_AL_FONDO,       // retroceder hasta la linea del fondo
  P4_ADELANTE,       // 40 cm para adelante, salir del area
  P5_AL_LATERAL,     // volver al mismo lateral
  P6_AL_MEDIO,       // y al medio por tiempo
  P7_AL_AREA,        // atras hasta la linea del area
  P8_SALIR_LINEA,    // y un poquito adelante, para no quedar sobre el blanco
  LISTO,
  FALLO
};


// ---------------------------------------------------------------- motores

void aplicar(int v1, int v2, int v3) {
  int p[3] = { v1, v2, v3 };
  int ina[3] = { INA1, INA2, INA3 };
  int inb[3] = { INB1, INB2, INB3 };
  int pwm[3] = { PWM1, PWM2, PWM3 };
  for (int i = 0; i < 3; i++) {
    int v = p[i];
    if (v > 255)  v = 255;
    if (v < -255) v = -255;
    digitalWrite(ina[i], v >= 0 ? 1 : 0);
    digitalWrite(inb[i], v >= 0 ? 0 : 1);
    analogWrite(pwm[i], abs(v));
  }
}

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Freno electrico: cortocircuita los motores. parar() solo los suelta y el
// robot sigue de largo.
void frenar() {
  digitalWrite(INA1, 0); digitalWrite(INB1, 0); analogWrite(PWM1, 255);
  digitalWrite(INA2, 0); digitalWrite(INB2, 0); analogWrite(PWM2, 255);
  digitalWrite(INA3, 0); digitalWrite(INB3, 0); analogWrite(PWM3, 255);
}

void reiniciarRampa() { potenciaRampa = 0; t_rampa = millis(); }

int rampa(int objetivo) {
  if (potenciaRampa < objetivo && millis() - t_rampa >= RAMPA_MS) {
    t_rampa = millis();
    potenciaRampa += RAMPA_PASO;
    if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  }
  if (potenciaRampa > objetivo) potenciaRampa = objetivo;
  return potenciaRampa;
}


// ---------------------------------------------------------------- sensores

bool veBlancoIzq()   { return analogRead(LINEA_ATRAS_IZQ) >= UMBRAL_BLANCO; }
bool veBlancoDer()   { return analogRead(LINEA_ATRAS_DER) >= UMBRAL_BLANCO; }
bool algunoDeAtras() { return veBlancoIzq() || veBlancoDer(); }


// ---------------------------------------------------------------- rumbo

float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// Devuelve el rumbo, o -1 si el sensor no esta dando datos.
// 🚨 NO llamar a bno.getSystemStatus(): esa funcion de la libreria termina
// con delay(200) y mete medio segundo de ceguera en el lazo de control. Fue
// la causa del zigzagueo del 2026-10-06.
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

// Cuanto sumarle a las TRES ruedas para que el robot no gire.
int correccionDeRumbo() {
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


// ---------------------------------------------------------------- movimientos

// s > 0 va a la DERECHA, s < 0 a la IZQUIERDA.
void deCostado(int s) {
  int fuerza = rampa(abs(s));
  if (s < 0) fuerza = -fuerza;
  int frente  = (fuerza * LADO_FRENTE)  / 100;
  int trasera = (fuerza * LADO_TRASERA) / 100;
  int c = correccionDeRumbo();
  aplicar(frente + c, frente + c, -trasera + c);
}

void adelante(int v) {
  int p = rampa(v);
  int c = correccionDeRumbo();
  aplicar(+p + c, -p + c, c);
}

void atras(int v) {
  int p = rampa(v);
  int c = correccionDeRumbo();
  aplicar(-p + c, +p + c, c);
}


// ---------------------------------------------------------------- la rutina


Paso paso = CUENTA;
unsigned long t_paso = 0;

// Hacia que lateral se va. -1 = izquierda. El tiempo MS_AL_MEDIO se midio
// desde la izquierda, asi que si algun dia se cambia a la derecha hay que
// usar el otro numero (2331 ms, medido el mismo dia).
const int LADO = -1;

// Cuanto tardo cada paso: se informa al final.
unsigned long duracion[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char* nombrePaso(Paso p) {
  switch (p) {
    case CUENTA:        return "cuenta de arranque";
    case P0_DESPEGARSE: return "0. despegarse de la linea";
    case P1_AL_LATERAL: return "1. al lateral";
    case P2_ALEJARSE:   return "2. alejarse del lateral";
    case P3_AL_FONDO:   return "3. atras hasta el fondo";
    case P4_ADELANTE:   return "4. 50 cm adelante";
    case P5_AL_LATERAL: return "5. al lateral de nuevo";
    case P6_AL_MEDIO:   return "6. al medio por tiempo";
    case P7_AL_AREA:    return "7. atras hasta la linea del area";
    case P8_SALIR_LINEA:return "8. salir de la linea";
    case LISTO:         return "LISTO";
    default:            return "FALLO";
  }
}

void pasarA(Paso siguiente) {
  if (paso >= P0_DESPEGARSE && paso <= P8_SALIR_LINEA) {
    duracion[paso] = millis() - t_paso;
    Serial.print("   "); Serial.print(nombrePaso(paso));
    Serial.print(": "); Serial.print(duracion[paso]); Serial.println(" ms");
  }
  frenar();
  delay(MS_FRENO);
  parar();
  delay(MS_QUIETO);
  reiniciarRampa();
  reiniciarCorreccion();
  paso = siguiente;
  t_paso = millis();
  if (siguiente >= P0_DESPEGARSE && siguiente <= P8_SALIR_LINEA) {
    Serial.print(">> "); Serial.println(nombrePaso(siguiente));
  }
}

void fallar(const char* motivo) {
  parar();
  Serial.println();
  Serial.print("!! FALLO en "); Serial.print(nombrePaso(paso));
  Serial.print(": "); Serial.println(motivo);
  Serial.println("   (5 destellos en el LED)");
  paso = FALLO;
  t_paso = millis();
}

void informeFinal() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" TERMINO. Quedo centrado en el arco, y a la distancia del");
  Serial.println(" area igual que cuando arranca el programa de juego.");
  Serial.println();
  Serial.println(" Cuanto tardo cada paso:");
  for (int i = P0_DESPEGARSE; i <= P8_SALIR_LINEA; i++) {
    if (duracion[i] == 0 && i == P0_DESPEGARSE) continue;   // no hizo falta
    Serial.print("   "); Serial.print(nombrePaso((Paso)i));
    Serial.print(": "); Serial.print(duracion[i]); Serial.println(" ms");
  }
  Serial.println();
  Serial.println(" COMO SABER SI SALIO BIEN, sin mirar numeros:");
  Serial.println("  - ¿quedo en el medio del arco? Eso es todo lo que importa.");
  Serial.println("  - el paso 5 tendria que tardar MUCHO MENOS que el 1: en el");
  Serial.println("    1 cruza media cancha, en el 5 solo vuelve lo que se corrio");
  Serial.println("    al retroceder y avanzar. Si tarda parecido, algo lo movio");
  Serial.println("    de costado mucho mas de lo esperado.");
  Serial.println("=========================================================");
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  parar();
  pinMode(LINEA_ADELANTE,  INPUT);
  pinMode(LINEA_ATRAS_IZQ, INPUT);
  pinMode(LINEA_ATRAS_DER, INPUT);

  Serial.begin(BAUDIOS);
  Wire.begin();
  delay(800);

  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" IR-AL-MEDIO — volver al centro usando las lineas");
  Serial.println(" 🚨 EL ROBOT SE MUEVE. Esto va en la cancha, no en la mesa.");
  Serial.println("=========================================================");

  Serial.print(" giroscopio: ");
  for (int i = 0; i < 10 && !hayGiroscopo; i++) {
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (!hayGiroscopo) {
    Serial.println("NO CONTESTA. No me muevo (parpadeo rapido).");
    Serial.println(" Sin rumbo, de costado el robot gira solo y la rutina");
    Serial.println(" no tiene sentido. ¿Esta prendida la bateria?");
    return;
  }
  bno.setExtCrystalUse(true);   // primero el cristal: reinicia la fusion
  delay(700);                   // y DESPUES la espera
  Serial.println("OK");

  Serial.print(" esperando que se calibre (quieto): ");
  unsigned long tc = millis();
  uint8_t sis = 0, gir = 0, ace = 0, mag = 0;
  while (millis() - tc < MS_MAX_CALIBRAR) {
    bno.getCalibration(&sis, &gir, &ace, &mag);
    if (gir >= 3) break;
    delay(100);
  }
  Serial.print("giroscopo "); Serial.print(gir); Serial.print("/3 en ");
  Serial.print(millis() - tc); Serial.println(" ms");

  paso = CUENTA;
  t_paso = millis();
  Serial.print(" arranca en "); Serial.print(MS_CUENTA / 1000);
  Serial.println(" s. NO LO TOQUES.");
}


void loop() {
  unsigned long ahora = millis();

  if (!hayGiroscopo) {
    parar();
    digitalWrite(LED, ((ahora / 50) % 2) ? HIGH : LOW);   // rapido = sin giro
    return;
  }

  switch (paso) {

    case CUENTA:
      parar();
      digitalWrite(LED, ((ahora / 400) % 2) ? HIGH : LOW);
      if (ahora - t_paso >= MS_CUENTA) {
        // El rumbo a sostener se fija ACA, con el robot ya quieto.
        float r = rumboActual();
        if (r < 0) { fallar("el giroscopio no da datos"); break; }
        rumboBase = r;
        Serial.print(" rumbo de referencia: "); Serial.println(rumboBase, 1);
        // 🎯 SE PUEDE APOYAR EN CUALQUIER LADO DE LA CANCHA, incluso sobre
        // una linea. Si arranca encima de una, el paso 1 terminaria al
        // instante creyendo que ya llego al lateral. Asi que primero se
        // despega, y recien despues empieza a buscar.
        if (algunoDeAtras()) pasarA(P0_DESPEGARSE);
        else                 pasarA(P1_AL_LATERAL);
      }
      break;

    // ---- 0: si arranco sobre una linea, salirse ----
    //
    // Se mueve hacia el mismo lado al que va a ir despues, asi no deshace
    // camino. Cuando los dos sensores dejan de ver blanco, sigue un rato
    // mas (MS_ALEJARSE) para no quedar justo en el borde: desde ahi el
    // primer temblorcito del rumbo lo volveria a poner sobre la linea.
    case P0_DESPEGARSE:
      digitalWrite(LED, HIGH);
      deCostado(LADO * VEL_COSTADO_BUSCAR);
      if (!algunoDeAtras()) {
        // Marca el momento en que se solto, para contar el margen desde ahi.
        static unsigned long t_solto = 0;
        if (t_solto == 0 || t_solto < t_paso) t_solto = ahora;
        if (ahora - t_solto >= MS_ALEJARSE) { t_solto = 0; pasarA(P1_AL_LATERAL); }
      }
      if (ahora - t_paso >= MS_TOPE_LATERAL) {
        fallar("no logro salirse de la linea");
      }
      break;

    // ---- 1: de costado hasta la linea lateral ----
    case P1_AL_LATERAL:
      digitalWrite(LED, HIGH);
      deCostado(LADO * VEL_COSTADO_BUSCAR);
      if (algunoDeAtras()) { pasarA(P2_ALEJARSE); break; }
      if (ahora - t_paso >= MS_TOPE_LATERAL) {
        fallar("no encontro la linea lateral");
      }
      break;

    // ---- 2: despegarse de esa linea ----
    //
    // Sin esto, el paso 3 terminaria al instante: los sensores de atras ya
    // estan viendo el blanco de la linea lateral.
    case P2_ALEJARSE:
      digitalWrite(LED, HIGH);
      deCostado(-LADO * VEL_COSTADO_BUSCAR);
      if (ahora - t_paso >= MS_ALEJARSE) pasarA(P3_AL_FONDO);
      break;

    // ---- 3: retroceder hasta la linea del fondo ----
    case P3_AL_FONDO:
      digitalWrite(LED, HIGH);
      atras(VEL_ATRAS);
      if (algunoDeAtras()) { pasarA(P4_ADELANTE); break; }
      if (ahora - t_paso >= MS_TOPE_FONDO) {
        fallar("no encontro la linea del fondo");
      }
      break;

    // ---- 4: 50 cm para adelante, para salir del area ----
    case P4_ADELANTE:
      digitalWrite(LED, HIGH);
      adelante(VEL_ADELANTE);
      if (ahora - t_paso >= MS_ADELANTE) pasarA(P5_AL_LATERAL);
      break;

    // ---- 5: volver al mismo lateral ----
    //
    // Retroceder y avanzar pueden haberlo corrido de costado. El cronometro
    // del paso 6 arranca desde el lateral, asi que hay que estar ahi.
    case P5_AL_LATERAL:
      digitalWrite(LED, HIGH);
      deCostado(LADO * VEL_COSTADO_BUSCAR);
      if (algunoDeAtras()) { pasarA(P6_AL_MEDIO); break; }
      if (ahora - t_paso >= MS_TOPE_LATERAL) {
        fallar("no encontro la linea lateral la segunda vez");
      }
      break;

    // ---- 6: al medio, por tiempo ----
    case P6_AL_MEDIO:
      digitalWrite(LED, HIGH);
      deCostado(-LADO * VEL_COSTADO_MEDIR);   // 🚨 100, atado a MS_AL_MEDIO
      if (ahora - t_paso >= MS_AL_MEDIO) pasarA(P7_AL_AREA);
      break;

    // ---- 7: atras hasta la linea del area ----
    //
    // Igual que el arranque del programa de juego: la linea del area es una
    // marca fisica, siempre esta en el mismo lugar. Con el paso 6 el robot
    // ya quedo centrado a lo ancho; esto le fija la profundidad.
    case P7_AL_AREA:
      digitalWrite(LED, ((ahora / 200) % 2) ? HIGH : LOW);
      atras(VEL_ATRAS);
      if (algunoDeAtras()) { pasarA(P8_SALIR_LINEA); break; }
      if (ahora - t_paso >= MS_TOPE_AREA) {
        fallar("no encontro la linea del area al volver del medio");
      }
      break;

    // ---- 8: un poquito adelante, para no quedar sobre el blanco ----
    case P8_SALIR_LINEA:
      digitalWrite(LED, HIGH);
      adelante(VEL_ADELANTE);
      if (ahora - t_paso >= MS_SALIR_LINEA) {
        duracion[P8_SALIR_LINEA] = millis() - t_paso;
        frenar();
        delay(MS_FRENO);
        parar();
        paso = LISTO; t_paso = millis();
        informeFinal();
      }
      break;

    case LISTO:
      parar();
      // Latido lento: un destello corto cada 2 segundos.
      digitalWrite(LED, ((ahora % 2000) < 120) ? HIGH : LOW);
      break;

    case FALLO:
      parar();
      // 5 destellos y una pausa, repitiendo.
      {
        unsigned long t = ahora % 2500;
        bool on = (t < 1000) && ((t / 100) % 2 == 0);
        digitalWrite(LED, on ? HIGH : LOW);
      }
      break;
  }
}
