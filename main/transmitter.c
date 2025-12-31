#include "transmitter.h"
#include "protocol.h"
#include "gpio_hal.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <assert.h>

/**
 * Busy-wait for accurate microsecond delays
 */
static inline __attribute__((always_inline)) void delay_us(uint32_t us)
{
    int64_t start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < us) {
        // Busy wait for accuracy
    }
}

/**
 * Send a single pulse with specified level and duration
 */
static void send_pulse(uint32_t level, uint32_t duration_us)
{
    gpio_set_level(TX_GPIO, level);
    delay_us(duration_us);
}

void transmitter_send_frame(const uint8_t *frame)
{
    assert(frame != NULL);  // Validate input
    
    // Send all 48 bits (LSB first to match analyzer)
    for (size_t byte_idx = 0; byte_idx < FRAME_BYTES; byte_idx++) {
        for (size_t bit_idx = 0; bit_idx < 8; bit_idx++) {
            uint32_t bit = (frame[byte_idx] >> bit_idx) & 1;
            send_pulse(bit, BIT_US);
        }
    }
    
    // Send frame gap (high level)
    send_pulse(1, FRAME_GAP_US);
}
