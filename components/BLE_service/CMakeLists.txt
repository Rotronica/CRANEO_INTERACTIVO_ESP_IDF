idf_component_register(SRCS "BLE_service.c"
                    INCLUDE_DIRS "include"
                        REQUIRES 
                                bt           # Para todas las APIs de Bluetooth (GAP, GATT, Bluedroid)
                                nvs_flash    # Para inicializar NVS (necesario para BLE)
                                )
