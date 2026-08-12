/* =====================================================================
   VER LOS SENSORES DE LINEA — cual es cual, y cuanto marcan
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-11
   =====================================================================

   PARA QUE SIRVE
   El programa tiene tres sensores de linea llamados 1, 2 y 3. Estan en los
   pines A11, A13 y A12. Pero **nadie sabe cual de esos numeros es el sensor
   de adelante y cuales los dos de atras**. Es el mismo problema que tuvimos
   con las ruedas: el codigo tiene nombres, el robot tiene posiciones, y
   nadie escribio la correspondencia.

   Tampoco sabemos con que numero cada sensor "ve blanco". El codigo 2025
   usa 500, 650 y 600, pero son de la luz de otro lugar y otro año.

   Este sketch NO MUEVE NINGUN MOTOR. Solo muestra los tres numeros en vivo.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Robot con bateria y cable USB. Monitor serie a 19200.
   2. Apoyalo sobre el VERDE de la cancha y mirá los tres numeros: eso es
      "verde".
   3. Pasá algo blanco por debajo de UN sensor por vez. El numero que salta
      te dice cual es ese sensor.
   4. Anotá para cada sensor: cuanto marca en verde y cuanto en blanco.

   El sketch se acuerda del MINIMO y el MAXIMO que vio cada sensor, asi no
   hay que leer al vuelo mientras se mueve la hoja. La tecla 'r' los borra
   para empezar de nuevo.

   ---------------------------------------------------------------------
   POR QUE IMPORTA EL MINIMO Y EL MAXIMO
   ---------------------------------------------------------------------
   El umbral no se elige a ojo: se pone JUSTO EN EL MEDIO entre el verde y
   el blanco. Si queda muy cerca del verde, el robot ve lineas donde no hay;
   si queda muy cerca del blanco, no las ve nunca.

   Y si para algun sensor el verde y el blanco dan casi lo mismo, ese sensor
   no sirve para decidir nada — mejor saberlo ahora que en la cancha.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        r = borrar los minimos y maximos guardados
        ? = ayuda

   ---------------------------------------------------------------------
   PINES (arquero, ROBOT1 — de zirconLib.cpp:259-261)
   ---------------------------------------------------------------------
        sensor 1 -> A11        sensor 2 -> A13        sensor 3 -> A12
   ===================================================================== */

#define LINE_PIN1 A11
#define LINE_PIN2 A13
#define LINE_PIN3 A12

const unsigned long MS_ENTRE_INFORMES = 200;

int minimo[3] = { 9999, 9999, 9999 };
int maximo[3] = { -1, -1, -1 };
unsigned long t_informe = 0;


void ayuda() {
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" SENSORES DE LINEA — no muevo ningun motor");
  Serial.println("=================================================");
  Serial.println("  Apoyalo en el VERDE y mira los numeros.");
  Serial.println("  Pasa algo BLANCO bajo un sensor por vez.");
  Serial.println("  El que salta, es ese.");
  Serial.println("  r = borrar minimos y maximos");
  Serial.println("=================================================");
  Serial.println("      s1(A11)        s2(A13)        s3(A12)");
}


void setup() {
  pinMode(LINE_PIN1, INPUT);
  pinMode(LINE_PIN2, INPUT);
  pinMode(LINE_PIN3, INPUT);

  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }
  ayuda();
  t_informe = millis();
}


void loop() {

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') {
      for (int i = 0; i < 3; i++) { minimo[i] = 9999; maximo[i] = -1; }
      Serial.println("   (borrado — volve a pasar el blanco)");
    } else if (c == '?') {
      ayuda();
    }
  }

  int v[3];
  v[0] = analogRead(LINE_PIN1);
  v[1] = analogRead(LINE_PIN2);
  v[2] = analogRead(LINE_PIN3);

  for (int i = 0; i < 3; i++) {
    if (v[i] < minimo[i]) minimo[i] = v[i];
    if (v[i] > maximo[i]) maximo[i] = v[i];
  }

  if (millis() - t_informe < MS_ENTRE_INFORMES) return;
  t_informe = millis();

  for (int i = 0; i < 3; i++) {
    Serial.print("  ");
    if (v[i] < 1000) Serial.print(" ");
    if (v[i] < 100)  Serial.print(" ");
    if (v[i] < 10)   Serial.print(" ");
    Serial.print(v[i]);
    Serial.print(" ["); Serial.print(minimo[i]);
    Serial.print("-");  Serial.print(maximo[i]); Serial.print("]");
  }
  Serial.println();
}
