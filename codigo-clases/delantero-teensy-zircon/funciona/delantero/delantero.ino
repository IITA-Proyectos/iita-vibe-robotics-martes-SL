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
   NO alcanza con "ver el arco": hay que verlo DETRAS de la pelota. O sea:
   robot -> pelota -> arco, los tres en la misma linea. Ahi empujar la
   pelota derecho la manda al arco.

   Hasta el 2026-08-11 eso se decidia restando CENTIMETROS
   (abs(Yp - Yarco) <= 12) y estaba mal, porque los centimetros de la
   pelota se miden a 17 cm y los del arco a 100. Ahora se comparan
   ANGULOS, como hacia el campeon 2025 (delantero.ino:311-313). El detalle
   completo, con el numero que muestra el error, esta en el bloque
   "A. ALINEACION POR ANGULO" mas abajo.

   ---------------------------------------------------------------------
   El arco AMARILLO es el byte 202 del paquete de la camara y el AZUL el 203.
   A cual se le apunta lo decide objetivoEsAmarillo (ver bloque B).
   Correr en el PISO, con espacio. Monitor serie a 19200.
   ===================================================================== */

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


// ================= PERILLAS =================

const bool GIRO_INVERTIDO   = true;   // sentido del giro de busqueda/centrado
const bool ORBITA_INVERTIDA = true;  // <<< para que orbite al otro lado

// --- histeresis del centrado ---
const int TOL_ENTRA = 10;
const int TOL_SALE  = 5;

// --- distancias ---
const int XP_ORBITA = 22;   // mas cerca que esto -> empieza a orbitar
const int XP_SUELTA = 55;   // si se le aleja mas que esto, vuelve a avanzar
const int XP_MAX    = 150;  // arriba de esto no le creo A LA PELOTA (la camara recorta en 200)

// EL ARCO NO LLEVA EL MISMO TECHO QUE LA PELOTA. [2026-08-11]
// Hasta hoy se le aplicaba XP_MAX = 150 tambien al arco, y estaba mal: el arco
// casi siempre esta LEJOS. Un arco a 170 cm se tiraba a la basura, y por eso el
// robot decia "no veo el arco" mientras la camara lo estaba viendo perfecto.
// Sintomas que explicaba: angArco = -- siempre, y "0 muestras" de los dos arcos
// al arrancar.
//
// El campeon 2025 no tenia techo para el arco (delantero.ino:335-345): le
// alcanzaba con  Xam != 0.  Le copiamos el criterio de la pelota al arco sin
// preguntarnos si tenia sentido. Una pelota lejos es sospechosa (es chiquita y
// se confunde con cualquier mancha naranja); un arco lejos es simplemente un
// arco lejos: es un objeto grande y la camara le pide 300-600 pixeles.
//
// Ademas, para ALINEAR no nos importa a que distancia esta el arco: nos importa
// en que DIRECCION. El angulo sirve igual este a 80 o a 200 cm.
const int XARCO_MAX = 200;  // 200 es el tope que manda la camara: acepto todo

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


// =====================================================================
//  LO NUEVO (2026-08-11) — tres cosas, cada una con SU interruptor
// =====================================================================
//
//  Se agregaron juntas pero NO se prueban juntas. Si se encienden las tres
//  y el robot empeora, no se sabe cual fue. El orden para probarlas esta
//  abajo de todo, en el comentario "COMO PROBAR ESTO".
//
//  ---------------------------------------------------------------------
//  A. ALINEACION POR ANGULO — activa
//  ---------------------------------------------------------------------
//  ANTES pateabamos con  abs(Yp - Yarco) <= 12 cm.  Eso esta MAL, y no por
//  poco: Yp se mide a la distancia de la PELOTA (~17-22 cm) y Yarco a la
//  distancia del ARCO (~100 cm o mas). Son centimetros medidos a distintas
//  distancias — restarlos es como restar "3 pasos mios" menos "3 pasos de
//  un gigante".
//
//  El numero: una pelota con Yp = 12 cm a 17 cm de distancia esta a 35
//  grados. Un arco con Yarco = 12 cm a 100 cm esta a 6,8 grados. La cuenta
//  vieja da |12-12| = 0 y canta "alineado" cuando en realidad estan a 28
//  grados uno del otro. Por eso pateaba desviado.
//
//  La solucion es del propio delantero campeon 2025 (delantero.ino:311-313),
//  que ya calculaba los angulos y nosotros no habiamos copiado:
//
//        angulo = atan2(Y, X) * 180 / PI
//
//  Un angulo NO depende de la distancia: 35 grados son 35 grados este la
//  pelota cerca o lejos. Ahora se comparan angulos con angulos.
//
//  Y se pide una condicion mas, que el 2025 tambien tenia
//  (tolerancia_apuntado): la pelota tiene que estar ADELANTE. Si la pelota
//  esta a 40 grados y el arco tambien, la resta da 0 — pero el robot al
//  avanzar derecho ni la toca.
const float TOL_ANG_PELOTA   = 10.0;  // la pelota tiene que estar a menos de esto del frente
const float TOL_ANG_ALINEADO = 10.0;  // y el arco a menos de esto de la pelota
//         ^ el 2025 usaba 15 grados (tolerancia_apuntado). Arrancamos en 15 y
//           el 2026-08-11 Gustavo lo probo en el piso: "es mucho y patea muy
//           mal de direccion". Bajados los dos a 8. El 2026-08-18 subidos a
//           10 a pedido de Gustavo. OJO: 10 NO alcanza para patear orbitando —
//           en el log del 11/08 la pelota estaba a 22-34 grados durante la
//           orbita. Sirve para la patada de frente, no para la de la orbita.
//
//           POR QUE LOS DOS Y NO SOLO EL DEL ARCO. El robot empuja DERECHO.
//           Si la pelota esta 14 grados al costado, la toca de refilon y sale
//           para cualquier lado — o sea que la direccion tambien la arruina
//           TOL_ANG_PELOTA, no solo la del arco. Por eso bajan juntos.
//
//           A 100 cm del arco, 8 grados son ~14 cm de desvio. La escalera si
//           ahora NUNCA patea: 8 -> 10 -> 12. Si sigue pateando torcido: 6.
//           OJO: antes la perilla era TOL_ALINEADO y estaba en CENTIMETROS.
//           Estas son GRADOS. No son la misma cosa.

