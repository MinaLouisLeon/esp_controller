#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "LittleFS.h"
// create asyncWebServer object on port 80
AsyncWebServer server(80);
// search parameter in http post request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
//var to save values from html form
String ssid;
String pass;
String ip;
String gateway;
// files paths to save wifi settings permanently
const char* ssidPath = "/M2Lssid.txt";
const char* passPath = "/M2Lpass.txt";
const char* ipPath = "/M2Lip.txt";
const char* gatewayPath = "/M2Lgateway.txt";
const char* ssidListPath = "/m2lssidlist.txt";
//IP Address
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255,255,0,0);
//timer var
unsigned long previousMillis = 0;
//interval to wait for wifi to connect
const long interval = 10000;
//esp restart check
boolean restart = false;
//init LittleFS
void initFS(){
  if(!LittleFS.begin()){
    Serial.println("error mounting LittleFS");
  }else{
    Serial.println("LittleFS mounted done");
  }
}
//Read from LittleFS
String readFile(fs::FS &fs,const char * path){
  Serial.printf("Reading file : %s\r\n",path);
  File file = fs.open(path , "r");
  if(!file || file.isDirectory()){
    Serial.println("error : failed to open file for reading");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}
//write to LittleFs
void writeFile(fs::FS &fs,const char* path,const char* message){
  Serial.printf("Writing file : %s\r\n",path);
  File file = fs.open(path,"w");
  if(!file){
    Serial.println("error : failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("file written");
  } else {
    Serial.println("error : write failed");
  }
  file.close();
}
//init Wi-Fi
bool initWiFi(){
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address");
    return false;
  }
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(ip.c_str());
  if(!WiFi.config(localIP,localGateway,subnet)){
    Serial.println("error : failed to configure WIFI_STA");
    return false;
  }
  WiFi.begin(ssid.c_str(),pass.c_str());
  Serial.println("connecting to Wi-Fi ...");
  delay(20000);
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("error : Failed to connect to wifi");
    return false;
  }
  Serial.println("connect to wifi");
  Serial.println(WiFi.localIP());
  return true;
}
//init softAP
void initSoftAP() {
  Serial.println("setting AP");
  WiFi.softAP("M2L",NULL);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address :");
  Serial.println(IP);
  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(LittleFS,"index.html","text/html");
  });
  server.on("/ssidList",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(LittleFS,"/m2lssidlist.txt","text/plain");
  });
  server.serveStatic("/",LittleFS,"/");
}
//scan for network
String scanNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  String ssidList;
  int n = WiFi.scanNetworks();
  for(int i=0;i<n;i++){
    Serial.println(WiFi.SSID(i));
    ssidList += WiFi.SSID(i);
    ssidList += "\r\n";
  }
  Serial.println(ssidList);
  return ssidList;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initFS();
  String ssids = scanNetworks();
  writeFile(LittleFS,ssidListPath,ssids.c_str());
  initSoftAP();
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}