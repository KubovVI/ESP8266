#define _ssid "EspModbus01"
#define _version "EspModbus01"
#define _pass "123456789" 

#define _newMode "Station"
#define _newSSID "netSSID"
#define _newPASS "123456789"
#define _newIP "192.168.0.50"
#define _newGate "192.168.0.1"
#define _newMask "255.255.255.0"
#define _ReqInterval 100//ms

#define _ConfigTime 300000 //5*60*1000

#define  _RfLed 2 // 
#define _Button 0 // Flash

#define OPER_MODE 0
#define CONFIG_MODE 1

#include <ESP8266WiFi.h>

void setupWiFi();
IPAddress StrToIP(String strIP);

void setLed(boolean LedOn);
void Blink(int n);
