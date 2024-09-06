#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include <freertos/queue.h>

// ButtonInterruptInfo structure
typedef struct {
    int buttonPin;
    QueueHandle_t buttonEventQueue;
} ButtonInterruptInfo;

// Structure for a button event
typedef struct {
    int buttonPin;
    bool buttonState;
} ButtonEvent;

#endif