//  ---------------------------------------------------------------------
//  B. A QUE ARCO ATACAR, decidido AL ENCENDER — arranca APAGADA
//  ---------------------------------------------------------------------
//  En un partido no siempre atacamos el azul: se cambia de lado. Hasta hoy
//  el arco estaba clavado en el codigo.
//
//  EL RITUAL: se apoya el robot MIRANDO AL ARCO RIVAL y se lo enciende.
//  Durante los primeros MS_MIRAR_ARCOS el robot mira sin moverse y se queda
//  con el arco que ve MAS CENTRADO. Ese pasa a ser su objetivo.
//
//  El mismo gesto sirve para dos cosas: define el arco Y define el "cero"
//  del giroscopio (C). Un solo ritual, dos funciones.
const bool ELEGIR_ARCO_AL_ENCENDER = true;   // 2026-08-11: ENCENDIDA a pedido de Gustavo
const unsigned long MS_MIRAR_ARCOS = 2000;
const int MUESTRAS_MINIMAS_ARCO    = 3;   // menos que esto, no le creo

//  ---------------------------------------------------------------------
//  C. GIROSCOPO (BNO055 en I2C 0x28) — arranca APAGADA
//  ---------------------------------------------------------------------
//  ⚠️ EL FIRMWARE NUNCA LO USO. El codigo 2025 y el arquero de hoy si, pero
//  en ESTA placa no esta comprobado que el sensor conteste. Por eso arranca
//  apagado y por eso, si falla, el robot sigue andando como hasta ayer.
//
//  Dos usos:
//   C1. Si da la vuelta entera y no encuentra el arco, en vez de rendirse
//       gira hasta mirar al rumbo de arranque (el "cero") y patea ahi.
//   C2. Elegir para que lado orbitar: el camino MAS CORTO hasta el arco.
const bool USAR_GIROSCOPO   = true;    // 2026-08-11: ENCENDIDA a pedido de Gustavo
const bool PATEAR_AL_RUMBO0 = true;    // C1 (solo hace algo si USAR_GIROSCOPO)
const bool ORBITA_CAMINO_CORTO = false;// C2 — apagada para que el primer flasheo
                                       // cambie UNA sola cosa (la patada por angulo).
                                       // Anda con el arco a la vista aunque no haya
                                       // giroscopo; con giroscopo anda tambien a ciegas.
const float TOL_RUMBO = 12.0;          // grados: "ya estoy mirando al cero"
const unsigned long MS_APUNTAR_MAX = 6000;

//  ---------------------------------------------------------------------
//  PERILLAS DE SIGNO — [SIN VERIFICAR EN BANCO]
//  ---------------------------------------------------------------------
//  No sabemos de que lado del robot es "Y positivo", ni que sentido de giro
//  produce sentidoA = true. El equipo ya lo venia tapando por prueba y error
//  con GIRO_INVERTIDO. Estas dos son lo mismo para lo nuevo: si el robot
//  gira PARA EL LADO CONTRARIO al que deberia, se da vuelta la que
//  corresponda. Son un booleano, no una cuenta: se prueban en 2 minutos.
const bool SENTIDO_ORBITA_INVERTIDO = false;  // si orbita alejandose del arco, ponelo en true
const bool GIRO_RUMBO_INVERTIDO     = false;  // si al apuntar al cero se aleja, ponelo en true

//  ---------------------------------------------------------------------
//  D. LINEA BLANCA — escapar. PRIORIDAD ABSOLUTA
//  ---------------------------------------------------------------------
//  Si un sensor de linea pisa el blanco, se ANULA lo que sea que este
//  haciendo el robot —incluida la patada— y se escapa. Salir de la cancha
//  es lo peor que puede pasar: es lo unico que interrumpe a todo lo demas.
//
//  HACIA DONDE SE ESCAPA. Cada sensor vigila UN LADO del triangulo, y
//  enfrente de cada sensor hay una rueda en el vertice opuesto:
//
//        sensor 1  -> se va hacia la rueda DI  (delantera izquierda, M1)
//        sensor 2  -> se va hacia la rueda DD  (delantera derecha,   M2)
//        sensor 3  -> se va hacia la rueda T   (trasera,             M3)
//
//  LA CUENTA SALE GRATIS, igual que la orbita. Para moverse en la direccion
//  de una rueda, ESA rueda no tiene que girar (su empuje es perpendicular a
//  ese movimiento) y las otras dos empujan iguales y al reves entre si:
//
//        sensor 1 -> IZQ apagada,  DER y TRA opuestas
//        sensor 2 -> DER apagada,  IZQ y TRA opuestas
//        sensor 3 -> TRA apagada,  IZQ y DER opuestas
//
//  No lo invente: es exactamente retroceder1/2/3 del campeon 2025
//  (delantero.ino:164-178), que usaba PWM 100 durante 400 ms.
//
//  LAS ESQUINAS. Si saltan DOS sensores a la vez, las dos direcciones se
//  SUMAN. Y como las tres direcciones suman cero, sensor1+sensor2 da
//  exactamente lo contrario de sensor3: alejarse de la rueda T. Sale solo,
//  sin medir ningun angulo.
const bool LINEA_ACTIVA = true;
const int  VEL_ESCAPE   = 100;              // el del campeon 2025
const unsigned long MS_ESCAPE_EXTRA = 400;  // sigue 400 ms DESPUES de dejar de verla
const unsigned long MS_PARA_ARMAR   = 500;  // verde de corrido antes de armar el escape

