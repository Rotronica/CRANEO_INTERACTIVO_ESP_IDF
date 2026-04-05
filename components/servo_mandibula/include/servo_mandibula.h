/**
 * @file servo_mandibula.h
 * @brief Realiza el control de la mandibula del craneo mediante un servo
 * @version 1.0.0
 * @date 03 de abril de 2026
 *
 * Control de la mandibula mediante un servo motor
 *
 * @note [notas importantes para el usuario]
 * @warning fase de prueba
 * @author Rodrigo C.C
 */

#ifndef SERVO_MANDIBULA_H
#define SERVO_MANDIBULA_H

//==============================================================================
// INCLUDES NECESARIOS
//==============================================================================
// [Incluye aquí los headers que necesitas]
// #include <stdint.h>
#include "esp_err.h"

//==============================================================================
// SOPORTE PARA C++
//==============================================================================
#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// CONSTANTES Y MACROS
//==============================================================================
// [Define aquí tus constantes]
#define SERVO_GPIO GPIO_NUM_18

    //==============================================================================
    // ENUMERACIONES
    //==============================================================================
    // [Define aquí tus enums]
    // typedef enum {
    //     MI_OPCION_1,  ///< [descripción]
    //     MI_OPCION_2   ///< [descripción]
    // } mi_opcion_t;

    //==============================================================================
    // ESTRUCTURAS
    //==============================================================================
    // [Define aquí tus structs]
    // typedef struct {
    //     int parametro1;  ///< [descripción]
    //     float parametro2; ///< [descripción]
    // } mi_config_t;

    //==============================================================================
    // CÓDIGOS DE ERROR
    //==============================================================================
    // [Define aquí errores específicos]
    // #define ESP_ERR_MI_LIBRERIA_ERROR1 (ESP_ERR_INVALID_ARG)

    //==============================================================================
    // DECLARACIÓN DE FUNCIONES
    //==============================================================================

    /**
     * @brief Inicializacion del servo para la mandibula
     *
     * @return
     *     - ESP_OK: Cuando se inicializa sin errores
     *     - ESP_FAIL: Error al inicialiar el servo para la mandibula
     *
     * @note [notas importantes]
     * @see [otras funciones relacionadas]
     */
    esp_err_t mandibula_servo_init(void);

    /**
     * @brief Control del angulo
     * @note [notas importantes]
     * @see [otras funciones relacionadas]
     */
    void control_angulo_servo(int angulo);

//==============================================================================
// FINALIZACIÓN
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif // [NOMBRE]_H