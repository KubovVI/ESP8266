//Kubov V.I. 2018 
#define _ssid "Sonoff17"
#define _version "SonoffServer05"
#define _pass "123456789" 
#define _ConfigTime 300000 //5*60*1000
#define _ReqInterval 5 //sec
#define _TempResolution 12

#define _TempPin 14
#define _Relay 12
#define _Led 13
#define _Button 0

#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

#include <OneWire.h>
#include <DallasTemperature.h>
#define _maxDev 16

OneWire oneWire(_TempPin);
DallasTemperature sensors(&oneWire);
DeviceAddress Address[_maxDev];
float Temp[_maxDev];
int Devices=_maxDev;

#define _BufLen 20

boolean Led, Relay;
boolean WiFiSetup;
char Buffer[_BufLen];
String newMode="Station";
String newSSID="netSSID";
String newPASS="123456789";
String newIP="192.168.4.10";
String newGate="192.168.4.1";
String newMask="255.255.255.0";
String newReqInt=String(_ReqInterval);
String operMode, operSSID, operPASS, operIP, operGate, operMask, operReqInt;
String strMAC;
long ReqInterval_ms; 
//----------------------------- Configuration -------------------------- 
String configHTML(){
  String form=
"\
<html>\n\
<head>\n\
<style>\n\
body,table,button,input,select {font-size:30px; height:40px; text-align:center} \n\
</style>\n\
</head>\n\ 
<body>\n\
<h4 align=center>WiFi Configuration</h4>\n\
<form action='act' align=center>\n\
<table border=3 align=center>\n\
<tr><td>Mode:</td><td><select name='MODE'><option val='Station' selected>Station</option>\n\
<option val='AccessPoint'>AccessPoint</option>\n</select>\n</td></tr>\n\
<tr><td>SSID:</td><td><input type=text name='SSID' value='"+newSSID+"'></td></tr>\n\
<tr><td>Pass:</td><td><input type=text name='PASS' value='"+newPASS+"'></td></tr>\n\
<tr><td>LocalIP:</td><td><input type=text name='ST_IP' value='"+newIP+"'></td></tr>\n\
<tr><td>Gateway:</td><td><input type=text name='GATE' value='"+newGate+"'></td></tr>\n\
<tr><td>SubnetMask:</td><td><input type=text name='MASK' value='"+newMask+"'></td></tr>\n\
<tr><td>ReqInreval:</td><td><input type=text name='RINT' value='"+newReqInt+"'></td></tr>\n\
<tr><td colspan=2 align=center><input type=submit value='Submit'></td></tr>\n\
</table>\n\
</form>\n\
<p align=center>MAC: "+strMAC+"<br>"+_version+"</p>\n\
</body>\n\
</html>\
";
  return (form);
}//configHTML

void config_root() {
  server.send(200,"text/html",configHTML());
  Serial.println("config_root");
  Blink(3);  
}//config_root

void config_act() {
  newMode=server.arg("MODE");
  newSSID=server.arg("SSID");
  newPASS=server.arg("PASS");
  newIP=server.arg("ST_IP");
  newGate=server.arg("GATE");
  newMask=server.arg("MASK"); 
  newReqInt=server.arg("RINT");
  server.send(200,"text/html",configHTML());
  Serial.println("config_act");
  SPIFFS.begin();
  File file=SPIFFS.open("/init.txt", "w");
  if (!file) {
    SPIFFS.format(); //First time only
    Serial.println("fomat SPIFFS");
    file=SPIFFS.open("/init.txt", "w");
   }// 
   if (file) {    
    Serial.println("write file open");
    file.print(newMode); file.print(char(0));
    file.print(newSSID); file.print(char(0));
    file.print(newPASS); file.print(char(0));
    file.print(newIP);   file.print(char(0));
    file.print(newGate); file.print(char(0));
    file.print(newMask); file.print(char(0));
    file.print(newReqInt); file.print(char(0));
    file.close();
    Serial.println("file close");
  } else {  
    Serial.println("file open Error");
  }// if file
  SPIFFS.end();
  Blink(5);  
}//config_act

