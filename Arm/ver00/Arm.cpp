#include "Arm.h"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver sPWM = Adafruit_PWMServoDriver();

#include <Ticker.h>
Ticker myTicker;
#define _Tick 10 //ms

extern int16_t HoldReg[];

#define _servoNum 6
int initialPos[_servoNum]={285,320,200,150,300,300};
int minPos[_servoNum]={80,140,80,120,80,220};
int maxPos[_servoNum]={520,490,510,380,520,380};
int stepPos[_servoNum]={1,1,1,1,10,200};
int currentPos[_servoNum];
int targetPos[_servoNum];
int movDir[_servoNum];
boolean servoDone[_servoNum];
boolean tickerRun=false;
boolean servoLimited=true;
boolean servoSmooth=true;

void servoSet(int servo, int pos, boolean limit){ //direct move to target position
  tickerRun=false; //transaction protection
  if (limit){
    if (pos<minPos[servo]) pos=minPos[servo];
    if (pos>maxPos[servo]) pos=maxPos[servo];
  }//if limit  
  currentPos[servo]=pos; targetPos[servo]=pos;
  movDir[servo]=0;
  sPWM.setPWM(servo,0,pos);
  servoDone[servo]=true;
  tickerRun=true;
}//servoSet

void servoMoveTo(int servo, int pos){ // smooth move to target position
  tickerRun=false; //transaction protection
  if (pos<minPos[servo]) pos=minPos[servo];
  if (pos>maxPos[servo]) pos=maxPos[servo];
  targetPos[servo]=pos;
  int dPos=pos-currentPos[servo];
  if (dPos>0) movDir[servo]=1;
  else if (dPos<0) movDir[servo]=-1;
  if (dPos==0){
    movDir[servo]=0;
    servoDone[servo]=true;
  } else servoDone[servo]=false;   
   tickerRun=true;
}//servoMoveTo

void servoProcessing(int servo){
  if (servoDone[servo]) return;
  if (currentPos[servo]==targetPos[servo]){
    servoDone[servo]=true;
    return;
  }//if
  tickerRun=false; //transaction protection
  int pos=currentPos[servo];
  if (movDir[servo]>0){ 
    pos=pos+stepPos[servo];
    if (pos>targetPos[servo]) pos=targetPos[servo];
  } else {
    pos=pos-stepPos[servo];
    if (pos<targetPos[servo]) pos=targetPos[servo];    
  }//if else
  currentPos[servo]=pos;
  tickerRun=true;
  //Serial.print(servo); Serial.print("="); Serial.print(pos); Serial.print(" ");  
  sPWM.setPWM(servo,0,pos);
}//servoProcessing

void servosProcessing(){
  if (!tickerRun) return; //transaction protection
  for (int i=0; i<_servoNum; i++){
    if (!servoDone[i]) servoProcessing(i);
  }//for i
}//servosProcessing 

void armBegin(){
  sPWM.begin();
  sPWM.setOscillatorFrequency(27360000); //25e6 default 27328-50.3 27378-49.9
  sPWM.setPWMFreq(50); //Analog servos ~50 Hz -> 20ms min=1ms max=2ms
  // 20ms=4024; 1ms=201; 2ms=402; 1.5ms=302
  // PWM range 300+/-120 = 80...420
  for (int i=0; i<_servoNum; i++){
    HoldReg[i+1]=initialPos[i];
    servoSet(i,initialPos[i],servoLimited);
  }//for i
  myTicker.attach_ms(_Tick, servosProcessing);  
}//armBegin

void armProcessing(){
 for (int i=0; i<_servoNum; i++){
   if (servoSmooth) servoMoveTo(i,HoldReg[i+1]);
   else servoSet(i,HoldReg[i+1],servoLimited);
 }//for i
}//armProcessing 


