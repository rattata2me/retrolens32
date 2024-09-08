#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#include "ButtonService.h"

ButtonService::ButtonService(int buttonPin, int buttonActive)
    : buttonPin(buttonPin), buttonActive(buttonActive), numSubscribers(0), lastButtonState(LOW),
      lastUpdateTime(0), buttonTask(nullptr), buttonEventQueue(nullptr) {}

void ButtonService::begin() {
    // Set the pin mode based on the buttonActive value
    if (buttonActive == LOW) {
        pinMode(buttonPin, INPUT_PULLUP);
    } else {
        pinMode(buttonPin, INPUT_PULLDOWN);
    }
    
    // Attach the interrupt handler to the button pin
    buttonEventQueue = xQueueCreate(5, sizeof(int));
    buttonInterruptInfo = {buttonPin, buttonActive, buttonEventQueue};
    
    attachInterruptArg(digitalPinToInterrupt(buttonPin), handleButtonChange, (void *) &buttonInterruptInfo, CHANGE);

    // Create a task to handle button events
    xTaskCreate(buttonServiceTask, "ButtonServiceTask", 2048, this, 1, &buttonTask);
}

void ButtonService::end() {
    // Delete the button task if it exists
    if (buttonTask != nullptr) {
        vTaskDelete(buttonTask);
    }
    
    // Delete the button event queue if it exists
    if (buttonEventQueue != nullptr) {
        vQueueDelete(buttonEventQueue);
    }
    
    // Detach the interrupt from the button pin
    detachInterrupt(digitalPinToInterrupt(buttonPin));
}

int ButtonService::subscribe(QueueHandle_t queue) {
    // Check if there is space to add a new subscriber
    if (numSubscribers < MAX_SUBSCRIBERS) {
        // Add the queue to the list of subscribers
        subscriberQueues[numSubscribers++] = queue;
        return 0;
    } else {
        // Return an error code if there is no space for a new subscriber
        return -1;
    }
}

void ButtonService::unsubscribe(QueueHandle_t queue) {
    // Find the queue in the list of subscribers and remove it
    for (int i = 0; i < numSubscribers; i++) {
        if (subscriberQueues[i] == queue) {
            // Shift the remaining subscribers to fill the gap
            for (int j = i; j < numSubscribers - 1; j++) {
                subscriberQueues[j] = subscriberQueues[j + 1];
            }
            numSubscribers--;
            break;
        }
    }
}

// Read the button value and convert it to 1 or 0 based on the buttonActive value
#define READ_BUTTON_VALUE(buttonInterruptInfo) \
    (digitalRead(buttonInterruptInfo->buttonPin) == buttonInterruptInfo->buttonActive ? BUTTON_PRESSED : BUTTON_RELEASED)


void ButtonService::handleButtonChange(void *arg) {
    ButtonInterruptInfo *buttonInterruptInfo = (ButtonInterruptInfo *) arg;
    
    int value = READ_BUTTON_VALUE(buttonInterruptInfo);
    // Send the button value to the button event queue from the interrupt context
    xQueueSendFromISR(buttonInterruptInfo->buttonEventQueue, &value, NULL);
}

void ButtonService::buttonServiceTask(void *p) {
    // Cast the argument to a ButtonService pointer
    ButtonService *buttonService = static_cast<ButtonService *>(p);

    int buttonEvent;
    while (true) {
        // Wait for a button event to be received from the button event queue
        if (xQueueReceive(buttonService->buttonEventQueue, &buttonEvent, PERIODIC_CHECK_MS / portTICK_PERIOD_MS)) {
            // Check if enough time has passed since the last button update to debounce the button
            long currentTime = millis();
            if (buttonService->lastButtonState != buttonEvent && currentTime - buttonService->lastUpdateTime > DEBOUNCE_TIME_MS) {
                // Update the last button state
                buttonService->lastButtonState = buttonEvent;

                // Update the last update time
                buttonService->lastUpdateTime = currentTime;

                // Reset the long press flag if the button is released
                if (buttonEvent == BUTTON_RELEASED) {
                    buttonService->longPress = false;
                }

                // Send the button event to all subscribers
                for (int i = 0; i < buttonService->numSubscribers; i++) {
                    xQueueSend(buttonService->subscriberQueues[i], &buttonEvent, 0);
                }
            }
        } else {
            // If no button event was received, check the button state periodically
            int value = READ_BUTTON_VALUE((&buttonService->buttonInterruptInfo));
            if (value != buttonService->lastButtonState && millis() - buttonService->lastUpdateTime > DEBOUNCE_TIME_MS) {
                // Update the last button state
                buttonService->lastButtonState = value;

                // Update the last update time
                buttonService->lastUpdateTime = millis();

                // Send the button event to all subscribers
                for (int i = 0; i < buttonService->numSubscribers; i++) {
                    xQueueSend(buttonService->subscriberQueues[i], &value, 0);
                }
            }
            // If the button is pressed for a long time, send a long press event
            else if (value == BUTTON_PRESSED && !buttonService->longPress 
                    && millis() - buttonService->lastUpdateTime > LONG_PRESS_TIME_MS) {
                buttonService->longPress = true;
                for (int i = 0; i < buttonService->numSubscribers; i++) {
                    int longPressEvent = BUTTON_LONG_PRESSED;
                    xQueueSend(buttonService->subscriberQueues[i], &longPressEvent, 0);
                }
            }
        }
    }
}