//----------------------------- Operation  -------------------------- 
String operHTML(){
  getSensTemp(); //DS1820
  String vRelay,sRelay; 
  if (Relay) {sRelay="On"; vRelay="Off";
  }else {sRelay="Off"; vRelay="On";}
  String form ="<html>\n<head>\n<style>\n\
  body,table,button,input {font-size:30px; text-align:center} \n\
  </style>\n</head>\n<body>\n\  
  <table border=3 cellpadding=10 align=center>\n\
  <tr><td>SensorID</td><td>Temp,C</td></tr>\n";
  for (int i=0; i<Devices; i++){
    form=form+"<tr><td>"+stringAddress(Address[i])+"</td><td>";
    form=form+String(Temp[i],2)+"</td></tr>\n";
  }//for i
  form=form+"</table>\n\
  <form action='ctl' align=center>\n\
  Relay: state is "+sRelay+"; set to <button name='Relay' value='"+vRelay+"'>"+vRelay+"</button>\n\
  </form>\n\
  <p align=center>MAC: "+strMAC+"</p>\n\
  </body>\n</html>";
  return (form);
}//operHTML

String dataHTML(){
  getSensTemp(); //DS1820
  String form ="";
  for (int i=0; i<Devices; i++){
    form=form+stringAddress(Address[i])+";"+String(Temp[i],2)+";";
  }//for i
  return (form);
}//dataHTML

void oper_root() {
  server.send(200,"text/html",operHTML());
  Serial.println("oper_root");
  Blink(2);
}//oper_root

void handle_ctl() {
  if (server.arg("Relay")=="On") Relay=true;
  if (server.arg("Relay")=="Off") Relay=false;
  setRelay(Relay);
  server.send(200,"text/html",operHTML());
  Serial.println("handle_ctl");
  Blink(5);
}//handle_ctl

void data_root() {
  long t1=millis();  
  server.send(200,"text/html",dataHTML());
  Serial.print("data_root ");
  Serial.print("dt="); Serial.println(millis()-t1,DEC);
  Blink(1);
}//data_root

//------------------------------- restoreConfiguration -----------------------------------
bool restoreConfiguration(){
  Serial.println("Restore Configuration");
  SPIFFS.begin();
  File file=SPIFFS.open("/init.txt", "r");
  if (file){
    Serial.println("read file");
    operMode=file.readStringUntil(char(0)); Serial.println(operMode);     
    operSSID=file.readStringUntil(char(0)); Serial.println(operSSID);     
    operPASS=file.readStringUntil(char(0)); Serial.println(operPASS);     
    operIP=  file.readStringUntil(char(0)); Serial.println(operIP);
    operGate=file.readStringUntil(char(0)); Serial.println(operGate);
    operMask=file.readStringUntil(char(0)); Serial.println(operMask);          
    operReqInt=file.readStringUntil(char(0)); Serial.println(operReqInt);
    ReqInterval_ms=operReqInt.toInt()*1000;          
    file.close();
  } else {  
    Serial.println("file open Error");
    return false;
  }// if file
  SPIFFS.end();  
  Serial.println("Operation Mode");  
  char sSSID[_BufLen]; operSSID.toCharArray(sSSID,_BufLen);
  char sPASS[_BufLen]; operPASS.toCharArray(sPASS,_BufLen);
  if (operMode=="Station"){ //Station
    IPAddress wIP, wGate, wMask;
    wIP=StrToIP(operIP);
    wGate=StrToIP(operGate);
    wMask=StrToIP(operMask);
    WiFi.mode(WIFI_AP); WiFi.disconnect(); //Disable AP Mode
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(sSSID,sPASS);
    WiFi.config(wIP,wGate,wMask);
    while (WiFi.status() != WL_CONNECTED) { delay(500);
      Serial.print(".");
    }//while
    Serial.println(); 
    Serial.println(WiFi.localIP());
  }else{ // Access Point
    Serial.println("AccessPoint");
    WiFi.mode(WIFI_STA); WiFi.disconnect(); //Disable Station Mode
    delay(100);
    WiFi.softAP(sSSID,sPASS); 
    Serial.println(WiFi.softAPIP());     
  }//Acess Point
  server.on("/", oper_root);
  server.on("/ctl", handle_ctl);
  server.on("/data", data_root);
  //server.onNotFound(oper_root);
  server.begin();
  Serial.println("HTTP server started");
  return true;
}//restoreConfiguration      

