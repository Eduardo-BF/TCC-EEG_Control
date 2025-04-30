#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"

// Definir Pinos Motores  (A-Dir, B_Esq)

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


// Variáveis
int16_t direcao;
int16_t velA = 1940, velB = 2000;

uint8_t ble_addr_type;
int16_t dado_recebido;
char *TAG = "BLE-Server";

// Funções
void pwm_init(void);
void gpio_init(void);
void set_velocidade_IA(int VEL_A , int VEL_B);
void set_velocidade_IB(int VEL_A , int VEL_B);
void movimento_motores(int SENTIDO);
void ble_app_advertise(void);
void init_ble(void);

QueueHandle_t xQueue;

// Main
void app_main(void)
{   
    // Desativa logs no boot
    //esp_log_level_set("*", ESP_LOG_NONE); 

    // Inic. GPIO
    //gpio_init();
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

// Desenvolvimento Funções
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

void gpio_init(void)
{
    esp_rom_gpio_pad_select_gpio(A_IA);
    gpio_set_direction(A_IA,GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(B_IA);
    gpio_set_direction(B_IA,GPIO_MODE_OUTPUT);
    
    ESP_LOGI("SYS", "GPIO configurado\n");
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

// --------------- BLE  ---------------
// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char * data = (char *)ctxt->om->om_data;
    // Converte a string para um inteiro
    int dado_recebido = atoi(data);

    // Envia o valor para a fila
    if (xQueueSend(xQueue, &dado_recebido, portMAX_DELAY) != pdTRUE) {
        ESP_LOGI(TAG, "Erro ao enviar para a fila\n");
    }

    ESP_LOGI(TAG, "Data from the client: %.*s", ctxt->om->om_len, ctxt->om->om_data);
    ESP_LOGI(TAG, "Número convertido: %d\n", dado_recebido);
    
    return 0;
}

// Read data from ESP32 defined as server
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
    return 0;
}

// Array of pointers to other service definitions
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Aviso conexão
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "Status conexão BLE GAP:  %s", event->connect.status == 0 ? "OK!" : "FALHA!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}

// Inicializa BLE
void init_ble(void){
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("Car_BLE_Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}
