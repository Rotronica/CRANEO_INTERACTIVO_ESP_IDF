#include <stdio.h>
#include "servo_mandibula.h"
#include "iot_servo.h"
#include "esp_log.h"

static const char *TAG = "SERVO";
esp_err_t mandibula_servo_init(void)
{
    // inicializa la configuracion del servomotor
    servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin = {
                SERVO_GPIO,
            },
            .ch = {
                LEDC_CHANNEL_0,
            },
        },
        .channel_number = 1,
    };

    // Initialize the servo
    esp_err_t err_servo = iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);
    if (err_servo != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al configurar servo %s", esp_err_to_name(err_servo));
    }
    else
    {
        ESP_LOGI(TAG, "Servo configurado");
    }

    return ESP_OK;
}
void control_angulo_servo(int angulo)
{
    esp_err_t err_serv_write = iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, angulo);
    if (err_serv_write != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al enviar angulos: %s", esp_err_to_name(err_serv_write));
        return;
    }
}