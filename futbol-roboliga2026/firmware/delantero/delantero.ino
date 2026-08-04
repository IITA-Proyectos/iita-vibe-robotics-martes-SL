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
   CAMBIO DEL 2026-08-04 — LA ORBITA SE REESCRIBIO ENTERA
   ---------------------------------------------------------------------
   Lo demas (buscar, centrar, avanzar, patear, leer la camara, histeresis)
   quedo INTACTO. Solo cambio la orbita. Por que:

   La orbita vieja mandaba adelante=50 y trasera=100 en polaridad contraria.
   Ese 1:2 exacto es, segun la cinematica, el punto donde el robot DEJA DE
   GIRAR y solo se desliza de costado: se va derecho al lado de la pelota
   en vez de rodearla. Que es exactamente el sintoma que vimos en banco.

   La cuenta (vale para CUALQUIER robot de 3 ruedas omni repartidas a 120
   grados, sin importar cuanto valga el radio del chasis ni como esten
   orientadas las ruedas):

        velocidad de giro  =  (v_izq + v_der + v_trasera) / (3 * L)

   O sea: el GIRO sale de la SUMA de las tres, con signo. Con adelante +50,
   +50 y trasera -100, la suma da 50 + 50 - 100 = CERO. Giro cero. Sin giro
   no hay circulo: hay una linea recta de costado.
   Esto es DEDUCIDO de la cinematica, no medido. Pero encaja con los tres
   intentos del banco: trasera 210 (suma +110, rodeaba pero patinaba),
   130 (suma +30, "seguia yendose"), 100 (suma 0, se va del todo).

   ---------------------------------------------------------------------
   LA ORBITA NUEVA — el mecanismo, en castellano
   ---------------------------------------------------------------------
   Pensa en una persona caminando alrededor de una mesa SIN dejar de
   mirarla. Hace dos cosas al mismo tiempo:
       1) camina de costado  (eso lo hace la rueda TRASERA)
       2) va girando el cuerpo (eso lo hacen las dos de ADELANTE)
   Si camina de costado y no gira, deja la mesa atras. Si gira y no camina,
   gira en el lugar. El circulo es la RELACION EXACTA entre las dos cosas.

   Y aca esta la parte que ordena todo:

       * la RELACION entre las ruedas fija el RADIO del circulo
       * la ESCALA de las tres juntas fija la VELOCIDAD

   Son dos perillas separadas. Todo el martes estuvimos moviendo la que no
   era: subiamos y bajabamos la trasera (que es la de velocidad) buscando
   cerrar el circulo (que es la de relacion).

   ---------------------------------------------------------------------
   POR QUE PULSOS Y NO PWM DISTINTOS
   ---------------------------------------------------------------------
   La cuenta pide que las de adelante vayan MUCHISIMO mas despacio que la
   trasera (del orden de 1 a 6, no 1 a 2). Ese PWM tan bajo cae debajo del
   piso: el motor zumba y no gira. Y peor: el PWM NO es proporcional a la
   velocidad, asi que mandar la mitad de PWM no da la mitad de velocidad.

   Solucion: las TRES ruedas usan EL MISMO PWM (uno solo, arriba del piso),
   y lo que cambia es CUANTO TIEMPO esta prendida cada una.
       - la TRASERA queda prendida SIEMPRE (es la que mantiene el robot en
         movimiento; por eso las de adelante nunca tienen que arrancar
         desde quieto y les alcanza con el piso bajo)
       - las de ADELANTE se prenden solo una fraccion de cada ventana

   El milisegundo SI es lineal: 30 ms mueven exactamente la mitad que 60 ms.
   Y como las tres usan el mismo PWM, la relacion entre ellas ya no depende
   de en que parte de la curva del motor estemos. Bajar PWM_ORB para ir mas
   lento NO cambia el radio del circulo.
   (Supuesto no medido: que los tres motores tienen curvas parecidas. Si no,
    queda un sesgo; el lazo de la camara lo come, ver mas abajo.)

   ---------------------------------------------------------------------
   CUANDO PATEA — el criterio del 2025 (SIN CAMBIOS)
   ---------------------------------------------------------------------
        abs(Yp - Yarco) <= TOL_ALINEADO        (delantero.ino:621 del 2025)

   Yp es hacia donde esta la PELOTA y Yarco hacia donde esta el ARCO. Si
   los dos numeros son parecidos, es que estan en la MISMA DIRECCION
   vistos desde el robot: o sea, robot -> pelota -> arco alineados.

   ---------------------------------------------------------------------
   REVISION DEL 2026-08-04 (pase de refutacion, antes de cargar al robot)
   ---------------------------------------------------------------------
   Lo que se VERIFICO y quedo bien:
     - Compila limpio (g++ -Wall -Wextra sobre el sketch con stubs de
       Arduino: cero errores, cero warnings). NO se compilo con Teensyduino
       ni se cargo al robot: FALTA VALIDAR EN BANCO.
     - Los pines son los MEDIDOS y no se toco ninguno.
     - Ningun PWM sale calculado: todos son constantes entre 0 y 255.
       No hay forma de mandar un valor negativo ni mayor a 255.
     - El signo del lazo de la camara es el correcto (menos duty => circulo
       mas chico; se verifico derivando la formula del duty).

   Lo que se CORRIGIO en esta revision (6 cosas):
     1. UNA lectura mentirosa de la camara sacaba al robot de la orbita.
        Esta MEDIDO que con la pelota quieta Xp salto a 146 y XP_SUELTA
        vale 60. Ese unico cuadro lo mandaba a AVANZANDO, que va DERECHO
        contra la pelota a PWM 55. Ahora hay que estar lejos SOSTENIDO
        (MS_CONFIRMA_SUELTA).
     2. Con el lazo APAGADO no se muestreaba nada, pero el log seguia
        imprimiendo "radio medido=" con el valor de entrada, clavado.
        Justo en el modo que sirve para comparar. Ahora se mide siempre;
        lo que se apaga es la correccion.
     3. Cuando el duty pedido era muy chico, la ventana se recortaba en
        600 ms y el duty REAL quedaba en 0.05 en vez del pedido — y el log
        mostraba el pedido. Pasa con L=9 (el valor por defecto) apenas el
        ajuste llega a su tope. Ahora avisa: "RELACION no se sostiene".
     4. L_estimado se imprimia siempre, incluso con duty 0, donde da Xp/2:
        un numero con cara de medicion del chasis que no mide nada. Ahora
        solo sale cuando significa algo, y si no, dice por que no.
     5. Cada vez que se cortaba la orbita (timeout o "se me alejo") el lazo
        borraba todo lo aprendido y empezaba de cero. Como el timeout no es
        una salida real (ver MS_ORBITA_MAX), nunca terminaba de converger.
        Ahora se acuerda por MS_MEMORIA_ORB.
     6. INA=1 e INB=1 al mismo tiempo: no quedaba nunca de forma estable,
        pero SI habia un instante entre las dos escrituras al cambiar de
        sentido (por ejemplo PATEA_ADEL -> PATEA_ATRAS). Ahora toda la
        direccion pasa por direccionMotor(), que apaga las dos patas antes
        de prender la que va. El estado (1,1) dejo de ser alcanzable.

   Lo que sigue SIN RESOLVER y hay que mirar en banco (no es codigo):
     - L_CHASIS_CM = 9.0 es un SUPUESTO. Medirlo (M1 y M2).
     - TRASERA_OPUESTA: si esta al reves, el lazo corrige para el lado
       equivocado y el ajuste se va a pegar al tope. El log ahora lo grita.
     - Si el circulo sale con forma de tuerca, es el supuesto de
       frenteSuelto() (ver el comentario ahi), no el duty.

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
const bool ORBITA_INVERTIDA = true;   // <<< para que orbite al otro lado

