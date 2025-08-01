
#include "motor.h"
#include "config.h"
#include "server_ble.h"
#include "sdkconfig.h"


// Variáveis

int16_t velA = 1940;
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
    ESP_LOGI("GATT", "Handles: 12 (read) --- 14 (write)\n");

    // Inicializa a fila com capacidade para 10 itens
    xQueue = xQueueCreate(5, sizeof(int));
    while (1)
    {   
        
        movimento_motores(0);
        vTaskDelay(pdMS_TO_TICKS(2000));  

        // Recebe o dado da fila
        int valor_recebido;
        if (xQueueReceive(xQueue, &valor_recebido, 0) == pdTRUE) {
            movimento_motores(valor_recebido);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }    
}

