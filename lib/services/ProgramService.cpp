#include <Wire.h>
#include <OLEDDisplay.h>
#include <SSD1306Wire.h>
#include "GlobalState.h"
#include "StaticImages.h"
#include "ProgramService.h"


ProgramService::ProgramService() {
    display = new SSD1306Wire(0x3c, SCREEN_I2C_SDA, SCREEN_I2C_SCL);
    buttonEventQueue = xQueueCreate(10, sizeof(int));
    GlobalState::getButtonService()->subscribe(buttonEventQueue);
}

void ProgramService::initProgram() {

    xTaskCreate(programTaskFunction, "ProgramTask", 2048, this, 1, &programTask);
}

void ProgramService::setNextState(void (ProgramService::*nextState)()) {
    this->nextState = nextState;
}

void ProgramService::programTaskFunction(void *p) {
    ProgramService *programService = static_cast<ProgramService *>(p);
    // Set the initial state
    programService->setNextState(&ProgramService::homeScreen);
    while (1) {
        // Call the next state function
        (programService->*programService->nextState)();
    }
}

#define HOME_SCREEN_TIMEOUT 10000
void ProgramService::homeScreen() {
    drawHomeScreen();

    // Wait for button press
    int buttonEvent;
    if (xQueueReceive(buttonEventQueue, &buttonEvent, HOME_SCREEN_TIMEOUT / portTICK_PERIOD_MS)) {
        if (buttonEvent == BUTTON_PRESSED) {
            // Wait for button release
            if (xQueueReceive(buttonEventQueue, &buttonEvent, BUTTON_CANCEL_TIMEOUT / portTICK_PERIOD_MS)) {
                if (buttonEvent == BUTTON_RELEASED) {
                    // Take a picture if no image save is in progress
                    if (GlobalState::getSaveService()->isImageSaveInProgress() == false) {
                        QueueHandle_t saveImageResultQueue = xQueueCreate(1, sizeof(SaveErrorMessage));
                        if (isFlashOn) {
                            GlobalState::setFlashState(true);
                        }
                        GlobalState::getSaveService()->startImageSaveTask(saveImageResultQueue);
                        drawTakingPictureScreen();
                        SaveErrorMessage result;
                        if (xQueueReceive(saveImageResultQueue, &result, portMAX_DELAY)) {
                            if (result.code == 0) {
                                // Set the next state to the film download screen
                                setNextState(&ProgramService::homeScreen);
                                Serial.println("Image saved successfully");
                                if (isFlashOn) {
                                    GlobalState::setFlashState(false);
                                }
                                return;
                            }
                        } else {
                            // TODO: Handle error
                            if (isFlashOn) {
                                GlobalState::setFlashState(false);
                            }
                        }
                    }
                } else if (buttonEvent == BUTTON_LONG_PRESSED) {
                    setNextState(&ProgramService::flashScreen);
                    return;
                }
            }
        }
    }

    // Check battery status
    GlobalState::getBatteryReaderService()->startBatteryReadTask();
    setNextState(&ProgramService::homeScreen);
}

void ProgramService::drawTakingPictureScreen() {
    GlobalState::safelyTakeScreen();
    display->init();
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, "Taking Picture...");
    display->drawXbm(10, 10, HAPPY_XBM_WIDTH, HAPPY_XBM_HEIGHT, HAPPY_XBM_IMAGE);
    display->display();
    display->end();
    GlobalState::safelyFreeScreen();
}

void ProgramService::drawBatteryStatus() {
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    float percentage = GlobalState::getBatteryReaderService()->getLastBatteryLevel();
    display->drawXbm(5, 5, BATTERY_XBM_WIDTH, BATTERY_XBM_HEIGHT, BATTERY_XBM_IMAGE);
    // Draw vertical lines depending on the battery level
    for (int i = 0; i < 5; i++) {
        if (true){//percentage > i * 20) {
            display->drawVerticalLine(7 + i * 4, 7, 9);
            display->drawVerticalLine(8 + i * 4, 7, 9);
            display->drawVerticalLine(9 + i * 4, 7, 9);
        }
    }
}

