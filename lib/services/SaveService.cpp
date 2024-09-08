#include <SD_MMC.h>

#include "GlobalState.h"
#include "SaveService.h"


SaveService::SaveService() 
    : sdInitialized(false), saveImageResultQueue(nullptr), 
    saveImageTaskHandle(nullptr), saveImageInProgress(false) {
    saveImageSemaphore = xSemaphoreCreateMutex();
}

SaveErrorMessage SaveService::initSdCard(const char* mountPath, long timeout) {

    // Safely take the SD card resource
    if (!GlobalState::safelyTakeSdCard(timeout)) {
        return SaveErrorMessage{SD_TIMEOUT, "Failed to take SD card resource"};
    }

    if (!SD_MMC.begin(mountPath, true)) {
        GlobalState::safelyFreeSdCard();
        return SaveErrorMessage{SD_INIT_ERROR, "Failed to mount SD card"};
    }

    if (!isSdCardAvailable()) {
        GlobalState::safelyFreeSdCard();
        return SaveErrorMessage{SD_MOUNT_ERROR, "No SD Card attached"};
    }

    sdInitialized = true;
    return SaveErrorMessage{0, ""};
}

void SaveService::closeSdCard() {
    if (sdInitialized) {
        SD_MMC.end();
        sdInitialized = false;
        GlobalState::safelyFreeSdCard();
    }
}

bool SaveService::isSdCardAvailable() {
    uint8_t cardType = SD_MMC.cardType();
    return (cardType != CARD_NONE);
}

SaveErrorMessage SaveService::saveImageToSdCard(camera_fb_t* fb, const String& path) {
    if (!sdInitialized) {
        return SaveErrorMessage{SD_INIT_ERROR, "SD card is not initialized"};
    }

    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (!file) {
        return SaveErrorMessage{FILE_OPEN_ERROR, "Failed to open file for writing"};
    }

    file.write(fb->buf, fb->len);
    file.close();

    return {0, ""};
}

bool SaveService::isImageSaveInProgress() {
    if (xSemaphoreTake(saveImageSemaphore, portMAX_DELAY) == pdTRUE) {
        // Check the flag
        bool result = saveImageInProgress;
        xSemaphoreGive(saveImageSemaphore); // Release the semaphore
        return result;
    }
    return true;
}

void SaveService::setSaveImageInProgress(bool value) {
    if (xSemaphoreTake(saveImageSemaphore, portMAX_DELAY) == pdTRUE) {
        saveImageInProgress = value;
        xSemaphoreGive(saveImageSemaphore); // Release the semaphore
    }
}

bool SaveService::startImageSaveTask(QueueHandle_t resultQueue) {
    // Check if an image save is already in progress
    if (xSemaphoreTake(saveImageSemaphore, portMAX_DELAY) == pdTRUE) {
        if (saveImageInProgress) {
            xSemaphoreGive(saveImageSemaphore); // Release the semaphore
            return false;
        }
        saveImageInProgress = true;
        xSemaphoreGive(saveImageSemaphore); // Release the semaphore
    }
    saveImageResultQueue = resultQueue;

    // Create the imageSaveTask
    if (xTaskCreate(imageSaveTask, "ImageSaveTask", 4096, this, 1, &saveImageTaskHandle) != pdPASS) {
        return false;
    }

    return true;
}

void SaveService::imageSaveTask(void* p) {
    SaveService* service = static_cast<SaveService*>(p);

    // Initialize the SD card
    service->saveImageErr = service->initSdCard(SD_PATH);
    if (service->saveImageErr.code != 0) {
        service->closeSdCard();
        service->setSaveImageInProgress(false);
        if (service->saveImageResultQueue != nullptr)
            xQueueSend(service->saveImageResultQueue, &(service->saveImageErr), 0);
        vTaskDelete(nullptr);
    }

    // Capture the image
    camera_fb_t* fb = cameraCaptureImage();

    // Save the image to the SD card
    service->saveImageErr = service->saveImageToSdCard(fb);

    // Release the frame buffer
    cameraReleaseFrameBuffer(fb);

    // Close the SD card
    service->closeSdCard();
    service->setSaveImageInProgress(false);

    // Send the result to the result queue
    if (service->saveImageResultQueue != nullptr)
        xQueueSend(service->saveImageResultQueue, &(service->saveImageErr), 0);

    // Delete the task
    vTaskDelete(nullptr);
}
