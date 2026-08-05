/* =====================================================================
   VER LA CAMARA — que esta mandando la OpenMV, si es que manda algo
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-04
   =====================================================================

   PARA QUE SIRVE
   Antes de programar cualquier cosa que dependa de "ver la pelota", hay que
   saber si la camara existe, si esta enchufada al Teensy y si esta hablando.
   Este sketch NO MUEVE NINGUN MOTOR. Solo escucha y traduce.

   Es la primera pregunta, y nunca la hicimos.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Robot con bateria (la camara tambien se alimenta de ahi).
   2. Cargar esto y mirar el monitor serie a 19200.
   3. Poner la pelota naranja adelante del robot y sacarla, varias veces.

   ---------------------------------------------------------------------
   QUE VAS A VER, Y QUE SIGNIFICA CADA COSA
   ---------------------------------------------------------------------
   "no llega NADA"          -> la camara no esta mandando. Puede ser que no
                               este enchufada al Teensy (pines 0 y 1), que
                               no tenga programa adentro, o que no tenga
                               alimentacion. Mirale los LEDs a la camara.

   "llegan bytes pero no
    los entiendo"           -> algo manda, pero no con este protocolo. O la
                               camara tiene otro programa, o esta a otra
                               velocidad, o los cables 0 y 1 estan cruzados.

   "PELOTA a NN cm"         -> anda todo. Ese numero es lo que vamos a usar.

   ---------------------------------------------------------------------
   EL PROTOCOLO (de vision-openmv/README.md)
   ---------------------------------------------------------------------
   9 bytes de corrido, sin parar, a 19200:

     [201][Xp][Yp+100] [202][Xam][Yam+100] [203][Xaz][Yaz+100]

   Xp = distancia de la pelota en cm. **Xp = 0 significa NO LA VEO**, no
   significa "la tengo pegada". Es la confusion mas facil de cometer.
   Al Y hay que restarle 100 porque un byte no puede llevar negativos.
   ===================================================================== */

const unsigned long BAUDIOS = 19200;

byte paquete[9];
int  cuantos = 0;              // bytes juntados del paquete actual
bool sincronizado = false;

unsigned long bytesRecibidos = 0;
unsigned long paquetesBuenos = 0;
unsigned long paquetesRotos  = 0;
unsigned long t_informe = 0;

// ultimo dato bueno
int Xp = 0, Yp = 0, Xam = 0, Yam = 0, Xaz = 0, Yaz = 0;


void setup() {
  Serial.begin(BAUDIOS);          // el cable USB, para nosotros
  Serial1.begin(BAUDIOS);         // la camara, pines 0 (RX) y 1 (TX)

  while (!Serial && millis() < 4000) { }
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" VER LA CAMARA — no muevo ningun motor");
  Serial.println("=================================================");
  Serial.println(" Pone la pelota naranja adelante y sacala.");
  Serial.println(" Informe cada 1 segundo.");
  Serial.println("=================================================");
  t_informe = millis();
}


void loop() {

  // ---- juntar bytes y buscar el comienzo del paquete ----
  while (Serial1.available()) {
    byte b = Serial1.read();
    bytesRecibidos++;

    if (!sincronizado) {
      // Buscamos el 201, que marca el arranque del paquete. Hasta no
      // encontrarlo, todo lo que llega se tira: leer corrido da numeros
      // absurdos, que es peor que no leer nada.
      if (b == 201) { sincronizado = true; paquete[0] = b; cuantos = 1; }
      continue;
    }

    paquete[cuantos++] = b;
    if (cuantos < 9) continue;

    // paquete completo: validar las tres marcas
    cuantos = 0;
    sincronizado = false;

    if (paquete[0] == 201 && paquete[3] == 202 && paquete[6] == 203) {
      paquetesBuenos++;
      Xp  = paquete[1];  Yp  = paquete[2] - 100;
      Xam = paquete[4];  Yam = paquete[5] - 100;
      Xaz = paquete[7];  Yaz = paquete[8] - 100;
    } else {
      paquetesRotos++;
    }
  }

  // ---- informe una vez por segundo ----
  if (millis() - t_informe < 1000) return;
  t_informe = millis();

  if (bytesRecibidos == 0) {
    Serial.println("no llega NADA por Serial1 — la camara no esta hablando");
    Serial.println("   revisar: pines 0 y 1, alimentacion, y los LEDs de la camara");
    return;
  }

  if (paquetesBuenos == 0) {
    Serial.print("llegan bytes ("); Serial.print(bytesRecibidos);
    Serial.println(") pero NO los entiendo — otro programa u otra velocidad?");
    bytesRecibidos = 0; paquetesRotos = 0;
    return;
  }

  Serial.print(bytesRecibidos); Serial.print(" bytes | ");
  Serial.print(paquetesBuenos); Serial.print(" paquetes ok");
  if (paquetesRotos) { Serial.print(" | "); Serial.print(paquetesRotos);
                       Serial.print(" rotos"); }
  Serial.print("  ->  ");

  if (Xp == 0) {
    Serial.print("no veo la pelota");
  } else {
    Serial.print("PELOTA a "); Serial.print(Xp); Serial.print(" cm");
    Serial.print(", desviada "); Serial.print(Yp); Serial.print(" cm");
  }
  Serial.print("   [arco amarillo "); Serial.print(Xam == 0 ? 0 : Xam);
  Serial.print(" / azul "); Serial.print(Xaz == 0 ? 0 : Xaz);
  Serial.println("]");

  bytesRecibidos = 0; paquetesBuenos = 0; paquetesRotos = 0;
}