// --- histeresis del centrado ---
const int TOL_ENTRA = 10;
const int TOL_SALE  = 5;

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


/* =====================================================================
   ORBITA — LAS PERILLAS
   ===================================================================== */

// ---------------------------------------------------------------------
// 1) EL OBJETIVO, en CENTIMETROS. No es un PWM magico: es el radio del
//    circulo que queremos que describa el robot alrededor de la pelota.
//    SUBIRLO  -> circulo mas grande
//    BAJARLO  -> circulo mas chico
//    OJO: esta en "centimetros de camara". Si la camara no esta calibrada,
//    Xp=30 no son 30 cm de verdad. Prueba M3 del plan: pelota a 20/30/40 cm
//    con cinta metrica y anotar que Xp reporta. Si reporta distinto, corregi
//    este numero (no la cinta metrica).
// ---------------------------------------------------------------------
const float RADIO_ORBITA_CM = 30.0;

// ---------------------------------------------------------------------
// 2) EL RADIO DEL CHASIS: del centro del robot al centro de una rueda.
//    *** ESTE NUMERO NO ESTA MEDIDO. 9.0 es un valor de arranque tomado
//        de un ejemplo del skill, NO de este robot. MEDILO CON REGLA. ***
//    Dos formas de medirlo (que se controlan entre si):
//      a) regla: medi las 3 distancias entre puntos de apoyo de las ruedas.
//         Si las tres dan igual (d), entonces  L = d / 1.732
//      b) comportamiento: prendé SOLO la trasera 3 s y medi el circulo que
//         traza. Ese circulo tiene radio 2*L exacto. L = diametro / 4.
//    Si (a) y (b) no coinciden, hay algo mal en un supuesto: pará y resolvelo.
//
//    De aca sale la relacion entre ruedas. Formula (deducida, no medida):
//         duty = (RADIO - 2*L) / (2 * (L + RADIO))
//    Tabla para RADIO = 30 cm:
//         L= 7 cm -> duty 0.216 -> trasera:adelante = 4.6 : 1
//         L= 8 cm -> duty 0.184 -> 5.4 : 1
//         L= 9 cm -> duty 0.154 -> 6.5 : 1
//         L=10 cm -> duty 0.125 -> 8.0 : 1
//         L=12 cm -> duty 0.071 -> 14  : 1
//         L=15 cm -> duty 0.000 -> las de adelante APAGADAS (2*L ya es 30)
//    Fijate lo sensible que es: 2 cm de error en L duplican la relacion.
//    Por eso el lazo de la camara (perilla 6) no es un lujo.
// ---------------------------------------------------------------------
const float L_CHASIS_CM = 9.0;    // <<< MEDIR. Hoy es un supuesto.