//  EL GOLPE DE FRENO [2026-08-18]
//
//  POR QUE. Si el robot cruza la linea con mas de medio cuerpo es GOL EN
//  CONTRA (regla que trajo Gustavo del reglamento nuevo). Pateando va a
//  VEL_PATADA = 240, y el escape normal empuja a 100: le opone menos de la
//  mitad de lo que traia. Peor: parar() SUELTA las ruedas, no las traba —
//  pone las seis patas de direccion en 0, que es rueda libre. En todo el
//  firmware no habia ni un freno activo.
//
//  COMO SE FRENA SIN FRENO. Se empuja al reves. Cuando el sensor de ADELANTE
//  ve la linea, la direccion de escape ya apunta hacia la rueda TRASERA — o
//  sea, para atras — asi que el escape SI se opone al avance. Lo que faltaba
//  era fuerza: para matar un envion hecho con 240 hay que oponer 240, no 100.
//
//  ES LA MISMA FORMA QUE EL IMPULSO DE LA ORBITA, AL REVES: fuerte primero,
//  normal despues.
//
//  SOLO SALIENDO DE LA PATADA. Avanzar va a 55 y la orbita a 48; si les
//  metiera 240 de freno, el robot saldria disparado contra la linea de
//  enfrente. La patada es el unico estado rapido.
const int VEL_FRENO = 240;                 // igual que la patada: lo que trajo, se le opone
const unsigned long MS_FRENO = 150;        // cuanto dura el golpe de freno

//  UMBRALES DEL 2025, luz del laboratorio del anio pasado. Ya nos paso con
//  los umbrales de color: se re-miden con pruebas/sensores-de-linea/ antes
//  de confiar. Mientras tanto hay una AUTOPROTECCION al arrancar: si un
//  sensor ya lee "blanco" con el robot apoyado en el verde, el umbral esta
//  mal y la funcion se desactiva sola en vez de escapar para siempre.
//  ACTUALIZADO 2026-08-11 con la medicion en cancha de la mesa del ARQUERO
//  (su bitacora 2026-08-11-sensores-de-linea-y-arquero-completo.md):
//        verde  350 a 468     blanco  ~760     umbral = punto medio = 620
//  Los 650/650/750 del 2025 quedaban demasiado altos: 750 esta pegado al
//  blanco real (760), asi que el sensor 3 casi no disparaba nunca.
//  [FALTA CONFIRMAR EN EL DELANTERO] — es otro robot; la autoproteccion del
//  arranque avisa si estos numeros no sirven para esta placa.
int UMBRAL_LINEA[3] = { 620, 620, 620 };

//  Pines: se autodetectan leyendo el pin 32, igual que zirconLib.cpp:52-60.
const int PIN_VERSION_PLACA = 32;

//  ---------------------------------------------------------------------
//  COMO PROBAR ESTO — de a una, en este orden
//  ---------------------------------------------------------------------
//  Se agregaron tres cosas juntas pero se encienden de a una. Si se prenden
//  todas y el robot empeora, no se sabe cual fue.
//
//  PASO 1 — la patada por angulo (ya esta activa, no hay que tocar nada).
//     Que mirar: el monitor imprime ahora "angPelota", "angArco" y
//     "separacion", los tres en grados. Ponele el arco atras de la pelota a
//     ojo y fijate si "separacion" baja de 15 justo cuando VOS dirias que
//     estan alineados. Si patea siempre, bajar TOL_ANG_ALINEADO; si no patea
//     nunca, subirlo. ANOTAR los grados a los que pateo.
//
//  PASO 2 — el camino corto de la orbita: poner ORBITA_CAMINO_CORTO = true.
//     Que mirar: poner el arco claramente de UN lado y soltar el robot cerca
//     de la pelota. Tiene que arrancar a orbitar HACIA el arco, no al reves.
//     Si arranca para el lado contrario: SENTIDO_ORBITA_INVERTIDO = true.
//     Ese es el unico ajuste; es un booleano, se prueba en dos intentos.
//
//  PASO 3 — el arco al encender: poner ELEGIR_ARCO_AL_ENCENDER = true.
//     Que mirar: apoyar el robot mirando al arco AMARILLO y encenderlo. El
//     monitor tiene que decir "ATACO EL ARCO AMARILLO". Repetir mirando al
//     azul. Si se equivoca, mirar cuantas muestras vio de cada uno: si son
//     pocas, el problema es la camara (umbrales), no esta logica.
//
//  PASO 4 — el giroscopo: poner USAR_GIROSCOPO = true.
//     ⚠️ Lo primero NO es probar la patada al rumbo 0: es ver si el sensor
//     contesta. El monitor dice "Giroscopo: OK" o "NO CONTESTA" al arrancar.
//     Si contesta, girar el robot a mano y ver que "rumbo=" cambie y vuelva.
//     Recien despues probar el plan B (dejarlo orbitar sin arco a la vista y
//     ver si apunta al rumbo de arranque). Si gira para el lado contrario:
//     GIRO_RUMBO_INVERTIDO = true.

// ============================================

int Xp = 0, Yp = 0;
int Xam = 0, Yam = 0;              // arco AMARILLO (byte 202)
int Xaz = 0, Yaz = 0;              // arco AZUL     (byte 203)

int XpBueno = 0, YpBueno = 0;      // ultima posicion BUENA de la pelota
int XamBueno = 0, YamBueno = 0;
int XazBueno = 0, YazBueno = 0;

