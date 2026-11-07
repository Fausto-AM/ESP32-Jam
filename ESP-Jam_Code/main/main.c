#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "mirf.h"

static const char *TAG = "ESP-JAM";

#define NRF_SPI_HOST      SPI2_HOST
#define NRF_PIN_SCK       18
#define NRF_PIN_MOSI      23
#define NRF_PIN_MISO      19
#define NRF1_PIN_CE       5
#define NRF1_PIN_CSN      22
#define NRF2_PIN_CE       21
#define NRF2_PIN_CSN      4
#define BTN_JAMMER_PIN    25
#define LED_STATUS_PIN    26

static volatile bool jammer_running = false;
static volatile bool hw_error       = false;

#define REG_EN_AA       0x01
#define REG_SETUP_RETR  0x04
#define REG_RF_SETUP    0x06

static void led_task(void *pvParameters)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << LED_STATUS_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    bool phase = false;
    TickType_t last_tick = xTaskGetTickCount();

    while (1) {
        if (hw_error) {
            gpio_set_level(LED_STATUS_PIN, 1);
        } 
        else if (jammer_running) {
            TickType_t now = xTaskGetTickCount();
            if ((now - last_tick) >= pdMS_TO_TICKS(100)) {
                phase = !phase;
                gpio_set_level(LED_STATUS_PIN, phase);
                last_tick = now;
            }
        } 
        else {
            // Apagado
            gpio_set_level(LED_STATUS_PIN, 0);
            phase = false;
            last_tick = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void button_task(void *pvParameters)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << BTN_JAMMER_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    int last_state = 1;
    while (1) {
        int current_state = gpio_get_level(BTN_JAMMER_PIN);
        // Flanco de bajada (botón presionado)
        if (last_state == 1 && current_state == 0) {
            jammer_running = !jammer_running;
            ESP_LOGI(TAG, "Jammer -> %s", jammer_running ? "ON" : "OFF");
            vTaskDelay(pdMS_TO_TICKS(250));
            last_state = 1;
            continue;
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void jammer_task(void *pvParameters)
{
    NRF24_t dev1;
    NRF24_t dev2;

    Nrf24_init(&dev1);
    Nrf24_config(&dev1, 0, 32);

    Nrf24_init(&dev2);
    Nrf24_config(&dev2, 63, 32);

    uint8_t zero = 0x00;
    uint8_t rf_setup = 0x0E;

    if (Nrf24_setTADDR(&dev1, (uint8_t *)"AAAAA") != ESP_OK) {
        ESP_LOGE(TAG, "Fallo Módulo 1 (CE=%d, CSN=%d). Revisa cables.", NRF1_PIN_CE, NRF1_PIN_CSN);
        hw_error = true;
    } else {
        Nrf24_WriteRegister(&dev1, REG_EN_AA, &zero, 1);
        Nrf24_WriteRegister(&dev1, REG_SETUP_RETR, &zero, 1);
        Nrf24_WriteRegister(&dev1, REG_RF_SETUP, &rf_setup, 1);
    }

    if (Nrf24_setTADDR(&dev2, (uint8_t *)"BBBBB") != ESP_OK) {
        ESP_LOGE(TAG, "Fallo Módulo 2 (CE=%d, CSN=%d). Revisa cables.", NRF2_PIN_CE, NRF2_PIN_CSN);
        hw_error = true;
    } else {
        Nrf24_WriteRegister(&dev2, REG_EN_AA, &zero, 1);
        Nrf24_WriteRegister(&dev2, REG_SETUP_RETR, &zero, 1);
        Nrf24_WriteRegister(&dev2, REG_RF_SETUP, &rf_setup, 1);
    }

    if (hw_error) {
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    Nrf24_printDetails(&dev1);
    Nrf24_printDetails(&dev2);

    uint8_t buf[32];
    for (int i = 0; i < 32; i++) {
        buf[i] = (uint8_t)(esp_random() & 0xFF);
    }

    uint8_t ch1 = 0;
    uint8_t ch2 = 63;

    ESP_LOGW(TAG, "Sistema listo. Esperando botón...");

    while (1) {
        if (!jammer_running) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        Nrf24_setChannel(&dev1, ch1);
        Nrf24_send(&dev1, buf);
        Nrf24_isSend(&dev1, 1);
        
        ch1++;
        if (ch1 > 62) ch1 = 0;

        Nrf24_setChannel(&dev2, ch2);
        Nrf24_send(&dev2, buf);
        Nrf24_isSend(&dev2, 1);

        ch2++;
        if (ch2 > 125) ch2 = 63;
        
        taskYIELD();
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando ESP-JAM");

    xTaskCreate(led_task,    "led",    2048, NULL,  3, NULL);
    xTaskCreate(button_task, "button", 2048, NULL,  5, NULL);
    xTaskCreate(jammer_task, "jammer", 4096, NULL, 10, NULL);
}