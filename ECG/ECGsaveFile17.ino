// Kubov V.I. 2018
#define _ssid "EspCardio01"
#define _pass "123456789" // if need
#define _login "admin"
#define _dTime 5 //ms
#define _dataSize 100
#define _segSize 10
#define _ringSize 20

uint16_t Datas[2][_dataSize+1];
uint16_t Ring[_ringSize][_dataSize];
int rBank, wBank, wIndex;
int ringIndex;
bool bankReady;
int iTest;
int adminNum=0; //first Web-socket client.
//int adminNum=-1; //nobody exclude admin.
long AutoSaveMin=0;


// ESP-8266MOD D1 mini
#define _RfLed 2 // On-LOW, Off-HIGH 

#include <FS.h>
File file;
String fileName="C0001.txt";

#include <Ticker.h>
Ticker myTicker;
bool Cardio=true;
bool fSave=false; 
bool wsConnected=false;
void Sampling(){
  if (!wsConnected) return;
  if(Cardio) Datas[wBank][wIndex]=analogRead(0);
  else Datas[wBank][wIndex]=millis() % 1024;
  wIndex++;
  if (wIndex>=_dataSize){
    wIndex=0; rBank=wBank; wBank=1-wBank; //wBank=0..1
    bankReady=true;
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
void wsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    case WStype_DISCONNECTED: 
         Serial.println("Disconnected!");
         break;
    case WStype_CONNECTED: wsConnected=true; clientIP= ws.remoteIP(num);
         Serial.print("Connected to ");
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
/*  Not any effect on Communication break
    case WStype_ERROR: clientIP= ws.remoteIP(num);
         Serial.print("Error on ");
         for (int i=0; i<4; i++){ Serial.print(clientIP[i],DEC); Serial.print("."); } 
         Serial.println();
         break;  
*/         
  }//switch
}//

