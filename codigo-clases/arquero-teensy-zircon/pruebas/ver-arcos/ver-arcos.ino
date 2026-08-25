/* =====================================================================
   VER-ARCOS — ¿sirve el arco azul para centrarse?
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-25
   =====================================================================

   PARA QUE SIRVE
   El arquero se para con SU arco a la espalda, asi que la camara nunca lo
   ve. Lo que si ve es el arco del RIVAL, al fondo de la cancha — y como
   los dos arcos estan sobre la misma linea central, centrarse con el de
   enfrente es lo mismo que centrarse con el propio.

   Antes de escribir ese movimiento hay que contestar tres preguntas que
   NO se pueden deducir leyendo el codigo:

     1. ¿Yaz cambia de signo al pasar por el centro?
        Si no cruza el cero, no sirve como referencia.

     2. ¿Que signo es cada lado?
        Con la pelota ya nos paso: Yp negativo resulto ser la DERECHA, al
        reves de lo que suponia el codigo. Si nos equivocamos aca, el
        robot se va a alejar del centro en vez de acercarse.

     3. ¿Yaz es estable, o salta?
        Si salta, el robot va a temblar buscando el centro y hay que
        filtrarlo.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar. Al robot lo
   mueven ustedes, a mano.

   ---------------------------------------------------------------------
   COMO SE USA — en la cancha, sin computadora
   ---------------------------------------------------------------------
   1. Marcar tres lugares sobre la linea del area: uno corrido a la
      IZQUIERDA (~30 cm), el CENTRO del arco, y uno a la DERECHA (~30 cm).
      Izquierda y derecha VISTAS DESDE EL ROBOT, no desde ustedes.
   2. Apoyar el robot en el primero, mirando la cancha, y prender la
      bateria.
   3. Seguir al LED. En cada posicion, apoyar el robot MIRANDO AL FRENTE,
      igual que las otras dos. Que no quede girado: si lo giran, Yaz
      cambia por el giro y no por el lugar, y la medicion no sirve.
   4. Cuando el LED quede latiendo despacio, termino.
   5. Traerlo a la compu CON LA BATERIA PRENDIDA y mandar la tecla 'm'.

   ⚠️ El programa anota el RUMBO de cada posicion justamente para poder
   darse cuenta de si el robot quedo girado entre una y otra. Si los tres
   rumbos son parecidos, las tres mediciones se pueden comparar.

   ---------------------------------------------------------------------
   EL IDIOMA DEL LED
   ---------------------------------------------------------------------
        parpadeo lento         -> arrancando, alejate
        parpadeo rapido        -> faltan 3 segundos
        (oscuridad larga)      -> ahora viene el numero de posicion
        N destellos LARGOS     -> "viene la posicion N"
                                  1 = IZQUIERDA   2 = CENTRO   3 = DERECHA
        LED apagado            -> acomoda el robot en esa marca
        parpadeo rapido        -> faltan 2 segundos, sacale las manos
        LED fijo prendido      -> MIDIENDO, no lo toques
        latido lento           -> termino, llevame a la compu

   🔧 Corregido respecto de `tabla-camara`: ahi el primer anuncio salia
   pegado al parpadeo del arranque, no se veia, y toda la tabla quedo
   corrida un lugar. Aca hay 1,5 s de oscuridad antes del primer anuncio
   y los destellos son mas largos, para poder contarlos.

   ---------------------------------------------------------------------
   EL LED DE LA CAMARA (no el del robot) tambien avisa
   ---------------------------------------------------------------------
   Es un solo LED de tres canales:
        rojo = ve la pelota        verde = ve el arco AMARILLO
        azul = ve el arco AZUL     y los colores se MEZCLAN
   Si durante la medicion el LED de la camara no tiene nada de azul, el
   arco no se esta viendo y esa fila va a salir vacia.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        m = MOSTRAR LA TABLA        v = ver los arcos AHORA, en vivo
        r = borrar y empezar de nuevo
        s = saltear a la posicion siguiente
        ? = ayuda
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define LED 13

// Motores: se apagan una vez en setup() y no se tocan nunca mas.
#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

const unsigned long BAUDIOS = 19200;

// ---- las tres posiciones ----
const int PUESTOS = 3;
const char* NOMBRES[PUESTOS] = { "IZQUIERDA", "CENTRO", "DERECHA" };

// ---- tiempos ----
const unsigned long MS_CUENTA_INICIAL = 10000;
const unsigned long MS_OSCURIDAD      =  1500;  // separa la cuenta del anuncio
const unsigned long MS_DESTELLO_ON    =   400;  // destellos largos: se cuentan
const unsigned long MS_DESTELLO_OFF   =   300;
const unsigned long MS_PREPARAR       = 12000;  // mover el robot lleva mas que mover la pelota
const unsigned long MS_AVISO_FINAL    =  2000;
const unsigned long MS_MIDIENDO       =  3000;

// ---- lo que se guarda de cada arco en cada posicion ----
struct Vista {
  int  vistas;               // paquetes en los que el arco aparecia
  int  xMin, xMax;  long xSuma;
  int  yMin, yMax;  long ySuma;
};

struct Puesto {
  int   paquetes;            // paquetes totales de la ventana
  Vista azul;
  Vista amarillo;
  float rumbo;               // para saber si el robot quedo girado
};
Puesto datos[PUESTOS];

int  puesto = 0;
bool midiendo = false;

void borrarVista(Vista &v) {
  v.vistas = 0;
  v.xMin =  9999; v.xMax = -9999; v.xSuma = 0;
  v.yMin =  9999; v.yMax = -9999; v.ySuma = 0;
}

void borrarDatos() {
  for (int i = 0; i < PUESTOS; i++) {
    datos[i].paquetes = 0;
    datos[i].rumbo = -1;
    borrarVista(datos[i].azul);
    borrarVista(datos[i].amarillo);
  }
}

// ---- giroscopio ----
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool hayGiroscopo = false;

// Devuelve el rumbo, o -1 si el sensor esta mudo. Un numero imposible en
// vez de 0 evita la confusion de siempre: 0.0 es a la vez un rumbo valido
// y el sintoma de que el chip no contesta.
float rumboActual() {
  if (!hayGiroscopo) return -1;
  sensors_event_t e;
  bno.getEvent(&e);
  if (e.orientation.x == 0.0 && e.orientation.y == 0.0
      && e.orientation.z == 0.0) return -1;
  return e.orientation.x;
}

// ---- camara: 9 bytes, tres grupos de tres ----
//   [201, Xp, Yp+100,  202, Xam, Yam+100,  203, Xaz, Yaz+100]
//    pelota            arco AMARILLO        arco AZUL
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0,  Yp = 0;
int  Xam = 0, Yam = 0;
int  Xaz = 0, Yaz = 0;
unsigned long t_ultimoPaquete = 0;

void anotarVista(Vista &v, int x, int y) {
  if (x == 0) return;              // 0 = no lo veo
  v.vistas++;
  v.xSuma += x;
  if (x < v.xMin) v.xMin = x;
  if (x > v.xMax) v.xMax = x;
  v.ySuma += y;
  if (y < v.yMin) v.yMin = y;
  if (y > v.yMax) v.yMax = y;
}

// Se anota adentro del lector para no perder paquetes: entre vuelta y
// vuelta del loop puede llegar mas de uno.
void leerCamara() {
  while (Serial1.available()) {
    byte b = Serial1.read();
    if (!sincronizado) {
      if (b == 201) { sincronizado = true; paquete[0] = b; cuantos = 1; }
      continue;
    }
    paquete[cuantos++] = b;
    if (cuantos < 9) continue;

    cuantos = 0; sincronizado = false;
    if (paquete[0] == 201 && paquete[3] == 202 && paquete[6] == 203) {
      Xp  = paquete[1];  Yp  = paquete[2] - 100;
      Xam = paquete[4];  Yam = paquete[5] - 100;
      Xaz = paquete[7];  Yaz = paquete[8] - 100;
      t_ultimoPaquete = millis();

      if (midiendo) {
        datos[puesto].paquetes++;
        anotarVista(datos[puesto].azul,     Xaz, Yaz);
        anotarVista(datos[puesto].amarillo, Xam, Yam);
      }
    }
  }
}

// ---- estado ----
enum Fase { CUENTA_INICIAL, OSCURIDAD, ANUNCIO, PREPARAR, MIDIENDO, LISTO };
Fase fase = CUENTA_INICIAL;
unsigned long t_fase = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}


// ---------------------------------------------------------------- tabla

void filaDeArco(const char* etiqueta, Vista &v, int paquetes) {
  Serial.print("     "); Serial.print(etiqueta); Serial.print(": ");
  if (paquetes == 0) { Serial.println("no llego ningun paquete"); return; }
  if (v.vistas == 0) { Serial.println("NO LO VIO NUNCA"); return; }

  float xProm = (float)v.xSuma / v.vistas;
  float yProm = (float)v.ySuma / v.vistas;

  Serial.print("visto en ");  Serial.print(v.vistas);
  Serial.print("/");          Serial.print(paquetes);
  Serial.print("   Y = ");    Serial.print(yProm, 1);
  Serial.print("  [");        Serial.print(v.yMin);
  Serial.print(" a ");        Serial.print(v.yMax);
  Serial.print("]  salto ");  Serial.print(v.yMax - v.yMin);
  Serial.print("   X = ");    Serial.println(xProm, 1);
}

void mostrarTabla() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" LOS ARCOS VISTOS DESDE TRES LUGARES DEL AREA");
  Serial.println("=========================================================");

  for (int i = 0; i < PUESTOS; i++) {
    Serial.print("  "); Serial.print(i + 1); Serial.print(". ");
    Serial.print(NOMBRES[i]);
    Serial.print("   (rumbo ");
    if (datos[i].rumbo < 0) Serial.print("GIROSCOPIO MUDO");
    else                    Serial.print(datos[i].rumbo, 1);
    Serial.println(")");
    filaDeArco("ARCO AZUL   ", datos[i].azul,     datos[i].paquetes);
    filaDeArco("arco amarillo", datos[i].amarillo, datos[i].paquetes);
  }

  Serial.println("---------------------------------------------------------");
  Serial.println(" QUE MIRAR");
  Serial.println("  1. La Y del ARCO AZUL, ¿cambia de signo entre");
  Serial.println("     IZQUIERDA y DERECHA, y da cerca de 0 en el CENTRO?");
  Serial.println("     Si si -> sirve para centrarse.");
  Serial.println("  2. ¿Que signo dio la IZQUIERDA? Ese es el dato que no");
  Serial.println("     se puede deducir leyendo el codigo.");
  Serial.println("  3. La columna 'salto' es cuanto se movio la lectura");
  Serial.println("     estando el robot QUIETO. Si es chica, es estable.");
  Serial.println("  4. Los tres rumbos tienen que ser parecidos. Si no, el");
  Serial.println("     robot quedo girado y la comparacion no vale.");
  Serial.println("=========================================================");
}

void ayuda() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" VER-ARCOS — el robot NO se mueve, lo mueven ustedes");
  Serial.println("=========================================================");
  Serial.println("  m = MOSTRAR LA TABLA     v = ver los arcos en vivo");
  Serial.println("  r = borrar y empezar de nuevo");
  Serial.println("  s = saltear a la posicion siguiente");
  Serial.println("---------------------------------------------------------");
  Serial.println("  Tres lugares sobre la linea del area:");
  Serial.println("    1 destello  = IZQUIERDA (~30 cm del centro)");
  Serial.println("    2 destellos = CENTRO del arco");
  Serial.println("    3 destellos = DERECHA (~30 cm del centro)");
  Serial.println("  Izquierda y derecha VISTAS DESDE EL ROBOT.");
  Serial.println("  En las tres, el robot MIRANDO AL FRENTE igual.");
  Serial.println("---------------------------------------------------------");
  Serial.println("  Traelo a la compu CON LA BATERIA PRENDIDA.");
  Serial.println("=========================================================");
}

void verEnVivo() {
  if (millis() - t_ultimoPaquete > 1000) {
    Serial.println("   la camara no manda nada");
    return;
  }
  Serial.print("   pelota  X="); Serial.print(Xp);
  Serial.print(" Y=");           Serial.print(Yp);
  Serial.println(Xp == 0 ? "   (no la ve)" : "");
  Serial.print("   AZUL    X="); Serial.print(Xaz);
  Serial.print(" Y=");           Serial.print(Yaz);
  Serial.println(Xaz == 0 ? "   (NO LO VE)" : "");
  Serial.print("   amarillo X="); Serial.print(Xam);
  Serial.print(" Y=");            Serial.print(Yam);
  Serial.println(Xam == 0 ? "   (no lo ve)" : "");
  Serial.print("   rumbo: ");
  float r = rumboActual();
  if (r < 0) Serial.println("GIROSCOPIO MUDO (¿bateria apagada?)");
  else       Serial.println(r, 1);
}

void arrancarDeCero() {
  borrarDatos();
  puesto = 0;
  midiendo = false;
  fase = CUENTA_INICIAL;
  t_fase = millis();
  Serial.println();
  Serial.println(">> 10 segundos para apoyarlo y alejarse.");
  Serial.println("   Primera posicion: IZQUIERDA.");
}

void leerConsola() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'm': mostrarTabla();   break;
    case 'v': verEnVivo();      break;
    case '?': ayuda();          break;
    case 'r': arrancarDeCero(); break;

    case 's':
      if (fase == LISTO) { Serial.println("   ya termino. 'r' repite."); break; }
      midiendo = false;
      puesto++;
      if (puesto >= PUESTOS) {
        fase = LISTO; t_fase = millis();
        Serial.println("   salteado. No queda ninguna mas.");
      } else {
        fase = ANUNCIO; t_fase = millis();
        Serial.print("   salteo a: "); Serial.println(NOMBRES[puesto]);
      }
      break;

    default:
      Serial.print("   tecla '"); Serial.print(c);
      Serial.println("' no hace nada. ? para la ayuda.");
      break;
  }
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar en todo el programa

  Serial.begin(BAUDIOS);
  Serial1.begin(BAUDIOS);

  // Sin giroscopio la medicion de los arcos sirve igual; lo unico que se
  // pierde es poder chequear que el robot no quedo girado.
  hayGiroscopo = bno.begin();
  if (hayGiroscopo) {
    delay(1000);
    bno.setExtCrystalUse(true);
    Serial.println(">> giroscopio OK");
  } else {
    Serial.println("!! sin giroscopio — no voy a poder avisar si quedo girado");
  }

  ayuda();
  arrancarDeCero();
}


void loop() {
  leerConsola();
  leerCamara();
  unsigned long ahora = millis();

  switch (fase) {

    case CUENTA_INICIAL: {
      unsigned long falta = (ahora - t_fase >= MS_CUENTA_INICIAL)
                            ? 0 : MS_CUENTA_INICIAL - (ahora - t_fase);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_CUENTA_INICIAL) {
        fase = OSCURIDAD; t_fase = ahora;
      }
      break;
    }

    // Oscuridad franca antes del primer anuncio. Sin esto, el destello de
    // la posicion 1 se confunde con el parpadeo rapido del arranque — que
    // es exactamente lo que corrio la tabla entera en `tabla-camara`.
    case OSCURIDAD:
      digitalWrite(LED, LOW);
      if (ahora - t_fase >= MS_OSCURIDAD) { fase = ANUNCIO; t_fase = ahora; }
      break;

    case ANUNCIO: {
      unsigned long ciclo  = MS_DESTELLO_ON + MS_DESTELLO_OFF;
      unsigned long largo  = (unsigned long)(puesto + 1) * ciclo;
      unsigned long t      = ahora - t_fase;
      if (t < largo) {
        digitalWrite(LED, ((t % ciclo) < MS_DESTELLO_ON) ? HIGH : LOW);
      } else {
        digitalWrite(LED, LOW);
        if (t >= largo + MS_OSCURIDAD) {
          Serial.print(">> posicion "); Serial.print(puesto + 1);
          Serial.print(" de ");         Serial.print(PUESTOS);
          Serial.print(": ");           Serial.print(NOMBRES[puesto]);
          Serial.println("  — acomodalo MIRANDO AL FRENTE y solta");
          fase = PREPARAR; t_fase = ahora;
        }
      }
      break;
    }

    case PREPARAR: {
      unsigned long t = ahora - t_fase;
      if (t < MS_PREPARAR - MS_AVISO_FINAL) {
        digitalWrite(LED, LOW);                                // acomodalo
      } else {
        digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);   // sacale las manos
      }
      if (t >= MS_PREPARAR) {
        // El rumbo se anota al EMPEZAR a medir, con el robot ya soltado.
        datos[puesto].rumbo = rumboActual();
        fase = MIDIENDO; t_fase = ahora;
        midiendo = true;
        Serial.println("   MIDIENDO...");
      }
      break;
    }

    case MIDIENDO: {
      digitalWrite(LED, HIGH);
      if (ahora - t_fase >= MS_MIDIENDO) {
        midiendo = false;
        Puesto &d = datos[puesto];
        Serial.print("   listo: ");  Serial.print(d.paquetes);
        Serial.print(" paquetes, arco azul visto ");
        Serial.print(d.azul.vistas); Serial.println(" veces");
        puesto++;
        if (puesto >= PUESTOS) {
          fase = LISTO; t_fase = ahora;
          Serial.println();
          Serial.println(">> TERMINE. Llevame a la compu SIN CORTAR LA BATERIA");
          Serial.println("   y manda la tecla 'm'.");
          mostrarTabla();
        } else {
          fase = ANUNCIO; t_fase = ahora;
        }
      }
      break;
    }

    case LISTO:
      digitalWrite(LED, ((ahora % 2000) < 120) ? HIGH : LOW);
      break;
  }
}
