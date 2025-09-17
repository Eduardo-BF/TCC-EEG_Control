#include "server_ble.h"

const char *TAG = "BLE-Server"; // tag de log 
uint8_t ble_addr_type = 0;         
QueueHandle_t xQueue = NULL; 
//----------------------------------------------------------

static const char *NOME_SERVER = "Car_BLE_Server"; // nome para conexão

//----------------------------------------------------------

// Escrita
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char * data = (char *)ctxt->om->om_data;
    int dado_recebido = atoi(data); // Converte a string para um inteiro

    // Envia o valor para a fila
    if (xQueueSend(xQueue, &dado_recebido, portMAX_DELAY) != pdTRUE) {
        ESP_LOGI(TAG, "Erro ao enviar para a fila\n");
    }

    ESP_LOGI(TAG, "Data from the client: %.*s", ctxt->om->om_len, ctxt->om->om_data);
    ESP_LOGI(TAG, "Número convertido: %d\n", dado_recebido);
    
    return 0;
}

// Leitura 
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
    return 0;
}

// def serviços 
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),         // UUID device type

     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),   // UUID reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
          
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),   // UUID writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};


static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "Status conexão BLE GAP:  %s", event->connect.status == 0 ? "OK!" : "FALHA!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
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


void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); 
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // conexão
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // descoberta
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); 
    ble_app_advertise();                     
}

void host_task(void *param)
{
    nimble_port_run(); 
}

// Inicializa BLE
void init_ble(void){
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set(NOME_SERVER); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}
