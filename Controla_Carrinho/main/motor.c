#include "motor.h"
#include "config.h"


// Funções
void movimento_motores (int SENTIDO)
{
    switch (SENTIDO) {
        case 0: // Parado
            set_velocidade_IA(0.0, 0.0);
            set_velocidade_IB(0.0, 0.0);
            ESP_LOGI("Comando", "Parado\n");
            break;
        case 1: // Virar Esquerda
            set_velocidade_IA(velA* 0, velB* 0.75);
            set_velocidade_IB(velA* 0.75, velB* 0);
            ESP_LOGI("Comando", "Vira Esquerda\n");
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        case 2: // Virar Direita
            set_velocidade_IA(velA* 0.75, velB* 0);
            set_velocidade_IB(velA* 0, velB* 0.75);
            ESP_LOGI("Comando", "Vira Direita\n");
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        case 3: // Andar Frente
            set_velocidade_IA(velA* 0.8, velB* 0.8);
            set_velocidade_IB(0, 0.0);
            ESP_LOGI("Comando", "Andar Frente\n");
            vTaskDelay(pdMS_TO_TICKS(600));
            break;
        case 4: // Andar Ré
            set_velocidade_IA(0.0, 0);
            set_velocidade_IB(velA* 0.8, velB* 0.8);
            ESP_LOGI("Comando", "Andar Trás\n");
            vTaskDelay(pdMS_TO_TICKS(400));
            break;
        default:
            ESP_LOGI("Comando", "Comando inválido\n");
            set_velocidade_IA(0.0, 0.0);
            set_velocidade_IB(0.0, 0.0);
            break;
    }

}


void set_velocidade_IA(int VEL_A, int VEL_B)
{   
    if(VEL_A>2000) VEL_A=2000;      // Assegura Duty < 98%
    if(VEL_B>2000) VEL_B=2000;

    // Configurar o duty cycle para cada canal
    set_duty(VEL_A, LEDC_CHANNEL_0); // %duty no canal 0 (D14)
    upt_duty(LEDC_CHANNEL_0);

    set_duty(VEL_B, LEDC_CHANNEL_2); // %duty no canal 1 (D26)
    upt_duty(LEDC_CHANNEL_2);
}

void set_velocidade_IB(int VEL_A, int VEL_B)
{   
    if(VEL_A>2000) VEL_A=2000;      // Assegura Duty < 98%
    if(VEL_B>2000) VEL_B=2000;

    // Configurar o duty cycle para cada canal
    set_duty(VEL_A, LEDC_CHANNEL_1); // %duty no canal 0 (D25)
    upt_duty(LEDC_CHANNEL_1);

    set_duty(VEL_B, LEDC_CHANNEL_3); // %duty no canal 1 (D27)
    upt_duty(LEDC_CHANNEL_3);
}

