/* =====================================================================
   DELANTERO — firmware del robot, Roboliga 2026
   IITA Salta — taller de los martes
   =====================================================================

   ESTE ES EL FIRMWARE VIVO DEL DELANTERO. Lo que se prueba y anda termina
   aca. Las pruebas sueltas de diagnostico viven en ../../pruebas/ y son
   descartables; esto no.

   Arranca como copia de pruebas/buscar-pelota/, validado en banco el
   2026-07-28. Robot: el que el equipo llama "robot 2" = DELANTERO.

   Mapeo de ruedas MEDIDO en banco (no deducido del codigo):
        pines  8 / 7 / 6   = IZQUIERDA
        pines 11 / 12 / 4  = DERECHA
        pines  2 / 5 / 3   = TRASERA

   Antes de tocar nada, leer ../README.md: que se sabe medido, que NO esta
   confirmado, y como no pisarse con la sesion del arquero.

   ---------------------------------------------------------------------
   LA MAQUINA DE ESTADOS
   ---------------------------------------------------------------------

     BUSCANDO    no ve la pelota           -> gira lento a pulsitos
     CENTRANDO   la ve de costado          -> gira a pulsitos HACIA ella
     AVANZANDO   la ve y centrada          -> va derecho hacia ella
     ORBITANDO   la tiene cerca (<25 cm)   -> da la vuelta ALREDEDOR de la
                                              pelota buscando el arco AZUL
     PATEA_ADEL  pelota alineada con arco  -> 1 s a maxima potencia
     PATEA_ATRAS despues de patear         -> retrocede y vuelve a buscar

   ---------------------------------------------------------------------
   LA ORBITA — de donde sale
   ---------------------------------------------------------------------
   No la invente: es la maniobra del delantero que gano el Nacional 2025,
   estado CENTRANDO_horario (delantero.ino:613-617). Manda las dos ruedas
   de adelante SUAVE para un lado y la TRASERA FUERTE para el otro, en
   relacion 1 : 1 : 3.

   Esa asimetria es la clave: si las tres fueran iguales el robot giraria
   sobre su propio eje y la pelota se le escaparia. Con la trasera
   empujando mucho mas, el robot describe un ARCO AMPLIO y la pelota le
   queda adentro de la curva. Por eso "orbita" en vez de "girar".

   Los numeros originales (60/60/180 x c=0.4 => 24/24/72) quedan por
   DEBAJO del piso de arranque de este robot hoy, asi que estan escalados
   manteniendo la relacion.

   ---------------------------------------------------------------------
   CUANDO PATEA — el criterio del 2025
   ---------------------------------------------------------------------
        abs(Yp - Yarco) <= TOL_ALINEADO        (delantero.ino:621)

   Yp es hacia donde esta la PELOTA y Yarco hacia donde esta el ARCO. Si
   los dos numeros son parecidos, es que estan en la MISMA DIRECCION
   vistos desde el robot: o sea, robot -> pelota -> arco alineados. Ahi
   empujar la pelota derecho la manda al arco.

   NO alcanza con "ver el arco": hay que verlo DETRAS de la pelota.

   ---------------------------------------------------------------------
   El arco AZUL es el byte 203 del paquete de la camara.
   Correr en el PISO, con espacio. Monitor serie a 19200.
   ===================================================================== */

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


// ================= PERILLAS =================

const bool GIRO_INVERTIDO   = true;   // sentido del giro de busqueda/centrado
const bool ORBITA_INVERTIDA = true;  // <<< para que orbite al otro lado

// --- histeresis del centrado ---
const int TOL_ENTRA = 10;
const int TOL_SALE  = 5;

// --- distancias ---
const int XP_ORBITA = 22;   // mas cerca que esto -> empieza a orbitar
const int XP_SUELTA = 55;   // si se le aleja mas que esto, vuelve a avanzar
const int XP_MAX    = 150;  // arriba de esto no le creo (la camara recorta en 200)

// --- alineacion con el arco ---
const int TOL_ALINEADO = 12;   // |Yp - Yarco| menor a esto = alineado. Subilo si nunca patea.

