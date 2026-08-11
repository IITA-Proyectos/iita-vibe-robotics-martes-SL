/* =====================================================================
   CUADRADO CON GIROSCOPO — lazo cerrado de rumbo
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-07-28
   =====================================================================

   POR QUE ESTE SKETCH EXISTE
   `cuadrado-lento` gira por CRONOMETRO: "girá 400 ms y esperemos que sean
   90 grados". Eso nunca cierra bien, y no por estar mal programado: al
   soltar los motores el robot sigue de largo por inercia, y cuanto queda
   de bateria cambia cuanto gira en el mismo tiempo. Medimos 700 ms = 160
   grados, pero ese numero no vale mañana.

   Aca el robot MIRA cuanto giro y corrige hasta llegar. La inercia deja de
   importar: si se pasa, vuelve. Lo mismo en los tramos rectos, donde
   sostiene el rumbo en vez de rezar.

   Esto es lo que el analisis del codigo 2025 marcaba como la mejora mas
   jugosa: alli el giroscopio se lee, se calcula `correccion = error*0.3`
   ... y despues no se usa para nada. Codigo muerto.

   ---------------------------------------------------------------------
   🚨 COMO SE PARA
   ---------------------------------------------------------------------
   SIN LA COMPUTADORA, LA UNICA FORMA ES LA LLAVE DE LA BATERIA.

   Arranca con 10 segundos de espera y el LED del Teensy avisando:
        parpadeo LENTO  -> falta mas de 3 s, todavia lo podes tocar
        parpadeo RAPIDO -> faltan menos de 3 s, SOLTALO
        LED FIJO        -> en movimiento
        TRIPLE parpadeo repetido -> EL GIROSCOPIO NO RESPONDE (ver abajo)

   PROTOCOLO: apoyar -> prender bateria -> sacar las manos. Nunca al reves.

   ⚠️ Durante esos 10 segundos el robot tiene que estar QUIETO. El BNO055
   fija ahi su rumbo de referencia; si lo movés mientras tanto, todo el
   cuadrado sale torcido y no es culpa del programa.

   ---------------------------------------------------------------------
   SI EL GIROSCOPIO NO RESPONDE
   ---------------------------------------------------------------------
   El sketch NO se mueve. Prefiere quedarse quieto antes que manejar a
   ciegas creyendo que tiene rumbo. El LED hace triple parpadeo sin parar.
   Revisar: cable I2C (pines 18 y 19) y que el BNO este en la direccion
   0x28. El codigo 2025 en este caso se colgaba en un bucle infinito sin
   avisar nada — de ahi el LED.

   ---------------------------------------------------------------------
   LOS SIGNOS: HAY QUE DESCUBRIRLOS, NO SE PUEDEN ADIVINAR
   ---------------------------------------------------------------------
   Dos cosas no se saben de antemano y se averiguan probando:

     1. Para que lado gira el robot con `girar()`, y si el giroscopio
        cuenta ese giro como positivo o negativo.
     2. Para que lado empuja la rueda trasera al corregir en recta.

   Si estan al reves, el robot se autocorrige EN CONTRA y se va cada vez
   mas torcido — se nota enseguida. Con el cable puesto:

        i = invertir el sentido de GIRO
        k = invertir el sentido de la CORRECCION en recta

   Cuando encuentres la combinacion que anda, ANOTALA y cambiá los dos
   `bool` de abajo para que quede fija.

   ---------------------------------------------------------------------
   TECLAS (solo con la PC conectada)
   ---------------------------------------------------------------------
        0 = PARAR TODO            g = cuadrado completo
        e = SOLO RECTA — avanza derecho sin girar nunca. Sirve para probar
            la correccion de rumbo aislada: si el robot se curva cada vez
            mas en vez de ir derecho, el signo esta al reves -> tecla 'k'.
        h = decir el rumbo actual (para probar el giroscopio a mano)
        i = invertir sentido de giro
        k = invertir sentido de correccion en recta
        d / a = velocidad de avance +10 / -10
        t / r = tiempo de cada lado +200 / -200 ms
        ? = ayuda

   ---------------------------------------------------------------------
   ESTE SKETCH ES DEL ARQUERO (ROBOT1)
   ---------------------------------------------------------------------
   Pines y direcciones sacados de `arquero.ino`. Nombres de rueda segun lo
   MEDIDO en banco el 2026-07-28 (los comentarios del codigo 2025 estan
   espejados):
        U5  = pines 2/5/3    -> IZQUIERDA
        U17 = pines 8/7/6    -> DERECHA
        U7  = pines 11/12/4  -> TRASERA (no empuja al avanzar recto;
                                aca es la que corrige el rumbo)
   ===================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


// ---- ARQUERO (ROBOT1) ----
#define INA1 2
#define INB1 5
#define PWM1 3      // M1 = IZQUIERDA (U5)

#define INA2 8
#define INB2 7
#define PWM2 6      // M2 = DERECHA (U17)

#define INA3 11
#define INB3 12
#define PWM3 4      // M3 = TRASERA (U7) — la que corrige

#define LED 13


Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);


// ---- los dos signos a descubrir ----
bool giroInvertido       = false;   // tecla 'i'
bool correccionInvertida = false;   // tecla 'k'

// ---- lo que se puede tocar en caliente ----
int velAvance = 70;
unsigned long msLado = 1500;

// ---- constantes del lazo cerrado ----
const float GRADOS_POR_GIRO   = 90.0;
const float TOLERANCIA_GIRO   = 3.0;    // grados: se da por llegado
const float KP_GIRO           = 1.5;    // PWM por grado de error
const int   PWM_MIN_GIRO      = 35;     // abajo de esto no vence el roce
const int   PWM_MAX_GIRO      = 60;
const unsigned long MS_ASENTAR      = 300;   // esperar que frene la inercia
const unsigned long MS_TIMEOUT_GIRO = 5000;  // si no llega, seguir igual
const int   MAX_REINTENTOS_GIRO = 4;

// --- ARRANQUE SUAVE DEL GIRO ---
// El giro es el unico momento en que las TRES ruedas pasan de quietas a
// fondo de golpe. Ese tiron de corriente es el sospechoso de dejar mudo al
// giroscopio: las dos veces que fallo en el piso fue girando, y en el banco
// (sin moverse de verdad) nunca fallo por mas que girara media hora.
// En vez de saltar a full, sube de a poco: ~300 ms hasta la potencia plena.
const int   RAMPA_GIRO_PASO   = 3;    // cuanto sube por escalon
const unsigned long RAMPA_GIRO_MS = 15;   // cada cuanto sube un escalon
int pwmGiroAplicado = 0;
unsigned long t_rampaGiro = 0;

void reiniciarRampaGiro() {
  pwmGiroAplicado = 0;
  t_rampaGiro = millis();
}

// --- correccion en recta: proporcional + acumulada (PI) ---
// MEDIDO el 2026-08-04 con solo proporcional (KP=3, sin acumular): el robot
// se desviaba hasta 11.9 grados y se quedaba en 7.2. Corregia — el numero
// bajaba — pero nunca terminaba de enderezarse.
//
// Por que: hay algo que lo empuja SIEMPRE para el mismo lado (una rueda que
// tira mas, o que roza). Con correccion solo proporcional eso termina en
// empate: el robot corrige lo justo para no torcerse mas, y se queda ahi.
// Para volver a cero hay que ACUMULAR: si hace rato que viene torcido para
// el mismo lado, insistir cada vez mas fuerte.
const float KP_RECTO          = 5.0;    // PWM por grado de error
const float KI_RECTO          = 1.5;    // PWM por (grado x segundo) acumulado
const float LIMITE_INTEGRAL   = 30.0;   // tope del acumulador
const int   PWM_MAX_CORRECCION = 80;
const float ZONA_MUERTA_RECTO = 1.0;    // grados: no vale la pena corregir

float integralRumbo = 0;
unsigned long t_ultimoControl = 0;

const unsigned long MS_ESPERA_INICIAL = 10000;
const unsigned long MS_PAUSA_ENTRE_VUELTAS = 2000;
const int LADOS = 4;

// Que hace al terminar la cuenta regresiva, SIN computadora conectada:
//   true  -> camina derecho MS_RECTA_SOLA y se para. Sirve para mirar a ojo
//            si se endereza solo, sin necesidad de leer numeros.
//   false -> hace el cuadrado completo.
// 2026-08-04: la recta sola ya quedo probada (max 5.2 grados, termino en
// -1.8). Pasamos al cuadrado completo, que ejercita giros y rectas juntos.
bool soloRecta = false;
const unsigned long MS_RECTA_SOLA = 8000;
unsigned long t_recta = 0;

// El robot se acuerda de como le fue. Sirve para largarlo en el piso SIN
// cable (que es donde hay lugar) y despues venir, enchufar y preguntarle
// con la tecla 'm'. Sin esto habria que elegir entre tener lugar o tener
// numeros.
// OJO: si al enchufar el USB el Teensy se reinicia, esto se borra. Para que
// sobreviva, dejar la BATERIA PRENDIDA al enchufar el cable.
bool  huboCorrida   = false;
float desvioMax     = 0;    // el peor desvio, con signo
float desvioFinal   = 0;
unsigned long msCorrida = 0;


enum Fase { SIN_GIROSCOPO, RECUPERANDO, ESPERANDO, LADO, GIRO, ASENTAR,
            PAUSA, RECTA, QUIETO };
Fase fase = ESPERANDO;

// Cuantas veces se cayo el giroscopio en toda la sesion. Este numero es el
// dato: con "se cayo una vez" no se puede saber si un arreglo sirvio; con
// "se cae 3 veces cada 10 vueltas" si.
int caidas = 0;
int intentosRecuperar = 0;
const int MAX_INTENTOS_RECUPERAR = 5;
const unsigned long MS_ENTRE_INTENTOS = 1000;

// Contexto de la ULTIMA vez que se quedo mudo. Sin esto solo sabemos "paso
// una vez"; con esto sabemos si siempre pasa arrancando el giro, o siempre
// en la misma vuelta, o al azar. Eso decide que arreglo probar.
Fase faseMudo      = QUIETO;
int  vueltaMudo    = 0;
int  ladoMudo      = 0;
unsigned long msEnFaseMudo = 0;
int  pwmGiroMudo   = 0;
bool huboMudo      = false;

// Recibe int y no Fase a proposito: el Arduino IDE genera solo los anuncios
// de las funciones y los pone ARRIBA DE TODO, antes de que exista el enum.
// Con `Fase` en la firma, no compila ("'Fase' was not declared in this
// scope") y el error apunta a una linea que uno ni escribio.
const char* nombreFase(int f) {
  switch (f) {
    case SIN_GIROSCOPO: return "ciego";
    case RECUPERANDO:   return "recuperando";
    case ESPERANDO:     return "cuenta regresiva";
    case LADO:          return "AVANZANDO";
    case GIRO:          return "GIRANDO";
    case ASENTAR:       return "acomodandose";
    case PAUSA:         return "pausa";
    case RECTA:         return "recta sola";
    default:            return "quieto";
  }
}

float rumboObjetivo = 0;
int  ladosHechos = 0;
int  vuelta = 0;
int  reintentos = 0;
unsigned long t_fase = 0;

// Deteccion de sensor caido. El 2026-07-28 el BNO055 empezo a devolver
// exactamente 0.0 y no se recupero; el sketch no se dio cuenta y siguio
// girando con datos basura. La libreria Adafruit devuelve ceros cuando no
// puede leer el chip: si los TRES angulos dan 0.000 exacto varias veces
// seguidas, no es una postura real, es el sensor que no contesta.
int contadorCeros = 0;
const int CEROS_PARA_DARLO_POR_CAIDO = 10;
float ultimoRumbo = 0;


// ---------------------------------------------------------------- rumbo

float rumboActual() {
  sensors_event_t evento;
  bno.getEvent(&evento);

  if (evento.orientation.x == 0.0 && evento.orientation.y == 0.0
      && evento.orientation.z == 0.0) {
    if (contadorCeros < CEROS_PARA_DARLO_POR_CAIDO) contadorCeros++;
  } else {
    contadorCeros = 0;
  }

  ultimoRumbo = evento.orientation.x;
  return ultimoRumbo;                   // 0..360
}

bool giroscopoCaido() {
  return contadorCeros >= CEROS_PARA_DARLO_POR_CAIDO;
}

// Frena y trata de revivir el sensor. Lo PRIMERO es cortar los motores:
// si la causa es el tiron de corriente de las tres ruedas juntas, mientras
// sigan andando el chip no va a arrancar nunca.
void perdiElRumbo() {
  parar();
  caidas++;

  // Guardar QUE estaba haciendo justo cuando se quedo mudo, ANTES de
  // cambiar de fase — si no, se pierde el unico dato que sirve.
  faseMudo   = fase;
  vueltaMudo = vuelta;
  ladoMudo   = ladosHechos + 1;
  msEnFaseMudo = millis() - t_fase;
  pwmGiroMudo  = pwmGiroAplicado;
  huboMudo     = true;

  intentosRecuperar = 0;
  fase = RECUPERANDO;
  t_fase = millis();
  Serial.print("!! GIROSCOPIO MUDO (n. "); Serial.print(caidas);
  Serial.print(") mientras estaba "); Serial.print(nombreFase(faseMudo));
  Serial.print(", vuelta "); Serial.print(vueltaMudo);
  Serial.print(", lado "); Serial.print(ladoMudo);
  Serial.print(", a los "); Serial.print(msEnFaseMudo);
  Serial.print(" ms de esa fase, PWM de giro "); Serial.println(pwmGiroMudo);
  Serial.println("   motores cortados, reintentando");
}

// Reinicia el bus I2C y el sensor. Devuelve true SOLO si de verdad volvio
// a dar datos.
//
// 🚨 Version anterior daba FALSOS POSITIVOS: leia UNA sola vez y con eso
// cantaba "recuperado". Pero el sintoma de este sensor cuando esta mal es
// justamente contestar que existe y devolver ceros — con una lectura no se
// distingue de uno sano. Ahora hay que ver varias lecturas NO nulas.
bool revivirGiroscopo() {
  Wire.end();
  delay(50);
  Wire.begin();
  delay(50);
  if (!bno.begin()) return false;   // ni siquiera contesta quien es
  delay(700);                       // el BNO tarda en arrancar la fusion
  bno.setExtCrystalUse(true);

  int buenas = 0;
  for (int i = 0; i < 20; i++) {
    sensors_event_t e;
    bno.getEvent(&e);
    if (e.orientation.x != 0.0 || e.orientation.y != 0.0
        || e.orientation.z != 0.0) buenas++;
    delay(50);
  }
  if (buenas < 10) {
    Serial.print("   contesta pero da ceros ("); Serial.print(buenas);
    Serial.println("/20 lecturas utiles) — no lo doy por bueno");
    return false;
  }
  contadorCeros = 0;
  return true;
}

// Devuelve la diferencia mas corta entre dos rumbos, en (-180, 180].
// Sin esto, ir de 350 a 10 grados se leeria como un giro de -340.
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)  d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

float normalizar(float a) {
  while (a >= 360.0) a -= 360.0;
  while (a < 0.0)    a += 360.0;
  return a;
}


// ---------------------------------------------------------------- motores

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Avanza sosteniendo `rumboObjetivo`. Las dos de adelante empujan parejo y
// la trasera hace de timon, igual que las funciones proporcionales del
// codigo 2025 — pero con el error real del giroscopio, no con tablas fijas.
// Se llama al empezar cada recta: el acumulado de una recta no tiene nada
// que ver con el de la siguiente, y arrastrarlo haria que arranque torcido.
void reiniciarCorreccion() {
  integralRumbo = 0;
  t_ultimoControl = millis();
}

void avanzarConRumbo() {
  analogWrite(PWM1, velAvance); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, velAvance); digitalWrite(INA2, 0); digitalWrite(INB2, 1);

  float error = diferencia(rumboObjetivo, rumboActual());

  unsigned long ahora = millis();
  float dt = (ahora - t_ultimoControl) / 1000.0;
  t_ultimoControl = ahora;
  if (dt > 0.5) dt = 0.5;               // por si hubo una pausa larga

  if (fabs(error) < ZONA_MUERTA_RECTO) {
    // Dentro de la zona muerta no se acumula, pero TAMPOCO se borra lo
    // acumulado: eso es lo que sostiene al robot derecho contra el empuje.
    analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
    return;
  }

  // Anti-windup: el acumulador tiene tope. Sin esto, un rato torcido lo
  // infla tanto que despues el robot se pasa de largo para el otro lado.
  integralRumbo += error * dt;
  if (integralRumbo >  LIMITE_INTEGRAL) integralRumbo =  LIMITE_INTEGRAL;
  if (integralRumbo < -LIMITE_INTEGRAL) integralRumbo = -LIMITE_INTEGRAL;

  float correccion = error * KP_RECTO + integralRumbo * KI_RECTO;

  int pwm = (int)fabs(correccion);
  if (pwm > PWM_MAX_CORRECCION) pwm = PWM_MAX_CORRECCION;

  // El sentido lo manda la correccion TOTAL, no el error solo: si el
  // acumulado ya da vuelta el signo, la rueda tiene que ir para el otro lado.
  bool sentido = (correccion > 0);
  if (correccionInvertida) sentido = !sentido;

  digitalWrite(INA3, sentido ? 1 : 0);
  digitalWrite(INB3, sentido ? 0 : 1);
  analogWrite(PWM3, pwm);
}

// Gira sobre el eje hacia `rumboObjetivo`, mas despacio cuanto mas cerca.
// Devuelve true cuando llego.
bool girarHaciaObjetivo() {
  float error = diferencia(rumboObjetivo, rumboActual());

  if (fabs(error) <= TOLERANCIA_GIRO) { parar(); return true; }

  int pwm = (int)(fabs(error) * KP_GIRO);
  if (pwm > PWM_MAX_GIRO) pwm = PWM_MAX_GIRO;
  if (pwm < PWM_MIN_GIRO) pwm = PWM_MIN_GIRO;

  // Arranque suave: subir de a escalones en vez de saltar. Bajar si puede
  // ser de golpe — menos corriente nunca es el problema.
  unsigned long ahora = millis();
  if (pwm < pwmGiroAplicado) {
    pwmGiroAplicado = pwm;
  } else if (ahora - t_rampaGiro >= RAMPA_GIRO_MS) {
    t_rampaGiro = ahora;
    pwmGiroAplicado += RAMPA_GIRO_PASO;
    if (pwmGiroAplicado > pwm) pwmGiroAplicado = pwm;
  }

  bool sentido = (error > 0);
  if (giroInvertido) sentido = !sentido;

  // Las tres al mismo sentido = rotacion pura, como girar() de arquero.ino
  digitalWrite(INA1, sentido ? 1 : 0); digitalWrite(INB1, sentido ? 0 : 1);
  digitalWrite(INA2, sentido ? 1 : 0); digitalWrite(INB2, sentido ? 0 : 1);
  digitalWrite(INA3, sentido ? 1 : 0); digitalWrite(INB3, sentido ? 0 : 1);
  analogWrite(PWM1, pwmGiroAplicado);
  analogWrite(PWM2, pwmGiroAplicado);
  analogWrite(PWM3, pwmGiroAplicado);
  return false;
}


// ---------------------------------------------------------------- consola

void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" CUADRADO CON GIROSCOPO — arquero (ROBOT1)");
  Serial.println("=================================================");
  Serial.println("  SIN LA PC, SOLO PARA CON LA LLAVE DE LA BATERIA");
  Serial.println("-------------------------------------------------");
  Serial.println("  0 = PARAR           g = cuadrado completo");
  Serial.println("  e = SOLO RECTA (sin giros, para probar la correccion)");
  Serial.println("  m = como le fue en la ultima corrida");
  Serial.println("  h = decir rumbo actual");
  Serial.println("  i = invertir sentido de GIRO");
  Serial.println("  k = invertir sentido de CORRECCION en recta");
  Serial.println("  d / a = velocidad avance  +10 / -10");
  Serial.println("  t / r = tiempo lado       +200 / -200 ms");
  Serial.print  ("  velAvance="); Serial.print(velAvance);
  Serial.print  ("  msLado=");    Serial.print(msLado);
  Serial.print  ("  giroInv=");   Serial.print(giroInvertido);
  Serial.print  ("  corrInv=");   Serial.println(correccionInvertida);
  Serial.println("=================================================");
}

void arrancar() {
  ladosHechos = 0; vuelta = 0; reintentos = 0;
  fase = ESPERANDO; t_fase = millis();
  Serial.println(">> ESPERANDO 10 s — apoyalo, sacá las manos, NO LO MUEVAS");
}

void leerConsola() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  if (fase == SIN_GIROSCOPO && c != '?' && c != 'h' && c != 'm' && c != 'g') {
    Serial.println("   el giroscopio no responde — no me muevo");
    Serial.println("   ('g' vuelve a intentar revivirlo)");
    return;
  }
  // Desde el estado ciego, 'g' reintenta la recuperacion en vez de arrancar.
  if (fase == SIN_GIROSCOPO && c == 'g') {
    intentosRecuperar = 0;
    fase = RECUPERANDO; t_fase = millis();
    Serial.println(">> reintentando revivir el giroscopio");
    return;
  }

  switch (c) {
    case 'g': arrancar(); break;

    // Modo SOLO RECTA: sin giros, para aislar la correccion de rumbo.
    // Si el signo esta al reves, el robot se curva cada vez mas en vez de
    // ir derecho — se ve en dos segundos y se arregla con la tecla 'k'.
    // Que le fue en la ultima corrida. La idea es largarlo en el piso sin
    // cable y despues venir a preguntarle.
    case 'm':
      Serial.print("   VECES QUE EL GIROSCOPIO QUEDO MUDO: ");
      Serial.println(caidas);
      if (huboMudo) {
        Serial.println("   --- la ultima vez fue ---");
        Serial.print("   estaba          "); Serial.println(nombreFase(faseMudo));
        Serial.print("   vuelta / lado   "); Serial.print(vueltaMudo);
        Serial.print(" / "); Serial.println(ladoMudo);
        Serial.print("   llevaba         "); Serial.print(msEnFaseMudo);
        Serial.println(" ms en esa fase");
        Serial.print("   PWM de giro     "); Serial.println(pwmGiroMudo);
        Serial.println("   (si el PWM es bajo, murio en el ARRANQUE del giro:");
        Serial.println("    ahi el culpable es el tiron de corriente)");
      }
      if (!huboCorrida) { Serial.println("   todavia no corrio en recta"); break; }
      Serial.println("   --- ultima corrida en recta ---");
      Serial.print("   duro            "); Serial.print(msCorrida); Serial.println(" ms");
      Serial.print("   desvio MAXIMO   "); Serial.print(desvioMax, 1);
      Serial.println(desvioMax > 0 ? "  (se fue para un lado)" : "  (se fue para el otro)");
      Serial.print("   desvio FINAL    "); Serial.println(desvioFinal, 1);
      Serial.println("   chico y parecido al maximo -> la correccion trabaja");
      Serial.println("   grande y creciendo         -> signo al reves, tecla 'k'");
      break;

    case 'e':
      rumboObjetivo = rumboActual();
      fase = RECTA; t_fase = millis(); t_recta = millis(); reiniciarCorreccion();
      desvioMax = 0; desvioFinal = 0; huboCorrida = true;
      digitalWrite(LED, HIGH);
      Serial.print(">> SOLO RECTA — sosteniendo rumbo ");
      Serial.println(rumboObjetivo, 1);
      Serial.println("   ('0' para parar, 'k' si se curva cada vez mas)");
      break;

    case '0': parar(); fase = QUIETO; digitalWrite(LED, LOW);
              Serial.println(">> PARADO — 'g' para arrancar de nuevo"); break;

    case 'h': Serial.print("   rumbo actual = ");
              Serial.print(rumboActual(), 1);
              Serial.print("   objetivo = "); Serial.println(rumboObjetivo, 1);
              break;

    case 'i': giroInvertido = !giroInvertido;
              Serial.print("   sentido de GIRO invertido = ");
              Serial.println(giroInvertido); break;

    case 'k': correccionInvertida = !correccionInvertida;
              Serial.print("   sentido de CORRECCION invertido = ");
              Serial.println(correccionInvertida); break;

    case 'd': velAvance += 10; if (velAvance > 255) velAvance = 255;
              Serial.print("   velAvance = "); Serial.println(velAvance); break;
    case 'a': velAvance -= 10; if (velAvance < 0) velAvance = 0;
              Serial.print("   velAvance = "); Serial.println(velAvance); break;

    case 't': msLado += 200; Serial.print("   msLado = "); Serial.println(msLado); break;
    case 'r': if (msLado > 200) msLado -= 200;
              Serial.print("   msLado = "); Serial.println(msLado); break;

    case '?': ayuda(); break;

    default: Serial.print("   tecla '"); Serial.print(c);
             Serial.println("' no hace nada. ? para la ayuda."); break;
  }
}


// ---------------------------------------------------------------- programa

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  pinMode(LED, OUTPUT);
  parar();

  Serial.begin(19200);
  ayuda();

  // A diferencia del codigo 2025, si el BNO falla NO se cuelga en un
  // while(1) mudo: avisa por serie, avisa por LED, y no se mueve.
  if (!bno.begin()) {
    fase = SIN_GIROSCOPO;
    Serial.println("!! EL GIROSCOPIO NO RESPONDE (BNO055 en 0x28)");
    Serial.println("!! No me voy a mover. Revisar I2C, pines 18 y 19.");
    return;
  }
  delay(1000);
  bno.setExtCrystalUse(true);
  Serial.println(">> giroscopio OK");

  arrancar();
}


void loop() {

  leerConsola();
  unsigned long ahora = millis();

  // Vigilancia permanente del sensor: si se cae en mitad de una recta o de
  // un giro, frenamos. Antes esto no existia y el robot seguia como si nada.
  if (fase == LADO || fase == GIRO || fase == RECTA) {
    rumboActual();                      // actualiza el contador de ceros
    if (giroscopoCaido()) { perdiElRumbo(); return; }
  }

  switch (fase) {

    case SIN_GIROSCOPO: {
      parar();
      // triple parpadeo repetido: se distingue de lejos del resto
      unsigned long t = ahora % 1600;
      bool on = (t < 100) || (t >= 200 && t < 300) || (t >= 400 && t < 500);
      digitalWrite(LED, on ? HIGH : LOW);
      break;
    }

    case RECUPERANDO: {
      parar();
      // parpadeo doble mientras intenta revivir
      unsigned long t = ahora % 1200;
      digitalWrite(LED, ((t < 100) || (t >= 200 && t < 300)) ? HIGH : LOW);

      if (ahora - t_fase < MS_ENTRE_INTENTOS) break;
      t_fase = ahora;
      intentosRecuperar++;

      Serial.print("   intento de revivir n. "); Serial.println(intentosRecuperar);
      if (revivirGiroscopo()) {
        // 🚨 NO arranca solo. Despues de una falla, que el robot vuelva a
        // moverse por su cuenta es peligroso: el que esta al lado no lo
        // espera. Queda quieto y arranca cuando alguien se lo pida.
        fase = QUIETO;
        digitalWrite(LED, LOW);
        Serial.println(">> GIROSCOPIO RECUPERADO — quedo QUIETO a proposito");
        Serial.println("   mandá 'g' cuando quieras arrancar de nuevo");
      } else if (intentosRecuperar >= MAX_INTENTOS_RECUPERAR) {
        fase = SIN_GIROSCOPO;
        Serial.println("!! NO REVIVIO EN 5 INTENTOS — me quedo quieto");
        Serial.println("!! Revisar bateria, y el cable I2C de los pines 18 y 19.");
      }
      break;
    }

    case QUIETO:
      digitalWrite(LED, LOW);
      break;

    case ESPERANDO: {
      unsigned long transcurrido = ahora - t_fase;
      unsigned long falta = (transcurrido >= MS_ESPERA_INICIAL)
                            ? 0 : (MS_ESPERA_INICIAL - transcurrido);
      unsigned long periodo = (falta <= 3000) ? 100 : 500;
      digitalWrite(LED, ((ahora / periodo) % 2) ? HIGH : LOW);

      if (transcurrido >= MS_ESPERA_INICIAL) {
        // El rumbo de referencia se fija ACA, con el robot ya quieto.
        rumboObjetivo = rumboActual();
        vuelta = 1; ladosHechos = 0;
        digitalWrite(LED, HIGH);
        Serial.print(">> rumbo de referencia = ");
        Serial.println(rumboObjetivo, 1);

        if (soloRecta) {
          fase = RECTA; t_fase = ahora; t_recta = ahora; reiniciarCorreccion();
          desvioMax = 0; desvioFinal = 0; huboCorrida = true;
          Serial.println(">> SOLO RECTA — 8 segundos derecho y para");
        } else {
          fase = LADO; t_fase = ahora; reiniciarCorreccion();
          Serial.println(">> VUELTA 1 — LADO 1");
        }
      }
      break;
    }

    case LADO:
      avanzarConRumbo();
      if (ahora - t_fase >= msLado) {
        parar();
        rumboObjetivo = normalizar(rumboObjetivo + GRADOS_POR_GIRO);
        reintentos = 0;
        fase = GIRO; t_fase = ahora; reiniciarRampaGiro();
        Serial.print("   giro hacia "); Serial.println(rumboObjetivo, 1);
      }
      break;

    case GIRO:
      if (girarHaciaObjetivo()) {
        fase = ASENTAR; t_fase = ahora;
      } else if (ahora - t_fase >= MS_TIMEOUT_GIRO) {
        // No llego en 5 s: probablemente los signos esten al reves.
        parar();
        Serial.print("!! giro sin llegar en 5 s — rumbo ");
        Serial.print(rumboActual(), 1);
        Serial.println(" — probá la tecla 'i'");
        fase = ASENTAR; t_fase = ahora;
      }
      break;

    case ASENTAR:
      // Frena y espera a que la inercia termine, DESPUES vuelve a medir.
      // Aca es donde el lazo cerrado le gana al cronometro: si se paso,
      // corrige; el sketch por tiempo no se enteraba.
      parar();
      if (ahora - t_fase >= MS_ASENTAR) {
        float error = diferencia(rumboObjetivo, rumboActual());
        if (fabs(error) > TOLERANCIA_GIRO && reintentos < MAX_REINTENTOS_GIRO) {
          reintentos++;
          Serial.print("   se paso "); Serial.print(error, 1);
          Serial.print(" grados — corrigiendo ("); Serial.print(reintentos);
          Serial.println(")");
          fase = GIRO; t_fase = ahora; reiniciarRampaGiro();
        } else {
          Serial.print("   quedo en "); Serial.print(rumboActual(), 1);
          Serial.print(" (error "); Serial.print(error, 1); Serial.println(")");
          ladosHechos++;
          if (ladosHechos >= LADOS) {
            fase = PAUSA; t_fase = ahora;
            Serial.println(">> cuadrado cerrado — pausa");
          } else {
            fase = LADO; t_fase = ahora; reiniciarCorreccion();
            Serial.print(">> LADO "); Serial.println(ladosHechos + 1);
          }
        }
      }
      break;

    case RECTA: {
      // Se para sola: asi se puede largar en el piso sin cable y sin que
      // se escape. Sin esto habria que correrla para apagarla.
      if (ahora - t_recta >= MS_RECTA_SOLA) {
        parar();
        fase = QUIETO;
        digitalWrite(LED, LOW);
        desvioFinal = diferencia(rumboObjetivo, ultimoRumbo);
        msCorrida = ahora - t_recta;
        Serial.print(">> LISTO — desvio final ");
        Serial.print(desvioFinal, 1);
        Serial.print(" / maximo "); Serial.println(desvioMax, 1);
        break;
      }
      avanzarConRumbo();

      // Guardar el PEOR desvio, no solo el del final: si el robot se torcio
      // y despues se enderezo, el numero final no lo cuenta y perderiamos
      // justo el dato que dice si la correccion trabaja o no.
      {
        float d = diferencia(rumboObjetivo, ultimoRumbo);
        if (fabs(d) > fabs(desvioMax)) desvioMax = d;
      }
      // Cada medio segundo dice cuanto se esta desviando. Si el numero
      // crece sin parar, el signo de correccion esta al reves (tecla 'k').
      static unsigned long t_aviso = 0;
      if (ahora - t_aviso >= 500) {
        t_aviso = ahora;
        Serial.print("   desvio = ");
        Serial.print(diferencia(rumboObjetivo, ultimoRumbo), 1);
        Serial.println(" grados");
      }
      break;
    }

    case PAUSA:
      parar();
      if (ahora - t_fase >= MS_PAUSA_ENTRE_VUELTAS) {
        vuelta++; ladosHechos = 0;
        fase = LADO; t_fase = ahora; reiniciarCorreccion();
        Serial.print(">> VUELTA "); Serial.print(vuelta);
        Serial.println(" — LADO 1");
      }
      break;
  }
}
