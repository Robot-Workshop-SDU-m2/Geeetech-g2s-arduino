#include <AccelStepper.h>
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
#define MAX_SPEED 2000

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

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int stringLenght = 0;

#define MAX_COORDS 4

int ipointer = 0;
int opointer = 0;

long xCords[MAX_COORDS];
long yCords[MAX_COORDS];
long zCords[MAX_COORDS];

boolean moving = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(X_MAX_PIN, INPUT);
  pinMode(Y_MAX_PIN, INPUT);
  pinMode(Z_MAX_PIN, INPUT);
  
  xmotor.setMaxSpeed(MAX_SPEED);
  xmotor.setAcceleration(ACC);

  ymotor.setMaxSpeed(MAX_SPEED);
  ymotor.setAcceleration(ACC);
  
  zmotor.setMaxSpeed(MAX_SPEED);
  zmotor.setAcceleration(ACC);

  Serial.begin(115200);
  Serial.println("Booting v.1");

  inputString.reserve(50);
  
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
      Serial.println("Homing Done");
      break;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    if(stringLenght > 1){
      if(inputString[0] == 'G'){
        switch(inputString[1]){
          case '9': 
            home_r();
            break;
          case '0':
            Serial.println("G0 started");
            decodeSting();
            Serial.println("G0 ended");
            break;
        }
      }
    }
    inputString = "";
    stringComplete = false;
    stringLenght = 0;
  }
  if(!moving){
    if(opointer != ipointer){
      
      Serial.println("Poitner not equal");
      
      Serial.println(xCords[opointer]);
      Serial.println(yCords[opointer]);
      Serial.println(zCords[opointer]);
      
      xmotor.moveTo(xCords[opointer]);
      ymotor.moveTo(yCords[opointer]);
      zmotor.moveTo(zCords[opointer]);
      opointer = (opointer + 1) % MAX_COORDS;
      moving = true;
    }
  }else{
    xmotor.run();
    ymotor.run();
    zmotor.run();
    //Serial.println(xmotor.distanceToGo());
    if(xmotor.distanceToGo() == 0 && ymotor.distanceToGo() == 0 && zmotor.distanceToGo() == 0){
      moving = false;
    }
  }
}

void decodeSting(){
  int pointer = 2;
  int chordpos = 0;
  String pos = "";
  pos.reserve(8);
  while(true){
    if(inputString[++pointer] != ';'){
      pos += inputString[pointer];
    }else{
      Serial.println(pos);
      switch(chordpos++){
        case 0: xCords[ipointer] = pos.toInt(); pos = ""; break;
        case 1: yCords[ipointer] = pos.toInt(); pos = ""; break;
        case 2: zCords[ipointer] = pos.toInt(); pos = ""; break;
      }
      if(chordpos == 3) break;
    }
  }
  Serial.println("Egern er seje");
  ipointer = (ipointer + 1) % MAX_COORDS;
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    stringLenght++;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
