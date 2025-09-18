#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <stdint.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21
#define LED_PIN_R 5
#define LED_PIN_Y 10

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    if ((events & GPIO_IRQ_EDGE_FALL)) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t id;

        if (gpio == BTN_PIN_R) {
            id = 0;
            xQueueSendFromISR(xQueueBtn, &id, &xHigherPriorityTaskWoken);
        } else if (gpio == BTN_PIN_Y) {
            id = 1;
            xQueueSendFromISR(xQueueBtn, &id, &xHigherPriorityTaskWoken);
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void btn_task(void *p) {
    uint8_t id;
    while (true) {
        if (xQueueReceive(xQueueBtn, &id, portMAX_DELAY) == pdTRUE) {
            if (id == 0) {
                xSemaphoreGive(xSemaphoreLedR);
            } else if (id == 1) {
                xSemaphoreGive(xSemaphoreLedY);
            }
        }
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int blinking = 0;
    int delay_g = 100; 
    while (true) {
        if (xSemaphoreTake(xSemaphoreLedR, portMAX_DELAY) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_R, 0);
            } else {
                if (delay_g < 1000) delay_g += 100;
                else delay_g = 100;
            }
        }

        if (blinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay_g));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay_g));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    int blinking = 0;
    int delay_g = 100;   

    while (true) {
        if (xSemaphoreTake(xSemaphoreLedY, portMAX_DELAY) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_Y, 0);
            } else {
                if (delay_g < 1000) delay_g += 100;
                else delay_g = 100;
            }
        }

        if (blinking) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(delay_g));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay_g));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}



int main() {
    stdio_init_all();

    xQueueBtn      = xQueueCreate(32, sizeof(uint8_t));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(btn_task, "BTN Task", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "LED R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
