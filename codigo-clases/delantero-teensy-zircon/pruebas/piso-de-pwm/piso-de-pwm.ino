/* =====================================================================
   PISO DE PWM — a partir de que potencia arranca cada rueda
   IITA Salta — martes — 2026-07-28
   =====================================================================

   DE DONDE SALE ESTO
   En banco se vio que la rueda DERECHA no arrancaba con PWM 100 en un
   sentido, y SI arranca con PWM 120. O sea: no esta rota, tiene el piso
   de arranque mas alto que las otras.

   QUE ES EL "PISO DE PWM"
   Un motor con reductor no arranca con cualquier potencia: por debajo de
   cierto valor el imán no vence el rozamiento del engranaje y el motor
   solo zumba. Ese valor minimo es el PISO. Cada motor tiene el suyo, y
   puede ser distinto en cada sentido de giro (el rozamiento no es igual).

   POR QUE IMPORTA
   avanzar() del codigo 2025 usa PWM 100. Si el piso de alguna rueda esta
   arriba de 100, ese codigo no arranca confiable en este robot HOY.

   ---------------------------------------------------------------------
   COMO SE CORRE
   ---------------------------------------------------------------------
   ROBOT LEVANTADO, ruedas al aire. Bateria puesta.
   Monitor serie a 19200 — ACA SI HACE FALTA, es donde se lee el numero.

   El programa sube la potencia de a 10, desde 40 hasta 200, en una rueda
   y un sentido por vez. Vos mira la rueda y anota el numero que aparece
   en pantalla JUSTO CUANDO empieza a girar. Ese es el piso.

   Son 6 rampas (3 ruedas x 2 sentidos). Cada una dura ~25 s.

   OJO — el piso con las ruedas AL AIRE es MENOR que el piso en el piso
   con el peso del robot encima. Este numero es el minimo absoluto; para
   competencia hay que medirlo tambien apoyado.
   ===================================================================== */

// ARQUERO (ROBOT1)
#define INA1 2      // IZQUIERDA
#define INB1 5
#define PWM1 3

#define INA2 8      // DERECHA
#define INB2 7
#define PWM2 6

#define INA3 11     // TRASERA
#define INB3 12
#define PWM3 4

const int DESDE = 40;
const int HASTA = 200;
const int PASO  = 10;
const int MS_ESCALON = 1500;


void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void rampa(const char* nombre, int ina, int inb, int pwmPin, int a, int b) {
  parar();
  delay(2500);
  Serial.println();
  Serial.println("--------------------------------------");
  Serial.print("RAMPA: "); Serial.println(nombre);
  Serial.println("Mira ESA rueda. Anota el numero de cuando arranca.");
  Serial.println("--------------------------------------");

  digitalWrite(ina, a);
  digitalWrite(inb, b);

  for (int v = DESDE; v <= HASTA; v += PASO) {
    analogWrite(pwmPin, v);
    Serial.print("   PWM = "); Serial.println(v);
    delay(MS_ESCALON);
  }

  parar();
  Serial.println("   (fin de la rampa)");
}


void setup() {
  pinMode(INA1, OUTPUT); pinMode(INB1, OUTPUT); pinMode(PWM1, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(PWM2, OUTPUT);
  pinMode(INA3, OUTPUT); pinMode(INB3, OUTPUT); pinMode(PWM3, OUTPUT);
  parar();

  Serial.begin(19200);
  while (!Serial && millis() < 6000) { }
  Serial.println();
  Serial.println("======================================");
  Serial.println("PISO DE PWM — robot LEVANTADO");
  Serial.println("Anota el PWM en que arranca cada rueda.");
  Serial.println("======================================");
  Serial.println("Arranca en 5 segundos.");
  for (int s = 5; s >= 1; s--) { Serial.print(s); Serial.println("..."); delay(1000); }
}


void loop() {

  rampa("IZQUIERDA sentido A", INA1, INB1, PWM1, 1, 0);
  rampa("IZQUIERDA sentido B", INA1, INB1, PWM1, 0, 1);

  rampa("DERECHA sentido A  <<< la que interesa", INA2, INB2, PWM2, 1, 0);
  rampa("DERECHA sentido B  <<< LA QUE FALLABA",  INA2, INB2, PWM2, 0, 1);

  rampa("TRASERA sentido A", INA3, INB3, PWM3, 1, 0);
  rampa("TRASERA sentido B", INA3, INB3, PWM3, 0, 1);

  parar();
  Serial.println();
  Serial.println("=== TERMINO ===");
  Serial.println("Anotalos asi, en la bitacora:");
  Serial.println("   izquierda  A=___  B=___");
  Serial.println("   derecha    A=___  B=___");
  Serial.println("   trasera    A=___  B=___");
  Serial.println();
  Serial.println("El mas alto de los seis manda: ninguna orden de motor");
  Serial.println("deberia mandar menos que ese numero + un margen.");
  Serial.println("Apreta RESET para repetir.");

  while (true) { parar(); delay(1000); }
}