//---------------------------- storeConfiguration -----------------------------
void storeConfiguration(){
  Serial.println("Configuration");  
  Serial.println("Access Point Mode");  
  WiFi.mode(WIFI_STA); WiFi.disconnect(); //Disable Station Mode
  delay(100);
  WiFi.softAP(_ssid,_pass); 
  Serial.println(WiFi.softAPIP());
  server.on("/", config_root);
  server.on("/act", config_act);
  //server.onNotFound(config_root);
  server.begin();
  Serial.println("HTTP server started");
}//storeConfiguration

//-----------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  Serial.println("");

  pinMode(_Led,OUTPUT);   setLed(0);
  pinMode(_Relay,OUTPUT); setRelay(0);

  Serial.println("Wait for setup");
  Serial.println("Set Flash-pin to Ground");
  
  WiFiSetup=false;
  for (int i=0; i<100; i++){ //100ms*100=10s
    if (digitalRead(_Button)==LOW){
      WiFiSetup=true;
      break;
    }//if  
    Blink(1); //100ms
  }//for
  
  if (!WiFiSetup) { // Operation Mode
    // Try to restore configuration
    if (!restoreConfiguration()) 
      WiFiSetup=true; // Error on Attempt to restore
  }// if !WiFiSetup
  
  if (WiFiSetup) storeConfiguration();  
  else getSensAddress(); // Operation Mode
  
  strMAC=WiFi.macAddress();  
  Serial.println("MAC: "+strMAC);
}//setup

//-----------------------------------------------------------
long tNext;

void loop(void) {
  long tC=millis();
  server.handleClient();
  if (WiFiSetup){
    if (tC>_ConfigTime) { Serial.println("Reset");    
      ESP.reset(); }//if Config
  }else{ // Operation mode
     if (ReqInterval_ms>0){
       if (tC>tNext){  
         sensors.requestTemperatures();
         Serial.print("T");
         tNext=millis()+ReqInterval_ms; }//if Req
     }// if ReqInterval     
  }//if WiFiSetup    
}//loop
//-----------------------------------------------------------

IPAddress StrToIP(String strIP){
  uint8_t b[4]; int iP;
  for (int i=0;i<3;i++){ 
    iP=strIP.indexOf("."); 
    b[i]=strIP.substring(0,iP).toInt(); 
    strIP=strIP.substring(iP+1);   
  }//for
  b[3]=strIP.toInt();
  return IPAddress(b);
}//StrToIP

void setLed(boolean LedOn){
  if (LedOn) digitalWrite(_Led,LOW); 
  else digitalWrite(_Led,HIGH); 
}//setLed 

void Blink(int n){
  for (int i=0; i<n; i++){
    setLed(true); delay(50);
    setLed(false); delay(50); 
  }//for
}//Blink 

void setRelay(boolean Relay){
  if (Relay) digitalWrite(_Relay,HIGH); 
  else digitalWrite(_Relay,LOW); 
  Serial.print("Relay="); Serial.println(Relay,DEC);
}//setRelay  

//---------------------- DS1820 -------------------------------
void getSensAddress(){
  sensors.begin();
  sensors.setResolution(_TempResolution);
  Devices=sensors.getDeviceCount();
  Serial.print(Devices,DEC); Serial.println(" devices");
  if (Devices>_maxDev) Devices=_maxDev;
  for (int i=0; i<Devices; i++){
    sensors.getAddress(Address[i], i);
    Serial.print(i,DEC); Serial.print(": ");
    Serial.println(stringAddress(Address[i])); 
  }//for 
  if (sensors.isParasitePowerMode()) Serial.print("Parasite"); else Serial.print("Active");
  Serial.println(" Power Mode"); 
}//getSensAddress

void getSensTemp(){
  //long t1=millis();
  if (ReqInterval_ms<=0) sensors.requestTemperatures(); //else Moved to Loop
  for (int i=0; i<Devices; i++){
    Temp[i]=sensors.getTempC(Address[i]);
    //Serial.print(stringAddress(Address[i])+" "); 
    //Serial.print(String(Temp[i],2)+"; ");
  }//for
  //Serial.print("dt="); Serial.println(millis()-t1,DEC);
}//getSensTemp  

String stringAddress(DeviceAddress deviceAddress){
  String st="";
  for (int i=0; i<8; i++){
    if (deviceAddress[i]<0x10) st=st+"0";
    st=st+String(deviceAddress[i],HEX);
  }//for
  st.toUpperCase();
  return st;
}//stringAddress

