# Taller 101: Tu Primer Firmware con ESP32-S3 y LED RGB WS2812
>  **Licencia:** Este material está bajo licencia [Creative Commons Atribución 4.0 Internacional (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/).  
>  **Autor:** Omar Velazquez —  ovelazquezj@gmail.com

---

¡Bienvenido! En este taller aprenderás a crear la estructura básica de un proyecto para el **ESP32-S3** utilizando el framework oficial **ESP-IDF (v5.5)** y controlar un **LED RGB direccionable (WS2812/NeoPixel)**. Al final, tendrás un LED que parpadea en colores y entenderás cómo organizar tus archivos.

---

## ¿Qué es un LED RGB WS2812 (NeoPixel)?

Tu placa ESP32-S3 incluye un **LED RGB inteligente** con características especiales:

| Característica | Descripción |
|---------------|-------------|
| **Tipo** | LED RGB direccionable (WS2812/NeoPixel) |
| **Control** | Protocolo serial de un solo cable (datos + timing preciso) |
| **Colores** | 16.7 millones de combinaciones (256 niveles por canal R, G, B) |
| **GPIO** | Conectado típicamente al **GPIO 48** en ESP32-S3-DevKit |
| **Formato** | Los datos se envían en orden **GRB** (Green-Red-Blue), no RGB |
| **Ventaja** | Puedes encender/apagar y cambiar color sin hardware adicional |

### Diferencia clave con LEDs normales

```
LED normal:     GPIO ──▶ LED ──▶ GND  (encendido = nivel alto/bajo)
LED WS2812:     GPIO ──▶ [Chip integrado] ──▶ LED RGB  (encendido = protocolo de datos)
```

El LED WS2812 tiene un **chip controlador integrado** que interpreta una señal de datos con timing muy preciso. Por eso usamos la biblioteca `led_strip` de ESP-IDF, que maneja todo el protocolo por nosotros.

---

## Requisitos Previos

Antes de empezar, asegúrate de tener:

1.  **ESP-IDF v5.5 instalado** en tu computadora.
2.  **Placa ESP32-S3 con LED RGB** (ej. variante con cámara, N8R2/N16R8) y cable USB.
3.  **Editor de código** (VS Code recomendado).
4.  **Terminal/Consola** abierta.
5.  **Conexión a internet** (para descargar el componente `led_strip`).

> **Importante:** Antes de escribir cualquier comando, debes activar el entorno de ESP-IDF.
> *   **Windows:** Ejecuta `Export.bat` en la carpeta de instalación.
> *   **Linux/Mac:** Ejecuta `. ./export.sh` en la carpeta de instalación.

---

## Paso 1: Estructura del Proyecto

ESP-IDF usa un sistema de compilación llamado **CMake**. Necesitas crear carpetas y archivos específicos para que funcione.

Crea una carpeta llamada `mi_primer_proyecto` y dentro crea la siguiente estructura:

```text
mi_primer_proyecto/
├── main/
│   ├── CMakeLists.txt          ← Configuración del código principal
│   ├── hello_world_main.c      ← Tu código C (control del LED RGB)
│   └── idf_component.yml       ← ← ¡NUEVO! Dependencias del proyecto
├── CMakeLists.txt              ← Configuración general del proyecto
├── README.md                   ← Este archivo
└── LICENSE.md                  ← Licencia CC BY 4.0
```

> **Nota:** No crees la carpeta `build/`, el sistema la generará automáticamente después.

---

## Paso 2: Crear los Archivos

Copia y pega el siguiente contenido en cada archivo correspondiente.

### 1. Archivo Raíz: `CMakeLists.txt`

Este archivo le dice al compilador que esto es un proyecto ESP-IDF.

```cmake
# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.16)

# Incluir las herramientas de ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Nombre de tu proyecto (será el nombre del archivo .bin)
project(mi_primer_proyecto)
```

### 2. Archivo Principal: `main/CMakeLists.txt`

Este archivo registra tu código fuente y las dependencias necesarias.

```cmake
idf_component_register(
    SRCS "hello_world_main.c"
    INCLUDE_DIRS "."
    REQUIRES "driver" "led_strip"  # ← led_strip es esencial para WS2812
)
```

### 3. Dependencias: `main/idf_component.yml`

Este archivo le dice a ESP-IDF qué componentes externos descargar del registry.

```yaml
## IDF Component Manager Manifest File
dependencies:
  espressif/led_strip: "^2.4.0"  # ← Biblioteca oficial para LEDs direccionables
  idf:
    version: ">=5.0.0"
```

> **Alternativa:** Puedes crear este archivo manualmente o ejecutando:
> ```bash
> idf.py add-dependency "espressif/led_strip^2.4.0"
> ```

### 4. Tu Código: `main/hello_world_main.c`

Este programa hace parpadear el **LED RGB embebido** en color verde.

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"

// Etiqueta para los logs
static const char *TAG = "LED_RGB";

//️ CONFIGURACIÓN DEL LED RGB WS2812
#define LED_GPIO        48              // GPIO del LED (típico en ESP32-S3)
#define LED_COUNT       1               // Número de LEDs en la tira
#define DELAY_MS        3000            // Tiempo encendido/apagado (milisegundos)

// Colores en formato RGB (0-255 por canal)
// Nota: El WS2812 internamente usa formato GRB, pero la librería lo convierte
#define COLOR_OFF       0, 0, 0         // Apagado (negro)
#define COLOR_RED       255, 0, 0       // Rojo puro
#define COLOR_GREEN     0, 255, 0       // Verde puro
#define COLOR_BLUE      0, 0, 255       // Azul puro
#define COLOR_WHITE     255, 255, 255   // Blanco (máximo brillo)
#define COLOR_YELLOW    255, 255, 0     // Amarillo (Rojo + Verde)
#define COLOR_CYAN      0, 255, 255     // Cyan (Verde + Azul)
#define COLOR_MAGENTA   255, 0, 255     // Magenta (Rojo + Azul)

// Handle global para la tira de LED
static led_strip_handle_t led_strip;

void app_main(void)
{
    ESP_LOGI(TAG, "================================================");
    ESP_LOGI(TAG, "  HELLO WORLD - LED RGB WS2812 ESP32-S3         ");
    ESP_LOGI(TAG, "  Autor: Omar Velazquez                         ");
    ESP_LOGI(TAG, "  Email: ovelazquezj@gmail.com                  ");
    ESP_LOGI(TAG, "  Licencia: CC BY 4.0                           ");
    ESP_LOGI(TAG, "================================================");

    // ========================================
    // 1️⃣ CONFIGURAR LA TIRA DE LED
    // ========================================
    ESP_LOGI(TAG, "Configurando LED strip en GPIO %d...", LED_GPIO);

    // Configuración básica del LED strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,           // GPIO donde está conectado el LED
        .max_leds = LED_COUNT,                // Cantidad de LEDs a controlar
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,  // Formato de datos del WS2812
        .led_model = LED_MODEL_WS2812,        // Modelo específico del LED
    };

    // Configuración del periférico RMT (para timing preciso)
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10000000,            // Resolución de 10 MHz para el RMT
    };

    // Crear el handle de la tira de LED
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "❌ Error al crear LED strip: %s", esp_err_to_name(err));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }  // Bloquear en caso de error
    }

    ESP_LOGI(TAG, "✓ LED strip configurado exitosamente");

    // Limpiar/Inicializar todos los LEDs a apagado
    led_strip_clear(led_strip);
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "Delay: %d ms (%.1f segundos)", DELAY_MS, DELAY_MS / 1000.0);
    ESP_LOGI(TAG, "================================================\n");

    // ========================================
    // 2️⃣ BUCLE PRINCIPAL - BLINK RGB
    // ========================================
    while (1) {
        // 🔵 ENCENDER LED (Verde para distinguir del LED de poder rojo)
        ESP_LOGI(TAG, "✓ LED ENCENDIDO (Verde)");
        led_strip_set_pixel(led_strip, 0, COLOR_GREEN);  // LED índice 0, color verde
        led_strip_refresh(led_strip);                    // ¡Importante! Enviar datos al LED
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

        // ⚫ APAGAR LED
        ESP_LOGI(TAG, "✗ LED APAGADO");
        led_strip_clear(led_strip);                      // Apagar todos los LEDs
        led_strip_refresh(led_strip);                    // Actualizar estado
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}
```

---

## Paso 3: Compilar y Flashear

Abre tu terminal en la carpeta `mi_primer_proyecto` y ejecuta los siguientes comandos en orden.

### 1. Configurar el Chip (solo la primera vez)

```bash
idf.py set-target esp32s3
```

### 2. Limpiar build anterior (recomendado al agregar dependencias)

```bash
idf.py fullclean
```

### 3. Compilar y Subir (Flashear)

```bash
idf.py -p PORT flash monitor
```

>  **Reemplaza `PORT`**:
> *   **Windows:** Usualmente `COM3`, `COM4`, etc. (ej. `-p COM3`)
> *   **Linux:** Usualmente `/dev/ttyUSB0` (ej. `-p /dev/ttyUSB0`)
> *   **Mac:** Usualmente `/dev/cu.usbserial-...`
> *   *Si solo tienes una placa conectada, a veces puedes omitir `-p PORT`.*

---

## Guía de Colores para WS2812

El LED RGB usa tres canales de color: **Rojo (R)**, **Verde (G)** y **Azul (B)**. Cada uno va de `0` (apagado) a `255` (máximo brillo).

### Tabla de Colores Comunes

| Color | R | G | B | Código para `led_strip_set_pixel()` |
|-------|---|---|---|-------------------------------------|
| ⚫ Apagado | 0 | 0 | 0 | `0, 0, 0` |
| 🔴 Rojo | 255 | 0 | 0 | `255, 0, 0` |
| 🟢 Verde | 0 | 255 | 0 | `0, 255, 0` |
| 🔵 Azul | 0 | 0 | 255 | `0, 0, 255` |
| ⚪ Blanco | 255 | 255 | 255 | `255, 255, 255` |
| 🟡 Amarillo | 255 | 255 | 0 | `255, 255, 0` |
| 🟣 Magenta | 255 | 0 | 255 | `255, 0, 255` |
| 🩵 Cyan | 0 | 255 | 255 | `0, 255, 255` |
| 🟠 Naranja | 255 | 127 | 0 | `255, 127, 0` |
| 🟣 Violeta | 148 | 0 | 211 | `148, 0, 211` |

### Consejos de Uso

```c
//  Correcto: Usar macros definidas para legibilidad
led_strip_set_pixel(led_strip, 0, COLOR_GREEN);

