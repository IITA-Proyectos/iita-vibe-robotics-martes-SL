#include "zirconLib.h"
#include <Arduino.h>

Adafruit_BNO055 bno; // Define the Adafruit_BNO055 object

String ZirconVersion;
boolean compassCalibrated = false;

#define motorLimit 100

int motor1dir1;
int motor1dir2;
int motor1pwm;
int motor2dir1;
int motor2dir2;
int motor2pwm;
int motor3dir1;
int motor3dir2;
int motor3pwm;

int ballpin1;
int ballpin2;
int ballpin3;
int ballpin4;
int ballpin5;
int ballpin6;
int ballpin7;
int ballpin8;

int ballpins[8];

int buttonpin1;
int buttonpin2;

int linepin1;
int linepin2;
int linepin3;


void InitializeZircon() {
  
  setZirconVersion();
  initializePins();


  //CalibrateCompass();
  
  
  
}

void setZirconVersion() {
  pinMode(32, INPUT_PULLDOWN);

  if (digitalRead(32) == LOW) {
    ZirconVersion = "Mark1";
  } else {
    ZirconVersion = "Naveen1";
  }
}

/**void CalibrateCompass() {
  if (ZirconVersion == "Mark1") {
    bno = Adafruit_BNO055(55, 0x28, &Wire);
  } else if (ZirconVersion == "Naveen1") {
    bno = Adafruit_BNO055(55, 0x28, &Wire2);
  }
  
  if (bno.begin())
  {
    // COMPASS DETECTED
    uint8_t system, gyro, accel, mag = 0;

    while (mag < 3) {
      bno.getCalibration(&system, &gyro, &accel, &mag);
      Serial.println("Calibrate your compass sensor!");
      Serial.println(String(mag) + "/3 magnetometer");
      // Serial.println(String(gyro) + "/3 gyroscope");
      Serial.println();
      delay(100);

    }
    compassCalibrated = true;

  
  } else {
    // NO COMPASS DETECTED
    compassCalibrated = false;
  }

  
}**/

double readCompass() {
  if (compassCalibrated) {
    sensors_event_t orientationData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    return orientationData.orientation.x;
  } else {
    Serial.println("Trying to read compass but compass is not calibrated! Check if compass is plugged in.");
    return 0;
  }
  
}

int readBall(int ballSensorNumber) {
  // ball sensor numbers are from 1 to 8
  // so to index into array we subtract 1
  int ballpin;
  switch (ballSensorNumber) {
    case 1:
      ballpin = ballpin1; // Replace with the appropriate pin for sensor 1
      break;
    case 2:
      ballpin = ballpin2; // Replace with the appropriate pin for sensor 2
      break;
    case 3:
      ballpin = ballpin3; // Replace with the appropriate pin for sensor 3
      break;
    case 4:
      ballpin = ballpin4; // Replace with the appropriate pin for sensor 4
      break;
    case 5:
      ballpin = ballpin5; // Replace with the appropriate pin for sensor 5
      break;
    case 6:
      ballpin = ballpin6; // Replace with the appropriate pin for sensor 6
      break;
    case 7:
      ballpin = ballpin7; // Replace with the appropriate pin for sensor 7
      break;
    case 8:
      ballpin = ballpin8; // Replace with the appropriate pin for sensor 8
      break;
    default:
      // Invalid sensor number, return an error value or handle it as desired
      Serial.println("Trying to read invalid ball sensor number: " + String(ballSensorNumber));
      return -1;
  }

  return 1024 - analogRead(ballpin);
}

int readButton(int buttonNumber) {
  // buttonNumber is from 1 to 2
  // so to index into array we subtract 1
  if (buttonNumber == 1) {
    return digitalRead(buttonpin1);
  } else if (buttonNumber == 2) {
    return digitalRead(buttonpin2);
  } else {
    Serial.println("Trying to read invalid button number: " + String(buttonNumber));
    return 0;
  }
}

int readLine(int lineNumber) {
  // buttonNumber is from 1 to 3
  // so to index into array we subtract 1
  if (lineNumber == 1) {
    return analogRead(linepin1);
  } else if (lineNumber == 2) {
    return analogRead(linepin2);
  } else if (lineNumber == 3) {
    return analogRead(linepin3);
  } else {
    Serial.println("Trying to read invalid button number: " + String(lineNumber));
    return 0;
  }
}


void motor1(int power, bool direction) {
  power = min(power, motorLimit);
  if (ZirconVersion == "Naveen1") {
    if (direction == 0) {
      analogWrite(motor1dir1, 0);  // DIR 1
      analogWrite(motor1dir2, power);  // DIR 2
    } else {
      analogWrite(motor1dir1, power);  // DIR 1
      analogWrite(motor1dir2, 0);  // DIR 2
    }

  } else if (ZirconVersion == "Mark1") {
    digitalWrite(motor1dir1, direction);  // DIR 1
    digitalWrite(motor1dir2, !direction); // DIR 2
    analogWrite(motor1pwm, power);        // POWER
  } else {
    Serial.println("Zircon version not defined");
  }
  
}

