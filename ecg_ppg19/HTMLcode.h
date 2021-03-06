/*
  Kubov V.I. 2019
*/
#define _PPGdecimation 4 // 100/25
#define _ecgSize 100
#define _ppgMax 25
#define _ppgSize 50 // 25*2
#define _dataSize 150 //_ecgSize+_ppgSize
#define _segSize 10
// buffer=(ecg+ppg)*seg

extern long AutoSaveMin;
extern String serverIP;
extern bool Cardio, PPGon;
extern uint8_t LedRed,LedIR;
extern int32_t RawRed,RawIR;

String formHTML(){
  String dataSize=String(_dataSize); String segSize=String(_segSize); 
  String ecgSize=String(_ecgSize); String ppgSize=String(_ppgSize);
  String ppgMax=String(_ppgMax); 
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
  "ecgSize="+ecgSize+"; ppgSize="+ppgSize+";\n"
  "ppgMax="+ppgMax+";\n"  
  "Vmax=1500; Vmin=-Vmax; // 3300/2/1.1 uV R9=1Mohm\n"
  "//Ymin=picH; Ymax=0; //pixel\n"
  "dVs=500;//uV\n"
  "ecgScale=1; ppgScale=1;\n"
  "bufferSize=dataSize*segSize; var buffer=new Array(bufferSize);\n"
  "for (i=0; i<bufferSize; i++) buffer[i]=0;\n"
  "Run=true; Cardio=true; fRecord=false;\n"
  "RecCount=0;\n"
  "var vMax=[]; var vMin=[];\n"
   
  "ws=new WebSocket('ws://192.168.4.1:81/');\n"
  "ws.onmessage=function(msg){\n"
    "process=true;\n"
    "ws.binaryType='arraybuffer';\n"
    "if (msg.data instanceof ArrayBuffer){\n"
      "Datas=new Int16Array(msg.data,0,dataSize+1);\n"
      "cnt.value=Datas[dataSize];\n"
      "if (Run){\n"
        "for (i=dataSize; i<bufferSize; i++) buffer[i-dataSize]=buffer[i];\n"     
        "for (i=0; i<dataSize; i++) buffer[i+bufferSize-dataSize]=Datas[i];\n"
      "};\n"      
      "Draw();\n"
      "SpO2();\n"
    "};\n"
  "};\n"
  "function Draw(){ gr.save();\n"
    "gr.clearRect(0,0,picW,picH);\n"
    "Grid();\n"
    "Graph(0); Graph(1); Graph(2);\n"    
    "gr.restore();\n"
  "};\n"
  "function Grid(){dXgr=100; dYgr=100;\n"
    "gr.strokeStyle='#8080ff'; gr.lineWidth=2; gr.strokeRect(1,1,picW-2,picH-2);\n"
    "gr.lineWidth=1;\n"
    "yScale=ecgScale; if (yScale==0) yScale=1;\n"
    "for (i=0; i<segSize; i++) gr.strokeRect(dXgr*i,0,0,picH);\n"  
    "dV=dVs*yScale; iMax=Math.floor(Vmax/dV);\n"
    "for (i=-iMax; i<=iMax; i++) {\n"
      "y=remap(dV*i,Vmin,Vmax,Ymin,Ymax);\n"
      "if(i==0)gr.lineWidth=2; else gr.lineWidth=1;\n"
      "gr.strokeRect(0,y,picW,0);\n"
    "};\n"  
  "};\n"
  "function Graph(gX){\n"
    "gr.lineWidth=2;\n"
    "if (gX==0){//ECG\n"
      "gr.strokeStyle='#000000'; xScale=ecgScale;\n"
      "p0=0; dP=1; dX=1; iSize=ecgSize; dSize=ppgSize;\n"
    "}else{//PPG\n"
       "xScale=ppgScale;\n"
       "dP=2; dX=4; iSize=ppgMax; dSize=ecgSize;\n"
       "if (gX==1){//Red\n"
         "gr.strokeStyle='#ff0000'; p0=ecgSize;\n"
       "}else{//IR\n"
         "gr.strokeStyle='#ff00c0'; p0=ecgSize+1;\n"
       "};\n"  
    "};\n"  
    "if (xScale==0) return;\n"  
     "gr.beginPath();\n"
     "p=p0; x=0;\n"
     "vMax[gX]=buffer[p]; vMin[gX]=buffer[p];\n" 
     "for (j=0; j<segSize; j++) {\n"
       "for (i=0; i<iSize; i++) {\n"  
         "//if (gX==1) console.log(p);\n"   
         "y=remap(buffer[p]*xScale,Vmin,Vmax,Ymin,Ymax);\n"
         "gr.lineTo(x,y);\n"
         "if (buffer[p]<vMin[gX]) vMin[gX]=buffer[p];\n"
         "if (buffer[p]>vMax[gX]) vMax[gX]=buffer[p];\n"
         "p+=dP; x+=dX;\n"
       "};\n" 
       "p+=dSize;\n"          
     "};\n"  
     "gr.stroke();\n"
  "};\n"
  "function RunStop(){\n"
    "Run=!Run;\n"
    "if (Run) btn.value='Stop'; else btn.value='Run';\n"
  "};\n"
  "function ScaleY(v){\n"
    "if (v==0) ecgScale=ecg.value;\n"
    "if (v==1) ppgScale=ppg.value;\n"
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
"function SpO2(){\n"
  "dRed=vMax[1]-vMin[1];\n"
  "dIR=vMax[2]-vMin[2];\n"
  "if (dIR>0) r=dRed/dIR; else r=10;\n"
  "s=110-r*25;\n"
  "if (s<0) s=0;\n"
  "if (s>100) s=100;\n" 
  "spo2.value=Math.round(s);\n"
"};\n"  
"</script></head>\n"
  "<body>\n"
  "<script> if (!window.WebSocket) alert('Sockets Not Supported');\n"
  +scrAutoSave+"</script>\n"
  "<table align=center border=0>\n"
  "<tr><td colspan=9><canvas id=pic height=402 width=1002></canvas></td></tr>\n"
  "<tr><td><input type=password id=psw value='user' onChange='Login()'></td>\n"
  "<td><input type=text class=ww id=fnm value='Name'></td>\n"
  "<td><select id=sopt>\n"
  "<option value='V'>V</option><option value='LR'>LR</option><option value='RF'>RF</option>\n"
  "<option value='LF'>LF</option><option value=''></option></select></td>\n"
  "<td><input type=button id=rec value='Save' onClick='Save()'></td>\n"
 "<td><input type=text class=wn id=cnt value=0></td>\n"
  "<td><input type=button id=btn value='Stop' onClick='RunStop()'></td>\n"
  "<td>SpO<sub>2</sub>:<input type=text class=wn id=spo2 value=100></td>\n"
  "<td><b>ECG</b>:<select id=ecg onChange='ScaleY(0)'><option value=1>x1</option>"
  "<option value=2>x2</option><option value=4>x4</option><option value=0>Off</option></select></td>\n"
  "<td><b>PPG</b>:<select id=ppg onChange='ScaleY(1)'><option value=0.5>0.5</option><option value=1 selected>x1</option>"
  "<option value=2>x2</option><option value=4>x4</option><option value=0>Off</option></select></td>\n"
  "</tr>\n"
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
  String sPPG; if (PPGon) sPPG="Off"; else sPPG="On";
  String sASM=String(AutoSaveMin);
  String form =""
  "<html><head><style>\n"
  "body,table,input {font-size:24px}\n"
  "</style></head>\n"
  "<body>\n"
  "<form action=test id=act><table align=center border=0>\n"
  "<tr><td><input type=submit value='Reset' name=Reset> Go to:" 
  "<a href='http://192.168.4.1'>192.168.4.1</a></td></tr>\n"
  "<tr><td>AutoSave(min): <input type=text size=5 style='text-align:right' name=AutoSave value="+sASM+">\n"
  " &nbsp <input type=submit value='Set'></td></tr>\n"
  "<tr><td>Switch to <input type=submit name=mode value='"+sMode+"'></td></tr>\n"
  "<tr><td>Pulse Sensor LEDs <input type=submit name=ppg value='"+sPPG+"'></td></tr>\n"
  "</table></form>\n"
  "</body></html>";
  return (form);  
}//testHtml

String ppgHTML(){
  String sLedRed=String(LedRed); String sLedIR=String(LedIR);
  String sRawRed=String(RawRed); String sRawIR=String(RawIR);
  String form =""
  "<html><head><style>\n"
  "body,table,input {font-size:24px}\n"
  "input[type=text]{text-align:right}\n"
  "</style></head>\n"
  "<body>\n"
  "<form><table align=center border=0>\n"
  "<tr><td>Go to:</td><td colspan=2 align=center><a href='http://192.168.4.1'>192.168.4.1</a></td></tr>\n"
  "<tr><td>Red:</td><td><input type=text size=3 name=Red value="+sLedRed+"></td>\n"
  "<td><input type=text size=6 value="+sRawRed+"></td></tr>\n"
  "<tr><td>IR:</td><td><input type=text size=3 name=IR value="+sLedIR+"></td>\n"
  "<td><input type=text size=6 value="+sRawIR+"></td></tr>\n"
  "<tr><td colspan=3 align=center><input type=submit value='Update'></td></tr>\n"
  "</table></form>\n"
  "</body></html>";
  return (form);  
}//ppgHtml