// ---------------------------------------------------------------------
// 3) LA VELOCIDAD. Es el PWM que usan LAS TRES ruedas por igual.
//    SUBIRLO  -> da la vuelta mas rapido (y arriba de ~200 patina)
//    BAJARLO  -> mas lento (lo que queremos). Abajo del piso de sosten el
//                robot se planta y deja de moverse.
//    *** BAJAR ESTO NO CAMBIA EL RADIO DEL CIRCULO. *** Es la unica perilla
//    de velocidad. Empeza en 90 y bajá de a 10 hasta que el robot se plante:
//    ese es "lo mas lento posible" de este robot. Ese numero NO esta medido.
// ---------------------------------------------------------------------
const int PWM_ORB = 90;

// ---------------------------------------------------------------------
// 4) EL ARRANQUE. Desde quieto el piso es ~70 y con el robot cargado puede
//    ser mas. Entonces el primer envion se manda fuerte y corto, y despues
//    baja a PWM_ORB. Sin esto la orbita puede no arrancar nunca.
//    (piso de arranque ~70 = MEDIDO grueso; piso de sosten ~40 = MEDIDO hoy)
// ---------------------------------------------------------------------
const int PWM_KICK_ORB = 170;
const int MS_KICK_ORB  = 90;

// ---------------------------------------------------------------------
// 5) LA VENTANA DE PULSOS. Cada ventana las de adelante se prenden un ratito
//    y el resto van sueltas (arrastradas). La trasera nunca se apaga.
//    SUBIR MS_VENTANA_ORB  -> movimiento mas "a tirones" pero pulsos mas
//                             largos (mas confiables si son muy cortos)
//    BAJARLA               -> mas suave, pero si el pulso queda mas corto
//                             que MS_PULSO_MIN_ORB el motor no llega a mover
//    MS_PULSO_MIN_ORB: el pulso mas corto que todavia mueve la rueda.
//    *** NO ESTA MEDIDO. 30 ms es prudencia, no dato. ***
//    Si el duty calculado da un pulso mas corto que eso, el codigo ALARGA la
//    ventana solo, para no perder la relacion.
// ---------------------------------------------------------------------
const int MS_VENTANA_ORB  = 200;
const int MS_PULSO_MIN_ORB = 30;
// Tope duro de cuanto puede estirarse la ventana. Mas larga que esto y el
// movimiento deja de promediar: son tirones sueltos, no un circulo.
const int MS_VENTANA_MAX_ORB = 600;

// ---------------------------------------------------------------------
// 6) EL LAZO DE LA CAMARA: corrige el radio mirando Xp.
//    Esto es lo que salva que L no este medido: si el circulo sale mas
//    grande que RADIO_ORBITA_CM, el robot le va sacando duty de a poquito
//    hasta cerrarlo; si sale mas chico, le agrega. Va LENTO a proposito.
//    KI_RADIO  subir -> corrige mas rapido pero puede entrar y salir a los
//                       tirones (zig-zag radial)
//              bajar -> mas tranquilo, tarda mas en llegar a los 30
//    AJUSTE_MAX: tope de cuanto puede corregir. Si el ajuste se queda pegado
//                al tope, es que L esta MUY mal medido: medilo de nuevo.
//    BANDA_XP: "si estoy a menos de esto del objetivo, no toco nada".
//              Evita perseguir el ruido de la camara.
//    ORBITA_LAZO_CERRADO = false  -> apaga el lazo y queda orbita a lazo
//              abierto puro, para comparar las dos en la misma tarde con la
//              misma bateria. UN CAMBIO POR VEZ.
// ---------------------------------------------------------------------
const bool  ORBITA_LAZO_CERRADO = true;
const float KI_RADIO   = 0.0012;   // duty por (cm de error x ventana)
const float AJUSTE_MAX = 0.12;
const int   BANDA_XP   = 4;        // cm

// ---------------------------------------------------------------------
// 7) ENTRADA Y SALIDA DE LA ORBITA.
//    XP_ORBITA tiene que ser MAYOR que el radio objetivo: si no, el robot
//    solo empieza a orbitar cuando ya esta MAS CERCA que el circulo que
//    quiere, y el lazo tiene que empujarlo para afuera. (Antes valia 25 con
//    objetivo 30: al reves. Cambiado el 2026-08-04.)
// ---------------------------------------------------------------------
const int XP_ORBITA = 40;   // mas cerca que esto -> empieza a orbitar
const int XP_SUELTA = 60;   // si se le aleja mas que esto, vuelve a avanzar
const int XP_MAX    = 150;  // arriba de esto no le creo (la camara recorta en 200)

