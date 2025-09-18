#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId2;

volatile int delay_r = 0;
volatile int delay_g = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            if (delay_r < 1000) {
                delay_r += 100;
            } else {
                delay_r = 100;
            }
            xQueueSendFromISR(xQueueButId, &delay_r, 0);
        }
        if (gpio == BTN_PIN_G) {
            if (delay_g < 1000) {
                delay_g += 100;
            } else {
                delay_g = 100;
            }
            xQueueSendFromISR(xQueueButId2, &delay_g, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, portMAX_DELAY)) {
            if (delay > 0) {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(delay));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(delay));
            }
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId2, &delay, portMAX_DELAY)) {
            if (delay > 0) {
                gpio_put(LED_PIN_G, 1);
                vTaskDelay(pdMS_TO_TICKS(delay));
                gpio_put(LED_PIN_G, 0);
                vTaskDelay(pdMS_TO_TICKS(delay));
            }
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButId  = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_1_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}
