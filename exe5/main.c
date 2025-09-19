#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21
#define LED_PIN_R 5
#define LED_PIN_Y 10

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;
TimerHandle_t xTimerRed;
TimerHandle_t xTimerYellow;

typedef struct {
    uint gpio;
} BtnMessage;

void vTimerCallbackRed(TimerHandle_t xTimer) {
    static int state = 0;
    state = !state;
    gpio_put(LED_PIN_R, state);
}

void vTimerCallbackYellow(TimerHandle_t xTimer) {
    static int state = 0;
    state = !state;
    gpio_put(LED_PIN_Y, state);
}

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BtnMessage msg = { .gpio = gpio };
    xQueueSendFromISR(xQueueBtn, &msg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_task(void *p) {
    BtnMessage msg;
    while (1) {
        if (xQueueReceive(xQueueBtn, &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.gpio == BTN_PIN_R) {
                if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
                    xTimerStop(xTimerRed, 0);
                    gpio_put(LED_PIN_R, 0);
                } else {
                    xSemaphoreGive(xSemaphoreLedR);
                    xTimerStart(xTimerRed, 0);
                }
            }
            if (msg.gpio == BTN_PIN_Y) {
                if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
                    xTimerStop(xTimerYellow, 0);
                    gpio_put(LED_PIN_Y, 0);
                } else {
                    xSemaphoreGive(xSemaphoreLedY);
                    xTimerStart(xTimerYellow, 0);
                }
            }
        }
    }
}

int main() {
    stdio_init_all();
    xQueueBtn = xQueueCreate(10, sizeof(BtnMessage));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();
    gpio_init(LED_PIN_R); gpio_set_dir(LED_PIN_R, true);
    gpio_init(LED_PIN_Y); gpio_set_dir(LED_PIN_Y, true);
    gpio_init(BTN_PIN_R); gpio_set_dir(BTN_PIN_R, false); gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y); gpio_set_dir(BTN_PIN_Y, false); gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    xTimerRed = xTimerCreate("TimerRed", pdMS_TO_TICKS(100), pdTRUE, NULL, vTimerCallbackRed);
    xTimerYellow = xTimerCreate("TimerYellow", pdMS_TO_TICKS(100), pdTRUE, NULL, vTimerCallbackYellow);
    xTaskCreate(btn_task, "BtnTask", 256, NULL, 2, NULL);
    vTaskStartScheduler();
    while (1) {}
    return 0;
}
