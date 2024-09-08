#include <Arduino.h>
#include <WiFi.h> 
#include <ESPAsyncWebServer.h> 
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Wire.h>
#include <OLEDDisplay.h>
#include <SSD1306Wire.h>

#include "SystemConfig.h"

#include "CameraPins.h"
#include "ButtonService.h"
#include "SaveService.h"
#include "GlobalState.h"

const char* ssid = "RETROLENS32-WIFI";
const char* password = "NotSoSecretPassword";

AsyncWebServer server(80);

ButtonService buttonService(SHUTTER_BUTTON_PIN, SHUTTER_BUTTON_ACTIVE);
QueueHandle_t buttonEventQueue;

SaveService saveService;
QueueHandle_t saveResultQueue;



void setup(){
  
  GlobalState::initialize();

  pinMode(LAMP_PIN, OUTPUT);


  buttonEventQueue = xQueueCreate(5, sizeof(int));

  // Subscribe to button events
  buttonService.subscribe(buttonEventQueue);
  buttonService.begin();

  saveResultQueue = xQueueCreate(1, sizeof(SaveErrorMessage));

}

int i = 0;
void loop() {
  while (1) {
    int buttonEvent;
    if (xQueueReceive(buttonEventQueue, &buttonEvent, portMAX_DELAY)) {
      if (buttonEvent == HIGH) {
        digitalWrite(LAMP_PIN, HIGH);
        Serial.println("Button pressed");
        if (saveService.isImageSaveInProgress() == false) {
          // Capture and save an image
          saveService.startImageSaveTask(saveResultQueue);
        } else {
          Serial.println("Image save already in progress");
        }
      } else {
        digitalWrite(LAMP_PIN, LOW);
        Serial.println("Button released");
      }
    }
  }
}
