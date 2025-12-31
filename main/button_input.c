#include "button_input.h"
#include "protocol.h"
#include "gpio_hal.h"
#include "driver/gpio.h"

const uint8_t *button_select_frame(void)
{
    int p1 = gpio_get_level(BTN_PRESET1_GPIO);
    int p2 = gpio_get_level(BTN_PRESET2_GPIO);

    if (p1 && !p2) {
        return FRAME_PRESET_1;
    }
    if (p2 && !p1) {
        return FRAME_PRESET_2;
    }
    return FRAME_IDLE;
}
