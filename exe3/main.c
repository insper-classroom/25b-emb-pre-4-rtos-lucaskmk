#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId2;

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int delay = 250;  // blink delay
    bool last = true;

    while (true) {
        bool now = gpio_get(BTN_PIN_R);
        if (last && !now) {  // button press
            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(10));  // wait for release
            }
            xQueueSend(xQueueButId, &delay, 0);
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    int delay = 250;  // blink delay
    bool last = true;

    while (true) {
        bool now = gpio_get(BTN_PIN_G);
        if (last && !now) {  // button press
            while (!gpio_get(BTN_PIN_G)) {
                vTaskDelay(pdMS_TO_TICKS(10));  // wait for release
            }
            xQueueSend(xQueueButId2, &delay, 0);
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(10));
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
    printf("Start RTOS\n");

    xQueueButId  = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));

    xTaskCreate(btn_1_task, "BTN_Task R", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task G", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}
