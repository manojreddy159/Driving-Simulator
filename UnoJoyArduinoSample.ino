
#include "UnoJoy.h"


// used for the Ultrasonic Distance sensor
const uint8_t trigPin = 6;
const uint8_t echoPin = 7;


void setup(){
  setupPins();
  setupUnoJoy();//  It sets up the hardware UnoJoy needs to work properly
}

void loop(){
   
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);
}

void setupPins(void){
  
  
   pinMode(32, INPUT);// for brake
   digitalWrite(32, HIGH);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

dataForController_t getControllerData(void){
  int X,Y;
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
// Buttons are low initially and become high when pressed
  //digitalWrite(41,LOW);
 // flag=true;
  controllerData.circleOn = LOW;
  controllerData.squareOn = LOW;
  controllerData.dpadUpOn = LOW;
  controllerData.dpadDownOn = LOW;
  controllerData.dpadLeftOn = LOW;
  controllerData.dpadRightOn = LOW;


  // for gear
  X=analogRead(A5);
  Y=analogRead(A6);
  
// each button is for one gear
  if(Y<260 && X < 260){
     
    controllerData.circleOn = HIGH;
  }
  else if(Y<260 && X> 260 && X<530){
   controllerData.squareOn = HIGH;
    
  }
  else if(Y<260 && X>700 ){
    controllerData.dpadUpOn = HIGH;
   }
  else if(Y>750 && X> 260 && X<530){
   controllerData.dpadDownOn = HIGH;
    
  }
  else if(Y>750 && X<260){
    controllerData.dpadLeftOn = HIGH;
  }
  else if(Y>750 && X>700){
   
   controllerData.dpadRightOn = HIGH;
  }
  
  //Initialising the other buttons to low
  controllerData.triangleOn = digitalRead(32);
  controllerData.crossOn = LOW;
  controllerData.l1On = LOW;
  controllerData.r1On = LOW;
  controllerData.selectOn = LOW;
  controllerData.startOn = LOW;
  controllerData.homeOn = LOW;
  
  int duration, distance;
  
  //accelerator using ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = map(duration,170,470,0,256);
  distance = constrain(distance,0,256);// even though we map things are exceeding 256 when initial exceed 470 so constraining it
  distance=distance*2;// left axis limit from 0 to 512 so multiplying by 2
  
  
  
  int X1=analogRead(A3);
  int Y1=analogRead(A4);
  
  
  if(X1>750 && Y1> 260 && Y1<530){
     
   controllerData.r1On = HIGH;
  }
  else if(X1<270 && Y1> 250 && Y1<530){
   controllerData.selectOn = HIGH;
  } 
  
//X1=map(X1,254,767,0,1023);
  Y1=map(Y1,239,782,0,1023);
  
  // Set the analog sticks
  //  Since analogRead(pin) returns a 10 bit value,
  //  we need to perform a bit shift operation to
  //  lose the 2 least significant bits and get an
  //  8 bit number that we can use  
  controllerData.leftStickX = analogRead(A0) >> 2;// A0 gives potentiometer value for steering
  controllerData.leftStickY = distance >> 2;
  controllerData.rightStickX = 512 >> 2;// 512 is the standard input
  controllerData.rightStickY = Y1 >> 2;

  return controllerData;
  
}
