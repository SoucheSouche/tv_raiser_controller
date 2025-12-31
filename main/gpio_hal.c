#include "gpio_hal.h"
#include "esp_log.h"

#define TAG "GPIO_HAL"

void gpio_hal_init(void)
{
    gpio_reset_pin(TX_GPIO);
    gpio_reset_pin(RX_GPIO);
    gpio_reset_pin(BTN_PRESET1_GPIO);
    gpio_reset_pin(BTN_PRESET2_GPIO);

    // TX: Open-drain drive (pull low for 0, release for 1)
    // Requires external pull-up to 3.3V or bus voltage
    gpio_config_t tx_cfg = {
        .pin_bit_mask = 1ULL << TX_GPIO,
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,   // use external pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&tx_cfg));
    
    // RX: Input, no pulls (bus provides level)
    gpio_config_t rx_cfg = {
        .pin_bit_mask = 1ULL << RX_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&rx_cfg));

    // Buttons: Input with pulldown; pull high externally to request command
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << BTN_PRESET1_GPIO) | (1ULL << BTN_PRESET2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn_cfg));
    
    // Start TX with high level (idle state)
    gpio_set_level(TX_GPIO, 1);
    
    ESP_LOGI(TAG, "Initialized: TX=%d, RX=%d, BTN_PRESET1=%d, BTN_PRESET2=%d", 
             TX_GPIO, RX_GPIO, BTN_PRESET1_GPIO, BTN_PRESET2_GPIO);
}
