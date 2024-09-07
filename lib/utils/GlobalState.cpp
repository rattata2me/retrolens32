#include "GlobalState.h"

// Static Semaphore handles
SemaphoreHandle_t GlobalState::screenPinsMutex = nullptr;
SemaphoreHandle_t GlobalState::batteryPinsMutex = nullptr;

/**
 * @brief Initializes global state resources such as semaphores and serial communication.
 */
void GlobalState::initialize() {
    // Initialize serial communication
    Serial.begin(115200, SERIAL_8N1, NOT_CONNECTED_PIN, -1);

    // Create semaphores for shared resources
    screenPinsMutex = xSemaphoreCreateMutex();
    batteryPinsMutex = xSemaphoreCreateMutex();

    // Check if semaphores were successfully created
    if (screenPinsMutex == nullptr || batteryPinsMutex == nullptr) {
        Serial.println("Failed to create semaphores!");
    }
}

/**
 * @brief Safely acquires the screen.
 * 
 * @param timeout Time (in ticks) to wait for the semaphore.
 * @return true if the semaphore was successfully taken, false otherwise.
 */
bool GlobalState::safelyTakeScreen(long timeout) {
    return xSemaphoreTake(screenPinsMutex, timeout);
}

/**
 * @brief Releases the screen resource.
 */
void GlobalState::safelyFreeScreen() {
    xSemaphoreGive(screenPinsMutex);
}

/**
 * @brief Safely acquires the battery resource.
 * 
 * @param timeout Time (in ticks) to wait for the semaphore.
 * @return true if the semaphore was successfully taken, false otherwise.
 */
bool GlobalState::safelyTakeBattery(long timeout) {
    return xSemaphoreTake(batteryPinsMutex, timeout);
}

/**
 * @brief Releases the battery resource by giving back the battery semaphore.
 */
void GlobalState::safelyFreeBattery() {
    xSemaphoreGive(batteryPinsMutex);
}

/**
 * @brief Safely acquires the SD card resource.
 * 
 * SD card access requires both screen and battery resources to be available.
 * 
 * @param timeout Time (in ticks) to wait for the semaphores.
 * @return true if both semaphores were successfully taken, false otherwise.
 */
bool GlobalState::safelyTakeSdCard(long timeout) {
    bool result = xSemaphoreTake(screenPinsMutex, timeout);
    if (!result) {
        return result;
    }
    return xSemaphoreTake(batteryPinsMutex, timeout);
}

/**
 * @brief Releases the SD card resource.
 */
void GlobalState::safelyFreeSdCard() {
    xSemaphoreGive(batteryPinsMutex);
    xSemaphoreGive(screenPinsMutex);
}

/**
 * @brief Safely acquires the WiFi resource.
 * 
 * @param timeout Time (in ticks) to wait for the semaphore.
 * @return true if the semaphore was successfully taken, false otherwise.
 */
bool GlobalState::safelyTakeWifi(long timeout) {
    return xSemaphoreTake(batteryPinsMutex, timeout);
}

/**
 * @brief Releases the WiFi resource by giving back the battery semaphore.
 */
void GlobalState::safelyFreeWifi() {
    xSemaphoreGive(batteryPinsMutex);
}
