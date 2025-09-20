/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"


const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_g;

volatile int BR = 0;
volatile int BY = 0;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
        BR++;
        xSemaphoreGiveFromISR(xSemaphore_r, &xHigherPriorityTaskWoken);
    }
    if (gpio == BTN_PIN_Y && (events & GPIO_IRQ_EDGE_FALL)) {
        BY++;
        xSemaphoreGiveFromISR(xSemaphore_g, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100)); }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));   }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 250;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY) == pdTRUE) {
            if (BR%2==0) {gpio_put(LED_PIN_R, 0);}
            else {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
        }}
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int delay = 250;
    while (true) {
        if (xSemaphoreTake(xSemaphore_g, portMAX_DELAY) == pdTRUE) {
            if (BY%2==0) {gpio_put(LED_PIN_Y, 0);}
            else {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));}
        }
    }
}

int main() {
    stdio_init_all();

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_g = xSemaphoreCreateBinary();

    xTaskCreate(btn_1_task, "BTN R", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED R", 256, NULL, 1, NULL);

    xTaskCreate(btn_2_task, "BTN y", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}

