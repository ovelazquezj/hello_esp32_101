#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"

// Etiqueta para los logs
static const char *TAG = "LED_RGB";

// CONFIGURACIÓN DEL LED RGB
#define LED_GPIO        48              // GPIO del LED RGB
#define LED_COUNT       1               // Número de LEDs en la tira
#define DELAY_MS        3000            // 3 segundos encendido/apagado

// Colores RGB (0-255)
#define COLOR_OFF       0, 0, 0         // Apagado
#define COLOR_GREEN     0, 255, 0       // Verde brillante
#define COLOR_BLUE      0, 0, 255       // Azul brillante
#define COLOR_RED       255, 0, 0       // Rojo brillante
#define COLOR_WHITE     255, 255, 255   // Blanco

// Variable global para la tira de LED
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

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,           // GPIO del LED
        .max_leds = LED_COUNT,                // Número de LEDs
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,  // Formato WS2812
        .led_model = LED_MODEL_WS2812,        // Modelo del LED
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10000000,            // 10MHz resolución RMT
    };

    // Crear el handle de la tira de LED
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error al crear LED strip: %s", esp_err_to_name(err));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    ESP_LOGI(TAG, "✓ LED strip configurado exitosamente");

    // Limpiar/Inicializar todos los LEDs a apagado
    led_strip_clear(led_strip);
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "Delay: %d ms (%.1f segundos)", DELAY_MS, DELAY_MS / 1000.0);
    ESP_LOGI(TAG, "================================================\n");

    // ========================================
    // BUCLE PRINCIPAL - BLINK RGB
    // ========================================
    while (1) {
        // ENCENDER LED (Verde para distinguir del LED de poder)
        ESP_LOGI(TAG, "✓ LED ENCENDIDO (Verde)");
        led_strip_set_pixel(led_strip, 0, COLOR_GREEN);  // LED 0, color verde
        led_strip_refresh(led_strip);                    // Actualizar LED
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

        // APAGAR LED
        ESP_LOGI(TAG, "✗ LED APAGADO");
        led_strip_clear(led_strip);                      // Apagar todos
        led_strip_refresh(led_strip);                    // Actualizar LED
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}
