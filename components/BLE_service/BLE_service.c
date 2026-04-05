/**
 * @file BLE_service.c
 * @brief Implementación del servicio BLE para cráneo interactivo
 *
 * Este archivo implementa un servidor GATT completo que expone:
 * - Un servicio personalizado (UUID 0x00FF)
 * - Dos características: HUESO (0xFF01) y SERVO (0xFF02)
 *
 * El flujo general es:
 * 1. Inicializar el stack Bluetooth (controlador + Bluedroid)
 * 2. Registrar callbacks para eventos GAP y GATTS
 * 3. Configurar advertising para que el dispositivo sea visible
 * 4. Crear el servicio y sus características
 * 5. Esperar conexiones y comandos de escritura
 */

// =============================================================================
// INCLUDES NECESARIOS
// =============================================================================

#include "BLE_service.h"         // Cabecera pública de este componente
#include <stdio.h>               // Funciones estándar de entrada/salida
#include <string.h>              // Funciones de manejo de memoria (memcpy, memset)
#include "freertos/FreeRTOS.h"   // Sistema operativo FreeRTOS
#include "freertos/task.h"       // Para tareas y retardos (vTaskDelay)
#include "esp_log.h"             // Para logs (ESP_LOGI, ESP_LOGE, etc.)
#include "nvs_flash.h"           // Almacenamiento no volátil (guarda claves de enlace BLE)
#include "esp_bt.h"              // Funciones base de Bluetooth
#include "esp_bt_main.h"         // Inicialización de Bluedroid (pila Bluetooth)
#include "esp_bt_device.h"       // Para obtener la dirección MAC del dispositivo
#include "esp_gap_ble_api.h"     // GAP: Advertising, conexiones, escaneo
#include "esp_gatts_api.h"       // GATT Server: creación de servicios y características
#include "esp_gatt_common_api.h" // Tipos comunes: permisos, propiedades, etc.

// =============================================================================
// CONSTANTES PRIVADAS
// =============================================================================

/**
 * @brief Etiqueta para los logs de este módulo
 * Aparecerá al inicio de cada mensaje: "BLE_SERVICE: mensaje"
 */
static const char *TAG = "BLE_SERVICE";

/**
 * @brief ID de la aplicación (único para el perfil)
 * Se usa para identificar este perfil GATT cuando se registra en el stack
 */
#define CRANEO_APP_ID 0x00

/**
 * @brief Bandera para control del advertising
 * Se usa para saber si el paquete de advertising ya fue configurado
 * Se combina con la bandera para scan response si se usara
 */
#define ADV_CONFIG_FLAG (1 << 0)

// =============================================================================
// ESTRUCTURAS PRIVADAS
// =============================================================================

/**
 * @brief Estructura que mantiene todo el estado del perfil GATT
 *
 * Esta estructura es el "corazón" del servidor. Guarda todos los handles
 * (identificadores) que el stack asigna a cada atributo, así como el
 * estado de la conexión.
 *
 * Los handles son números que el stack BLE asigna a cada atributo
 * (servicio, característica, descriptor) para poder referirse a ellos.
 */
typedef struct
{
    // ---------- Callback y estado base ----------
    esp_gatts_cb_t gatts_cb; // Función callback para eventos GATT de este perfil
    uint16_t gatts_if;       // Interfaz GATT asignada por el stack (identificador)
    uint16_t app_id;         // ID de la aplicación (CRANEO_APP_ID)
    uint16_t conn_id;        // ID de conexión (cuando un cliente se conecta)

    // ---------- Servicio ----------
    uint16_t service_handle;       // Handle del servicio (asignado por el stack)
    esp_gatt_srvc_id_t service_id; // Estructura con la definición del servicio (UUID, tipo)

    // ---------- Características ----------
    uint16_t char_hueso_handle; // Handle de la característica HUESO (escritura 1-22)
    uint16_t char_servo_handle; // Handle de la característica SERVO (escritura 0-180)

    // ---------- Para añadir características (variables temporales) ----------
    esp_bt_uuid_t char_uuid;       // UUID de la característica que estamos añadiendo
    esp_gatt_perm_t perm;          // Permisos de la característica (READ, WRITE, etc.)
    esp_gatt_char_prop_t property; // Propiedades (NOTIFY, INDICATE, WRITE, etc.)

    // ---------- Estado ----------
    bool connected;           // true si hay un cliente conectado
    uint8_t next_char_to_add; // 0 = añadiendo HUESO, 1 = añadiendo SERVO (no usado actualmente)

} craneo_gatts_profile_t;