// CUANTO TIENE QUE DURAR el "se me alejo" para hacerle caso.
// Esto NO es un detalle: esta MEDIDO que con la pelota QUIETA la camara
// reporto un salto de 60 a 146 cm. XP_SUELTA vale 60. O sea que UNA sola
// lectura mentirosa alcanzaba para sacar al robot de la orbita y mandarlo a
// AVANZANDO, que va DERECHO contra la pelota a PWM 55 y se la lleva puesta.
// Y de paso se perdia todo lo aprendido por el lazo (reiniciarOrbita borra
// el ajuste). Con esto, para soltar, tiene que estar lejos de forma
// SOSTENIDA durante este tiempo. 250 ms son varios cuadros de camara.
// Si de verdad se le aleja la pelota, tarda 250 ms mas en reaccionar: barato.
const unsigned long MS_CONFIRMA_SUELTA = 250;

// Cuanto aguanta orbitando sin encontrar el arco.
// En BANCO subilo (una vuelta lenta tarda 12-20 s y con 9000 nunca la completa).
// Para PARTIDO bajalo a ~9000: orbitar 20 s es regalar la pelota.
//
// OJO, esto NO es una salida de verdad: al cumplirse el tiempo el robot va a
// BUSCANDO, pero si sigue viendo la pelota cerca vuelve a ORBITANDO en la
// iteracion siguiente. En los hechos orbita para siempre; lo unico que hace el
// timeout es imprimir una linea y reiniciar la orbita. NO es un cuelgue (el
// robot se sigue moviendo y la maquina de estados sigue viva), pero tampoco
// resuelve "no encuentro el arco". Si en partido hace falta que haga OTRA cosa
// al vencerse el tiempo (patear igual, retroceder, cambiar de sentido), hay que
// escribir esa conducta: hoy no existe.
const unsigned long MS_ORBITA_MAX = 20000;

// Cuanto tiempo se acuerda el lazo de lo que aprendio, si sale y vuelve a
// entrar a la orbita. Sin esto, cada vez que el timeout la cortaba (o un
// "se me alejo") el ajuste volvia a cero y el lazo empezaba de nuevo: nunca
// terminaba de converger, y en el monitor parecia que el lazo no servia.
const unsigned long MS_MEMORIA_ORB = 4000;

// Filtro anti-saltos de la camara. Medido: con la pelota QUIETA, Xp salto de
// 60 a 146 cm. Un lazo que le cree a eso pega un volantazo por nada.
// La mediana de 3 no puede dejar pasar un valor absurdo aislado, nunca.
const int MAX_SALTO_XP = 25;   // cm entre muestras. Mas que esto: sospechoso.
const int RECHAZOS_MAX = 4;    // si rechaza 4 seguidas, se rinde y acepta

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

// --- estado interno de la orbita ---
float dutyNominal   = 0.0;   // el que sale de la cuenta con L y RADIO
float ajusteRadio   = 0.0;   // lo que corrigio la camara (integrador lento)
float dutyEfectivo  = 0.0;
int   msFrenteOrb   = 0;     // cuantos ms se prenden las de adelante
int   msVentanaOrb  = MS_VENTANA_ORB;
bool  relacionRota  = false; // true = el duty pedido NO se pudo sostener
unsigned long t_ventanaOrb  = 0;
unsigned long t_avisoOrb    = 0;
unsigned long t_cercaOrb    = 0;   // ultima vez que la vio DENTRO de XP_SUELTA
unsigned long t_salioOrbita = 0;   // cuando dejo de orbitar la ultima vez
float ajusteGuardado = 0.0f;       // lo que habia aprendido el lazo ahi
int   histXp[3]   = {0, 0, 0};
int   nHistXp     = 0;
int   XpOrb       = 0;       // Xp filtrado, el unico que mira el lazo
int   rechazosXp  = 0;
bool  hayMedidaOrb = false;  // ya hay al menos una mediana valida?

enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO, PATEA_ADEL, PATEA_ATRAS };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

bool avisadoSinCamara = false;


// ---------- motores ----------

/* Pone la direccion de UN motor SIN pasar nunca por INA=1 e INB=1.

   Por que existe esta funcion: escribiendo digitalWrite(INA,..) y despues
   digitalWrite(INB,..), hay un instante entre las dos escrituras en que las
   DOS patas pueden quedar en 1. Pasa cada vez que un motor cambia de sentido
   (por ejemplo al pasar de PATEA_ADEL a PATEA_ATRAS). Dura nanosegundos y en
   la mayoria de los drivers eso es "freno", no un corto — pero el driver de
   la Zircon NO esta identificado en ningun papel del repo, asi que no lo
   damos por bueno.
   Aca se apagan las DOS primero y recien despues se prende la que va: el
   estado (1,1) deja de ser alcanzable por cualquier camino.

   El estado FINAL es exactamente el mismo que antes. Lo unico que cambia es
   por donde pasa en el medio. No cambia ningun comportamiento.
   dir:  +1 = INA,  -1 = INB,  0 = suelto (las dos en 0).                   */
void direccionMotor(int pinA, int pinB, int dir) {
  digitalWrite(pinA, 0);
  digitalWrite(pinB, 0);
  if (dir > 0)      digitalWrite(pinA, 1);
  else if (dir < 0) digitalWrite(pinB, 1);
}

