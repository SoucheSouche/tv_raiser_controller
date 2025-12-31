#pragma once

#include "driver/gpio.h"

/* GPIO Pin Assignments */
#define TX_GPIO          GPIO_NUM_13    // Output pin - controller RX / remote TX line
#define RX_GPIO          GPIO_NUM_12    // Input pin - controller TX / remote RX line
#define BTN_PRESET1_GPIO GPIO_NUM_25    // Input pin - pull high to request PRESET 1
#define BTN_PRESET2_GPIO GPIO_NUM_26    // Input pin - pull high to request PRESET 2

/**
 * Initialize all GPIO pins (TX, RX, buttons)
 * - TX: open-drain output (requires external pull-up to bus voltage)
 * - RX: input (no internal pulls)
 * - Buttons: input with pull-down (external high to trigger)
 */
void gpio_hal_init(void);
