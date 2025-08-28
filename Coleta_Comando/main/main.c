#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/adc.h"
//#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_device.h"
#include "esp_err.h"
#include "nvs_flash.h"

//Defines
// UUIDs do serviço e característica do servidor
#define SERVER_UUID          0x180 // UUID do serviço
#define WRITE_CHARACTERISTIC 0xDEAD // UUID de escrita

// Definir os canais ADC
//#define ADC1_CHANNEL_3 ADC1_CHANNEL_3  // GPIO39 (VN)
//#define ADC1_CHANNEL_6 ADC1_CHANNEL_6  // GPIO34
//#define ADC1_CHANNEL_7 ADC1_CHANNEL_7  // GPIO35
#define ADC2_CHANNEL_A ADC2_CHANNEL_8  // GPIO25
#define ADC2_CHANNEL_B ADC2_CHANNEL_9  // GPIO26

#define UART_PORT UART_NUM_0  // UART0 


// Variáveis globais
static esp_gatt_if_t client_if;
static uint16_t conn_id;
static esp_bd_addr_t server_address = {0x80, 0x64, 0x6F, 0x1A, 0x64, 0x1A}; // MAC servidor
static bool is_connected = false; 
uint16_t cmd;

static const char *TAG = "BLE_CLIENT";

// --------- BLE Client  
// Função de callback para eventos GATT
static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
    switch (event) {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(TAG, "Client GATT Registrado, conectando...");
        esp_ble_gattc_open(gattc_if, server_address, BLE_ADDR_TYPE_PUBLIC, true);// Conecta ao servidor BLE
        break;

    case ESP_GATTC_CONNECT_EVT:
        ESP_LOGI(TAG, "Conectado ao servidor");
        conn_id = param->connect.conn_id;
        client_if = gattc_if;
        is_connected = true;
        break;

    case ESP_GATTC_WRITE_CHAR_EVT:
        ESP_LOGI(TAG, "Status Escrita: %d", param->write.status);
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        ESP_LOGI(TAG, "Desconectado do servidor");
        is_connected = false;
        break;

    default:
        break;
    }
}

// Função para inicializar BLE
void ble_client_init(void) {
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ret = esp_ble_gattc_register_callback(gattc_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "Failed to register GATT client callback, error: %d", ret);
        return;
    }

    ret = esp_ble_gattc_app_register(0);
    if (ret) {
        ESP_LOGE(TAG, "Failed to register GATT client app, error: %d", ret);
        return;
    }

    ESP_LOGI(TAG, "BLE Client Initialized");
}

// Inicializa o NVS
void nvs_init(void){
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); 
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// Função para escrever na característica
void ble_write_to_server(const char *data) {
    esp_ble_gattc_write_char(client_if, conn_id,
                         14, // handle da característica
                         strlen(data), (uint8_t *)data,
                         ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    ESP_LOGI(TAG, "Enviando comando: %s", data);                                    
}


// Configurar a UART
void init_uart(int baud_rate) {
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config);
    uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);  
}

// Função para configurar o ADC
void init_adc(void) {
    //adc1_config_width(ADC_WIDTH_BIT_12);  
    //adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12);  // Atenuação 12dB
    //adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12);   
    //adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12); ~
    adc2_config_channel_atten(ADC2_CHANNEL_A, ADC_ATTEN_DB_11);
    adc2_config_channel_atten(ADC2_CHANNEL_B, ADC_ATTEN_DB_11); 
}

// Lê os ADCs 
void adc_task(void *pvParameter) {
    //-
    int adc_a_val;
    int adc_b_val;
    esp_err_t result_a;
    esp_err_t result_b;
    // -

    while (1) {
        // int adc_3 = adc1_get_raw(ADC1_CHANNEL_3); // ADC1_3 (GPIO39)
        // int adc_6 = adc1_get_raw(ADC1_CHANNEL_6); // ADC1_6 (GPIO34)
        
        // float voltage_3 = ((adc_3) / 4095.0) * 3.3;  // Converte para tensão
        // float voltage_6 = ((adc_6) / 4095.0) * 3.3;  

        // Enviar os valores das tensões pela porta serial
        //printf("%d \n", adc_6);

        result_a = adc2_get_raw(ADC2_CHANNEL_A, ADC_WIDTH_BIT_12, &adc_a_val);
        result_b = adc2_get_raw(ADC2_CHANNEL_B, ADC_WIDTH_BIT_12, &adc_b_val);
        
        if (result_a == ESP_OK) {
            float voltage_a = (adc_a_val / 4095.0) * 3.3;
            // Faça o que precisar com o voltage_a
            // Ex: printf("Tensão A: %.2fV\n", voltage_a);
        } else {
            // A leitura falhou, provavelmente o rádio estava ativo. Ignore esta leitura.
        }

        if (result_b == ESP_OK) {
            float voltage_b = (adc_b_val / 4095.0) * 3.3;
            // Faça o que precisar com o voltage_b
             printf("Valor ADC B: %d \n", adc_b_val); // Seu printf original, agora no pino novo
        } else {
            // A leitura falhou.
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Main
void app_main(void) {
    
    esp_log_level_set("*", ESP_LOG_NONE);  // Desliga Logs

    init_uart(115200);
    init_adc();
    nvs_init();
    ble_client_init();

    // Task para leitura dos ADCs
    xTaskCreate(adc_task, "ADC Task", 2048, NULL, 5, NULL);

    while (1) {
        if (is_connected) {
            ESP_LOGI(TAG, "Enviando comando: ");
            ble_write_to_server("1");
        } else {
            ESP_LOGW(TAG, "Aguardando conexão...");
            esp_ble_gattc_open(client_if, server_address, BLE_ADDR_TYPE_PUBLIC, true);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Aguarda antes de tentar novamente
    }
    
    
    
}
