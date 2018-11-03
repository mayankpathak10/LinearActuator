# include <Stepper.h>
#include <Encoder.h>
#include "pins_arduino.h"

/* Componenets to be used in this program
  Arduino Mega
  Adafruit TB6612 Motor Driver - AIN1,AIN2,BIN1,BIN2,STBY
  Rotary Encoder - CLK,DT,SW
  Adafruit Nema 17 Stepper Motor
  Arudino Mega Interrupt pins: 2,3,21,20,19,18
*/

/*
 * To include the number of cycles and inFurnaceStopTime
 * NumCycles can vary, generally from 1 to 2000,
 * inFurnaceTime can vary from 1 sec to 30 sec., use millis() function for it. 
 * 
*/

/* Define pins to be used in this program
   AIN2 - 8   (GRAY)
   AIN1 - 9   (GREEN)
   BIN1 - 10  (YELLOW)
   BIN2 - 11  (RED)
   STBY - 7
   CLK - 2
   SW - 3
   DT - 4
*/
const int CLK = 2;
const int SW = 3;
const int DT = 18;
const int STBY = 7;
const int AIN2 = 8;
const int AIN1 = 9;
const int BIN1 = 10;
const int BIN2 = 11;



/* Define Variable used for this program
    powerStatus: boolean, volatile
    knobRotationDirection: boolean, ?volatile
    stepsPerRevolution: int, 200
    motorSpeed: int, 20
*/
int motorSpeed = 25;
int numCycles = 20;
int minSteps = 0;
int maxSteps = 650;
int minspeed = 0;
int maxspeed = 30;
unsigned long downStopTime = 30000; // milliSeconds
unsigned long upStopTime = 130000;  // milliSeconds
int stepCount = 0;
int cstate;
int newSpeed;
int lastState;
int UpWaitTime = 0;
int LastCyclePrint = 0;
int CurrentTime = 0;
int LastUpTimePrint = 0;
int stepsPerIteration = 5;
unsigned long current_millis = millis();
unsigned long down_start_millis = millis();
unsigned long up_start_millis =  millis();
int stepsPerRevolution = 200;
volatile boolean powerStatus = false;
boolean KnobRotationDirection;
boolean goingDown = true;

// Initialize Stepper Library
Encoder myEnc(CLK, DT);
Stepper myStepper(stepsPerRevolution, AIN2, AIN1, BIN1, BIN2);

/*
   @brief: Function to read interrupt switch status
           and turn the servo ON/OFF.
   @Description: Attached to interrupt 0, ie Pin D2,
                 Writes to output to Pin D14.
   Connect the STBY pin of motor driver to Pin14
   The
*/
void powerSwitch() {
  if (!powerStatus) {
    digitalWrite(STBY, HIGH);
          Serial.println("ON");
    powerStatus = true;  // toggle the variable
  }
  else {
    digitalWrite(STBY, LOW);
          Serial.println("OFF");
    powerStatus = false;  // toggle the variable
  }
}

//void speedControl {
//
//}
void setup() {
//  pinMode(CLK, INPUT);
//  pinMode(DT, INPUT);
//  lastState = digitalRead(CLK);
  lastState = myEnc.read();
  attachInterrupt(1, powerSwitch, RISING); // to SW, for power switch
  //   attachInterrupt(0,speedControl,CHANGE);
  Serial.begin(9600);
  myStepper.setSpeed(motorSpeed);
  Serial.println("Starting Program");
  Serial.println("Power OFF");
  Serial.print("Number of Cycles Programmed for: ");
  Serial.println(numCycles);
  Serial.print("Inside Furnace Time(s): ");
  Serial.println(downStopTime/1000);
  Serial.print("Time Gap Between two cycles(s): ");
  Serial.println(upStopTime/1000);
  Serial.println("Push Button to Start");
//  unsigned long current_millis = millis();
}

void loop()
{
//  stepCount = 0;
  if(powerStatus)
  {
    int i=0; 
    while (i<numCycles) {
//   for( int i=0; i<numCycles; ++i) {

      myStepper.setSpeed(motorSpeed);
      if (minSteps < (stepCount + 1) and (stepCount + 1) <= maxSteps and goingDown == true)
      {
        if (i > 0) {
          if ((millis() - up_start_millis) > upStopTime) {
            if (LastCyclePrint != i+1) {
            Serial.print("\nStarted Cycle No.: ");
            Serial.print(i+1);
            Serial.print("/");
            Serial.println(numCycles);
                  //      Serial.println("Moving 5 steps");
            LastCyclePrint = i+1;
            }
            myStepper.step(stepsPerIteration);
            stepCount = stepCount + stepsPerIteration;
            if (stepCount >= maxSteps) {
              goingDown = false;
              down_start_millis = millis();
            }
          }
          else {
            if ((millis() - up_start_millis) > 1000) {
                CurrentTime = (millis() - up_start_millis)/1000;
                if ( LastUpTimePrint != CurrentTime) {
                Serial.print("Up Waiting Time.: ");
                Serial.print((millis() - up_start_millis)/1000);
                Serial.print("/");
                Serial.print(upStopTime/1000);
                Serial.println(" seconds");
                
                LastUpTimePrint = (millis() - up_start_millis)/1000;
                }
            }
          }
        }
        else {  
        //      Serial.println("Moving 5 steps");
        if (LastCyclePrint != i+1) {
        Serial.print("Started Cycle No.: ");
        Serial.print(i+1);
        Serial.print("/");
        Serial.println(numCycles);
        LastCyclePrint = i+1;
        }
        myStepper.step(stepsPerIteration);
        stepCount = stepCount + stepsPerIteration;
        if (stepCount >= maxSteps) {
          goingDown = false;
          down_start_millis = millis();
        }
        }
      }
      
      else if (minSteps <= (stepCount - 1) and (stepCount - 1) <= maxSteps and goingDown == false and (millis() - down_start_millis) > downStopTime) 
      {
        //      Serial.println("Moving Up 5 steps");
        myStepper.step(-stepsPerIteration);
        stepCount = stepCount - stepsPerIteration;
        if (stepCount == minSteps) {
          Serial.print("Cycle: ");
          Serial.print(i+1);
          Serial.println(" Finished\n");
          goingDown = true;
          i = i+1;
          up_start_millis = millis();
        }
      }
      //    Serial.print("CurrentSteps: ");
      //    Serial.println(stepCount);
      if ( i == numCycles-1) {
        powerStatus = false;
//        Serial.println("Finished All Cycles");
//        start_millis = 0;
      }
  }
  }
  else {
//    Serial.println("Power Off");
  }

  cstate = myEnc.read();
//  Serial.print("CurrentState");
//  Serial.println(cstate);
  newSpeed = map(cstate,-999,999,0,30);   // Modify this range to get smoother velocity change
  if (cstate != lastState) {
    lastState = cstate;
    newSpeed = motorSpeed + cstate;   
    Serial.print("New Speed: ");
    Serial.println(newSpeed); 

      if (newSpeed <= maxspeed and minspeed <= newSpeed) {
        motorSpeed  = newSpeed;
        Serial.print("CurrentSpeed: ");
        Serial.println(motorSpeed);
      }

//    motorSpeed = map(motorSpeed, 0, 30, 0, 30);
    }
}
