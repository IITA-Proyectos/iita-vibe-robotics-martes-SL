/* =====================================================================
   GIROSCOPO-DFROBOT — el mismo chip, pero con la libreria de la otra marca
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-09-29
   =====================================================================

   PARA QUE SIRVE
   El equipo encontro los programas de ejemplo de DFRobot para el BNO055 y
   propuso probarlos. La idea es buena y vale la pena por un motivo preciso:

        DFRobot es un driver COMPLETAMENTE INDEPENDIENTE del de Adafruit.
        Otro codigo, otra gente, escrito de cero para el mismo chip.

   Si el sensor le contesta a uno y no al otro, el problema es la libreria.
   Si no le contesta a ninguno, el problema no es software y no hay libreria
   que lo arregle: es el conector (VIN, GND, SDA 18, SCL 19).

   ---------------------------------------------------------------------
   Y ADEMAS EL DRIVER DE DFROBOT ES MEJOR EN DOS COSAS
   ---------------------------------------------------------------------
   1. NINGUN LAZO SIN SALIDA. Su begin() primero lee el CHIP_ID y se va si
      no coincide; y la espera despues del reset tiene tope de 1 segundo.
      El begin() de Adafruit, en cambio, tiene esto (linea 40 de
      Adafruit_BNO055.cpp):

           while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) { delay(10); }

      Sin tope. Si el chip no vuelve del reset, el programa muere ahi.

   2. DICE POR QUE FALLO, no solo que fallo. Adafruit devuelve true o false;
      DFRobot distingue:

           device not detected     -> no contesta en el bus (electrico)
           device ready time out   -> contesta, pero no termina de arrancar
           device internal status  -> arranco mal por dentro

      Son tres causas distintas que apuntan a lugares distintos.

   ---------------------------------------------------------------------
   ⚠️ OJO CON UNA DIFERENCIA QUE NO SE VE
   ---------------------------------------------------------------------
   Las dos librerias configuran los EJES distinto:

           Adafruit -> REMAP_CONFIG_P2
           DFRobot  -> eMapConfig_P1

   O sea que el mismo giro fisico puede dar el rumbo subiendo con una
   libreria y bajando con la otra. Si algun dia cambiamos de libreria en el
   programa de juego, TODOS los signos hay que volver a probarlos.

   ---------------------------------------------------------------------
   QUE SE LE CAMBIO AL EJEMPLO DE DFROBOT, Y POR QUE
   ---------------------------------------------------------------------
   1. 19200 baudios en vez de 115200, que es a lo que abre nuestro
      `mirar.bat`. Con 115200 se ve todo como basura.
   2. LOS MOTORES SE APAGAN en setup(), como todas nuestras herramientas.
   3. NO SE CUELGA. El original hace `while(bno.begin() != eStatusOK)` y no
      sale nunca. Aca reintenta y sigue informando, asi se puede MENEAR EL
      CONECTOR y ver si aparece.
   4. PARPADEO RAPIDO del LED mientras no encuentra el chip — el mismo
      aviso que todos nuestros programas. En la cancha no hay cable.
   5. Una tecla para cambiar de modo, que es la pregunta que tenemos
      abierta: `n` = NDOF (con brujula), `u` = IMU (sin brujula).

   ESTE PROGRAMA NO MUEVE EL ROBOT.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA, Y PRENDIDA ANTES DEL USB: el
   giroscopio come de la bateria, y con el USB ya puesto prender la bateria
   NO reinicia el Teensy.

        prender la bateria -> enchufar el USB -> abrir mirar.bat
   ===================================================================== */

#include "DFRobot_BNO055.h"
#include "Wire.h"

#define LED 13

// Motores: se apagan una vez y no se tocan nunca mas.
#define INA1 2
#define INB1 5
#define PWM1 3
#define INA2 8
#define INB2 7
#define PWM2 6
#define INA3 11
#define INB3 12
#define PWM3 4

typedef DFRobot_BNO055_IIC    BNO;
BNO bno(&Wire, 0x28);

const unsigned long BAUDIOS = 19200;   // el de mirar.bat, NO los 115200 del ejemplo
const unsigned long MS_ENTRE_INTENTOS = 2000;
const unsigned long MS_ENTRE_INFORMES = 1000;
const unsigned long MS_PARPADEO_AVISO = 50;    // 10 destellos por segundo

bool anduvo = false;
int  intentos = 0;
BNO::eOprMode_t modoPedido = BNO::eOprModeNdof;

void apagarMotores() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

// Lo mejor del ejemplo de DFRobot: no dice "fallo", dice POR QUE fallo.
void decirElEstado(BNO::eStatus_t e) {
  switch (e) {
    case BNO::eStatusOK:
      Serial.println("todo bien"); break;
    case BNO::eStatusErr:
      Serial.println("error desconocido"); break;
    case BNO::eStatusErrDeviceNotDetect:
      Serial.println("NO SE DETECTA EL CHIP");
      Serial.println("     -> no contesta en el bus. Eso es electrico:");
      Serial.println("        VIN, GND, SDA (18), SCL (19). ¿Bateria prendida?");
      break;
    case BNO::eStatusErrDeviceReadyTimeOut:
      Serial.println("CONTESTA PERO NO TERMINA DE ARRANCAR");
      Serial.println("     -> 🎯 esto es distinto y es un dato NUEVO: el chip");
      Serial.println("        esta ahi y se reinicia, pero no llega a estar");
      Serial.println("        listo. Apunta a alimentacion floja, no a cable");
      Serial.println("        cortado.");
      break;
    case BNO::eStatusErrDeviceStatus:
      Serial.println("ARRANCO MAL POR DENTRO");
      break;
    default:
      Serial.println("estado desconocido"); break;
  }
}