// =============================================================================
// VARIABLES PRIVADAS
// =============================================================================

/**
 * @brief Instancia única del perfil (patrón singleton)
 *
 * Esta variable contiene todo el estado del servidor BLE.
 * Se inicializa con valores por defecto (ceros/null).
 * static significa que solo es visible dentro de este archivo .c
 */
static craneo_gatts_profile_t craneo_profile = {
    .gatts_cb = NULL,             // Callback aún no asignado
    .gatts_if = ESP_GATT_IF_NONE, // ESP_GATT_IF_NONE = -1, indica "no asignado"
    .app_id = CRANEO_APP_ID,      // Nuestro ID de aplicación
    .conn_id = 0,                 // Sin conexión activa
    .service_handle = 0,          // Handle del servicio (0 = no creado)
    .char_hueso_handle = 0,       // Handle de HUESO (0 = no creado)
    .char_servo_handle = 0,       // Handle de SERVO (0 = no creado)
    .connected = false,           // No hay cliente conectado
    .next_char_to_add = 0,        // Empezamos con la característica HUESO
};

/**
 * @brief Callback de usuario registrado desde main.c
 *
 * Esta función se llama cuando se recibe un comando por BLE.
 * El usuario la registra con ble_service_register_callback()
 */
static ble_command_callback_t user_callback = NULL;

/**
 * @brief Estado de la configuración del advertising
 *
 * Bit 0 (ADV_CONFIG_FLAG): 1 = falta configurar advertising, 0 = ya configurado
 * Cuando llega a 0, se inicia el advertising automáticamente
 */
static uint8_t adv_config_done = 0;

// =============================================================================
// DATOS DE ADVERTISING
// =============================================================================

/**
 * @brief Datos del paquete de advertising
 *
 * El paquete de advertising es lo que ven los teléfonos cuando escanean
 * antes de conectarse. Contiene información como el nombre del dispositivo.
 */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,                   // false = es advertising normal, no scan response
    .include_name = true,                    // Incluir el nombre del dispositivo en el paquete
    .include_txpower = false,                // No incluir la potencia de transmisión
    .min_interval = 0x0006,                  // Intervalo mínimo de advertising (6 * 0.625ms = 3.75ms)
    .max_interval = 0x0010,                  // Intervalo máximo de advertising (16 * 0.625ms = 10ms)
    .appearance = 0x00,                      // Apariencia del dispositivo (0 = desconocido)
    .manufacturer_len = 0,                   // Sin datos personalizados del fabricante
    .p_manufacturer_data = NULL,             // Sin datos del fabricante
    .service_data_len = 0,                   // Sin datos de servicio
    .p_service_data = NULL,                  // Sin datos de servicio
    .service_uuid_len = 0,                   // Sin UUIDs de servicio en el advertising
    .p_service_uuid = NULL,                  // Sin UUIDs de servicio
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC |     // Bandera: dispositivo de descubrimiento general
             ESP_BLE_ADV_FLAG_BREDR_NOT_SPT) // Bandera: no soporta Bluetooth Classic (solo BLE)
};

/**
 * @brief Parámetros del advertising
 *
 * Controla cómo se envía el paquete de advertising (frecuencia, tipo, etc.)
 */
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,                                    // Intervalo mínimo: 32 * 0.625ms = 20ms
    .adv_int_max = 0x40,                                    // Intervalo máximo: 64 * 0.625ms = 40ms
    .adv_type = ADV_TYPE_IND,                               // Advertising conectable (permitir conexiones)
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,                  // Usar dirección MAC pública del dispositivo
    .channel_map = ADV_CHNL_ALL,                            // Usar todos los 3 canales de advertising (37,38,39)
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // Cualquiera puede escanear y conectar
};

// =============================================================================
// DECLARACIONES DE FUNCIONES PRIVADAS
// =============================================================================

/**
 * @brief Prototipos de funciones para que el compilador las conozca antes de usarlas
 *
 * Estas funciones están definidas más abajo en el archivo.
 */
static void craneo_gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                               esp_gatt_if_t gatts_if,
                                               esp_ble_gatts_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);