// --- giro para BUSCAR ---
const int VEL_GIRO       = 80;
const int MS_PULSO_BUSC  = 60;
const int MS_ESPERA_BUSC = 380;

// --- giro para CENTRAR ---
const int VEL_CENT       = 78;
const int MS_PULSO_CENT  = 32;
const int MS_ESPERA_CENT = 320;

// --- avance ---
const int VEL_AVANCE = 55;

// --- ORBITA PEGADA A LA PELOTA (2026-08-04) ---
//
// OBJETIVO: girar alrededor de la pelota casi rozandola, ~5 cm de aire entre
// el robot y la pelota. En numeros: el centro del robot a unos 17-18 cm del
// centro de la pelota.
//
// POR QUE ESE NUMERO SALE GRATIS. La cinematica del omni de 3 ruedas dice que
// si las dos de ADELANTE no giran (velocidad cero) y solo empuja la TRASERA,
// el radio del circulo queda clavado en:
//
//         R = 2 * L        (L = del centro del robot al centro de una rueda)
//
// Con L medido con regla = 8,75 cm  ->  R = 17,5 cm. Justo lo que queremos.
// Y fijate lo que NO hace falta: ni la curva PWM->velocidad, ni la camara, ni
// ningun lazo de control. El radio no depende de con cuanta fuerza empujes:
// solo de la geometria del chasis. La trasera decide la VELOCIDAD, no el radio.
//
// COMO SE LOGRA QUE LAS DE ADELANTE "NO GIREN". No se apagan: se les manda un
// PWM por DEBAJO del piso de arranque (~70). Con eso el motor queda energizado
// pero no llega a vencer el rozamiento del engranaje: zumba y se planta. Una
// rueda omni plantada es justo la condicion v=0 que pide la formula.
//
// ASI LO HACIA EL CAMPEON 2025. Su orbita (delantero.ino:613-617, con c=0.4)
// mandaba 24 / 24 / 72. Ese 24 esta MUY por debajo de cualquier piso: sus
// ruedas de adelante NO giraban. O sea que su "relacion 1:3" nunca fue una
// relacion — era exactamente este caso, la trasera sola. Nos costo toda una
// tarde entenderlo.
//
// EL IMPULSO DE ARRANQUE (2026-08-04) — recuperado del delantero campeon 2025.
//
// EL PROBLEMA. Un motor parado necesita ~70 de PWM para arrancar, pero YA
// RODANDO se sostiene con ~40. Son dos numeros distintos. Si mandas 48 desde
// quieto, el motor zumba y no arranca; si mandas 48 cuando ya viene girando,
// sigue girando tranquilo. O sea: la velocidad lenta que queremos EXISTE, pero
// no se puede alcanzar desde el reposo yendo directo.
//
// LA SOLUCION. Arrancar fuerte un ratito y despues bajar. El golpe vence el
// rozamiento estatico; una vez en movimiento, la inercia hace el resto y el
// motor se sostiene muy por debajo de su piso de arranque. Es lo mismo que
// empujar un auto: cuesta despegarlo, despues rueda con un dedo.
//
// ASI LO HACIA EL CAMPEON 2025 (robots-2025/delantero/delantero.ino, bloque
// ROBOT2, con c=0.4 e ic=0.55). Su orbita eran DOS estados encadenados:
//
//   IMPULSO_CENTRANDO_horario      33 / 33 /  99   durante 300 ms
//   CENTRANDO_horario              24 / 24 /  72   el resto del tiempo
//
// (en el otro sentido el impulso duraba 500 ms). Nunca orbito a potencia alta
// sostenida. Nosotros veniamos corriendo la trasera a 120 FIJO, sin impulso.
//
// POR QUE EL IMPULSO DE LA ORBITA ES MAS SUAVE QUE EL DE GIRAR EN EL EJE. Para
// girar sobre su propio eje el 2025 usaba 150; para orbitar, 99. No es un
// descuido: orbitando la pelota esta a ~17 cm, y un golpe muy bruto LA EMPUJA.
// Si la pelota se corre, deja de estar en el centro de la orbita y el robot se
// descentra solo. Impulso suave = la pelota se queda quieta.
//
// SINTONIA — dos perillas y un tiempo:
//   VEL_ORB_TRASERA  = velocidad de la vuelta, YA rodando. Mas bajo = mas lento.
//                      NO cambia el radio. El 2025 uso 72; abajo de 40 se planta.
//   VEL_ORB_IMPULSO  = el golpe inicial. Si la orbita no arranca, NO subas esto
//                      primero: subi MS_ORB_IMPULSO. Mas golpe empuja la pelota.
//   MS_ORB_IMPULSO   = cuanto dura el golpe. Es la perilla correcta para "no
//                      arranca".
//   VEL_ORB_FRENTE   = 30. Tiene que quedar DEBAJO del piso (~70) para que las
//                      de adelante no giren. Si al mirarlas ves que giran,
//                      bajalo a 20. Si el robot se traba y no avanza, subilo
//                      de a 5 — pero nunca cerca de 70.
//
// COMO VOLVER A LO DE ANTES, exacto: VEL_ORB_IMPULSO = VEL_ORB_TRASERA = 120
// y MS_ORBITA_MAX = 9000. Queda igual que el 2026-08-04 a la manana.
const int VEL_ORB_FRENTE  = 30;    // DEBAJO del piso a proposito: no deben girar
const int VEL_ORB_IMPULSO = 99;    // el golpe. Es el 180*ic del campeon 2025.
const int MS_ORB_IMPULSO  = 300;   // cuanto dura el golpe. El 2025: 300 y 500 ms.
const int VEL_ORB_TRASERA = 48;    // <<< velocidad de la vuelta ya rodando