String formHTML(){
  String dataSize=String(_dataSize);
  String segSize=String(_segSize); 
  String scrAutoSave="";
  if (AutoSaveMin>0){
    scrAutoSave="setInterval(Save,"+String(AutoSaveMin*60)+"000);\n";    
  }//if AutoSave
  String form =""
  "<html><head>\n"
  "<style>\n"
  "body,table,input,select {font-size:24px; height:40px; text-align:center}\n"
  "input[type=text]{height:40px}\n"
  "input[type=button]{width:90px}\n"
  "input[type=password]{width:90px}\n"
  "input[type=submit]{width:90px}\n"
  "input[type=range]{width:360px}\n"
  "select {width:60px}\n"
  ".ww {width:160px}\n"
  ".wn {width:50px}\n"
  "</style>\n"
  "<script>\n"
  "dataSize="+dataSize+"; segSize="+segSize+";\n"
  "Cmin=0; Cmax=1024; //Code ADC\n"
  "Vmax=3.3/2/0.6; Vmin=-Vmax; //in 2.75mV\n"
  "//Ymin=picH; Ymax=0; //pixel\n"
  "dVs=0.5;//mV\n"
  "xScale=1;\n"
  "yScale=0.4;//temp\n"
  "YpSize=dataSize*segSize; var Yp=new Array(YpSize);\n"
  "for (i=0; i<YpSize; i++) Yp[i]=0;\n"
  "Run=true; Cardio=true; fRecord=false;\n"
  "RecCount=0;\n"
  
  "ws=new WebSocket('ws://192.168.4.1:81/');\n"
  "ws.onmessage=function(msg){\n"
    "ws.binaryType='arraybuffer';\n"
    "if (msg.data instanceof ArrayBuffer){\n"
      "Datas=new Uint16Array(msg.data,0,dataSize+1);\n"
      "cnt.value=Datas[dataSize];\n"
      "if (Run){\n"
        "for (i=dataSize; i<YpSize; i++) Yp[i-dataSize]=Yp[i];\n"     
        "for (i=0; i<dataSize; i++) Yp[i+YpSize-dataSize]=Datas[i];\n"
      "};\n"      
      "Draw();\n"
    "};\n"  
  "};\n"
  "function Draw(){ gr.save();\n"
    "gr.clearRect(0,0,picW,picH);\n"
    "Grid();\n"
    "Graph();\n"
    "gr.restore();\n"
  "};\n"
  "function Grid(){dXgr=100; dYgr=100;\n"
    "gr.strokeStyle='#8080ff'; gr.lineWidth=2; gr.strokeRect(1,1,picW-2,picH-2);\n"
    "gr.lineWidth=1;\n"
    "for (i=0; i<segSize; i++) gr.strokeRect(dXgr*i,0,0,picH);\n"  
    "dV=dVs*xScale; iMax=Math.floor(Vmax/dV);\n"
    "for (i=-iMax; i<=iMax; i++) {\n"
      "y=remap(dV*i,Vmin,Vmax,Ymin,Ymax);\n"
      "if(i==0)gr.lineWidth=2; else gr.lineWidth=1;\n"
      "gr.strokeRect(0,y,picW,0);\n"
    "};\n"  
  "};\n"
  "function Graph(){\n"
     "gr.strokeStyle='#000000'; gr.lineWidth=2;\n"  
     "gr.beginPath();\n"
     "for (i=0; i<YpSize; i++) {\n"
       "v=remap(Yp[i],Cmin,Cmax,Vmin,Vmax)*xScale;\n"
       "y=remap(v,Vmin,Vmax,Ymin,Ymax);\n"
       "gr.lineTo(i,y);\n"
     "};\n"  
     "gr.stroke();\n"
  "};\n"
  "function RunStop(){\n"
    "Run=!Run;\n"
    "if (Run) btn.value='Stop'; else btn.value='Run';\n"
  "};\n"
  "function ScaleY(){\n"
    "xScale=rng.value;\n"
    //"alert(xScale);\n"
  "};\n"
  "function remap(x,xMin,xMax,yMin,yMax){\n"
    "return(yMin+(yMax-yMin)*(x-xMin)/(xMax-xMin));\n"
  "};\n"
  "function Login(){\n"
    "ws.send(psw.value);\n"
  "};\n"
  "function DateToStr(){"
    "var Dt=new Date();\n"
    "yy=Dt.getFullYear(); mn=Dt.getMonth()+1; dd=Dt.getDate();\n"
    "var s=''+yy;\n" 
    "if (mn<10) s=s+'0'; s=s+mn; if (dd<10) s=s+'0'; s=s+dd;\n"
    "hh=Dt.getHours(); mm=Dt.getMinutes();\n"
    "if (hh<10) s=s+'0'; s=s+hh; if (mm<10) s=s+'0'; s=s+mm;\n"
    "return s;\n"
  "};\n"
  "function Load(){\n"
    //"frm.action=fnm.value+DateToStr()+sopt.value+'.Log'; frm.submit();\n"
    //"window.open('http://"+serverIP+"/'+fnm.value+DateToStr()+sopt.value+'.Log','_self');\n"
    //"window.open('http://"+serverIP+"/'+fnm.value+DateToStr()+sopt.value+'.Log','_self','download');\n"
    //"window.open('http://"+serverIP+"/'+fnm.value+DateToStr()+sopt.value+'.Log','_blank','download');\n"
    "window.open('http://"+serverIP+"/'+fnm.value+DateToStr()+sopt.value+'.Log','_blank');\n"
  "};\n"
    "function Save(){\n"
    "ws.send('Save');\n"
    //"setTimeout(Load,1000);\n"
    "setTimeout(Load,1250);\n"
  "};\n"
"</script></head>\n"
  "<body>\n"
  "<script> if (!window.WebSocket) alert('Sockets Not Supported');\n"
  +scrAutoSave+"</script>\n"
  "<table align=center border=0>\n"
  "<tr><td colspan=7><canvas id=pic height=402 width=1002></canvas></td></tr>\n"
  "<tr><td><input type=password id=psw value='user' onChange='Login()'></td>\n"
  "<td><input type=text class=ww id=fnm value='Name'></td>\n"
  "<td><select id=sopt>\n"
  "<option value='V'>V</option><option value='LR'>LR</option><option value='RF'>RF</option>\n"
  "<option value='LF'>LF</option><option value=''></option></select></td>\n"
  "<td><input type=button id=rec value='Save' onClick='Save()'></td>\n"
  "<td><input type=text class=wn id=cnt value=0></td>\n"
  "<td><input type=button id=btn value='Stop' onClick='RunStop()'></td>\n"
  "<td>s<input type=range id=rng value=1 min=1 max=5 step=0.1 onChange='ScaleY()'><b>S</b></td></tr>\n"
  "</table>\n"
  "<form hiden id=frm></form>\n"
  "<script>\n"
  "gr=pic.getContext('2d'); picW=pic.width; picH=pic.height;\n"
  "Ymin=picH; Ymax=0;\n"
  "Draw();\n" 
  "</script>\n"
  "</body></html>";
  return (form);
}//formHTML

