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

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    bool last = true;
    while (true) {
        bool now = gpio_get(BTN_PIN_R);
        if (last && !now) {             // pressed
            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(10)); // wait for release
            }
            xSemaphoreGive(xSemaphore_r); // release LED task
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    bool last = true;
    while (true) {
        bool now = gpio_get(BTN_PIN_G);
        if (last && !now) {             // pressed
            while (!gpio_get(BTN_PIN_G)) {
                vTaskDelay(pdMS_TO_TICKS(10)); // wait for release
            }
            xSemaphoreGive(xSemaphore_g); // release LED task
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(10));
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

    xTaskCreate(btn_1_task, "BTN_Task R", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task G", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task G", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