void parar() {
  analogWrite(IZQ_PWM, 0); direccionMotor(IZQ_INA, IZQ_INB, 0);
  analogWrite(DER_PWM, 0); direccionMotor(DER_INA, DER_INB, 0);
  analogWrite(TRA_PWM, 0); direccionMotor(TRA_INA, TRA_INB, 0);
}

void motoresRotando(bool sentidoA, int vel) {
  int d = sentidoA ? +1 : -1;          // las TRES iguales = rota sobre su eje
  analogWrite(IZQ_PWM, vel); direccionMotor(IZQ_INA, IZQ_INB, d);
  analogWrite(DER_PWM, vel); direccionMotor(DER_INA, DER_INB, d);
  analogWrite(TRA_PWM, vel); direccionMotor(TRA_INA, TRA_INB, d);
}

void avanzar(int vel) {
  // MEDIDO: las dos de adelante necesitan polaridad OPUESTA entre si.
  analogWrite(IZQ_PWM, vel); direccionMotor(IZQ_INA, IZQ_INB, +1);
  analogWrite(DER_PWM, vel); direccionMotor(DER_INA, DER_INB, -1);
  analogWrite(TRA_PWM, 0);   direccionMotor(TRA_INA, TRA_INB,  0);
}

void retroceder(int vel) {
  analogWrite(IZQ_PWM, vel); direccionMotor(IZQ_INA, IZQ_INB, -1);
  analogWrite(DER_PWM, vel); direccionMotor(DER_INA, DER_INB, +1);
  analogWrite(TRA_PWM, 0);   direccionMotor(TRA_INA, TRA_INB,  0);
}

void rotarPulsado(bool sentidoA, int vel, int msPulso, int msEspera) {
  unsigned long fase = millis() - t_cicloPulso;
  if (fase < (unsigned long)msPulso)                    motoresRotando(sentidoA, vel);
  else if (fase < (unsigned long)(msPulso + msEspera))  parar();
  else                                                  t_cicloPulso = millis();
}


/* =====================================================================
   ORBITA — LOS MOTORES
   =====================================================================
   TRASERA_OPUESTA = true  -> la trasera recibe la polaridad CONTRARIA a las
                              de adelante (es lo que hace el codigo 2025)
                      false -> la trasera recibe la MISMA polaridad

   Regla para elegir, mirando el robot: las tres ruedas NO tienen que girar
   todas para el mismo lado. Si giran todas igual, el robot rota sobre su eje
   y la pelota se queda donde estaba: ahi hay que dar vuelta esta perilla.
   (Como esta montado el motor trasero NO esta medido.)

   OJO, hay una contradiccion abierta que hay que resolver en banco:
   esta MEDIDO que "las tres con la misma polaridad => rota sobre su eje".
   Si con TRASERA_OPUESTA=true seguis viendo las tres girar para el mismo
   lado, entonces o el motor trasero esta cableado al reves, o lo que se
   ve girando no es lo que parece. Prueba M5 del plan: prendé SOLO la
   trasera y mirá.
   ===================================================================== */
const bool TRASERA_OPUESTA = true;

// las dos de ADELANTE, con la MISMA polaridad entre si.
// Esa igualdad ES la condicion "no me acerco ni me alejo de la pelota".
void frenteOrbita(bool sentidoA, int pwm) {
  int d = sentidoA ? -1 : +1;
  analogWrite(IZQ_PWM, pwm); direccionMotor(IZQ_INA, IZQ_INB, d);
  analogWrite(DER_PWM, pwm); direccionMotor(DER_INA, DER_INB, d);
}

// las dos de ADELANTE sueltas. Las DOS patas en 0: es la unica forma
// confiable de apagar (solo PWM=0 no lo es). Sueltas = arrastradas por el
// chasis, sin frenar.
//
// ATENCION — esto es un SUPUESTO no medido, y es el mas fuerte de todo el
// diseño: la cuenta del duty trata a las de adelante como si tuvieran una
// velocidad PROMEDIO igual a duty x (la de la trasera). Pero cuando estan
// sueltas no tienen velocidad impuesta: giran a lo que el chasis les
// imponga. Durante la pausa quedan DOS ruedas libres y una sola empujando,
// y ahi el movimiento lo deciden la inercia y el rozamiento, no la cuenta.
// La aproximacion aguanta mientras cada ventana sea un pedacito chico de
// vuelta (con 200 ms y una vuelta de ~12 s son ~6 grados por ventana).
// Si el circulo sale con forma de tuerca en vez de redondo, ES ESTO:
// bajar MS_VENTANA_ORB, no tocar el duty.
void frenteSuelto() {
  analogWrite(IZQ_PWM, 0); direccionMotor(IZQ_INA, IZQ_INB, 0);
  analogWrite(DER_PWM, 0); direccionMotor(DER_INA, DER_INB, 0);
}

// la TRASERA: el "motor de la vuelta". Nunca se apaga durante la orbita.
void traseraOrbita(bool sentidoA, int pwm) {
  int d  = sentidoA ? -1 : +1;              // el mismo signo que las de adelante
  int dt = TRASERA_OPUESTA ? -d : d;
  analogWrite(TRA_PWM, pwm); direccionMotor(TRA_INA, TRA_INB, dt);
}

