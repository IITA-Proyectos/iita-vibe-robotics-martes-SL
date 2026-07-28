#include <Arduino.h>
#include <zirconLib.h>
#include <Wire.h> 
#include <Adafruit_Sensor.h> 
#include <Adafruit_BNO055.h>

// Pines motores 

// ELEGI 1 
#define ROBOT1
//#define ROBOT2 

#if defined(ROBOT2)
  #define INA1 8
  #define INB1 7
  #define PWM1 6

  #define INA2 11
  #define INB2 12
  #define PWM2 4

  #define INA3 2
  #define INB3 5
  #define PWM3 3

  #define blanco1 650
  #define blanco2 650
  #define blanco3 750

  #define patadM2 200
  #define patadM1 250 
  #define c 0.4
  #define ic 0.55
  
#endif

#if defined(ROBOT1) 
  #define INA1 2
  #define INB1 5
  #define PWM1 3
  
  #define INA2 8
  #define INB2 7
  #define PWM2 6
  
  #define INA3 11
  #define INB3 12
  #define PWM3 4
  
  #define blanco1 500
  #define blanco2 650
  #define blanco3 600 

  #define patadM2 150
  #define patadM1 250
  #define c 0.4  
  #define ic 0.5 
  
#endif

// c: velocidad centrando 
// ic: velocidad impulso centrando 

// ARCO CONTRINCANTE
bool ARCO_CONTRINCANTE = false; 

 //giroscopo
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
float correccion;
float error = 0;
float initialYaw = 0;
float currentYaw;
float kp = 0.3;

// Comunicación serial
#define START_BYTE 0xAA;
const long BAUD_RATE = 19200;

// Variables velocidades
float g = 0.3; 
float a = 0.4; 

// Variables ángulos
float anguloPelota = 0.0;
float anguloArco_Amarillo = 0.0; 
float anguloArco_Azul = 0.0;
float pd = 1;

// Posiciones codificadas
int codedYp = 0, codedYam = 0, codedYaz = 0;
int codedXp = 0, codedXam = 0, codedXaz = 0;

// Posiciones decodificadas
float Xp = 0.0, Yp = 0.0;
float Xam = 0.0, Yam = 0.0;
float Xaz = 0.0, Yaz = 0.0;

// Headers 
int header1 = 0; 
int header2 = 0; 
int header3 = 0; 

// Presencia de objetos
bool haypelota = false;
bool hayarco_amarillo = false;
bool hayarco_azul = false;

// Tolerancias
const float tolerancia_centrado = 30.0;
const float tolerancia_cercania = 140.0;
const float tolerancia_apuntado = 15.0; 

// Máquina de estados
enum Estado { 
  // --- ARQUERO --- 
  impulso_inicial, 
  moverce_izquierda, moverce_derecha, 
  impulso_izquierda, impulso_derecha,
  PATEANDO_pausa_inicial_arquero, PATEANDO_adelante_arquero, PATEANDO_atras_arquero, PATEANDO_pausa_arquero,avanzar_despues_de_patear,

  // --- DELANTERO --- 
  AVANCE_INICIO, PRIMER_IMPULSO_INICIAL_GIRANDO, 
  IMPULSO_INICIAL_GIRANDO, GIRANDO, 
  APUNTAR_PELOTA, APUNTAR_PELOTA_antihorario, AVANZANDO, 
  CENTRANDO_horario, IMPULSO_CENTRANDO_antihorario, CENTRANDO_antihorario, IMPULSO_CENTRANDO_horario, CENTRANDO_giroscopo, 
  PATEANDO_corto_pausa_inicial, PATEANDO_corto_adelante, PATEANDO_corto_pausa, PATEANDO_corto_atras, 
  PATEANDO_pausa_inicial, PATEANDO_adelante, PATEANDO_pausa, PATEANDO_atras, 
  AVANZANDO_POR_TIEMPO, 
  DETECTA_LINEA_1, DETECTA_LINEA_2, DETECTA_LINEA_3 
}; 
Estado estado = impulso_inicial; 