// OJO: esto va de la mano con VEL_ORB_TRASERA. Si la vuelta se hace mas lenta y
// el tiempo maximo no se sube, el robot SE RINDE ANTES DE COMPLETAR UNA VUELTA
// y parece que "empeoro al ir mas lento". Tiene que alcanzar para ~2 vueltas:
// cronometren una vuelta y pongan el doble.
const unsigned long MS_ORBITA_MAX = 20000;  // si no encuentra el arco, se rinde

// --- patada ---
const int VEL_PATADA    = 240;
const int MS_PATADA     = 1000;
const int VEL_RETROCESO = 110;
const int MS_RETROCESO  = 700;

const unsigned long MS_GRACIA    = 300;
const unsigned long SIN_DATOS_MS = 1500;

// ============================================

int Xp = 0, Yp = 0, Xaz = 0, Yaz = 0;
int XpBueno = 0, YpBueno = 0;      // ultima posicion BUENA de la pelota
int YazBueno = 0;                  // ultima direccion BUENA del arco azul

unsigned long t_ultimaPelota  = 0;
unsigned long t_ultimoArcoAzul = 0;
unsigned long t_ultimoPaquete = 0;
unsigned long t_ultimoAviso   = 0;
unsigned long t_cicloPulso    = 0;
unsigned long t_entroEstado   = 0;

enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO, PATEA_ADEL, PATEA_ATRAS };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

bool avisadoSinCamara = false;


// ---------- motores ----------

void parar() {
  analogWrite(IZQ_PWM, 0); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, 0); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0); digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void motoresRotando(bool sentidoA, int vel) {
  int a = sentidoA ? 1 : 0;
  int b = sentidoA ? 0 : 1;
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, vel); digitalWrite(TRA_INA, a); digitalWrite(TRA_INB, b);
}

void avanzar(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 1); digitalWrite(IZQ_INB, 0);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 0); digitalWrite(DER_INB, 1);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); digitalWrite(IZQ_INA, 0); digitalWrite(IZQ_INB, 1);
  analogWrite(DER_PWM, vel); digitalWrite(DER_INA, 1); digitalWrite(DER_INB, 0);
  analogWrite(TRA_PWM, 0);   digitalWrite(TRA_INA, 0); digitalWrite(TRA_INB, 0);
}

