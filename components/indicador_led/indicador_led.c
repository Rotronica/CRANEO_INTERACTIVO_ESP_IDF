#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "indicador_led.h"
#include "led_strip.h"
#include "servo_mandibula.h"

#define SALIDA_WS2812 GPIO_NUM_4 // Salida GPIO para LEDS WS2812
#define INDICADOR_BLE_ON 0       // Es el numero del led el cual se encendera para el indicador de conexion BLE
#define MAX_LEDS_CONECT 12
static const char *TAG = "LED_WS2812";
static led_strip_handle_t led_strip = NULL;
static bool modo_indicador_activo = false; // Variable para el indicador de conexion con el ble

// Definición del array de nombres (se guarda en FLASH con const)
const char *NOMBRE_HUESOS[] = {
    [0] = "INVÁLIDO",
    [1] = "Frontal",
    [2] = "Parietal Derecho",
    [3] = "Parietal Izquierdo",
    [4] = "Temporal Derecho",
    [5] = "Temporal Izquierdo",
    [6] = "Occipital",
    [7] = "Esfenoides",
    [8] = "Etmoides",
    [9] = "Mandíbula",
    [10] = "Maxilar Derecho",
    [11] = "Maxilar Izquierdo",
    [12] = "Cigomático Derecho",
    [13] = "Cigomático Izquierdo",
    [14] = "Nasal Derecho",
    [15] = "Nasal Izquierdo",
    [16] = "Lagrimal Derecho",
    [17] = "Lagrimal Izquierdo",
    [18] = "Palatino Derecho",
    [19] = "Palatino Izquierdo",
    [20] = "Cornete Inferior Derecho",
    [21] = "Cornete Inferior Izquierdo",
    [22] = "Vómer"};

// Tabla de colores constante - se guarda en FLASH, no en RAM
static const color_rgb_t HUESO_COLORS[] = {
    [HUESO_FRONTAL] = {255, 220, 180},              // Amarillo cálido
    [HUESO_PARETAL_DERECHO] = {255, 0, 0},          // Rojo
    [HUESO_PARETAL_IZQUIERDO] = {0, 255, 0},        // Verde
    [HUESO_TEMPORAL_DERECHO] = {0, 0, 255},         // Azul
    [HUESO_TEMPORAL_IZQUIERDO] = {0, 255, 255},     // Cian
    [HUESO_OCCIPITAL] = {255, 0, 255},              // Magenta
    [HUESO_ESFENOIDES] = {255, 165, 0},             // Naranja
    [HUESO_ETMOIDES] = {255, 255, 0},               // Amarillo
    [HUESO_MANDIBULA] = {200, 0, 0},                // Rojo intenso
    [HUESO_MAXILAR_DERECHO] = {255, 105, 180},      // Rosa
    [HUESO_MAXILAR_IZQUIERDO] = {255, 182, 193},    // Rosa claro
    [HUESO_CIGOMATICO_DERECHO] = {128, 0, 128},     // Púrpura
    [HUESO_CIGOMATICO_IZQUIERDO] = {238, 130, 238}, // Violeta
    [HUESO_NASAL_DERECHO] = {205, 133, 63},         // Marrón claro
    [HUESO_NASAL_IZQUIERDO] = {160, 82, 45},        // Marrón
    [HUESO_LAGRIMAL_DERECHO] = {127, 255, 212},     // Aguamarina
    [HUESO_LAGRIMAL_IZQUIERDO] = {64, 224, 208},    // Turquesa
    [HUESO_PALATINO_DERECHO] = {255, 215, 0},       // Oro
    [HUESO_PALATINO_IZQUIERDO] = {192, 192, 192},   // Plata
    [HUESO_CORNETE_DERECHO] = {250, 128, 114},      // Salmón
    [HUESO_CORNETE_IZQUIERDO] = {255, 127, 80},     // Coral
    [HUESO_VOMER] = {255, 248, 220},                // Blanco hueso
};

void indicador_modo_normal(void);

esp_err_t led_ws2812_init(void)
{
    // Configuración completa del LED strip (siguiendo la documentación)
    led_strip_config_t strip_config = {
        .strip_gpio_num = SALIDA_WS2812,
        .max_leds = MAX_LEDS_CONECT,                                 // 22 huesos del cráneo
        .led_model = LED_MODEL_WS2812,                               // ← OBLIGATORIO: define temporización
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // ← OBLIGATORIO: WS2812 usa GRB
        .flags = {
            .invert_out = false, // ← No invertir señal (a menos que uses inversor)
        }};

    // Configuración del backend RMT (recomendada para WS2812)
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // ← Fuente de reloj por defecto
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz (estándar para WS2812)
        .mem_block_symbols = 64,           // ← Tamaño de memoria del canal RMT
        .flags = {
            .with_dma = false, // ← ¡CRÍTICO! DMA para 22 LEDs en ESP32-S3(Un esp32 normal No tiene esta funcionalidad)
        }};

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falla al configurar LED Strip: %s", esp_err_to_name(ret));
        return ret;
    }

    // Apagar todos los LEDs al iniciar
    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);

    ESP_LOGI(TAG, "LED Strip configurado para: %d LEDs", MAX_LEDS_CONECT);
    return ESP_OK;
}

void indicador_led_encender_hueso(uint8_t hueso)
{
    uint8_t indice = hueso - 1; // este es el indice para el led el cual encendera led por led
    uint8_t r, g, b;

    led_strip_clear(led_strip); // ← ¡CRÍTICO! Apaga todo antes de encender el nuevo

    if (hueso >= 1 && hueso <= TOTAL_HUESOS)
    {
        r = HUESO_COLORS[hueso].r;
        g = HUESO_COLORS[hueso].g;
        b = HUESO_COLORS[hueso].b;
    }
    else
    {
        r = 255;
        g = 255;
        b = 255; // Default blanco
    }

    led_strip_set_pixel(led_strip, indice, r, g, b);
    led_strip_refresh(led_strip);

    ESP_LOGI(TAG, "Hueso %d encendido - Color RGB(%d,%d,%d)", hueso, r, g, b);
}
void indicador_modo_conectado(bool activar)
{
    if (activar)
    {
        // Detener modo buscando
        modo_indicador_activo = false;
        led_strip_clear(led_strip); // Apagar todo los leds
        control_angulo_servo(0);    // Inicializamos el angulo del servo en 0 grados

        // Encender LED indicador en verde fijo
        led_strip_set_pixel(led_strip, INDICADOR_BLE_ON, 0, 255, 0); // Verde
        led_strip_refresh(led_strip);

        ESP_LOGI(TAG, "Modo CONECTADO activado - LED indicador verde fijo");

        // Esperar 2 segundos y luego apagar para modo normal
        vTaskDelay(pdMS_TO_TICKS(2000));
        indicador_modo_normal();
    }
}
void indicador_modo_desconectado(bool activar)
{
    if (activar)
    {
        ESP_LOGI(TAG, "LLamada a modo desconexion bluetooth");
        led_strip_clear(led_strip); // Apagar todo los leds
        control_angulo_servo(0);    // Inicializamos el angulo del servo en 0 grados
    }
}
void indicador_modo_normal(void)
{
    // Detener modo indicador
    modo_indicador_activo = false;

    // Apagar solo el LED indicador (no todos los LEDs)
    led_strip_set_pixel(led_strip, INDICADOR_BLE_ON, 0, 0, 0);
    led_strip_refresh(led_strip);

    ESP_LOGI(TAG, "Modo NORMAL activado - LED indicador apagado");
}