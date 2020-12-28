#include "WiFiConfig.h"

#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
extern ESP8266WebServer server;

#define _BufLen 20
char Buffer[_BufLen];
String newMode=_newMode;
String newSSID=_newSSID;
String newPASS=_newPASS;
String newIP=_newIP;
String newGate=_newGate;
String newMask=_newMask;
String newReqInt=String(_ReqInterval);
String operMode, operSSID, operPASS, operIP, operGate, operMask, operReqInt;
String strMAC;
long ReqInterval_ms; 

bool restoreConfiguration();
void setupConfiguration();

void setupWiFi(){ 
  Serial.println("Wait for setup");
  Serial.println("Set D3(Flash-pin) to Ground");
  
  boolean WiFiSetup=false;
  // wait on D3(Flash-pin) to Ground
  for (int i=0; i<100; i++){ //100ms*100=10s
    if (digitalRead(_Button)==LOW){
      WiFiSetup=true; //reconfiguretion
      break;
    }//if  
    Blink(1); //100ms
  }//for
  
  if (!WiFiSetup) { // Operation Mode
    // Try to restore configuration
    if (!restoreConfiguration())
       WiFiSetup=true; // Error on Attempt to restore
  }// if !WiFiSetup

  strMAC=WiFi.macAddress();  

  if (WiFiSetup) { //reconfiguration
    setupConfiguration();
    // wait to reset
      while (millis()<_ConfigTime){
      server.handleClient();         
    }//while
    Serial.println("Reset"); 
    ESP.reset();
  }//if WiFiSetup 
  
}//setupWiFi  

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
<tr><td>Timer,ms:</td><td><input type=text name='RINT' value='"+newReqInt+"'></td></tr>\n\
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
    ReqInterval_ms=operReqInt.toInt();          
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
  return true;
}//restoreConfiguration      

//---------------------------- setupConfiguration -----------------------------
void setupConfiguration(){
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
}//setupConfiguration

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
  if (LedOn) digitalWrite(_RfLed,LOW); 
  else digitalWrite(_RfLed,HIGH); 
}//setLed 

void Blink(int n){
  for (int i=0; i<n; i++){
    setLed(true); delay(50);
    setLed(false); delay(50); 
  }//for
}//Blink 
