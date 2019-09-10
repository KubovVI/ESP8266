// Kubov V.I. 2019
#define _dataSize 100
#define _segSize 10

extern long AutoSaveMin;
extern bool Cardio;
extern String serverIP;

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
  "Vmax=3.3/2/1.1; Vmin=-Vmax; //in 1.5mV R9=1Mohm\n"
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