unsigned long t_ultimaPelota   = 0;
unsigned long t_ultimoAmarillo = 0;
unsigned long t_ultimoAzul     = 0;
unsigned long t_ultimoPaquete  = 0;
unsigned long t_ultimoAviso    = 0;
unsigned long t_cicloPulso     = 0;
unsigned long t_entroEstado    = 0;

// A que arco le apuntamos. Si ELEGIR_ARCO_AL_ENCENDER esta apagado, se queda
// con este valor — que es lo que veniamos haciendo.
bool objetivoEsAmarillo = false;

// --- giroscopo ---
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool  hayGiroscopo = false;
float rumboCero    = 0;    // hacia donde miraba al encenderse = hacia el arco rival
float ultimoRumbo  = 0;
int   contadorCeros = 0;
const int CEROS_PARA_DARLO_POR_CAIDO = 10;

// --- sensores de linea ---
int  pinLinea[3]  = { A11, A13, A12 };   // Mark1; se corrige en setup()
const char* versionPlaca = "?";
bool lineaArmada  = false;   // se arma sola la primera vez que ve VERDE
unsigned long t_verdeDesde = 0;
bool frenoFuerte = false;    // entramos al escape viniendo de la patada?
int  mascaraLinea = 0;
unsigned long t_ultimaLinea = 0;

enum Estado { BUSCANDO, CENTRANDO, AVANZANDO, ORBITANDO,
              APUNTA_RUMBO0, PATEA_ADEL, PATEA_ATRAS, ESCAPA_LINEA };
Estado estado = BUSCANDO;
Estado estadoAnterior = PATEA_ATRAS;

bool avisadoSinCamara = false;

// --- medicion del enlace con la camara [2026-08-11] ---
// Gustavo planteo que podiamos estar leyendo lento y quedandonos con datos
// viejos. En vez de discutirlo, se mide. Se imprime cada 2 s:
//   paq/s     paquetes de 9 bytes VALIDOS por segundo = cuadros de camara
//   tirados/s bytes descartados buscando el 201. Si esto es alto, hay
//             desincronizacion, que es justo el sintoma de buffer desbordado.
//   loops/s   vueltas del loop(). Si son miles, no estamos leyendo lento.
unsigned long nPaquetes = 0, nTirados = 0, nLoops = 0;
unsigned long t_contadores = 0;


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

// ---------- linea blanca ----------

// Devuelve una mascara: bit 0 = sensor 1, bit 1 = sensor 2, bit 2 = sensor 3.
int leerLineas() {
  int m = 0;
  for (int i = 0; i < 3; i++) {
    if (analogRead(pinLinea[i]) >= UMBRAL_LINEA[i]) m |= (1 << i);
  }
  return m;
}

// Escapa de la(s) linea(s) que se estan viendo. Suma las direcciones, asi
// que las esquinas (dos sensores a la vez) salen solas.
void escaparDeLinea(int m, int velocidad) {
  //          IZQ(M1) DER(M2) TRA(M3)
  int v[3] = {   0,      0,      0   };
  if (m & 1) { v[1] -= 1; v[2] += 1; }   // hacia la DI  -> IZQ apagada
  if (m & 2) { v[0] += 1; v[2] -= 1; }   // hacia la DD  -> DER apagada
  if (m & 4) { v[0] -= 1; v[1] += 1; }   // hacia la T   -> TRA apagada

  int pico = 0;
  for (int i = 0; i < 3; i++) if (abs(v[i]) > pico) pico = abs(v[i]);

  if (pico == 0) {
    // Los tres sensores a la vez: las tres direcciones se cancelan y no hay
    // para donde ir. Casi seguro son los umbrales mal puestos. Parar es lo
    // honesto: salir para un lado elegido al azar seria inventar.
    parar();
    return;
  }

  int pwm[3];
  for (int i = 0; i < 3; i++) pwm[i] = (v[i] * velocidad) / pico;

  analogWrite(IZQ_PWM, abs(pwm[0]));
  digitalWrite(IZQ_INA, pwm[0] > 0 ? 1 : 0);
  digitalWrite(IZQ_INB, pwm[0] < 0 ? 1 : 0);

  analogWrite(DER_PWM, abs(pwm[1]));
  digitalWrite(DER_INA, pwm[1] > 0 ? 1 : 0);
  digitalWrite(DER_INB, pwm[1] < 0 ? 1 : 0);

  analogWrite(TRA_PWM, abs(pwm[2]));
  digitalWrite(TRA_INA, pwm[2] > 0 ? 1 : 0);
  digitalWrite(TRA_INB, pwm[2] < 0 ? 1 : 0);
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
    if (h1 != 201) { nTirados++; continue; }

    int xp  = Serial1.read();
    int yp  = Serial1.read();
    int h2  = Serial1.read();
    int xam = Serial1.read();
    int yam = Serial1.read();
    int h3  = Serial1.read();
    int xaz = Serial1.read();
    int yaz = Serial1.read();

    if (h2 == 202 && h3 == 203) {
      Xp  = xp;   Yp  = yp  - 100;
      Xam = xam;  Yam = yam - 100;    // antes se tiraba: ahora hace falta para elegir arco
      Xaz = xaz;  Yaz = yaz - 100;
      t_ultimoPaquete = millis();
      nPaquetes++;

      // 200 y +-100 son los TOPES de recorte de la camara: casi siempre manchas.
      if ((Xp > 0) && (Xp <= XP_MAX) && (abs(Yp) < 100)) {
        XpBueno = Xp; YpBueno = Yp;
        t_ultimaPelota = millis();
      }
      if ((Xam > 0) && (Xam <= XARCO_MAX) && (abs(Yam) < 100)) {
        XamBueno = Xam; YamBueno = Yam;
        t_ultimoAmarillo = millis();
      }
      if ((Xaz > 0) && (Xaz <= XARCO_MAX) && (abs(Yaz) < 100)) {
        XazBueno = Xaz; YazBueno = Yaz;
        t_ultimoAzul = millis();
      }
    }
  }
}


