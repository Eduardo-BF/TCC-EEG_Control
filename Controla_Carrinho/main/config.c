#include "config.h"

void gpio_init(void)
{
    esp_rom_gpio_pad_select_gpio(A_IA);
    gpio_set_direction(A_IA,GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(B_IA);
    gpio_set_direction(B_IA,GPIO_MODE_OUTPUT);
    
    ESP_LOGI("SYS", "GPIO configurado\n");
}

void pwm_init(void)
{
    ledc_timer_config_t pwm_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_11_BIT,
        .freq_hz         = 10000,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_ch0 = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = A_IA,                   // saída PWM no GPIO 14
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&pwm_ch0);

    ledc_channel_config_t pwm_ch1 = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = A_IB,                   // saída PWM no GPIO 27
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&pwm_ch1);

    ledc_channel_config_t pwm_ch2 = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_2,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = B_IA,                   // saída PWM no GPIO 26
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&pwm_ch2);

    ledc_channel_config_t pwm_ch3 = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_3,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = B_IB,                   // saída PWM no GPIO 25
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&pwm_ch3);

}

