/*
  Kubov V.I. 2019
*/

#define _ledBrightness 0x1F //0=Off to 255=50mA by 0.2mA
#define _sampleAverage 16 //1,2,4,8,16,32
#define _ledMode 2 //1=Red, 2=Red+IR, 3=Red+IR+Green
#define _sampleRate 1000 //50,100,200,400,800,1000,1600,3200
#define _pulseWidth 118 //69,118,215,411
#define _adcRange 4096 //2048,4096,8192,16384
// default: powerLevel=0x1F,sampleAverage=4,ledMode=3,sampleRate=400,pulseWidth=411,adcRange=4096

#include "MAX30105.h"
MAX30105 Sensor;

int32_t dat[2]; // 0-Red, 1-IR, 
int32_t nDiff[2];

void readSensor(){
  Sensor.check();
  dat[0]=Sensor.getFIFORed(); 
  dat[1]=Sensor.getFIFOIR();
  while (Sensor.available()) Sensor.nextSample();
}//readSensor

//----------------------------  Low Pass Filter -----------------------------
#define _ringSizeLF 4
#define _divShiftLF 2 // 2**2=4
int32_t ringLF[2][_ringSizeLF];
int32_t sumLF[2]; //static 
int indexLF=0; //static

void FilterLF() {
  for (int j=0; j<2; j++){
    sumLF[j]-=(ringLF[j][indexLF]>>_divShiftLF); 
    ringLF[j][indexLF]=dat[j]; 
    sumLF[j]+=(ringLF[j][indexLF]>>_divShiftLF);
    dat[j]=sumLF[j]; // avg=sum/2**_divShift
  }//for j
  indexLF++; if (indexLF>=_ringSizeLF) indexLF=0;
}//FilterLF

//----------------------------  Hight Pass Filter -----------------------------
#define _ringSizeHF 128
#define _divShiftHF 7 // 2**7=128
#define _offsetHF 64 // =128/2
#define _scaleDiff 100000L
int32_t ringHF[2][_ringSizeHF];
int32_t sumHF[2]; //static 
int indexHF=0; //static

void FilterHF() {
  int iC;
  int32_t diff, sum;
  iC=indexHF+_offsetHF; if (iC>=_ringSizeHF) iC-=_ringSizeHF;
  for (int j=0; j<2; j++){
    sumHF[j]-=(ringHF[j][indexHF]>>_divShiftHF); 
    ringHF[j][indexHF]=dat[j]; 
    sumHF[j]+=(ringHF[j][indexHF]>>_divShiftHF);
    diff=sumHF[j]-ringHF[j][iC]; //inversion
    sum=sumHF[j]; if (sum==0) sum=1;
    nDiff[j]=_scaleDiff*diff/sum; //hang Up when zero devision
  }//for j
  indexHF++; if (indexHF>=_ringSizeHF) indexHF=0;
}//FilterHF

void getPPG(int16_t *Red, int16_t *IR){
  readSensor();
  FilterLF();
  FilterHF();
  *Red=nDiff[0]; *IR=nDiff[1];
}//getPPG

char setupPPG(void){
  if (!Sensor.begin(Wire,I2C_SPEED_FAST)) return 0; // 400KHz no sensor Error
  //if (!Sensor.begin()) return 0; // 100KHz no sensor Error
  Sensor.setup(_ledBrightness,_sampleAverage,_ledMode,_sampleRate,_pulseWidth,_adcRange);   
  delay(20);
  readSensor(); //dummy read for sync
  return 1;
}//setupPPG

void getPPGraw(int32_t *Red, int32_t *IR){
  readSensor();
  *Red=dat[0]; *IR=dat[1];    
}//getPPGraw

void powerPPG(char power) { //Sensor LEDs On/Off
  if (power) Sensor.wakeUp();
  else Sensor.shutDown(); 
}//powerPPG

void setLeds(uint8_t Red, uint8_t IR){
   Sensor.setPulseAmplitudeRed(Red);
   Sensor.setPulseAmplitudeIR(IR);
}//setLeds