// ---------- angulos ----------
//
// LA IDEA, del delantero campeon 2025 (delantero.ino:311-313): pasar de
// "cuantos centimetros esta corrido" a "en que direccion esta". El angulo no
// depende de la distancia, y por eso SI se pueden comparar la pelota (cerca)
// con el arco (lejos).
//
//   atan2(Y, X) da el angulo del punto (X,Y) visto desde el robot.
//   X = para adelante, Y = para el costado. Angulo 0 = justo adelante.

float anguloDe(int X, int Y) {
  if (X <= 0) return 0.0;                    // sin dato, no invento un angulo
  return atan2((float)Y, (float)X) * 180.0 / PI;
}

// Diferencia mas corta entre dos angulos, en (-180, 180]. Sin esto, ir de 350
// a 10 grados se leeria como un giro de -340 en vez de +20.
// Copiada del cuadrado-giroscopo del arquero, que ya la tiene andando.
float diferencia(float objetivo, float actual) {
  float d = objetivo - actual;
  while (d > 180.0)   d -= 360.0;
  while (d <= -180.0) d += 360.0;
  return d;
}

// ---------- el arco al que le apuntamos ----------

int   arcoX()       { return objetivoEsAmarillo ? XamBueno : XazBueno; }
int   arcoY()       { return objetivoEsAmarillo ? YamBueno : YazBueno; }
unsigned long arcoT() { return objetivoEsAmarillo ? t_ultimoAmarillo : t_ultimoAzul; }
const char* arcoNombre() { return objetivoEsAmarillo ? "AMARILLO" : "AZUL"; }


// ---------- giroscopo ----------
//
// La deteccion de "sensor caido" es prestada del arquero, que la pago caro:
// el 2026-07-28 su BNO055 empezo a devolver 0.000 exacto en los tres angulos
// y el programa siguio girando con datos basura sin enterarse. Si el chip no
// contesta, la libreria devuelve ceros — y un cero es una postura posible, asi
// que con UNA lectura no se distingue. Por eso se cuentan varias seguidas.

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
  return ultimoRumbo;                        // 0..360
}

bool giroscopoSano() {
  return hayGiroscopo && contadorCeros < CEROS_PARA_DARLO_POR_CAIDO;
}


const char* nombreEstado(Estado e) {
  switch (e) {
    case BUSCANDO:    return "BUSCANDO";
    case CENTRANDO:   return "CENTRANDO";
    case AVANZANDO:   return "AVANZANDO";
    case ORBITANDO:   return "ORBITANDO";
    case APUNTA_RUMBO0: return "al rumbo 0";
    case PATEA_ADEL:  return "PATEANDO!";
    case PATEA_ATRAS: return "retrocede";
    case ESCAPA_LINEA: return "!LINEA!";
  }
  return "?";
}


// ---------- arranque del giroscopo y eleccion del arco ----------

// Enciende el BNO055. Devuelve false si no contesta o si contesta puros
// ceros. NO se cuelga el programa si falla: se sigue sin giroscopo.
// ERROR PROPIO, CORREGIDO EL 2026-08-18.
//
// La version anterior tomaba las 20 lecturas de prueba INMEDIATAMENTE despues
// de setExtCrystalUse(). Y esa llamada pasa el BNO a modo configuracion,
// cambia el reloj y vuelve: el algoritmo de fusion arranca DE CERO y devuelve
// 0.000 legitimos mientras converge. O sea que yo le tomaba el pulso al sensor
// mientras despertaba, contaba esos ceros como fallas, y lo declaraba muerto.
//
// El numero lo gritaba y no lo escuche: daba SIEMPRE "9 de 20 utiles". Con
// lecturas cada 50 ms son 11 ceros al principio y 9 buenas al final — el
// sensor despertando a la mitad de mi propia prueba.
//
// El arquero, que lo tiene andando, hace begin() + delay(1000) +
// setExtCrystalUse() y NO VERIFICA NADA. Si corriera mi prueba, le daria igual
// de mal.
//
// Ahora: se espera DESPUES del cambio de reloj, y se juzga por las ULTIMAS
// lecturas, no por el total — porque lo que importa es como termina, no como
// empieza. Y se imprime la secuencia entera para poder VERLO.
bool arrancarGiroscopo() {
  if (!bno.begin()) {
    Serial.println("no contesta en el bus I2C (0x28)");
    return false;
  }
  delay(1000);                     // igual que el arquero
  bno.setExtCrystalUse(true);
  delay(1000);                     // <<< LO QUE FALTABA: dejarlo re-arrancar

  int buenas = 0, seguidasAlFinal = 0;
  Serial.println();
  Serial.print("   secuencia (. = dato, 0 = cero): ");
  for (int i = 0; i < 20; i++) {
    sensors_event_t e;
    bno.getEvent(&e);
    bool ok = (e.orientation.x != 0.0 || e.orientation.y != 0.0 || e.orientation.z != 0.0);
    Serial.print(ok ? '.' : '0');
    if (ok) { buenas++; seguidasAlFinal++; } else { seguidasAlFinal = 0; }
    delay(50);
  }
  Serial.println();

  uint8_t sys = 0, gy = 0, ac = 0, mg = 0;
  bno.getCalibration(&sys, &gy, &ac, &mg);
  Serial.print("   calibracion  sistema="); Serial.print(sys);
  Serial.print(" giro="); Serial.print(gy);
  Serial.print(" acel="); Serial.print(ac);
  Serial.print(" magn="); Serial.print(mg);
  Serial.println("   (0 = sin calibrar, 3 = calibrado)");

  // El criterio es como TERMINA, no el total: si las ultimas 5 son buenas, el
  // sensor esta entregando datos ahora, que es lo unico que importa.
  if (seguidasAlFinal < 5) {
    Serial.print("   "); Serial.print(buenas);
    Serial.print("/20 utiles y solo "); Serial.print(seguidasAlFinal);
    Serial.println(" seguidas al final — no lo doy por bueno");
    return false;
  }
  Serial.print("   "); Serial.print(buenas);
  Serial.print("/20 utiles, "); Serial.print(seguidasAlFinal);
  Serial.println(" seguidas al final");
  contadorCeros = 0;
  return true;
}