unsigned long millis_inicio_estado = millis(); 
unsigned long millis_inicio_centrando = millis(); 
unsigned long millis_pelota = millis(); 
int i = 0; 

// ---- FUNCIONES DE MOVIMIENTO ----

void girar() {
  analogWrite(PWM1, 100 * g); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, 100 * g); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 100 * g); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
}

void parar() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 0); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0); digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}
void avanzar() {
  analogWrite(PWM1, 100); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, 100); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}
// RETROCEDER
void retroceder1() {
  analogWrite(PWM1, 0); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, 100); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 100);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}
void retroceder2() {
  analogWrite(PWM1, 100); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, 0); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 100);   digitalWrite(INA3, 0); digitalWrite(INB3, 1);
}
void retroceder3() {
  analogWrite(PWM1, 100); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, 100); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 0); digitalWrite(INB3, 1);
}

// PATEAR 
void avanzar_patear() {
  analogWrite(PWM1, patadM1); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
  analogWrite(PWM2, patadM2); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 0); digitalWrite(INB3, 0);
}

void retroceder_patear() {
  analogWrite(PWM1, patadM1); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
  analogWrite(PWM2, patadM2); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
  analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
}

// FUNCION AVANCE IZQUIERDO PROPORCIONAL 
void aiproporcional() {
  if (error>-1 && error <1){
  digitalWrite(INA2, 0);digitalWrite(INB2, 1);analogWrite(PWM2, pd*50);//60
  digitalWrite(INA1, 0);digitalWrite(INB1, 1);analogWrite(PWM1, pd*50);//60
  digitalWrite(INA3, 1);digitalWrite(INB3, 0);analogWrite(PWM3, pd*89);//99
  }
  else if (error > 0){ 
   //motor izquierdo
   digitalWrite(INA2, 0);digitalWrite(INB2, 1);analogWrite(PWM2, pd*50 );//60
   //motor derecho
   digitalWrite(INA1, 0);digitalWrite(INB1, 1);analogWrite(PWM1, pd*50 );//60
   //motor atras
   digitalWrite(INA3, 1);digitalWrite(INB3, 0);analogWrite(PWM3, pd*40);//50
  }
  else if (error < 0){
   //motor izquierdo
   digitalWrite(INA2, 0);digitalWrite(INB2, 1);analogWrite(PWM2, pd*65 );//75
   //motor derecho
   digitalWrite(INA1, 0);digitalWrite(INB1, 1);analogWrite(PWM1, pd*40 );//50
   //motor atras
   digitalWrite(INA3, 1);digitalWrite(INB3, 0);analogWrite(PWM3, pd*100);//120
  }
}
  //FUNCION AVANCE DERECHO PROPORCIONAL  
  void adproporcional() {
  if (error>-1 && error <1){
  digitalWrite(INA2, 1);digitalWrite(INB2, 0);analogWrite(PWM2, pd*50);//60
  digitalWrite(INA1, 1);digitalWrite(INB1, 0);analogWrite(PWM1, pd*50);//60
  digitalWrite(INA3, 0);digitalWrite(INB3, 1);analogWrite(PWM3, pd*89);//99
  }
  else if (error > 0){ 
   //motor izquierdo
   digitalWrite(INA2, 1);digitalWrite(INB2, 0);analogWrite(PWM2, pd*50 );//60
   //motor derecho
   digitalWrite(INA1, 1);digitalWrite(INB1, 0);analogWrite(PWM1, pd*50 );//60
   //motor atras
   digitalWrite(INA3, 0);digitalWrite(INB3, 1);analogWrite(PWM3, pd*100);//120
  }
  else if (error < 0){
   //motor izquierdo
   digitalWrite(INA2, 1);digitalWrite(INB2, 0);analogWrite(PWM2, pd*65 );//75
   //motor derecho
   digitalWrite(INA1, 1);digitalWrite(INB1, 0);analogWrite(PWM1, pd*40);//50
   //motor atras
   digitalWrite(INA3, 0);digitalWrite(INB3, 1);analogWrite(PWM3, pd*40);//50
  }
}

