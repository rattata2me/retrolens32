#ifndef RETROLENS_GLOBALSTATE_HPP
#define RETROLENS_GLOBALSTATE_HPP

#include <Arduino.h>
#include <freertos/semphr.h>

#include "SystemConfig.h"

/**
 * @class GlobalState
 * @brief Manages global resources such as screen, battery, SD card, and WiFi. 
 *        Provides thread-safe access using semaphores.
 */
class GlobalState {
public:
    /**
     * @brief Initializes global state resources such as semaphores and serial communication.
     * 
     * Call this function once during the setup phase to initialize semaphores
     * and set up the serial communication.
     */
    static void initialize();

    /**
     * @brief Safely acquires the screen resource by taking the screen semaphore.
     * 
     * @param timeout Time (in ticks) to wait for the semaphore.
     * @return true if the semaphore was successfully taken, false otherwise.
     */
    static bool safelyTakeScreen(long timeout);

    /**
     * @brief Releases the screen resource by giving back the screen semaphore.
     */
    static void safelyFreeScreen();

    /**
     * @brief Safely acquires the battery resource by taking the battery semaphore.
     * 
     * @param timeout Time (in ticks) to wait for the semaphore.
     * @return true if the semaphore was successfully taken, false otherwise.
     */
    static bool safelyTakeBattery(long timeout);

    /**
     * @brief Releases the battery resource by giving back the battery semaphore.
     */
    static void safelyFreeBattery();

    /**
     * @brief Safely acquires the SD card resource by taking both the screen and battery semaphores.
     * 
     * SD card access requires both screen and battery resources to be available.
     * 
     * @param timeout Time (in ticks) to wait for the semaphores.
     * @return true if both semaphores were successfully taken, false otherwise.
     */
    static bool safelyTakeSdCard(long timeout);

    /**
     * @brief Releases the SD card resource by giving back both the screen and battery semaphores.
     */
    static void safelyFreeSdCard();

    /**
     * @brief Safely acquires the WiFi resource by taking the battery semaphore.
     * 
     * @param timeout Time (in ticks) to wait for the semaphore.
     * @return true if the semaphore was successfully taken, false otherwise.
     */
    static bool safelyTakeWifi(long timeout);

    /**
     * @brief Releases the WiFi resource by giving back the battery semaphore.
     */
    static void safelyFreeWifi();

private:
    /// Semaphore for controlling access to the screen resource
    static SemaphoreHandle_t screenPinsMutex;

    /// Semaphore for controlling access to the battery resource
    static SemaphoreHandle_t batteryPinsMutex;
};

#endif
