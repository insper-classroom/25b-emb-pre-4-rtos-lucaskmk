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

QueueHandle_t xQueueBtn; 
SemaphoreHandle_t xSemaphoreLedR; 
SemaphoreHandle_t xSemaphoreLedY;

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);
    int led_r_state = 0;
    while (1) {
        if (xSemaphoreTake(xSemaphoreLedR,0)){
            led_r_state= !led_r_state; }
            if (led_r_state) {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(100)); }
            else gpio_put(LED_PIN_R, 0);
        }
    }

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);
    int led_y_state = 0;
    while (1) {
        if (xSemaphoreTake(xSemaphoreLedY,0)){
            led_y_state= !led_y_state; }
            if (led_y_state) {
                gpio_put(LED_PIN_Y, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_Y, 0);
                vTaskDelay(pdMS_TO_TICKS(100)); }
            else gpio_put(LED_PIN_Y, 0);
        }
    }

void btn_task(void* p) {
    int led;
    while (true) {
        if(xQueueReceive(xQueueBtn, &led, 0)){
            if (led == BTN_PIN_R){
                xSemaphoreGive(xSemaphoreLedR);
            }
            else if (led == BTN_PIN_Y){
                xSemaphoreGive(xSemaphoreLedY);
            }
        }
    }
}

void btn_callback(uint gpio, uint32_t events) {
    int next_led;
    if (gpio == BTN_PIN_R) {
        if (events & GPIO_IRQ_EDGE_FALL) { // pressionado
              next_led = BTN_PIN_R;
              
        }}
    if (gpio == BTN_PIN_Y) {
        if (events & GPIO_IRQ_EDGE_FALL) { // pressionado
             next_led = BTN_PIN_Y;
        }
        } xQueueSendFromISR(xQueueBtn, &next_led,0) ;
    }

int main() {
    stdio_init_all();
    xQueueBtn = xQueueCreate(8, sizeof(int));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();
    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "led_r_task 2", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "led_y_task 3", 256, NULL, 1, NULL);
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    vTaskStartScheduler();

    while(1){}

    return 0;
}