// ---- SETUP ----
void setup() {
  InitializeZircon();
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);

  Serial.println("Decodificador iniciado");

  millis_inicio_estado = millis();
  pinMode(LED_BUILTIN,OUTPUT);
//----------GIROSCOPO-------
  if (!bno.begin()) {
    Serial.println("¡No se pudo encontrar el BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

 sensors_event_t event;
  bno.getEvent(&event);
  initialYaw   = event.orientation.x; // 0..360
  millis_inicio_estado = millis(); 
}

// ---- LOOP ----
void loop() {

 digitalWrite(LED_BUILTIN,haypelota);
  // Lectura de datos serial
  if (Serial1.available() >= 9)
  {
    header1 = Serial1.read();
    if (header1 == 201)
    {
      codedXp = Serial1.read();
      codedYp = Serial1.read();
      header2 = Serial1.read(); 
      codedXam = Serial1.read();
      codedYam = Serial1.read();
      header3 = Serial1.read(); 
      codedXaz = Serial1.read();
      codedYaz = Serial1.read(); 

      if (header1 == 201 && header2 == 202 && header3 == 203)
      {
        Xp = codedXp; 
        Yp = codedYp - 100; 
        Xam = codedXam; 
        Yam = codedYam - 100; 
        Xaz = codedXaz; 
        Yaz = codedYaz - 100; 

        // Calcular el ángulos
          anguloPelota = atan2(Yp, Xp) * 180.0 / PI;
          anguloArco_Amarillo = atan2(Yam, Xam) * 180.0 / PI;
          anguloArco_Azul = atan2(Yaz, Xaz) * 180.0 / PI;

        // Mostrar datos
  //      Serial.print("X pelota: "); Serial.print(Xp);
  //      Serial.print(" | Y pelota: "); Serial.print(Yp);
  //      Serial.print(" | X arco amarillo: "); Serial.print(Xam);
  //      Serial.print(" | Y arco amarillo: "); Serial.print(Yam);
  //      Serial.print(" | X arco azul: "); Serial.print(Xaz);
  //      Serial.print(" | Y arco azul: "); Serial.println(Yaz);


        if ( Xp == 0 )
        { haypelota = false; }
        else
        { 
          haypelota = true; 
          millis_pelota = millis();           
        }

        if ( Xam == 0 )
        { hayarco_amarillo = false; }
        else
        { hayarco_amarillo = true; }
        
        if ( Xaz == 0 )
        { hayarco_azul = false; }
        else
        { hayarco_azul = true; }
  
  //      Serial.print("pelota: "); Serial.print(haypelota); 
  //      Serial.print("| arco amarillo: "); Serial.print(hayarco_amarillo);
  //      Serial.print("| arco azul: "); Serial.println(hayarco_azul);
      }
    } 
    else
    {
     hayarco_azul= false;
     hayarco_amarillo=false;
     haypelota=false; 
    }  
  }
// --- COLOCAR CUAL ES EL ARCO AL QUE AHI QUE HACER GOL --- 
  ARCO_CONTRINCANTE = hayarco_amarillo; 
  
  // GIROSCOPO
  sensors_event_t event;
  bno.getEvent(&event);
  currentYaw   = event.orientation.x;
  error =  currentYaw - initialYaw;
  if ( error > 180 ) error = error - 360;
  if (error < -180) error = error + 360;
  correccion= error * kp;

// Serial.println(currentYaw); 
  // LECTURA SENSORES DE LINEA 
  int s1 = readLine(1); // Sensor izquierdo
  int s2 = readLine(2); // Sensor centro
  int s3 = readLine(3); // Sensor derecho

    //Serial.print(" s1: ");
    //Serial.print(s1);
    //Serial.print(" | s2: ");
    //Serial.print(s2);
    //Serial.print(" | s3: ");
    //Serial.println(s3);

// Máquina de estados
  switch(estado) 
  { 
    case AVANCE_INICIO: 
      avanzar_patear();
      if (millis() - millis_inicio_estado >= 700) 
      {
        parar();
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO;
      }
    break; 
    
    case IMPULSO_INICIAL_GIRANDO: 
    // girar pero con más potencia
    analogWrite(PWM1, 150 ); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
    analogWrite(PWM2, 150 ); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
    analogWrite(PWM3, 150 ); digitalWrite(INA3, 0); digitalWrite(INB3, 1); 
    
    if (millis() - millis_inicio_estado >= 50) 
    { 
      millis_inicio_estado = millis(); 
      estado = GIRANDO; 
    } 

    if (haypelota) 
    { 
      parar(); 
      if(millis() - millis_inicio_estado >= 1000) // esperar un sgundo por la inercia 
      {
        estado = APUNTAR_PELOTA;
        millis_inicio_estado = millis(); 
      }
    } 
     
    if ( s1 >= blanco1 )
    { 
      estado = DETECTA_LINEA_1; 
      millis_inicio_estado = millis();
    }
    if ( s2 >= blanco2 )
    { 
      estado = DETECTA_LINEA_2; 
      millis_inicio_estado = millis();
    }
    if ( s3 >= blanco3 )
    { 
      estado = DETECTA_LINEA_3; 
      millis_inicio_estado = millis();
    }
    break; 

    case GIRANDO:
      if (haypelota) 
      { 
        parar(); 
        if(millis() - millis_inicio_estado >= 700) // esperar un sgundo por la inercia 
        {
          estado = APUNTAR_PELOTA;
          millis_inicio_estado = millis(); 
        }
      } 
      else 
      {
        girar();
      }

      if ((millis() - millis_inicio_estado >= 8000) && ((error <= 0) or (error >= 350)))  
      { 
        millis_inicio_estado = millis(); 
        estado = AVANZANDO_POR_TIEMPO;
      }

      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }

      break;

    case AVANZANDO_POR_TIEMPO: 
      avanzar(); 

      if (millis() - millis_inicio_estado >= 500)
      { 
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO; 
      }

    // SALIDAS
      if ( haypelota == true ) 
      {
        millis_inicio_estado = millis(); 
        estado = APUNTAR_PELOTA; 
      }

      // salida sensores blanco 
      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
    break; 

    case APUNTAR_PELOTA: 
    if (abs(anguloPelota) >= tolerancia_apuntado) 
    {
      if (anguloPelota > 0) 
      { 
        // enciende motores en sentido Anti horario 
        analogWrite(PWM1, 100 * a); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
        analogWrite(PWM2, 100 * a); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
        analogWrite(PWM3, 100 * a); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
      } 
      else 
      { 
        // enciende motores en sentido Horario 
        analogWrite(PWM1, 100 * a); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
        analogWrite(PWM2, 100 * a); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
        analogWrite(PWM3, 100 * a); digitalWrite(INA3, 1); digitalWrite(INB3, 0);
      } 
    }
    else
    {
      millis_inicio_estado = millis(); 
      estado = AVANZANDO; 
    }

    // si esta apuntando al arco idealmente 
    if (ARCO_CONTRINCANTE && haypelota && (abs(Yp - Yam)<= tolerancia_centrado)) 
    {
       millis_inicio_estado = millis();
       estado = PATEANDO_pausa_inicial;
    } 
        
    if (millis() - millis_pelota >= 500)   // si deja de ver la pelota
    {
      millis_inicio_estado = millis(); 
      estado = IMPULSO_INICIAL_GIRANDO;
    } 

    if (millis() - millis_inicio_estado >= 20000)   // timeout 
    {
      millis_inicio_estado = millis();
      estado = IMPULSO_INICIAL_GIRANDO;
    }

    // salida sensores blanco 
    if ( s1 >= blanco1 )
    { 
      estado = DETECTA_LINEA_1; 
      millis_inicio_estado = millis();
    }
    if ( s2 >= blanco2 )
    { 
      estado = DETECTA_LINEA_2; 
      millis_inicio_estado = millis();
    }
    if ( s3 >= blanco3 )
    { 
      estado = DETECTA_LINEA_3; 
      millis_inicio_estado = millis();
    }
    break;


    case AVANZANDO:
      avanzar();
    
    // SALIDAS 
      // si ya esta lo suficientemente cerca de la pelota 
      if (haypelota && Xp <= tolerancia_cercania) 
      {
        millis_inicio_centrando = millis(); 
        millis_inicio_estado = millis();
        estado = CENTRANDO_horario;
      } 

      // si deja de apuntar a la pelota 
      if (abs(anguloPelota) >= tolerancia_apuntado)
      { 
        millis_inicio_estado = millis(); 
        estado = APUNTAR_PELOTA;
      }

      // si deja de ver la pelota,  VER DESPUES 
      if (millis() - millis_pelota >= 500) 
      {
        millis_inicio_estado = millis(); 
        estado = IMPULSO_INICIAL_GIRANDO;
      } 

      // timeout
      if (millis() - millis_inicio_estado >= 20000)
      {
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO;
      }

      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
    break;

    case CENTRANDO_horario: 
            
        analogWrite(PWM1, 60 * c); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
        analogWrite(PWM2, 60 * c); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
        analogWrite(PWM3, 180 * c); digitalWrite(INA3, 1); digitalWrite(INB3, 0);
        
        // SALIDAS 
        // si esta apuntando al arco idealmente 
        if (ARCO_CONTRINCANTE && haypelota && (abs(Yp - Yam)<= tolerancia_centrado)) 
        {
            millis_inicio_estado = millis();
            estado = PATEANDO_pausa_inicial;
        } 

        if ( (millis()- millis_inicio_estado >= 5000) && ((currentYaw <= 10) or (currentYaw >= 350)))
        {           
           millis_inicio_estado = millis(); 
           estado = PATEANDO_pausa_inicial; 
        } 
            
        // si ya pasaron mas de 5 segundos y esta viendo amarillo pero no esta centrado el arco y la pelota 
        if ( (millis() - millis_inicio_estado >= 4500) && ARCO_CONTRINCANTE)
        {
            millis_inicio_estado = millis(); 
            estado = PATEANDO_pausa_inicial;
        }

        // corrección para que orbite bien la pelota 
        if (abs(anguloPelota) >= tolerancia_apuntado)
        { 
            millis_inicio_estado = millis(); 
            estado = APUNTAR_PELOTA;
        } 

        // si deja de ver la pelota por más de 4 segundos 
        if (millis() - millis_pelota >= 4000) 
        {
            millis_inicio_estado = millis();
            estado = IMPULSO_INICIAL_GIRANDO;
        } 
        // timeout 
        if (millis() - millis_inicio_estado >= 20000)
        {
            millis_inicio_estado = millis(); 
            estado = PATEANDO_pausa_inicial;
        }

        // salidas lineas blancas 
        if ( s1 >= blanco1 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial;
            }
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }        
        }
        if ( s2 >= blanco2 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial;
            }  
            
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }  
        }
        if ( s3 >= blanco3 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial;
            }  
            
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }  
        } 

    break;
    
    case IMPULSO_CENTRANDO_antihorario: 
        analogWrite(PWM1, 60 * ic); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
        analogWrite(PWM2, 60 * ic); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
        analogWrite(PWM3, 180 * ic); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
        
        if(millis() - millis_inicio_estado >= 500) 
        {
            millis_inicio_estado = millis();
            estado = CENTRANDO_antihorario; 
        }
    break; 
       
    case CENTRANDO_antihorario: 
        analogWrite(PWM1, 60 * c); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
        analogWrite(PWM2, 60 * c); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
        analogWrite(PWM3, 180 * c); digitalWrite(INA3, 0); digitalWrite(INB3, 1);

    // SALIDAS 
        // si esta apuntando al arco idealmente 
        if (ARCO_CONTRINCANTE && haypelota && (abs(Yp - Yam)<= tolerancia_centrado)) 
        {
            millis_inicio_estado = millis();
            estado = PATEANDO_pausa_inicial;
        } 

        if ( (millis()- millis_inicio_estado >= 5000) && ((currentYaw <= 10) or (currentYaw >= 350)))
        {           
           millis_inicio_estado = millis(); 
           estado = PATEANDO_pausa_inicial; 
        } 
        
        // si ya pasaron mas de 5 segundos y esta viendo amarillo pero no esta centrado el arco y la pelota 
        if ( (millis() - millis_inicio_estado >= 4500) && ARCO_CONTRINCANTE)
        {
            millis_inicio_estado = millis(); 
            estado = PATEANDO_pausa_inicial; 
        }

        // corrección para que orbite bien la pelota 
        if (abs(anguloPelota) >= tolerancia_apuntado)
        { 
            millis_inicio_estado = millis(); 
            estado = APUNTAR_PELOTA_antihorario;
        } 

        // si deja de ver la pelota por más de 3 segundos 
        if (millis() - millis_pelota >= 3000) 
        {
            millis_inicio_estado = millis();
            estado = IMPULSO_INICIAL_GIRANDO;
        } 

        // timeout 
        if (millis() - millis_inicio_estado >= 20000)
        {
            millis_inicio_estado = millis(); 
            estado = PATEANDO_pausa_inicial;
        }
        // salidas lineas blancas 
        if ( s1 >= blanco1 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial; 
            } 
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }     
        }
        if ( s2 >= blanco2 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial; 
            }
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }  
        }
        if ( s3 >= blanco3 )
        { 
            if ((currentYaw <= 90) or (currentYaw >= 270))
            {
           
               millis_inicio_estado = millis();
               estado = PATEANDO_corto_pausa_inicial; 
            }
            else
            {
              millis_inicio_estado = millis();
              estado = IMPULSO_CENTRANDO_antihorario;    
            }  
        } 

    break; 
    
    case IMPULSO_CENTRANDO_horario: 
        analogWrite(PWM1, 60 * ic); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
        analogWrite(PWM2, 60 * ic); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
        analogWrite(PWM3, 180 * ic); digitalWrite(INA3, 1); digitalWrite(INB3, 0);
        
        if(millis() - millis_inicio_estado >= 500) 
        {
            millis_inicio_estado = millis();
            estado = CENTRANDO_antihorario; 
        }
    break; 

    case APUNTAR_PELOTA_antihorario: 
      if (abs(anguloPelota) >= tolerancia_apuntado) 
      {
        if (anguloPelota > 0) 
        { 
          // enciende motores en sentido Anti horario 
          analogWrite(PWM1, 100 * a); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
          analogWrite(PWM2, 100 * a); digitalWrite(INA2, 0); digitalWrite(INB2, 1);
          analogWrite(PWM3, 100 * a); digitalWrite(INA3, 0); digitalWrite(INB3, 1);
        } 
        else 
        { 
          // enciende motores en sentido Horario 
          analogWrite(PWM1, 100 * a); digitalWrite(INA1, 1); digitalWrite(INB1, 0);
          analogWrite(PWM2, 100 * a); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
          analogWrite(PWM3, 100 * a); digitalWrite(INA3, 1); digitalWrite(INB3, 0);
        } 
      }
      else
      {
        millis_inicio_estado = millis(); 
        estado = CENTRANDO_antihorario; 
      }

    break;

    // --- PATADA CORTA--- 
    
    case PATEANDO_corto_pausa_inicial:
      parar();
      if (millis() - millis_inicio_estado >= 500) 
      {
        estado = PATEANDO_corto_adelante;
        millis_inicio_estado = millis();
      }  
      
    break; 
      
    case PATEANDO_corto_adelante:
      avanzar_patear();
      if (millis() - millis_inicio_estado >= 200) 
      {
        parar();
        millis_inicio_estado = millis();
        estado = PATEANDO_corto_pausa;
      }
      
    break;

    case PATEANDO_corto_pausa: 
      parar();
      if (millis() - millis_inicio_estado >= 500) 
      {
        estado = PATEANDO_corto_atras;
        millis_inicio_estado = millis();
      }  
      
    break;
      
    case PATEANDO_corto_atras: 
      retroceder_patear();
      if (millis() - millis_inicio_estado >= 400) 
      {
        parar();
        estado = IMPULSO_INICIAL_GIRANDO; 
        millis_inicio_estado = millis(); 
      }
      
    break; 
    
    // --- PATADA --- 
    
    case PATEANDO_pausa_inicial:
      parar();
      if (millis() - millis_inicio_estado >= 1000) 
      {
        estado = PATEANDO_adelante;
        millis_inicio_estado = millis();
      }  
      
      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
    break; 
      
    case PATEANDO_adelante:
      avanzar_patear();
      if (millis() - millis_inicio_estado >= 500) 
      {
        parar();
        millis_inicio_estado = millis();
        estado = PATEANDO_pausa;
      }
      
      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
    break;

    case PATEANDO_pausa: 
      parar();
      if (millis() - millis_inicio_estado >= 500) 
      {
        estado = PATEANDO_atras;
        millis_inicio_estado = millis();
      }  
      
      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
    break;
      
    case PATEANDO_atras: 
      retroceder_patear();
      if (millis() - millis_inicio_estado >= 200) 
      {
        parar();
        estado = IMPULSO_INICIAL_GIRANDO; 
        millis_inicio_estado = millis(); 
      }
      
      if ( s1 >= blanco1 )
      { 
        estado = DETECTA_LINEA_1; 
        millis_inicio_estado = millis();
      }
      if ( s2 >= blanco2 )
      { 
        estado = DETECTA_LINEA_2; 
        millis_inicio_estado = millis();
      }
      if ( s3 >= blanco3 )
      { 
        estado = DETECTA_LINEA_3; 
        millis_inicio_estado = millis();
      }
      
    break; 

    case DETECTA_LINEA_1: 
    
      retroceder1(); 
      if (millis() - millis_inicio_estado >= 400)
      {
        parar();
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO;  
      }
      
    break; 

    case DETECTA_LINEA_2: 
    
      retroceder2(); 
      if (millis() - millis_inicio_estado >= 400)
      {
        parar();
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO; 
      }
      
    break; 

    case DETECTA_LINEA_3: 
    
      retroceder3(); 
      if (millis() - millis_inicio_estado >= 400)
      {
        parar();
        millis_inicio_estado = millis();
        estado = IMPULSO_INICIAL_GIRANDO; 
      }
      
    break; 

    ///////////////////////// ARQUERO ///////////////////////// 

    case impulso_inicial:
    
      digitalWrite(INA2, 1);digitalWrite(INB2, 0);analogWrite(PWM2, 1.8*50); 
      digitalWrite(INA1, 1);digitalWrite(INB1, 0);analogWrite(PWM1, 1.8*50); 
      digitalWrite(INA3, 0);digitalWrite(INB3, 1);analogWrite(PWM3, 1.8*85); 
      
      if(millis() - millis_inicio_estado >= 40)
      {
        estado = moverce_derecha; 
        millis_inicio_estado = millis(); 
      }
      
    break; 
    
    case moverce_derecha: 
      adproporcional(); 

    // SALIDAS  
      // si ve pelota 
      
      if (haypelota) 
      {
        if( (Xp <= tolerancia_cercania) && (abs(Yp) <= 3) ) 
        {
          parar();
          estado = PATEANDO_pausa_inicial_arquero; 
          millis_inicio_estado = millis();
        }
        
        if (abs(Yp) >= 5)
        { 
          pd = 1.5;
            
          if( Yp < 0 )
          {
            estado = moverce_derecha;
            millis_inicio_estado = millis(); 
          }
          else
          {
            estado = moverce_izquierda;
            millis_inicio_estado = millis();  
          } 
        }
        else
        {parar();}
      }    
      else 
      {
        pd = 1; 
      }

    // si ve blanco 
    
      if (s1 >= blanco1 or s2 >= blanco2)
      {
          parar();
          estado = impulso_izquierda; 
          millis_inicio_estado = millis();
      }
    break; 
      
    case moverce_izquierda: 
      aiproporcional();

    // SALIDAS  
      // si ve pelota 
      
      if (haypelota) 
      {
        if( (Xp <= tolerancia_cercania) && (abs(Yp) <= 3) ) 
        {
            parar();
            estado = PATEANDO_pausa_inicial_arquero; 
            millis_inicio_estado = millis();
        } 
        
        if (abs(Yp) >= 5)
        { 
            pd = 1.5;
            
            if( Yp < 0 )
            {
              estado = moverce_derecha;
              millis_inicio_estado = millis(); 
            }
            else
            {
              estado = moverce_izquierda;
              millis_inicio_estado = millis();  
            } 
        }
        else
        {parar();}
      }    
      else 
      {
          pd = 1; 
      }

      // si ve blanco       
      if (s1 >= blanco1 or s2 >= blanco2)
      {
          parar();
          estado = impulso_derecha; 
          millis_inicio_estado = millis();
      }
      
    break; 

    case impulso_derecha: 
    // este caso es porque en los costados se traba con el blanco, porque cambia de moverce izquierda a moverce derecha erraticamente 
      adproporcional(); 
      
      if (millis() - millis_inicio_estado >= 350) 
      {
         estado = moverce_derecha;
         millis_inicio_estado = millis();
      } 
      
    break; 

    case impulso_izquierda: 
    // este caso es porque en los costados se traba con el blanco, porque cambia de moverce izquierda a moverce derecha erraticamente 
      aiproporcional(); 
      
      if (millis() - millis_inicio_estado >= 350) 
      {
        estado = moverce_izquierda;
        millis_inicio_estado = millis();
      } 
      
    break; 

    //--- PATADA ---
    case PATEANDO_pausa_inicial_arquero: 
    // por la inercia de la patada, ahi que esperar un rato antes de pasar de un movimiento horizontal a uno vertical como el de la patada 
      parar();
      if (millis() - millis_inicio_estado >= 200)
      {
         millis_inicio_estado = millis(); 
         estado = PATEANDO_adelante_arquero;  
      }
      
    break; 
    
    case PATEANDO_adelante_arquero: 
    
      avanzar_patear();
      if (millis() - millis_inicio_estado >= 450) 
      {
        parar();
        millis_inicio_estado = millis();
        estado = PATEANDO_pausa_arquero;
      }
      
    break;
  
    case PATEANDO_pausa_arquero: 
    // por la inercia de la patada, ahi que esperar un rato antes de pasar de un movimiento horizontal a uno vertical como el de la patada 
      parar();
      if (millis() - millis_inicio_estado >= 1000) 
      {
          estado = PATEANDO_atras_arquero;
          millis_inicio_estado = millis();
      }  
    break;
    
    case PATEANDO_atras_arquero: 
    // va atras hasta que detecta blanco 
      analogWrite(PWM1, 150); digitalWrite(INA1, 0); digitalWrite(INB1, 1);
      analogWrite(PWM2, 150); digitalWrite(INA2, 1); digitalWrite(INB2, 0);
      analogWrite(PWM3, 0);   digitalWrite(INA3, 1); digitalWrite(INB3, 0);
      
      if ((s1 >= blanco1) or (s2 >= blanco2) or (s3 >= blanco3)) 
      {
        estado = avanzar_despues_de_patear;
        millis_inicio_estado = millis();
      }
    break;
    
    case avanzar_despues_de_patear: 
    // avanza por tiempo hasta su lugar 
      avanzar();
      if(millis() - millis_inicio_estado >=1000)
      {
        estado = moverce_derecha;
        millis_inicio_estado = millis();
      }
    break;
  }
} 
