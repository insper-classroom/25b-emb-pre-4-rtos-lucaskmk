#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdint.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21
#define LED_PIN_R 5
#define LED_PIN_Y 10

QueueHandle_t xQueueBtn;

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

void led_task(void *p) {
    uint8_t led_id = (uint8_t)(uintptr_t)p;
    uint led_pin = (led_id == 0) ? LED_PIN_R : LED_PIN_Y;

    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_put(led_pin, 0);

    int blinking = 0;
    uint8_t id;

    while (true) {
        // Check if a button event arrived (non-blocking)
        if (xQueueReceive(xQueueBtn, &id, 0) == pdTRUE) {
            if (id == led_id) {
                blinking = !blinking;
                if (!blinking) {
                    gpio_put(led_pin, 0);
                }
            }
        }

        if (blinking) {
            gpio_put(led_pin, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(led_pin, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(32, sizeof(uint8_t));

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    // Create two LED tasks, passing their ID (0=red, 1=yellow)
    xTaskCreate(led_task, "LED R", 256, (void *)0, 1, NULL);
    xTaskCreate(led_task, "LED Y", 256, (void *)1, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
