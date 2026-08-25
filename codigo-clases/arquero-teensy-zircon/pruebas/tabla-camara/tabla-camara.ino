/* =====================================================================
   TABLA DE CONVERSION DE LA CAMARA — cuanto exagera, y si exagera parejo
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-25
   =====================================================================

   PARA QUE SIRVE
   La camara no mide distancia: ve un punto en la imagen y lo traduce a
   centimetros con una tabla de conversion calibrada en 2025 para una
   camara a 18,7 cm de altura. Si desde entonces la camara se movio o se
   inclino, esa traduccion quedo mal.

   Medido con regla el 2026-08-11: cuando la camara dice 48, la pelota
   esta a 12 cm de VERDAD. Exagera por cuatro.

   Este programa toma las cinco mediciones que hacen falta para saber
   CUANTO exagera y —lo mas importante— SI EXAGERA PAREJO.

        Si el factor sale parejo    -> cambio la ALTURA de la camara.
                                       Se arregla con una multiplicacion.
        Si el factor CRECE con la
        distancia                   -> cambio la INCLINACION.
                                       Hace falta una tabla completa.

   Las dos hipotesis explican igual de bien lo que ya vimos (que exagera).
   Por eso hace falta medir mas de un punto: con uno solo no se separan.
   Es el mismo motivo por el que el 04/08 hicieron falta TRES saltos para
   calibrar la distancia, y no uno.

   ---------------------------------------------------------------------
   ESTE PROGRAMA NO MUEVE EL ROBOT
   ---------------------------------------------------------------------
   Los motores se apagan en setup() y no se vuelven a tocar. Se puede
   prender arriba de la mesa sin que se escape.

   ---------------------------------------------------------------------
   COMO SE USA — sin computadora, en la cancha
   ---------------------------------------------------------------------
   El cable USB no llega a la cancha, asi que el robot mide SOLO, se
   acuerda de todo, y despues se le pregunta en la mesa.

   1. Marcar en el piso, con cinta, cinco posiciones para la pelota:
      10, 20, 30, 40 y 50 cm, medidas con regla DESDE EL FRENTE DEL
      CHASIS del robot. Anotar desde donde se midio.
   2. Apoyar el robot mirando las marcas y prender la bateria.
   3. Seguir al LED (abajo esta el idioma del LED).
   4. Cuando el LED quede LATIENDO despacio, termino.
   5. LLEVARLO A LA COMPU CON LA BATERIA PRENDIDA. Si se corta la
      bateria SE BORRA TODO — esto vive en la memoria de trabajo del
      Teensy, que no se guarda sola.
   6. Enchufar el USB y mandar la tecla 'm'.

   ---------------------------------------------------------------------
   EL IDIOMA DEL LED
   ---------------------------------------------------------------------
        N destellos lentos     -> "viene la posicion N"
                                  1=10cm  2=20cm  3=30cm  4=40cm  5=50cm
                                  6 destellos = SACAR LA PELOTA (control)
        LED apagado            -> acomoda la pelota en esa marca
        parpadeo rapido        -> faltan 2 segundos, SACA LA MANO
        LED fijo prendido      -> MIDIENDO, no te muevas
        latido lento           -> termino todo, llevame a la compu

   ---------------------------------------------------------------------
   POR QUE HAY UNA MEDICION "SIN PELOTA" AL FINAL
   ---------------------------------------------------------------------
   Es el control. Sin pelota la camara TIENE que decir que no ve nada.
   Si igual manda un numero, es que estaba mirando otra cosa naranja —
   y entonces ninguna de las cinco mediciones sirve.

   La camara se queda con la mancha naranja MAS GRANDE que ve, no con la
   mas parecida a una pelota. Una silla naranja al fondo le gana a la
   pelota aunque este mucho mas lejos.

   Un test que no puede fallar no prueba nada. Este puede fallar, y por
   eso vale.

   ---------------------------------------------------------------------
   TECLAS (en la mesa, con el USB puesto)
   ---------------------------------------------------------------------
        m = MOSTRAR LA TABLA (esto es lo que venimos a buscar)
        v = ver lo que la camara manda AHORA, en vivo
        r = borrar todo y empezar de nuevo
        s = saltear a la posicion siguiente
        ? = ayuda
   ===================================================================== */

#include <Arduino.h>

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

// ---- las posiciones a medir ----
const int POSICIONES = 5;
const int REALES[POSICIONES] = { 10, 20, 30, 40, 50 };   // cm con regla
const int RANURAS = POSICIONES + 1;                      // +1 el control

// ---- tiempos ----
const unsigned long MS_CUENTA_INICIAL = 10000;  // para apoyarlo y alejarse
const unsigned long MS_PREPARAR       = 10000;  // acomodar la pelota
const unsigned long MS_AVISO_FINAL    =  2000;  // parpadeo: saca la mano
const unsigned long MS_MIDIENDO       =  3000;  // ventana de medicion
const unsigned long MS_DESTELLO       =   250;  // medio destello del anuncio