// La cuenta del duty. duty = cuanto vale la velocidad de las de adelante
// comparada con la trasera. Sale de imponer que el robot describa un circulo
// de radio RADIO_ORBITA_CM sin acercarse ni alejarse.
//      duty = (RADIO - 2L) / (2 * (L + RADIO))
// Casos borde que vale la pena entender:
//   RADIO = 2L  -> duty 0: con SOLO la trasera el robot ya traza ese circulo.
//                  Es el circulo MAS CHICO que este robot puede hacer.
//   RADIO < 2L  -> duty negativo: no se puede. El codigo lo recorta en 0 y
//                  avisa por serie.
float calcularDutyNominal() {
  float L = L_CHASIS_CM;
  float R = RADIO_ORBITA_CM;
  if (L <= 0.0) return 0.0;
  float d = (R - 2.0 * L) / (2.0 * (L + R));
  if (d < 0.0) d = 0.0;
  if (d > 0.45) d = 0.45;   // arriba de esto la cuenta se dispara (circulo infinito)
  return d;
}

/* Traduce el duty (una RELACION de velocidades) a tiempos: cuantos ms de
   cada ventana estan prendidas las de adelante.

   Si el pulso que sale es mas corto que el minimo util, ALARGA la ventana en
   vez de agrandar el pulso: asi la RELACION (que es la que fija el radio)
   queda intacta. El precio es que el movimiento sale mas a tirones.

   Y si ni con la ventana mas larga que aceptamos entra: se AVISA. Antes esto
   se tapaba solo (se recortaba la ventana en 600 y se dejaba el pulso en 30,
   o sea un duty real de 0.05 en vez del pedido) y el log seguia mostrando el
   duty pedido como si se estuviera cumpliendo. Eso es un radio distinto del
   que dice la pantalla. Ahora lo dice.                                      */
void recalcularVentanaOrbita() {
  dutyEfectivo = dutyNominal + ajusteRadio;
  if (dutyEfectivo < 0.0f)  dutyEfectivo = 0.0f;
  if (dutyEfectivo > 0.45f) dutyEfectivo = 0.45f;

  relacionRota = false;
  msVentanaOrb = MS_VENTANA_ORB;
  msFrenteOrb  = (int)(dutyEfectivo * (float)msVentanaOrb + 0.5f);

  if (dutyEfectivo > 0.001f && msFrenteOrb < MS_PULSO_MIN_ORB) {
    float ventanaPedida = (float)MS_PULSO_MIN_ORB / dutyEfectivo;
    if (ventanaPedida > (float)MS_VENTANA_MAX_ORB) {
      msVentanaOrb = MS_VENTANA_MAX_ORB;
      relacionRota = true;          // el radio real va a salir mas CHICO
    } else {
      msVentanaOrb = (int)(ventanaPedida + 0.5f);
    }
    msFrenteOrb = MS_PULSO_MIN_ORB;
  }
  if (dutyEfectivo <= 0.001f) msFrenteOrb = 0;

  if (msFrenteOrb > msVentanaOrb) msFrenteOrb = msVentanaOrb;   // red de seguridad
}

void reiniciarOrbita() {
  dutyNominal  = calcularDutyNominal();
  // Retoma lo aprendido si volvio a entrar enseguida (ver MS_MEMORIA_ORB).
  if (millis() - t_salioOrbita < MS_MEMORIA_ORB) ajusteRadio = ajusteGuardado;
  else                                           ajusteRadio = 0.0f;
  recalcularVentanaOrbita();      // <- la MISMA cuenta que en cada ventana.
                                  //    Antes la primera ventana usaba una
                                  //    cuenta propia que se saltaba el pulso
                                  //    minimo: arrancaba con un pulso corto
                                  //    que podia no mover nada.
  t_ventanaOrb = millis();
  t_cercaOrb   = millis();
  nHistXp      = 0;
  rechazosXp   = 0;
  hayMedidaOrb = false;
  XpOrb        = XpBueno;
  t_avisoOrb   = millis();
}

int mediana3(int a, int b, int c) {
  int t;
  if (a > b) { t = a; a = b; b = t; }
  if (b > c) { t = b; b = c; c = t; }
  if (a > b) { t = a; a = b; b = t; }
  return b;
}

// Toma una muestra de Xp, la filtra, y devuelve true si es confiable.
bool muestrearXpOrbita() {
  histXp[nHistXp % 3] = XpBueno;
  if (nHistXp < 1000) nHistXp++;      // tope: que el indice no se vaya nunca
                                      // a negativo si alguien sube el tiempo
                                      // maximo de orbita
  if (nHistXp < 3) return false;

  int med = mediana3(histXp[0], histXp[1], histXp[2]);

  // compuerta de plausibilidad: el robot no puede cambiar 25 cm de distancia
  // entre una ventana y la otra. Si salta mas, es la camara, no el robot.
  // Pero si rechaza muchas seguidas se rinde: capaz la pelota SI se movio.
  if (abs(med - XpOrb) > MAX_SALTO_XP && rechazosXp < RECHAZOS_MAX) {
    rechazosXp++;
    return false;
  }
  rechazosXp = 0;
  XpOrb = med;
  hayMedidaOrb = true;
  return true;
}

