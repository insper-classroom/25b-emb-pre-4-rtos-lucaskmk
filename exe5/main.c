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

QueueHandle_t xQueueRed;
QueueHandle_t xQueueYellow;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BTN_PIN_R) {
        uint8_t msg = 1;
        xQueueSendFromISR(xQueueRed, &msg, &xHigherPriorityTaskWoken);
    }
    else if (gpio == BTN_PIN_Y) {
        uint8_t msg = 1;
        xQueueSendFromISR(xQueueYellow, &msg, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_task(void *pvParameters) {
    int led_pin = ((int*)pvParameters)[0];
    QueueHandle_t queue = (QueueHandle_t)((void**)pvParameters)[1];

    gpio_init(led_pin);
    gpio_set_dir(led_pin, true);

    int isBlinking = 0;
    uint8_t msg;

    while (1) {
        // Verifica se houve evento de botão
        if (xQueueReceive(queue, &msg, 0) == pdTRUE) {
            isBlinking = !isBlinking;  // alterna estado
            if (!isBlinking) {
                gpio_put(led_pin, 0); // apaga se parar
            }
        }

        if (isBlinking) {
            gpio_put(led_pin, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(led_pin, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); // aguarda para não travar CPU
        }
    }
}

int main() {
    stdio_init_all();

    // Cria filas
    xQueueRed = xQueueCreate(5, sizeof(uint8_t));
    xQueueYellow = xQueueCreate(5, sizeof(uint8_t));

    // Configura botões
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, false);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, false);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    static int red_params[2];
    red_params[0] = LED_PIN_R;
    red_params[1] = (int)xQueueRed;

    static int yellow_params[2];
    yellow_params[0] = LED_PIN_Y;
    yellow_params[1] = (int)xQueueYellow;

    xTaskCreate(led_task, "LED Red", 256, red_params, 1, NULL);
    xTaskCreate(led_task, "LED Yellow", 256, yellow_params, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
    return 0;
}