// ---- lo que se guarda de cada posicion ----
struct Medicion {
  int  paquetes;     // paquetes de camara que llegaron en la ventana
  int  conCero;      // cuantos venian con Xp == 0 ("no veo la pelota")
  int  validos;      // cuantos traian una pelota de verdad
  int  xMin, xMax;
  long xSuma;
  int  yMin, yMax;
  long ySuma;
};
Medicion datos[RANURAS];

int  ranura = 0;
bool midiendo = false;      // true solo durante la ventana de medicion

void borrarDatos() {
  for (int i = 0; i < RANURAS; i++) {
    datos[i].paquetes = 0;
    datos[i].conCero  = 0;
    datos[i].validos  = 0;
    datos[i].xMin =  9999; datos[i].xMax = -9999; datos[i].xSuma = 0;
    datos[i].yMin =  9999; datos[i].yMax = -9999; datos[i].ySuma = 0;
  }
}

// ---- camara: mismo protocolo de 9 bytes que el resto de los sketches ----
byte paquete[9];
int  cuantos = 0;
bool sincronizado = false;
int  Xp = 0, Yp = 0;
unsigned long t_ultimoPaquete = 0;

// Se anota CADA paquete, adentro del lector. Si se anotara desde loop()
// se perderian los paquetes que llegan de a dos entre vuelta y vuelta.
void anotar(int x, int y) {
  Medicion &d = datos[ranura];
  d.paquetes++;
  if (x == 0) { d.conCero++; return; }   // 0 = "no veo la pelota"
  d.validos++;
  d.xSuma += x;
  if (x < d.xMin) d.xMin = x;
  if (x > d.xMax) d.xMax = x;
  d.ySuma += y;
  if (y < d.yMin) d.yMin = y;
  if (y > d.yMax) d.yMax = y;
}

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
      Xp = paquete[1];
      Yp = paquete[2] - 100;
      t_ultimoPaquete = millis();
      if (midiendo) anotar(Xp, Yp);
    }
  }
}

// ---- estado ----
enum Fase { CUENTA_INICIAL, ANUNCIO, PREPARAR, MIDIENDO, LISTO };
Fase fase = CUENTA_INICIAL;
unsigned long t_fase = 0;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void nombreRanura(int i) {
  if (i < POSICIONES) { Serial.print(REALES[i]); Serial.print(" cm"); }
  else                  Serial.print("SIN PELOTA (control)");
}


// ---------------------------------------------------------------- tabla

void mostrarTabla() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" TABLA DE CONVERSION DE LA CAMARA");
  Serial.println("=========================================================");
  Serial.println("   real | paq | Xp=0 | min | max | PROMEDIO | FACTOR | Yprom");
  Serial.println("  ------+-----+------+-----+-----+----------+--------+------");

  for (int i = 0; i < POSICIONES; i++) {
    Medicion &d = datos[i];
    Serial.print("   ");
    if (REALES[i] < 10) Serial.print(" ");
    Serial.print(REALES[i]); Serial.print("cm | ");
    Serial.print(d.paquetes); Serial.print(" | ");
    Serial.print(d.conCero);  Serial.print(" | ");

    if (d.paquetes == 0) {
      Serial.println("  NO LLEGO NINGUN PAQUETE DE LA CAMARA");
      continue;
    }
    if (d.validos == 0) {
      Serial.println("  NUNCA VIO LA PELOTA en esta posicion");
      continue;
    }

    float xProm  = (float)d.xSuma / d.validos;
    float yProm  = (float)d.ySuma / d.validos;
    float factor = xProm / (float)REALES[i];

    Serial.print(d.xMin);   Serial.print(" | ");
    Serial.print(d.xMax);   Serial.print(" |   ");
    Serial.print(xProm, 1); Serial.print("   |  ");
    Serial.print(factor, 2); Serial.print("  | ");
    Serial.println(yProm, 1);
  }

  // ---- el control ----
  Serial.println("  ------+-----+------+-----+-----+----------+--------+------");
  Medicion &c = datos[POSICIONES];
  Serial.print("   CONTROL (sin pelota): ");
  Serial.print(c.paquetes); Serial.print(" paquetes, ");
  Serial.print(c.validos);  Serial.println(" con algo naranja a la vista");

  if (c.paquetes == 0) {
    Serial.println("   ?? no llego nada de la camara — no se puede validar");
  } else if (c.validos == 0) {
    Serial.println("   OK: sin pelota no vio nada. Las mediciones sirven.");
  } else {
    Serial.println("   !! VIO ALGO SIN PELOTA. Habia otra cosa naranja:");
    Serial.print("      leia entre "); Serial.print(c.xMin);
    Serial.print(" y ");              Serial.println(c.xMax);
    Serial.println("      Las cinco mediciones quedan en duda. Repetir con 'r'.");
  }

  // ---- que quiere decir la columna FACTOR ----
  Serial.println("=========================================================");
  Serial.println(" COMO SE LEE LA COLUMNA 'FACTOR'");
  Serial.println("   parejo en las 5 filas  -> cambio la ALTURA");
  Serial.println("                             se arregla multiplicando");
  Serial.println("   crece con la distancia -> cambio la INCLINACION");
  Serial.println("                             hace falta la tabla entera");
  Serial.println("=========================================================");
}

