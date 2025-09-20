#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemaphore_r, &xHigherPriorityTaskWoken);
    }
    if (gpio == BTN_PIN_Y && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemaphore_y, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_red_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, true);
    gpio_put(LED_PIN_R, 0);
    int isBlinking = 0;
    while (1) {
        if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY) == pdTRUE) {
            isBlinking = !isBlinking;
        }
        if (isBlinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_yellow_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, true);
    gpio_put(LED_PIN_Y, 0);
    int isBlinking = 0;
    while (1) {
        if (xSemaphoreTake(xSemaphore_y, portMAX_DELAY) == pdTRUE) {
            isBlinking = !isBlinking;
        }
        if (isBlinking) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main() {
    stdio_init_all();

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R); gpio_set_dir(BTN_PIN_R, false); gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y); gpio_set_dir(BTN_PIN_Y, false); gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_red_task, "LED R", 256, NULL, 1, NULL);
    xTaskCreate(led_yellow_task, "LED Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1) {}
}
