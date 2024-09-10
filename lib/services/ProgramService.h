#ifndef RETROLENS_PROGRAM_SERVICE_H
#define RETROLENS_PROGRAM_SERVICE_H
#include <SSD1306Wire.h>
#include "SystemConfig.h"

#define BUTTON_CANCEL_TIMEOUT 5000

class ProgramService {
public:
    ProgramService();

    void initProgram();

    void homeScreen();

    void flashScreen();

    void filmDownloadScreen();

    void setNextState(void (ProgramService::*nextState)());

private:
    void drawHomeScreen();

    void drawTakingPictureScreen();

    void drawFlashScreen();

    void drawFilmDownloadScreen();

    static void programTaskFunction(void *p);

    QueueHandle_t buttonEventQueue;
    SSD1306Wire* display;
    TaskHandle_t programTask;


    // Pointer to the next state function
    void (ProgramService::*nextState)();

    bool isFlashOn = false;
};

#endif //RETROLENS_PROGRAM_SERVICE_H