#include "GlobalState.h"

// Semaphores
SemaphoreHandle_t GlobalState::screenPinsMutex;
SemaphoreHandle_t GlobalState::batteryPinsMutex;
SemaphoreHandle_t GlobalState::batteryAnalogPinsMutex;

// Services
ButtonService* GlobalState::buttonService;
SaveService* GlobalState::saveService;
BatteryReaderService* GlobalState::batteryReaderService;
ProgramService* GlobalState::programService;

void GlobalState::initialize() {
    // Initialize serial communication
    Serial.begin(115200, SERIAL_8N1, NOT_CONNECTED_PIN, -1);

    pinMode(LAMP_PIN, OUTPUT);  // Set the lamp pin as output

    // Create semaphores for shared resources
    screenPinsMutex = xSemaphoreCreateMutex();
    batteryPinsMutex = xSemaphoreCreateMutex();
    batteryAnalogPinsMutex = xSemaphoreCreateMutex();

    // Check if semaphores were successfully created
    if (screenPinsMutex == nullptr || batteryPinsMutex == nullptr) {
        Serial.println("Failed to create semaphores!");
    }

    // Initialize the camera
    esp_err_t err = initializeCamera();
    if (err != ESP_OK) {
        Serial.println("Failed to initialize camera!");
    }

    // Initialize services
    GlobalState::buttonService = new ButtonService(SHUTTER_BUTTON_PIN, SHUTTER_BUTTON_ACTIVE);
    GlobalState::saveService = new SaveService();
    GlobalState::programService = new ProgramService();

    buttonService->begin();
    programService->initProgram();
}

ButtonService* GlobalState::getButtonService() {
    return buttonService;
}

SaveService* GlobalState::getSaveService() {
    return saveService;
}

bool GlobalState::safelyTakeScreen(long timeout) {
    return xSemaphoreTake(screenPinsMutex, timeout);
}

void GlobalState::safelyFreeScreen() {
    xSemaphoreGive(screenPinsMutex);
}

bool GlobalState::safelyTakeBattery(long timeout) {
    if (xSemaphoreTake(batteryAnalogPinsMutex, timeout) == pdTRUE) {
        return xSemaphoreTake(batteryPinsMutex, timeout);
    }
    return false;
}

void GlobalState::safelyFreeBattery() {
    xSemaphoreGive(batteryPinsMutex);
}

bool GlobalState::safelyTakeSdCard(long timeout) {
    if (xSemaphoreTake(screenPinsMutex, timeout) == pdTRUE) {
        return xSemaphoreTake(batteryPinsMutex, timeout);
    }
    return false;
}

void GlobalState::safelyFreeSdCard() {
    xSemaphoreGive(batteryPinsMutex);
    xSemaphoreGive(screenPinsMutex);
}

bool GlobalState::safelyTakeWifi(long timeout) {
    return xSemaphoreTake(batteryAnalogPinsMutex, timeout);
}

void GlobalState::safelyFreeWifi() {
    xSemaphoreGive(batteryAnalogPinsMutex);
}

void GlobalState::setFlashState(bool state) {
    digitalWrite(LAMP_PIN, state);
}