// Mira sin moverse y se queda con el arco MAS CENTRADO. El ritual es apoyar
// el robot mirando al arco rival y recien ahi encenderlo.
void elegirArcoMirando() {
  long   nAm = 0, nAz = 0;
  double sumAm = 0, sumAz = 0;
  unsigned long t0 = millis();

  Serial.print("Mirando "); Serial.print(MS_MIRAR_ARCOS / 1000);
  Serial.println(" s para ver a que arco apunto. NO LO MUEVAS.");

  // Se cuenta UNA VEZ POR CUADRO DE CAMARA, no una vez por vuelta del loop.
  // El loop corre a ~400.000 vueltas por segundo y la camara manda 46 cuadros:
  // contar por vuelta daba millones de "muestras" con un solo vistazo fugaz, y
  // MUESTRAS_MINIMAS_ARCO dejaba de filtrar nada. [medido 2026-08-11]
  unsigned long visto_am = 0, visto_az = 0;
  while (millis() - t0 < MS_MIRAR_ARCOS) {
    leerCamara();
    if (t_ultimoAmarillo != visto_am) {          // llego un dato NUEVO del amarillo
      visto_am = t_ultimoAmarillo;
      nAm++;  sumAm += fabs(anguloDe(XamBueno, YamBueno));
    }
    if (t_ultimoAzul != visto_az) {
      visto_az = t_ultimoAzul;
      nAz++;  sumAz += fabs(anguloDe(XazBueno, YazBueno));
    }
  }

  float medAm = nAm ? (float)(sumAm / nAm) : 999.0;
  float medAz = nAz ? (float)(sumAz / nAz) : 999.0;

  Serial.print("   amarillo: "); Serial.print(nAm); Serial.print(" muestras");
  if (nAm) { Serial.print(", a "); Serial.print(medAm, 1); Serial.print(" grados"); }
  Serial.println();
  Serial.print("   azul:     "); Serial.print(nAz); Serial.print(" muestras");
  if (nAz) { Serial.print(", a "); Serial.print(medAz, 1); Serial.print(" grados"); }
  Serial.println();

  bool sirveAm = (nAm >= MUESTRAS_MINIMAS_ARCO);
  bool sirveAz = (nAz >= MUESTRAS_MINIMAS_ARCO);

  if (!sirveAm && !sirveAz) {
    Serial.print("   NO VI NINGUN ARCO -> me quedo con el de siempre: ");
    Serial.println(arcoNombre());
    return;
  }
  if (sirveAm && !sirveAz)      objetivoEsAmarillo = true;
  else if (sirveAz && !sirveAm) objetivoEsAmarillo = false;
  else                          objetivoEsAmarillo = (medAm < medAz);

  Serial.print("   *** ATACO EL ARCO "); Serial.print(arcoNombre()); Serial.println(" ***");
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
  Serial.print("patea si pelota a menos de "); Serial.print(TOL_ANG_PELOTA, 0);
  Serial.print(" grados del frente Y el arco a menos de "); Serial.print(TOL_ANG_ALINEADO, 0);
  Serial.println(" grados de la pelota");
  Serial.print("orbita: impulso "); Serial.print(VEL_ORB_IMPULSO);
  Serial.print(" x "); Serial.print(MS_ORB_IMPULSO);
  Serial.print(" ms  ->  crucero "); Serial.print(VEL_ORB_TRASERA);
  Serial.print("   (max "); Serial.print(MS_ORBITA_MAX / 1000); Serial.println(" s)");
  Serial.println("==============================================");

  // --- sensores de linea (D) ---
  pinMode(PIN_VERSION_PLACA, INPUT_PULLDOWN);
  delay(10);
  if (digitalRead(PIN_VERSION_PLACA) == LOW) {
    versionPlaca = "Mark1";
    pinLinea[0] = A11; pinLinea[1] = A13; pinLinea[2] = A12;
  } else {
    versionPlaca = "Naveen1";
    pinLinea[0] = A8;  pinLinea[1] = A9;  pinLinea[2] = A12;
  }
  Serial.print("Placa (pin 32): "); Serial.print(versionPlaca);
  Serial.print("   sensores de linea en pines ");
  Serial.print(pinLinea[0]); Serial.print(", ");
  Serial.print(pinLinea[1]); Serial.print(", "); Serial.println(pinLinea[2]);

  if (LINEA_ACTIVA) {
    // ARMADO DIFERIDO [2026-08-18]
    //
    // La version anterior desactivaba el escape PARA SIEMPRE si al encender
    // algun sensor leia blanco. El supuesto era "el robot se enciende sobre el
    // verde". Es falso en la practica: se enciende sobre la MESA, que es clara,
    // y entonces el escape quedaba muerto toda la corrida — el robot ni leia
    // los sensores. Sintoma: "no detecta las lineas blancas".
    //
    // Ahora no se apaga nada. El escape se ARMA SOLO la primera vez que los
    // tres sensores ven verde de corrido. O sea: lo apoyas en la cancha y se
    // arma; lo dejas en la mesa y espera, avisando.
    //
    // Se conserva lo bueno de la proteccion: si un umbral esta de verdad mal,
    // los tres nunca dan verde juntos, nunca se arma, y el robot NO sale
    // corriendo por una linea que no existe.
    Serial.print("Linea: sensores leen ");
    Serial.print(analogRead(pinLinea[0])); Serial.print(" / ");
    Serial.print(analogRead(pinLinea[1])); Serial.print(" / ");
    Serial.print(analogRead(pinLinea[2]));
    Serial.print("   umbrales "); Serial.print(UMBRAL_LINEA[0]);
    Serial.print(" / "); Serial.print(UMBRAL_LINEA[1]);
    Serial.print(" / "); Serial.println(UMBRAL_LINEA[2]);
    Serial.print("Linea: se arma sola cuando vea verde en los tres (");
    Serial.print(MS_PARA_ARMAR); Serial.println(" ms seguidos). Apoyalo en la cancha.");
  } else {
    Serial.println("Linea: apagada por configuracion.");
  }

  // --- giroscopo (C) ---
  if (USAR_GIROSCOPO) {
    Serial.print("Giroscopo: ");
    hayGiroscopo = arrancarGiroscopo();
    if (hayGiroscopo) {
      rumboCero = rumboActual();
      Serial.print("OK. Rumbo cero = "); Serial.print(rumboCero, 1);
      Serial.println(" grados (hacia donde mira AHORA)");
    } else {
      Serial.println("NO CONTESTA. Sigo sin el, como hasta ayer.");
    }
  } else {
    Serial.println("Giroscopo: apagado por configuracion.");
  }

  // --- que arco atacar (B) ---
  if (ELEGIR_ARCO_AL_ENCENDER) {
    elegirArcoMirando();
  } else {
    Serial.print("Arco objetivo: "); Serial.print(arcoNombre());
    Serial.println("  (fijo por configuracion)");
  }

  Serial.println("==============================================");
  Serial.println("Arranca en 3 segundos.");
  delay(3000);

  t_ultimoPaquete = millis();
  t_contadores    = millis();   // si no, la primera medicion sale con dt enorme
  t_verdeDesde    = millis();
  cambiarA(BUSCANDO);
}


