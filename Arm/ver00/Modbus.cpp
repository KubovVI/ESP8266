#define _cMax 100
#include "Modbus.h"


byte ByteArray[BufferSize];
int16_t  HoldReg[maxHoldReg];

#include <ESP8266WiFi.h>
extern WiFiServer MBserver;
extern WiFiClient MBclient;

int cCount;

void MBserverBegin(){ //not necessary
  MBserver.begin();
  while (!MBclient){ 
    MBclient=MBserver.available();
    delay(100); Serial.print("-");
    cCount++; if (cCount>_cMax){Serial.println(); cCount=0;}
  }//while
  Serial.println();
  Serial.println("Client available");   
}//MBserverBegin

boolean MBtransaction(){
  int Start,WordDataLength,ByteDataLength,MessageLength;
  byte byteFN=MB_FC_NONE;
  if (!MBclient.connected()){ // reconnect
    MBclient=MBserver.available();
    delay(100); Serial.print("+");
    cCount++; if (cCount>_cMax){Serial.println(); cCount=0;}
  }//if  
  if (!MBclient) return false;
  if (!MBclient.available()) return false;
 
  // Read data stream
  int i=0;
  while(MBclient.available()&&(i<BufferSize)){
    ByteArray[i]=MBclient.read(); i++;
  }//while
  MBclient.flush(); // clear stream

   // Modbus TCP processing
   byteFN = ByteArray[MB_TCP_FUNC];
   Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
   WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
   if (byteFN==MB_FC_WRITE_REGISTER) WordDataLength=1;
   
#ifdef MBdebug   
   Serial.print("F="); Serial.print(byteFN,DEC); Serial.print(":");
   Serial.print("S="); Serial.print(Start,DEC); Serial.print(";");
   Serial.print("L="); Serial.print(WordDataLength,DEC); Serial.print(". ");
#endif
   // ---------------------  Handle request -----------------------------------
   switch(byteFN){
      case MB_FC_NONE: 
        break;
        
      case MB_FC_READ_REGISTERS: // 03 Read Holding Registers
        ByteDataLength=WordDataLength*2;
        ByteArray[5]=ByteDataLength+3; //Number of bytes after this one.
        ByteArray[8]=ByteDataLength; //Number of bytes after this one (or number of bytes of data).
        for(int i=0; i<WordDataLength; i++){
          ByteArray[9+i*2]=highByte(HoldReg[Start+i]); ByteArray[10+i*2]=lowByte(HoldReg[Start+i]);
        }//for
        MessageLength=ByteDataLength+9;
        MBclient.write((const uint8_t *)ByteArray,MessageLength);
        byteFN=MB_FC_NONE;
        break;
        
      case MB_FC_WRITE_REGISTER: // 06 Write Holding Register
        HoldReg[Start]=word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
        ByteArray[5]=6; //Number of bytes after this one.
        MessageLength=12;
        MBclient.write((const uint8_t *)ByteArray,MessageLength);
        byteFN=MB_FC_NONE;
        break;
        
      case MB_FC_WRITE_MULTIPLE_REGISTERS: //16 Write Holding Registers
        ByteDataLength=WordDataLength*2;
        ByteArray[5]=6; //Number of bytes after this one.
        for(int i=0; i<WordDataLength; i++){
          HoldReg[Start+i]=word(ByteArray[13+i*2],ByteArray[14+i*2]);
        }//for
        MessageLength=12;
        MBclient.write((const uint8_t *)ByteArray,MessageLength); 
        byteFN=MB_FC_NONE;
        break;
    }//switch
    
 #ifdef MBdebug   
    // Print Holding Registers  
    for (int i=Start; i<Start+WordDataLength; i++){
      Serial.print("HR["); Serial.print(i); Serial.print("]="); 
      Serial.print(HoldReg[i]); Serial.print(";");
    }//for
    Serial.println("");
#endif
    return true;
}//MBtransaction  
