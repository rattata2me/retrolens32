
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#include "button_service.h"

ButtonService::ButtonService(int buttonPin, int buttonActive) : buttonPin(buttonPin), buttonActive(buttonActive) {
    numSubscribers = 0;
    lastUpdateTime = 0;
    buttonTask = NULL;
    buttonEventQueue = NULL;
}

void ButtonService::begin() {
    if (buttonActive == LOW) {
        pinMode(buttonPin, INPUT_PULLUP);
    } else {
        pinMode(buttonPin, INPUT_PULLDOWN);
    }
    buttonEventQueue = xQueueCreate(5, sizeof(int));
    buttonInterruptInfo = {buttonPin, buttonActive, buttonEventQueue};
    attachInterruptArg(digitalPinToInterrupt(buttonPin), handleButtonChange, (void *) &buttonInterruptInfo, CHANGE);

    xTaskCreate(buttonServiceTask, "ButtonServiceTask", 2048, this, 1, &buttonTask);
}

void ButtonService::end() {
    if (buttonTask != NULL) {
        vTaskDelete(buttonTask);
    }
    if (buttonEventQueue != NULL) {
        vQueueDelete(buttonEventQueue);
    }
    detachInterrupt(digitalPinToInterrupt(buttonPin));
}

int ButtonService::subscribe(QueueHandle_t queue) {
    if (numSubscribers < MAX_SUBSCRIBERS) {
        subscriberQueues[numSubscribers++] = queue;
        return 0;
    } else {
        return -1;
    }
}

void ButtonService::unsubscribe(QueueHandle_t queue) {
    for (int i = 0; i < numSubscribers; i++) {
        if (subscriberQueues[i] == queue) {
            for (int j = i; j < numSubscribers - 1; j++) {
                subscriberQueues[j] = subscriberQueues[j + 1];
            }
            numSubscribers--;
            break;
        }
    }
}

void ButtonService::handleButtonChange(void *arg) {
    ButtonInterruptInfo *buttonInterruptInfo = (ButtonInterruptInfo *) arg;
    int value = digitalRead(buttonInterruptInfo->buttonPin) == buttonInterruptInfo->buttonActive ? 1 : 0;
    xQueueSendFromISR(buttonInterruptInfo->buttonEventQueue, &value, NULL);
}

void ButtonService::buttonServiceTask(void *p) {
    ButtonService *buttonService = (ButtonService *) p;

    int buttonEvent;
    while (1) {
        if (xQueueReceive(buttonService->buttonEventQueue, &buttonEvent, portMAX_DELAY)) {
            long currentTime = millis();
            if (currentTime - buttonService->lastUpdateTime > DEBOUNCE_TIME_MS) {
                buttonService->lastUpdateTime = currentTime;
                for (int i = 0; i < buttonService->numSubscribers; i++) {
                    xQueueSend(buttonService->subscriberQueues[i], &buttonEvent, 0);
                }
            }
        }
    }
}
