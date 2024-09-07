#include <Arduino.h>
#include <WiFi.h> 
#include <ESPAsyncWebServer.h> 
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Wire.h>
#include <OLEDDisplay.h>
#include <SSD1306Wire.h>

#include "system_config.h"

#include "camera_pins.h"
#include <button_service.h>

const char* ssid = "RETROLENS32-WIFI";
const char* password = "NotSoSecretPassword";

AsyncWebServer server(80);

ButtonService buttonService(SHUTTER_BUTTON_PIN, SHUTTER_BUTTON_ACTIVE);

QueueHandle_t buttonEventQueue;

#define NOT_CONNECTED_PIN 20

void setup(){
  Serial.begin(115200, SERIAL_8N1, NOT_CONNECTED_PIN, -1);
  pinMode(LAMP_PIN, OUTPUT);

  buttonService.begin();

  buttonEventQueue = xQueueCreate(5, sizeof(int));

  // Subscribe to button events
  buttonService.subscribe(buttonEventQueue);
}

int i = 0;
void loop() {
  while (1) {
    int buttonEvent;
    if (xQueueReceive(buttonEventQueue, &buttonEvent, portMAX_DELAY)) {
      if (buttonEvent == HIGH) {
        digitalWrite(LAMP_PIN, HIGH);
        Serial.println("Button pressed");
      } else {
        digitalWrite(LAMP_PIN, LOW);
        Serial.println("Button released");
      }
    }
  }
}
