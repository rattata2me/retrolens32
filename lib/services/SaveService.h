#ifndef RETROLENS_SAVE_SERVICE_H
#define RETROLENS_SAVE_SERVICE_H

#include <Arduino.h>
#include <esp_camera.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "CameraUtils.h"

#define TIMEOUT_MS 100
#define SD_PATH "/sdcard"

#define SD_TIMEOUT 1
#define SD_INIT_ERROR 2
#define SD_MOUNT_ERROR 3
#define CAPTURE_ERROR 4
#define FILE_OPEN_ERROR 5

/**
 * @struct SaveErrorMessage
 * @brief Error messages for SaveService.
 */
struct SaveErrorMessage {
    int code; ///< Error code.
    const char* message; ///< Error message.
};

/**
 * @class SaveService
 * @brief Service to handle capturing and saving images to the SD card using a task.
 * 
 * Example usage:
 * @code
 * SaveService saveService;
 * QueueHandle_t resultQueue;
 * 
 * void setup() {
 *    resultQueue = xQueueCreate(1, sizeof(int));
 *    saveService.startImageSaveTask(resultQueue);
 * }
 * 
 * void loop() {
 *    int result;
 *    if (xQueueReceive(resultQueue, &result, portMAX_DELAY)) {
 *        if (result == 0) {
 *            Serial.println("Image saved successfully");
 *        } else {
 *           Serial.println("Failed to save image");
 *        }
 *    }
 * }
 * @endcode
 */
class SaveService {
public:
    /**
     * @brief Constructor for SaveService.
     */
    SaveService();

    /**
     * @brief Initializes the SD card and prepares the service for saving images.
     * 
     * @param mountPath The path to mount the SD card (default: "/sdcard").
     * @return SaveErrorMessage containing error code and message.
     */
    SaveErrorMessage initSdCard(const char* mountPath = "/sdcard", long timeout = TIMEOUT_MS / portTICK_PERIOD_MS);

    /**
     * @brief Closes the SD card and releases the resources.
     */
    void closeSdCard();

    /**
     * @brief Saves the captured image to the SD card.
     * 
     * @param fb Pointer to the camera framebuffer containing the image.
     * @param path The file path to save the image (default: "/picture.jpg").
     * @return SaveErrorMessage containing error code and message.
     */
    SaveErrorMessage saveImageToSdCard(camera_fb_t* fb, const String& path = "/picture.jpg");


    /**
     * @brief Creates a task that captures an image and saves it to the SD card.
     * 
     * @param resultQueue The queue to send the result of the task. If nullptr, no result is sent.
     * @return true if the task was successfully created, false otherwise.
     */
    bool startImageSaveTask(QueueHandle_t resultQueue);

    /**
     * @brief Checks if an image save is in progress.
     * 
     * @return true if an image save is in progress, false otherwise.
     */
    bool isImageSaveInProgress();

    /**
     * @brief Starts a task that reads the film status and sends the result to a queue.
     * 
     * @param resultQueue The FreeRTOS queue to send the result to.
     * @return True if the task was created successfully, false otherwise.
     */
    bool startReadFilmStatusTask(QueueHandle_t resultQueue);


private:
    /**
     * @brief The task function that captures and saves an image.
     * 
     * @param p Pointer to SaveService object.
     */
    static void imageSaveTask(void* p);

    /**
     * @brief The task function that reads the film status.
     * 
     * @param p Pointer to SaveService object.
     */
    static void readFilmStatusTask(void* p);

    /**
     * @brief Checks if an SD card is present and accessible.
     * 
     * @return true if SD card is detected and ready, false otherwise.
     */
    bool isSdCardAvailable();

    /**
     * @brief Sets the saveImageInProgress flag.
     * 
     * @param value The value to set the flag to.
     */
    void setSaveImageInProgress(bool value);

    volatile bool sdInitialized;      ///< Flag to indicate if the SD card is initialized.
    
    // Save image task variables
    volatile bool saveImageInProgress;     ///< Flag to indicate if an image save is in progress.
    SemaphoreHandle_t saveImageSemaphore;  ///< Semaphore to protect the saveImageInProgress flag.
    TaskHandle_t saveImageTaskHandle; ///< Handle for the task that saves the image.
    QueueHandle_t saveImageResultQueue; ///< Queue for sending task result.
    SaveErrorMessage saveImageErr; ///< Error message for the task.
};

#endif
