#include "motor.h"
#include "config.h"
#include "server_ble.h"
#include "sdkconfig.h"


// Variáveis 

int16_t velA = 1960; //Ajuste diferença vel motores
int16_t velB = 2000;

int16_t dado_recebido;

// Main
void app_main(void)
{   
    // Desativa logs no boot
    //esp_log_level_set("*", ESP_LOG_NONE); 

    // Inic. o Bluetooth
    init_ble();
    // Inic. PWM
    pwm_init();
    set_velocidade_IA(0, 0);
    set_velocidade_IB(0, 0);

    vTaskDelay(pdMS_TO_TICKS(1000)); 
    ESP_LOGI("GATT", "Handles: 12 (read) --- 14 (write)\n"); // mostra

    
    xQueue = xQueueCreate(5, sizeof(int)); // Inicializa a fila com capacidade para 10 itens

    while (1)
    {   
        // movimento_motores(1);
        // vTaskDelay(pdMS_TO_TICKS(500));  // tira ou baixa muito depois
        // movimento_motores(2);
        // vTaskDelay(pdMS_TO_TICKS(500));

        // Recebe o dado da fila
        int valor_recebido;
        if (xQueueReceive(xQueue, &valor_recebido, 0) == pdTRUE) {
            movimento_motores(valor_recebido);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }    
}

