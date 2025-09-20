#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;
const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (events & GPIO_IRQ_EDGE_FALL) {
        xQueueSendFromISR(xQueueBtn, &gpio, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_task(void *p) {
    int gpio;
    while (1) {
        if (xQueueReceive(xQueueBtn, &gpio, portMAX_DELAY)) {
            if (gpio == BTN_PIN_R) {
                if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
                    // já estava ativo, libera para parar
                } else {
                    xSemaphoreGive(xSemaphoreLedR); // ativa
                }
            }
            if (gpio == BTN_PIN_Y) {
                if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
                    // já estava ativo, libera para parar
                } else {
                    xSemaphoreGive(xSemaphoreLedY); // ativa
                }
            }
        }
    }
}

void led_red_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, true);
    gpio_put(LED_PIN_R, 0);

    bool isBlinking = false;
    while (1) {
        if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
            isBlinking = !isBlinking;
        }
        if (isBlinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_yellow_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, true);
    gpio_put(LED_PIN_Y, 0);

    bool isBlinking = false;
    while (1) {
        if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
            isBlinking = !isBlinking;
        }
        if (isBlinking) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(10, sizeof(int));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R); gpio_set_dir(BTN_PIN_R, false); gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y); gpio_set_dir(BTN_PIN_Y, false); gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(btn_task, "BTN", 256, NULL, 2, NULL);
    xTaskCreate(led_red_task, "LED R", 256, NULL, 1, NULL);
    xTaskCreate(led_yellow_task, "LED Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1) {}
}
