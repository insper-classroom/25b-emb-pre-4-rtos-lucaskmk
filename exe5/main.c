#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21
#define LED_PIN_R 5
#define LED_PIN_Y 10

// Recursos RTOS obrigatórios
QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

// Estrutura para mensagem da fila
typedef struct {
    uint gpio;
} BtnMessage;

// Callback de interrupção dos botões
void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BtnMessage msg = { .gpio = gpio };
    xQueueSendFromISR(xQueueBtn, &msg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Task que processa mensagens da fila de botões
void btn_task(void *p) {
    BtnMessage msg;
    while (1) {
        if (xQueueReceive(xQueueBtn, &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.gpio == BTN_PIN_R) {
                // Alterna semáforo do LED vermelho
                if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
                    // Se estava "pegado", devolve para desligar
                } else {
                    // Se estava livre, dá para ativar
                    xSemaphoreGive(xSemaphoreLedR);
                }
            } else if (msg.gpio == BTN_PIN_Y) {
                // Alterna semáforo do LED amarelo
                if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
                    // Já estava ativo, libera
                } else {
                    xSemaphoreGive(xSemaphoreLedY);
                }
            }
        }
    }
}

void led_red_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, true);

    while (1) {
        if (xSemaphoreTake(xSemaphoreLedR, portMAX_DELAY) == pdTRUE) {
            // Enquanto ativo, pisca
            while (uxSemaphoreGetCount(xSemaphoreLedR) > 0) {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            gpio_put(LED_PIN_R, 0); // garante apagado
        }
    }
}

void led_yellow_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, true);

    while (1) {
        if (xSemaphoreTake(xSemaphoreLedY, portMAX_DELAY) == pdTRUE) {
            while (uxSemaphoreGetCount(xSemaphoreLedY) > 0) {
                gpio_put(LED_PIN_Y, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_Y, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            gpio_put(LED_PIN_Y, 0);
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(10, sizeof(BtnMessage));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, false);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, false);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(btn_task, "BtnTask", 256, NULL, 2, NULL);
    xTaskCreate(led_red_task, "LedRed", 256, NULL, 1, NULL);
    xTaskCreate(led_yellow_task, "LedYellow", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
    return 0;
}
