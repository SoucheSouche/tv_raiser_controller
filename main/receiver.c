#include "receiver.h"
#include "protocol.h"
#include "gpio_hal.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define TAG "RECEIVER"

static EventGroupHandle_t g_events = NULL;

// Polling state (persistent across calls)
static uint32_t s_last_level = 0;
static int64_t s_last_time = 0;
static uint8_t s_current_frame[6] = {0};
static size_t s_bit_counter = 0;
static bool s_started = false;
static bool s_initialized = false;

/**
 * Helper function to print frame with repeat count
 * Handles both named and unnamed frames
 */
static void print_frame(const char *prefix, const uint8_t *frame, uint32_t repeat_count, const char *name)
{
    if (name) {
        printf("%s: %02X %02X %02X %02X %02X %02X (x%lu) [%s]",
               prefix, frame[0], frame[1], frame[2],
               frame[3], frame[4], frame[5],
               (unsigned long)repeat_count, name);
    } else {
        printf("%s: %02X %02X %02X %02X %02X %02X (x%lu)",
               prefix, frame[0], frame[1], frame[2],
               frame[3], frame[4], frame[5],
               (unsigned long)repeat_count);
    }
}

/**
 * Log a received frame with repeat counting
 */
static void log_response_frame(const uint8_t *frame, size_t bit_count)
{
    static uint8_t last_frame[6] = {0};
    static uint32_t repeat_count = 0;
    static bool has_last = false;

    if (bit_count != FRAME_BITS) {
        ESP_LOGW(TAG, "Incomplete response: %zu bits (expected %d)", bit_count, FRAME_BITS);
        has_last = false;
        repeat_count = 0;
        return;
    }

    if (has_last && memcmp(frame, last_frame, 6) == 0) {
        repeat_count++;
        const char *name = identify_ctrl_frame(last_frame);
        printf("\r");
        print_frame("RESP", last_frame, repeat_count, name);
        printf("     ");  // Clear trailing characters on overwrites
    } else {
        if (has_last) {
            printf("\n");
        }
        memcpy(last_frame, frame, 6);
        repeat_count = 1;
        has_last = true;

        const char *name = identify_ctrl_frame(last_frame);
        print_frame("RESP", last_frame, repeat_count, name);
    }
}

void receiver_init(EventGroupHandle_t events)
{
    assert(events != NULL);  // Validate input
    g_events = events;
    
    // Initialize polling state explicitly
    s_last_level = gpio_get_level(RX_GPIO);
    s_last_time = esp_timer_get_time();
    memset(s_current_frame, 0, sizeof(s_current_frame));
    s_bit_counter = 0;
    s_started = false;
    s_initialized = true;
}

void receiver_poll(void)
{
    // Initialization moved to receiver_init() - should already be done
    assert(s_initialized);  // Verify initialization
    
    uint32_t level = gpio_get_level(RX_GPIO);
    int64_t now = esp_timer_get_time();

    if (level != s_last_level) {
        uint32_t pulse_level = s_last_level;
        uint32_t pulse_duration = (uint32_t)(now - s_last_time);
        s_last_time = now;

        if (pulse_duration < MIN_PULSE_US) {
            s_last_level = level;
            return;
        }

        if (!s_started && pulse_duration < FRAME_GAP_US) {
            s_last_level = level;
            return;
        }
        s_started = true;

        // Frame boundary
        if (pulse_duration >= FRAME_GAP_US) {
            if (s_bit_counter > 0) {
                log_response_frame(s_current_frame, s_bit_counter);
                if (g_events) {
                    xEventGroupSetBits(g_events, EVT_RESPONSE);
                }
            }
            memset(s_current_frame, 0, sizeof(s_current_frame));
            s_bit_counter = 0;
            s_last_level = level;
            return;
        }

        size_t nb_bits = (pulse_duration * 100 + (BIT_US * BIT_TOLERANCE)) / (BIT_US * 100);
        for (size_t i = 0; i < nb_bits; i++) {
            if (s_bit_counter >= FRAME_BITS) {
                ESP_LOGW(TAG, "Frame overrun: received %zu bits, expected max %d", 
                         s_bit_counter + 1, FRAME_BITS);
                break;
            }
            size_t byte_index = bit_to_byte_index(s_bit_counter);
            size_t bit_index = bit_to_bit_index(s_bit_counter);
            if (pulse_level) {
                s_current_frame[byte_index] |= (1 << bit_index);
            }
            s_bit_counter++;
        }

        s_last_level = level;
    }
}
