 /*
   Robot Arm 2020
   Modbus TCP/IP by WiFi with HTML additional control. 
   Needs 1MB SPIFFS for save configuration
   I2C PWM board:
     I2C-default pins: SCL-D1; SDA-D2.
     Vcc=3.3V from ESP8266
     V+=3.7-4.2V from Li-ion Accumulator (3.7V is too low for servo)
   ESP power(+5V) from V+ I2C PWM board   
 */

#include "WiFiConfig.h"
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

#include "Modbus.h"
WiFiServer MBserver(_ModbusTCP_port);
WiFiClient MBclient = MBserver.available();
// Holding Registers 16bit. Total 20 registers in Modbus.h 
extern int16_t HoldReg[];

#include "Arm.h"
extern boolean servoLimited;
extern boolean servoSmooth;

#include "Web.h"

extern long ReqInterval_ms; 

void server_root() {
  server.send(200,"text/html",serverHTML());
  Serial.println("server_root");
  Blink(3);  
}//server_root

boolean webData=false;

void server_act() {
  actionHTMLdecode();
  Blink(3);  
  webData=true;      
}//server_act

void setup(){
  pinMode(_RfLed,OUTPUT); setLed(0);
  Serial.begin(115200);
  Serial.println();
  
  setupWiFi();
  while (WiFi.status() != WL_CONNECTED){
    delay(500); Serial.print("."); 
  }//while
  server.on("/", server_root);
  server.on("/act", server_act);
  server.begin();
  Serial.print("Web-server:");
  Serial.print(WiFi.localIP()); 
  Serial.println(":80");
  Serial.println("Start");
  
  Serial.println(); Serial.print("Modbus TCP/IP:");
  Serial.print(WiFi.localIP()); 
  Serial.print(":");  Serial.println(_ModbusTCP_port,DEC);
  Serial.println("Start");
  MBserver.begin();
  //MBserverBegin();  
  
  armBegin();
  
}//setup

boolean newData=false; 
 
void loop(){
  // Write Holding Registers. Read from ESP 
  HoldReg[0]=millis()/1000;   
  server.handleClient();        
  newData=MBtransaction();
     
  // Read Holding Registers. Write to ESP
  if (newData){ 
    Serial.print("HR[1-6]=");
    for (int i=1; i<7; i++) {
      Serial.print(HoldReg[i]); Serial.print(";");
    }//for
    Serial.println();
  }// if newData
  
  if (newData || webData){ //Arm processing
    armProcessing();
    webData=false;
  }//if newData
}//loop
