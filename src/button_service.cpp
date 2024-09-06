#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <button_service.h>

#define DEBOUNCE_TIME_MS 50  // Debounce time in milliseconds

// ISR for button press
static void IRAM_ATTR handleButtonChange(void *arg) {
    ButtonInterruptInfo *buttonInterruptInfo = (ButtonInterruptInfo *) arg;
    ButtonEvent buttonEvent = {buttonInterruptInfo->buttonPin, digitalRead(buttonInterruptInfo->buttonPin)};
    xQueueSendFromISR(buttonInterruptInfo->buttonEventQueue, &buttonEvent, NULL);  // Send button event to queue
}


class ButtonService {
public:
    ButtonService(int buttonPin) : buttonPin(buttonPin) {
        pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with pull-up

        buttonEventQueue = xQueueCreate(5, sizeof(int));  // Create queue for button events

        ButtonInterruptInfo buttonInterruptInfo = {buttonPin, buttonEventQueue};
        attachInterruptArg(digitalPinToInterrupt(buttonPin), handleButtonChange, (void *) &buttonInterruptInfo, CHANGE);  // Attach interrupt for button press

        debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, (void *) 0, debounceTimerCallback);  // Create debounce timer

        xTaskCreate(buttonServiceTask, "ButtonServiceTask", 2048, NULL, 1, NULL);  // Create button service task
    }


private:
    int buttonPin;
    volatile bool debounceInProgress = false;  // To track debounce state

    // Software timer for debouncing
    TimerHandle_t debounceTimer;

    // Button task
    TaskHandle_t buttonTask;

    // Queue for button events
    QueueHandle_t buttonEventQueue;

    // Debounce timer callback (called after debounce time)
    static void debounceTimerCallback(TimerHandle_t xTimer) {
        debounceInProgress = false;  // Allow new button presses after debounce time
    }

    // Button service task
    static void buttonServiceTask(void *pvParameters) {
        int buttonEvent;
        while (1) {
            if (xQueueReceive(buttonEventQueue, &buttonEvent, portMAX_DELAY)) {  // Wait for button event
                for (int i = 0; i < numSubscribers; i++) {
                    xTaskNotifyGive(subscriberTasks[i]);  // Notify each subscriber
                }
            }
        }
    }
};

// Example subscriber task
void task1(void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Wait for button press notification
        Serial.println("Task 1: Button Press Detected!");
    }
}

void setup() {
    ButtonService buttonService(3);  // Create an instance of ButtonService with button pin 3
    buttonService.subscribe(task1);  // Subscribe task1 to the button service
}

void loop() {
    // Your main code here
}