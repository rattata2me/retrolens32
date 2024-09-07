#ifndef SAVE_IMAGE_SERVICE_H
#define SAVE_IMAGE_SERVICE_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "esp_camera.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#define SD_INIT_ERROR 1
#define SD_MOUNT_ERROR 2
#define CAPTURE_ERROR 3
#define FILE_OPEN_ERROR 4

/**
 * @struct SaveImageErrorMessage
 * @brief Error messages for SaveImageService.
 */
struct SaveImageErrorMessage {
    int error; ///< Error code.
    const char* message; ///< Error message.
};

/**
 * @class SaveImageService
 * @brief Service to handle capturing and saving images to the SD card using a task.
 */
class SaveImageService {
public:
    /**
     * @brief Constructor for SaveImageService.
     */
    SaveImageService();

    /**
     * @brief Initializes the SD card and prepares the service for saving images.
     * 
     * @param mountPath The path to mount the SD card (default: "/sdcard").
     * @return SaveImageErrorMessage containing error code and message.
     */
    SaveImageErrorMessage initSdCard(const char* mountPath = "/sdcard", long timeout = portMAX_DELAY);

    /**
     * @brief Saves the captured image to the SD card.
     * 
     * @param fb Pointer to the camera framebuffer containing the image.
     * @param path The file path to save the image (default: "/picture.jpg").
     * @return SaveImageErrorMessage containing error code and message.
     */
    SaveImageErrorMessage saveImageToSdCard(camera_fb_t* fb, const String& path = "/picture.jpg");


    /**
     * @brief Creates a task that captures an image and saves it to the SD card.
     * 
     * @param resultQueue The queue to send the result of the task (0 for success, non-zero for error).
     * @return true if the task was successfully created, false otherwise.
     */
    bool startImageSaveTask(QueueHandle_t resultQueue);

    /**
     * @brief Frees the camera framebuffer after the image has been processed.
     */
    void freeFrameBuffer(camera_fb_t* fb);

private:
    /**
     * @brief Captures an image using the camera.
     * 
     * @return A pointer to the captured camera frame (camera_fb_t). Returns nullptr on failure.
     */
    camera_fb_t* captureImage();

    /**
     * @brief The task function that captures and saves an image.
     * 
     * @param p Pointer to SaveImageService object.
     */
    static void imageSaveTask(void* p);

    /**
     * @brief Checks if an SD card is present and accessible.
     * 
     * @return true if SD card is detected and ready, false otherwise.
     */
    bool isSdCardAvailable();

    bool sdInitialized;      ///< Flag to indicate if the SD card is initialized.
    TaskHandle_t taskHandle; ///< Handle for the task that saves the image.
    QueueHandle_t resultQueue; ///< Queue for sending task result.
};

#endif
