#include <WiFi.h> 
#include <ESPAsyncWebServer.h> 

const char* ssid = "RETROLENS32-WIFI";
const char* password = "NotSoSecretPassword";

AsyncWebServer server(80);

void setup(){
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });
  server.begin();
}