void ProgramService::drawFlashStatus() {
    if (isFlashOn) {
        display->drawXbm(108, 5, FLASH_SMALL_XBM_WIDTH, FLASH_SMALL_XBM_HEIGHT, FLASH_SMALL_XBM_IMAGE);
        return;
    }
    display->drawXbm(108, 5, FLASH_SMALL_OFF_XBM_WIDTH, FLASH_SMALL_OFF_XBM_HEIGHT, FLASH_SMALL_OFF_XBM_IMAGE);
}

void ProgramService::drawHomeScreen() {
    GlobalState::safelyTakeScreen();
    display->init();
    display->clear();
    drawBatteryStatus();
    drawFlashStatus();
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 30, "Home Screen");
    display->display();
    display->end();
    GlobalState::safelyFreeScreen();
}

#define FLASH_SCREEN_TIMEOUT 50000
void ProgramService::flashScreen() {
    drawFlashScreen();

    // Wait for button press
    int buttonEvent;
    if (xQueueReceive(buttonEventQueue, &buttonEvent, FLASH_SCREEN_TIMEOUT / portTICK_PERIOD_MS)) {
        if (buttonEvent == BUTTON_PRESSED) {
            // Wait for button release
            if (xQueueReceive(buttonEventQueue, &buttonEvent, BUTTON_CANCEL_TIMEOUT / portTICK_PERIOD_MS)) {
                if (buttonEvent == BUTTON_RELEASED) {
                    // Set the next state to the home screen
                    setNextState(&ProgramService::filmDownloadScreen);
                    return;
                } else if (buttonEvent == BUTTON_LONG_PRESSED) {
                    // Toggle the flash
                    isFlashOn = !isFlashOn;
                    setNextState(&ProgramService::flashScreen);
                    return;
                }
            }
        } else {
            setNextState(&ProgramService::flashScreen);
            return;
        }
    }
    
    setNextState(&ProgramService::homeScreen);
}

void ProgramService::drawFlashScreen() {
    GlobalState::safelyTakeScreen();
    display->init();
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, "Flash Screen");
    // Draw the flash status
    display->drawString(0, 10, "Flash: ");
    display->drawString(0, 20, isFlashOn ? "On" : "Off");
    display->display();
    display->end();
    GlobalState::safelyFreeScreen();  
}

#define FILM_DOWNLOAD_SCREEN_TIMEOUT 30000
void ProgramService::filmDownloadScreen() {
    drawFilmDownloadScreen();

    // Wait for button press
    int buttonEvent;
    if (xQueueReceive(buttonEventQueue, &buttonEvent, FILM_DOWNLOAD_SCREEN_TIMEOUT / portTICK_PERIOD_MS)) {
        if (buttonEvent == BUTTON_PRESSED) {
            // Wait for button release
            if (xQueueReceive(buttonEventQueue, &buttonEvent, BUTTON_CANCEL_TIMEOUT / portTICK_PERIOD_MS)) {
                if (buttonEvent == BUTTON_RELEASED) {
                    // Set the next state to the home screen
                    setNextState(&ProgramService::homeScreen);
                    return;
                } else if (buttonEvent == BUTTON_LONG_PRESSED) {
                    // Start film download
                    //GlobalState::getDownloadService()->startFilmDownload();
                    setNextState(&ProgramService::homeScreen);
                    return;
                }
            }
        } else {
            setNextState(&ProgramService::filmDownloadScreen);
            return;
        }
    }
    
    setNextState(&ProgramService::homeScreen);
}

void ProgramService::drawFilmDownloadScreen() {
    GlobalState::safelyTakeScreen();
    display->init();
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, "Film Download Screen");
    display->display();
    display->end();
    GlobalState::safelyFreeScreen();  
}