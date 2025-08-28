#include "client_ble.h" 
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"


// ------------ Servidor Alvo ------------
#define TARGET_DEVICE_NAME   "Car_BLE_Server" 
#define SERVICE_UUID         0x0180
#define CHARACTERISTIC_UUID  0xDEAD
// ------------ Servidor Alvo ------------


static const char* TAG = "NIMBLE_CLIENTE_MOD";

// Variáveis de estado
static volatile uint16_t conn_handle;
static uint16_t service_start_handle;
static uint16_t service_end_handle;
static uint16_t char_handle;
static bool device_found = false;
static volatile bool g_is_connected = false;

// Protótipos
static void start_scan(void);
static int on_char_discovered(uint16_t conn_handle_cb, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg);
static int on_service_discovered(uint16_t conn_handle_cb, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg);
static int ble_gap_event_handler(struct ble_gap_event *event, void *arg);
static void ble_app_on_sync(void);
static void host_task(void *param);


static int on_char_discovered(uint16_t conn_handle_cb, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg) {
    if (error->status == 0 && chr != NULL) {
        if (ble_uuid_u16(&chr->uuid.u) == CHARACTERISTIC_UUID) {
            ESP_LOGI(TAG, "Característica alvo encontrada! Handle: %d", chr->val_handle);
            char_handle = chr->val_handle;
        }
    }
    if (error->status != 0 || chr == NULL) {
        if (char_handle != 0) {
            ESP_LOGI(TAG, "Pronto para enviar dados!");
        } else {
            ESP_LOGE(TAG, "Característica alvo não encontrada.");
        }
    }
    return 0;
}

static int on_service_discovered(uint16_t conn_handle_cb, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg) {
    if (error->status == 0 && service != NULL) {
        if (ble_uuid_u16(&service->uuid.u) == SERVICE_UUID) {
            service_start_handle = service->start_handle;
            service_end_handle = service->end_handle;
        }
    }
    if (error->status != 0 || service == NULL) {
        if (service_start_handle != 0) {
            ble_gattc_disc_all_chrs(conn_handle_cb, service_start_handle, service_end_handle, on_char_discovered, NULL);
        } else {
             ESP_LOGE(TAG, "Serviço alvo não encontrado.");
        }
    }
    return 0;
}

static int ble_gap_event_handler(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_DISC:
            struct ble_hs_adv_fields fields;
            if (ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data) == 0) {
                if (fields.name != NULL && fields.name_len > 0 &&
                    (strncmp((char *)fields.name, TARGET_DEVICE_NAME, fields.name_len) == 0 || strncmp((char *)fields.name, "Meu_Server", fields.name_len) == 0) ) {
                    if(device_found) return 0;
                    device_found = true;
                    ESP_LOGI(TAG, "Dispositivo alvo '%s' encontrado!", (char*)fields.name);
                    ble_gap_disc_cancel();
                    ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr, 1000, NULL, ble_gap_event_handler, NULL);
                }
            }
            break;

        case BLE_GAP_EVENT_CONNECT:
            // <<< MUDANÇA 2: Atualizamos nossa flag de estado aqui
            if (event->connect.status == 0) {
                g_is_connected = true; // ESTAMOS CONECTADOS
                conn_handle = event->connect.conn_handle;
                ESP_LOGI(TAG, "Conectado com sucesso! Handle: %d", conn_handle);
                ble_gattc_disc_all_svcs(conn_handle, on_service_discovered, NULL);
            } else {
                g_is_connected = false; // NÃO ESTAMOS CONECTADOS
                ESP_LOGE(TAG, "Falha na conexão, status=%d.", event->connect.status);
                start_scan();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            g_is_connected = false; // NÃO ESTAMOS MAIS CONECTADOS
            ESP_LOGW(TAG, "Desconectado. Motivo: %d.", event->disconnect.reason);
            conn_handle = 0;
            service_start_handle = 0;
            char_handle = 0;
            device_found = false;
            start_scan();
            break;
            
        default:
            break;
    }
    return 0;
}

static void start_scan(void) {
    struct ble_gap_disc_params disc_params = {0};
    disc_params.passive = 1;
    disc_params.filter_duplicates = 1;
    ESP_LOGI(TAG, "Iniciando escaneamento...");
    ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, ble_gap_event_handler, NULL);
}

static void ble_app_on_sync(void) {
    start_scan();
}

static void host_task(void *param) {
    nimble_port_run();
}

// --- Funções Públicas ---

bool ble_client_is_connected(void) {
    // <<< MUDANÇA 4: A verificação agora é na nossa flag de estado
    return g_is_connected;
}

bool ble_client_write(const char* data) {
    if (!ble_client_is_connected()) {
        ESP_LOGE(TAG, "Falha ao escrever: Não conectado.");
        return false;
    }
    if (char_handle == 0) {
        ESP_LOGE(TAG, "Falha ao escrever: Handle da característica desconhecido.");
        return false;
    }
    int rc = ble_gattc_write_flat(conn_handle, char_handle, data, strlen(data), NULL, NULL);
    if (rc == 0) {
        ESP_LOGI(TAG, "Enviando dados: '%s'", data);
        return true;
    } else {
        ESP_LOGE(TAG, "Falha ao enviar dados; erro=%d", rc);
        return false;
    }
}

void ble_client_init(void) {
    nimble_port_init();
    ble_svc_gap_device_name_set("ESP32-NIMBLE-CLIENT");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}
