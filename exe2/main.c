#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_g;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemaphore_r, 0);
    }
    if (gpio == BTN_PIN_G && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemaphore_g, 0);
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 250;

    while (true) {
        if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY) == pdTRUE) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 250;

    while (true) {
        if (xSemaphoreTake(xSemaphore_g, portMAX_DELAY) == pdTRUE) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS\n");

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_g = xSemaphoreCreateBinary();

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_1_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}
