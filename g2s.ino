#include <AccelStepper.h>
#include <MultiStepper.h>
const int stepsPerRevolution = 6400;

/**** Pin configuration ****/
#define X_STEP_PIN 25
#define X_DIR_PIN 23
#define X_MAX_PIN 24
#define X_ENABLE_PIN 27

#define Y_STEP_PIN 31
#define Y_DIR_PIN 33
#define Y_MAX_PIN 28
#define Y_ENABLE_PIN 29

#define Z_STEP_PIN 37
#define Z_DIR_PIN 39
#define Z_MAX_PIN 32
#define Z_ENABLE_PIN 35

#define ACC 1600
#define MAX_SPEED 1000

/**** States and constants for homing ****/
#define HOME_INIT 0
#define HOME_FAST 1
#define HOME_SLOW 2
#define HOME_DONE 3

#define HOME_FAST_SPEED -800
#define HOME_SLOW_SPEED 100
#define HOME_MOVE -400

int xState = HOME_INIT;
int yState = HOME_INIT;
int zState = HOME_INIT;

AccelStepper xmotor(1, X_STEP_PIN, X_DIR_PIN);
AccelStepper ymotor(1, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper zmotor(1, Z_STEP_PIN, Z_DIR_PIN);

MultiStepper motor;

byte serialData[20]; // To hold serial data
byte serialDataLength = 0; // lengt of serial data
boolean serialDataComplete = false;  // whether the string is complete

#define MAX_COORDS 40

byte ipointer = 0;
byte opointer = 0;

long coord[MAX_COORDS][3];

boolean moving = false;

void setup() {
  pinMode(X_MAX_PIN, INPUT);
  pinMode(Y_MAX_PIN, INPUT);
  pinMode(Z_MAX_PIN, INPUT);
  
  xmotor.setMaxSpeed(MAX_SPEED);
  xmotor.setAcceleration(ACC);

  ymotor.setMaxSpeed(MAX_SPEED);
  ymotor.setAcceleration(ACC);
  
  zmotor.setMaxSpeed(MAX_SPEED);
  zmotor.setAcceleration(ACC);

  motor.addStepper(xmotor);
  motor.addStepper(ymotor);
  motor.addStepper(zmotor);

  Serial.begin(1000000);
  Serial.println("Booting");

  home_r();
}

void home_r(){
  xState = HOME_FAST;
  yState = HOME_FAST;
  zState = HOME_FAST;

  xmotor.setSpeed(HOME_FAST_SPEED);
  ymotor.setSpeed(HOME_FAST_SPEED);
  zmotor.setSpeed(HOME_FAST_SPEED);

  while(true){
    if(digitalRead(X_MAX_PIN) && xState == HOME_FAST){
      xState = HOME_SLOW;
    }
    if(digitalRead(Y_MAX_PIN) && yState == HOME_FAST){
      yState = HOME_SLOW;
    }
    if(digitalRead(Z_MAX_PIN) && zState == HOME_FAST){
      zState = HOME_SLOW;
    }

    if(xState == HOME_FAST){
      xmotor.runSpeed();
    }
    if(yState == HOME_FAST){
      ymotor.runSpeed();
    }
    if(zState == HOME_FAST){
      zmotor.runSpeed();
    }
    if(xState == HOME_SLOW && yState == HOME_SLOW && zState == HOME_SLOW){
      xmotor.setCurrentPosition(0);
      ymotor.setCurrentPosition(0);
      zmotor.setCurrentPosition(0);
      long zeros[] = {0,0,0};
      motor.moveTo(zeros);
      Serial.println("Homing Done");
      break;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (serialDataComplete) {
    // clear the string:
    if(serialDataLength > 1){
      if(serialData[0] == 'G'){
        switch(serialData[1]){
          case '9': 
            home_r();
            break;
          case '0':
            decodeSting();
            break;
        }
      }
    }
    serialDataComplete = false;
    serialDataLength = 0;
  }
  if(!motor.run()){
    if(opointer != ipointer){
      motor.moveTo(coord[opointer]);
      opointer = (opointer + 1) % MAX_COORDS;
      Serial.print("O\n");
    }
  }
}

void decodeSting(){
  if(serialDataLength == 9){
    coord[ipointer][0] = serialData[2] << 8 | serialData[3];
    coord[ipointer][1] = serialData[4] << 8 | serialData[5];
    coord[ipointer][2] = serialData[6] << 8 | serialData[7];
  }
  ipointer = (ipointer + 1) % MAX_COORDS;
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    byte inChar = Serial.read();
    // add it to the inputString:
    serialData[serialDataLength++] = inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      serialDataComplete = true;
    }
  }
}