const char* nombreModo(BNO::eOprMode_t m) {
  if (m == BNO::eOprModeImu)  return "IMU (sin brujula)";
  if (m == BNO::eOprModeNdof) return "NDOF (con brujula)";
  return "otro";
}

// Intenta levantarlo. Devuelve true solo si de verdad arranco.
bool intentarArrancar() {
  intentos++;
  Serial.print("["); Serial.print(intentos); Serial.print("] reset + begin: ");
  bno.reset();                          // esto el ejemplo lo hace y Adafruit no
  BNO::eStatus_t e = bno.begin();
  decirElEstado(e);
  if (e != BNO::eStatusOK) return false;

  // begin() deja el chip en NDOF. Si queremos IMU hay que pasar por config.
  if (modoPedido != BNO::eOprModeNdof) {
    bno.setOprMode(BNO::eOprModeConfig);
    delay(30);
    bno.setOprMode(modoPedido);
    delay(50);
  }
  Serial.print("    >> ARRANCO, en modo "); Serial.println(nombreModo(modoPedido));
  return true;
}

void leerTeclado() {
  while (Serial.available()) {
    char t = Serial.read();
    if (t == '\n' || t == '\r') continue;
    if (t == 'n' || t == 'u') {
      modoPedido = (t == 'u') ? BNO::eOprModeImu : BNO::eOprModeNdof;
      Serial.println();
      Serial.print(">> pasando a modo "); Serial.println(nombreModo(modoPedido));
      anduvo = intentarArrancar();
    } else if (t == 'r') {
      Serial.println();
      Serial.println(">> reintentando a mano");
      anduvo = intentarArrancar();
    } else if (t == '?') {
      Serial.println();
      Serial.println("  n = modo NDOF (con brujula)    u = modo IMU (sin brujula)");
      Serial.println("  r = reintentar                 ? = esta ayuda");
    }
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  apagarMotores();          // y no se vuelven a tocar

  Serial.begin(BAUDIOS);
  Wire.begin();
  delay(800);

  Serial.println();
  Serial.println("=========================================================");
  Serial.println(" GIROSCOPO-DFROBOT — el mismo chip, otra libreria");
  Serial.println(" (el robot NO se mueve)");
  Serial.println("=========================================================");
  Serial.println(" Si contesta ACA y no con Adafruit, el problema era la");
  Serial.println(" libreria. Si no contesta con ninguna de las dos, es el");
  Serial.println(" conector y no hay software que lo arregle.");
  Serial.println();
  Serial.println(" 🎯 MENEA EL CONECTOR mientras esto reintenta.");
  Serial.println(" 🚨 LA BATERIA TIENE QUE ESTAR PRENDIDA.");
  Serial.println(" Teclas: n = NDOF   u = IMU   r = reintentar   ? = ayuda");
  Serial.println("=========================================================");

  anduvo = intentarArrancar();
}

unsigned long t_intento = 0, t_informe = 0;

void loop() {
  leerTeclado();
  unsigned long ahora = millis();

  // ---- todavia no aparecio: reintentar y avisar con el LED ----
  if (!anduvo) {
    digitalWrite(LED, ((ahora / MS_PARPADEO_AVISO) % 2) ? HIGH : LOW);
    if (ahora - t_intento >= MS_ENTRE_INTENTOS) {
      t_intento = ahora;
      anduvo = intentarArrancar();
      if (anduvo) {
        Serial.println("    🎯 SI APARECIO JUSTO AL MOVER EL CABLE, ES ESE CABLE.");
      }
    }
    return;
  }

  // ---- ya lo tenemos: leerlo ----
  if (ahora - t_informe < MS_ENTRE_INFORMES) return;
  t_informe = ahora;
  digitalWrite(LED, HIGH);

  BNO::sEulAnalog_t    eul = bno.getEul();
  BNO::sAxisAnalog_t   gyr = bno.getAxis(BNO::eAxisGyr);
  BNO::sRegCalibState_t cal = bno.getCalStatus();

  // Los tres en cero exacto sigue siendo la firma de "no puedo leer el chip".
  bool mudo = (eul.head == 0.0 && eul.roll == 0.0 && eul.pitch == 0.0);

  Serial.print("modo "); Serial.print(nombreModo(modoPedido));
  if (mudo) {
    Serial.print("   MUDO (puros ceros)");
  } else {
    Serial.print("   head "); Serial.print(eul.head, 1);
    Serial.print("  roll ");  Serial.print(eul.roll, 1);
    Serial.print("  pitch "); Serial.print(eul.pitch, 1);
  }
  // La velocidad de giro en grados por segundo. Con el robot QUIETO tiene
  // que dar casi cero: lo que sobre de cero es la deriva que vamos a
  // arrastrar en modo IMU, y se puede leer directo aca.
  Serial.print("   giro dps x "); Serial.print(gyr.x, 1);
  Serial.print(" y ");            Serial.print(gyr.y, 1);
  Serial.print(" z ");            Serial.print(gyr.z, 1);
  Serial.print("   calib sis ");  Serial.print(cal.SYS);
  Serial.print(" giro ");         Serial.print(cal.GYR);
  Serial.print(" acel ");         Serial.print(cal.ACC);
  Serial.print(" brujula ");      Serial.println(cal.MAG);

  if (mudo) {
    anduvo = false;                 // volver a la rutina de reintentos
    Serial.println("   se quedo mudo: vuelvo a intentar levantarlo");
  }
}
