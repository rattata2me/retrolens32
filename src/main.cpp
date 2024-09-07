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

const char* ssid = "RETROLENS32-WIFI";
const char* password = "NotSoSecretPassword";

AsyncWebServer server(80);

camera_config_t camera_config;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

SSD1306Wire display(0x3c, SCREEN_I2C_SDA, SCREEN_I2C_SCL);

void initCamera(){
  // Set up the camera configuration
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 4 * 1000000;
  camera_config.pixel_format = PIXFORMAT_JPEG;

  camera_config.frame_size = FRAMESIZE_QSXGA;
  camera_config.jpeg_quality = 12;
  camera_config.fb_location = CAMERA_FB_IN_PSRAM;
  camera_config.fb_count = 2;
  camera_config.grab_mode = CAMERA_GRAB_LATEST;


  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

}

camera_fb_t * captureImage(){
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return NULL;
  }
  Serial.println("Camera capture success");
  return fb;
}

#define NOT_CONNECTED_PIN 20

void setup(){
  Serial.begin(115200, SERIAL_8N1, NOT_CONNECTED_PIN, -1);
  initCamera();

  // Write HIGH to pin 4
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);
  delay(1000);
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);

  // Write HIGH to pin 12
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  delay(1000);
  // Read analog value from pin 2
  int val = analogRead(2);
  //int val = 2;
  delay(1000);
  digitalWrite(12, LOW);
  Serial.print("The value of pin 2 is: ");
  Serial.println(val);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Hello World");
  display.drawString(0, 10, "The value of pin 2 is: ");
  display.drawString(0, 20, String(val));

  display.display();
  delay(1000);
  pinMode(2, INPUT_PULLUP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    camera_fb_t * fb = captureImage();
    if(!fb) {
      request->send(500, "text/plain", "Camera capture failed");
      return;
    }
    
    // Set pin 12 to low
    //pinMode(12, OUTPUT);
    //digitalWrite(12, LOW);

    // Start sd card
    if(!SD_MMC.begin("/sdcard", true)){
      request->send(500, "text/plain", "SD Card Mount Failed");
      return;
    }

    // Check if an sd card is present
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
      request->send(500, "text/plain", "No SD Card attached");
      return;
    }
    
    // Save the image to the filesystem
    String path = String("/picture.jpg");
    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    if(!file) {
      request->send(500, "text/plain", "Failed to open file for writing");
      return;
    }
    file.write(fb->buf, fb->len);
    file.close();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
    response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    esp_camera_fb_return(fb);
  });
  server.begin();
}

int i = 0;
void loop() {
  // Read input from pin 16 and write ON/OFF to display
  pinMode(3, INPUT_PULLUP);
  int val = digitalRead(3);
  i = i + 1;
  Serial.println(val);
  delay(1000);
}
