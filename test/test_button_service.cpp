#include <Arduino.h>
#include <unity.h>
#include <button_service.h>

class TestButtonService : public ButtonService {
public:
    TestButtonService(int buttonPin, int buttonActive) : ButtonService(buttonPin, buttonActive) {}
    
    void wrappedHandleButtonUp() {
        int value = HIGH;
        xQueueSend(buttonEventQueue, &value, 0);
    }
    void wrappedHandleButtonDown() {
        int value = LOW;
        xQueueSend(buttonEventQueue, &value, 0);
    }
};

void setUp(void) {
    // Set up code here, if needed
}

void tearDown(void) {
    // Clean up code here, if needed
}

volatile bool buttonPressed = false;
volatile bool buttonReleased = false;

void buttonListeningTask(void *pvParameters) {
    int buttonEvent;
    QueueHandle_t queue = (QueueHandle_t) pvParameters;
    while (1) {
        if (xQueueReceive(queue, &buttonEvent, portMAX_DELAY)) {
            if (buttonEvent == HIGH) {
                buttonPressed = true;
            } else {
                buttonReleased = true;
            }
        }
    }
}

QueueHandle_t buttonEventQueue;

void testButtonService() {
    // Create a button service object
    TestButtonService testButtonService(2, LOW);
    
    // Create a queue to receive button events
    buttonEventQueue = xQueueCreate(10, sizeof(int));

    // Subscribe to button events
    testButtonService.subscribe(buttonEventQueue);

    // Create a task to process button events
    xTaskCreate(buttonListeningTask, "Button Listener", 2048, (void *) buttonEventQueue, 1, NULL);

    // Simulate a button press
    testButtonService.wrappedHandleButtonUp();
    delay(1000);
    TEST_ASSERT_TRUE(buttonPressed);
    
    // Simulate a button release
    testButtonService.wrappedHandleButtonDown();
    delay(1000);
    TEST_ASSERT_TRUE(buttonReleased);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testButtonService);
    UNITY_END();
}

void loop() {
}