void ayuda() {
  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" TABLA DE CONVERSION DE LA CAMARA — el robot NO se mueve");
  Serial.println("=========================================================");
  Serial.println("  m = MOSTRAR LA TABLA      v = ver la camara en vivo");
  Serial.println("  r = borrar y empezar de nuevo");
  Serial.println("  s = saltear a la posicion siguiente");
  Serial.println("---------------------------------------------------------");
  Serial.println("  LED: N destellos = viene la posicion N");
  Serial.println("       1=10cm 2=20cm 3=30cm 4=40cm 5=50cm 6=SIN PELOTA");
  Serial.println("       apagado = acomoda la pelota");
  Serial.println("       parpadeo rapido = faltan 2 s, saca la mano");
  Serial.println("       fijo = MIDIENDO      latido lento = termino");
  Serial.println("---------------------------------------------------------");
  Serial.println("  Traelo a la compu CON LA BATERIA PRENDIDA.");
  Serial.println("  Si se corta la bateria se borra todo.");
  Serial.println("=========================================================");
}

void verEnVivo() {
  if (millis() - t_ultimoPaquete > 1000) {
    Serial.println("   la camara no manda nada");
    return;
  }
  if (Xp == 0) {
    Serial.println("   la camara anda, pero no ve la pelota (Xp = 0)");
  } else {
    Serial.print("   Xp = "); Serial.print(Xp);
    Serial.print("   Yp = "); Serial.println(Yp);
  }
}

void arrancarDeCero() {
  borrarDatos();
  ranura = 0;
  midiendo = false;
  fase = CUENTA_INICIAL;
  t_fase = millis();
  Serial.println();
  Serial.println(">> 10 segundos para apoyarlo y alejarse.");
  Serial.println("   Despues segui al LED.");
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
      if (fase == LISTO) {
        Serial.println("   ya termino. 'r' para repetir todo.");
        break;
      }
      midiendo = false;
      ranura++;
      if (ranura >= RANURAS) {
        fase = LISTO; t_fase = millis();
        Serial.println("   salteado. No queda ninguna mas.");
      } else {
        fase = ANUNCIO; t_fase = millis();
        Serial.print("   salteo a: "); nombreRanura(ranura); Serial.println();
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

  ayuda();
  arrancarDeCero();
}


void loop() {
  leerConsola();
  leerCamara();
  unsigned long ahora = millis();

  switch (fase) {

    case CUENTA_INICIAL: {
      // Parpadeo lento; en los ultimos 3 s se apura, igual que los otros
      // sketches que arrancan solos.
      unsigned long falta = (ahora - t_fase >= MS_CUENTA_INICIAL)
                            ? 0 : MS_CUENTA_INICIAL - (ahora - t_fase);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);
      if (ahora - t_fase >= MS_CUENTA_INICIAL) {
        fase = ANUNCIO; t_fase = ahora;
      }
      break;
    }

    case ANUNCIO: {
      // N destellos y despues medio segundo de oscuridad, para que se
      // note donde termina la cuenta.
      unsigned long transcurrido = ahora - t_fase;
      unsigned long largoCuenta  = (unsigned long)(ranura + 1) * 2 * MS_DESTELLO;
      if (transcurrido < largoCuenta) {
        digitalWrite(LED, ((transcurrido / MS_DESTELLO) % 2) ? LOW : HIGH);
      } else {
        digitalWrite(LED, LOW);
        if (transcurrido >= largoCuenta + 500) {
          Serial.print(">> posicion "); Serial.print(ranura + 1);
          Serial.print(" de ");         Serial.print(RANURAS);
          Serial.print(": ");           nombreRanura(ranura);
          Serial.println("  — acomoda y sali de la vista");
          fase = PREPARAR; t_fase = ahora;
        }
      }
      break;
    }

    case PREPARAR: {
      unsigned long transcurrido = ahora - t_fase;
      if (transcurrido < MS_PREPARAR - MS_AVISO_FINAL) {
        digitalWrite(LED, LOW);                                // acomoda tranquilo
      } else {
        digitalWrite(LED, ((ahora / 100) % 2) ? HIGH : LOW);   // saca la mano
      }
      if (transcurrido >= MS_PREPARAR) {
        fase = MIDIENDO; t_fase = ahora;
        midiendo = true;
        Serial.println("   MIDIENDO...");
      }
      break;
    }

    case MIDIENDO: {
      digitalWrite(LED, HIGH);
      // Los paquetes los anota leerCamara(), para no perder ninguno.
      if (ahora - t_fase >= MS_MIDIENDO) {
        midiendo = false;
        Medicion &d = datos[ranura];
        Serial.print("   listo: ");   Serial.print(d.paquetes);
        Serial.print(" paquetes, ");  Serial.print(d.validos);
        Serial.println(" con pelota");
        ranura++;
        if (ranura >= RANURAS) {
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
      // Latido lento: un destello corto cada 2 segundos.
      digitalWrite(LED, ((ahora % 2000) < 120) ? HIGH : LOW);
      break;
  }
}