// =============================================================================
// IMPLEMENTACIÓN DEL PERFIL GATT (EL MANEJADOR PRINCIPAL)
// =============================================================================

/**
 * @brief Manejador de eventos del perfil del cráneo
 *
 * Esta es la función más importante. El stack BLE la llama cada vez que ocurre
 * un evento relacionado con este perfil GATT.
 *
 * Los eventos principales que maneja son:
 * - ESP_GATTS_REG_EVT: la aplicación se registró correctamente
 * - ESP_GATTS_CREATE_EVT: el servicio fue creado
 * - ESP_GATTS_ADD_CHAR_EVT: una característica fue añadida
 * - ESP_GATTS_CONNECT_EVT: un cliente se conectó
 * - ESP_GATTS_DISCONNECT_EVT: un cliente se desconectó
 * - ESP_GATTS_WRITE_EVT: un cliente escribió en una característica
 * - ESP_GATTS_READ_EVT: un cliente leyó una característica
 *
 * @param event    Tipo de evento que ocurrió
 * @param gatts_if Interfaz GATT (identificador de la instancia)
 * @param param    Parámetros específicos del evento (contiene handles, datos, etc.)
 */
static void craneo_gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                               esp_gatt_if_t gatts_if,
                                               esp_ble_gatts_cb_param_t *param)
{
    // Puntero a nuestra estructura de estado (para escribir más corto)
    craneo_gatts_profile_t *p = &craneo_profile;
    esp_err_t ret; // Para almacenar códigos de retorno de funciones

    // Switch sobre el tipo de evento
    switch (event)
    {
    // -------------------------------------------------------------------------
    // EVENTO: Registro de la aplicación completado
    // Ocurre después de llamar a esp_ble_gatts_app_register()
    // -------------------------------------------------------------------------
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "REG_EVT: app_id %d, status %d", param->reg.app_id, param->reg.status);

        // Guardar la interfaz GATT asignada por el stack
        p->gatts_if = gatts_if;

        // Configurar la estructura del servicio (todavía no se crea, solo se define)
        p->service_id.is_primary = true;                     // Servicio primario (no incluido dentro de otro)
        p->service_id.id.inst_id = 0x00;                     // Instancia 0 (solo una instancia de este servicio)
        p->service_id.id.uuid.len = ESP_UUID_LEN_16;         // UUID de 16 bits
        p->service_id.id.uuid.uuid.uuid16 = CRANEO_SVC_UUID; // UUID 0x00FF

        // Configurar los datos de advertising (el nombre del dispositivo, etc.)
        ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)
        {
            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
            break; // Si falla, salimos del switch
        }

        // Crear el servicio en el stack BLE
        // Parámetros: interfaz, estructura del servicio, número de handles a reservar
        esp_ble_gatts_create_service(gatts_if, &p->service_id, CRANEO_NUM_HANDLE);
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Servicio creado exitosamente
    // Ocurre después de esp_ble_gatts_create_service()
    // -------------------------------------------------------------------------
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "CREATE_EVT: status %d, service_handle %d",
                 param->create.status, param->create.service_handle);

        // Guardar el handle del servicio (lo necesitaremos para añadir características)
        p->service_handle = param->create.service_handle;

        // Iniciar el servicio (activarlo para que los clientes puedan verlo)
        esp_ble_gatts_start_service(p->service_handle);

        // Configurar la primera característica: HUESO (UUID 0xFF01)
        p->char_uuid.len = ESP_UUID_LEN_16;                // UUID de 16 bits
        p->char_uuid.uuid.uuid16 = CRANEO_CHAR_HUESO_UUID; // 0xFF01
        p->perm = ESP_GATT_PERM_WRITE;                     // Permiso: el cliente puede escribir
        p->property = ESP_GATT_CHAR_PROP_BIT_WRITE;        // Propiedad: soporta escritura

        // Añadir la característica al servicio
        // Parámetros: handle del servicio, UUID, permisos, propiedades, valor inicial, control
        ret = esp_ble_gatts_add_char(p->service_handle, &p->char_uuid,
                                     p->perm, p->property,
                                     NULL, NULL);
        if (ret)
        {
            ESP_LOGE(TAG, "add char HUESO failed, error code = %x", ret);
        }
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Característica añadida exitosamente
    // Ocurre después de esp_ble_gatts_add_char()
    // -------------------------------------------------------------------------
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(TAG, "ADD_CHAR_EVT: attr_handle %d, char_uuid %x",
                 param->add_char.attr_handle, param->add_char.char_uuid.uuid.uuid16);

        // Verificar qué característica se acaba de añadir (por su UUID)
        if (param->add_char.char_uuid.uuid.uuid16 == CRANEO_CHAR_HUESO_UUID)
        {
            // Es la característica HUESO: guardar su handle
            p->char_hueso_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Característica HUESO creada, handle: %d", p->char_hueso_handle);

            // Ahora añadir la segunda característica: SERVO (UUID 0xFF02)
            p->char_uuid.uuid.uuid16 = CRANEO_CHAR_SERVO_UUID;
            ret = esp_ble_gatts_add_char(p->service_handle, &p->char_uuid,
                                         ESP_GATT_PERM_WRITE,
                                         ESP_GATT_CHAR_PROP_BIT_WRITE,
                                         NULL, NULL);
            if (ret)
            {
                ESP_LOGE(TAG, "add char SERVO failed, error code = %x", ret);
            }
        }
        else if (param->add_char.char_uuid.uuid.uuid16 == CRANEO_CHAR_SERVO_UUID)
        {
            // Es la característica SERVO: guardar su handle
            p->char_servo_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "Característica SERVO creada, handle: %d", p->char_servo_handle);
        }
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Cliente conectado
    // Ocurre cuando un teléfono (u otro dispositivo) se conecta a nuestro ESP32
    // -------------------------------------------------------------------------
    case ESP_GATTS_CONNECT_EVT:
        p->conn_id = param->connect.conn_id; // Guardar ID de conexión
        p->connected = true;                 // Marcar como conectado
        ESP_LOGI(TAG, "Cliente conectado, conn_id: %d, remote: " ESP_BD_ADDR_STR,
                 p->conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));

        // Actualizar parámetros de conexión para mejor latencia
        // Esto optimiza la velocidad de transmisión de datos
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        conn_params.latency = 0;    // Sin latencia adicional
        conn_params.max_int = 0x20; // Intervalo máximo de conexión
        conn_params.min_int = 0x10; // Intervalo mínimo de conexión
        conn_params.timeout = 400;  // Timeout de conexión
        esp_ble_gap_update_conn_params(&conn_params);
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Cliente desconectado
    // Ocurre cuando el teléfono se desconecta (o la conexión se pierde)
    // -------------------------------------------------------------------------
    case ESP_GATTS_DISCONNECT_EVT:
        p->connected = false; // Marcar como desconectado
        ESP_LOGI(TAG, "Cliente desconectado, razón: 0x%02x", param->disconnect.reason);

        // Reiniciar el advertising para que otros dispositivos puedan conectarse
        esp_ble_gap_start_advertising(&adv_params);
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Escritura en una característica
    // Este es el evento MÁS IMPORTANTE para tu proyecto.
    // Aquí es donde recibes los comandos desde la app Flutter.
    // -------------------------------------------------------------------------
    case ESP_GATTS_WRITE_EVT:
        // Log de depuración (solo se muestra si el nivel de log es DEBUG)
        ESP_LOGD(TAG, "WRITE_EVT: handle %d, len %d", param->write.handle, param->write.len);
        ESP_LOG_BUFFER_HEX(TAG, param->write.value, param->write.len);

        // Identificar qué característica fue escrita (por su handle)
        if (param->write.handle == p->char_hueso_handle && param->write.len >= 1)
        {
            // Comando para encender un hueso
            uint8_t hueso = param->write.value[0]; // El primer byte es el número del hueso
            ESP_LOGI(TAG, "Comando HUESO recibido: %d", hueso);

            // Llamar al callback del usuario (definido en main.c)
            if (user_callback)
            {
                user_callback(0x01, hueso); // 0x01 = comando hueso
            }
        }
        else if (param->write.handle == p->char_servo_handle && param->write.len >= 1)
        {
            // Comando para mover el servo de la mandíbula
            uint8_t angulo = param->write.value[0]; // El primer byte es el ángulo (0-180)
            ESP_LOGI(TAG, "Comando SERVO recibido: %d", angulo);

            // Llamar al callback del usuario
            if (user_callback)
            {
                user_callback(0x02, angulo); // 0x02 = comando servo
            }
        }

        // Si el cliente espera una respuesta (need_rsp = true), se la enviamos
        // Esto confirma que el dato fue recibido correctamente
        if (param->write.need_rsp)
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                        param->write.trans_id, ESP_GATT_OK, NULL);
        }
        break;

    // -------------------------------------------------------------------------
    // EVENTO: Lectura de una característica
    // Ocurre cuando el cliente lee el valor de una característica
    // (En este proyecto no es esencial, pero lo manejamos por completitud)
    // -------------------------------------------------------------------------
    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "READ_EVT: handle %d", param->read.handle);

        // Si el cliente espera una respuesta, enviamos un valor por defecto
        if (param->read.need_rsp)
        {
            esp_gatt_rsp_t rsp = {0};                   // Estructura de respuesta vacía
            rsp.attr_value.handle = param->read.handle; // Handle que se leyó
            rsp.attr_value.len = 0;                     // Sin datos (valor vacío)
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id,
                                        param->read.trans_id, ESP_GATT_OK, &rsp);
        }
        break;

    // -------------------------------------------------------------------------
    // EVENTOS NO UTILIZADOS (no necesitamos manejarlos)
    // -------------------------------------------------------------------------
    default:
        // Cualquier otro evento se ignora (no es necesario hacer nada)
        break;
    }
}