String testHTML(){
  String sMode; if (Cardio) sMode="Timer"; else sMode="Cardio";
  String sASM=String(AutoSaveMin);
  String form =""
  "<html><head><style>\n"
  "body,table,input {font-size:24px}\n"
  "</style></head>\n"
  "<body>\n"
  "<form action=test id=act><table align=center border=0>\n"
  "<tr><td><input type=submit value='Reset' name=Reset></td></tr>\n"
  "<tr><td>AutoSave(min): <input type=text size=5 style='text-align:right' name=AutoSave value="+sASM+">\n"
  " &nbsp <input type=submit value='Set'></td></tr>\n"
  "<tr><td>Switch to <input type=submit name=mode value='"+sMode+"'></td></tr>\n"
  "</table></form>\n"
  "</body></html>";
  return (form);  
}//testHtml

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
    AutoSaveMin=server.arg("AutoSave").toInt();
    Serial.print("AutoSave="); Serial.println(AutoSaveMin,DEC); 
    //server.send(200,"text/html",formHTML());// old URI
    server.sendHeader("Location","/"); server.send(303); //redirect to root
  } else server.send(200,"text/html",testHTML());
}//handle_test

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
  
  ws.begin();
  ws.onEvent(wsEvent);
 
  server.on("/", handle_root);
  server.on("/test", handle_test);
  server.onNotFound(handle_notRoot);
 
  server.begin();
  Serial.println("HTTP server started");

  wBank=0; wIndex=0; bankReady=false;
  ringIndex=0;
  myTicker.attach_ms(_dTime, Sampling);
  
}//setup

void loop(void) {
  ws.loop();
  server.handleClient();
  if (bankReady){
    bankReady=false;
    Datas[rBank][_dataSize]=ringIndex; 
    digitalWrite(_RfLed,LOW);
    ws.broadcastBIN((uint8_t*)&Datas[rBank][0],_dataSize*2+2); 
    digitalWrite(_RfLed,HIGH);
    if (!fSave) {
      for (int i=0; i<_dataSize; i++) Ring[ringIndex][i]=Datas[rBank][i];
      Serial.print(ringIndex,DEC); Serial.print("."); //Test
      ringIndex++; if (ringIndex>=_ringSize) ringIndex=0;
    }//if Save  
    Serial.print(rBank,DEC); Serial.print(":0="); Serial.println(Datas[rBank][0],DEC); //Test
  }//if bankReady  
}//loop

void saveFile(){
  if (fSave) return;
  fSave=true; 
  file=SPIFFS.open("/"+fileName,"w");
  Serial.println("File open");
  int t=0;
  for (int i=0; i<_ringSize; i++){
    //file.write((uint8_t*)&Ring[ringIndex][0],_dataSize*2);
    for (int j=0; j<_dataSize;j++) {
      file.print(t,DEC); file.print("\t");
      file.println(Ring[ringIndex][j],DEC);
      t++;
    }//for j
    Serial.print(ringIndex,DEC); Serial.print(" ");
    ringIndex++; if (ringIndex>=_ringSize) ringIndex=0;
  }//for i
  Serial.println();
  file.close();
  Serial.println("File close");
  fSave=false;
}//saveFile

