#ifndef RETROLENS_GLOBALSTATE_HPP
#define RETROLENS_GLOBALSTATE_HPP

#include <Arduino.h>
#include <freertos/semphr.h>

#include "SystemConfig.h"
#include "CameraUtils.h"
#include "ButtonService.h"
#include "SaveService.h"
#include "BatteryReaderService.h"
#include "ProgramService.h"

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
    static bool safelyTakeScreen(long timeout = portMAX_DELAY);

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
    static bool safelyTakeBattery(long timeout = portMAX_DELAY);

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
    static bool safelyTakeSdCard(long timeout = portMAX_DELAY);

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
    static bool safelyTakeWifi(long timeout = portMAX_DELAY);

    /**
     * @brief Releases the WiFi resource by giving back the battery semaphore.
     */
    static void safelyFreeWifi();

    /**
     * @brief Get the Button Service object.
     * 
     * @return ButtonService* Pointer to the Button Service object.
     */
    static ButtonService* getButtonService();

    /**
     * @brief Get the Save Service object.
     * 
     * @return SaveService* Pointer to the Save Service object.
     */
    static SaveService* getSaveService();

    /**
     * @brief Get the Battery Reader Service object.
     * 
     * @return BatteryReaderService* Pointer to the Battery Reader Service object.
     */
    static BatteryReaderService* getBatteryReaderService();

    /**
     * @brief Get the Program Service object.
     * 
     * @return ProgramService* Pointer to the Program Service object.
     */
    static ProgramService* getProgramService();

    /**
     * @brief Set the flash state.
     * 
     * @param state The state to set the flash to.
     */
    static void setFlashState(bool state);

private:
    /// Semaphore for controlling access to the screen resource
    static SemaphoreHandle_t screenPinsMutex;

    /// Semaphore for controlling access to the battery resource
    static SemaphoreHandle_t batteryPinsMutex;

    /// Semaphore for controlling analog battery pins
    static SemaphoreHandle_t batteryAnalogPinsMutex;

    /// Button service instance
    static ButtonService* buttonService;

    /// Save service instance
    static SaveService* saveService;

    /// Battery reader service instance
    static BatteryReaderService* batteryReaderService;

    /// Program service instance
    static ProgramService* programService;
};

#endif
