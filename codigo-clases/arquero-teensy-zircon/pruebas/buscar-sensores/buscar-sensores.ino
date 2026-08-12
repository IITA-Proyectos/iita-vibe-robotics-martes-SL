/* =====================================================================
   BUSCAR LOS SENSORES — en que pines estan realmente
   IITA Salta — taller de los martes — Roboliga 2026 — 2026-08-11
   =====================================================================

   POR QUE EXISTE ESTE SKETCH
   Leyendo los pines que dice la libreria (A11, A13, A12) resulta que solo
   UNO de los tres sensores de linea responde. Los otros dos dan un numero
   fijo, quieto como una piedra — que es exactamente lo que da un pin sin
   nada conectado.

   Puede ser que esos dos sensores esten rotos o desenchufados... o que en
   esta placa esten en OTROS pines. `zirconLib.cpp` tiene tres versiones de
   placa distintas: una usa A11/A13/A12 y otra usa A8/A9/A12. Nadie anoto
   cual es la nuestra.

   En vez de adivinar, le preguntamos a todos los pines a la vez.

   ---------------------------------------------------------------------
   COMO SE USA
   ---------------------------------------------------------------------
   1. Robot con bateria y cable. Monitor serie a 19200.
   2. Dejalo quieto 3 segundos: aprende cuanto marca cada pin en reposo.
   3. Pasá la hoja blanca por debajo, lento, varias veces, cubriendo TODO.
   4. El sketch lista los pines ORDENADOS por cuanto se movieron.

   Los que se movieron mucho tienen algo conectado que reacciona a la luz.
   Los que no se movieron estan al aire, rotos, o no son sensores de luz.

   ---------------------------------------------------------------------
   OJO CON ESTO AL LEER EL RESULTADO
   ---------------------------------------------------------------------
   Los pines 14 a 23 (A0..A9) son, segun el mapa de pines, los sensores
   INFRARROJOS de pelota — no los de linea. Pueden reaccionar igual a una
   hoja blanca. Que un pin se mueva no prueba que sea un sensor de linea;
   prueba que hay algo vivo ahi.

   Lo que buscamos son DOS pines que se muevan parecido al que ya sabemos
   que anda (A13), y que no sean los de la pelota.

   ---------------------------------------------------------------------
   TECLAS
   ---------------------------------------------------------------------
        r = volver a aprender el reposo y empezar de nuevo
        l = listar ahora el ranking
   ===================================================================== */

// Todas las entradas analogicas del Teensy 4.1
const int PINES[] = {
  A0, A1, A2, A3, A4, A5, A6, A7, A8,
  A9, A10, A11, A12, A13, A14, A15, A16, A17
};
const char* NOMBRES[] = {
  "A0 (14)", "A1 (15)", "A2 (16)", "A3 (17)", "A4 (18)", "A5 (19)",
  "A6 (20)", "A7 (21)", "A8 (22)", "A9 (23)", "A10(24)", "A11(25)",
  "A12(26)", "A13(27)", "A14(38)", "A15(39)", "A16(40)", "A17(41)"
};
const int CUANTOS = 18;

const unsigned long MS_APRENDER = 3000;

int reposo[CUANTOS];
int minimo[CUANTOS];
int maximo[CUANTOS];

bool aprendiendo = true;
unsigned long t_inicio = 0;
unsigned long t_listado = 0;


void reiniciar() {
  for (int i = 0; i < CUANTOS; i++) {
    reposo[i] = analogRead(PINES[i]);
    minimo[i] = 9999;
    maximo[i] = -1;
  }
  aprendiendo = true;
  t_inicio = millis();
  Serial.println();
  Serial.println(">> aprendiendo el reposo — NO TOQUES NADA por 3 segundos");
}


void listar() {
  // orden por cuanto se movio, de mayor a menor
  int orden[CUANTOS];
  for (int i = 0; i < CUANTOS; i++) orden[i] = i;
  for (int i = 0; i < CUANTOS - 1; i++) {
    for (int j = 0; j < CUANTOS - 1 - i; j++) {
      int a = orden[j], b = orden[j + 1];
      if ((maximo[a] - minimo[a]) < (maximo[b] - minimo[b])) {
        orden[j] = b; orden[j + 1] = a;
      }
    }
  }

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" PINES ORDENADOS POR CUANTO SE MOVIERON");
  Serial.println("=================================================");
  Serial.println("  pin        reposo   min   max   se movio");
  for (int k = 0; k < CUANTOS; k++) {
    int i = orden[k];
    int rango = maximo[i] - minimo[i];
    Serial.print("  "); Serial.print(NOMBRES[i]);
    Serial.print("   ");  Serial.print(reposo[i]);
    Serial.print("   ");  Serial.print(minimo[i]);
    Serial.print("   ");  Serial.print(maximo[i]);
    Serial.print("     "); Serial.print(rango);
    if (rango > 50) Serial.print("   <-- ALGO HAY ACA");
    Serial.println();
  }
  Serial.println("=================================================");
  Serial.println(" mas de ~50 = reacciona. menos de ~20 = pin al aire");
  Serial.println("=================================================");
}


void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 4000) { }
  Serial.println();
  Serial.println("=================================================");
  Serial.println(" BUSCAR LOS SENSORES — no muevo ningun motor");
  Serial.println("=================================================");
  Serial.println(" 1. dejalo quieto 3 segundos");
  Serial.println(" 2. pasa la hoja blanca por debajo, varias veces");
  Serial.println(" 3. mira el ranking (sale solo cada 5 s, o tecla 'l')");
  reiniciar();
}


void loop() {

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') reiniciar();
    else if (c == 'l') listar();
  }

  if (aprendiendo) {
    if (millis() - t_inicio >= MS_APRENDER) {
      for (int i = 0; i < CUANTOS; i++) reposo[i] = analogRead(PINES[i]);
      aprendiendo = false;
      t_listado = millis();
      Serial.println(">> listo. AHORA pasa la hoja blanca por debajo.");
    }
    return;
  }

  for (int i = 0; i < CUANTOS; i++) {
    int v = analogRead(PINES[i]);
    if (v < minimo[i]) minimo[i] = v;
    if (v > maximo[i]) maximo[i] = v;
  }

  if (millis() - t_listado >= 5000) { t_listado = millis(); listar(); }
}