/* La orbita, ventana por ventana.
   Durante los primeros MS_KICK_ORB ms manda todo fuerte para despegar.
   Despues, en cada ventana:
      - la trasera prendida TODO el tiempo, a PWM_ORB
      - las de adelante prendidas los primeros msFrenteOrb ms, sueltas el resto
   Y una vez por ventana recalcula msFrenteOrb con lo que ve la camara.

   Nota sobre el parametro laVeo: en la practica siempre llega en true, porque
   la maquina de estados ya se fue a BUSCANDO si perdio la pelota. Queda igual
   por si alguien cambia esa salida: si llegara en false, no se muestrea y el
   ajuste se congela en vez de corregir a ciegas.                             */
void orbitarRegulado(bool sentidoA, unsigned long enEstado, bool laVeo) {

  // --- envion inicial: sin esto puede no arrancar nunca ---
  if (enEstado < (unsigned long)MS_KICK_ORB) {
    frenteOrbita(sentidoA, PWM_KICK_ORB);
    traseraOrbita(sentidoA, PWM_KICK_ORB);
    t_ventanaOrb = millis();
    return;
  }

  unsigned long fase = millis() - t_ventanaOrb;

  // ---------- arranca ventana nueva: se decide UNA sola vez ----------
  if (fase >= (unsigned long)msVentanaOrb) {
    t_ventanaOrb = millis();
    fase = 0;

    // El MUESTREO va SIEMPRE que la vea, este el lazo prendido o apagado.
    // (Antes el && cortaba antes de muestrear cuando el lazo estaba en false:
    //  XpOrb se quedaba clavado en el valor de entrada y el log seguia
    //  imprimiendo "radio medido=..." como si lo estuviera midiendo. Justo en
    //  el modo que sirve para COMPARAR contra el lazo cerrado.)
    bool medidaNueva = laVeo && muestrearXpOrbita();

    // Lo que se apaga con ORBITA_LAZO_CERRADO=false es la CORRECCION, no la medicion.
    if (medidaNueva && ORBITA_LAZO_CERRADO) {
      int error = XpOrb - (int)RADIO_ORBITA_CM;   // + = estoy MAS LEJOS de lo que quiero
      if (abs(error) > BANDA_XP) {
        // estoy lejos -> el circulo tiene que ser MAS CERRADO -> menos duty
        ajusteRadio -= KI_RADIO * (float)error;
        if (ajusteRadio >  AJUSTE_MAX) ajusteRadio =  AJUSTE_MAX;
        if (ajusteRadio < -AJUSTE_MAX) ajusteRadio = -AJUSTE_MAX;
      }
    }
    // si NO la ve: el ajuste se congela. No se corrige a ciegas.

    recalcularVentanaOrbita();
  }

  // ---------- durante la ventana: se EJECUTA lo decidido ----------
  traseraOrbita(sentidoA, PWM_ORB);                       // nunca se apaga
  if (fase < (unsigned long)msFrenteOrb) frenteOrbita(sentidoA, PWM_ORB);
  else                                   frenteSuelto();
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
  // Al SALIR de la orbita se guarda lo que aprendio el lazo, antes de pisar
  // el estado. Si vuelve a orbitar enseguida, retoma en vez de empezar de cero.
  if (estado == ORBITANDO && nuevo != ORBITANDO) {
    ajusteGuardado = ajusteRadio;
    t_salioOrbita  = millis();
  }
  estado = nuevo;
  t_entroEstado = millis();
  t_cicloPulso  = millis();
  if (nuevo == ORBITANDO) reiniciarOrbita();
}