// Para que lado orbitar. Prioridad:
//   1. si VEO el arco, para el lado donde esta
//   2. si no, y hay giroscopo, hacia el rumbo de arranque (el "cero")
//   3. si no, el de siempre (ORBITA_INVERTIDA)
// Los dos primeros dependen de una hipotesis de signo SIN VERIFICAR: se
// corrigen con SENTIDO_ORBITA_INVERTIDO, que es un booleano, no una cuenta.
bool sentidoParaOrbitar() {
  bool porDefecto = !ORBITA_INVERTIDA;
  if (!ORBITA_CAMINO_CORTO) return porDefecto;

  bool haciaElPositivo;
  if (millis() - arcoT() < MS_GRACIA) {
    haciaElPositivo = (anguloDe(arcoX(), arcoY()) > 0);
  } else if (giroscopoSano()) {
    haciaElPositivo = (diferencia(rumboCero, rumboActual()) > 0);
  } else {
    return porDefecto;
  }

  if (SENTIDO_ORBITA_INVERTIDO) haciaElPositivo = !haciaElPositivo;
  return haciaElPositivo;
}


void loop() {

  nLoops++;
  leerCamara();

  // ---------- LA LINEA BLANCA MANDA SOBRE TODO ----------
  // Va antes que cualquier otra cosa y anula el estado en curso, incluida la
  // patada. Salir de la cancha es peor que perder una jugada.
  if (LINEA_ACTIVA) {
    int m = leerLineas();

    if (!lineaArmada) {
      // Todavia no se armo: espero a ver verde en los TRES, de corrido.
      if (m != 0) {
        t_verdeDesde = millis();          // vio blanco -> se reinicia la cuenta
      } else if (millis() - t_verdeDesde >= MS_PARA_ARMAR) {
        lineaArmada = true;
        Serial.println("*** LINEA: verde confirmado -> ESCAPE ARMADO");
      }
      m = 0;                              // sin armar no se escapa de nada
    }

    if (m != 0) {
      t_ultimaLinea = millis();
      mascaraLinea  = m;
      if (estado != ESCAPA_LINEA) {
        // Si veniamos pateando, el envion es mucho mas grande: primero freno.
        frenoFuerte = (estado == PATEA_ADEL);
        Serial.print("!!! LINEA BLANCA (sensores");
        for (int i = 0; i < 3; i++) if (m & (1 << i)) { Serial.print(" "); Serial.print(i + 1); }
        Serial.print(") estando en "); Serial.print(nombreEstado(estado));
        if (frenoFuerte) {
          Serial.print(" -> FRENO A FONDO ("); Serial.print(VEL_FRENO);
          Serial.print(" x "); Serial.print(MS_FRENO); Serial.println(" ms) y escapo");
        } else {
          Serial.println(" -> ESCAPO");
        }
        cambiarA(ESCAPA_LINEA);
      }
    }
  }

  bool laVeo    = (millis() - t_ultimaPelota) < MS_GRACIA;
  bool veoArco  = (millis() - arcoT())        < MS_GRACIA;
  unsigned long enEstado = millis() - t_entroEstado;

  // Los angulos, que es con lo que se decide la patada. Ver el bloque
  // "A. ALINEACION POR ANGULO" arriba: comparar centimetros medidos a
  // distancias distintas es lo que hacia que pateara desviado.
  float angPelota = anguloDe(XpBueno, YpBueno);
  float angArco   = anguloDe(arcoX(), arcoY());
  bool  pelotaAdelante = (fabs(angPelota) <= TOL_ANG_PELOTA);
  bool  arcoAlineado   = (fabs(diferencia(angArco, angPelota)) <= TOL_ANG_ALINEADO);

  // ---------- ESCAPA_LINEA: lo primero, no lo interrumpe nadie ----------
  if (estado == ESCAPA_LINEA) {
    if (millis() - t_ultimaLinea > MS_ESCAPE_EXTRA) {
      Serial.println("... ya me despegue de la linea");
      cambiarA(BUSCANDO);
    } else {
      // Los primeros MS_FRENO ms van a fondo SOLO si veniamos pateando.
      bool frenando = (frenoFuerte && enEstado < MS_FRENO);
      escaparDeLinea(mascaraLinea, frenando ? VEL_FRENO : VEL_ESCAPE);
    }
  }

  // ---------- la patada no se interrumpe (salvo por la linea) ----------
  else if (estado == PATEA_ADEL) {
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
    else if (veoArco && pelotaAdelante && arcoAlineado) {
      Serial.print("*** ALINEADO con el arco "); Serial.print(arcoNombre());
      Serial.print("  (pelota a "); Serial.print(angPelota, 1);
      Serial.print(" grados, arco a "); Serial.print(angArco, 1);
      Serial.print(", separados "); Serial.print(fabs(diferencia(angArco, angPelota)), 1);
      Serial.println(")  -> PATADA");
      cambiarA(PATEA_ADEL);
    }
    else if (enEstado > MS_ORBITA_MAX) {            // dio la vuelta y no lo vio
      Serial.print("... orbite "); Serial.print(MS_ORBITA_MAX / 1000);
      Serial.print(" s y no encontre el arco "); Serial.println(arcoNombre());

      // C1: en vez de rendirse, apuntar al rumbo con el que se encendio —
      // que es hacia donde estaba el arco rival cuando lo apoyaron.
      if (PATEAR_AL_RUMBO0 && giroscopoSano()) {
        Serial.print("    -> voy a apuntar al rumbo de arranque (");
        Serial.print(rumboCero, 1); Serial.println(" grados) y patear ahi");
        cambiarA(APUNTA_RUMBO0);
      } else {
        cambiarA(BUSCANDO);
      }
    }
    else {
      // Los primeros MS_ORB_IMPULSO ms de CADA entrada a ORBITANDO van con el
      // golpe de arranque; despues baja a la velocidad de crucero y sigue por
      // inercia. enEstado se reinicia solo en cambiarA(), asi que el golpe se
      // da una vez por orbita y no se repite.
      bool enImpulso = (enEstado < (unsigned long)MS_ORB_IMPULSO);
      orbitar(sentidoParaOrbitar(), enImpulso ? VEL_ORB_IMPULSO : VEL_ORB_TRASERA);
    }
  }

  // ---------- APUNTA_RUMBO0 (plan B del giroscopo) ----------
  // Gira sobre el eje hasta mirar al rumbo con el que se encendio, y ahi
  // patea. Es peor que patear al arco de verdad, pero es MUCHO mejor que
  // rendirse: la pelota igual va para el lado correcto de la cancha.
  else if (estado == APUNTA_RUMBO0) {

    if (!laVeo) {
      Serial.println("... perdi la pelota apuntando al rumbo 0");
      cambiarA(BUSCANDO);
    }
    else if (!giroscopoSano()) {
      Serial.println("!!! el giroscopo se quedo mudo apuntando -> vuelvo a buscar");
      cambiarA(BUSCANDO);
    }
    else {
      float err = diferencia(rumboCero, rumboActual());
      if (fabs(err) <= TOL_RUMBO) {
        Serial.print("*** mirando al rumbo de arranque (error ");
        Serial.print(err, 1); Serial.println(" grados) -> PATADA");
        cambiarA(PATEA_ADEL);
      }
      else if (enEstado > MS_APUNTAR_MAX) {
        Serial.println("... no llegue a apuntar al rumbo 0 -> vuelvo a buscar");
        cambiarA(BUSCANDO);
      }
      else {
        // Mismo truco de pulsos que CENTRANDO: girar despacio sin bajar del
        // piso de arranque. El signo es una HIPOTESIS -> GIRO_RUMBO_INVERTIDO.
        bool haciaUnLado = (err > 0);
        if (GIRO_RUMBO_INVERTIDO) haciaUnLado = !haciaUnLado;
        rotarPulsado(haciaUnLado, VEL_CENT, MS_PULSO_CENT, MS_ESPERA_CENT);
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
      Serial.print(" cm -> a orbitar buscando el arco ");
      Serial.println(arcoNombre());
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
    Serial.print(" (a "); Serial.print(angPelota, 1); Serial.print(" grados)");
    Serial.print("  arco "); Serial.print(arcoNombre()); Serial.print(": ");
    if (veoArco) { Serial.print("a "); Serial.print(angArco, 1); Serial.println(" grados"); }
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
    Serial.print("  angPelota="); Serial.print(angPelota, 1);
    Serial.print("  angArco=");
    if (veoArco) Serial.print(angArco, 1); else Serial.print("--");
    Serial.print("  separacion=");
    if (veoArco) Serial.print(fabs(diferencia(angArco, angPelota)), 1); else Serial.print("--");
    if (giroscopoSano()) { Serial.print("  rumbo="); Serial.print(ultimoRumbo, 0); }
    Serial.print("  linea=");
    Serial.print(analogRead(pinLinea[0])); Serial.print("/");
    Serial.print(analogRead(pinLinea[1])); Serial.print("/");
    Serial.print(analogRead(pinLinea[2]));
    Serial.print(lineaArmada ? " [armado]" : " [SIN ARMAR]");
    Serial.println();

    unsigned long dt = millis() - t_contadores;
    if (dt > 0) {
      Serial.print("        camara: "); Serial.print(nPaquetes * 1000UL / dt);
      Serial.print(" paq/s   "); Serial.print(nTirados * 1000UL / dt);
      Serial.print(" bytes tirados/s   loop: "); Serial.print(nLoops * 1000UL / dt);
      Serial.println(" /s");
    }
    nPaquetes = nTirados = nLoops = 0;
    t_contadores = millis();
  }
}
