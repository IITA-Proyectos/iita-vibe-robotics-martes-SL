/* =====================================================================
   PATADA DERECHA — la patada COMPLETA (golpe + retroceso), medida
   IITA Salta — taller de los martes
   Reescrito el 2026-10-13 a pedido del equipo.
   =====================================================================

   PARA QUE SIRVE. Es el banco de pruebas de la patada. Aca se ajusta hasta
   que vaya recta, y RECIEN DESPUES de verla andar se lleva el cambio al
   programa principal (funciona/delantero/delantero.ino). Desde aca el
   principal no se toca.

   QUE HACE. No busca la pelota: espera teclas por el monitor (19200). Con
   la tecla "p" hace lo mismo que el robot en el partido cuando decide
   patear:
        GOLPE       150 x 500 ms, con la rampa, el trim, el corrector
                    KP/KD del giroscopio y la rueda trasera trabada
        RETROCESO   110 x 700 ms, sin freno en el medio
   y al terminar dice cuantos grados se torcio: al final del golpe, el PICO,
   el pico de los primeros 200 ms (que es lo que ve la pelota) y como quedo
   despues del retroceso. Con un grafico, para ver CUANDO se tuerce.

   LOS NUMEROS SE CAMBIAN CON TECLAS, sin recargar: trim, KP, KD, potencia y
   duracion. Cuando la patada salga recta, esos numeros son los que hay que
   llevar al panel de control del principal.

   ⚠ LAS FUNCIONES DE LA PATADA SON COPIA TEXTUAL DEL PRINCIPAL del 13/10
   (motoresPatada, avanzarDerecho, rampaPatada, retroceder, rumboActual y lo
   que hace al entrar a PATEA_ADEL). Estan marcadas con "COPIA DEL PRINCIPAL".
   Si se cambia una ACA y anda, hay que hacer el MISMO cambio ALLA. Y al
   reves: si el principal cambia, esta prueba queda midiendo una patada que
   el robot ya no pega. Ya paso dos veces (22/09 y 06/10).

   QUE NO HACE, a proposito: no mira la linea blanca ni la camara. Hay que
   ponerlo con lugar adelante. Cualquier tecla mientras se mueve lo FRENA.

   🚨 MUEVE EL ROBOT. EN EL PISO O EN LA CANCHA, nunca sobre la mesa.
   🚨 Con el cable enchufado: dejarlo flojo, que el robot avanza y vuelve.

   Se carga con PROBAR-PATADA.bat (carga y abre la consola), o con la
   opcion 12 de MEDIR-TUTI-ROBOT.bat y despues un monitor serie a 19200.
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// Mapeo MEDIDO en banco 2026-07-28 (robot DELANTERO)
#define IZQ_INA 8
#define IZQ_INB 7
#define IZQ_PWM 6
#define DER_INA 11
#define DER_INB 12
#define DER_PWM 4
#define TRA_INA 2
#define TRA_INB 5
#define TRA_PWM 3
#define LED 13

// =====================================================================
//   LOS NUMEROS DE LA PATADA — los mismos del panel del principal (13/10)
//   Estos cinco se cambian con teclas. Arrancan con el valor de partido.
// =====================================================================
int   VEL_PATADA  = 180;   // potencia del golpe            teclas v / V
int   MS_PATADA   = 450;   // cuanto dura el golpe          teclas m / M
int   TRIM_PATADA = 5;     // compensacion fija             teclas t / T
float KP_PATADA   = 6.0;   // corrige por GRADO de desvio   teclas k / K
float KD_PATADA   = 0.6;   // corrige por GRADO/SEGUNDO     teclas d / D

// Estos no se cambian con teclas. Son los del principal.
const int           RESTA_MAX          = 120;
const int           RAMPA_PATADA_PASO  = 15;
const unsigned long RAMPA_PATADA_MS    = 5;
const unsigned long MS_DERIVADA_PATADA = 10;
const int           VEL_RETROCESO      = 110;
const int           MS_RETROCESO       = 700;
const unsigned long MS_CHEQUEO_GIRO    = 500;
const bool          GIRO_SIN_BRUJULA   = true;   // modo IMU, como el principal desde el 12/10

// ---------- interruptores de la prueba (teclas) ----------
bool lazoActivo = true;    // tecla l: false = golpe con el trim solo, sin corrector.
                           // Sirve para ajustar el TRIM: se apaga el corrector y se
                           // mueve el trim hasta que el golpe "crudo" salga derecho.
bool traseraTrabada = true; // tecla b: true = como el principal. false = trasera SUELTA.
bool sinEspera  = false;   // tecla c: ver el bloque "EL CHEQUEO QUE CLAVA EL PROGRAMA"
bool grafico    = true;    // tecla g

// ---------- giroscopio (igual que el principal) ----------
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiroscopo = false;
bool  giroCaido    = false;
float ultimoRumbo  = 0;
float rumboAlPatear = 0;
bool  rumboPatadaValido = false;
unsigned long t_chequeoGiro = 0;

// ---------- estado de la prueba ----------
int   fase = 0;                       // 0 = en espera, 1 = golpe, 2 = retroceso
unsigned long t_entroFase = 0;
unsigned long t_disparo = 0;          // 0 = no hay cuenta regresiva en marcha
unsigned long t_freno   = 0;
int   numeroPatada = 0;
bool  frenadaATecla = false;

// ---------- lo que se anota de cada patada ----------
float finGolpe = 0, pico = 0, pico200 = 0, finRetroceso = 0, picoRetroceso = 0;
unsigned long picoCuando = 0, duroGolpe = 0;
unsigned long huecoMax = 0, huecoCuando = 0, demoraEntrada = 0;
unsigned long msLoopPrevio = 0, t_muestra = 0, t_inicioGolpe = 0;
bool  hizoRetroceso = false, midiendoGolpe = false;
#define MUESTRAS 200                  // una cada 10 ms: alcanza para 2 s
int      nMuestras = 0, nMuestrasGolpe = 0;
uint16_t muestraT[MUESTRAS];
float    muestraErr[MUESTRAS];


// =====================================================================
//                    MOTORES — COPIA DEL PRINCIPAL
// =====================================================================

// SOLTAR los motores. NO es frenar: el robot sigue de largo por inercia.
void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// FRENO ELECTRICO: las mismas patas en 0, pero con el PWM a fondo.
void frenar() {
  digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0); analogWrite(IZQ_PWM, 255);
  digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0); analogWrite(DER_PWM, 255);
  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
}

// COPIA DEL PRINCIPAL. Las dos de adelante empujan y la TRASERA va TRABADA.
//
// Lo unico agregado es el "if (traseraTrabada)" (tecla b): deja comparar el
// mismo golpe con la trasera trabada (como el principal) y suelta (como la
// prueba vieja). Trabarla es de la mejora 3 del 22/09, que se anoto como
// hipotesis ("no deberia frenar el avance") y nunca se midio por separado.
//
// OJO, para no repetir un error: el 13/10 el robot se movio 1 cm en la primera
// corrida de esta prueba y Claude sospecho de esta rueda trabada. NO ERA ESO:
// el equipo encontro que fue un descuido propio (no quedo anotado cual). No hay
// ninguna medicion que diga que trabar la trasera frene el arranque.
void motoresPatada(int vi, int vd) {
  if (vi < 0) vi = 0;   if (vi > 255) vi = 255;
  if (vd < 0) vd = 0;   if (vd > 255) vd = 255;
  analogWrite(IZQ_PWM, vi); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vd); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  if (traseraTrabada) {
    digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0); analogWrite(TRA_PWM, 255);
  } else {
    analogWrite(TRA_PWM, 0);  digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
  }
}

// COPIA DEL PRINCIPAL. Marcha atras: las dos de adelante al reves, trasera suelta.
void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}


// =====================================================================
//                  GIROSCOPIO — COPIA DEL PRINCIPAL
// =====================================================================

// Diferencia mas corta entre dos angulos, en (-180, 180].
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// ---------- EL CHEQUEO QUE CLAVA EL PROGRAMA ----------
// rumboActual() le pregunta al chip cada MS_CHEQUEO_GIRO (500 ms) si su fusion
// sigue corriendo, con bno.getSystemStatus(). Leyendo la libreria (Adafruit
// BNO055 1.6.4, Adafruit_BNO055.cpp) esa funcion TERMINA CON delay(200): son
// 200 ms con el programa entero clavado. Y el golpe dura 500 ms.
//
// En ese rato no se corrige el rumbo: los motores siguen con lo ultimo que se
// les mando. Podria ser lo que el equipo ve como "se tuerce, intenta acomodarse
// pero la pelota ya salio". ES UNA HIPOTESIS LEIDA EN EL CODIGO, NO MEDIDA: esta
// prueba es para medirla. El informe dice si el programa se clavo durante el
// golpe, cuanto y en que momento.
//
// Con la tecla c el chequeo se hace leyendo el registro del chip (0x39,
// SYS_STATUS) directo por I2C, sin la espera. Es el mismo dato. Si con c la
// patada sale mejor, ese es el cambio a llevar al principal.
uint8_t estadoGiroSinEspera() {
  Wire.beginTransmission(0x28);
  Wire.write(0x39);
  if (Wire.endTransmission(false) != 0) return 0;
  if (Wire.requestFrom(0x28, 1) != 1) return 0;
  return Wire.read();
}

// COPIA DEL PRINCIPAL (lo unico agregado es el "if (sinEspera)").
float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);
  ultimoRumbo = evento.orientation.x;        // 0..360

  if (millis() - t_chequeoGiro > MS_CHEQUEO_GIRO) {
    t_chequeoGiro = millis();
    if (sinEspera) {                         // tecla c
      giroCaido = (estadoGiroSinEspera() != 5);
      return ultimoRumbo;
    }
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    giroCaido = (sys != 5);                  // 5 = algoritmo de fusion corriendo
  }
  return ultimoRumbo;
}

bool giroscopoSano() {
  return hayGiroscopo && !giroCaido;
}

// COPIA DEL PRINCIPAL.
bool arrancarGiroscopo() {
  if (!bno.begin(GIRO_SIN_BRUJULA ? OPERATION_MODE_IMUPLUS : OPERATION_MODE_NDOF)) return false;
  bno.setExtCrystalUse(true);
  delay(700);
  uint8_t sys = 0, autotest = 0, err = 0;
  bno.getSystemStatus(&sys, &autotest, &err);
  if (sys != 5) {
    Serial.print("   la fusion NO esta corriendo (SYS_STATUS="); Serial.print(sys);
    Serial.print(", SYS_ERR="); Serial.print(err);
    Serial.println("). Corre pruebas/giroscopo-crudo/ para ver que pasa.");
    return false;
  }
  giroCaido = false;
  t_chequeoGiro = millis();
  return true;
}


// =====================================================================
//           EL GOLPE DERECHO — COPIA DEL PRINCIPAL
// =====================================================================

float         errPatadaPrevio = 0;
unsigned long t_derivPatada   = 0;
float         derivPatada     = 0;

void reiniciarDerivadaPatada() {
  errPatadaPrevio = 0;
  t_derivPatada   = 0;
  derivPatada     = 0;
}

// COPIA DEL PRINCIPAL: trim fijo + proporcional + derivativo, repartido entre
// las dos ruedas (a una le suma y a la otra le resta).
void avanzarDerecho(int vel, float rumboObjetivo) {
  int vi = vel - TRIM_PATADA;
  int vd = vel + TRIM_PATADA;

  float err = diferencia(rumboObjetivo, rumboActual());
  unsigned long ahora = millis();
  if (t_derivPatada == 0) {
    errPatadaPrevio = err;
    t_derivPatada   = ahora;
  } else if (ahora - t_derivPatada >= MS_DERIVADA_PATADA) {
    derivPatada     = (err - errPatadaPrevio) * 1000.0 / (float)(ahora - t_derivPatada);
    errPatadaPrevio = err;
    t_derivPatada   = ahora;
  }
  float mando = KP_PATADA * err + KD_PATADA * derivPatada;

  int mitad = (int)(fabs(mando) / 2.0);
  if (mitad > RESTA_MAX / 2) mitad = RESTA_MAX / 2;
  if (mando > 0) { vd -= mitad; vi += mitad; }
  else           { vi -= mitad; vd += mitad; }

  motoresPatada(vi, vd);
}

// COPIA DEL PRINCIPAL: la rampa de arranque del golpe.
int           pwmRampaPatada = 0;
unsigned long t_rampaPatada  = 0;

void reiniciarRampaPatada() {
  pwmRampaPatada = 0;
  t_rampaPatada  = millis();
}

int rampaPatada(int objetivo) {
  unsigned long ahora = millis();
  if (objetivo < pwmRampaPatada) {
    pwmRampaPatada = objetivo;
  } else if (ahora - t_rampaPatada >= RAMPA_PATADA_MS) {
    t_rampaPatada = ahora;
    pwmRampaPatada += RAMPA_PATADA_PASO;
    if (pwmRampaPatada > objetivo) pwmRampaPatada = objetivo;
  }
  return pwmRampaPatada;
}


// =====================================================================
//                     LA CONSOLA Y EL INFORME
// =====================================================================

void valores() {
  Serial.print("  AHORA: "); Serial.print(VEL_PATADA);
  Serial.print(" x "); Serial.print(MS_PATADA);
  Serial.print(" ms   trim "); Serial.print(TRIM_PATADA);
  Serial.print("   KP "); Serial.print(KP_PATADA, 1);
  Serial.print("   KD "); Serial.print(KD_PATADA, 1);
  Serial.print(lazoActivo ? "   corrector SI" : "   corrector NO (trim solo)");
  Serial.print(traseraTrabada ? "   trasera TRABADA (como el principal)" : "   trasera SUELTA");
  Serial.print(sinEspera ? "   chequeo SIN espera" : "   chequeo CON espera de 200 ms (como juega hoy)");
  Serial.println(grafico ? "   grafico SI" : "   grafico NO");
}

void ayuda() {
  Serial.println();
  Serial.println("----------------- PRUEBA DE PATADA -----------------");
  Serial.println("  p = PATEAR: golpe + retroceso, como en el partido");
  Serial.println("      (cualquier tecla mientras se mueve = FRENA)");
  Serial.println("  t/T trim -1/+1       k/K KP -0.5/+0.5     d/D KD -0.1/+0.1");
  Serial.println("  v/V potencia -5/+5   m/M duracion -20/+20 ms");
  Serial.println("  l = corrector del giroscopio SI / NO (NO = trim solo, para ajustar el trim)");
  Serial.println("  b = rueda trasera TRABADA / SUELTA durante el golpe");
  Serial.println("  c = chequeo del giroscopio CON / SIN la espera de 200 ms");
  Serial.println("  g = grafico si/no    ? = esta ayuda");
  valores();
}

// Vacia lo que haya llegado por el monitor. Devuelve true si habia alguna
// tecla de verdad (un Enter suelto no cuenta).
bool hayTeclaDeVerdad() {
  bool hay = false;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c != '\r' && c != '\n') hay = true;
  }
  return hay;
}

void informe() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.print(" PATADA "); Serial.print(numeroPatada); Serial.print(":");
  valores();
  if (frenadaATecla) Serial.println(" !!! la frenaron con una tecla: los numeros son de lo que llego a hacer");

  if (!hayGiroscopo || !rumboPatadaValido) {
    Serial.println(" SIN GIROSCOPIO: pateo con el trim fijo y NO se pudo medir el desvio.");
    Serial.println(" Mirar el cartel de arranque (Giroscopo: ...) y pruebas/giroscopo-crudo/.");
    Serial.println("=====================================================");
    return;
  }

  Serial.println("   grados: NEGATIVO = se fue a la DERECHA, POSITIVO = a la IZQUIERDA");
  Serial.printf("   al terminar el golpe ......... %6.1f\n", finGolpe);
  Serial.printf("   PICO durante el golpe ........ %6.1f   (a los %lu ms)\n", pico, picoCuando);
  Serial.printf("   pico en los primeros 200 ms .. %6.1f   <- lo que ve la pelota\n", pico200);
  if (hizoRetroceso && !frenadaATecla)
    Serial.printf("   al terminar el retroceso ..... %6.1f   (pico %.1f)\n", finRetroceso, picoRetroceso);
  Serial.printf("   el golpe duro ................ %lu ms  (pedido: %d)\n", duroGolpe, MS_PATADA);

  if (demoraEntrada > 50)
    Serial.printf("   >>> ANTES de arrancar el golpe el programa estuvo clavado %lu ms\n", demoraEntrada);
  if (huecoMax > 50) {
    Serial.printf("   >>> EL PROGRAMA ESTUVO CLAVADO %lu ms EN MEDIO DEL GOLPE, desde los %lu ms.\n",
                  huecoMax, huecoCuando);
    Serial.println("       En ese rato no corrige el rumbo: los motores siguen con lo");
    Serial.println("       ultimo que se les mando. Probar la tecla c y comparar.");
  } else {
    Serial.printf("   el programa no se clavo durante el golpe (la vuelta mas larga: %lu ms)\n", huecoMax);
  }

  if (grafico && nMuestras > 0) {
    Serial.println();
    Serial.println("     ms   grados      IZQUIERDA <<<  |  >>> DERECHA    (1 simbolo = 1 grado)");
    bool separado = false;
    for (int i = 0; i < nMuestras; i += 2) {         // una fila cada 20 ms
      if (!separado && i >= nMuestrasGolpe) {
        Serial.println("   ------------- aca termina el golpe y empieza el retroceso -------------");
        separado = true;
      }
      int n = (int)(fabs(muestraErr[i]) + 0.5);
      if (n > 30) n = 30;
      Serial.printf("   %4u  %6.1f   ", muestraT[i], muestraErr[i]);
      if (muestraErr[i] > 0) {
        for (int k = 0; k < 30 - n; k++) Serial.print(' ');
        for (int k = 0; k < n; k++)      Serial.print('<');
        Serial.println('|');
      } else {
        for (int k = 0; k < 30; k++) Serial.print(' ');
        Serial.print('|');
        for (int k = 0; k < n; k++) Serial.print('>');
        Serial.println();
      }
    }
  }
  Serial.println("=====================================================");
  Serial.println(" Se va siempre a la DERECHA (negativos)? subir el trim (T).");
  Serial.println(" Siempre a la IZQUIERDA (positivos)? bajarlo (t).");
  Serial.println(" Para los dos lados, con pico grande y final chico? es el corrector: d/D y k/K.");
  Serial.println(" p = otra patada      ? = ayuda");
}

void tecla(char c) {
  switch (c) {
    case 'p': case 'P':
      if (!hayGiroscopo) Serial.println("!!! OJO: sin giroscopio va a patear, pero no puede medir el desvio");
      t_disparo = millis() + 1500;
      Serial.println(">>> PATEO en 1,5 s. Saca la mano.   (otra tecla = cancelar)");
      return;
    case 't': TRIM_PATADA -= 1; break;
    case 'T': TRIM_PATADA += 1; break;
    case 'k': KP_PATADA -= 0.5; if (KP_PATADA < 0) KP_PATADA = 0; break;
    case 'K': KP_PATADA += 0.5; break;
    case 'd': KD_PATADA -= 0.1; if (KD_PATADA < 0.05) KD_PATADA = 0; break;
    case 'D': KD_PATADA += 0.1; break;
    case 'v': VEL_PATADA -= 5;  if (VEL_PATADA < 60)  VEL_PATADA = 60;  break;
    case 'V': VEL_PATADA += 5;  if (VEL_PATADA > 255) VEL_PATADA = 255; break;
    case 'm': MS_PATADA -= 20;  if (MS_PATADA < 100)  MS_PATADA = 100;  break;
    case 'M': MS_PATADA += 20;  if (MS_PATADA > 1000) MS_PATADA = 1000; break;
    case 'l': case 'L': lazoActivo = !lazoActivo; break;
    case 'b': case 'B': traseraTrabada = !traseraTrabada; break;
    case 'g': case 'G': grafico = !grafico; break;
    case 'c': case 'C':
      if (sinEspera) { sinEspera = false; break; }
      // Antes de confiar en la lectura rapida se la compara con la de la
      // libreria. Si no dicen lo mismo, la rapida esta mal y no se activa.
      {
        uint8_t lento = 0, autotest = 0, err = 0;
        bno.getSystemStatus(&lento, &autotest, &err);
        uint8_t rapido = estadoGiroSinEspera();
        if (hayGiroscopo && rapido == lento) {
          sinEspera = true;
        } else {
          Serial.print("!!! NO la activo: la lectura rapida dio "); Serial.print(rapido);
          Serial.print(" y la de la libreria "); Serial.print(lento);
          Serial.println(". Avisar.");
        }
      }
      break;
    case '?': ayuda(); return;
    case '\r': case '\n': case ' ': return;
    default:
      Serial.print("  la tecla '"); Serial.print(c); Serial.println("' no hace nada.   ? = ayuda");
      return;
  }
  valores();
}


// =====================================================================
//                        LA SECUENCIA
// =====================================================================

// Arranca el golpe. La segunda mitad es COPIA de lo que hace el principal al
// entrar a PATEA_ADEL (cambiarA).
void empezarGolpe() {
  t_disparo = 0;
  numeroPatada++;
  nMuestras = 0; nMuestrasGolpe = 0; t_muestra = 0;
  finGolpe = 0; pico = 0; picoCuando = 0; pico200 = 0; duroGolpe = 0;
  finRetroceso = 0; picoRetroceso = 0; hizoRetroceso = false;
  huecoMax = 0; huecoCuando = 0; frenadaATecla = false;

  // En espera nadie le pregunta al giroscopio si sigue vivo, asi que se le
  // pregunta aca. Tarda 200 ms, pero el robot todavia esta quieto.
  if (hayGiroscopo) {
    uint8_t sys = 0, autotest = 0, err = 0;
    bno.getSystemStatus(&sys, &autotest, &err);
    giroCaido = (sys != 5);
  }

  // EN EL PARTIDO se llega a la patada desde la orbita, que le pregunta el
  // rumbo al giroscopio cada 20 ms: el chequeo de 200 ms se repite cada
  // ~500-520 ms, y mientras esta clavado no puede decidir patear. Entonces al
  // entrar a la patada el ultimo chequeo fue hace entre ~200 y ~520 ms, y el
  // siguiente cae en los primeros ~300 ms del golpe (o al entrar, todavia
  // quieto, si ya pasaron mas de 500). Aca se sortea esa EDAD del ultimo
  // chequeo, entre 200 y 520 ms (random(a, b) da de a hasta b-1), para que la
  // prueba no caiga siempre en el mismo lugar. El informe dice donde cayo.
  // [27/10] Antes era random(0, 500) y 4 de cada 10 patadas caian despues de
  // los 300 ms, cosa que en el partido no pasa. Igual que pruebas/tablero.
  randomSeed(micros());
  t_chequeoGiro = millis() - random(200, MS_CHEQUEO_GIRO + 21);

  digitalWrite(LED, HIGH);
  unsigned long t0 = millis();

  // ---- COPIA DEL PRINCIPAL: lo que hace cambiarA(PATEA_ADEL) ----
  rumboPatadaValido = giroscopoSano();
  if (rumboPatadaValido) rumboAlPatear = rumboActual();
  else Serial.println("!!! patada sin rumbo del giroscopo -> va con trim fijo, sin lazo");
  reiniciarRampaPatada();
  reiniciarDerivadaPatada();
  // ----------------------------------------------------------------

  demoraEntrada = millis() - t0;
  fase = 1;
  t_entroFase   = millis();
  t_inicioGolpe = t_entroFase;
  midiendoGolpe = true;
}

void terminar() {
  informe();
  fase    = 0;
  t_freno = millis();
  digitalWrite(LED, LOW);
  hayTeclaDeVerdad();                // tira lo que se haya tecleado mientras tanto
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 3000) { }

  Serial.println();
  Serial.println("#####################################################");
  Serial.println("#  PATADA DERECHA  -  golpe + retroceso, medidos     #");
  Serial.println("#  NO ES EL DE PARTIDO. No busca la pelota:          #");
  Serial.println("#  espera teclas.                                    #");
  Serial.println("#  EN EL PISO, con lugar adelante y el cable flojo.  #");
  Serial.println("#####################################################");

  Serial.print("Giroscopo: ");
  hayGiroscopo = arrancarGiroscopo();
  if (hayGiroscopo) {
    Serial.println(GIRO_SIN_BRUJULA ? "OK (fusion corriendo, modo IMU sin brujula)"
                                    : "OK (fusion corriendo, modo NDOF con brujula)");
  } else {
    Serial.println("NO CONTESTA. Se puede patear, pero no medir.");
  }
  Serial.print("retroceso: "); Serial.print(VEL_RETROCESO);
  Serial.print(" x "); Serial.print(MS_RETROCESO); Serial.println(" ms (sin freno en el medio)");
  ayuda();
}

void loop() {
  // Cuanto tardo la vuelta anterior. Si el programa se quedo clavado en medio
  // del golpe, aca se ve.
  unsigned long ahora = millis();
  if (midiendoGolpe && msLoopPrevio >= t_inicioGolpe) {
    unsigned long hueco = ahora - msLoopPrevio;
    if (hueco > huecoMax) { huecoMax = hueco; huecoCuando = msLoopPrevio - t_inicioGolpe; }
  }
  if (fase != 1) midiendoGolpe = false;   // la ultima vuelta del golpe tambien se midio
  msLoopPrevio = ahora;

  // Cualquier tecla mientras se mueve = FRENO.
  if (fase != 0 && hayTeclaDeVerdad()) {
    frenar();
    frenadaATecla = true;
    Serial.println("!!! FRENADA con una tecla");
    terminar();
    return;
  }

  unsigned long enFase = millis() - t_entroFase;

  // ---------- EL GOLPE ----------
  if (fase == 1) {
    // ---- COPIA DEL PRINCIPAL: el estado PATEA_ADEL ----
    int vel = rampaPatada(VEL_PATADA);
    bool conLazo = (giroscopoSano() && rumboPatadaValido);
    if (conLazo && lazoActivo) avanzarDerecho(vel, rumboAlPatear);     // "&& lazoActivo" es de la prueba (tecla l)
    else                       motoresPatada(vel - TRIM_PATADA, vel + TRIM_PATADA);
    float errP = conLazo ? diferencia(rumboAlPatear, rumboActual()) : 0.0;
    // ----------------------------------------------------

    // anotar
    duroGolpe = enFase;
    finGolpe  = errP;
    if (fabs(errP) > fabs(pico)) { pico = errP; picoCuando = enFase; }
    if (enFase <= 200 && fabs(errP) > fabs(pico200)) pico200 = errP;
    if (millis() - t_muestra >= 10 && nMuestras < MUESTRAS) {
      t_muestra = millis();
      muestraT[nMuestras]   = (uint16_t)enFase;
      muestraErr[nMuestras] = errP;
      nMuestras++;
      nMuestrasGolpe = nMuestras;
    }

    if (enFase >= (unsigned long)MS_PATADA) {
      Serial.print("*** golpe terminado: se torcio "); Serial.print(errP, 1);
      Serial.print(" grados (pico "); Serial.print(pico, 1); Serial.println(")");
      fase = 2;                       // en el principal: cambiarA(PATEA_ATRAS), sin freno
      t_entroFase = millis();
      duroGolpe   = t_entroFase - t_inicioGolpe;   // lo que duro DE VERDAD
      hizoRetroceso = true;
    }
  }

  // ---------- EL RETROCESO ----------
  else if (fase == 2) {
    retroceder(VEL_RETROCESO);        // COPIA DEL PRINCIPAL: el estado PATEA_ATRAS

    // El retroceso del partido NO mira el giroscopio. Aca se lo lee aparte,
    // cada 10 ms y sin el chequeo de salud: es solo para anotar.
    if (hayGiroscopo && rumboPatadaValido && millis() - t_muestra >= 10) {
      t_muestra = millis();
      sensors_event_t e;
      bno.getEvent(&e);
      float err = diferencia(rumboAlPatear, e.orientation.x);
      finRetroceso = err;
      if (fabs(err) > fabs(picoRetroceso)) picoRetroceso = err;
      if (nMuestras < MUESTRAS) {
        muestraT[nMuestras]   = (uint16_t)(duroGolpe + enFase);
        muestraErr[nMuestras] = err;
        nMuestras++;
      }
    }

    if (enFase >= (unsigned long)MS_RETROCESO) terminar();
  }

  // ---------- EN ESPERA ----------
  else {
    // Frena de verdad un momento y despues suelta, para poder acomodarlo a mano.
    if (millis() - t_freno < 400) frenar(); else parar();

    if (t_disparo != 0) {            // cuenta regresiva en marcha
      if (hayTeclaDeVerdad()) { t_disparo = 0; Serial.println("... cancelado"); }
      else if (millis() >= t_disparo) empezarGolpe();
      return;
    }
    while (Serial.available() && t_disparo == 0) tecla((char)Serial.read());
  }
}
