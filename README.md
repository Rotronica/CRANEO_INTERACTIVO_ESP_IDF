# 🧠 CRANEO_INTERACTIVO - Control BLE de Cráneo 3D con LEDs WS2812 y Servo

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue)](https://www.espressif.com/)
[![Framework](https://img.shields.io/badge/Framework-ESP--IDF_v6.0-red)](https://docs.espressif.com/projects/esp-idf/)
[![BLE](https://img.shields.io/badge/BLE-4.2-green)](https://www.bluetooth.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow)](LICENSE)

---

## 👤 Autor y Versión

| Campo | Información |
|-------|-------------|
| **Autor** | Rodrigo C.C. |
| **Versión** | 1.0.0 |
| **Fecha** | Abril 2026 |
| **Framework** | ESP-IDF v6.0 |
| **Plataforma** | ESP32 |

---

## 📋 Descripción del Proyecto

Este proyecto implementa un **cráneo anatómico interactivo en 3D** que permite:

- **Iluminar cada uno de los 22 huesos del cráneo** individualmente mediante LEDs WS2812 (NeoPixels)
- **Controlar la mandíbula** con un servo motor de 180 grados
- **Comunicación inalámbrica** vía Bluetooth Low Energy (BLE)
- **Control desde cualquier smartphone** sin necesidad de aplicación dedicada (funciona con apps BLE genéricas como nRF Connect)

El sistema está diseñado para ser educativo, demostrando la anatomía del cráneo humano de manera interactiva y visualmente atractiva.

## 🎯 Características

- ✨ **22 LEDs WS2812** - Cada hueso tiene su propio color distintivo
- 🦴 **Huesos identificables** - Nombres de los 22 huesos del cráneo
- 🔧 **Servo mandibular** - Movimiento realista de la mandíbula (0° a 180°)
- 📱 **Control BLE** - Servicio personalizado con 2 características
- 🎨 **Colores personalizados** - Cada hueso tiene un color único asignado
- 🔌 **Plug & Play** - Conexión inmediata sin configuración compleja

## 🏗️ Arquitectura del Proyecto

```
CRANEO_ESP_IDF/
├── .devcontainer/ # Configuración contenedor dev
├── .vscode/ # Configuración VS Code
├── build/ # Archivos compilados
├── components/ # Componentes personalizados
│ ├── BLE_service/ # Servicio BLE completo
│ │ ├── include/
| | | ├──BLE_service.h
│ │ └── BLE_service.c
│ ├── indicador_led/ # Control LEDs WS2812
│ │ ├── include/
| | | ├──indicador_led.h
│ │ └── indicador_led.c
│ └── iot_servo/ # Driver servo
│ │ ├── include/
| | | ├──iot_servo.h
│ │ └── iot_sevo.c
│ └── servo_mandibula/ # Control servo mandíbula
│ │ ├── include/
| | | ├──servo_mandibula.h
│ │ └── servo_mandibula.c
├── main/ # Código principal
│ ├── main.c # Punto de entrada
│ ├── CMakeLists.txt
│ └── idf_component.yml # Dependencias
├── managed_components/ # Componentes gestionados
├── CMakeLists.txt # Build principal
├── sdkconfig # Configuración ESP-IDF
└── dependencies.lock # Versiones dependencias
```


## 🔧 Componentes Principales

### 1. Servicio BLE (`BLE_service`)
- **UUID Servicio**: `0x00FF`
- **Característica HUESO**: `0xFF01` (Escritura 1-22)
- **Característica SERVO**: `0xFF02` (Escritura 0-180)
- Nombre dispositivo: `CRANEO_INTERACTIVO`

### 2. Control LEDs (`indicador_led`)
- GPIO: `4` (configurable)
- Tipo: WS2812 (NeoPixel)
- Protocolo: GRB de 24 bits
- 22 LEDs individuales con colores únicos

### 3. Control Servo (`servo_mandibula`)
- GPIO: `18` (configurable)
- Ángulo: 0° a 180°
- PWM: 50Hz (periodo 20ms)
- Pulso mínimo: 500µs
- Pulso máximo: 2500µs

## 📊 Mapeo de Huesos y Colores

| ID | Hueso | Color (RGB) |
|----|-------|-------------|
| 1 | Frontal | (255, 220, 180) Amarillo cálido |
| 2 | Parietal Derecho | (255, 0, 0) Rojo |
| 3 | Parietal Izquierdo | (0, 255, 0) Verde |
| 4 | Temporal Derecho | (0, 0, 255) Azul |
| 5 | Temporal Izquierdo | (0, 255, 255) Cian |
| 6 | Occipital | (255, 0, 255) Magenta |
| 7 | Esfenoides | (255, 165, 0) Naranja |
| 8 | Etmoides | (255, 255, 0) Amarillo |
| 9 | Mandíbula | (200, 0, 0) Rojo intenso |
| 10 | Maxilar Derecho | (255, 105, 180) Rosa |
| 11 | Maxilar Izquierdo | (255, 182, 193) Rosa claro |
| 12 | Cigomático Derecho | (128, 0, 128) Púrpura |
| 13 | Cigomático Izquierdo | (238, 130, 238) Violeta |
| 14 | Nasal Derecho | (205, 133, 63) Marrón claro |
| 15 | Nasal Izquierdo | (160, 82, 45) Marrón |
| 16 | Lagrimal Derecho | (127, 255, 212) Aguamarina |
| 17 | Lagrimal Izquierdo | (64, 224, 208) Turquesa |
| 18 | Palatino Derecho | (255, 215, 0) Oro |
| 19 | Palatino Izquierdo | (192, 192, 192) Plata |
| 20 | Cornete Derecho | (250, 128, 114) Salmón |
| 21 | Cornete Izquierdo | (255, 127, 80) Coral |
| 22 | Vómer | (255, 248, 220) Blanco hueso |

## 🚀 Requisitos Previos

### Hardware
- ESP32 (cualquier versión)
- Tira de 22 LEDs WS2812 (o módulos individuales)
- Servo motor SG90 o equivalente (0-180°)
- Fuente de alimentación 5V (mínimo 2A para LEDs)
- Cables de conexión

### Software
- **ESP-IDF v6.0** o superior
- Python 3.8+
- Git
- nRF Connect (para pruebas) o aplicación Flutter (futuro)

## 📦 Instalación y Configuración

### 1. Clonar el repositorio
```bash
git clone https://github.com/Rotronica/CRANEO_ESP_IDF.git
cd CRANEO_ESP_IDF
```
## 2. Configurar el proyecto
```bash
# Abrir menú de configuración
idf.py menuconfig
```
## Configuraciones recomendadas:

- Serial flasher config → Puerto serial correcto

- Component config → BLE → Habilitar BLE

- Component config → LED Strip → Configurar según necesidades
## 3. Compilar y flashear

```bash
# Limpiar build anterior
idf.py clean

# Compilar proyecto
idf.py build

# Flashear al ESP32
idf.py -p PORT flash monitor
```
## 🎮 Uso del Sistema
- Conexión BLE (Pruebas con nRF Connect)
Abrir nRF Connect en tu smartphone

- Escaneo: Buscar dispositivo CRANEO_INTERACTIVO

- Conectar: Tocar el nombre del dispositivo

- Servicio: Buscar servicio 00FF (Unknown Service)

## Características:

- FF01 (Hueso) - Enviar número del 1 al 22

- FF02 (Servo) - Enviar número del 0 al 180
## Comandos BLE

```bash
# Encender hueso frontal (ID: 1)
Escribir en FF01: [0x01]

# Mover mandíbula a 90 grados
Escribir en FF02: [0x5A]  # 90 en hexadecimal

# Encender hueso occipital (ID: 6)
Escribir en FF01: [0x06]
```
## Salida en Monitor Serial


```bash
I (1234) Principal: Inicializacion exitosa!!
I (1240) Principal: Servo mandibula inicializado
I (1245) BLE_SERVICE: Servicio BLE inicializado correctamente
I (1250) BLE_SERVICE: Nombre del dispositivo: CRANEO_INTERACTIVO
I (1256) Principal: ✅ BLE inicializado correctamente
I (3456) Principal: 📱 Recibido: Frontal (hueso 1)
I (3457) LED_WS2812: Hueso 1 encendido - Color RGB(255,220,180)
I (4567) Principal: 📱 Recibido: Mover mandíbula a 90°
```
## 🔌 Esquema de Conexiones
```
ESP32 DevKit                    Componentes
┌─────────────┐                 ┌──────────────┐
│ GPIO4       ├────────────────►│ WS2812 DATA  │
│ GPIO18      ├────────────────►│ Servo Signal │
│ 5V          ├────────────────►│ Servo VCC    │
│ 5V          ├────────────────►│ WS2812 VCC   │
│ GND         ├────────────────►│ GND (ambos)  │
└─────────────┘                 └──────────────┘
```
## Nota importante:

- Usar convertidor de nivel lógico si el servo requiere 5V

- Añadir capacitor de 1000µF entre VCC y GND de LEDs

- Resistencia de 300-500Ω en línea DATA del WS2812
## 🐛 Solución de Problemas Comunes

| Problema | Posible Causa | Solución |
|----------|--------------|----------|
| No se ve dispositivo BLE | BLE no inicializado | Verificar logs, reiniciar ESP32 |
| | BLE en modo incorrecto | Verificar que `esp_bt_controller_enable(ESP_BT_MODE_BLE)` se ejecute correctamente |
| | Antena desconectada | Verificar conexión de antena externa (si aplica) |
| LEDs no encienden | GPIO incorrecto | Verificar conexión GPIO4 |
| | Falta pull-up/resistencia | Añadir resistor 330Ω en línea DATA |
| | Alimentación insuficiente | Usar fuente externa 5V/2A |
| | LEDs dañados | Probar con un LED individual primero |
| Servo no se mueve | PWM no inicializado | Verificar GPIO18 en configuración |
| | Ángulo fuera de rango | Enviar valores entre 0 y 180 |
| | Falta ground común | Conectar GND del ESP32 con GND del servo |
| | Alimentación insuficiente | El servo necesita 5V/500mA mínimo |
| Compilación falla | Versión ESP-IDF incorrecta | Usar v6.0 o superior |
| | Dependencias faltantes | Ejecutar `idf.py update-dependencies` |
| | Componente iot_servo mal copiado | Verificar que iot_servo.c use `esp_driver_ledc` |
| Conexión BLE inestable | Interferencia WiFi | Cambiar canal WiFi o apagar WiFi |
| | Distancia excesiva | Acercar dispositivo (<10 metros) |
| | Múltiples conexiones | Desconectar otros dispositivos BLE cercanos |
| LED parpadea o color incorrecto | Protocolo erróneo | Verificar que use `LED_STRIP_COLOR_COMPONENT_FMT_GRB` |
| | Timing incorrecto | Revisar configuración RMT (10MHz, 64 símbolos) |
| | DMA conflictivo | En ESP32 normal, `with_dma = false` |
| Servo vibra o no mantiene posición | PWM inestable | Añadir capacitor de 100µF cerca del servo |
| | Ángulo fuera de especificación | Mantener entre 0° y 180° |
| | Ciclo de trabajo incorrecto | Verificar 500µs (0°) y 2500µs (180°) |
| App Flutter no conecta (futuro) | UUID incorrecto | Verificar servicio 0x00FF, características 0xFF01 y 0xFF02 |
| | Permisos Bluetooth | Solicitar permisos en Android/iOS |
| | MTU muy pequeño | Configurar MTU=500 en ESP32 |
## 🚧 Próximos Pasos (Roadmap)

- [ ] **Aplicación Flutter** - Interfaz intuitiva para iOS/Android
  - [ ] Pantalla principal con modelo 3D del cráneo
  - [ ] Selector visual de huesos (tocar el hueso en el modelo 3D)
  - [ ] Control deslizante para la mandíbula
  - [ ] Modo oscuro/claro
- [ ] **Efectos de iluminación** - Secuencias y animaciones
  - [ ] Modo "Rainbow" (efecto arcoíris cíclico)
  - [ ] Modo "Respiración" (fade in/out)
  - [ ] Modo "Latido" (pulso sincronizado)
  - [ ] Secuencias personalizables por el usuario
- [ ] **Modo educativo** - Información anatómica
  - [ ] Descripción de cada hueso al seleccionarlo
  - [ ] Datos curiosos sobre anatomía craneal
  - [ ] Quiz interactivo para aprender los huesos
  - [ ] Soporte multilenguaje (ES, EN, PT)
- [ ] **Control por voz** - Integración con asistentes
  - [ ] Comandos de voz para encender huesos específicos
  - [ ] Control de mandíbula por voz
- [ ] **Mejoras de hardware**
  - [ ] Servo más preciso (control de apertura/cierre gradual)
  - [ ] Sensor de proximidad para activación automática
  - [ ] Batería recargable para portabilidad
- [ ] **Mejoras de software**
  - [ ] Modo multiusuario (múltiples dispositivos conectados)
  - [ ] OTA Updates (actualizaciones inalámbricas)
  - [ ] Configuración de colores personalizables vía BLE
  - [ ] Modo ahorro de energía (deep sleep)
  - [ ] Registro de estadísticas de uso

---

## 📚 Dependencias

### Dependencias del Proyecto

```yaml
# main/idf_component.yml
dependencies:
  espressif/led_strip: '*'     # Control de LEDs WS2812 (NeoPixel)
  idf:
    version: '>=6.0.0'         # ESP-IDF versión 6.0 o superior
```
### Dependencias del Componente Servo (Adaptado)

```cmake
# components/servo_mandibula/CMakeLists.txt
idf_component_register(SRCS "servo_mandibula.c"
                       INCLUDE_DIRS "include"
                       REQUIRES   esp_driver_ledc
                                  esp_driver_gpio)
```
### Dependencias del Componente LED
```cmake
# components/indicador_led/CMakeLists.txt
idf_component_register(SRCS "indicador_led.c"
                       INCLUDE_DIRS "include"
                       REQUIRES led_strip)
```
### Dependencias del Componente BLE
```
# components/BLE_service/CMakeLists.txt
idf_component_register(SRCS "BLE_service.c"
                       INCLUDE_DIRS "include"
                       REQUIRES nvs_flash
                                bt
                                esp_timer)
```
## Notas Importantes sobre Dependencias
⚠️ ADVERTENCIA: La librería iot_servo es una adaptación local debido a incompatibilidades con ESP-IDF v6.0. El componente original espressif/servo no ha sido actualizado para v6.0, por lo que se creó un componente personalizado iot_servo con las modificaciones necesarias para usar esp_driver_ledc en lugar del driver legacy.

## 📞 Contacto
rodrigocallecondori@gmail.com
## 📄 Licencia
```
Este proyecto está bajo la Licencia MIT - un permiso de software libre permisivo.
MIT License

Copyright (c) 2026 Rodrigo C.C.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```