//  También válido: Valores directos (R, G, B)
led_strip_set_pixel(led_strip, 0, 0, 255, 0);  // Verde

// ️ Importante: Siempre llamar a led_strip_refresh() después de set_pixel
led_strip_set_pixel(led_strip, 0, COLOR_BLUE);
led_strip_refresh(led_strip);  // ← ¡Sin esto, el LED no cambia!

// 🧹 Para apagar todos los LEDs de una vez:
led_strip_clear(led_strip);
led_strip_refresh(led_strip);
```

---

## Paso 4: Solución de Problemas

| Problema | Causa Probable | Solución |
|----------|---------------|----------|
| **Error: `led_strip.h not found`** | Componente no agregado | Ejecuta `idf.py add-dependency "espressif/led_strip^2.4.0"` |
| **Error: `rebuilding 'build.ninja' failed`** | Build corrupto por cambio de dependencias | Ejecuta `idf.py fullclean` y recompila |
| **LED no enciende o color incorrecto** | GPIO equivocado o formato GRB/RGB | Verifica que sea GPIO 48 y que usas `LED_PIXEL_FORMAT_GRB` |
| **LED parpadea muy rápido** | `DELAY_MS` muy pequeño | Aumenta el valor (ej. 3000 para 3 segundos) |
| **Logs no aparecen** | Puerto o baud rate incorrecto | Usa `idf.py monitor` o verifica 115200 bauds |

---

##  ¡Felicidades!

Si ves el LED RGB parpadeando en **verde** cada 3 segundos y mensajes en la consola que dicen `LED ENCENDIDO` / `LED APAGADO`, **¡has completado tu primer proyecto con ESP-IDF y LED RGB!**

---

##  Retos Propuestos (Enfocados en Colores y Timing)

Ahora que tienes el blink básico funcionando, ¡es hora de mejorar tus habilidades! Completa estos retos en orden:

### Reto 1: Ciclo de Colores Básico
**Objetivo:** Hacer que el LED cambie de color en cada ciclo.

```c
// Pista: Usa un array de colores y un índice
const uint8_t colores[][3] = {
    {255, 0, 0},    // Rojo
    {0, 255, 0},    // Verde
    {0, 0, 255},    // Azul
};