// ORBITA: la maniobra del delantero 2025 (delantero.ino:613-617).
// Adelante suave, trasera fuerte y al reves => arco amplio alrededor
// de la pelota, en vez de girar sobre el propio eje.
// velTrasera se pasa desde afuera porque los primeros MS_ORB_IMPULSO ms va el
// golpe de arranque y despues la velocidad de crucero. Las de adelante van
// SIEMPRE igual: su trabajo es quedarse plantadas, no empujar.
void orbitar(bool sentidoA, int velTrasera) {
  int a = sentidoA ? 0 : 1;    // las dos de adelante
  int b = sentidoA ? 1 : 0;
  analogWrite(IZQ_PWM, VEL_ORB_FRENTE);  digitalWrite(IZQ_INA, a); digitalWrite(IZQ_INB, b);
  analogWrite(DER_PWM, VEL_ORB_FRENTE);  digitalWrite(DER_INA, a); digitalWrite(DER_INB, b);
  analogWrite(TRA_PWM, velTrasera);      digitalWrite(TRA_INA, b); digitalWrite(TRA_INB, a);
}

void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso)                    motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera))  parar();
  else                                                  t_cicloPulso = millis();
}


// ---------- camara ----------

void leerCamara() {
  while (Serial1.available() >= 9) {
    int h1 = Serial1.read();
    if (h1 != 201) continue;

    int xp  = Serial1.read();
    int yp  = Serial1.read();
    int h2  = Serial1.read();
    int xam = Serial1.read();
    int yam = Serial1.read();
    int h3  = Serial1.read();
    int xaz = Serial1.read();
    int yaz = Serial1.read();
    (void)xam; (void)yam;

    if (h2 == 202 && h3 == 203) {
      Xp = xp;  Yp = yp - 100;
      Xaz = xaz; Yaz = yaz - 100;
      t_ultimoPaquete = millis();

      // 200 y +-100 son los TOPES de recorte de la camara: casi siempre manchas.
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xaz > 0) && (Xaz <= XP_MAX) && (abs(Yaz) < 100)) {
        YazBueno = Yaz;
        t_ultimoArcoAzul = millis();
      }
    }
  }
}


const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO:    return "BUSCANDO";
    case CENTRANDO:   return "CENTRANDO";
    case AVANZANDO:   return "AVANZANDO";
    case ORBITANDO:   return "ORBITANDO";
    case PATEA_ADEL:  return "PATEANDO!";
    case PATEA_ATRAS: return "retrocede";
  }
  return "?";
}

void cambiarA(Estado nuevo) {
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
}


void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Serial1.begin(19200);

  while (!Serial && millis() < 3000) { }
  Serial.println();
  Serial.println("==============================================");
  Serial.println("BUSCAR - CENTRAR - AVANZAR - ORBITAR - PATEAR");
  Serial.print("orbita si Xp<"); Serial.println(XP_ORBITA);
  Serial.print("patea si |Yp - Yarcoazul| <= "); Serial.println(TOL_ALINEADO);
  Serial.print("orbita: impulso "); Serial.print(VEL_ORB_IMPULSO);
  Serial.print(" x "); Serial.print(MS_ORB_IMPULSO);
  Serial.print(" ms  ->  crucero "); Serial.print(VEL_ORB_TRASERA);
  Serial.print("   (max "); Serial.print(MS_ORBITA_MAX / 1000); Serial.println(" s)");
  Serial.println("==============================================");
  Serial.println("Arranca en 3 segundos.");
  delay(3000);

  t_ultimoPaquete = millis();
  cambiarA(BUSCANDO);
}


