/*
 * TV Raiser Desk Controller
 * 
 * Controls desk height via half-duplex protocol with handshake.
 * Sends PRESET commands and listens for controller responses.
 * 
 * Protocol:
 * - 48-bit frames (6 bytes)
 * - Pulse width encoding: ~104µs per bit
 * - Frame gap: 7000µs between frames
 * - Half-duplex: send frame → wait for response → next frame
 * 
 * Architecture:
 * - Core 0: RX task (dedicated, high priority) - listens for controller responses
 * - Core 1: TX task (dedicated, high priority) - sends commands and coordinates handshake
 * - No delays in polling loops for maximum efficiency
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "protocol.h"
#include "gpio_hal.h"
#include "transmitter.h"
#include "receiver.h"
#include "button_input.h"

#define TAG "TV_RAISER"

// Global event group for RX/TX synchronization
static EventGroupHandle_t g_events = NULL;

/**
 * RX Task - Runs on Core 0
 * High priority, dedicated core for efficient RX polling
 * Listens for controller responses without any delays
 */
static void rx_task(void *arg)
{
    (void)arg;  // Unused parameter
    ESP_LOGI(TAG, "RX task started on core %d (priority %d)", xPortGetCoreID(), uxTaskPriorityGet(NULL));
    receiver_init(g_events);
    
    // Continuous polling - no delays
    while (1) {
        receiver_poll();
    }
}

/**
 * TX Task - Runs on Core 1
 * High priority, dedicated core for command transmission
 * Coordinates half-duplex handshake with precise timing
 */
static void tx_task(void *arg)
{
    (void)arg;  // Unused parameter
    ESP_LOGI(TAG, "TX task started on core %d (priority %d)", xPortGetCoreID(), uxTaskPriorityGet(NULL));
    
    // Allow RX task time to initialize
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "Starting half-duplex TX loop with timing optimization");
    
    // Main control loop: send frame → wait for response → repeat
    while (1) {
        // Select frame based on button state (non-blocking read)
        const uint8_t *frame = button_select_frame();

        // Clear previous response flag
        xEventGroupClearBits(g_events, EVT_RESPONSE);
        
        // Send frame to controller
        transmitter_send_frame(frame);
        
        // Record time after sending frame (start measuring ACK latency)
        int64_t frame_send_time = esp_timer_get_time();

        // Wait for controller response (40ms timeout)
        // Uses efficient event waiting instead of polling
        EventBits_t bits = xEventGroupWaitBits(g_events, EVT_RESPONSE, true, false, pdMS_TO_TICKS(40));
        
        if (bits & EVT_RESPONSE) {
            // Response received - calculate elapsed time
            int64_t response_time = esp_timer_get_time();
            uint32_t ack_latency_us = (uint32_t)(response_time - frame_send_time);
            
            // Protocol requires 7ms frame gap from frame send to next frame send
            // Wait for remaining time: remaining = FRAME_GAP_US - ack_latency
            if (ack_latency_us < FRAME_GAP_US) {
                uint32_t remaining_us = FRAME_GAP_US - ack_latency_us;
                // Convert to milliseconds for vTaskDelay (rounding up)
                uint32_t remaining_ms = (remaining_us + 999) / 1000;
                vTaskDelay(pdMS_TO_TICKS(remaining_ms));
            }
            // If ack_latency >= FRAME_GAP_US, no delay needed (already waited long enough)
        } else {
            // No controller response within 40ms
            ESP_LOGW(TAG, "No controller response within 40ms");
            // Still wait for minimum frame gap
            vTaskDelay(pdMS_TO_TICKS(7));
        }
        
        // Continue to next iteration
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "TV Raiser Desk Controller (half-duplex protocol)");
    ESP_LOGI(TAG, "TX GPIO %d (to controller RX), RX GPIO %d (controller TX)", TX_GPIO, RX_GPIO);
    ESP_LOGI(TAG, "Buttons: PRESET1 %d, PRESET2 %d (pull high to request)", BTN_PRESET1_GPIO, BTN_PRESET2_GPIO);
    ESP_LOGI(TAG, "Timing: %d µs/bit, %d µs frame gap", BIT_US, FRAME_GAP_US);
    
    // Unbuffered stdout for real-time logging
    setvbuf(stdout, NULL, _IONBF, 0);
    
    // Initialize hardware
    gpio_hal_init();

    // Create event group for RX/TX synchronization
    g_events = xEventGroupCreate();
    configASSERT(g_events);

    // Create RX task on Core 0 (high priority)
    xTaskCreatePinnedToCore(
        rx_task,
        "rx_task",
        2048,
        NULL,
        configMAX_PRIORITIES - 1,  // Highest priority
        NULL,
        0  // Core 0
    );

    // Create TX task on Core 1 (high priority)
    xTaskCreatePinnedToCore(
        tx_task,
        "tx_task",
        2048,
        NULL,
        configMAX_PRIORITIES - 1,  // Highest priority
        NULL,
        1  // Core 1
    );
    
    // Main task suspends itself - RX and TX tasks handle everything
    vTaskSuspend(NULL);
}