// =============================================================================
// MANEJADOR GAP (ADVERTISING Y CONEXIONES)
// =============================================================================

/**
 * @brief Manejador de eventos GAP (Generic Access Profile)
 *
 * GAP controla la capa de acceso genérico: advertising, escaneo, conexiones.
 * Este manejador se encarga de:
 * - Confirmar que los datos de advertising se configuraron correctamente
 * - Iniciar el advertising cuando todo está listo
 * - Reportar si el advertising comenzó bien o falló
 *
 * @param event    Tipo de evento GAP
 * @param param    Parámetros del evento
 */
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    // Evento: Datos de advertising configurados correctamente
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG, "ADV_DATA_SET_COMPLETE_EVT, status %d", param->adv_data_cmpl.status);

        // Limpiar la bandera de advertising pendiente
        adv_config_done &= (~ADV_CONFIG_FLAG);

        // Si ya no hay configuraciones pendientes, iniciar el advertising
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    // Evento: Advertising iniciado (o falló)
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
        }
        else
        {
            ESP_LOGI(TAG, "Advertising started successfully");
        }
        break;

    // Otros eventos GAP no son necesarios para este proyecto
    default:
        break;
    }
}

// =============================================================================
// MANEJADOR GATT PRINCIPAL (DISTRIBUIDOR DE EVENTOS)
// =============================================================================

