#include <SD_MMC.h>
#include <string>

#include "GlobalState.h"
#include "SaveService.h"
#include "Films.h"


SaveService::SaveService() 
    : sdInitialized(false), saveImageResultQueue(nullptr), 
    saveImageTaskHandle(nullptr), saveImageInProgress(false) {
    saveImageSemaphore = xSemaphoreCreateMutex();
}

SaveServiceErrorMessage SaveService::initSdCard(const char* mountPath, long timeout) {

    // Safely take the SD card resource
    if (!GlobalState::safelyTakeSdCard(timeout)) {
        return SaveServiceErrorMessage{SD_TIMEOUT, "Failed to take SD card resource"};
    }

    if (!SD_MMC.begin(mountPath, true)) {
        GlobalState::safelyFreeSdCard();
        return SaveServiceErrorMessage{SD_INIT_ERROR, "Failed to mount SD card"};
    }

    if (!isSdCardAvailable()) {
        GlobalState::safelyFreeSdCard();
        return SaveServiceErrorMessage{SD_MOUNT_ERROR, "No SD Card attached"};
    }

    sdInitialized = true;
    return SaveServiceErrorMessage{0, ""};
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

SaveServiceErrorMessage SaveService::saveImageToSdCard(camera_fb_t* fb, const String& path) {
    if (!sdInitialized) {
        return SaveServiceErrorMessage{SD_INIT_ERROR, "SD card is not initialized"};
    }

    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (!file) {
        return SaveServiceErrorMessage{FILE_OPEN_ERROR, "Failed to open file for writing"};
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

FilmsStatus SaveService::readFilmStatus() {
    SaveServiceErrorMessage saveImageErr = {0, ""};

    // Get all the folders inside SD_PATH + SD_FILMS_PATH
    String path = String(SD_PATH) + String(SD_FILMS_PATH);
    File root = SD_MMC.open(path.c_str());

    // Read the folders
    int i = 0;
    
    while (root.openNextFile()) {
        if (root.isDirectory()) {

            // Read the folder name
            String folderName = root.name();
            // Check if folder name follows the format "XXX_filmtype"
            String id = folderName.substring(0, 3);
            String filmType = folderName.substring(4);

            // Make sure id is numeric
            if (!id.toInt()) {
                continue;
            }

            // Check if filmType is valid
            if (!isValidFilmType(filmType.c_str())) {
                continue;
            }

            // Read the files inside the folder
            File folder = SD_MMC.open(folderName.c_str());
            int numFiles = 0;
            while (folder.openNextFile()) {
                numFiles++;
            }

            // Add the film to the films array
            filmsStatus.films[i].framesRemaining = getFilmCapacity(filmType.c_str()) - numFiles;
            filmsStatus.films[i].filmType = filmType;
            filmsStatus.films[i].filmPath = folderName;
            i = (i + 1) % MAX_FILMS;
            filmsStatus.numFilms++;
        }

    }
    
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

bool SaveService::startReadFilmStatusTask(QueueHandle_t resultQueue) {
    // Create the readFilmStatusTask
    if (xTaskCreate(readFilmStatusTask, "ReadFilmStatusTask", 4096, this, 1, &saveImageTaskHandle) != pdPASS) {
        return false;
    }

    return true;
}

void SaveService::readFilmStatusTask(void* p) {
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

    // Read the film status
    // Assuming readFilmStatus() is a function that reads the film status and returns a string
    String filmStatus = "Hello"; //readFilmStatus();

    // Send the film status to the result queue
    if (service->saveImageResultQueue != nullptr)
        xQueueSend(service->saveImageResultQueue, &filmStatus, 0);

    // Close the SD card
    service->closeSdCard();
    service->setSaveImageInProgress(false);

    // Delete the task
    vTaskDelete(nullptr);
}
