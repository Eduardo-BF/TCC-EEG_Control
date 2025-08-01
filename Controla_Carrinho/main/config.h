#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// Pinos Motores  (A-Dir, B_Esq)

#define A_IA  GPIO_NUM_14  // A-1A
#define A_IB  GPIO_NUM_27  // A-1B
#define B_IA  GPIO_NUM_26  // B-1A
#define B_IB  GPIO_NUM_25  // B-1B

// Macros 
#define set_duty(duty, channel) ledc_set_duty(LEDC_LOW_SPEED_MODE, \
                                              channel,            \
                                              duty)

#define upt_duty(channel)       ledc_update_duty(LEDC_LOW_SPEED_MODE, \
                                                 channel)


// Funções
void pwm_init(void);
void gpio_init(void);

#endif