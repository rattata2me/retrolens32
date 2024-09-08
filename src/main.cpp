#include <Arduino.h>
#include <WiFi.h> 
#include <ESPAsyncWebServer.h> 

#include "SystemConfig.h"

#include "GlobalState.h"

const char* ssid = "RETROLENS32-WIFI";
const char* password = "NotSoSecretPassword";

AsyncWebServer server(80);

void setup(){
    GlobalState::initialize();
}

void loop() {
}