void setup() {
  pinMode(IZQ_INA, OUTPUT); pinMode(IZQ_INB, OUTPUT); pinMode(IZQ_PWM, OUTPUT);
  pinMode(DER_INA, OUTPUT); pinMode(DER_INB, OUTPUT); pinMode(DER_PWM, OUTPUT);
  pinMode(TRA_INA, OUTPUT); pinMode(TRA_INB, OUTPUT); pinMode(TRA_PWM, OUTPUT);
  parar();

  Serial.begin(19200);
  Serial1.begin(19200);

  while (!Serial && millis() < 3000) { }

  dutyNominal = calcularDutyNominal();

  Serial.println();
  Serial.println("==============================================");
  Serial.println("BUSCAR - CENTRAR - AVANZAR - ORBITAR - PATEAR");
  Serial.print("orbita si Xp<"); Serial.println(XP_ORBITA);
  Serial.print("patea si |Yp - Yarcoazul| <= "); Serial.println(TOL_ALINEADO);
  Serial.println("----------- ORBITA -----------");
  Serial.print("radio objetivo = "); Serial.print(RADIO_ORBITA_CM);
  Serial.println(" cm (unidades de camara)");
  Serial.print("L chasis (SIN MEDIR) = "); Serial.print(L_CHASIS_CM); Serial.println(" cm");
  Serial.print("circulo mas chico posible = 2*L = ");
  Serial.print(2.0 * L_CHASIS_CM); Serial.println(" cm");
  Serial.print("duty de las de adelante = "); Serial.print(dutyNominal, 3);
  Serial.print("   -> trasera:adelante = ");
  if (dutyNominal > 0.001) Serial.println(1.0 / dutyNominal, 1);
  else                     Serial.println("adelante APAGADAS");
  if (RADIO_ORBITA_CM < 2.0 * L_CHASIS_CM) {
    Serial.println("!!! OJO: el radio pedido es MAS CHICO que 2*L.");
    Serial.println("!!! Este robot no puede hacer ese circulo. Va a hacer el mas chico que puede.");
  }
  Serial.print("PWM de las tres = "); Serial.print(PWM_ORB);
  Serial.println("   (esta es la perilla de VELOCIDAD, no cambia el radio)");
  Serial.print("lazo de camara: "); Serial.println(ORBITA_LAZO_CERRADO ? "ENCENDIDO" : "apagado");
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

    // "Se me alejo" tiene que ser SOSTENIDO, no una lectura suelta.
    // Ver el comentario de MS_CONFIRMA_SUELTA: con la pelota quieta la camara
    // reporto 146 cm. Con la version anterior ese unico cuadro sacaba al robot
    // de la orbita y lo mandaba derecho contra la pelota.
    if (XpBueno <= XP_SUELTA) t_cercaOrb = millis();
    bool lejosSostenido = (millis() - t_cercaOrb) > MS_CONFIRMA_SUELTA;

    if (!laVeo) {                                   // se le escapo la pelota
      Serial.println("... perdi la pelota orbitando");
      cambiarA(BUSCANDO);
    }
    else if (lejosSostenido) {                      // se le alejo DE VERDAD
      Serial.print("... se me alejo (Xp="); Serial.print(XpBueno);
      Serial.println(") sostenido -> vuelvo a AVANZAR");
      cambiarA(AVANZANDO);
    }
    else if (veoArco && abs(YpBueno - YazBueno) <= TOL_ALINEADO) {
      Serial.print("*** ALINEADO con el arco azul  (Yp="); Serial.print(YpBueno);
      Serial.print("  Yarco="); Serial.print(YazBueno);
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (enEstado > MS_ORBITA_MAX) {            // se rinde
      Serial.println("... se acabo el tiempo de orbita y no encontre el arco azul alineado");
      cambiarA(BUSCANDO);
    }
    else {
      orbitarRegulado(!ORBITA_INVERTIDA, enEstado, laVeo);

      // ---- telemetria para SINTONIZAR (esto es lo que hay que mirar) ----
      if (millis() - t_avisoOrb > 400) {
        t_avisoOrb = millis();
        Serial.print("[ORB] radio ");
        if (hayMedidaOrb) { Serial.print("medido="); Serial.print(XpOrb); }
        else              { Serial.print("(sin medida aun)"); }
        Serial.print(" objetivo="); Serial.print((int)RADIO_ORBITA_CM);
        Serial.print(" | err="); Serial.print(XpOrb - (int)RADIO_ORBITA_CM);
        Serial.print(" | duty="); Serial.print(dutyEfectivo, 3);
        Serial.print(" (nom "); Serial.print(dutyNominal, 3);
        Serial.print(" ajuste "); Serial.print(ajusteRadio, 3); Serial.print(")");
        Serial.print(" | adelante "); Serial.print(msFrenteOrb);
        Serial.print("/"); Serial.print(msVentanaOrb); Serial.print("ms");
        Serial.print(" | Yp="); Serial.print(YpBueno);

        // El robot estima su PROPIO radio de chasis: si el lazo llego al
        // objetivo, el duty que quedo dice cuanto vale L de verdad.
        // PERO el numero solo significa algo si TODO esto es cierto a la vez:
        //   - hay una medida real de la camara (no el valor de arranque)
        //   - el lazo esta cerrado (si no, el duty no persigue nada y el
        //     numero es pura aritmetica sobre Xp: no mide el chasis)
        //   - el duty no esta pegado a un tope ni la relacion quedo rota
        //     (si esta pegado, la cuenta describe un duty que NO se ejecuta)
        // Antes se imprimia SIEMPRE, incluso con duty=0: ahi daba Xp/2, un
        // numero que parece una medicion del chasis y no lo es.
        bool dutyPegado = (ajusteRadio >=  AJUSTE_MAX - 0.0005f) ||
                          (ajusteRadio <= -AJUSTE_MAX + 0.0005f);
        if (hayMedidaOrb && ORBITA_LAZO_CERRADO && !relacionRota &&
            !dutyPegado && dutyEfectivo > 0.001f) {
          float Lest = (float)XpOrb * (1.0f - 2.0f * dutyEfectivo) / (2.0f * (1.0f + dutyEfectivo));
          Serial.print(" | L_estimado="); Serial.print(Lest, 1);
        } else {
          Serial.print(" | L_estimado=-- (todavia no significa nada)");
        }
        if (dutyPegado)   Serial.print("  !! AJUSTE PEGADO AL TOPE: L esta muy mal medido");
        if (relacionRota) Serial.print("  !! duty demasiado chico: la RELACION no se sostiene, el circulo va a salir MAS CHICO");
        Serial.println();
      }
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

  if (estado != ORBITANDO && millis() - t_ultimoAviso > 2000) {
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
