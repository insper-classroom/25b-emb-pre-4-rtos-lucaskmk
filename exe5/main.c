#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemBtnR;
SemaphoreHandle_t xSemBtnY;

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemBtnR, 0);
    }
    if (gpio == BTN_PIN_Y && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(xSemBtnY, 0);
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int blinking = 0;

    while (true) {
        if (xSemaphoreTake(xSemBtnR, portMAX_DELAY) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_R, 0);
            }
        }

        if (blinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int blinking = 0;

    while (true) {
        if (xSemaphoreTake(xSemBtnY, portMAX_DELAY) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_Y, 0);
            }
        }

        if (blinking) {
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
    printf("Start RTOS\n");

    xSemBtnR = xSemaphoreCreateBinary();
    xSemBtnY = xSemaphoreCreateBinary();

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_r_task, "LED R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
