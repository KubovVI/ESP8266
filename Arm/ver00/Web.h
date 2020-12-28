extern boolean servoLimited;
extern boolean servoSmooth;
extern int16_t HoldReg[];
//#include <ESP8266WebServer.h>
extern ESP8266WebServer server;

String serverHTML(){
  String vSmth=""; if (servoSmooth) vSmth=" checked";
  String vLim=""; if (servoLimited) vLim=" checked";
  String form="<html>\n<head>\n<style>\n\
  body,table,button,input {font-size:24px; height:30px; text-align:center} \n\
  input[type=text]{width:100px; text-align:right}\n\ 
  input[type=checkbox]{width:24px; height:24px}\n\ 
  </style></head><body>\n\
  <h4 align=center>Holding Registers</h4>\n\
  <form action='act' align=center>\n\
  <table border=3 align=center>\n\
  <tr><td>HR</td><td>Value</td></tr>\n";
  for (int i=0; i<7;i++){
    form=form+"<tr><td>"+String(i)+"</td><td><input type=text name='HR"+String(i);
    form=form+"' value="+String(HoldReg[i])+"></td></tr>\n";  
  }//for i
  form=form+"<tr><td><input type=checkbox name='Smooth' value=1"+vSmth+"></td><td>Smooth</td></tr>\n";
  form=form+"<tr><td><input type=checkbox name='Limited' value=1"+vLim+"></td><td>Limited</td></tr>\n";
  form=form+"<tr><td colspan=2 align=center><input type=submit value='Submit'></td></tr>\n\
  </table>\n</form>\n</body>\n</html>";
  return (form);
}//serverHTML

void actionHTMLdecode(){
  for (int i=1; i<7; i++) HoldReg[i]=server.arg("HR"+String(i)).toInt();
  servoLimited=(server.arg("Limited").toInt()==1);
  servoSmooth=(server.arg("Smooth").toInt()==1);
  server.send(200,"text/html",serverHTML());
  Serial.println("server_act");
  Serial.print("HR[1-6]=");
  for (int i=1; i<7; i++) {
    Serial.print(HoldReg[i]); Serial.print(";");
  }//for
  Serial.println();
}//actionHTMLdecode


