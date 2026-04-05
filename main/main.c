/**
 * @file main.c
 * @brief Codigo fuente el orquestador
 * @author Rodrigo C.C
 *
 * Este programa hace la implementacion del proyecto uniendo todos los
 * componentes.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "BLE_service.h"
#include "indicador_led.h"
#include "servo_mandibula.h"

static const char *TAG_CRANEO = "Principal";

// Callback que recibe los comandos BLE
static void ble_command_handler(uint8_t comando, uint8_t valor)
{
    switch (comando)
    {
    case 0x01:
        if (valor >= 1 && valor <= 22)
        {
            // Log con el nombre del hueso (mucho más legible)
            ESP_LOGI(TAG_CRANEO, "📱 Recibido: %s (hueso %d)",
                     NOMBRE_HUESOS[valor], valor);
            // Encender el LED
            indicador_led_encender_hueso(valor);
        }
        else
        {
            ESP_LOGW(TAG_CRANEO, "Hueso inválido: %d (debe ser 1-22)", valor);
        }
        break;
    case 0x02:
        if (valor >= 1 && valor <= 180)
        {
            ESP_LOGI(TAG_CRANEO, "📱 Recibido: Mover mandíbula a %d°", valor);
            control_angulo_servo(valor);
        }
        else
        {
            ESP_LOGW(TAG_CRANEO, "Ángulo inválido: %d (debe ser 0-180)", valor);
        }

        break;

    default:
        ESP_LOGW(TAG_CRANEO, "Comando desconocido: 0x%02X", comando);
        break;
    }
}

void app_main(void)
{
    // Inicializando los LEDS ws2812
    esp_err_t err_init = led_ws2812_init();
    if (err_init != ESP_OK)
    {
        ESP_LOGE(TAG_CRANEO, "Error al inicializar");
        return;
    }
    ESP_LOGI(TAG_CRANEO, "Inicializacion exitosa!!");

    //  Inicializacion del serovo de manibula
    esp_err_t err_init_servo = mandibula_servo_init();
    if (err_init_servo != ESP_OK)
    {
        ESP_LOGE(TAG_CRANEO, "Error al inicializar el servo: %s", esp_err_to_name(err_init_servo));
        return;
    }
    ESP_LOGI(TAG_CRANEO, "Servo mandibula inicializado");
    control_angulo_servo(0); // Inicializamos el angulo del servo en 0 grados

    // Inicializar BLE
    esp_err_t ret = ble_service_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_CRANEO, "❌ Error al inicializar BLE: %s", esp_err_to_name(ret));
        return;
    }

    // Registrar callback
    ble_service_register_callback(ble_command_handler);
    ESP_LOGI(TAG_CRANEO, "");
    ESP_LOGI(TAG_CRANEO, "✅ BLE inicializado correctamente");
}
/*#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "BLE_service.h"

static const char *TAG = "TEST_BLE";

// Callback que recibe los comandos BLE
static void ble_command_handler(uint8_t comando, uint8_t valor)
{
    switch (comando)
    {
    case 0x01: // Comando HUESO
        ESP_LOGI(TAG, "✅ COMANDO RECIBIDO - HUESO: %d", valor);
        break;

    case 0x02: // Comando SERVO
        ESP_LOGI(TAG, "✅ COMANDO RECIBIDO - SERVO: %d grados", valor);
        break;

    default:
        ESP_LOGW(TAG, "Comando desconocido: 0x%02X", comando);
        break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "   PRUEBA DEL COMPONENTE BLE_SERVICE");
    ESP_LOGI(TAG, "========================================");

    // Inicializar BLE
    esp_err_t ret = ble_service_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "❌ Error al inicializar BLE: %s", esp_err_to_name(ret));
        return;
    }

    // Registrar callback
    ble_service_register_callback(ble_command_handler);

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✅ BLE inicializado correctamente");
    ESP_LOGI(TAG, "   Nombre del dispositivo: %s", BLE_DEVICE_NAME);
    ESP_LOGI(TAG, "   UUID del servicio: 0x%04X", CRANEO_SVC_UUID);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "📱 Conéctate desde nRF Connect y escribe:");
    ESP_LOGI(TAG, "   - UUID 0x%04X (HUESO): escribe 1-22", CRANEO_CHAR_HUESO_UUID);
    ESP_LOGI(TAG, "   - UUID 0x%04X (SERVO): escribe 0-180", CRANEO_CHAR_SERVO_UUID);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🔵 Esperando comandos BLE...");

    // El sistema queda en espera de eventos BLE
    // No es necesario un bucle while
}
*/
/*void app_main(void)
{
    // Inicializacion del serovo de manibula
    esp_err_t err_init_servo = mandibula_servo_init();
    if (err_init_servo != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar el servo: %s", esp_err_to_name(err_init_servo));
        return;
    }
    ESP_LOGI(TAG, "Servo mandibula inicializado");
    while (1)
    {
        for (int i = 0; i <= 180; i++)
        {
            control_angulo_servo(i);
            ESP_LOGI(TAG, "Angulo servo subida: %d", i);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        for (int i = 180; i >= 0; i--)
        {
            control_angulo_servo(i);
            ESP_LOGI(TAG, "Angulo servo bajada: %d", i);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}*/
/*void app_main(void)
{
    esp_err_t err_init = led_ws2812_init();
    if (err_init != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar");
        return;
    }
    ESP_LOGI(TAG, "Inicializacion exitosa!!");
    while (1)
    {
        for (int i = 0; i <= TOTAL_HUESOS; i++)
        {
            indicador_led_encender_hueso(i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}*/
