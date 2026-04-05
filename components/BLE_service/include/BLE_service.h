/**
 * @file BLE_service.h
 * @brief Servicio BLE para control del cráneo interactivo
 */

#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// CONSTANTES PÚBLICAS
//==============================================================================

/** UUID del servicio personalizado del cráneo */
#define CRANEO_SVC_UUID 0x00FF

/** UUID de la característica de control de hueso (1-22) */
#define CRANEO_CHAR_HUESO_UUID 0xFF01

/** UUID de la característica de control del servo (0-180) */
#define CRANEO_CHAR_SERVO_UUID 0xFF02

/** Número de handles del servicio: 1 servicio + (2 características × 2) = 5 */
#define CRANEO_NUM_HANDLE 5

/** Nombre del dispositivo que aparecerá en el escaneo BLE */
#define BLE_DEVICE_NAME "CRANEO_INTERACTIVO"

    //==============================================================================
    // TIPOS DE DATOS
    //==============================================================================

    /**
     * @brief Callback para notificar comandos recibidos por BLE
     * @param comando Tipo de comando: 0x01 = hueso, 0x02 = servo
     * @param valor   Valor del comando (hueso: 1-22, servo: 0-180)
     */
    typedef void (*ble_command_callback_t)(uint8_t comando, uint8_t valor);

    //==============================================================================
    // FUNCIONES PÚBLICAS
    //==============================================================================

    /**
     * @brief Inicializa el servicio BLE completo
     * @return ESP_OK si éxito
     */
    esp_err_t ble_service_init(void);

    /**
     * @brief Registra un callback para recibir comandos BLE
     * @param callback Función que será llamada cuando se reciba un comando
     */
    void ble_service_register_callback(ble_command_callback_t callback);

    /**
     * @brief Verifica si hay un cliente conectado
     * @return true si hay conexión activa
     */
    bool ble_service_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SERVICE_H