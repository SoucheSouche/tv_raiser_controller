#pragma once

#include <stdint.h>
#include "driver/gpio.h"  // For GPIO macros used in button_input.c

/**
 * Read button states and select appropriate frame
 * @return Pointer to selected frame (IDLE, PRESET_1, or PRESET_2)
 */
const uint8_t *button_select_frame(void);
