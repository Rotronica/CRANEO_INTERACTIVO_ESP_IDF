/**
 * @file indicadores_led.h
 * @brief Componente para el control de los leds ws2812
 * @version 1.0.0
 * @date 4 de abril 2026
 *
 * [descripción detallada de la librería]
 *
 * @note [notas importantes para el usuario]
 * @warning [advertencias]
 * @author Rodrigo C.C.
 */

#ifndef INDICADOR_LED_H
#define INDICADOR_LED_H

//==============================================================================
// INCLUDES NECESARIOS
//==============================================================================
// [Incluye aquí los headers que necesitas]
// #include <stdint.h>
#include "esp_err.h"
#include "led_strip.h"
#include "esp_log.h"
//==============================================================================
//  SOPORTE PARA C++
//==============================================================================
#ifdef __cplusplus
extern "C"
{
#endif

    //==============================================================================
    // CONSTANTES Y MACROS
    //==============================================================================
#define TOTAL_HUESOS 22 // Número total de huesos
    extern const char *NOMBRE_HUESOS[];

    //==============================================================================
    // ENUMERACIONES
    //==============================================================================
    typedef enum
    {
        HUESO_FRONTAL = 1,
        HUESO_PARETAL_DERECHO,
        HUESO_PARETAL_IZQUIERDO,
        HUESO_TEMPORAL_DERECHO,
        HUESO_TEMPORAL_IZQUIERDO,
        HUESO_OCCIPITAL,
        HUESO_ESFENOIDES,
        HUESO_ETMOIDES,
        HUESO_MANDIBULA,
        HUESO_MAXILAR_DERECHO,
        HUESO_MAXILAR_IZQUIERDO,
        HUESO_CIGOMATICO_DERECHO,
        HUESO_CIGOMATICO_IZQUIERDO,
        HUESO_NASAL_DERECHO,
        HUESO_NASAL_IZQUIERDO,
        HUESO_LAGRIMAL_DERECHO,
        HUESO_LAGRIMAL_IZQUIERDO,
        HUESO_PALATINO_DERECHO,
        HUESO_PALATINO_IZQUIERDO,
        HUESO_CORNETE_DERECHO,
        HUESO_CORNETE_IZQUIERDO,
        HUESO_VOMER
    } hueso_craneo_t;

    //==============================================================================
    // ESTRUCTURAS
    //==============================================================================
    typedef struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } color_rgb_t;

    //==============================================================================
    // CÓDIGOS DE ERROR
    //==============================================================================
    // [Define aquí errores específicos]
    // #define ESP_ERR_MI_LIBRERIA_ERROR1 (ESP_ERR_INVALID_ARG)

    //==============================================================================
    // DECLARACIÓN DE FUNCIONES
    //==============================================================================

    /**
     * @brief Inicializacion de los leds ws2812
     *
     * @return
     *     - ESP_OK: Cuando la funcion se a inicializado correctamente
     *
     *     - ESP_FAIL: Cuando la funcion a fallado en la inicializacion
     */
    esp_err_t led_ws2812_init(void);

    /**
     * @brief Encender un led ws2812 para un hueso seleccionado
     *
     * @param[in] hueso Indica el hueso que se encendera
     * @return
     *     - Sin retorno
     *
     * @note [notas importantes]
     * @see [otras funciones relacionadas]
     */
    void indicador_led_encender_hueso(uint8_t hueso);

    /**
     * @brief Modo indicador de conexión exitosa - Verde fijo
     * @param[in] activar true para iniciar modo, false para detener
     */
    void indicador_modo_conectado(bool activar);
    void indicador_modo_desconectado(bool activar);
//==============================================================================
// FINALIZACIÓN
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif // [NOMBRE]_H