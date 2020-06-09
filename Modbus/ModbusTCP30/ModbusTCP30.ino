 /*
   Kubov V.I. 2020
   Modbus TCP/IP by WiFi with HTML additional control. 
   Needs 1MB SPIFFS for save configuration
 */

#include "WiFiConfig.h"
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

#include "Modbus.h"
WiFiServer MBserver(_ModbusTCP_port);
WiFiClient MBclient = MBserver.available();
// Holding Registers 16bit. Total 20 registers in Modbus.h 
extern int16_t HoldReg[];

extern long ReqInterval_ms; 

String serverHTML(){
  String form="<html>\n<head>\n<style>\n\
  body,table,button,input {font-size:30px; height:40px; text-align:center} \n\
  input[type=text]{width:150px; text-align:right}\n\ 
  </style></head><body>\n\
  <h4 align=center>Holding Registers</h4>\n\
  <form action='act' align=center>\n\
  <table border=3 align=center>\n\
  <tr><td>HR</td><td>Value</td></tr>\n";
  for (int i=0; i<4;i++){
    form=form+"<tr><td>"+String(i)+"</td><td><input type=text name='HR"+String(i);
    form=form+"' value="+String(HoldReg[i])+"></td></tr>\n";
  }//for i
  form=form+"<tr><td colspan=2 align=center><input type=submit value='Submit'></td></tr>\n\
  </table>\n</form>\n</body>\n</html>";
  return (form);
}//serverHTML

void server_root() {
  server.send(200,"text/html",serverHTML());
  Serial.println("server_root");
  Blink(3);  
}//server_root

void server_act() {
  HoldReg[2]=server.arg("HR2").toInt();
  server.send(200,"text/html",serverHTML());
  Serial.println("server_act");
  Blink(3);    
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
}//setup
 
void loop(){
  // Write Holding Registers. Read from to ESP 
  HoldReg[0]=millis()/1000;   
  HoldReg[1]=analogRead(0); delay(10); // delay>2 - Modbus Ok
  //HoldReg[1]=analogRead(0); // Modbus Error ???
  HoldReg[3]=HoldReg[1]-512;
  server.handleClient();        
  MBtransaction();   
  // Read Holding Registers. Write to ESP 
  setLed(HoldReg[2]); 
}//loop