void loop() {

  leerCamara();

  bool laVeo    = (millis() - t_ultimaPelota)   < MS_GRACIA;
  bool veoArco  = (millis() - t_ultimoArcoAzul) < MS_GRACIA;
  unsigned long enEstado = millis() - t_entroEstado;

  // ---------- la patada NO se interrumpe ----------
  if (estado == PATEA_ADEL) {
    avanzar(VEL_PATADA);
    if (enEstado >= (unsigned long)MS_PATADA) cambiarA(PATEA_ATRAS);
  }
  else if (estado == PATEA_ATRAS) {
    retroceder(VEL_RETROCESO);
    if (enEstado >= (unsigned long)MS_RETROCESO) cambiarA(BUSCANDO);
  }

  // ---------- ORBITANDO ----------
  else if (estado == ORBITANDO) {

    if (!laVeo) {                                   // se le escapo la pelota
      Serial.println("... perdi la pelota orbitando");
      cambiarA(BUSCANDO);
    }
    else if (XpBueno > XP_SUELTA) {                 // se le alejo: vuelve a ir
      cambiarA(AVANZANDO);
    }
    else if (veoArco && abs(YpBueno - YazBueno) <= TOL_ALINEADO) {
      Serial.print("*** ALINEADO con el arco azul  (Yp="); Serial.print(YpBueno);
      Serial.print("  Yarco="); Serial.print(YazBueno);
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (enEstado > MS_ORBITA_MAX) {            // se rinde
      Serial.print("... orbite "); Serial.print(MS_ORBITA_MAX / 1000);
      Serial.println(" s y no encontre el arco azul alineado");
      cambiarA(BUSCANDO);
    }
    else {
      // Los primeros MS_ORB_IMPULSO ms de CADA entrada a ORBITANDO van con el
      // golpe de arranque; despues baja a la velocidad de crucero y sigue por
      // inercia. enEstado se reinicia solo en cambiarA(), asi que el golpe se
      // da una vez por orbita y no se repite.
      bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
      orbitar(!ORBITA_INVERTIDA, enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
    }
  }

  // ---------- el resto ----------
  else {
    if (!laVeo) {
      if (estado != BUSCANDO) cambiarA(BUSCANDO);
      rotarPulsado(!GIRO_INVERTIDO, VEL_GIRO, MS_PULSO_BUSC, MS_ESPERA_BUSC);
    }
    else if (XpBueno < XP_ORBITA) {
      Serial.print("*** llegue a "); Serial.print(XpBueno);
      Serial.println(" cm -> a orbitar buscando el arco azul");
      cambiarA(ORBITANDO);
    }
    else {
      int desvio = abs(YpBueno);
      if (estado == CENTRANDO) {
        if (desvio < TOL_SALE) cambiarA(AVANZANDO);
      } else {
        if (desvio > TOL_ENTRA)        cambiarA(CENTRANDO);
        else if (estado != AVANZANDO)  cambiarA(AVANZANDO);
      }

      if (estado == CENTRANDO) {
        bool haciaUnLado = (YpBueno > 0);
        if (GIRO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
      } else {
        avanzar(VEL_AVANCE);
      }
    }
  }

  // ---------- avisos ----------
  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    Serial.print(">>> "); Serial.print(nombreEstado(estado));
    Serial.print("   Xp="); Serial.print(XpBueno);
    Serial.print(" Yp=");  Serial.print(YpBueno);
    Serial.print("  arco: ");
    if (veoArco) { Serial.print("Yaz="); Serial.println(YazBueno); }
    else         { Serial.println("no lo veo"); }
  }

  bool sinDatos = (millis() - t_ultimoPaquete > SIN_DATOS_MS);
  if (sinDatos && !avisadoSinCamara) {
    Serial.println("!!! NO LLEGAN DATOS DE LA CAMARA !!!");
    avisadoSinCamara = true;
  }
  if (!sinDatos) avisadoSinCamara = false;

  if (millis() - t_ultimoAviso > 2000) {
    t_ultimoAviso = millis();
    Serial.print("   ["); Serial.print(nombreEstado(estado));
    Serial.print("]  Xp="); Serial.print(XpBueno);
    Serial.print(" Yp="); Serial.print(YpBueno);
    Serial.print("  Yaz=");
    if (veoArco) Serial.print(YazBueno); else Serial.print("--");
    Serial.print("  dif=");
    if (veoArco) Serial.println(abs(YpBueno - YazBueno)); else Serial.println("--");
  }
}
