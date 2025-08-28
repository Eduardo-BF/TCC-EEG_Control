#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "client_ble.h" 

void rotina_teste(void);


void app_main(void) {
    
    nvs_flash_init(); //Non-Volatile Storage, precisa para Bt
    ble_client_init();// Inicia BLE
    vTaskDelay(pdMS_TO_TICKS(5000));
   
    while(1){
        ESP_LOGI("Main", "Entrando Rotina Teste");
        rotina_teste();
        ESP_LOGI("Main", "Rotina Teste Encerrada");
        vTaskDelay(pdMS_TO_TICKS(3000));

    }
    
}


void rotina_teste(){
    int lista_comandos[] = {3,3,0,4,4,0,1,1,2,2};
    int TAM_LIS = sizeof(lista_comandos) / sizeof(lista_comandos[0]);
    char char_envio[2];
    int i = 0;
    
    while(i<TAM_LIS) {
        sprintf(char_envio, "%d", lista_comandos[i]);
        if(ble_client_write(char_envio)){
            ESP_LOGI("Rotina Teste", "Enviando dados: '%s'", char_envio);
            i++;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}