/**
 * @brief Manejador principal de eventos GATT
 *
 * Esta función recibe TODOS los eventos GATT y los distribuye al perfil
 * correspondiente. Como solo tenemos un perfil (el cráneo), simplemente
 * lo reenvía a nuestro manejador específico.
 *
 * @param event    Tipo de evento GATT
 * @param gatts_if Interfaz GATT
 * @param param    Parámetros del evento
 */
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    // Evento de registro de aplicación
    if (event == ESP_GATTS_REG_EVT)
    {
        // Verificar si el registro fue exitoso
        if (param->reg.status == ESP_GATT_OK)
        {
            craneo_profile.gatts_if = gatts_if; // Guardar la interfaz
        }
        else
        {
            ESP_LOGI(TAG, "Reg app failed, app_id %04x, status %d",
                     param->reg.app_id, param->reg.status);
            return;
        }
    }

    // Si el callback del perfil aún no está asignado, asignarlo
    if (craneo_profile.gatts_cb == NULL)
    {
        craneo_profile.gatts_cb = craneo_gatts_profile_event_handler;
    }

    // Llamar al callback del perfil con el evento recibido
    craneo_profile.gatts_cb(event, gatts_if, param);
}

// =============================================================================
// FUNCIONES PÚBLICAS (API visible desde main.c)
// =============================================================================

/**
 * @brief Inicializa el servicio BLE completo
 *
 * Esta función debe ser llamada desde app_main() una sola vez.
 * Realiza todos los pasos necesarios para inicializar el stack Bluetooth:
 * 1. Inicializa NVS (necesario para guardar claves de enlace)
 * 2. Libera memoria de Bluetooth Classic (solo usamos BLE)
 * 3. Inicializa y habilita el controlador Bluetooth (hardware)
 * 4. Inicializa y habilita Bluedroid (pila de protocolos)
 * 5. Configura el nombre del dispositivo
 * 6. Registra los callbacks GAP y GATTS
 * 7. Registra la aplicación GATT
 *
 * @return ESP_OK si éxito, otro valor si error
 */
