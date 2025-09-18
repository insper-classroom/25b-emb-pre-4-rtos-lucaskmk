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

void btn_callback(uint gpio, uint32_t events) {
    static int delay_r = 0;
    static int delay_g = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
        if (delay_r < 1000) delay_r += 100;
        else delay_r = 100;
        xQueueSendFromISR(xQueueButId, &delay_r, &xHigherPriorityTaskWoken);
    }
    if (gpio == BTN_PIN_G && (events & GPIO_IRQ_EDGE_FALL)) {
        if (delay_g < 1000) delay_g += 100;
        else delay_g = 100;
        xQueueSendFromISR(xQueueButId2, &delay_g, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, portMAX_DELAY)) {
            while (delay > 0) {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(delay));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(delay));

                int newDelay;
                if (xQueueReceive(xQueueButId, &newDelay, 0)) {
                    delay = newDelay;
                }
            }
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_put(LED_PIN_G, 0);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId2, &delay, portMAX_DELAY)) {
            while (delay > 0) {
                gpio_put(LED_PIN_G, 1);
                vTaskDelay(pdMS_TO_TICKS(delay));
                gpio_put(LED_PIN_G, 0);
                vTaskDelay(pdMS_TO_TICKS(delay));

                int newDelay;
                if (xQueueReceive(xQueueButId2, &newDelay, 0)) {
                    delay = newDelay;
                }
            }
        }
    }
}

int main() {
    stdio_init_all();

    xQueueButId  = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));

    xTaskCreate(btn_1_task, "BTN R", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN G", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED G", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
