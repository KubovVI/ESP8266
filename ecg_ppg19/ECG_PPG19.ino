/*
  Kubov V.I. 2019
  ESP8266
  PPG. Max30102. i2c default pins: 4-SDA-D2; 5-SCL-D1
  ECG. AD8232 R9=1Mohm K=1100 pin: Output - A0
*/
#define _ssid "EspECGPPG02"
#define _pass "123456789" // if need
#define _login "admin"
#define _dTime 5 //ms min 3

#define _ringSize 20

#include "PPG.h"
#include "ECG.h"
#include "HTMLcode.h"

int16_t Datas[2][_dataSize+1];
int16_t Ring[_ringSize][_dataSize];
int rBank, wBank, wIndex;
int ECGwIndex, PPGwIndex, PPGdIndex;
int ringIndex;
bool bankReady;
int iTest;
int adminNum=0; //first Web-socket client.
//int adminNum=-1; //nobody exclude admin.
long AutoSaveMin=0;
uint8_t LedRed=0x1F;
uint8_t LedIR=0x1F;
int32_t RawRed,RawIR;

// ESP-8266MOD D1 mini
#define _RfLed 2 // On-LOW, Off-HIGH 

#include <FS.h>
File file;
String fileName="C0001.txt";

#include <Ticker.h>
Ticker myTicker;
bool Cardio=true;
bool enablePPG=true;
bool PPGon=true;
bool fSave=false; 
bool wsConnected=false;
int pageCounter=0;

void Sampling(){
  if (!wsConnected) return;
  int16_t ecgV,ppgRed,ppgIR;
  // ---------- ECG ------------
  ecgV=getECG();
  if(!Cardio) ecgV=millis()%3000-1500; //Test
  Datas[wBank][ECGwIndex]=ecgV; 
  ECGwIndex++;
  // ---------- PPG ------------
  if (PPGdIndex==0){ //PPG
    if (enablePPG) getPPG(&ppgRed,&ppgIR);
    else {ppgRed=0; ppgIR=0;}
    //ppgRed=ecg>>1; ppgIR=-ecg; //Test
    if(!Cardio){ ppgRed=ecgV>>1; ppgIR=-ecgV; } //Test  
    Datas[wBank][PPGwIndex+_ecgSize]=ppgRed;
    Datas[wBank][PPGwIndex+1+_ecgSize]=ppgIR;
    PPGwIndex+=2; if (PPGwIndex>=_ppgSize) PPGwIndex=0;    
  }//if PPGdIindex
  PPGdIndex++; if (PPGdIndex>=_PPGdecimation)PPGdIndex=0; 
  // Buffer filling control
  if (ECGwIndex>=_ecgSize){ //switch data buffer
    ECGwIndex=0; rBank=wBank; wBank=1-wBank; //wBank=0..1
    PPGwIndex=0; PPGdIndex=0;
    bankReady=true;
    Serial.print(pageCounter,DEC); //Test
    pageCounter++; if (pageCounter>=_ringSize) pageCounter=0; //Test
  }//if wIndex
}//Sampling

#include <ESP8266WiFi.h>
// WebSocketsServer needs Hash
#include <WebSocketsServer.h>
#include <Hash.h>
//This needs to prevent Error on WStype_t type
void wsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
WebSocketsServer ws(81);
String serverIP;
IPAddress clientIP; //maybe Array?
bool onClient[WEBSOCKETS_SERVER_CLIENT_MAX];

void wsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  int clients;
  Serial.print("Event on "); Serial.print(num,DEC); Serial.print(":");
  switch(type) {
     case WStype_ERROR:
         Serial.println("Error!!!");
         break;          
    case WStype_DISCONNECTED: 
         onClient[num]=false;  
         Serial.println("Disconnected!");
         onClient[num]=false;  
         break;
    case WStype_CONNECTED: wsConnected=true; clientIP= ws.remoteIP(num);
         onClient[num]=true;  
         Serial.print(" Connected to ");
         for (int i=0; i<4; i++){ Serial.print(clientIP[i],DEC); Serial.print("."); } 
         Serial.println(); 
         break;
   case WStype_TEXT:
         String s=""; for (int i=0; i<lenght; i++) s=s+char(payload[i]);
         Serial.print(num); Serial.print(":"); Serial.println(s);        
         if (s==_login) {Serial.println("Login"); adminNum=num; }
         if (num==adminNum){ 
           if (s=="Save") {Serial.println("Save"); 
             long cTime=millis();
             saveFile();
             Serial.println(millis()-cTime, DEC); 
           }// if save
         }// if adminNum
         break;
  }//switch
}//

void handle_root() {
  server.send(200,"text/html",formHTML());
  Serial.println("handle_root");
}//handle_root

void handle_test() {
  Serial.println("handle_test");
  if (server.args()>0){
    if (server.arg("Reset")=="Reset"){ Serial.println("Reset");
      ESP.reset(); //Reset
    }//if Reset
    if (server.arg("mode")=="Timer") { Serial.println("Timer");
      Cardio=false;
    }//if Timer
    if (server.arg("mode")=="Cardio") { Serial.println("Cardio");
      Cardio=true;  
    }//if Cardio
    if (server.arg("ppg")=="On") { Serial.println("PPGon");
      PPGon=true; powerPPG(PPGon); 
    }//if PPGon
    if (server.arg("ppg")=="Off") { Serial.println("PPGoff");
      PPGon=false; powerPPG(PPGon);  
    }//if PPGoff
    AutoSaveMin=server.arg("AutoSave").toInt();
    Serial.print("AutoSave="); Serial.println(AutoSaveMin,DEC); 
    //server.send(200,"text/html",formHTML());// old URI
    server.sendHeader("Location","/"); server.send(303); //redirect to root
  } else server.send(200,"text/html",testHTML());
}//handle_test

