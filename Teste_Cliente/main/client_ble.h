#ifndef CLIENT_BLE_H
#define CLIENT_BLE_H

#include <stdbool.h>

/**
 * @brief Inicializa e inicia o cliente BLE.
 * * Esta função configura o stack NimBLE, registra os callbacks
 * e inicia a tarefa do host Bluetooth em background.
 */
void ble_client_init(void);

/**
 * @brief Verifica se o cliente BLE está conectado a um servidor.
 * * @return true Se estiver conectado.
 * @return false Se não estiver conectado.
 */
bool ble_client_is_connected(void);

/**
 * @brief Envia uma string de dados para a característica do servidor.
 * * A função só tentará enviar se o cliente estiver conectado e a 
 * característica correta já tiver sido descoberta.
 * * @param data Ponteiro para a string a ser enviada.
 * @return true Se a solicitação de escrita foi enviada com sucesso.
 * @return false Se não estava conectado ou se houve um erro.
 */
bool ble_client_write(const char* data);

/**
 * @brief Força a desconexão e reinicia o processo de escaneamento.
 */
void ble_client_force_reset(void);

#endif // CLIENT_BLE_H