int indice = 0;
while (1) {
    led_strip_set_pixel(led_strip, 0, colores[indice][0], colores[indice][1], colores[indice][2]);
    led_strip_refresh(led_strip);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    indice = (indice + 1) % 3;  // Ciclar entre 0, 1, 2
}
```

### Reto 2: Velocidad Variable con `millis()` (Non-blocking)
**Objetivo:** Cambiar el color cada X segundos **sin usar `vTaskDelay()`** que bloquea el CPU.

>  **¿Qué es `millis()`?**  
> Es una función que devuelve el tiempo transcurrido desde que la placa se encendió (en milisegundos). Nos permite medir tiempo sin detener la ejecución del programa.

```c
// Pista: Implementar tu propia función millis() basada en esp_timer
#include "esp_timer.h"

// Función auxiliar: devuelve milisegundos desde el inicio
uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// Variables para timing non-blocking
uint32_t ultimo_cambio = 0;
const uint32_t intervalo_ms = 2000;  // Cambiar cada 2 segundos
int color_actual = 0;

while (1) {
    uint32_t ahora = millis();
    
    // ¿Pasó el intervalo de tiempo?
    if (ahora - ultimo_cambio >= intervalo_ms) {
        ultimo_cambio = ahora;  // Actualizar referencia de tiempo
        
        // Cambiar al siguiente color
        color_actual = (color_actual + 1) % 3;
        led_strip_set_pixel(led_strip, 0, colores[color_actual][0], colores[color_actual][1], colores[color_actual][2]);
        led_strip_refresh(led_strip);
        
        ESP_LOGI(TAG, "Cambio a color %d", color_actual);
    }
    
    // ⚠️ Importante: NO usar vTaskDelay() largo aquí
    // Esto permite que otras tareas se ejecuten mientras esperamos
    vTaskDelay(pdMS_TO_TICKS(10));  // Pequeño delay para ceder CPU
}
```

### Reto 3: Efecto "Fade" Suave entre Colores
**Objetivo:** Transicionar gradualmente de un color a otro (ej. rojo → verde).

```c
// Pista: Interpolar valores RGB paso a paso
void fade_to_color(uint8_t r_target, uint8_t g_target, uint8_t b_target, uint16_t pasos, uint16_t delay_paso) {
    uint8_t r_actual = 0, g_actual = 0, b_actual = 0;  // Obtener color actual si es necesario
    
    for (int i = 0; i <= pasos; i++) {
        // Interpolación lineal
        uint8_t r = r_actual + ((r_target - r_actual) * i) / pasos;
        uint8_t g = g_actual + ((g_target - g_actual) * i) / pasos;
        uint8_t b = b_actual + ((b_target - b_actual) * i) / pasos;
        
        led_strip_set_pixel(led_strip, 0, r, g, b);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(delay_paso));
    }
}
```

### Reto 4: Control de Velocidad con "millis()" Avanzado
**Objetivo:** Crear un sistema donde puedas cambiar la velocidad del efecto en tiempo real (ej. con un botón o comando serial).

```c
// Pista: Usar una variable global para intervalo_ms que se pueda modificar
volatile uint32_t intervalo_ms = 2000;  // 'volatile' para que el compilador no optimice

