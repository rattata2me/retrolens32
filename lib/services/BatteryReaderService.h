#ifndef RETROLENS_BATTERY_READER_SERVICE_H
#define RETROLENS_BATTERY_READER_SERVICE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/**
 * @class BatteryReaderService
 * @brief Service to handle analog reading of the battery voltage from a specific pin, 
 *        with an option to perform the reading in a task.
 */
class BatteryReaderService {
public:
    /**
     * @brief Constructor for BatteryReaderService.
     * 
     * @param analogPin The analog pin to read the battery voltage from.
     */
    BatteryReaderService(int analogPin);

    /**
     * @brief Initialize the BatteryReaderService.
     */
    void initBatteryRead(long timeout = portMAX_DELAY);

    /**
     * @brief Close the BatteryReaderService.
     */
    void closeBatteryRead();

    /**
     * @brief Reads the battery level (in volts) and stores it.
     * 
     * @return The last read battery voltage.
     */
    float readBatteryLevel();

    /**
     * @brief Get the last read battery voltage.
     * 
     * @return The stored battery voltage.
     */
    float getLastBatteryLevel();

    /**
     * @brief Starts a task that reads the battery level and sends the result to a queue.
     * 
     * @param resultQueue The FreeRTOS queue to send the result to.
     * @return True if the task was created successfully, false otherwise.
     */
    bool startBatteryReadTask(QueueHandle_t resultQueue);

private:
    /**
     * @brief Task function that reads the battery level and sends the result to the queue.
     * 
     * @param p A pointer to the BatteryReaderService instance.
     */
    static void batteryReadTask(void* p);

    /**
     * @brief Reads the raw analog value and converts it to a battery voltage.
     * 
     * @return The battery voltage in volts.
     */
    float analogReadBatteryLevel();

        /**
     * @brief Converts the battery voltage to a percentage (0-100%) based on predefined thresholds.
     * 
     * @param voltage The battery voltage.
     * @return The battery level as a percentage.
     */
    float batteryLevelToPercentage(float voltage);

    int analogPin;                  ///< Analog pin to read from.
    float lastBatteryLevel;          ///< Last read battery voltage.
    SemaphoreHandle_t batteryMutex;  ///< Mutex to protect battery reading operations.
    TaskHandle_t batteryReadTaskHandle; ///< Handle for the battery read task.
    QueueHandle_t resultQueue;       ///< Queue to send the result to.

    // Battery voltage thresholds
    const float minVoltage = 3.0;    ///< Minimum voltage for 0% battery
    const float maxVoltage = 4.2;    ///< Maximum voltage for 100% battery
};

#endif
