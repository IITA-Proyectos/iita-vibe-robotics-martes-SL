/* =====================================================================
   MEDIR-ANCHO — cuanto tarda el arquero en ir de un costado al otro
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-21
   =====================================================================

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
        latido lento             -> TERMINO BIEN, numeros guardados
        destellos que se repiten -> ERROR, NO se guardo nada:
             2 destellos = arranco con un sensor de atras sobre una linea
             3 destellos = sin giroscopio (no aparecio, o se cayo andando)
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

   ⚠️ Sin giroscopio NO SE MUEVE, a proposito. De costado, la mezcla
   50/50/89 hace girar al robot si nadie lo corrige, y una medicion hecha
   girando da un numero falso. Mejor ningun numero que uno malo.

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
   abrir `pruebas/herramientas/mirar.bat`. Sin bateria el giroscopio no
   tiene corriente, asi que el robot NO SE MUEVE (termina en "sin
   giroscopio", que en este caso es lo esperado) y muestra lo guardado.
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
//
// 🎯 2026-10-06: PASO JUSTO ESO. El robot oscilaba de costado, poco al
// principio y mucho despues. Oscilacion que CRECE es la firma de un lazo de
// control con una ZONA MUERTA adentro, y aca la zona muerta estaba a la
// vista: con las ruedas de adelante en 50 y el piso en ~70, la correccion
// de rumbo empuja y NO PASA NADA. El termino acumulado se infla mientras el
// robot no responde; cuando la suma por fin cruza el piso, la rueda arranca
// de golpe y se pasa de largo. Y al otro lado, lo mismo pero mas grande.
//
//      100  ->  ruedas de adelante en 50
//      160  ->  ruedas de adelante en 80
//
// 🎯 2026-10-06 — EL CAMINO CORTO ERA OTRO.
//
// Con 100 el robot zigzagueaba desde el medio; con 160 ese tramo salia
// derecho. La conclusion facil era "hay que ir mas rapido", y es la
// equivocada: a 160 el robot llega al borde y choca contra la madera del
// piso con todo.
//
// Lo dijo el equipo, y tienen razon: lo que habia que subir no era la
// velocidad, era EL PISO DE LA CORRECCION.
//
// El problema nunca fue que el robot se moviera poco — rodando se mueve
// con ~40 de PWM. El problema era que cuando la correccion de rumbo pedia
// unos pocos PWM de diferencia entre ruedas, ESA DIFERENCIA NO ALCANZABA
// para vencer el roce, y el robot no respondia hasta que el error ya era
// grande. Subir la velocidad entera era darle margen a la correccion por
// el lado caro: mas envion, mas golpe, y los tiempos medidos valdrian para
// una velocidad que el juego no usa.
//
// Entonces vuelve a 100 (la misma del 21/09 y la del programa de juego,
// asi los numeros son comparables) y la fuerza se la damos donde falta:
// en PWM_MIN_CORRECCION, aca abajo.
//
// ⚠️ El numero medido SOLO VALE PARA ESTA VELOCIDAD. Si se cambia, hay que
// volver a medir y cambiarla tambien en el programa de juego.
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

// ---- enderezarse despues del golpe ----
//
// 🎯 EL PROBLEMA QUE ESTO ARREGLA (2026-10-06)
//
// Al llegar al lateral el robot choca contra el borde de la cancha, y el
// golpe lo GIRA. Si el tramo siguiente arrancara asi, pasarian dos cosas
// malas a la vez: la correccion de rumbo tendria que pelear un error enorme
// MIENTRAS cruza (de ahi el zigzagueo), y el tiempo cronometrado seria el de
// un robot yendo en diagonal, no cruzando derecho. O sea: el numero no
// valdria.
//
// La solucion es separar las dos cosas. Primero se endereza QUIETO, girando
// en el lugar hasta volver al rumbo de referencia. Recien cuando esta
// cuadrado arranca el cronometro. Asi cada cruce empieza igual que el
// anterior, que es la unica forma de que los tiempos se puedan comparar.
//
// Los PWM salen de lo medido el 2026-09-15: desde quieto hace falta ~75
// para vencer el roce, y arriba de ~85 el giro se pasa y oscila.
const int           PWM_MIN_ENDEREZAR   = 75;
const int           PWM_MAX_ENDEREZAR   = 85;
const float         TOLERANCIA_ENDEREZAR = 3.0;    // grados
const unsigned long MS_MAX_ENDEREZAR    = 2500;    // si no llega, seguir igual

int   vecesQueSeEnderezo = 0;
float peorGolpe = 0;        // el mayor desvio encontrado al llegar al lateral
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
// ---- 🔴 EL PISO DE LA CORRECCION: SE PROBO Y SE SACO (2026-10-06) ----
//
// La idea era: si la correccion pide pocos PWM, el roce se los come y el
// robot no responde. Entonces ponerle un minimo. Se probo con 30 y con 70.
// NO ARREGLO NADA, y con 70 empeoro bastante. La cuenta de por que:
//
//   yendo a la derecha, las ruedas de adelante reciben 50 (mezcla 50/50/89)
//   si la correccion pide c = -70  ->  50 - 70 = -20
//
// ⚠️ LAS RUEDAS DE ADELANTE SE DAN VUELTA. Con un piso de 70 la correccion
// deja de ser un ajuste y pasa a ser un interruptor que destruye el
// movimiento lateral cada vez que dispara. Eso ES el zigzagueo.
//
// Y la prueba de que el piso nunca hizo falta: el programa de juego NO LO
// TIENE —su correccion devuelve el valor crudo, sea 3 o 40— y mueve el
// robot de costado todo el partido sin zigzaguear, con las ruedas de
// adelante en 30, menos todavia que los 50 de aca.
//
// El problema real era el delay(200) escondido en getSystemStatus(); ver el
// bloque de preguntarSiFusiona(). Queda escrito para no volver a intentarlo.
const int PWM_MIN_CORRECCION = 0;        // 0 = sin piso, como el juego

// La zona muerta vuelve a 1,0: es la del programa de juego. Se habia
// ensanchado a 2,0 solo para acompañar al piso, que ya no esta.
const float ZONA_MUERTA_RUMBO = 1.0;

// ---- cuanto se torcio en cada tramo ----
//
// Para saber si un cambio mejoro la oscilacion hace falta un NUMERO, no una
// impresion. Se guarda el peor desvio de cada tramo (contra el rumbo que
// tendria que estar sosteniendo) y se informa al final.
//
// Como leerlo: si el peor desvio es de 2 o 3 grados, el robot va derecho y
// lo que se vio fue el deslizamiento normal. Si es de 15 o 20, oscila de
// verdad. Y si crece tramo a tramo, el lazo se esta yendo.
float desvioPeor[4] = {0, 0, 0, 0};

// Lo ultimo que devolvio rumboActual(). Existe para NO tener que leer el
// giroscopio dos veces por vuelta: cada lectura es una charla por I2C de
// un par de milisegundos, y en un lazo de control eso es retardo puro.
float ultimoRumboLeido = -1;

float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

// Una lectura en cero suelta no alcanza para decir que se cayo: puede ser
// un tropiezo del bus. Recien con varias seguidas se da por caido (es lo
// que hace `cuadrado-giroscopo`).
const int CEROS_PARA_CAIDO = 10;
int cerosSeguidos = 0;
const unsigned long MS_ESPERA_DATOS_GIRO = 5000;

// 🎯 ESPERAR A QUE EL GIROSCOPIO SE CALIBRE, QUIETO.
// 2026-09-29: el arranque del giroscopio se rehizo juntando todo lo que
// aprendimos. Ver el bloque de rumboActual() y el de setExtCrystalUse.
// Y el motivo por el que se vuelve a medir: los numeros del 21/09
// (4618/4396 ms de cruce) se tomaron con EL CABLE DEL GIROSCOPIO ROTO, que
// se descubrio el 29/09. Si se caia en medio de un cruce, el robot se
// torcia sin corregir y el tiempo medido no es el de ir derecho.
//
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
// Esta medicion vive al principio de la EEPROM. ⚠️ La EEPROM es UNA SOLA y
// la comparten todos los programas: el 29/09 `derecho-y-vuelta` guardaba su
// historial tambien en la direccion 0 y piso esta medicion. Quedo repartida
// asi, y el que agregue un programa que guarde algo tiene que respetarlo:
//      0 .. 255   esta medicion (medir-ancho y medir-ancho-sin-giro)
//    256 .. 767   derecho-y-vuelta
//    768 .. 4095  libre
const int DIR_EEPROM_ANCHO = 0;

MedicionAncho medicion = { 0, 0, 0, 0, 0, 0 };


// ---- estado ----
enum Fase { CUENTA, MOVIENDO, FRENANDO, ENDEREZANDO, TERMINADO, ERROR };
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

enum Error { ERR_NINGUNO = 0, ERR_SOBRE_LINEA = 2, ERR_SIN_GIRO = 3,
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

// 🎯 PREGUNTARLE AL CHIP EN VEZ DE ADIVINAR (2026-09-29)
//
// Hasta hoy, "el sensor se cayo" se deducia de que los tres angulos dieran
// 0.0 exacto, porque la libreria Adafruit devuelve ceros cuando no puede
// leer el chip. Hoy vimos que ESA DEDUCCION FALLA EN LAS DOS DIRECCIONES:
//
//   - da "muerto" con el chip SANO: recien arrancado, la fusion todavia no
//     produce angulos y devuelve 0,0,0 — pero los datos crudos del
//     giroscopo estaban llegando perfectamente. Lo vimos en vivo.
//   - da "sano" con el chip MUERTO: si el rumbo de reposo cae en 0.0, tres
//     ceros es una postura real. Y ESTE robot se para justo en 359.9, a un
//     paso del borde. A la otra mesa esto ya los mordio en pleno juego.
//
// El registro 0x39 (SYS_STATUS) del BNO055 lo dice sin adivinar: 5 = "el
// algoritmo de fusion esta corriendo". Es lo que usa el delantero desde el
// 01/09. Se consulta cada MS_CHEQUEO_FUSION para no llenar el bus.
const unsigned long MS_CHEQUEO_FUSION = 400;
unsigned long t_chequeoFusion = 0;
bool fusionCorriendo = true;

// 🚨 NO USAR bno.getSystemStatus(): TERMINA CON delay(200).
//
// Esta es la causa del zigzagueo, y estuvo escondida una semana entera.
// Adafruit_BNO055.cpp, en getSystemStatus(), despues de leer los registros
// hace un delay(200) antes de volver. Llamandola cada 400 ms desde
// rumboActual(), el robot manejaba A CIEGAS 200 de cada 400 milisegundos
// —la mitad del tiempo— con la ultima correccion clavada en los motores,
// porque el PWM lo sigue generando el hardware mientras el programa duerme.
//
// Eso es RETARDO, no ganancia. Y por eso no sirvio subir la velocidad, ni
// poner piso a la correccion, ni ensanchar la zona muerta: las tres mueven
// el TAMAÑO de la correccion, y el problema era el TIEMPO EN QUE NADIE MIRA.
//
// La misma informacion se saca leyendo el registro 0x39 a mano, en un par
// de milisegundos y sin dormir a nadie.
bool preguntarSiFusiona() {
  Wire.beginTransmission(0x28);
  Wire.write(0x39);                      // SYS_STATUS
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(0x28, 1) != 1)    return false;
  return Wire.read() == 5;               // 5 = la fusion esta corriendo
}

// El rumbo, o -1 si el sensor no esta dando datos utiles.
float rumboActual() {
  if (!hayGiroscopo) return -1;

  unsigned long ahora = millis();
  if (ahora - t_chequeoFusion >= MS_CHEQUEO_FUSION) {
    t_chequeoFusion = ahora;
    fusionCorriendo = preguntarSiFusiona();
  }

  sensors_event_t e;
  bno.getEvent(&e);
  bool tresCeros = (e.orientation.x == 0.0 && e.orientation.y == 0.0
                    && e.orientation.z == 0.0);

  if (!fusionCorriendo) {
    // Esto SI es una caida de verdad: el chip mismo dice que no fusiona.
    if (cerosSeguidos < CEROS_PARA_CAIDO) cerosSeguidos++;
    return -1;
  }
  if (tresCeros) {
    // Fusiona y da tres ceros: es una postura real (el robot parado en el
    // borde de 0 grados). No se cuenta como caida.
    cerosSeguidos = 0;
    return 0.01;      // no 0.0 exacto, para no confundirlo con "sin dato"
  }
  cerosSeguidos = 0;
  ultimoRumboLeido = e.orientation.x;
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

// Anota el peor desvio del tramo. Va ACA y no arriba junto a desvioPeor[]
// porque en un .ino el IDE genera los prototipos de todas las funciones y
// los mete antes de la primera que encuentre: definida mas arriba, el
// prototipo de fallar(Error) quedaba antes del `enum Error` y no compilaba.
// 🚨 NO LLAMA A rumboActual(). Usa el valor que ya leyo la correccion de
// rumbo en esta misma vuelta.
//
// Cuando esta funcion se escribio (2026-10-06) leia el giroscopio de nuevo,
// y con eso DUPLICABA las lecturas I2C del lazo de movimiento. Cada lectura
// del BNO055 son un par de milisegundos: en un lazo de control, eso es
// retardo, y el retardo hace oscilar aunque las ganancias esten bien.
// Medir no puede cambiar lo que se mide.
void anotarDesvio(int n) {
  if (n < 0 || n > 3 || rumboBase < 0 || ultimoRumboLeido < 0) return;
  float d = fabs(diferencia(rumboBase, ultimoRumboLeido));
  if (d > desvioPeor[n]) desvioPeor[n] = d;
}

// Cuanto sumarle a las TRES ruedas para que el robot no gire.
// Proporcional + acumulada con tope (medido el 04/08: solo proporcional
// dejaba 7 grados de error fijo).
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

  // ⚠️ ACA HUBO UN PISO (PWM_MIN_CORRECCION) Y FUE UN ERROR. Ver el bloque
  // de esa constante, mas arriba.
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
    Serial.print("arrancando, buscando el giroscopio (intento ");
    Serial.print(intentoGiro); Serial.println(" de 10)");
    return;
  }
  switch (fase) {
    case CUENTA:    Serial.println("cuenta de arranque / esperando que el giroscopio se calibre"); break;
    case MOVIENDO:
    case FRENANDO:
      Serial.print("MIDIENDO, tramo "); Serial.print(tramo + 1);
      Serial.println(" de 4"); break;
    case ENDEREZANDO:
      Serial.print("enderezandose despues del golpe, antes del tramo ");
      Serial.println(tramo + 2); break;
    case TERMINADO: Serial.println("TERMINO BIEN. La medicion de arriba es la que acaba de hacer."); break;
    case ERROR:
      Serial.print("ERROR, no se guardo nada: ");
      if (error == ERR_SOBRE_LINEA) {
        Serial.println("arranco con un sensor de atras sobre una linea.");
      } else if (error == ERR_SIN_GIRO) {
        Serial.println("sin giroscopio.");
        Serial.println("       (si la bateria esta APAGADA es lo esperado: el");
        Serial.println("        giroscopio se alimenta de ahi y el robot no se mueve)");
      } else if (error == ERR_SIN_LINEA) {
        Serial.print("no encontro la linea a tiempo, en el tramo ");
        Serial.print(tramo + 1); Serial.println(" (freno de emergencia).");
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
  Serial.println("=============== MEDIR-ANCHO ===============");
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
  // El peor desvio de cada tramo: es lo que dice si oscilo o no.
  if (desvioPeor[0] > 0 || desvioPeor[1] > 0
      || desvioPeor[2] > 0 || desvioPeor[3] > 0) {
    Serial.print("  peor desvio por tramo (grados): ");
    for (int i = 0; i < 4; i++) {
      Serial.print(desvioPeor[i], 1);
      if (i < 3) Serial.print("  ");
    }
    Serial.println();
    float peor = 0;
    for (int i = 0; i < 4; i++) if (desvioPeor[i] > peor) peor = desvioPeor[i];
    if (vecesQueSeEnderezo > 0) {
      Serial.print("  se tuvo que enderezar "); Serial.print(vecesQueSeEnderezo);
      Serial.print(" vez/veces; el peor golpe lo dejo ");
      Serial.print(peorGolpe, 1); Serial.println(" grados torcido");
      Serial.println("    (eso es el choque contra el borde: si es mucho, el");
      Serial.println("     robot llega demasiado rapido y hay que bajar VEL_MEDIR)");
    } else {
      Serial.println("  no hizo falta enderezarlo en ningun momento");
    }
    if (peor > 10.0) {
      Serial.println("    -> MAS DE 10 GRADOS: oscilo de verdad, el numero medido");
      Serial.println("       no sirve (fue zigzagueando, no derecho).");
    } else if (peor > 5.0) {
      Serial.println("    -> entre 5 y 10 grados: se movio bastante, mirarlo.");
    } else {
      Serial.println("    -> menos de 5 grados: fue derecho.");
    }
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

void fallar(Error e) {
  frenar();
  error = e;
  fase = ERROR; t_fase = millis();
}

void guardar() {
  medicion.marca = MARCA_ANCHO;
  medicion.velocidad = VEL_MEDIR;
  medicion.conGiroscopio = 1;            // este programa no mide sin el
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

  // El giroscopio tarda en despertar: se insiste (igual que el juego).
  while (!hayGiroscopo && intentoGiro < 10) {
    intentoGiro++;
    informarSiToca();
    hayGiroscopo = bno.begin();
    if (!hayGiroscopo) delay(300);
  }
  if (!hayGiroscopo) { buscandoGiro = false; fallar(ERR_SIN_GIRO); return; }

  // ⚠️ EL ORDEN IMPORTA, Y LO TENIAMOS AL REVES (lo encontro el delantero).
  // setExtCrystalUse() pasa el chip a modo CONFIG y lo vuelve a sacar, o sea
  // que REINICIA LA FUSION. Esperar antes de llamarlo es tirar ese tiempo a
  // la basura: la cuenta arranca de nuevo igual. Primero el cristal,
  // despues la espera.
  bno.setExtCrystalUse(true);
  delay(700);

  // Y ahora se le pregunta al chip si de verdad esta fusionando, en vez de
  // deducirlo de los angulos. Si arranco mal, mejor saberlo aca.
  t_chequeoFusion = 0;
  if (!preguntarSiFusiona()) {
    // Puede tardar un poco mas en arrancar la fusion: se le da tiempo.
    unsigned long tf = millis();
    while (!preguntarSiFusiona() && millis() - tf < MS_ESPERA_DATOS_GIRO) {
      informarSiToca();
      delay(50);
    }
  }
  fusionCorriendo = preguntarSiFusiona();
  buscandoGiro = false;
  if (!fusionCorriendo) { fallar(ERR_SIN_GIRO); return; }

  // Saludar no alcanza: hay que esperar a que DE UN DATO.
  unsigned long t0 = millis();
  while (rumboActual() < 0 && millis() - t0 < MS_ESPERA_DATOS_GIRO) {
    informarSiToca();
    delay(50);
  }
  if (rumboActual() < 0) { fallar(ERR_SIN_GIRO); return; }

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
        uint8_t sis = 0, giro = 0, acel = 0, mag = 0;
        bno.getCalibration(&sis, &giro, &acel, &mag);
        if (giro < 3 && ahora - t_fase < MS_CUENTA + MS_MAX_CALIBRAR_GIRO) break;

        // Si ya esta sobre una linea, "llegar al costado" no significaria
        // nada: mejor avisar que medir mal.
        if (veBlancoIzq() || veBlancoDer()) { fallar(ERR_SOBRE_LINEA); break; }
        // El rumbo a sostener se fija aca, con el robot quieto.
        float r = rumboActual();
        if (r < 0) { fallar(ERR_SIN_GIRO); break; }
        rumboBase = r;
        empezarTramo(0);
      }
      break;

    case MOVIENDO: {
      digitalWrite(LED, HIGH);
      moverDeCostado(dirTramo * VEL_MEDIR);   // tambien lee el giroscopio
      anotarDesvio(tramo);

      if (giroscopoCaido()) { fallar(ERR_SIN_GIRO); break; }

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
      // Antes de seguir: ¿quedo torcido por el golpe contra el borde?
      if (tramo + 1 < TRAMOS && hayGiroscopo && rumboBase >= 0) {
        float r = rumboActual();
        if (r >= 0) {
          float err = fabs(diferencia(rumboBase, r));
          if (err > fabs(peorGolpe)) peorGolpe = err;
          if (err > TOLERANCIA_ENDEREZAR) {
            vecesQueSeEnderezo++;
            fase = ENDEREZANDO; t_fase = ahora;
            break;
          }
        }
      }
      if (tramo + 1 < TRAMOS) {
        empezarTramo(tramo + 1);
      } else {
        guardar();                             // recien aca, todo salio bien
        fase = TERMINADO; t_fase = ahora;
        informe();                             // y avisa enseguida
      }
      break;

    case ENDEREZANDO: {
      // Gira en el lugar: las tres ruedas al mismo sentido. El signo es el
      // mismo que usa correccionDeRumbo(), que ya esta probado.
      digitalWrite(LED, ((ahora / 150) % 2) ? HIGH : LOW);   // parpadeo medio
      float r = rumboActual();
      bool listo = false;

      if (r < 0) {
        listo = true;                    // sin rumbo no se puede enderezar
      } else {
        float err = diferencia(rumboBase, r);
        if (fabs(err) <= TOLERANCIA_ENDEREZAR) listo = true;
        else {
          int pwm = (int)(fabs(err) * KP_RUMBO);
          if (pwm > PWM_MAX_ENDEREZAR) pwm = PWM_MAX_ENDEREZAR;
          if (pwm < PWM_MIN_ENDEREZAR) pwm = PWM_MIN_ENDEREZAR;
          int c = (err > 0) ? pwm : -pwm;
          aplicar(c, c, c);
        }
      }
      // Tope de tiempo: si no llega, mejor seguir que quedarse trabado.
      if (ahora - t_fase >= MS_MAX_ENDEREZAR) listo = true;

      if (listo) {
        frenar();                        // frenar, no soltar: si no se pasa
        delay(MS_FRENO);
        parar();
        delay(MS_QUIETO);
        empezarTramo(tramo + 1);
      }
      break;
    }

    case TERMINADO:
      parar();
      // Latido lento: un destello corto cada 2 segundos.
      digitalWrite(LED, ((ahora % 2000) < 120) ? HIGH : LOW);
      break;

    case ERROR:
      if (ahora - t_fase < MS_FRENO) frenar();
      else parar();
      destellos((int)error, ahora);
      break;
  }
}