esp_err_t ble_service_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Inicializando servicio BLE...");

    // -------------------------------------------------------------------------
    // 1. Inicializar NVS (Non-Volatile Storage)
    // El stack BLE necesita NVS para guardar claves de enlace y configuración
    // -------------------------------------------------------------------------
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // Si NVS está corrupto o es una versión nueva, borrar y reinicializar
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); // Si falla, el programa se detiene aquí

    // -------------------------------------------------------------------------
    // 2. Liberar memoria de Bluetooth Classic (no lo usamos)
    // El ESP32 tiene memoria compartida entre Classic BT y BLE.
    // Al liberar Classic BT, ganamos más memoria para BLE.
    // -------------------------------------------------------------------------
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // -------------------------------------------------------------------------
    // 3. Inicializar el controlador Bluetooth (el hardware)
    // -------------------------------------------------------------------------
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar controlador BT: %s", esp_err_to_name(ret));
        return ret;
    }

    // Habilitar el controlador en modo BLE (no Classic BT)
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al habilitar controlador BT: %s", esp_err_to_name(ret));
        return ret;
    }

    // -------------------------------------------------------------------------
    // 4. Inicializar Bluedroid (la pila de protocolos Bluetooth)
    // -------------------------------------------------------------------------
    esp_bluedroid_config_t cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al habilitar bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    // -------------------------------------------------------------------------
    // 5. Configurar el nombre del dispositivo (visible en el escaneo)
    // -------------------------------------------------------------------------
    esp_ble_gap_set_device_name(BLE_DEVICE_NAME);

    // -------------------------------------------------------------------------
    // 6. Registrar los callbacks GAP y GATTS
    // -------------------------------------------------------------------------
    // GAP: para advertising y conexiones
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al registrar callback GAP: %s", esp_err_to_name(ret));
        return ret;
    }

    // GATTS: para el servidor GATT
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al registrar callback GATTS: %s", esp_err_to_name(ret));
        return ret;
    }

    // -------------------------------------------------------------------------
    // 7. Registrar la aplicación GATT
    // Esto inicia el proceso de creación del perfil
    // -------------------------------------------------------------------------
    ret = esp_ble_gatts_app_register(CRANEO_APP_ID);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al registrar app GATT: %s", esp_err_to_name(ret));
        return ret;
    }

    // -------------------------------------------------------------------------
    // 8. Configurar el MTU (Maximum Transmission Unit)
    // Un MTU más grande permite enviar más datos por paquete.
    // 500 bytes es un buen valor para la mayoría de los casos.
    // -------------------------------------------------------------------------
    esp_ble_gatt_set_local_mtu(500);

    // -------------------------------------------------------------------------
    // 9. Logs de confirmación
    // -------------------------------------------------------------------------
    ESP_LOGI(TAG, "Servicio BLE inicializado correctamente");
    ESP_LOGI(TAG, "Nombre del dispositivo: %s", BLE_DEVICE_NAME);
    ESP_LOGI(TAG, "UUID del servicio: 0x%04X", CRANEO_SVC_UUID);
    ESP_LOGI(TAG, "Características: HUESO=0x%04X, SERVO=0x%04X",
             CRANEO_CHAR_HUESO_UUID, CRANEO_CHAR_SERVO_UUID);

    return ESP_OK;
}

/**
 * @brief Registra un callback para recibir comandos BLE
 *
 * Esta función permite que main.c reciba notificaciones cuando se recibe
 * un comando por BLE. El callback recibe:
 * - comando: 0x01 para hueso, 0x02 para servo
 * - valor: el dato escrito (hueso 1-22, ángulo 0-180)
 *
 * @param callback Función que será llamada cuando se reciba un comando
 */
void ble_service_register_callback(ble_command_callback_t callback)
{
    user_callback = callback;
    ESP_LOGI(TAG, "Callback registrado");
}

/**
 * @brief Verifica si hay un cliente conectado
 *
 * @return true si hay un cliente conectado, false si no
 */
bool ble_service_is_connected(void)
{
    return craneo_profile.connected;
}