void handle_ppg(){
  Serial.println("handle_ppg");
  if (server.args()>0){
    LedRed=server.arg("Red").toInt();
    LedIR=server.arg("IR").toInt();
    setLeds(LedRed,LedIR);
    delay(100);
  }//if
  getPPGraw(&RawRed, &RawIR);  
  server.send(200,"text/html",ppgHTML()); 
}//handle_ppg

void handle_file(){
  Serial.println("handle_file");
  file=SPIFFS.open("/"+fileName,"r");
  Serial.println("File open");
  server.streamFile(file, "application/octet-stream");
  file.close();
  Serial.println("File close");
}//handle_file

void handle_notRoot(){
  String filePath=server.uri();
  Serial.println("handle_notFound:"+filePath);
  if (filePath.endsWith(".Log")) handle_file();
  else server.send(404,"text/plain","404: Not Found");
}//handle_notrRoot

void setup(void) {
  Serial.begin(115200);
  Serial.println("");
  Serial.print(ESP.getCpuFreqMHz(),DEC); Serial.println("MHz");  
  SPIFFS.begin();
  //SPIFFS.format(); //Only First Time
  
  pinMode(_RfLed,OUTPUT); digitalWrite(_RfLed,HIGH);

  //This needs only at first time to disable Station mode
  //WiFi.mode(WIFI_STA); WiFi.disconnect(); //Disable Station Mode
  delay(100);
  WiFi.softAP(_ssid,_pass); // set password if needs
  //WiFi.softAP(_ssid);
  serverIP=WiFi.softAPIP().toString();
  Serial.println(serverIP);

  for (uint8_t num=0; num<WEBSOCKETS_SERVER_CLIENT_MAX; num++) 
    onClient[num]=false;
  ws.begin();
  ws.onEvent(wsEvent);
 
  server.on("/", handle_root);
  server.on("/test", handle_test);
  server.on("/ppg", handle_ppg);
  server.onNotFound(handle_notRoot);
 
  server.begin();
  Serial.println("HTTP server started");

  wBank=0; wIndex=0; bankReady=false;
  ECGwIndex=0; PPGwIndex=0; PPGdIndex=0;
  ringIndex=0;

  clearECG();
  
  enablePPG=setupPPG(); // dTmin=16+2ms
  Serial.print("PPG ");
  if (enablePPG) Serial.println("Enable");
  else Serial.println("Disable");
  
  // _dTime>=dTmin/_PPGdecimation
  myTicker.attach_ms(_dTime, Sampling);
}//setup

#define _dTmax 500
 
void loop(void) {
  ws.loop();
  server.handleClient();

  if (bankReady){
    bankReady=false;
    Datas[rBank][_dataSize]=ringIndex; 
    digitalWrite(_RfLed,LOW);
    wsBroadcast();
    digitalWrite(_RfLed,HIGH);
    if (!fSave) {
      for (int i=0; i<_dataSize; i++) Ring[ringIndex][i]=Datas[rBank][i];
      Serial.print(ringIndex,DEC); Serial.print(" "); //Test
      ringIndex++; 
      if (ringIndex>=_ringSize) { ringIndex=0;
        Serial.println();
      }//if ringIndex
    }//if Save  
    //Serial.print(rBank,DEC); Serial.print(":0="); Serial.println(Datas[rBank][0],DEC); //Test
  }//if bankReady  
}//loop

void wsBroadcast(){
  for (uint8_t num=0; num<WEBSOCKETS_SERVER_CLIENT_MAX; num++){
    if (onClient[num]){
      Serial.print("."); // Test
      if (!ws.sendBIN(num,(uint8_t*)&Datas[rBank][0],_dataSize*2+2)){
        ws.disconnect(num);
        onClient[num]=false;
        Serial.print("Error on "); Serial.println(num,DEC);
     }//if  ws.send
    }//if onClient 
  }//for num
  if (ws.connectedClients()<1) wsConnected=false;
}//wsBroadcast

void saveFile(){
  if (fSave) return;
  fSave=true; 
  file=SPIFFS.open("/"+fileName,"w");
  Serial.println("File open");
  // print ECG
  int ringIndexP=ringIndex;
  file.print("*\t"); file.println(_ecgSize,DEC);
  for (int i=0; i<_ringSize; i++){
    for (int j=0; j<_ecgSize;j++) {
       file.println(Ring[ringIndexP][j],DEC);
     }//for j
    Serial.print("ECG:"); Serial.print(ringIndexP,DEC); Serial.print(" ");
    ringIndexP++; if (ringIndexP>=_ringSize) ringIndexP=0;
  }//for i
  // print PPG
  ringIndexP=ringIndex;
  file.print("**\t"); file.println(_ppgMax,DEC);
  for (int i=0; i<_ringSize; i++){
    for (int j=_ecgSize; j<_dataSize; j+=2) {
       file.print(Ring[ringIndexP][j],DEC); file.print("\t");
       file.println(Ring[ringIndexP][j+1],DEC);       
     }//for j
    Serial.print("PPG:"); Serial.print(ringIndexP,DEC); Serial.print(" ");
    ringIndexP++; if (ringIndexP>=_ringSize) ringIndexP=0;
  }//for i
  Serial.println();
  file.close();
  Serial.println("File close");
  fSave=false;
}//saveFile

