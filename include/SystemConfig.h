#ifndef RETROLENS_SYSTEM_CONFIG_H
#define RETROLENS_SYSTEM_CONFIG_H

// Flash LED pin and configuration
#define LAMP_PIN           4 // LED FloodLamp.

// Shutter button pin and configuration
#define SHUTTER_BUTTON_PIN 3
#define SHUTTER_BUTTON_ACTIVE LOW

// Screen configuration
#define SCREEN_I2C_SDA 15
#define SCREEN_I2C_SCL 13
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Not connected pin
#define NOT_CONNECTED_PIN 20

#endif