// En otra parte del código (ej. callback de botón o UART):
void cambiar_velocidad(uint32_t nuevo_intervalo) {
    intervalo_ms = nuevo_intervalo;
    ESP_LOGI(TAG, "Velocidad cambiada a %d ms", nuevo_intervalo);
}

// En el loop principal, usar 'intervalo_ms' en lugar de un valor fijo
if (ahora - ultimo_cambio >= intervalo_ms) {
    // ... lógica de cambio de color
}
```

---

##  Diagrama: Timing con `millis()` vs `vTaskDelay()`

```
CON vTaskDelay() (BLOCKING):
Tiempo:  0s    1s    2s    3s    4s
CPU:     [Cambiando color] ──[ESPERA 2s]── [Cambiando color] ──[ESPERA 2s]──
         ❌ No puede hacer nada más durante la espera

CON millis() (NON-BLOCKING):
Tiempo:  0s    1s    2s    3s    4s
CPU:     [Chequea tiempo] [Otras tareas] [Chequea tiempo→Cambia] [Otras tareas]
         ✅ Siempre disponible para responder a eventos
```

---

##  Licencia y Atribución (CC BY 4.0)

Este tutorial y sus archivos asociados están licenciados bajo **Creative Commons Atribución 4.0 Internacional (CC BY 4.0)**.

### Eres libre de:

*   **Compartir**: Copiar y redistribuir este material en cualquier medio o formato.
*   **Adaptar**: Remezclar, transformar y construir sobre este material para cualquier propósito, incluso comercialmente.

### Condiciones:

*   **Atribución**: Debes dar crédito apropiado, proporcionar un enlace a la licencia, e indicar si se realizaron cambios. Puedes hacerlo de cualquier manera razonable, pero no de forma que sugiera que el autor te respalda a ti o a tu uso.

### Cómo atribuir este trabajo (Ejemplos):

**Formato corto:**
> "Taller 101: ESP32-S3 con LED RGB WS2812" por Omar Velazquez, bajo licencia CC BY 4.0.

**Formato completo:**
> Este proyecto se basa en el material "Taller 101: Tu Primer Firmware con ESP32-S3 y LED RGB", creado por **Omar Velazquez** (ovelazquezj@gmail.com), disponible bajo licencia [Creative Commons Atribución 4.0 Internacional](https://creativecommons.org/licenses/by/4.0/).

### Enlaces útiles:

*   [Ver licencia completa (Español)](https://creativecommons.org/licenses/by/4.0/deed.es)
*   [Texto legal completo](https://creativecommons.org/licenses/by/4.0/legalcode.es)
*   [Componente led_strip en Espressif Registry](https://components.espressif.com/components/espressif/led_strip)

---

> © 2026 Omar Velazquez. Este trabajo está licenciado bajo [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

---
### Recursos Adicionales

| Recurso | Descripción |
|------|-------------|
| [Guía oficial de led_strip](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/led_strip.html) | Documentación técnica del componente |
| [Calculadora de colores RGB](https://www.rapidtables.com/web/color/RGB_Color.html) | Para experimentar con combinaciones |
| [esp_timer API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/esp_timer.html) | Para implementar `millis()` preciso |
| [WS2812 Datasheet](https://cdn.sparkfun.com/datasheets/Components/LED/WS2812B.pdf) | Entender el protocolo a bajo nivel |

---

