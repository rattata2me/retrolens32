#include "GlobalState.h"
#include "BatteryReaderService.h"

BatteryReaderService::BatteryReaderService(uint8_t analogPin, uint8_t controlPin)
    : analogPin(analogPin), controlPin(controlPin), lastBatteryLevel(0.0f), batteryReadTaskHandle(nullptr), resultQueue(nullptr) {
    batteryMutex = xSemaphoreCreateMutex();
}

void BatteryReaderService::initBatteryRead(long timeout) {
    // Safely take the battery resource
    if (!GlobalState::safelyTakeBattery(timeout)) {
        return;
    }
    pinMode(analogPin, INPUT_PULLUP);  // Initialize the analog pin
}

void BatteryReaderService::closeBatteryRead() {
    // Safely free the battery resource
    GlobalState::safelyFreeBattery();
}

float BatteryReaderService::readBatteryLevel() {
    if (xSemaphoreTake(batteryMutex, portMAX_DELAY) == pdTRUE) {
        // Read the battery voltage and store the result
        lastBatteryLevel = analogReadBatteryLevel();
        xSemaphoreGive(batteryMutex); // Release the mutex after reading
    }
    return lastBatteryLevel;
}

float BatteryReaderService::getLastBatteryLevel() {
    float batteryLevel = -1.0f;
    if (xSemaphoreTake(batteryMutex, portMAX_DELAY) == pdTRUE) {
        // Return the stored battery level
        batteryLevel = lastBatteryLevel;
        xSemaphoreGive(batteryMutex); // Release the mutex
    }
    return batteryLevel;
}

float BatteryReaderService::analogReadBatteryLevel() {
    pinMode(controlPin, OUTPUT);  // Set control pin as output
    digitalWrite(controlPin, HIGH);  // Enable the battery voltage divider
    vTaskDelay(CONTROL_PIN_DELAY_MS / portTICK_PERIOD_MS);  // Wait for the control pin to stabilize
    // Set analog pin to read
    int rawAnalogValue = analogRead(analogPin);  // Read raw analog value
    digitalWrite(controlPin, LOW);  // Disable the battery voltage divider
    pinMode(analogPin, INPUT_PULLUP); // Set analog pin back to input mode
    Serial.println(rawAnalogValue);
    float voltage = (rawAnalogValue / 4095.0) * 3.3;  // Convert raw value to voltage (assuming a 3.3V reference)
    voltage = batteryLevelToPercentage(voltage);  // Convert voltage to percentage
    return voltage;
}

bool BatteryReaderService::startBatteryReadTask(QueueHandle_t resultQueue) {
    this->resultQueue = resultQueue;
    // Create the task for reading battery level
    if (xTaskCreate(batteryReadTask, "BatteryReadTask", 2048, this, 1, &batteryReadTaskHandle) != pdPASS) {
        return false; // Task creation failed
    }

    return true;
}

void BatteryReaderService::batteryReadTask(void* p) {
    BatteryReaderService* service = static_cast<BatteryReaderService*>(p);
    // Initialize the battery reading
    service->initBatteryRead();
    // Read the battery level
    float batteryLevel = service->readBatteryLevel();

    // Close the battery reading
    service->closeBatteryRead();
    
    // Send the battery level to the queue
    if (service->resultQueue != nullptr) {
        xQueueSend(service->resultQueue, &batteryLevel, 0);
    }

    // Task has finished, delete it
    service->batteryReadTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

float BatteryReaderService::batteryLevelToPercentage(float voltage) {
    // Clamp the voltage within the min/max range
    voltage = max(minVoltage, min(maxVoltage, voltage));

    // Map the voltage to a percentage (0% at minVoltage, 100% at maxVoltage)
    float percentage = ((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;

    return percentage;
}
