#include "SaveImageService.h"
#include <GlobalState.h>

SaveImageService::SaveImageService() 
    : sdInitialized(false), taskHandle(nullptr), resultQueue(nullptr) {}

SaveImageErrorMessage SaveImageService::initSdCard(const char* mountPath, long timeout) {

    // Safely take the SD card resource
    if (!GlobalState::safelyTakeSdCard(timeout)) {
        return {SD_INIT_ERROR, "Failed to take SD card resource"};
    }

    if (!SD_MMC.begin(mountPath, true)) {
        return {SD_MOUNT_ERROR, "Failed to mount SD card"};
    }

    if (!isSdCardAvailable()) {
        Serial.println("No SD Card attached");
        return {SD_MOUNT_ERROR, "No SD Card attached"};
    }

    sdInitialized = true;
    return {0, ""};
}

bool SaveImageService::isSdCardAvailable() {
    uint8_t cardType = SD_MMC.cardType();
    return (cardType != CARD_NONE);
}

camera_fb_t* SaveImageService::captureImage() {
    camera_fb_t* fb = esp_camera_fb_get();  // Capture the image
    if (!fb) {
        return nullptr;
    }
    return fb;
}

SaveImageErrorMessage SaveImageService::saveImageToSdCard(camera_fb_t* fb, const String& path) {
    if (!sdInitialized) {
        Serial.println("SD card is not initialized");
        return {SD_INIT_ERROR, "SD card is not initialized"};
    }

    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (!file) {
        return {FILE_OPEN_ERROR, "Failed to open file for writing"};
    }

    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Image saved successfully");

    return {0, ""};
}

void SaveImageService::freeFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool SaveImageService::startImageSaveTask(QueueHandle_t resultQueue) {
    this->resultQueue = resultQueue;
    if (xTaskCreate(imageSaveTask, "ImageSaveTask", 4096, this, 1, &taskHandle) != pdPASS) {
        Serial.println("Failed to create image save task");
        return false;
    }

    return true;
}

void SaveImageService::imageSaveTask(void* p) {
    SaveImageService* service = static_cast<SaveImageService*>(p);

    if (!service->initSdCard()) {
        int error = SD_INIT_ERROR;
        xQueueSend(service->resultQueue, &error, portMAX_DELAY);
        vTaskDelete(service->taskHandle);
        return;
    }

    if (!service->isSdCardAvailable()) {
        int error = SD_MOUNT_ERROR;
        xQueueSend(service->resultQueue, &error, portMAX_DELAY);
        vTaskDelete(service->taskHandle);
        return;
    }

    camera_fb_t* fb = service->captureImage();
    if (fb == nullptr) {
        int error = CAPTURE_ERROR;
        xQueueSend(service->resultQueue, &error, portMAX_DELAY);
        vTaskDelete(service->taskHandle);
        return;
    }

    if (!service->saveImageToSdCard(fb, "/sdcard/picture.jpg")) {
        int error = 1;
        xQueueSend(service->resultQueue, &error, portMAX_DELAY);
        service->freeFrameBuffer(fb);
        vTaskDelete(service->taskHandle);
        return;
    }

    service->freeFrameBuffer(fb);

    int success = 0;
    xQueueSend(service->resultQueue, &success, portMAX_DELAY);
    vTaskDelete(service->taskHandle);
}