void motor2(int power, bool direction) {
  power = min(power, motorLimit);
  if (ZirconVersion == "Naveen1") {
    if (direction == 0) {
      analogWrite(motor2dir1, 0);  // DIR 1
      analogWrite(motor2dir2, power);  // DIR 2
    } else {
      analogWrite(motor2dir1, power);  // DIR 1
      analogWrite(motor2dir2, 0);  // DIR 2
    }

  } else if (ZirconVersion == "Mark1") {
    digitalWrite(motor2dir1, direction);  // DIR 1
    digitalWrite(motor2dir2, !direction); // DIR 2
    analogWrite(motor2pwm, power);        // POWER
  } else {
    Serial.println("Zircon version not defined");
  }
}

void motor3(int power, bool direction) {
  power = min(power, motorLimit);
  if (ZirconVersion == "Naveen1") {
    if (direction == 0) {
      analogWrite(motor3dir1, 0);  // DIR 1
      analogWrite(motor3dir2, power);  // DIR 2
    } else {
      analogWrite(motor3dir1, power);  // DIR 1
      analogWrite(motor3dir2, 0);  // DIR 2
    }

  } else if (ZirconVersion == "Mark1") {
    digitalWrite(motor3dir1, direction);  // DIR 1
    digitalWrite(motor3dir2, !direction); // DIR 2
    analogWrite(motor3pwm, power);        // POWER
  } else {
    Serial.println("Zircon version not defined");
  }
}

void initializePins() {
  if (ZirconVersion == "Mark1") {
    motor1dir1 = 2;
    motor1dir2 = 5;
    motor1pwm = 3;
    motor2dir1 = 8;
    motor2dir2 = 7;
    motor2pwm = 6;
    motor3dir1 = 11;
    motor3dir2 = 12;
    motor3pwm = 4;


    ballpin1 = 14;
    ballpin2 = 15;
    ballpin3 = 16;
    ballpin4 = 17;
    ballpin5 = 20;
    ballpin6 = 21;
    ballpin7 = 22;
    ballpin8 = 23;

    buttonpin1 = 9;
    buttonpin2 = 10;

    linepin1 = A11;
    linepin2 = A13;
    linepin3 = A12;

  } else if (ZirconVersion == "Naveen1") {
    motor1dir1 = 3;
    motor1dir2 = 4;
    // motor1pwm = 3;
    motor2dir1 = 6;
    motor2dir2 = 7;
    // motor2pwm = 6;
    motor3dir1 = 5;
    motor3dir2 = 2;
    // motor3pwm = 4;

    ballpin1 = 20;
    ballpin2 = 21;
    ballpin3 = 14;
    ballpin4 = 15;
    ballpin5 = 16;
    ballpin6 = 17;
    ballpin7 = 18;
    ballpin8 = 19;

    buttonpin1 = 8;
    buttonpin2 = 10;

    linepin1 = A8;
    linepin2 = A9;
    linepin3 = A12;

  } else {
    motor1dir1 = 2;
    motor1dir2 = 5;
    motor1pwm = 3;
    motor2dir1 = 8;
    motor2dir2 = 7;
    motor2pwm = 6;
    motor3dir1 = 12;
    motor3dir2 = 11;
    motor3pwm = 4;

    ballpin1 = 14;
    ballpin2 = 15;
    ballpin3 = 16;
    ballpin4 = 17;
    ballpin5 = 20;
    ballpin6 = 21;
    ballpin7 = 22;
    ballpin8 = 23;

    buttonpin1 = 9;
    buttonpin2 = 10;

    linepin1 = A11;
    linepin2 = A13;
    linepin3 = A12;
    
  }

  //initialize motor pins
  pinMode(motor1dir1, OUTPUT);
  pinMode(motor1dir2, OUTPUT);
  pinMode(motor1pwm, OUTPUT);
  pinMode(motor2dir1, OUTPUT);
  pinMode(motor2dir2, OUTPUT);
  pinMode(motor2pwm, OUTPUT);
  pinMode(motor3dir1, OUTPUT);
  pinMode(motor3dir2, OUTPUT);
  pinMode(motor3pwm, OUTPUT);

  //initialize ball sensor pins and line sensor pins
  pinMode(ballpin1, INPUT);
  pinMode(ballpin2, INPUT);
  pinMode(ballpin3, INPUT);
  pinMode(ballpin4, INPUT);
  pinMode(ballpin5, INPUT);
  pinMode(ballpin6, INPUT);
  pinMode(ballpin7, INPUT);
  pinMode(ballpin8, INPUT);
  pinMode(buttonpin1, INPUT);
  pinMode(buttonpin2, INPUT);
  
}

String getZirconVersion() {
  return ZirconVersion;
}


bool isCompassCalibrated() {
  return compassCalibrated;
}



}
