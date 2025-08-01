#ifndef SERVER_BLE
#define SERVER_BLE

#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "config.h"


extern const char *TAG;
extern uint8_t ble_addr_type;
extern QueueHandle_t xQueue;

// Funções
void ble_app_advertise(void);
void init_ble(void);

#endif