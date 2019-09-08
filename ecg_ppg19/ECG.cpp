/*
  Kubov V.I. 2019
  ECG. AD8232 R9=1Mohm K=1100 pin: Output - A0
*/

#define _ECGscale 2.93 // 3000/1024
#define _ECGoffset 512 // ADC

#include <Arduino.h>

int16_t getECGraw(void){
  return (analogRead(0)-_ECGoffset)*_ECGscale;
}//getECGraw

#define _ringSize 268
int16_t ringDelay[_ringSize];
int iC=0;
int16_t getECG(void){
  ringDelay[iC]=(analogRead(0)-_ECGoffset)*_ECGscale;
  iC++; if (iC>=_ringSize) iC=0;
  return ringDelay[iC];  
}//getECG

void clearECG(void){
  for(int i=0; i<_ringSize; i++)ringDelay[i]=0; 
}//clearECG

