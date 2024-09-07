#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#define MAX_SUBSCRIBERS 10
#define DEBOUNCE_TIME_MS 50

struct ButtonInterruptInfo {
    int buttonPin;
    int buttonActive;
    QueueHandle_t buttonEventQueue;
};

/**
 * @brief ButtonService class for handling button events.
 * 
 * This class provides functionality for handling button events. It allows subscribing to button events and receiving them through a queue. The class uses interrupts to detect button changes and debounce the button signal.
 * 
 * Example usage:
 * @code
 * // Create a button service object
 * ButtonService buttonService(2, LOW);
 * 
 * // Create a queue to receive button events
 * QueueHandle_t buttonEventQueue = xQueueCreate(10, sizeof(int));
 * 
 * // Subscribe to button events
 * buttonService.subscribe(buttonEventQueue);
 * 
 * // Create a task to process button events
 * void buttonListeningTask(void *pvParameters) {
 *     int buttonEvent;
 *     while (1) {
 *         if (xQueueReceive(buttonEventQueue, &buttonEvent, portMAX_DELAY)) {
 *             // Process button event
 *         }
 *     }
 * }
 * xTaskCreate(buttonListeningTask, "Button Listener", 2048, NULL, 1, NULL);
 * @endcode
 */
class ButtonService {
public:

    ButtonInterruptInfo buttonInterruptInfo; /**< Button interrupt information. */

    /**
     * @brief Construct a new Button Service object.
     * 
     * @param buttonPin Pin number for the button.
     * @param buttonActive Active state of the button.
     */
    ButtonService(int buttonPin, int buttonActive);

    /**
     * @brief Start the button service.
     * 
     * This function should be called to start the button service. It will configure the button pin and attach the interrupt.
     */
    void begin();

    /**
     * @brief End the button service.
     * 
     * This function should be called to end the button service. It will detach the interrupt and delete the task and queue.
     */
    void end();


    /**
     * @brief Subscribe to button events.
     * 
     * @param queue Queue to receive button events.
     * @return int 0 on success, -1 on failure.
     */
    int subscribe(QueueHandle_t queue);

    /**
     * @brief Unsubscribe from button events.
     * 
     * @param queue Queue to remove from subscribers.
     */
    void unsubscribe(QueueHandle_t queue);

private:
    int buttonPin;           /**< Pin number for the button. */
    int buttonActive;        /**< Active state of the button. */
    long lastUpdateTime;     /**< Last update time for debouncing. */
    QueueHandle_t subscriberQueues[MAX_SUBSCRIBERS]; /**< Array of subscriber queues. */
    int numSubscribers;      /**< Number of subscribers. */

protected:
    TaskHandle_t buttonTask; /**< Task handle for the button service task. */
    QueueHandle_t buttonEventQueue; /**< Queue for button events. */

    /**
     * @brief Handle button state changes (interrupt service routine).
     * 
     * @param arg Pointer to the ButtonService instance.
     */
    static void IRAM_ATTR handleButtonChange(void *arg);

    /**
     * @brief Task to process button events and notify subscribers.
     * 
     * @param p Pointer to the ButtonService instance.
     */
    static void buttonServiceTask(void *p);

};

#endif // BUTTON_SERVICE_H